/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xsysmonpsu_polled_example.c
*
* This file contains a design example using the driver functions
* of the System Monitor driver. The example here shows the
* driver/device in polled mode to check the on-chip temperature and voltages.
*
* @note
*
* This examples also assumes that there is a STDIO device in the system.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.0   kvn    12/15/15 First release
*       ms     04/05/17 Modified Comment lines in functions to
*                       recognize it as documentation block for doxygen
*                       generation.
* 2.3   ms    12/12/17 Added peripheral test support.
*       mn    03/08/18 Update code to run at higher frequency and remove sleep
* 2.6   aad   11/21/19 Removed reading of AUX channels
*       aad   11/22/19 Added support for XSYSMON_PL
* 2.9   cog    07/20/23 Added support for SDT flow
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xsysmonpsu.h"
#ifndef SDT
#include "xparameters.h"
#endif
#include "xstatus.h"
#include "stdio.h"

/************************** Constant Definitions ****************************/
#ifndef SDT
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define SYSMON_DEVICE_ID 	XPAR_XSYSMONPSU_0_DEVICE_ID
#else
#define SYSMON_DEVICE_ID 	0xffa50000
#endif

/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/

/* User needs to define the macro PL_EXAMPLE to run the code on PL SYSMON.
 * By default the example will run on XSYSMON_PS
 */
#ifdef PL_EXAMPLE
#define XSYSMON_TYPE	XSYSMON_PL
#else
#define XSYSMON_TYPE	XSYSMON_PS
#endif

#define printf xil_printf /* Small foot-print printf function */

/************************** Function Prototypes *****************************/

int SysMonPsuPolledPrintfExample(u32 SysMonDeviceId);
static int SysMonPsuFractionToInt(float FloatNum);

/************************** Variable Definitions ****************************/

static XSysMonPsu SysMonInst;      /* System Monitor driver instance */
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

	/*
	 * Run the SysMonitor polled example, specify the Device ID that is
	 * generated in xparameters.h.
	 */
	Status = SysMonPsuPolledPrintfExample(SYSMON_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Sysmon Polled Example Test Failed\r\n");
		return XST_FAILURE;
	}

#ifdef PL_EXAMPLE
	xil_printf("Successfully ran Sysmon PL Polled Example Test\r\n");
#else
	xil_printf("Successfully ran Sysmon PS Polled Example Test\r\n");
#endif

	return XST_SUCCESS;
}
#endif

/****************************************************************************/
/**
*
* This function runs a test on the System Monitor device using the
* driver APIs.
* This function does the following tasks:
*	- Initiate the System Monitor device driver instance
*	- Run self-test on the device
*	- Setup the sequence registers to continuously monitor on-chip
*	temperature, Supply 1 and Supply 3
*	- Setup configuration registers to start the sequence
*	- Read the latest on-chip temperature, Supply 1 and Supply 3
*
* @param	SysMonDeviceId is the XPAR_<SYSMON_instance>_DEVICE_ID value
*		from xparameters.h.
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note   	None
*
****************************************************************************/
int SysMonPsuPolledPrintfExample(u32 SysMonDeviceId)
{
	int Status;
	XSysMonPsu_Config *ConfigPtr;
	u32 TempRawData;
	u32 Supply3RawData;
	u32 Supply1RawData;
	float TempData;
	float Supply3Data;
	float Supply1Data;
	float MaxData;
	float MinData;
	u64 IntrStatus;
	XSysMonPsu *SysMonInstPtr = &SysMonInst;

	printf("\r\nEntering the SysMon Polled Example. \r\n");

	/* Initialize the SysMon driver. */
	ConfigPtr = XSysMonPsu_LookupConfig(SysMonDeviceId);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}
	XSysMonPsu_CfgInitialize(SysMonInstPtr, ConfigPtr,
				ConfigPtr->BaseAddress);

	/* Self Test the System Monitor device */
	Status = XSysMonPsu_SelfTest(SysMonInstPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Disable the Channel Sequencer before configuring the Sequence
	 * registers.
	 */
	XSysMonPsu_SetSequencerMode(SysMonInstPtr, XSM_SEQ_MODE_SAFE, XSYSMON_TYPE);


	/* Disable all the alarms in the Configuration Register 1. */
	XSysMonPsu_SetAlarmEnables(SysMonInstPtr, 0x0, XSYSMON_TYPE);


	/*
	 * Setup the Averaging to be done for the channels in the
	 * Configuration 0 register as 16 samples:
	 */
	XSysMonPsu_SetAvg(SysMonInstPtr, XSM_AVG_16_SAMPLES, XSYSMON_TYPE);


	/*
	 * Enable the averaging on the following channels in the Sequencer
	 * registers:
	 * 	- On-chip Temperature, Supply 1/Supply 3  supply sensors
	  *	- Calibration Channel
	 */
	Status =  XSysMonPsu_SetSeqAvgEnables(SysMonInstPtr, XSYSMONPSU_SEQ_CH0_TEMP_MASK |
			XSYSMONPSU_SEQ_CH0_SUP1_MASK |
			XSYSMONPSU_SEQ_CH0_SUP3_MASK |
			XSYSMONPSU_SEQ_CH0_CALIBRTN_MASK, XSYSMON_TYPE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the following channels in the Sequencer registers:
	 * 	- On-chip Temperature, Supply 1/Supply 3 supply sensors
	 *	- Calibration Channel
	 */
	Status =  XSysMonPsu_SetSeqChEnables(SysMonInstPtr, XSYSMONPSU_SEQ_CH0_TEMP_MASK |
			XSYSMONPSU_SEQ_CH0_SUP1_MASK |
			XSYSMONPSU_SEQ_CH0_SUP3_MASK |
			XSYSMONPSU_SEQ_CH0_CALIBRTN_MASK, XSYSMON_TYPE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Clear any bits set in the Interrupt Status Register. */
	IntrStatus = XSysMonPsu_IntrGetStatus(SysMonInstPtr);
	XSysMonPsu_IntrClear(SysMonInstPtr, IntrStatus);

	/* Enable the Channel Sequencer in continuous sequencer cycling mode. */
	XSysMonPsu_SetSequencerMode(SysMonInstPtr, XSM_SEQ_MODE_CONTINPASS, XSYSMON_TYPE);

	/* Wait till the End of Sequence occurs */
	while ((XSysMonPsu_IntrGetStatus(SysMonInstPtr) & ((u64)XSYSMONPSU_ISR_1_EOS_MASK<< 32)) !=
			((u64)XSYSMONPSU_ISR_1_EOS_MASK<< 32));

	/*
	 * Read the on-chip Temperature Data (Current/Maximum/Minimum)
	 * from the ADC data registers.
	 */
	TempRawData = XSysMonPsu_GetAdcData(SysMonInstPtr, XSM_CH_TEMP, XSYSMON_TYPE);
	TempData = XSysMonPsu_RawToTemperature_OnChip(TempRawData);
	printf("\r\nThe Current Temperature is %0d.%03d Centigrades.\r\n",
				(int)(TempData), SysMonPsuFractionToInt(TempData));


	TempRawData = XSysMonPsu_GetMinMaxMeasurement(SysMonInstPtr, XSM_MAX_TEMP, XSYSMON_TYPE);
	MaxData = XSysMonPsu_RawToTemperature_OnChip(TempRawData);
	printf("The Maximum Temperature is %0d.%03d Centigrades. \r\n",
				(int)(MaxData), SysMonPsuFractionToInt(MaxData));

	TempRawData = XSysMonPsu_GetMinMaxMeasurement(SysMonInstPtr, XSM_MIN_TEMP, XSYSMON_TYPE);
	MinData = XSysMonPsu_RawToTemperature_OnChip(TempRawData);
	printf("The Minimum Temperature is %0d.%03d Centigrades. \r\n",
				(int)(MinData), SysMonPsuFractionToInt(MinData));

	/*
	 * Read the Supply 1 Votage Data (Current/Maximum/Minimum) from the
	 * ADC data registers.
	 */
	Supply1RawData = XSysMonPsu_GetAdcData(SysMonInstPtr, XSM_CH_SUPPLY1, XSYSMON_TYPE);
	Supply1Data = XSysMonPsu_RawToVoltage(Supply1RawData);
	printf("\r\nThe Current Supply 1 is %0d.%03d Volts. \r\n",
			(int)(Supply1Data), SysMonPsuFractionToInt(Supply1Data));

	Supply1RawData = XSysMonPsu_GetMinMaxMeasurement(SysMonInstPtr,
							XSM_MAX_SUPPLY1, XSYSMON_TYPE);
	MaxData = XSysMonPsu_RawToVoltage(Supply1RawData);
	printf("The Maximum Supply 1 is %0d.%03d Volts. \r\n",
			(int)(MaxData), SysMonPsuFractionToInt(MaxData));

	Supply1RawData = XSysMonPsu_GetMinMaxMeasurement(SysMonInstPtr,
							XSM_MIN_SUPPLY1, XSYSMON_TYPE);
	MinData = XSysMonPsu_RawToVoltage(Supply1RawData);
	printf("The Minimum Supply 1 is %0d.%03d Volts. \r\n",
			(int)(MinData), SysMonPsuFractionToInt(MinData));

	/*
	 * Read the Supply 3 Votage Data (Current/Maximum/Minimum) from the
	 * ADC data registers.
	 */
	Supply3RawData = XSysMonPsu_GetAdcData(SysMonInstPtr, XSM_CH_SUPPLY3, XSYSMON_TYPE);
	Supply3Data = XSysMonPsu_RawToVoltage(Supply3RawData);
	printf("\r\nThe Current Supply 3 is %0d.%03d Volts. \r\n",
			(int)(Supply3Data), SysMonPsuFractionToInt(Supply3Data));

	Supply3RawData = XSysMonPsu_GetMinMaxMeasurement(SysMonInstPtr,
							XSM_MAX_SUPPLY3, XSYSMON_TYPE);
	MaxData = XSysMonPsu_RawToVoltage(Supply3RawData);
	printf("The Maximum Supply 3 is %0d.%03d Volts. \r\n",
				(int)(MaxData), SysMonPsuFractionToInt(MaxData));


	Supply3RawData = XSysMonPsu_GetMinMaxMeasurement(SysMonInstPtr,
							XSM_MIN_SUPPLY3, XSYSMON_TYPE);
	MinData = XSysMonPsu_RawToVoltage(Supply3RawData);
	printf("The Minimum Supply 3 is %0d.%03d Volts. \r\n\r\n",
				(int)(MinData), SysMonPsuFractionToInt(MinData));

	printf("Exiting the SysMon Polled Example. \r\n");

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
* This function is used in the printing of floating point data to a STDIO device
* using the xil_printf function. The xil_printf is a very small foot-print
* printf function and does not support the printing of floating point numbers.
*
*****************************************************************************/
int SysMonPsuFractionToInt(float FloatNum)
{
	float Temp;

	Temp = FloatNum;
	if (FloatNum < 0) {
		Temp = -(FloatNum);
	}

	return( ((int)((Temp -(float)((int)Temp)) * (1000.0f))));
}
