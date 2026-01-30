/******************************************************************************
* Copyright (C) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xrtcpsu_sinit.c
* @addtogroup rtcpsu Overview
* @{
*
* This file contains the implementation of the XRtcPsu driver's static
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
* 1.00  kvn    04/21/15 First release.
* 1.7   sne    03/01/19 Added Versal support.
* 1.7   sne    03/01/19 Fixed violations according to MISRAC-2012 standards
*                       modified the code such as
*                       No brackets to loop body,Declared the pointer param
*                       as Pointer to const,No brackets to then/else,
*                       Literal value requires a U suffix,Casting operation to a pointer
*                       Array has no bounds specified,Logical conjunctions need brackets.
* 1.13  ht     06/22/23 Added support for system device-tree flow.
* 1.17  vlt    12/16/25 Update Doxygen comments to include SDT flow details.
*       vlt    01/30/26 Fixed codespell errors.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xrtcpsu.h"
#ifndef SDT
#include "xparameters.h"
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
*
* Looks up the device configuration based on the unique device ID/BaseAddress.
* The XRtcPsu_ConfigTable[] contains the configuration info for each device in
* the system.
*
* @if SDT
* @param	BaseAddress contains the base address of the device
* @else
* @param	DeviceId contains the unique ID of the device
* @endif
*
* @return       A pointer to the configuration found or NULL if the specified
*               device ID/BaseAddress was not found. See xrtcpsu.h for the
*               definition of XRtcPsu_Config.
*
* @note        In XSCT/classic flow, DeviceId is used to look up the device
*              configuration.
*
******************************************************************************/
#ifndef SDT
XRtcPsu_Config *XRtcPsu_LookupConfig(u16 DeviceId)
{
	XRtcPsu_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; Index < (u32)XPAR_XRTCPSU_NUM_INSTANCES; Index++) {
		if (XRtcPsu_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XRtcPsu_ConfigTable[Index];
			break;
		}
	}

	return (XRtcPsu_Config *)CfgPtr;
}
#else
XRtcPsu_Config *XRtcPsu_LookupConfig(UINTPTR BaseAddress)
{
	XRtcPsu_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; XRtcPsu_ConfigTable[Index].Name != NULL; Index++) {
		if ((XRtcPsu_ConfigTable[Index].BaseAddr == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XRtcPsu_ConfigTable[Index];
			break;
		}
	}

	return (XRtcPsu_Config *)CfgPtr;
}
#endif
/** @} */
