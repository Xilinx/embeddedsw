/**************************************************************************************************
* Copyright (C) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xtrngpsv_sinit.c
 * @addtogroup Overview
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
 * 1.01  ng   06/30/23 Added support for system device-tree flow
 *
 * </pre>
 *
 **************************************************************************************************/

/***************************** Include Files *****************************************************/

#include "xtrngpsv.h"
#ifndef SDT
#include "xparameters.h"
#endif

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
#ifndef SDT
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
#else
XTrngpsv_Config *XTrngpsv_LookupConfig(UINTPTR BaseAddress)
{
	XTrngpsv_Config *CfgPtr = NULL;
	u32 Index;

	/* Checks all the instances */
	for (Index = 0U; XTrngpsv_ConfigTable[Index].Name != NULL; Index++) {
		if ((XTrngpsv_ConfigTable[Index].BaseAddress == BaseAddress) ||
			 !BaseAddress) {
			CfgPtr = &XTrngpsv_ConfigTable[Index];
			break;
		}
	}

	return (XTrngpsv_Config *)CfgPtr;
}
#endif
/** @} */
