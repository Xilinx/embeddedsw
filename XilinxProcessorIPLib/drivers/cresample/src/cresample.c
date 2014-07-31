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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file cresample.c
*
* This is main code of Xilinx Chroma Resampler (CRESAMPLE)
* device driver. Please see cresample.h for more details of the driver.
*
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- ----     -------- -------------------------------------------------------
* 1.00a gaborz   08/04/11  Updated for CRESAMPLE V1.0
* 2.00a vyc      04/24/12  Updated for CRESAMPLE V2.00.a
* 2.00a vyc      07/25/12  Switched from Xuint32 to u32
* 2.00a vyc      10/16/12  Switch order of functions to remove compile warning
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "cresample.h"
#include "xenv.h"

/*****************************************************************************/
// Note: Many functions are currently implemented as high-performance macros
// within cresample.h
/*****************************************************************************/

void clear_coef_values(u32 BaseAddress)
{
  // set all coefficient values to 0
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF00_HPHASE0, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF01_HPHASE0, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF02_HPHASE0, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF03_HPHASE0, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF04_HPHASE0, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF05_HPHASE0, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF06_HPHASE0, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF07_HPHASE0, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF08_HPHASE0, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF09_HPHASE0, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF10_HPHASE0, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF11_HPHASE0, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF12_HPHASE0, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF13_HPHASE0, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF14_HPHASE0, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF15_HPHASE0, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF16_HPHASE0, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF17_HPHASE0, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF18_HPHASE0, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF19_HPHASE0, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF20_HPHASE0, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF21_HPHASE0, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF22_HPHASE0, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF23_HPHASE0, 0);

	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF00_HPHASE1, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF01_HPHASE1, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF02_HPHASE1, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF03_HPHASE1, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF04_HPHASE1, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF05_HPHASE1, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF06_HPHASE1, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF07_HPHASE1, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF08_HPHASE1, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF09_HPHASE1, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF10_HPHASE1, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF11_HPHASE1, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF12_HPHASE1, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF13_HPHASE1, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF14_HPHASE1, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF15_HPHASE1, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF16_HPHASE1, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF17_HPHASE1, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF18_HPHASE1, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF19_HPHASE1, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF20_HPHASE1, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF21_HPHASE1, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF22_HPHASE1, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF23_HPHASE1, 0);

	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF00_VPHASE0, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF01_VPHASE0, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF02_VPHASE0, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF03_VPHASE0, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF04_VPHASE0, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF05_VPHASE0, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF06_VPHASE0, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF07_VPHASE0, 0);

	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF00_VPHASE1, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF01_VPHASE1, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF02_VPHASE1, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF03_VPHASE1, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF04_VPHASE1, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF05_VPHASE1, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF06_VPHASE1, 0);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF07_VPHASE1, 0);
}

void configure_444_to_422(u32 BaseAddress, int NUM_H_TAPS, int *coef_vector  )
{   
  // clear out existing coefficient register values
  clear_coef_values(BaseAddress);
  
  // set new coefficient register values from coef_vector  
  if (NUM_H_TAPS > 0) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF00_HPHASE0, coef_vector[0]);
	}
  if (NUM_H_TAPS > 1) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF01_HPHASE0, coef_vector[1]);
	}
  if (NUM_H_TAPS > 2) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF02_HPHASE0, coef_vector[2]);
	}
  if (NUM_H_TAPS > 3) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF03_HPHASE0, coef_vector[3]);
	}
  if (NUM_H_TAPS > 4) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF04_HPHASE0, coef_vector[4]);
	}
  if (NUM_H_TAPS > 5) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF05_HPHASE0, coef_vector[5]);
	}
  if (NUM_H_TAPS > 6) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF06_HPHASE0, coef_vector[6]);
	}
  if (NUM_H_TAPS > 7) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF07_HPHASE0, coef_vector[7]);
	}
  if (NUM_H_TAPS > 8) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF08_HPHASE0, coef_vector[8]);
	}
  if (NUM_H_TAPS > 9) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF09_HPHASE0, coef_vector[9]);
	}
  if (NUM_H_TAPS > 10) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF10_HPHASE0, coef_vector[10]);
	}
  if (NUM_H_TAPS > 11) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF11_HPHASE0, coef_vector[11]);
	}
  if (NUM_H_TAPS > 12) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF12_HPHASE0, coef_vector[12]);
	}
  if (NUM_H_TAPS > 13) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF13_HPHASE0, coef_vector[13]);
	}
  if (NUM_H_TAPS > 14) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF14_HPHASE0, coef_vector[14]);
	}
  if (NUM_H_TAPS > 15) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF15_HPHASE0, coef_vector[15]);
	}
  if (NUM_H_TAPS > 16) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF16_HPHASE0, coef_vector[16]);
	}
  if (NUM_H_TAPS > 17) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF17_HPHASE0, coef_vector[17]);
	}
  if (NUM_H_TAPS > 18) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF18_HPHASE0, coef_vector[18]);
	}
  if (NUM_H_TAPS > 19) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF19_HPHASE0, coef_vector[19]);
	}
  if (NUM_H_TAPS > 20) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF20_HPHASE0, coef_vector[20]);
	}
  if (NUM_H_TAPS > 21) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF21_HPHASE0, coef_vector[21]);
	}
  if (NUM_H_TAPS > 22) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF22_HPHASE0, coef_vector[22]);
	}
  if (NUM_H_TAPS > 23) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF23_HPHASE0, coef_vector[23]);
	}
}

void configure_422_to_444(u32 BaseAddress, int NUM_H_TAPS, int *coef_phase1_vector  )
{
  // clear out existing coefficient register values
  clear_coef_values(BaseAddress);

  // set new coefficient register values from coef_vector
  if (NUM_H_TAPS > 0) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF00_HPHASE1, coef_phase1_vector[0]);
	}
  if (NUM_H_TAPS > 1) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF01_HPHASE1, coef_phase1_vector[1]);
	}
  if (NUM_H_TAPS > 2) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF02_HPHASE1, coef_phase1_vector[2]);
	}
  if (NUM_H_TAPS > 3) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF03_HPHASE1, coef_phase1_vector[3]);
	}
  if (NUM_H_TAPS > 4) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF04_HPHASE1, coef_phase1_vector[4]);
	}
  if (NUM_H_TAPS > 5) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF05_HPHASE1, coef_phase1_vector[5]);
	}
  if (NUM_H_TAPS > 6) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF06_HPHASE1, coef_phase1_vector[6]);
	}
  if (NUM_H_TAPS > 7) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF07_HPHASE1, coef_phase1_vector[7]);
	}
  if (NUM_H_TAPS > 8) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF08_HPHASE1, coef_phase1_vector[8]);
	}
  if (NUM_H_TAPS > 9) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF09_HPHASE1, coef_phase1_vector[9]);
	}
  if (NUM_H_TAPS > 10) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF10_HPHASE1, coef_phase1_vector[10]);
	}
  if (NUM_H_TAPS > 11) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF11_HPHASE1, coef_phase1_vector[11]);
	}
  if (NUM_H_TAPS > 12) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF12_HPHASE1, coef_phase1_vector[12]);
	}
  if (NUM_H_TAPS > 13) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF13_HPHASE1, coef_phase1_vector[13]);
	}
  if (NUM_H_TAPS > 14) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF14_HPHASE1, coef_phase1_vector[14]);
	}
  if (NUM_H_TAPS > 15) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF15_HPHASE1, coef_phase1_vector[15]);
	}
  if (NUM_H_TAPS > 16) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF16_HPHASE1, coef_phase1_vector[16]);
	}
  if (NUM_H_TAPS > 17) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF17_HPHASE1, coef_phase1_vector[17]);
	}
  if (NUM_H_TAPS > 18) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF18_HPHASE1, coef_phase1_vector[18]);
	}
  if (NUM_H_TAPS > 19) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF19_HPHASE1, coef_phase1_vector[19]);
	}
  if (NUM_H_TAPS > 20) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF20_HPHASE1, coef_phase1_vector[20]);
	}
  if (NUM_H_TAPS > 21) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF21_HPHASE1, coef_phase1_vector[21]);
	}
  if (NUM_H_TAPS > 22) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF22_HPHASE1, coef_phase1_vector[22]);
	}
  if (NUM_H_TAPS > 23) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF23_HPHASE1, coef_phase1_vector[23]);
	}
}

void configure_422_to_420(u32 BaseAddress, int NUM_V_TAPS, int *coef_vector  )
{
  // clear out existing coefficient register values
  clear_coef_values(BaseAddress);

  // set new coefficient register values from coef_vector
  if (NUM_V_TAPS > 0) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF00_VPHASE0, coef_vector[0]);
	}
  if (NUM_V_TAPS > 1) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF01_VPHASE0, coef_vector[1]);
	}
  if (NUM_V_TAPS > 2) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF02_VPHASE0, coef_vector[2]);
	}
  if (NUM_V_TAPS > 3) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF03_VPHASE0, coef_vector[3]);
	}
  if (NUM_V_TAPS > 4) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF04_VPHASE0, coef_vector[4]);
	}
  if (NUM_V_TAPS > 5) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF05_VPHASE0, coef_vector[5]);
	}
  if (NUM_V_TAPS > 6) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF06_VPHASE0, coef_vector[6]);
	}
  if (NUM_V_TAPS > 7) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF07_VPHASE0, coef_vector[7]);
	}
}

void configure_420_to_422(u32 BaseAddress, int NUM_V_TAPS, int *coef_phase0_vector, int *coef_phase1_vector   )
{
  // clear out existing coefficient register values
  clear_coef_values(BaseAddress);

  // set new coefficient register values from coef_vector
  if (NUM_V_TAPS > 0) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF00_VPHASE0, coef_phase0_vector[0]);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF00_VPHASE1, coef_phase1_vector[0]);
	}
  if (NUM_V_TAPS > 1) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF01_VPHASE0, coef_phase0_vector[1]);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF01_VPHASE1, coef_phase1_vector[1]);
	}
  if (NUM_V_TAPS > 2) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF02_VPHASE0, coef_phase0_vector[2]);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF02_VPHASE1, coef_phase1_vector[2]);
	}
  if (NUM_V_TAPS > 3) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF03_VPHASE0, coef_phase0_vector[3]);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF03_VPHASE1, coef_phase1_vector[3]);
	}
  if (NUM_V_TAPS > 4) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF04_VPHASE0, coef_phase0_vector[4]);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF04_VPHASE1, coef_phase1_vector[4]);
	}
  if (NUM_V_TAPS > 5) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF05_VPHASE0, coef_phase0_vector[5]);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF05_VPHASE1, coef_phase1_vector[5]);
	}
  if (NUM_V_TAPS > 6) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF06_VPHASE0, coef_phase0_vector[6]);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF06_VPHASE1, coef_phase1_vector[6]);
	}
  if (NUM_V_TAPS > 7) {
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF07_VPHASE0, coef_phase0_vector[7]);
	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF07_VPHASE1, coef_phase1_vector[7]);
	}
}

void configure_444_to_420(u32 BaseAddress, int NUM_H_TAPS, int NUM_V_TAPS, int *hcoef_vector, int *vcoef_vector   )
{
	  // clear out existing coefficient register values
	  clear_coef_values(BaseAddress);

	  // set new coefficient register values from coef_vector
	  if (NUM_H_TAPS > 0) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF00_HPHASE0, hcoef_vector[0]);
		}
	  if (NUM_H_TAPS > 1) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF01_HPHASE0, hcoef_vector[1]);
		}
	  if (NUM_H_TAPS > 2) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF02_HPHASE0, hcoef_vector[2]);
		}
	  if (NUM_H_TAPS > 3) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF03_HPHASE0, hcoef_vector[3]);
		}
	  if (NUM_H_TAPS > 4) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF04_HPHASE0, hcoef_vector[4]);
		}
	  if (NUM_H_TAPS > 5) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF05_HPHASE0, hcoef_vector[5]);
		}
	  if (NUM_H_TAPS > 6) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF06_HPHASE0, hcoef_vector[6]);
		}
	  if (NUM_H_TAPS > 7) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF07_HPHASE0, hcoef_vector[7]);
		}
	  if (NUM_H_TAPS > 8) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF08_HPHASE0, hcoef_vector[8]);
		}
	  if (NUM_H_TAPS > 9) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF09_HPHASE0, hcoef_vector[9]);
		}
	  if (NUM_H_TAPS > 10) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF10_HPHASE0, hcoef_vector[10]);
		}
	  if (NUM_H_TAPS > 11) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF11_HPHASE0, hcoef_vector[11]);
		}
	  if (NUM_H_TAPS > 12) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF12_HPHASE0, hcoef_vector[12]);
		}
	  if (NUM_H_TAPS > 13) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF13_HPHASE0, hcoef_vector[13]);
		}
	  if (NUM_H_TAPS > 14) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF14_HPHASE0, hcoef_vector[14]);
		}
	  if (NUM_H_TAPS > 15) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF15_HPHASE0, hcoef_vector[15]);
		}
	  if (NUM_H_TAPS > 16) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF16_HPHASE0, hcoef_vector[16]);
		}
	  if (NUM_H_TAPS > 17) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF17_HPHASE0, hcoef_vector[17]);
		}
	  if (NUM_H_TAPS > 18) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF18_HPHASE0, hcoef_vector[18]);
		}
	  if (NUM_H_TAPS > 19) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF19_HPHASE0, hcoef_vector[19]);
		}
	  if (NUM_H_TAPS > 20) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF20_HPHASE0, hcoef_vector[20]);
		}
	  if (NUM_H_TAPS > 21) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF21_HPHASE0, hcoef_vector[21]);
		}
	  if (NUM_H_TAPS > 22) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF22_HPHASE0, hcoef_vector[22]);
		}
	  if (NUM_H_TAPS > 23) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF23_HPHASE0, hcoef_vector[23]);
		}

	  if (NUM_V_TAPS > 0) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF00_VPHASE0, vcoef_vector[0]);
		}
	  if (NUM_V_TAPS > 1) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF01_VPHASE0, vcoef_vector[1]);
		}
	  if (NUM_V_TAPS > 2) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF02_VPHASE0, vcoef_vector[2]);
		}
	  if (NUM_V_TAPS > 3) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF03_VPHASE0, vcoef_vector[3]);
		}
	  if (NUM_V_TAPS > 4) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF04_VPHASE0, vcoef_vector[4]);
		}
	  if (NUM_V_TAPS > 5) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF05_VPHASE0, vcoef_vector[5]);
		}
	  if (NUM_V_TAPS > 6) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF06_VPHASE0, vcoef_vector[6]);
		}
	  if (NUM_V_TAPS > 7) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF07_VPHASE0, vcoef_vector[7]);
		}
}

void configure_420_to_444(u32 BaseAddress, int NUM_H_TAPS, int NUM_V_TAPS, int *hcoef_phase1_vector, int *vcoef_phase0_vector, int *vcoef_phase1_vector   )
{
	  // clear out existing coefficient register values
	  clear_coef_values(BaseAddress);

	  // set new coefficient register values from coef_vector
	  if (NUM_V_TAPS > 0) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF00_VPHASE0, vcoef_phase0_vector[0]);
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF00_VPHASE1, vcoef_phase1_vector[0]);
		}
	  if (NUM_V_TAPS > 1) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF01_VPHASE0, vcoef_phase0_vector[1]);
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF01_VPHASE1, vcoef_phase1_vector[1]);
		}
	  if (NUM_V_TAPS > 2) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF02_VPHASE0, vcoef_phase0_vector[2]);
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF02_VPHASE1, vcoef_phase1_vector[2]);
		}
	  if (NUM_V_TAPS > 3) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF03_VPHASE0, vcoef_phase0_vector[3]);
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF03_VPHASE1, vcoef_phase1_vector[3]);
		}
	  if (NUM_V_TAPS > 4) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF04_VPHASE0, vcoef_phase0_vector[4]);
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF04_VPHASE1, vcoef_phase1_vector[4]);
		}
	  if (NUM_V_TAPS > 5) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF05_VPHASE0, vcoef_phase0_vector[5]);
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF05_VPHASE1, vcoef_phase1_vector[5]);
		}
	  if (NUM_V_TAPS > 6) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF06_VPHASE0, vcoef_phase0_vector[6]);
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF06_VPHASE1, vcoef_phase1_vector[6]);
		}
	  if (NUM_V_TAPS > 7) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF07_VPHASE0, vcoef_phase0_vector[7]);
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF07_VPHASE1, vcoef_phase1_vector[7]);
		}

	  if (NUM_H_TAPS > 0) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF00_HPHASE1, hcoef_phase1_vector[0]);
		}
	  if (NUM_H_TAPS > 1) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF01_HPHASE1, hcoef_phase1_vector[1]);
		}
	  if (NUM_H_TAPS > 2) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF02_HPHASE1, hcoef_phase1_vector[2]);
		}
	  if (NUM_H_TAPS > 3) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF03_HPHASE1, hcoef_phase1_vector[3]);
		}
	  if (NUM_H_TAPS > 4) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF04_HPHASE1, hcoef_phase1_vector[4]);
		}
	  if (NUM_H_TAPS > 5) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF05_HPHASE1, hcoef_phase1_vector[5]);
		}
	  if (NUM_H_TAPS > 6) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF06_HPHASE1, hcoef_phase1_vector[6]);
		}
	  if (NUM_H_TAPS > 7) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF07_HPHASE1, hcoef_phase1_vector[7]);
		}
	  if (NUM_H_TAPS > 8) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF08_HPHASE1, hcoef_phase1_vector[8]);
		}
	  if (NUM_H_TAPS > 9) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF09_HPHASE1, hcoef_phase1_vector[9]);
		}
	  if (NUM_H_TAPS > 10) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF10_HPHASE1, hcoef_phase1_vector[10]);
		}
	  if (NUM_H_TAPS > 11) {
		CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF11_HPHASE1, hcoef_phase1_vector[11]);
		}
	  if (NUM_H_TAPS > 12) {
	 	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF12_HPHASE1, hcoef_phase1_vector[12]);
	 	}
	   if (NUM_H_TAPS > 13) {
	 	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF13_HPHASE1, hcoef_phase1_vector[13]);
	 	}
	   if (NUM_H_TAPS > 14) {
	 	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF14_HPHASE1, hcoef_phase1_vector[14]);
	 	}
	   if (NUM_H_TAPS > 15) {
	 	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF15_HPHASE1, hcoef_phase1_vector[15]);
	 	}
	   if (NUM_H_TAPS > 16) {
	 	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF16_HPHASE1, hcoef_phase1_vector[16]);
	 	}
	   if (NUM_H_TAPS > 17) {
	 	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF17_HPHASE1, hcoef_phase1_vector[17]);
	 	}
	   if (NUM_H_TAPS > 18) {
	 	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF18_HPHASE1, hcoef_phase1_vector[18]);
	 	}
	   if (NUM_H_TAPS > 19) {
	 	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF19_HPHASE1, hcoef_phase1_vector[19]);
	 	}
	   if (NUM_H_TAPS > 20) {
	 	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF20_HPHASE1, hcoef_phase1_vector[20]);
	 	}
	   if (NUM_H_TAPS > 21) {
	 	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF21_HPHASE1, hcoef_phase1_vector[21]);
	 	}
	   if (NUM_H_TAPS > 22) {
	 	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF22_HPHASE1, hcoef_phase1_vector[22]);
	 	}
	   if (NUM_H_TAPS > 23) {
	 	CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_COEF23_HPHASE1, hcoef_phase1_vector[23]);
	 	}
}

