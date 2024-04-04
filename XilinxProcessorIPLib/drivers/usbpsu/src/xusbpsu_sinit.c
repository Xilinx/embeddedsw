/******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xusbpsu_sinit.c
* @addtogroup usbpsu_api USBPSU APIs
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
* 1.14	pm    21/06/23 Added support for system device-tree flow.
* 1.15  np    26/03/24 Add doxygen and editorial fixes
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files *********************************/

#include "xusbpsu.h"
#ifndef SDT
#include "xparameters.h"
#endif

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
* @brief
* Looks up the device configuration based on the unique device ID. The table
* contains the configuration info for each device in the system.
*
* @param DeviceId Unique device ID of the device being looked up.
*
* @return
* A pointer to the configuration table entry corresponding to the given
* device ID, or NULL if no match is found.
*
*
*
******************************************************************************/
#ifndef SDT
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
#else
XUsbPsu_Config *XUsbPsu_LookupConfig(u32 BaseAddress)
{
	XUsbPsu_Config *CfgPtr = NULL;
	u32 i;

	for (i = 0U; XUsbPsu_ConfigTable[i].Name != NULL; i++) {
		if ((XUsbPsu_ConfigTable[i].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XUsbPsu_ConfigTable[i];
			break;
		}
	}

	return (XUsbPsu_Config *)(CfgPtr);
}
#endif
/** @} */
