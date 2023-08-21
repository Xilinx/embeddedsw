/*******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xdppsu_sinit.c
 *
 * This file contains static initialization methods for the XDpPsu driver.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   aad  05/17/17 Initial release.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xdppsu.h"
#ifndef SDT
#include "xparameters.h"
#endif

/*************************** Variable Declarations ****************************/
/*************************** Constant Declarations ****************************/

/**
 * A table of configuration structures containing the configuration information
 * for each DisplayPort TX core in the system.
 */
#ifndef SDT
extern XDpPsu_Config XDpPsu_ConfigTable[XPAR_XDPPSU_NUM_INSTANCES];
#else
extern XDpPsu_Config XDpPsu_ConfigTable[];
#endif

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function looks for the device configuration based on the unique device
 * ID. The table XDpPsu_ConfigTable[] contains the configuration information for
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
XDpPsu_Config *XDpPsu_LookupConfig(u16 DeviceId)
{
	XDpPsu_Config *CfgPtr;
	u32 Index;

	for (Index = 0; Index < XPAR_XDPPSU_NUM_INSTANCES; Index++) {
		if (XDpPsu_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XDpPsu_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#else
XDpPsu_Config *XDpPsu_LookupConfig(u32 BaseAddress)
{
	XDpPsu_Config *CfgPtr;
	u32 Index;

    for (Index = 0; XDpPsu_ConfigTable[Index].Name != NULL; Index++) {
		if (XDpPsu_ConfigTable[Index].BaseAddr == BaseAddress) {
			CfgPtr = &XDpPsu_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#endif
