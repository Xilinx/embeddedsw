/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcanps_sinit.c
* @addtogroup canps Overview
* @{
*
* This file contains the implementation of the XCanPs driver's static
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
* 1.00a xd/sv  01/12/10 First release
* 3.00  kvn    02/13/15 Modified code for MISRA-C:2012 compliance.
* 3.5	sne    07/01/20 Fixed MISRAC warnings.
* 3.7	ht     06/28/23 Added support for system device-tree flow.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcanps.h"
#ifndef SDT
#include "xparameters.h"
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/


/*****************************************************************************/
/**
*
* This function looks for the device configuration based on the unique device
* ID. The table XCanPs_ConfigTable[] contains the configuration information for
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
#ifndef SDT
XCanPs_Config *XCanPs_LookupConfig(u16 DeviceId)
{
	XCanPs_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; Index < (u32)XPAR_XCANPS_NUM_INSTANCES; Index++) {
		if (XCanPs_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XCanPs_ConfigTable[Index];
			break;
		}
	}

	return (XCanPs_Config *)CfgPtr;
}
#else
XCanPs_Config *XCanPs_LookupConfig(u32 BaseAddress)
{
	XCanPs_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; XCanPs_ConfigTable[Index].Name != NULL; Index++) {
		if ((XCanPs_ConfigTable[Index].BaseAddr == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XCanPs_ConfigTable[Index];
			break;
		}
	}

	return (XCanPs_Config *)CfgPtr;
}

#endif
/** @} */
