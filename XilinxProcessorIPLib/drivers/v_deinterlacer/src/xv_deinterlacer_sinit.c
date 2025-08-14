/*******************************************************************
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
/**
* @file xv_deinterlacer_sinit.c
* @addtogroup v_deinterlacer Overview
*/

#ifndef __linux__

#include "xstatus.h"
#ifndef SDT
#include "xparameters.h"
#endif
#include "xv_deinterlacer.h"

#ifndef XPAR_XV_DEINTERLACER_NUM_INSTANCES
#define XPAR_XV_DEINTERLACER_NUM_INSTANCES  0
#endif

extern XV_deinterlacer_Config XV_deinterlacer_ConfigTable[];

#ifndef SDT
/**
 * XV_deinterlacer_LookupConfig - Looks up the configuration for a specific device ID.
 *
 * This function searches the XV_deinterlacer_ConfigTable for a configuration
 * structure that matches the provided DeviceId. If a match is found, a pointer
 * to the corresponding configuration structure is returned. If no match is found,
 * NULL is returned.
 *
 * @param DeviceId: The unique identifier for the device whose configuration is to be found.
 *
 * @return
 *   - Pointer to the XV_deinterlacer_Config structure if a match is found.
 *   - NULL if no matching configuration is found.
 */
XV_deinterlacer_Config *XV_deinterlacer_LookupConfig(u16 DeviceId) {
    XV_deinterlacer_Config *ConfigPtr = NULL;

    int Index;

    for (Index = 0; Index < XPAR_XV_DEINTERLACER_NUM_INSTANCES; Index++) {
        if (XV_deinterlacer_ConfigTable[Index].DeviceId == DeviceId) {
            ConfigPtr = &XV_deinterlacer_ConfigTable[Index];
            break;
        }
    }

    return ConfigPtr;
}

/**
 * XV_deinterlacer_Initialize - Initializes the XV_deinterlacer instance.
 *
 * This function looks up the configuration for the specified device ID and
 * initializes the XV_deinterlacer instance with the configuration data.
 * If the configuration cannot be found, the function sets the instance as
 * not ready and returns an error code.
 *
 * @param InstancePtr Pointer to the XV_deinterlacer instance to be initialized.
 * @param DeviceId    Device ID of the XV_deinterlacer core to look up.
 *
 * @return
 *   - XST_SUCCESS if initialization is successful.
 *   - XST_DEVICE_NOT_FOUND if the device configuration is not found.
 */
int XV_deinterlacer_Initialize(XV_deinterlacer *InstancePtr, u16 DeviceId) {
    XV_deinterlacer_Config *ConfigPtr;

    Xil_AssertNonvoid(InstancePtr != NULL);

    ConfigPtr = XV_deinterlacer_LookupConfig(DeviceId);
    if (ConfigPtr == NULL) {
        InstancePtr->IsReady = 0;
        return (XST_DEVICE_NOT_FOUND);
    }

    return XV_deinterlacer_CfgInitialize(InstancePtr,
                                         ConfigPtr,
                                         ConfigPtr->BaseAddress);
}
#else
/**
 * XV_deinterlacer_LookupConfig - Looks up the configuration for a deinterlacer instance.
 *
 * This function searches the XV_deinterlacer_ConfigTable for a configuration
 * entry that matches the specified BaseAddress. If BaseAddress is zero (0),
 * it returns the first configuration entry found. If a matching configuration
 * is found, a pointer to the configuration structure is returned; otherwise,
 * NULL is returned.
 *
 * @param	BaseAddress	The base address of the deinterlacer instance to look up.
 *
 * @return	A pointer to the matching XV_deinterlacer_Config structure if found,
 *          or NULL if no matching configuration is found.
 */
XV_deinterlacer_Config *XV_deinterlacer_LookupConfig(UINTPTR BaseAddress) {
    XV_deinterlacer_Config *ConfigPtr = NULL;

    int Index;

    for (Index = (u32)0x0; XV_deinterlacer_ConfigTable[Index].Name != NULL; Index++) {
		if ((XV_deinterlacer_ConfigTable[Index].BaseAddress == BaseAddress) ||
		     !BaseAddress) {
			ConfigPtr = &XV_deinterlacer_ConfigTable[Index];
			break;
		}
	}

    return ConfigPtr;
}

/**
 * XV_deinterlacer_Initialize - Initialize the XV_deinterlacer instance.
 *
 * This function initializes an XV_deinterlacer instance using the configuration
 * found for the specified base address. It first asserts that the instance pointer
 * is not NULL, then looks up the configuration for the given base address. If the
 * configuration is not found, it sets the instance's IsReady flag to 0 and returns
 * XST_DEVICE_NOT_FOUND. Otherwise, it calls XV_deinterlacer_CfgInitialize to
 * complete the initialization.
 *
 * @param InstancePtr   Pointer to the XV_deinterlacer instance to be initialized.
 * @param BaseAddress   Base address of the device to initialize.
 *
 * @return
 *   - XST_SUCCESS if initialization was successful.
 *   - XST_DEVICE_NOT_FOUND if the configuration could not be found.
 */
int XV_deinterlacer_Initialize(XV_deinterlacer *InstancePtr, UINTPTR BaseAddress) {
    XV_deinterlacer_Config *ConfigPtr;

    Xil_AssertNonvoid(InstancePtr != NULL);

    ConfigPtr = XV_deinterlacer_LookupConfig(BaseAddress);
    if (ConfigPtr == NULL) {
        InstancePtr->IsReady = 0;
        return (XST_DEVICE_NOT_FOUND);
    }

    return XV_deinterlacer_CfgInitialize(InstancePtr,
                                         ConfigPtr,
                                         ConfigPtr->BaseAddress);
}
#endif
#endif
