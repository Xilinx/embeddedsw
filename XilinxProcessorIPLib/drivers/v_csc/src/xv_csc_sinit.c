// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_csc_sinit.c
 * @addtogroup v_csc Overview
*/
#ifndef __linux__

#include "xstatus.h"
#include "xv_csc.h"

#ifndef XPAR_XV_CSC_NUM_INSTANCES
#define XPAR_XV_CSC_NUM_INSTANCES   0
#endif

extern XV_csc_Config XV_csc_ConfigTable[];

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
