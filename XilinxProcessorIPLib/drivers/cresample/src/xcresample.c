/******************************************************************************
*
* Copyright (C) 2011 - 2014 Xilinx, Inc.  All rights reserved.
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
* @file xcresample.c
* @addtogroup cresample_v4_1
* @{
*
* This file contains the implementation of the interface functions for the
* Chroma Resampler core. Refer to the header file xcresample.h for more
* detailed information.
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- ----------------------------------------------------
* 1.00a gaborz 08/04/11 Updated for CRESAMPLE V1.0
* 2.00a vyc    04/24/12 Updated for CRESAMPLE V2.00.a
* 2.00a vyc    07/25/12 Switched from Xuu3232 to u32
* 2.00a vyc    10/16/12 Switch order of functions to remove compile warning
* 4.0   adk    03/12/14 Changed the filename cresample.c to xcresample.c.
*                       The Following functions are removed:
*                       clear_coef_values, configure_444_to_422,
*                       configure_422_to_444, configure_422_to_420,
*                       configure_420_to_422, configure_444_to_420 and
*                       configure_420_to_444.
*                       Implemented the following functions :
*                       StubCallBack, StubErrorCallBack,
*                       XCresample_CfgInitialize,
*                       XCresample_GetVersion, XCresample_EnableDbgByPass,
*                       XCresample_IsDbgByPassEnabled,
*                       XCresample_DisableDbgByPass,
*                       XCresample_SetDbgTestPattern,
*                       XCresample_IsDbgTestPatternEnabled,
*                       XCresample_DisableDbgTestPattern
*                       XCresample_GetDbgFrameCount,
*                       XCresample_GetDbgLineCount,
*                       XCresample_GetDbgPixelCount,
*                       XCresample_SetActiveSize, XCresample_GetActiveSize,
*                       XCresample_SetFieldParity, XCresample_GetFieldParity,
*                       XCresample_SetChromaParity,
*                       XCresample_GetChromaParity
*                       XCresample_SetHCoefs, XCresample_GetHCoefs,
*                       XCresample_SetVCoefs, XCresample_GetVCoefs,
*                       XCresample_Clear_HCoef_Values, and
*                       XCresample_Clear_VCoef_Values.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcresample.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

static void StubCallBack(void *CallBackRef);
static void StubErrCallBack(void *CallBackRef, u32 ErrorMask);
static float XCresample_FixedToFloatConv(u32 Input);
static s32 XCresample_FloatToFixedConv(float Input);

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function initializes the Cresample core. This function must be called
* prior to using the Cresample core. Initialization of the Cresample includes
* setting up the instance data and ensuring the hardware is in a quiescent
* state.
*
* @param	InstancePtr is a pointer to the XCresample instance.
* @param	CfgPtr is a reference to a structure containing information
*		about a specific Chroma Resampler driver.
* @param	EffectiveAddr is the device base address in the virtual memory
*		address space. The caller is responsible for keeping the
*		address mapping from EffectiveAddr to the device physical base
*		address unchanged once this function is invoked. Unexpected
*		errors may occur if the address mapping changes after this
*		function is called. If address translation is not used, pass in
*		the physical address instead.
*
* @return	- XST_SUCCESS if initialization was successful.
*
* @note		None.
*
******************************************************************************/
int XCresample_CfgInitialize(XCresample *InstancePtr,
				XCresample_Config *CfgPtr, u32 EffectiveAddr)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddr != ((u32)0x0));

	/* Setup the instance */
	(void)memset((void *)InstancePtr, 0, sizeof(XCresample));
	(void)memcpy((void *)(&(InstancePtr->Config)), (const void *)CfgPtr,
			sizeof(XCresample_Config));
	InstancePtr->Config.BaseAddress = EffectiveAddr;

	/*
	 * Set all handlers to stub values, let user configure this data later.
	 */
	InstancePtr->ProcStartCallBack =
				(XCresample_CallBack)((void *)StubCallBack);
	InstancePtr->FrameDoneCallBack =
				(XCresample_CallBack)((void *)StubCallBack);
	InstancePtr->ErrCallBack =
			(XCresample_ErrorCallBack)((void *)StubErrCallBack);

	/*
	 * Reset the hardware and set the flag to indicate the driver is
	 * ready.
	 */
	XCresample_Reset(InstancePtr);
	InstancePtr->IsReady = (u32)(XIL_COMPONENT_IS_READY);

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function enables the bypass mode by setting bypass bit of the Control
* register to switch the core to bypass mode if debug feature is enabled in
* the core.
*
* @param	InstancePtr is a pointer to the XCresample instance to be
*		worked on.
*
* @return	None.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
void XCresample_EnableDbgByPass(XCresample *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->Config.HasDebug != (u16)0x0);

	XCresample_WriteReg(InstancePtr->Config.BaseAddress,
				(XCRE_CONTROL_OFFSET),
			((XCresample_ReadReg(InstancePtr->Config.BaseAddress,
			(XCRE_CONTROL_OFFSET))) | (XCRE_CTL_BPE_MASK)));

}

/*****************************************************************************/
/**
*
* This function returns the current bypass mode settings from Control register
* of the Chroma Resampler core.
*
* @param	InstancePtr is a pointer to the XCresample instance to be
*		worked on.
*
* @return	Core debug bypass mode.
*		- TRUE = Bypass mode is enabled.
*		- FALSE = Bypass mode is not enabled.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
int XCresample_IsDbgByPassEnabled(XCresample *InstancePtr)
{
	u32 DbgByPass;
	int Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.HasDebug != (u16)0x0);

	DbgByPass = (XCresample_ReadReg(InstancePtr->Config.BaseAddress,
				(XCRE_CONTROL_OFFSET))) & (XCRE_CTL_BPE_MASK);
	if (DbgByPass == (XCRE_CTL_BPE_MASK)) {
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
* This function disables bypass mode of the Chroma Resampler core.
*
* @param	InstancePtr is a pointer to the XCresample instance to be
*		worked on.
*
* @return	None.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
void XCresample_DisableDbgBypass(XCresample *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->Config.HasDebug != (u16)0x0);

	XCresample_WriteReg(InstancePtr->Config.BaseAddress,
				(XCRE_CONTROL_OFFSET),
			((XCresample_ReadReg(InstancePtr->Config.BaseAddress,
			(XCRE_CONTROL_OFFSET))) & (~(XCRE_CTL_BPE_MASK))));
}

/*****************************************************************************/
/**
*
* This function enables the test-pattern mode if debug feature is enabled
* by setting test-pattern bit of the Control register of the Chroma Resampler
* core.
*
* @param	InstancePtr is a pointer to the XCresample instance to be
*		worked on.
*
* @return	None.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
void XCresample_EnableDbgTestPattern(XCresample *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->Config.HasDebug != (u16)0x0);

	XCresample_WriteReg(InstancePtr->Config.BaseAddress,
				(XCRE_CONTROL_OFFSET),
			((XCresample_ReadReg(InstancePtr->Config.BaseAddress,
			(XCRE_CONTROL_OFFSET))) | (XCRE_CTL_TPE_MASK)));
}

/*****************************************************************************/
/**
*
* This function returns the test-pattern mode (enabled or not) from Control
* register of the Chroma Resampler core.
*
* @param	InstancePtr is a pointer to the XCresample instance to
*		be worked on.
*
* @return	Test-pattern generator mode.
*		- TRUE = Test-pattern mode is enabled.
*		- FALSE = Test-pattern mode is not enabled.
*
* @note		Debug functionality should be enabled.
*
*****************************************************************************/
int XCresample_IsDbgTestPatternEnabled(XCresample *InstancePtr)
{
	u32 DbgTestPattern;
	int Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.HasDebug != (u16)0x0);

	DbgTestPattern = (XCresample_ReadReg(InstancePtr->Config.BaseAddress,
				(XCRE_CONTROL_OFFSET))) & (XCRE_CTL_TPE_MASK);
	if (DbgTestPattern == (XCRE_CTL_TPE_MASK)) {
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
* Chroma Resampler core, if Debug feature is enabled.
*
* @param	InstancePtr is a pointer to the XCresample instance to be
*		worked on.
*
* @return	None.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
void XCresample_DisableDbgTestPattern(XCresample *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->Config.HasDebug != (u16)0x0);
	XCresample_WriteReg(InstancePtr->Config.BaseAddress,
					(XCRE_CONTROL_OFFSET),
		((XCresample_ReadReg(InstancePtr->Config.BaseAddress,
			(XCRE_CONTROL_OFFSET))) & (~(XCRE_CTL_TPE_MASK))));
}

/*****************************************************************************/
/**
*
* This function returns the version of the Chroma Resampler core.
*
* @param	InstancePtr is a pointer to the XCresample instance.
*
* @return	Content of Version register is returned..
*
* @note		None.
*
******************************************************************************/
u32 XCresample_GetVersion(XCresample *InstancePtr)

{
	u32 Data;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	Data = XCresample_ReadReg(InstancePtr->Config.BaseAddress,
					(XCRE_VERSION_OFFSET));

	return Data;
}

/*****************************************************************************/
/**
*
* This function returns the number of frames processed since power-up or last
* time the core is reset.
*
* @param	InstancePtr is a pointer to the XCresample instance.
*
* @return	FrameCount is the number of frames processed since power-up.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
u32 XCresample_GetDbgFrameCount(XCresample *InstancePtr)
{
	u32 FrameCount;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.HasDebug != (u16)0x0);

	/* Frame throughput monitor */
	FrameCount = XCresample_ReadReg(InstancePtr->Config.BaseAddress,
					(XCRE_SYSDEBUG0_OFFSET));

	return FrameCount;
}

/*****************************************************************************/
/**
*
* This function returns the number of lines processed since power-up or last
* time the core is reset.
*
* @param	InstancePtr is a pointer to the XCresample instance.
*
* @return	LineCount is the number of lines processed since power-up.
*
* @note		Debug functionality should be enabled.
*
******************************************************************************/
u32 XCresample_GetDbgLineCount(XCresample *InstancePtr)
{
	u32 LineCount;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.HasDebug != (u16)0x0);

	/* Line throughput monitor */
	LineCount = XCresample_ReadReg(InstancePtr->Config.BaseAddress,
					(XCRE_SYSDEBUG1_OFFSET));

	return LineCount;
}

/*****************************************************************************/
/**
*
* This function returns the number of pixels processed since power-up or last
* time the core is reset.
*
* @param	InstancePtr is a pointer to the XCresample instance.
*
* @return	PixelCount is the number of pixels processed since power-up.

* @note		Debug functionality should be enabled.
*
******************************************************************************/
u32 XCresample_GetDbgPixelCount(XCresample *InstancePtr)
{
	u32 PixelCount;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.HasDebug != (u16)0x0);

	/* Pixel throughput monitor */
	PixelCount = XCresample_ReadReg(InstancePtr->Config.BaseAddress,
					(XCRE_SYSDEBUG2_OFFSET));

	return PixelCount;
}

/*****************************************************************************/
/**
*
* This function sets the active H/V sizes in the Active Size register.
*
* @param	InstancePtr is a pointer to the XCresample instance.
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
void XCresample_SetActiveSize(XCresample *InstancePtr, u16 HSize, u16 VSize)
{
	u32 Size;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((VSize >= (u16)(XCRE_ACT_SIZE_FIRST)) &&
			(VSize <= (u16)(XCRE_ACT_SIZE_LAST)));
	Xil_AssertVoid((HSize >= (u16)(XCRE_ACT_SIZE_FIRST)) &&
			(HSize <= (u16)(XCRE_ACT_SIZE_LAST)));

	Size = (((u32)VSize) << ((u32)(XCRE_ACTSIZE_NUM_LINE_SHIFT))) |
								(HSize);
	XCresample_WriteReg(InstancePtr->Config.BaseAddress,
				(XCRE_ACTIVE_SIZE_OFFSET), Size);
}

/*****************************************************************************/
/**
*
* This function gets the number of Active Pixels per Scan line and number of
* Active Lines per Frame from the Active Size register.
*
* @param	InstancePtr is a pointer to the XCresample instance.
* @param	HSize is a pointer to 16-bit variable in which the number of
*		Active Pixels per Scan Line is returned (Range is 32 to 7680).
* @param	VSize is a pointer to 16-bit variable in which the number of
*		Active Lines per Frame is returned (Range is 32 to 7680).
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XCresample_GetActiveSize(XCresample *InstancePtr, u16 *HSize, u16 *VSize)
{
	u32 Data;
	/* Verify arguments*/
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(HSize != NULL);
	Xil_AssertVoid(VSize != NULL);

	Data = XCresample_ReadReg(InstancePtr->Config.BaseAddress,
			(XCRE_ACTIVE_SIZE_OFFSET));

	/* Number of active pixels per scan line */
	*HSize = (u16)(Data & (XCRE_ACTSIZE_NUM_PIXEL_MASK));

	/* Number of active lines per frame */
	*VSize = (u16)((Data & (XCRE_ACTSIZE_NUM_LINE_MASK)) >>
				(XCRE_ACTSIZE_NUM_LINE_SHIFT));
}

/*****************************************************************************/
/**
*
* This functions sets the field parity value of the Chroma Resampler core in
* the Encoding register.
*
* @param	InstancePtr is a pointer to the XCresample instance.
* @param	FieldParity specifies the parity value which needs to be
*		set.(0 or 1).
*		- 0 - for even or bottom field.
*		- 1 - for odd or top field.
*
* @return	None.
*
* @note		To this feature need to use interlaced video.
*
******************************************************************************/
void XCresample_SetFieldParity(XCresample *InstancePtr, u8 FieldParity)
{
	/* Verify arguments*/
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->Config.Interlaced == 1);
	Xil_AssertVoid((FieldParity == (XCRE_PARITY_ODD)) ||
			(FieldParity == (XCRE_PARITY_EVEN)));

	/* Checking the type of field parity */
	if ((FieldParity) == (XCRE_PARITY_ODD)) {
		XCresample_WriteReg(InstancePtr->Config.BaseAddress,
			(XCRE_ENCODING_OFFSET),
			((XCresample_ReadReg(InstancePtr->Config.BaseAddress,
				(XCRE_ENCODING_OFFSET))) |
					((u32)(XCRE_ENCODING_FIELD_MASK))));
	}
	if ((FieldParity) == (XCRE_PARITY_EVEN)) {
		XCresample_WriteReg(InstancePtr->Config.BaseAddress,
			(XCRE_ENCODING_OFFSET),
			((XCresample_ReadReg(InstancePtr->Config.BaseAddress,
			(XCRE_ENCODING_OFFSET))) &
					((u32)(~(XCRE_ENCODING_FIELD_MASK)))));
	}
}

/*****************************************************************************/
/**
*
* This function returns the field parity value of the Chroma Resampler core
* from Encoding register.
*
* @param	InstancePtr is a pointer to the XCresample instance.
*
* @return	Field parity value is returned.
*		0 - for even or bottom field.
*		1 - for odd or top field.
*
* @note		None.
*
******************************************************************************/
u8 XCresample_GetFieldParity(XCresample *InstancePtr)
{
	u32 Data;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	Data = ((XCresample_ReadReg(InstancePtr->Config.BaseAddress,
		(XCRE_ENCODING_OFFSET))) & (XCRE_ENCODING_FIELD_MASK)) >>
				(XCRE_ENCODING_FIELD_SHIFT);

	return (u8)Data;
}

/*****************************************************************************/
/**
*
* This functions sets the Chroma parity value of the Chroma Resampler core in
* the Encoding register.
*
* @param	InstancePtr is a pointer to the XCresample instance.
* @param	ChromaParity specifies the parity value which needs to be
*		set.(0 or 1).
*		- 0 - Chroma information on Odd or First line.
*		- 1 - Chroma information on Even lines.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XCresample_SetChromaParity(XCresample *InstancePtr, u8 ChromaParity)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((ChromaParity == (XCRE_PARITY_ODD)) ||
			(ChromaParity == (XCRE_PARITY_EVEN)));

	/* Checking type of ChromaParity value */
	if ((ChromaParity) == (XCRE_PARITY_ODD)) {
		XCresample_WriteReg(InstancePtr->Config.BaseAddress,
			(XCRE_ENCODING_OFFSET),
			((XCresample_ReadReg(InstancePtr->Config.BaseAddress,
				(XCRE_ENCODING_OFFSET))) |
					((u32)(XCRE_ENCODING_CHROMA_MASK))));
	}
	if ((ChromaParity) == (XCRE_PARITY_EVEN)) {
		XCresample_WriteReg(InstancePtr->Config.BaseAddress,
			(XCRE_ENCODING_OFFSET),
			(XCresample_ReadReg(InstancePtr->Config.BaseAddress,
				(XCRE_ENCODING_OFFSET)) &
					(u32)(~(XCRE_ENCODING_CHROMA_MASK))));
	}
}

/*****************************************************************************/
/**
*
* This function returns the value of chroma parity of the Chroma Resampler core
* from Encoding register.
*
* @param	InstancePtr is a pointer to the XCresample instance.
*
* @return	Chroma parity value is returned.
*		- 0 - Chroma information on Odd or First line.
*		- 1 - Chroma information on Even lines.
*
* @note		None.
*
******************************************************************************/
u8 XCresample_GetChromaParity(XCresample *InstancePtr)
{
	u32 Data;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	Data = ((XCresample_ReadReg(InstancePtr->Config.BaseAddress,
		(XCRE_ENCODING_OFFSET))) & (XCRE_ENCODING_CHROMA_MASK)) >>
				(XCRE_ENCODING_CHROMA_SHIFT);

	return (u8)Data;
}

/*****************************************************************************/
/**
*
* This function sets the horizontal coefficient values of the Chroma Resampler
* core for phase 0 or phase 1 or both phases.
*
* @param	InstancePtr is a pointer to the XCresample instance.
* @param	Coeff is a pointer to the structure XHorizontal_Coeffs which
*		has horizontal coefficients of both phases (Phase 0 and Phase 1)
*		which needs to be set.
*		Range of coefficient values is [-2, 2).
* @param	Phases specifies number of phases needs to be set.
*		- 0 - Phase 0.
*		- 1 - Phase 1.
*		- 2 - Phase 0 and Phase 1.
*
* @return	None.
*
* @note		For pre defined fixed coefficients filter values, ConvertType
*		should be set with 1.
*
******************************************************************************/
void XCresample_SetHCoefs(XCresample *InstancePtr, XHorizontal_Coeffs *Coeff,
							u32 Phases)
{
	u32 Index1;
	u32 Index2;
	u32 Offset;
	s32 Data;
	float Value;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Coeff != NULL);
	Xil_AssertVoid(Phases <= XCRE_NUM_OF_PHASES);
	Xil_AssertVoid(InstancePtr->Config.ConvertType == 1);

	XCresample_Clear_HCoef_Values(InstancePtr);
	if ((Phases == 0) || (Phases == 2)) {
		Offset = XCRE_COEF00_HPHASE0_OFFSET;
		Index1 = 0;
	}
	if (Phases == 1) {
		Offset = XCRE_COEF00_HPHASE1_OFFSET;
		Index1 = 1;
	}
	/* Setting horizontal coefficients based on number of Phases */
	do {
		for (Index2 = (u32)0x0;
			Index2 < (InstancePtr->Config.NumHTaps); Index2++) {
			Value = Coeff->HCoeff[Index1][Index2];
			Data = XCresample_FloatToFixedConv(Value);
			Data = Data & (XCRE_COEFF_MASK);
			XCresample_WriteReg(InstancePtr->Config.BaseAddress,
						Offset, Data);
			Offset = Offset + (XCRE_OFFSET_DIFF);
		}
		Index1 = 2;
		if (((Phases == 1) || (Phases == 2)) && (Index1 == 0)) {
			Offset = XCRE_COEF00_HPHASE1_OFFSET;
			Index1 = 1;
		}
	}while (Index1 < XCRE_NUM_OF_PHASES);

}

/*****************************************************************************/
/**
*
* This function gets the coefficient values from all the Horizontal Coefficient
* registers of phase0 and phase1 of the Chroma Resampler core.
*
* @param	InstancePtr is a pointer to the XCresample instance.
* @param	Coeff is a pointer to the structure XHorizontal_Coeffs which
*		has horizontal coefficients of both phases.(Phase 0 and Phase 1)
*		in which coefficients are updated.
*		Range of coefficient values is [-2, 2).
*
* @return	None.
*
* @note		For pre defined fixed coefficients filter values, ConvertType
*		should be set with 1.
*
******************************************************************************/
void XCresample_GetHCoefs(XCresample *InstancePtr, XHorizontal_Coeffs *Coeff)
{
	u32 Index1;
	u32 Index2;
	u32 Offset;
	s32 Data;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Coeff != NULL);
	Xil_AssertVoid(InstancePtr->Config.ConvertType == 1);

	Offset = (u32)(XCRE_COEF00_HPHASE0_OFFSET);

	/*
	 * Getting coefficient values from all Horizontal Coefficient
	 * registers
	 */
	for (Index1 = (u32)0x0; Index1 < (XCRE_NUM_OF_PHASES); Index1++) {
		for (Index2 = (u32)0x0; Index2 < (XCRE_NUM_HCOEFS);
								Index2++ ) {
			Data =
			(s32)(XCresample_ReadReg(
				InstancePtr->Config.BaseAddress, Offset));
			Coeff->HCoeff[Index1][Index2] =
					XCresample_FixedToFloatConv(Data);
			Offset = Offset + (XCRE_OFFSET_DIFF);
		}
	}
}

/*****************************************************************************/
/**
*
* This function sets the vertical coefficient values of the Chroma Resampler
* for phase 0 or for phase 1 or for both phases.
*
* @param	InstancePtr is a pointer to the XCresample instance.
* @param	Coeff is a pointer to the structure XHorizontal_Coeffs which
*		has vertical coefficients of both phases (Phase 0 and Phase 1)
*		which needs to be set.
* @param	Phases specifies number of phases needs to be set.
*		- 0 - Phase 0.
*		- 1 - Phase 1.
*		- 2 - Phase 0 and Phase 1.
*
* @return	None.
*
* @note		For pre defined fixed coefficients filter values, ConvertType
*		should be set with 1.
*
******************************************************************************/
void XCresample_SetVCoefs(XCresample *InstancePtr, XVertical_Coeffs *Coeff,
								u32 Phases)
{
	u32 Index1;
	u32 Index2;
	u32 Offset;
	s32 Data;
	float Value;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Coeff != NULL);
	Xil_AssertVoid(Phases <= XCRE_NUM_OF_PHASES);
	Xil_AssertVoid(InstancePtr->Config.ConvertType == 1);

	XCresample_Clear_VCoef_Values(InstancePtr);

	/*
	 * Setting coefficient values in all Vertical
	 * Coefficient registers
	 */

	if ((Phases == 0) || (Phases == 2)) {
		Offset = XCRE_COEF00_VPHASE0_OFFSET;
		Index1 = 0;
	}
	if (Phases == 1) {
		Index1 = 1;
		Offset = XCRE_COEF00_VPHASE1_OFFSET;
	}
	/* Setting Vertical Coefficients based on number of Phases */
	do {
		for (Index2 = (u32)0x0;
			Index2 < (InstancePtr->Config.NumVTaps); Index2++) {
			Value = Coeff->VCoeff[Index1][Index2];
			Data = XCresample_FloatToFixedConv(Value);
			Data = Data & (XCRE_COEFF_MASK);
			XCresample_WriteReg(InstancePtr->Config.BaseAddress,
						Offset, Data);
			Offset = Offset + (XCRE_OFFSET_DIFF);
		}
		Index1 = 2;
		if (((Phases == 1) || (Phases == 2)) && (Index1 == 0)) {
			Index1 = 1;
			Offset = XCRE_COEF00_VPHASE1_OFFSET;
		}
	} while (Index1 < XCRE_NUM_OF_PHASES);

}

/*****************************************************************************/
/**
*
* This function gets the vertical coefficient values of the Chroma Resampler
* core from all Vertical Coefficient registers of phase0 and phase1.
*
* @param	InstancePtr is a pointer to the XCresample instance.
* @param	Coeff is a pointer to the structure XVertical_Coeffs which has
*		vertical coefficients of both phases.(Phase 0 and Phase 1) in
*		which coefficients are updated.
*		Range of coefficient values is [-2, 2).
*
* @return	None.
*
* @note		For pre defined fixed coefficients filter values, ConvertType
*		should be set with 1.
*
******************************************************************************/
void XCresample_GetVCoefs(XCresample *InstancePtr, XVertical_Coeffs *Coeff)
{
	u32 Index1;
	u32 Index2;
	u32 Offset;
	s32 Data;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Coeff != NULL);
	Xil_AssertVoid(InstancePtr->Config.ConvertType == 1);

	Offset = (u32)(XCRE_COEF00_VPHASE0_OFFSET);

	/*
	 * Getting coefficient values from all Vertical
	 * Coefficient registers
	 */
	for (Index1 = (u32)0x0; Index1 < (XCRE_NUM_OF_PHASES); Index1++) {
		for (Index2 = (u32)0x0; Index2 < (XCRE_NUM_VCOEFS);
								Index2++) {
			Data =
					(s32)(XCresample_ReadReg(
					InstancePtr->Config.BaseAddress,
						Offset));
			Coeff->VCoeff[Index1][Index2] =
					XCresample_FixedToFloatConv(Data);
			Offset = Offset + (XCRE_OFFSET_DIFF);
		}
	}
}

/*****************************************************************************/
/**
*
* This function clears all horizontal coefficients of the Chroma Resampler core
* which are previously set.
*
* @param	InstancePtr is a pointer to the XCresample instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XCresample_Clear_HCoef_Values (XCresample *InstancePtr)
{
	u32 Index1;
	u32 Index2;
	u32 Offset;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	Offset = XCRE_COEF00_HPHASE0_OFFSET;
	/* Set all horizontal coefficient values to 0 */
	for (Index1 = 0; Index1 < XCRE_NUM_OF_PHASES; Index1++) {
		for (Index2 = 0; Index2 < XCRE_NUM_HCOEFS; Index2++) {
			XCresample_WriteReg(InstancePtr->Config.BaseAddress,
							Offset, 0x00);
			Offset = Offset + XCRE_OFFSET_DIFF;
		}
	}
}

/*****************************************************************************/
/**
*
* This function clears all vertical coefficients of a the Chroma Resampler
* which are previously set.
*
* @param	InstancePtr is a pointer to the XCresample instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XCresample_Clear_VCoef_Values (XCresample *InstancePtr)
{
	u32 Index1;
	u32 Index2;
	u32 Offset;
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	Offset = XCRE_COEF00_VPHASE0_OFFSET;
	/* Set all vertical coefficient values to 0 */
	for (Index1 = 0; Index1 < XCRE_NUM_OF_PHASES; Index1++) {
		for (Index2 = 0; Index2 < XCRE_NUM_VCOEFS; Index2++) {
			XCresample_WriteReg(InstancePtr->Config.BaseAddress,
							Offset, 0x00);
			Offset = Offset + XCRE_OFFSET_DIFF;
		}
	}
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
static float XCresample_FixedToFloatConv(u32 Input)
{
	float Value;
	u32 Sign;
	Sign = 0;

	if ((Input & XCRE_COEFF_MASK) == XCRE_COEF_SIGN_MASK) {
		return (XCRE_COEF_FIRST);
	}
	if ((Input & XCRE_COEF_SIGN_MASK) != 0) {
		Sign = 1;
		Input = (Input << XCRE_COEFF_SIGN_SHIFT) >>
						(XCRE_COEFF_SIGN_SHIFT);
		Input = (~Input) + 1;
	}
	Value = (float)((Input & XCRE_COEF_DECI_MASK) >> XCRE_COEFF_SHIFT);
	Value = Value +
		((float)(Input & XCRE_COEFF_FRAC_MASK) /
					(float)(1 << XCRE_COEFF_SHIFT));
	if (Sign == 1) {
		Value = (Value) * (float)(XCRE_SIGN_MUL);
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
static s32 XCresample_FloatToFixedConv(float Input)
{
	s32 Value;
	Value = (s32)(Input * (float)(1 << XCRE_COEFF_SHIFT));
	if ((Value & XCRE_SIGNBIT_MASK) != 0) {
		Value = XCRE_MAX_VALUE + Value + 1;
	}

	return Value;
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
*		layer when setting the callback functions, and passed back to
*		the upper layer when the callback is invoked.
*
* @return	None.
*
******************************************************************************/
static void StubCallBack(void *CallBackRef)
{
	(void)CallBackRef;
	/* Verify arguments. */
	Xil_AssertVoidAlways();
}

/*****************************************************************************/
/**
* This routine is a stub for the asynchronous error interrupt callback. The
* stub is here in case the upper layer forgot to set the handler. On
* initialization, Error interrupt handler is set to this callback.
*
* @param	CallBackRef is a callback reference passed in by the upper
*		layer when setting the callback functions, and passed back to
*		the upper layer when the callback is invoked.
* @param	ErrorMask is a bit mask indicating the cause of the error. Its
* 		value equals 'OR'ing one or more XCRE_IXR_* values defined
* 		in xcresample_hw.h.
*
* @return	None.
*
******************************************************************************/
static void StubErrCallBack(void *CallBackRef, u32 ErrorMask)
{
	(void)CallBackRef;
	(void)ErrorMask;
	/* Verify arguments. */
	Xil_AssertVoidAlways();
}
/** @} */
