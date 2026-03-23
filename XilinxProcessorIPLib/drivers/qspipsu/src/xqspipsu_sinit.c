/******************************************************************************
* Copyright (C) 2014 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xqspipsu_sinit.c
* @addtogroup qspipsu_api QSPIPSU APIs
* @{
*
* The implementation of the XQspiPsu component's static initialization
* functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.0   hk  08/21/14 First release
* 1.15  akm 10/26/21 Fix MISRA-C violations.
* 1.18  sb  06/07/23 Added support for system device-tree flow.
* 1.23  vlt 12/16/25 Update Doxygen comments to include SDT flow details.
* 1.23  vlt 03/14/26 Updated BaseAddress type from u32 to UINTPTR
*                    to support 64-bit addressing.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xqspipsu.h"
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
* The XQspiPsu_ConfigTable[] contains the configuration info for each device
* in the system.
* @if SDT
* @param	BaseAddress contains the base address of the device
* @else
* @param	DeviceId contains the unique ID of the device
* @endif
*
* @return       A pointer to the configuration found or NULL if the specified
*               device ID/BaseAddress was not found. See xqspipsu.h for the
*               definition of XQspiPsu_Config.
*
* @note        In XSCT/classic flow, DeviceId is used to look up the device
*              configuration.
*
******************************************************************************/
#ifndef SDT
XQspiPsu_Config *XQspiPsu_LookupConfig(u16 DeviceId)
{
	XQspiPsu_Config *CfgPtr = NULL;
	s32 Index;

	for (Index = 0; Index < XPAR_XQSPIPSU_NUM_INSTANCES; Index++) {
		if (XQspiPsu_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XQspiPsu_ConfigTable[Index];
			break;
		}
	}
	return (XQspiPsu_Config *)CfgPtr;
}
#else
XQspiPsu_Config *XQspiPsu_LookupConfig(UINTPTR BaseAddress)
{
       XQspiPsu_Config *CfgPtr = NULL;
       s32 Index;

       for (Index = 0; XQspiPsu_ConfigTable[Index].Name != NULL; Index++) {
               if ((XQspiPsu_ConfigTable[Index].BaseAddress == BaseAddress) ||
                   !BaseAddress) {
                       CfgPtr = &XQspiPsu_ConfigTable[Index];
                       break;
               }
       }
       return (XQspiPsu_Config *)CfgPtr;
}
#endif
/** @} */
