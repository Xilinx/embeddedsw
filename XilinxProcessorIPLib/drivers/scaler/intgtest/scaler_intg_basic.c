/******************************************************************************
*
*       XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS"
*       AS A COURTESY TO YOU, SOLELY FOR USE IN DEVELOPING PROGRAMS AND
*       SOLUTIONS FOR XILINX DEVICES.  BY PROVIDING THIS DESIGN, CODE,
*       OR INFORMATION AS ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE,
*       APPLICATION OR STANDARD, XILINX IS MAKING NO REPRESENTATION
*       THAT THIS IMPLEMENTATION IS FREE FROM ANY CLAIMS OF INFRINGEMENT,
*       AND YOU ARE RESPONSIBLE FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE
*       FOR YOUR IMPLEMENTATION.  XILINX EXPRESSLY DISCLAIMS ANY
*       WARRANTY WHATSOEVER WITH RESPECT TO THE ADEQUACY OF THE
*       IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR
*       REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM CLAIMS OF
*       INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*       FOR A PARTICULAR PURPOSE.
*
*       (c) Copyright 2014 Xilinx Inc.
*       All rights reserved.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file scaler_intg_basic.c
*
* @note
*
* This test works with Zynq702 system.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------ -------- ---------------------------------------------
* 7.0   adk   22/08/14 First release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xscaler.h"
#include "xscaler_hw.h"
#include "xparameters.h"
#include "scaler_intgtest.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/*****************************************************************************/
/**
 *
 * Compare numbers. If not equal, then output message and increment fail
 * counter. The message contains the line number of the failure, the
 * name of the variable, and its actual and expected values in hex and
 * decimal. If equal, display current test pass number to user.
 * An example:
 *
 * <pre>
 *     1  UINT32 result = 5;
 *     2  UINT32 expected = 17;
 *     3
 *     4  CT_CMP_NUM(UINT32, result, expected)
 *
 * yields the output:
 *
 *     FAIL: 0004: result=5(5h), expected 17(11h)
 * </pre>
 *
 * @param	Type is data type to compare (must be an ordinal type such
 *		as int)
 * @param	Val_16 is the actual data retrieved from test
 * @param	Val_32 is the expected value
 *
 * @note	Usage: CT_CMP_NUM(<type>, actual, expected)
 *
 *****************************************************************************/
#define check_status_update(Type, Val_16, Val_32) \
	if((Type)(Val_16) != (Type)(Val_32)){ \
		CT_CMP_NUM(Type, XST_FAILURE, XST_SUCCESS); \
	} \
	else{ \
		CT_NotifyNextPass(); \
	} \

/**************************** Type Definitions *******************************/
#define round(x) ((x) >= 0 ? (s32)((x) + 0.5) : (s32)((x) - 0.5))

/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function executes the XScaler Self test.
*
* @param	TestLoops is the number of times to execute test.
*
* @return	Returns the total number of test failures.
*
* @note		None.
*
******************************************************************************/
int Scaler_Intg_SelfTest(int TestLoops)
{
	u32 Status = (u32)XST_SUCCESS;

	double HoriScaleFactor;
	double VertScaleFactor;


	u32 OldScalerReg;
	u32 NewScalerReg;
	u32 LumaLeftH;
	u32 LumaTopV;
	u32 ChromaLeftH;
	u32 ChromaTopV;

	u32 OutSize;
	u32 InLine;
	u32 InPixel;
	u32 SrcSize;
	u32 QuantizedHoriSize;
	u32 QuantizedVertSize;
	u32 QuantizedInLastPixel;
	u32 QuantizedInLastLine;

	u32 PhaseRegValue;
	u16 VertPhaseNum;
	u16 HoriPhaseNum;

	u8 ChromaFormat;
	u8 ChromaLumaShareCoeff;
	u8 HoriVertShareCoeff;
	u8 VertSetIndex;
	u8 HoriSetIndex;

	u32 InLine_1;
	u32 InPixel_1;
	u32 OutSize_1 ;
	u32 SrcSize_1 ;
	double HoriScaleFactor_1;
	double VertScaleFactor_1;

	XScaler ScalerInst;

	XScalerStartFraction StartFractionInst;

	XScalerCoeffBank CoeffBankPtr;

	XScalerAperture AperturePtr;
	XScalerAperture AperturePtr_1;

	XScaler_Config *Config;

	CT_TestReset("Scaler Self Test .. ..");
	while (TestLoops--) {
	CT_NotifyNextPass();

	/* Initialize the XScaler instance. */
	Status = Scaler_Initialize(&ScalerInst, (u16)SCALER_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return Status;
	}

/*****************************************************************************/
/* XScaler_GetVersion */

	/*Test case 1 */
	NewScalerReg = XScaler_GetVersion(&ScalerInst);
	if (NewScalerReg != 0x00000000) {
		check_status_update(u32, 1, 1);
	}
	else {
		check_status_update(u32, 0, 1);
	}

	/*Test case 3 */
//	XScaler_GetVersion(Null);

/*****************************************************************************/
	/* XScaler_Disable */

	/*Test case 1 */
	XScaler_Disable(&ScalerInst);
	if (0x0 == ((XScaler_ReadReg(ScalerInst.Config.BaseAddress,
			XSCL_CTL_OFFSET)) & ~(XSCL_CTL_SW_EN_MASK))) {
		check_status_update(u32, 1, 1);
	}
	else {
		check_status_update(u32, 0, 1);
	}

	/*
	 * Negative Test Case
	 */
//	XScaler_Disable(&ScalerInst);

/*****************************************************************************/
	/* XScaler_Enable */

	/* check the bit 0 of control register, default is 0*/
	/*Test case 1 */
	XScaler_Enable(&ScalerInst);
	if(0x00000001 == ((XScaler_ReadReg(ScalerInst.Config.BaseAddress,
		XSCL_CTL_OFFSET)) | (XSCL_CTL_SW_EN_MASK))){
		check_status_update(u32, 1, 1);
	}
	else{
		check_status_update(u32, 0, 1);
	}
	XScaler_Enable(&ScalerInst);

/*****************************************************************************/
	/* XScaler_SetPhaseNum */

	/* Test case 1*/
	VertPhaseNum = 1;
	ScalerInst.Config.MaxPhaseNum = 1;
	HoriPhaseNum = 1;
	ScalerInst.Config.MaxPhaseNum = 1;

	XScaler_SetPhaseNum(&ScalerInst, VertPhaseNum, HoriPhaseNum);
	OldScalerReg = XScaler_ReadReg(ScalerInst.Config.BaseAddress,
					XSCL_NUMPHASE_OFFSET);
	NewScalerReg = (VertPhaseNum << XSCL_NUMPHASE_VERT_SHIFT) &
					XSCL_NUMPHASE_VERT_MASK;
	NewScalerReg |= HoriPhaseNum & XSCL_NUMPHASE_HORI_MASK;
	if (NewScalerReg == OldScalerReg) {
		check_status_update(u32, 1, 1);
	}
	else {
		check_status_update(u32, 0, 1);
	}

	/* Test case 2 */
	XScaler_SetPhaseNum(&ScalerInst, VertPhaseNum, HoriPhaseNum);
	OldScalerReg = XScaler_ReadReg(ScalerInst.Config.BaseAddress,
					XSCL_NUMPHASE_OFFSET);
	if ((VertPhaseNum == (OldScalerReg & XSCL_NUMPHASE_VERT_MASK) >>
				XSCL_NUMPHASE_VERT_SHIFT) &&
				(HoriPhaseNum == (OldScalerReg &
					XSCL_NUMPHASE_HORI_MASK))) {
		check_status_update(u32, 1, 1);
	}
	else {
		check_status_update(u32, 0, 1);
	}

	/*
	 * Negative Test Case
	 */

	/* Test case 3 */
	//XScaler_SetPhaseNum(Null, VertPhaseNum, HoriPhaseNum);

/*****************************************************************************/
	/* XScaler_GetPhaseNum */
	XScaler_GetPhaseNum(&ScalerInst, &VertPhaseNum, &HoriPhaseNum);
	PhaseRegValue = XScaler_ReadReg((ScalerInst).Config.BaseAddress,
						XSCL_NUMPHASE_OFFSET);
	OldScalerReg = (PhaseRegValue & XSCL_NUMPHASE_VERT_MASK) >>
			XSCL_NUMPHASE_VERT_SHIFT;
	NewScalerReg = PhaseRegValue & XSCL_NUMPHASE_HORI_MASK;
	if ((NewScalerReg == HoriPhaseNum) && (OldScalerReg == VertPhaseNum)) {
		check_status_update(u32, 1, 1);
	}
	else {
		check_status_update(u32, 0, 1);
	}

	/*
	 * Negative Test Case
	 */

	/* Test case 3 */
	//XScaler_GetPhaseNum(Null, &VertPhaseNum, &HoriPhaseNum);

/*****************************************************************************/
	XScaler_SetStartFraction(&ScalerInst, &StartFractionInst);

	LumaLeftH = XScaler_ReadReg(ScalerInst.Config.BaseAddress,
					XSCL_FRCTLUMALEFT_OFFSET);
	LumaTopV = XScaler_ReadReg(ScalerInst.Config.BaseAddress,
					XSCL_FRCTLUMATOP_OFFSET);
	ChromaLeftH = XScaler_ReadReg(ScalerInst.Config.BaseAddress,
					XSCL_FRCTCHROMALEFT_OFFSET);
	ChromaTopV = XScaler_ReadReg(ScalerInst.Config.BaseAddress,
					XSCL_FRCTCHROMATOP_OFFSET);

	if(((LumaLeftH)==((u32)StartFractionInst.LumaLeftHori &
			XSCL_FRCTLUMALEFT_VALUE_MASK)) &&
			( LumaTopV == ((u32)StartFractionInst.LumaTopVert &
					XSCL_FRCTLUMATOP_VALUE_MASK)) &&
			(ChromaLeftH == ((u32)StartFractionInst.ChromaLeftHori &
					XSCL_FRCTCHROMALEFT_VALUE_MASK)) &&
			(ChromaTopV == ((u32)StartFractionInst.ChromaTopVert &
					XSCL_FRCTCHROMATOP_VALUE_MASK))) {
		check_status_update(u32, 1, 1);
	}
	else {
		check_status_update(u32, 0, 1);
	}

	/*
	 * Negative Test Case
	 */

	//XScaler_SetStartFraction(Null, Null1);

/*****************************************************************************/
	XScaler_GetStartFraction(&ScalerInst, &StartFractionInst);

	LumaLeftH = XScaler_ReadReg(ScalerInst.Config.BaseAddress,
					XSCL_FRCTLUMALEFT_OFFSET) &
						XSCL_FRCTLUMALEFT_VALUE_MASK;
	LumaTopV = XScaler_ReadReg(ScalerInst.Config.BaseAddress,
					XSCL_FRCTLUMATOP_OFFSET) &
						XSCL_FRCTLUMATOP_VALUE_MASK;
	ChromaLeftH = XScaler_ReadReg(ScalerInst.Config.BaseAddress,
					XSCL_FRCTCHROMALEFT_OFFSET) &
						XSCL_FRCTCHROMALEFT_VALUE_MASK;
	ChromaTopV = XScaler_ReadReg(ScalerInst.Config.BaseAddress,
						XSCL_FRCTCHROMATOP_OFFSET) &
						XSCL_FRCTCHROMATOP_VALUE_MASK;

	if ((LumaLeftH == StartFractionInst.LumaLeftHori) &&
		(LumaTopV == StartFractionInst.LumaTopVert) &&
			(ChromaLeftH == StartFractionInst.ChromaLeftHori) &&
			(ChromaTopV == StartFractionInst.ChromaTopVert)) {
		check_status_update(u32, 1, 1);
	}
	else
	{
		check_status_update(u32, 0, 1);
	}

	/*
	 * Negative Test Case
	 */

	//XScaler_GetStartFraction(Null, Null1);

/*****************************************************************************/
	 XScaler_GetCoeffBankSharingInfo(&ScalerInst,
						&ChromaFormat,
						&ChromaLumaShareCoeff,
						&HoriVertShareCoeff);

	 if (ChromaFormat == ScalerInst.Config.ChromaFormat) {
	 			check_status_update(u32, 1, 1);
	 	}
	 	else
	 	{
	 		check_status_update(u32, 0, 1);
	 	}
	 if (ScalerInst.Config.SeparateHvCoef !=0 ) {
		 if (HoriVertShareCoeff == 0) {
			 check_status_update(u32, 1, 1);
		 }
		 else
		 {
			 check_status_update(u32, 0, 1);
		 }
	 }
	 else {
		 if (HoriVertShareCoeff == 1) {
			 check_status_update(u32, 1, 1);
		 }
		 else
		 {
			 check_status_update(u32, 0, 1);
		 }
	 }
	 if (ChromaFormat ==XSCL_CHROMA_FORMAT_422 ) {
		 if(ScalerInst.Config.SeparateYcCoef!=0)
		 {
			 if (ChromaLumaShareCoeff == 0) {
				 check_status_update(u32, 1, 1);
			 }
			 else
			 {
				 check_status_update(u32, 0, 1);
			 }
		 }
	}
	 if(ChromaFormat == XSCL_CHROMA_FORMAT_422){
		 if (ChromaLumaShareCoeff == 1) {
			 check_status_update(u32, 1, 1);
		 }
		 else
		 {
			 check_status_update(u32, 0, 1);
		}
	 }

/*****************************************************************************/
	InLine_1= 23;
	OutSize_1 = 22;
	SrcSize_1 = 11 ;
	InPixel_1 = 63;

	 XScaler_CoefValueLookup(InLine_1, OutSize_1, SrcSize_1, InPixel_1);
	 if(Xil_AssertStatus == XIL_ASSERT_NONE) {
		 check_status_update(u32, 1, 1);
	 }
	 else{
		 check_status_update(u32, 0, 1);
	 }


/*****************************************************************************/
	/* Xil_AssertVoid(CoeffBankPtr->SetIndex <
				InstancePtr->Config.CoeffSetNum);
	Xil_AssertVoid(CoeffBankPtr->CoeffValueBuf != NULL);
	Xil_AssertVoid(CoeffBankPtr->PhaseNum >= (XSCL_MIN_PHASE_NUM));
	Xil_AssertVoid(CoeffBankPtr->PhaseNum <=
				InstancePtr->Config.MaxPhaseNum);
	Xil_AssertVoid(CoeffBankPtr->TapNum > 0U);
	Xil_AssertVoid(CoeffBankPtr->TapNum <= (XSCL_MAX_TAP_NUM)); */


	CoeffBankPtr.SetIndex = 1;
	ScalerInst.Config.CoeffSetNum = 2;
	CoeffBankPtr.PhaseNum = 6;
	ScalerInst.Config.MaxPhaseNum = 6;
	CoeffBankPtr.TapNum = (XSCL_MAX_TAP_NUM);
	CoeffBankPtr.CoeffValueBuf = XScaler_CoefValueLookup(640, 480,CoeffBankPtr.TapNum, CoeffBankPtr.PhaseNum);

	XScaler_LoadCoeffBank(&ScalerInst, &CoeffBankPtr);

	NewScalerReg = CoeffBankPtr.SetIndex & XSCL_COEFFSETADDR_ADDR_MASK;
	OldScalerReg = XScaler_ReadReg(ScalerInst.Config.BaseAddress,
					XSCL_COEFFSETADDR_OFFSET);

	if (NewScalerReg == OldScalerReg) {
		 check_status_update(u32, 1, 1);
	}
	else {
		check_status_update(u32, 0, 1);
	}

	/*
	 * Negative Test Case
	 */
	 //XScaler_LoadCoeffBank(Null, Null2);

/****************************************************************************/

	VertSetIndex = 0 ;
	ScalerInst.Config.CoeffSetNum = 1 ;
	HoriSetIndex = 0;
	ScalerInst.Config.CoeffSetNum = 1;

	XScaler_SetActiveCoeffSet(&ScalerInst, VertSetIndex, HoriSetIndex);

	OldScalerReg = ((u32)HoriSetIndex) & XSCL_COEFFSETS_HORI_MASK;
	OldScalerReg |= (((u32)VertSetIndex) << XSCL_COEFFSETS_VERT_SHIFT) &
				XSCL_COEFFSETS_VERT_MASK;

	NewScalerReg = XScaler_ReadReg(ScalerInst.Config.BaseAddress,
					XSCL_COEFFSETS_OFFSET);

	if (NewScalerReg == OldScalerReg) {
		 check_status_update(u32, 1, 1);
	}
	else {
		check_status_update(u32, 0, 1);
	}

	/*
	 * Negative Test Case
	 */
	//XScaler_SetActiveCoeffSet(Null, VertSetIndex, HoriSetIndex);

/*****************************************************************************/
	XScaler_GetActiveCoeffSet(&ScalerInst, &VertSetIndex, &HoriSetIndex);
	NewScalerReg = XScaler_ReadReg(ScalerInst.Config.BaseAddress,
						XSCL_COEFFSETS_OFFSET);
	if (((u8)(NewScalerReg & XSCL_COEFFSETS_VERT_MASK) >>
			XSCL_COEFFSETS_VERT_SHIFT) == VertSetIndex &&
			(u8)(NewScalerReg & XSCL_COEFFSETS_HORI_MASK) ==
			HoriSetIndex) {
		check_status_update(u32, 1, 1);
	} else {
		check_status_update(u32, 0, 1);
	}

/*****************************************************************************/
	memset((void *)&AperturePtr, 0, sizeof(XScalerAperture));
	memset((void *)&AperturePtr_1, 0, sizeof(XScalerAperture));
//	memset((void *)&ScalerInst, 0, sizeof(XScaler));
	AperturePtr.InFirstLine = 0;
	AperturePtr.InLastLine = 719;
	AperturePtr.InLastPixel = 1279;
	AperturePtr.InFirstPixel = 0;
	AperturePtr.OutVertSize = 1280;
	AperturePtr.OutHoriSize =720;

	XScaler_SetAperture(&ScalerInst, &AperturePtr);
	/* Calculate vertical and horizontal scale factors */
	VertScaleFactor = (double)(AperturePtr.InLastLine -
				AperturePtr.InFirstLine + 1.0);
	VertScaleFactor /= (double)(AperturePtr.OutVertSize);
	HoriScaleFactor = (double)(AperturePtr.InLastPixel -
				AperturePtr.InFirstPixel + 1.0);
	HoriScaleFactor /= (double)AperturePtr.OutHoriSize;

	/* Convert HoriScaleFactor and VertScaleFactor values into a format
	 * to write to HSF and VSF registers.
	 */
	VertScaleFactor = (u32)(VertScaleFactor * XSCL_SHRINK_FACTOR);
	HoriScaleFactor = (u32)(HoriScaleFactor * XSCL_SHRINK_FACTOR);

	/* Quantize Aperture - feed scale-factor back in to provide the
	 * actual aperture required to generate the desired number of output
	 * samples.
	 */
	QuantizedHoriSize = AperturePtr.OutHoriSize - 1;
	QuantizedHoriSize = (u32)(((double)QuantizedHoriSize *
				HoriScaleFactor) / XSCL_SHRINK_FACTOR);
	QuantizedHoriSize += 1 + (ScalerInst.Config.HoriTapNum + 1) / 2;

	QuantizedInLastPixel = AperturePtr.InFirstPixel + QuantizedHoriSize
	 			- 1;
	if (QuantizedInLastPixel > AperturePtr.InLastPixel)
		QuantizedInLastPixel = AperturePtr.InLastPixel;

	QuantizedVertSize = AperturePtr.OutVertSize - 1;
	QuantizedVertSize = (u32)(((float)QuantizedVertSize *
				VertScaleFactor) / XSCL_SHRINK_FACTOR);
	QuantizedVertSize += 1 + (ScalerInst.Config.VertTapNum + 1) / 2;

	QuantizedInLastLine = AperturePtr.InFirstLine + QuantizedVertSize - 1;
	if (QuantizedInLastLine > AperturePtr.InLastLine)
		QuantizedInLastLine = AperturePtr.InLastLine;

	/* Calculate input line, pixel and output size values */
	InLine = AperturePtr.InFirstLine & XSCL_APTVERT_FIRSTLINE_MASK;
	InLine |= (QuantizedInLastLine << XSCL_APTVERT_LASTLINE_SHIFT)
						& XSCL_APTVERT_LASTLINE_MASK;
	InPixel = AperturePtr.InFirstPixel & XSCL_APTHORI_FIRSTPXL_MASK;
	InPixel |= (QuantizedInLastPixel << XSCL_APTHORI_LASTPXL_SHIFT)
						& XSCL_APTHORI_LASTPXL_MASK;
	OutSize = AperturePtr.OutHoriSize & XSCL_OUTSIZE_NUMPXL_MASK;
	OutSize |= (AperturePtr.OutVertSize << XSCL_OUTSIZE_NUMLINE_SHIFT)
						& XSCL_OUTSIZE_NUMLINE_MASK;

	SrcSize = AperturePtr.SrcHoriSize & XSCL_SRCSIZE_NUMPXL_MASK;
	SrcSize |= (AperturePtr.SrcVertSize << XSCL_SRCSIZE_NUMLINE_SHIFT)
						& XSCL_SRCSIZE_NUMLINE_MASK;

	//InLine_1 = XScaler_ReadReg(ScalerInst.Config.BaseAddress,
	//				XSCL_FRCTLUMALEFT_OFFSET);
	InLine_1 = XScaler_ReadReg(ScalerInst.Config.BaseAddress,
					XSCL_APTVERT_OFFSET);
	InPixel_1 = XScaler_ReadReg(ScalerInst.Config.BaseAddress,
					XSCL_APTHORI_OFFSET);
	OutSize_1 =XScaler_ReadReg(ScalerInst.Config.BaseAddress,
					XSCL_OUTSIZE_OFFSET);
	SrcSize_1 =XScaler_ReadReg(ScalerInst.Config.BaseAddress,
					XSCL_SRCSIZE_OFFSET);
	HoriScaleFactor_1 =XScaler_ReadReg(ScalerInst.Config.BaseAddress,
					XSCL_HSF_OFFSET);
	HoriScaleFactor_1 = (u32)round(HoriScaleFactor_1);
	VertScaleFactor_1=XScaler_ReadReg(ScalerInst.Config.BaseAddress,
					XSCL_VSF_OFFSET);
	VertScaleFactor_1 = (u32)round(VertScaleFactor_1);
	/*if ((InLine == XScaler_ReadReg(ScalerInst.Config.BaseAddress,
		XSCL_FRCTLUMALEFT_OFFSET)) && (InPixel ==
		XScaler_ReadReg(ScalerInst.Config.BaseAddress,
			XSCL_APTHORI_OFFSET)) &&
		(OutSize == XScaler_ReadReg(ScalerInst.Config.BaseAddress,
			XSCL_OUTSIZE_OFFSET)) &&
		(SrcSize == XScaler_ReadReg(ScalerInst.Config.BaseAddress,
			XSCL_SRCSIZE_OFFSET)) &&
		((u32)(round(HoriScaleFactor)) ==
		XScaler_ReadReg(ScalerInst.Config.BaseAddress,
		XSCL_HSF_OFFSET)) &&
		((u32)(round(VertScaleFactor)) ==
		XScaler_ReadReg(ScalerInst.Config.BaseAddress,
		XSCL_VSF_OFFSET))) */
	if (InLine == InLine_1 && InPixel == InPixel_1 && OutSize == OutSize_1
		&& SrcSize== SrcSize_1 && HoriScaleFactor == HoriScaleFactor_1
			&& VertScaleFactor == VertScaleFactor_1) {
		check_status_update(u32, 1, 1);
	}
	else {
		check_status_update(u32, 0, 1);
	}

	/*
	 * Negative Test Case
	 */
	//XScaler_SetAperture(Null, Null3);

 /****************************************************************************/
	AperturePtr.InFirstLine = 0;
	AperturePtr.InLastLine = 719;
	AperturePtr.InLastPixel = 1279;
	AperturePtr.InFirstPixel = 0;
	AperturePtr.OutVertSize = 1280;
	AperturePtr.OutHoriSize = 720;

	XScaler_GetAperture(&ScalerInst, &AperturePtr);

	InLine = XScaler_ReadReg(ScalerInst.Config.BaseAddress,
			 (XSCL_APTVERT_OFFSET));
	InPixel = XScaler_ReadReg(ScalerInst.Config.BaseAddress,
			 (XSCL_APTHORI_OFFSET));
	OutSize = XScaler_ReadReg(ScalerInst.Config.BaseAddress,
			 (XSCL_OUTSIZE_OFFSET));

	/* Parse the info and populate the aperture structure */
	AperturePtr_1.InFirstLine = (InLine) & (XSCL_APTVERT_FIRSTLINE_MASK);
	AperturePtr_1.InLastLine = ((InLine) & (XSCL_APTVERT_LASTLINE_MASK)) >>
		 	(XSCL_APTVERT_LASTLINE_SHIFT);

	AperturePtr_1.InFirstPixel = (InPixel) & (XSCL_APTHORI_FIRSTPXL_MASK);
	AperturePtr_1.InLastPixel = ((InPixel) &
			(XSCL_APTHORI_LASTPXL_MASK)) >>
		 		(XSCL_APTHORI_LASTPXL_SHIFT);

	AperturePtr_1.OutHoriSize = (OutSize) & (XSCL_OUTSIZE_NUMPXL_MASK);
	AperturePtr_1.OutVertSize = ((OutSize) &
			(XSCL_OUTSIZE_NUMLINE_MASK)) >>
				(XSCL_OUTSIZE_NUMLINE_SHIFT);

	if(!(memcmp(&AperturePtr, &AperturePtr_1, sizeof(XScalerAperture)))) {
	 	check_status_update(u32, 1, 1);
	}
	else {
	 	check_status_update(u32, 0, 1);
	}

	/*
	 * Run XScaler self test.
	 */
//	Status = XScaler_SelfTest(&ScalerInst);
//	if (Status != XST_SUCCESS) {
//		check_status_update(u32, 0, 1);
//		return XST_FAILURE;
//	}
//	else{
//		check_status_update(u32, 1, 1);
//	}
	/*
	 * Test case for lookupconfig
	 */
	Config = XScaler_LookupConfig(XSCALAR_DEVICEID);
	if(Config == NULL) {
		check_status_update(u32, 1, 1);
	}
	else {
		check_status_update(u32, 0, 1);
	}


	CT_CMP_NUM(int, Status, XST_SUCCESS);
	}
	return (u32)(CT_GetTestFailures());

}
