/******************************************************************************
* Copyright (C) 2007 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xsysmon_polled_example.c
*
* This file contains a design example using the driver functions
* of the System Monitor driver. The example here shows the
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
* 2.00a sdm    09/26/08 Added code to return temperature value to the main
*			function. TestappPeripheral prints the temperature
* 4.00a ktn    10/22/09 Updated the example to use macros that have been
*		        renamed to remove _m from the name of the macro.
* 5.03a bss    04/25/13 Modified SysMonPolledExample function to set
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

#include "xsysmon.h"
#include "xparameters.h"
#include "xstatus.h"
#include "xil_printf.h"

/************************** Constant Definitions ****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define SYSMON_DEVICE_ID 	XPAR_SYSMON_0_DEVICE_ID


/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/

int SysMonPolledExample(u16 SysMonDeviceId, int *Temp);

/************************** Variable Definitions ****************************/

static XSysMon SysMonInst;      /* System Monitor driver instance */

#ifndef TESTAPP_GEN
/****************************************************************************/
/**
*
* Main function that invokes the polled example in this file.
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
	int Temp;

	/*
	 * Run the SysMonitor polled example, specify the Device ID that is
	 * generated in xparameters.h .
	 */
	Status = SysMonPolledExample(SYSMON_DEVICE_ID, &Temp);
	if (Status != XST_SUCCESS) {
		xil_printf("Sysmon polled Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Sysmon polled Example\r\n");
	return XST_SUCCESS;
}
#endif /* TESTAPP_GEN */

/****************************************************************************/
/**
*
* This function runs a test on the System Monitor/ADC device using the
* driver APIs.
* This function does the following tasks:
*	- Initiate the System Monitor device driver instance
*	- Run self-test on the device
*	- Setup alarms for on-chip temperature and VCCAUX
*	- Setup the sequence registers to continuously monitor on-chip
*	temperature and VCCAUX
*	- Setup configuration registers to start the sequence
*	- Read latest on-chip temperature and VCCAUX, as well as their maximum
*	 and minimum values. Also check if alarm(s) are set
*
* @param	SysMonDeviceId is the XPAR_<SYSMON_ADC_instance>_DEVICE_ID value
*		from xparameters.h.
* @param	Temp is an output parameter, it is a pointer through which the
*		current temperature value is returned to the main function.
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note   	None
*
****************************************************************************/
int SysMonPolledExample(u16 SysMonDeviceId, int *Temp)
{
	int Status;
	volatile u32 Value;
	XSysMon_Config *ConfigPtr;
	u16 TempData;
	u16 VccauxData;
	XSysMon *SysMonInstPtr = &SysMonInst;


	/*
	 * Initialize the SysMon driver.
	 */
	ConfigPtr = XSysMon_LookupConfig(SysMonDeviceId);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}
	XSysMon_CfgInitialize(SysMonInstPtr, ConfigPtr,
				ConfigPtr->BaseAddress);

	/*
	 * Self Test the System Monitor/ADC device
	 */
	Status = XSysMon_SelfTest(SysMonInstPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Disable the Channel Sequencer before configuring the Sequence
	 * registers.
	 */
	XSysMon_SetSequencerMode(SysMonInstPtr, XSM_SEQ_MODE_SAFE);


	/*
	 * Setup the Averaging to be done for the channels in the
	 * Configuration 0 register as 16 samples:
	 */
	XSysMon_SetAvg(SysMonInstPtr, XSM_AVG_16_SAMPLES);

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
	Status = XSysMon_SetSeqInputMode(SysMonInstPtr, XSM_SEQ_CH_AUX00);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XSysMon_SetSeqAcqTime(SysMonInstPtr, XSM_SEQ_CH_AUX15 |
						XSM_SEQ_CH_AUX00);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Enable the averaging on the following channels in the Sequencer
	 * registers:
	 * 	- On-chip Temperature
	 * 	- On-chip VCCAUX supply sensor
	 * 	- 1st Auxiliary Channel
	 * 	- 16th Auxiliary Channel
	 */
	Status =  XSysMon_SetSeqAvgEnables(SysMonInstPtr, XSM_SEQ_CH_TEMP |
						XSM_SEQ_CH_VCCAUX |
						XSM_SEQ_CH_AUX00 |
						XSM_SEQ_CH_AUX15);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the following channels in the Sequencer registers:
	 * 	- On-chip Temperature
	 * 	- On-chip VCCAUX supply sensor
	 * 	- 1st Auxiliary Channel
	 * 	- 16th Auxiliary Channel
	 */
	Status =  XSysMon_SetSeqChEnables(SysMonInstPtr, XSM_SEQ_CH_TEMP |
						XSM_SEQ_CH_VCCAUX |
						XSM_SEQ_CH_AUX00 |
						XSM_SEQ_CH_AUX15);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Set the ADCCLK frequency equal to 1/32 of System clock for the System
	 * Monitor/ADC in the Configuration Register 2.
	 */
	XSysMon_SetAdcClkDivisor(SysMonInstPtr, 32);

	/*
	 * Enable the Channel Sequencer in continuous sequencer cycling mode.
	 */
	XSysMon_SetSequencerMode(SysMonInstPtr, XSM_SEQ_MODE_CONTINPASS);

	/*
	 * Wait till the End of Sequence occurs
	 */
	XSysMon_GetStatus(SysMonInstPtr); /* Clear the old status */
	while ((XSysMon_GetStatus(SysMonInstPtr) & XSM_SR_EOS_MASK) !=
			XSM_SR_EOS_MASK);

	/*
	 * Disable all the alarms in the Configuration Register 1
	 */
	XSysMon_SetAlarmEnables(SysMonInstPtr, 0x0);


	/*
	 * Read the ADC converted Data from the data registers for on-chip
	 * temperature and on-chip VCCAUX
	 */
	TempData = XSysMon_GetAdcData(SysMonInstPtr, XSM_CH_TEMP);
	VccauxData = XSysMon_GetAdcData(SysMonInstPtr, XSM_CH_VCCAUX);

	/*
	 * Convert the ADC data into temperature
	 */
	*Temp = XSysMon_RawToTemperature(TempData);

	/*
	 * Set up Alarm threshold registers for
	 * On-chip Temperature High limit
	 * On-chip Temperature Low limit
	 * VCCAUX High limit
	 * VCCAUX Low limit
	 */
	XSysMon_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_TEMP_UPPER,
						TempData - 0x007F);
	XSysMon_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_TEMP_LOWER,
						TempData - 0x007F);
	XSysMon_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_VCCAUX_UPPER,
						VccauxData - 0x007F);
	XSysMon_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_VCCAUX_LOWER,
						VccauxData + 0x007F);

	/*
	 * Enable Alarm 0 for on-chip temperature and Alarm 2 for on-chip
	 * VCCAUX in the Configuration Register 1.
	 */
	XSysMon_SetAlarmEnables(SysMonInstPtr, (XSM_CFR1_ALM_VCCAUX_MASK |
						XSM_CFR1_ALM_TEMP_MASK));

	/*
	 * Enable the Channel Sequencer in continuous cycling mode.
	 */
	XSysMon_SetSequencerMode(SysMonInstPtr, XSM_SEQ_MODE_CONTINPASS);

	/*
	 * Read the current value of on-chip Temperature.
	 */
	Value = XSysMon_GetAdcData(SysMonInstPtr, XSM_CH_TEMP);

	/*
	 * Read the Maximum value of on-chip Temperature.
	 */
	Value = XSysMon_GetMinMaxMeasurement(SysMonInstPtr, XSM_MAX_TEMP);

	/*
	 * Read the Minimum value of on-chip Temperature.
	 */
	Value = XSysMon_GetMinMaxMeasurement(SysMonInstPtr, XSM_MIN_TEMP);

	/*
	 * Check if alarm for on-chip temperature is set.
	 */
	Value = XSysMon_GetAlarmOutputStatus(SysMonInstPtr) & XSM_AOR_TEMP_MASK;
	if (Value) {
		/*
		 * Alarm for on-chip temperature is set.
		 * The required processing should be put here.
		 */
	}

	/*
	 * Read the current value of on-chip VCCAUX.
	 */
	Value = XSysMon_GetAdcData(SysMonInstPtr, XSM_CH_VCCAUX);

	/*
	 * Read the Maximum value of on-chip VCCAUX.
	 */
	Value = XSysMon_GetMinMaxMeasurement(SysMonInstPtr, XSM_MAX_VCCAUX);

	/*
	 * Read the Minimum value of on-chip VCCAUX.
	 */
	Value = XSysMon_GetMinMaxMeasurement(SysMonInstPtr, XSM_MIN_VCCAUX);

	/*
	 * Check if alarm for on-chip VCCAUX is set.
	 */
	Value = XSysMon_GetAlarmOutputStatus(SysMonInstPtr) &
			XSM_AOR_VCCAUX_MASK;
	if (Value) {
		/*
		 * Alarm for on-chip VCCAUX is set.
		 * The required processing should be put here.
		 */
	}

	return XST_SUCCESS;
}
