/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpsmfw_default.h
*
* This file contains default headers and definitions used by PSM Firmware
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who		Date		Changes
* ---- ---- -------- ------------------------------
* 1.00  ma   04/09/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPSMFW_DEFAULT_H_
#define XPSMFW_DEFAULT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xpsmfw_config.h"
#include "xpsmfw_util.h"
#include "xpsmfw_debug.h"

/* BSP Headers */
#include "xil_io.h"
#include "xil_types.h"
#include "xil_util.h"
#include "mb_interface.h"
#include "xstatus.h"
/* REGDB Headers */
#include "ipi.h"

/* Register Access Macros */
#define XPsmFw_Write32(Addr, Value)  Xil_Out32((Addr), (Value))

#define XPsmFw_Read32(Addr)  Xil_In32((Addr))

#define XPsmFw_RMW32  XPsmFw_UtilRMW

#define ARRAYSIZE(x)	(u32)(sizeof(x)/sizeof(x[0]))

#define MICROSECOND_TO_TICKS(x)		((x) * (((u32)XPAR_CPU_CORE_CLOCK_FREQ_HZ)/1000000U))
#define NANOSECOND_TO_TICKS(x)			((MICROSECOND_TO_TICKS(x))/1000U)

/* Custom Flags */
#define MASK32_ALL_HIGH	((u32)0xFFFFFFFFU)
#define MASK32_ALL_LOW	((u32)0x0U)


#define YES 0x01U
#define NO 0x00U

#define TRUE_VALUE	(u8)(1U)
#define FALSE_VALUE	(u8)(0U)

/* Handler Table Structure */
typedef void (*VoidFunction_t)(void);
struct HandlerTable {
	u32 Shift;
	u32 Mask;
	VoidFunction_t Handler;
};

#ifdef __cplusplus
}
#endif

#endif /* XPSMFW_DEFAULT_H_ */
