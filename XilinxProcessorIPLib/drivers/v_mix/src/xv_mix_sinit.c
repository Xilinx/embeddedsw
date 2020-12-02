// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

#ifndef __linux__

#include "xstatus.h"
#include "xparameters.h"
#include "xv_mix.h"

extern XV_mix_Config XV_mix_ConfigTable[];

XV_mix_Config *XV_mix_LookupConfig(u16 DeviceId) {
	XV_mix_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XV_MIX_NUM_INSTANCES; Index++) {
		if (XV_mix_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XV_mix_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XV_mix_Initialize(XV_mix *InstancePtr, u16 DeviceId) {
	XV_mix_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XV_mix_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XV_mix_CfgInitialize(InstancePtr,
			                    ConfigPtr,
			                    ConfigPtr->BaseAddress);
}

#endif
