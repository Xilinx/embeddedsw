/******************************************************************************
* Copyright (C) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
/*****************************************************************************/
/**
* @file xgpiops_polled_example.c
*
* This file contains an example for using GPIO hardware and driver. This
* example provides the usage of APIs for reading/writing to the individual pins.
* Please see xgpiops.h file for description of the pin numbering.
*
* @note		This example assumes that there is a Uart device in the HW
* design. This example is to provide support only for zcu102 on
* ZynqMp Platform and only for zc702 on Zynq Platform.
* For ZynqMP Platform, Input pin is 22(sw19 on zcu102 board) and Output Pin is
* 23(DS50 on zcu102 board).
* For Zynq Platform, Input Pins are 12(sw14 on zc702 board), 14(sw13 on
* zc702 board) and Output Pin is 10(DS23 on zc702 board).
* In Versal Platform we have two GPIOPS instances(PMC GPIO and PS GPIO),PMC GPIO
* contain 4 banks and 116 pins, PS GPIO contains 2 banks and 58 pins.
* Driver supports both PS GPIO and PMC GPIO.
* For accessing PMC GPIOs application need to configure "GPIO.PmcGpio =1" else
* it works for PS GPIO.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a sv   01/18/10 First Release
* 1.01a sv   04/15/12 Removed the calling of some unnecessary APIs.
*		      Updated the examples for a ZC702 board .
*		      Updated the example to use only pin APIs.
* 3.3   ms   04/17/17 Added notes about input and output pin description
*                     for zcu102 and zc702 boards.
* 3.7	sne  12/04/19 Reverted versal example support.
* 3.8	sne  09/17/20 Added description for Versal PS and PMC GPIO pins.
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xparameters.h"
#include "xgpiops.h"
#include "xstatus.h"
#include "xplatform_info.h"
#include <xil_printf.h>

/************************** Constant Definitions ****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place for ZYNQ & ZYNQMP.
 */

#ifndef GPIO_DEVICE_ID
#define GPIO_DEVICE_ID		XPAR_XGPIOPS_0_DEVICE_ID
#endif

/*
 * The following constant is used to wait after an LED is turned on to make
 * sure that it is visible to the human eye.  This constant might need to be
 * tuned for faster or slower processor speeds.
 */
#define LED_DELAY		10000000

#define LED_MAX_BLINK		0x10	/* Number of times the LED Blinks */

#define printf			xil_printf	/* Smalller foot-print printf */

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions *******************/

/************************** Function Prototypes ****************************/

static int GpioOutputExample(void);
static int GpioInputExample(u32 *DataRead);
int GpioPolledExample(u16 DeviceId, u32 *DataRead);

/************************** Variable Definitions **************************/
static u32 Input_Pin; /* Switch button */
static u32 Output_Pin; /* LED button */

/*
 * The following are declared globally so they are zeroed and can be
 * easily accessible from a debugger.
 */
XGpioPs Gpio;	/* The driver instance for GPIO Device. */

/*****************************************************************************/
/**
*
* Main function to call the example.
*
* @param	None
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note		None
*
******************************************************************************/
int main(void)
{
	int Status;
	u32 InputData;

	printf("GPIO Polled Mode Example Test \r\n");
	Status = GpioPolledExample(GPIO_DEVICE_ID, &InputData);
	if (Status != XST_SUCCESS) {
		printf("GPIO Polled Mode Example Test Failed\r\n");
		return XST_FAILURE;
	}

	printf("Data read from GPIO Input is  0x%x \n\r", (int)InputData);
	printf("Successfully ran GPIO Polled Mode Example Test\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* The purpose of this function is to illustrate how to use the GPIO driver to
* turn on/off an LED and read the inputs using the pin APIs.
*
* @param	DeviceId is the XPAR_<GPIO_instance>_DEVICE_ID value from
*		xparameters.h
* @param	DataRead is the pointer where the data read from GPIO Input is
*		returned.
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note		This function will not return if the test is running.
*
******************************************************************************/
int GpioPolledExample(u16 DeviceId, u32 *DataRead)
{
	int Status;
	XGpioPs_Config *ConfigPtr;
	int Type_of_board;

	/* Initialize the GPIO driver. */
	ConfigPtr = XGpioPs_LookupConfig(GPIO_DEVICE_ID);
	Type_of_board = XGetPlatform_Info();
	switch (Type_of_board) {
		case XPLAT_ZYNQ_ULTRA_MP:
			Input_Pin = 22;
			Output_Pin = 23;
			break;

		case XPLAT_ZYNQ:
			Input_Pin = 14;
			Output_Pin = 10;
			break;
	}

	Status = XGpioPs_CfgInitialize(&Gpio, ConfigPtr,
					ConfigPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/* Run the Output Example. */
	Status = GpioOutputExample();
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Run the Input Example. */
	Status = GpioInputExample(DataRead);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function does a minimal test on the GPIO device configured as OUTPUT.
*
* @param	None.
*
* @return	- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note		None.
*
****************************************************************************/
static int GpioOutputExample(void)
{
	u32 Data;
	volatile int Delay;
	u32 LedLoop;

	/*
	 * Set the direction for the pin to be output and
	 * Enable the Output enable for the LED Pin.
	 */
	XGpioPs_SetDirectionPin(&Gpio, Output_Pin, 1);
	XGpioPs_SetOutputEnablePin(&Gpio, Output_Pin, 1);

	/* Set the GPIO output to be low. */
	XGpioPs_WritePin(&Gpio, Output_Pin, 0x0);


	for (LedLoop = 0; LedLoop < LED_MAX_BLINK; LedLoop ++) {

#ifndef __SIM__
		/* Wait a small amount of time so the LED is visible. */
		for (Delay = 0; Delay < LED_DELAY; Delay++);

#endif
		/* Set the GPIO Output to High. */
		XGpioPs_WritePin(&Gpio, Output_Pin, 0x1);

		/*
		 * Read the state of the data and verify. If the data
		 * read back is not the same as the data written then
		 * return FAILURE.
		 */
		Data = XGpioPs_ReadPin(&Gpio, Output_Pin);
		if (Data != 1 ) {
			return XST_FAILURE;
		}

#ifndef __SIM__
		/* Wait a small amount of time so the LED is visible. */
		for (Delay = 0; Delay < LED_DELAY; Delay++);

#endif

		/* Clear the GPIO Output. */
		XGpioPs_WritePin(&Gpio, Output_Pin, 0x0);

		/*
		 * Read the state of the data and verify. If the data
		 * read back is not the same as the data written then
		 * return FAILURE.
		 */
		Data = XGpioPs_ReadPin(&Gpio, Output_Pin);
		if (Data != 0) {
			return XST_FAILURE;
		}
	}
	return XST_SUCCESS;
}

/******************************************************************************/
/**
*
* This function performs a test on the GPIO driver/device with the GPIO
* configured as INPUT.
*
* @param	DataRead is the pointer where the data read from GPIO Input is
*		returned
*
* @return	- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note		None.
*
******************************************************************************/
static int GpioInputExample(u32 *DataRead)
{

	/* Set the direction for the specified pin to be input. */
	XGpioPs_SetDirectionPin(&Gpio, Input_Pin, 0x0);
	/* Read the state of the data so that it can be  verified. */
	*DataRead = XGpioPs_ReadPin(&Gpio, Input_Pin);

	return XST_SUCCESS;
}
