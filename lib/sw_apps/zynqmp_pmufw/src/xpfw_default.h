/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/


#ifndef XPFW_DEFAULT_H_
#define XPFW_DEFAULT_H_


#include "xpfw_config.h"
#include "xpfw_codes.h"
#include "xpfw_util.h"
#include "xpfw_debug.h"

/* BSP Headers */
#include "xil_io.h"
#include "xil_types.h"
#include "mb_interface.h"
#include "xstatus.h"
/* REGDB Headers */
#include "pmu_local.h"
#include "pmu_iomodule.h"
#include "pmu_global.h"
#include "ipi.h"
#include "uart0.h"
#include "uart1.h"
#include "crl_apb.h"
#include "lpd_slcr.h"
#include "rtc.h"


/* RAM address used for scrubbing */
#define PARAM_RAM_LOW_ADDRESS		0Xffdc0000U
#define PARAM_RAM_HIGH_ADDRESS		0Xffdcff00U

/* RAM base address for general usage */
#define PMU_RAM_BASE_ADDR		0Xffdc0000U

/* Register Access Macros */

#define XPfw_Write32(Addr, Value)  Xil_Out32((Addr), (Value))

#define XPfw_Read32(Addr)  Xil_In32((Addr))

#define XPfw_RMW32  XPfw_UtilRMW

#define ARRAYSIZE(x)	(u32)(sizeof(x)/sizeof(x[0]))
/* Custom Flags */

#define MASK_ALL 	0XffffffffU
#define ENABLE_ALL	0XffffffffU
#define ALL_HIGH	0XffffffffU
#define FLAG_ALL	0XffffffffU


#define MASK32_ALL_HIGH	((u32)0xFFFFFFFFU)
#define MASK32_ALL_LOW	((u32)0x0U)


#define YES 0x01U
#define NO 0x00U


#define XPFW_ACCESS_ALLOWED 0x01U
#define XPFW_ACCESS_DENIED	0x00U

/* Handler Table Structure */
typedef void (*VoidFunction_t)(void);
struct HandlerTable{
	u32 Mask;
	VoidFunction_t Handler;
};

#endif /* XPFW_DEFAULT_H_ */
