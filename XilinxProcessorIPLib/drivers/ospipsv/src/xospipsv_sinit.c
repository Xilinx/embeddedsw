/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xospipsv_sinit.c
* @addtogroup ospipsv_v1_3
* @{
*
* The implementation of the XOspiPsv component's static initialization
* functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.0   hk  02/19/18 First release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xospipsv.h"
#include "xstatus.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
* @brief
* Looks up the device configuration based on the unique device ID. A table
* contains the configuration info for each device in the system.
*
* @param	DeviceId contains the ID of the device to look up the
*		configuration for.
*
* @return
*		A pointer to the configuration found or NULL if the specified device ID
* 		was not found. See XOspiPsv.h for the definition of XOspiPsv_Config.
*
******************************************************************************/
XOspiPsv_Config *XOspiPsv_LookupConfig(u16 DeviceId)
{
	XOspiPsv_Config *CfgPtr = NULL;
	s32 Index;

	for (Index = 0; Index < XPAR_XOSPIPSV_NUM_INSTANCES; Index++) {
		if (XOspiPsv_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XOspiPsv_ConfigTable[Index];
			break;
		}
	}
	return (XOspiPsv_Config *)CfgPtr;
}
/** @} */
