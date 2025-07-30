// ==============================================================
// Copyright (c) 1986 - 2021 Xilinx Inc. All rights reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
*
* @file xv_frmbufrd_sinit.c
* @addtogroup v_frmbuf_rd Overview
*/

#ifndef __linux__

#include "xstatus.h"
#ifndef SDT
#include "xparameters.h"
#endif
#include "xv_frmbufrd.h"

extern XV_frmbufrd_Config XV_frmbufrd_ConfigTable[];

#ifndef SDT
/**
 * Looks up the configuration for a frame buffer reader instance based on the given device ID.
 *
 * This function searches the XV_frmbufrd_ConfigTable for an entry whose DeviceId matches
 * the specified DeviceId. If a matching entry is found, a pointer to its configuration
 * structure is returned. If no matching entry is found, NULL is returned.
 *
 * @param DeviceId Device ID of the frame buffer reader core to look up.
 *
 * @return A pointer to the configuration structure if found, or NULL otherwise.
 */
XV_frmbufrd_Config *XV_frmbufrd_LookupConfig(u16 DeviceId) {
    XV_frmbufrd_Config *ConfigPtr = NULL;

    int Index;

    for (Index = 0; Index < XPAR_XV_FRMBUFRD_NUM_INSTANCES; Index++) {
        if (XV_frmbufrd_ConfigTable[Index].DeviceId == DeviceId) {
            ConfigPtr = &XV_frmbufrd_ConfigTable[Index];
            break;
        }
    }

    return ConfigPtr;
}

/**
 * Initializes the frame buffer read core instance.
 *
 * @param InstancePtr Pointer to the XV_frmbufrd instance to be initialized.
 * @param DeviceId    Device ID of the frame buffer read core to look up configuration.
 *
 * This function looks up the configuration for the specified device ID and
 * initializes the XV_frmbufrd instance with the configuration data. If the
 * configuration is not found, the function marks the instance as not ready
 * and returns an error code.
 *
 * @return XST_SUCCESS if initialization is successful.
 *         XST_DEVICE_NOT_FOUND if the configuration for the device ID is not found.
 */
int XV_frmbufrd_Initialize(XV_frmbufrd *InstancePtr, u16 DeviceId) {
    XV_frmbufrd_Config *ConfigPtr;

    Xil_AssertNonvoid(InstancePtr != NULL);

    ConfigPtr = XV_frmbufrd_LookupConfig(DeviceId);
    if (ConfigPtr == NULL) {
        InstancePtr->IsReady = 0;
        return (XST_DEVICE_NOT_FOUND);
    }

    return XV_frmbufrd_CfgInitialize(InstancePtr,
                                      ConfigPtr,
                                      ConfigPtr->BaseAddress);
}
#else
/**
 * Looks up the configuration for a frame buffer reader instance based on the given base address.
 *
 * This function searches the XV_frmbufrd_ConfigTable for an entry whose BaseAddress matches
 * the specified BaseAddress. If BaseAddress is zero (0), the function returns the first entry
 * in the table. If a matching entry is found, a pointer to its configuration structure is returned.
 * If no matching entry is found, NULL is returned.
 *
 * @param	BaseAddress	The base address of the device to look up.
 *
 * @return	A pointer to the configuration structure if found, or NULL otherwise.
 */
XV_frmbufrd_Config *XV_frmbufrd_LookupConfig(UINTPTR BaseAddress) {
    XV_frmbufrd_Config *ConfigPtr = NULL;

    int Index;

    for (Index = 0; XV_frmbufrd_ConfigTable[Index].Name != NULL; Index++) {
        if ((XV_frmbufrd_ConfigTable[Index].BaseAddress == BaseAddress) ||
				!BaseAddress) {
            ConfigPtr = &XV_frmbufrd_ConfigTable[Index];
            break;
        }
    }

	return ConfigPtr;
}

/**
 * Initializes the frame buffer read core instance.
 *
 * This function looks up the configuration for the frame buffer read core
 * using the provided base address, and initializes the core instance with
 * the found configuration. If the configuration is not found, the function
 * sets the instance as not ready and returns an error code.
 *
 * @param InstancePtr   Pointer to the XV_frmbufrd instance to be initialized.
 * @param BaseAddress   Base address of the device to initialize.
 *
 * @return
 *   - XST_SUCCESS if initialization was successful.
 *   - XST_DEVICE_NOT_FOUND if the configuration could not be found.
 */
int XV_frmbufrd_Initialize(XV_frmbufrd *InstancePtr, UINTPTR BaseAddress) {
    XV_frmbufrd_Config *ConfigPtr;

    Xil_AssertNonvoid(InstancePtr != NULL);

    ConfigPtr = XV_frmbufrd_LookupConfig(BaseAddress);
    if (ConfigPtr == NULL) {
        InstancePtr->IsReady = 0;
        return (XST_DEVICE_NOT_FOUND);
    }

    return XV_frmbufrd_CfgInitialize(InstancePtr,
                                      ConfigPtr,
                                      ConfigPtr->BaseAddress);
}
#endif
#endif
