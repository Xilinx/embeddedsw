/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xscugic_sinit.c
* @addtogroup scugic_api SCUGIC APIs
* @{
*
* The xscugic_sinit.c contains static init functions for the XScuGic driver for the Interrupt
* Controller.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- --------------------------------------------------------
* 1.00a drg  01/19/10 First release
* 3.00  kvn  02/13/15 Modified code for MISRA-C:2012 compliance.
* 3.10  mus  07/17/18 Updated file to fix the various coding style issues
*                     reported by checkpatch. It fixes CR#1006344.
* 4.6	sk   08/05/21 Remove XScuGic_Config variable definition to fix
* 		      misrac violation.
* 5.2   ml   03/03/23 Add description to fix Doxygen warnings.
* 5.2   ml   09/07/23 Added comments to fix HIS COMF violations.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xscugic.h"
#ifndef SDT
#include "xparameters.h"
#endif

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/


/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
*
* Looks up the device configuration based on the unique device ID. A table
* contains the configuration info for each device in the system.
*
* @param	DeviceId Unique identifier for a device.
*
* @return	A pointer to the XScuGic configuration structure for the
*		specified device, or NULL if the device is not found.
*
*
******************************************************************************/
#ifndef SDT
XScuGic_Config *XScuGic_LookupConfig(u16 DeviceId)
{
	XScuGic_Config *CfgPtr = NULL;
	u32 Index;

	/*
	 * checks the device Id in configuration table
	 * whether it matches or not
	 */
	for (Index = 0U; Index < (u32)XPAR_SCUGIC_NUM_INSTANCES; Index++) {
		if (XScuGic_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XScuGic_ConfigTable[Index];
			break;
		}
	}
	/* Return statement */
	return (XScuGic_Config *)CfgPtr;
}
/*****************************************************************************/

/**
*
* Looks up the device configuration based on the BaseAddress. The return value
* will refer to an entry in the device configuration table defined in the
* xscugic_g.c file.
*
* @param        BaseAddress Base address of the device.
*
* @return       A pointer to the XScuGic configuration structure for the
*               specified device, or NULL if the device is not found.
*
*
******************************************************************************/
XScuGic_Config *XScuGic_LookupConfigBaseAddr(UINTPTR BaseAddress)
{
	XScuGic_Config *CfgPtr = NULL;
	u32 Index;

	/*
	 * checks the BaseAddress in configuration table
		 * whether it matches or not
		 */
	for (Index = 0U; Index < (u32)XPAR_SCUGIC_NUM_INSTANCES; Index++) {
		if (XScuGic_ConfigTable[Index].DistBaseAddress == BaseAddress) {
			CfgPtr = &XScuGic_ConfigTable[Index];
			break;
		}
	}
	/* Return statement */
	return (XScuGic_Config *)CfgPtr;
}
#else
XScuGic_Config *XScuGic_LookupConfig(UINTPTR BaseAddress)
{
	XScuGic_Config *CfgPtr = NULL;
	u32 Index;

	/* checks the BaseAddress in configuration table
	* whether it matches or not
	*/
	for (Index = 0U; XScuGic_ConfigTable[Index].Name != NULL; Index++) {
		if ((XScuGic_ConfigTable[Index].DistBaseAddress == BaseAddress) || !BaseAddress) {
			CfgPtr = &XScuGic_ConfigTable[Index];
			break;
		}
	}
	/* Return statement */
	return (XScuGic_Config *)CfgPtr;
}
#endif
/** @} */
