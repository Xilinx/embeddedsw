/******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xusbpsu_sinit.c
* @addtogroup usbpsu USBPSU v1.12
* @{
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   sg   06/06/16 First release
* 1.9   pm   03/15/21 Fixed doxygen warnings
* 1.12   pm   08/10/22 Update doxygen tag and addtogroup version
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xusbpsu.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
* @brief
* Lookup the device configuration based on the unique device ID.  The table
* contains the configuration info for each device in the system.
*
* @param DeviceId is the unique device ID of the device being looked up.
*
* @return
* A pointer to the configuration table entry corresponding to the given
* device ID, or NULL if no match is found.
*
* @note none
*
******************************************************************************/
XUsbPsu_Config *XUsbPsu_LookupConfig(u16 DeviceId)
{
	XUsbPsu_Config *CfgPtr = NULL;
	u32 i;

	for (i = 0U; i < (u32)XPAR_XUSBPSU_NUM_INSTANCES; i++) {
			if (XUsbPsu_ConfigTable[i].DeviceId == DeviceId) {
			CfgPtr = &XUsbPsu_ConfigTable[i];
			break;
		}
	}

	return (XUsbPsu_Config *)(CfgPtr);
}
/** @} */
