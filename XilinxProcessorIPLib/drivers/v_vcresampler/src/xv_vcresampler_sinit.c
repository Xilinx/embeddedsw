// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================
/**
 * @file xv_vcresampler_sinit.c
 * @addtogroup v_vcresampler Overview
 */
#ifndef __linux__

#include "xstatus.h"
#include "xv_vcresampler.h"

#ifndef XPAR_XV_VCRESAMPLER_NUM_INSTANCES
#define XPAR_XV_VCRESAMPLER_NUM_INSTANCES   0
#endif

extern XV_vcresampler_Config XV_vcresampler_ConfigTable[];


/**
 * Looks up the configuration structure for the VCR resampler based on the provided base address.
 *
 * This function searches the XV_vcresampler_ConfigTable for an entry whose BaseAddress matches
 * the specified BaseAddress. If BaseAddress is zero, the first entry is returned. If a matching
 * configuration is found, a pointer to the configuration structure is returned; otherwise, NULL is returned.
 *
 * @param BaseAddress The base address of the VCR resampler instance to look up.
 *
 * @return Pointer to the matching XV_vcresampler_Config structure if found, otherwise NULL.
 */

XV_vcresampler_Config *XV_vcresampler_LookupConfig(UINTPTR BaseAddress) {
    XV_vcresampler_Config *ConfigPtr = NULL;

    int Index;

    for (Index = (u32)0x0; XV_vcresampler_ConfigTable[Index].Name != NULL; Index++) {
		if ((XV_vcresampler_ConfigTable[Index].BaseAddress == BaseAddress) ||
		     !BaseAddress) {
			ConfigPtr = &XV_vcresampler_ConfigTable[Index];
			break;
		}
	}

    return ConfigPtr;
}



 /** This function looks up the configuration for the given base address,
 * validates the instance pointer, and initializes the hardware instance
 * with the configuration found. If the configuration is not found,
 * the function sets the instance as not ready and returns an error code.
 *
 * @param InstancePtr Pointer to the XV_vcresampler instance to be initialized.
 * @param BaseAddress Base address of the device to initialize.
 *
 * @return
 *   - XST_SUCCESS if initialization is successful.
 *   - XST_DEVICE_NOT_FOUND if the configuration for the given base address is not found.
 */

int XV_vcresampler_Initialize(XV_vcresampler *InstancePtr, UINTPTR BaseAddress) {
    XV_vcresampler_Config *ConfigPtr;

    Xil_AssertNonvoid(InstancePtr != NULL);

    ConfigPtr = XV_vcresampler_LookupConfig(BaseAddress);
    if (ConfigPtr == NULL) {
        InstancePtr->IsReady = 0;
        return (XST_DEVICE_NOT_FOUND);
    }

    return XV_vcresampler_CfgInitialize(InstancePtr,
                                        ConfigPtr,
                                        ConfigPtr->BaseAddress);
}
#endif
