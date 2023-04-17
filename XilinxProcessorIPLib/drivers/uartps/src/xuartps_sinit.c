/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xuartps_sinit.c
* @addtogroup uartps Overview
* @{
*
* The implementation of the XUartPs driver's static initialization
* functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date	Changes
* ----- ------ -------- -----------------------------------------------
* 1.00  drg/jz 01/13/10 First Release
* 3.00  kvn    02/13/15 Modified code for MISRA-C:2012 compliance.
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xstatus.h"
#ifndef SDT
#include "xparameters.h"
#endif
#include "xuartps.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/
#ifndef SDT
extern XUartPs_Config XUartPs_ConfigTable[XPAR_XUARTPS_NUM_INSTANCES];
#else
extern XUartPs_Config XUartPs_ConfigTable[];
#endif


/************************** Function Prototypes *****************************/

/****************************************************************************/
/**
*
* Looks up the device configuration based on the unique device ID. The table
* contains the configuration info for each device in the system.
*
* @param	DeviceId contains the ID of the device
*
* @return	A pointer to the configuration structure or NULL if the
*		specified device is not in the system.
*
* @note		None.
*
******************************************************************************/
#ifndef SDT
XUartPs_Config *XUartPs_LookupConfig(u16 DeviceId)
{
	XUartPs_Config *CfgPtr = NULL;

	u32 Index;

	for (Index = 0U; Index < (u32)XPAR_XUARTPS_NUM_INSTANCES; Index++) {
		if (XUartPs_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XUartPs_ConfigTable[Index];
			break;
		}
	}

	return (XUartPs_Config *)CfgPtr;
}
#else
XUartPs_Config *XUartPs_LookupConfig(u32 BaseAddress)
{
	XUartPs_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; XUartPs_ConfigTable[Index].Name != NULL; Index++) {
		if ((XUartPs_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XUartPs_ConfigTable[Index];
			break;
		}
	}

	return (XUartPs_Config *)CfgPtr;
}
#endif
/** @} */
