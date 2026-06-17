// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_vscaler_sinit.c
 * @addtogroup xv_vscaler Overview
 */

#ifndef __linux__

#include "xstatus.h"
#include "xv_vscaler.h"

#ifndef XPAR_XV_VSCALER_NUM_INSTANCES
#define XPAR_XV_VSCALER_NUM_INSTANCES   0
#endif

extern XV_vscaler_Config XV_vscaler_ConfigTable[];

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
