/******************************************************************************
*
* Copyright (C) 2007 - 2016 Xilinx, Inc.  All rights reserved.
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
/*****************************************************************************/
/**
* @file xhwicap_testapp_example.c
*
* This file contains a design example using the HwIcap device driver and
* hardware device.
*
* @note
*
* This example can be run on a 7 series device, Zynq device, Ultrascale and
* ZynqMP Ultrascale FPGAs.
*
* In a Zynq device the ICAP needs to be selected using the
* XDcfg_SelectIcapInterface API of the DevCfg driver (clear the PCAP_PR bit of
* Control register in the Device Config Interface)  before it can be
* accessed using the HwIcap.
* In case of ZynqMP clear the PCAP_PR bit of pcap_ctrl register in Module
* Configuration Security Unit(CSU) using register write.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 2.00a sv   10/04/07 Initial release.
* 11.0  ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"	/* XPAR parameters */
#include "xhwicap.h"		/* HWICAP device driver */
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place. This definition is not
 * included if the example is generated from the TestAppGen test tool.
 */
#ifndef TESTAPP_GEN
#define HWICAP_DEVICE_ID		XPAR_HWICAP_0_DEVICE_ID
#endif


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int HwIcapTestAppExample(u16 DeviceId);


/************************** Variable Definitions *****************************/

static XHwIcap  HwIcap;	/* The instance of the HWICAP device */

/*****************************************************************************/
/**
*
* Main function to call the HwIcap example. This function is not included if the
* example is generated from the TestAppGen test tool.
*
* @param	None.
*
* @return	XST_SUCCESS to indicate success, otherwise XST_FAILURE.
*
* @note		None.
*
****************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	int Status;

	/*
	 * Run the HwIcap Example, specify the Device ID generated in
	 * xparameters.h
	 */
	Status = HwIcapTestAppExample(HWICAP_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Hwicap testapp Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Hwicap testapp Example\r\n");
	return XST_SUCCESS;
}
#endif

/****************************************************************************/
/**
*
* The purpose of this function is to illustrate the usage of the HwIcap driver.
*
* @param	HwIcapDeviceId is device ID of the XHwIcap Device, typically
*		XPAR_<HWICAP_instance>_DEVICE_ID value from xparameters.h
*
* @return	XST_SUCCESS to indicate success, otherwise XST_FAILURE.
*
* @note		None.
*
****************************************************************************/
int HwIcapTestAppExample(u16 HwIcapDeviceId)
{
	int Status;

	XHwIcap_Config *ConfigPtr;
	u32 ConfigRegData;

	/*
	 * Initialize the HwIcap driver.
	 */
	ConfigPtr = XHwIcap_LookupConfig(HwIcapDeviceId);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}

	Status = XHwIcap_CfgInitialize(&HwIcap, ConfigPtr,
				ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Perform a self-test to ensure that the hardware was built correctly.
	 */
	Status = XHwIcap_SelfTest(&HwIcap);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Read the ID Code register inside the FPGA.
	 */
	Status = XHwIcap_GetConfigReg(&HwIcap, XHI_IDCODE, &ConfigRegData);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	return XST_SUCCESS;
}
