// ==============================================================
// Copyright (c) 1986 - 2022 Xilinx Inc. All rights reserved.
// Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_gamma_lut_sinit.c
 * @addtogroup v_gamma_lut Overview
 */

#ifndef __linux__

/***************************** Include Files *********************************/
#include "xstatus.h"
#ifndef SDT
#include "xparameters.h"
#endif
#include "xv_gamma_lut.h"


/***************** Macros (Inline Functions) Definitions *********************/
/** Default number of Gamma LUT instances if not defined */
#ifndef XPAR_XV_GAMMA_LUT_NUM_INSTANCES
#define XPAR_XV_GAMMA_LUT_NUM_INSTANCES   0
#endif

/************************** Variable Definitions *****************************/
extern XV_gamma_lut_Config XV_gamma_lut_ConfigTable[];

#ifndef SDT
/*****************************************************************************/
/**
 * @brief Looks up the device configuration based on device ID
 *
 * This function searches the configuration table for a device matching the
 * provided device ID and returns a pointer to its configuration structure.
 *
 * @param  DeviceId is the unique device ID to search for.
 *
 * @return Pointer to the XV_gamma_lut_Config structure if found, NULL otherwise
 *
 * @note None
 *
 *******************************************************************************/
XV_gamma_lut_Config *XV_gamma_lut_LookupConfig(u16 DeviceId) {
	XV_gamma_lut_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XV_GAMMA_LUT_NUM_INSTANCES; Index++) {
		if (XV_gamma_lut_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XV_gamma_lut_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

/*****************************************************************************/
/**
 * @brief Initializes the Gamma LUT driver instance
 *
 * This function initializes a Gamma LUT driver instance by looking up the
 * configuration based on the device ID and calling the configuration
 * initialization function.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 * @param  DeviceId is the device ID to initialize.
 *
 * @return XST_SUCCESS on success, XST_DEVICE_NOT_FOUND if device not found
 *
 * @note None
 *
 *******************************************************************************/
int XV_gamma_lut_Initialize(XV_gamma_lut *InstancePtr, u16 DeviceId) {
	XV_gamma_lut_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XV_gamma_lut_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XV_gamma_lut_CfgInitialize(InstancePtr,
									  ConfigPtr,
									  ConfigPtr->BaseAddress);
}
#else
/*****************************************************************************/
/**
 * @brief Looks up the device configuration based on base address (SDT)
 *
 * This function searches the configuration table for a device matching the
 * provided base address and returns a pointer to its configuration structure.
 * This version is used with System Device Tree (SDT) configurations.
 *
 * @param  BaseAddress is the base address to search for.
 *
 * @return Pointer to the XV_gamma_lut_Config structure if found, NULL otherwise
 *
 * @note None
 *
 *******************************************************************************/
XV_gamma_lut_Config *XV_gamma_lut_LookupConfig(UINTPTR BaseAddress) {
	XV_gamma_lut_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; XV_gamma_lut_ConfigTable[Index].Name != NULL; Index++) {
		if ((XV_gamma_lut_ConfigTable[Index].BaseAddress == BaseAddress) ||
				!BaseAddress) {
			ConfigPtr = &XV_gamma_lut_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

/*****************************************************************************/
/**
 * @brief Initializes the Gamma LUT driver instance (SDT)
 *
 * This function initializes a Gamma LUT driver instance by looking up the
 * configuration based on the base address and calling the configuration
 * initialization function. This version is used with System Device Tree (SDT)
 * configurations.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 * @param  BaseAddress is the base address to initialize.
 *
 * @return XST_SUCCESS on success, XST_DEVICE_NOT_FOUND if device not found
 *
 * @note None
 *
 *******************************************************************************/
int XV_gamma_lut_Initialize(XV_gamma_lut *InstancePtr, UINTPTR BaseAddress) {
	XV_gamma_lut_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XV_gamma_lut_LookupConfig(BaseAddress);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XV_gamma_lut_CfgInitialize(InstancePtr, ConfigPtr,
					  ConfigPtr->BaseAddress);
}
#endif
#endif
