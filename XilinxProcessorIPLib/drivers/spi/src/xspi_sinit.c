/******************************************************************************
* Copyright (C) 2005 - 2023 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xspi_sinit.c
*
* @brief Implements the static initialization functionality of the XSpi
* component.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.01a jvb  10/13/05 First release
* 1.11a wgr  03/22/07 Converted to new coding style.
* 4.11  sb   07/11/23 Added support for system device-tree flow.
* 4.16  vlt  12/18/25 Update Doxygen comments to include SDT flow details.
* 4.17  tb   09/30/26 Align Doxygen comments with AMD documentation standards
*
* </pre>
*
******************************************************************************/

/**
* @addtogroup spi_api SPI APIs
* @{
*/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xspi.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

extern XSpi_Config XSpi_ConfigTable[];

/*****************************************************************************/
/**
*
* @brief Looks up the device configuration based on the unique device
* ID/BaseAddress.
*
* The XSpi_ConfigTable[] contains the configuration info for each device in the
* system.
*
* @if SDT
* @param[in]	BaseAddress Base address of the device.
* @else
* @param[in]	DeviceId Unique ID of the device.
* @endif
*
* @return       A pointer to the configuration found or NULL if the specified
*               device ID/BaseAddress was not found. See xspi.h for the
*               definition of XSpi_Config.
*
* @note        In XSCT/classic flow, the device configuration is looked up
*              by DeviceId.
*
******************************************************************************/
#ifndef SDT
XSpi_Config *XSpi_LookupConfig(u16 DeviceId)
{
	XSpi_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0; Index < XPAR_XSPI_NUM_INSTANCES; Index++) {
		if (XSpi_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XSpi_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#else
XSpi_Config *XSpi_LookupConfig(UINTPTR BaseAddress)
{
	XSpi_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; XSpi_ConfigTable[Index].Name != NULL; Index++) {
		if ((XSpi_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XSpi_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#endif

/*****************************************************************************/
/**
*
* @brief Initializes a specific XSpi instance such that the driver is ready to
* use.
*
* The state of the device after initialization is:
*	- Device is disabled
*	- Slave mode
*	- Active high clock polarity
*	- Clock phase 0
*
* @param[in]	InstancePtr Pointer to the XSpi instance to be worked on.
* @if SDT
* @param[in]	BaseAddress Base address of the device.
* @else
* @param[in]	DeviceId Unique id of the device controlled by this XSpi
*		instance. Passing in a device id associates the generic XSpi
*		instance to a specific device, as chosen by the caller or
*		application developer.
* @endif
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_DEVICE_IS_STARTED if the device is started. It must be
*		  stopped to re-initialize.
*		- XST_DEVICE_NOT_FOUND if the device was not found in the
*		  configuration such that initialization could not be
*		  accomplished.
*
* @note		In XSCT/classic flow, the device configuration is looked up
*		by DeviceId.
*
******************************************************************************/
#ifndef SDT
int XSpi_Initialize(XSpi *InstancePtr, u16 DeviceId)
#else
int XSpi_Initialize(XSpi *InstancePtr, UINTPTR BaseAddress)
#endif
{
	XSpi_Config *ConfigPtr;	/* Pointer to Configuration ROM data */

	Xil_AssertNonvoid(InstancePtr != NULL);

	/*
	 * Lookup the device configuration in the temporary CROM table. Use this
	 * configuration info down below when initializing this component.
	 */
#ifndef SDT
	ConfigPtr = XSpi_LookupConfig(DeviceId);
#else
	ConfigPtr = XSpi_LookupConfig(BaseAddress);
#endif
	if (ConfigPtr == NULL) {
		return XST_DEVICE_NOT_FOUND;
	}

	return XSpi_CfgInitialize(InstancePtr, ConfigPtr,
				  ConfigPtr->BaseAddress);

}
/** @} */
