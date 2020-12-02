// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

#ifndef __linux__

#include "xstatus.h"
#include "xparameters.h"
#include "xv_csc.h"

#ifndef XPAR_XV_CSC_NUM_INSTANCES
#define XPAR_XV_CSC_NUM_INSTANCES   0
#endif

extern XV_csc_Config XV_csc_ConfigTable[];

XV_csc_Config *XV_csc_LookupConfig(u16 DeviceId) {
    XV_csc_Config *ConfigPtr = NULL;

    int Index;

    for (Index = 0; Index < XPAR_XV_CSC_NUM_INSTANCES; Index++) {
        if (XV_csc_ConfigTable[Index].DeviceId == DeviceId) {
            ConfigPtr = &XV_csc_ConfigTable[Index];
            break;
        }
    }

    return ConfigPtr;
}

int XV_csc_Initialize(XV_csc *InstancePtr, u16 DeviceId) {
    XV_csc_Config *ConfigPtr;

    Xil_AssertNonvoid(InstancePtr != NULL);

    ConfigPtr = XV_csc_LookupConfig(DeviceId);
    if (ConfigPtr == NULL) {
        InstancePtr->IsReady = 0;
        return (XST_DEVICE_NOT_FOUND);
    }

    return XV_csc_CfgInitialize(InstancePtr,
                                ConfigPtr,
                                ConfigPtr->BaseAddress);
}

#endif
