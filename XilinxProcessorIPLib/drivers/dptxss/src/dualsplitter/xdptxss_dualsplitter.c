/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc. All rights reserved.
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
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file xdptxss_dualsplitter.c
*
* This file contains a minimal set of functions for the Dual Splitter core
* to configure.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- -----------------------------------------------------------
* 1.00 sha 01/29/15 Initial release.
* 1.00 sha 07/21/15 Renamed file name with prefix xdptxss_* and function
*                   name with prefix XDpTxSs_*.
* 2.00 sha 08/07/15 Removed video mode check.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdptxss_dualsplitter.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

#if (XPAR_XDUALSPLITTER_NUM_INSTANCES > 0)
/*****************************************************************************/
/**
*
* This function configures dual splitter with required parameters based on
* number of streams.
*
* @param	InstancePtr is a pointer to the XDualSplitter core instance.
* @param	VertSplit specifies a TURE/FALSE flag to indicate whether the
*		core need to program in split mode or bypass mode respectively.
* @param	MsaConfig is a pointer to main stream attributes structure
*		that will be used to extract timing values.
*
* @return
*		- XST_SUCCESS if Dual Splitter configured successfully.
*
* @note		None.
*
******************************************************************************/
u32 XDpTxSs_DsSetup(XDualSplitter *InstancePtr, u8 VertSplit,
				XDp_TxMainStreamAttributes *MsaConfig)
{
	u8 ISamples;
	u8 OSamples;
	u8 ImgSegments;
	u8 Overlap;
	u16 Height;
	u16 Width;
	u32 UserPixelWidth;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(MsaConfig != NULL);
	Xil_AssertNonvoid((VertSplit == TRUE) || (VertSplit == FALSE));

	UserPixelWidth = MsaConfig->UserPixelWidth;
	Height = MsaConfig->Vtm.Timing.VActive;

	/* Dual Splitter register update enable */
	XDualSplitter_RegUpdateEnable(InstancePtr);

	/* Required parameters based on vertical split mode and 4k2k@60p */
	if (VertSplit) {
		ImgSegments = 0x2;
		OSamples = 0x4;
		ISamples = 0x4;
		Width = MsaConfig->Vtm.Timing.HActive / 2;
	}
	else {
		ImgSegments = 0x1;
		OSamples = 0x4;
		ISamples = 0x4;
		Width = MsaConfig->Vtm.Timing.HActive / UserPixelWidth;
	}

	/* overlap is common for streams */
	Overlap = 0x0;

	/* set stream parameters */
	XDualSplitter_SetImgParam(InstancePtr, ISamples, OSamples, ImgSegments,
				Overlap);

	/* Update width x height resolution */
	XDualSplitter_SetImageSize(InstancePtr, Height, Width);

	/* Enable Dual Splitter */
	XDualSplitter_Enable(InstancePtr);

	return XST_SUCCESS;
}
#endif
