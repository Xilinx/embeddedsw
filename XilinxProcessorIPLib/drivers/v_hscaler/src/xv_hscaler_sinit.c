// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

#ifndef __linux__

#include "xstatus.h"
#include "xparameters.h"
#include "xv_hscaler.h"

#ifndef XPAR_XV_HSCALER_NUM_INSTANCES
#define XPAR_XV_HSCALER_NUM_INSTANCES   0
#endif

extern XV_hscaler_Config XV_hscaler_ConfigTable[];

XV_hscaler_Config *XV_hscaler_LookupConfig(u16 DeviceId) {
    XV_hscaler_Config *ConfigPtr = NULL;

    int Index;

    for (Index = 0; Index < XPAR_XV_HSCALER_NUM_INSTANCES; Index++) {
        if (XV_hscaler_ConfigTable[Index].DeviceId == DeviceId) {
            ConfigPtr = &XV_hscaler_ConfigTable[Index];
            break;
        }
    }

    return ConfigPtr;
}

int XV_hscaler_Initialize(XV_hscaler *InstancePtr, u16 DeviceId) {
    XV_hscaler_Config *ConfigPtr;

    Xil_AssertNonvoid(InstancePtr != NULL);

    ConfigPtr = XV_hscaler_LookupConfig(DeviceId);
    if (ConfigPtr == NULL) {
        InstancePtr->IsReady = 0;
        return (XST_DEVICE_NOT_FOUND);
    }

    return XV_hscaler_CfgInitialize(InstancePtr,
                                    ConfigPtr,
                                    ConfigPtr->BaseAddress);
}

#endif
