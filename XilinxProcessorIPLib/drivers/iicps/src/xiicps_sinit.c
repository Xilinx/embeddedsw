/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xiicps_sinit.c
* @addtogroup iicps_api IICPS APIs
* @{
*
* The xiicps_sinit.c file contains implementation of the XIicPs component's
* static initialization functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------
* 1.00a drg/jz 01/30/10 First release
* 3.00	sk     01/31/15	Modified the code according to MISRAC 2012 Compliant.
* 3.18  gm     07/14/23 Added SDT support.
* 3.23  vlt    12/12/25 Update Doxygen comments to include SDT flow details.
* 3.23  vlt    03/14/26 Updated BaseAddress type from u32 to UINTPTR
*                       to support 64-bit addressing.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xstatus.h"
#ifndef SDT
#include "xparameters.h"
#endif
#include "xiicps.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
*
* Looks up the device configuration based on the unique device ID/BaseAddress.
* The XIicPs_ConfigTable[] contains the configuration info for each device in
* the system.
*
* @if SDT
* @param	BaseAddress contains the base address of the device
* @else
* @param	DeviceId contains the unique ID of the device
* @endif
*
* @return       A pointer to the configuration found or NULL if the specified
*               device ID/BaseAddress was not found. See xiicps.h for the
*               definition of XIicPs_Config.
*
* @note        In XSCT/classic flow, DeviceId is used to look up the device
*              configuration.
*
******************************************************************************/
#ifndef SDT
XIicPs_Config *XIicPs_LookupConfig(u16 DeviceId)
{
	XIicPs_Config *CfgPtr = NULL;
	s32 Index;

	for (Index = 0; Index < XPAR_XIICPS_NUM_INSTANCES; Index++) {
		if (XIicPs_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XIicPs_ConfigTable[Index];
			break;
		}
	}

	return (XIicPs_Config *)CfgPtr;
}
#else
XIicPs_Config *XIicPs_LookupConfig(UINTPTR BaseAddress)
{
	XIicPs_Config *CfgPtr = NULL;
	s32 Index;

	for (Index = 0; XIicPs_ConfigTable[Index].Name != NULL; Index++) {
		if (XIicPs_ConfigTable[Index].BaseAddress == BaseAddress ||
		    !BaseAddress) {
			CfgPtr = &XIicPs_ConfigTable[Index];
			break;
		}
	}

	return (XIicPs_Config *)CfgPtr;
}
#endif
/** @} */
