// ==============================================================
// Copyright (c) 1986 - 2021 Xilinx Inc. All rights reserved.
// Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_demosaic_sinit.c
 * @addtogroup v_demosaic Overview
 */

#ifndef __linux__

/***************************** Include Files *********************************/
#include "xstatus.h"
#ifndef SDT
#include "xparameters.h"
#endif
#include "xv_demosaic.h"

/************************** Constant Definitions *****************************/
/** Default number of Demosaic instances when not defined by system */
#ifndef XPAR_XV_DEMOSAIC_NUM_INSTANCES
#define XPAR_XV_DEMOSAIC_NUM_INSTANCES   0
#endif

/************************** Variable Definitions *****************************/
extern XV_demosaic_Config XV_demosaic_ConfigTable[];

/************************** Function Definitions *****************************/

#ifndef SDT
/*****************************************************************************/
/**
 * @brief Looks up the device configuration based on the unique device ID.
 *
 * This function searches the configuration table for a Demosaic device
 * matching the specified device ID. The configuration table is generated
 * by the build tools and contains all Demosaic instances in the system.
 *
 * @param DeviceId The unique device ID of the Demosaic instance to look up.
 *
 * @return Pointer to the configuration structure if found, NULL otherwise.
 *
 * @note This function is used in non-SDT builds. For SDT builds, use the
 *       base address variant of this function.
 *
 *******************************************************************************/
XV_demosaic_Config *XV_demosaic_LookupConfig(u16 DeviceId) {
	XV_demosaic_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XV_DEMOSAIC_NUM_INSTANCES; Index++) {
		if (XV_demosaic_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XV_demosaic_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

/*****************************************************************************/
/**
 * @brief Initializes a Demosaic instance using the device ID.
 *
 * This function looks up the configuration for the specified device ID and
 * calls the CfgInitialize function to initialize the Demosaic instance.
 * The instance is initialized with the configuration data, including the
 * base address and hardware parameters.
 *
 * @param InstancePtr Pointer to the XV_demosaic instance to initialize.
 * @param DeviceId The unique device ID of the Demosaic instance.
 *
 * @return XST_SUCCESS if initialization is successful, XST_DEVICE_NOT_FOUND
 *         if the device configuration cannot be found.
 *
 * @note This function is used in non-SDT builds. The instance must be
 *       properly allocated before calling this function.
 *
 *******************************************************************************/
int XV_demosaic_Initialize(XV_demosaic *InstancePtr, u16 DeviceId) {
	XV_demosaic_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XV_demosaic_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XV_demosaic_CfgInitialize(InstancePtr,
									  ConfigPtr,
									  ConfigPtr->BaseAddress);
}
#else
/*****************************************************************************/
/**
 * @brief Looks up the device configuration based on the base address (SDT).
 *
 * This function searches the configuration table for a Demosaic device
 * matching the specified base address. If BaseAddress is 0, it returns
 * the first available configuration. This function is used in SDT-based
 * builds where device lookup is done by base address instead of device ID.
 *
 * @param BaseAddress The base address of the Demosaic instance, or 0 to
 *                    return the first available configuration.
 *
 * @return Pointer to the configuration structure if found, NULL otherwise.
 *
 * @note This function is used in SDT builds. For non-SDT builds, use the
 *       device ID variant of this function.
 *
 *******************************************************************************/
XV_demosaic_Config *XV_demosaic_LookupConfig(UINTPTR BaseAddress) {
	XV_demosaic_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; XV_demosaic_ConfigTable[Index].Name != NULL; Index++) {
		if ((XV_demosaic_ConfigTable[Index].BaseAddress == BaseAddress) ||
				!BaseAddress) {
			ConfigPtr = &XV_demosaic_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

/*****************************************************************************/
/**
 * @brief Initializes a Demosaic instance using the base address (SDT).
 *
 * This function looks up the configuration for the specified base address
 * and calls the CfgInitialize function to initialize the Demosaic instance.
 * The instance is initialized with the configuration data, including hardware
 * parameters. This version is used in SDT-based builds.
 *
 * @param InstancePtr Pointer to the XV_demosaic instance to initialize.
 * @param BaseAddress The base address of the Demosaic instance.
 *
 * @return XST_SUCCESS if initialization is successful, XST_DEVICE_NOT_FOUND
 *         if the device configuration cannot be found.
 *
 * @note This function is used in SDT builds. The instance must be properly
 *       allocated before calling this function.
 *
 *******************************************************************************/
int XV_demosaic_Initialize(XV_demosaic *InstancePtr, UINTPTR BaseAddress) {
	XV_demosaic_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XV_demosaic_LookupConfig(BaseAddress);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XV_demosaic_CfgInitialize(InstancePtr, ConfigPtr,
					 ConfigPtr->BaseAddress);
}
#endif
#endif
