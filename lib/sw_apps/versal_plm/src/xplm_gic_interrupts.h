/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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

/*****************************************************************************/
/**
*
* @file xplm_gic_interrupts.h
*
* This is the header file for xplm_gic_interrupts.c
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  mg   10/08/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPLM_GIC_INTERRUPTS_H
#define XPLM_GIC_INTERRUPTS_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplm_default.h"
#include "xplmi_ipi.h"

/************************** Constant Definitions *****************************/
#define XPLM_GICP_IRQ_STATUS				0xF11400A0
#define XPLM_GICP0_IRQ_STATUS				0xF1140000
#define XPLM_GICP_SOURCE_COUNT				0x5
#define XPLM_NO_OF_BITS_IN_REG				32U

#define XPLM_GICP0_IPI_INTR_MASK			0x8000000

#define XPLM_GICP0_INDEX			0x0
#define XPLM_GICP1_INDEX			0x1
#define XPLM_GICP2_INDEX			0x2
#define XPLM_GICP3_INDEX			0x3
#define XPLM_GICP4_INDEX			0x4

#define XPLM_IPI_INDEX				27U

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/* Functions defined in xplm_main.c */
void XPlm_GicIntrHandler(void *CallbackRef);

/* Handler Table Structure */
typedef int (*Function_t)(void);
struct GicIntrHandlerTable {
	u32 Mask;
	Function_t GicHandler;
};

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XPLM_GIC_INTERRUPTS_H */
