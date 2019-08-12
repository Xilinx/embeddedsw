/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xhdcp22_mmult_sinit.c
* @addtogroup hdcp22_mmult_v1_1
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
