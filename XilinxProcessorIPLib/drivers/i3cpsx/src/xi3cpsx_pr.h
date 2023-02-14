/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/


/******************************************************************************
* I3C spec defines the following:
* Common command codes (CCC)
* Error types
* Command structure/framing
* Characterisitcs registers
*
******************************************************************************/

#ifndef XI3CPS_PR_H
#define XI3CPS_PR_H

#include "xil_types.h"
#include "xil_assert.h"

#define BIT(N)		(1U << N)


#define I3C_BUS_TYP_I3C_SCL_RATE	12500000
#define I3C_BUS_I2C_FM_PLUS_SCL_RATE	1000000
#define I3C_BUS_I2C_FM_SCL_RATE		400000
#define I3C_BUS_TLOW_OD_MIN_NS		200


/***********************Common command codes (CCC)****************************/

/* I3C CCC (Common Command Codes) related definitions */
#define I3C_CCC_DIRECT			BIT(7)

#define I3C_CCC_ID(id, broadcast)	\
	((id) | ((broadcast) ? 0 : I3C_CCC_DIRECT))

/* Commands valid in both broadcast and unicast modes */
#define I3C_CCC_ENEC(broadcast)		I3C_CCC_ID(0x0, broadcast)
#define I3C_CCC_DISEC(broadcast)	I3C_CCC_ID(0x1, broadcast)
#define I3C_CCC_ENTAS(as, broadcast)	I3C_CCC_ID(0x2 + (as), broadcast)
#define I3C_CCC_RSTDAA(broadcast)	I3C_CCC_ID(0x6, broadcast)
#define I3C_CCC_SETMWL(broadcast)	I3C_CCC_ID(0x9, broadcast)
#define I3C_CCC_SETMRL(broadcast)	I3C_CCC_ID(0xa, broadcast)
#define I3C_CCC_SETXTIME(broadcast)	((broadcast) ? 0x28 : 0x98)
#define I3C_CCC_VENDOR(id, broadcast)	((id) + ((broadcast) ? 0x61 : 0xe0))

/* Broadcast-only commands */
#define I3C_CCC_ENTDAA			I3C_CCC_ID(0x7, TRUE)
#define I3C_CCC_DEFSLVS			I3C_CCC_ID(0x8, TRUE)
#define I3C_CCC_ENTTM			I3C_CCC_ID(0xb, TRUE)
#define I3C_CCC_ENTHDR(x)		I3C_CCC_ID(0x20 + (x), TRUE)

/* Unicast-only commands */
#define I3C_CCC_SETDASA			I3C_CCC_ID(0x7, FALSE)
#define I3C_CCC_SETNEWDA		I3C_CCC_ID(0x8, false)
#define I3C_CCC_GETMWL			I3C_CCC_ID(0xb, FALSE)
#define I3C_CCC_GETMRL			I3C_CCC_ID(0xc, FALSE)
#define I3C_CCC_GETPID			I3C_CCC_ID(0xd, FALSE)
#define I3C_CCC_GETBCR			I3C_CCC_ID(0xe, FALSE)
#define I3C_CCC_GETDCR			I3C_CCC_ID(0xf, FALSE)
#define I3C_CCC_GETSTATUS		I3C_CCC_ID(0x10, FALSE)
#define I3C_CCC_GETACCMST		I3C_CCC_ID(0x11, FALSE)
#define I3C_CCC_SETBRGTGT		I3C_CCC_ID(0x13, FALSE)
#define I3C_CCC_GETMXDS			I3C_CCC_ID(0x14, FALSE)
#define I3C_CCC_GETHDRCAP		I3C_CCC_ID(0x15, FALSE)
#define I3C_CCC_GETXTIME		I3C_CCC_ID(0x19, FALSE)

#define I3C_CCC_EVENT_SIR		BIT(0)
#define I3C_CCC_EVENT_MR		BIT(1)
#define I3C_CCC_EVENT_HJ		BIT(3)
/*FIXME */
#define GENMASK(h, l)			(~(u32)0 - ( (u32)1 << l) + 1) & \
					(~(u32)0 >> (32 - 1 - h))

#define COMMAND_PORT_TOC		BIT(30)
#define COMMAND_PORT_READ_TRANSFER	BIT(28)
#define COMMAND_PORT_SDAP		BIT(27)
#define COMMAND_PORT_ROC		BIT(26)
#define COMMAND_PORT_SPEED(x)		((x) << 21)
#define COMMAND_PORT_DEV_INDEX(x)	((x) << 16)
#define COMMAND_PORT_CP			BIT(15)
#define COMMAND_PORT_CMD(x)		((x) << 7)
#define COMMAND_PORT_TID(x)		((x) << 3)

#define COMMAND_PORT_ARG_DATA_LEN(x)	((x) << 16)
#define COMMAND_PORT_ARG_DATA_LEN_MAX	65536
#define COMMAND_PORT_TRANSFER_ARG	0x01

#define COMMAND_PORT_SDA_DATA_BYTE_3(x)	((x) << 24)
#define COMMAND_PORT_SDA_DATA_BYTE_2(x)	((x) << 16)
#define COMMAND_PORT_SDA_DATA_BYTE_1(x)	((x) << 8)
#define COMMAND_PORT_SDA_BYTE_STRB_3	BIT(5)
#define COMMAND_PORT_SDA_BYTE_STRB_2	BIT(4)
#define COMMAND_PORT_SDA_BYTE_STRB_1	BIT(3)
#define COMMAND_PORT_SHORT_DATA_ARG	0x02

#define COMMAND_PORT_DEV_COUNT(x)	((x) << 21)
#define COMMAND_PORT_ADDR_ASSGN_CMD	0x03

#define RESPONSE_PORT_ERR_STATUS(x)	(((x) & GENMASK(31, 28)) >> 28)
#define RESPONSE_NO_ERROR		0
#define RESPONSE_ERROR_CRC		1
#define RESPONSE_ERROR_PARITY		2
#define RESPONSE_ERROR_FRAME		3
#define RESPONSE_ERROR_IBA_NACK		4
#define RESPONSE_ERROR_ADDRESS_NACK	5
#define RESPONSE_ERROR_OVER_UNDER_FLOW	6
#define RESPONSE_ERROR_TRANSF_ABORT	8
#define RESPONSE_ERROR_I2C_W_NACK_ERR	9
#define RESPONSE_PORT_TID(x)		(((x) & GENMASK(27, 24)) >> 24)
#define RESPONSE_PORT_DATA_LEN(x)	((x) & 0xFFFF)


#define SCL_I3C_TIMING_HCNT(x)		(((x) << 16) & GENMASK(23, 16))
#define SCL_I3C_TIMING_LCNT(x)		((x) & GENMASK(7, 0))
#define SCL_I3C_TIMING_CNT_MIN		5
#define I3C_BUS_I2C_FM_TLOW_MIN_NS	1300
#define I3C_BUS_I2C_FMP_TLOW_MIN_NS	500
#define I3C_BUS_THIGH_MAX_NS		41

#endif
