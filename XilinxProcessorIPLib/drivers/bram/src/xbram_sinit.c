/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xbram_sinit.c
* @addtogroup bram_v4_6
* @{
*
* The implementation of the XBram driver's static initialization
* functionality.
*
* @note
*
* None
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 2.01a jvb  10/13/05 First release
* 2.11a mta  03/21/07 Updated to new coding style
* 4.2   ms   08/07/17 Fixed compilation warnings.
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xstatus.h"
#include "xparameters.h"
#include "xbram.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/
extern XBram_Config XBram_ConfigTable[];

/************************** Function Prototypes *****************************/


/*****************************************************************************/
/**
* Lookup the device configuration based on the unique device ID.  The table
* ConfigTable contains the configuration info for each device in the system.
*
* @param	DeviceId is the device identifier to lookup.
*
* @return
*		 - A pointer of data type XBram_Config which
*		points to the device configuration if DeviceID is found.
* 		- NULL if DeviceID is not found.
*
* @note		None.
*
******************************************************************************/
XBram_Config *XBram_LookupConfig(u16 DeviceId)
{
	XBram_Config *CfgPtr = NULL;

	u32 Index;

	for (Index = 0U; Index < XPAR_XBRAM_NUM_INSTANCES; Index++) {
		if (XBram_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XBram_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
/** @} */
