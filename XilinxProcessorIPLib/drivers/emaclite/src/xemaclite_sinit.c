/******************************************************************************
* Copyright (C) 2007 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xemaclite_sinit.c
* @addtogroup emaclite Overview
* @{
*
* @brief Implements EmacLite static initialization and lookup helpers.
*
* Contains the implementation of the XEmacLite driver's static
* initialization functionality.
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
 * @brief Implementation of XEmacLite_LookupConfig(). See xemaclite.h for the API
 * description.
 *
 *****************************************************************************/
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
* @brief Initialize a specific XEmacLite instance/driver.  The initialization entails:
* - Initialize fields of the XEmacLite instance structure.
*
* The driver defaults to polled mode operation.
*
* @param[in] InstancePtr is a pointer to the XEmacLite instance.
* @if SDT
* @param[in] BaseAddress contains the base address of the device.
* @else
* @param[in] DeviceId is the unique id of the device controlled by this
*		XEmacLite instance. Passing in a device id associates the
*		generic XEmacLite instance to a specific device, as chosen by
*		the caller or application developer.
* @endif
*
* @return
* 		- XST_SUCCESS if initialization was successful.
* 		- XST_DEVICE_NOT_FOUND/XST_FAILURE if device configuration
*		information was not found for a device with the supplied
*		device ID.
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
