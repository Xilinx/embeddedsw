/******************************************************************************
* Copyright (C) 2017 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xuartpsv_sinit.c
*
* @brief Implements the static initialization for the XUartPsv driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who  Date      Changes
* ---  ---  --------- -----------------------------------------------
* 1.0  sg   09/18/17  First Release
* 1.14 vlt 12/18/25  Update Doxygen comments to include SDT flow details.
* 1.15  vn   09/10/26 Align Doxygen comments with AMD standards
*
* </pre>
*
******************************************************************************/

/** @addtogroup uartpsv Overview
 * @{
 */

/***************************** Include Files *********************************/

#include "xstatus.h"
#ifndef SDT
#include "xparameters.h"
#endif
#include "xuartpsv.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
*
* @brief Looks up the device configuration based on the unique device ID or
* base address.
*
* The XUartPsv_ConfigTable[] contains the configuration info for each device
* in the system.
*
* @if SDT
* @param[in]   BaseAddress Base address of the device.
* @else
* @param[in]   DeviceId Unique ID of the device.
* @endif
*
* @return      A pointer to the configuration found or NULL if the specified
*              device ID/BaseAddress was not found. See xuartpsv.h for the
*              definition of XUartPsv_Config.
*
* @note        In XSCT/classic flow, DeviceId is used to look up the device
*              configuration.
*
******************************************************************************/
#ifndef SDT
XUartPsv_Config *XUartPsv_LookupConfig(u16 DeviceId)
{
	XUartPsv_Config *CfgPtr = NULL;

	u32 Index;

	for (Index = 0U; Index < (u32)XPAR_XUARTPSV_NUM_INSTANCES; Index++) {
		if (XUartPsv_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XUartPsv_ConfigTable[Index];
			break;
		}
	}

	return (XUartPsv_Config *)CfgPtr;
}
#else
XUartPsv_Config *XUartPsv_LookupConfig(UINTPTR BaseAddress)
{
	XUartPsv_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; XUartPsv_ConfigTable[Index].Name != NULL; Index++) {
		if ((XUartPsv_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XUartPsv_ConfigTable[Index];
			break;
		}
	}

	return (XUartPsv_Config *)CfgPtr;
}
#endif
/** @} */
