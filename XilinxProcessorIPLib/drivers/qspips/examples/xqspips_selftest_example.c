/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
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
* 3.11  akm    07/10/23 Add support for system device-tree flow for example.
* </pre>
*
*******************************************************************************/

/***************************** Include Files **********************************/

#ifndef SDT
#include "xparameters.h"
#endif
#include "xqspips.h"
#include "xil_printf.h"

/************************** Constant Definitions ******************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define QSPI_DEVICE_ID		XPAR_XQSPIPS_0_DEVICE_ID
#endif

/**************************** Type Definitions ********************************/

/***************** Macros (Inline Functions) Definitions **********************/

/************************** Function Prototypes *******************************/
#ifndef SDT
int QspiPsSelfTestExample(u16 DeviceId);
#else
int QspiPsSelfTestExample(UINTPTR BaseAddress);
#endif

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
#ifndef SDT
	Status = QspiPsSelfTestExample(QSPI_DEVICE_ID);
#else
	Status = QspiPsSelfTestExample(XPAR_XQSPIPS_0_BASEADDR);
#endif
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
#ifndef SDT
int QspiPsSelfTestExample(u16 DeviceId)
#else
int QspiPsSelfTestExample(UINTPTR BaseAddress)
#endif
{
	int Status;
	XQspiPs_Config *QspiConfig;

	/*
	 * Initialize the QSPI device.
	 */
#ifndef SDT
	QspiConfig = XQspiPs_LookupConfig(DeviceId);
#else
	QspiConfig = XQspiPs_LookupConfig(BaseAddress);
#endif
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
