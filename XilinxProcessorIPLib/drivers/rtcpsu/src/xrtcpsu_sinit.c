/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xrtcpsu_sinit.c
* @addtogroup rtcpsu_v1_10
* @{
*
* This file contains the implementation of the XRtcPsu driver's static
* initialization functionality.
*
* @note		None.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date	Changes
* ----- -----  -------- -----------------------------------------------
* 1.00  kvn    04/21/15 First release.
* 1.7   sne    03/01/19 Added Versal support.
* 1.7   sne    03/01/19 Fixed violations according to MISRAC-2012 standards
*                       modified the code such as
*                       No brackets to loop body,Declared the poiner param
*                       as Pointer to const,No brackets to then/else,
*                       Literal value requires a U suffix,Casting operation to a pointer
*                       Array has no bounds specified,Logical conjunctions need brackets.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xrtcpsu.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
*
* This function looks for the device configuration based on the unique device
* ID. The table XRtcPsu_ConfigTable[] contains the configuration information for
* each device in the system.
*
* @param	DeviceId is the unique device ID of the device being looked up.
*
* @return	A pointer to the configuration table entry corresponding to the
*		given device ID, or NULL if no match is found.
*
* @note		None.
*
******************************************************************************/
XRtcPsu_Config *XRtcPsu_LookupConfig(u16 DeviceId)
{
	XRtcPsu_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; Index < (u32)XPAR_XRTCPSU_NUM_INSTANCES; Index++) {
		if (XRtcPsu_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XRtcPsu_ConfigTable[Index];
			break;
		}
	}

	return (XRtcPsu_Config *)CfgPtr;
}
/** @} */
