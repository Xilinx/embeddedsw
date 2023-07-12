/******************************************************************************
* Copyright (C) 2008 - 2023 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
* @file xspi_selftest_example.c
*
* This file contains a example for using the SPI Hardware and driver.
*
* @note
*
* None
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a sv   05/16/05 Initial release for TestApp integration.
* 1.11a sdm  03/03/08 Minor changes to comply to coding guidelines
* 3.00a ktn  10/28/09 Converted all register accesses to 32 bit access.
*		      Updated to use the HAL APIs/macros. Replaced call to
*		      XSpi_Initialize API with XSpi_LookupConfig and
*		      XSpi_CfgInitialize.
* 4.2   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
* 4.11  sb   07/11/23 Added support for system device-tree flow.
* </pre>
*
*******************************************************************************/

/***************************** Include Files **********************************/

#include "xparameters.h"
#include "xspi.h"
#include "xspi_l.h"
#include "xil_printf.h"

/************************** Constant Definitions ******************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define SPI_DEVICE_ID       XPAR_SPI_0_DEVICE_ID
#endif

/**************************** Type Definitions ********************************/


/***************** Macros (Inline Functions) Definitions **********************/


/************************** Function Prototypes *******************************/

#ifndef SDT
int SpiSelfTestExample(u16 DeviceId);
#else
int SpiSelfTestExample(UINTPTR BaseAddress);
#endif

/************************** Variable Definitions ******************************/

XSpi Spi; /* The instance of the SPI device */


/******************************************************************************/
/**
* Main function to call the example. This function is not included if the
* example is generated from the TestAppGen test tool.
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

	/*
	 * Call the example , specify the device ID that is generated in
	 * xparameters.h
	 */
#ifndef SDT
	Status = SpiSelfTestExample(SPI_DEVICE_ID);
#else
	Status = SpiSelfTestExample(XPAR_XSPI_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("Spi selftest Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Spi selftest Example\r\n");
	return XST_SUCCESS;
}
#endif


/*****************************************************************************/
/**
*
* This function does a selftest and loopback test on the SPI device and
* XSpi driver as an example. The purpose of this function is to illustrate
* how to use the XSpi component.
*
*
* @param	DeviceId is the XPAR_<SPI_instance>_DEVICE_ID value from
*		xparameters.h
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful
*
* @note		None
*
****************************************************************************/
#ifndef SDT
int SpiSelfTestExample(u16 DeviceId)
#else
int SpiSelfTestExample(UINTPTR BaseAddress)
#endif
{
	int Status;
	XSpi_Config *ConfigPtr;	/* Pointer to Configuration data */

	/*
	 * Initialize the SPI driver so that it is  ready to use.
	 */
#ifndef SDT
	ConfigPtr = XSpi_LookupConfig(DeviceId);
#else
	ConfigPtr = XSpi_LookupConfig(BaseAddress);
#endif
	if (ConfigPtr == NULL) {
		return XST_DEVICE_NOT_FOUND;
	}

	Status = XSpi_CfgInitialize(&Spi, ConfigPtr,
				    ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built correctly
	 */
	Status = XSpi_SelfTest(&Spi);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

