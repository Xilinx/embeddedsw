/******************************************************************************
* Copyright (C) 2002 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xintc_options.c
* @addtogroup intc Overview
* @{
*
* Contains option functions for the XIntc driver. These functions allow the
* user to configure an instance of the XIntc driver.  This file requires other
* files of the component to be linked in also.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------
* 1.00b jhl  02/21/02 First release
* 1.00c rpm  10/17/03 New release. Support the relocation of the options flag
*                     from the instance structure to the xintc_g.c
*                     configuration table.
* 1.10c mta  03/21/07 Updated to new coding style
* 2.00a ktn  10/20/09 Updated to use HAL Processor APIs
* 2.06a bss  01/28/13 To support Cascade mode:
*		      Modified XIntc_SetOptions API.
* 3.19  adk  08/05/24 In SDT flow inorder to loop over config table use the
* 		      XPAR_INTC_NUM_DRV_INSTANCES define instead of config table
* 		      to reduce the size.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xintc.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
*
* Set the options for the interrupt controller driver. In Cascade mode same
* Option is set to Slave controllers.
*
* @param	InstancePtr is a pointer to the XIntc instance to be worked on.
* @param	Options to be set. The available options are described in
*		xintc.h.
*
* @return
* 		- XST_SUCCESS if the options were set successfully
* 		- XST_INVALID_PARAM if the specified option was not valid
*
* @note		None.
*
****************************************************************************/
int XIntc_SetOptions(XIntc *InstancePtr, u32 Options)
{
	XIntc_Config *CfgPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Make sure option request is valid
	 */
	if ((Options == XIN_SVC_SGL_ISR_OPTION) ||
	    (Options == XIN_SVC_ALL_ISRS_OPTION)) {
		InstancePtr->CfgPtr->Options = Options;
		/* If Cascade mode set the option for all Slaves */
		if (InstancePtr->CfgPtr->IntcType != XIN_INTC_NOCASCADE) {
			int Index;
#ifndef SDT
			for (Index = 1; Index <= XPAR_XINTC_NUM_INSTANCES - 1;
			     Index++) {
				CfgPtr = XIntc_LookupConfig(Index);
				CfgPtr->Options = Options;
			}
#else
			for (Index = 1; Index <= XPAR_INTC_NUM_DRV_INSTANCES - 1;
			     Index++) {
				CfgPtr = XIntc_LookupConfig(XIntc_ConfigTable[Index].BaseAddress);
				CfgPtr->Options = Options;
			}
#endif
		}
		return XST_SUCCESS;
	} else {
		return XST_INVALID_PARAM;
	}
}

/*****************************************************************************/
/**
*
* Return the currently set options.
*
* @param	InstancePtr is a pointer to the XIntc instance to be worked on.
*
* @return	The currently set options. The options are described in xintc.h.
*
* @note		None.
*
****************************************************************************/
u32 XIntc_GetOptions(XIntc *InstancePtr)
{
	/*
	 * Assert the arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return InstancePtr->CfgPtr->Options;
}
/** @} */
