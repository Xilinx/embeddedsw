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
#ifndef SDT
#define UARTPSV_DEVICE_ID		XPAR_XUARTPSV_0_DEVICE_ID
#else
#define XUARTPSV_BASEADDRESS		XPAR_XUARTPSV_0_BASEADDR
#endif

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

#ifndef SDT
int UartPsvSelfTestExample(u16 DeviceId);
#else
int UartPsvSelfTestExample(UINTPTR BaseAddress);
#endif

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
#ifndef SDT
	Status = UartPsvSelfTestExample(UARTPSV_DEVICE_ID);
#else
	Status = UartPsvSelfTestExample(XUARTPSV_BASEADDRESS);
#endif

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
#ifndef SDT
int UartPsvSelfTestExample(u16 DeviceId)
#else
int UartPsvSelfTestExample(UINTPTR BaseAddress)
#endif
{
	int Status;
	XUartPsv_Config *Config;

	/*
	 * Initialize the UARTPSV driver so that it's ready to use
	 * Look up the configuration in the config table,
	 * then initialize it.
	 */
#ifndef SDT
	Config = XUartPsv_LookupConfig(DeviceId);
#else
	Config = XUartPsv_LookupConfig(BaseAddress);
#endif

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
