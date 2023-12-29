/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_sdirxss_coreinit.c
* @addtogroup v_sdirxss Overview
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
#ifndef SDT
		ConfigPtr = XV_SdiRx_LookupConfig(SdiRxSsPtr->Config.SdiRx.DeviceId);
#else
		ConfigPtr = XV_SdiRx_LookupConfig(SdiRxSsPtr->Config.SdiRx.AbsAddr);
#endif
		if (ConfigPtr == NULL) {
			xdbg_printf(XDBG_DEBUG_GENERAL,
				"SDIRXSS ERR:: SDI RX device not found\r\n");
			return XST_FAILURE;
		}

#ifdef SDT
		SdiRxSsPtr->Config.SdiRx.AbsAddr += SdiRxSsPtr->Config.BaseAddress;
		ConfigPtr->BaseAddress += SdiRxSsPtr->Config.BaseAddress;
#endif

		/* Initialize core */
		Status = XV_SdiRx_CfgInitialize(SdiRxSsPtr->SdiRxPtr,
		ConfigPtr,
		SdiRxSsPtr->Config.SdiRx.AbsAddr);

		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_GENERAL,
				"SDIRXSS ERR:: SDI RX Initialization failed\r\n");
			return XST_FAILURE;
		}

		/* Set bit depth into sdirx core*/
		XV_SdiRx_SetBitDepth(SdiRxSsPtr->SdiRxPtr,
				SdiRxSsPtr->Config.bitdepth);
	}

	return XST_SUCCESS;
}

/** @} */
