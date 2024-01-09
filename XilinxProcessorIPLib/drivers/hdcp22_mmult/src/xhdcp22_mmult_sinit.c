/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2023-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xhdcp22_mmult_sinit.c
* @addtogroup hdcp22_mmult Overview
* @{
* @details
*
* This file contains the static initialization file for the Xilinx
* Montgomery Multiplier (Mmult) core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  MH     10/01/15 Initial release.
* </pre>
*
******************************************************************************/

#ifndef __linux__

#include "xstatus.h"
#include "xparameters.h"
#include "xhdcp22_mmult.h"

#ifndef XPAR_XHDCP22_MMULT_NUM_INSTANCES
#define XPAR_XHDCP22_MMULT_NUM_INSTANCES 0
#endif

extern XHdcp22_mmult_Config XHdcp22_mmult_ConfigTable[];

#ifndef SDT
XHdcp22_mmult_Config *XHdcp22_mmult_LookupConfig(u16 DeviceId) {
	XHdcp22_mmult_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XHDCP22_MMULT_NUM_INSTANCES; Index++) {
		if (XHdcp22_mmult_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XHdcp22_mmult_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}
#else
XHdcp22_mmult_Config *XHdcp22_mmult_LookupConfig(UINTPTR BaseAddress)
{
	XHdcp22_mmult_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; XHdcp22_mmult_ConfigTable[Index].Name != NULL; Index++) {
		if ((XHdcp22_mmult_ConfigTable[Index].BaseAddress == BaseAddress)
		    || (!BaseAddress)) {
			ConfigPtr = &XHdcp22_mmult_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}
#endif
#ifndef SDT
int XHdcp22_mmult_Initialize(XHdcp22_mmult *InstancePtr, u16 DeviceId)
#else
int XHdcp22_mmult_Initialize(XHdcp22_mmult *InstancePtr, UINTPTR BaseAddress)
#endif
{
	XHdcp22_mmult_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

#ifndef SDT
	ConfigPtr = XHdcp22_mmult_LookupConfig(DeviceId);
#else
	ConfigPtr = XHdcp22_mmult_LookupConfig(BaseAddress);
#endif
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XHdcp22_mmult_CfgInitialize(InstancePtr, ConfigPtr, ConfigPtr->BaseAddress);
}

#endif

/** @} */
