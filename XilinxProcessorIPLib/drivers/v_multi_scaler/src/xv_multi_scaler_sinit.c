/*************************************************************************
 * Copyright (c) 1986 - 2022 Xilinx, Inc. All Rights Reserved.
 * Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
******************************************************************************/

/**
 * @file xv_multi_scaler_sinit.c
 * @addtogroup v_multi_scaler Overview
 */

#ifndef __linux__

/***************************** Include Files *********************************/
#include "xstatus.h"
#ifndef SDT
#include "xparameters.h"
#endif
#include "xv_multi_scaler.h"

#ifndef SDT
/*****************************************************************************/
/**
 * @brief Lookup the device configuration by device ID.
 *
 * This function searches the configuration table for a device matching the
 * specified device ID. The configuration table is populated at compile time
 * based on the hardware configuration.
 *
 * @param  DeviceId is the unique device identifier from xparameters.h.
 *
 * @return Pointer to the configuration structure if found, NULL otherwise.
 *
 * @note   This function is used in non-SDT (Software Development Toolkit) builds.
 *
 ******************************************************************************/
XV_multi_scaler_Config *XV_multi_scaler_LookupConfig(u16 DeviceId)
{
	XV_multi_scaler_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XV_MULTI_SCALER_NUM_INSTANCES; Index++) {
		if (XV_multi_scaler_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XV_multi_scaler_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

/*****************************************************************************/
/**
 * @brief Initialize the Multi Scaler driver instance.
 *
 * This function locates the device configuration based on the device ID and
 * calls the configuration initialization function. The instance must be
 * initialized before using any other driver functions.
 *
 * @param  InstancePtr is a pointer to the Multi Scaler instance to be initialized.
 * @param  DeviceId is the unique device identifier from xparameters.h.
 *
 * @return XST_SUCCESS if initialization was successful, XST_DEVICE_NOT_FOUND
 *         if the configuration for the device ID was not found.
 *
 * @note   This function is used in non-SDT (Software Development Toolkit) builds.
 *
 ******************************************************************************/
int XV_multi_scaler_Initialize(XV_multi_scaler *InstancePtr, u16 DeviceId)
{
	XV_multi_scaler_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XV_multi_scaler_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return XST_DEVICE_NOT_FOUND;
	}

	return XV_multi_scaler_CfgInitialize(InstancePtr, ConfigPtr);
}
#else
/*****************************************************************************/
/**
 * @brief Lookup the device configuration by base address.
 *
 * This function searches the configuration table for a device matching the
 * specified base address. The configuration table is populated at compile time
 * based on the device tree configuration. If BaseAddress is 0, the first
 * entry in the table is returned.
 *
 * @param  BaseAddress is the physical base address of the device. If 0, returns
 *         the first configuration entry.
 *
 * @return Pointer to the configuration structure if found, NULL otherwise.
 *
 * @note   This function is used in SDT (Software Development Toolkit) builds.
 *
 ******************************************************************************/
XV_multi_scaler_Config *XV_multi_scaler_LookupConfig(UINTPTR BaseAddress)
{
	XV_multi_scaler_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; XV_multi_scaler_ConfigTable[Index].Name != NULL; Index++) {
		if ((XV_multi_scaler_ConfigTable[Index].Ctrl_BaseAddress == BaseAddress) ||
				!BaseAddress) {
			ConfigPtr = &XV_multi_scaler_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

/*****************************************************************************/
/**
 * @brief Initialize the Multi Scaler driver instance using base address.
 *
 * This function locates the device configuration based on the base address and
 * calls the configuration initialization function. The instance must be
 * initialized before using any other driver functions. The base address is
 * typically obtained from the device tree.
 *
 * @param  InstancePtr is a pointer to the Multi Scaler instance to be initialized.
 * @param  BaseAddress is the physical base address of the device.
 *
 * @return XST_SUCCESS if initialization was successful, XST_DEVICE_NOT_FOUND
 *         if the configuration for the base address was not found.
 *
 * @note   This function is used in SDT (Software Development Toolkit) builds.
 *
 ******************************************************************************/
int XV_multi_scaler_Initialize(XV_multi_scaler *InstancePtr, UINTPTR BaseAddress)
{
	XV_multi_scaler_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XV_multi_scaler_LookupConfig(BaseAddress);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return XST_DEVICE_NOT_FOUND;
	}

	return XV_multi_scaler_CfgInitialize(InstancePtr, ConfigPtr);
}
#endif
#endif
