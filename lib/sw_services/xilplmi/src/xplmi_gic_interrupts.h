/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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

#ifndef XPLMI_GIC_INTERRUPTS_H
#define XPLMI_GIC_INTERRUPTS_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "pmc_global.h"
#include "xplmi_ipi.h"

/************************** Constant Definitions *****************************/
#define XPLMI_GICP_IRQ_STATUS				0xF11400A0
#define XPLMI_GICP0_IRQ_STATUS				0xF1140000
#define XPLMI_GICP0_IRQ_MASK				0xF1140004
#define XPLMI_GICP_SOURCE_COUNT				0x5
#define XPLMI_NO_OF_BITS_IN_REG				32U

#define XPLMI_GICP_MASK			(0xFF00U)
#define XPLMI_GICPX_MASK		(0xFF0000U)

/**
 * PMC GIC interrupts
 */
enum {
	XPLMI_PMC_GIC_IRQ_GICP0 = 0,
	XPLMI_PMC_GIC_IRQ_GICP1,
	XPLMI_PMC_GIC_IRQ_GICP2,
	XPLMI_PMC_GIC_IRQ_GICP3,
	XPLMI_PMC_GIC_IRQ_GICP4,
};

/**
 * PMC GICP0 interrupts
 */
enum {
	XPLMI_GICP0_SRC27 = 27, /**< IPI Interrupt */
};

/**
 * PMC GICP4 interrupts
 */
enum {
	XPLMI_GICP4_SRC8 = 8, /**< SBI interrupt */
};

/**************************** Type Definitions *******************************/
/* Handler Table Structure */
typedef int (*Function_t)(void *Data);
struct GicIntrHandlerTable {
	void *Data;
	Function_t GicHandler;
};
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/* Functions defined in xplm_main.c */
void XPlmi_GicIntrHandler(void *CallbackRef);
void XPlmi_GicRegisterHandler(u32 PlmIntrId, Function_t Handler, void *Data);
void XPlmi_GicIntrEnable(u32 PlmIntrId);
void XPlmi_GicIntrDisable(u32 PlmIntrId);

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_GIC_INTERRUPTS_H */
