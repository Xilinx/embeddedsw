// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

#ifndef __linux__

#include "xstatus.h"
#include "xparameters.h"
#include "xv_letterbox.h"

#ifndef XPAR_XV_LETTERBOX_NUM_INSTANCES
#define XPAR_XV_LETTERBOX_NUM_INSTANCES   0
#endif

extern XV_letterbox_Config XV_letterbox_ConfigTable[];

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

#endif
