// ==============================================================
// Copyright (c) 2015 - 2022 Xilinx Inc. All rights reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
*
* @file xv_mix_sinit.c
* @addtogroup v_mix Overview
*/

#ifndef __linux__

#include "xstatus.h"
#ifndef SDT
#include "xparameters.h"
#endif
#include "xv_mix.h"

extern XV_mix_Config XV_mix_ConfigTable[];

#ifndef SDT
/**
 * Looks up the hardware configuration for a given device ID.
 *
 * This function searches the configuration table for an entry that matches the
 * specified DeviceId. If a match is found, it returns a pointer to the corresponding
 * XV_mix_Config structure. If no match is found, it returns NULL.
 *
 * @param DeviceId The unique identifier for the device whose configuration is to be found.
 *
 * @return
 *   - Pointer to the XV_mix_Config structure corresponding to the DeviceId, if found.
 *   - NULL if no matching configuration is found.
 */
XV_mix_Config *XV_mix_LookupConfig(u16 DeviceId) {
	XV_mix_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XV_MIX_NUM_INSTANCES; Index++) {
		if (XV_mix_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XV_mix_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

/**
 * Initializes the XV_mix instance.
 *
 * This function initializes a specific XV_mix device instance using the
 * provided DeviceId. It looks up the device configuration, and if found,
 * initializes the instance with the configuration data. If the configuration
 * is not found, it sets the instance as not ready and returns an error code.
 *
 * @param InstancePtr Pointer to the XV_mix instance to be initialized.
 * @param DeviceId    Unique identifier for the device to initialize.
 *
 * @return
 *   - XST_SUCCESS if initialization is successful.
 *   - XST_DEVICE_NOT_FOUND if the device configuration is not found.
 */
int XV_mix_Initialize(XV_mix *InstancePtr, u16 DeviceId) {
	XV_mix_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XV_mix_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XV_mix_CfgInitialize(InstancePtr,
			                    ConfigPtr,
			                    ConfigPtr->BaseAddress);
}
#else
/**
 * Looks up the configuration for a XV_mix device.
 *
 * This function searches the XV_mix_ConfigTable for a configuration entry
 * that matches the specified BaseAddress. If a matching entry is found,
 * a pointer to its configuration structure is returned. If BaseAddress is
 * zero, the function returns the first entry in the table.
 *
 * @param	BaseAddress	The base address of the XV_mix device to look up.
 *
 * @return	A pointer to the configuration structure if found, or NULL if
 *          no matching entry exists.
 */
XV_mix_Config *XV_mix_LookupConfig(UINTPTR BaseAddress) {
	XV_mix_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; XV_mix_ConfigTable[Index].Name != NULL; Index++) {
		if ((XV_mix_ConfigTable[Index].BaseAddress == BaseAddress) ||
				!BaseAddress) {
			ConfigPtr = &XV_mix_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

/**
 * Initialize the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance to be initialized.
 * @param BaseAddress Base address of the device configuration.
 *
 * This function looks up the configuration for the device using the provided
 * base address, and initializes the XV_mix instance with the configuration
 * parameters. If the configuration is not found, the function sets the
 * IsReady flag to 0 and returns XST_DEVICE_NOT_FOUND.
 *
 * @return XST_SUCCESS if initialization is successful,
 *         XST_DEVICE_NOT_FOUND if the configuration is not found.
 */
int XV_mix_Initialize(XV_mix *InstancePtr, UINTPTR BaseAddress) {
	XV_mix_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XV_mix_LookupConfig(BaseAddress);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XV_mix_CfgInitialize(InstancePtr,
			            ConfigPtr,
			            ConfigPtr->BaseAddress);
}
#endif
#endif
