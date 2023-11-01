// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

#ifndef __linux__

#include "xstatus.h"
#ifndef SDT
#include "xparameters.h"
#endif
#include "xv_deinterlacer.h"

#ifndef XPAR_XV_DEINTERLACER_NUM_INSTANCES
#define XPAR_XV_DEINTERLACER_NUM_INSTANCES  0
#endif

extern XV_deinterlacer_Config XV_deinterlacer_ConfigTable[];

#ifndef SDT
XV_deinterlacer_Config *XV_deinterlacer_LookupConfig(u16 DeviceId) {
    XV_deinterlacer_Config *ConfigPtr = NULL;

    int Index;

    for (Index = 0; Index < XPAR_XV_DEINTERLACER_NUM_INSTANCES; Index++) {
        if (XV_deinterlacer_ConfigTable[Index].DeviceId == DeviceId) {
            ConfigPtr = &XV_deinterlacer_ConfigTable[Index];
            break;
        }
    }

    return ConfigPtr;
}

int XV_deinterlacer_Initialize(XV_deinterlacer *InstancePtr, u16 DeviceId) {
    XV_deinterlacer_Config *ConfigPtr;

    Xil_AssertNonvoid(InstancePtr != NULL);

    ConfigPtr = XV_deinterlacer_LookupConfig(DeviceId);
    if (ConfigPtr == NULL) {
        InstancePtr->IsReady = 0;
        return (XST_DEVICE_NOT_FOUND);
    }

    return XV_deinterlacer_CfgInitialize(InstancePtr,
                                         ConfigPtr,
                                         ConfigPtr->BaseAddress);
}
#else
XV_deinterlacer_Config *XV_deinterlacer_LookupConfig(UINTPTR BaseAddress) {
    XV_deinterlacer_Config *ConfigPtr = NULL;

    int Index;

    for (Index = (u32)0x0; XV_deinterlacer_ConfigTable[Index].Name != NULL; Index++) {
		if ((XV_deinterlacer_ConfigTable[Index].BaseAddress == BaseAddress) ||
		     !BaseAddress) {
			ConfigPtr = &XV_deinterlacer_ConfigTable[Index];
			break;
		}
	}

    return ConfigPtr;
}

int XV_deinterlacer_Initialize(XV_deinterlacer *InstancePtr, UINTPTR BaseAddress) {
    XV_deinterlacer_Config *ConfigPtr;

    Xil_AssertNonvoid(InstancePtr != NULL);

    ConfigPtr = XV_deinterlacer_LookupConfig(BaseAddress);
    if (ConfigPtr == NULL) {
        InstancePtr->IsReady = 0;
        return (XST_DEVICE_NOT_FOUND);
    }

    return XV_deinterlacer_CfgInitialize(InstancePtr,
                                         ConfigPtr,
                                         ConfigPtr->BaseAddress);
}
#endif
#endif
