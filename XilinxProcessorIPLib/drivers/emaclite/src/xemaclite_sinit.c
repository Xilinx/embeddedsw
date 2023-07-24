/******************************************************************************
* Copyright (C) 2007 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xemaclite_sinit.c
* @addtogroup emaclite Overview
* @{
*
* This file contains the implementation of the XEmacLite driver's static
* initialization functionality.
*
* @note		None.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.12a sv   11/28/07 First release
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#ifndef SDT
#include "xparameters.h"
#endif
#include "xemaclite.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
extern XEmacLite_Config XEmacLite_ConfigTable[];

/*****************************************************************************/
/**
*
* Lookup the device configuration based on the unique device ID.  The table
* XEmacLite_ConfigTable contains the configuration info for each device in the
* system.
*
* @param 	DeviceId is the unique device ID of the device being looked up.
*
* @return	A pointer to the configuration table entry corresponding to the
*		given device ID, or NULL if no match is found.
*
* @note		None.
*
******************************************************************************/
#ifdef SDT
XEmacLite_Config *XEmacLite_LookupConfig(UINTPTR BaseAddress)
{
	XEmacLite_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = (u32)0x0; XEmacLite_ConfigTable[Index].Name != NULL; Index++) {
		if ((XEmacLite_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XEmacLite_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#else
XEmacLite_Config *XEmacLite_LookupConfig(u16 DeviceId)
{
	XEmacLite_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0; Index < XPAR_XEMACLITE_NUM_INSTANCES; Index++) {
		if (XEmacLite_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XEmacLite_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#endif

/*****************************************************************************/
/**
*
* Initialize a specific XEmacLite instance/driver.  The initialization entails:
* - Initialize fields of the XEmacLite instance structure.
*
* The driver defaults to polled mode operation.
*
* @param	InstancePtr is a pointer to the XEmacLite instance.
* @param 	DeviceId is the unique id of the device controlled by this
*		XEmacLite instance.  Passing in a device id associates the
*		generic XEmacLite instance to a specific device, as chosen by
*		the caller or application developer.
*
* @return
* 		- XST_SUCCESS if initialization was successful.
* 		- XST_DEVICE_NOT_FOUND/XST_FAILURE if device configuration
*		information was not found for a device with the supplied
*		device ID.
*
* @note		None
*
******************************************************************************/
#ifdef SDT
int XEmacLite_Initialize(XEmacLite *InstancePtr, UINTPTR BaseAddress)
#else
int XEmacLite_Initialize(XEmacLite *InstancePtr, u16 DeviceId)
#endif
{
	int Status;
	XEmacLite_Config *EmacLiteConfigPtr;/* Pointer to Configuration data. */

	/*
	 * Verify that each of the inputs are valid.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/*
	 * Lookup the device configuration in the configuration table. Use this
	 * configuration info down below when initializing this driver.
	 */
#ifdef SDT
	EmacLiteConfigPtr = XEmacLite_LookupConfig(BaseAddress);
#else
	EmacLiteConfigPtr = XEmacLite_LookupConfig(DeviceId);
#endif
	if (EmacLiteConfigPtr == NULL) {
		return XST_DEVICE_NOT_FOUND;
	}

	Status = XEmacLite_CfgInitialize(InstancePtr,
					 EmacLiteConfigPtr,
					 EmacLiteConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}


/** @} */
