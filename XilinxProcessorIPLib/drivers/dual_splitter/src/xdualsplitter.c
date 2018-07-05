/******************************************************************************
*
* Copyright (C) 2014 - 2015 Xilinx, Inc.  All rights reserved.
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
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
* IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
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
* @file xdualsplitter.c
* @addtogroup dual_splitter_v1_1
* @{
*
* This file contains the implementation of the interface functions for
* Dual Splitter core. Please refer to the header file xdualsplitter.h for
* more detailed information.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- --------------------------------------------------
* 1.00  sha 07/21/14 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdualsplitter.h"
#include "string.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

static void StubErrCallback(void *CallbackRef, u32 ErrorMask);

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function initializes the Dual Splitter core. This function must be
* called prior to using the Dual Splitter core. Initialization of the core
* includes setting up the instance data and ensuring the hardware is in a
* quiescent state.
*
* @param	InstancePtr is a pointer to the XDualSplitter core instance.
* @param	CfgPtr points to the configuration structure associated with
*		the Dual Splitter core.
* @param	EffectiveAddr is the base address of the device. If address
*		translation is being used, then this parameter must reflect the
*		virtual base address. Otherwise, the physical address should be
*		used.
*
* @return
*		- XST_SUCCESS if XDualSplitter_CfgInitialize was successful.
*
* @note		None.
*
******************************************************************************/
s32 XDualSplitter_CfgInitialize(XDualSplitter *InstancePtr,
				XDualSplitter_Config *CfgPtr,
				u32 EffectiveAddr)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddr != (u32)0x0);


	/* Setup the instance */
	(void)memset((void *)InstancePtr, 0, sizeof(XDualSplitter));
	(void)memcpy((void *)&(InstancePtr->Config), (const void *)CfgPtr,
			sizeof(XDualSplitter_Config));
	InstancePtr->Config.BaseAddress = EffectiveAddr;

	/* Set all handlers to stub values, let user configure
	 * this data later
	 */
	InstancePtr->ErrCallback =
			(XDualSplitter_ErrCallback)((void *)StubErrCallback);

	/* Reset the hardware and set the flag to indicate the driver is
	 * ready
	 */
	XDualSplitter_Reset(InstancePtr);
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function resets the Dual Splitter core instance. This reset effects the
* core immediately and may cause image tearing.
*
* @param	InstancePtr is a pointer to the XDualSplitter core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDualSplitter_Reset(XDualSplitter *InstancePtr)
{
	u32 Reset;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Read General Control register */
	Reset = XDualSplitter_ReadReg(InstancePtr->Config.BaseAddress,
			XDUSP_GENR_CTL_OFFSET) | XDUSP_GENR_CTL_RST_MASK;

	/* Write into General Control register */
	XDualSplitter_WriteReg(InstancePtr->Config.BaseAddress,
			XDUSP_GENR_CTL_OFFSET, Reset);
}

/*****************************************************************************/
/**
*
* This function sets the image size (width x height) of the Dual Splitter core.
*
* @param	InstancePtr is a pointer to the XDualSplitter core instance.
* @param	Height specifies the height of the input image that needs to
*		be set within the range [0, 2160].
* @param	Width specifies the width of the input image that needs to be
*		set within the range [0, 3840].
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDualSplitter_SetImageSize(XDualSplitter *InstancePtr, u16 Height,
				u16 Width)
{
	u32 ImageSize;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Height <= XDUSP_MAX_IMG_HEIGHT);
	Xil_AssertVoid(Width <= XDUSP_MAX_IMG_WIDTH);

	/* Calculate image size to fit into 32-bit register. */
	ImageSize = Width & XDUSP_TIME_CTL_WIDTH_MASK;
	ImageSize |= ((u32)Height << XDUSP_TIME_CTL_HEIGHT_SHIFT) &
				XDUSP_TIME_CTL_HEIGHT_MASK;

	/* Write into Time Control register */
	XDualSplitter_WriteReg(InstancePtr->Config.BaseAddress,
				XDUSP_TIME_CTL_OFFSET, ImageSize);
}

/*****************************************************************************/
/**
*
* This function gets the image size (width x height) of the Dual Splitter core.
*
* @param	InstancePtr is a pointer to the XDualSplitter core instance.
* @param	Height specifies a pointer to the 16-bit variable that will be
*		filled with height which is in the range [0, 2160].
* @param	Width specifies a pointer to the 16-bit variable that will be
*		filled with width which is in the range [0, 3840].
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDualSplitter_GetImageSize(XDualSplitter *InstancePtr, u16 *Height,
				u16 *Width)
{
	u32 ImageSize;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Height != NULL);
	Xil_AssertVoid(Width != NULL);

	/* Read from Time Control register */
	ImageSize = XDualSplitter_ReadReg(InstancePtr->Config.BaseAddress,
				XDUSP_TIME_CTL_OFFSET);

	*Width = (u16)(ImageSize & XDUSP_TIME_CTL_WIDTH_MASK);
	*Height = (u16)((ImageSize >> XDUSP_TIME_CTL_HEIGHT_SHIFT) &
			XDUSP_TIME_CTL_WIDTH_MASK);
}

/*****************************************************************************/
/**
*
* This function sets the image parameters to split into multiple segments.
*
* @param	InstancePtr is a pointer to the XDualSplitter core instance.
* @param	InputSamples specifies the number of input samples per clock
*		that needs to be set within the range [0, 4].
* @param	OutputSamples specifies the number of output samples per clock
*		that needs to be set within the range [0, 4].
* @param	ImageSegments specifies the number of image segments
*		that needs to be set within the range [0, 4].
* @param	Overlap specifies the overlap of the samples in the segments.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDualSplitter_SetImgParam(XDualSplitter *InstancePtr, u8 InputSamples,
				u8 OutputSamples, u8 ImageSegments, u8 Overlap)
{
	u32 Data;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InputSamples <= XDUSP_MAX_INPUT_SAMPLES);
	Xil_AssertVoid(OutputSamples <= XDUSP_MAX_OUTPUT_SAMPLES);
	Xil_AssertVoid(ImageSegments <= XDUSP_MAX_SEGMENTS);

	/* Calculate Image parameters to fit into 32-bit register. */
	Data = InputSamples & XDUSP_CORE_CTL_IN_SAMPLES_MASK;
	Data |= ((u32)OutputSamples <<
			XDUSP_CORE_CTL_OUT_SAMPLES_SHIFT) &
				XDUSP_CORE_CTL_OUT_SAMPLES_MASK;
	Data |= ((u32)ImageSegments <<
			XDUSP_CORE_CTL_IMG_SEG_SHIFT) &
				XDUSP_CORE_CTL_IMG_SEG_MASK;
	Data |= ((u32)Overlap <<
			XDUSP_CORE_CTL_OVRLAP_SEG_SHIFT) &
				XDUSP_CORE_CTL_OVRLAP_SEG_MASK;

	/* Write into Core Control register */
	XDualSplitter_WriteReg(InstancePtr->Config.BaseAddress,
			XDUSP_CORE_CTL_OFFSET, Data);
}

/*****************************************************************************/
/**
*
* This function gets the image parameters of the Dual Splitter core.
*
* @param	InstancePtr is a pointer to the XDualSplitter core instance.
* @param	InputSamples specifies a pointer to the 8-bit variable that
*		will be filled with number of input samples which is in the
*		range [0, 4]
* @param	OutputSamples specifies a pointer to the 8-bit variable that
*		will be filled with number of output samples which is in the
*		range [0, 4]
* @param	ImageSegments specifies a pointer to the 8-bit variable that
*		will be filled with number of image segments which is in the
*		range [0, 4]
* @param	Overlap specifies a pointer to the 8-bit variable that will be
*		filled with number of samples overlapping the segments.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDualSplitter_GetImgParam(XDualSplitter *InstancePtr, u8 *InputSamples,
				u8 *OutputSamples, u8 *ImageSegments,
				u8 *Overlap)
{
	u32 Data;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InputSamples != NULL);
	Xil_AssertVoid(OutputSamples != NULL);
	Xil_AssertVoid(ImageSegments != NULL);
	Xil_AssertVoid(Overlap != NULL);

	/* Read from Core Control register */
	Data = XDualSplitter_ReadReg(InstancePtr->Config.BaseAddress,
				XDUSP_CORE_CTL_OFFSET);

	/* Extract Image parameters. */
	*InputSamples = (u8)(Data & XDUSP_CORE_CTL_IN_SAMPLES_MASK);
	*OutputSamples = (u8)((Data &
			XDUSP_CORE_CTL_OUT_SAMPLES_MASK) >>
				XDUSP_CORE_CTL_OUT_SAMPLES_SHIFT);
	*ImageSegments = (u8)((Data &
			XDUSP_CORE_CTL_IMG_SEG_MASK) >>
				XDUSP_CORE_CTL_IMG_SEG_SHIFT);
	*Overlap = (u8)((Data &
			XDUSP_CORE_CTL_OVRLAP_SEG_MASK) >>
				XDUSP_CORE_CTL_OVRLAP_SEG_SHIFT);
}

/*****************************************************************************/
/**
*
* This routine is a stub for the asynchronous error interrupt callback. The
* stub is here in case the upper layer forgot to set the handler. On
* initialization, error interrupt handler is set to this callback. It is
* considered as an error for this handler to be invoked.
*
* @param	CallbackRef is a callback reference passed in by the upper
*		layer when setting the callback functions and passed back
*		to the upper layer when the callback is invoked.
* @param	ErrorMask is a bit mask indicating the cause of the error. Its
* 		value equals 'OR'ing one or more XDUSP_ERR_*_MASK values
*		defined in xdualsplitter_hw.h.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void StubErrCallback(void *CallbackRef, u32 ErrorMask)
{
	(void)CallbackRef;
	(void)ErrorMask;
	Xil_AssertVoidAlways();
}
/** @} */
