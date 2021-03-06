/* Copyright 2013-2014 IBM Corp.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * 	http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <skiboot.h>
#include <lpc.h>
#include <console.h>
#include <opal.h>
#include <device.h>
#include <interrupts.h>
#include <processor.h>
#include <fsp-elog.h>
#include <trace.h>

DEFINE_LOG_ENTRY(OPAL_RC_UART_INIT, OPAL_PLATFORM_ERR_EVT, OPAL_UART,
		 OPAL_CEC_HARDWARE, OPAL_PREDICTIVE_ERR_GENERAL,
		 OPAL_NA, NULL);

/* UART reg defs */
#define REG_RBR		0
#define REG_THR		0
#define REG_DLL		0
#define REG_IER		1
#define REG_DLM		1
#define REG_FCR		2
#define REG_IIR		2
#define REG_LCR		3
#define REG_MCR		4
#define REG_LSR		5
#define REG_MSR		6
#define REG_SCR		7

#define LSR_DR   0x01  /* Data ready */
#define LSR_OE   0x02  /* Overrun */
#define LSR_PE   0x04  /* Parity error */
#define LSR_FE   0x08  /* Framing error */
#define LSR_BI   0x10  /* Break */
#define LSR_THRE 0x20  /* Xmit holding register empty */
#define LSR_TEMT 0x40  /* Xmitter empty */
#define LSR_ERR  0x80  /* Error */

#define LCR_DLAB 0x80  /* DLL access */

static uint32_t uart_base;
static bool has_irq, irq_disabled;

/*
 * We implement a simple buffer to buffer input data as some bugs in
 * Linux make it fail to read fast enough after we get an interrupt.
 *
 * We use it on non-interrupt operations as well while at it because
 * it doesn't cost us much and might help in a few cases where Linux
 * is calling opal_poll_events() but not actually reading.
 *
 * Most of the time I expect we'll flush it completely to Linux into
 * it's tty flip buffers so I don't bother with a ring buffer.
 */
#define IN_BUF_SIZE	0x1000
static uint8_t	*in_buf;
static uint32_t	in_count;

static void uart_trace(u8 ctx, u8 cnt, u8 irq_state, u8 in_count)
{
	union trace t;

	t.uart.ctx = ctx;
	t.uart.cnt = cnt;
	t.uart.irq_state = irq_state;
	t.uart.in_count = in_count;
	trace_add(&t, TRACE_UART, sizeof(struct trace_uart));
}

static inline uint8_t uart_read(unsigned int reg)
{
	return lpc_inb(uart_base + reg);
}

static inline void uart_write(unsigned int reg, uint8_t val)
{
	lpc_outb(val, uart_base + reg);
}

static size_t uart_con_write(const char *buf, size_t len)
{
	size_t written = 0;

	while(written < len) {
		while ((uart_read(REG_LSR) & LSR_THRE) == 0) {
			int i = 0;

			/* Give the simulator some breathing space */
			for (; i < 1000; ++i)
				smt_very_low();
		}
		smt_medium();
		uart_write(REG_THR, buf[written++]);
	};

	return written;
}

/* Must be called with console lock held */
static void uart_read_to_buffer(void)
{
	/* As long as there is room in the buffer */
	while(in_count < IN_BUF_SIZE) {
		/* Read status register */
		uint8_t lsr = uart_read(REG_LSR);

		/* Nothing to read ... */
		if ((lsr & LSR_DR) == 0)
			break;

		/* Read and add to buffer */
		in_buf[in_count++] = uart_read(REG_RBR);
	}

	if (!has_irq)
		return;

	/* If the buffer is full disable the interrupt */
	if (in_count == IN_BUF_SIZE) {
		if (!irq_disabled)
			uart_write(REG_IER, 0x00);
		irq_disabled = true;
	} else {
		/* Otherwise, enable it */
		if (irq_disabled) 
			uart_write(REG_IER, 0x01);
		irq_disabled = false;
	}
}

/* This is called with the console lock held */
static size_t uart_con_read(char *buf, size_t len)
{
	size_t read_cnt = 0;
	uint8_t lsr = 0;

	if (!in_buf)
		return 0;

	/* Read from buffer first */
	if (in_count) {
		read_cnt = in_count;
		if (len < read_cnt)
			read_cnt = len;
		memcpy(buf, in_buf, read_cnt);
		len -= read_cnt;
		if (in_count != read_cnt)
			memmove(in_buf, in_buf + read_cnt, in_count - read_cnt);
		in_count -= read_cnt;
	}

	/*
	 * If there's still room in the user buffer, read from the UART
	 * directly
	 */
	while(len) {
		lsr = uart_read(REG_LSR);
		if ((lsr & LSR_DR) == 0)
			break;
		buf[read_cnt++] = uart_read(REG_RBR);
		len--;
	}

	/* Finally, flush whatever's left in the UART into our buffer */
	uart_read_to_buffer();
	
	/* Adjust the OPAL event */
	if (in_count)
		opal_update_pending_evt(OPAL_EVENT_CONSOLE_INPUT,
					OPAL_EVENT_CONSOLE_INPUT);
	else
		opal_update_pending_evt(OPAL_EVENT_CONSOLE_INPUT, 0);

	uart_trace(TRACE_UART_CTX_READ, read_cnt, irq_disabled, in_count);

	return read_cnt;
}

static struct con_ops uart_con_driver = {
	.read = uart_con_read,
	.write = uart_con_write
};

bool uart_console_poll(void)
{
	if (!in_buf)
		return false;

	/* Grab what's in the UART and stash it into our buffer */
	uart_read_to_buffer();

	uart_trace(TRACE_UART_CTX_POLL, 0, irq_disabled, in_count);

	return !!in_count;
}

void uart_irq(void)
{
	if (!in_buf)
		return;

	/* This needs locking vs read() */
	lock(&con_lock);

	/* Grab what's in the UART and stash it into our buffer */
	uart_read_to_buffer();

	/* Set the event if the buffer has anything in it */
	if (in_count)
		opal_update_pending_evt(OPAL_EVENT_CONSOLE_INPUT,
					OPAL_EVENT_CONSOLE_INPUT);

	uart_trace(TRACE_UART_CTX_IRQ, 0, irq_disabled, in_count);
	unlock(&con_lock);
}

static bool uart_init_hw(unsigned int speed, unsigned int clock)
{
	unsigned int dll = (clock / 16) / speed;

	/* Clear line control */
	uart_write(REG_LCR, 0x00);

	/* Check if the UART responds */
	uart_write(REG_IER, 0x01);
	if (uart_read(REG_IER) != 0x01)
		goto detect_fail;
	uart_write(REG_IER, 0x00);
	if (uart_read(REG_IER) != 0x00)
		goto detect_fail;

	uart_write(REG_LCR, LCR_DLAB);
	uart_write(REG_DLL, dll & 0xff);
	uart_write(REG_DLM, dll >> 8);
	uart_write(REG_LCR, 0x03); /* 8N1 */
	uart_write(REG_MCR, 0x03); /* RTS/DTR */
	uart_write(REG_FCR, 0x07); /* clear & en. fifos */
	return true;

 detect_fail:
	prerror("UART: Presence detect failed !\n");
	return false;
}

void uart_init(bool enable_interrupt)
{
	const struct dt_property *prop;
	struct dt_node *n;
	char *path __unused;
	uint32_t irqchip, irq;

	if (!lpc_present())
		return;

	/* We support only one */
	n = dt_find_compatible_node(dt_root, NULL, "ns16550");
	if (!n)
		return;

	/* Get IO base */
	prop = dt_find_property(n, "reg");
	if (!prop) {
		log_simple_error(&e_info(OPAL_RC_UART_INIT),
				"UART: Can't find reg property\n");
		return;
	}
	if (dt_property_get_cell(prop, 0) != OPAL_LPC_IO) {
		log_simple_error(&e_info(OPAL_RC_UART_INIT),
				"UART: Only supports IO addresses\n");
		return;
	}
	uart_base = dt_property_get_cell(prop, 1);

	if (!uart_init_hw(dt_prop_get_u32(n, "current-speed"),
			  dt_prop_get_u32(n, "clock-frequency"))) {
		prerror("UART: Initialization failed\n");
		dt_add_property_strings(n, "status", "bad");
		return;
	}

	/*
	 * Mark LPC used by the console (will mark the relevant
	 * locks to avoid deadlocks when flushing the console)
	 */
	lpc_used_by_console();

	/* Install console backend for printf() */
	set_console(&uart_con_driver);

	/* Setup the interrupts properties since HB couldn't do it */
	irqchip = dt_prop_get_u32(n, "ibm,irq-chip-id");
	irq = get_psi_interrupt(irqchip) + P8_IRQ_PSI_HOST_ERR;
	printf("UART: IRQ connected to chip %d, irq# is 0x%x\n", irqchip, irq);
	if (enable_interrupt) {
		dt_add_property_cells(n, "interrupts", irq);
		dt_add_property_cells(n, "interrupt-parent", get_ics_phandle());
	}

	if (dummy_console_enabled()) {
		/*
		 * If the dummy console is enabled, we mark the UART as
		 * reserved since we don't want the kernel to start using it
		 * with its own 8250 driver
		 */
		dt_add_property_strings(n, "status", "reserved");

		/*
		 * If the interrupt is enabled, turn on RX interrupts (and
		 * only these for now
		 */
		if (enable_interrupt) {
			uart_write(REG_IER, 0x01);
			has_irq = true;
			irq_disabled = false;
		}

		/* Allocate an input buffer */
		in_buf = zalloc(IN_BUF_SIZE);
		printf("UART: Enabled as OS console\n");
	} else {
		/* Else, we expose it as our chosen console */
		dt_add_property_strings(n, "status", "ok");
		path = dt_get_path(n);
		dt_add_property_string(dt_chosen, "linux,stdout-path", path);
		free(path);
		printf("UART: Enabled as OS pass-through\n");
	}
}
