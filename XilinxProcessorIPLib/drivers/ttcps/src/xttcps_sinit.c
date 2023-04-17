/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xttcps_sinit.c
* @addtogroup ttcps Overview
* @{
*
* The implementation of the XTtcPs driver's static initialization functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- ---------------------------------------------
* 1.00a drg/jz 01/21/10 First release
* 3.00  kvn    02/13/15 Modified code for MISRA-C:2012 compliance.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xttcps.h"
#ifndef SDT
#include "xparameters.h"
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
#ifndef SDT
extern XTtcPs_Config XTtcPs_ConfigTable[XPAR_XTTCPS_NUM_INSTANCES];
#else
extern XTtcPs_Config XTtcPs_ConfigTable[];
#endif

/*****************************************************************************/
/**
*
* Looks up the device configuration based on the unique device ID. A table
* contains the configuration info for each device in the system.
*
* @param	DeviceId contains the unique ID of the device
*
* @return
*
* A pointer to the configuration found or NULL if the specified device ID was
* not found. See xttcps.h for the definition of XTtcPs_Config.
*
* @note		None.
*
******************************************************************************/
#ifndef SDT
XTtcPs_Config *XTtcPs_LookupConfig(u16 DeviceId)
{
	XTtcPs_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; Index < (u32)XPAR_XTTCPS_NUM_INSTANCES; Index++) {
		if (XTtcPs_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XTtcPs_ConfigTable[Index];
			break;
		}
	}

	return (XTtcPs_Config *)CfgPtr;
}
#else
XTtcPs_Config *XTtcPs_LookupConfig(u32 BaseAddress)
{
	XTtcPs_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; XTtcPs_ConfigTable[Index].Name != NULL; Index++) {
		if ((XTtcPs_ConfigTable[Index].BaseAddress == BaseAddress) ||
		     !BaseAddress) {
			CfgPtr = &XTtcPs_ConfigTable[Index];
			break;
		}
	}

	return (XTtcPs_Config *)CfgPtr;
}
#endif
/** @} */
