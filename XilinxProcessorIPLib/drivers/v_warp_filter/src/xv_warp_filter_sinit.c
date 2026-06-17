// ==============================================================
// Copyright (c) 1986 - 2022 Xilinx Inc. All rights reserved.
// Copyright (C) 2022-2026, Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_warp_filter_sinit.c
 * @addtogroup v_warp_filter Overview
 */

#ifndef __linux__

/***************************** Include Files *********************************/
#include "xstatus.h"
#include "xv_warp_filter.h"

/************************** Variable Definitions *****************************/
/** External reference to configuration table */
extern XV_warp_filter_Config XV_warp_filter_ConfigTable[];

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
 * @brief Looks up the device configuration based on the base address.
 *
 * This function searches the configuration table for a device matching the
 * specified base address and returns a pointer to its configuration structure.
 * If BaseAddress is 0, returns the first configuration entry.
 *
 * @param BaseAddress Base address of the device to search for, or 0 for first entry.
 *
 * @return Pointer to the configuration structure if found, NULL otherwise.
 *
 * @note This function is only available when SDT is defined.
 *
 *******************************************************************************/
XV_warp_filter_Config *XV_warp_filter_LookupConfig(UINTPTR BaseAddress) {
	XV_warp_filter_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; XV_warp_filter_ConfigTable[Index].Name != NULL; Index++) {
		if ((XV_warp_filter_ConfigTable[Index].Control_BaseAddress == BaseAddress) ||
				!BaseAddress) {
			ConfigPtr = &XV_warp_filter_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

/**
 * @brief Initializes the Warp Filter instance using base address.
 *
 * This function looks up the configuration for the specified base address
 * and initializes the instance using the configuration found.
 *
 * @param InstancePtr Pointer to the XV_warp_filter instance to initialize.
 * @param BaseAddress Base address of the Warp Filter control interface.
 *
 * @return XST_SUCCESS if initialization is successful.
 * @return XST_DEVICE_NOT_FOUND if the base address is not found.
 *
 * @note This function is only available when SDT is defined.
 * @note InstancePtr must not be NULL.
 *
 *******************************************************************************/
s32 XV_warp_filter_Initialize(XV_warp_filter *InstancePtr, UINTPTR BaseAddress) {
	XV_warp_filter_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XV_warp_filter_LookupConfig(BaseAddress);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XV_warp_filter_CfgInitialize(InstancePtr, ConfigPtr);
}
#endif
