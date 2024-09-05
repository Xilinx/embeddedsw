/******************************************************************************
*
* Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xddrcpsu.c
* @addtogroup ddrcpsu Overview
* @{
*
* This file contains the implementation of the interface functions for DDRCPSU
* driver. Refer to the header file xddrcpsu.h for more detailed information.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ---------------------------------------------------
* 1.5	ht	08/03/23 Added support for system device-tree flow.
* 1.6   rma     01/12/23 Update driver sources to fix compilation warnings
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xddrcpsu.h"
#include <string.h>

/************************** Function Prototypes ******************************/


/************************** Function Definitions *****************************/

#ifdef SDT
/*****************************************************************************/
/**
*
* @param	InstancePtr is a pointer to the XCsuDma instance.
* @param	CfgPtr is a reference to a structure containing information
*		about a specific XCsuDma instance.
*
* @return
*		- XST_SUCCESS if initialization was successful.
*
* @note		None.
*
******************************************************************************/
s32 XDdrcPsu_CfgInitialize(XDdrcPsu *InstancePtr, XDdrcpsu_Config *CfgPtr)
{

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);

	/* Setup the instance */
	(void)memcpy((void *) & (InstancePtr->Config), (const void *)CfgPtr,
		     sizeof(XDdrcpsu_Config));


	InstancePtr->IsReady = (u32)(XIL_COMPONENT_IS_READY);

	return (XST_SUCCESS);

}
#endif
/** @} */
