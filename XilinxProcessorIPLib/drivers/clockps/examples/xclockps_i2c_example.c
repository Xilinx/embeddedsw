/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xclockps_example.c
*
* This file contains a design example using the Clock Controller (ClockPs)
* driver. It uses I2C peripheral clock for clock operations
*
* @note
*
* <pre>
* MODIFICATION HISTORY:
* Ver   Who    Date     Changes
* ----- ------ -------- ---------------------------------------------
* 1.00  cjp    2/20/18 First release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xclockps.h"
#include "xstatus.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/
#define CLOCK_DEVICE_ID              (XPAR_XCLOCKPS_DEVICE_ID)
#define XCLOCK_I2C0_REF_RATE1        (1428428)
#define XCLOCK_I2C0_REF_RATE2        (33330000)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static XStatus ClockPsExample(XClock *ClockInstancePtr, u16 ClockDevId);

/************************** Variable Definitions *****************************/
XClock ClockInstance;		/* Instance of clock Controller */
XClockPs_Config *ConfigPtr;

/*****************************************************************************/
/**
* Main function to call Clock example.
*
* @param	None.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	int Status;

	xil_printf("Clock Example Test\r\n");

	/* Execute clock operations */
	Status = ClockPsExample(&ClockInstance, CLOCK_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Clock Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Clock Test\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function tests the functioning of the clock controller driver by
* undergoing clock operations for I2C0_REF clock.
*
*
* @param	ClockInstancePtr is a pointer to instance of XClockPs driver.
* @param	ClockDeviceId is the device ID of clock controller.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
static XStatus ClockPsExample(XClock *ClockInstancePtr, u16 ClockDeviceId)
{
	XStatus    Status;
	XClockRate Rate;

	/* Lookup clock configurations */
	ConfigPtr = XClock_LookupConfig(ClockDeviceId);

	/* Initialize the Clock controller driver */
	Status = XClock_CfgInitialize(ClockInstancePtr, ConfigPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed: Fetching clock configuration\r\n");
		return Status;
	}

	/* Enabling i2c clock */
	Status = XClock_EnableClock(I2C0_REF);
	if (XST_SUCCESS != Status) {
		xil_printf("Failed: Enabling I2C0_REF clock\r\n");
		return Status;
	}

	/* Fetch Rate */
	Status = XClock_GetRate(I2C0_REF, &Rate);
	if (XST_SUCCESS == Status) {
		xil_printf("I2C0_REF Operating Rate = %ld\r\n", Rate);
	} else {
		xil_printf("Failed: Fetching I2C0_REF rate\r\n");
		return Status;
	}

	/* Set Rate */
	Rate = XCLOCK_I2C0_REF_RATE1;
	xil_printf("Setting rate for I2C0_REF to Rate = %ld\r\n", Rate);
	Status = XClock_SetRate(I2C0_REF, Rate, &Rate);
	if (XST_SUCCESS != Status) {
		xil_printf("Failed: Setting I2C0_REF rate\r\n");
		return Status;
	}

	/* Fetch Rate */
	Status = XClock_GetRate(I2C0_REF, &Rate);
	if (XST_SUCCESS == Status) {
		xil_printf("I2C0_REF Operating Rate = %ld\r\n", Rate);
	} else {
		xil_printf("Failed: Fetching I2C0_REF rate\r\n");
		return Status;
	}

	/* Set Rate */
	Rate = XCLOCK_I2C0_REF_RATE2;
	xil_printf("Setting rate for I2C0_REF to Rate = %ld\r\n", Rate);
	Status = XClock_SetRate(I2C0_REF, Rate, &Rate);
	if (XST_SUCCESS != Status) {
		xil_printf("Failed: Setting I2C0_REF rate\r\n");
		return Status;
	}

	/* Fetch Rate */
	Status = XClock_GetRate(I2C0_REF, &Rate);
	if (XST_SUCCESS == Status) {
		xil_printf("I2C0_REF Operating Rate = %ld\r\n", Rate);
	} else {
		xil_printf("Failed: Fetching I2C0_REF rate\r\n");
		return Status;
	}

	/* Disabling i2c clock */
	Status = XClock_DisableClock(I2C0_REF);
	if (XST_SUCCESS != Status) {
		xil_printf("\tFailed: Disabling I2C0_REF clock\r\n");
	}

	return Status;
}
