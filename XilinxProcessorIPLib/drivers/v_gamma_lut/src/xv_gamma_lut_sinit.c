// ==============================================================
// Copyright (c) 1986 - 2021 Xilinx Inc. All rights reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

#ifndef __linux__

#include "xstatus.h"
#include "xparameters.h"
#include "xv_gamma_lut.h"

#ifndef XPAR_XV_GAMMA_LUT_NUM_INSTANCES
#define XPAR_XV_GAMMA_LUT_NUM_INSTANCES   0
#endif

extern XV_gamma_lut_Config XV_gamma_lut_ConfigTable[];

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

#endif
