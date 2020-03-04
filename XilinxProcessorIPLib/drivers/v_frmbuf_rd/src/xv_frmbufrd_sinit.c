// ==============================================================
// Copyright (c) 1986 - 2020 Xilinx Inc. All rights reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

#ifndef __linux__

#include "xstatus.h"
#include "xparameters.h"
#include "xv_frmbufrd.h"

extern XV_frmbufrd_Config XV_frmbufrd_ConfigTable[];

XV_frmbufrd_Config *XV_frmbufrd_LookupConfig(u16 DeviceId) {
    XV_frmbufrd_Config *ConfigPtr = NULL;

    int Index;

    for (Index = 0; Index < XPAR_XV_FRMBUFRD_NUM_INSTANCES; Index++) {
        if (XV_frmbufrd_ConfigTable[Index].DeviceId == DeviceId) {
            ConfigPtr = &XV_frmbufrd_ConfigTable[Index];
            break;
        }
    }

    return ConfigPtr;
}

int XV_frmbufrd_Initialize(XV_frmbufrd *InstancePtr, u16 DeviceId) {
    XV_frmbufrd_Config *ConfigPtr;

    Xil_AssertNonvoid(InstancePtr != NULL);

    ConfigPtr = XV_frmbufrd_LookupConfig(DeviceId);
    if (ConfigPtr == NULL) {
        InstancePtr->IsReady = 0;
        return (XST_DEVICE_NOT_FOUND);
    }

    return XV_frmbufrd_CfgInitialize(InstancePtr,
                                      ConfigPtr,
                                      ConfigPtr->BaseAddress);
}

#endif
