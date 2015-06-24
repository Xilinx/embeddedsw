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
* @file xscaler_coefs.c
*
* This file contains Lanczos coefficient generation for use in the Xilinx Video Scaler.
*
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a xd   06/15/09 First release. Coefficients are auto-generated in
*                     Matlab using
*                     /Video_Scaler/reference_model/src/CreateCoefficients.m
* 2.00a xd   12/14/09 Updated doxygen document tags
* 5.00a mpv  12/13/13 Updated to dynamic coeff generation to reduce driver size
* 7.0   adk  08/22/14 Removed typedef unsigned short s16 as it was already
*                     defined in xil_types.h.
*                     Updated doxygen document tags.
*                     XScaler_coef_table is made as a global variable.
*                     Memory allocated was freed after usage.
* </pre>
*
******************************************************************************/

#include "xscaler.h"

/************************** Constant Definitions *****************************/
/** @name PI definition
 *  @{
 */
#define PI 3.14159265358979
/*@}*/

/************************* Data Structure Definitions ************************/

/**
* XScaler_CoefficientsBinScalingFactors contains scaling factors calculated
* using (Output_Size * 10000 / Input_Size). This table could help find
* the index of coefficient Bin given an input size and a output size.
*/
u16 XScaler_CoefficientBinScalingFactors[XSCL_NUM_COEF_BINS] = {
	20000,
	625,
	1250,
	1875,
	2500,
	3125,
	3750,
	4375,
	5000,
	5625,
	6250,
	6875,
	7500,
	8125,
	8750,
	9375,
	10000,
	10000,
	6666
};

s16* XScaler_coef_table = NULL; /**< Coefficients table */

/**
* This structure contains a pointer to double dimensional array which will be
* filled by coefficients.
*/
struct coefs_struct{
	short int** coefficients;
}SingleFrameCoefs;


/************************** Function Definitions ******************************/

/*****************************************************************************/
/**
* This function allocates memory to a double dimensional array.
*
* @param	Coefs is a pointer to Coefs_Struct structure which has a
*		double dimensional array which needs to be allocated memory.
* @param	Max_Taps indicates the number of taps which is used to allocate
*		that number of columns.
* @param	Max_Phases indicates the number of phases which is used to
*		allocate that number of rows.
*
* @return
*		1 - Indicates allocation of memory is failed.
*		0 - Indicates allocation of memory is success.
*
* @note		None.
*
******************************************************************************/
int XScaler_AllocCoefsBuff(struct coefs_struct* Coefs, u32 Max_Taps,
							u32 Max_Phases)
{
	u32 Phase;
	if ((Coefs->coefficients =
		  (s16 **) ((s16**)calloc(Max_Phases, sizeof(s16*)))) == 0) {
	  return(1);
	}
	for (Phase = 0; Phase < Max_Phases; Phase++) {
		if ((Coefs->coefficients[Phase] =
			(s16 *) ((s16*)calloc(Max_Taps, sizeof(s16)))) == 0) {
			return(1);
		}
	}
	return(0);
}

/*****************************************************************************/
/**
* This Sine generation algorithm implements Taylor series decomposition of the
* Sine function.
*
* @param	x is a float variable which needs to be set.
*
* @return	Returns Taylor series.
*
* @note		It works according to
*	http://en.wikipedia.org/wiki/Taylor_series#Approximation_and_convergence
*
*
******************************************************************************/
float XScaler_Sine(float x)
{
	int n, fac=1;
	float px, taylor=0;
	float rng = ((x-PI)/PI);
	x = x - rng*PI;
	px = x;
	for (n=0;n<6;n++)
	{
	taylor +=  (px / fac);
	px *= -(x*x);
	fac *= (2*n+2)*(2*n+3);
	}
	return taylor;
	}


/*****************************************************************************/
/**
* This function generates coffficient.
*
* @param	x is a float varaible based on which coefficient is
*		generated.
* @param	a is a 32 bit variable which holds half of the number of taps.
*
* @return	Returns Coefficient value depending on inputs.
*
* @note		It works according to
* http://en.wikipedia.org/wiki/Taylor_series#Approximation_and_convergence
*
*
******************************************************************************/
float XScaler_Lanczos(float x, int a)
{
	return((x<-a) ? 0 : ((x>a) ? 0 : ((x==0) ?
	1.0 : ( a*XScaler_Sine(PI*x)*XScaler_Sine(PI*x/a)/(PI*PI*x*x)))));

}
/*****************************************************************************/
/**
* This coefficient generation algorithm implements the Lanczos coefficients.
* For a particular scaling ratio, the coefficients can be pre-canned to memory.
*
* @param	p is a float variable.
* @param	icoeffs is a pointer to a single row of Coefficients array
*		which is in SingleFrameCoefs structure.
* @param	NCOEFF is variable which holds number of taps which indicates
*		the number of columns
*
* @return	None.
*
* @note		It works according to
*		http://en.wikipedia.org/wiki/Lanczos_resampling.
*
******************************************************************************/
void XScaler_GetLanczosCoeffs(float p, short int icoeffs[], int NCOEFF)
	{
	  float s=0;
	  float coeff[64];
	  int   i;

	  /* To implement convolution using the
	   * 2D FIR kernel, coefficient order is reversed
	   */
	  for (i=0; i<NCOEFF; i++) {
		  s+=(coeff[i] =
		  	XScaler_Lanczos( i-(NCOEFF>>1)+p, (NCOEFF>>1)));
	  }
	  /* Normalize coefficients, so sum()=1 for all phases.*/
	  for (i=0; i<NCOEFF; i++) {
		  icoeffs[i]=(int) (0.5+coeff[i]*16384/s);
	  }
	}
/*****************************************************************************/
/**
* This function generates a table that contains the coefficient values
* for scaling operations
*
* @param	Tap indicates the number of taps configured to the
*		Scaler device.
* @param	Phase indicates the number of phase configured to
*		the Scaler device.
*
* @return	The pointer to XScaler_Coef_Table.
*
* @note		None.
*
******************************************************************************/
s16 *XScaler_GenCoefTable(u32 Tap, u32 Phase)
{
	u32 i , j;
	s16 *current_phase_ptr;
	float dy;

	XScaler_AllocCoefsBuff(&SingleFrameCoefs, Tap, Phase);

	if (XScaler_coef_table != NULL) {
		free(XScaler_coef_table);
	}
	XScaler_coef_table = calloc(Tap*Phase, sizeof(s16));

	for (i = 0; i < Phase; i++) {
		dy = ((float) i)/ (float)Phase;
		current_phase_ptr = SingleFrameCoefs.coefficients[i];
		XScaler_GetLanczosCoeffs(dy,current_phase_ptr, Tap);
		for (j=0; j<Tap; j++){
			XScaler_coef_table[((i*Tap) + j)] =
					SingleFrameCoefs.coefficients[i][j];
		}
	}

	free(SingleFrameCoefs.coefficients);
	return (XScaler_coef_table);

}



