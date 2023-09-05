/**************************************************************************************************
* Copyright (c) 2023 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xtrngpsx_sinit.c
 * @addtogroup Overview
 * @{
 *
 * This file contains static initialization method for Xilinx versalnet TRNG driver
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.00  kpt  01/04/23 First release
 * 1.01  ng   09/04/23 Added SDT support
 *
 * </pre>
 *
 **************************************************************************************************/

/***************************** Include Files *****************************************************/

#include "xtrngpsx.h"
#include "xparameters.h"

/************************************ Constant Definitions ***************************************/

/************************** Macros (Inline Functions) Definitions ********************************/

/************************************** Type Definitions *****************************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/************************************ Function Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief
 * This function returns a reference to an XTrngpsx_Config structure based on the DeviceId. The return
 * value will refer to an entry in the device configuration table defined in the xtrngpsx_g.c file.
 *
 * @param	DeviceId is the unique device ID of the device being lookedup.
 *
 * @return	XTrngpsx_LookupConfig returns a reference to a config record in the configuration
 * 		table (in xtrngpsx.c) corresponding to DeviceId, or NULL if no match is found.
 *
 **************************************************************************************************/
#ifndef SDT
XTrngpsx_Config *XTrngpsx_LookupConfig(u16 DeviceId)
{
	XTrngpsx_Config *CfgPtr = NULL;
	u32 Index;

	/* Checking for device id for which instance it is matching */
	for (Index = 0U; Index < (u32)(XPAR_XTRNGPSX_NUM_INSTANCES); Index++) {

		/* Assigning address of config table if both device ids are matched */
		if (XTrngpsx_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XTrngpsx_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#else
XTrngpsx_Config *XTrngpsx_LookupConfig(UINTPTR BaseAddress)
{
	XTrngpsx_Config *CfgPtr = NULL;
	u32 Index;

	/* Checks all the instances */
	for (Index = 0U; XTrngpsx_ConfigTable[Index].Name != NULL; Index++) {
		if ((XTrngpsx_ConfigTable[Index].BaseAddress == BaseAddress) || !BaseAddress) {
			CfgPtr = &XTrngpsx_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#endif
/** @} */
