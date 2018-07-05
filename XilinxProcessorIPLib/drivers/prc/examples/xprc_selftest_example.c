/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc.  All rights reserved.
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
*
* @file xprc_selftest_example.c
*
* This file contains a design example using the PRC driver (XPrc) to do
* self test on the device.
*
* @note		None
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who     Date	    Changes
* ---- ---- ------------ -------------------------------------------
* 1.0   ms   07/18/2016   First release
*       ms   04/05/2017   Modified comment lines notation in functions to
*                         avoid unnecessary description displayed
*                         while generating doxygen.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xprc.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/**
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define XPRC_DEVICE_ID	XPAR_PRC_DEVICE_ID

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

u32 XPrc_SelfTestExample(u16 DeviceId);
extern u32 XPrc_GetRegisterAddress(XPrc *InstancePtr, u32 VsmId,
			u8 RegisterType, u16 TableRow);
extern u32 XPrc_GetRegisterOffset(XPrc *InstancePtr, u32 VsmId, u8 BankId,
			u8 TableInBank, u8 TableRow);
extern u16 XPrc_GetOffsetForTableType(u8 TableId);

/************************** Variable Definitions *****************************/

XPrc Prc;		/* Instance of the PRC */

/*****************************************************************************/
/**
*
* This is the main function to call the example.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	u32 Status;

	/*
	 * Run the PRC self test example, specify the Device ID that is
	 * generated in xparameters.h
	 */
	Status = XPrc_SelfTestExample((u16)XPRC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("PRC Selftest example is failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran PRC Selftest example\r\n");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function does a minimal test on the PRC device and driver as a
* design example. The purpose of this function is to illustrate
* how to use the XPrc component.
*
*
* @param	DeviceId is the XPAR_<prc_instance>_DEVICE_ID value from
*		xparameters.h.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed.
*
* @note		None.
*
******************************************************************************/
u32 XPrc_SelfTestExample(u16 DeviceId)
{
	int Status;
	XPrc_Config *CfgPtr;

	/*
	 * Initialize the PRC driver so that it's ready to use.
	 * Look up the configuration in the config table, then initialize it.
	 */
	CfgPtr = XPrc_LookupConfig(DeviceId);
	if (NULL == CfgPtr) {
		return XST_FAILURE;
	}

	Status = XPrc_CfgInitialize(&Prc, CfgPtr, CfgPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built correctly.
	 */
	Status = XPrc_SelfTest(&Prc);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
