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

#ifndef __XSCOM_H
#define __XSCOM_H

#include <stdint.h>
#include <processor.h>
#include <cpu.h>

/*
 * SCOM "partID" definitions:
 *
 * All Ids are 32-bits long, top nibble is reserved for the
 * 'type' field:
 *     0x0 = Processor Chip
 *     0x8 = Memory Buffer (Centaur) Chip
 *     0x4 = EX/Core Chiplet
 *
 * Processor Chip = Logical Fabric Id = PIR>>7
 *     0b0000.0000.0000.0000.0000.0000.00NN.NCCC
 *     N=Node, C=Chip
 * Centaur Chip = Associated Processor Chip with memory channel
 * appended and flag set
 *     0b1000.0000.0000.0000.0000.00NN.NCCC.MMMM
 *     N=Node, C=Chip, M=Memory Channel
 * Processor EX/Core chiplet = PIR >> 3 with flag set
 *     0b0100.0000.0000.0000.0000.00NN.NCCC.PPPP
 *     N=Node, C=Chip, P=Processor core
 */

/*
 * SCOM Address definition extracted from HWPs for documentation
 * purposes
 *
 * "Normal" (legacy) format
 *
 *            111111 11112222 22222233 33333333 44444444 44555555 55556666
 * 01234567 89012345 67890123 45678901 23456789 01234567 89012345 67890123
 * -------- -------- -------- -------- -------- -------- -------- --------
 * 00000000 00000000 00000000 00000000 0MCCCCCC ????PPPP 00LLLLLL LLLLLLLL
 *                                      ||          |    |
 *                                      ||          |    `-> Local Address*
 *                                      ||          |
 *                                      ||          `-> Port
 *                                      ||
 *                                      |`-> Chiplet ID**
 *                                      |
 *                                      `-> Multicast bit
 *
 *  * Local address is composed of "00" + 4-bit ring + 10-bit ID
 *    The 10-bit ID is usually 4-bit sat_id and 6-bit reg_id
 *
 * ** Chiplet ID turns into multicast operation type and group number
 *    if the multicast bit is set
 *
 * "Indirect" format
 *
 *
 *            111111 11112222 22222233 33333333 44444444 44555555 55556666
 * 01234567 89012345 67890123 45678901 23456789 01234567 89012345 67890123
 * -------- -------- -------- -------- -------- -------- -------- --------
 * 10000000 0000IIII IIIIIGGG GGGLLLLL 0MCCCCCC ????PPPP 00LLLLLL LLLLLLLL
 *              |         |      |      ||          |    |
 *              |         |      |      ||          |    `-> Local Address*
 *              |         |      |      ||          |
 *              |         |      |      ||          `-> Port
 *              |         |      |      ||
 *              |         |      |      |`-> Chiplet ID**
 *              |         |      |      |
 *              |         |      |      `-> Multicast bit
 *              |         |      |
 *              |         |      `-> Lane ID
 *              |         |
 *              |         `-> RX or TX Group ID
 *              |
 *              `-> Indirect Register Address
 *
 *  * Local address is composed of "00" + 4-bit ring + 4-bit sat_id + "111111"
 *
 * ** Chiplet ID turns into multicast operation type and group number
 *    if the multicast bit is set
 */

/*
 * Generate a local address from a given ring/satellite/offset
 * combination:
 *
 *     Ring    Satelite     offset
 *  +---------+---------+-------------+
 *  |    4    |    4    |     6       |
 *  +---------+---------+-------------+
 */
#define XSCOM_SAT(_r, _s, _o)	\
	(((_r) << 10) | ((_s) << 6) | (_o))

/*
 * Additional useful definitions
 */
#define P8_EX_PCB_SLAVE_BASE	0x100F0000

#define XSCOM_ADDR_P8_EX_SLAVE(core, offset) \
     (P8_EX_PCB_SLAVE_BASE | (((core) & 0xF) << 24) | ((offset) & 0xFFFF))

#define XSCOM_ADDR_P8_EX(core, addr) \
		((((core) & 0xF) << 24) | (addr))

/* Per core power mgt registers */
#define PM_OHA_MODE_REG		0x1002000D

/* EX slave per-core power mgt slave regisers */
#define EX_PM_GP0			0x0100
#define EX_PM_GP1			0x0103
#define EX_PM_SPECIAL_WAKEUP_FSP	0x010B
#define EX_PM_SPECIAL_WAKEUP_OCC	0x010C
#define EX_PM_SPECIAL_WAKEUP_PHYP	0x010D
#define EX_PM_IDLE_STATE_HISTORY_PHYP	0x0110
#define EX_PM_IDLE_STATE_HISTORY_FSP	0x0111
#define EX_PM_IDLE_STATE_HISTORY_OCC	0x0112
#define EX_PM_IDLE_STATE_HISTORY_PERF	0x0113
#define EX_PM_CORE_PFET_VRET		0x0130
#define EX_PM_CORE_ECO_VRET		0x0150
#define EX_PM_PPMSR			0x0153
#define EX_PM_PPMCR			0x0159

/* Power mgt bits in GP0 */
#define EX_PM_GP0_SPECIAL_WAKEUP_DONE		PPC_BIT(31)

/* Power mgt settings in GP1 */
#define EX_PM_SETUP_GP1_FAST_SLEEP		0xD820000000000000ULL
#define EX_PM_SETUP_GP1_DEEP_SLEEP		0x2420000000000000ULL
#define EX_PM_SETUP_GP1_PM_SPR_OVERRIDE_EN	0x0010000000000000ULL
#define EX_PM_SETUP_GP1_DPLL_FREQ_OVERRIDE_EN	0x0020000000000000ULL

/* Fields in history regs */
#define EX_PM_IDLE_ST_HIST_PM_STATE_MASK	PPC_BITMASK(0, 2)
#define EX_PM_IDLE_ST_HIST_PM_STATE_LSH		PPC_BITLSHIFT(2)


/*
 * Error handling:
 *
 * Error codes TBD, 0 = success
 */

/* Targeted SCOM access */
extern int xscom_read(uint32_t partid, uint64_t pcb_addr, uint64_t *val);
extern int xscom_write(uint32_t partid, uint64_t pcb_addr, uint64_t val);

/* This chip SCOM access */
extern int xscom_readme(uint64_t pcb_addr, uint64_t *val);
extern int xscom_writeme(uint64_t pcb_addr, uint64_t val);
extern void xscom_init(void);

/* Mark XSCOM lock as being in console path */
extern void xscom_used_by_console(void);

#endif /* __XSCOM_H */
