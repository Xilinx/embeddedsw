/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdptxss_sinit.c
* @addtogroup dptxss Overview
* @{
*
* This file contains static initialization method for Xilinx DisplayPort
* Transmitter Subsystem core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- --------------------------------------------------
* 1.00 sha 01/29/15 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdptxss.h"
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
* This function returns a reference to an XDptxss_Config structure based
* on the core id, <i>DeviceId</i>. The return value will refer to an entry in
* the device configuration table defined in the xdptxss_g.c file.
*
* @param	DeviceId is the unique core ID of the XDpTxSs core for
*		the lookup operation.
*
* @return	XDptxss_LookupConfig returns a reference to a config record
*		in the configuration table (in xdptxss_g.c) corresponding
*		to <i>DeviceId</i>, or NULL if no match is found.
*
* @note		None.
*
******************************************************************************/
#ifndef SDT
XDpTxSs_Config *XDpTxSs_LookupConfig(u16 DeviceId)
{
	extern XDpTxSs_Config XDpTxSs_ConfigTable[XPAR_XDPTXSS_NUM_INSTANCES];
	XDpTxSs_Config *CfgPtr = NULL;
	u32 Index;

	/* Checking for device id for which instance it is matching */
	for (Index = (u32)0x0; Index < (u32)XPAR_XDPTXSS_NUM_INSTANCES;
								Index++) {
		/* Assigning address of config table if both device ids
		 * are matched
		 */
		if (XDpTxSs_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XDpTxSs_ConfigTable[Index];
			break;
		}
	}

	return (XDpTxSs_Config *)CfgPtr;
}
#else
XDpTxSs_Config *XDpTxSs_LookupConfig(UINTPTR BaseAddress)
{
	extern XDpTxSs_Config XDpTxSs_ConfigTable[XPAR_XDPTXSS_NUM_INSTANCES];
	XDpTxSs_Config *CfgPtr = NULL;
	u32 Index;

	/* Checking for device id for which instance it is matching */
	for (Index = (u32)0x0; XDpTxSs_ConfigTable[Index].Name != NULL;
								Index++) {
		/* Assigning address of config table if both device ids
		 * are matched
		 */
		if ((XDpTxSs_ConfigTable[Index].BaseAddress == BaseAddress) ||
           (!BaseAddress)){
			CfgPtr = &XDpTxSs_ConfigTable[Index];
			break;
		}
	}

	return (XDpTxSs_Config *)CfgPtr;
}
#endif
#ifdef SDT
/*****************************************************************************/
/**
* This function returns the Index number of config table using BaseAddress.
*
* @param  Base address of the instance
*
* @return Index number of the config table
*
********************************************************************************/

u32 XDpTxSs_GetDrvIndex(UINTPTR BaseAddress)
{
	extern XDpTxSs_Config XDpTxSs_ConfigTable[XPAR_XDPTXSS_NUM_INSTANCES];
	u32 Index = 0;

 for (Index = 0U; XDpTxSs_ConfigTable[Index].Name != NULL; Index++) {
   if ((XDpTxSs_ConfigTable[Index].BaseAddress) == BaseAddress) {
	break;
   }
 }
 return Index;
}
#endif
/** @} */
