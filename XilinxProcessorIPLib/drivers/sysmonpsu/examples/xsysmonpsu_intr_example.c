/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
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
* The values of the on-chip Temperature, VccInt voltage and VccAux voltage are
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
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xsysmonpsu.h"
#include "xparameters.h"
#include "xstatus.h"
#include "xil_exception.h"
#include "xscugic.h"
#include "xil_printf.h"

/************************** Constant Definitions ****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define SYSMON_DEVICE_ID	XPAR_XSYSMONPSU_0_DEVICE_ID

/* SCUGIC Interrupt Controller */
#define XSCUGIC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define INTR_ID			XPAR_XSYSMONPSU_INTR


/*
 * The following are the definitions of the Alarm Limits to be programmed to
 * the threshold registers. The user needs to change these according to the
 * needs of the application.
 */
#define TEST_TEMP_UPPER		85.0f /* Temperature Upper Alarm Limit */
#define TEST_TEMP_LOWER		65.0f /* Temperature Lower Alarm Limit */

#define TEST_VCCINT_UPPER	1.05f /* VccInt Upper Alarm Limit */
#define TEST_VCCINT_LOWER	0.95f /* VccInt Lower Alarm Limit */

#define TEST_VCCAUX_UPPER	2.625f /* VccAux Upper Alarm Limit */
#define TEST_VCCAUX_LOWER	2.375f /* VccAux Lower Alarm Limit */

#define printf xil_printf 	/* Small foot-print printf function */

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/

static int SysMonPsuIntrExample(XScuGic* XScuGicInstPtr,
			XSysMonPsu* SysMonInstPtr,
			u16 SysMonDeviceId,
			u16 SysMonIntrId);


static void SysMonPsuInterruptHandler(void *CallBackRef);

static int SysMonPsuSetupInterruptSystem(XScuGic* XScuGicInstPtr,
				      XSysMonPsu *SysMonPtr,
				      u16 IntrId );

static int SysMonPsuFractionToInt(float FloatNum);

/************************** Variable Definitions ****************************/

static XSysMonPsu SysMonInst; 		/* System Monitor driver instance */
static XScuGic XScuGicInst; 			/* Instance of the XXScuGic driver */

/* Shared variables used to test the callbacks. */
volatile static int TempIntrActive = FALSE;	/* Temperature alarm intr active */
volatile static int VccIntIntr = FALSE;		/* VCCINT alarm interrupt */
volatile static int VccAuxIntr = FALSE;		/* VCCAUX alarm interrupt */

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

	xil_printf("Successfully ran SysMonPsu Interrupt Example Test\r\n");
	return XST_SUCCESS;
}

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
*	- Set up alarms for on-chip temperature, VCCINT and VCCAUX
*	- Set up sequence registers to continuously monitor on-chip
*	temperature, VCCINT and  VCCAUX
*	- Setup interrupt system
*	- Enable interrupts
*	- Set up configuration registers to start the sequence
*	- Wait until temperature alarm interrupt or VCCINT alarm interrupt
*	or VCCAUX alarm interrupt occurs
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
static int SysMonPsuIntrExample(XScuGic* XScuGicInstPtr, XSysMonPsu* SysMonInstPtr,
			u16 SysMonDeviceId, u16 SysMonIntrId)
{
	int Status;
	XSysMonPsu_Config *ConfigPtr;
	u32 Data;
	u32 TempRawData;
	u32 VccAuxRawData;
	u32 VccIntRawData;
	float TempData;
	float VccAuxData;
	float VccIntData;
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
	XSysMonPsu_SetSequencerMode(SysMonInstPtr, XSM_SEQ_MODE_SAFE, XSYSMON_PS);

	/*
	 * Setup the Averaging to be done for the channels in the
	 * Configuration 0 register as 16 samples:
	 */
	XSysMonPsu_SetAvg(SysMonInstPtr, XSM_AVG_16_SAMPLES, XSYSMON_PS);

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
	Status = XSysMonPsu_SetSeqInputMode(SysMonInstPtr,
					XSYSMONPSU_SEQ_CH1_VAUX00_MASK << 16, XSYSMON_PS);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XSysMonPsu_SetSeqAcqTime(SysMonInstPtr,
			(XSYSMONPSU_SEQ_CH1_VAUX0F_MASK | XSYSMONPSU_SEQ_CH1_VAUX00_MASK) << 16,
			XSYSMON_PS);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the averaging on the following channels in the Sequencer
	 * registers:
	 * 	- On-chip Temperature
	 * 	- On-chip VCCINT supply sensor
	 *	- On-chip VCCAUX supply sensor
	 * 	- 1st Auxiliary Channel
	 * 	- 16th Auxiliary Channel
	 *	- Calibration Channel
	 */
	Status =  XSysMonPsu_SetSeqAvgEnables(SysMonInstPtr, XSYSMONPSU_SEQ_CH0_TEMP_MASK |
			XSYSMONPSU_SEQ_CH0_SUP1_MASK |
			XSYSMONPSU_SEQ_CH0_SUP3_MASK |
			((XSYSMONPSU_SEQ_CH1_VAUX00_MASK |
			XSYSMONPSU_SEQ_CH1_VAUX0F_MASK) << 16) |
			XSYSMONPSU_SEQ_CH0_CALIBRTN_MASK, XSYSMON_PS);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the following channels in the Sequencer registers:
	 * 	- On-chip Temperature
	 * 	- On-chip VCCINT supply sensor
	 * 	- On-chip VCCAUX supply sensor
	 * 	- 1st Auxiliary Channel
	 * 	- 16th Auxiliary Channel
	 *	- Calibration Channel
	 */
	Status =  XSysMonPsu_SetSeqChEnables(SysMonInstPtr, XSYSMONPSU_SEQ_CH0_TEMP_MASK |
			XSYSMONPSU_SEQ_CH0_SUP1_MASK |
			XSYSMONPSU_SEQ_CH0_SUP3_MASK |
			((XSYSMONPSU_SEQ_CH1_VAUX00_MASK |
			XSYSMONPSU_SEQ_CH1_VAUX0F_MASK) << 16) |
			XSYSMONPSU_SEQ_CH0_CALIBRTN_MASK, XSYSMON_PS);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the ADCCLK frequency equal to 1/32 of System clock for the System
	 * Monitor/ADC in the Configuration Register 2.
	 */
	XSysMonPsu_SetAdcClkDivisor(SysMonInstPtr, 32, XSYSMON_PS);

	/* Enable the Channel Sequencer in continuous sequencer cycling mode. */
	XSysMonPsu_SetSequencerMode(SysMonInstPtr, XSM_SEQ_MODE_CONTINPASS,
			XSYSMON_PS);

	/* Wait till the End of Sequence occurs. */
	while ((XSysMonPsu_IntrGetStatus(SysMonInstPtr) & ((u64)XSYSMONPSU_ISR_1_EOS_MASK<< 32)) !=
			((u64)XSYSMONPSU_ISR_1_EOS_MASK<< 32));

	/*
	 * Read the ADC converted Data from the data registers for on-chip
	 * temperature, on-chip VCCINT voltage and on-chip VCCAUX voltage.
	 */
	TempRawData	= XSysMonPsu_GetAdcData(SysMonInstPtr, XSM_CH_TEMP, XSYSMON_PS);
	VccIntRawData	= XSysMonPsu_GetAdcData(SysMonInstPtr, XSM_CH_SUPPLY1, XSYSMON_PS);
	VccAuxRawData	= XSysMonPsu_GetAdcData(SysMonInstPtr, XSM_CH_SUPPLY3, XSYSMON_PS);

	/* Convert the Raw Data to Degrees Centigrade and Voltage. */
	TempData	= XSysMonPsu_RawToTemperature_OnChip(TempRawData);
	VccIntData	= XSysMonPsu_RawToVoltage(VccIntRawData);
	VccAuxData	= XSysMonPsu_RawToVoltage(VccAuxRawData);


	printf("\r\nThe Current Temperature is %0d.%03d Centigrade.\r\n",
			(int)(TempData), SysMonPsuFractionToInt(TempData));
	printf("\r\nThe Current VCCINT is %0d.%03d Volts. \r\n",
			(int)(VccIntData), SysMonPsuFractionToInt(VccIntData));
	printf("\r\nThe Current VCCAUX is %0d.%03d Volts. \r\n",
			(int)(VccAuxData), SysMonPsuFractionToInt(VccAuxData));

	/* Disable all the alarms in the Configuration Register 1. */
	XSysMonPsu_SetAlarmEnables(SysMonInstPtr, 0x0, XSYSMON_PS);

	/*
	 * Set up Alarm threshold registers for the on-chip temperature and
	 * VCCAUX/VCCINT High limit and lower limit so that the alarms
	 * DONOT occur.
	 */
	XSysMonPsu_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_TEMP_UPPER,
				XSysMonPsu_TemperatureToRaw_OnChip(TEST_TEMP_UPPER), XSYSMON_PS);
	XSysMonPsu_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_TEMP_LOWER,
				XSysMonPsu_TemperatureToRaw_OnChip(TEST_TEMP_LOWER), XSYSMON_PS);

	XSysMonPsu_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_SUP1_UPPER,
				XSysMonPsu_VoltageToRaw(TEST_VCCINT_UPPER), XSYSMON_PS);
	XSysMonPsu_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_SUP1_LOWER,
				XSysMonPsu_VoltageToRaw(TEST_VCCINT_LOWER), XSYSMON_PS);

	XSysMonPsu_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_SUP3_UPPER,
				XSysMonPsu_VoltageToRaw(TEST_VCCAUX_UPPER), XSYSMON_PS);
	XSysMonPsu_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_SUP3_LOWER,
				XSysMonPsu_VoltageToRaw(TEST_VCCAUX_LOWER), XSYSMON_PS);

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
	 * 	- VCCINT High/Low limit
	 *	- VCCAUX High/Low limit
	 * so that the Alarms occur.
	 */
	XSysMonPsu_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_TEMP_UPPER,
				XSysMonPsu_TemperatureToRaw_OnChip(TempData - 10), XSYSMON_PS);
	XSysMonPsu_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_TEMP_LOWER,
				XSysMonPsu_TemperatureToRaw_OnChip(TempData - 20), XSYSMON_PS);

	XSysMonPsu_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_SUP1_UPPER,
				XSysMonPsu_VoltageToRaw(VccIntData - 0.2), XSYSMON_PS);
	XSysMonPsu_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_SUP1_LOWER,
				XSysMonPsu_VoltageToRaw(VccIntData + 0.2), XSYSMON_PS);

	XSysMonPsu_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_SUP3_UPPER,
				XSysMonPsu_VoltageToRaw(VccAuxData - 0.2), XSYSMON_PS);
	XSysMonPsu_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_SUP3_LOWER,
				XSysMonPsu_VoltageToRaw(VccAuxData + 0.2), XSYSMON_PS);


	/* Read the Temperature Alarm Threshold registers. */
	Data	= XSysMonPsu_GetAlarmThreshold(SysMonInstPtr, XSM_ATR_TEMP_UPPER, XSYSMON_PS);
	MaxData	= XSysMonPsu_RawToTemperature_OnChip(Data);
	printf("\r\nTemperature Alarm(0) ");
	printf("HIGH Threshold is %0d.%03d Centigrade. \r\n",
			(int)(MaxData), SysMonPsuFractionToInt(MaxData));

	Data = XSysMonPsu_GetAlarmThreshold(SysMonInstPtr, XSM_ATR_TEMP_LOWER, XSYSMON_PS);
	MinData = XSysMonPsu_RawToTemperature_OnChip(Data);
	printf("Temperature Alarm(0) ");
	printf("LOW Threshold is %0d.%03d Centigrade. \r\n",
			(int)(MinData), SysMonPsuFractionToInt(MinData));

	/* Read the VCCINT Alarm Threshold registers. */
	Data	= XSysMonPsu_GetAlarmThreshold(SysMonInstPtr,
			XSM_ATR_SUP1_UPPER, XSYSMON_PS);
	MaxData	= XSysMonPsu_RawToVoltage(Data);
	printf("VCCINT Alarm(1) HIGH Threshold is %0d.%03d Volts. \r\n",
			(int)(MaxData), SysMonPsuFractionToInt(MaxData));

	Data	= XSysMonPsu_GetAlarmThreshold(SysMonInstPtr,
			XSM_ATR_SUP1_LOWER, XSYSMON_PS);
	MinData	= XSysMonPsu_RawToVoltage(Data);
	printf("VCCINT Alarm(1) LOW Threshold is %0d.%03d Volts. \r\n",
			(int)(MinData), SysMonPsuFractionToInt(MinData));

	/* Read the VCCAUX Alarm Threshold registers. */
	Data	= XSysMonPsu_GetAlarmThreshold(SysMonInstPtr,
			XSM_ATR_SUP3_UPPER, XSYSMON_PS);
	MaxData	= XSysMonPsu_RawToVoltage(Data);
	printf("VCCAUX Alarm(3) HIGH Threshold is %0d.%03d Volts. \r\n",
			(int)(MaxData), SysMonPsuFractionToInt(MaxData));

	Data	= XSysMonPsu_GetAlarmThreshold(SysMonInstPtr,
			XSM_ATR_SUP3_LOWER, XSYSMON_PS);
	MinData	= XSysMonPsu_RawToVoltage(Data);
	printf("VCCAUX Alarm(3) LOW Threshold is %0d.%03d Volts. \r\n\r\n",
			(int)(MinData), SysMonPsuFractionToInt(MinData));


	/*
	 * Enable Alarm 0 for on-chip temperature , Alarm 1 for on-chip VCCINT
	 * and Alarm 3 for on-chip VCCAUX in the Configuration Register 1.
	 */
	XSysMonPsu_SetAlarmEnables(SysMonInstPtr, (XSM_CFR_ALM_TEMP_MASK |
						XSM_CFR_ALM_SUPPLY1_MASK |
						XSM_CFR_ALM_SUPPLY3_MASK), XSYSMON_PS);

	/*
	 * Enable Alarm 0 interrupt for on-chip temperature,
	 * Alarm 1 interrupt for on-chip VCCINT and
	 * Alarm 3 interrupt for on-chip VCCAUX.
	 */
	XSysMonPsu_IntrEnable(SysMonInstPtr,
			XSYSMONPSU_IER_0_PS_ALM_0_MASK |
			XSYSMONPSU_IER_0_PS_ALM_1_MASK |
			XSYSMONPSU_IER_0_PS_ALM_3_MASK );

	/* Wait until an Alarm 0 or Alarm 1 or Alarm 3 interrupt occurs. */
	while (1) {
		if (TempIntrActive == TRUE) {
			/*
			 * Alarm 0 - Temperature alarm interrupt has occurred.
			 * The required processing should be put here.
			 */
			printf("Alarm 0 - Temperature alarm has occured \r\n");
			break;
		}

		if (VccIntIntr == TRUE) {
			/*
			 * Alarm 1 - VCCINT alarm interrupt has occurred.
			 * The required processing should be put here.
			 */
			printf("Alarm 1 - PS VCCINTLP alarm has occured \r\n");
			break;
		}

		if (VccAuxIntr == TRUE) {
			/*
			 * Alarm 3 - VCCAUX alarm interrupt has occurred.
			 * The required processing should be put here.
			 */
			printf("Alarm 3 - PS VCCAUX alarm has occured \r\n");
			break;
		}
	}

	/*
	 * Read the on-chip Temperature Data (Current/Maximum/Minimum)
	 * from the ADC data registers.
	 */
	TempRawData	= XSysMonPsu_GetAdcData(SysMonInstPtr, XSM_CH_TEMP, XSYSMON_PS);
	TempData	= XSysMonPsu_RawToTemperature_OnChip(TempRawData);

	TempRawData	= XSysMonPsu_GetMinMaxMeasurement(SysMonInstPtr,
							XSM_MAX_TEMP, XSYSMON_PS);
	MaxData		= XSysMonPsu_RawToTemperature_OnChip(TempRawData);

	TempRawData	= XSysMonPsu_GetMinMaxMeasurement(SysMonInstPtr,
							XSM_MIN_TEMP, XSYSMON_PS);
	MinData		= XSysMonPsu_RawToTemperature_OnChip(TempRawData);

	printf("\r\nThe Current Temperature is %0d.%03d Centigrade.\r\n",
			(int)(TempData), SysMonPsuFractionToInt(TempData));
	printf("The Maximum Temperature is %0d.%03d Centigrade. \r\n",
			(int)(MaxData), SysMonPsuFractionToInt(MaxData));
	printf("The Minimum Temperature is %0d.%03d Centigrade. \r\n",
			(int)(MinData), SysMonPsuFractionToInt(MinData));


	/*
	 * Read the VccInt Votage Data (Current/Maximum/Minimum) from the
	 * ADC data registers.
	 */
	VccIntRawData	= XSysMonPsu_GetAdcData(SysMonInstPtr, XSM_CH_SUPPLY1, XSYSMON_PS);
	VccIntData		= XSysMonPsu_RawToVoltage(VccIntRawData);

	VccIntRawData	= XSysMonPsu_GetMinMaxMeasurement(SysMonInstPtr,
							XSM_MAX_SUPPLY1, XSYSMON_PS);
	MaxData			= XSysMonPsu_RawToVoltage(VccIntRawData);

	VccIntRawData	= XSysMonPsu_GetMinMaxMeasurement(SysMonInstPtr,
							XSM_MIN_SUPPLY1, XSYSMON_PS);
	MinData			= XSysMonPsu_RawToVoltage(VccIntRawData);

	printf("\r\nThe Current VCCINT is %0d.%03d Volts. \r\n",
			(int)(VccIntData), SysMonPsuFractionToInt(VccIntData));
	printf("The Maximum VCCINT is %0d.%03d Volts. \r\n",
			(int)(MaxData), SysMonPsuFractionToInt(MaxData));
	printf("The Minimum VCCINT is %0d.%03d Volts. \r\n",
			(int)(MinData), SysMonPsuFractionToInt(MinData));


	/*
	 * Read the VccAux Voltage Data (Current/Maximum/Minimum) from the
	 * ADC data registers.
	 */
	VccAuxRawData	= XSysMonPsu_GetAdcData(SysMonInstPtr, XSM_CH_SUPPLY3, XSYSMON_PS);
	VccAuxData		= XSysMonPsu_RawToVoltage(VccAuxRawData);

	VccAuxRawData	= XSysMonPsu_GetMinMaxMeasurement(SysMonInstPtr,
							XSM_MAX_SUPPLY3, XSYSMON_PS);
	MaxData			= XSysMonPsu_RawToVoltage(VccAuxRawData);

	VccAuxRawData	= XSysMonPsu_GetMinMaxMeasurement(SysMonInstPtr,
							XSM_MIN_SUPPLY3, XSYSMON_PS);
	MinData			= XSysMonPsu_RawToVoltage(VccAuxRawData);

	printf("\r\nThe Current VCCAUX is %0d.%03d Volts. \r\n",
			(int)(VccAuxData), SysMonPsuFractionToInt(VccAuxData));
	printf("The Maximum VCCAUX is %0d.%03d Volts. \r\n",
				(int)(MaxData), SysMonPsuFractionToInt(MaxData));
	printf("The Minimum VCCAUX is %0d.%03d Volts. \r\n\r\n",
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

	if (IntrStatusValue & XSYSMONPSU_ISR_0_PS_ALM_0_MASK) {
		/*
		 * Set Temperature interrupt flag so the code
		 * in application context can be aware of this interrupt.
		 */
		TempIntrActive = TRUE;
	}

	if (IntrStatusValue & XSYSMONPSU_ISR_0_PS_ALM_1_MASK) {
		/*
		 * Set VCCINT interrupt flag so the code in application context
		 * can be aware of this interrupt.
		 */
		VccIntIntr = TRUE;
	}

	if (IntrStatusValue & XSYSMONPSU_ISR_0_PS_ALM_3_MASK) {
		/*
		 * Set VCCAUX interrupt flag so the code in application context
		 * can be aware of this interrupt.
		 */
		VccAuxIntr = TRUE;
	}

	/* Clear all bits in Interrupt Status Register. */
	XSysMonPsu_IntrClear(SysMonPtr, IntrStatusValue);

	/*
	 * Disable Alarm 0 interrupt for on-chip temperature,
	 * Alarm 1 interrupt for on-chip VCCINT and
	 * Alarm 3 interrupt for on-chip VCCAUX.
	 */
	XSysMonPsu_IntrDisable(SysMonPtr,
			XSYSMONPSU_IER_0_PS_ALM_0_MASK |
			XSYSMONPSU_IER_0_PS_ALM_1_MASK |
			XSYSMONPSU_IER_0_PS_ALM_3_MASK );

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

	/* Enable interrupts */
	 Xil_ExceptionEnable();

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
