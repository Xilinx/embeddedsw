/******************************************************************************
*
* Copyright (C) 2011 - 2014 Xilinx, Inc.  All rights reserved.
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
* @file xadc_polled_printf_example.c
*
* This file contains a design example using the driver functions
* of the XADC driver. The example here shows the driver/device in polled mode
* to check the on-chip temperature and voltages.
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
* 1.00a ssb    12/22/11 First release based on the XPS/AXI SysMon driver
* 2.2   ms     01/23/17 Added xil_printf statement in main function to
*                       ensure that "Successfully ran" and "Failed" strings
*                       are available in all examples. This is a fix for
*                       CR-965028.
*       ms     04/05/17 Modified Comment lines in functions to
*                       recognize it as documentation block for doxygen
*                       generation.
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xparameters.h"
#include "xadcps.h"
#include "xstatus.h"
#include "stdio.h"
#include "xil_printf.h"

/************************** Constant Definitions ****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define XADC_DEVICE_ID 		XPAR_XADCPS_0_DEVICE_ID


/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/

#define printf xil_printf /* Small foot-print printf function */

/************************** Function Prototypes *****************************/

static int XAdcPolledPrintfExample(u16 XAdcDeviceId);
static int XAdcFractionToInt(float FloatNum);

/************************** Variable Definitions ****************************/

static XAdcPs XAdcInst;      /* XADC driver instance */

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
	 * Run the polled example, specify the Device ID that is
	 * generated in xparameters.h.
	 */
	Status = XAdcPolledPrintfExample(XADC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("adcps polled printf Example Failed\r\n");
		return XST_FAILURE;
	}
	xil_printf("Successfully ran adcps polled printf Example\r\n");
	return XST_SUCCESS;
}


/****************************************************************************/
/**
*
* This function runs a test on the XADC/ADC device using the
* driver APIs.
* This function does the following tasks:
*	- Initiate the XADC device driver instance
*	- Run self-test on the device
*	- Setup the sequence registers to continuously monitor on-chip
*	temperature and, VCCPINT, VCCPAUX and VCCPDRO Voltages
*	- Setup configuration registers to start the sequence
*	- Read the latest on-chip temperature and, VCCPINT, VCCPAUX and VCCPDRO
*	  Voltages
*
* @param	XAdcDeviceId is the XPAR_<SYSMON_ADC_instance>_DEVICE_ID value
*		from xparameters.h.
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note   	None
*
****************************************************************************/
int XAdcPolledPrintfExample(u16 XAdcDeviceId)
{
	int Status;
	XAdcPs_Config *ConfigPtr;
	u32 TempRawData;
	u32 VccPintRawData;
	u32 VccPauxRawData;
	u32 VccPdroRawData;
	float TempData;
	float VccPintData;
	float VccPauxData;
	float MaxData;
	float MinData;
	XAdcPs *XAdcInstPtr = &XAdcInst;

	printf("\r\nEntering the XAdc Polled Example. \r\n");

	/*
	 * Initialize the XAdc driver.
	 */
	ConfigPtr = XAdcPs_LookupConfig(XAdcDeviceId);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}
	XAdcPs_CfgInitialize(XAdcInstPtr, ConfigPtr,
				ConfigPtr->BaseAddress);

	/*
	 * Self Test the XADC/ADC device
	 */
	Status = XAdcPs_SelfTest(XAdcInstPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Disable the Channel Sequencer before configuring the Sequence
	 * registers.
	 */
	XAdcPs_SetSequencerMode(XAdcInstPtr, XADCPS_SEQ_MODE_SAFE);
	/*
	 * Read the on-chip Temperature Data (Current/Maximum/Minimum)
	 * from the ADC data registers.
	 */
	TempRawData = XAdcPs_GetAdcData(XAdcInstPtr, XADCPS_CH_TEMP);
	TempData = XAdcPs_RawToTemperature(TempRawData);
	printf("\r\nThe Current Temperature is %0d.%03d Centigrades.\r\n",
				(int)(TempData), XAdcFractionToInt(TempData));


	TempRawData = XAdcPs_GetMinMaxMeasurement(XAdcInstPtr, XADCPS_MAX_TEMP);
	MaxData = XAdcPs_RawToTemperature(TempRawData);
	printf("The Maximum Temperature is %0d.%03d Centigrades. \r\n",
				(int)(MaxData), XAdcFractionToInt(MaxData));

	TempRawData = XAdcPs_GetMinMaxMeasurement(XAdcInstPtr, XADCPS_MIN_TEMP);
	MinData = XAdcPs_RawToTemperature(TempRawData & 0xFFF0);
	printf("The Minimum Temperature is %0d.%03d Centigrades. \r\n",
				(int)(MinData), XAdcFractionToInt(MinData));

	/*
	 * Read the VccPint Votage Data (Current/Maximum/Minimum) from the
	 * ADC data registers.
	 */
	VccPintRawData = XAdcPs_GetAdcData(XAdcInstPtr, XADCPS_CH_VCCPINT);
	VccPintData = XAdcPs_RawToVoltage(VccPintRawData);
	printf("\r\nThe Current VCCPINT is %0d.%03d Volts. \r\n",
			(int)(VccPintData), XAdcFractionToInt(VccPintData));

	VccPintRawData = XAdcPs_GetMinMaxMeasurement(XAdcInstPtr,
							XADCPS_MAX_VCCPINT);
	MaxData = XAdcPs_RawToVoltage(VccPintRawData);
	printf("The Maximum VCCPINT is %0d.%03d Volts. \r\n",
			(int)(MaxData), XAdcFractionToInt(MaxData));

	VccPintRawData = XAdcPs_GetMinMaxMeasurement(XAdcInstPtr,
							XADCPS_MIN_VCCPINT);
	MinData = XAdcPs_RawToVoltage(VccPintRawData);
	printf("The Minimum VCCPINT is %0d.%03d Volts. \r\n",
			(int)(MinData), XAdcFractionToInt(MinData));

	/*
	 * Read the VccPaux Votage Data (Current/Maximum/Minimum) from the
	 * ADC data registers.
	 */
	VccPauxRawData = XAdcPs_GetAdcData(XAdcInstPtr, XADCPS_CH_VCCPAUX);
	VccPauxData = XAdcPs_RawToVoltage(VccPauxRawData);
	printf("\r\nThe Current VCCPAUX is %0d.%03d Volts. \r\n",
			(int)(VccPauxData), XAdcFractionToInt(VccPauxData));

	VccPauxRawData = XAdcPs_GetMinMaxMeasurement(XAdcInstPtr,
								XADCPS_MAX_VCCPAUX);
	MaxData = XAdcPs_RawToVoltage(VccPauxRawData);
	printf("The Maximum VCCPAUX is %0d.%03d Volts. \r\n",
				(int)(MaxData), XAdcFractionToInt(MaxData));


	VccPauxRawData = XAdcPs_GetMinMaxMeasurement(XAdcInstPtr,
								XADCPS_MIN_VCCPAUX);
	MinData = XAdcPs_RawToVoltage(VccPauxRawData);
	printf("The Minimum VCCPAUX is %0d.%03d Volts. \r\n\r\n",
				(int)(MinData), XAdcFractionToInt(MinData));


	/*
	 * Read the VccPdro Votage Data (Current/Maximum/Minimum) from the
	 * ADC data registers.
	 */
	VccPdroRawData = XAdcPs_GetAdcData(XAdcInstPtr, XADCPS_CH_VCCPDRO);
	VccPintData = XAdcPs_RawToVoltage(VccPdroRawData);
	printf("\r\nThe Current VCCPDDRO is %0d.%03d Volts. \r\n",
			(int)(VccPintData), XAdcFractionToInt(VccPintData));

	VccPdroRawData = XAdcPs_GetMinMaxMeasurement(XAdcInstPtr,
							XADCPS_MAX_VCCPDRO);
	MaxData = XAdcPs_RawToVoltage(VccPdroRawData);
	printf("The Maximum VCCPDDRO is %0d.%03d Volts. \r\n",
			(int)(MaxData), XAdcFractionToInt(MaxData));

	VccPdroRawData = XAdcPs_GetMinMaxMeasurement(XAdcInstPtr,
							XADCPS_MIN_VCCPDRO);
	MinData = XAdcPs_RawToVoltage(VccPdroRawData);
	printf("The Minimum VCCPDDRO is %0d.%03d Volts. \r\n",
			(int)(MinData), XAdcFractionToInt(MinData));


	printf("Exiting the XAdc Polled Example. \r\n");

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
int XAdcFractionToInt(float FloatNum)
{
	float Temp;

	Temp = FloatNum;
	if (FloatNum < 0) {
		Temp = -(FloatNum);
	}

	return( ((int)((Temp -(float)((int)Temp)) * (1000.0f))));
}
