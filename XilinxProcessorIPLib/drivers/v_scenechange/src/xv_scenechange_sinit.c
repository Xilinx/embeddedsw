// ==============================================================
// Copyright (c) 1986 - 2022 Xilinx, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

#ifndef __linux__

#include "xstatus.h"
#include "xparameters.h"
#include "xv_scenechange.h"

extern XV_scenechange_Config XV_scenechange_ConfigTable[];

XV_scenechange_Config *XV_scenechange_LookupConfig(u16 DeviceId) {
	XV_scenechange_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XV_SCENECHANGE_NUM_INSTANCES; Index++) {
		if (XV_scenechange_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XV_scenechange_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XV_scenechange_Initialize(XV_scenechange *InstancePtr, u16 DeviceId) {
	XV_scenechange_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XV_scenechange_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	InstancePtr->ScdConfig = ConfigPtr;

	return XV_scenechange_CfgInitialize(InstancePtr, ConfigPtr);
}

#endif
