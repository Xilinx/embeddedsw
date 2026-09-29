/******************************************************************************
* Copyright (C) 2013 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xllfifo_sinit.c
* @brief Implements XLlFifo static initialization.
*
* This file contains static initialization functionality for Axi Streaming FIFO
* driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 3.00a adk 9/10/2013 initial release
* 6.1   vlt 12/12/25 Update Doxygen comments to include SDT flow details.
* 6.2   tb  09/15/26 Align Doxygen comments with AMD standards.
* </pre>
*
******************************************************************************/

/**
* @addtogroup llfifo_api LLFIFO APIs
* @{
*/

/***************************** Include Files *********************************/

#include "xllfifo.h"
#ifndef SDT
#include "xparameters.h"
#endif

/*****************************************************************************/
/**
* @brief Looks up the XLlFifo configuration.
*
* Looks up the device configuration based on the unique device ID/BaseAddress.
* The XLlFifo_ConfigTable[] contains the configuration info for each device
* in the system.
*
* @if SDT
* @param[in]	BaseAddress Contains the base address of the device.
* @else
* @param[in]	DeviceId Contains the unique ID of the device.
* @endif
*
* @return      A pointer to the configuration found or NULL if the specified
*              device ID/BaseAddress was not found. See xllfifo.h for the
*              definition of XLlFifo_Config.
*
* @note        In XSCT/classic flow, DeviceId is used to look up the device
*              configuration.
*
******************************************************************************/
#ifndef SDT
XLlFifo_Config *XLlFfio_LookupConfig(u32 DeviceId)
{
	extern XLlFifo_Config XLlFifo_ConfigTable[];
	XLlFifo_Config *CfgPtr;
	u32 Index;

	CfgPtr = NULL;

	for (Index = 0; Index < XPAR_XLLFIFO_NUM_INSTANCES; Index++) {
		if (XLlFifo_ConfigTable[Index].DeviceId == DeviceId) {

			CfgPtr = &XLlFifo_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#else
XLlFifo_Config *XLlFfio_LookupConfig(UINTPTR BaseAddress)
{
	extern XLlFifo_Config XLlFifo_ConfigTable[];
	XLlFifo_Config *CfgPtr;
	u32 Index;

	CfgPtr = NULL;

	for (Index = (u32)0x0; XLlFifo_ConfigTable[Index].Name != NULL; Index++) {
		if ((XLlFifo_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XLlFifo_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#endif
/** @} */
