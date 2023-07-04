/******************************************************************************
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xstatus.h"
#ifndef SDT
#include "xparameters.h"
#endif
#include "xsdfec.h"

extern XSdFec_Config XSdFec_ConfigTable[];

#ifndef SDT
XSdFec_Config *XSdFecLookupConfig(u16 DeviceId) {
	XSdFec_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XSDFEC_NUM_INSTANCES; Index++) {
		if (XSdFec_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XSdFec_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}
#else
XSdFec_Config *XSdFec_LookupConfig(UINTPTR BaseAddress)
{
       XSdFec_Config *CfgPtr = NULL;
       u32 Index;

       /* Checks all the instances */
       for (Index = (u32)0x0; XSdFec_ConfigTable[Index].Name != NULL; Index++) {
               if ((XSdFec_ConfigTable[Index].BaseAddress == BaseAddress) ||
                    !BaseAddress) {
                       CfgPtr = &XSdFec_ConfigTable[Index];
                       break;
               }
       }

       return CfgPtr;
}
#endif

int XSdFecInitialize(XSdFec *InstancePtr, u16 DeviceId) {
	XSdFec_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XSdFecLookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XSdFecCfgInitialize(InstancePtr, ConfigPtr);
}


