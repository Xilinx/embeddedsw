/**************************************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xtrngpsv_sinit.c
 * @addtogroup xtrngpsv_v1_0
 * @{
 *
 * This file contains static initialization method for Xilinx TRNG driver
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.00  ssc  09/05/21 First release
 *
 * </pre>
 *
 **************************************************************************************************/

/***************************** Include Files *****************************************************/

#include "xtrngpsv.h"
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
 * This function returns a reference to an XTrng_Config structure based on the DeviceId. The return
 * value will refer to an entry in the device configuration table defined in the xtrng_g.c file.
 *
 * @param	DeviceId is the unique device ID of the device being lookedup.
 *
 * @return	XTrng_LookupConfig returns a reference to a config record in the configuration
 * 		table (in xtrngpsv.c) corresponding to DeviceId, or NULL if no match is found.
 *
 **************************************************************************************************/
XTrngpsv_Config *XTrngpsv_LookupConfig(u16 DeviceId)
{
	XTrngpsv_Config *CfgPtr = NULL;
	u32 Index;

	/* Checking for device id for which instance it is matching */
	for (Index = 0U; Index < (u32)(XPAR_XTRNGPSV_NUM_INSTANCES); Index++) {

		/* Assigning address of config table if both device ids are matched */
		if (XTrngpsv_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XTrngpsv_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
/** @} */
