/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xaudioformatter_sinit.c
* @addtogroup audio_formatter_v1_1
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
