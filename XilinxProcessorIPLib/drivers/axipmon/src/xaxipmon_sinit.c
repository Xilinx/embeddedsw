/******************************************************************************
* Copyright (C) 2012 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xaxipmon_sinit.c
* @addtogroup axipmon_v6_9
* @{
*
* This file contains the implementation of the XAxiPmon driver's static
* initialization functionality.
*
* @note	None.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.00a bss  02/27/12 First release
* 2.00a bss  06/23/12 Updated to support v2_00a version of IP.
* 6.3   kvn  07/02/15 Modified code according to MISRA-C:2012 guidelines.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xaxipmon.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
extern XAxiPmon_Config XAxiPmon_ConfigTable[];

/*****************************************************************************/
/**
*
* This function looks up the device configuration based on the unique device ID.
* The table XAxiPmon_ConfigTable contains the configuration info for each device
* in the system.
*
* @param	DeviceId contains the ID of the device for which the
*		device configuration pointer is to be returned.
*
* @return
*		- A pointer to the configuration found.
*		- NULL if the specified device ID was not found.
*
* @note		None.
*
******************************************************************************/
XAxiPmon_Config *XAxiPmon_LookupConfig(u16 DeviceId)
{
	XAxiPmon_Config *CfgPtr = NULL;
	u32 Index;

	for (Index=0U; Index < (u32)XPAR_XAXIPMON_NUM_INSTANCES; Index++) {
		if (XAxiPmon_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XAxiPmon_ConfigTable[Index];
			break;
		}
	}

	return (XAxiPmon_Config *)CfgPtr;
}
/** @} */
