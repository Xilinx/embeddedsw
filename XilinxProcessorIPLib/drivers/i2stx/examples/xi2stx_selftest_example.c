/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 * @file xi2stx_selftest_example.c
 *
 * This file contains a example for using the I2s Transmitter hardware device
 * and I2s Transmitter driver.
 *
 *
 * <pre> MODIFICATION HISTORY:
 *
 * Ver	 Who Date    	Changes
 * ----- --- -------- 	-----------------------------------------------
 * 1.0   kar 01/25/18 	First release
 *
 * </pre>
 *
 ****************************************************************************/

/***************************** Include Files **********************************/
#include "xparameters.h"
#include "xi2stx.h"
#include "xil_printf.h"

/************************** Constant Definitions ******************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#ifndef TESTAPP_GEN
#define I2S_TX_DEVICE_ID	XPAR_XI2STX_0_DEVICE_ID
#endif
#endif

/**************************** Type Definitions ********************************/

/************************** Function Prototypes *******************************/
#ifndef SDT
int I2sSelfTestExample(u16 DeviceId);
#else
int I2sSelfTestExample(UINTPTR BaseAddress);
#endif

/************************** Variable Definitions ******************************/

XI2s_Tx I2s_tx;		/* Instance of the I2s Transmitter device */

/******************************************************************************/
/******************************************************************************/
/**
*
* Main function to call the Self Test example.
*
* @param	None.
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful.
*
* @note		None.
*
*******************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	int Status;

	xil_printf("I2S Self Test Example \r\n");

	/*
	 * Run the I2S TX Self Test example, specify the Device ID that is
	 * generated in xparameters.h
	 */
#ifndef SDT
	Status = I2sSelfTestExample(I2S_TX_DEVICE_ID);
#else
	Status = I2sSelfTestExample(XPAR_XI2STX_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("I2S TX Self Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran I2S TX Self Test Example Test\r\n");
	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* This function does a minimal test on the I2s Transmitter device and
* driver as a design example. The purpose of this function is to illustrate
* how to use the Xi2s transmitter component.
*
*
* @param	DeviceId is the Device ID of the I2s TX Device and is the
*		XPAR_<I2S_TRANSMITTER_instance>_DEVICE_ID value from xparameters.h
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
*
*******************************************************************************/
#ifndef SDT
int I2sSelfTestExample(u16 DeviceId)
#else
int I2sSelfTestExample(UINTPTR BaseAddress)
#endif
{
	int Status;
	XI2stx_Config *Config;
	/*
	 * Initialize the I2s TX driver so that it's ready to use
	 * Look up the configuration in the config table, then initialize it.
	 */
#ifndef SDT
	Config = XI2s_Tx_LookupConfig(DeviceId);
#else
	Config = XI2s_Tx_LookupConfig(BaseAddress);
#endif
	if (Config == NULL) {
		return XST_FAILURE;
	}

	Status = XI2s_Tx_CfgInitialize(&I2s_tx, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test.
	 */
	Status = XI2s_Tx_SelfTest(&I2s_tx);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

