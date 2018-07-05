/*******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xdpdma_sinit.c
 * @addtogroup dpdma_v1_2
 * @{
 *
 * This file contains static initialization methods for the XDpDma driver.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   aad  01/20/15 Initial release.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xdpdma.h"
#include "xparameters.h"

/*************************** Variable Declarations ****************************/

/**
 * A table of configuration structures containing the configuration information
 * for each DisplayPort TX core in the system.
 */
extern XDpDma_Config XDpDma_ConfigTable[XPAR_XDPDMA_NUM_INSTANCES];

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function looks for the device configuration based on the unique device
 * ID. The table XDpDma_ConfigTable[] contains the configuration information for
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
XDpDma_Config *XDpDma_LookupConfig(u16 DeviceId)
{
	XDpDma_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0; Index < XPAR_XDPDMA_NUM_INSTANCES; Index++) {
		if (XDpDma_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XDpDma_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
/** @} */
