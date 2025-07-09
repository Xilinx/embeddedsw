/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xsysmonpsu_intr_example.c
*
* This file contains a design example using the driver functions
* of the System Monitor driver. This example here shows the usage of the
* driver/device in interrupt mode to handle on-chip temperature and voltage
* alarm interrupts.
*
*
* @note
*
* This code assumes that no Operating System is being used.
*
* The values of the on-chip Temperature, Supply 1 voltage and Supply 3 voltage are
* read from the device and then the alarm thresholds are set in such a manner
* that the alarms occur.
*
* This examples also assumes that there is a STDIO device in the system.
* This example has floating point calculations and uses printfs for outputting
* floating point data, therefore the memory allocated for the Stack must be
* more.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.0   kvn    12/15/15 First release
*              02/15/16 Corrected order of Enabling / Disabling of
*                       interrupts.
*       ms     04/05/17 Modified Comment lines in functions to
*                       recognize it as documentation block for doxygen
*                       generation.
* 2.3  ms      12/12/17 Added peripheral test support
*      mn      03/08/18 Update code to run at higher frequency
* 2.6  aad     11/21/19 Removed reading of AUX channels
*      aad     11/22/19 Added support for PL_EXAMPLE
* 2.9   cog    07/20/23 Added support for SDT flow
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xsysmonpsu.h"
#ifndef SDT
#include "xparameters.h"
#endif
#include "xstatus.h"
#include "xil_exception.h"
#include "xscugic.h"
#include "xil_printf.h"

/************************** Constant Definitions ****************************/
#ifndef SDT
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define SYSMON_DEVICE_ID	XPAR_XSYSMONPSU_0_DEVICE_ID

/* SCUGIC Interrupt Controller */
#define XSCUGIC_DEVICE_ID	XPAR_SCUGIC_SINGLE_DEVICE_ID
#define INTR_ID			XPAR_XSYSMONPSU_INTR
#else
#define SYSMON_DEVICE_ID	0xffa50000
#define XSCUGIC_DEVICE_ID	0
#define INTR_ID	(56U + 32U)
#endif
/* User needs to define the macro PL_EXAMPLE to run the code on PL SYSMON.
 * By default the example will run on XSYSMON_PS
 */
#ifdef PL_EXAMPLE
#define XSYSMON_TYPE	XSYSMON_PL
#else
#define XSYSMON_TYPE	XSYSMON_PS
#endif

/*
 * The following are the definitions of the Alarm Limits to be programmed to
 * the threshold registers. The user needs to change these according to the
 * needs of the application.
 */
#define TEST_TEMP_UPPER		85.0f /* Temperature Upper Alarm Limit */
#define TEST_TEMP_LOWER		65.0f /* Temperature Lower Alarm Limit */

#define TEST_Supply_1_UPPER	1.05f /* Supply 1 Upper Alarm Limit */
#define TEST_Supply_1_LOWER	0.95f /* Supply 1 Lower Alarm Limit */

#define TEST_Supply_3_UPPER	2.625f /* Supply 3 Upper Alarm Limit */
#define TEST_Supply_3_LOWER	2.375f /* Supply 3 Lower Alarm Limit */

#define printf xil_printf 	/* Small foot-print printf function */

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/

int SysMonPsuIntrExample(XScuGic* XScuGicInstPtr,
			XSysMonPsu* SysMonInstPtr,
			u32 SysMonDeviceId,
			u16 SysMonIntrId);


static void SysMonPsuInterruptHandler(void *CallBackRef);

static int SysMonPsuSetupInterruptSystem(XScuGic* XScuGicInstPtr,
				      XSysMonPsu *SysMonPtr,
				      u16 IntrId );

static int SysMonPsuFractionToInt(float FloatNum);

/************************** Variable Definitions ****************************/

#ifndef TESTAPP_GEN
static XSysMonPsu SysMonInst; 		/* System Monitor driver instance */
static XScuGic XScuGicInst; 			/* Instance of the XXScuGic driver */
#endif

/* Shared variables used to test the callbacks. */
static volatile int TempIntrActive = FALSE;	/* Temperature alarm intr active */
static volatile int Supply1Intr = FALSE;	/* Supply 1 alarm interrupt */
static volatile int Supply3Intr = FALSE;	/* Supply 3 alarm interrupt */

#ifndef TESTAPP_GEN
/****************************************************************************/
/**
*
* Main function that invokes the Interrupt example.
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
	 * Run the SysMonitor interrupt example, specify the parameters that
	 * are generated in xparameters.h.
	 */
	Status = SysMonPsuIntrExample(&XScuGicInst, &SysMonInst, SYSMON_DEVICE_ID,
								INTR_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("SysMonPsu Interrupt Example Test Failed\r\n");
		return XST_FAILURE;
	}

#ifdef PL_EXAMPLE
	xil_printf("Successfully ran SysMonPsu Interrupt PL Example Test\r\n");
#else
	xil_printf("Successfully ran SysMonPsu Interrupt PS Example Test\r\n");
#endif

	return XST_SUCCESS;
}
#endif

/****************************************************************************/
/**
*
* This function runs a test on the System Monitor device using the
* driver APIs.
*
* The function does the following tasks:
*	- Initiate the System Monitor device driver instance
*	- Run self-test on the device
*	- Reset the device
*	- Set up alarms for on-chip temperature, Supply 1 and Supply 3
*	- Set up sequence registers to continuously monitor on-chip
*	temperature, Supply 1 and  Supply 3
*	- Setup interrupt system
*	- Enable interrupts
*	- Set up configuration registers to start the sequence
*	- Wait until temperature alarm interrupt or Supply 1 alarm interrupt
*	or Supply 3 alarm interrupt occurs
*
* @param	XScuGicInstPtr is a pointer to the Interrupt Controller
*		driver Instance.
* @param	SysMonInstPtr is a pointer to the XSysMonPsu driver Instance.
* @param	SysMonDeviceId is the XPAR_<SYSMON_instance>_DEVICE_ID
*		value from xparameters.h.
* @param	SysMonIntrId is
*		XPAR_<SYSMON_instance>_INTR value from xparameters_ps.h
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note		This function may never return if no interrupt occurs.
*
****************************************************************************/
int SysMonPsuIntrExample(XScuGic* XScuGicInstPtr, XSysMonPsu* SysMonInstPtr,
			u32 SysMonDeviceId, u16 SysMonIntrId)
{
	int Status;
	XSysMonPsu_Config *ConfigPtr;
	u32 Data;
	u32 TempRawData;
	u32 Supply3RawData;
	u32 Supply1RawData;
	float TempData;
	float Supply3Data;
	float Supply1Data;
	float MaxData;
	float MinData;
	u32 IntrStatus;

	printf("\r\nEntering the SysMonPsu Interrupt Example. \r\n");

	/* Initialize the SysMon driver. */
	ConfigPtr = XSysMonPsu_LookupConfig(SysMonDeviceId);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}
	XSysMonPsu_CfgInitialize(SysMonInstPtr, ConfigPtr, ConfigPtr->BaseAddress);

	/* Self Test the System Monitor device. */
	Status = XSysMonPsu_SelfTest(SysMonInstPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Disable the Channel Sequencer before configuring the Sequence
	 * registers.
	 */
	XSysMonPsu_SetSequencerMode(SysMonInstPtr, XSM_SEQ_MODE_SAFE, XSYSMON_TYPE);

	/*
	 * Setup the Averaging to be done for the channels in the
	 * Configuration 0 register as 16 samples:
	 */
	XSysMonPsu_SetAvg(SysMonInstPtr, XSM_AVG_16_SAMPLES, XSYSMON_TYPE);


	/*
	 * Enable the averaging on the following channels in the Sequencer
	 * registers:
	 *	In SYSMON_PS Mode:
	 *	- On-chip Temperature
	 *	- On-chip VCC_PSINTLP supply sensor
	 *	- On-chip VCC_PSAUX supply sensor
	 *	- Calibration Channel
	 *
	 *	In PL_EXAMPLE Mode:
	 *	- On-chip Temperature
	 *	- On-chip Supply 1 supply sensor
	 *	- On-chip VCCBRAM supply sensor
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
	 *	In SYSMON_PS Mode:
	 *	- On-chip Temperature
	 *	- On-chip VCC_PSINTLP supply 1 sensor
	 *	- On-chip VCC_PSAUX supply 3 sensor
	 *	- Calibration Channel
	 *
	 *	In PL_EXAMPLE Mode:
	 *	- On-chip Temperature
	 *	- On-chip Supply 1 supply 1 sensor
	 *	- On-chip VCCBRAM supply 3 sensor
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
	XSysMonPsu_SetSequencerMode(SysMonInstPtr, XSM_SEQ_MODE_CONTINPASS,
			XSYSMON_TYPE);

	/* Wait till the End of Sequence occurs. */
	while ((XSysMonPsu_IntrGetStatus(SysMonInstPtr) & ((u64)XSYSMONPSU_ISR_1_EOS_MASK<< 32)) !=
			((u64)XSYSMONPSU_ISR_1_EOS_MASK<< 32));

	/*
	 * Read the ADC converted Data from the data registers for on-chip
	 * temperature, on-chip Supply 1 voltage and on-chip Supply 3 voltage.
	 */
	TempRawData	= XSysMonPsu_GetAdcData(SysMonInstPtr, XSM_CH_TEMP, XSYSMON_TYPE);
	Supply1RawData	= XSysMonPsu_GetAdcData(SysMonInstPtr, XSM_CH_SUPPLY1, XSYSMON_TYPE);
	Supply3RawData	= XSysMonPsu_GetAdcData(SysMonInstPtr, XSM_CH_SUPPLY3, XSYSMON_TYPE);

	/* Convert the Raw Data to Degrees Centigrade and Voltage. */
	TempData	= XSysMonPsu_RawToTemperature_OnChip(TempRawData);
	Supply1Data	= XSysMonPsu_RawToVoltage(Supply1RawData);
	Supply3Data	= XSysMonPsu_RawToVoltage(Supply3RawData);


	printf("\r\nThe Current Temperature is %0d.%03d Centigrade.\r\n",
			(int)(TempData), SysMonPsuFractionToInt(TempData));
	printf("\r\nThe Current Supply 1 is %0d.%03d Volts. \r\n",
			(int)(Supply1Data), SysMonPsuFractionToInt(Supply1Data));
	printf("\r\nThe Current Supply 3 is %0d.%03d Volts. \r\n",
			(int)(Supply3Data), SysMonPsuFractionToInt(Supply3Data));

	/* Disable all the alarms in the Configuration Register 1. */
	XSysMonPsu_SetAlarmEnables(SysMonInstPtr, 0x0, XSYSMON_TYPE);

	/*
	 * Set up Alarm threshold registers for the on-chip temperature and
	 * Supply 1/Supply 3 High limit and lower limit so that the alarms
	 * DONOT occur.
	 */
	XSysMonPsu_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_TEMP_UPPER,
				XSysMonPsu_TemperatureToRaw_OnChip(TEST_TEMP_UPPER), XSYSMON_TYPE);
	XSysMonPsu_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_TEMP_LOWER,
				XSysMonPsu_TemperatureToRaw_OnChip(TEST_TEMP_LOWER), XSYSMON_TYPE);

	XSysMonPsu_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_SUP1_UPPER,
				XSysMonPsu_VoltageToRaw(TEST_Supply_1_UPPER), XSYSMON_TYPE);
	XSysMonPsu_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_SUP1_LOWER,
				XSysMonPsu_VoltageToRaw(TEST_Supply_1_LOWER), XSYSMON_TYPE);

	XSysMonPsu_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_SUP3_UPPER,
				XSysMonPsu_VoltageToRaw(TEST_Supply_3_UPPER), XSYSMON_TYPE);
	XSysMonPsu_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_SUP3_LOWER,
				XSysMonPsu_VoltageToRaw(TEST_Supply_3_LOWER), XSYSMON_TYPE);

	/* Setup the interrupt system. */
	Status = SysMonPsuSetupInterruptSystem(XScuGicInstPtr,
					    SysMonInstPtr,
					    SysMonIntrId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Clear any bits set in the Interrupt Status Register. */
	IntrStatus = XSysMonPsu_IntrGetStatus(SysMonInstPtr);
	XSysMonPsu_IntrClear(SysMonInstPtr, IntrStatus);

	/*
	 * Set up Alarm threshold registers for
	 * 	- On-chip Temperature High/Low limit
	 * 	- Supply 1 High/Low limit
	 *	- Supply 3 High/Low limit
	 * so that the Alarms occur.
	 */
	XSysMonPsu_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_TEMP_UPPER,
				XSysMonPsu_TemperatureToRaw_OnChip(TempData - 10), XSYSMON_TYPE);
	XSysMonPsu_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_TEMP_LOWER,
				XSysMonPsu_TemperatureToRaw_OnChip(TempData - 20), XSYSMON_TYPE);

	XSysMonPsu_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_SUP1_UPPER,
				XSysMonPsu_VoltageToRaw(Supply1Data - 0.2), XSYSMON_TYPE);
	XSysMonPsu_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_SUP1_LOWER,
				XSysMonPsu_VoltageToRaw(Supply1Data + 0.2), XSYSMON_TYPE);

	XSysMonPsu_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_SUP3_UPPER,
				XSysMonPsu_VoltageToRaw(Supply3Data - 0.2), XSYSMON_TYPE);
	XSysMonPsu_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_SUP3_LOWER,
				XSysMonPsu_VoltageToRaw(Supply3Data + 0.2), XSYSMON_TYPE);


	/* Read the Temperature Alarm Threshold registers. */
	Data	= XSysMonPsu_GetAlarmThreshold(SysMonInstPtr, XSM_ATR_TEMP_UPPER, XSYSMON_TYPE);
	MaxData	= XSysMonPsu_RawToTemperature_OnChip(Data);
	printf("\r\nTemperature Alarm(0) ");
	printf("HIGH Threshold is %0d.%03d Centigrade. \r\n",
			(int)(MaxData), SysMonPsuFractionToInt(MaxData));

	Data = XSysMonPsu_GetAlarmThreshold(SysMonInstPtr, XSM_ATR_TEMP_LOWER, XSYSMON_TYPE);
	MinData = XSysMonPsu_RawToTemperature_OnChip(Data);
	printf("Temperature Alarm(0) ");
	printf("LOW Threshold is %0d.%03d Centigrade. \r\n",
			(int)(MinData), SysMonPsuFractionToInt(MinData));

	/* Read the Supply 1 Alarm Threshold registers. */
	Data	= XSysMonPsu_GetAlarmThreshold(SysMonInstPtr,
			XSM_ATR_SUP1_UPPER, XSYSMON_TYPE);
	MaxData	= XSysMonPsu_RawToVoltage(Data);
	printf("Supply 1 Alarm(1) HIGH Threshold is %0d.%03d Volts. \r\n",
			(int)(MaxData), SysMonPsuFractionToInt(MaxData));

	Data	= XSysMonPsu_GetAlarmThreshold(SysMonInstPtr,
			XSM_ATR_SUP1_LOWER, XSYSMON_TYPE);
	MinData	= XSysMonPsu_RawToVoltage(Data);
	printf("Supply 1 Alarm(1) LOW Threshold is %0d.%03d Volts. \r\n",
			(int)(MinData), SysMonPsuFractionToInt(MinData));

	/* Read the Supply 3 Alarm Threshold registers. */
	Data	= XSysMonPsu_GetAlarmThreshold(SysMonInstPtr,
			XSM_ATR_SUP3_UPPER, XSYSMON_TYPE);
	MaxData	= XSysMonPsu_RawToVoltage(Data);
	printf("Supply 3 Alarm(3) HIGH Threshold is %0d.%03d Volts. \r\n",
			(int)(MaxData), SysMonPsuFractionToInt(MaxData));

	Data	= XSysMonPsu_GetAlarmThreshold(SysMonInstPtr,
			XSM_ATR_SUP3_LOWER, XSYSMON_TYPE);
	MinData	= XSysMonPsu_RawToVoltage(Data);
	printf("Supply 3 Alarm(3) LOW Threshold is %0d.%03d Volts. \r\n\r\n",
			(int)(MinData), SysMonPsuFractionToInt(MinData));


	/*
	 * Enable Alarm 0 for on-chip temperature , Alarm 1 for on-chip Supply 1
	 * and Alarm 3 for on-chip Supply 3 in the Configuration Register 1.
	 */
	XSysMonPsu_SetAlarmEnables(SysMonInstPtr, (XSM_CFR_ALM_TEMP_MASK |
						XSM_CFR_ALM_SUPPLY1_MASK |
						XSM_CFR_ALM_SUPPLY3_MASK), XSYSMON_TYPE);

	/*
	 * Enable Alarm 0 interrupt for on-chip temperature,
	 * Alarm 1 interrupt for on-chip Supply 1 and
	 * Alarm 3 interrupt for on-chip Supply 3.
	 */
#ifdef PL_EXAMPLE
	XSysMonPsu_IntrEnable(SysMonInstPtr,
			XSYSMONPSU_IER_0_PL_ALM_0_MASK |
			XSYSMONPSU_IER_0_PL_ALM_1_MASK |
			XSYSMONPSU_IER_0_PL_ALM_3_MASK );

#else
	XSysMonPsu_IntrEnable(SysMonInstPtr,
			XSYSMONPSU_IER_0_PS_ALM_0_MASK |
			XSYSMONPSU_IER_0_PS_ALM_1_MASK |
			XSYSMONPSU_IER_0_PS_ALM_3_MASK );
#endif

	/* Wait until an Alarm 0 or Alarm 1 or Alarm 3 interrupt occurs. */
	while (1) {
		if (TempIntrActive == TRUE) {
			/*
			 * Alarm 0 - Temperature alarm interrupt has occurred.
			 * The required processing should be put here.
			 */
			printf("Alarm 0 - Temperature alarm has occurred \r\n");
			break;
		}

		if (Supply1Intr == TRUE) {
			/*
			 * Alarm 1 - Supply 1 alarm interrupt has occurred.
			 * The required processing should be put here.
			 */
			printf("Alarm 1 - PS Supply 1LP alarm has occurred \r\n");
			break;
		}

		if (Supply3Intr == TRUE) {
			/*
			 * Alarm 3 - Supply 3 alarm interrupt has occurred.
			 * The required processing should be put here.
			 */
			printf("Alarm 3 - PS Supply 3 alarm has occurred \r\n");
			break;
		}
	}

	/*
	 * Read the on-chip Temperature Data (Current/Maximum/Minimum)
	 * from the ADC data registers.
	 */
	TempRawData	= XSysMonPsu_GetAdcData(SysMonInstPtr, XSM_CH_TEMP, XSYSMON_TYPE);
	TempData	= XSysMonPsu_RawToTemperature_OnChip(TempRawData);

	TempRawData	= XSysMonPsu_GetMinMaxMeasurement(SysMonInstPtr,
							XSM_MAX_TEMP, XSYSMON_TYPE);
	MaxData		= XSysMonPsu_RawToTemperature_OnChip(TempRawData);

	TempRawData	= XSysMonPsu_GetMinMaxMeasurement(SysMonInstPtr,
							XSM_MIN_TEMP, XSYSMON_TYPE);
	MinData		= XSysMonPsu_RawToTemperature_OnChip(TempRawData);

	printf("\r\nThe Current Temperature is %0d.%03d Centigrade.\r\n",
			(int)(TempData), SysMonPsuFractionToInt(TempData));
	printf("The Maximum Temperature is %0d.%03d Centigrade. \r\n",
			(int)(MaxData), SysMonPsuFractionToInt(MaxData));
	printf("The Minimum Temperature is %0d.%03d Centigrade. \r\n",
			(int)(MinData), SysMonPsuFractionToInt(MinData));


	/*
	 * If
	 * Read the Supply 1 Votage Data (Current/Maximum/Minimum) from the
	 * ADC data registers.
	 */
	Supply1RawData	= XSysMonPsu_GetAdcData(SysMonInstPtr, XSM_CH_SUPPLY1, XSYSMON_TYPE);
	Supply1Data		= XSysMonPsu_RawToVoltage(Supply1RawData);

	Supply1RawData	= XSysMonPsu_GetMinMaxMeasurement(SysMonInstPtr,
							XSM_MAX_SUPPLY1, XSYSMON_TYPE);
	MaxData			= XSysMonPsu_RawToVoltage(Supply1RawData);

	Supply1RawData	= XSysMonPsu_GetMinMaxMeasurement(SysMonInstPtr,
							XSM_MIN_SUPPLY1, XSYSMON_TYPE);
	MinData			= XSysMonPsu_RawToVoltage(Supply1RawData);

	printf("\r\nThe Current Supply 1 is %0d.%03d Volts. \r\n",
			(int)(Supply1Data), SysMonPsuFractionToInt(Supply1Data));
	printf("The Maximum Supply 1 is %0d.%03d Volts. \r\n",
			(int)(MaxData), SysMonPsuFractionToInt(MaxData));
	printf("The Minimum Supply 1 is %0d.%03d Volts. \r\n",
			(int)(MinData), SysMonPsuFractionToInt(MinData));


	/*
	 * Read the Supply 3 Voltage Data (Current/Maximum/Minimum) from the
	 * ADC data registers.
	 */
	Supply3RawData	= XSysMonPsu_GetAdcData(SysMonInstPtr, XSM_CH_SUPPLY3, XSYSMON_TYPE);
	Supply3Data		= XSysMonPsu_RawToVoltage(Supply3RawData);

	Supply3RawData	= XSysMonPsu_GetMinMaxMeasurement(SysMonInstPtr,
							XSM_MAX_SUPPLY3, XSYSMON_TYPE);
	MaxData			= XSysMonPsu_RawToVoltage(Supply3RawData);

	Supply3RawData	= XSysMonPsu_GetMinMaxMeasurement(SysMonInstPtr,
							XSM_MIN_SUPPLY3, XSYSMON_TYPE);
	MinData			= XSysMonPsu_RawToVoltage(Supply3RawData);

	printf("\r\nThe Current Supply 3 is %0d.%03d Volts. \r\n",
			(int)(Supply3Data), SysMonPsuFractionToInt(Supply3Data));
	printf("The Maximum Supply 3 is %0d.%03d Volts. \r\n",
				(int)(MaxData), SysMonPsuFractionToInt(MaxData));
	printf("The Minimum Supply 3 is %0d.%03d Volts. \r\n\r\n",
				(int)(MinData), SysMonPsuFractionToInt(MinData));

	printf("Exiting the SysMon Interrupt Example. \r\n");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is the Interrupt Service Routine for the System Monitor device.
* It will be called by the processor whenever an interrupt is asserted
* by the device.
*
* This function only handles ALARM 0, ALARM 1 and ALARM 2 interrupts.
* User of this code may need to modify the code to meet the needs of the
* application.
*
* @param	CallBackRef is the callback reference passed from the Interrupt
*		controller driver, which in our case is a pointer to the
*		driver instance.
*
* @return	None.
*
* @note		This function is called within interrupt context.
*
******************************************************************************/
static void SysMonPsuInterruptHandler(void *CallBackRef)
{
	u64 IntrStatusValue;
	XSysMonPsu *SysMonPtr = (XSysMonPsu *)CallBackRef;

	/* Get the interrupt status from the device and check the value. */
	IntrStatusValue = XSysMonPsu_IntrGetStatus(SysMonPtr);

#ifdef PL_EXAMPLE
	if (IntrStatusValue & XSYSMONPSU_ISR_0_PL_ALM_0_MASK) {
		/*
		 * Set Temperature interrupt flag so the code
		 * in application context can be aware of this interrupt.
		 */
		TempIntrActive = TRUE;
	}

	if (IntrStatusValue & XSYSMONPSU_ISR_0_PL_ALM_1_MASK) {
		/*
		 * Set Supply 1 interrupt flag so the code in application context
		 * can be aware of this interrupt.
		 */
		Supply1Intr = TRUE;
	}

	if (IntrStatusValue & XSYSMONPSU_ISR_0_PL_ALM_3_MASK) {
		/*
		 * Set Supply 3 interrupt flag so the code in application context
		 * can be aware of this interrupt.
		 */
		Supply3Intr = TRUE;
	}

#else

	if (IntrStatusValue & XSYSMONPSU_ISR_0_PS_ALM_0_MASK) {
		/*
		 * Set Temperature interrupt flag so the code
		 * in application context can be aware of this interrupt.
		 */
		TempIntrActive = TRUE;
	}

	if (IntrStatusValue & XSYSMONPSU_ISR_0_PS_ALM_1_MASK) {
		/*
		 * Set Supply 1 interrupt flag so the code in application context
		 * can be aware of this interrupt.
		 */
		Supply1Intr = TRUE;
	}

	if (IntrStatusValue & XSYSMONPSU_ISR_0_PS_ALM_3_MASK) {
		/*
		 * Set Supply 3 interrupt flag so the code in application context
		 * can be aware of this interrupt.
		 */
		Supply3Intr = TRUE;
	}
#endif
	/* Clear all bits in Interrupt Status Register. */
	XSysMonPsu_IntrClear(SysMonPtr, IntrStatusValue);

	/*
	 * Disable Alarm 0 interrupt for on-chip temperature,
	 * Alarm 1 interrupt for on-chip Supply 1 and
	 * Alarm 3 interrupt for on-chip Supply 3.
	 */
#ifdef PL_EXAMPLE
	XSysMonPsu_IntrDisable(SysMonPtr,
			XSYSMONPSU_IER_0_PL_ALM_0_MASK |
			XSYSMONPSU_IER_0_PL_ALM_1_MASK |
			XSYSMONPSU_IER_0_PL_ALM_3_MASK );
#else
	XSysMonPsu_IntrDisable(SysMonPtr,
			XSYSMONPSU_IER_0_PS_ALM_0_MASK |
			XSYSMONPSU_IER_0_PS_ALM_1_MASK |
			XSYSMONPSU_IER_0_PS_ALM_3_MASK );
#endif
 }

/****************************************************************************/
/**
*
* This function sets up the interrupt system so interrupts can occur for the
* System Monitor.  The function is application-specific since the actual
* system may or may not have an interrupt controller. The System Monitor
* device could be directly connected to a processor without an interrupt
* controller. The user should modify this function to fit the application.
*
* @param	XScuGicInstPtr is a pointer to the Interrupt Controller driver
*		Instance.
* @param	SysMonPtr is a pointer to the driver instance for the System
* 		Monitor device which is going to be connected to the interrupt
*		controller.
* @param	IntrId is XPAR_<SYSMON_instance>_INTR
*		value from xparameters_ps.h
*
* @return	XST_SUCCESS if successful, or XST_FAILURE.
*
* @note		None.
*
*
****************************************************************************/
static int SysMonPsuSetupInterruptSystem(XScuGic* XScuGicInstPtr, XSysMonPsu *SysMonPtr,
				      u16 IntrId )
{
	int Status;

#ifndef TESTAPP_GEN
	XScuGic_Config *XScuGicConfig; /* Config for interrupt controller */

	/* Initialize the interrupt controller driver */
	XScuGicConfig = XScuGic_LookupConfig(XSCUGIC_DEVICE_ID);
	if (NULL == XScuGicConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(XScuGicInstPtr, XScuGicConfig,
					XScuGicConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the interrupt controller interrupt handler to the
	 * hardware interrupt handling logic in the processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler) XScuGic_InterruptHandler,
				XScuGicInstPtr);
#endif

	/*
	 * Connect a device driver handler that will be called when an
	 * interrupt for the device occurs, the device driver handler
	 * performs the specific interrupt processing for the device
	 */
	Status = XScuGic_Connect(XScuGicInstPtr, IntrId,
				  (Xil_ExceptionHandler) SysMonPsuInterruptHandler,
				  (void *) SysMonPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Enable the interrupt for the device */
	XScuGic_Enable(XScuGicInstPtr, IntrId);

#ifndef TESTAPP_GEN
	/* Enable interrupts */
	 Xil_ExceptionEnable();
#endif
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
int SysMonPsuFractionToInt(float FloatNum)
{
	float Temp;

	Temp = FloatNum;
	if (FloatNum < 0) {
		Temp = -(FloatNum);
	}

	return( ((int)((Temp -(float)((int)Temp)) * (1000.0f))));
}
