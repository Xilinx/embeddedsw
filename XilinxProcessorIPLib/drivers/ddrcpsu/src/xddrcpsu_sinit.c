/******************************************************************************
*
* Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xddrcpsu_sinit.c
* @addtogroup ddrcpsu Overview
* @{
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ---------------------------------------------------
* 1.5	ht	08/03/2023 Added support for system device-tree flow.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xddrcpsu.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

#ifdef SDT
/*****************************************************************************/
/**
*
* XDdrcPsu_LookupConfig returns a reference to an XDdrcpsu_Config structure
* based on the unique device id, <i>DeviceId</i>. The return value will refer
* to an entry in the device configuration table defined in the xddrcpsu_g.c
* file.
*
* @param	BaseAddress is the unique base address of the device for the
* 		lookup operation.
*
* @return	CfgPtr is a reference to a config record in the configuration
*		table (in xddrcpsu_g.c) corresponding to <i>BaseAddress</i>, or
*		NULL if no match is found.
*
* @note		None.
******************************************************************************/
XDdrcpsu_Config *XDdrcPsu_LookupConfig(UINTPTR BaseAddress)
{
	extern XDdrcpsu_Config XDdrcpsu_ConfigTable[];
	XDdrcpsu_Config *CfgPtr = NULL;
	u32 Index;

	/* Checks all the instances */
	for (Index = (u32)0x0; XDdrcpsu_ConfigTable[Index].Name != NULL; Index++) {
		if ((XDdrcpsu_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XDdrcpsu_ConfigTable[Index];
			break;
		}
	}

	return (XDdrcpsu_Config *)CfgPtr;
}
#endif
/** @} */
