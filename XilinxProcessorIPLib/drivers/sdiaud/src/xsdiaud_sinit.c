/*******************************************************************************
 *
 * Copyright (C) 2017 - 2018 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
 ******************************************************************************/
/******************************************************************************/
/**
 *
 * @file xsdiaud_sinit.c
 * @addtogroup sdiaud_v1_0
 * @{
 *
 * <pre>
 *
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date      Changes
 * ----- ------ -------- --------------------------------------------------
 * 1.0   kar    02/14/18  Initial release.
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
