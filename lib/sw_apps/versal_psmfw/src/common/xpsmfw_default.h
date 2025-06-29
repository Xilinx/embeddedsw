/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
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

#define XPsmFw_Write32(Addr, Value)  Xil_Out32((Addr), (Value)) /**< Register Access Macro */

#define XPsmFw_Read32(Addr)  Xil_In32((Addr)) /**< Register Access Macro */

#define XPsmFw_RMW32  XPsmFw_UtilRMW /**< Register Access Macro */

#define ARRAYSIZE(x)	(u32)(sizeof(x)/sizeof(x[0])) /**< Calculates array size */

#define MICROSECOND_TO_TICKS(x)		((x) * (((u32)XPAR_CPU_CORE_CLOCK_FREQ_HZ)/1000000U)) /**< Microseconds to ticks conversion */
#define NANOSECOND_TO_TICKS(x)			((MICROSECOND_TO_TICKS(x))/1000U) /**< Nanoseconds to ticks conversion */

#define MASK32_ALL_HIGH	((u32)0xFFFFFFFFU) /**< Mask for 32bit value */
#define MASK32_ALL_LOW	((u32)0x0U) /**< Mask for 32bit value */


#define YES 0x01U /**< Custom Flags */
#define NO 0x00U /**< Custom Flags */

#define TRUE_VALUE	(u8)(1U) /**< Custom Flags */
#define FALSE_VALUE	(u8)(0U) /**< Custom Flags */

/* Handler Table Structure */
typedef void (*VoidFunction_t)(void);

/**
 * @brief Structure to hold handler information
 */
struct HandlerTable {
	u32 Shift; /**< The shift value for the handler */
	u32 Mask; /**< The mask value for the handler */
	VoidFunction_t Handler; /**< The function pointer to the handler */
};

#ifdef __cplusplus
}
#endif

#endif /* XPSMFW_DEFAULT_H_ */
