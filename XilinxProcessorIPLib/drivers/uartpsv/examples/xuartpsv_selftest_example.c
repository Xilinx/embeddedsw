/******************************************************************************
* Copyright (C) 2017 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xuartpsv_selftest_example.c
*
* This example runs on versal evaluation board (vck190), it performs self test
* by using XUartPsv driver.
*
* @note
*
* None
*
* MODIFICATION HISTORY:
* <pre>
* Ver  Who  Date      Changes
* ---  ---  --------- -----------------------------------------------
* 1.0  sg   09/18/17  First Release
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xuartpsv.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define UARTPSV_DEVICE_ID		XPAR_XUARTPSV_0_DEVICE_ID

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int UartPsvSelfTestExample(u16 DeviceId);

/************************** Variable Definitions *****************************/

XUartPsv Uart_Psv;		/* Instance of the UARTPSV Device */

/*****************************************************************************/
/**
*
* Main function to call the example.
*
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE
*
* @note 	None
*
******************************************************************************/
int main(void)
{
	int Status;

	/* Run the selftest example */
	Status = UartPsvSelfTestExample(UARTPSV_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("UartPsv Selftest Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran UartPsv Selftest Example\r\n");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function does a minimal test on the XUartPsv driver.
*
*
* @param	DeviceId is the XPAR_<UARTPSV_instance>_DEVICE_ID value from
*			xparameters.h
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE
*
* @note 	None
*
****************************************************************************/
int UartPsvSelfTestExample(u16 DeviceId)
{
	int Status;
	XUartPsv_Config *Config;

	/*
	 * Initialize the UARTPSV driver so that it's ready to use
	 * Look up the configuration in the config table,
	 * then initialize it.
	 */
	Config = XUartPsv_LookupConfig(DeviceId);
	if (NULL == Config) {
		return XST_FAILURE;
	}

	Status = XUartPsv_CfgInitialize(&Uart_Psv, Config,
				Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Perform a self-test to check hardware build. */
	Status = XUartPsv_SelfTest(&Uart_Psv);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
