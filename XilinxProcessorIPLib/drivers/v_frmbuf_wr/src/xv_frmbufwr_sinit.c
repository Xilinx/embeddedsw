// ==============================================================
// Copyright (c) 1986 - 2021 Xilinx Inc. All rights reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================
/**
 * *
* @file xv_frmbufwr_sinit.c
* @addtogroup v_frmbuf_wr Overview
*
**/
#ifndef __linux__

#include "xstatus.h"
#ifndef SDT
#include "xparameters.h"
#endif
#include "xv_frmbufwr.h"

extern XV_frmbufwr_Config XV_frmbufwr_ConfigTable[];

#ifndef SDT
/**
 * XV_frmbufwr_LookupConfig - Looks up the configuration for a device instance.
 *
 * @param DeviceId The unique device ID of the XV_frmbufwr instance to look up.
 *
 * @return Pointer to the configuration structure if found, NULL otherwise.
 *
 * This function searches the configuration table for an entry matching the
 * specified DeviceId and returns a pointer to its configuration structure.
 */
XV_frmbufwr_Config *XV_frmbufwr_LookupConfig(u16 DeviceId) {
    XV_frmbufwr_Config *ConfigPtr = NULL;

    int Index;

    for (Index = 0; Index < XPAR_XV_FRMBUFWR_NUM_INSTANCES; Index++) {
        if (XV_frmbufwr_ConfigTable[Index].DeviceId == DeviceId) {
            ConfigPtr = &XV_frmbufwr_ConfigTable[Index];
            break;
        }
    }

    return ConfigPtr;
}

/**
 * Initialize the frame buffer write core instance.
 *
 * This function initializes an XV_frmbufwr instance using the configuration
 * corresponding to the specified DeviceId. It looks up the configuration,
 * checks for validity, and then calls the core's configuration initialization
 * function. If the configuration is not found, the instance is marked as not
 * ready and an error code is returned.
 *
 * @param InstancePtr Pointer to the XV_frmbufwr instance to be initialized.
 * @param DeviceId    Unique device ID of the frame buffer write core.
 *
 * @return
 *   - XST_SUCCESS if initialization was successful.
 *   - XST_DEVICE_NOT_FOUND if the configuration for the given DeviceId was not found.
 */
int XV_frmbufwr_Initialize(XV_frmbufwr *InstancePtr, u16 DeviceId) {
    XV_frmbufwr_Config *ConfigPtr;

    Xil_AssertNonvoid(InstancePtr != NULL);

    ConfigPtr = XV_frmbufwr_LookupConfig(DeviceId);
    if (ConfigPtr == NULL) {
        InstancePtr->IsReady = 0;
        return (XST_DEVICE_NOT_FOUND);
    }

    return XV_frmbufwr_CfgInitialize(InstancePtr,
                                      ConfigPtr,
                                      ConfigPtr->BaseAddress);
}
#else
/**
 * XV_frmbufwr_LookupConfig - Looks up the configuration for a frame buffer write instance.
 *
 * This function searches the XV_frmbufwr_ConfigTable for a configuration entry
 * that matches the specified BaseAddress. If a matching BaseAddress is found,
 * a pointer to the corresponding configuration structure is returned. If the
 * BaseAddress is zero (0), the function returns the first entry in the table.
 *
 * @param	BaseAddress	The base address of the device to look up.
 *
 * @return	A pointer to the configuration structure if found, or NULL if no
 *          matching entry exists.
 */
XV_frmbufwr_Config *XV_frmbufwr_LookupConfig(UINTPTR BaseAddress) {
    XV_frmbufwr_Config *ConfigPtr = NULL;

    int Index;

    for (Index = 0; XV_frmbufwr_ConfigTable[Index].Name != NULL; Index++) {
        if ((XV_frmbufwr_ConfigTable[Index].BaseAddress == BaseAddress) ||
				!BaseAddress) {
            ConfigPtr = &XV_frmbufwr_ConfigTable[Index];
            break;
        }
    }

    return ConfigPtr;
}

/**
 * Initialize the frame buffer write core instance.
 *
 * This function initializes an XV_frmbufwr instance using the configuration
 * found for the specified base address. It performs a lookup for the hardware
 * configuration, and if found, initializes the instance with the configuration
 * data. If the configuration is not found, the function marks the instance as
 * not ready and returns an error code.
 *
 * @param InstancePtr   Pointer to the XV_frmbufwr instance to be initialized.
 * @param BaseAddress   Base address of the device to look up the configuration.
 *
 * @return
 *      - XST_SUCCESS if initialization is successful.
 *      - XST_DEVICE_NOT_FOUND if the configuration for the given base address is not found.
 */
int XV_frmbufwr_Initialize(XV_frmbufwr *InstancePtr, UINTPTR BaseAddress) {
    XV_frmbufwr_Config *ConfigPtr;

    Xil_AssertNonvoid(InstancePtr != NULL);

    ConfigPtr = XV_frmbufwr_LookupConfig(BaseAddress);
    if (ConfigPtr == NULL) {
        InstancePtr->IsReady = 0;
        return (XST_DEVICE_NOT_FOUND);
    }

    return XV_frmbufwr_CfgInitialize(InstancePtr,
                                      ConfigPtr,
                                      ConfigPtr->BaseAddress);
}
#endif
#endif
