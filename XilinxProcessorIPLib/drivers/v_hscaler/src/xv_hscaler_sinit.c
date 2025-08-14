// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_hscaler_sinit.c
 * @addtogroup v_hscaler_sinit Overview
 */

#ifndef __linux__

#include "xstatus.h"
#ifndef SDT
#include "xparameters.h"
#endif
#include "xv_hscaler.h"

#ifndef XPAR_XV_HSCALER_NUM_INSTANCES
#define XPAR_XV_HSCALER_NUM_INSTANCES   0
#endif

extern XV_hscaler_Config XV_hscaler_ConfigTable[];

#ifndef SDT

/*****************************************************************************/
/**
*
* This function looks up the configuration for a device instance by its DeviceId.
*
* @param    DeviceId is the unique device ID of the XV_hscaler instance.
*
* @return   A pointer to the configuration structure if found, or NULL otherwise.
*
* @note     None.
*
******************************************************************************/
XV_hscaler_Config *XV_hscaler_LookupConfig(u16 DeviceId) {
    XV_hscaler_Config *ConfigPtr = NULL;

    int Index;

    for (Index = 0; Index < XPAR_XV_HSCALER_NUM_INSTANCES; Index++) {
        if (XV_hscaler_ConfigTable[Index].DeviceId == DeviceId) {
            ConfigPtr = &XV_hscaler_ConfigTable[Index];
            break;
        }
    }

    return ConfigPtr;
}

/**
 * XV_hscaler_Initialize - Initializes an XV_hscaler instance.
 *
 * This function looks up the hardware configuration for the device specified by
 * DeviceId, and initializes the XV_hscaler instance pointed to by InstancePtr.
 * If the configuration is not found, the function sets the instance's IsReady
 * flag to 0 and returns XST_DEVICE_NOT_FOUND. Otherwise, it calls
 * XV_hscaler_CfgInitialize to complete the initialization.
 *
 * @param InstancePtr Pointer to the XV_hscaler instance to be initialized.
 * @param DeviceId    Unique device ID of the XV_hscaler core.
 *
 * @return
 *   - XST_SUCCESS if initialization was successful.
 *   - XST_DEVICE_NOT_FOUND if the device configuration could not be found.
 */

int XV_hscaler_Initialize(XV_hscaler *InstancePtr, u16 DeviceId) {
    XV_hscaler_Config *ConfigPtr;

    Xil_AssertNonvoid(InstancePtr != NULL);

    ConfigPtr = XV_hscaler_LookupConfig(DeviceId);
    if (ConfigPtr == NULL) {
        InstancePtr->IsReady = 0;
        return (XST_DEVICE_NOT_FOUND);
    }

    return XV_hscaler_CfgInitialize(InstancePtr,
                                    ConfigPtr,
                                    ConfigPtr->BaseAddress);
}
#else
/**
 * XV_hscaler_LookupConfig - Looks up the configuration for a V_HScaler device.
 *
 * This function searches the XV_hscaler_ConfigTable for a configuration entry
 * that matches the specified BaseAddress. If a matching entry is found, a pointer
 * to its configuration structure is returned. If BaseAddress is zero or NULL, the
 * function returns the first entry in the table.
 *
 * @param	BaseAddress	The base address of the device instance to look up.
 *
 * @return	A pointer to the configuration structure if found, or NULL if no
 *          matching entry exists.
 */

XV_hscaler_Config *XV_hscaler_LookupConfig(UINTPTR BaseAddress) {
    XV_hscaler_Config *ConfigPtr = NULL;

    int Index;

    for (Index = (u32)0x0; XV_hscaler_ConfigTable[Index].Name != NULL; Index++) {
		if ((XV_hscaler_ConfigTable[Index].BaseAddress == BaseAddress) ||
		     !BaseAddress) {
			ConfigPtr = &XV_hscaler_ConfigTable[Index];
			break;
		}
	}

    return ConfigPtr;
}
/**
 * XV_hscaler_Initialize - Initializes an instance of the XV_hscaler driver.
 *
 * This function looks up the configuration for a given device using the
 * specified base address, and then initializes the XV_hscaler instance
 * with the found configuration. If the configuration cannot be found,
 * the function sets the instance as not ready and returns an error code.
 *
 * @param InstancePtr   Pointer to the XV_hscaler instance to be initialized.
 * @param BaseAddress   Base address of the device to initialize.
 *
 * @return
 *   - XST_SUCCESS if initialization was successful.
 *   - XST_DEVICE_NOT_FOUND if the device configuration could not be found.
 *
 * @note
 *   - The caller must ensure that the InstancePtr is not NULL.
 *   - This function must be called before using the XV_hscaler instance.
 */

int XV_hscaler_Initialize(XV_hscaler *InstancePtr, UINTPTR BaseAddress) {
    XV_hscaler_Config *ConfigPtr;

    Xil_AssertNonvoid(InstancePtr != NULL);

    ConfigPtr = XV_hscaler_LookupConfig(BaseAddress);
    if (ConfigPtr == NULL) {
        InstancePtr->IsReady = 0;
        return (XST_DEVICE_NOT_FOUND);
    }

    return XV_hscaler_CfgInitialize(InstancePtr,
                                    ConfigPtr,
                                    ConfigPtr->BaseAddress);
}
#endif
#endif
