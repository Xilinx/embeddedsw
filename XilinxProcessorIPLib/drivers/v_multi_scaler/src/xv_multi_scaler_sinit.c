/*************************************************************************
 * Copyright (c) 1986 - 2022 Xilinx, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef __linux__

#include "xstatus.h"
#include "xparameters.h"
#include "xv_multi_scaler.h"

XV_multi_scaler_Config *XV_multi_scaler_LookupConfig(u16 DeviceId)
{
	XV_multi_scaler_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XV_MULTI_SCALER_NUM_INSTANCES; Index++) {
		if (XV_multi_scaler_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XV_multi_scaler_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XV_multi_scaler_Initialize(XV_multi_scaler *InstancePtr, u16 DeviceId)
{
	XV_multi_scaler_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XV_multi_scaler_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return XST_DEVICE_NOT_FOUND;
	}

	return XV_multi_scaler_CfgInitialize(InstancePtr, ConfigPtr);
}

#endif
