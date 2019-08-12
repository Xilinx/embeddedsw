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
* @file xycrcb2rgb.c
* @addtogroup ycrcb2rgb_v7_1
* @{
*
* This file contains the implementation of the interface functions for
* YCRCB2RGB core. Refer to the header file xycrcb2rgb.h for more detailed
* information.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ----   -------- ----------------------------------------------------
* 5.00a tb     02/28/12 Updated for YCRCB2RGB v5.00.a.
* 7.0   adk    01/31/14 Changed the file name from "ycrcb2rgb.c" to
*                       "xycrcb2rgb.c".
*
*                       Implemented following functions:
*                       XYCrCb2Rgb_CfgInitialize, XYCrCb2Rgb_EnableDbgByPass,
*                       XYCrCb2Rgb_IsDbgByPassEnabled,
*                       XYCrCb2Rgb_DisableDbgBypass,
*                       XYCrCb2Rgb_EnableDbgTestPattern,
*                       XYCrCb2Rgb_IsDbgTestPatternEnabled,
*                       XYCrCb2Rgb_DisableDbgTestPattern,
*                       XYCrCb2Rgb_GetVersion, XYCrCb2Rgb_GetDbgFrameCount,
*                       XYCrCb2Rgb_GetDbgLineCount,
*                       XYCrCb2Rgb_GetDbgPixelCount, XYCrCb2Rgb_Setup,
*                       XYCrCb2Rgb_SetActiveSize, XYCrCb2Rgb_GetActiveSize,
*                       XYCrCb2Rgb_SetRGBMax, XYCrCb2Rgb_GetRGBMax,
*                       XYCrCb2Rgb_SetRGBMin, XYCrCb2Rgb_GetRGBMin,
*                       XYCrCb2Rgb_SetROffset, XYCrCb2Rgb_GetROffset,
*                       XYCrCb2Rgb_SetGOffset, XYCrCb2Rgb_GetGOffset,
*                       XYCrCb2Rgb_SetBOffset, XYCrCb2Rgb_GetBOffset,
*                       XYCrCb2Rgb_SetCoefs, XYCrCb2Rgb_GetCoefs,
*                       XYCrCb2Rgb_Select_Standard,
*                       XYCrCb2Rgb_Coefficient_Translation,
*                       XYCrCb2Rgb_Select_OutputRange.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xycrcb2rgb.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

/**
* Each of the callback functions to be called on different types of interrupts.
* These stub functions are set during XYCrCb2Rgb_CfgInitialize as default
* callback functions. If application is not registered any of the callback
* function, these functions will be called for doing nothing.
*/
static void StubCallBack(void *CallBackRef);
static void StubErrCallBack(void *CallBackRef, u32 ErrorMask);

static double XYCrCb2Rgb_Pow2(u32 Num);

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function initializes the YCrCb2Rgb core. This function must be called
* prior to using the YCrCb2Rgb core. Initialization of the YCrCb2Rgb includes
* setting up the instance data and ensuring the hardware is in a quiescent
* state.
*
* @param	InstancePtr is a pointer to the XYCrCb2Rgb instance.
* @param	CfgPtr is a reference to a structure containing information
*		about a specific XYCrCb2Rgb driver.
* @param	EffectiveAddr is the core base address in the virtual memory
*		address space. The caller is responsible for keeping the
*		address mapping from EffectiveAddr to the core physical base
*		address unchanged once this function is invoked. Unexpected
*		errors may occur if the address mapping changes after this
*		function is called. If address translation is not used, pass
*		in the physical address instead.
*
* @return	- XST_SUCCESS if initialization was successful.
*
* @note		None.
*
******************************************************************************/
int XYCrCb2Rgb_CfgInitialize(XYCrCb2Rgb *InstancePtr,
				XYCrCb2Rgb_Config *CfgPtr, u32 EffectiveAddr)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddr != (u32)0x0);

	/* Setup the instance */
	(void)memset((void *)InstancePtr, 0, sizeof(XYCrCb2Rgb));
	(void)memcpy((void *)(&(InstancePtr->Config)), (const void *)CfgPtr,
					sizeof(XYCrCb2Rgb_Config));

	InstancePtr->Config.BaseAddress = EffectiveAddr;

	/* Set all handlers to stub values, let user configure this
	 * data later
	 */
	InstancePtr->ProcStartCallBack =
				(XYCrCb2Rgb_CallBack)((void *)StubCallBack);
	InstancePtr->FrameDoneCallBack =
				(XYCrCb2Rgb_CallBack)((void *)StubCallBack);
	InstancePtr->ErrCallBack =
			(XYCrCb2Rgb_ErrorCallBack)((void *)StubErrCallBack);

	/* Reset the hardware and set the flag to indicate the driver is
	 * ready
	 */
	XYCrCb2Rgb_Reset(InstancePtr);
	InstancePtr->IsReady = (u32)(XIL_COMPONENT_IS_READY);

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function sets the bypass bit of control register to switch the core to
* bypass mode if debug is enabled in the IP.
*
* @param	InstancePtr is a pointer to the XYCrCb2Rgb instance.
*
* @return	None.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
void XYCrCb2Rgb_EnableDbgByPass(XYCrCb2Rgb *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->Config.HasDebug != (u16)0x0);

	XYCrCb2Rgb_WriteReg(InstancePtr->Config.BaseAddress,
			(XYCC_CONTROL_OFFSET),
		XYCrCb2Rgb_ReadReg(InstancePtr->Config.BaseAddress,
			(XYCC_CONTROL_OFFSET)) | (XYCC_CTL_BPE_MASK));
}

/*****************************************************************************/
/**
*
* This function returns the current bypass mode of the core.
*
* @param	InstancePtr is a pointer to the XYCrCb2Rgb instance.
*
* @return
*		- TRUE if Bypass mode is enabled.
*		- FALSE if Bypass mode is not enabled.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
int XYCrCb2Rgb_IsDbgByPassEnabled(XYCrCb2Rgb *InstancePtr)
{
	u32 DbgByPass;
	int Status;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.HasDebug != (u16)0x0);

	DbgByPass = (XYCrCb2Rgb_ReadReg(InstancePtr->Config.BaseAddress,
			(XYCC_CONTROL_OFFSET))) & (XYCC_CTL_BPE_MASK);

	if (DbgByPass == (XYCC_CTL_BPE_MASK)) {
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
* This function disables Bypass mode of the core.
*
* @param	InstancePtr is a pointer to the XYCrCb2Rgb instance.
*
* @return	None.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
void XYCrCb2Rgb_DisableDbgBypass(XYCrCb2Rgb *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->Config.HasDebug != (u16)0x0);

	XYCrCb2Rgb_WriteReg(InstancePtr->Config.BaseAddress,
		(XYCC_CONTROL_OFFSET),
		((XYCrCb2Rgb_ReadReg(InstancePtr->Config.BaseAddress,
			(XYCC_CONTROL_OFFSET))) & (~(XYCC_CTL_BPE_MASK))));
}

/*****************************************************************************/
/**
*
* This function switches the core to test-pattern generator mode if debug
* feature is enabled.
*
* @param	InstancePtr is a pointer to the XYCrCb2Rgb instance.
*
* @return	None.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
void XYCrCb2Rgb_EnableDbgTestPattern(XYCrCb2Rgb *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->Config.HasDebug != (u16)0x0);

	XYCrCb2Rgb_WriteReg(InstancePtr->Config.BaseAddress,
		(XYCC_CONTROL_OFFSET),
		XYCrCb2Rgb_ReadReg(InstancePtr->Config.BaseAddress,
			(XYCC_CONTROL_OFFSET)) | (XYCC_CTL_TPE_MASK));
}

/*****************************************************************************/
/**
*
* This function returns the test-pattern generator mode (enabled or not), if
* debug feature is enabled.
*
* @param	InstancePtr is a pointer to the XYCrCb2Rgb instance.
*
* @return
*		- TRUE if Test-pattern mode is enabled.
*		- FALSE if Test-pattern mode is not enabled.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
int XYCrCb2Rgb_IsDbgTestPatternEnabled(XYCrCb2Rgb *InstancePtr)
{
	u32 DbgTestPattern;
	int Status;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.HasDebug != (u16)0x0);

	DbgTestPattern = XYCrCb2Rgb_ReadReg(InstancePtr->Config.BaseAddress,
			(XYCC_CONTROL_OFFSET)) & (XYCC_CTL_TPE_MASK);

	if (DbgTestPattern == (XYCC_CTL_TPE_MASK)) {
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
* @param	InstancePtr is a pointer to the XYCrCb2Rgb instance.
*
* @return	None.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
void XYCrCb2Rgb_DisableDbgTestPattern(XYCrCb2Rgb *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->Config.HasDebug != (u16)0x0);

	XYCrCb2Rgb_WriteReg(InstancePtr->Config.BaseAddress,
		(XYCC_CONTROL_OFFSET),
		((XYCrCb2Rgb_ReadReg(InstancePtr->Config.BaseAddress,
			(XYCC_CONTROL_OFFSET))) & (~(XYCC_CTL_TPE_MASK))));
}

/*****************************************************************************/
/**
*
* This function facilitates software identification of exact version of the
* YCrCb2rGB hardware (h/w).
*
*
* @param	InstancePtr is a pointer to the XYCrCb2Rgb instance.
*
* @return	Version, contents of a Version register.
*
* @note		None.
*
******************************************************************************/
u32 XYCrCb2Rgb_GetVersion(XYCrCb2Rgb *InstancePtr)
{
	u32 Version;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Read Version register */
	Version = XYCrCb2Rgb_ReadReg(InstancePtr->Config.BaseAddress,
					(XYCC_VERSION_OFFSET));

	return Version;
}

/*****************************************************************************/
/**
*
* This function returns the frame count, the number of frames processed since
* power-up. This is available only if the debugging feature is enabled.
*
* @param	InstancePtr is a pointer to the XYCrCb2Rgb instance.
*
* @return	DbgFrameCount, number of frames processed since power-up.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
u32 XYCrCb2Rgb_GetDbgFrameCount(XYCrCb2Rgb *InstancePtr)
{
	u32 DbgFrameCount;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.HasDebug != (u16)0x0);

	/* Frame Throughput monitor */
	DbgFrameCount = XYCrCb2Rgb_ReadReg(InstancePtr->Config.BaseAddress,
						(XYCC_SYSDEBUG0_OFFSET));

	return DbgFrameCount;
}

/*****************************************************************************/
/**
*
* This function returns Line count, the number of lines processed since
* power-up. This is available only if the debugging feature is enabled.
*
* @param	InstancePtr is a pointer to the XYCrCb2Rgb instance.
*
* @return	DbgLineCount, number of lines processed since power-up.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
u32 XYCrCb2Rgb_GetDbgLineCount(XYCrCb2Rgb *InstancePtr)
{
	u32 DbgLineCount;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.HasDebug != (u16)0x0);

	/* Line Throughput monitor */
	DbgLineCount = XYCrCb2Rgb_ReadReg(InstancePtr->Config.BaseAddress,
						(XYCC_SYSDEBUG1_OFFSET));

	return DbgLineCount;
}

/*****************************************************************************/
/**
*
* This function returns the pixel count, the number of pixels processed since
* power up. This is available only if the debugging feature is enabled.
*
* @param	InstancePtr is a pointer to the XYCrCb2Rgb instance.
*
* @return	DbgPixelCount, number of pixels processed since power-up.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
u32 XYCrCb2Rgb_GetDbgPixelCount(XYCrCb2Rgb *InstancePtr)
{
	u32 DbgPixelCount;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.HasDebug != (u16)0x0);

	/* Pixel Throughput monitor */
	DbgPixelCount = XYCrCb2Rgb_ReadReg(InstancePtr->Config.BaseAddress,
						(XYCC_SYSDEBUG2_OFFSET));

	return DbgPixelCount;
}

/*****************************************************************************/
/**
*
* This function sets up double buffered active size register and enables the
* register update.
*
* @param	InstancePtr is a pointer to the XYCrCb2Rgb instance.
*
* @return	None.
*
* @note 	None.
*
******************************************************************************/
void XYCrCb2Rgb_Setup(XYCrCb2Rgb *InstancePtr)
{
	u32 ActiveSize;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	XYCrCb2Rgb_RegUpdateDisable(InstancePtr);

	/* Write into active size register */
	ActiveSize = ((((u32)InstancePtr->VSize) &
			(u32)(XYCC_ACTSIZE_NUM_PIXEL_MASK)) <<
				(XYCC_ACTSIZE_NUM_LINE_SHIFT)) |
					((InstancePtr->HSize) &
					(u32)(XYCC_ACTSIZE_NUM_PIXEL_MASK));

	XYCrCb2Rgb_WriteReg(InstancePtr->Config.BaseAddress,
				(XYCC_ACTIVE_SIZE_OFFSET), ActiveSize);
	XYCrCb2Rgb_RegUpdateEnable(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function sets active H/V sizes in the active frame size register.
*
* @param	InstancePtr is a pointer to the XYCrCb2Rgb instance.
* @param	HSize specifies the number of Active Pixels per scanline that
*		needs to be set within the range [32, 8192].
* @param	VSize specifies the number of Active Lines per frame that needs
*		to be set within the range [32, 8192].
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XYCrCb2Rgb_SetActiveSize(XYCrCb2Rgb *InstancePtr, u16 HSize, u16 VSize)
{
	u32 Size;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((VSize >= (u16)(XYCC_ACT_SIZE_FIRST)) &&
			(VSize <= (u16)(XYCC_ACT_SIZE_LAST)));
	Xil_AssertVoid((HSize >= (u16)(XYCC_ACT_SIZE_FIRST)) &&
			(HSize <= (u16)(XYCC_ACT_SIZE_LAST)));

	Size = (((u32)VSize) << ((u32)(XYCC_ACTSIZE_NUM_LINE_SHIFT))) |
					(HSize);
	XYCrCb2Rgb_WriteReg(InstancePtr->Config.BaseAddress,
					(XYCC_ACTIVE_SIZE_OFFSET), Size);
}

/*****************************************************************************/
/**
*
* This function gets the active H/V sizes of the YCrCb2Rgb core from
* active size register.
*
* @param	InstancePtr is a pointer to the XYCrCb2Rgb instance.
* @param	HSize is a pointer to 16-bit variable in which the number of
*		Active Pixels per scanline is returned within the range
*		[32, 8192].
* @param	VSize is a pointer to 16-bit variable in which the number of
*		Active Lines per frame is returned within the range
*		[32, 8192].
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XYCrCb2Rgb_GetActiveSize(XYCrCb2Rgb *InstancePtr, u16 *HSize, u16 *VSize)
{
	u32 Data;
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(HSize != NULL);
	Xil_AssertVoid(VSize != NULL);

	Data = XYCrCb2Rgb_ReadReg(InstancePtr->Config.BaseAddress,
			(XYCC_ACTIVE_SIZE_OFFSET));
	/* Reads Number of Active Pixels per scan line */
	*VSize = (u16)((Data &
				(XYCC_ACTSIZE_NUM_LINE_MASK)) >>
					(XYCC_ACTSIZE_NUM_LINE_SHIFT));

	/* Reads number of active lines per frame */
	*HSize = (u16)(Data &
					(XYCC_ACTSIZE_NUM_PIXEL_MASK));
}

/*****************************************************************************/
/**
*
* This function sets the RGB maximum value on YCrCb2Rgb channels of the output.
*
* @param	InstancePtr is a pointer to the XYCrCb2Rgb instance.
* @param	RGBMax is the maximum value within the range [0, 255] on the
*		RGB channels of the output.
*
* @return	None.
*
* @note		Clipping functionality should be enabled.
*
******************************************************************************/
void XYCrCb2Rgb_SetRGBMax(XYCrCb2Rgb *InstancePtr, u32 RGBMax)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(RGBMax <= (u32)XYCC_RGBMAX_MIN_LAST);
	Xil_AssertVoid(InstancePtr->Config.HasClip != (u16)0x0);

	XYCrCb2Rgb_WriteReg(InstancePtr->Config.BaseAddress,
				(XYCC_RGBMAX_OFFSET), RGBMax);
}

/*****************************************************************************/
/**
*
* This function gets the maximum value on RGB channels of the output.
*
* @param	InstancePtr is a pointer to the XYCrCb2Rgb instance.
*
* @return	RGBMax is the maximum value within the range [0, 255] from RGB
*		channels of the output.
*
* @note		Clipping functionality should be enabled.
*
******************************************************************************/
u32 XYCrCb2Rgb_GetRGBMax(XYCrCb2Rgb *InstancePtr)
{
	u32 RGBMax;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.HasClip != (u16)0x0);

	RGBMax = (XYCrCb2Rgb_ReadReg(InstancePtr->Config.BaseAddress,
			(XYCC_RGBMAX_OFFSET)) & XYCC_16_BIT_MASK);

	return RGBMax;
}

/*****************************************************************************/
/**
*
* This function sets minimum value on RGB channels of the output.
*
* @param	InstancePtr is a pointer to the XYCrCb2Rgb instance.
* @param	RGBMin is the  minimum value within the range [0, 255] on RGB
*		channels of the output.
*
* @return	None.
*
* @note		Clamping functionality should be enabled.
*
******************************************************************************/
void XYCrCb2Rgb_SetRGBMin(XYCrCb2Rgb *InstancePtr, u32 RGBMin)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(RGBMin <= (u32)XYCC_RGBMAX_MIN_LAST);
	Xil_AssertVoid(InstancePtr->Config.HasClamp != (u16)0x0);

	XYCrCb2Rgb_WriteReg(InstancePtr->Config.BaseAddress,
				(XYCC_RGBMIN_OFFSET), RGBMin);
}

/*****************************************************************************/
/**
*
* This function gets the minimum value on RGB channels of the output.
*
* @param	InstancePtr is a pointer to the XYCrCb2Rgb instance.
*
* @return	RGBMin is the minimum value within the range [0, 255] from RGB
*		channels of the output.
*
* @note		Clamping functionality should be enabled.
*
******************************************************************************/
u32 XYCrCb2Rgb_GetRGBMin(XYCrCb2Rgb *InstancePtr)
{
	u32 RGBMin;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.HasClamp != (u16)0x0);

	RGBMin = (XYCrCb2Rgb_ReadReg(InstancePtr->Config.BaseAddress,
			(XYCC_RGBMIN_OFFSET)) & XYCC_16_BIT_MASK);

	return RGBMin;
}

/*****************************************************************************/
/**
*
* This function sets offset compensation value on the Red channel.
*
* @param	InstancePtr is a pointer to the XYCrCb2Rgb instance.
* @param	ROffset is compensation value within the range [0, 255] to be
*		set on the Red channel.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XYCrCb2Rgb_SetROffset(XYCrCb2Rgb *InstancePtr, u32 ROffset)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(ROffset <= (u32)(XYCC_RGBOFFSET_LAST));

	XYCrCb2Rgb_WriteReg(InstancePtr->Config.BaseAddress,
					(XYCC_ROFFSET_OFFSET), ROffset);
}

/*****************************************************************************/
/**
*
* This function gets offset compensation value from the Red channel.
*
* @param	InstancePtr is a pointer to the XYCrCb2Rgb instance.
*
* @return	ROffset is a compensation value within the range [0, 255] from
*		the Red channel.
*
* @note		None.
*
******************************************************************************/
u32 XYCrCb2Rgb_GetROffset(XYCrCb2Rgb *InstancePtr)
{
	u32 ROffset;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	ROffset = XYCrCb2Rgb_ReadReg(InstancePtr->Config.BaseAddress,
						(XYCC_ROFFSET_OFFSET));

	return ROffset;
}

/*****************************************************************************/
/**
*
* This function sets offset compensation value on the Green channel.
*
* @param	InstancePtr is a pointer to the XYCrCb2Rgb instance.
* @param	GOffset is a compensation value within the range [0, 255] to be
*		set on the Green channel.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XYCrCb2Rgb_SetGOffset(XYCrCb2Rgb *InstancePtr, u32 GOffset)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(GOffset <= (u32)(XYCC_RGBOFFSET_LAST));

	XYCrCb2Rgb_WriteReg(InstancePtr->Config.BaseAddress,
				(XYCC_GOFFSET_OFFSET), GOffset);
}

/*****************************************************************************/
/**
*
* This function gets offset compensation value from the Green channel.
*
* @param	InstancePtr is a pointer to the XYCrCb2Rgb instance.
*
* @return	GOffset is a compensation value within the range [0, 255] from
*		the Green channel.
*
* @note		None.
*
******************************************************************************/
u32 XYCrCb2Rgb_GetGOffset(XYCrCb2Rgb *InstancePtr)
{
	u32 GOffset;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	GOffset = XYCrCb2Rgb_ReadReg(InstancePtr->Config.BaseAddress,
					(XYCC_GOFFSET_OFFSET));

	return GOffset;
}

/*****************************************************************************/
/**
*
* This function sets offset compensation value on the Blue channel.
*
* @param	InstancePtr is a pointer to the XYCrCb2Rgb instance.
* @param	BOffset is a compensation value within the range [0, 255] to be
*		set on the Blue channel.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XYCrCb2Rgb_SetBOffset(XYCrCb2Rgb *InstancePtr, u32 BOffset)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(BOffset <= (u32)(XYCC_RGBOFFSET_LAST));

	XYCrCb2Rgb_WriteReg(InstancePtr->Config.BaseAddress,
					(XYCC_BOFFSET_OFFSET),BOffset);
}

/*****************************************************************************/
/**
*
* This function gets offset compensation value from the Blue channel.
*
* @param	InstancePtr is a pointer to the XYCrCb2Rgb instance.
*
* @return	BOffset is a compensation value within the range [0, 255] from
*		the Blue channel.
*
* @note		None.
*
******************************************************************************/
u32 XYCrCb2Rgb_GetBOffset(XYCrCb2Rgb *InstancePtr)
{
	u32 BOffset;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	BOffset = XYCrCb2Rgb_ReadReg(InstancePtr->Config.BaseAddress,
					(XYCC_BOFFSET_OFFSET));

	return BOffset;
}

/*****************************************************************************/
/**
*
* This function sets A, B, C and D coefficients.
*
* @param	InstancePtr is a pointer to the XYCrCb2Rgb instance.
* @param	Coef specifies a pointer to XYCrCb2Rgb_Coefficients structure
*		in which ACoef, BCoef, CCoef, DCoef members value within the
*		range [0.0, 1.0] that needs to be set.
*
* @return	None.
*
* @note		Floating point coefficients are represented in 17-bit fixed
*		point format where 17 bits indicates integer portion (Mantissa)
*		of the number exclusive of sign bit.
*
******************************************************************************/
void XYCrCb2Rgb_SetCoefs(XYCrCb2Rgb *InstancePtr,
				struct XYCrCb2Rgb_Coefficients *Coef)
{
	u32 CoefVal;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Coef != NULL);

	CoefVal = (u32)((Coef->ACoef) * (1 << (XYCC_16_BIT_COEF_SHIFT)));
	XYCrCb2Rgb_WriteReg(InstancePtr->Config.BaseAddress,
					(XYCC_ACOEF_OFFSET), CoefVal);

	CoefVal = (u32)((Coef->BCoef) * (1 << (XYCC_16_BIT_COEF_SHIFT)));
	XYCrCb2Rgb_WriteReg(InstancePtr->Config.BaseAddress,
					(XYCC_BCOEF_OFFSET), CoefVal);

	CoefVal = (u32)((Coef->CCoef) * (1 << (XYCC_16_BIT_COEF_SHIFT)));
	XYCrCb2Rgb_WriteReg(InstancePtr->Config.BaseAddress,
					(XYCC_CCOEF_OFFSET), CoefVal);

	CoefVal = (u32)((Coef->DCoef) * (1 << (XYCC_16_BIT_COEF_SHIFT)));
	XYCrCb2Rgb_WriteReg(InstancePtr->Config.BaseAddress,
					(XYCC_DCOEF_OFFSET), CoefVal);
}

/*****************************************************************************/
/**
*
* This function returns A, B, C and D coefficients.
*
* @param	InstancePtr is a pointer to the XYCrCb2Rgbb instance.
* @param	Coef specifies a pointer to XYCrCb2Rgb_Coefficients structure
*		in which ACoef, BCoef, CCoef, DCoef members value will be
*		updated within the range [0.0, 1.0].
*
* @return	None.
*
* @note		Floating point coefficients are represented in 17-bit fixed
*		point format where 17 bits indicates integer portion (Mantissa)
*		of the number exclusive of sign bit.
*
******************************************************************************/
void XYCrCb2Rgb_GetCoefs(XYCrCb2Rgb *InstancePtr,
				struct XYCrCb2Rgb_Coefficients *Coef)
{
	u32 CoefVal;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Coef != NULL);

	CoefVal = XYCrCb2Rgb_ReadReg(InstancePtr->Config.BaseAddress,
							(XYCC_ACOEF_OFFSET));
	Coef->ACoef = (double)CoefVal /
			(double)(1 << (XYCC_16_BIT_COEF_SHIFT));

	CoefVal = XYCrCb2Rgb_ReadReg(InstancePtr->Config.BaseAddress,
							(XYCC_BCOEF_OFFSET));
	Coef->BCoef = (double)CoefVal /
			(double)(1 << (XYCC_16_BIT_COEF_SHIFT));

	CoefVal = XYCrCb2Rgb_ReadReg(InstancePtr->Config.BaseAddress,
							(XYCC_CCOEF_OFFSET));
	Coef->CCoef = (double)CoefVal /
			(double)(1 << (XYCC_16_BIT_COEF_SHIFT));

	CoefVal = XYCrCb2Rgb_ReadReg(InstancePtr->Config.BaseAddress,
							(XYCC_DCOEF_OFFSET));
	Coef->DCoef = (double)CoefVal /
			(double)(1 << (XYCC_16_BIT_COEF_SHIFT));
}

/*****************************************************************************/
/**
*
* This function populates an XYCrCb2Rgb_Coef_Inputs structure with the values
* from the selected video standard.
*
* @param	InstancePtr is a pointer to the XYCrCb2Rgbb instance.
* @param	StandardSel needs to be set from enum XYcc_Standards value as :
*		0 = XYCC_STANDARD_ITU_601_SD
*		1 = XYCC_STANDARD_ITU_709_NTSC
*		2 = XYCC_STANDARD_ITU_709_PAL
*		3 = XYCC_STANDARD_YUV.
* @param	InputRange needs to be set from enum XYcc_OutputRanges value as:
*		0 = XYCC_TV_16_TO_240,
*		1 = XYCC_STUDIO_16_TO_235,
*		2 = XYCC_GRAPHICS_0_TO_255.
* @param	DataWidth specifies the valid range of [8,10,12,16] that needs
*		to be set.
* @param	CoefIn specifies a pointer to a XYCrCb2Rgb_Coef_Inputs
*		structure which is populated with the values from selected
*		video standard.
*
* @return	None.
*
* @note		Floating point coefficients are represented in 17-bit fixed
*		point format where 17 bits indicates integer portion (Mantissa)
*		of the number exclusive of sign bit.
*
******************************************************************************/
void XYCrCb2Rgb_Select_Standard(XYCrCb2Rgb *InstancePtr,
				enum XYcc_Standards StandardSel,
				enum XYcc_OutputRanges InputRange, u32 DataWidth,
				struct XYCrCb2Rgb_Coef_Inputs *CoefIn)
{

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CoefIn != NULL);
	Xil_AssertVoid( (DataWidth == (u32)(XYCC_DATA_WIDTH_8))  ||
			(DataWidth == (u32)(XYCC_DATA_WIDTH_10)) ||
			(DataWidth == (u32)(XYCC_DATA_WIDTH_12)) ||
			(DataWidth == (u32)(XYCC_DATA_WIDTH_16)));
	Xil_AssertVoid(StandardSel <(u32)(XYCC_STANDARD_CUSTOM));
	Xil_AssertVoid(InputRange <= (u32)(XYCC_GRAPHICS_0_TO_255));

	double ACoef[4][3] = {	{ 0.299, 0.299, 0.2568 },
				{ 0.299, 0.299, 0.2568 },
				{ 0.2126, 0.2126, 0.1819},
				{ 0.299, 0.299, 0.299 }
			};

	double BCoef[4][3] = {
				{ 0.114, 0.114, 0.0979 },
				{ 0.114, 0.114, 0.0979 },
				{ 0.0722, 0.0722, 0.0618 },
				{ 0.114, 0.114, 0.114 }
			};

	double CCoef[4][3] = {
				{ 0.713, 0.7295, 0.5910 },
				{ 0.713, 0.7295, 0.5910 },
				{ 0.6350, 0.6495, 0.6495 },
				{ 0.877283, 0.877283, 0.877283 }
			};
	double DCoef[4][3] = {
				{ 0.564, 0.5772, 0.5772 },
				{ 0.564, 0.5772, 0.5772 },
				{ 0.5389, 0.5512, 0.5512 },
				{ 0.492111, 0.492111, 0.492111}
			};

	u32 YOffset = (u32)((u32)1 << (DataWidth - 4));
	u32 COffset = (u32)((u32)1 << (DataWidth - 1));

	u32 Max[3] = { (240 * (1 << (DataWidth -8))),
			(235 * (1 << (DataWidth - 8))),
			((1 << DataWidth) - 1)
		};

	u32 Min[3] = {
			(16 * (1 << (DataWidth - 8))),
			(16 * (1 << (DataWidth - 8))),
			0
		};

	CoefIn->ACoef = ACoef[StandardSel][InputRange];
	CoefIn->BCoef = BCoef[StandardSel][InputRange];
	CoefIn->CCoef = CCoef[StandardSel][InputRange];
	CoefIn->DCoef = DCoef[StandardSel][InputRange];

	CoefIn->YOffset  = YOffset;
	CoefIn->CbOffset = COffset;
	CoefIn->CrOffset = COffset;

	CoefIn->RgbMax = Max[InputRange];
	CoefIn->RgbMin = Min[InputRange];
}

/*****************************************************************************/
/**
*
* This function translates the XYCrCb2Rgb_Coef_Inputs structure into the
* XYCrCb2Rgb_Coef_Outputs structure that can be used to program the core's
* registers. The XYCrCb2Rgb_Coef_Inputs structure uses the same values as the
* core's GUIs. The XYCrCb2Rgb_Coef_Outputs structure uses the values that can
* be programmed into the core's registers.
*
* @param	InstancePtr is a pointer to the XYCrCb2Rgbb instance.
* @param	CoefIn specifies a pointer to a XYCrCb2Rgb_Coef_Inputs
*		structure to be translated.
* @param	CoefOut specifies a pointer to a XYCrCb2Rgb_Coef_Outputs
*		structure with translated values.
* @param	DataWidth specifies a valid range of [8,10,12,16] that needs
*		to be set.
* @param	MWidth specifies a valid range from min(32, DataWidth + 17)
*
* @return	RetVal, returns value with following bit information:
*		- bit(0) = 1 = data width outside range [8, 10, 12, 16]
*		- bit(1) = Acoef + Bcoef > 1.0
*		- bit(2) = Y Offset outside data width range:
*			[-2^DataWidth, (2^DataWidth)-1].
*		- bit(3) = Cb Offset outside data width range:
*			[-2^DataWidth, (2^DataWidth)-1].
*		- bit(4) = Cr Offset outside data width range:
*			[-2^DataWidth, (2^DataWidth)-1].
*		- bit(5) = RGB Max outside data width range:
*			[0, (2^DataWidth)-1].
*		- bit(6) = RGB Min outside data width range:
*			[0, (2^DataWidth)-1].
*
* @note		Floating point coefficients are represented in 17-bit fixed
*		point format where 17 bits indicates integer portion (Mantissa)
*		of the number exclusive of sign bit.
*
******************************************************************************/
u32 XYCrCb2Rgb_Coefficient_Translation(XYCrCb2Rgb *InstancePtr,
				struct XYCrCb2Rgb_Coef_Inputs *CoefIn,
				struct XYCrCb2Rgb_Coef_Outputs *CoefOut,
				u32 DataWidth, u32 MWidth)
{
	u32 RetVal = 0;
	u32 ScaleCoeffs;
	double ScaleIc2m;
	u32 CoefRange;

	double ACoef;
	double BCoef;
	double CCoef;
	double DCoef;

	u32 YOffset;
	u32 CbOffset;
	u32 CrOffset;

	u32 RgbMax;
	u32 RgbMin;

	/* Converted coefficients */
	double AConvCoef;
	double BConvCoef;
	double CConvCoef;
	double DConvCoef;

	/* Quantized coefficients */
	u32 AQuntCoef;
	u32 BQuntCoef;
	u32 CQuntCoef;
	u32 DQuntCoef;

	/* Input Width */
	u32 IWidth;

	/* Output Width */
	u32 OWidth;

	/* Common Width */
	u32 CWidth;

	double RoundingConst;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CoefIn != NULL);
	Xil_AssertNonvoid(CoefOut != NULL);
	Xil_AssertNonvoid((DataWidth == (u32)(XYCC_DATA_WIDTH_8)) ||
			  (DataWidth == (u32)(XYCC_DATA_WIDTH_10)) ||
			  (DataWidth == (u32)(XYCC_DATA_WIDTH_12)) ||
			  (DataWidth == (u32)(XYCC_DATA_WIDTH_16)));
	Xil_AssertNonvoid(((MWidth) == (u32)(((u32)XYCC_DATA_WIDTH_8) +
			  ((u32)17))) ||
			  (MWidth == (u32)((u32)XYCC_DATA_WIDTH_10 +
			  (u32)17)) || (MWidth ==
			  (u32)((u32)XYCC_DATA_WIDTH_12 + (u32)17)) ||
			  (MWidth == (u32)(((u32)XYCC_DATA_WIDTH_16 +
			  (u32)17) - ((u32)1))));

	IWidth = DataWidth;
	OWidth = DataWidth;
	CoefRange = 2;
	CWidth = 17;

	/* Store values from XYCrCb2Rgb_Coef_Inputs structure into local
	 * variables.
	 */
	ACoef = CoefIn->ACoef;
	BCoef = CoefIn->BCoef;
	CCoef = CoefIn->CCoef;
	DCoef = CoefIn->DCoef;
	YOffset = CoefIn->YOffset;
	CbOffset = CoefIn->CbOffset;
	CrOffset = CoefIn->CrOffset;
	RgbMax = CoefIn->RgbMax;
	RgbMin = CoefIn->RgbMin;

	if ((ACoef + BCoef) > 1.0) {
		RetVal = (RetVal) | (u32)(0x1);
	}
	if (((int)YOffset < (-((int)(1 << (u32)IWidth)))) ||
			((int)YOffset > (int)((1 << (u32)IWidth) - 1))) {
		RetVal = (RetVal) | (u32)(0x2);
	}
	if (((int)CbOffset < (-((int)(1 << (u32)IWidth)))) ||
			((int)CbOffset > (int)((1 << (u32)IWidth) - 1))) {
		RetVal = (RetVal) | (u32)(0x4);
	}
	if (((int)CrOffset < (-((int)(1 << (u32)IWidth)))) ||
			((int)CrOffset > (int)((1 << (u32)IWidth) - 1))) {
		RetVal = (RetVal) | (u32)(0x8);
	}
	if ((int)RgbMax > (int)((1 << (u32)IWidth) - 1)) {
		RetVal = (RetVal) | (u32)(0x10);
	}
	if ((int)RgbMin > (int)((1 << (u32)IWidth) - 1)) {
			RetVal = (RetVal) | (u32)(0x20);
	}

	AConvCoef = 1 / CCoef;
	BConvCoef = -(ACoef / CCoef / (1 - ACoef - BCoef));
	CConvCoef = -(BCoef / DCoef / (1 - ACoef - BCoef));
	DConvCoef = 1 / DCoef;

	ScaleCoeffs = XYCrCb2Rgb_Max(1, (1 << (CWidth - CoefRange - 1)));

	/* +0.5 turns truncation to biased rounding */
	RoundingConst = XYCrCb2Rgb_Pow2(MWidth - OWidth - CoefRange -2);
	ScaleIc2m = XYCrCb2Rgb_Pow2(((u32)1 + MWidth) - IWidth - CWidth);

	/* Quantize coefficients */
	AQuntCoef = (u32)(AConvCoef * ScaleCoeffs);
	BQuntCoef = (u32)(BConvCoef * ScaleCoeffs);
	CQuntCoef = (u32)(CConvCoef * ScaleCoeffs);
	DQuntCoef = (u32)(DConvCoef * ScaleCoeffs);

	CoefOut->ACoef = AQuntCoef;
	CoefOut->BCoef = BQuntCoef;
	CoefOut->CCoef = CQuntCoef;
	CoefOut->DCoef = DQuntCoef;

	/* Assuming signals are OWIDTH + 1 bits wide after the offset
	 * compensating rounder's / adders.
	 */
	CoefOut->ROffset = (u32)((RoundingConst - (((AQuntCoef * CrOffset) +
			(YOffset * ScaleCoeffs)))) * ScaleIc2m);
	CoefOut->GOffset = (u32)((RoundingConst - (((BQuntCoef * CrOffset) +
			(CQuntCoef * CbOffset)) + (YOffset * ScaleCoeffs))) *
				ScaleIc2m);
	CoefOut->BOffset = (u32)((RoundingConst - ((DQuntCoef * CbOffset) +
			(YOffset * ScaleCoeffs))) * ScaleIc2m);

	CoefOut->RgbMax  = RgbMax;
	CoefOut->RgbMin  = RgbMin;

	return RetVal;
}

/*****************************************************************************/
/**
*
* This function governs the range of outputs R, G and B by affecting the
* conversion coefficients as well as the clipping and clamping values.
*
* @param	InstancePtr is a pointer to the XYCrCb2Rgb instance.
* @param	Range needs to be set from the enum XYcc_OutputRanges values as:
*		0 = XYCC_TV_16_TO_240,
*		1 = XYCC_STUDIO_16_TO_235,
*		2 = XYCC_GRAPHICS_0_TO_255.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XYCrCb2Rgb_Select_OutputRange(XYCrCb2Rgb *InstancePtr,
					enum XYcc_OutputRanges Range)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Range <= (u32)(XYCC_GRAPHICS_0_TO_255));

	/* Sets output range. */
	switch(Range) {
		case XYCC_TV_16_TO_240:
			InstancePtr->OutputRange = (u32)XYCC_TV_16_TO_240;
			break;

		case XYCC_STUDIO_16_TO_235:
			InstancePtr->OutputRange = (u32)XYCC_STUDIO_16_TO_235;
			break;

		case XYCC_GRAPHICS_0_TO_255:
			InstancePtr->OutputRange = (u32)XYCC_GRAPHICS_0_TO_255;
			break;
		default :
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
* 		layer when setting the callback functions, and passed back
* 		to the upper layer when the callback is invoked.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void StubCallBack(void *CallBackRef)
{
	(void)CallBackRef;
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
*		value equals 'OR'ing one or more XYCC_IXR_*_MASK values defined
*		in xycrcb2rgb_hw.h.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void StubErrCallBack(void *CallBackRef, u32 ErrorMask)
{
	(void)CallBackRef;
	(void)ErrorMask;
}

/*****************************************************************************/
/**
*
* This function converts a given number into pow(2, Num)
*
* @param	Num is a number used as a power for base 2.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static double XYCrCb2Rgb_Pow2(u32 Num)
{
	double Power ;

	if ((Num > (u32)0x0)) {
		Power = (1 << Num);
	}
	else {
		Power = (double)(1.000)/(1 << (-Num));
	}

	return Power;
}
/** @} */
