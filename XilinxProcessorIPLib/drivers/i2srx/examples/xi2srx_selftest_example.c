/*******************************************************************************
 *
 * Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
 *
 ******************************************************************************/
/******************************************************************************/
/**
 * @file xi2srx_selftest_example.c
 *
 * This file contains a example for using the I2S receiver hardware device
 * and I2S receiver driver.
 *
 *
 * <pre> MODIFICATION HISTORY:
 *
 * Ver	Who	   Date		Changes
 * ---- ---	 --------	-----------------------------------------------
 * 1.0	kar	  01/25/18	First release
 * </pre>
 *
 ****************************************************************************/

/***************************** Include Files **********************************/
#include "xparameters.h"
#include "xi2srx.h"
#include "xil_printf.h"

/************************** Constant Definitions ******************************/
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef TESTAPP_GEN
#define I2S_RX_DEVICE_ID	XPAR_XI2SRX_0_DEVICE_ID
#endif

/**************************** Type Definitions ********************************/

/************************** Function Prototypes *******************************/

int I2srx_SelfTest_Example(u16 DeviceId);

/************************** Variable Definitions ******************************/

XI2s_Rx I2s_rx;		/* Instance of the I2S Receiver device */

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
 ******************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	int Status;
	xil_printf("I2S Self Test Example \r\n");
	/*
	 * Run the I2S RX Self Test example, specify the Device ID that is
	 * generated in xparameters.h
	 */
	Status = I2srx_SelfTest_Example(I2S_RX_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("I2S RX Self Test Failed\r\n");
		return XST_FAILURE;
	}
	xil_printf("Successfully ran I2S RX Self Test Example Test\r\n");
	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
 *
 * This function does a minimal test on the I2S receiver device and
 * driver as a design example. The purpose of this function is to illustrate
 * how to use the xi2s_receiver component.
 *
 *
 * @param	DeviceId is the Device ID of the I2S RX Device and is the
 *		XPAR_<I2S_receiver_instance>_DEVICE_ID value from xparameters.h
 *
 * @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
 *
 * @note		None.
 *
 *
 ******************************************************************************/
int I2srx_SelfTest_Example(u16 DeviceId)
{
	int Status;
	XI2srx_Config *Config;
	/*
	 * Initialize the I2S RX driver so that it's ready to use
	 * Look up the configuration in the config table, then initialize it.
	 */
	Config = XI2s_Rx_LookupConfig(DeviceId);
	if (Config == NULL)
		return XST_FAILURE;

	Status = XI2s_Rx_CfgInitialize(&I2s_rx, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	return Status;
}
