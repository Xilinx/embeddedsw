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
 * This file demonstrates how to use Xilinx RGB to YCrCb Color Space Converter 
 * (RGB2YCRCB) driver on Xilinx RGB to YCrCb Color Space Converter (RGB2YCRCB) 
 * core. This code does not cover the Xilinx TimeBase setup and any other 
 * configuration which might be required to get the YCRCB2RGB device working properly.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 5.00a tb   02/27/12 Updates for the v5.00.a release
 * </pre>
 *
 * ***************************************************************************
 */

#include "rgb2ycrcb.h"
#include "xparameters.h"

/***************************************************************************/
// RGB to YCrCb Color Space Converter Register Reading Example
// This function provides an example of how to read the current configuration
// settings of the RGB2YCRCB core.
/***************************************************************************/
void report_rgb_settings(u32 BaseAddress) {
    xil_printf("RGB to YCrCb Color Space Converter Core Configuration:\r\n");
    xil_printf(" Version: 0x%08x\r\n", RGB_ReadReg(BaseAddress, RGB_VERSION));
    xil_printf(" Enable Bit: %1d\r\n", RGB_ReadReg(BaseAddress, RGB_CONTROL) & RGB_CTL_EN_MASK);
	  
    xil_printf(" Register Update Bit: %1d\r\n", (RGB_ReadReg(BaseAddress, RGB_CONTROL) & RGB_CTL_RUE_MASK) >> 1);
	  
    xil_printf(" Reset Bit: %1d\r\n", (RGB_ReadReg(BaseAddress, RGB_CONTROL) & RGB_RST_RESET) >> 31);
	  
    xil_printf(" AutoReset Bit: %1d\r\n", (RGB_ReadReg(BaseAddress, RGB_CONTROL) & RGB_RST_AUTORESET) >> 30);
	  
    xil_printf(" Columns=0x%8x, Rows=0x%8x\r\n", 
               (RGB_ReadReg(BaseAddress, RGB_ACTIVE_SIZE)>>16), 
               (RGB_ReadReg(BaseAddress, RGB_ACTIVE_SIZE)&0xFFFF));

    xil_printf(" Acoef=0x%8x, Bcoef=0x%8x, Ccoef=0x%8x, Dcoef=0x%8x\r\n",
               RGB_ReadReg(BaseAddress, RGB_ACOEF), 
               RGB_ReadReg(BaseAddress, RGB_BCOEF), 
               RGB_ReadReg(BaseAddress, RGB_CCOEF), 
               RGB_ReadReg(BaseAddress, RGB_DCOEF));

    xil_printf(" Y Offset=0x%8x, Cb Offset=0x%8x, Cr Offset=0x%8x\r\n", 
               RGB_ReadReg(BaseAddress, RGB_YOFFSET), 
               RGB_ReadReg(BaseAddress, RGB_CBOFFSET), 
               RGB_ReadReg(BaseAddress, RGB_CROFFSET));
	  
    xil_printf(" Y Max=0x%8x, Y Min=0x%8x\r\n", 
               RGB_ReadReg(BaseAddress, RGB_YMAX), 
               RGB_ReadReg(BaseAddress, RGB_YMIN));

    xil_printf(" Cb Max=0x%8x, Cb Min=0x%8x\r\n", 
               RGB_ReadReg(BaseAddress, RGB_CBMAX), 
               RGB_ReadReg(BaseAddress, RGB_CBMIN));

    xil_printf(" Cr Max=0x%8x, Cr Min=0x%8x\r\n", 
               RGB_ReadReg(BaseAddress, RGB_CRMAX), 
               RGB_ReadReg(BaseAddress, RGB_CRMIN));
}  



/***************************************************************************/
// RGB to YCrCb Color Space Converter Register Update Example
// This function provides an example of the process used to update
// the coefficient and offset registers in the RGB2YCrCb core.
// In most video systems, it is expected that this process would be executed 
// in response to an interrupt connected to the SOF timing signal
// or a timeout signal associated with a watchdog timer.
/***************************************************************************/
void RGB_Update_Example() {
    //Enable the RGB2YCRCB software enable
    RGB_Enable(XPAR_RGB2YCRCB_0_BASEADDR);
	
    //Disable register updates.
    //This is the default operating mode for the CCM core, and allows
    //registers to be updated without effecting the core's behavior.
    RGB_RegUpdateDisable(XPAR_RGB2YCRCB_0_BASEADDR);

    //Set the Active Columns and Rows
    RGB_WriteReg(XPAR_RGB2YCRCB_0_BASEADDR, RGB_ACTIVE_SIZE, (1280<<16)+720); //1280x720

    //Set the coefficients
    // These values are floating point values in the range: [0.0, 1.0)
    // These are represented as integers by multiplying by 2^16,
    // resulting in an integer value in the range from [0, 65535]
    RGB_WriteReg(XPAR_RGB2YCRCB_0_BASEADDR, RGB_ACOEF, 19595); //ACOEF = 0.299
    RGB_WriteReg(XPAR_RGB2YCRCB_0_BASEADDR, RGB_BCOEF,  7471); //BCOEF = 0.114
    RGB_WriteReg(XPAR_RGB2YCRCB_0_BASEADDR, RGB_CCOEF, 46727); //CCOEF = 0.713
    RGB_WriteReg(XPAR_RGB2YCRCB_0_BASEADDR, RGB_DCOEF, 36962); //DCOEF = 0.564	

    //Set the offsets
    // For 8-bit color:  Valid range = [  -256,   255]
    // For 10-bit color: Valid range = [ -1024,  1023]
    // For 12-bit color: Valid range = [ -4096,  4095]
    // For 16-bit color: Valid range = [-65536, 65535]
    RGB_WriteReg(XPAR_RGB2YCRCB_0_BASEADDR, RGB_YOFFSET,   16); //YOFFSET  =  16
    RGB_WriteReg(XPAR_RGB2YCRCB_0_BASEADDR, RGB_CBOFFSET, 128); //CBOFFSET = 128
    RGB_WriteReg(XPAR_RGB2YCRCB_0_BASEADDR, RGB_CROFFSET, 128); //CROFFSET = 128

    //Set the Clip/Clamp
    // For 8-bit color:  Valid range = [0,   255]
    // For 10-bit color: Valid range = [0,  1023]
    // For 12-bit color: Valid range = [0,  4095]
    // For 16-bit color: Valid range = [0, 65535]
    RGB_WriteReg(XPAR_RGB2YCRCB_0_BASEADDR, RGB_YMAX,  240); //YMAX  = 240
    RGB_WriteReg(XPAR_RGB2YCRCB_0_BASEADDR, RGB_YMIN,   16); //YMIN  =  16
    RGB_WriteReg(XPAR_RGB2YCRCB_0_BASEADDR, RGB_CBMAX, 240); //CBMAX = 240
    RGB_WriteReg(XPAR_RGB2YCRCB_0_BASEADDR, RGB_CBMIN,  16); //CBMIN =  16
    RGB_WriteReg(XPAR_RGB2YCRCB_0_BASEADDR, RGB_CRMAX, 240); //CRMAX = 240
    RGB_WriteReg(XPAR_RGB2YCRCB_0_BASEADDR, RGB_CRMIN,  16); //CRMIN =  16

    //Enable register updates.
    //This mode will cause the coefficient and offset registers internally
    //to the CCM core to automatically be updated on the next SOF.
    RGB_RegUpdateEnable(XPAR_RGB2YCRCB_0_BASEADDR);

}


/*****************************************************************************/
//
// This is the main function for the RGB2YCrCb example.
//
/*****************************************************************************/
int main(void)
{
    struct rgb_coef_inputs coef_in;
    struct rgb_coef_outputs coef_out;

    // Print the current settings for the RGB2YCrCb core
    report_rgb_settings(XPAR_RGB2YCRCB_0_BASEADDR);
 
    // Call the RGB2YCrCb example, specify the Device ID generated in xparameters.h
    RGB_Update_Example(XPAR_RGB2YCRCB_0_BASEADDR);

    // Read the current RGB2YCrCb core coefficients
    RGB_get_coefficients(XPAR_RGB2YCRCB_0_BASEADDR, &coef_out);

    // Setup coef_in for SD_ITU_601, 16_to_235_for_Studio_Equipment
    // and data width of 8-bits
    RGB_select_standard(0, 1, XPAR_RGB2YCRCB_0_M_AXIS_VIDEO_DATA_WIDTH, &coef_in);

    // Translate into RGB2YCrCb core coefficients
    RGB_coefficient_translation(&coef_in, &coef_out,XPAR_RGB2YCRCB_0_M_AXIS_VIDEO_DATA_WIDTH);

    // Program the new RGB2YCrCb core coefficients
    RGB_set_coefficients(XPAR_RGB2YCRCB_0_BASEADDR, &coef_out);
	 
    // Print the current settings for the RGB2YCrCb core
    report_rgb_settings(XPAR_RGB2YCRCB_0_BASEADDR);

    return 0;
}


