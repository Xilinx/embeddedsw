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
* @file xplm_gic_interrupts.c
*
* This file is to handle the GIC interrupts
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ====  ==== ======== ======================================================-
* 1.00  mg   10/08/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplm_gic_interrupts.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
static struct GicIntrHandlerTable g_GicPInterruptTable[XPLM_GICP_SOURCE_COUNT][XPLM_NO_OF_BITS_IN_REG] = {
#ifdef XPAR_XIPIPSU_0_DEVICE_ID
		[XPLM_GICP0_INDEX][XPLM_IPI_INDEX] = {XPLM_GICP0_IPI_INTR_MASK, XPlmi_IpiDispatchHandler},
#endif
};

/*****************************************************************************/
/**
 * @brief This is handler for GIC interrupts
 *
 * @param	CallbackRef is presently the interrupt number that is received
 *
 * @return	void
 *
 *****************************************************************************/
void XPlm_GicIntrHandler(void *CallbackRef)
{
	u32 GicPIntrStatus;
	u32 GicPNIntrStatus;
	u32 GicIndex;
	u32 GicPIndex;

	/*
	 * Indicate Interrupt received
	 */
	XPlmi_Printf(DEBUG_GENERAL,
	      "Received GIC Interrupt: 0x%0x\n\r", (u32) CallbackRef);

	GicPIntrStatus = Xil_In32(XPLM_GICP_IRQ_STATUS);
	XPlmi_Printf(DEBUG_GENERAL, "GicPIntrStatus: 0x%x\r\n", GicPIntrStatus);

	for (GicIndex = 0U; GicIndex < XPLM_GICP_SOURCE_COUNT; GicIndex++) {

		if (GicPIntrStatus & (1 << GicIndex)) {

			GicPNIntrStatus = Xil_In32(XPLM_GICP0_IRQ_STATUS + (GicIndex*0x14));
			XPlmi_Printf(DEBUG_GENERAL, "GicP%d Intr Status: 0x%x\r\n",
					GicIndex, GicPNIntrStatus);

			for (GicPIndex = 0U; GicPIndex < XPLM_NO_OF_BITS_IN_REG; GicPIndex++) {

				if (GicPNIntrStatus & g_GicPInterruptTable[GicIndex][GicPIndex].Mask) {

					g_GicPInterruptTable[GicIndex][GicPIndex].GicHandler();
					Xil_Out32((XPLM_GICP0_IRQ_STATUS + (GicIndex*0x14)),
							g_GicPInterruptTable[GicIndex][GicPIndex].Mask);

				}
			}
			Xil_Out32(XPLM_GICP_IRQ_STATUS, (GicPIntrStatus & (1 << GicIndex)));
		}
	}

}
