/******************************************************************************
* Copyright (C) 2015 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xnandpsu_sinit.c
* @brief Implements the static initialization of the XNandPsu driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date	   Changes
* ----- ----   ----------  -----------------------------------------------
* 1.0   nm     05/06/2014  First release
* 1.12  akm    06/27/23    Update the driver to support for system device-tree flow.
* 1.16  vlt    12/15/25    Update Doxygen comments to include SDT flow details.
* 1.17  tb     10/01/26 Align Doxygen comments with AMD documentation standards
* </pre>
*
******************************************************************************/

/**
* @addtogroup nandpsu_api NANDPSU APIs
* @{
*/

/***************************** Include Files ********************************/
#include "xstatus.h"
#ifndef SDT
#include "xparameters.h"
#endif
#include "xnandpsu.h"
/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/

extern XNandPsu_Config XNandPsu_ConfigTable[];

/************************** Function Prototypes *****************************/

/****************************************************************************/
/**
*
* @brief	Looks up the device configuration based on the unique device ID
*		or base address.
*
* The XNandPsu_ConfigTable[] contains the configuration info for each device
* in the system.
*
* @if SDT
* @param[in]	BaseAddress Base address of the device.
* @else
* @param[in]	DevID Unique ID of the device.
* @endif
*
* @return	Pointer to the configuration found, or NULL if the specified
*		device ID or base address is not found. See xnandpsu.h for
*		the definition of XNandPsu_Config.
*
* @note	In the XSCT/Classic flow, DevID selects the device
*		configuration. In the system device-tree flow, BaseAddress
*		selects it.
*
******************************************************************************/
#ifndef SDT
XNandPsu_Config *XNandPsu_LookupConfig(u16 DevID)
{
	XNandPsu_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; Index < (u32)XPAR_XNANDPSU_NUM_INSTANCES; Index++) {
		if (XNandPsu_ConfigTable[Index].DeviceId == DevID) {
			CfgPtr = &XNandPsu_ConfigTable[Index];
			break;
		}
	}

	return (XNandPsu_Config *)CfgPtr;
}
#else
XNandPsu_Config *XNandPsu_LookupConfig(UINTPTR BaseAddress)
{
	XNandPsu_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; XNandPsu_ConfigTable[Index].Name != NULL; Index++) {
		if ((XNandPsu_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XNandPsu_ConfigTable[Index];
			break;
		}
	}

	return (XNandPsu_Config *)CfgPtr;
}
#endif
/** @} */
