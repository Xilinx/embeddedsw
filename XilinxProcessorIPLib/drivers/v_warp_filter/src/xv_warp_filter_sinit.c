// ==============================================================
// Copyright (c) 1986 - 2022 Xilinx Inc. All rights reserved.
// Copyright (C) 2022-2023, Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

#ifndef __linux__

#include "xstatus.h"
#ifndef SDT
#include "xparameters.h"
#endif
#include "xv_warp_filter.h"

extern XV_warp_filter_Config XV_warp_filter_ConfigTable[];

#ifndef SDT
XV_warp_filter_Config *XV_warp_filter_LookupConfig(u16 DeviceId) {
	XV_warp_filter_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XV_WARP_FILTER_NUM_INSTANCES; Index++) {
		if (XV_warp_filter_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XV_warp_filter_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

s32 XV_warp_filter_Initialize(XV_warp_filter *InstancePtr, u16 DeviceId) {
	XV_warp_filter_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XV_warp_filter_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XV_warp_filter_CfgInitialize(InstancePtr, ConfigPtr);
}
#else
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
#endif
