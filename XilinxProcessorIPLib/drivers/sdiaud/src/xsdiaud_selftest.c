/*******************************************************************************
 *
 * Copyright (C) 2017 - 2018 Xilinx, Inc.  All rights reserved.
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
/******************************************************************************/
/**
 *
 * @file xsdiaud_selftest.c
 * @addtogroup sdiaud_v1_0
 * @{
 * Contains an basic self-test API
 * @note None
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date        Changes
 * ----- ----- ---------- -----------------------------------------------
 * 1.0    kar  02/14/18    Initial release.
 * </pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xsdiaud.h"
#include "xil_types.h"
#include "xil_assert.h"
#include "xsdiaud_hw.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/

/***************** Macros (In-line Functions) Definitions *********************/

/*****************************************************************************/
/**
 *
 * This macro returns the XSdiAud operating mode.
 *
 * @param  InstancePtr is a pointer to the XSdiAud core instance.
 *
 * @return
 *   - TRUE  : Audio Embed
 *   - FALSE : Audio Extract
 *
 * @note C-style signature:
 *   u8 XSdiAud_IsEmbed(XSdiAud *InstancePtr)
 *
 *****************************************************************************/
#define XSdiAud_IsEmbed(InstancePtr) \
	(((XSdiAud_ReadReg((InstancePtr)->Config.BaseAddress, (XSDIAUD_GUI_PARAM_REG_OFFSET))\
	   & XSDIAUD_GUI_AUDF_MASK) >> XSDIAUD_GUI_AUDF_SHIFT) ? TRUE : FALSE)

/*****************************************************************************/
/**
 *
 * Runs a self-test on the driver/device. The self-test  reads the XSdi_Aud
 * registers and verifies the value.
 *
 * @param	InstancePtr is a pointer to the XSdiAud instance.
 *
 * @return
 *		- XST_SUCCESS if successful i.e. if the self test passes.
 *		- XST_FAILURE if unsuccessful i.e. if the self test fails
 *
 * @note	None.
 *
 *****************************************************************************/
int XSdiAud_SelfTest(XSdiAud *InstancePtr)
{
	int Status = XST_SUCCESS;
	u32 SdiAud_IsEmbed;
	/* verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	/* Read the SDI Audio Module control register to know the
	 * operating mode i.e. to know whether the core is configured
	 * as a Audio Embed or Audio Extract.
	 */
	SdiAud_IsEmbed = XSdiAud_IsEmbed(InstancePtr);
	if (SdiAud_IsEmbed != InstancePtr->Config.IsEmbed) {

	xil_printf("Core configuration (%d) doesn't match GUI value (%d).\r\n",
				SdiAud_IsEmbed, InstancePtr->Config.IsEmbed);
		return XST_FAILURE;
	}
	return Status;
}
/** @} */
