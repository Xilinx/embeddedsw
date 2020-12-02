/******************************************************************************
* Copyright (C) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
* @file xspips_selftest_example.c
*
* This file contains an example for using the SPI Hardware, it does a simple
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
#include "xspips.h"
#include "xil_printf.h"

/************************** Constant Definitions ******************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define SPI_DEVICE_ID		XPAR_XSPIPS_0_DEVICE_ID

/**************************** Type Definitions ********************************/

/***************** Macros (Inline Functions) Definitions **********************/

/************************** Function Prototypes *******************************/

int SpiPsSelfTestExample(u16 DeviceId);

/************************** Variable Definitions ******************************/

XSpiPs Spi;			/* The instance of the SPI device */

/******************************************************************************/
/**
* Main function to call the Spi SelfTest example.
*
* @param	None
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

	xil_printf("SPI Selftest Example \r\n");

	/*
	 * Call the example , specify the device ID that is generated in
	 * xparameters.h
	 */
	Status = SpiPsSelfTestExample(SPI_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("SPI Selftest Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran SPI Selftest Example\r\n");
	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* This function does a selftest on the SPI device and XSpiPs driver as an
* example. The purpose of this function is to illustrate the usage of the
* XSpiPs driver.
*
*
* @param	DeviceId is the XPAR_<SPIPS_instance>_DEVICE_ID value from
*		xparameters.h
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful
*
* @note		None
*
****************************************************************************/
int SpiPsSelfTestExample(u16 DeviceId)
{
	int Status;
	XSpiPs_Config *SpiConfig;

	/*
	 * Initialize the SPI device.
	 */
	SpiConfig = XSpiPs_LookupConfig(DeviceId);
	if (NULL == SpiConfig) {
		return XST_FAILURE;
	}

	Status = XSpiPs_CfgInitialize(&Spi, SpiConfig, SpiConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to check hardware build.
	 */
	Status = XSpiPs_SelfTest(&Spi);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
