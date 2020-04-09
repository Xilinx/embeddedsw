/******************************************************************************
* Copyright (C) 2011 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************
**
* @file xaxipcie_sinit.c
* @addtogroup axipcie_v3_2
* @{
*
* This file contains the implementation of AXI PCIe driver's static
* initialization functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 1.00a rkv  03/03/11  Original code.
*
* </pre>
*
******************************************************************************/

/****************************** Include Files ********************************/

#include "xparameters.h"
#include "xaxipcie.h"

/*************************** Constant Definitions ****************************/

/***************************** Type Definitions ******************************/

/****************** Macros (Inline Functions) Definitions ********************/

/*************************** Variable Definitions ****************************/

extern XAxiPcie_Config XAxiPcie_ConfigTable[];

/*************************** Function Prototypes *****************************/

/*****************************************************************************/
/**
* Lookup the device configuration based on the unique device ID.  The table
* ConfigTable contains the configuration info for each device in the system.
*
* @param 	DeviceId is the device identifier to lookup.
*
* @return
* 		- XAxiPcie configuration structure pointer if DeviceID is
*		found.
* 		- NULL if DeviceID is not found.
*
* @note		None
*
******************************************************************************/
XAxiPcie_Config *XAxiPcie_LookupConfig(u16 DeviceId)
{
	XAxiPcie_Config *CfgPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XAXIPCIE_NUM_INSTANCES; Index++) {
		if (XAxiPcie_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XAxiPcie_ConfigTable[Index];
			break;
		}
	}

	return (CfgPtr);
}





/** @} */
