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
#include "xv_letterbox.h"

#ifndef XPAR_XV_LETTERBOX_NUM_INSTANCES
#define XPAR_XV_LETTERBOX_NUM_INSTANCES   0
#endif

extern XV_letterbox_Config XV_letterbox_ConfigTable[];

#ifndef SDT
XV_letterbox_Config *XV_letterbox_LookupConfig(u16 DeviceId) {
    XV_letterbox_Config *ConfigPtr = NULL;

    int Index;

    for (Index = 0; Index < XPAR_XV_LETTERBOX_NUM_INSTANCES; Index++) {
        if (XV_letterbox_ConfigTable[Index].DeviceId == DeviceId) {
            ConfigPtr = &XV_letterbox_ConfigTable[Index];
            break;
        }
    }

    return ConfigPtr;
}

int XV_letterbox_Initialize(XV_letterbox *InstancePtr, u16 DeviceId) {
    XV_letterbox_Config *ConfigPtr;

    Xil_AssertNonvoid(InstancePtr != NULL);

    ConfigPtr = XV_letterbox_LookupConfig(DeviceId);
    if (ConfigPtr == NULL) {
        InstancePtr->IsReady = 0;
        return (XST_DEVICE_NOT_FOUND);
    }

    return XV_letterbox_CfgInitialize(InstancePtr,
                                      ConfigPtr,
                                      ConfigPtr->BaseAddress);
}
#else
XV_letterbox_Config *XV_letterbox_LookupConfig(UINTPTR BaseAddress) {
    XV_letterbox_Config *ConfigPtr = NULL;

    int Index;

    for (Index = (u32)0x0; XV_letterbox_ConfigTable[Index].Name != NULL; Index++) {
		if ((XV_letterbox_ConfigTable[Index].BaseAddress == BaseAddress) ||
		     !BaseAddress) {
			ConfigPtr = &XV_letterbox_ConfigTable[Index];
			break;
		}
	}

    return ConfigPtr;
}

int XV_letterbox_Initialize(XV_letterbox *InstancePtr, UINTPTR BaseAddress) {
    XV_letterbox_Config *ConfigPtr;

    Xil_AssertNonvoid(InstancePtr != NULL);

    ConfigPtr = XV_letterbox_LookupConfig(BaseAddress);
    if (ConfigPtr == NULL) {
        InstancePtr->IsReady = 0;
        return (XST_DEVICE_NOT_FOUND);
    }

    return XV_letterbox_CfgInitialize(InstancePtr,
                                      ConfigPtr,
                                      ConfigPtr->BaseAddress);
}
#endif
#endif
