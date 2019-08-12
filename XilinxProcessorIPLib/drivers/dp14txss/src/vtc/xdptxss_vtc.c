/******************************************************************************
*
* Copyright (C) 2015 - 2016 Xilinx, Inc. All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
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
* @file xdptxss_vtc.c
*
* This file contains a minimal set of functions for the Video Timing controller
* core to configure.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- -----------------------------------------------------------
* 1.00 sha 01/29/15 Initial release.
* 1.00 sha 07/21/15 Renamed file name with prefix xdptxss_* and function
*                   name with prefix XDpTxSs_*
* 2.00 sha 08/07/15 Set interlace to zero when video mode is XVIDC_VM_CUSTOM.
* 4.1  als 08/03/16 Use video common API rather than internal structure when
*                   checking for interlaced mode.
* 5.0  tu  08/10/17 Adjusted BS symbol for equal timing
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdptxss_vtc.h"
#include "string.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function configures Video Timing Controller with video timing
* parameters from MSA config structure.
*
* @param	InstancePtr is a pointer to the XVtc instance.
* @param	MsaConfig is a pointer to main stream attributes structure
*		that will be used to extract timing values.
*
* @return
*		- XST_SUCCESS if video timing parameters set successfully.
*
* @note		None.
*
******************************************************************************/
u32 XDpTxSs_VtcSetup(XVtc *InstancePtr, XDp_TxMainStreamAttributes *MsaConfig,
			u8 VtcAdjustBs)
{
	u32 UserPixelWidth;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(MsaConfig != NULL);

	/* Polarity configuration */
	XVtc_Polarity Polarity;
	XVtc_SourceSelect SourceSelect;
	XVtc_Timing VideoTiming;

	/* user pixel width */
	UserPixelWidth = MsaConfig->UserPixelWidth;

	/* Disable Generator */
	XVtc_Reset(InstancePtr);
	XVtc_DisableGenerator(InstancePtr);
	XVtc_Disable(InstancePtr);

	/* Set up source select */
	memset((void *)&SourceSelect, 0, sizeof(SourceSelect));

	/* 1 = Generator registers, 0 = Detector registers */
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

	/* Set source */
	XVtc_SetSource(InstancePtr, &SourceSelect);

	/* Horizontal timing */
	VideoTiming.HActiveVideo =
			MsaConfig->Vtm.Timing.HActive / UserPixelWidth;
	VideoTiming.HFrontPorch =
			MsaConfig->Vtm.Timing.HFrontPorch / UserPixelWidth;
	VideoTiming.HSyncWidth =
			MsaConfig->Vtm.Timing.HSyncWidth / UserPixelWidth;
	VideoTiming.HBackPorch =
			MsaConfig->Vtm.Timing.HBackPorch / UserPixelWidth;
	if (VtcAdjustBs) {
		u16 HBlank;
		u16 HReducedBlank;

		/* Adjust bs timing */
		HBlank = MsaConfig->Vtm.Timing.HBackPorch +
			MsaConfig->Vtm.Timing.HFrontPorch +
			MsaConfig->Vtm.Timing.HSyncWidth;
		/* Reduced blanking starts at ceil(0.2 * HTotal). */
		HReducedBlank = 2 * MsaConfig->Vtm.Timing.HTotal;
		if (HReducedBlank % 10)
			HReducedBlank += 10;
		HReducedBlank /= 10;
		/* CVT spec. states HBlank is either 80 or 160 for reduced blanking. */
		if ((HBlank < HReducedBlank) && ((HBlank == 80) || (HBlank == 160))) {
			u32 tmp = VideoTiming.HFrontPorch;
			VideoTiming.HFrontPorch = 4;
			VideoTiming.HBackPorch += (tmp - 4);
			tmp = VideoTiming.HSyncWidth;
			VideoTiming.HSyncWidth = 4;
			VideoTiming.HBackPorch += (tmp - 4);
		} else {
			u32 tmp = VideoTiming.HFrontPorch;
			VideoTiming.HFrontPorch = 2;
			VideoTiming.HBackPorch += (tmp - 2);
			tmp = VideoTiming.HSyncWidth;
			VideoTiming.HSyncWidth = 2;
			VideoTiming.HBackPorch += (tmp - 2);
		}
	}
	VideoTiming.HSyncPolarity =
		MsaConfig->Vtm.Timing.HSyncPolarity;

	/* Vertical timing */
	VideoTiming.VActiveVideo = MsaConfig->Vtm.Timing.VActive;
	VideoTiming.V0FrontPorch = MsaConfig->Vtm.Timing.F0PVFrontPorch;
	VideoTiming.V0SyncWidth = MsaConfig->Vtm.Timing.F0PVSyncWidth;
	VideoTiming.V0BackPorch = MsaConfig->Vtm.Timing.F0PVBackPorch;
	VideoTiming.V1FrontPorch = MsaConfig->Vtm.Timing.F1VFrontPorch;
	VideoTiming.V1SyncWidth = MsaConfig->Vtm.Timing.F1VSyncWidth;
	VideoTiming.V1BackPorch = MsaConfig->Vtm.Timing.F1VBackPorch;
	VideoTiming.VSyncPolarity = MsaConfig->Vtm.Timing.VSyncPolarity;

	/* Check for interlaced mode */
	VideoTiming.Interlaced =
		(MsaConfig->Vtm.VmId == XVIDC_VM_CUSTOM ||
		 !XVidC_GetVideoModeData(MsaConfig->Vtm.VmId)->Timing.F1VTotal) ?
		0 : 1;

	/* Set timing */
	XVtc_SetGeneratorTiming(InstancePtr, &VideoTiming);

	/* Set up Polarity of all outputs */
	memset((void *)&Polarity, 0, sizeof(XVtc_Polarity));
	Polarity.ActiveChromaPol = 1;
	Polarity.ActiveVideoPol = 1;

	if (VideoTiming.Interlaced) {
		Polarity.FieldIdPol = 1;
	}
	else {
		Polarity.FieldIdPol = 0;
	}

	Polarity.VBlankPol = VideoTiming.VSyncPolarity;
	Polarity.VSyncPol = VideoTiming.VSyncPolarity;
	Polarity.HBlankPol = VideoTiming.HSyncPolarity;
	Polarity.HSyncPol = VideoTiming.HSyncPolarity;

	/* Set polarity */
	XVtc_SetPolarity(InstancePtr, &Polarity);

	/* VTC driver does not take care of the setting of the VTC in
	 * interlaced operation. As a work around the register
	 * is set manually */
	if (VideoTiming.Interlaced) {
		/* Interlaced mode */
		XVtc_WriteReg(InstancePtr->Config.BaseAddress,
				XVTC_GFENC_OFFSET, 0x42);
	}
	else {
		/* Progressive mode */
		XVtc_WriteReg(InstancePtr->Config.BaseAddress,
				XVTC_GFENC_OFFSET, 0x2);
	}

	XVtc_WriteReg(InstancePtr->Config.BaseAddress,
				XVTC_GPOL_OFFSET, 0x3F);

	/* Enable generator module */
	XVtc_Enable(InstancePtr);
	XVtc_EnableGenerator(InstancePtr);
	XVtc_RegUpdateEnable(InstancePtr);

	return XST_SUCCESS;
}
