/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef __linux__

#include "xstatus.h"
#include "xparameters.h"
#include "xmpegtsmux.h"

extern XMpegtsmux_Config XMpegtsmux_ConfigTable[];

XMpegtsmux_Config *XMpegtsmux_LookupConfig(u16 DeviceId) {
	XMpegtsmux_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XMPEGTSMUX_NUM_INSTANCES; Index++) {
		if (XMpegtsmux_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XMpegtsmux_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XMpegtsmux_Initialize(XMpegtsmux *InstancePtr, u16 DeviceId) {
	XMpegtsmux_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XMpegtsmux_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XMpegtsmux_CfgInitialize(InstancePtr, ConfigPtr);
}

#endif
