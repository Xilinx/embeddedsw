// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_axi4s_remap_sinit.c
 * @addtogroup v_axi4s_remap Overview
 */

#ifndef __linux__

/***************************** Include Files *********************************/
#include "xstatus.h"
#ifndef SDT
#include "xparameters.h"
#endif
#include "xv_axi4s_remap.h"

/************************** Constant Definitions *****************************/
#ifndef XPAR_XV_AXI4S_REMAP_NUM_INSTANCES
/** Default number of AXI4S Remap instances if not defined in xparameters.h */
#define XPAR_XV_AXI4S_REMAP_NUM_INSTANCES     0
#endif

/************************** Variable Definitions *****************************/

/** Configuration table defined in xv_axi4s_remap_g.c */
extern XV_axi4s_remap_Config XV_axi4s_remap_ConfigTable[];


/**************************** FUNCTION IMPLEMENTATIONS   *******************************/

#ifndef SDT
/*****************************************************************************/
/**
 * @brief Lookup configuration structure by device ID
 *
 * This function searches the configuration table for an AXI4-Stream Remap
 * device matching the specified device ID and returns a pointer to its
 * configuration structure.
 *
 * @param  DeviceId Unique device ID to search for in the configuration table
 *
 * @return Pointer to matching XV_axi4s_remap_Config structure if found
 *         NULL if no matching device ID is found
 *
 * @note This is the non-SDT version that uses device IDs from xparameters.h
 *
 *******************************************************************************/
XV_axi4s_remap_Config *XV_axi4s_remap_LookupConfig(u16 DeviceId) {
	XV_axi4s_remap_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XV_AXI4S_REMAP_NUM_INSTANCES; Index++) {
		if (XV_axi4s_remap_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XV_axi4s_remap_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

/*****************************************************************************/
/**
 * @brief Initialize AXI4-Stream Remap driver instance by device ID
 *
 * This function initializes an AXI4-Stream Remap driver instance by looking up
 * the device configuration based on the device ID, then calling the configuration
 * initialization function with the retrieved configuration.
 *
 * @param  InstancePtr Pointer to the driver instance structure to initialize
 * @param  DeviceId Unique device ID to identify the hardware instance
 *
 * @return XST_SUCCESS if initialization completed successfully
 *         XST_DEVICE_NOT_FOUND if device ID not found in configuration table
 *         Other error codes from XV_axi4s_remap_CfgInitialize()
 *
 * @note This is the non-SDT version that uses device IDs from xparameters.h
 * @note InstancePtr->IsReady is set to 0 if device lookup fails
 *
 *******************************************************************************/
int XV_axi4s_remap_Initialize(XV_axi4s_remap *InstancePtr, u16 DeviceId) {
	XV_axi4s_remap_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XV_axi4s_remap_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XV_axi4s_remap_CfgInitialize(InstancePtr,
			ConfigPtr,
			ConfigPtr->BaseAddress);
}
#else
/*****************************************************************************/
/**
 * @brief Lookup configuration structure by base address (SDT)
 *
 * This function searches the configuration table for an AXI4-Stream Remap
 * device matching the specified base address and returns a pointer to its
 * configuration structure. If BaseAddress is 0, returns the first entry.
 *
 * @param  BaseAddress Physical base address of the device to search for,
 *                     or 0 to return the first available device
 *
 * @return Pointer to matching XV_axi4s_remap_Config structure if found
 *         NULL if no matching base address is found
 *
 * @note This is the SDT (System Device Tree) version that uses base addresses
 *       instead of device IDs
 *
 *******************************************************************************/
XV_axi4s_remap_Config *XV_axi4s_remap_LookupConfig(UINTPTR BaseAddress) {
	XV_axi4s_remap_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; XV_axi4s_remap_ConfigTable[Index].Name != NULL; Index++) {
		if ((XV_axi4s_remap_ConfigTable[Index].BaseAddress == BaseAddress) ||
				!BaseAddress) {
			ConfigPtr = &XV_axi4s_remap_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

/**
 * @brief Initialize AXI4-Stream Remap driver instance by base address (SDT)
 *
 * This function initializes an AXI4-Stream Remap driver instance by looking up
 * the device configuration based on the physical base address, then calling the
 * configuration initialization function with the retrieved configuration.
 *
 * @param  InstancePtr Pointer to the driver instance structure to initialize
 * @param  BaseAddress Physical base address of the hardware instance
 *
 * @return XST_SUCCESS if initialization completed successfully
 *         XST_DEVICE_NOT_FOUND if base address not found in configuration table
 *         Other error codes from XV_axi4s_remap_CfgInitialize()
 *
 * @note This is the SDT (System Device Tree) version that uses base addresses
 *       instead of device IDs
 * @note InstancePtr->IsReady is set to 0 if device lookup fails
 *
 *******************************************************************************/
int XV_axi4s_remap_Initialize(XV_axi4s_remap *InstancePtr, UINTPTR BaseAddress) {
	XV_axi4s_remap_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XV_axi4s_remap_LookupConfig(BaseAddress);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XV_axi4s_remap_CfgInitialize(InstancePtr,
			ConfigPtr,
			ConfigPtr->BaseAddress);
}
#endif
#endif
