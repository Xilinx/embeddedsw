/******************************************************************************
*
* (c) Copyright 2011 - 2014 Xilinx, Inc. All rights reserved.
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
* @file xccm.c
* @addtogroup ccm_v6_1
* @{
*
* This file contains the implementation of the interface functions for CCM
* driver. Refer to the header file xccm.h for more detailed information.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------- -------- ------------------------------------------------------
* 2.00a jo      05/1/10  Updated for CCM V2.0
* 3.00a ren     09/11/11 Updated for CCM v3.0
* 4.00a jj      12/18/12 Converted from xio.h to xil_io.h,translating
*                        basic types,MB cache functions, exceptions
*                        and assertions to xil_io format.
* 6.0   adk     03/06/14 Changed filename ccm.c to xccm.c.
*                        Implemented the following functions:
*                        XCcm_CfgInitialize, XCcm_Setup, XCcm_GetVersion,
*                        XCcm_EnableDbgByPass, XCcm_IsDbgByPassEnabled,
*                        XCcm_DisableDbgByPass, XCcm_EnableDbgTestPattern,
*                        XCcm_IsDbgTestPatternEnabled,
*                        XCcm_DisableDbgTestPattern, XCcm_GetDbgFrameCount,
*                        XCcm_GetDbgLineCount, XCcm_GetDbgPixelCount,
*                        XCcm_SetActiveSize, XCcm_GetActiveSize,
*                        XCcm_SetCoefMatrix, XCcm_GetCoefMatrix,
*                        XCcm_SetRgbOffset, XCcm_GetRgbOffset,
*                        XCcm_SetClip, XCcm_GetClip,
*                        XCcm_SetClamp and XCcm_GetClamp XCcm_FloatToFixedConv,
*                        and XCcm_FixedToFloatConv.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xccm.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

static void StubCallBack(void *CallBackRef);
static void StubErrCallBack(void *CallBackRef, u32 ErrorMask);
static float XCcm_FixedToFloatConv(u32 Input);
static s32 XCcm_FloatToFixedConv(float Input);

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function initializes an CCM core. This function must be called
* prior to using an CCM core. Initialization of an CCM includes setting up
* the instance data and ensuring the hardware is in a quiescent state.
*
* @param	InstancePtr is a pointer to the XCcm instance.
* @param	CfgPtr is a reference to a structure containing information
*		about a specific XCcm instance.
* @param	EffectiveAddr is the device base address in the virtual memory
*		address space. The caller is responsible for keeping the
*		address mapping from EffectiveAddr to the device physical
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
int XCcm_CfgInitialize(XCcm *InstancePtr, XCcm_Config *CfgPtr,
			u32 EffectiveAddr)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddr != ((u32)0x0));

	/* Setup the instance */
	(void)memset((void *)InstancePtr, 0, sizeof(XCcm));
	(void)memcpy((void *)&(InstancePtr->Config), (const void *)CfgPtr,
							sizeof(XCcm_Config));
	InstancePtr->Config.BaseAddress = EffectiveAddr;

	/*
	 * Set all handlers to stub values, let user configure this data
	 * later
	 */
	InstancePtr->ProcStartCallBack =
				(XCcm_CallBack)((void *)StubCallBack);
	InstancePtr->FrameDoneCallBack =
				(XCcm_CallBack)((void *)StubCallBack);
	InstancePtr->ErrCallBack =
				(XCcm_ErrorCallBack)((void *)StubErrCallBack);

	/*
	 * Reset the hardware and set the flag to indicate the driver is
	 * ready
	 */
	XCcm_Reset(InstancePtr);
	InstancePtr->IsReady = (u32)(XIL_COMPONENT_IS_READY);

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function sets the input/output frame size in Active Size register and
* enables the register update.
*
* @param	InstancePtr is a pointer to the Xccm instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XCcm_Setup(XCcm *InstancePtr)
{
	u32 Data;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	XCcm_RegUpdateDisable(InstancePtr);

	/* Write into active size register */
	Data = ((((InstancePtr->VSize)) <<
			(u32)(XCCM_ACTSIZE_NUM_LINE_SHIFT)) &
				(XCCM_ACTSIZE_NUM_LINE_MASK)) |
		 (((InstancePtr->HSize)) & (u32)(XCCM_ACTSIZE_NUM_PIXEL_MASK));
	XCcm_WriteReg(InstancePtr->Config.BaseAddress,
					(XCCM_ACTIVE_SIZE_OFFSET), Data);
	XCcm_RegUpdateEnable(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function enables the bypass mode by setting bypass bit of the Control
* register to switch the core to bypass mode if debug feature is enabled in
* the core.
*
* @param	InstancePtr is a pointer to the XCcm instance to be worked on.
*
* @return	None.
*
* @note 	Debug functionality should be enabled.
*
******************************************************************************/
void XCcm_EnableDbgByPass(XCcm *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->Config.HasDebug != (u16)0x0);

	XCcm_WriteReg(InstancePtr->Config.BaseAddress,
		(XCCM_CONTROL_OFFSET),
			((XCcm_ReadReg(InstancePtr->Config.BaseAddress,
				(XCCM_CONTROL_OFFSET))) |
					(XCCM_CTL_BPE_MASK)));
}

/*****************************************************************************/
/**
*
* This function returns the current bypass mode settings from Control register
* of the CCM core.
*
* @param	InstancePtr is a pointer to the XCcm instance.
*
* @return	Core debug bypass mode.
*		- TRUE = Bypass mode is enabled.
*		- FALSE = Bypass mode is not enabled.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
int XCcm_IsDbgByPassEnabled(XCcm *InstancePtr)
{
	u32 DbgByPass;
	int Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.HasDebug != (u16)0x0);

	DbgByPass = (XCcm_ReadReg(InstancePtr->Config.BaseAddress,
				(XCCM_CONTROL_OFFSET))) & (XCCM_CTL_BPE_MASK);
	if (DbgByPass == (XCCM_CTL_BPE_MASK)) {
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
* This function disables the Bypass mode of the CCM core.
*
* @param	InstancePtr is a pointer to the XCcm instance to be worked on.
*
* @return	None.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
void XCcm_DisableDbgByPass(XCcm *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->Config.HasDebug != (u16)0x0);

	XCcm_WriteReg(InstancePtr->Config.BaseAddress,
		(XCCM_CONTROL_OFFSET),
		((XCcm_ReadReg(InstancePtr->Config.BaseAddress,
			(XCCM_CONTROL_OFFSET))) & (~(XCCM_CTL_BPE_MASK))));
}

/*****************************************************************************/
/**
*
* This function enables the test-pattern mode if debug feature is enabled
* by setting test-pattern bit of the Control register of the CCM core.
*
* @param	InstancePtr is a pointer to the XCcm instance.
*
* @return	None.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
void XCcm_EnableDbgTestPattern(XCcm *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->Config.HasDebug != (u16)0x0);

	XCcm_WriteReg(InstancePtr->Config.BaseAddress,
		(XCCM_CONTROL_OFFSET),
			((XCcm_ReadReg(InstancePtr->Config.BaseAddress,
			(XCCM_CONTROL_OFFSET))) | (XCCM_CTL_TPE_MASK)));
}

/*****************************************************************************/
/**
*
* This function returns the test-pattern mode (enabled or not) from Control
* register of the CCM core, if debug feature was enabled.
*
* @param	InstancePtr is a pointer to the XCcm instance.
*
* @return	Test-pattern generator mode.
*		- TRUE = Test-pattern mode is enabled.
*		- FALSE = Test-pattern mode is not enabled.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
int XCcm_IsDbgTestPatternEnabled(XCcm *InstancePtr)
{
	u32 DbgTestPattern;
	int Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.HasDebug != (u16)0x0);

	DbgTestPattern = (XCcm_ReadReg(InstancePtr->Config.BaseAddress,
				(XCCM_CONTROL_OFFSET))) & (XCCM_CTL_TPE_MASK);
	if (DbgTestPattern == (XCCM_CTL_TPE_MASK)) {
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
* This function disables debug test pattern mode in Control register of the
* CCM core, if Debug feature is enabled.
*
* @param	InstancePtr is a pointer to the XCcm core instance to be
*		worked on.
*
* @return	None.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
void XCcm_DisableDbgTestPattern(XCcm *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->Config.HasDebug != (u16)0x0);

	XCcm_WriteReg(InstancePtr->Config.BaseAddress,
		(XCCM_CONTROL_OFFSET),
		((XCcm_ReadReg(InstancePtr->Config.BaseAddress,
			(XCCM_CONTROL_OFFSET))) & (~(XCCM_CTL_TPE_MASK))));
}

/*****************************************************************************/
/**
*
* This function gets the Version of the CCM core.
*
* @param	InstancePtr is a pointer to the XCcm instance to be worked on.
*
* @return	Returns the contents of the Version register.
*
* @note		None.
*
******************************************************************************/
u32 XCcm_GetVersion(XCcm *InstancePtr)

{
	u32 Data;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Read version register of the CCM core. */
	Data = XCcm_ReadReg(InstancePtr->Config.BaseAddress,
						(XCCM_VERSION_OFFSET));

	return Data;
}

/*****************************************************************************/
/**
*
* This function gets number of frames processed since power-up or last
* time the core is reset.
*
* @param	InstancePtr is a pointer to the XCcm instance.
*
* @return	FrameCount is the number of frames processed since power up.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
u32 XCcm_GetDbgFrameCount(XCcm *InstancePtr)
{
	u32 FrameCount;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.HasDebug != (u16)0x0);

	/* Frame throughput monitor */
	FrameCount = XCcm_ReadReg(InstancePtr->Config.BaseAddress,
					(XCCM_SYSDEBUG0_OFFSET));

	return FrameCount;
}

/*****************************************************************************/
/**
*
* This function gets the number of lines processed since power-up or last
* time the core is reset.
*
* @param	InstancePtr is a pointer to the XCcm instance.
*
* @return	LineCount is the number of lines processed since power up.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
u32 XCcm_GetDbgLineCount(XCcm *InstancePtr)
{
	u32 LineCount;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.HasDebug != (u16)0x0);

	/* Line throughput monitor */
	LineCount = XCcm_ReadReg(InstancePtr->Config.BaseAddress,
					(XCCM_SYSDEBUG1_OFFSET));

	return LineCount;
}

/*****************************************************************************/
/**
*
* This function gets the number of pixels processed since power-up or last
* time the core is reset.
*
* @param	InstancePtr is a pointer to the XCcm instance.
*
* @return	PixelCount is the number of pixels processed since power up.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
u32 XCcm_GetDbgPixelCount(XCcm *InstancePtr)
{
	u32 PixelCount;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.HasDebug != (u16)0x0);

	/* Pixel throughput monitor */
	PixelCount = XCcm_ReadReg(InstancePtr->Config.BaseAddress,
					(XCCM_SYSDEBUG2_OFFSET));

	return PixelCount;
}

/*****************************************************************************/
/**
*
* This function sets the active H/V sizes in the Active Size register.
*
* @param	InstancePtr is a pointer to the XCcm instance.
* @param	HSize specifies the number of Active Pixels per Scan Line
*		that needs to be set (Range is 32 to 7680).
* @param	VSize specifies the number of Active Lines per Frame that
*		needs to be set (Range is 32 to 7680).
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XCcm_SetActiveSize(XCcm *InstancePtr, u16 HSize, u16 VSize)
{
	u32 Size;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((VSize >= (u16)(XCCM_ACT_SIZE_FIRST)) &&
			(VSize <= (u16)(XCCM_ACT_SIZE_LAST)));
	Xil_AssertVoid((HSize >= (u16)(XCCM_ACT_SIZE_FIRST)) &&
			(HSize <= (u16)(XCCM_ACT_SIZE_LAST)));

	Size = (((u32)VSize) << ((u32)(XCCM_ACTSIZE_NUM_LINE_SHIFT))) |
								(HSize);
	XCcm_WriteReg(InstancePtr->Config.BaseAddress,
				(XCCM_ACTIVE_SIZE_OFFSET),
							Size);
}

/*****************************************************************************/
/**
*
* This function gets the number of Active Pixels per Scan line and number of
* Active Lines per Frame from the Active Frame Size register.
*
* @param	InstancePtr is a pointer to the XCcm instance.
* @param	HSize is a pointer to 16-bit variable in which the number of
*		Active Pixels per Scan Line is returned (Range is 32 to 7680).
* @param	VSize is a pointer to 16-bit variable in which the number of
*		Active Lines per Frame is returned (Range is 32 to 7680).

* @return	None.
*
* @note		None.
*
******************************************************************************/
void XCcm_GetActiveSize(XCcm *InstancePtr, u16 *HSize, u16 *VSize)
{
	u32 Data;
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(HSize != NULL);
	Xil_AssertVoid(VSize != NULL);

	Data = XCcm_ReadReg(InstancePtr->Config.BaseAddress,
			(XCCM_ACTIVE_SIZE_OFFSET));
	/*
	 * Number of active pixels per scan line.
	 */
	*HSize = (u16)((Data) & (XCCM_ACTSIZE_NUM_PIXEL_MASK));

	/*
	 * Number of active lines per frame.
	 */
	*VSize = (u16)(((Data) & (XCCM_ACTSIZE_NUM_LINE_MASK)) >>
				(XCCM_ACTSIZE_NUM_LINE_SHIFT));
}

/*****************************************************************************/
/**
*
* This function sets the coefficients of color correction matrix in
* K11 to K33 registers of the CCM core.
*
* @param	InstancePtr is a pointer to the CCM instance.
* @param	CoefValues is a pointer to XCcm_Coefs structure which has
*		matrix coefficients that needs to be set.
*		(Range is floating point numbers [-8, 8)).
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XCcm_SetCoefMatrix(XCcm *InstancePtr, XCcm_Coefs *CoefValues)
{
	s32 Value;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CoefValues != NULL);
	Xil_AssertVoid(((CoefValues->K11) >= ((XCCM_COEF_FIRST))) &&
			((CoefValues->K11) <= (XCCM_COEF_LAST)));
	Xil_AssertVoid(((CoefValues->K12) >= ((XCCM_COEF_FIRST))) &&
			((CoefValues->K12) <= (XCCM_COEF_LAST)));
	Xil_AssertVoid(((CoefValues->K13) >= ((XCCM_COEF_FIRST))) &&
			((CoefValues->K13) <= (XCCM_COEF_LAST)));
	Xil_AssertVoid(((CoefValues->K21) >= ((XCCM_COEF_FIRST))) &&
			((CoefValues->K21) <= (XCCM_COEF_LAST)));
	Xil_AssertVoid(((CoefValues->K22) >= ((XCCM_COEF_FIRST))) &&
			((CoefValues->K22) <= (XCCM_COEF_LAST)));
	Xil_AssertVoid(((CoefValues->K23) >= ((XCCM_COEF_FIRST))) &&
			((CoefValues->K23) <= (XCCM_COEF_LAST)));
	Xil_AssertVoid(((CoefValues->K31) >= ((XCCM_COEF_FIRST))) &&
			((CoefValues->K31) <= (XCCM_COEF_LAST)));
	Xil_AssertVoid(((CoefValues->K32) >= ((XCCM_COEF_FIRST))) &&
			((CoefValues->K32) <= (XCCM_COEF_LAST)));
	Xil_AssertVoid(((CoefValues->K33) >= ((XCCM_COEF_FIRST))) &&
			((CoefValues->K33) <= (XCCM_COEF_LAST)));


	/* Setting K11 register */
	Value = XCcm_FloatToFixedConv(CoefValues->K11);
	XCcm_WriteReg(InstancePtr->Config.BaseAddress, (XCCM_K11_OFFSET),
								Value);

	/* Setting K12 register */
	Value = XCcm_FloatToFixedConv(CoefValues->K12);
	XCcm_WriteReg(InstancePtr->Config.BaseAddress, (XCCM_K12_OFFSET),
								Value);

	/* Setting K13 register */
	Value = XCcm_FloatToFixedConv(CoefValues->K13);
	XCcm_WriteReg(InstancePtr->Config.BaseAddress, (XCCM_K13_OFFSET),
								Value);

	/* Setting K21 register */
	Value = XCcm_FloatToFixedConv(CoefValues->K21);
	XCcm_WriteReg(InstancePtr->Config.BaseAddress, (XCCM_K21_OFFSET),
								Value);

	/* Setting K22 register */
	Value = XCcm_FloatToFixedConv(CoefValues->K22);
	XCcm_WriteReg(InstancePtr->Config.BaseAddress, (XCCM_K22_OFFSET),
								Value);

	/* Setting K23 register */
	Value = XCcm_FloatToFixedConv(CoefValues->K23);
	XCcm_WriteReg(InstancePtr->Config.BaseAddress, (XCCM_K23_OFFSET),
								Value);

	/* Setting K31 register */
	Value = XCcm_FloatToFixedConv(CoefValues->K31);
	XCcm_WriteReg(InstancePtr->Config.BaseAddress, (XCCM_K31_OFFSET),
								Value);

	/* Setting K32 register */
	Value = XCcm_FloatToFixedConv(CoefValues->K32);
	XCcm_WriteReg(InstancePtr->Config.BaseAddress, (XCCM_K32_OFFSET),
								Value);

	/* Setting K33 register */
	Value = XCcm_FloatToFixedConv(CoefValues->K33);
	XCcm_WriteReg(InstancePtr->Config.BaseAddress, (XCCM_K33_OFFSET),
								Value);
}

/*****************************************************************************/
/**
*
* This function gets the coefficient values of color correction matrix from
* K11 to K33 registers of the CCM core.
*
* @param	InstancePtr is a pointer to the CCM instance
* @param	CoefValues is a pointer to XCcm_Coefs structure which has
*		matrix coefficients is updated with coefficient values.
*		(Range is floating point numbers [-8, 8)).
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XCcm_GetCoefMatrix(XCcm *InstancePtr, XCcm_Coefs *CoefValues)
{
	u32 Data;
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CoefValues != NULL);

	Data = (XCcm_ReadReg(InstancePtr->Config.BaseAddress,
					(XCCM_K11_OFFSET)));
	CoefValues->K11 = XCcm_FixedToFloatConv(Data);

	Data = (XCcm_ReadReg(InstancePtr->Config.BaseAddress,
			(XCCM_K12_OFFSET)));
	CoefValues->K12 = XCcm_FixedToFloatConv(Data);

	Data = (XCcm_ReadReg(InstancePtr->Config.BaseAddress,
			(XCCM_K13_OFFSET)));
	CoefValues->K13 = XCcm_FixedToFloatConv(Data);

	Data = (XCcm_ReadReg(InstancePtr->Config.BaseAddress,
			(XCCM_K21_OFFSET)));
	CoefValues->K21 = XCcm_FixedToFloatConv(Data);

	Data = (XCcm_ReadReg(InstancePtr->Config.BaseAddress,
			(XCCM_K22_OFFSET)));
	CoefValues->K22 = XCcm_FixedToFloatConv(Data);

	Data = (XCcm_ReadReg(InstancePtr->Config.BaseAddress,
			(XCCM_K23_OFFSET)));
	CoefValues->K23 = XCcm_FixedToFloatConv(Data);

	Data = (XCcm_ReadReg(InstancePtr->Config.BaseAddress,
			(XCCM_K31_OFFSET)));
	CoefValues->K31 = XCcm_FixedToFloatConv(Data);

	Data = (XCcm_ReadReg(InstancePtr->Config.BaseAddress,
			(XCCM_K32_OFFSET)));
	CoefValues->K32 = XCcm_FixedToFloatConv(Data);

	Data = (XCcm_ReadReg(InstancePtr->Config.BaseAddress,
			(XCCM_K33_OFFSET)));
	CoefValues->K33 = XCcm_FixedToFloatConv(Data);
}

/*****************************************************************************/
/**
*
* This function sets the offset compensation for red, blue and green colors in
* corresponding Roffset, Goffset and Boffset registers of the CCM core.
*
* @param	InstancePtr is a pointer to the CCM instance.
* @param	ROffset specifies offset value of red color component which
*		needs to be set. (Range of offset is [-256 255]).
* @param	GOffset specifies offset value of green color component which
*		needs to be set. (Range of offset is [-256 255]).
* @param	BOffset specifies offset value of blue color component which
*		needs to be set. (Range of offset is [-256 255]).
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XCcm_SetRgbOffset(XCcm *InstancePtr, s32 ROffset, s32 GOffset,
						s32 BOffset)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(((s32)(ROffset) >= ((s32)(XCCM_OFFSET_FIRST))) &&
			((s32)(ROffset) <= (s32)(XCCM_OFFSET_LAST)));
	Xil_AssertVoid(((s32)(GOffset) >= ((s32)(XCCM_OFFSET_FIRST))) &&
			((s32)(GOffset) <= (s32)(XCCM_OFFSET_LAST)));
	Xil_AssertVoid(((s32)(BOffset) >= ((s32)(XCCM_OFFSET_FIRST))) &&
			((s32)(BOffset) <= (s32)(XCCM_OFFSET_LAST)));

	XCcm_WriteReg(InstancePtr->Config.BaseAddress, (XCCM_ROFFSET_OFFSET),
			ROffset);
	XCcm_WriteReg(InstancePtr->Config.BaseAddress, (XCCM_GOFFSET_OFFSET),
			GOffset);
	XCcm_WriteReg(InstancePtr->Config.BaseAddress, (XCCM_BOFFSET_OFFSET),
			BOffset);
}

/*****************************************************************************/
/**
*
* This function gets the offset compensation values of red, blue, green
* colors from Roffset, Goffset and Boffset registers.
*
* @param	InstancePtr is a pointer to the CCM instance.
* @param	ROffset is a pointer of signed 32 bit variable in which offset
*		of red color value is returned.
*		(Range of offset is [-256 255]).
* @param	GOffset is a pointer of signed 32 bit variable in which offset
*		of green color value is returned.
*		(Range of offset is [-256 255]).
* @param	BOffset is a pointer of signed 32 bit variable in which offset
*		of blue color value is returned.
*		(Range of offset is [-256 255]).
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XCcm_GetRgbOffset(XCcm *InstancePtr, s32 *ROffset, s32 *GOffset,
			s32 *BOffset)
{
	s32 Data;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(ROffset != NULL);
	Xil_AssertVoid(GOffset != NULL);
	Xil_AssertVoid(BOffset != NULL);

	Data = (s32)(XCcm_ReadReg(InstancePtr->Config.BaseAddress,
				(XCCM_ROFFSET_OFFSET)));
	*ROffset = (s32)((Data <<
			(XCCM_OFFSET_SIGN_SHIFT)) >> (XCCM_OFFSET_SIGN_SHIFT));
	Data = (s32)(XCcm_ReadReg(InstancePtr->Config.BaseAddress,
				(XCCM_GOFFSET_OFFSET)));
	*GOffset = (s32)((Data <<
			(XCCM_OFFSET_SIGN_SHIFT)) >> (XCCM_OFFSET_SIGN_SHIFT));
	Data = (s32)(XCcm_ReadReg(InstancePtr->Config.BaseAddress,
				(XCCM_BOFFSET_OFFSET)));
	*BOffset = (s32)((Data <<
			(XCCM_OFFSET_SIGN_SHIFT)) >> (XCCM_OFFSET_SIGN_SHIFT));
}

/*****************************************************************************/
/**
*
* This function sets the clip value in the Clip register of the CCM core.
*
* @param	InstancePtr is a pointer to the CCM instance.
* @param	Clip is the maximum output value which needs to be set.
*		(Range of Clip value is 0 to 255).
*
* @return	None.
*
* @note		If Output value greater than this Clip value it will be
*		replaced by this Clip value.
*
******************************************************************************/
void XCcm_SetClip(XCcm *InstancePtr, u32 Clip)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Clip <= (XCCM_CLIP_LAST));

	XCcm_WriteReg(InstancePtr->Config.BaseAddress, (XCCM_CLIP_OFFSET),
			(((Clip)) & (XCCM_CLIP_MASK)));
}

/*****************************************************************************/
/**
*
* This function gets the clip value from Clip register of the CCM core.
*
* @param	InstancePtr is a pointer to the CCM instance.
*
* @return	Clip value is returned.(Range is 0 to 255).
*
* @note		If Output value greater than this Clip value it will be
*		replaced by this Clip value.
*
******************************************************************************/
u32 XCcm_GetClip(XCcm *InstancePtr)
{
	u32 Value;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	Value = ((XCcm_ReadReg(InstancePtr->Config.BaseAddress,
				(XCCM_CLIP_OFFSET)))) & (XCCM_CLIP_MASK);

	return Value;
}

/*****************************************************************************/
/**
*
* This function sets the clamp value in the Clamp register.
*
* @param	InstancePtr is a pointer to the CCM instance
* @param	Clamp is the minimum output value which needs to be set.
*		(Range of Clamping value is 0 to 255).
*
* @return	None.
*
* @note		If Output value smaller than this Clamp value it will be
*		replaced by this value.
*
******************************************************************************/
void XCcm_SetClamp(XCcm *InstancePtr, u32 Clamp)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Clamp <= (XCCM_CLAMP_LAST));

	XCcm_WriteReg(InstancePtr->Config.BaseAddress, (XCCM_CLAMP_OFFSET),
				(((Clamp)) & (XCCM_CLAMP_MASK)));
}

/*****************************************************************************/
/**
*
* This function returns the clamp value from the Clamp register.
*
* @param	InstancePtr is a pointer to the CCM instance.
*
* @return	Clamp Value is returned.(Range is 0 to 255).
*
* @note		If Output value smaller than this clamp value it will be
*		replaced by this value.
*
******************************************************************************/
u32 XCcm_GetClamp(XCcm *InstancePtr)
{
	u32 Value;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	Value = (XCcm_ReadReg(InstancePtr->Config.BaseAddress,
				(XCCM_CLAMP_OFFSET)) & (XCCM_CLAMP_MASK));

	return Value;
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
*		layer when setting the callback functions, and passed back to
*		the upper layer when the callback is invoked.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void StubCallBack(void *CallBackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(CallBackRef != NULL);
	Xil_AssertVoidAlways();
}

/*****************************************************************************/
/**
* This routine is a stub for the asynchronous error interrupt callback. The
* stub is here in case the upper layer forgot to set the handler. On
* initialization, Error interrupt handler is set to this callback. It is
* considered an error for this handler to be invoked.
*
* @param	CallBackRef is a callback reference passed in by the upper
*		layer when setting the callback functions, and passed back to
*		the upper layer when the callback is invoked.
* @param	ErrorMask is a bit mask indicating the cause of the error. Its
* 		value equals 'OR'ing one or more XCCM_IXR_* values defined
* 		in xccm_hw.h.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void StubErrCallBack(void *CallBackRef, u32 ErrorMask)
{
	/* Verify arguments. */
	Xil_AssertVoid(ErrorMask != ((u32)0x0));
	(void)CallBackRef;
	Xil_AssertVoidAlways();
}

/*****************************************************************************/
/**
* This function converts 18.14 fixed point format numbers to floating point
* numbers.
*
* @param	Input is a 32-bit variable which should be given in 18.14 fixed
*		point number.
*
* @return	Value is returned after converting 18.14 to floating point.
*
* @note		None.
*
******************************************************************************/
static float XCcm_FixedToFloatConv(u32 Input)
{
	float Value;
	u32 Sign;
	Sign = 0;
	/*
	 * If all bits are 0 except sign bit returning lowest number in the
	 * given range.
	 */
	if ((Input & XCCM_COEF_MASK) == XCCM_COEF_SIGN_MASK) {
		return (XCCM_COEF_FIRST);
	}
	/* If number is negative finding 2's compliment of the number */
	if ((Input & XCCM_COEF_SIGN_MASK) != 0) {
		Sign = 1;
		Input = (Input << XCCM_COEF_SHIFT) >> (XCCM_COEF_SHIFT);
		Input = (~Input) + 1;
	}
	/* Getting integer part */
	Value = (float)((Input & XCCM_COEF_DECI_MASK) >> XCCM_COEF_SHIFT);
	/* Getting fractional part */
	Value = Value +
		((float)(Input & XCCM_COEFF_FRAC_MASK) /
					(float)(1 << XCCM_COEF_SHIFT));
	if (Sign == 1) {
		Value = (Value) * (float)(XCCM_SIGN_MUL);
	}

	return Value;
}

/*****************************************************************************/
/**
* This function converts floating point numbers to 18.14 fixed point format.
*
* @param	Input is a 32-bit variable which should be given in 18.14 fixed
*		point number.
*
* @return	Value is returned after converting 18.14 to floating point.
*
* @note		None.
*
******************************************************************************/
static s32 XCcm_FloatToFixedConv(float Input)
{
	s32 Value;
	/* Multiplying by 16384 times */
	Value = (s32)(Input * (float)(1 << XCCM_COEF_SHIFT));
	/* If number is negative finding 2's compliment */
	if ((Value & XCCM_SIGNBIT_MASK) != 0) {
		Value = XCCM_MAX_VALUE + Value + 1;
	}

	return Value;
}
/** @} */
