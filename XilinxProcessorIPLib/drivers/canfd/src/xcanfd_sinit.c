/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcanFd_sinit.c
* @addtogroup canfd_v2_3
* @{
*
* This file contains the implementation of the XCanFd driver's static
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
* 1.0   nsk    06/03/15 First release
* 1.3   ask    08/08/18 Fixed doxygen warnings
* 2.0	ask    09/12/18 Added support for canfd 2.0 spec sequential mode
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xcanfd.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
extern XCanFd_Config XCanFd_ConfigTable[];

/*****************************************************************************/
/**
*
* This function looks for the device configuration based on the unique device
* ID. The table XCanFd_ConfigTable[] contains the configuration information for
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
XCanFd_Config *XCanFd_LookupConfig(u16 DeviceId)
{
	XCanFd_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0; Index < XPAR_XCANFD_NUM_INSTANCES; Index++) {
		if (XCanFd_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XCanFd_ConfigTable[Index];
			break;
		}
	}

	return (XCanFd_Config *)CfgPtr;
}
/** @} */
