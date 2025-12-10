/******************************************************************************
* Copyright (C) 2006 Vreelin Engineering, Inc.  All Rights Reserved.
* Copyright (C) 2007 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
* 5.8   ka   11/09/25 Added 64-bit addressing support.
* 5.8   bdk  12/08/25 Updated comments to support SDT flow for Doxygen
*                     documentation.
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
* Looks up the device configuration based on the unique device ID/BaseAddress.
* A table contains the configuration info for each device in the system.
*
* @if SDT
* @param	BaseAddress contains the base address of the device
* @else
* @param	DeviceId contains the ID of the device for which the
*		device configuration pointer is to be returned.
* @endif
*
* @return
*		- A pointer to the configuration found.
*		- NULL if the specified device ID/BaseAddress was not found.
*
* @note		In XSCT/classic flow, DeviceId is used to look up the device
*		configuration.
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
XUsb_Config *XUsb_LookupConfig(UINTPTR BaseAddress)
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
