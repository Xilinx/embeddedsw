/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
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
#ifndef TESTAPP_GEN
#define I2S_TX_DEVICE_ID	XPAR_XI2STX_0_DEVICE_ID
#endif

/**************************** Type Definitions ********************************/

/************************** Function Prototypes *******************************/

int I2sSelfTestExample(u16 DeviceId);

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
	Status = I2sSelfTestExample(I2S_TX_DEVICE_ID);
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
int I2sSelfTestExample(u16 DeviceId)
{
	int Status;
	XI2stx_Config *Config;
	/*
	 * Initialize the I2s TX driver so that it's ready to use
	 * Look up the configuration in the config table, then initialize it.
	 */
	Config = XI2s_Tx_LookupConfig(DeviceId);
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

