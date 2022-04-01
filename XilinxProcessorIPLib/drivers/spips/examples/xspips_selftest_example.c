/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
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
* 3.7   asa    04/01/22 The selftest example uses the instance name as
*                       "Spi" in the instance definition. That causes
*                       issue when periphtest application is build for a
*                       design that has a soft spi IP along with the
*                       hard spips. This is fixed by renaming the
*                       instance name from "Spi" to "SpiPs"
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

XSpiPs SpiPs;			/* The instance of the SPI device */

/******************************************************************************/
/**
* Main function to call the Spi SelfTest example.
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

	xil_printf("SPIPS Selftest Example \r\n");

	/*
	 * Call the example , specify the device ID that is generated in
	 * xparameters.h
	 */
	Status = SpiPsSelfTestExample(SPI_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("SPIPS Selftest Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran SPIPS Selftest Example\r\n");
	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* This function does a selftest on the SPIPS device and XSpiPs driver as an
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
	 * Initialize the SPIPS device.
	 */
	SpiConfig = XSpiPs_LookupConfig(DeviceId);
	if (NULL == SpiConfig) {
		return XST_FAILURE;
	}

	Status = XSpiPs_CfgInitialize(&SpiPs, SpiConfig, SpiConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to check hardware build.
	 */
	Status = XSpiPs_SelfTest(&SpiPs);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
