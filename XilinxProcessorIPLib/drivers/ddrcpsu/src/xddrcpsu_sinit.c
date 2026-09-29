/******************************************************************************
*
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xddrcpsu_sinit.c
* @brief Implements XDdrcPsu configuration lookup.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ---------------------------------------------------
* 1.5	ht	08/03/2023 Added support for system device-tree flow.
* 1.9   tb      09/11/26 Align Doxygen comments with AMD standards.
* </pre>
*
******************************************************************************/

/**
* @addtogroup ddrcpsu_api DDRCPSU APIs
* @{
*/

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
* @brief Looks up XDdrcPsu configuration by base address.
*
* Returns a reference to an XDdrcpsu_Config record in the configuration table
* defined in xddrcpsu_g.c.
*
* @param[in] BaseAddress Unique base address of the device for the lookup
*                        operation.
*
* @return Pointer to the matching config record, or NULL if no match is found.
*
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
