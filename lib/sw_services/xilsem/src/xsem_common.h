/*****************************************************************************
* Copyright (C) 2018-2019 Xilinx, Inc.  All rights reserved.
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
/**
* @file xsem_cfr_common.h
*  This file contains structures, global variables and Macro definitions
*  required for the common routines
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	  Who      Date		     Changes
* ----  ----     --------    --------------------------------------------------
* 0.1	  mw	 06/26/2018  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XSEM_CFR_COMMON_H		/* prevent circular inclusions */
#define XSEM_CFR_COMMON_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xcframe.h"
#include "xcfupmc.h"
#include "xplmi_debug.h"

#include "xsem_defs.h"
#include "xsem_glbl_regs.h"
#include "xsem_cfr_common.h"

/***************************** Global variables ******************************/
extern u32 UartInitialized;

/***************** Macros (Inline Functions) Definitions *********************/
#define XSEM_VERSION "v0_1"

#define XSEM_REG_RST_LPD_IOU2		0xFF5E0238U
#define XSEM_REG_TS_REF_CTRL		0xFF5E0128U

#define XSEM_CXT_REG_CNTRCTRL		0xFF260000U
#define XSEM_CXT_REG_CNTRSTAT		0xFF260004U
#define XSEM_CXT_REG_CNTRLOW		0xFF260008U
#define XSEM_CXT_REG_CNTRHIGH		0xFF26000CU
#define XSEM_CXT_REG_CNTRFREQ		0xFF260020U

/* Change these parameters manually if the source PLL or PLL freq are changed */
#define XSEM_CXTINIT_PLL_SRC		0U	  /* SRCSEL = IOPLL */
#define XSEM_CXTINIT_CLKDIV		0xFU	  /* Assuming IOPLL=1500 MHz */
#define XSEM_CXTINIT_FREQ_HZ		0x05F5E100U  /* Freq = 100 MHz */

#define XSEM_CXTINIT_WAIT_LOOP		1000U

#if defined (__GNUC__)
#define xsem_asm			__asm__ __volatile__
#define xsem_mfcp(reg, val)		val=mfcp(reg)
#define xsem_mtcp(reg, val)		mtcp(reg, val)
#elif defined (__ICCARM__)
#define xsem_asm			__asm volatile
#define xsem_mfcp(reg, val)		mfcp(reg, val)
#define xsem_mtcp(reg, val)		mtcp(reg, val)
#endif

#define xsem_dmb			dmb
#define xsem_dsb			dsb
#define xsem_isb			isb

#define XSem_In64			Xil_In64
#define XSem_In32			Xil_In32
#define XSem_In16			Xil_In16
#define XSem_In8			Xil_In8

#define XSem_Out64			Xil_Out64
#define XSem_Out32			Xil_Out32
#define XSem_Out16			Xil_Out16
#define XSem_Out8			Xil_Out8

#define XSem_AssertNonvoid		Xil_AssertNonvoid
#define XSem_AssertVoid			Xil_AssertVoid

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
void XSem_WaitForUsecs(u32 Delay); /* TODO: remove if unused */
u32 XSem_CxtsgenInit(void);        /* TODO: remove if unused */
void XSem_ClrBit32(u32 Address, u32 BitLoc);
void XSem_SetBit32(u32 Address, u32 BitLoc);
u32 XSem_ClrVal32(u32 Data, u32 BitLoc);
void XSem_In128(u32 Addr, u32 *RegRd0, u32 *RegRd1, u32 *RegRd2, u32 *RegRd3);
void XSem_Out128(u32 Addr, u32 RegWr0, u32 RegWr1, u32 RegWr2, u32 RegWr3);
void XSem_FatalState(void);
void XSem_DumpRegisters();
void XSem_debug_print();
void XSem_cframe_debug_print();
void XSem_DumpIntRegisters();

#ifdef __cplusplus
}
#endif

#endif		/* XSEM_CFR_COMMON_H */
