/******************************************************************************
*
* (c) Copyright 2001-14 Xilinx, Inc. All rights reserved.
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
* @file xcfa.c
* @addtogroup cfa_v7_1
* @{
*
* This file contains the implementation of the interface functions for CFA
* core. Refer to the header file xcfa.h for more detailed information.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  drg/jz 01/13/10 First Release
* 3.00a gz     10/22/10 Updated for CFA V3.0
* 4.00a rc     09/11/11 Updated for CFA v4.0
* 5.00a se     12/01/11 Updated for CFA v5.0
* 7.0   adk    01/07/14 Changed the file name from
*                       cfa.c to xcfa.c
*                       Implemented the following functions:
*                       XCfa_CfgInitialize, XCfa_Setup,
*                       XCfa_GetVersion,
*                       XCfa_EnableDbgByPass,
*                       XCfa_IsDbgByPassEnabled,
*                       XCfa_DisableDbgBypass,
*                       XCfa_EnableDbgTestPattern,
*                       XCfa_IsDbgTestPatternEnabled,
*                       XCfa_DisableDbgTestPattern,
*                       XCfa_GetDbgFrameCount
*                       XCfa_GetDbgLineCount, XCfa_GetDbgPixelCount
*                       XCfa_SetActiveSize, XCfa_GetActiveSize
*                       XCfa_SetBayerPhase, XCfa_GetBayerPhase,
*                       StubCallBack, StubErrCallBack.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcfa.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

static void StubCallBack (void *CallBackRef);
static void StubErrCallBack (void *CallBackRef, u32 ErrorMask);

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function initializes the CFA core. This function must be called
* prior to using the CFA core. Initialization of the CFA includes setting up
* the instance data and ensuring the hardware is in a quiescent state.
*
* @param	InstancePtr is a pointer to the XCfa instance to be worked on.
* @param	CfgPtr is a reference to a configuration structure
*		containing information about the CFA core.
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
int XCfa_CfgInitialize(XCfa *InstancePtr, XCfa_Config *CfgPtr,
			u32 EffectiveAddr)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddr != ((u32)0x0));

	/* Setup the instance. */
	(void)memset((void *)InstancePtr, 0, sizeof(XCfa));
	(void)memcpy((void *)&(InstancePtr->Config), (const void *)CfgPtr,
			sizeof(XCfa_Config));
	InstancePtr->Config.BaseAddress = EffectiveAddr;

	/*
	 * Set all handlers to stub values, let user configure
	 * this data later.
	 */
	InstancePtr->ProcStartCallBack =
				(XCfa_CallBack)((void *)StubCallBack);
	InstancePtr->FrameDoneCallBack =
				(XCfa_CallBack)((void *)StubCallBack);
	InstancePtr->ErrCallBack =
			(XCfa_ErrorCallBack)((void *)StubErrCallBack);

	/*
	 * Reset the hardware and set the flag to indicate the core is
	 * ready.
	 */
	XCfa_Reset(InstancePtr);
	InstancePtr->IsReady = (u32)(XIL_COMPONENT_IS_READY);

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function sets the input/output frame size in Active Size register and
* enables the register update.
*
* @param	InstancePtr is a pointer to the XCfa instance to be worked on.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XCfa_Setup(XCfa *InstancePtr)
{
	u32 Data;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	XCfa_RegUpdateDisable(InstancePtr);

	/* Write into active size register */
	Data = (((InstancePtr->VSize) << (XCFA_ACTSIZE_NUM_LINE_SHIFT)) &
		(XCFA_ACTSIZE_NUM_LINE_MASK)) | ((InstancePtr->HSize) &
				(XCFA_ACTSIZE_NUM_PIXEL_MASK));
	XCfa_WriteReg(InstancePtr->Config.BaseAddress,
				(XCFA_ACTIVE_SIZE_OFFSET), Data);
	XCfa_RegUpdateEnable(InstancePtr);
}

/*****************************************************************************/
/**
*
* This sets the bypass bit of the control register to switch the core to bypass
* mode if debug is enabled in the CFA core.
*
* @param	InstancePtr is a pointer to the XCfa instance to be worked on.
*
* @return	None.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
void XCfa_EnableDbgByPass(XCfa *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->Config.HasDebug != (u16)0x0);

	/* Write into control register to set debug bypass. */
	XCfa_WriteReg(InstancePtr->Config.BaseAddress, (XCFA_CONTROL_OFFSET),
			((XCfa_ReadReg(InstancePtr->Config.BaseAddress,
				(XCFA_CONTROL_OFFSET))) |
					(XCFA_CTL_BPE_MASK)));
}

/*****************************************************************************/
/**
*
* This function gets the current status of the bypass setting of the CFA core.
*
* @param	InstancePtr is a pointer to the XCfa instance to be worked on.
*
* @return	Core debug bypass mode.
*		- TRUE = Bypass mode is enabled.
*		- FALSE = Bypass mode is not enabled.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
int XCfa_IsDbgByPassEnabled(XCfa *InstancePtr)
{
	u32 DbgByPass;
	int Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.HasDebug != (u16)0x0);

	/* Read from control register to know debug bypass status. */
	DbgByPass = (XCfa_ReadReg(InstancePtr->Config.BaseAddress,
				(XCFA_CONTROL_OFFSET))) & (XCFA_CTL_BPE_MASK);
	if (DbgByPass == (XCFA_CTL_BPE_MASK)) {
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
* This function disables bypass mode of the CFA core.
*
* @param	InstancePtr is a pointer to the XCfa instance to be worked on.
*
* @return	None.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
void XCfa_DisableDbgBypass(XCfa *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->Config.HasDebug != (u16)0x0);

	/* Write into control register to disable debug bypass. */
	XCfa_WriteReg(InstancePtr->Config.BaseAddress,
				(XCFA_CONTROL_OFFSET),
			((XCfa_ReadReg(InstancePtr->Config.BaseAddress,
			(XCFA_CONTROL_OFFSET))) & (~(XCFA_CTL_BPE_MASK))));
}

/*****************************************************************************/
/**
*
* This function sets the test-pattern mode of the CFA core if debug
* features is enabled.
*
* @param 	InstancePtr is a pointer to the XCfa instance to be worked on.
*
* @return	None.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
void XCfa_EnableDbgTestPattern(XCfa *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->Config.HasDebug != (u16)0x0);

	/* Write into control register to set test pattern. */
	XCfa_WriteReg(InstancePtr->Config.BaseAddress, (XCFA_CONTROL_OFFSET),
			((XCfa_ReadReg(InstancePtr->Config.BaseAddress,
			(XCFA_CONTROL_OFFSET))) | (XCFA_CTL_TPE_MASK)));
}

/*****************************************************************************/
/**
*
* This function gets the test-pattern mode if debug feature is enabled.
*
* @param	InstancePtr is a pointer to the XCfa instance to be worked on.
*
* @return	Test-pattern generator mode.
*		- TRUE = Test-pattern mode is enabled.
*		- FALSE = Test-pattern mode is not enabled.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
int XCfa_IsDbgTestPatternEnabled(XCfa *InstancePtr)
{
	u32 DbgTestPattern;
	int Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.HasDebug != (u16)0x0);

	/* Read from control register to know debug test pattern status. */
	DbgTestPattern = (XCfa_ReadReg(InstancePtr->Config.BaseAddress,
			(XCFA_CONTROL_OFFSET))) & (XCFA_CTL_TPE_MASK);
	if (DbgTestPattern == (XCFA_CTL_TPE_MASK)) {
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
* This function disables the test pattern mode of the CFA core.
*
* @param	InstancePtr is a pointer to the XCfa instance to be worked on.
*
* @return 	None.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
void XCfa_DisableDbgTestPattern(XCfa *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->Config.HasDebug != (u16)0x0);

	/* Write into control register to disable debug bypass. */
	XCfa_WriteReg(InstancePtr->Config.BaseAddress,
					(XCFA_CONTROL_OFFSET),
		((XCfa_ReadReg(InstancePtr->Config.BaseAddress,
			(XCFA_CONTROL_OFFSET))) & (~(XCFA_CTL_TPE_MASK))));
}

/*****************************************************************************/
/**
*
* This function gets the Version of the CFA core.
*
* @param	InstancePtr is a pointer to the XCfa instance to be worked on.
*
* @return	Returns the contents of the Version register.
*
* @note		None.
*
******************************************************************************/
u32 XCfa_GetVersion(XCfa *InstancePtr)

{
	u32 Data;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Read version register of the CFA core */
	Data = XCfa_ReadReg(InstancePtr->Config.BaseAddress,
				(XCFA_VERSION_OFFSET));

	return Data;
}

/*****************************************************************************/
/**
*
* This function gets number of frames processed since power-up or last
* time the core is reset.
*
* @param	InstancePtr is a pointer to the XCfa instance to be worked on.
*
* @return	FrameCount is the number of frames processed since power up.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
u32 XCfa_GetDbgFrameCount(XCfa *InstancePtr)
{
	u32 FrameCount;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.HasDebug != (u16)0x0);

	/* Reads Frame throughput monitor */
	FrameCount = XCfa_ReadReg(InstancePtr->Config.BaseAddress,
						(XCFA_SYSDEBUG0_OFFSET));

	return FrameCount;
}

/*****************************************************************************/
/**
*
* This function gets the number of lines processed since power-up or last
* time the core is reset.
*
* @param	InstancePtr is a pointer to the XCfa instance to be worked on.
*
* @return	LineCount is the number of lines processed since power up.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
u32 XCfa_GetDbgLineCount(XCfa *InstancePtr)
{
	u32 LineCount;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.HasDebug != (u16)0x0);

	/* Line throughput monitor */
	LineCount = XCfa_ReadReg(InstancePtr->Config.BaseAddress,
					(XCFA_SYSDEBUG1_OFFSET));

	return LineCount;
}

/*****************************************************************************/
/**
*
* This function gets the number of pixels processed since power-up or last
* time the core is reset.
*
* @param	InstancePtr is a pointer to the XCfa instance to be worked on.
*
* @return	PixelCount is the number of pixels processed since power up.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
u32 XCfa_GetDbgPixelCount(XCfa *InstancePtr)
{
	u32 PixelCount;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.HasDebug != (u16)0x0);

	/* Pixel throughput monitor */
	PixelCount = XCfa_ReadReg(InstancePtr->Config.BaseAddress,
					(XCFA_SYSDEBUG2_OFFSET));

	return PixelCount;
}

/*****************************************************************************/
/**
*
*  This function sets the active H/V sizes in the Active Size register.
*
* @param	InstancePtr is a pointer to the XCfa instance to be worked on.
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
void XCfa_SetActiveSize(XCfa *InstancePtr, u16 HSize, u16 VSize)
{
	u32 Size;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((VSize >= (u16)(XCFA_VSIZE_FIRST)) &&
			(VSize <= (u16)(XCFA_VSIZE_LAST)));
	Xil_AssertVoid((HSize >= (u16)(XCFA_HSIZE_FIRST)) &&
				(HSize <= (u16)(XCFA_HSIZE_LAST)));

	Size = (((u32)VSize) << ((u32)(XCFA_ACTSIZE_NUM_LINE_SHIFT))) |
					(HSize);
	XCfa_WriteReg(InstancePtr->Config.BaseAddress,
				(XCFA_ACTIVE_SIZE_OFFSET), Size);
}

/*****************************************************************************/
/**
*
* This function gets the number of Active Pixel per Scan line
* and number of Active Lines per Frame from the Active Frame Size register.
*
* @param	InstancePtr is a pointer to the XCfa instance to be worked on.
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
******************************************************************************/
void XCfa_GetActiveSize(XCfa *InstancePtr, u16 *HSize, u16 *VSize)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(HSize != NULL);
	Xil_AssertVoid(VSize != NULL);

	/* Number of active lines per frame */
	*HSize = (u16)(XCfa_ReadReg(InstancePtr->Config.BaseAddress,
		(XCFA_ACTIVE_SIZE_OFFSET)) & (XCFA_ACTSIZE_NUM_PIXEL_MASK));

	/* Number of active pixels per scan line */
	*VSize = (u16)(XCfa_ReadReg(InstancePtr->Config.BaseAddress,
		(XCFA_ACTIVE_SIZE_OFFSET)) & (XCFA_ACTSIZE_NUM_LINE_MASK)) >>
						(XCFA_ACTSIZE_NUM_LINE_SHIFT);
}

/*****************************************************************************/
/**
*
* This function specify whether the starting position pixel(0,0) of the
* Bayer sampling grid is on a red-green or blue-green line and whether the
* first pixel is green or not.
*
* @param	InstancePtr is a pointer to the XCfa instance to be worked on.
* @param	BayerPhase to be set to identify starting position.
*		Range of Phase is 0 to 3.
*		The Phase value combinations are
*		- Bayer Phase 0 is XCFA_RGRG_COMBINATION.
*		- Bayer Phase 1 is XCFA_GRGR_COMBINATION.
*		- Bayer Phase 2 is XCFA_GBGB_COMBINATION.
*		- Bayer Phase 3 is XCFA_BGBG_COMBINATION.
*		- It is a double buffered register.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XCfa_SetBayerPhase(XCfa *InstancePtr,
				enum XCfa_BayerPhaseCombination BayerPhase)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(BayerPhase <= XCFA_BGBG_COMBINATION);

	XCfa_WriteReg(InstancePtr->Config.BaseAddress,
				(XCFA_BAYER_PHASE_OFFSET),
			((u32)BayerPhase & XCFA_BAYER_PHASE_MASK));
}

/*****************************************************************************/
/**
*
* This function gets the Bayer phase value combination from Bayer phase
* register.
*
* @param	InstancePtr is a pointer to the XCfa instance to be worked on.
*
* @return	BayerPhase the Bayer Phase value 0 to 3.
*		The Phase value combinations are
*		- Bayer Phase 0 is XCFA_RGRG_COMBINATION.
*		- Bayer Phase 1 is XCFA_GRGR_COMBINATION.
*		- Bayer Phase 2 is XCFA_GBGB_COMBINATION.
*		- Bayer Phase 3 is XCFA_BGBG_COMBINATION.
*
* @note		- It is a double buffered register.
*
******************************************************************************/
u32 XCfa_GetBayerPhase(XCfa *InstancePtr)
{
	u32 BayerPhase;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	BayerPhase = (XCfa_ReadReg(InstancePtr->Config.BaseAddress,
			(XCFA_BAYER_PHASE_OFFSET))) & (XCFA_8_BIT_MASK);

	return BayerPhase;
}

/*****************************************************************************/
/**
*
* This routine is a stub for the asynchronous callbacks. The stub is here in
* case the upper layer forgot to set the handlers. On initialization, All
* handlers except error handler are set to this callback. It is considered as
* an error for this handler to be invoked.
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
* considered as an error for this handler to be invoked.
*
* @param	CallBackRef is a callback reference passed in by the upper
*		layer when setting the callback functions, and passed back
*		to the upper layer when the callback is invoked.
* @param	ErrorMask is a bit mask indicating the cause of the error. Its
* 		value equals 'OR'ing one or more XCFA_IXR_*_MASK values defined
* 		in xcfa_hw.h.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void StubErrCallBack(void *CallBackRef, u32 ErrorMask)
{
	/* Verify arguments. */
	Xil_AssertVoid(CallBackRef != NULL);
	Xil_AssertVoid(ErrorMask != ((u32)0x0));
	Xil_AssertVoidAlways();
}
/** @} */
