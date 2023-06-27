/******************************************************************************
* Copyright (C) 2017 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xuartpsv_hello_world_example.c
*
* This file contains a design example using the XUartPsv driver in polled
* mode.
* The example uses the default setting in the XUartPsv driver:
*	. baud rate 9600
*	. 8 bit data
*	. 1 stop bit
*	. no parity
*
* @note
* This example requires an external SchmartModule connected to the pins for
* the device to display the 'Hello World' message onto a hyper-terminal.
*
* MODIFICATION HISTORY:
* <pre>
* Ver  Who  Date      Changes
* ---  ---  --------- -----------------------------------------------
* 1.0  sg   09/18/17  First Release
* 1.2  rna  01/20/20  Add self test
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xuartpsv.h"

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
int UartPsvHelloWorldExample(u16 DeviceId);
#else
int UartPsvHelloWorldExample(UINTPTR BaseAddress);
#endif

/************************** Variable Definitions *****************************/

XUartPsv Uart_Psv;		/* The instance of the UARTPSV Driver */

/*****************************************************************************/
/**
*
* Main function to call the Hello World example.
*
*
* @return
*		- XST_FAILURE if the Test Failed .
*		- A non-negative number indicating the number of characters
*		  sent.
*
* @note 	None
*
******************************************************************************/
int main(void)
{
	int Status;

	/*
	 * Run the Hello World example , specify the the Device ID that is
	 * generated in xparameters.h
	 */
#ifndef SDT
	Status = UartPsvHelloWorldExample(UARTPSV_DEVICE_ID);
#else
	Status = UartPsvHelloWorldExample(XUARTPSV_BASEADDRESS);
#endif

	if (Status == XST_FAILURE) {
		xil_printf("UartPsv Hello World Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran UartPsv Hello World Example\r\n");

	return Status;
}

/*****************************************************************************/
/**
*
* This function sends 'Hello World' to an external terminal in polled mode.
* The purpose of this function is to illustrate how to use the XUartPsv
* driver.
*
*
* @param	DeviceId is the unique ID for the device from hardware build.
*
* @return
*		- XST_FAILURE if the UART driver could not be initialized
*		  successfully.
*		- A non-negative number indicating the number of characters
*		  sent.
*
* @note 	None.
*
******************************************************************************/
#ifndef SDT
int UartPsvHelloWorldExample(u16 DeviceId)
#else
int UartPsvHelloWorldExample(UINTPTR BaseAddress)
#endif
{
	u8 HelloWorld[] = "Hello World";
	int SentCount = 0;
	int Status;
	XUartPsv_Config *Config;

	/*
	 * Initialize the UART driver so that it's ready to use
	 * Look up the configuration in the config table and then
	 * initialize it.
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

	/* Check hardware build */
	Status = XUartPsv_SelfTest(&Uart_Psv);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	while (SentCount < (sizeof(HelloWorld) - 1)) {
		/* Transmit the data */
		SentCount += XUartPsv_Send(&Uart_Psv,
					   &HelloWorld[SentCount], 1);
	}

	xil_printf("Hello Test Passed\r\n");

	return SentCount;
}
