/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc. All rights reserved.
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
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xv_sdirxss_coreinit.c
* @addtogroup v_sdirxss_v1_1
* @{
* @details

* SDI RX Subsystem Sub-Cores initialization
* The functions in this file provides an abstraction from the initialization
* sequence for included sub-cores. Subsystem is assigned an address and range
* on the axi-lite interface. This address space is condensed where-in each
* sub-core is at a fixed offset from the subsystem base address. For processor
* to be able to access the sub-core this offset needs to be transalted into a
* absolute address within the subsystems addressable range
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  jsr  07/17/17 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xv_sdirxss_coreinit.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/

/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param	SdiRxSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return	XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XV_SdiRxSs_SubcoreInitSdiRx(XV_SdiRxSs *SdiRxSsPtr)
{
	int Status;
	XV_SdiRx_Config *ConfigPtr;

	if (SdiRxSsPtr->SdiRxPtr) {
		/* Get core configuration */
		XV_SdiRxSs_LogWrite(SdiRxSsPtr, XV_SDIRXSS_LOG_EVT_SDIRX_INIT, 0);
		ConfigPtr = XV_SdiRx_LookupConfig(SdiRxSsPtr->Config.SdiRx.DeviceId);
		if (ConfigPtr == NULL) {
			xdbg_printf(XDBG_DEBUG_GENERAL,
				"SDIRXSS ERR:: SDI RX device not found\r\n");
			return XST_FAILURE;
		}

		/* Initialize core */
		Status = XV_SdiRx_CfgInitialize(SdiRxSsPtr->SdiRxPtr,
		ConfigPtr,
		SdiRxSsPtr->Config.SdiRx.AbsAddr);

		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_GENERAL,
				"SDIRXSS ERR:: SDI RX Initialization failed\r\n");
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}

/** @} */
