/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hcresampler_sinit.c
* @addtogroup v_hcresampler Overview
*
*/

#ifndef __linux__

#include "xstatus.h"
#ifndef SDT
#include "xparameters.h"
#endif
#include "xv_hcresampler.h"

#ifndef XPAR_XV_HCRESAMPLER_NUM_INSTANCES
#define XPAR_XV_HCRESAMPLER_NUM_INSTANCES   0
#endif

extern XV_hcresampler_Config XV_hcresampler_ConfigTable[];

#ifndef SDT
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
#else
XV_hcresampler_Config *XV_hcresampler_LookupConfig(UINTPTR BaseAddress) {
    XV_hcresampler_Config *ConfigPtr = NULL;

    int Index;

    for (Index = (u32)0x0; XV_hcresampler_ConfigTable[Index].Name != NULL; Index++) {
		if ((XV_hcresampler_ConfigTable[Index].BaseAddress == BaseAddress) ||
		     !BaseAddress) {
			ConfigPtr = &XV_hcresampler_ConfigTable[Index];
			break;
		}
    }

    return ConfigPtr;
}

int XV_hcresampler_Initialize(XV_hcresampler *InstancePtr, UINTPTR BaseAddress) {
    XV_hcresampler_Config *ConfigPtr;

    Xil_AssertNonvoid(InstancePtr != NULL);

    ConfigPtr = XV_hcresampler_LookupConfig(BaseAddress);
    if (ConfigPtr == NULL) {
        InstancePtr->IsReady = 0;
        return (XST_DEVICE_NOT_FOUND);
    }

    return XV_hcresampler_CfgInitialize(InstancePtr,
                                        ConfigPtr,
                                        ConfigPtr->BaseAddress);
}
#endif
#endif
