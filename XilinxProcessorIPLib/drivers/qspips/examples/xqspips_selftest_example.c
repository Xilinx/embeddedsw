/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
* @file xqspips_selftest_example.c
*
* This file contains an example for using the QSPI Hardware, it does a simple
* hardware connection check.
*
* @note
*
* None
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -----------------------------------------------
* 1.00  drg/jz 01/25/10 First release.
* </pre>
*
*******************************************************************************/

/***************************** Include Files **********************************/

#include "xparameters.h"
#include "xqspips.h"
#include "xil_printf.h"

/************************** Constant Definitions ******************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define QSPI_DEVICE_ID		XPAR_XQSPIPS_0_DEVICE_ID

/**************************** Type Definitions ********************************/

/***************** Macros (Inline Functions) Definitions **********************/

/************************** Function Prototypes *******************************/

int QspiPsSelfTestExample(u16 DeviceId);

/************************** Variable Definitions ******************************/

XQspiPs Qspi;			/* The instance of the QSPI device */

/******************************************************************************/
/**
* Main function to call the Qspi Selftest example.
*
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful
*
* @note		None
*
*******************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	int Status;

	xil_printf("QSPI Selftest Example \r\n");

	/*
	 * Call the example , specify the device ID that is generated in
	 * xparameters.h
	 */
	Status = QspiPsSelfTestExample(QSPI_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("QSPI Selftest Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran QSPI Selftest Example\r\n");
	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* This function does a selftest on the QSPI device and XQspiPs driver as an
* example. The purpose of this function is to illustrate the usage of the
* XQspiPs driver.
*
*
* @param	DeviceId is the XPAR_<QSPIPS_instance>_DEVICE_ID value from
*		xparameters.h
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful
*
* @note		None
*
****************************************************************************/
int QspiPsSelfTestExample(u16 DeviceId)
{
	int Status;
	XQspiPs_Config *QspiConfig;

	/*
	 * Initialize the QSPI device.
	 */
	QspiConfig = XQspiPs_LookupConfig(DeviceId);
	if (NULL == QspiConfig) {
		return XST_FAILURE;
	}

	Status = XQspiPs_CfgInitialize(&Qspi, QspiConfig, QspiConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to check hardware build.
	 */
	Status = XQspiPs_SelfTest(&Qspi);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	return XST_SUCCESS;
}
