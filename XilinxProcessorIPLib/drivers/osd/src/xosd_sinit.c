/******************************************************************************
* Copyright (c) 2009 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xosd_sinit.c
* @addtogroup osd_v4_0
* @{
*
* This file contains the static initialization method for Xilinx Video
* On-Screen-Display (OSD) core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- ------------------------------------------------------
* 1.00a xd     08/18/08 First release
* 2.00a cjm    12/18/12 Converted from xio.h to xil_io.h, translating
*                       basic types, MB cache functions, exceptions and
*                       assertions to xil_io format.
* 4.0   adk    02/18/14 Renamed the following functions:
*                       XOSD_LookupConfig - > XOsd_LookupConfig
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xosd.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function gets a reference to an XOsd_Config structure based on the
* unique device id, <i>DeviceId</i>. The return value will refer to an entry
* in the core configuration table defined in the xosd_g.c file.
*
* @param	DeviceId is the unique core ID of the OSD core for the lookup
*		operation.
*
* @return	XOsd_Config is a reference to a config record in the
*		configuration table (in xosd_g.c) corresponding to
*		<i>DeviceId</i> or NULL if no match is found.
*
* @note		None.
*
******************************************************************************/
XOsd_Config *XOsd_LookupConfig(u16 DeviceId)
{
	extern XOsd_Config XOsd_ConfigTable[XPAR_XOSD_NUM_INSTANCES];
	XOsd_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = (u32)0x0; Index < (u32)(XPAR_XOSD_NUM_INSTANCES);
								Index++) {
		if (XOsd_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XOsd_ConfigTable[Index];
			break;
		}
	}

	return (XOsd_Config *)CfgPtr;
}
/** @} */
