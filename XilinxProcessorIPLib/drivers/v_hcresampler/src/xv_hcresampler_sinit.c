// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

#ifndef __linux__

#include "xstatus.h"
#include "xparameters.h"
#include "xv_hcresampler.h"

#ifndef XPAR_XV_HCRESAMPLER_NUM_INSTANCES
#define XPAR_XV_HCRESAMPLER_NUM_INSTANCES   0
#endif

extern XV_hcresampler_Config XV_hcresampler_ConfigTable[];

XV_hcresampler_Config *XV_hcresampler_LookupConfig(u16 DeviceId) {
    XV_hcresampler_Config *ConfigPtr = NULL;

    int Index;

    for (Index = 0; Index < XPAR_XV_HCRESAMPLER_NUM_INSTANCES; Index++) {
        if (XV_hcresampler_ConfigTable[Index].DeviceId == DeviceId) {
            ConfigPtr = &XV_hcresampler_ConfigTable[Index];
            break;
        }
    }

    return ConfigPtr;
}

int XV_hcresampler_Initialize(XV_hcresampler *InstancePtr, u16 DeviceId) {
    XV_hcresampler_Config *ConfigPtr;

    Xil_AssertNonvoid(InstancePtr != NULL);

    ConfigPtr = XV_hcresampler_LookupConfig(DeviceId);
    if (ConfigPtr == NULL) {
        InstancePtr->IsReady = 0;
        return (XST_DEVICE_NOT_FOUND);
    }

    return XV_hcresampler_CfgInitialize(InstancePtr,
                                        ConfigPtr,
                                        ConfigPtr->BaseAddress);
}

#endif
