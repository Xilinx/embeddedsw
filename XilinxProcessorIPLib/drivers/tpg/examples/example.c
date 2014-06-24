/*****************************************************************************
*
* Copyright (C) 2001 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"),to deal
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
*****************************************************************************/
 /**
 *
 * @file example.c
 *
 * This file demonstrates how to use Xilinx Test Pattern Generator (TPG)
 * ore pcore driver functions.
 *
 *
 * ***************************************************************************
 */

#include "tpg.h"
#include "xparameters.h"

/***************************************************************************/
// Test Pattern Generator Register Reading Example
// This function provides an example of how to read the current configuration
// settings of the TPG core.
/***************************************************************************/
void report_tpg_settings(u32 BaseAddress) {
  
  u32 status, reg_val;
  unsigned char inchar=0;

  xil_printf("Test Pattern Generator Core Configuration:\r\n");
  xil_printf(" Enable Bit: %1d\r\n", TPG_ReadReg(BaseAddress, TPG_CONTROL) & TPG_CTL_EN_MASK);  
  xil_printf(" Register Update Bit: %1d\r\n", (TPG_ReadReg(BaseAddress, TPG_CONTROL) & TPG_CTL_RUE_MASK) >> 1);	  
  xil_printf(" Reset Bit: %1d\r\n", TPG_ReadReg(BaseAddress, TPG_CONTROL) & TPG_RST_RESET);
  status = TPG_ReadReg(BaseAddress, TPG_STATUS);
  xil_printf(" TPG Status: %08x \r\n", status); 
  xil_printf(" Core Version:   %1d.%1d\r\n", TPG_ReadReg(BaseAddress, TPG_VERSION));  
  reg_val = TPG_ReadReg(BaseAddress, TPG_CONTROL );
  xil_printf("TPG_CONTROL           : %8x\r\n", reg_val);
  reg_val = TPG_ReadReg(BaseAddress, TPG_IRQ_EN );
  xil_printf("TPG_IRQ_EN : %8x\r\n", reg_val);
  reg_val = (TPG_ReadReg(BaseAddress, TPG_ACTIVE_SIZE ) && 0x1FFF);
  xil_printf("Active Rows      : %8d\r\n", reg_val);
  reg_val = (TPG_ReadReg(BaseAddress, TPG_ACTIVE_SIZE) >> 16);
  xil_printf("Active Columns      : %8d\r\n", reg_val);
  xil_printf("Press Space to continue!\r\n", reg_val);
  while (inchar != ' ') inchar = inbyte();  
}  

/*****************************************************************************/
//
// This is the main function for the TPG example.
//
/*****************************************************************************/
int main(void)
{
    // Print the current settings for the TPG core
	  report_tpg_settings(XPAR_TPG_0_BASEADDR); 
 
	  return 0;
}
