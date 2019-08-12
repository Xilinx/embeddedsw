/******************************************************************************
*
* Copyright (C) 2012 - 2014 Xilinx, Inc.  All rights reserved.
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
* @file xrgb2ycrcb.c
* @addtogroup rgb2ycrcb_v7_1
* @{
*
* This file contains the implementation of the interface functions for
* RGB2YCRCB driver. Refer to the header file xrgb2ycrcb.h for more detailed
* information.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------------
* 5.00a tb     02/27/12 Updated for RGB2YCRCB v5.00.a.
* 7.0   adk    01/07/14 Changed the file name from "rgb2ycrcb.c" to
*                       "xrgb2ycrcb.c".
*
*                       Implemented the following functions:
*                       XRgb2YCrCb_CfgInitialize, XRgb2YCrCb_EnableDbgByPass,
*                       XRgb2YCrCb_IsDbgByPassEnabled,
*                       XRgb2YCrCb_DisableDbgBypass,
*                       XRgb2YCrCb_EnableDbgTestPattern,
*                       XRgb2YCrCb_IsDbgTestPatternEnabled,
*                       XRgb2YCrCb_DisableDbgTestPattern,
*                       XRgb2YCrCb_GetVersion, XRgb2YCrCb_GetDbgFrameCount,
*                       XRgb2YCrCb_GetDbgLineCount,
*                       XRgb2YCrCb_GetDbgPixelCount, XRgb2YCrCb_Setup,
*                       XRgb2YCrCb_SetActiveSize, XRgb2YCrCb_GetActiveSize,
*                       XRgb2YCrCb_SetYMax, XRgb2YCrCb_GetYMax,
*                       XRgb2YCrCb_SetYMin, XRgb2YCrCb_GetYMin,
*                       XRgb2YCrCb_SetCbMax, XRgb2YCrCb_GetCbMax,
*                       XRgb2YCrCb_SetCbMin, XRgb2YCrCb_GetCbMin,
*                       XRgb2YCrCb_SetCrMax, XRgb2YCrCb_GetCrMax,
*                       XRgb2YCrCb_SetCrMin, XRgb2YCrCb_GetCrMin,
*                       XRgb2YCrCb_SetYOffset, XRgb2YCrCb_GetYOffset,
*                       XRgb2YCrCb_SetCbOffset, XRgb2YCrCb_GetCbOffset,
*                       XRgb2YCrCb_SetCrOffset, XRgb2YCrCb_GetCrOffset,
*                       XRgb2YCrCb_SetCoefs, XRgb2YCrCb_GetCoefs,
*                       XRgb2YCrCb_Select_Standard,
*                       XRgb2YCrCb_Coefficient_Translation,
*                       XRgb2YCrCb_Select_OutputRange.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xrgb2ycrcb.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

/**
* Each of Callback functions to be called on different types of interrupts.
* These stub functions are set during XRgb2YCrCb_CfgInitialize as default
* callback functions. If application is not registered any of the callback
* function, these functions will be called for doing nothing.
*/
static void StubCallBack(void *CallBackRef);
static void StubErrCallBack(void *CallBackRef, u32 ErrorMask);

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function initializes the Rgb2YCrCb core. This function must be called
* prior to using the Rgb2YCrCb core. Initialization of the Rgb2YCrCb includes
* setting up the instance data and ensuring the hardware is in a quiescent
* state.
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb instance.
* @param	CfgPtr is a reference to a structure containing information
*		about a specific XRgb2YCrCb driver.
* @param	EffectiveAddr is the core base address in the virtual memory
*		address space. The caller is responsible for keeping the
*		address mapping from EffectiveAddr to the core physical
*		base address unchanged once this function is invoked.
*		Unexpected errors may occur if the address mapping changes
*		after this function is called. If address translation is not
*		used, pass in the physical address instead.
*
* @return	- XST_SUCCESS if initialization was successful.
*
* @note		None.
*
******************************************************************************/
int XRgb2YCrCb_CfgInitialize(XRgb2YCrCb *InstancePtr,
				XRgb2YCrCb_Config *CfgPtr, u32 EffectiveAddr)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddr != (u32)0x0);

	/* Setup the instance */
	(void)memset((void *)InstancePtr, 0, sizeof(XRgb2YCrCb));
	(void)memcpy((void *)&(InstancePtr->Config), (const void *)CfgPtr,
			sizeof(XRgb2YCrCb_Config));

	InstancePtr->Config.BaseAddress = EffectiveAddr;

	/* Set all handlers to stub values, let user configure this
	 * data later
	 */
	InstancePtr->ProcStartCallBack =
			(XRgb2YCrCb_CallBack)((void *)StubCallBack);
	InstancePtr->FrameDoneCallBack =
			(XRgb2YCrCb_CallBack)((void *)StubCallBack);
	InstancePtr->ErrCallBack =
			(XRgb2YCrCb_ErrorCallBack)((void *)StubErrCallBack);

	/* Reset the hardware and set the flag to indicate the
	 * driver is ready
	 */
	XRgb2YCrCb_Reset(InstancePtr);
	InstancePtr->IsReady = (u32)(XIL_COMPONENT_IS_READY);

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function sets the bypass bit of control register to switch the core to
* bypass mode if debug is enabled in the IP.
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb instance.
*
* @return	None.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
void XRgb2YCrCb_EnableDbgByPass(XRgb2YCrCb *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->Config.HasDebug != (u16)0x0);

	XRgb2YCrCb_WriteReg(InstancePtr->Config.BaseAddress,
		(XRGB_CONTROL_OFFSET),
			((XRgb2YCrCb_ReadReg(InstancePtr->Config.BaseAddress,
				(XRGB_CONTROL_OFFSET))) |
					(XRGB_CTL_BPE_MASK)));
}

/*****************************************************************************/
/**
*
* This function returns the current bypass mode of a core.
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb instance.
*
* @return
*		- TRUE if ByPass mode is enabled.
*		- FALSE if ByPpass mode is not enabled.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
int XRgb2YCrCb_IsDbgByPassEnabled(XRgb2YCrCb *InstancePtr)
{
	u32 DbgByPass;
	int Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.HasDebug != (u16)0x0);

	DbgByPass = (XRgb2YCrCb_ReadReg(InstancePtr->Config.BaseAddress,
				(XRGB_CONTROL_OFFSET))) & (XRGB_CTL_BPE_MASK);
	if (DbgByPass == (XRGB_CTL_BPE_MASK)) {
		Status = (TRUE);
	}
	else {
		Status = (FALSE);
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function disables Bypass mode.
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb core instance to be
*		worked on.
*
* @return	None.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
void XRgb2YCrCb_DisableDbgBypass(XRgb2YCrCb *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((InstancePtr)->Config.HasDebug != (u16)0x0);

	XRgb2YCrCb_WriteReg(InstancePtr->Config.BaseAddress,
		(XRGB_CONTROL_OFFSET),
		((XRgb2YCrCb_ReadReg(InstancePtr->Config.BaseAddress,
			(XRGB_CONTROL_OFFSET))) & (~(XRGB_CTL_BPE_MASK))));
}

/*****************************************************************************/
/**
*
* This function switches the core to test-pattern generator mode if debug
* feature is enabled.
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb instance.
*
* @return	None.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
void XRgb2YCrCb_EnableDbgTestPattern(XRgb2YCrCb *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->Config.HasDebug != (u16)0x0);

	XRgb2YCrCb_WriteReg(InstancePtr->Config.BaseAddress,
		(XRGB_CONTROL_OFFSET),
			((XRgb2YCrCb_ReadReg(InstancePtr->Config.BaseAddress,
			(XRGB_CONTROL_OFFSET))) | (XRGB_CTL_TPE_MASK)));
}

/*****************************************************************************/
/**
*
* This function returns the test-pattern generator mode (enabled or not), if
* debug feature is enabled.
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb instance.
*
* @return
*		- TRUE if Test-pattern mode is enabled.
*		- FALSE if Test-pattern mode is not enabled.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
int XRgb2YCrCb_IsDbgTestPatternEnabled(XRgb2YCrCb *InstancePtr)
{
	u32 DbgTestPattern;
	int Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((InstancePtr)->Config.HasDebug != (u16)0x0);

	DbgTestPattern = (XRgb2YCrCb_ReadReg(InstancePtr->Config.BaseAddress,
				(XRGB_CONTROL_OFFSET))) & (XRGB_CTL_TPE_MASK);

	if (DbgTestPattern == (XRGB_CTL_TPE_MASK)) {
		Status = (TRUE);
	}
	else {
		Status = (FALSE);
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function disables debug test pattern mode.
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb core instance to be
*		worked on.
*
* @return	None.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
void XRgb2YCrCb_DisableDbgTestPattern(XRgb2YCrCb *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->Config.HasDebug != (u16)0x0);

	XRgb2YCrCb_WriteReg((InstancePtr)->Config.BaseAddress,
		(XRGB_CONTROL_OFFSET),
		((XRgb2YCrCb_ReadReg(InstancePtr->Config.BaseAddress,
			(XRGB_CONTROL_OFFSET))) & (~(XRGB_CTL_TPE_MASK))));
}

/*****************************************************************************/
/**
*
* This function facilitates software identification of exact version of the
* RGB2YCrCb hardware (h/w).
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb instance.
*
* @return	Version, contents of a Version register.
*
* @note		None.
*
******************************************************************************/
u32 XRgb2YCrCb_GetVersion(XRgb2YCrCb *InstancePtr)
{
	u32 Version;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Read version register */
	Version = XRgb2YCrCb_ReadReg(InstancePtr->Config.BaseAddress,
					(XRGB_VERSION_OFFSET));
	return Version;
}

/*****************************************************************************/
/**
*
* This function returns the frame count, the number of frames processed since
* power-up. This is available only if the debugging feature is enabled.
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb instance.
*
* @return	DbgFrameCount, number of frames processed since power-up.
*
* @note		The SYSDEBUG0, or Frame Throughput Monitor register indicates
*		the number of frames processed since power-up or the last time
*		the core was reset. The SYSDEBUG registers can be useful to
*		identify external memory / frame buffer/ throughput bottlenecks
*		in a video system.
*		Debug functionality should be enabled.
*
******************************************************************************/
u32 XRgb2YCrCb_GetDbgFrameCount(XRgb2YCrCb *InstancePtr)
{
	u32 DbgFrameCount;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.HasDebug != (u16)0x0);

	/* Frame Throughput monitor */
	DbgFrameCount = XRgb2YCrCb_ReadReg(InstancePtr->Config.BaseAddress,
					(XRGB_SYSDEBUG0_OFFSET));

	return DbgFrameCount;
}

/*****************************************************************************/
/**
*
* This function returns Line count, the number of lines processed since
* power-up. This is available only if the debugging feature is enabled.
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb instance.
*
* @return	DbgLineCount, number of lines processed since power-up.
*
* @note		The SYSDEBUG1 or Line Throughput Monitor register indicates
*		the number of lines processed since power-up or the last time
*		the core was reset. The SYSDEBUG registers can be useful to
*		identify external memory / Frame buffer / throughput
*		bottlenecks in a video system.
*		Debug functionality should be enabled.
*
******************************************************************************/
u32 XRgb2YCrCb_GetDbgLineCount(XRgb2YCrCb *InstancePtr)
{
	u32 DbgLineCount;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.HasDebug != (u16)0x0);

	/* Line Throughput monitor */
	DbgLineCount = XRgb2YCrCb_ReadReg(InstancePtr->Config.BaseAddress,
					(XRGB_SYSDEBUG1_OFFSET));

	return DbgLineCount;
}

/*****************************************************************************/
/**
*
* This function returns the pixel count, the number of pixels processed since
* power up. This is available only if the debugging feature is enabled.
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb instance.
*
* @return	DbgPixelCount, number of pixels processed since power-up.
*
* @note		The SYSDEBUG2, or Pixel Throughput Monitor register indicates
*		the number of pixel processed since power-up or the last time
*		the core was reset. The SYSDEBUG registers can be useful to
*		identify external memory / Frame buffer / throughput
*		bottlenecks in a video system.
*		Debug functionality should be enabled.
*
******************************************************************************/
u32 XRgb2YCrCb_GetDbgPixelCount(XRgb2YCrCb *InstancePtr)
{
	u32 DbgPixelCount;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.HasDebug != (u16)0x0);

	/* Pixel Throughput monitor */
	DbgPixelCount = XRgb2YCrCb_ReadReg(InstancePtr->Config.BaseAddress,
					(XRGB_SYSDEBUG2_OFFSET));

	return DbgPixelCount;
}

/*****************************************************************************/
/**
*
* This function sets up double buffered ACTIVE_SIZE register and enables the
* register update.
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XRgb2YCrCb_Setup(XRgb2YCrCb *InstancePtr)
{
	u32 ActiveSize;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	XRgb2YCrCb_RegUpdateDisable(InstancePtr);

	/* Write into active size register */
	ActiveSize = (u32)(((u32)((InstancePtr->VSize)) <<
				(u32)(XRGB_ACTSIZE_NUM_LINE_SHIFT)) &
					(u32)(XRGB_ACTSIZE_NUM_LINE_MASK)) |
					((u32)((InstancePtr->HSize)) &
					(u32)(XRGB_ACTSIZE_NUM_PIXEL_MASK));
	XRgb2YCrCb_WriteReg(InstancePtr->Config.BaseAddress,
		(XRGB_ACTIVE_SIZE_OFFSET), ActiveSize);

	XRgb2YCrCb_RegUpdateEnable(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function sets active H/V sizes in the active size register.
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb instance.
* @param	HSize specifies the number of Active Pixels per scanline that
*		needs to be set within the range [32, 7680].
* @param	VSize specifies the number of Active Lines per frame that needs
*		to be set within the range [32, 7680].
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XRgb2YCrCb_SetActiveSize(XRgb2YCrCb *InstancePtr, u16 HSize, u16 VSize)
{
	u32 Size;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((VSize >= (u16)(XRGB_VSIZE_FIRST)) &&
			(VSize <= (u16)(XRGB_VSIZE_LAST)));
	Xil_AssertVoid((HSize >= (u16)(XRGB_HSIZE_FIRST)) &&
			(HSize <= (u16)(XRGB_HSIZE_LAST)));

	/* The core supports spatial resolutions from 32x32 to 7680x7680 */
	Size = (((u32)VSize) << ((u32)(XRGB_ACTSIZE_NUM_LINE_SHIFT))) |
					(HSize);
	XRgb2YCrCb_WriteReg(InstancePtr->Config.BaseAddress,
				(XRGB_ACTIVE_SIZE_OFFSET), Size);
}

/*****************************************************************************/
/**
*
* This function gets the active H/V sizes of the Rgb2YCrCb core from
* active size register.
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb instance.
* @param	HSize is a pointer to 16-bit variable in which the number of
*		Active Pixels per scanline is returned within the range
*		[32, 7680].
* @param	VSize is a pointer to 16-bit variable in which the number of
*		Active Lines per frame is returned within the range
*		[32, 7680].
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XRgb2YCrCb_GetActiveSize(XRgb2YCrCb *InstancePtr, u16 *HSize, u16 *VSize)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(HSize != NULL);
	Xil_AssertVoid(VSize != NULL);

	/* Reads Number of Active Pixels per scan line */
	*HSize = (u16)(XRgb2YCrCb_ReadReg(InstancePtr->Config.BaseAddress,
			(XRGB_ACTIVE_SIZE_OFFSET))) &
				(XRGB_ACTSIZE_NUM_PIXEL_MASK);

	/* Reads number of active lines per frame */
	*VSize = (u16)(XRgb2YCrCb_ReadReg(InstancePtr->Config.BaseAddress,
			(XRGB_ACTIVE_SIZE_OFFSET)) &
				(XRGB_ACTSIZE_NUM_LINE_MASK)) >>
					(XRGB_ACTSIZE_NUM_LINE_SHIFT);
}

/*****************************************************************************/
/**
*
* This function sets the maximum value allowed on the Luma (Y) channel of the
* output.
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb instance.
* @param	YMax specifies the maximum value within range [0, 255] of Luma
*		channel that needs to be set.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XRgb2YCrCb_SetYMax(XRgb2YCrCb *InstancePtr, u32 YMax)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(YMax <= (u32)XRGB_YMAX_LAST);

	XRgb2YCrCb_WriteReg(InstancePtr->Config.BaseAddress,
				(XRGB_YMAX_OFFSET), YMax);
}

/*****************************************************************************/
/**
*
* This function returns the maximum value of the Luma (Y) channel of the
* output.
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb instance.
*
* @return	YMax, Maximum value within range [0, 255] of the Luma channel.
*
* @note		None.
*
******************************************************************************/
u32 XRgb2YCrCb_GetYMax(XRgb2YCrCb *InstancePtr)
{
	u32 YMax;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	YMax = (XRgb2YCrCb_ReadReg(InstancePtr->Config.BaseAddress,
				(XRGB_YMAX_OFFSET)) & (XRGB_16_BIT_MASK));

	return YMax;
}

/*****************************************************************************/
/**
*
* This function sets the minimum value allowed on the Luma (Y) channel of the
* output.
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb instance.
* @param	YMin specifies the minimum value within range [0, 255] of Luma
*		channel that needs to be set.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XRgb2YCrCb_SetYMin(XRgb2YCrCb *InstancePtr, u32 YMin)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(YMin <= (u32)XRGB_YMIN_LAST);

	XRgb2YCrCb_WriteReg(InstancePtr->Config.BaseAddress,
				(XRGB_YMIN_OFFSET), YMin);
}

/*****************************************************************************/
/**
*
* This function returns the minimum value of the Luma (Y) channel of the
* output.
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb instance.
*
* @return	YMin, minimum value within range [0, 255] of the Luma channel.
*
* @note		None.
*
******************************************************************************/
u32 XRgb2YCrCb_GetYMin(XRgb2YCrCb *InstancePtr)
{
	u32 YMin;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	YMin = (XRgb2YCrCb_ReadReg(InstancePtr->Config.BaseAddress,
				(XRGB_YMIN_OFFSET)) & (XRGB_16_BIT_MASK));

	return YMin;
}

/*****************************************************************************/
/**
*
* This function sets the maximum value allowed on the Cb Chroma channel of the
* output.
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb instance.
* @param	CbMax specifies the maximum value within range [0, 255] of Cb
*		Chroma channel that needs to be set.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XRgb2YCrCb_SetCbMax(XRgb2YCrCb *InstancePtr, u32 CbMax)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CbMax <= (u32)XRGB_CBMAX_LAST);

	XRgb2YCrCb_WriteReg(InstancePtr->Config.BaseAddress,
				(XRGB_CBMAX_OFFSET), CbMax);
}

/*****************************************************************************/
/**
*
* This function returns the maximum value on the Cb Chroma channel of the
* output.
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb instance.
*
* @return	CbMax, maximum value within range [0, 255] of the Cb Chroma
*		Channel.
*
* @note		None.
*
******************************************************************************/
u32 XRgb2YCrCb_GetCbMax(XRgb2YCrCb *InstancePtr)
{
	u32 CbMax;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	CbMax = (XRgb2YCrCb_ReadReg(InstancePtr->Config.BaseAddress,
				(XRGB_CBMAX_OFFSET)) & (XRGB_16_BIT_MASK));

	return CbMax;
}

/*****************************************************************************/
/**
*
* This function sets the minimum value allowed on the Cb Chroma channel of the
* output.
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb instance.
* @param	CbMin specifies the maximum value within range [0, 255] of Cb
*		Chroma channel needs to set.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XRgb2YCrCb_SetCbMin(XRgb2YCrCb *InstancePtr, u32 CbMin)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CbMin <= (u32)XRGB_CBMIN_LAST);

	XRgb2YCrCb_WriteReg(InstancePtr->Config.BaseAddress,
				(XRGB_CBMIN_OFFSET), CbMin);
}

/*****************************************************************************/
/**
*
* This function returns the minimum value on the Cb Chroma channel of the
* output.
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb instance.
*
* @return	CbMin, minimum value within range [0, 255] on the Cb Chroma
*		Channel.
*
* @note		None.
*
******************************************************************************/
u32 XRgb2YCrCb_GetCbMin(XRgb2YCrCb *InstancePtr)
{
	u32 CbMin;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	CbMin = (XRgb2YCrCb_ReadReg(InstancePtr->Config.BaseAddress,
				(XRGB_CBMIN_OFFSET)) & (XRGB_16_BIT_MASK));

	return CbMin;
}
/*****************************************************************************/
/**
*
* This function sets the maximum value allowed on the Cr Chroma channel of the
* output.
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb instance.
* @param	CrMax specifies the maximum value within range [0, 255] of Cr
*		Chroma channel that needs to be set.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XRgb2YCrCb_SetCrMax(XRgb2YCrCb *InstancePtr, u32 CrMax)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CrMax <= (u32)XRGB_CRMAX_LAST);

	XRgb2YCrCb_WriteReg(InstancePtr->Config.BaseAddress,
				(XRGB_CRMAX_OFFSET), CrMax);
}

/*****************************************************************************/
/**
*
* This function returns the maximum value on the Cr Chroma channel of the
* output
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb instance.
*
* @return	CrMax, maximum value within range [0, 255] on the Cr Chroma
*		Channel.
*
* @note		None.
*
******************************************************************************/
u32 XRgb2YCrCb_GetCrMax(XRgb2YCrCb *InstancePtr)
{
	u32 CrMax;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	CrMax = (XRgb2YCrCb_ReadReg(InstancePtr->Config.BaseAddress,
				(XRGB_CRMAX_OFFSET)) & (XRGB_16_BIT_MASK));

	return CrMax;
}

/*****************************************************************************/
/**
*
* This function sets the minimum value allowed on the Cr Chroma channel of the
* output.
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb instance.
* @param	CrMin specifies the minimum value within range [0, 255] of Cr
*		Chroma channel that needs to be set.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XRgb2YCrCb_SetCrMin(XRgb2YCrCb *InstancePtr, u32 CrMin)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CrMin <= (u32)XRGB_CRMIN_LAST);

	XRgb2YCrCb_WriteReg(InstancePtr->Config.BaseAddress,
				(XRGB_CRMIN_OFFSET), CrMin);
}

/*****************************************************************************/
/**
*
* This function returns the minimum value on the Cr Chroma channel of the
* output.
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb instance.
*
* @return	CrMin, minimum value within range [0, 255] on the Cr Chroma
*		Channel.
*
* @note		None.
*
******************************************************************************/
u32 XRgb2YCrCb_GetCrMin(XRgb2YCrCb *InstancePtr)
{
	u32 CrMin;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	CrMin = (XRgb2YCrCb_ReadReg(InstancePtr->Config.BaseAddress,
			(XRGB_CRMIN_OFFSET)) & (XRGB_16_BIT_MASK));

	return CrMin;
}

/*****************************************************************************/
/**
*
* This function sets the offset compensation value for the Luma (Y) channel.
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb instance.
* @param	YOffset specifies the compensation value within range [0, 255]
*		of Luma channel that needs to be set.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XRgb2YCrCb_SetYOffset(XRgb2YCrCb *InstancePtr, u32 YOffset)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(YOffset <= (u32)XRGB_YOFFSET_LAST);

	XRgb2YCrCb_WriteReg(InstancePtr->Config.BaseAddress,
				(XRGB_YOFFSET_OFFSET), YOffset);
}

/*****************************************************************************/
/**
*
* This function returns the offset compensation value for the Luma (Y) channel.
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb instance.
*
* @return	YOffset is compensation value within range [0, 255] for the
*		Luma (Y) channel.
*
* @note		None.
*
******************************************************************************/
u32 XRgb2YCrCb_GetYOffset(XRgb2YCrCb *InstancePtr)
{
	u32 YOffset;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	YOffset = (XRgb2YCrCb_ReadReg(InstancePtr->Config.BaseAddress,
				(XRGB_YOFFSET_OFFSET)) & (XRGB_16_BIT_MASK));

	return YOffset;
}

/*****************************************************************************/
/**
*
* This function sets the offset compensation value for the Cb Chroma channel.
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb instance.
* @param	CbOffset specifies the compensation value within range [0, 255]
*		of Cb Chroma channel that needs to be set.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XRgb2YCrCb_SetCbOffset(XRgb2YCrCb *InstancePtr, u32 CbOffset)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CbOffset <= (u32)XRGB_CBOFFSET_LAST);

	XRgb2YCrCb_WriteReg(InstancePtr->Config.BaseAddress,
				(XRGB_CBOFFSET_OFFSET), CbOffset);
}

/*****************************************************************************/
/**
*
* This function returns the offset compensation value for the Cb Chroma channel.
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb instance.
*
* @return	CbOffset is the compensation value within range [0, 255] for
*		the Cb Chroma channel.
*
* @note		None.
*
******************************************************************************/
u32 XRgb2YCrCb_GetCbOffset(XRgb2YCrCb *InstancePtr)
{
	u32 CbOffset;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	CbOffset = (XRgb2YCrCb_ReadReg(InstancePtr->Config.BaseAddress,
				(XRGB_CBOFFSET_OFFSET)) & (XRGB_16_BIT_MASK));

	return CbOffset;
}

/*****************************************************************************/
/**
*
* This function sets the offset compensation value for the Cr Chroma channel.
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb instance.
* @param	CrOffset specifies the compensation value within range [0, 255]
*		of Cr Chroma channel that needs to be set.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XRgb2YCrCb_SetCrOffset(XRgb2YCrCb *InstancePtr, u32 CrOffset)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CrOffset <= (u32)XRGB_CROFFSET_LAST);

	XRgb2YCrCb_WriteReg(InstancePtr->Config.BaseAddress,
				(XRGB_CROFFSET_OFFSET), CrOffset);
}

/*****************************************************************************/
/**
*
* This function returns the offset compensation value of the Cr Chroma channel.
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb instance.
*
* @return	CrOffset is the compensation value within range [0, 255] of the
*		Cr Chroma channel.
*
* @note		None.
*
******************************************************************************/
u32 XRgb2YCrCb_GetCrOffset(XRgb2YCrCb *InstancePtr)
{
	u32 CrOffset;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	CrOffset = (XRgb2YCrCb_ReadReg(InstancePtr->Config.BaseAddress,
				(XRGB_CROFFSET_OFFSET)) & (XRGB_16_BIT_MASK));

	return CrOffset;
}

/*****************************************************************************/
/**
*
* This function sets A, B, C and D coefficients.
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb instance.
* @param	ACoef specifies the A coefficient value within the range
*		[0.0 to 1.0] that needs to be set.
* @param	BCoef specifies the B coefficient value within the range
*		[0.0 to 1.0] that needs to be set.
* @param	CCoef specifies the C coefficient value within the range
*		[0.0 to 1.0] that needs to be set.
* @param	DCoef specifies the D coefficient value within the range
*		[0.0 to 1.0] that needs to be set.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XRgb2YCrCb_SetCoefs(XRgb2YCrCb *InstancePtr, double ACoef, double BCoef,
				double CCoef, double DCoef)
{
	u32 CoefVal;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((ACoef >= 0.000) && (ACoef <= 1.000));
	Xil_AssertVoid((BCoef >= 0.000) && (BCoef <= 1.000));
	Xil_AssertVoid((CCoef >= 0.000) && (CCoef <= 1.000));
	Xil_AssertVoid((DCoef >= 0.000) && (DCoef <= 1.000));

	CoefVal = (u32)((ACoef) * (1 << (XRGB_16_BIT_COEF_SHIFT)));
	XRgb2YCrCb_WriteReg(InstancePtr->Config.BaseAddress,
					(XRGB_ACOEF_OFFSET), CoefVal);

	CoefVal = (u32)((BCoef) * (1 << (XRGB_16_BIT_COEF_SHIFT)));
	XRgb2YCrCb_WriteReg(InstancePtr->Config.BaseAddress,
					(XRGB_BCOEF_OFFSET), CoefVal);

	CoefVal = (u32)((CCoef) * (1 << (XRGB_16_BIT_COEF_SHIFT)));
	XRgb2YCrCb_WriteReg(InstancePtr->Config.BaseAddress,
					(XRGB_CCOEF_OFFSET), CoefVal);

	CoefVal = (u32)((DCoef) * (1 << (XRGB_16_BIT_COEF_SHIFT)));
	XRgb2YCrCb_WriteReg(InstancePtr->Config.BaseAddress,
					(XRGB_DCOEF_OFFSET), CoefVal);
}

/*****************************************************************************/
/**
*
* This function returns A, B, C and D coefficients.
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb instance.
* @param	ACoef is a pointer to double variable in which A coefficient is
*		returned within the range [0.0 to 1.0].
* @param	BCoef is a pointer to double variable in which B coefficient is
*		returned within the range [0.0 to 1.0].
* @param	CCoef is a pointer to double variable in which C coefficient is
*		returned within the range [0.0 to 1.0].
* @param	DCoef is a pointer to double variable in which D coefficient is
*		returned within the range [0.0 to 1.0].
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XRgb2YCrCb_GetCoefs(XRgb2YCrCb *InstancePtr, double *ACoef, double *BCoef,
				double *CCoef, double *DCoef)
{
	u32 CoefVal;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(ACoef != NULL);
	Xil_AssertVoid(BCoef != NULL);
	Xil_AssertVoid(CCoef != NULL);
	Xil_AssertVoid(DCoef != NULL);

	CoefVal = XRgb2YCrCb_ReadReg(InstancePtr->Config.BaseAddress,
					(XRGB_ACOEF_OFFSET));
	*ACoef = (double)CoefVal /
			(double)(1 << (XRGB_16_BIT_COEF_SHIFT));

	CoefVal = XRgb2YCrCb_ReadReg(InstancePtr->Config.BaseAddress,
					(XRGB_BCOEF_OFFSET));
	*BCoef = (double)CoefVal /
			(double)(1 << (XRGB_16_BIT_COEF_SHIFT));

	CoefVal = XRgb2YCrCb_ReadReg(InstancePtr->Config.BaseAddress,
					(XRGB_CCOEF_OFFSET));
	*CCoef = (double)CoefVal /
			(double)(1 << (XRGB_16_BIT_COEF_SHIFT));

	CoefVal = XRgb2YCrCb_ReadReg(InstancePtr->Config.BaseAddress,
					(XRGB_DCOEF_OFFSET));
	*DCoef = (double)CoefVal /
			(double)(1 << (XRGB_16_BIT_COEF_SHIFT));
}

/*****************************************************************************/
/**
*
* This function populates an XRgb2YCrCb_Coef_Inputs structure with the values
* from the selected video standard.
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb instance.
* @param	StandardSel needs to be set from enum XRgb_Standards value as :
*		0 = XRGB_STANDARD_ITU_601_SD
*		1 = XRGB_STANDARD_ITU_709_NTSC
*		2 = XRGB_STANDARD_ITU_709_PAL
*		3 = XRGB_STANDARD_YUV.
* @param	InputRange needs to be set from enum OutputRanges value as:
*		0 = XRGB_TV_16_TO_240,
*		1 = XRGB_STUDIO_16_TO_235,
*		2 = XRGB_GRAPHICS_0_TO_255.
* @param	DataWidth specifies the valid range of [8,10,12,16] that needs
*		to be set.
* @param	CoefIn specifies a pointer to a XRgb2YCrCb_Coef_Inputs
*		structure which is populated with the values from selected
*		video standard.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XRgb2YCrCb_Select_Standard(XRgb2YCrCb *InstancePtr,
				enum XRgb_Standards StandardSel,
				enum XRgb_OutputRanges InputRange, u32 DataWidth,
				struct XRgb2YCrCb_Coef_Inputs *CoefIn)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CoefIn != NULL);
	Xil_AssertVoid((DataWidth == (u32)(XRGB_DATA_WIDTH_16))  ||
			(DataWidth == (u32)(XRGB_DATA_WIDTH_10)) ||
			(DataWidth == (u32)(XRGB_DATA_WIDTH_12)) ||
			(DataWidth == (u32)(XRGB_DATA_WIDTH_8)));

	Xil_AssertVoid(StandardSel <(u32)(XRGB_STANDARD_CUSTOM));
	Xil_AssertVoid(InputRange <= (u32)(XRGB_GRAPHICS_0_TO_255));

	double ACoef[4][3] = {
				{0.299, 0.299,  0.2568},
				{0.299, 0.299,  0.2568},
				{0.2126, 0.2126, 0.1819},
				{0.299, 0.299, 0.299}
			};

	double BCoef[4][3] = {
				{0.114, 0.114,  0.0979},
				{0.114, 0.114,  0.0979},
				{0.0722, 0.0722, 0.0618},
				{0.114, 0.114, 0.114}
			};

	double CCoef[4][3] = {
				{0.713, 0.7295, 0.5910},
				{0.713, 0.7295, 0.5910},
				{0.6350, 0.6495, 0.6495},
				{0.877283, 0.877283, 0.877283}
			};

	double DCoef[4][3] = {
				{0.564, 0.5772, 0.5772},
				{0.564, 0.5772, 0.5772},
				{0.5389, 0.5512, 0.5512},
				{0.492111, 0.492111, 0.492111}
			};

	u32 Max[3] = {
			(240 * (1 << (DataWidth - 8))),
			(235 * (1 << (DataWidth - 8))),
			((1 << DataWidth) - 1)
		};
	u32 Min[3] = {
			(16 * (1 << (DataWidth - 8))),
			(16 * (1 << (DataWidth - 8))),
			0
		};

	u32 YOffset = (1 << (DataWidth - 4));
	u32 COffset = (u32)(1 << (DataWidth - 1));

	CoefIn->ACoef = ACoef[StandardSel][InputRange];
	CoefIn->BCoef = BCoef[StandardSel][InputRange];
	CoefIn->CCoef = CCoef[StandardSel][InputRange];
	CoefIn->DCoef = DCoef[StandardSel][InputRange];

	CoefIn->YOffset  = YOffset;
	CoefIn->CbOffset = COffset;
	CoefIn->CrOffset = COffset;

	CoefIn->YMax = Max[InputRange];
	CoefIn->YMin = Min[InputRange];
	CoefIn->CbMax = Max[InputRange];
	CoefIn->CbMin = Min[InputRange];
	CoefIn->CrMax = Max[InputRange];
	CoefIn->CrMin = Min[InputRange];
}

/*****************************************************************************/
/**
*
* This function translates the XRgb2YCrCb_Coef_Inputs structure into the
* XRgb2YCrCb_Coef_Outputs structure that can be used to program the core's
* registers. The XRgb2YCrCb_Coef_Inputs structure uses the same values as the
* core's GUIs. The XRgb2YCrCb_Coef_Outputs structure uses the values that can
* be programmed into the core's registers.
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb instance.
* @param	CoefIn specifies a pointer to a XRgb2YCrCb_Coef_Inputs
*		structure to be translated.
* @param	CoefOut specifies a pointer to a XRgb2YCrCb_Coef_Outputs
*		structure with translated values.
* @param	DataWidth specifies a valid range of [8,10,12,16] that needs
*		to be set.
*
* @return	RetVal, returns the 32-bit value with following bit information:
*		- bit(0)= ACoef + BCoef > 1.0.
*		- bit(1)= Y Offset outside data width range as
*			[-2^Data_Width, (2^Data_Width)-1].
*		- bit(2)= Cb Offset outside data width range as
*			[-2^Data_Width, (2^Data_Width)-1].
*		- bit(3)= Cr Offset outside data width range as
*			[-2^Data_Width, (2^Data_Width)-1].
*		- bit(4)= Y Max outside data width range as
*			[0, (2^Data_Width)-1].
*		- bit(5)= Y Min outside data width range as
*			[0, (2^Data_Width)-1].
*		- bit(6)= Cb Max outside data width range as
*			[0, (2^Data_Width)-1].
*		- bit(7)= Cb Min outside data width range as
*			[0, (2^Data_Width)-1].
*		- bit(8)= Cr Max outside data width range as
*			[0, (2^Data_Width)-1].
*		- bit(9)= Cr Min outside data width range as
*			[0, (2^Data_Width)-1].
*
* @note		None.
*
******************************************************************************/
u32 XRgb2YCrCb_Coefficient_Translation(XRgb2YCrCb *InstancePtr,
				struct XRgb2YCrCb_Coef_Inputs *CoefIn,
				struct XRgb2YCrCb_Coef_Outputs *CoefOut,
				u32 DataWidth)
{
	u32 RetVal = 0;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CoefIn != NULL);
	Xil_AssertNonvoid(CoefOut != NULL);

	Xil_AssertNonvoid( (DataWidth == (u32)(XRGB_DATA_WIDTH_16))  ||
			(DataWidth == (u32)(XRGB_DATA_WIDTH_10)) ||
			(DataWidth == (u32)(XRGB_DATA_WIDTH_12)) ||
			(DataWidth == (u32)(XRGB_DATA_WIDTH_8)));

	if((CoefIn->ACoef + CoefIn->BCoef) > (1.0)) {
		RetVal = (RetVal) | (0x1);
	}
	if((((int)CoefIn->YOffset) < ((int)-(1 << DataWidth))) ||
		(((int)CoefIn->YOffset) > ((int)((1 << DataWidth) -1 )))) {
		RetVal = (RetVal) | (0x2);
	}
	if((((int)CoefIn->CbOffset) < ((int)-(1 << DataWidth))) ||
		(((int)CoefIn->CbOffset) > ((int)((1 << DataWidth) - 1)))) {
		RetVal = (RetVal) | (0x4);
	}
	if((((int)CoefIn->CrOffset) < ((int)-(1 << DataWidth))) ||
		(((int)CoefIn->CrOffset) > ((int)((1 << DataWidth) - 1)))) {
		RetVal = (RetVal) | (0x8);
	}
	if(((int)CoefIn->YMax) > ((int)((1 << DataWidth) - 1))) {
		RetVal = (RetVal) | (0x10);
	}
	if(((int)CoefIn->YMin) > ((int)((1 << DataWidth) - 1))) {
		RetVal = (RetVal) | (0x20);
	}
	if(((int)CoefIn->CbMax) > ((int)((1 << DataWidth) - 1))) {
		RetVal = (RetVal) | (0x40);
	}
	if(((int)CoefIn->CbMin) > ((int)((1 << DataWidth) - 1))) {
		RetVal = (RetVal) | (0x80);
	}
	if(((int)CoefIn->CrMax) > ((int)((1 << DataWidth) - 1))) {
		RetVal = (RetVal) | (0x100);
	}
	if(((int)CoefIn->CrMin) > ((int)((1 << DataWidth) - 1))) {
		RetVal = (RetVal) | (0x200);
	}

	CoefOut->ACoef = CoefIn->ACoef * (1 << 16);
	CoefOut->BCoef = CoefIn->BCoef * (1 << 16);
	CoefOut->CCoef = CoefIn->CCoef * (1 << 16);
	CoefOut->DCoef = CoefIn->DCoef * (1 << 16);
	CoefOut->YOffset  = CoefIn->YOffset;
	CoefOut->CbOffset = CoefIn->CbOffset;
	CoefOut->CrOffset = CoefIn->CrOffset;
	CoefOut->YMax  = CoefIn->YMax;
	CoefOut->YMin  = CoefIn->YMin;
	CoefOut->CbMax = CoefIn->CbMax;
	CoefOut->CbMin = CoefIn->CbMin;
	CoefOut->CrMax = CoefIn->CrMax;
	CoefOut->CrMin = CoefIn->CrMin;

	return RetVal;
}

/*****************************************************************************/
/**
*
* This function governs the range of outputs Y, Cr and Cb by affecting the
* conversion coefficients as well as the clipping and clamping values.
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb instance.
* @param	Range needs to be set from the enum XRgb_OutputRanges
*		values as:
*		0 = XRGB_TV_16_TO_240,
*		1 = XRGB_STUDIO_16_TO_235,
*		2 = XRGB_GRAPHICS_0_TO_255.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XRgb2YCrCb_Select_OutputRange(XRgb2YCrCb *InstancePtr,
					enum XRgb_OutputRanges Range)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Range <= (u32)(XRGB_GRAPHICS_0_TO_255));

	/* Sets output range. */
	switch (Range) {
		case XRGB_TV_16_TO_240:
			(InstancePtr)->OutputRange =
					(u32)XRGB_TV_16_TO_240;
			break;

		case XRGB_STUDIO_16_TO_235:
			(InstancePtr)->OutputRange =
					(u32)XRGB_STUDIO_16_TO_235;
			break;

		case XRGB_GRAPHICS_0_TO_255:
			(InstancePtr)->OutputRange =
					(u32)XRGB_GRAPHICS_0_TO_255;
			break;

		default:
			;
			break;
	}
}

/*****************************************************************************/
/**
*
* This routine is a stub for the asynchronous callbacks. The stub is here in
* case the upper layer forgot to set the handlers. On initialization, All
* handlers except error handler are set to this callback. It is considered an
* error for this handler to be invoked.
*
* @param	CallBackRef is a callback reference passed in by the upper
*		layer when setting the callback functions, and passed back
*		to the upper layer when the callback is invoked.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void StubCallBack(void *CallBackRef)
{
	Xil_AssertVoid(CallBackRef != NULL);
	Xil_AssertVoidAlways();
}

/*****************************************************************************/
/**
*
* This routine is a stub for the asynchronous error interrupt callback. The
* stub is here in case the upper layer forgot to set the handler. On
* initialization, Error interrupt handler is set to this callback. It is
* considered an error for this handler to be invoked.
*
* @param	CallBackRef is a callback reference passed in by the upper
*		layer when setting the callback functions, and passed back
*		to the upper layer when the callback is invoked.
* @param	ErrorMask is a bit mask indicating the cause of the error. Its
*		value equals 'OR'ing one or more XRGB_IXR_*_MASK values defined
*		in xrgb2ycrcb_hw.h
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void StubErrCallBack(void *CallBackRef, u32 ErrorMask)
{
	Xil_AssertVoid(CallBackRef != NULL);
	Xil_AssertVoid(ErrorMask != (u32)0x0);
	Xil_AssertVoidAlways();
}

/** @} */
