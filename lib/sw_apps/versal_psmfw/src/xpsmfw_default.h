/******************************************************************************
*
* Copyright (C) 2018 - 2019 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
*
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
#include "mb_interface.h"
#include "xstatus.h"
/* REGDB Headers */
#include "psm_local.h"
#include "ipi.h"

/* Register Access Macros */
#define XPsmFw_Write32(Addr, Value)  Xil_Out32((Addr), (Value))

#define XPsmFw_Read32(Addr)  Xil_In32((Addr))

#define XPsmFw_RMW32  XPsmFw_UtilRMW

#define ARRAYSIZE(x)	(u32)(sizeof(x)/sizeof(x[0]))

//TODO: This macro should come from other config file like xparameters.h
#define PSMFW_CLK_FREQ                                (400000000U)
#define MILLISECOND_TO_TICKS(x)                       ((x) * (PSMFW_CLK_FREQ/1000U))
#define MICROSECOND_TO_TICKS(x)                       ((x) * (PSMFW_CLK_FREQ/1000000U))
#define NENOSECOND_TO_TICKS(x)                        ((MICROSECOND_TO_TICKS(x))/1000U)

/* Custom Flags */
#define MASK32_ALL_HIGH	((u32)0xFFFFFFFFU)
#define MASK32_ALL_LOW	((u32)0x0U)


#define YES 0x01U
#define NO 0x00U

/* Handler Table Structure */
typedef void (*VoidFunction_t)(void);
struct HandlerTable {
	u32 Mask;
	VoidFunction_t Handler;
};

#ifdef __cplusplus
}
#endif

#endif /* XPSMFW_DEFAULT_H_ */
