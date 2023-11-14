/*******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xdp_sinit.c
 * @addtogroup dp Overview
 * @{
 *
 * This file contains static initialization methods for the XDp driver.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   als  01/20/15 Initial release. TX code merged from the dptx driver.
 * 6.0   tu   02/06/17 Initialize CfgPtr to NULL.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xdp.h"
#include "xparameters.h"

/*************************** Variable Declarations ****************************/

/**
 * A table of configuration structures containing the configuration information
 * for each DisplayPort TX core in the system.
 */
extern XDp_Config XDp_ConfigTable[XPAR_XDP_NUM_INSTANCES];

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function looks for the device configuration based on the unique device
 * ID. The table XDp_ConfigTable[] contains the configuration information for
 * each device in the system.
 *
 * @param	DeviceId is the unique device ID of the device being looked up.
 *
 * @return	A pointer to the configuration table entry corresponding to the
 *		given device ID, or NULL if no match is found.
 *
 * @note	None.
 *
*******************************************************************************/
#ifndef SDT
XDp_Config *XDp_LookupConfig(u16 DeviceId)
{
	XDp_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0; Index < XPAR_XDP_NUM_INSTANCES; Index++) {
		if (XDp_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XDp_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#else
XDp_Config *XDp_LookupConfig(UINTPTR BaseAddress)
{
	XDp_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0; XDp_ConfigTable[Index].Name != NULL; Index++) {
		if ( (XDp_ConfigTable[Index].BaseAddr == BaseAddress) ||
             !BaseAddress) {
			CfgPtr = &XDp_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#endif
/** @} */
