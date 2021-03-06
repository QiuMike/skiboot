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
#include <opal.h>
#ifndef __ELOG_H
#define __ELOG_H

#define ELOG_TYPE_PEL			0
#define MAX_RETRIES			3

/* Component IDs */
/* In PEL error log format, Creator ID is hypervisor
 * But we can have various component ID to distinguish
 * which component in hypervisor is reporting the error
 * This is 2 bytes long,
 *	first byte corresponds to Component IDs
 *	Second byte is reserved for the Reason code.
 * Component ID is mapped to readable 4-digit ascii
 * character name in FSP and displayed.
 */
/* SAPPHIRE components */
#define OPAL_CODEUPDATE			0x1000
#define OPAL_CONSOLE			0x2000
#define OPAL_CEC			0x3000
#define OPAL_CHIP			0x4000
#define OPAL_ELOG			0x5000
#define OPAL_NVRAM			0x6000
#define OPAL_RTC			0x7000
#define OPAL_SURVEILLANCE		0x8000
#define OPAL_SYSPARAM			0x9000
#define OPAL_LPC			0xa000
#define OPAL_UART			0xb000
#define OPAL_OCC			0xc000
#define OPAL_OP_PANEL			0xd000
#define OPAL_PHB3			0xe000
#define OPAL_PSI			0xf000
#define OPAL_VPD			0x1000
#define OPAL_XSCOM			0x1100
#define OPAL_PCI			0x1200
#define OPAL_MISC			0x1300
#define OPAL_ATTN			0x1400
#define OPAL_MEM_ERR			0x1500
#define OPAL_CENTAUR			0x1600
#define OPAL_MFSI			0x1700
#define OPAL_DUMP			0x1800
#define OPAL_LED			0x1900
#define OPAL_SENSOR			0x2000
#define OPAL_SLW			0x2100
#define OPAL_FSP			0x2200

enum opal_reasoncode {
/* code update */
	OPAL_RC_CU_FLASH        = OPAL_CODEUPDATE | 0x10,
	OPAL_RC_CU_INIT         = OPAL_CODEUPDATE | 0x11,
	OPAL_RC_CU_SG_LIST      = OPAL_CODEUPDATE | 0x12,
	OPAL_RC_CU_COMMIT	= OPAL_CODEUPDATE | 0x13,
	OPAL_RC_CU_MSG		= OPAL_CODEUPDATE | 0x14,
	OPAL_RC_CU_NOTIFY       = OPAL_CODEUPDATE | 0x15,
	OPAL_RC_CU_MARKER_LID	= OPAL_CODEUPDATE | 0x16,
/* NVRAM */
	OPAL_RC_NVRAM_INIT      = OPAL_NVRAM | 0x10,
	OPAL_RC_NVRAM_OPEN	= OPAL_NVRAM | 0x11,
	OPAL_RC_NVRAM_SIZE      = OPAL_NVRAM | 0x12,
	OPAL_RC_NVRAM_WRITE     = OPAL_NVRAM | 0x13,
	OPAL_RC_NVRAM_READ      = OPAL_NVRAM | 0x14,
/* CENTAUR */
	OPAL_RC_CENTAUR_INIT    = OPAL_CENTAUR | 0x10,
	OPAL_RC_CENTAUR_RW_ERR  = OPAL_CENTAUR | 0x11,
/* MFSI */
	OPAL_RC_MFSI_RW_ERR     = OPAL_MFSI | 0x10,
/* UART */
	OPAL_RC_UART_INIT       = OPAL_UART | 0x10,
/* OCC */
	OPAL_RC_OCC_RESET       = OPAL_OCC | 0x10,
	OPAL_RC_OCC_LOAD        = OPAL_OCC | 0x11,
	OPAL_RC_OCC_PSTATE_INIT = OPAL_OCC | 0x12,
/* RTC	*/
	OPAL_RC_RTC_READ	= OPAL_RTC | 0x10,
	OPAL_RC_RTC_TOD		= OPAL_RTC | 0x11,
/* SURVEILLANCE */
	OPAL_RC_SURVE_INIT      = OPAL_SURVEILLANCE | 0x10,
	OPAL_RC_SURVE_STATUS	= OPAL_SURVEILLANCE | 0x11,
/* SYSPARAM */
	OPAL_RC_SYSPARM_INIT    = OPAL_SYSPARAM | 0x10,
	OPAL_RC_SYSPARM_MSG     = OPAL_SYSPARAM | 0x11,
/* LPC */
	OPAL_RC_LPC_READ        = OPAL_LPC | 0x10,
	OPAL_RC_LPC_WRITE       = OPAL_LPC | 0x11,
/* OP_PANEL */
	OPAL_RC_PANEL_WRITE     = OPAL_OP_PANEL | 0x10,
/* PSI */
	OPAL_RC_PSI_INIT        = OPAL_PSI | 0x10,
	OPAL_RC_PSI_IRQ_RESET   = OPAL_PSI | 0x11,
/* XSCOM */
	OPAL_RC_XSCOM_RW		= OPAL_XSCOM | 0x10,
	OPAL_RC_XSCOM_INDIRECT_RW	= OPAL_XSCOM | 0x11,
	OPAL_RC_XSCOM_RESET		= OPAL_XSCOM | 0x12,
/* PCI */
	OPAL_RC_PCI_INIT_SLOT   = OPAL_PCI | 0x10,
	OPAL_RC_PCI_ADD_SLOT    = OPAL_PCI | 0x11,
	OPAL_RC_PCI_SCAN        = OPAL_PCI | 0x12,
	OPAL_RC_PCI_RESET_PHB   = OPAL_PCI | 0x10,
/* ATTN */
	OPAL_RC_ATTN		= OPAL_ATTN | 0x10,
/* MEM_ERR */
	OPAL_RC_MEM_ERR_RES	= OPAL_MEM_ERR | 0x10,
	OPAL_RC_MEM_ERR_DEALLOC	= OPAL_MEM_ERR | 0x11,
/* DUMP */
	OPAL_RC_DUMP_INIT	= OPAL_DUMP | 0x10,
	OPAL_RC_DUMP_LIST	= OPAL_DUMP | 0x11,
	OPAL_RC_DUMP_ACK	= OPAL_DUMP | 0x12,
	OPAL_RC_DUMP_MDST_INIT	= OPAL_DUMP | 0x13,
	OPAL_RC_DUMP_MDST_UPDATE = OPAL_DUMP | 0x14,
/* LED	*/
	OPAL_RC_LED_SPCN	= OPAL_LED | 0x10,
	OPAL_RC_LED_BUFF	= OPAL_LED | 0x11,
	OPAL_RC_LED_LC		= OPAL_LED | 0x12,
	OPAL_RC_LED_STATE	= OPAL_LED | 0x13,
	OPAL_RC_LED_SUPPORT	= OPAL_LED | 0x14,
/* SENSOR */
	OPAL_RC_SENSOR_INIT	= OPAL_SENSOR | 0x10,
	OPAL_RC_SENSOR_READ	= OPAL_SENSOR | 0x11,
	OPAL_RC_SENSOR_ASYNC_COMPLETE
				= OPAL_SENSOR | 0x12,
/* SLW */
	OPAL_RC_SLW_INIT	= OPAL_SLW | 0x10,
	OPAL_RC_SLW_SET		= OPAL_SLW | 0x11,
	OPAL_RC_SLW_GET		= OPAL_SLW | 0x12,
	OPAL_RC_SLW_REG		= OPAL_SLW | 0x13,
/* FSP	*/
	OPAL_RC_FSP_POLL_TIMEOUT
				= OPAL_FSP | 0x10,
};

/* Data Structures for PEL data. */

#define PRIVATE_HEADER_SECTION_SIZE		48
#define USER_HEADER_SECTION_SIZE		24
#define SRC_SECTION_SIZE			80
#define SRC_SUBSECTION_SIZE			 4
#define SRC_LENGTH				72
#define OPAL_MAX_SRC_BYTES			32
#define EXTENDED_HEADER_SECTION_SIZE		76
#define MTMS_SECTION_SIZE			28
#define IO_EVENT_SECTION_SIZE			16

#define OPAL_ELOG_VERSION		1
#define OPAL_ELOG_SST			0
#define OPAL_SRC_MAX_WORD_COUNT		8

#define OPAL_SRC_FORMAT         0x80
#define OPAL_FAILING_SUBSYSTEM  0x82

#define OPAL_SYS_MODEL_LEN      8
#define OPAL_SYS_SERIAL_LEN     12
#define OPAL_VER_LEN            16
#define OPAL_SYMPID_LEN         80
#define OPAL_RC_NONE		0

#define OPAL_IO_MAX_RPC_DATA	216
#define OPAL_SRC_SEC_VER	0x02
#define OPAL_EXT_HRD_VER	0x01

/* Error log reporting action */
#define ERRL_ACTION_REPORT     0x2000
#define ERRL_ACTION_NONE       0x0000

enum elogSectionId {
	ELOG_SID_PRIVATE_HEADER                 = 0x5048, /* PH */
	ELOG_SID_USER_HEADER                    = 0x5548, /* UH */
	ELOG_SID_EXTENDED_HEADER                = 0x4548, /* EH */
	ELOG_SID_PRIMARY_SRC                    = 0x5053, /* PS */
	ELOG_SID_MACHINE_TYPE                   = 0x4D54, /* MT */
	ELOG_SID_SECONDARY_SRC                  = 0x5353, /* SS */
	ELOG_SID_CALL_HOME                      = 0x4348, /* CH */
	ELOG_SID_DUMP_LOCATOR                   = 0x4448, /* DH */
	ELOG_SID_SOFTWARE_ERROR			= 0x5357, /* SW */
	ELOG_SID_PARTITION                      = 0x4C50, /* LP */
	ELOG_SID_LOGICAL_RESOURCE               = 0x4C52, /* LR */
	ELOG_SID_HMC_ID                         = 0x484D, /* HM */
	ELOG_SID_EPOW                           = 0x4550, /* EP */
	ELOG_SID_IO_EVENT                       = 0x4945, /* IE */
	ELOG_SID_MFG_INFORMATION                = 0x4D49, /* MI */
	ELOG_SID_USER_DEFINED                   = 0x5544  /* UD */
};

struct opal_v6_header {
	enum elogSectionId id:16;	/* section id */
	uint16_t    length;		/* section length */
	uint8_t    version;		/* section version */
	uint8_t    subtype;		/* section sub-type id */
	uint16_t   component_id;	/* component id of section creator */
};

/* opal_srctype */
#define OPAL_SRC_TYPE_ERROR 0xBB

#define OPAL_CID_SAPPHIRE	'K'	/* creator ID for sapphire log */
#define OPAL_CID_POWERNV	'P'	/* creator ID for powernv log */

/* Origin of error, elog_origin */
#define ORG_SAPPHIRE	1
#define ORG_POWERNV	2

/*struct opal_private head section_ */
struct opal_private_header_section {

	struct opal_v6_header v6header;
	uint32_t create_date;
	uint32_t create_time;
	uint32_t commit_date;
	uint32_t commit_time;

	uint32_t creator_id:8;		/* subsystem component id */
	uint32_t reserved_0:16;
	uint32_t section_count:8;	/* number of sections in log */
	uint32_t reserved_1;
	uint32_t creator_subid_hi;
	uint32_t creator_subid_lo;
	uint32_t plid;			/* platform log id */
	uint32_t log_entry_id;		/* Unique log entry id */
};

/* opal user header section */
struct opal_user_header_section {

	struct opal_v6_header v6header;

	uint8_t subsystem_id;	/* subsystem id */
	uint8_t event_scope;
	uint8_t event_severity;
	uint8_t event_type;	/* error/event severity */

	uint32_t reserved_0;
	uint16_t reserved_1;
	uint16_t action_flags;	/* error action code */
	uint32_t reserved_2;
};

struct opal_src_section {
	struct opal_v6_header v6header;
	uint8_t		version;
	uint8_t		flags;
	uint8_t		reserved_0;
	uint8_t		wordcount;
	uint16_t	reserved_1;
	uint16_t	srclength;
	uint32_t	hexwords[OPAL_SRC_MAX_WORD_COUNT];
	char		srcstring[OPAL_MAX_SRC_BYTES];
};

struct opal_extended_header_section {
	struct	opal_v6_header v6header;
	char	model[OPAL_SYS_MODEL_LEN];
	char	serial_no[OPAL_SYS_SERIAL_LEN];
	char	opal_release_version[OPAL_VER_LEN];
	char	opal_subsys_version[OPAL_VER_LEN];
	uint16_t reserved_0;
	uint32_t extended_header_date;
	uint32_t extended_header_time;
	uint16_t reserved_1;
	uint8_t reserved_2;
	uint8_t opal_symid_len;
	char	opalsymid[OPAL_SYMPID_LEN];
};

/* opal MTMS section */
struct opal_mtms_section {
	struct opal_v6_header v6header;
	char        model[OPAL_SYS_MODEL_LEN];
	char        serial_no[OPAL_SYS_SERIAL_LEN];
};

/* User defined section */
struct opal_user_section {
	struct opal_v6_header v6header;
	char dump[1];
};

struct opal_err_info {
	uint32_t reason_code;
	uint8_t err_type;
	uint16_t cmp_id;
	uint8_t subsystem;
	uint8_t sev;
	uint8_t event_subtype;
	void (*call_out)(struct opal_errorlog *buf, void *data, uint16_t size);
};

#define DEFINE_LOG_ENTRY(reason, type, id, subsys,			\
severity, subtype, callout_func) struct opal_err_info err_##reason =	\
{ .reason_code = reason, .err_type = type, .cmp_id = id,		\
.subsystem = subsys, .sev = severity, .event_subtype = subtype,		\
.call_out = callout_func }

#define e_info(reason_code) err_##reason_code

struct opal_errorlog *opal_elog_create(struct opal_err_info *e_info);

int opal_elog_update_user_dump(struct opal_errorlog *buf, unsigned char *data,
						uint32_t tag, uint16_t size);

int elog_fsp_commit(struct opal_errorlog *buf);

/* This is wrapper around the error log function, which creates
 * and commits the error to FSP.
 * Used for simple error logging
 */
void log_simple_error(struct opal_err_info *e_info, const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
void log_error(struct opal_err_info *e_info, void *data, uint16_t size,
		const char *fmt, ...) __attribute__ ((format (printf, 4, 5)));

#endif /* __ELOG_H */
