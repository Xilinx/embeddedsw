/*******************************************************************************
 *
 * Copyright (C) 2017 - 2018 Xilinx, Inc.  All rights reserved.
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
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
 ******************************************************************************/
/******************************************************************************/
/**
 * @file xsdiaud_selftest_example.c
 *
 * This file contains a example for using the SdiAud hardware device
 * and SdiAud driver.
 *
 *
 * <pre> MODIFICATION HISTORY:
 *
 * Ver	 Who	Date		Changes
 * ----  ---	--------	-----------------------------------------------
 * 1.0	 kar	02/16/18	First release
 * </pre>
 *
 ****************************************************************************/

/***************************** Include Files **********************************/
#include "xparameters.h"
#include "xsdiaud.h"
#include "xil_printf.h"

/************************** Constant Definitions ******************************/
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef TESTAPP_GEN
#define SDIAUD_0_DEVICE_ID	XPAR_SDI_RX_PATH_V_UHDSDI_AUDIO_DEMUX_DEVICE_ID
#define SDIAUD_1_DEVICE_ID	XPAR_SDI_TX_PATH_V_UHDSDI_AUDIO_MUX_DEVICE_ID
#endif

/**************************** Type Definitions ********************************/

/************************** Function Prototypes *******************************/

int SdiAud_SelfTestExample(u16 DeviceId);

/************************** Variable Definitions ******************************/

XSdiAud SdiAud;		/* Instance of the SdiAud device */

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
 *****************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	int Status;

	xil_printf("SDI Audio Self Test Example \r\n");

	/*
	 * Run the SdiAud Self Test example, specify the Device ID that is
	 * generated in xparameters.h
	 */
	Status = SdiAud_SelfTestExample(SDIAUD_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Sdi Audio Self Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Sdi Audio Self Test Example Test\r\n");
	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
 *
 * This function does a minimal test on the SdiAud device and
 * driver as a design example.
 *
 * @param	DeviceId is the Device ID of the SdiAud Device and is the
 *		XPAR_<sdiaud_instance>_DEVICE_ID value from xparameters.h
 *
 * @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
 *
 * @note		None.
 *
 *
 *****************************************************************************/
int SdiAud_SelfTestExample(u16 DeviceId)
{
	int Status;
	XSdiAud_Config *Config;
	/*
	 * Initialize the SdiAud driver so that it's ready to use
	 * Look up the configuration in the config table, then initialize it.
	 */
	Config = XSdiAud_LookupConfig(DeviceId);

	if (Config == NULL)
		return XST_FAILURE;

	Status = XSdiAud_CfgInitialize(&SdiAud, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	return Status;
}
/** @} */
