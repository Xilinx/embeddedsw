// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_letterbox_sinit.c
 * @addtogroup xv_letterbox Overview
 */

#ifndef __linux__

#include "xstatus.h"
#ifndef SDT
#include "xparameters.h"
#endif
#include "xv_letterbox.h"

#ifndef XPAR_XV_LETTERBOX_NUM_INSTANCES
#define XPAR_XV_LETTERBOX_NUM_INSTANCES   0
#endif

extern XV_letterbox_Config XV_letterbox_ConfigTable[];

#ifndef SDT

/**
 * XV_letterbox_LookupConfig - Looks up the configuration for a specific XV_letterbox device.
 *
 * This function searches the configuration table for an entry that matches the
 * provided DeviceId. If a matching configuration is found, a pointer to the
 * configuration structure is returned. If no match is found, NULL is returned.
 *
 * @param    DeviceId is the unique identifier for the XV_letterbox device.
 *
 * @return   A pointer to the configuration structure corresponding to the given
 *           DeviceId, or NULL if no matching device is found.
 */
XV_letterbox_Config *XV_letterbox_LookupConfig(u16 DeviceId) {
    XV_letterbox_Config *ConfigPtr = NULL;

    int Index;

    for (Index = 0; Index < XPAR_XV_LETTERBOX_NUM_INSTANCES; Index++) {
        if (XV_letterbox_ConfigTable[Index].DeviceId == DeviceId) {
            ConfigPtr = &XV_letterbox_ConfigTable[Index];
            break;
        }
    }

    return ConfigPtr;
}

/**
 * XV_letterbox_Initialize - Initializes an XV_letterbox instance.
 *
 * This function initializes the specified XV_letterbox instance using the
 * configuration data corresponding to the provided DeviceId. It first looks up
 * the configuration for the given DeviceId. If the configuration is found, it
 * initializes the instance with the configuration data and base address.
 * If the configuration is not found, the function sets the instance's IsReady
 * flag to 0 and returns XST_DEVICE_NOT_FOUND.
 *
 * @param    InstancePtr is a pointer to the XV_letterbox instance to be
 *           initialized.
 * @param    DeviceId is the unique identifier for the XV_letterbox device.
 *
 * @return   XST_SUCCESS if initialization is successful, or
 *           XST_DEVICE_NOT_FOUND if the device configuration is not found.
 */
int XV_letterbox_Initialize(XV_letterbox *InstancePtr, u16 DeviceId) {
    XV_letterbox_Config *ConfigPtr;

    Xil_AssertNonvoid(InstancePtr != NULL);

    ConfigPtr = XV_letterbox_LookupConfig(DeviceId);
    if (ConfigPtr == NULL) {
        InstancePtr->IsReady = 0;
        return (XST_DEVICE_NOT_FOUND);
    }

    return XV_letterbox_CfgInitialize(InstancePtr,
                                      ConfigPtr,
                                      ConfigPtr->BaseAddress);
}
#else

/**
 * XV_letterbox_LookupConfig - Looks up the configuration for a XV_letterbox
 * instance based on the provided BaseAddress. This function iterates through
 * the XV_letterbox_ConfigTable to find a configuration entry whose BaseAddress
 * matches the given BaseAddress. If a match is found, a pointer to the
 * configuration structure is returned. If BaseAddress is zero or NULL, the
 * function returns the first configuration entry. If no match is found, NULL
 * is returned.
 *
 * @param   BaseAddress is the base address of the XV_letterbox instance to
 *          look up.
 *
 * @return  A pointer to the configuration structure if a match is found,
 *          otherwise NULL.
 */
XV_letterbox_Config *XV_letterbox_LookupConfig(UINTPTR BaseAddress) {
    XV_letterbox_Config *ConfigPtr = NULL;

    int Index;

    for (Index = (u32)0x0; XV_letterbox_ConfigTable[Index].Name != NULL; Index++) {
		if ((XV_letterbox_ConfigTable[Index].BaseAddress == BaseAddress) ||
		     !BaseAddress) {
			ConfigPtr = &XV_letterbox_ConfigTable[Index];
			break;
		}
	}

    return ConfigPtr;
}


/**
 * XV_letterbox_Initialize - Initializes the XV_letterbox instance.
 *
 * This function looks up the configuration for the XV_letterbox device using the
 * provided base address, and initializes the instance with the configuration
 * data. If the configuration cannot be found, the function sets the IsReady
 * field of the instance to 0 and returns an error code.
 *
 * @param InstancePtr: Pointer to the XV_letterbox instance to be initialized.
 * @param BaseAddress: Base address of the device to be looked up and
 *                     initialized.
 *
 * @return Returns XST_SUCCESS if initialization is successful. Returns
 *         XST_DEVICE_NOT_FOUND if the configuration cannot be found for the
 *         given base address.
 */
int XV_letterbox_Initialize(XV_letterbox *InstancePtr, UINTPTR BaseAddress) {
    XV_letterbox_Config *ConfigPtr;

    Xil_AssertNonvoid(InstancePtr != NULL);

    ConfigPtr = XV_letterbox_LookupConfig(BaseAddress);
    if (ConfigPtr == NULL) {
        InstancePtr->IsReady = 0;
        return (XST_DEVICE_NOT_FOUND);
    }

    return XV_letterbox_CfgInitialize(InstancePtr,
                                      ConfigPtr,
                                      ConfigPtr->BaseAddress);
}
#endif
#endif
