// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_vscaler_sinit.c
 * @addtogroup xv_vscaler Overview
 */

#ifndef __linux__

#include "xstatus.h"
#ifndef SDT
#include "xparameters.h"
#endif
#include "xv_vscaler.h"

#ifndef XPAR_XV_VSCALER_NUM_INSTANCES
#define XPAR_XV_VSCALER_NUM_INSTANCES   0
#endif

extern XV_vscaler_Config XV_vscaler_ConfigTable[];

#ifndef SDT
/**
 * Looks up the configuration for a specific XV_vscaler device instance.
 *
 * This function searches the XV_vscaler configuration table for an entry that matches
 * the specified DeviceId. If a matching configuration is found, a pointer to the
 * configuration structure is returned; otherwise, NULL is returned.
 *
 * @param DeviceId The unique identifier of the XV_vscaler device instance to look up.
 * @return Pointer to the XV_vscaler_Config structure if found, or NULL if no matching
 *         configuration exists.
 */
XV_vscaler_Config *XV_vscaler_LookupConfig(u16 DeviceId) {
    XV_vscaler_Config *ConfigPtr = NULL;

    int Index;

    for (Index = 0; Index < XPAR_XV_VSCALER_NUM_INSTANCES; Index++) {
        if (XV_vscaler_ConfigTable[Index].DeviceId == DeviceId) {
            ConfigPtr = &XV_vscaler_ConfigTable[Index];
            break;
        }
    }

    return ConfigPtr;
}

/**
 * Initializes a specific XV_vscaler device instance.
 *
 * This function initializes the XV_vscaler device pointed to by InstancePtr
 * using the configuration data corresponding to the specified DeviceId.
 * It first looks up the configuration for the given DeviceId. If the
 * configuration is found, it calls XV_vscaler_CfgInitialize to initialize
 * the device instance. If the configuration is not found, the function
 * sets the IsReady flag of the instance to 0 and returns
 * XST_DEVICE_NOT_FOUND.
 *
 * @param InstancePtr Pointer to the XV_vscaler instance to be initialized.
 * @param DeviceId    Unique identifier for the XV_vscaler device instance.
 *
 * @return XST_SUCCESS if initialization is successful, or
 *         XST_DEVICE_NOT_FOUND if the configuration for the specified
 *         DeviceId does not exist.
 */
int XV_vscaler_Initialize(XV_vscaler *InstancePtr, u16 DeviceId) {
    XV_vscaler_Config *ConfigPtr;

    Xil_AssertNonvoid(InstancePtr != NULL);

    ConfigPtr = XV_vscaler_LookupConfig(DeviceId);
    if (ConfigPtr == NULL) {
        InstancePtr->IsReady = 0;
        return (XST_DEVICE_NOT_FOUND);
    }

    return XV_vscaler_CfgInitialize(InstancePtr,
                                    ConfigPtr,
                                    ConfigPtr->BaseAddress);
}
#else
/**
 * XV_vscaler_LookupConfig - Looks up the configuration for a V-Scaler instance.
 *
 * This function searches the XV_vscaler_ConfigTable for a configuration entry
 * that matches the specified BaseAddress. If a matching entry is found, a pointer
 * to its configuration structure is returned. If BaseAddress is zero or NULL, the
 * first entry in the table is returned. If no matching entry is found, NULL is returned.
 *
 * @param  BaseAddress  The base address of the V-Scaler instance to look up.
 *
 * @return Pointer to the configuration structure if found, otherwise NULL.
 */
XV_vscaler_Config *XV_vscaler_LookupConfig(UINTPTR BaseAddress) {
    XV_vscaler_Config *ConfigPtr = NULL;

    int Index;

    for (Index = (u32)0x0; XV_vscaler_ConfigTable[Index].Name != NULL; Index++) {
		if ((XV_vscaler_ConfigTable[Index].BaseAddress == BaseAddress) ||
		     !BaseAddress) {
			ConfigPtr = &XV_vscaler_ConfigTable[Index];
			break;
		}
    }

    return ConfigPtr;
}

/**
 * XV_vscaler_Initialize - Initialize the XV_vscaler instance.
 *
 * This function initializes an XV_vscaler instance using the configuration
 * found for the given BaseAddress. It performs a lookup for the configuration
 * structure, validates the instance pointer, and calls the configuration
 * initialization function. If the configuration is not found, it marks the
 * instance as not ready and returns an error code.
 *
 * @param InstancePtr Pointer to the XV_vscaler instance to be initialized.
 * @param BaseAddress Base address of the device to look up the configuration.
 *
 * @return
 *   - XST_SUCCESS if initialization is successful.
 *   - XST_DEVICE_NOT_FOUND if the configuration for the given base address is not found.
 */
int XV_vscaler_Initialize(XV_vscaler *InstancePtr, UINTPTR BaseAddress) {
    XV_vscaler_Config *ConfigPtr;

    Xil_AssertNonvoid(InstancePtr != NULL);

    ConfigPtr = XV_vscaler_LookupConfig(BaseAddress);
    if (ConfigPtr == NULL) {
        InstancePtr->IsReady = 0;
        return (XST_DEVICE_NOT_FOUND);
    }

    return XV_vscaler_CfgInitialize(InstancePtr,
                                    ConfigPtr,
                                    ConfigPtr->BaseAddress);
}
#endif
#endif
