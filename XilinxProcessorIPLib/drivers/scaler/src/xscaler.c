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
/**
*
* @file xscaler.c
*
* This is main code of Xilinx MVI Video Scaler device driver. The Scaler device
* converts a specified rectangular area of an input digital video image from
* one original sampling grid to a desired target sampling grid. Please see
* xscaler.h for more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ----    -------- -------------------------------------------------------
* 1.00a xd      02/10/09 First release
* 2.00a xd      12/14/09 Updated Doxygen document tags
* 4.01a cw      06/27/12 Updated tcl file with new parameter names (num_x_taps)
*                        Updated mdd file with updated supported_peripherals
*                        field.
* 4.02a mpv     03/11/13 Updated the Driver to select the correct coeff bin.
*                        Changed RegValue variable to a volatile type
*                        Removed 10.x patch in the Tcl file
* 4.03a mpv     05/28/13 Fixed version limit in MDD file
*                        Updated the Driver input, output and aperture size
*                        mask
* 5.00a mpv     12/13/13 Updated to dynamic coeff generation to reduce driver
*                        size
* 7.0   adk     08/22/14 Modified prototype of XScaler_GetVersion API.
*                        and functionality of StubCallBack. Modified assert
*                        conditions in functions XScaler_CfgInitialize,
*                        XScaler_SetPhaseNum, XScaler_LoadCoeffBank.
*                        Removed error callback from XScaler_CfgInitialize
*                        function.
*                        Uncommented XScaler_Reset in XScaler_CfgInitialize
*                        function.
*                        Removed ErrorMask parameter in StubCallBack as there
*                        was only one interrupt.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xscaler.h"
#include "xenv.h"
#include "xil_io.h"
#include "xil_assert.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

/** @name Utility Macros
 *	@{
 */

/*****************************************************************************/
/**
*
* This macro calculates the integral value nearest to x rounding half-way cases
* away from zero, regardless of the current rounding direction.
*
* @param	x has a float type value
*
* @return	The integral value nearest to x rounding half-way cases away
*		from zero, regardless of the current rounding direction.
*
* @note		C-style signature: s32 round(float x);
*
******************************************************************************/
#define round(x) ((x) >= 0 ? (s32)((x) + 0.5) : (s32)((x) - 0.5))

/*@}*/

/************************** Function Prototypes ******************************/

static u32 XScaler_CoeffBinOffset(u32 InSize, u32 OutSize);
static void StubCallBack(void *CallBackRef);

/************************* Data Structure Definitions ************************/

/**
 * XScaler_CoefficientsBinScalingFactors contains scaling factors calculated
 * using (Output_Size * 10000 / Input_Size). This table could help find
 * the index of coefficient Bin given an input size and a output size.
 */
extern u16 XScaler_CoefficientBinScalingFactors[XSCL_NUM_COEF_BINS];

/**
 * XScaler_GenCoefTable generates XScaler_coef_table containing the coefficient 
 * values for scaling operations
 */

extern s16 *XScaler_GenCoefTable(u32 Tap, u32 Phase);

/************************** Function Definition ******************************/

/*****************************************************************************/
/**
 * This function initializes a Scaler device.  This function must be called
 * prior to using a Scaler device. Initialization of a Scaler includes setting
 * up the instance data, and ensuring the hardware is in a quiescent state.
 *
 * @param	InstancePtr is a pointer to the Scaler device instance to be
 *		worked on.
 * @param	CfgPtr points to the configuration structure associated with
 *		the Scaler device.
 * @param	EffectiveAddr is the base address of the device. If address
 *		translation is being used, then this parameter must
 *		reflect the virtual base address. Otherwise, the physical
 *		address should be used.
 *
 * @return	XST_SUCCESS
 *
 * @note	None.
 *
 *****************************************************************************/
int XScaler_CfgInitialize(XScaler *InstancePtr, XScaler_Config *CfgPtr,
			  u32 EffectiveAddr)
{

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);

	Xil_AssertNonvoid(CfgPtr->MaxPhaseNum <= XSCL_MAX_PHASE_NUM);

	Xil_AssertNonvoid(CfgPtr->HoriTapNum <= XSCL_MAX_TAP_NUM);

	Xil_AssertNonvoid(CfgPtr->VertTapNum <= XSCL_MAX_TAP_NUM);

	Xil_AssertNonvoid(CfgPtr->CoeffSetNum <= XSCL_MAX_COEFF_SET_NUM);

	Xil_AssertNonvoid(EffectiveAddr != (u32)NULL);

	/* Setup the instance */
	memset((void *)InstancePtr, 0, sizeof(XScaler));
	memcpy((void *)&(InstancePtr->Config), (const void *)CfgPtr,
				sizeof(XScaler_Config));
	InstancePtr->Config.BaseAddress = EffectiveAddr;

	/* Set all handlers to stub values, let user configure this data later
	 */
	InstancePtr->CallBack = (XScaler_CallBack)StubCallBack;

	/* Reset the hardware and set the flag to indicate the driver is ready
	 */

	XScaler_Reset(InstancePtr);
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This function sets up aperture of a Scaler device.  The aperture setting
 * consists of input video aperture and output video size. This function
 * calculates the scale factor accordingly based on the aperture setting and
 * sets up the Scaler appropriately.
 *
 * @param	InstancePtr is a pointer to the Scaler device instance to be
 *		worked on.
 * @param	AperturePtr points to the aperture setting structure to set up
 *		the Scaler device.
 *
 * @return	XST_SUCCESS.
 *
  * @note	None.
 *
 *****************************************************************************/
int XScaler_SetAperture(XScaler *InstancePtr, XScalerAperture *AperturePtr)
{
	double VertScaleFactor;
	double HoriScaleFactor;
	u32 InLine;
	u32 InPixel;
	u32 OutSize;
	u32 SrcSize;
	u32 QuantizedHoriSize;
	u32 QuantizedVertSize;
	u32 QuantizedInLastPixel;
	u32 QuantizedInLastLine;

	/* Assert bad arguments and conditions */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(AperturePtr != NULL);
	Xil_AssertNonvoid(AperturePtr->InFirstLine <= AperturePtr->InLastLine);
	Xil_AssertNonvoid(AperturePtr->InFirstPixel <= AperturePtr->InLastPixel);
	Xil_AssertNonvoid(AperturePtr->OutVertSize > 0);
	Xil_AssertNonvoid(AperturePtr->OutHoriSize > 0);

	/* Calculate vertical and horizontal scale factors */
	VertScaleFactor =
		(float)(AperturePtr->InLastLine - AperturePtr->InFirstLine
			+ 1);
	VertScaleFactor /=
		(float)(AperturePtr->OutVertSize);
	HoriScaleFactor =
		(float)(AperturePtr->InLastPixel - AperturePtr->InFirstPixel
			+ 1);
	HoriScaleFactor /=
		(float)AperturePtr->OutHoriSize;

	/* Convert HoriScaleFactor and VertScaleFactor values into a format
	 * to write to HSF and VSF registers.
	 */
	VertScaleFactor *= XSCL_SHRINK_FACTOR;
	HoriScaleFactor *= XSCL_SHRINK_FACTOR;

	/* Quantize Aperture - feed scale-factor back in to provide the
	 * actual aperture required to generate the desired number of output
	 * samples.
	 */
	QuantizedHoriSize = AperturePtr->OutHoriSize - 1;
	QuantizedHoriSize =
		(u32)(((float)QuantizedHoriSize * HoriScaleFactor) /
			XSCL_SHRINK_FACTOR);
	QuantizedHoriSize += 1 + (InstancePtr->Config.HoriTapNum + 1) / 2;

	QuantizedInLastPixel = AperturePtr->InFirstPixel + QuantizedHoriSize
				- 1;
	if (QuantizedInLastPixel > AperturePtr->InLastPixel)
		QuantizedInLastPixel = AperturePtr->InLastPixel;

	QuantizedVertSize = AperturePtr->OutVertSize - 1;
	QuantizedVertSize =
		(u32)(((float)QuantizedVertSize * VertScaleFactor) /
			XSCL_SHRINK_FACTOR);
	QuantizedVertSize += 1 + (InstancePtr->Config.VertTapNum + 1) / 2;

	QuantizedInLastLine = AperturePtr->InFirstLine + QuantizedVertSize - 1;
	if (QuantizedInLastLine > AperturePtr->InLastLine)
		QuantizedInLastLine = AperturePtr->InLastLine;

	/* Calculate input line, pixel and output size values */
	InLine = AperturePtr->InFirstLine & XSCL_APTVERT_FIRSTLINE_MASK;
	InLine |= (QuantizedInLastLine << XSCL_APTVERT_LASTLINE_SHIFT)
					& XSCL_APTVERT_LASTLINE_MASK;
	InPixel = AperturePtr->InFirstPixel & XSCL_APTHORI_FIRSTPXL_MASK;
	InPixel |= (QuantizedInLastPixel << XSCL_APTHORI_LASTPXL_SHIFT)
					& XSCL_APTHORI_LASTPXL_MASK;
	OutSize = AperturePtr->OutHoriSize & XSCL_OUTSIZE_NUMPXL_MASK;
	OutSize |= (AperturePtr->OutVertSize << XSCL_OUTSIZE_NUMLINE_SHIFT)
					& XSCL_OUTSIZE_NUMLINE_MASK;

	SrcSize = AperturePtr->SrcHoriSize & XSCL_SRCSIZE_NUMPXL_MASK;
	SrcSize |= (AperturePtr->SrcVertSize << XSCL_SRCSIZE_NUMLINE_SHIFT)
					& XSCL_SRCSIZE_NUMLINE_MASK;

	/* Set up aperture related register in the Scaler */
	XScaler_WriteReg(InstancePtr->Config.BaseAddress,
				(XSCL_APTVERT_OFFSET), InLine);
	XScaler_WriteReg(InstancePtr->Config.BaseAddress,
				(XSCL_APTHORI_OFFSET), InPixel);
	XScaler_WriteReg(InstancePtr->Config.BaseAddress,
				(XSCL_OUTSIZE_OFFSET), OutSize);
	XScaler_WriteReg(InstancePtr->Config.BaseAddress,
				(XSCL_SRCSIZE_OFFSET), SrcSize);

	XScaler_WriteReg(InstancePtr->Config.BaseAddress,
		(XSCL_HSF_OFFSET), (u32)(round(HoriScaleFactor)));

	XScaler_WriteReg(InstancePtr->Config.BaseAddress,
		(XSCL_VSF_OFFSET), (u32)(round(VertScaleFactor)));

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This function gets aperture of a Scaler device. The aperture setting
 * consists of input video aperture and output video size.
 *
 * @param	InstancePtr is a pointer to the Scaler device instance to be
 *		worked on.
 * @param	AperturePtr points to the aperture structure to store the
 *		current Scaler device setting after this function returns.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void XScaler_GetAperture(XScaler *InstancePtr, XScalerAperture *AperturePtr)
{
	u32 InLine;
	u32 InPixel;
	u32 OutSize;

	/* Assert bad arguments and conditions */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(AperturePtr != NULL);

	/* Read the first/last line and pixel info for input side and
	 * vertical/horizontal size for output size
	 */
	InLine = XScaler_ReadReg((InstancePtr)->Config.BaseAddress,
					XSCL_APTVERT);
	InPixel = XScaler_ReadReg((InstancePtr)->Config.BaseAddress,
					XSCL_APTHORI);
	OutSize = XScaler_ReadReg((InstancePtr)->Config.BaseAddress,
					XSCL_OUTSIZE);

	/* Parse the info and populate the aperture structure */
	AperturePtr->InFirstLine = InLine & XSCL_APTVERT_FIRSTLINE_MASK;
	AperturePtr->InLastLine =
		(InLine & XSCL_APTVERT_LASTLINE_MASK) >>
			XSCL_APTVERT_LASTLINE_SHIFT;

	AperturePtr->InFirstPixel = InPixel & XSCL_APTHORI_FIRSTPXL_MASK;
	AperturePtr->InLastPixel =
		(InPixel & XSCL_APTHORI_LASTPXL_MASK) >>
			XSCL_APTHORI_LASTPXL_SHIFT;

	AperturePtr->OutHoriSize = OutSize & XSCL_OUTSIZE_NUMPXL_MASK;
	AperturePtr->OutVertSize =
		(OutSize & XSCL_OUTSIZE_NUMLINE_MASK) >>
			XSCL_OUTSIZE_NUMLINE_SHIFT;

}

/*****************************************************************************/
/**
 * This function sets the numbers of vertical and horizontal phases to be used
 * by a Scaler device.
 *
 * @param	InstancePtr is a pointer to the Scaler device instance to be
 *		worked on.
 * @param	VertPhaseNum is the number of vertical phase to set to
 * @param	HoriPhaseNum is the number of horizontal phase to set to
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void XScaler_SetPhaseNum(XScaler *InstancePtr, u16 VertPhaseNum,
				u16 HoriPhaseNum)
{
	u32 PhaseRegValue;

	/* Assert bad arguments and conditions */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(VertPhaseNum <= InstancePtr->Config.MaxPhaseNum);
	Xil_AssertVoid(HoriPhaseNum <= InstancePtr->Config.MaxPhaseNum);

	/* Calculate the value to write to "Number of Phases Register" */
	PhaseRegValue =
		(VertPhaseNum << XSCL_NUMPHASE_VERT_SHIFT) &
			XSCL_NUMPHASE_VERT_MASK;
	PhaseRegValue |= HoriPhaseNum & XSCL_NUMPHASE_HORI_MASK;

	/* Set up the Scaler core using the numbers of phases */
	XScaler_WriteReg(InstancePtr->Config.BaseAddress,
		(XSCL_NUMPHASE_OFFSET), PhaseRegValue);
}

/*****************************************************************************/
/**
 * This function gets the numbers of vertical and horizontal phases currently
 * used by a Scaler device.
 *
 * @param	InstancePtr is a pointer to the Scaler device instance to be
 *		worked on.
 * @param	VertPhaseNumPtr will point to the number of vertical phases
 *		used after this function returns.
 * @param	HoriPhaseNumPtr will point to the number of horizontal phases
 *		used after this function returns.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void XScaler_GetPhaseNum(XScaler *InstancePtr, u16 *VertPhaseNumPtr,
				u16 *HoriPhaseNumPtr)
{
	u32 PhaseRegValue;

	/* Assert bad arguments and conditions */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(VertPhaseNumPtr != NULL);
	Xil_AssertVoid(HoriPhaseNumPtr != NULL);

	/* Get the value of "Number of Phases Register" */
	PhaseRegValue = XScaler_ReadReg(InstancePtr->Config.BaseAddress,
				(XSCL_NUMPHASE_OFFSET));

	/* Parse the value and store the results */
	*VertPhaseNumPtr =
		(PhaseRegValue & XSCL_NUMPHASE_VERT_MASK) >>
			XSCL_NUMPHASE_VERT_SHIFT;
	*HoriPhaseNumPtr = PhaseRegValue & XSCL_NUMPHASE_HORI_MASK;

}

/*****************************************************************************/
/**
 * This function sets up Luma and Chroma start fractional values used by a
 * Scaler device.
 *
 * @param	InstancePtr is a pointer to the Scaler device instance to be
 *		worked on.
 * @param	StartFractionPtr is a pointer to a start fractional value set
 *		to be used.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void XScaler_SetStartFraction(XScaler *InstancePtr,
				  XScalerStartFraction *StartFractionPtr)
{
	/* Assert bad arguments and conditions */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(StartFractionPtr != NULL);

	/* Set up the fractional values */
	XScaler_WriteReg((InstancePtr)->Config.BaseAddress,
				 XSCL_FRCTLUMALEFT_OFFSET,
			 (u32)StartFractionPtr->LumaLeftHori &
			 XSCL_FRCTLUMALEFT_VALUE_MASK);

	XScaler_WriteReg((InstancePtr)->Config.BaseAddress,
			 XSCL_FRCTLUMATOP_OFFSET,
			 (u32)StartFractionPtr->LumaTopVert &
			 XSCL_FRCTLUMATOP_VALUE_MASK);

	XScaler_WriteReg((InstancePtr)->Config.BaseAddress,
			 XSCL_FRCTCHROMALEFT,
			 (u32)StartFractionPtr->ChromaLeftHori &
			 XSCL_FRCTCHROMALEFT_VALUE_MASK);

	XScaler_WriteReg((InstancePtr)->Config.BaseAddress,
				XSCL_FRCTCHROMATOP_OFFSET,
			 (u32)StartFractionPtr->ChromaTopVert &
			 XSCL_FRCTCHROMATOP_VALUE_MASK);

}

/*****************************************************************************/
/**
 * This function gets Luma and Chroma start fractional values currently used
 * by a Scaler device.
 *
 * @param	InstancePtr is a pointer to the Scaler device instance to be
 *		worked on.
 * @param	StartFractionPtr is a pointer to a start fractional value
 *		structure to be populated with the fractional values after this
 *		function returns.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void XScaler_GetStartFraction(XScaler *InstancePtr,
				  XScalerStartFraction *StartFractionPtr)
{
	/* Assert bad arguments and conditions */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(StartFractionPtr != NULL);

	/* Fetch the fractional values */
	StartFractionPtr->LumaLeftHori = (s32)
		XScaler_ReadReg((InstancePtr)->Config.BaseAddress,
			XSCL_FRCTLUMALEFT_OFFSET)
			& XSCL_FRCTLUMALEFT_VALUE_MASK;

	StartFractionPtr->LumaTopVert = (s32)
		XScaler_ReadReg((InstancePtr)->Config.BaseAddress,
			XSCL_FRCTLUMATOP_OFFSET)
			& XSCL_FRCTLUMATOP_VALUE_MASK;

	StartFractionPtr->ChromaLeftHori = (s32)
		XScaler_ReadReg((InstancePtr)->Config.BaseAddress,
			XSCL_FRCTCHROMALEFT_OFFSET)
			& XSCL_FRCTCHROMALEFT_VALUE_MASK;

	StartFractionPtr->ChromaTopVert = (s32)
		XScaler_ReadReg((InstancePtr)->Config.BaseAddress,
			XSCL_FRCTCHROMATOP_OFFSET)
			& XSCL_FRCTCHROMATOP_VALUE_MASK;

}

/*****************************************************************************/
/**
 * This function fetches the color space format and coefficient bank sharing
 * decisions made on a Scaler device at build-time.
 *
 * @param	InstancePtr is a pointer to the Scaler device instance to be
 *		worked on.
 * @param	ChromaFormat points to an 8-bit variable that will be assigned
 *		with the Chroma format chosen for the Scaler device at the
 *		build time after this function returns. Please use
 *		XSCL_CHROMA_FORMAT_* defined in xscaler_hw.h to interpret the
 *		variable value.
 * @param	ChromaLumaShareCoeff points to an 8-bit variable that will be
 *		assigned by this function with the decision value on coefficient
 *		bank sharing between Chroma and Luma filter operations. The
 *		decision is made for the Scaler device at build time and can
 *		NOT be changed at run-time. Value 0 indicates that each of Chroma
 *		and Luma filter operations has its own coefficient bank. Value
 *		1 indicates that Chroma and Luma filter operations share one
 *		common coefficient bank.
 * @param	HoriVertShareCoeff points to an 8-bit variable that will be
 *		assigned by this function with the decision value on coefficient
 *		bank sharing between Horizontal and Vertical filter operations.
 *		The decision is made for the Scaler device at build time and
 *		can NOT be changed at run-time. Value 0 indicates that each of
 *		Horizontal and Vertical filter operations has its own
 *		coefficient bank. Value 1 indicates that Horizontal and
 *		Vertical filter operations share one common coefficient bank.
 *
 * @return	None.
 *
 * @note
 *
 * !!!IMPORTANT!!!
 *
 * The application of this function is responsible for loading the correct
 * number of coefficient banks in the proper sequence order. The number of
 * coefficient banks to load and the proper loading sequence totally depends
 * on the values of the output parameters of this function. Please use the
 * table below as reference.
 *
 * <pre>
 * ChromaFormat ChromaLumaShareCoeff HoriVertShareCoeff # of      sequence
 *                                                      coeff     of
 *                                                      banks     loading
 *                                                      to load   coeff
 *                                                                banks
 * ------------ -------------------- ------------------ --------  -------------
 * YUV420       1                    1                  1         1.Single bank
 * ------------ -------------------- ------------------ --------  -------------
 * YUV420       1                    0                  2         1.Hori bank
 *                                                                2.Vert bank
 * ------------ -------------------- ------------------ --------  -------------
 * YUV420       0                    1                  2         1.Luma bank
 *                                                                2.Chroma bank
 * ------------ -------------------- ------------------ --------  -------------
 * YUV420       0                    0                  4         1.Hori Luma
 *                                                                2.Hori Chroma
 *                                                                3.Vert Luma
 *                                                                4.Vert Chroma
 * ------------ -------------------- ------------------ --------  -------------
 * YUV422       1                    1                  1         1.Single bank
 * ------------ -------------------- ------------------ --------  -------------
 * YUV422       1                    0                  2         1.Hori bank
 *                                                                2.Vert bank
 * ------------ -------------------- ------------------ --------  -------------
 * YUV422       0                    1                  2         1.Luma bank
 *                                                                2.Chroma bank
 * ------------ -------------------- ------------------ --------  -------------
 * YUV422       0                    0                  4         1.Hori Luma
 *                                                                2.Hori Chroma
 *                                                                3.Vert Luma
 *                                                                4.Vert Chroma
 * ------------ -------------------- ------------------ --------  -------------
 * YUV444       Always 1             1                  1         1.Single bank
 * ------------ -------------------- ------------------ --------  -------------
 * YUV444       Always 1             0                  2         1.Hori bank
 *                                                                2.Vert bank
 * ------------ -------------------- ------------------ --------  -------------
 * </pre>
 *
 *****************************************************************************/
void XScaler_GetCoeffBankSharingInfo(XScaler *InstancePtr,
					u8 *ChromaFormat,
					u8 *ChromaLumaShareCoeff,
					u8 *HoriVertShareCoeff)
{
	/* Assert bad arguments and conditions */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(ChromaFormat != NULL);
	Xil_AssertVoid(ChromaLumaShareCoeff != NULL);
	Xil_AssertVoid(HoriVertShareCoeff != NULL);

	/* Output the Chroma format info */
	*ChromaFormat = InstancePtr->Config.ChromaFormat;

	/* Output the Coefficient bank sharing info between Horizontal and
	 * Vertical filter operations */
	if (InstancePtr->Config.SeparateHvCoef)
		*HoriVertShareCoeff = 0;
	else
		*HoriVertShareCoeff = 1;

	/* Output the Coefficient bank sharing info between Chroma and
	 * Luma filter operations */
	switch (*ChromaFormat) {

	case XSCL_CHROMA_FORMAT_420:
	case XSCL_CHROMA_FORMAT_422:
		if (InstancePtr->Config.SeparateYcCoef)
			*ChromaLumaShareCoeff = 0;
		else
			*ChromaLumaShareCoeff = 1;
		break;
	case XSCL_CHROMA_FORMAT_444:
		*ChromaLumaShareCoeff = 1;
		break;
	}

}

/*****************************************************************************/
/**
 * This function returns the pointer to the coefficients for a scaling
 * operation given input/output sizes and the Tap and Phase numbers.
 *
 * @param	InSize indicates the size (width or height) of the input video.
 * @param	OutSize indicates the size (width or height) of the output
 *		video.
 * @param	Tap indicates the Tap number.
 * @param	Phase indicates the Phase number.
 *
 * @return	The points to the coefficients ready for the scaling operation.
 *
 * @note	None.
 *
 *****************************************************************************/
s16 *XScaler_CoefValueLookup(u32 InSize, u32 OutSize, u32 Tap, u32 Phase)
{
	u32 CoeffBinIndex;
	u32 CoeffValueOffset;

	/* Validate the input parameters */
	Xil_AssertNonvoid(InSize > 0);
	Xil_AssertNonvoid(OutSize > 0);
	Xil_AssertNonvoid(Tap >= XSCL_MIN_TAP_NUM);
	Xil_AssertNonvoid(Tap <= XSCL_MAX_TAP_NUM);
	Xil_AssertNonvoid(Phase >= XSCL_MIN_PHASE_NUM);
	Xil_AssertNonvoid(Phase <= XSCL_MAX_PHASE_NUM);

	/* Find the index of the Coefficient Bin */
	CoeffBinIndex = XScaler_CoeffBinOffset(InSize, OutSize);

	/* Find the offset of the Coefficients within the Bin */
	CoeffValueOffset = XScaler_CoefTapOffset(Tap);
	CoeffValueOffset += XScaler_CoefPhaseOffset(Tap, Phase);

	return (XScaler_GenCoefTable(Tap,Phase));
}

/*****************************************************************************/
/**
 * This function loads a coefficient bank to the Scaler core. A complete
 * coefficient set contains 4 banks (if Luma, Chroma, Horizontal and Vertical
 * filter operations do not share common banks. For more details see
 * XScaler_GetCoeffBankSharingInfo()): Horizontal Luma, Horizontal Chroma,
 * Vertical Luma and Vertical Chroma. all 4 banks must be loaded back to back
 * in the order listed here. The caller is responsible for ensuring the
 * sequence and this function does not check it.
 *
 * An example sequence to load an whole coefficient set is like:
 * <pre>
 *	   XScaler_LoadCoeffBank(&Scaler, &HoriLumaCoeffBank);
 *	   XScaler_LoadCoeffBank(&Scaler, &HoriChromaCoeffBank);
 *	   XScaler_LoadCoeffBank(&Scaler, &VertLumaCoeffBank);
 *	   XScaler_LoadCoeffBank(&Scaler, &VertChromaCoeffBank);
 * </pre>
 *
 * @param	InstancePtr is a pointer to the Scaler device instance to be
 *		worked on.
 * @param	CoeffBankPtr is a pointer to a coefficient bank that is to be
 *		loaded.
 *
 * @return	None.
 *
 *****************************************************************************/
void XScaler_LoadCoeffBank(XScaler *InstancePtr,
				XScalerCoeffBank *CoeffBankPtr)
{
	int PhaseIndex;
	int TapIndex;
	s16 *CoeffValueTapBase;
	u32 CoeffValue;

	/* Assert bad arguments and conditions */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(CoeffBankPtr != NULL);
	Xil_AssertVoid(CoeffBankPtr->SetIndex < InstancePtr->Config.CoeffSetNum);
	Xil_AssertVoid(CoeffBankPtr->CoeffValueBuf != NULL);

	Xil_AssertVoid(CoeffBankPtr->PhaseNum <=
			InstancePtr->Config.MaxPhaseNum);

	Xil_AssertVoid(CoeffBankPtr->TapNum <= XSCL_MAX_TAP_NUM);

	/* Start the coefficient bank loading by writing the bank index first
	 */
	XScaler_WriteReg(InstancePtr->Config.BaseAddress,
		(XSCL_COEFFSETADDR_OFFSET), ((CoeffBankPtr->SetIndex) &
			(XSCL_COEFFSETADDR_ADDR_MASK)));

	/* Now load the valid values */
	CoeffValueTapBase = CoeffBankPtr->CoeffValueBuf;
	for (PhaseIndex = 0;
		PhaseIndex < CoeffBankPtr->PhaseNum; PhaseIndex++) {
		for (TapIndex = 0; TapIndex < CoeffBankPtr->TapNum;) {
			CoeffValue =  ((u32)CoeffValueTapBase[TapIndex++]) &
					0xFFFF;
			if (TapIndex < CoeffBankPtr->TapNum) {
				CoeffValue |=
					(((u32)CoeffValueTapBase[TapIndex++]) &
						0xFFFF)	<< 16;
			}
			XScaler_WriteReg(InstancePtr->Config.BaseAddress,
				(XSCL_COEFFVALUE_OFFSET), CoeffValue);
		}
		CoeffValueTapBase +=  CoeffBankPtr->TapNum;
	}

	/*
	 * Load padding if the real phase number is less than the maximum phase
	 * number
	 */
	for (PhaseIndex = CoeffBankPtr->PhaseNum;
		PhaseIndex < InstancePtr->Config.MaxPhaseNum; PhaseIndex++) {
		for (TapIndex = 0; TapIndex < (CoeffBankPtr->TapNum + 1) / 2;
				TapIndex++) {
			XScaler_WriteReg((InstancePtr)->Config.BaseAddress,
					XSCL_COEFFVALUE_OFFSET, 0);
		}
	}

}

/*****************************************************************************/
/**
*
* This function chooses the active vertical and horizontal coefficient sets to
* be used by a Scaler device.
*
* Each coefficient set contains 4 banks: Horizontal Luma, Horizontal Chroma,
* Vertical Luma and Vertical Chroma. The horizontal part is independent from
* the vertical part and the Scaler device supports using the horizontal part
* of one coefficient set w/ the vertical part of a different coefficient set.
*
* @param	InstancePtr is a pointer to the Scaler device instance to be
*		worked on.
* @param	VertSetIndex indicates the index of the coefficient set in which
*		the vertical part will be used by the Scaler device. Valid value
*		is from 0 to (the number of the coefficient sets implemented by
*		the Scaler device - 1).
* @param	HoriSetIndex indicates the index of the coefficient set in which
*		the horizontal part will be used by the Scaler device. Valid
*		value is from 0 to (the number of the coefficient sets
*		implemented by the Scaler device - 1).
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XScaler_SetActiveCoeffSet(XScaler *InstancePtr,
				   u8 VertSetIndex,
				   u8 HoriSetIndex)
{
	volatile u32 RegValue;

	/* Assert bad arguments and conditions */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(VertSetIndex < InstancePtr->Config.CoeffSetNum);
	Xil_AssertVoid(HoriSetIndex < InstancePtr->Config.CoeffSetNum);

	RegValue = ((u32)HoriSetIndex) & XSCL_COEFFSETS_HORI_MASK;
	RegValue |= (((u32)VertSetIndex) << XSCL_COEFFSETS_VERT_SHIFT) &
			XSCL_COEFFSETS_VERT_MASK;

	XScaler_WriteReg((InstancePtr)->Config.BaseAddress,
				XSCL_COEFFSETS_OFFSET, RegValue);

}

/*****************************************************************************/
/**
*
* This function fetches the indexes of active vertical and horizontal
* coefficient sets being used by a Scaler device.
*
* Each coefficient set contains 4 banks: Horizontal Luma, Horizontal Chroma,
* Vertical Luma and Vertical Chroma. The horizontal part is independent from
* the vertical part and the Scaler device supports using the horizontal part
* of one coefficient set w/ the vertical part of a different coefficient set.
*
* @param	InstancePtr is a pointer to the Scaler device instance to be
*		worked on.
* @param	VertSetIndexPtr points to the index of the active coefficient
*		set in which the vertical part is being used by the Scaler
*		device after this function returns.
* @param	HoriSetIndexPtr points to the index of the active coefficient
*		set in which the horizontal part is being used by the Scaler
*		device after this function returns.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XScaler_GetActiveCoeffSet(XScaler *InstancePtr,
				u8 *VertSetIndexPtr,
				u8 *HoriSetIndexPtr)
{
	u32 RegValue;

	/* Assert bad arguments and conditions */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(VertSetIndexPtr != NULL);
	Xil_AssertVoid(HoriSetIndexPtr != NULL);

	RegValue = XScaler_ReadReg((InstancePtr)->Config.BaseAddress,
				   XSCL_COEFFSETS);

	*VertSetIndexPtr = (u8)
		((RegValue & XSCL_COEFFSETS_VERT_MASK) >>
			XSCL_COEFFSETS_VERT_SHIFT);

	*HoriSetIndexPtr = (u8)(RegValue & XSCL_COEFFSETS_HORI_MASK);

}

/*****************************************************************************/
/**
 * This function calculates the index of the coefficient Bin to use based on
 * the input and output video size (Width or Height)
 *
 * @param	InSize indicates the size (width or height) of the input video.
 * @param	OutSize indicates the size (width or height) of the output
 *		video.
 *
 * @return	The index of the coefficient Bin.
 *
 *****************************************************************************/
static u32 XScaler_CoeffBinOffset(u32 InSize, u32 OutSize)
{
	u32 CoeffBinIndex;

	/* Validate the input parameters */
	Xil_AssertNonvoid(InSize > 0);
	Xil_AssertNonvoid(OutSize > 0);

	if (OutSize > InSize)
		CoeffBinIndex = 0;
	else
	  CoeffBinIndex = 1 + (OutSize * 16 / InSize);

	return CoeffBinIndex;
}

/*****************************************************************************/
/**
*
* This function returns the contents of version register of the Scaler core.
*
* @param	InstancePtr is a pointer to the Scaler core instance to be
*		worked on.
*
* @return	Contents of the version register.
*
* @note		None.
*
******************************************************************************/
u32 XScaler_GetVersion(XScaler *InstancePtr)
{
	u32 Data;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	Data = XScaler_ReadReg(InstancePtr->Config.BaseAddress,
				(XSCL_VER_OFFSET));

	return Data;
}

/*****************************************************************************/
/*
*
* This routine is a stub for the frame done interrupt callback. The stub is
* here in case the upper layer forgot to set the callback. On initialization,
* the frame done interrupt callback is set to this stub. It is considered an
* error for this function to be invoked.
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

	(void)CallBackRef;
	Xil_AssertVoidAlways();
}
