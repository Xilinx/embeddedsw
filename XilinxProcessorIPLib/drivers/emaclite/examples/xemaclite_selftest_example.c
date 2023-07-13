/******************************************************************************
* Copyright (C) 2004 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xemaclite_selftest_example.c
*
* This file contains a design example using the EmacLite driver.
*
* @note
*
* None.
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a ecm  01/25/05 Initial release for TestApp integration.
* 1.00a sv   06/06/05 Minor changes to comply to Doxygen and coding guidelines
* 2.00a ktn  04/14/09 Removed support for TestApp
* 4.3   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
*
******************************************************************************/
/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xemaclite.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifdef SDT
#define EMACLITE_BASEADDR	XPAR_XEMACLITE_0_BASEADDR
#else
#define EMAC_DEVICE_ID			XPAR_EMACLITE_0_DEVICE_ID
#endif

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
#ifdef SDT
int EMACLiteSelfTestExample(UINTPTR BaseAddress);
#else
int EMACLiteSelfTestExample(u16 DeviceId);
#endif

/************************** Variable Definitions *****************************/

/*
 * Instance of the driver
 */
static XEmacLite EmacLite;

/****************************************************************************/
/**
*
* This function is the main function of the EmacLite selftest example.
*
* @param	None
*
* @return	XST_SUCCESS to indicate success, else XST_FAILURE.
*
* @note		None
*
*****************************************************************************/
int main(void)
{
	int Status;

	/*
	 * Run the EmacLite Self test example, specify the Device ID that is
	 * generated in xparameters.h
	 */
#ifdef SDT
	Status = EMACLiteSelfTestExample(EMACLITE_BASEADDR);
#else
	Status = EMACLiteSelfTestExample(EMAC_DEVICE_ID);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("Emaclite selftest Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Emaclite selftest Example\r\n");
	return XST_SUCCESS;

}

/*****************************************************************************/
/**
*
* The main entry point for the EmacLite driver selftest example.
*
* @param	DeviceId is the XPAR_<xemaclite_instance>_DEVICE_ID value from
*		xparameters.h
*
* @return	XST_SUCCESS to indicate success, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
#ifdef SDT
int EMACLiteSelfTestExample(UINTPTR BaseAddress)
#else
int EMACLiteSelfTestExample(u16 DeviceId)
#endif
{
	int Status;
	XEmacLite_Config *ConfigPtr;
	XEmacLite *InstancePtr = &EmacLite;

	/*
	 * Initialize the EmacLite device.
	 */
#ifdef SDT
	ConfigPtr = XEmacLite_LookupConfig(BaseAddress);
#else
	ConfigPtr = XEmacLite_LookupConfig(DeviceId);
#endif
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}
	Status = XEmacLite_CfgInitialize(InstancePtr,
					 ConfigPtr,
					 ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Run the Self Test
	 */
	Status = XEmacLite_SelfTest(InstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
