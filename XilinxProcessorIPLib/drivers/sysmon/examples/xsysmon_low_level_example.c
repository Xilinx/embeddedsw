/******************************************************************************
*
* Copyright (C) 2007 - 2014 Xilinx, Inc.  All rights reserved.
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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/****************************************************************************/
/**
*
* @file xsysmon_low_level_example.c
*
* This file contains a design example using the basic driver functions
* of the System Monitor driver. The example here shows using the
* driver/device in polled mode to check the on-chip temperature and voltages.
*
*
* @note
*
* The values of the on-chip temperature and the on-chip Vccaux voltage are read
* from the device and then the alarm thresholds are set in such a manner that
* the alarms occur.
*
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.00a xd/sv  05/22/07 First release
* 2.00a sv     07/07/08 Changed the example to read 16 bits of data from the
*			the ADC data registers.
* 4.00a ktn    10/22/09 Updated the example to use macros that have been
*		        renamed to remove _m from the name of the macro.
* 5.01a bss    03/13/12 Modified while loop condition to wait for EOS bit
*				to become high
* 5.03a bss    04/25/13 Modified SysMonLowLevelExample function to set
*			Sequencer Mode as Safe mode instead of Single
*			channel mode before configuring Sequencer registers.
*			CR #703729
* 7.3   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xsysmon_hw.h"
#include "xparameters.h"
#include "xstatus.h"
#include "xil_printf.h"

/************************** Constant Definitions ****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define SYSMON_BASEADDR		XPAR_SYSMON_0_BASEADDR

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/

int SysMonLowLevelExample(u32 BaseAddress);

/************************** Variable Definitions ****************************/

/****************************************************************************/
/**
*
* Main function that invokes the example given in this file.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note		None.
*
*****************************************************************************/
int main(void)
{
	int Status;

	/*
	 * Run the SysMonitor Low level example, specify the Base Address that
	 * is generated in xparameters.h.
	 */
	Status = SysMonLowLevelExample(SYSMON_BASEADDR);
	if (Status != XST_SUCCESS) {
		xil_printf("Sysmon lowlevel Example Failed\r\n");
		return XST_FAILURE;
	}
	xil_printf("Successfully ran Sysmon lowlevel Example\r\n");
	return XST_SUCCESS;

}

/****************************************************************************/
/**
*
* This function runs a test on the System Monitor device using the
* basic driver functions.
* The function does the following tasks:
*	- Reset the device
*	- Setup alarm thresholds for on-chip temperature and VCCAUX.
*	- Setup sequence registers to continuously monitor on-chip temperature
*	 and VCCAUX.
*	- Setup configuration registers to start the sequence.
*	- Read latest on-chip temperature and VCCAUX, as well as their maximum
*	 and minimum values. Also check if alarm(s) are set.
*
* @param	BaseAddress is the XPAR_<SYSMON_ADC_instance>_BASEADDRESS value
*		from xparameters.h.
*
* @return	XST_SUCCESS
*
* @note		None.
*
****************************************************************************/
int SysMonLowLevelExample(u32 BaseAddress)
{
	u32 RegValue;
	u16 TempData;
	u16 VccauxData;

	/*
	 * Reset the device.
	 */
	XSysMon_WriteReg(BaseAddress, XSM_SRR_OFFSET, XSM_SRR_IPRST_MASK);

	/*
	 * Disable the Channel Sequencer before configuring the Sequence
	 * registers.
	 */
	RegValue = XSysMon_ReadReg(BaseAddress, XSM_CFR1_OFFSET) &
			(~ XSM_CFR1_SEQ_VALID_MASK);
	XSysMon_WriteReg(BaseAddress, XSM_CFR1_OFFSET,	RegValue |
				XSM_CFR1_SEQ_SAFEMODE_MASK);


	/*
	 * Setup the Averaging to be done for the channels in the
	 * Configuration 0 register as 16 samples:
	 */
	RegValue = XSysMon_ReadReg(BaseAddress,
				XSM_CFR0_OFFSET) & (~XSM_CFR0_AVG_VALID_MASK);
	XSysMon_WriteReg(BaseAddress, XSM_CFR0_OFFSET,
				RegValue | XSM_CFR0_AVG16_MASK);

	/*
	 * Setup the Sequence register for 1st Auxiliary channel
	 * Setting is:
	 *	- Add acquisition time by 6 ADCCLK cycles.
	 *	- Bipolar Mode
	 *
	 * Setup the Sequence register for 16th Auxiliary channel
	 * Setting is:
	 *	- Add acquisition time by 6 ADCCLK cycles.
	 *	- Unipolar Mode
	 */

	/*
	 *  Set the Acquisition time for the specified channels.
	 */
	XSysMon_WriteReg(BaseAddress,XSM_SEQ07_OFFSET,
				(XSM_SEQ_CH_AUX00 | XSM_SEQ_CH_AUX15) >>
				XSM_SEQ_CH_AUX_SHIFT);

	/*
	 *  Set the input mode for the specified channels.
	 */
	XSysMon_WriteReg(BaseAddress, XSM_SEQ05_OFFSET,
				XSM_SEQ_CH_AUX00 >> XSM_SEQ_CH_AUX_SHIFT);



	/*
	 * Enable the averaging on the following channels in the Sequencer
	 * registers:
	 * 	- On-chip Temperature
	 * 	- On-chip VCCAUX supply sensor
	 * 	- 1st Auxiliary Channel
	 * 	- 16th Auxiliary Channel
	 */
	XSysMon_WriteReg(BaseAddress,XSM_SEQ02_OFFSET,
				XSM_SEQ_CH_TEMP | XSM_SEQ_CH_VCCAUX);

	XSysMon_WriteReg(BaseAddress, XSM_SEQ03_OFFSET,
				(XSM_SEQ_CH_AUX00 | XSM_SEQ_CH_AUX15) >>
				XSM_SEQ_CH_AUX_SHIFT);


	/*
	 * Enable the following channels in the Sequencer registers:
	 * 	- On-chip Temperature
	 * 	- On-chip VCCAUX supply sensor
	 * 	- 1st Auxiliary Channel
	 * 	- 16th Auxiliary Channel
	 */
	XSysMon_WriteReg(BaseAddress, XSM_SEQ00_OFFSET,
				XSM_SEQ_CH_TEMP | XSM_SEQ_CH_VCCAUX);
	XSysMon_WriteReg(BaseAddress, XSM_SEQ01_OFFSET,
				(XSM_SEQ_CH_AUX00 | XSM_SEQ_CH_AUX15) >>
				XSM_SEQ_CH_AUX_SHIFT);


	/*
	 * Set the ADCCLK frequency equal to 1/32 of System clock for the System
	 * Monitor/ADC in the Configuration Register 2.
	 */
	XSysMon_WriteReg(BaseAddress, XSM_CFR2_OFFSET, 32 <<
					XSM_CFR2_CD_SHIFT);


	/*
	 * Enable the Channel Sequencer in continuous sequencer cycling mode.
	 */
	RegValue = XSysMon_ReadReg(BaseAddress, XSM_CFR1_OFFSET) &
			(~ XSM_CFR1_SEQ_VALID_MASK);
	XSysMon_WriteReg(BaseAddress, XSM_CFR1_OFFSET,	RegValue |
				XSM_CFR1_SEQ_CONTINPASS_MASK);


	/*
	 * Wait till the End of Sequence occurs
	 */
	XSysMon_ReadReg(BaseAddress, XSM_SR_OFFSET); /* Clear the old status */
	while (((XSysMon_ReadReg(BaseAddress, XSM_SR_OFFSET)) &
			XSM_SR_EOS_MASK) != XSM_SR_EOS_MASK);


	/*
	 * Read the current value of the on-chip temperature.
	 */
	TempData = XSysMon_ReadReg(BaseAddress, XSM_TEMP_OFFSET);
	/*
	 * Read the current value of the on-chip VCCAUX voltage.
	 */
	VccauxData = XSysMon_ReadReg(BaseAddress, XSM_VCCAUX_OFFSET);


	/*
	 * Disable all the alarms in the Configuration Register 1.
	 */
	RegValue = XSysMon_ReadReg(BaseAddress, XSM_CFR1_OFFSET);
	RegValue |= XSM_CFR1_ALM_ALL_MASK;
	XSysMon_WriteReg(BaseAddress, XSM_CFR1_OFFSET, RegValue);

	/*
	 * Setup Alarm threshold registers for
	 * On-chip Temperature High limit
	 * VCCAUX High limit
	 * VCCAUX Low limit
	 */
	XSysMon_WriteReg(BaseAddress, XSM_ATR_TEMP_UPPER_OFFSET,
					TempData - 0x6F);
	XSysMon_WriteReg(BaseAddress, XSM_ATR_VCCAUX_UPPER_OFFSET,
					VccauxData - 0x6F);
	XSysMon_WriteReg(BaseAddress, XSM_ATR_VCCAUX_LOWER_OFFSET,
					VccauxData + 0x6F);


	/*
	 * Enable Alarm 0 for on-chip temperature and Alarm 2 for on-chip
	 * VCCAUX in the Configuration Register 1.
	 */
	RegValue = XSysMon_ReadReg(BaseAddress,
				XSM_CFR1_OFFSET) & (~XSM_CFR1_ALM_ALL_MASK);
	RegValue |= ((~(XSM_CFR1_ALM_VCCAUX_MASK | XSM_CFR1_ALM_TEMP_MASK)) &
				XSM_CFR1_ALM_ALL_MASK);
	XSysMon_WriteReg(BaseAddress, XSM_CFR1_OFFSET, RegValue);



	/*
	 * Read the current value of on-chip temperature.
	 */
	RegValue = XSysMon_ReadReg(BaseAddress, XSM_TEMP_OFFSET);

	/*
	 * Read the Maximum value of on-chip temperature.
	 */
	RegValue = XSysMon_ReadReg(BaseAddress, XSM_MAX_TEMP_OFFSET);

	/*
	 * Read the Minimum value of on-chip temperature.
	 */
	RegValue = XSysMon_ReadReg(BaseAddress, XSM_MIN_TEMP_OFFSET);

	/*
	 * Check if alarm for on-chip temperature is set.
	 */
	RegValue = XSysMon_ReadReg(BaseAddress, XSM_AOR_OFFSET) &
			XSM_AOR_TEMP_MASK;
	if (RegValue) {
		/*
		 * Alarm for on-chip temperature is set.
		 * The required processing should be put here.
		 */
	}


	/*
	 * Read the current value of on-chip VCCAUX.
	 */
	RegValue = XSysMon_ReadReg(BaseAddress, XSM_VCCAUX_OFFSET);

	/*
	 * Read the Maximum value of on-chip VCCAUX.
	 */
	RegValue = XSysMon_ReadReg(BaseAddress, XSM_MAX_VCCAUX_OFFSET);

	/*
	 * Read the Minimum value of on-chip VCCAUX.
	 */
	RegValue = XSysMon_ReadReg(BaseAddress, XSM_MIN_VCCAUX_OFFSET);

	/*
	 * Check if the alarm for on-chip VCCAUX is set.
	 */
	RegValue = XSysMon_ReadReg(BaseAddress, XSM_AOR_OFFSET) &
				XSM_AOR_VCCAUX_MASK;
	if (RegValue) {
		/*
		 * Alarm for on-chip VCCAUX is set.
		 * The required processing should be put here.
		 */
	}

	return XST_SUCCESS;
}
