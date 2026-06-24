/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hdmirx_sinit.c
*
* This file contains static initialization method for Xilinx HDMI RX core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.0   gm, mg 10/07/15 Initial release.

* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xv_hdmirx.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/

extern XV_HdmiRx_Config XV_HdmiRx_ConfigTable[];

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function returns a reference to an XV_HdmiRx_Config structure based
* on the base address, <i>BaseAddress</i>. The return value will refer to an entry in
* the device configuration table defined in the xv_hdmirx_g.c file.
*
* @param    BaseAddress is the unique base address of the HDMI RX core for the
*       lookup operation.
*
* @return   XV_HdmiRx_LookupConfig returns a reference to a config record
*       in the configuration table (in xv_hdmirx_g.c) corresponding
*       to <i>BaseAddress</i>, or NULL if no match is found.
*
* @note     None.
*
******************************************************************************/
XV_HdmiRx_Config *XV_HdmiRx_LookupConfig(UINTPTR BaseAddress)
{
	XV_HdmiRx_Config *CfgPtr = NULL;
	u32 Index;

	/* Checking for device id for which instance it is matching */
	for (Index = 0U; XV_HdmiRx_ConfigTable[Index].Name != NULL; Index++) {
		if ((XV_HdmiRx_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
		CfgPtr = &XV_HdmiRx_ConfigTable[Index];
		break;
		}
	}
	return (XV_HdmiRx_Config *)CfgPtr;
}
