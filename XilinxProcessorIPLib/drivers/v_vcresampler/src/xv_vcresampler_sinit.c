// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

#ifndef __linux__

#include "xstatus.h"
#include "xparameters.h"
#include "xv_vcresampler.h"

#ifndef XPAR_XV_VCRESAMPLER_NUM_INSTANCES
#define XPAR_XV_VCRESAMPLER_NUM_INSTANCES   0
#endif

extern XV_vcresampler_Config XV_vcresampler_ConfigTable[];

XV_vcresampler_Config *XV_vcresampler_LookupConfig(u16 DeviceId) {
    XV_vcresampler_Config *ConfigPtr = NULL;

    int Index;

    for (Index = 0; Index < XPAR_XV_VCRESAMPLER_NUM_INSTANCES; Index++) {
        if (XV_vcresampler_ConfigTable[Index].DeviceId == DeviceId) {
            ConfigPtr = &XV_vcresampler_ConfigTable[Index];
            break;
        }
    }

    return ConfigPtr;
}

int XV_vcresampler_Initialize(XV_vcresampler *InstancePtr, u16 DeviceId) {
    XV_vcresampler_Config *ConfigPtr;

    Xil_AssertNonvoid(InstancePtr != NULL);

    ConfigPtr = XV_vcresampler_LookupConfig(DeviceId);
    if (ConfigPtr == NULL) {
        InstancePtr->IsReady = 0;
        return (XST_DEVICE_NOT_FOUND);
    }

    return XV_vcresampler_CfgInitialize(InstancePtr,
                                        ConfigPtr,
                                        ConfigPtr->BaseAddress);
}

#endif
