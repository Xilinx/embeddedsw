// ==============================================================
// Copyright (c) 1986 - 2021 Xilinx Inc. All rights reserved.
// Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
*
* @file xv_frmbufrd_sinit.c
* @addtogroup v_frmbuf_rd Overview
*/

#ifndef __linux__

#include "xstatus.h"
#include "xv_frmbufrd.h"

extern XV_frmbufrd_Config XV_frmbufrd_ConfigTable[];

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
