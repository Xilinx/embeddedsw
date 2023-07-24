/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
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

int XHdcp22_mmult_Initialize(XHdcp22_mmult *InstancePtr, u16 DeviceId) {
	XHdcp22_mmult_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XHdcp22_mmult_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XHdcp22_mmult_CfgInitialize(InstancePtr, ConfigPtr, ConfigPtr->BaseAddress);
}

#endif

/** @} */
