// ==============================================================
// Copyright (c) 1986 - 2021 Xilinx Inc. All rights reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

#ifndef __linux__

#include "xstatus.h"
#include "xparameters.h"
#include "xv_demosaic.h"

#ifndef XPAR_XV_DEMOSAIC_NUM_INSTANCES
#define XPAR_XV_DEMOSAIC_NUM_INSTANCES   0
#endif

extern XV_demosaic_Config XV_demosaic_ConfigTable[];

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

#endif
