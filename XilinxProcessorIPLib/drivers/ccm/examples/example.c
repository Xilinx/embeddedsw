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
 * This file demonstrates how to use Xilinx Color Correction Matrix (CCM)
 * driver on Xilinx Color Correction Matrix (CCM) core. This code does not 
 * cover the Xilinx TimeBase setup and any other configuration which might be
 * required to get the CCM device working properly.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 4.00a tb   02/27/12 Updates for the v4.00.a release
 * 1.00a xd   05/15/09 First release
 * </pre>
 *
 * ***************************************************************************
 */

#include "ccm.h"
#include "xparameters.h"

/***************************************************************************/
// Color Correction Matrix Register Reading Example
// This function provides an example of how to read the current configuration
// settings of the CCM core.
/***************************************************************************/
void report_ccm_settings(u32 BaseAddress) {
    xil_printf("Color Correction Matrix Core Configuration:\r\n");

    xil_printf(" CCM Version: 0x%08x\r\n", CCM_ReadReg(BaseAddress, CCM_VERSION));

    xil_printf(" CCM Enable Bit: %1x\r\n", (CCM_ReadReg(BaseAddress, CCM_CONTROL) & CCM_CTL_EN_MASK));
	  
    xil_printf(" CCM Register Update Bit: %1d\r\n", (CCM_ReadReg(BaseAddress, CCM_CONTROL) & CCM_CTL_RUE_MASK) >> 1);
	  
    xil_printf(" CCM Reset Bit: %1d\r\n", (CCM_ReadReg(BaseAddress, CCM_CONTROL) & CCM_RST_RESET) >> 31);
	  
    xil_printf(" CCM AutoReset Bit: %1d\r\n", (CCM_ReadReg(BaseAddress, CCM_CONTROL) & CCM_RST_AUTORESET) >> 30);
	  
    xil_printf(" Active Columns=%d, Active Rows=%d\r\n",
    	    CCM_ReadReg(BaseAddress, CCM_ACTIVE_SIZE)&0x0000FFFF,
    	    CCM_ReadReg(BaseAddress, CCM_ACTIVE_SIZE)>>16);
    xil_printf(" [K11=%8x, K12=%8x, K13=%8x ] [ROFFSET=%3d]\r\n", 
    CCM_ReadReg(BaseAddress, CCM_K11), 
    CCM_ReadReg(BaseAddress, CCM_K12), 
    CCM_ReadReg(BaseAddress, CCM_K13), 
    CCM_ReadReg(BaseAddress, CCM_ROFFSET));
	  
  xil_printf(" [K21=%8x, K22=%8x, K23=%8x ] [GOFFSET=%3d]\r\n", 
    CCM_ReadReg(BaseAddress, CCM_K21), 
    CCM_ReadReg(BaseAddress, CCM_K22), 
    CCM_ReadReg(BaseAddress, CCM_K23), 
    CCM_ReadReg(BaseAddress, CCM_GOFFSET));
	  
    xil_printf(" [K31=%8x, K32=%8x, K33=%8x ] [BOFFSET=%3d]\r\n", 
    CCM_ReadReg(BaseAddress, CCM_K31), 
    CCM_ReadReg(BaseAddress, CCM_K32), 
    CCM_ReadReg(BaseAddress, CCM_K33), 
    CCM_ReadReg(BaseAddress, CCM_BOFFSET));

    xil_printf(" [CLIP=%3d, CLAMP=%3d]\r\n",
    CCM_ReadReg(BaseAddress, CCM_CLIP), 
    CCM_ReadReg(BaseAddress, CCM_CLAMP));
}  



/***************************************************************************/
// Color Correction Matrix Register Update Example
//  This function provides an example of the process used to update
// the coefficient and offset registers in the CCM core.
//  In most video systems, it is expected that this process would be executed 
// in response to an interrupt connected to the VBlank video timing signal
// or a timeout signal associated with a watchdog timer.
/***************************************************************************/
void CCM_Update_Example(u32 BaseAddress) {
    //Enable the CCM software enable
    CCM_Enable(BaseAddress);
	
    //Disable register updates.
    //This is the default operating mode for the CCM core, and allows
    //registers to be updated without effecting the core's behavior.
    CCM_RegUpdateDisable(BaseAddress);

    //Set the coefficients
    // These values are floating point values in the range: [-4.0, 4.0)
    // These are represented as integers by multiplying by 2^15,
    // resulting in an integer value in the range from [-131072, 131071]
    CCM_WriteReg(BaseAddress, CCM_K11, 32768); //K11 = 1.0
    CCM_WriteReg(BaseAddress, CCM_K12, 32768); //K12 = 1.0
    CCM_WriteReg(BaseAddress, CCM_K13, 32768); //K13 = 1.0
    CCM_WriteReg(BaseAddress, CCM_K21, 32768); //K21 = 1.0
    CCM_WriteReg(BaseAddress, CCM_K22, 32768); //K22 = 1.0
    CCM_WriteReg(BaseAddress, CCM_K23, 32768); //K23 = 1.0
    CCM_WriteReg(BaseAddress, CCM_K31, 32768); //K31 = 1.0
    CCM_WriteReg(BaseAddress, CCM_K32, 32768); //K32 = 1.0
    CCM_WriteReg(BaseAddress, CCM_K33, 32768); //K33 = 1.0

    //Set the offsets
    // For 8-bit color:  Valid range = [  -256,   255]
    // For 10-bit color: Valid range = [ -1024,  1023]
    // For 12-bit color: Valid range = [ -4096,  4095]
    // For 16-bit color: Valid range = [-65536, 65535]
    CCM_WriteReg(BaseAddress, CCM_ROFFSET,  5); //ROFFSET = 5
    CCM_WriteReg(BaseAddress, CCM_GOFFSET, 10); //GOFFSET = 10
    CCM_WriteReg(BaseAddress, CCM_BOFFSET, 15); //BOFFSET = 15

    //Set the Clip/Clamp
    // For 8-bit color:  Valid range = [0,   255]
    // For 10-bit color: Valid range = [0,  1023]
    // For 12-bit color: Valid range = [0,  4095]
    // For 16-bit color: Valid range = [0, 65535]
    CCM_WriteReg(BaseAddress, CCM_CLIP, 240); //CLIP  = 240
    CCM_WriteReg(BaseAddress, CCM_CLAMP, 16); //CLAMP = 16

    //Enable register updates.
    //This mode will cause the coefficient and offset registers internally
    //to the CCM core to automatically be updated on the next SOF.
    CCM_RegUpdateEnable(BaseAddress);

}


/*****************************************************************************/
//
// This is the main function for the CCM example.
//
/*****************************************************************************/
int main(void)
{
    // Print the current settings for the CCM core
    report_ccm_settings(XPAR_CCM_0_BASEADDR);
 
    // Call the CCM example, specify the Device ID generated in xparameters.h
    CCM_Update_Example(XPAR_CCM_0_BASEADDR);
	 
    return 0;
}


