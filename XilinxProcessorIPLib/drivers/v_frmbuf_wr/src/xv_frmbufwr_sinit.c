// ==============================================================
// Copyright (c) 1986 - 2021 Xilinx Inc. All rights reserved.
// Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

#ifndef __linux__

#include "xstatus.h"
#ifndef SDT
#include "xparameters.h"
#endif
#include "xv_frmbufwr.h"

extern XV_frmbufwr_Config XV_frmbufwr_ConfigTable[];

#ifndef SDT
XV_frmbufwr_Config *XV_frmbufwr_LookupConfig(u16 DeviceId) {
    XV_frmbufwr_Config *ConfigPtr = NULL;

    int Index;

    for (Index = 0; Index < XPAR_XV_FRMBUFWR_NUM_INSTANCES; Index++) {
        if (XV_frmbufwr_ConfigTable[Index].DeviceId == DeviceId) {
            ConfigPtr = &XV_frmbufwr_ConfigTable[Index];
            break;
        }
    }

    return ConfigPtr;
}

int XV_frmbufwr_Initialize(XV_frmbufwr *InstancePtr, u16 DeviceId) {
    XV_frmbufwr_Config *ConfigPtr;

    Xil_AssertNonvoid(InstancePtr != NULL);

    ConfigPtr = XV_frmbufwr_LookupConfig(DeviceId);
    if (ConfigPtr == NULL) {
        InstancePtr->IsReady = 0;
        return (XST_DEVICE_NOT_FOUND);
    }

    return XV_frmbufwr_CfgInitialize(InstancePtr,
                                      ConfigPtr,
                                      ConfigPtr->BaseAddress);
}
#else
XV_frmbufwr_Config *XV_frmbufwr_LookupConfig(UINTPTR BaseAddress) {
    XV_frmbufwr_Config *ConfigPtr = NULL;

    int Index;

    for (Index = 0; XV_frmbufwr_ConfigTable[Index].Name != NULL; Index++) {
        if ((XV_frmbufwr_ConfigTable[Index].BaseAddress == BaseAddress) ||
				!BaseAddress) {
            ConfigPtr = &XV_frmbufwr_ConfigTable[Index];
            break;
        }
    }

    return ConfigPtr;
}

int XV_frmbufwr_Initialize(XV_frmbufwr *InstancePtr, UINTPTR BaseAddress) {
    XV_frmbufwr_Config *ConfigPtr;

    Xil_AssertNonvoid(InstancePtr != NULL);

    ConfigPtr = XV_frmbufwr_LookupConfig(BaseAddress);
    if (ConfigPtr == NULL) {
        InstancePtr->IsReady = 0;
        return (XST_DEVICE_NOT_FOUND);
    }

    return XV_frmbufwr_CfgInitialize(InstancePtr,
                                      ConfigPtr,
                                      ConfigPtr->BaseAddress);
}
#endif
#endif
