/******************************************************************************
*
* Copyright (C) 2008 - 2014 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
 * @file example.c
 *
 * This file demonstrates how to use Xilinx Video Timing Controller driver on
 * Xilinx MVI Video Timing Controller core.
 *
 * This code makes the following assumptions:
 * - Caching is disabled. Flushing and Invalidation operations for data buffer
 *   need to be added to this code if it is not the case.
 *
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00a xd   04/23/09 First release
 * 2.00a xd   04/23/09 Renamed to "Video Timing Controller"
 * </pre>
 *
 * ***************************************************************************
 */

#include "xvtc.h"
#include "xparameters.h"

/*
 * Device related constants. Defined in xparameters.h.
 */
#define VTC_DEVICE_ID	XPAR_VTC_0_DEVICE_ID

/*
 * Video Timing Controller Device related data structures
 */
XVtc Vtc;			/* Device driver instance */
XVtc_Signal Signal;		/* VTC Signal configuration */
XVtc_Polarity Polarity;		/* Polarity configuration */
XVtc_SourceSelect SourceSelect;	/* Source Selection configuration */

/*
 * Function prototypes
 */
static void SignalSetup(XVtc_Signal *SignalCfgPtr);
static int VtcExample(u16 VtcDeviceID);

/*****************************************************************************/
/**
*
* This is the main function for the Video Timing Controller example.
*
* @param	None.
*
* @return	0 to indicate success, otherwise 1.
*
* @note	None.
*
****************************************************************************/
int main(void)
{
	int Status;

	/*
	 * Call the Video Timing Controller example , specify the Device ID
	 * generated in xparameters.h
	 */
	Status = VtcExample(VTC_DEVICE_ID);
	if (Status != 0) {
		return 1;
	}

	return 0;
}

/*****************************************************************************/
/**
*
* This function sets up the Video Timing Controller Signal configuration.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
static void SignalSetup(XVtc_Signal *SignalCfgPtr)
{
	int HFrontPorch;
	int HSyncWidth;
	int HBackPorch;
	int VFrontPorch;
	int VSyncWidth;
	int VBackPorch;
	int LineWidth;
	int FrameHeight;

	/* Choose the configuration for 720P60 */

	HFrontPorch = 110;
	HSyncWidth = 40;
	HBackPorch = 220;
	VFrontPorch = 5;
	VSyncWidth = 5;
	VBackPorch = 20;
	LineWidth = 1280;
	FrameHeight = 720;

	/* Clear the VTC Signal config structure */

	memset((void *)SignalCfgPtr, 0, sizeof(XVtc_Signal));

	/* Populate the VTC Signal config structure. Ignore the Field 1 */

	SignalCfgPtr->HFrontPorchStart = 0;
	SignalCfgPtr->HTotal = HFrontPorch + HSyncWidth + HBackPorch
				+ LineWidth - 1;
	SignalCfgPtr->HBackPorchStart = HFrontPorch + HSyncWidth;
	SignalCfgPtr->HSyncStart = HFrontPorch;
	SignalCfgPtr->HActiveStart = HFrontPorch + HSyncWidth + HBackPorch;

	SignalCfgPtr->V0FrontPorchStart = 0;
	SignalCfgPtr->V0Total = VFrontPorch + VSyncWidth + VBackPorch
				+ FrameHeight - 1;
	SignalCfgPtr->V0BackPorchStart = VFrontPorch + VSyncWidth;
	SignalCfgPtr->V0SyncStart = VFrontPorch;
	SignalCfgPtr->V0ChromaStart = VFrontPorch + VSyncWidth + VBackPorch;
	SignalCfgPtr->V0ActiveStart = VFrontPorch + VSyncWidth + VBackPorch;

	 return;
}

/*****************************************************************************/
/**
*
* This function is the entry of the feature demonstrations on MVI Video
* Timing Controller core. It initializes the Video Timing Controller device,
* then sets up the video timing controller signal for the generator module,
* polarities of the output, selects source, and last start the Video Timing
* Controller device.
*
* @param	VtcDeviceID is the device ID of the Video Timing Controller core.
*
* @return	0 if all tests pass, 1 otherwise.
*
* @note		None.
*
******************************************************************************/
static int VtcExample(u16 VtcDeviceID)
{
	int Status;
	XVtc_Config *VtcCfgPtr;

	/* Look for the device configuration info for the Video Timing
	 * Controller.
	 */
	VtcCfgPtr = XVtc_LookupConfig(VTC_DEVICE_ID);
	if (VtcCfgPtr == NULL) {
		return 1;
	}

	/* Initialize the Video Timing Controller instance */

	Status = XVtc_CfgInitialize(&Vtc, VtcCfgPtr,
		VtcCfgPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return 1;
	}

	/* Set up Generator */

	SignalSetup(&Signal);
	XVtc_SetGenerator(&Vtc, &Signal);

	/* Set up Polarity of all outputs */

	memset((void *)&Polarity, 0, sizeof(Polarity));
	Polarity.ActiveChromaPol = 1;
	Polarity.ActiveVideoPol = 1;
	Polarity.FieldIdPol = 0;
	Polarity.VBlankPol = 1;
	Polarity.VSyncPol = 1;
	Polarity.HBlankPol = 1;
	Polarity.HSyncPol = 1;

	XVtc_SetPolarity(&Vtc, &Polarity);

	/* Set up source select */

	memset((void *)&SourceSelect, 0, sizeof(SourceSelect));
	SourceSelect.VChromaSrc = 1;
	SourceSelect.VActiveSrc = 1;
	SourceSelect.VBackPorchSrc = 1;
	SourceSelect.VSyncSrc = 1;
	SourceSelect.VFrontPorchSrc = 1;
	SourceSelect.VTotalSrc = 1;
	SourceSelect.HActiveSrc = 1;
	SourceSelect.HBackPorchSrc = 1;
	SourceSelect.HSyncSrc = 1;
	SourceSelect.HFrontPorchSrc = 1;
	SourceSelect.HTotalSrc = 1;

	XVtc_SetSource(&Vtc, &SourceSelect);

	/* Enable both generator and detector modules */

	XVtc_Enable(&Vtc);
	XVtc_EnableGenerator(&Vtc);
	XVtc_EnableDetector(&Vtc);

	/* Return success */

	return 0;
}
