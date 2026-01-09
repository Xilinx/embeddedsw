/******************************************************************************
* Copyright (C) 2005 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xiic_sinit.c
* @addtogroup Overview
* @{
*
* The implementation of the Xiic component's static initialization functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- --- ------- -----------------------------------------------
* 1.02a jvb  10/13/05 release
* 1.13a wgr  03/22/07 Converted to new coding style.
* 2.00a ktn  10/22/09 Converted all register accesses to 32 bit access.
*		      Updated to use the HAL APIs/macros.
*		      Some of the macros have been renamed to remove _m from
*		      the name and some of the macros have been renamed to be
*		      consistent, see the xiic_i.h and xiic_l.h files for further
*		      information
* 3.10  gm   07/09/23 Added SDT support.
* 3.15  vlt  11/05/25 Add 64-bit Addressing support.
* 3.15  vlt  12/12/25 Update Doxygen comments to include SDT flow details.
* </pre>
*
****************************************************************************/

/***************************** Include Files *******************************/

#include "xstatus.h"
#include "xparameters.h"
#include "xiic_i.h"

/************************** Constant Definitions ***************************/


/**************************** Type Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *******************/


/************************** Function Prototypes ****************************/

/************************** Variable Definitions **************************/


/*****************************************************************************/
/**
*
* Looks up the device configuration based on the unique device ID/BaseAddress.
* The XIic_ConfigTable[] contains the configuration info for each device in
* the system.
*
* @if SDT
* @param    BaseAddress contains the base address of the device
* @else
* @param    DeviceId contains the unique ID of the device
* @endif
*
* @return   A pointer to the configuration found or NULL if the specified
*           device ID/BaseAddress was not found. See xiic.h for the
*           definition of XIic_Config.
*
* @note        In XSCT/classic flow, DeviceId is used to look up the device
*              configuration.
*
******************************************************************************/
#ifndef SDT
XIic_Config *XIic_LookupConfig(u16 DeviceId)
{
	XIic_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0; Index < XPAR_XIIC_NUM_INSTANCES; Index++) {
		if (XIic_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XIic_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#else
XIic_Config *XIic_LookupConfig(UINTPTR BaseAddress)
{
	XIic_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0; XIic_ConfigTable[Index].Name != NULL; Index++) {
		if (XIic_ConfigTable[Index].BaseAddress == BaseAddress ||
		    !BaseAddress) {
			CfgPtr = &XIic_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#endif
/*****************************************************************************/
/**
*
* Initializes a specific XIic instance.  The initialization entails:
*
* - Check the device has an entry in the configuration table.
* - Initialize the driver to allow access to the device registers and
*   initialize other subcomponents necessary for the operation of the device.
* - Default options to:
*     - 7-bit slave addressing
*     - Send messages as a slave device
*     - Repeated start off
*     - General call recognition disabled
* - Clear messageing and error statistics
*
* The XIic_Start() function must be called after this function before the device
* is ready to send and receive data on the IIC bus.
*
* Before XIic_Start() is called, the interrupt control must connect the ISR
* routine to the interrupt handler. This is done by the user, and not
* XIic_Start() to allow the user to use an interrupt controller of their choice.
*
* @param	InstancePtr is a pointer to the XIic instance to be worked on.
* @if SDT
* @param	BaseAddress contains the base address of the device
* @else
* @param	DeviceId is the unique id of the device controlled by this XIic
*		instance.  Passing in a device id associates the generic XIic
*		instance to a specific device, as chosen by the caller or
*		application developer.
* @endif
*
* @return
*		- XST_SUCCESS when successful
*		- XST_DEVICE_NOT_FOUND indicates the given device id/BaseAddress isn't found
*		- XST_DEVICE_IS_STARTED indicates the device is started
*		(i.e. interrupts enabled and messaging is possible).
*		Must stop before re-initialization is allowed.
*
* @note		In XSCT/classic flow, DeviceId is used to look up the device
*		configuration.
*
****************************************************************************/
#ifndef SDT
int XIic_Initialize(XIic *InstancePtr, u16 DeviceId)
#else
int XIic_Initialize(XIic *InstancePtr, UINTPTR BaseAddress)
#endif
{
	XIic_Config *ConfigPtr;	/* Pointer to configuration data */

	/*
	 * Asserts test the validity of selected input arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/*
	 * Lookup the device configuration in the temporary CROM table. Use this
	 * configuration info down below when initializing this component.
	 */
#ifndef SDT
	ConfigPtr = XIic_LookupConfig(DeviceId);
#else
	ConfigPtr = XIic_LookupConfig(BaseAddress);
#endif
	if (ConfigPtr == NULL) {
		return XST_DEVICE_NOT_FOUND;
	}

	return XIic_CfgInitialize(InstancePtr, ConfigPtr,
				  ConfigPtr->BaseAddress);
}
/** @} */
