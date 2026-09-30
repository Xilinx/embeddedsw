/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XI3CPS_PR_H	/* prevent circular inclusions */
#define XI3CPS_PR_H

/*****************************************************************************/
/**
*
* @file xi3cpsx_pr.h
* @brief Defines I3C private command codes and framing helpers.
*
* This file contains the following:
* Common command codes (CCC)
* Error types
* Command structure/framing
* Characteristics registers
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------.
* 1.00  sd  06/10/22 First release
* 1.4   gm  10/07/24 Added DBP and slave transmit macros.
* 1.8   vn  07/30/26 Fix Doxygen warnings
* 1.8   tb  09/22/26 Align Doxygen comments with AMD documentation standards
*
* </pre>
*
******************************************************************************/

/**
* @addtogroup i3cpsx_api I3CPSX APIs
* @{
*/

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_assert.h"

#define BIT(N)		(1U << N)	/**< Single-bit mask helper. */

/** Typical I3C SCL rate in Hz. */
#define I3C_BUS_TYP_I3C_SCL_RATE	12500000
/** I2C Fast-mode Plus SCL rate in Hz. */
#define I3C_BUS_I2C_FM_PLUS_SCL_RATE	1000000
/** I2C Fast-mode SCL rate in Hz. */
#define I3C_BUS_I2C_FM_SCL_RATE		400000
/** Minimum open-drain SCL low time in ns. */
#define I3C_BUS_TLOW_OD_MIN_NS		200

/***********************Common command codes (CCC)****************************/

/* I3C CCC (Common Command Codes) related definitions */
/** Direct (unicast) CCC selector bit. */
#define I3C_CCC_DIRECT			BIT(7)

/**
 * @brief Builds a broadcast or direct CCC value.
 *
 * @note
 * C-style signature
 *	u32 I3C_CCC_ID(u32 id, u32 broadcast)
 */
#define I3C_CCC_ID(id, broadcast)	\
	((id) | ((broadcast) ? 0 : I3C_CCC_DIRECT))

/* Commands valid in both broadcast and unicast modes */
/** Enable events CCC. */
#define I3C_CCC_ENEC(broadcast)		I3C_CCC_ID(0x0, broadcast)
/** Disable events CCC. */
#define I3C_CCC_DISEC(broadcast)	I3C_CCC_ID(0x1, broadcast)
/** Enter activity state CCC. */
#define I3C_CCC_ENTAS(as, broadcast)	I3C_CCC_ID(0x2 + (as), broadcast)
/** Reset dynamic address assignment CCC. */
#define I3C_CCC_RSTDAA(broadcast)	I3C_CCC_ID(0x6, broadcast)
/** Set maximum write length CCC. */
#define I3C_CCC_SETMWL(broadcast)	I3C_CCC_ID(0x9, broadcast)
/** Set maximum read length CCC. */
#define I3C_CCC_SETMRL(broadcast)	I3C_CCC_ID(0xa, broadcast)
/** Set exchange timing CCC. */
#define I3C_CCC_SETXTIME(broadcast)	((broadcast) ? 0x28 : 0x98)
/** Vendor CCC helper. */
#define I3C_CCC_VENDOR(id, broadcast)	((id) + ((broadcast) ? 0x61 : 0xe0))

/* Broadcast-only commands */
/** Enter dynamic address assignment CCC. */
#define I3C_CCC_ENTDAA			I3C_CCC_ID(0x7, TRUE)
/** Define list of slaves CCC. */
#define I3C_CCC_DEFSLVS			I3C_CCC_ID(0x8, TRUE)
/** Enter test mode CCC. */
#define I3C_CCC_ENTTM			I3C_CCC_ID(0xb, TRUE)
/** Enter HDR mode CCC. */
#define I3C_CCC_ENTHDR(x)		I3C_CCC_ID(0x20 + (x), TRUE)

/* Unicast-only commands */
/** Set dynamic address from static address CCC. */
#define I3C_CCC_SETDASA			I3C_CCC_ID(0x7, FALSE)
/** Set new dynamic address CCC. */
#define I3C_CCC_SETNEWDA		I3C_CCC_ID(0x8, false)
/** Get maximum write length CCC. */
#define I3C_CCC_GETMWL			I3C_CCC_ID(0xb, FALSE)
/** Get maximum read length CCC. */
#define I3C_CCC_GETMRL			I3C_CCC_ID(0xc, FALSE)
/** Get provisioned ID CCC. */
#define I3C_CCC_GETPID			I3C_CCC_ID(0xd, FALSE)
/** Get bus characteristics register CCC. */
#define I3C_CCC_GETBCR			I3C_CCC_ID(0xe, FALSE)
/** Get device characteristics register CCC. */
#define I3C_CCC_GETDCR			I3C_CCC_ID(0xf, FALSE)
/** Get device status CCC. */
#define I3C_CCC_GETSTATUS		I3C_CCC_ID(0x10, FALSE)
/** Get accept mastership CCC. */
#define I3C_CCC_GETACCMST		I3C_CCC_ID(0x11, FALSE)
/** Set bridge targets CCC. */
#define I3C_CCC_SETBRGTGT		I3C_CCC_ID(0x13, FALSE)
/** Get maximum data speed CCC. */
#define I3C_CCC_GETMXDS			I3C_CCC_ID(0x14, FALSE)
/** Get HDR capability CCC. */
#define I3C_CCC_GETHDRCAP		I3C_CCC_ID(0x15, FALSE)
/** Get exchange timing CCC. */
#define I3C_CCC_GETXTIME		I3C_CCC_ID(0x19, FALSE)

/** Secondary master interrupt event bit. */
#define I3C_CCC_EVENT_SIR		BIT(0)
/** Master-request event bit. */
#define I3C_CCC_EVENT_MR		BIT(1)
#define I3C_CCC_EVENT_HJ		BIT(3)	/**< Hot-Join event bit. */
/*FIXME */
/**
 * @brief Builds an inclusive bit-field mask for bits h down to l.
 *
 * @note
 * C-style signature
 *	u32 GENMASK(u32 h, u32 l)
 */
#define GENMASK(h, l)			(~(u32)0 - ( (u32)1 << l) + 1) & \
	(~(u32)0 >> (32 - 1 - h))

/** Terminate-on-completion command bit. */
#define COMMAND_PORT_TOC		BIT(30)
/** Read-transfer command bit. */
#define COMMAND_PORT_READ_TRANSFER	BIT(28)
/** Short-data argument present bit. */
#define COMMAND_PORT_SDAP		BIT(27)
/** Response-on-completion command bit. */
#define COMMAND_PORT_ROC		BIT(26)
/** Defining-byte present command bit. */
#define COMMAND_PORT_DBP		BIT(25)
/** Transfer speed field helper. */
#define COMMAND_PORT_SPEED(x)		((x) << 21)
/** Device-index field helper. */
#define COMMAND_PORT_DEV_INDEX(x)	((x) << 16)
#define COMMAND_PORT_CP			BIT(15)	/**< Command-present bit. */
/** CCC/command field helper. */
#define COMMAND_PORT_CMD(x)		((x) << 7)
/** Transaction ID field helper. */
#define COMMAND_PORT_TID(x)		((x) << 3)

/** Argument data-length field helper. */
#define COMMAND_PORT_ARG_DATA_LEN(x)	((x) << 16)
/** Maximum argument data length. */
#define COMMAND_PORT_ARG_DATA_LEN_MAX	65536
/** Slave-transmit argument value. */
#define COMMAND_PORT_SLAVE_TRANSMIT	0x0
/** Transfer-argument identifier. */
#define COMMAND_PORT_TRANSFER_ARG	0x01

/** Short-data byte 3 field helper. */
#define COMMAND_PORT_SDA_DATA_BYTE_3(x)	((x) << 24)
/** Short-data byte 2 field helper. */
#define COMMAND_PORT_SDA_DATA_BYTE_2(x)	((x) << 16)
/** Short-data byte 1 field helper. */
#define COMMAND_PORT_SDA_DATA_BYTE_1(x)	((x) << 8)
/** Short-data byte 3 strobe bit. */
#define COMMAND_PORT_SDA_BYTE_STRB_3	BIT(5)
/** Short-data byte 2 strobe bit. */
#define COMMAND_PORT_SDA_BYTE_STRB_2	BIT(4)
/** Short-data byte 1 strobe bit. */
#define COMMAND_PORT_SDA_BYTE_STRB_1	BIT(3)
/** Short-data argument identifier. */
#define COMMAND_PORT_SHORT_DATA_ARG	0x02

/** Device-count field helper. */
#define COMMAND_PORT_DEV_COUNT(x)	((x) << 21)
/** Address-assignment command identifier. */
#define COMMAND_PORT_ADDR_ASSGN_CMD	0x03

/** Response error-status field helper. */
#define RESPONSE_PORT_ERR_STATUS(x)	(((x) & GENMASK(31, 28)) >> 28)
#define RESPONSE_NO_ERROR		0	/**< Response success status. */
/** Response CRC error status. */
#define RESPONSE_ERROR_CRC		1
/** Response parity error status. */
#define RESPONSE_ERROR_PARITY		2
/** Response frame error status. */
#define RESPONSE_ERROR_FRAME		3
/** Response IBA NACK error status. */
#define RESPONSE_ERROR_IBA_NACK		4
/** Response address NACK error status. */
#define RESPONSE_ERROR_ADDRESS_NACK	5
/** Response overflow/underflow error status. */
#define RESPONSE_ERROR_OVER_UNDER_FLOW	6
/** Response transfer-abort error status. */
#define RESPONSE_ERROR_TRANSF_ABORT	8
/** Response I2C write NACK error status. */
#define RESPONSE_ERROR_I2C_W_NACK_ERR	9
/** Response transaction-ID field helper. */
#define RESPONSE_PORT_TID(x)		(((x) & GENMASK(27, 24)) >> 24)
/** Response data-length field helper. */
#define RESPONSE_PORT_DATA_LEN(x)	((x) & 0xFFFF)

/** SCL high-count field helper. */
#define SCL_I3C_TIMING_HCNT(x)		(((x) << 16) & GENMASK(23, 16))
/** SCL low-count field helper. */
#define SCL_I3C_TIMING_LCNT(x)		((x) & GENMASK(7, 0))
#define SCL_I3C_TIMING_CNT_MIN		5U	/**< Minimum SCL high/low count value for timing register */
#define SCL_I3C_TIMING_CNT_MAX		255U	/**< Maximum SCL high/low count value, limited to 8-bit timing register field width */
/** Minimum I2C Fast-mode SCL low time in ns. */
#define I3C_BUS_I2C_FM_TLOW_MIN_NS	1300
/** Minimum I2C Fast-mode Plus SCL low time in ns. */
#define I3C_BUS_I2C_FMP_TLOW_MIN_NS	500
/** Maximum I3C SCL high time in ns. */
#define I3C_BUS_THIGH_MAX_NS		41

#ifdef __cplusplus
}
#endif

#endif
/** @} */
