/******************************************************************************
* Copyright (C) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xnandpsu_sinit.c
* @addtogroup nandpsu_v1_8
* @{
*
* The implementation of the XNandPsu driver's static initialization
* functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date	   Changes
* ----- ----   ----------  -----------------------------------------------
* 1.0   nm     05/06/2014  First release
* </pre>
*
******************************************************************************/

/***************************** Include Files ********************************/
#include "xstatus.h"
#include "xparameters.h"
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
* Looks up the controller configuration based on the unique controller ID. A
* table contains the configuration info for each controller in the system.
*
* @param	DevID is the ID of the controller to look up the
*		configuration for.
*
* @return
*		A pointer to the configuration found or NULL if the specified
*		controller ID was not found.
*
******************************************************************************/
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
/** @} */
