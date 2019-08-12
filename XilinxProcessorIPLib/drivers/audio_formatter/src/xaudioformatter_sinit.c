/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
* @file xaudioformatter_sinit.c
* @addtogroup audio_formatter_v1_0
* @{
*
* This file contains static initialization methods for Xilinx audio formatter
* core.
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xaudioformatter.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* XAudioFormatter_LookupConfig returns a reference to an XAudioFormatter_Config
* structure based on the unique device id, <i>DeviceId</i>. The return value
* will refer to an entry in the device configuration table defined in the
* xaudioformatter_g.c file.
*
* @param	DeviceId is the unique device ID of the device for the lookup
*		operation.
*
* @return	CfgPtr is a reference to a config record in the configuration
*		table (in xaudioformatter_g.c) corresponding to <i>DeviceId</i>,
*		or NULL if no match is found.
*
* @note		None.
******************************************************************************/
XAudioFormatter_Config *XAudioFormatter_LookupConfig(u16 DeviceId)
{
	XAudioFormatter_Config *CfgPtr = NULL;
	u32 Index;

	/* Checks all the instances */
	for (Index = 0; Index < XPAR_XAUDIOFORMATTER_NUM_INSTANCES;
								Index++) {
		if (XAudioFormatter_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XAudioFormatter_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}

u32 XAudioFormatter_Initialize(XAudioFormatter *InstancePtr, u16 DeviceId)
{
	XAudioFormatter_Config *CfgPtr = NULL;

	Xil_AssertNonvoid(InstancePtr != NULL);

	CfgPtr = XAudioFormatter_LookupConfig(DeviceId);
	if (CfgPtr == NULL) {
		InstancePtr->IsReady = 0;
		return XST_DEVICE_NOT_FOUND;
	}

	return XAudioFormatter_CfgInitialize(InstancePtr, CfgPtr);
}
/** @} */
