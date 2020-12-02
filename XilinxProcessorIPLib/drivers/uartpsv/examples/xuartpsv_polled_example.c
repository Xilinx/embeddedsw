/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file	xuartpsv_polled_example.c
*
* This file contains a design example using the XUartPsv driver in polled
* mode. It sends data and expects to receive the same data through the device
* using the local loopback mode.
*
*
* @note
* If the device does not work properly, the example may hang.
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who   Date     Changes
* ----- ----- -------- ----------------------------------------------
* 1.0   sd    05/23/19 First Release
* 1.2   rna   01/20/20 Add self test
* </pre>
****************************************************************************/

/***************************** Include Files *******************************/

#include "xparameters.h"
#include "xplatform_info.h"
#include "xuartpsv.h"
#include "xil_printf.h"

/************************** Constant Definitions **************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define UARTPSV_DEVICE_ID		XPAR_XUARTPSV_0_DEVICE_ID
/*
 * The following constant controls the length of the buffers to be sent
 * and received with the UART,
 */
#define TEST_BUFFER_SIZE	10


/**************************** Type Definitions ******************************/

/************************** Function Prototypes *****************************/

int UartPsvPolledExample(u16 DeviceId);

/************************** Variable Definitions ***************************/

XUartPsv UartPsv;		/* Instance of the UART Device */

/*
 * The following buffers are used in this example to send and receive data
 * with the UART.
 */
static u8 SendBuffer[TEST_BUFFER_SIZE];	/* Buffer for Transmitting Data */
static u8 RecvBuffer[TEST_BUFFER_SIZE];	/* Buffer for Receiving Data */

/*
 * The following counters are used to determine when the entire buffer has
 * been sent and received.
 */
volatile int TotalReceivedCount;
volatile int TotalSentCount;
int TotalErrorCount;

/**************************************************************************/
/**
*
* Main function to call the Uart Polled mode example.
*
* @param	None
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful
*
* @note		None
*
**************************************************************************/
#ifndef TESTAPP_GEN

int main(void)
{
	int Status;

	/* Run the UartPsv Polled example, specify the the Device ID */
	Status = UartPsvPolledExample(UARTPSV_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("UartPsv Polled Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran UartPsv Polling Example Test\r\n");
	return XST_SUCCESS;
}
#endif

/**************************************************************************/
/**
*
* This function does a minimal test on the UartPsv device and driver as a
* design example. The purpose of this function is to illustrate
* how to use the XUartPsv driver.
*
* This function does a minimal test on the XUartPs device in polled mode.
* This function sends data and expects to receive the same data through the
* device using the local loopback mode.
*
*
* @param	DeviceId is the device Id of the UART device and is typically
*		XPAR_<UartPsv_instance>_DEVICE_ID value from xparameters.h.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note
* This function polls the UART, it may hang if the hardware is not
* working correctly.
*
**************************************************************************/
int UartPsvPolledExample(u16 DeviceId)
{
	int Status;
	XUartPsv_Config *Config;
	int Index;
	int BadByteCount = 0;
	unsigned int ReceivedCount;

	/*
	 * Initialize the UART driver so that it's ready to use.
	 * Look up the configuration in the config table, then initialize it.
	 */
	Config = XUartPsv_LookupConfig(DeviceId);
	if (NULL == Config) {
		return XST_FAILURE;
	}

	Status = XUartPsv_CfgInitialize(&UartPsv, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Check hardware build */
	Status = XUartPsv_SelfTest(&UartPsv);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Use local loopback mode. */
	XUartPsv_SetOperMode(&UartPsv, XUARTPSV_OPER_MODE_LOCAL_LOOP);

	/*
	 * Initialize the send buffer bytes with a pattern and the
	 * the receive buffer bytes to zero to allow the receive data to be
	 * verified
	 */
	for (Index = 0; Index < TEST_BUFFER_SIZE; Index++) {
		SendBuffer[Index] = (Index % 26) + 'A';
		RecvBuffer[Index] = 0;
	}

	TotalSentCount = XUartPsv_Send(&UartPsv, SendBuffer, TEST_BUFFER_SIZE);

	while (XUartPsv_IsTransmitbusy(UartPsv.Config.BaseAddress));
	ReceivedCount = 0;
	while (ReceivedCount < TEST_BUFFER_SIZE) {
		ReceivedCount +=
		XUartPsv_Recv(&UartPsv, &RecvBuffer[ReceivedCount],
				      (TEST_BUFFER_SIZE - ReceivedCount));
	}
	/* Verify the entire receive buffer was successfully received */
	for (Index = 0; Index < TEST_BUFFER_SIZE; Index++) {
		if (RecvBuffer[Index] != SendBuffer[Index]) {
			BadByteCount++;
		}
	}

	/* Set the UART in Normal Mode */
	XUartPsv_SetOperMode(&UartPsv, XUARTPSV_OPER_MODE_NORMAL);

	/* If any bytes were not correct, return an error */
	if (BadByteCount != 0) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
