// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

#ifndef __linux__

#include "xstatus.h"
#include "xparameters.h"
#include "xv_axi4s_remap.h"

#ifndef XPAR_XV_AXI4S_REMAP_NUM_INSTANCES
#define XPAR_XV_AXI4S_REMAP_NUM_INSTANCES     0
#endif

extern XV_axi4s_remap_Config XV_axi4s_remap_ConfigTable[];

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

#endif
