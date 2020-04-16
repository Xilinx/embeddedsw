/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xstatus.h"
#include "xparameters.h"
#include "xsdfec.h"

extern XSdFec_Config XSdFec_ConfigTable[];

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


