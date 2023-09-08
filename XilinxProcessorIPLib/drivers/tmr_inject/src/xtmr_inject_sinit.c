/******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xtmr_inject_sinit.c
* @addtogroup Overview
* @{
*
* The implementation of the XTMRInject component's static initialzation
* functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.0   sa   04/05/17 First release
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xstatus.h"
#ifndef SDT
#include "xparameters.h"
#endif
#include "xtmr_inject_i.h"

/************************** Constant Definitions ****************************/


/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/


/************************** Variable Definitions ****************************/


/************************** Function Prototypes *****************************/

/****************************************************************************
*
* Looks up the device configuration based on the unique device ID.  The table
* TmrInjectConfigTable contains the configuration info for each device in the
* system.
*
* @param	DeviceId is the unique device ID to match on.
*
* @return	A pointer to the configuration data for the device, or
*		NULL if no match was found.
*
* @note		None.
*
******************************************************************************/
#ifndef SDT
XTMR_Inject_Config *XTMR_Inject_LookupConfig(u16 DeviceId)
{
	XTMR_Inject_Config *CfgPtr = NULL;
	u32 Index;

	for (Index=0; Index < XPAR_XTMR_INJECT_NUM_INSTANCES; Index++) {
		if (XTMR_Inject_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XTMR_Inject_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#else
XTMR_Inject_Config *XTMR_Inject_LookupConfig(UINTPTR BaseAddr)
{
	XTMR_Inject_Config *CfgPtr = NULL;
	u32 Index;

	for (Index=0; XTMR_Inject_ConfigTable[Index].Name != NULL; Index++) {
		if (XTMR_Inject_ConfigTable[Index].RegBaseAddr == BaseAddr) {
			CfgPtr = &XTMR_Inject_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#endif

/****************************************************************************/
/**
*
* Initialize a XTMR_Inject instance.
*
* @param	InstancePtr is a pointer to the XTMR_Inject instance.
* @param	DeviceId is the unique id of the device controlled by this
*		XTMR_Inject instance.  Passing in a device id associates the
*		generic XTMR_Inject instance to a specific device, as chosen by
*		the caller or application developer.
*
* @return
* 		- XST_SUCCESS if everything starts up as expected.
* 		- XST_DEVICE_NOT_FOUND if the device is not found in the
*			configuration table.
*
* @note		None.
*
*****************************************************************************/
#ifndef SDT
int XTMR_Inject_Initialize(XTMR_Inject *InstancePtr, u16 DeviceId)
#else
int XTMR_Inject_Initialize(XTMR_Inject *InstancePtr, UINTPTR BaseAddr)
#endif
{
	XTMR_Inject_Config *ConfigPtr;

	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/*
	 * Lookup the device configuration in the configuration table. Use this
	 * configuration info when initializing this component.
	 */
#ifndef SDT
	ConfigPtr = XTMR_Inject_LookupConfig(DeviceId);
#else
	ConfigPtr = XTMR_Inject_LookupConfig(BaseAddr);
#endif

	if (ConfigPtr == (XTMR_Inject_Config *)NULL) {
		return XST_DEVICE_NOT_FOUND;
	}
	return XTMR_Inject_CfgInitialize(InstancePtr, ConfigPtr,
					ConfigPtr->RegBaseAddr);
}

/** @} */
