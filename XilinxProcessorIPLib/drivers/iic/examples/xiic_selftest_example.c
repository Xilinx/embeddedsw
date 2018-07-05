/******************************************************************************
*
* Copyright (C) 2002 - 2014 Xilinx, Inc.  All rights reserved.
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
* @file xiic_selftest_example.c
*
* This file contains a example for using the IIC hardware device and
* XIic driver.
*
* @note
*
* None
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a sv   05/09/05 Initial release for TestApp integration.
* 2.00a sdm  09/22/09 Updated to use the HAL APIs, replaced call to
*		      XIic_Initialize API with XIic_LookupConfig and
*		      XIic_CfgInitialize. Minor changes made as per
*		      coding guidelines.
* 3.4   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
* </pre>
*
*******************************************************************************/

#include "xparameters.h"
#include "xiic.h"
#include "xil_printf.h"

/************************** Constant Definitions ******************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef TESTAPP_GEN
#define IIC_DEVICE_ID	   XPAR_IIC_0_DEVICE_ID
#endif

/**************************** Type Definitions ********************************/


/***************** Macros (Inline Functions) Definitions **********************/


/************************** Function Prototypes *******************************/

int IicSelfTestExample(u16 DeviceId);

/************************** Variable Definitions ******************************/

/*
 * The following are declared globally so they are zeroed and so they are
 * easily accessible from a debugger.
 */
XIic Iic; /* The driver instance for IIC Device */


/******************************************************************************/
/**
* Main function to call the example. This function is not included if the
* example is generated from the TestAppGen test tool.
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

	/*
	 * Run the example, specify the device ID that is generated in
	 * xparameters.h.
	 */
	Status = IicSelfTestExample(IIC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("IIC selftest Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran IIC selftest Example\r\n");
	return XST_SUCCESS;

}
#endif

/*****************************************************************************/
/**
*
* This function does a selftest on the IIC device and XIic driver as an
* example.
*
* @param	DeviceId is the XPAR_<IIC_instance>_DEVICE_ID value from
*		xparameters.h.
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful.
*
* @note		None.
*
****************************************************************************/
int IicSelfTestExample(u16 DeviceId)
{
	int Status;
	XIic_Config *ConfigPtr;	/* Pointer to configuration data */

	/*
	 * Initialize the IIC driver so that it is ready to use.
	 */
	ConfigPtr = XIic_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}

	Status = XIic_CfgInitialize(&Iic, ConfigPtr, ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Perform a self-test to ensure that the hardware was built
	 * correctly.
	 */
	Status = XIic_SelfTest(&Iic);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
