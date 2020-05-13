/*******************************************************************************
* Copyright (c) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xsdiaud_sinit.c
 * @addtogroup sdiaud_v2_1
 * @{
 *
 * <pre>
 *
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date      Changes
 * ----- ------ -------- --------------------------------------------------
 * 1.0   kar    02/14/18  Initial release.
 * 2.0   vve    09/27/18  Add 32 channel support
 *                        Add support for channel status extraction logic both
 *                        on embed and extract side.
 *                        Add APIs to detect group change, sample rate change,
 *                        active channel change
 * </pre>
 *
 ******************************************************************************/
/***************************** Include Files *********************************/
#include "xsdiaud.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
extern XSdiAud_Config XSdiAud_ConfigTable[];

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 *
 * This function returns a reference to an XSdiAud_Config structure
 * based on the core id, <i>DeviceId</i>. The return value will refer to an
 * entry in the device configuration table defined in the xsdiaud_g.c file.
 *
 * @param  DeviceId is the unique core ID of the XSdiAud core for the
 *         lookup operation.
 *
 * @return returns a reference to a config record in the configuration table
 *         corresponding to <i>DeviceId</i>, or NULL if no match is found.
 *
 * @note   None.
 *
 ******************************************************************************/
XSdiAud_Config *XSdiAud_LookupConfig(u16 DeviceId)
{
	XSdiAud_Config *CfgPtr = NULL;
	u32 Index;

	/* Checking for device id for which instance it is matching */
	for (Index = 0; Index < (u32)(XPAR_XSDIAUD_NUM_INSTANCES);
			Index++) {

		/* Assigning address of config table if both device ids
		 * are matched
		 */
		if (XSdiAud_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XSdiAud_ConfigTable[Index];
			break;
		}
	}
	return CfgPtr;
}

/*****************************************************************************/
/**
 *
 * Initializes a specific XsdiAud instance such that the driver is ready to use.
 *
 * @param InstancePtr is a pointer to the XSdiAud instance to be worked on.
 * @param DeviceId is the unique id of the device controlled by this XSdiAud
 *        instance. Passing in a device id associates the generic XSdiAud
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
int XSdiAud_Initialize(XSdiAud *InstancePtr, u16 DeviceId)
{
	XSdiAud_Config *ConfigPtr; /* Pointer to Configuration ROM data */

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/*
	 * Lookup the device configuration in the temporary CROM table. Use this
	 * configuration info down below when initializing this component.
	 */
	ConfigPtr = XSdiAud_LookupConfig(DeviceId);
	if (ConfigPtr == NULL)
		return XST_DEVICE_NOT_FOUND;

	return XSdiAud_CfgInitialize(InstancePtr, ConfigPtr,
			ConfigPtr->BaseAddress);
}
/** @} */
