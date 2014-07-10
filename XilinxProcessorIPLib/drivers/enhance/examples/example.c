/******************************************************************************
* (c) Copyright 2009-2013 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information
* of Xilinx, Inc. and is protected under U.S. and
* international copyright and other intellectual property
* laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any
* rights to the materials distributed herewith. Except as
* otherwise provided in a valid license issued to you by
* Xilinx, and to the maximum extent permitted by applicable
* law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
* WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
* AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
* BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
* INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
* (2) Xilinx shall not be liable (whether in contract or tort,
* including negligence, or under any other theory of
* liability) for any loss or damage of any kind or nature
* related to, arising under or in connection with these
* materials, including for any direct, or any indirect,
* special, incidental, or consequential loss or damage
* (including loss of data, profits, goodwill, or any type of
* loss or damage suffered as a result of any action brought
* by a third party) even if such damage or loss was
* reasonably foreseeable or Xilinx had been advised of the
* possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-
* safe, or for use in any application requiring fail-safe
* performance, such as life-support or safety devices or
* systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any
* other applications that could lead to death, personal
* injury, or severe property or environmental damage
* (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and
* liability of any use of Xilinx products in Critical
* Applications, subject only to applicable laws and
* regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
* PART OF THIS FILE AT ALL TIMES.
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file example.c
 *
 * This file demonstrates how to use Xilinx Image Enhancement (Enhance)
 * driver of the Xilinx Image Enhancement core. This code does not 
 * cover the Enhance setup and any other configuration which might be
 * required to get the Enhance device working properly.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 5.00a vc   06/19/13 Updated for new edge enhancement and noise reduction algorithms and registers
 * 4.00a vc   12/18/12 Updated for new gain quantization levels
 * 4.00a vc   10/16/12 Switched from Xuint32 to u32
 *                     Renamed example function to main()
 *                     Renamed reference to XPAR_ENHANCE_0_BASEADDR
 * 4.00a vc   04/24/12 Updated for v4.00.a
 * 2.00a vc   12/14/10 First release
 * </pre>
 *
 * ***************************************************************************
 */

#include "enhance.h"
#include "xparameters.h"

/***************************************************************************/
// Image Enhancement Register Reading Example
// This function provides an example of how to read the current configuration
// settings of the Enhance core.
/***************************************************************************/
void report_enhance_settings(u32 BaseAddress) {
  
  xil_printf("Image Edge Enhancement Core Configuration:\r\n");
  xil_printf(" Enhance Version: 0x%08x\r\n", ENHANCE_ReadReg(BaseAddress, ENHANCE_VERSION));
  xil_printf(" Enhance Enable Bit: %1d\r\n", ENHANCE_ReadReg(BaseAddress, ENHANCE_CONTROL) & ENHANCE_CTL_EN_MASK);  
  xil_printf(" Enhance Register Update Bit: %1d\r\n", (ENHANCE_ReadReg(BaseAddress, ENHANCE_CONTROL) & ENHANCE_CTL_RU_MASK) >> 1);	  
  xil_printf(" Enhance Reset Bit: %1d\r\n", (ENHANCE_ReadReg(BaseAddress, ENHANCE_CONTROL) & ENHANCE_CTL_RESET) >> 31);
  xil_printf(" Enhance AutoReset Bit: %1d\r\n", (ENHANCE_ReadReg(BaseAddress, ENHANCE_CONTROL) & ENHANCE_CTL_AUTORESET) >> 30);
  xil_printf(" Active Columns=%d, Active Rows=%d\r\n",
      ENHANCE_ReadReg(BaseAddress, ENHANCE_ACTIVE_SIZE)&0x0000FFFF,
      ENHANCE_ReadReg(BaseAddress, ENHANCE_ACTIVE_SIZE)>>16);
  xil_printf(" Noise Threshold=%d, Enhance Strength=%d, Halo Suppression=%d\r\n",
      ENHANCE_ReadReg(BaseAddress, ENHANCE_NOISE_THRESHOLD), 
      ENHANCE_ReadReg(BaseAddress, ENHANCE_ENHANCE_STRENGTH), 
      ENHANCE_ReadReg(BaseAddress, ENHANCE_HALO_SUPPRESS));  	  
}  

/***************************************************************************/
// Image Enhancement Register Update Example
//  This function provides an example of the process used to update
//  the noise, enhance, and halo registers in the Enhance core.
//  In most video systems, it is expected that this process would be executed 
//  in response to an interrupt connected to the SOF video timing signal
//  or a timeout signal associated with a watchdog timer.
/***************************************************************************/
void ENHANCE_Update_Example(u32 BaseAddress) {
  
	//Enable the Enhance software enable    
	ENHANCE_Enable(BaseAddress);
	
	//Disable register updates.
	//This is the default operating mode for the Enhance core, and allows
	//registers to be updated without effecting the core's behavior.
	ENHANCE_RegUpdateDisable(BaseAddress);
	
        //Set the noise threshold
	// These values are integers in the range: [0, 2^DATA_WIDTH-1]
	ENHANCE_WriteReg(BaseAddress, ENHANCE_NOISE_THRESHOLD,   255); //maximum value for 8 bit data

        //Set the enhance strength and halo suppression factor
	// These values are floating point values in the range: [0, 1]
	// These are represented as integers by multiplying by 2^15,
	// resulting in an integer value in the range from [0, 32728]
	ENHANCE_WriteReg(BaseAddress, ENHANCE_ENHANCE_STRENGTH,   32768); //maximum value
	ENHANCE_WriteReg(BaseAddress, ENHANCE_HALO_SUPPRESS,   32768); //maximum value
   
        //Enable register updates.
	//This mode will cause the active size and noise, enhance, halo registers internal
	//to the Enhance core to automatically be updated on the next SOF
	ENHANCE_RegUpdateEnable(BaseAddress);

}


/*****************************************************************************/
//
// This is the main function for the Enhance example.
//
/*****************************************************************************/
int main(void)
{
    //Print the current settings for the Enhance core
    report_enhance_settings(XPAR_ENHANCE_0_BASEADDR);
	 
    //Call the Enhance example, specify the Device ID generated in xparameters.h
    ENHANCE_Update_Example(XPAR_ENHANCE_0_BASEADDR);
	
    return 0;
}
