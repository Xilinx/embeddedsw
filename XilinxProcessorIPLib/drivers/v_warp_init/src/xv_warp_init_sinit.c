// ==============================================================
// Copyright (c) 1986 - 2022 Xilinx Inc. All rights reserved.
// Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_warp_init_sinit.c
 * @addtogroup v_warp_init Overview
 */

#ifndef __linux__

/***************************** Include Files *********************************/
#include "xstatus.h"
#ifndef SDT
#include "xparameters.h"
#endif
#include "xv_warp_init.h"


/************************** Function Prototypes ******************************/
extern XV_warp_init_Config XV_warp_init_ConfigTable[];

/************************** Function Definitions *****************************/
#ifndef SDT
/*****************************************************************************/
/**
 * @brief Lookup configuration structure by device ID
 *
 * This function searches the configuration table for a matching device ID
 * and returns a pointer to the corresponding configuration structure.
 *
 * @param  DeviceId The device ID to search for in the configuration table
 *
 * @return Pointer to the matching XV_warp_init_Config structure, or NULL
 *         if no match is found
 *
 * @note This function is only available in non-SDT builds
 *
 ******************************************************************************/
XV_warp_init_Config *XV_warp_init_LookupConfig(u16 DeviceId) {
	XV_warp_init_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XV_WARP_INIT_NUM_INSTANCES; Index++) {
		if (XV_warp_init_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XV_warp_init_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

/*****************************************************************************/
/**
 * @brief Initialize the warp init driver instance by device ID
 *
 * This function initializes the XV_warp_init driver instance by looking up
 * the configuration structure based on the device ID and calling the
 * configuration initialization function.
 *
 * @param  InstancePtr Pointer to the XV_warp_init instance to initialize
 * @param  DeviceId Device ID to look up in the configuration table
 *
 * @return XST_SUCCESS on successful initialization
 *         XST_DEVICE_NOT_FOUND if the device ID is not found
 *
 * @note This function is only available in non-SDT builds. The InstancePtr
 *       must not be NULL.
 *
 ******************************************************************************/
int XV_warp_init_Initialize(XV_warp_init *InstancePtr, u16 DeviceId) {
	XV_warp_init_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XV_warp_init_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XV_warp_init_CfgInitialize(InstancePtr, ConfigPtr);
}
#else
/*****************************************************************************/
/**
 * @brief Lookup configuration structure by base address
 *
 * This function searches the configuration table for a matching base address
 * and returns a pointer to the corresponding configuration structure.
 *
 * @param  BaseAddress The base address to search for in the configuration
 *                     table, or 0 to return the first entry
 *
 * @return Pointer to the matching XV_warp_init_Config structure, or NULL
 *         if no match is found
 *
 * @note This function is only available in SDT builds
 *
 ******************************************************************************/
XV_warp_init_Config *XV_warp_init_LookupConfig(UINTPTR BaseAddress) {
	XV_warp_init_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; XV_warp_init_ConfigTable[Index].Name != NULL; Index++) {
		if ((XV_warp_init_ConfigTable[Index].Ctrl_BaseAddress == BaseAddress) ||
				!BaseAddress) {
			ConfigPtr = &XV_warp_init_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

/*****************************************************************************/
/**
 * @brief Initialize the warp init driver instance by base address
 *
 * This function initializes the XV_warp_init driver instance by looking up
 * the configuration structure based on the base address and calling the
 * configuration initialization function.
 *
 * @param  InstancePtr Pointer to the XV_warp_init instance to initialize
 * @param  BaseAddress Base address to look up in the configuration table,
 *                     or 0 to use the first available configuration
 *
 * @return XST_SUCCESS on successful initialization
 *         XST_DEVICE_NOT_FOUND if the base address is not found
 *
 * @note This function is only available in SDT builds. The InstancePtr
 *       must not be NULL.
 *
 ******************************************************************************/
int XV_warp_init_Initialize(XV_warp_init *InstancePtr, UINTPTR BaseAddress) {
	XV_warp_init_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XV_warp_init_LookupConfig(BaseAddress);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XV_warp_init_CfgInitialize(InstancePtr, ConfigPtr);
}
#endif
#endif
