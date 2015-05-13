/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
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
/*****************************************************************************/
/**
 *
 * @file scaler_example.c
 *
 * This file demonstrates how to use Xilinx XScaler driver on Xilinx MVI Video
 * Scaler core. This code does not cover the Video DMA (VDMA) setup and any
 * other configuration which might be required to get the Scaler working
 * properly.
 *
 * This code makes the following assumptions:
 *  - Caching is disabled. Flushing and Invalidation operations for data buffer
 *    need to be added to this code if it is not the case.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date     Changes
 * ----- ---   -------- ------------------------------------------------------
 * 1.00a xd    02/09/09 First release
 * 7.00 adk    22/08/14 Renamed example.c to scaler_example.c
  *                     XPAR_SCALER_0_DEVICE_ID is changed to
 *                      XPAR_XSCALER_0_DEVICE_ID.
 *                      Chaged typecast of XScaler_CoefValueLookup from u16 to
 *                      s16.
 * </pre>
 *
 *****************************************************************************/

#include "xscaler.h"
#include "xparameters.h"

/*
 * Device related constants. Defined in xparameters.h.
 */
#define SCALER_DEVICE_ID    XPAR_XSCALER_0_DEVICE_ID

/*
 * Index of Coefficient Set to load and use
 */
#define COEFF_SET_INDEX     0

/*
 * Scaler Device related data structures
 */
XScaler ScalerInstance;			/* Device driver instance */
XScalerAperture Aperture;		/* Aperture setting */
XScalerStartFraction StartFraction;	/* Luma/Chroma Start Fraction setting*/
XScalerCoeffBank CoeffBank;		/* Coefficient bank */

static int  ScalerExample(u16 ScalerDeviceID);
static void DownScale(XScaler *ScalerInstPtr);
static void UpScale(XScaler *ScalerInstPtr);
static void ScalerSetup(XScaler *ScalerInstPtr,
			int ScalerInWidth, int ScalerInHeight,
			int ScalerOutWidth, int ScalerOutHeight);

/*****************************************************************************/
/**
*
* This is the main function for the Scaler example. This function is not
* included if the example is generated from the TestAppGen test tool.
*
* @param	None.
*
* @return	0 to indicate success, otherwise 1.
*
* @note		None.
*
****************************************************************************/
int main(void)
{
	int Status;

	/*
	 * Call the Scaler example , specify the Device ID generated in
	 * xparameters.h
	 */
	Status = ScalerExample(SCALER_DEVICE_ID);
	if (Status != 0) {
		return 1;
	}
	printf("\n Successfully ran Scaler example \n");
	return 0;
}

/*****************************************************************************/
/**
*
* This function is the entry of the feature demonstrations on MVI Video Scaler
* core. It initializes the Scaler device, then starts operations like
* format downscaling and format up scaling.
*
* @param	ScalerDeviceID is the device ID of the scaler core.
*
* @return	0 if all tests pass, 1 otherwise.
*
* @note		None.
*
******************************************************************************/
static int ScalerExample(u16 ScalerDeviceID)
{
	int Status;
	XScaler_Config *ScalerCfgPtr;
	u32 Rdy;

	/* Initialize the Scaler instance */
	ScalerCfgPtr = XScaler_LookupConfig(ScalerDeviceID);
	Status = XScaler_CfgInitialize(&ScalerInstance, ScalerCfgPtr,
			ScalerCfgPtr->BaseAddress);
	if (Status) {
		return 1;
	}

	/* DownScaling example */
	DownScale(&ScalerInstance);

	/* UpScaling example */
	UpScale(&ScalerInstance);

	/* All tests passed. Return success */
	return 0;
}

/*****************************************************************************/
/**
*
* This function shows the downscaling feature. This function downscales the
* video format from 1280x720 to 640x480.
*
* @param	ScalerInstPtr is the pointer to the instance of the Scaler
*		device.
*
* @return	None
*
* @note		None.
*
******************************************************************************/
static void DownScale(XScaler *ScalerInstPtr)
{
	int InWidth, InHeight, OutWidth, OutHeight;

	InWidth = 1280;
	InHeight = 720;
	OutWidth = 640;
	OutHeight = 480;

	ScalerSetup (ScalerInstPtr, InWidth, InHeight, OutWidth, OutHeight);

	return;
}

/*****************************************************************************/
/**
*
* This function shows the up scaling feature. This function up scales the
* video format from 640x480 to 1280x720.
*
* @param	ScalerInstPtr is the pointer to the instance of the Scaler
*		device.
*
* @return	None
*
* @note		None.
*
******************************************************************************/
static void UpScale(XScaler *ScalerInstPtr)
{
	int InWidth, InHeight, OutWidth, OutHeight;

	InWidth = 640;
	InHeight = 480;
	OutWidth = 1280;
	OutHeight = 720;

	ScalerSetup (ScalerInstPtr, InWidth, InHeight, OutWidth, OutHeight);

	return;
}

/*****************************************************************************/
/**
*
* This function sets up the scaler core for the feature demonstrations below.
* After the execution of this function, a set of coefficient value (containing
* 2 vertical and 2 horizontal coefficient banks) are loaded; aperture is set
* up and the scaling operation is started. This function is utilized by
* DownScale() and UpScale().
*
* @param	ScalerInstPtr is the pointer to the instance of the Scaler
*		device.
*
* @param	ScalerInWidth is the width of the input aperture.
*
* @param	ScalerInHeight is the height of the input aperture.
*
* @param	ScalerOutWidth is the width of the output aperture.
*
* @param	ScalerOutHeight is the height of the output aperture.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void ScalerSetup(XScaler *ScalerInstPtr,
			int ScalerInWidth, int ScalerInHeight,
			int ScalerOutWidth, int ScalerOutHeight)
{
	int i;
	u8 ChromaFormat;
	u8 ChromaLumaShareCoeffBank;
	u8 HoriVertShareCoeffBank;

	/*
	 * Disable the scaler before setup and tell the device not to pick up
	 * the register updates until all are done
	 */
	XScaler_Disable(ScalerInstPtr);
	XScaler_DisableRegUpdate(ScalerInstPtr);

	/*
	 * Load a set of Coefficient values
	 */

	/* Fetch Chroma Format and Coefficient sharing info */
	XScaler_GetCoeffBankSharingInfo(ScalerInstPtr,
					&ChromaFormat,
					&ChromaLumaShareCoeffBank,
					&HoriVertShareCoeffBank);

	CoeffBank.SetIndex = COEFF_SET_INDEX;
	CoeffBank.PhaseNum = ScalerInstPtr->Config.MaxPhaseNum;
	CoeffBank.TapNum = ScalerInstPtr->Config.VertTapNum;

	/* Locate coefficients for Horizontal scaling */
	CoeffBank.CoeffValueBuf = (u16 *)
		XScaler_CoefValueLookup(ScalerInWidth,
					ScalerOutWidth,
					CoeffBank.TapNum,
					CoeffBank.PhaseNum);

	/* Load coefficient bank for Horizontal Luma */
	XScaler_LoadCoeffBank(ScalerInstPtr, &CoeffBank);

	/* Horizontal Chroma bank is loaded only if chroma/luma sharing flag
	 * is not set */
	if (!ChromaLumaShareCoeffBank)
		XScaler_LoadCoeffBank(ScalerInstPtr, &CoeffBank);

	/* Vertical coeff banks are loaded only if horizontal/vertical sharing
	 * flag is not set
	 */
	if (!HoriVertShareCoeffBank) {

		/* Locate coefficients for Vertical scaling */
		CoeffBank.CoeffValueBuf = (u16 *)
			XScaler_CoefValueLookup(ScalerInHeight,
					ScalerOutHeight,
					CoeffBank.TapNum,
					CoeffBank.PhaseNum);

		/* Load coefficient bank for Vertical Luma */
		XScaler_LoadCoeffBank(ScalerInstPtr, &CoeffBank);

		/* Vertical Chroma coeff bank is loaded only if chroma/luma
		 * sharing flag is not set
		 */
		if (!ChromaLumaShareCoeffBank)
			XScaler_LoadCoeffBank(ScalerInstPtr, &CoeffBank);
	}

	/*
	 * Load phase-offsets into scaler
	 */
	StartFraction.LumaLeftHori = 0;
	StartFraction.LumaTopVert = 0;
	StartFraction.ChromaLeftHori = 0;
	StartFraction.ChromaTopVert = 0;
	XScaler_SetStartFraction(ScalerInstPtr, &StartFraction);

	/*
	 * Set up Aperture.
	 */
	Aperture.InFirstLine = 0;
	Aperture.InLastLine = ScalerInHeight - 1;

	Aperture.InFirstPixel = 0;
	Aperture.InLastPixel = ScalerInWidth - 1;

	Aperture.OutVertSize = ScalerOutHeight;
	Aperture.OutHoriSize = ScalerOutWidth;

	Aperture.SrcVertSize = ScalerInHeight;
        Aperture.SrcHoriSize = ScalerInWidth;

	XScaler_SetAperture(ScalerInstPtr, &Aperture);

	/*
	 * Set up phases
	 */
	XScaler_SetPhaseNum(ScalerInstPtr, ScalerInstPtr->Config.MaxPhaseNum,
				ScalerInstPtr->Config.MaxPhaseNum);

	/*
	 * Choose active set indexes for both vertical and horizontal directions
	 */
	XScaler_SetActiveCoeffSet(ScalerInstPtr, COEFF_SET_INDEX,
					COEFF_SET_INDEX);

	/*
	 * Enable the scaling operation
	 */
	XScaler_EnableRegUpdate(ScalerInstPtr);
	XScaler_Enable(ScalerInstPtr);

	return;
}
