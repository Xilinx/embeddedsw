/*******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xi2srx_sinit.c
 * @addtogroup i2srx_v2_1
 * @{
 *
 * <pre>
 *
 * MODIFICATION HISTORY:
 *
 * Ver    Who    Date      Changes
 * ----- ------ -------- --------------------------------------------------
 * 1.0    kar   01/25/18   Initial release.
 * </pre>
 *
 *****************************************************************************/

/***************************** Include Files *********************************/
#include "xi2srx.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
extern XI2srx_Config XI2srx_ConfigTable[];

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 *
 * This function returns a reference to an XI2srx_Config structure
 * based on the core id, <i>DeviceId</i>. The return value will refer to an
 * entry in the device configuration table defined in the
 * xi2srx_g.c file.
 *
 * @param  DeviceId is the unique core ID of the XI2s Receiver core for the
 *         lookup operation.
 *
 * @return returns a reference to a config record in the configuration table
 *         corresponding to <i>DeviceId</i>, or NULL if no match is found.
 *
 * @note   None.
 *
 *****************************************************************************/
XI2srx_Config *XI2s_Rx_LookupConfig(u16 DeviceId)
{
	XI2srx_Config *CfgPtr = NULL;
	u32 Index;

	/* Checking for device id for which instance it is matching */
	for (Index = 0; Index < (u32)(XPAR_XI2SRX_NUM_INSTANCES);
			Index++) {

		/* Assigning address of config table if both device ids
		 * are matched
		 */
		if (XI2srx_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XI2srx_ConfigTable[Index];
			break;
		}
	}
	return CfgPtr;
}
/*****************************************************************************/
/**
 *
 * Initializes a specific XI2s_Rx instance such that the driver is ready to use.
 *
 * @param InstancePtr is a pointer to the XI2s_Rx instance to be worked on.
 * @param DeviceId is the unique id of the device controlled by this XI2s_Rx
 *        instance. Passing in a device id associates the generic XI2s_Rx
 *        instance to a specific device, as chosen by the caller or
 *        application developer.
 *
 * @return
 *   - XST_SUCCESS if successful.
 *   - XST_DEVICE_NOT_FOUND if the device was not found in the configuration
 *     such that initialization could not be accomplished.
 *   - XST_INVALID_VERSION if version mismatched
 *
 *****************************************************************************/
int XI2s_Rx_Initialize(XI2s_Rx *InstancePtr, u16 DeviceId)
{
	XI2srx_Config *ConfigPtr; /* Pointer to Configuration ROM data */

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	/*
	 * Lookup the device configuration in the temporary CROM table. Use this
	 * configuration info down below when initializing this component.
	 */
	ConfigPtr = XI2s_Rx_LookupConfig(DeviceId);
	if (ConfigPtr == NULL)
		return XST_DEVICE_NOT_FOUND;

	return XI2s_Rx_CfgInitialize(InstancePtr, ConfigPtr,
			ConfigPtr->BaseAddress);
}
/** @} */
