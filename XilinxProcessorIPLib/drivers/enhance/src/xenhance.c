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
* @file xenhance.c
* @addtogroup enhance_v7_1
* @{
*
* This file contains the implementation of the interface functions for
* Enhance core. Refer to the header file xenhance.h for more detailed
* information.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- --- -------- ---------------------------------------------------
* 2.00a vc  12/14/10 Updated for ENHANCE V2.0
* 3.00a rc  09/11/11 Updated for ENHANCE V3.0
* 4.00a vyc 04/24/12 Updated for ENHANCE V4.00.a
*                    Converted from xio.h to xil_io.h, translating
*                    basic type, MB cache functions, exceptions and
*                    assertion to xil_io format.
* 5.00a vyc 06/19/13 Updated for ENHANCE V8.0
*                    New edge enhancement algorithm and registers
*                    Noise reduction support added
* 6.0   adk 19/12/13 Updated as per the New Tcl API's
* 7.0   adk 02/19/14 Changed the filename from enhance.c to xenhance.c.
*                    Modified the following functions
*                    XENHANCE_CfgInitialize -> XEnhance_CfgInitialize
*                    XENHANCE_Setup -> XEnhance_Setup
*
*                    Implemented the following functions:
*                    XEnhance_GetVersion, XEnhance_EnableDbgByPass
*                    XEnhance_IsDbgByPassEnabled, XEnhance_DisableDbgBypass
*                    XEnhance_EnableDbgTestPattern,
*                    XEnhance_IsDbgTestPatternEnabled
*                    XEnhance_DisableDbgTestPattern
*                    XEnhance_GetDbgFrameCount, XEnhance_GetDbgLineCount,
*                    XEnhance_GetDbgPixelCount, XEnhance_SetActiveSize,
*                    XEnhance_GetActiveSize, XEnhance_SetNoiseThreshold,
*                    XEnhance_GetNoiseThreshold, XEnhance_SetEdgeStrength,
*                    XEnhance_GetEdgeStrength, XEnhance_SetHaloSuppress
*                    XEnhance_GetHaloSuppress.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xenhance.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

static void StubErrCallBack(void *CallBackRef, u32 ErrorMask);
static void StubCallBack(void *CallBackRef);

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/****************************************************************************/
/**
*
* This function initializes a Enhance core.  This function must be called
* prior to using a Enhance core. Initialization of a Enhance includes
* setting up the instance data, and ensuring the hardware is in a quiescent
* state.
*
* @param	InstancePtr is a pointer to XEnhance instance to be worked on.
* @param	CfgPtr is a reference to a configuration structure
*		containing information about the Enhance core.
* @param	EffectiveAddr is the base address of the core. If address
*		translation is being used then this parameter must
*		reflect the virtual base address. Otherwise, the physical
*		address should be used.
*
* @return
*		- XST_SUCCESS if initialization was successful.
*
* @note		None.
*
******************************************************************************/
int XEnhance_CfgInitialize(XEnhance *InstancePtr, XEnhance_Config *CfgPtr,
				u32 EffectiveAddr)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddr != ((u32)0x0U));

	/* Setup the instance */
	(void)memset((void *)InstancePtr, 0, sizeof(XEnhance));
	(void)memcpy((void *)&(InstancePtr->Config), (const void *)CfgPtr,
			sizeof(XEnhance_Config));
	InstancePtr->Config.BaseAddress = EffectiveAddr;

	/* Set all handlers to stub values, let user configure this data
	 * later
	 */
	InstancePtr->ProcStartCallBack =
				(XEnhance_CallBack)((void *)StubCallBack);
	InstancePtr->FrameDoneCallBack =
				(XEnhance_CallBack)((void *)StubCallBack);
	InstancePtr->ErrCallBack =
			(XEnhance_ErrorCallBack)((void *)StubErrCallBack);

	/* Reset the hardware and set the flag to indicate the core is
	 * ready
	 */
	XEnhance_Reset(InstancePtr);
	InstancePtr->IsReady = (u32)(XIL_COMPONENT_IS_READY);

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function sets the input/output frame size in Active Size register and
* enables the register update.
*
* @param	InstancePtr is a pointer to XEnhance instance to be worked on.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XEnhance_Setup(XEnhance *InstancePtr)
{
	u32 Data;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	XEnhance_RegUpdateDisable(InstancePtr);

	/* Write active size register */
	Data = ((((InstancePtr)->VSize) << (XENH_ACTSIZE_NUM_LINE_SHIFT)) &
			(XENH_ACTSIZE_NUM_LINE_MASK)) |
				(((InstancePtr)->HSize) &
					(XENH_ACTSIZE_NUM_PIXEL_MASK));

	XEnhance_WriteReg((InstancePtr)->Config.BaseAddress,
		(XENH_ACTIVE_SIZE_OFFSET), Data);
	XEnhance_RegUpdateEnable(InstancePtr);
}

/*****************************************************************************/
/**
*
* This sets the bypass bit of the control register to switch the core to bypass
* mode if debug is enabled in the Enhance core.
*
* @param	InstancePtr is a pointer to XEnhance instance to be worked on.
*
* @return	None.
*
* @note		Debug functionality should be enabled..
*
******************************************************************************/
void XEnhance_EnableDbgByPass(XEnhance *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((InstancePtr)->Config.HasDebug != (u16)0x0U);

	/* Write into control register to set debug bypass. */
	XEnhance_WriteReg((InstancePtr)->Config.BaseAddress,
		(XENH_CONTROL_OFFSET),
		(XEnhance_ReadReg((InstancePtr)->Config.BaseAddress,
			(XENH_CONTROL_OFFSET)) | ((XENH_CTL_BPE_MASK))));
}

/*****************************************************************************/
/**
*
* This function gets the current status of the bypass setting of the Enhance
* core.
*
* @param	InstancePtr is a pointer to XEnhance instance to be worked on.
*
* @return	Core debug bypass mode.
*		- TRUE = Bypass mode is enabled.
*		- FALSE = Bypass mode is not enabled.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
int XEnhance_IsDbgByPassEnabled(XEnhance *InstancePtr)
{
	u32 Data;
	int Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((InstancePtr)->Config.HasDebug != (u16)0x0U);

	/* Read from control register to know debug bypass status. */
	Data = XEnhance_ReadReg((InstancePtr)->Config.BaseAddress,
			(XENH_CONTROL_OFFSET)) & ((u32)((XENH_CTL_BPE_MASK)));
	if (Data == (XENH_CTL_BPE_MASK)) {
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
* This function disables Bypass mode of the Enhance core.
*
* @param	InstancePtr is a pointer to XEnhance instance to be worked on.
*
* @return	None.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
void XEnhance_DisableDbgBypass(XEnhance *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((InstancePtr)->Config.HasDebug != (u16)0x0U);

	/* Write into control register to disable debug bypass. */
	XEnhance_WriteReg((InstancePtr)->Config.BaseAddress,
		(XENH_CONTROL_OFFSET),
		(XEnhance_ReadReg((InstancePtr)->Config.BaseAddress,
			(XENH_CONTROL_OFFSET)) & (~(XENH_CTL_BPE_MASK))));
}

/*****************************************************************************/
/**
*
* This function sets the test-pattern mode of the Enhance core if debug
* features is enabled.
*
* @param	InstancePtr is a pointer to XEnhance instance to be worked on.
*
* @return	None.
*
* @note 	Debug functionality should be enabled.
*
******************************************************************************/
void XEnhance_EnableDbgTestPattern(XEnhance *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->Config.HasDebug != (u16)0x0U);

	/* Write into control register to set test pattern. */
	XEnhance_WriteReg((InstancePtr)->Config.BaseAddress,
		(XENH_CONTROL_OFFSET),
			((XEnhance_ReadReg((InstancePtr)->Config.BaseAddress,
				(XENH_CONTROL_OFFSET))) |
					((XENH_CTL_TPE_MASK))));
}

/*****************************************************************************/
/**
*
* This function gets the test-pattern mode if debug feature is enabled.
*
* @param	InstancePtr is a pointer to XEnhance instance to be worked on.
*
* @return	Test-pattern generator mode.
*		- TRUE = Test-pattern mode is enabled.
*		- FALSE = Test-pattern mode is not enabled.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
int XEnhance_IsDbgTestPatternEnabled(XEnhance *InstancePtr)
{
	u32 Data;
	int Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.HasDebug != (u16)0x0U);

	/* Read from control register to know debug test pattern status. */
	Data = XEnhance_ReadReg((InstancePtr)->Config.BaseAddress,
			(XENH_CONTROL_OFFSET)) & ((XENH_CTL_TPE_MASK));
	if (Data == (XENH_CTL_TPE_MASK)) {
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
* This function disables the test Pattern mode of the Enhance core.
*
* @param	InstancePtr is a pointer to XEnhance instance to be worked on.
*
* @return	None.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
void XEnhance_DisableDbgTestPattern(XEnhance *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((InstancePtr)->Config.HasDebug != (u16)0x0U);

	/* Write into control register to disable debug bypass. */
	XEnhance_WriteReg((InstancePtr)->Config.BaseAddress,
		(XENH_CONTROL_OFFSET),
			((XEnhance_ReadReg((InstancePtr)->Config.BaseAddress,
					(XENH_CONTROL_OFFSET))) &
					(~(XENH_CTL_TPE_MASK))));
}
/*****************************************************************************/
/**
*
* This function returns the contents of Version register.
*
* @param	InstancePtr is a pointer to XEnhance instance to be worked on.
*
* @return	Returns the contents of the Version register.
*
* @note		None.
*
******************************************************************************/
u32 XEnhance_GetVersion(XEnhance *InstancePtr)
{
	u32 Data;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Read version register of the Enhance core */
	Data = XEnhance_ReadReg((InstancePtr)->Config.BaseAddress,
					(XENH_VERSION_OFFSET));

	return Data;
}

/*****************************************************************************/
/**
*
* This function gets number of frames processed since power-up or last
* time the core is reset.
*
* @param	InstancePtr is a pointer to XEnhance instance to be worked on.
*
* @return	FrameCount is the number of frames processed since power-up.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
u32 XEnhance_GetDbgFrameCount(XEnhance *InstancePtr)
{
	u32 FrameCount;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.HasDebug != (u16)0x0);

	/* Reads Frame Throughput monitor */
	FrameCount = XEnhance_ReadReg(InstancePtr->Config.BaseAddress,
			(XENH_SYSDEBUG0_OFFSET));

	return FrameCount;
}

/*****************************************************************************/
/**
*
* This function gets the number of lines processed since power-up or last
* time the core is reset.
*
* @param	InstancePtr is a pointer to XEnhance instance to be worked on.
*
* @return	LineCount is the number of lines processed since power-up.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
u32 XEnhance_GetDbgLineCount(XEnhance *InstancePtr)
{
	u32 LineCount;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.HasDebug != (u16)0x0);

	/* Reads Line throughput monitor */
	LineCount = XEnhance_ReadReg(InstancePtr->Config.BaseAddress,
			(XENH_SYSDEBUG1_OFFSET));

	return LineCount;
}

/*****************************************************************************/
/**
*
* This function gets the number of pixels processed since power-up or last
* time the core is reset.
*
* @param	InstancePtr is a pointer to XEnhance instance to be worked on.
*
* @return	PixelCount is the number of pixels processed since power-up.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
u32 XEnhance_GetDbgPixelCount(XEnhance *InstancePtr)
{
	u32 PixelCount;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.HasDebug != (u16)0x0);

	/* Reads Pixel Throughput monitor */
	PixelCount=XEnhance_ReadReg(InstancePtr->Config.BaseAddress,
			(XENH_SYSDEBUG2_OFFSET));

	return PixelCount;
}

/*****************************************************************************/
/**
*
* This function sets active H/V sizes in the Active size register.
*
* @param	InstancePtr is a pointer to XEnhance instance to be worked on.
* @param	HSize is number of Active Pixels per scan line to be set.
*		Range of HSize is 32 to 7680.
* @param	VSize is number of Active Lines per frame to be set.
*		Range of VSize is 32 to 7680.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XEnhance_SetActiveSize(XEnhance *InstancePtr, u16 HSize, u16 VSize)
{
	u32 Size;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((VSize >= (u16)(XENH_VSIZE_FIRST)) &&
					(VSize <= (u16)(XENH_VSIZE_LAST)));
	Xil_AssertVoid((HSize >= (u16)(XENH_HSIZE_FIRST)) &&
					(HSize <= (u16)(XENH_HSIZE_LAST)));

	Size = (((u32)VSize) << ((u32)(XENH_ACTSIZE_NUM_LINE_SHIFT))) | (HSize);
	XEnhance_WriteReg(InstancePtr->Config.BaseAddress,
		(XENH_ACTIVE_SIZE_OFFSET), Size);
}

/*****************************************************************************/
/**
*
* This function gets the number of Active Pixel per Scan line
* and number of Active Lines per Frame from the Active Frame Size register.
*
* @param	InstancePtr is a pointer to XEnhance instance to be worked on.
* @param	HSize is a pointer to 16-bit variable in which
*		the number of Active Pixels per Scan Line is returned.
*		(Range is 32 to 7680).
* @param	VSize is a pointer to 16-bit variable in which
*		the number of Active Lines per Frame is returned.
*		(Range is 32 to 7680).
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XEnhance_GetActiveSize(XEnhance *InstancePtr, u16 *HSize, u16 *VSize)
{
	u32 Data;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(HSize != NULL);
	Xil_AssertVoid(VSize != NULL);

	Data = XEnhance_ReadReg(InstancePtr->Config.BaseAddress,
				XENH_ACTIVE_SIZE_OFFSET);

	/* Reads Number of Active Lines per Frame */
	*HSize = (u16)((Data) & (XENH_ACTSIZE_NUM_PIXEL_MASK));

	/* Reads Number of Active Lines per Frame */
	*VSize = (u16)((Data) & (XENH_ACTSIZE_NUM_LINE_MASK)) >>
					(XENH_ACTSIZE_NUM_LINE_SHIFT);
}

/*****************************************************************************/
/**
*
* This function sets the Noise Threshold value for the Enhance core
* The amount of noise reduction can be controlled through Noise
* Threshold parameter.
*
* @param	InstancePtr is a pointer to XEnhance instance to be worked on.
* @param	Threshold is the value to set the Noise Threshold.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XEnhance_SetNoiseThreshold(XEnhance *InstancePtr, u32 Threshold)
{
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Threshold <= (u32)XENH_NOISETHRES_MAX);

	XEnhance_WriteReg((InstancePtr)->Config.BaseAddress,
			(XENH_NOISE_THRESHOLD_OFFSET), Threshold);

}

/*****************************************************************************/
/**
*
* This function gets the Noise Threshold value for the Enhance core.
*
* @param	InstancePtr is a pointer to XEnhance instance to be worked on.
*
* @return	The amount of noise reduction that can be controlled.
*
* @note		None.
*
******************************************************************************/
u32 XEnhance_GetNoiseThreshold(XEnhance *InstancePtr)
{
	u32 NoiseThreshold;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	NoiseThreshold = (XEnhance_ReadReg(InstancePtr-> Config.BaseAddress,
				(XENH_NOISE_THRESHOLD_OFFSET))) &
					(XENH_NOISE_THRESHOLD_MASK);

	return NoiseThreshold;
}

/*****************************************************************************/
/**
*
* This function sets the Edge Strength value for the Enhance core.
* i.e. The amount of edge enhancement can be controlled through the
* programmable Enhance Strength parameter.
*
* @param	InstancePtr is a pointer to XEnhance instance to be worked on.
* @param	Strength is the value to set the Edge Strength of the core.
*
* @return	None.
*
* @note		The larger the strength, the stronger the edge enhancement.
*
******************************************************************************/
void XEnhance_SetEdgeStrength(XEnhance *InstancePtr, u32 Strength)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Strength <= (u32)XENH_ENHSTRENGTH_MAX);

	XEnhance_WriteReg((InstancePtr)->Config.BaseAddress,
		(XENH_ENHANCE_STRENGTH_OFFSET), Strength);
}

/*****************************************************************************/
/**
*
* This function gets the Edge Strength value for the Enhance core.
*
* @param	InstancePtr is a pointer to XEnhance instance to be worked on.
*
* @return	The amount of edge enhancement that can be controlled.
*
* @note		None.
*
******************************************************************************/
u32 XEnhance_GetEdgeStrength(XEnhance *InstancePtr)
{
	u32 EdgeStrength;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	EdgeStrength = (XEnhance_ReadReg(InstancePtr->Config.BaseAddress,
				(XENH_ENHANCE_STRENGTH_OFFSET))) &
					(XENH_STRENGTH_MASK);

	return EdgeStrength;
}

/*****************************************************************************/
/**
*
* This function sets the Halo Suppress value for the Enhance core.
* i.e. The amount of halo suppression can be controlled through the
* programmable Halo Suppress parameter.
*
* @param	InstancePtr is a pointer to XEnhance instance to be worked on.
* @param	Suppress is the value to set the Suppression value.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XEnhance_SetHaloSuppress(XEnhance *InstancePtr, u32 Suppress)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Suppress <= (u32)XENH_HALOSUPPRESS_MAX);

	XEnhance_WriteReg((InstancePtr)->Config.BaseAddress,
			(XENH_HALO_SUPPRESS_OFFSET), Suppress);
}

/*****************************************************************************/
/**
*
* This function gets the Halo Suppress value for the Enhance core.
*
* @param	InstancePtr is a pointer to XEnhance instance to be worked on.
*
* @return	The amount of halo suppression that can be controlled.
*
* @note		None.
*
******************************************************************************/
u32 XEnhance_GetHaloSuppress(XEnhance *InstancePtr)
{
	u32 HaloSuppress;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	HaloSuppress = (XEnhance_ReadReg(InstancePtr->Config.BaseAddress,
				(XENH_HALO_SUPPRESS_OFFSET))) &
					(XENH_HALO_SUPPRESS_MASK);

	return HaloSuppress;
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
*****************************************************************************/
static void StubCallBack(void *CallBackRef)
{
	/* Verify arguments. */
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
*		value equals 'OR'ing one or more XENH_IXR_*_MASK values defined
*		in xenhance_hw.h.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
static void StubErrCallBack(void *CallBackRef, u32 ErrorMask)
{
	/* Verify arguments. */
	Xil_AssertVoid((ErrorMask == ((u32)0x0U)) ||
				(ErrorMask > ((u32)0x0U)));
	Xil_AssertVoid(CallBackRef != NULL);
	Xil_AssertVoidAlways();
}
/** @} */
