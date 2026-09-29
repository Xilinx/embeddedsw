/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
* @file xgpiops_polled_example.c
* @brief Demonstrates polled-mode usage of the GPIO PS hardware and driver.
*
* This file contains an example for using GPIO hardware and driver. This
* example shows the usage of APIs for reading and writing the individual
* pins. See xgpiops.h for a description of the pin numbering.
*
* @note
* This example assumes that the hardware design contains a UART device.
* This example supports only the zcu102 board on the ZynqMP platform and only
* the zc702 board on the Zynq platform.
* For the ZynqMP platform, the input pin is 22 (sw19 on the zcu102 board) and
* the output pin is 23 (DS50 on the zcu102 board).
* For the Zynq platform, the input pins are 12 (sw14 on the zc702 board) and
* 14 (sw13 on the zc702 board), and the output pin is 10 (DS23 on the zc702
* board).
* This example supports the VCK190 and VMK180 for Versal, but requires a PL
* shim. See Answer Record AR# 75677 and Figure 61 in AM011 Versal TRM
* for more details.
* The driver supports both PS GPIO and PMC GPIO.
* To access PMC GPIOs, set "GPIO.PmcGpio = 1" in the application. Otherwise,
* the application accesses PS GPIO.
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
* 3.9	sne  11/19/20 Added versal PmcGpio example support.
* 3.12  gm   07/11/23 Added SDT support.
* 3.15  vlt  12/12/25 Update Doxygen comments to include SDT flow details.
* 3.16  vn   07/30/26 Fix Doxygen warnings
* 3.16  vn   09/10/26 Align Doxygen comments with AMD standards
*
* </pre>
*
******************************************************************************/

/****************************** Include Files ********************************/

#include "xparameters.h"
#include "xgpiops.h"
#include "xstatus.h"
#include "xplatform_info.h"
#include <xil_printf.h>

/*************************** Constant Definitions ****************************/

/** @name Device parameters
 *
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here so that you can easily change
 * all the needed parameters in one place for ZYNQ & ZYNQMP.
 * @{
 */
#ifndef SDT
#ifndef GPIO_DEVICE_ID
/** GPIO device ID. */
#define GPIO_DEVICE_ID		XPAR_XGPIOPS_0_DEVICE_ID
#endif
#else
/** GPIO base address. */
#define	XGPIOPS_BASEADDR	XPAR_XGPIOPS_0_BASEADDR
#endif
/** @} */

/**
 * @brief LED visibility delay count.
 *
 * The example uses this constant to wait after it turns on an LED so that the
 * LED is visible to the human eye. You might need to tune this constant for
 * faster or slower processor speeds.
 */
#define LED_DELAY		10000000

/** Number of times the LED blinks. */
#define LED_MAX_BLINK		0x10

/** Smaller footprint printf. */
#define printf			xil_printf

/***************************** Type Definitions ******************************/

/****************** Macros (Inline Functions) Definitions ********************/

/*************************** Function Prototypes *****************************/

static int GpioOutputExample(void);
static int GpioInputExample(u32 *DataRead);
#ifndef SDT
int GpioPolledExample(u16 DeviceId, u32 *DataRead);
#else
int GpioPolledExample(UINTPTR BaseAddress, u32 *DataRead);
#endif

/*************************** Variable Definitions ****************************/
static u32 Input_Pin; /**< Switch button. */
static u32 Output_Pin; /**< LED button. */

/**
 * @brief Holds the global GPIO driver instance.
 *
 * The instance is declared globally so it is zeroed and easily accessible
 * from a debugger.
 */
XGpioPs Gpio;

/*****************************************************************************/
/**
* @brief Calls the GPIO polled-mode example.
*
* @return
*              - XST_SUCCESS if the example has completed successfully.
*              - XST_FAILURE if the example has failed.
*
******************************************************************************/
int main(void)
{
	int Status;
	u32 InputData;

	printf("GPIO Polled Mode Example Test \r\n");
#ifndef SDT
	Status = GpioPolledExample(GPIO_DEVICE_ID, &InputData);
#else
	Status = GpioPolledExample(XGPIOPS_BASEADDR, &InputData);
#endif
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
* @brief Demonstrates LED control and input reads using the GPIO pin APIs.
*
* Turns an LED on and off and reads inputs using the GPIO pin APIs.
*
* @if SDT
* @param[in]   BaseAddress Base address of the device.
* @else
* @param[in]   DeviceId XPAR_<GPIO_instance>_DEVICE_ID value from
*              xparameters.h.
* @endif
* @param[out]  DataRead Pointer where the data read from the GPIO input is
*              returned.
*
* @return
*              - XST_SUCCESS if the example has completed successfully.
*              - XST_FAILURE if the example has failed.
*
* @note        This function does not return if the test is running.
*              In XSCT/classic flow, DeviceId is used to look up the device
*              configuration.
*
******************************************************************************/
#ifndef SDT
int GpioPolledExample(u16 DeviceId, u32 *DataRead)
#else
int GpioPolledExample(UINTPTR BaseAddress, u32 *DataRead)
#endif
{
	int Status;
	XGpioPs_Config *ConfigPtr;
	int Type_of_board;

	/* Initialize the GPIO driver. */
#ifndef SDT
	ConfigPtr = XGpioPs_LookupConfig(GPIO_DEVICE_ID);
#else
	ConfigPtr = XGpioPs_LookupConfig(BaseAddress);
#endif
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
#ifdef versal
		case XPLAT_VERSAL:
			/* Accessing PMC GPIO by setting field to 1 */
			Gpio.PmcGpio =  1;
			Input_Pin    = 56;
			Output_Pin   = 52;
			break;
#endif
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
* @brief Performs a minimal test on the GPIO device configured as output.
*
* @return
*              - XST_SUCCESS if the example has completed successfully.
*              - XST_FAILURE if the example has failed.
*
******************************************************************************/
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

/*****************************************************************************/
/**
* @brief Tests the GPIO driver and device with the GPIO configured as input.
*
* @param[out]  DataRead Pointer where the data read from the GPIO input is
*              returned.
*
* @return
*              - XST_SUCCESS if the example has completed successfully.
*              - XST_FAILURE if the example has failed.
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
