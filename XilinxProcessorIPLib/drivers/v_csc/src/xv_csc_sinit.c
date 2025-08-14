// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_csc_sinit.c
 * @addtogroup v_csc Overview
*/
#ifndef __linux__

#include "xstatus.h"
#ifndef SDT
#include "xparameters.h"
#endif
#include "xv_csc.h"

#ifndef XPAR_XV_CSC_NUM_INSTANCES
#define XPAR_XV_CSC_NUM_INSTANCES   0
#endif

extern XV_csc_Config XV_csc_ConfigTable[];

#ifndef SDT
/**
 * XV_csc_LookupConfig - Looks up the configuration for a specific Color Space Converter (CSC) device.
 *
 * @param DeviceId: The unique identifier for the device instance to look up.
 *
 * This function searches the configuration table for an entry that matches the
 * provided DeviceId. If a matching entry is found, a pointer to its configuration
 * structure is returned. If no match is found, NULL is returned.
 *
 * @return
 *   - Pointer to the configuration structure for the matching device, or
 *   - NULL if no matching device is found.
 */

XV_csc_Config *XV_csc_LookupConfig(u16 DeviceId) {
    XV_csc_Config *ConfigPtr = NULL;

    int Index;

    for (Index = 0; Index < XPAR_XV_CSC_NUM_INSTANCES; Index++) {
        if (XV_csc_ConfigTable[Index].DeviceId == DeviceId) {
            ConfigPtr = &XV_csc_ConfigTable[Index];
            break;
        }
    }

    return ConfigPtr;
}

/**
 * XV_csc_Initialize - Initialize the Color Space Converter (CSC) core.
 *
 * This function initializes an instance of the XV_csc driver. It looks up the
 * configuration for the specified device ID, and if found, initializes the
 * hardware and driver instance with the configuration data.
 *
 * @param InstancePtr Pointer to the XV_csc instance to be initialized.
 * @param DeviceId    Device ID of the CSC core to look up the configuration.
 *
 * @return
 *   - XST_SUCCESS if initialization was successful.
 *   - XST_DEVICE_NOT_FOUND if the configuration for the given DeviceId was not found.
 *
 * @note
 *   - The InstancePtr argument must not be NULL.
 *   - This function must be called before using other functions of the driver.
 */
int XV_csc_Initialize(XV_csc *InstancePtr, u16 DeviceId) {
    XV_csc_Config *ConfigPtr;

    Xil_AssertNonvoid(InstancePtr != NULL);

    ConfigPtr = XV_csc_LookupConfig(DeviceId);
    if (ConfigPtr == NULL) {
        InstancePtr->IsReady = 0;
        return (XST_DEVICE_NOT_FOUND);
    }

    return XV_csc_CfgInitialize(InstancePtr,
                                ConfigPtr,
                                ConfigPtr->BaseAddress);
}
#else
/**
 * XV_csc_LookupConfig - Looks up the configuration for a Color Space Converter (CSC) instance.
 *
 * @param BaseAddress: The base address of the CSC instance to look up. If 0, returns the first entry.
 *
 * This function searches the XV_csc_ConfigTable for an entry matching the provided BaseAddress.
 * If a matching BaseAddress is found, or if BaseAddress is 0 (in which case the first entry is returned),
 * a pointer to the corresponding configuration structure is returned. If no match is found,
 * NULL is returned.
 *
 * @return
 *   - Pointer to the configuration structure if found.
 *   - NULL if no matching configuration is found.
 */

XV_csc_Config *XV_csc_LookupConfig(UINTPTR BaseAddress) {
    XV_csc_Config *ConfigPtr = NULL;

    int Index;

    for (Index = (u32)0x0; XV_csc_ConfigTable[Index].Name != NULL; Index++) {
		if ((XV_csc_ConfigTable[Index].BaseAddress == BaseAddress) ||
		     !BaseAddress) {
			ConfigPtr = &XV_csc_ConfigTable[Index];
			break;
		}
	}

    return ConfigPtr;
}

/**
 * XV_csc_Initialize - Initializes an instance of the XV_csc driver.
 *
 * @param InstancePtr: Pointer to the XV_csc instance to be initialized.
 * @param BaseAddress: Base address of the device to be looked up and initialized.
 *
 * This function looks up the configuration for a device using the provided
 * base address, and then initializes the XV_csc instance with the found
 * configuration. If the configuration cannot be found, the function sets
 * the instance's IsReady flag to 0 and returns XST_DEVICE_NOT_FOUND.
 *
 * @return
 *   - XST_SUCCESS if initialization was successful.
 *   - XST_DEVICE_NOT_FOUND if the device configuration could not be found.
 */

int XV_csc_Initialize(XV_csc *InstancePtr, UINTPTR BaseAddress) {
    XV_csc_Config *ConfigPtr;

    Xil_AssertNonvoid(InstancePtr != NULL);

    ConfigPtr = XV_csc_LookupConfig(BaseAddress);
    if (ConfigPtr == NULL) {
        InstancePtr->IsReady = 0;
        return (XST_DEVICE_NOT_FOUND);
    }

    return XV_csc_CfgInitialize(InstancePtr,
                                ConfigPtr,
                                ConfigPtr->BaseAddress);
}
#endif
#endif
