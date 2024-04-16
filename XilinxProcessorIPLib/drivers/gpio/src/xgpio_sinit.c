/******************************************************************************
* Copyright (C) 2003 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xgpio_sinit.c
* @addtogroup gpio_api GPIO APIs
* @{
*
* The xgpio_sinit.c file contains the implementation of the static initialization functionality
* of the XGpio driver.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 2.01a jvb  10/13/05 First release
* 2.11a mta  03/21/07 Updated to new coding style
* 4.0   sha  07/15/15 Defined macro XPAR_XGPIO_NUM_INSTANCES if not
*		      defined in xparameters.h
* 4.10  gm   07/11/23 Added SDT support.
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xstatus.h"
#include "xparameters.h"
#include "xgpio_i.h"

/************************** Constant Definitions ****************************/

#ifndef XPAR_XGPIO_NUM_INSTANCES
#define XPAR_XGPIO_NUM_INSTANCES		0 /**< GPIO instances */
#endif

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/


/************************** Function Prototypes *****************************/


/******************************************************************************/
/**
* Lookup the device configuration based on the unique device ID.  The table
* ConfigTable contains the configuration info for each device in the system.
*
* @param	DeviceId Device identifier to lookup.
*
* @return
*		- A pointer of data type XGpio_Config which points to the
*		  device configuration if DeviceID is found.
*		- NULL if DeviceID is not found.
*
* @note		None.
*
******************************************************************************/
#ifndef SDT
XGpio_Config *XGpio_LookupConfig(u16 DeviceId)
{
	XGpio_Config *CfgPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XGPIO_NUM_INSTANCES; Index++) {
		if (XGpio_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XGpio_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#else
XGpio_Config *XGpio_LookupConfig(UINTPTR BaseAddress)
{
	XGpio_Config *CfgPtr = NULL;

	int Index;

	for (Index = 0U; XGpio_ConfigTable[Index].Name != NULL; Index++) {
		if ((XGpio_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XGpio_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#endif
/****************************************************************************/
/**
* Initializes the XGpio instance provided by the caller based on the
* given DeviceID.
*
* Only InstancePtr is initialized.
*
* @param	InstancePtr Pointer to an XGpio instance. The memory that the
*		pointer references must be pre-allocated by the caller. Further
*		calls to manipulate the instance/driver through the XGpio API
*		must be made with this pointer.
* @param	DeviceId Unique ID of the device controlled by this XGpio
*		instance. Passing in a device ID associates the generic XGpio
*		instance to a specific device, as chosen by the caller or
*		application developer.
*
* @return
*		- XST_SUCCESS if the initialization was successful.
*		- XST_DEVICE_NOT_FOUND  if the device configuration data was not
*		  found for a device with the supplied device ID.
*
* @note		None.
*
*****************************************************************************/
#ifndef SDT
int XGpio_Initialize(XGpio * InstancePtr, u16 DeviceId)
#else
int XGpio_Initialize(XGpio * InstancePtr, UINTPTR BaseAddress)
#endif
{
	XGpio_Config *ConfigPtr;

	/*
	 * Assert arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/*
	 * Lookup configuration data in the device configuration table.
	 * Use this configuration info down below when initializing this
	 * driver.
	 */
#ifndef SDT
	ConfigPtr = XGpio_LookupConfig(DeviceId);
#else
	ConfigPtr = XGpio_LookupConfig(BaseAddress);
#endif
	if (ConfigPtr == (XGpio_Config *) NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XGpio_CfgInitialize(InstancePtr, ConfigPtr,
				   ConfigPtr->BaseAddress);
}
/** @} */
