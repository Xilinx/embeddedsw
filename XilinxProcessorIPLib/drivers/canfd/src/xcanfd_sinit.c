/******************************************************************************
* Copyright (C) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcanfd_sinit.c
* @addtogroup canfd Overview
* @{
*
* This file contains the implementation of the XCanFd driver's static
* initialization functionality.
*
* @note		None.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date	Changes
* ----- -----  -------- -----------------------------------------------
* 1.0   nsk    06/03/15 First release
* 1.3   ask    08/08/18 Fixed doxygen warnings
* 2.0	ask    09/12/18 Added support for canfd 2.0 spec sequential mode
* 2.8	ht     06/19/23 Added support for system device-tree flow
* 2.12  vlt    11/05/25 Add 64-bit Addressing Support.
* 2.12  vlt    12/12/25 Update Doxygen comments to include SDT flow details.


* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcanfd.h"
#ifndef SDT
#include "xparameters.h"
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/


/*****************************************************************************/
/**
*
* Looks up the device configuration based on the unique device ID/BaseAddress.
* The XCanFd_ConfigTable[] contains the configuration info for each device
* in the system.
*
* @if SDT
* @param	BaseAddress contains the base address of the device
* @else
* @param	DeviceId contains the unique ID of the device
* @endif
*
* @return       A pointer to the configuration found or NULL if the specified
*               device ID/BaseAddress was not found. See xcanfd.h for the
*               definition of XCanFd_Config.
*
* @note        In XSCT/classic flow, DeviceId is used to look up the device
*              configuration.
*
******************************************************************************/
#ifndef SDT
XCanFd_Config *XCanFd_LookupConfig(u16 DeviceId)
{
	XCanFd_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0; Index < XPAR_XCANFD_NUM_INSTANCES; Index++) {
		if (XCanFd_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XCanFd_ConfigTable[Index];
			break;
		}
	}

	return (XCanFd_Config *)CfgPtr;
}
#else
XCanFd_Config *XCanFd_LookupConfig(UINTPTR BaseAddress)
{
	XCanFd_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; XCanFd_ConfigTable[Index].Name != NULL; Index++) {
		if ((XCanFd_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XCanFd_ConfigTable[Index];
			break;
		}
	}

	return (XCanFd_Config *)CfgPtr;
}
#endif
/** @} */
