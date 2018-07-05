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
* @file xsysmon_intr_printf_example.c
*
* This file contains a design example using the driver functions
* of the System Monitor/ADC driver. This example here shows the usage of the
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
* 1.00a xd/sv  05/22/07 First release
* 2.00a sv     06/22/08 Added printfs and used conversion macros
* 4.00a ktn    10/22/09 Updated the example to use HAL Processor APIs/macros.
*		        Updated the example to use macros that have been
*		        renamed to remove _m from the name of the macro.
* 5.01a bss    03/13/12 Updated for Zynq.
* 5.03a bss    04/25/13 Modified SysMonIntrExample function to set
*			Sequencer Mode as Safe mode instead of Single
*			channel mode before configuring Sequencer registers.
*			CR #703729
* 7.3   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
*       ms   04/05/17 Modified Comment lines in functions to
*                     recognize it as documentation block for doxygen
*                     generation.
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xsysmon.h"
#include "xparameters.h"
#include "xstatus.h"
#include "stdio.h"
#include "xil_exception.h"

#ifdef XPAR_INTC_0_DEVICE_ID
#include "xintc.h"
#include <stdio.h>
#else
#include "xscugic.h"
#include "xil_printf.h"
#endif


/************************** Constant Definitions ****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define SYSMON_DEVICE_ID	XPAR_SYSMON_0_DEVICE_ID


#ifdef XPAR_INTC_0_DEVICE_ID	/* Interrupt Controller */
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#define INTR_ID			XPAR_INTC_0_SYSMON_0_VEC_ID
#else	/* SCUGIC Interrupt Controller */
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define INTR_ID		XPAR_FABRIC_SYSTEM_MANAGEMENT_WIZ_0_IP2INTC_IRPT_INTR
#endif /* XPAR_INTC_0_DEVICE_ID */



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

#ifdef XPAR_INTC_0_DEVICE_ID	/* Interrupt Controller */
#define INTC			XIntc
#define INTC_HANDLER		XIntc_InterruptHandler
#else	/* SCUGIC Interrupt Controller */
#define INTC			XScuGic
#define INTC_HANDLER		XScuGic_InterruptHandler
#endif /* XPAR_INTC_0_DEVICE_ID */



/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/

static int SysMonIntrExample(INTC* IntcInstPtr,
			XSysMon* SysMonInstPtr,
			u16 SysMonDeviceId,
			u16 SysMonIntrId);


static void SysMonInterruptHandler(void *CallBackRef);

static int SysMonSetupInterruptSystem(INTC* IntcInstPtr,
				      XSysMon *SysMonPtr,
				      u16 IntrId );

static int SysMonFractionToInt(float FloatNum);

/************************** Variable Definitions ****************************/

static XSysMon SysMonInst; 		/* System Monitor driver instance */
static INTC IntcInst; 			/* Instance of the XIntc driver */

/*
 * Shared variables used to test the callbacks.
 */
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
	Status = SysMonIntrExample(&IntcInst, &SysMonInst, SYSMON_DEVICE_ID,
								INTR_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Sysmon interrupt printf Example Failed\r\n");
		return XST_FAILURE;
	}
	xil_printf("Successfully ran Sysmon interrupt printf Example\r\n");
	return XST_SUCCESS;

}

/****************************************************************************/
/**
*
* This function runs a test on the System Monitor/ADC device using the
* driver APIs.
*
* The function does the following tasks:
*	- Initiate the System Monitor/ADC device driver instance
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
* @param	IntcInstPtr is a pointer to the Interrupt Controller
*		driver Instance.
* @param	SysMonInstPtr is a pointer to the XSysMon driver Instance.
* @param	SysMonDeviceId is the XPAR_<SYSMON_ADC_instance>_DEVICE_ID
*		value from xparameters.h.
* @param	SysMonIntrId is
*		XPAR_<INTC_instance>_<SYSMON_ADC_instance>_VEC_ID value from
*		xparameters.h.
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note		This function may never return if no interrupt occurs.
*
****************************************************************************/
static int SysMonIntrExample(INTC* IntcInstPtr, XSysMon* SysMonInstPtr,
			u16 SysMonDeviceId, u16 SysMonIntrId)
{
	int Status;
	XSysMon_Config *ConfigPtr;
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

	printf("\r\nEntering the SysMon Interrupt Example. \r\n");

	/*
	 * Initialize the SysMon driver.
	 */
	ConfigPtr = XSysMon_LookupConfig(SysMonDeviceId);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}
	XSysMon_CfgInitialize(SysMonInstPtr, ConfigPtr, ConfigPtr->BaseAddress);

	/*
	 * Self Test the System Monitor/ADC device.
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
	 * 	- On-chip VCCINT supply sensor
	 *	- On-chip VCCAUX supply sensor
	 * 	- 1st Auxiliary Channel
	 * 	- 16th Auxiliary Channel
	 *	- Calibration Channel
	 */
	Status =  XSysMon_SetSeqAvgEnables(SysMonInstPtr, XSM_SEQ_CH_TEMP |
						XSM_SEQ_CH_VCCINT |
						XSM_SEQ_CH_VCCAUX |
						XSM_SEQ_CH_AUX00 |
						XSM_SEQ_CH_AUX15 |
						XSM_SEQ_CH_CALIB);
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
	Status =  XSysMon_SetSeqChEnables(SysMonInstPtr, XSM_SEQ_CH_TEMP |
						XSM_SEQ_CH_VCCINT |
						XSM_SEQ_CH_VCCAUX |
						XSM_SEQ_CH_AUX00 |
						XSM_SEQ_CH_AUX15 |
						XSM_SEQ_CH_CALIB);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the ADCCLK frequency equal to 1/32 of System clock for the System
	 * Monitor/ADC in the Configuration Register 2.
	 */
	XSysMon_SetAdcClkDivisor(SysMonInstPtr, 32);

	/*
	 * Set the Calibration enables.
	 */
	XSysMon_SetCalibEnables(SysMonInstPtr,
				XSM_CFR1_CAL_PS_GAIN_OFFSET_MASK |
				XSM_CFR1_CAL_ADC_GAIN_OFFSET_MASK);

	/*
	 * Enable the Channel Sequencer in continuous sequencer cycling mode.
	 */
	XSysMon_SetSequencerMode(SysMonInstPtr, XSM_SEQ_MODE_CONTINPASS);

	/*
	 * Wait till the End of Sequence occurs.
	 */
	XSysMon_GetStatus(SysMonInstPtr); /* Clear the old status */
	while ((XSysMon_GetStatus(SysMonInstPtr) & XSM_SR_EOS_MASK) !=
			XSM_SR_EOS_MASK);

	/*
	 * Read the ADC converted Data from the data registers for on-chip
	 * temperature, on-chip VCCINT voltage and on-chip VCCAUX voltage.
	 */
	TempRawData	= XSysMon_GetAdcData(SysMonInstPtr, XSM_CH_TEMP);
	VccIntRawData	= XSysMon_GetAdcData(SysMonInstPtr, XSM_CH_VCCINT);
	VccAuxRawData	= XSysMon_GetAdcData(SysMonInstPtr, XSM_CH_VCCAUX);

	/*
	 * Convert the Raw Data to Degrees Centigrade and Voltage.
	 */
	TempData	= XSysMon_RawToTemperature(TempRawData);
	VccIntData	= XSysMon_RawToVoltage(VccIntRawData);
	VccAuxData	= XSysMon_RawToVoltage(VccAuxRawData);


	printf("\r\nThe Current Temperature is %0d.%03d Centigrade.\r\n",
			(int)(TempData), SysMonFractionToInt(TempData));
	printf("\r\nThe Current VCCINT is %0d.%03d Volts. \r\n",
			(int)(VccIntData), SysMonFractionToInt(VccIntData));
	printf("\r\nThe Current VCCAUX is %0d.%03d Volts. \r\n",
			(int)(VccAuxData), SysMonFractionToInt(VccAuxData));

	/*
	 * Disable all the alarms in the Configuration Register 1.
	 */
	XSysMon_SetAlarmEnables(SysMonInstPtr, 0x0);

	/*
	 * Set up Alarm threshold registers for the on-chip temperature and
	 * VCCAUX/VCCINT High limit and lower limit so that the alarms
	 * DONOT occur.
	 */
	XSysMon_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_TEMP_UPPER,
				XSysMon_TemperatureToRaw(TEST_TEMP_UPPER));
	XSysMon_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_TEMP_LOWER,
				XSysMon_TemperatureToRaw(TEST_TEMP_LOWER));

	XSysMon_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_VCCINT_UPPER,
				XSysMon_VoltageToRaw(TEST_VCCINT_UPPER));
	XSysMon_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_VCCINT_LOWER,
				XSysMon_VoltageToRaw(TEST_VCCINT_LOWER));

	XSysMon_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_VCCAUX_UPPER,
				XSysMon_VoltageToRaw(TEST_VCCAUX_UPPER));
	XSysMon_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_VCCAUX_LOWER,
				XSysMon_VoltageToRaw(TEST_VCCAUX_LOWER));

	/*
	 * Setup the interrupt system.
	 */
	Status = SysMonSetupInterruptSystem(IntcInstPtr,
					    SysMonInstPtr,
					    SysMonIntrId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Clear any bits set in the Interrupt Status Register.
	 */
	IntrStatus = XSysMon_IntrGetStatus(SysMonInstPtr);
	XSysMon_IntrClear(SysMonInstPtr, IntrStatus);


	/*
	 * Enable Alarm 0 interrupt for on-chip temperature,
	 * Alarm 1 interrupt for on-chip VCCINT and
	 * Alarm 2 interrupt for on-chip VCCAUX.
	 */
	XSysMon_IntrEnable(SysMonInstPtr,
				XSM_IPIXR_TEMP_MASK |
				XSM_IPIXR_VCCINT_MASK |
				XSM_IPIXR_VCCAUX_MASK );

	/*
	 * Enable global interrupt of System Monitor.
	 */
	XSysMon_IntrGlobalEnable(SysMonInstPtr);

	/*
	 * Set up Alarm threshold registers for
	 * 	- On-chip Temperature High/Low limit
	 * 	- VCCINT High/Low limit
	 *	- VCCAUX High/Low limit
	 * so that the Alarms occur.
	 */
	XSysMon_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_TEMP_UPPER,
				XSysMon_TemperatureToRaw(TempData - 10));
	XSysMon_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_TEMP_LOWER,
				XSysMon_TemperatureToRaw(TempData - 20));

	XSysMon_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_VCCINT_UPPER,
				XSysMon_VoltageToRaw(VccIntData - 0.2));
	XSysMon_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_VCCINT_LOWER,
				XSysMon_VoltageToRaw(VccIntData + 0.2));

	XSysMon_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_VCCAUX_UPPER,
				XSysMon_VoltageToRaw(VccAuxData - 0.2));
	XSysMon_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_VCCAUX_LOWER,
				XSysMon_VoltageToRaw(VccAuxData + 0.2));


	/*
	 * Read the Temperature Alarm Threshold registers.
	 */
	Data	= XSysMon_GetAlarmThreshold(SysMonInstPtr, XSM_ATR_TEMP_UPPER);
	MaxData	= XSysMon_RawToTemperature(Data);
	printf("\r\nTemperature Alarm(0) ");
	printf("HIGH Threshold is %0d.%03d Centigrade. \r\n",
			(int)(MaxData), SysMonFractionToInt(MaxData));

	Data = XSysMon_GetAlarmThreshold(SysMonInstPtr, XSM_ATR_TEMP_LOWER);
	MinData = XSysMon_RawToTemperature(Data);
	printf("Temperature Alarm(0) ");
	printf("LOW Threshold is %0d.%03d Centigrade. \r\n",
			(int)(MinData), SysMonFractionToInt(MinData));

	/*
	 * Read the VCCINT Alarm Threshold registers.
	 */
	Data	= XSysMon_GetAlarmThreshold(SysMonInstPtr,
							XSM_ATR_VCCINT_UPPER);
	MaxData	= XSysMon_RawToVoltage(Data);
	printf("VCCINT Alarm(1) HIGH Threshold is %0d.%03d Volts. \r\n",
			(int)(MaxData), SysMonFractionToInt(MaxData));

	Data	= XSysMon_GetAlarmThreshold(SysMonInstPtr,
							XSM_ATR_VCCINT_LOWER);
	MinData	= XSysMon_RawToVoltage(Data);
	printf("VCCINT Alarm(1) LOW Threshold is %0d.%03d Volts. \r\n",
			(int)(MinData), SysMonFractionToInt(MinData));

	/*
	 * Read the VCCAUX Alarm Threshold registers.
	 */
	Data	= XSysMon_GetAlarmThreshold(SysMonInstPtr,
							XSM_ATR_VCCAUX_UPPER);
	MaxData	= XSysMon_RawToVoltage(Data);
	printf("VCCAUX Alarm(2) HIGH Threshold is %0d.%03d Volts. \r\n",
			(int)(MaxData), SysMonFractionToInt(MaxData));

	Data	= XSysMon_GetAlarmThreshold(SysMonInstPtr,
							XSM_ATR_VCCAUX_LOWER);
	MinData	= XSysMon_RawToVoltage(Data);
	printf("VCCAUX Alarm(2) LOW Threshold is %0d.%03d Volts. \r\n\r\n",
			(int)(MinData), SysMonFractionToInt(MinData));


	/*
	 * Enable Alarm 0 for on-chip temperature , Alarm 1 for on-chip VCCINT
	 * and Alarm 2 for on-chip VCCAUX in the Configuration Register 1.
	 */
	XSysMon_SetAlarmEnables(SysMonInstPtr, (XSM_CFR1_ALM_TEMP_MASK |
						XSM_CFR1_ALM_VCCINT_MASK |
						XSM_CFR1_ALM_VCCAUX_MASK));

	/*
	 * Wait until an Alarm 0 or Alarm 1 or Alarm 2 interrupt occurs.
	 */
	while (1) {
		if (TempIntrActive == TRUE) {
			/*
			 * Alarm 0 - Temperature alarm interrupt has occurred.
			 * The required processing should be put here.
			 */
			printf("Alarm 0 - Temperature alarm has occurred \r\n");
			break;
		}

		if (VccIntIntr == TRUE) {
			/*
			 * Alarm 1 - VCCINT alarm interrupt has occurred.
			 * The required processing should be put here.
			 */
			printf("Alarm 1 - VCCINT alarm has occurred \r\n");
			break;
		}

		if (VccAuxIntr == TRUE) {
			/*
			 * Alarm 2 - VCCAUX alarm interrupt has occurred.
			 * The required processing should be put here.
			 */
			printf("Alarm 2 - VCCAUX alarm has occurred \r\n");
			break;
		}
	}

	/*
	 * Read the on-chip Temperature Data (Current/Maximum/Minimum)
	 * from the ADC data registers.
	 */
	TempRawData	= XSysMon_GetAdcData(SysMonInstPtr, XSM_CH_TEMP);
	TempData	= XSysMon_RawToTemperature(TempRawData);

	TempRawData	= XSysMon_GetMinMaxMeasurement(SysMonInstPtr,
							XSM_MAX_TEMP);
	MaxData		= XSysMon_RawToTemperature(TempRawData);

	TempRawData	= XSysMon_GetMinMaxMeasurement(SysMonInstPtr,
							XSM_MIN_TEMP);
	MinData		= XSysMon_RawToTemperature(TempRawData);

	printf("\r\nThe Current Temperature is %0d.%03d Centigrade.\r\n",
			(int)(TempData), SysMonFractionToInt(TempData));
	printf("The Maximum Temperature is %0d.%03d Centigrade. \r\n",
			(int)(MaxData), SysMonFractionToInt(MaxData));
	printf("The Minimum Temperature is %0d.%03d Centigrade. \r\n",
			(int)(MinData), SysMonFractionToInt(MinData));


	/*
	 * Read the VccInt Votage Data (Current/Maximum/Minimum) from the
	 * ADC data registers.
	 */
	VccIntRawData	= XSysMon_GetAdcData(SysMonInstPtr, XSM_CH_VCCINT);
	VccIntData		= XSysMon_RawToVoltage(VccIntRawData);

	VccIntRawData	= XSysMon_GetMinMaxMeasurement(SysMonInstPtr,
							XSM_MAX_VCCINT);
	MaxData			= XSysMon_RawToVoltage(VccIntRawData);

	VccIntRawData	= XSysMon_GetMinMaxMeasurement(SysMonInstPtr,
							XSM_MIN_VCCINT);
	MinData			= XSysMon_RawToVoltage(VccIntRawData);

	printf("\r\nThe Current VCCINT is %0d.%03d Volts. \r\n",
			(int)(VccIntData), SysMonFractionToInt(VccIntData));
	printf("The Maximum VCCINT is %0d.%03d Volts. \r\n",
			(int)(MaxData), SysMonFractionToInt(MaxData));
	printf("The Minimum VCCINT is %0d.%03d Volts. \r\n",
			(int)(MinData), SysMonFractionToInt(MinData));


	/*
	 * Read the VccAux Voltage Data (Current/Maximum/Minimum) from the
	 * ADC data registers.
	 */
	VccAuxRawData	= XSysMon_GetAdcData(SysMonInstPtr, XSM_CH_VCCAUX);
	VccAuxData		= XSysMon_RawToVoltage(VccAuxRawData);

	VccAuxRawData	= XSysMon_GetMinMaxMeasurement(SysMonInstPtr,
							XSM_MAX_VCCAUX);
	MaxData			= XSysMon_RawToVoltage(VccAuxRawData);

	VccAuxRawData	= XSysMon_GetMinMaxMeasurement(SysMonInstPtr,
							XSM_MIN_VCCAUX);
	MinData			= XSysMon_RawToVoltage(VccAuxRawData);

	printf("\r\nThe Current VCCAUX is %0d.%03d Volts. \r\n",
			(int)(VccAuxData), SysMonFractionToInt(VccAuxData));
	printf("The Maximum VCCAUX is %0d.%03d Volts. \r\n",
				(int)(MaxData), SysMonFractionToInt(MaxData));
	printf("The Minimum VCCAUX is %0d.%03d Volts. \r\n\r\n",
				(int)(MinData), SysMonFractionToInt(MinData));

	printf("Exiting the SysMon Interrupt Example. \r\n");

	/*
	 * Disable global interrupt of System Monitor.
	 */
	XSysMon_IntrGlobalDisable(SysMonInstPtr);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is the Interrupt Service Routine for the System Monitor device.
* It will be called by the processor whenever an interrupt is asserted
* by the device.
*
* There are 10 different interrupts supported
*	- Over Temperature
*	- ALARM 0
*	- ALARM 1
*	- ALARM 2
*	- End of Sequence
*	- End of Conversion
*	- JTAG Locked
*	- JATG Modified
*	- Over Temperature DeActive
*	- ALARM 0 DeActive
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
static void SysMonInterruptHandler(void *CallBackRef)
{
	u32 IntrStatusValue;
	XSysMon *SysMonPtr = (XSysMon *)CallBackRef;

	/*
	 * Get the interrupt status from the device and check the value.
	 */
	IntrStatusValue = XSysMon_IntrGetStatus(SysMonPtr);

	if (IntrStatusValue & XSM_IPIXR_TEMP_MASK) {
		/*
		 * Set Temperature interrupt flag so the code
		 * in application context can be aware of this interrupt.
		 */
		TempIntrActive = TRUE;
	}

	if (IntrStatusValue & XSM_IPIXR_VCCINT_MASK) {
		/*
		 * Set VCCINT interrupt flag so the code in application context
		 * can be aware of this interrupt.
		 */
		VccIntIntr = TRUE;
	}


	if (IntrStatusValue & XSM_IPIXR_VCCAUX_MASK) {
		/*
		 * Set VCCAUX interrupt flag so the code in application context
		 * can be aware of this interrupt.
		 */
		VccAuxIntr = TRUE;
	}

	/*
	 * Clear all bits in Interrupt Status Register.
	 */
	XSysMon_IntrClear(SysMonPtr, IntrStatusValue);
 }

/****************************************************************************/
/**
*
* This function sets up the interrupt system so interrupts can occur for the
* System Monitor/ADC.  The function is application-specific since the actual
* system may or may not have an interrupt controller. The System Monitor/ADC
* device could be directly connected to a processor without an interrupt
* controller. The user should modify this function to fit the application.
*
* @param	IntcInstPtr is a pointer to the Interrupt Controller driver
*		Instance.
* @param	SysMonPtr is a pointer to the driver instance for the System
* 		Monitor device which is going to be connected to the interrupt
*		controller.
* @param	IntrId is XPAR_<INTC_instance>_<SYSMON_ADC_instance>_VEC_ID
*		value from xparameters.h
*
* @return	XST_SUCCESS if successful, or XST_FAILURE.
*
* @note		None.
*
*
****************************************************************************/
static int SysMonSetupInterruptSystem(INTC* IntcInstPtr, XSysMon *SysMonPtr,
				      u16 IntrId )
{
	int Status;

#ifdef XPAR_INTC_0_DEVICE_ID
	/*
	 * Initialize the interrupt controller driver so that it's ready to
	 * use.
	 */
	Status = XIntc_Initialize(IntcInstPtr, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the handler that will be called when an interrupt
	 * for the device occurs, the handler defined above performs the
	 * specific interrupt processing for the device.
	 */
	Status = XIntc_Connect(IntcInstPtr,
				IntrId,
				(XInterruptHandler) SysMonInterruptHandler,
				SysMonPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the interrupt controller so interrupts are enabled for all
	 * devices that cause interrupts. Specify real mode so that the System
	 * Monitor/ACD device can cause interrupts through the interrupt
	 * controller.
	 */
	Status = XIntc_Start(IntcInstPtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the interrupt for the System Monitor/ADC device.
	 */
	XIntc_Enable(IntcInstPtr, IntrId);
#else

	XScuGic_Config *IntcConfig;

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(IntcInstPtr, IntcConfig,
					IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XScuGic_SetPriorityTriggerType(IntcInstPtr, IntrId, 0xA0, 0x3);

	/*
	 * Connect the interrupt handler that will be called when an
	 * interrupt occurs for the device.
	 */
	Status = XScuGic_Connect(IntcInstPtr, IntrId,
				 (Xil_ExceptionHandler)SysMonInterruptHandler,
				 SysMonPtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Enable the interrupt for the Timer device.
	 */
	XScuGic_Enable(IntcInstPtr, IntrId);

#endif


	/*
	 * Initialize the exception table.
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler) INTC_HANDLER,
				IntcInstPtr);

	/*
	 * Enable non-critical exceptions.
	 */
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
int SysMonFractionToInt(float FloatNum)
{
	float Temp;

	Temp = FloatNum;
	if (FloatNum < 0) {
		Temp = -(FloatNum);
	}

	return( ((int)((Temp -(float)((int)Temp)) * (1000.0f))));
}
