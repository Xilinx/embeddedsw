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
/*****************************************************************************/
/**
 *
 * @file example.c
 *
 * This file demonstrates how to use Xilinx Chroma Resampler (cresample)
 * driver of the Xilinx Chroma Resampler core. This code does not 
 * cover the core setup and any other configuration which might be
 * required to get the Chroma Resampler device working properly.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 2.00a vc   04/24/12 Updated for v2.00.a
 * 2.00a vc   10/16/12 Switched from Xuint32 to u32
 *                     Renamed example function to main()
 *                     Renamed reference to XPAR_CRESAMPLE_0_BASEADDR
 * </pre>
 *
 * ***************************************************************************
 */

#include "cresample.h"
#include "xparameters.h"

/***************************************************************************/
// Chroma Resampler Register Reading Example
// This function provides an example of how to read the current configuration
// settings of the Chroma Resampler core.
/***************************************************************************/
void report_cresample_settings(u32 BaseAddress) {
  
  u32 reg_val;
  unsigned char inchar=0;

  xil_printf("Chroma Resampler Core Configuration:\r\n");
  xil_printf(" Enable Bit: %1d\r\n", CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_CONTROL) & CRESAMPLE_CTL_EN_MASK);  
  xil_printf(" Register Update Bit: %1d\r\n", (CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_CONTROL) & CRESAMPLE_CTL_RU_MASK) >> 1);	  
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_CONTROL );
  xil_printf("CRESAMPLE_CONTROL           : %8x\r\n", reg_val);

  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF00_HPHASE0 );
  xil_printf("CRESAMPLE6_COEF00_HPHASE0    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF01_HPHASE0 );
  xil_printf("CRESAMPLE_COEF01_HPHASE0    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF02_HPHASE0 );
  xil_printf("CRESAMPLE_COEF02_HPHASE0    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF03_HPHASE0 );
  xil_printf("CRESAMPLE_COEF03_HPHASE0    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF04_HPHASE0 );
  xil_printf("CRESAMPLE_COEF04_HPHASE0    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF05_HPHASE0 );
  xil_printf("CRESAMPLECOEF05_HPHASE0    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF06_HPHASE0 );
  xil_printf("CRESAMPLE_COEF06_HPHASE0    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF07_HPHASE0 );
  xil_printf("CRESAMPLE_COEF07_HPHASE0    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF08_HPHASE0 );
  xil_printf("CRESAMPLE_COEF08_HPHASE0    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF09_HPHASE0 );
  xil_printf("CRESAMPLE_COEF09_HPHASE0    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF10_HPHASE0 );
  xil_printf("CRESAMPLE_COEF10_HPHASE0    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF11_HPHASE0 );
  xil_printf("CRESAMPLE_COEF11_HPHASE0    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF12_HPHASE0 );
  xil_printf("CRESAMPLE_COEF12_HPHASE0    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF13_HPHASE0 );
  xil_printf("CRESAMPLE_COEF13_HPHASE0    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF14_HPHASE0 );
  xil_printf("CRESAMPLE_COEF14_HPHASE0    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF15_HPHASE0 );
  xil_printf("CRESAMPLE_COEF15_HPHASE0    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF16_HPHASE0 );
  xil_printf("CRESAMPLE_COEF16_HPHASE0    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF17_HPHASE0 );
  xil_printf("CRESAMPLE_COEF17_HPHASE0    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF18_HPHASE0 );
  xil_printf("CRESAMPLE_COEF18_HPHASE0    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF19_HPHASE0 );
  xil_printf("CRESAMPLE_COEF19_HPHASE0    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF20_HPHASE0 );
  xil_printf("CRESAMPLE_COEF20_HPHASE0    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF21_HPHASE0 );
  xil_printf("CRESAMPLE_COEF21_HPHASE0    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF22_HPHASE0 );
  xil_printf("CRESAMPLE_COEF22_HPHASE0    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF23_HPHASE0 );
  xil_printf("CRESAMPLE_COEF23_HPHASE0    : %8d\r\n", reg_val);

  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF00_HPHASE1 );
  xil_printf("CRESAMPLE_COEF00_HPHASE1    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF01_HPHASE1 );
  xil_printf("CRESAMPLE_COEF01_HPHASE1    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF02_HPHASE1 );
  xil_printf("CRESAMPLE_COEF02_HPHASE1    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF03_HPHASE1 );
  xil_printf("CRESAMPLE_COEF03_HPHASE1    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF04_HPHASE1 );
  xil_printf("CRESAMPLE_COEF04_HPHASE1    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF05_HPHASE1 );
  xil_printf("CRESAMPLE_COEF05_HPHASE1    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF06_HPHASE1 );
  xil_printf("CRESAMPLE_COEF06_HPHASE1    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF07_HPHASE1 );
  xil_printf("CRESAMPLE_COEF07_HPHASE1    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF08_HPHASE1 );
  xil_printf("CRESAMPLE_COEF08_HPHASE1    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF09_HPHASE1 );
  xil_printf("CRESAMPLE_COEF09_HPHASE1    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF10_HPHASE1 );
  xil_printf("CRESAMPLE_COEF10_HPHASE1    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF11_HPHASE1 );
  xil_printf("CRESAMPLE_COEF11_HPHASE1    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF12_HPHASE1 );
  xil_printf("CRESAMPLE_COEF12_HPHASE1    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF13_HPHASE1 );
  xil_printf("CRESAMPLE_COEF13_HPHASE1    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF14_HPHASE1 );
  xil_printf("CRESAMPLE_COEF14_HPHASE1    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF15_HPHASE1 );
  xil_printf("CRESAMPLE_COEF15_HPHASE1    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF16_HPHASE1 );
  xil_printf("CRESAMPLE_COEF16_HPHASE1    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF17_HPHASE1 );
  xil_printf("CRESAMPLE_COEF17_HPHASE1    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF18_HPHASE1 );
  xil_printf("CRESAMPLE_COEF18_HPHASE1    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF19_HPHASE1 );
  xil_printf("CRESAMPLE_COEF19_HPHASE1    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF20_HPHASE1 );
  xil_printf("CRESAMPLE_COEF20_HPHASE1    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF21_HPHASE1 );
  xil_printf("CRESAMPLE_COEF21_HPHASE1    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF22_HPHASE1 );
  xil_printf("CRESAMPLE_COEF22_HPHASE1    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF23_HPHASE1 );
  xil_printf("CRESAMPLE_COEF23_HPHASE1    : %8d\r\n", reg_val);
  
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF00_VPHASE0 );
  xil_printf("CRESAMPLE_COEF00_VPHASE0    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF01_VPHASE0 );
  xil_printf("CRESAMPLE_COEF01_VPHASE0    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF02_VPHASE0 );
  xil_printf("CRESAMPLE_COEF02_VPHASE0    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF03_VPHASE0 );
  xil_printf("CRESAMPLE_COEF03_VPHASE0    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF04_VPHASE0 );
  xil_printf("CRESAMPLE_COEF04_VPHASE0    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF05_VPHASE0 );
  xil_printf("CRESAMPLE_COEF05_VPHASE0    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF06_VPHASE0 );
  xil_printf("CRESAMPLE_COEF06_VPHASE0    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF07_VPHASE0 );
  xil_printf("CRESAMPLE_COEF07_VPHASE0    : %8d\r\n", reg_val);
 
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF00_VPHASE1 );
  xil_printf("CRESAMPLE_COEF00_VPHASE1    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF01_VPHASE1 );
  xil_printf("CRESAMPLE_COEF01_VPHASE1    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF02_VPHASE1 );
  xil_printf("CRESAMPLE_COEF02_VPHASE1    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF03_VPHASE1 );
  xil_printf("CRESAMPLE_COEF03_VPHASE1    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF04_VPHASE1 );
  xil_printf("CRESAMPLE_COEF04_VPHASE1    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF05_VPHASE1 );
  xil_printf("CRESAMPLE_COEF05_VPHASE1    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF06_VPHASE1 );
  xil_printf("CRESAMPLE_COEF06_VPHASE1    : %8d\r\n", reg_val);
  reg_val = CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_COEF07_VPHASE1 );
  xil_printf("CRESAMPLE_COEF07_VPHASE1    : %8d\r\n", reg_val);
  
  xil_printf("Press Space to continue!\r\n", reg_val);
  while (inchar != ' ') inchar = inbyte();  
}  

/*****************************************************************************/
//
// This is the main function for the Chroma Resampler example.
//
/*****************************************************************************/
int main(void)
{
    //Print the current settings for the Chroma Resampler core
    report_cresample_settings(XPAR_CRESAMPLE_0_BASEADDR); 
 
    return 0;
}
