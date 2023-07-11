/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
 *
 * @file xusbps_sinit.c
* @addtogroup usbps Overview
* @{
 *
 * The implementation of the XUsbPs driver's static initialization
 * functionality.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.00a wgr  10/10/10 First release
 * 2.8   pm   07/07/23 Added support for system device-tree flow.
 * </pre>
 *
 *****************************************************************************/

/***************************** Include Files ********************************/

#include "xstatus.h"
#include "xusbps.h"
#include "xparameters.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/

extern XUsbPs_Config XUsbPs_ConfigTable[];

/************************** Function Prototypes *****************************/

/****************************************************************************/
/**
*
* Looks up the controller configuration based on the unique controller ID. A
* table contains the configuration info for each controller in the system.
*
* @param	DeviceID is the ID of the controller to look up the
*		configuration for.
*
* @return
*		A pointer to the configuration found or NULL if the specified
*		controller ID was not found.
*
******************************************************************************/
#ifndef SDT
XUsbPs_Config *XUsbPs_LookupConfig(u16 DeviceID)
{
	XUsbPs_Config *CfgPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XUSBPS_NUM_INSTANCES; Index++) {
		if (XUsbPs_ConfigTable[Index].DeviceID == DeviceID) {
			CfgPtr = &XUsbPs_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#else
XUsbPs_Config *XUsbPs_LookupConfig(u32 BaseAddress)
{
	XUsbPs_Config *CfgPtr = NULL;

	int Index;

	for (Index = 0U; XUsbPs_ConfigTable[Index].Name != NULL; Index++) {
		if ((XUsbPs_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XUsbPs_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#endif
/** @} */
