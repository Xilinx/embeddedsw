/******************************************************************************
*
* Copyright (C) 2012 - 2014 Xilinx, Inc.  All rights reserved.
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
 * This file demonstrates how to use Xilinx YCrCb to RGB Color Space Converter 
 * (YCRCB2RGB) driver on Xilinx YCrCb to RGB Color Space Converter (YCRCB2RGB) 
 * core. This code does not cover the Xilinx TimeBase setup and any other 
 * configuration which might be required to get the YCRCB2RGB device working properly.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 5.00a tb   02/28/12 Updates for the v5.00.a release
 * </pre>
 *
 * ***************************************************************************
 */

#include "ycrcb2rgb.h"
#include "xparameters.h"

/***************************************************************************/
// YCrCb to RGB Color Space Converter Register Reading Example
// This function provides an example of how to read the current configuration
// settings of the YCRCB2RGB core.
/***************************************************************************/
void report_ycc_settings(u32 BaseAddress) {
    xil_printf("YCrCb to RGB Color Space Converter Core Configuration:\r\n");
    xil_printf(" Version: 0x%08x\r\n", YCC_ReadReg(BaseAddress, YCC_VERSION));
    xil_printf(" Enable Bit: %1d\r\n", YCC_ReadReg(BaseAddress, YCC_CONTROL) & YCC_CTL_EN_MASK);
	  
    xil_printf(" Register Update Bit: %1d\r\n", (YCC_ReadReg(BaseAddress, YCC_CONTROL) & YCC_CTL_RUE_MASK) >> 1);
	  
    xil_printf(" Reset Bit: %1d\r\n", (YCC_ReadReg(BaseAddress, YCC_CONTROL) & YCC_RST_RESET) >> 31);
	  
    xil_printf(" AutoReset Bit: %1d\r\n", (YCC_ReadReg(BaseAddress, YCC_CONTROL) & YCC_RST_AUTORESET) >> 30);
	  
    xil_printf(" Columns=0x%08x, Rows=0x%08x\r\n",
               (YCC_ReadReg(BaseAddress, YCC_ACTIVE_SIZE)>>16), 
               (YCC_ReadReg(BaseAddress, YCC_ACTIVE_SIZE)&0xFFFF));

    xil_printf(" Acoef=0x%08x, Bcoef=0x%08x, Ccoef=0x%08x, Dcoef=0x%08x\r\n",
               YCC_ReadReg(BaseAddress, YCC_ACOEF), 
               YCC_ReadReg(BaseAddress, YCC_BCOEF), 
               YCC_ReadReg(BaseAddress, YCC_CCOEF), 
               YCC_ReadReg(BaseAddress, YCC_DCOEF));

    xil_printf(" R Offset=0x%08x, G Offset=0x%08x, B Offset=0x%08x\r\n",
               YCC_ReadReg(BaseAddress, YCC_ROFFSET), 
               YCC_ReadReg(BaseAddress, YCC_GOFFSET), 
               YCC_ReadReg(BaseAddress, YCC_BOFFSET));
	  
    xil_printf(" RGB Max=0x%08x, RGB Min=0x%08x\r\n",
               YCC_ReadReg(BaseAddress, YCC_RGBMAX), 
               YCC_ReadReg(BaseAddress, YCC_RGBMIN));
}  



/***************************************************************************/
// RGB to YCrCb Color Space Converter Register Update Example
// This function provides an example of the process used to update
// the coefficient and offset registers in the YCrCb2RGB core.
// In most video systems, it is expected that this process would be executed 
// in response to an interrupt connected to the SOF timing signal
// or a timeout signal associated with a watchdog timer.
/***************************************************************************/
void YCC_Update_Example() {
    //Enable the YCRCB2RGB software enable
    YCC_Enable(XPAR_YCRCB2RGB_0_BASEADDR);
	
    //Disable register updates.
    //This is the default operating mode for the CCM core, and allows
    //registers to be updated without effecting the core's behavior.
    YCC_RegUpdateDisable(XPAR_YCRCB2RGB_0_BASEADDR);

    //Set the Active Columns and Rows
    YCC_WriteReg(XPAR_YCRCB2RGB_0_BASEADDR, YCC_ACTIVE_SIZE, (1280<<16)+720); //1280x720

    //Set the coefficients
    // These values are floating point values in the range: [0.0, 1.0)
    // These are represented as integers by multiplying by 2^16,
    // resulting in an integer value in the range from [0, 65535]
    YCC_WriteReg(XPAR_YCRCB2RGB_0_BASEADDR, YCC_ACOEF,  22978);
    YCC_WriteReg(XPAR_YCRCB2RGB_0_BASEADDR, YCC_BCOEF, -11704);
    YCC_WriteReg(XPAR_YCRCB2RGB_0_BASEADDR, YCC_CCOEF,  -5641);
    YCC_WriteReg(XPAR_YCRCB2RGB_0_BASEADDR, YCC_DCOEF,  29049);	

    //Set the offsets
    // For Valid range = [-2147483648, 2147483647]
    YCC_WriteReg(XPAR_YCRCB2RGB_0_BASEADDR, YCC_ROFFSET, -6390272); //ROFFSET =  16
    YCC_WriteReg(XPAR_YCRCB2RGB_0_BASEADDR, YCC_GOFFSET,  3932416); //GOFFSET = 128
    YCC_WriteReg(XPAR_YCRCB2RGB_0_BASEADDR, YCC_BOFFSET, -7944448); //BOFFSET = 128

    //Set the Clip/Clamp
    // For 8-bit color:  Valid range = [0,   255]
    // For 10-bit color: Valid range = [0,  1023]
    // For 12-bit color: Valid range = [0,  4095]
    // For 16-bit color: Valid range = [0, 65535]
    YCC_WriteReg(XPAR_YCRCB2RGB_0_BASEADDR, YCC_RGBMAX,  240); //RGBMAX  = 240
    YCC_WriteReg(XPAR_YCRCB2RGB_0_BASEADDR, YCC_RGBMIN,   16); //RGBMIN  =  16

    //Enable register updates.
    //This mode will cause the coefficient and offset registers internally
    //to the CCM core to automatically be updated on the next SOF.
    YCC_RegUpdateEnable(XPAR_YCRCB2RGB_0_BASEADDR);

}


/*****************************************************************************/
//
// This is the main function for the YCrCb2RGB example.
//
/*****************************************************************************/
int main(void)
{
    struct ycc_coef_inputs coef_in;
    struct ycc_coef_outputs coef_out;

    // Print the current settings for the YCrCb2RGB core
    report_ycc_settings(XPAR_YCRCB2RGB_0_BASEADDR);
 
    // Call the YCrCb2RGB example, specify the Device ID generated in xparameters.h
    YCC_Update_Example(XPAR_YCRCB2RGB_0_BASEADDR);

    // Read the current YCrCb2RGB core coefficients
    YCC_get_coefficients(XPAR_YCRCB2RGB_0_BASEADDR, &coef_out);

    // Setup coef_in for SD_ITU_601, 16_to_235_for_Studio_Equipment
    // and data width of 8-bits
    YCC_select_standard(0, 1, XPAR_YCRCB2RGB_0_S_AXIS_VIDEO_DATA_WIDTH, &coef_in);

    // Translate into YCrCb2RGB core coefficients
    YCC_coefficient_translation(&coef_in, &coef_out, XPAR_YCRCB2RGB_0_S_AXIS_VIDEO_DATA_WIDTH, XPAR_YCRCB2RGB_0_MWIDTH);

    // Program the new YCrCb2RGB core coefficients
    YCC_set_coefficients(XPAR_YCRCB2RGB_0_BASEADDR, &coef_out);
	 
    // Print the current settings for the YCrCb2RGB core
    report_ycc_settings(XPAR_YCRCB2RGB_0_BASEADDR);

    return 0;
}


