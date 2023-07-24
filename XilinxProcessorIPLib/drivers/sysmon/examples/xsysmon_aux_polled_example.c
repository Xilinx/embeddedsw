/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xsysmon_aux_polled_example.c
*
* This file contains a design example using the driver functions
* of the System Monitor/ADC driver. This example shows the usage of the
* driver/device in polled mode with external mux and XADC in continuous pass
* Sequencer mode. It is provided to illustrate the usage of external mux.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 7.5   mn     09/12/18 First release
* 7.8   cog    07/20/23 Added support for SDT flow
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xsysmon.h"
#include "xstatus.h"
#include "xil_printf.h"

/************************** Constant Definitions ****************************/
#ifndef SDT
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define SYSMON_DEVICE_ID 	XPAR_SYSMON_0_DEVICE_ID
#else
#define SYSMON_DEVICE_ID	0
#endif
/* Use four External Channels for this Example */
#define XSM_SEQ_CH_AUX_MASK	XSM_SEQ_CH_AUX00 | \
				XSM_SEQ_CH_AUX01 | \
				XSM_SEQ_CH_AUX02 | \
				XSM_SEQ_CH_AUX03



/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/

int SysMonAuxPolledExample(u16 SysMonDeviceId);

static int SysMonFractionToInt(float FloatNum);

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

	xil_printf("Running Sysmon AUX polled Example\r\n");
	/*
	 * Run the SysMonitor AUX polled example, specify the Device ID that is
	 * generated in xparameters.h .
	 */
	Status = SysMonAuxPolledExample(SYSMON_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Sysmon AUX polled Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Sysmon AUX polled Example\r\n");
	return XST_SUCCESS;
}
#endif /* TESTAPP_GEN */

/****************************************************************************/
/**
*
* This function runs a test on the System Monitor/ADC device using the
* driver APIs.
* This function does the following tasks:
*	- Initiate the XADC device driver instance
*	- Run self-test on the device
*	- Reset the device
*	- Set up sequencer registers to continuously monitor the auxiliary
*	  channels available in XADC
*	- Set up configuration registers to start the sequencer in continuous
*	  pass mode
*	- Wait until End of sequence interrupt occurs and read the conversion
*	  data
*
* @param	SysMonDeviceId is the XPAR_<SYSMON_ADC_instance>_DEVICE_ID value
*		from xparameters.h.
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note   	None
*
****************************************************************************/
int SysMonAuxPolledExample(u16 SysMonDeviceId)
{
	int Status;
	XSysMon_Config *ConfigPtr;
	u16 VAuxData[4];
	u16 VAuxRawData[4];
	XSysMon *SysMonInstPtr = &SysMonInst;
	u32 Index;

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
	 * Setup the Sequence register for 1st to 4th Auxiliary
	 * channels
	 */
	Status = XSysMon_SetSeqInputMode(SysMonInstPtr, XSM_SEQ_CH_AUX00);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XSysMon_SetSeqAcqTime(SysMonInstPtr, XSM_SEQ_CH_AUX_MASK);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the averaging on the following channels in the Sequencer
	 * registers:
	 * 	- Auxiliary Channels - 0 to 3
	 */
	Status =  XSysMon_SetSeqAvgEnables(SysMonInstPtr, XSM_SEQ_CH_AUX_MASK);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the following channels in the Sequencer registers:
	 * 	- Auxiliary Channels - 0 to 3
	 */
	Status =  XSysMon_SetSeqChEnables(SysMonInstPtr, XSM_SEQ_CH_AUX_MASK);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Set the ADCCLK frequency equal to 1/32 of System clock for the System
	 * Monitor/ADC in the Configuration Register 2.
	 */
	XSysMon_SetAdcClkDivisor(SysMonInstPtr, 32);

	/*
	 * Enable external Mux and connect to Aux CH0.
	 */
	XSysMon_SetExtenalMux(SysMonInstPtr, 0x10); /* 0b'10000 to CH[4:0] */

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

	XSysMon_GetStatus(SysMonInstPtr);	/* Clear the latched status */

	/*
	 * Read the ADC converted Data from the data registers.
	 */
	/* Read ADC data for channels 0 - 3 */
	for (Index = 0; Index < 4; Index++) {
		VAuxRawData[Index] = XSysMon_GetAdcData(SysMonInstPtr,
					XSM_CH_AUX_MIN + Index);
		VAuxData[Index] = XSysMon_RawToVoltage(VAuxRawData[Index]);
		xil_printf("\r\nThe VAUX%02d is %0d.%03d Volts. \r\n", Index,
		(int)(VAuxData[Index]), SysMonFractionToInt(VAuxData[Index]));
	}


	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function converts the fraction part of the given floating point number
* (after the decimal point)to an integer.
*
* @param	FloatNum is the floating point number.
*
* @return	Integer number to a precision of 3 digits.
*
* @note
* This function is used in the printing of floating point data to a STDIO
* device using the xil_printf function. The xil_printf is a very small
* foot-print printf function and does not support the printing of floating
* point numbers.
*
*****************************************************************************/
int SysMonFractionToInt(float FloatNum)
{
	float Temp;

	Temp = FloatNum;
	if (FloatNum < 0) {
		Temp = -(FloatNum);
	}

	return( ((int)((Temp -(float)((int)Temp)) * (1000.0f))));
}
