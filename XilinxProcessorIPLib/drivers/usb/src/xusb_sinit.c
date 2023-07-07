/******************************************************************************
* Copyright (C) 2006 Vreelin Engineering, Inc.  All Rights Reserved.
* Copyright (C) 2007 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xusb_sinit.c
* @addtogroup usb Overview
* @{
*
* This file contains the implementation of the XUsb driver's static
* initialization functionality.
*
* @note		None.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a hvm  12/28/06 First release
* 5.6   pm   07/05/23 Added support for system device-tree flow.
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xstatus.h"
#include "xparameters.h"
#include "xusb.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/

extern XUsb_Config XUsb_ConfigTable[];

/************************** Function Prototypes *****************************/

/****************************************************************************/
/**
*
* Looks up the device configuration based on the unique device ID. A table
* contains the configuration info for each device in the system.
*
* @param	DeviceId contains the ID of the device for which the
*		device configuration pointer is to be returned.
*
* @return
*		- A pointer to the configuration found.
*		- NULL if the specified device ID was not found.
*
******************************************************************************/
#ifndef SDT
XUsb_Config *XUsb_LookupConfig(u16 DeviceId)
{
	XUsb_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0; Index < XPAR_XUSB_NUM_INSTANCES; Index++) {
		if (XUsb_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XUsb_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#else
XUsb_Config *XUsb_LookupConfig(u32 BaseAddress)
{
	XUsb_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; XUsb_ConfigTable[Index].Name != NULL; Index++) {
		if ((XUsb_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XUsb_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#endif
/** @} */
