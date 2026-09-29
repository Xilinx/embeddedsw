/******************************************************************************
*
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xddrcpsu.c
* @brief Implements DdrcPsu interface functions.
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
#include <string.h>

/************************** Function Prototypes ******************************/


/************************** Function Definitions *****************************/

#ifdef SDT
/*****************************************************************************/
/**
* @brief Initializes a specific XDdrcPsu instance.
*
* @param[out] InstancePtr Pointer to the XDdrcPsu instance.
* @param[in] CfgPtr      Reference to a structure containing information
*                        about a specific XDdrcPsu instance.
*
* @return
*        - XST_SUCCESS if initialization was successful.
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
