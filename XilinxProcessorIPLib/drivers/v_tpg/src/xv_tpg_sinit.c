// ==============================================================
// Copyright (c) 2015 - 2021 Xilinx Inc. All rights reserved.
// Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

#ifndef __linux__

#include "xstatus.h"
#ifndef SDT
#include "xparameters.h"
#endif
#include "xv_tpg.h"

#ifndef XPAR_XV_TPG_NUM_INSTANCES
#define XPAR_XV_TPG_NUM_INSTANCES   0
#endif

extern XV_tpg_Config XV_tpg_ConfigTable[];

#ifndef SDT
XV_tpg_Config *XV_tpg_LookupConfig(u16 DeviceId) {
	XV_tpg_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XV_TPG_NUM_INSTANCES; Index++) {
		if (XV_tpg_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XV_tpg_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XV_tpg_Initialize(XV_tpg *InstancePtr, u16 DeviceId) {
	XV_tpg_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XV_tpg_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XV_tpg_CfgInitialize(InstancePtr,
                                ConfigPtr,
                                ConfigPtr->BaseAddress);
}
#else
XV_tpg_Config *XV_tpg_LookupConfig(UINTPTR BaseAddress) {
	XV_tpg_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0U; XV_tpg_ConfigTable[Index].Name != NULL; Index++) {
		if ((XV_tpg_ConfigTable[Index].BaseAddress == BaseAddress) ||
			!BaseAddress) {
			ConfigPtr = &XV_tpg_ConfigTable[Index];
		break;
		}
	}

	return ConfigPtr;
}

int XV_tpg_Initialize(XV_tpg *InstancePtr, UINTPTR BaseAddress) {
	XV_tpg_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XV_tpg_LookupConfig(BaseAddress);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XV_tpg_CfgInitialize(InstancePtr,
                                ConfigPtr,
                                ConfigPtr->BaseAddress);
}
#endif
#endif
