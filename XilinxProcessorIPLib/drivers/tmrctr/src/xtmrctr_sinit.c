/******************************************************************************
* Copyright (C) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xtmrctr_sinit.c
* @addtogroup Overview
* @{
*
* This file contains static initialization methods for the XTmrCtr driver.
*
* @note	None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 4.0   als  09/30/15 Creation of this file. Moved LookupConfig from xtmrctr.c.
* 4.4	mn   07/31/17 Resolve Compilation warning
* 4.13  ml   09/19/24 Fix compilation warning by removing unused variable.
* </pre>
*
******************************************************************************/

/******************************* Include Files *******************************/

#include "xtmrctr.h"
#ifndef SDT
#include "xparameters.h"
#endif

/************************** Constant Definitions *****************************/

#ifndef XPAR_XTMRCTR_NUM_INSTANCES
#define XPAR_XTMRCTR_NUM_INSTANCES	0
#endif

/*************************** Variable Declarations ***************************/

/* A table of configuration structures containing the configuration information
 * for each timer core in the system. */
#ifndef SDT
extern XTmrCtr_Config XTmrCtr_ConfigTable[XPAR_XTMRCTR_NUM_INSTANCES];
#else
extern XTmrCtr_Config XTmrCtr_ConfigTable[];
#endif

/**************************** Function Definitions ***************************/

/*****************************************************************************/
/**
* Looks up the device configuration based on the unique device ID. The table
* TmrCtr_ConfigTable contains the configuration info for each device in the
* system.
*
* @param	DeviceId is the unique device ID to search for in the config
*		table.
*
* @return	A pointer to the configuration that matches the given device
* 		ID, or NULL if no match is found.
*
* @note		None.
*
******************************************************************************/
#ifndef SDT
XTmrCtr_Config *XTmrCtr_LookupConfig(u16 DeviceId)
{
	XTmrCtr_Config *CfgPtr = NULL;
	u16 Index;

	for (Index = 0; Index < XPAR_XTMRCTR_NUM_INSTANCES; Index++) {
		if (XTmrCtr_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XTmrCtr_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#else
XTmrCtr_Config *XTmrCtr_LookupConfig(UINTPTR BaseAddress)
{
	XTmrCtr_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0; XTmrCtr_ConfigTable[Index].Name != NULL; Index++) {
		/*
		 * If BaseAddress is 0, return Configuration for 0th instance of
		 * AXI timer device.
		 * As AXI timer instance base address varies based on designs,
		 * driver examples can pass base address as 0 , to use available
		 * instance of AXI timer.
		 */
		if ((XTmrCtr_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress)  {
			CfgPtr = &XTmrCtr_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#endif
/** @} */
