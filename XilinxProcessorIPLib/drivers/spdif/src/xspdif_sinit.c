/*******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xspdif_sinit.c
 * @addtogroup spdif_v1_1
 * @{
 *
 * <pre>
 *
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date      Changes
 * ----- ------ -------- --------------------------------------------------
 * 1.0   kar    01/25/18  Initial release.
 * </pre>
 *
 ******************************************************************************/


/***************************** Include Files *********************************/
#include "xspdif.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
extern XSpdif_Config XSpdif_ConfigTable[];

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 *
 * This function returns a reference to an XSpdif_Config structure
 * based on the core id, <i>DeviceId</i>. The return value will refer to an
 * entry in the device configuration table defined in the xspdif_g.c file.
 *
 * @param  DeviceId is the unique core ID of the XSpdif core for the
 *         lookup operation.
 *
 * @return returns a reference to a config record in the configuration table
 *         corresponding to <i>DeviceId</i>, or NULL if no match is found.
 *
 * @note   None.
 *
 ******************************************************************************/
XSpdif_Config *XSpdif_LookupConfig(u16 DeviceId)
{
	XSpdif_Config *CfgPtr = NULL;
	u32 Index;

	/* Checking for device id for which instance it is matching */
	for (Index = 0; Index < (u32)(XPAR_XSPDIF_NUM_INSTANCES);
			Index++) {

		/* Assigning address of config table if both device ids
		 * are matched
		 */
		if (XSpdif_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XSpdif_ConfigTable[Index];
			break;
		}
	}
	return CfgPtr;
}

/*****************************************************************************/
/**
 *
 * Initializes a specific Xspdif instance such that the driver is ready to use.
 *
 * @param InstancePtr is a pointer to the XSpdif instance to be worked on.
 * @param DeviceId is the unique id of the device controlled by this XSpdif
 *        instance. Passing in a device id associates the generic XSpdif
 *        instance to a specific device, as chosen by the caller or
 *        application developer.
 *
 * @return
 *   - XST_SUCCESS if successful.
 *   - XST_DEVICE_NOT_FOUND if the device was not found in the configuration
 *     such that initialization could not be accomplished.
 *   - XST_FAILURE if version mismatched
 *
 ******************************************************************************/
int XSpdif_Initialize(XSpdif *InstancePtr, u16 DeviceId)
{
	XSpdif_Config *ConfigPtr; /* Pointer to Configuration ROM data */

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/*
	 * Lookup the device configuration in the temporary CROM table. Use this
	 * configuration info down below when initializing this component.
	 */
	ConfigPtr = XSpdif_LookupConfig(DeviceId);
	if (ConfigPtr == NULL)
		return XST_DEVICE_NOT_FOUND;

	return XSpdif_CfgInitialize(InstancePtr, ConfigPtr,
			ConfigPtr->BaseAddress);
}

/** @} */

