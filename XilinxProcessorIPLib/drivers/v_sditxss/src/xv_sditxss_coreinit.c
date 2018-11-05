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
* @file xv_sditxss_coreinit.c
* @addtogroup v_sditxss_v1_1
* @{
* @details

* SDI TX Subsystem Sub-Cores initialization
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
#include "xv_sditxss_coreinit.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/

/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  SdiTxSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XV_SdiTxSs_SubcoreInitSdiTx(XV_SdiTxSs *SdiTxSsPtr)
{
	int Status;
	/* UINTPTR AbsAddr; */
	XV_SdiTx_Config *ConfigPtr;

	if (SdiTxSsPtr->SdiTxPtr) {
		/* Get core configuration */
		XV_SdiTxSs_LogWrite(SdiTxSsPtr, XV_SDITXSS_LOG_EVT_SDITX_INIT, 0);
		ConfigPtr  = XV_SdiTx_LookupConfig(SdiTxSsPtr->Config.SdiTx.DeviceId);
		if (ConfigPtr == NULL) {
			xdbg_printf(XDBG_DEBUG_GENERAL,
				"SDITXSS ERR:: SDI TX device not found\r\n");
			return XST_FAILURE;
		}

		/* Initialize core */
		Status = XV_SdiTx_CfgInitialize(SdiTxSsPtr->SdiTxPtr,
		ConfigPtr,
		SdiTxSsPtr->Config.SdiTx.AbsAddr);

		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_GENERAL,
				"SDITXSS ERR:: SDI TX Initialization failed\r\n");
			return XST_FAILURE;
		}
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  SdiTxSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XV_SdiTxSs_SubcoreInitVtc(XV_SdiTxSs *SdiTxSsPtr)
{
	int Status;
	/* UINTPTR AbsAddr; */
	XVtc_Config *ConfigPtr;

	if (SdiTxSsPtr->VtcPtr) {
		/* Get core configuration */
		XV_SdiTxSs_LogWrite(SdiTxSsPtr, XV_SDITXSS_LOG_EVT_VTC_INIT, 0);
		ConfigPtr  = XVtc_LookupConfig(SdiTxSsPtr->Config.Vtc.DeviceId);
		if (ConfigPtr == NULL) {
			xdbg_printf(XDBG_DEBUG_GENERAL,
				"SDITXSS ERR:: VTC device not found\r\n");
			return XST_FAILURE;
		}

		/* Initialize core */
		Status = XVtc_CfgInitialize(SdiTxSsPtr->VtcPtr,
		ConfigPtr,
		SdiTxSsPtr->Config.Vtc.AbsAddr);

		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_GENERAL,
				"SDITXSS ERR:: VTC Initialization failed\r\n");
			return XST_FAILURE;
		}
	}
	return XST_SUCCESS;
}

/** @} */
