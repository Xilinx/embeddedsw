/******************************************************************************
* Copyright (C) 2008 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xtft_sinit.c
* @addtogroup tft_v6_3
* @{
*
* This file defines the implementation of Tft device static initialization
* functionality.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver    Who   Date      Changes
* -----  ----  --------  -----------------------------------------------
* 1.00a  sg    03/24/08  First release
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/
#include "xparameters.h"
#include "xtft.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes ****************************/

/************************** Variable Definitions ****************************/
extern XTft_Config XTft_ConfigTable[];

/************************** Function Definitions ****************************/
/****************************************************************************/
/**
*
* This function obtains the Configuration pointer of the device whose ID
* is being passed as the parameter to the function.
*
* @param	DeviceId is the unique number of the device.
*
* @return	Configuration pointer of the Device whose ID is given.
*
* @note		None.
*
*****************************************************************************/
XTft_Config *XTft_LookupConfig(u16 DeviceId)
{
	XTft_Config *CfgPtr = NULL;
	u32 Index;

	/*
	 * Based on the number of instances of tft defined, compare
	 * the given Device ID with the ID of each instance and get
	 * the configptr of the matched instance.
	 */
	for (Index=0; Index < XPAR_XTFT_NUM_INSTANCES; Index++) {
		if (XTft_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XTft_ConfigTable[Index];
		}
	}

	return CfgPtr;
}

/** @} */
