/******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xtmr_manager_sinit.c
* @addtogroup tmr_manager_v1_3
* @{
*
* The implementation of the XTMR_Manager component's static initialzation
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
#include "xparameters.h"
#include "xtmr_manager_i.h"

/************************** Constant Definitions ****************************/


/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/


/************************** Variable Definitions ****************************/


/************************** Function Prototypes *****************************/

/****************************************************************************
*
* Looks up the device configuration based on the unique device ID.  The table
* TMRManagerConfigTable contains the configuration info for each device in
* the system.
*
* @param	DeviceId is the unique device ID to match on.
*
* @return	A pointer to the configuration data for the device, or
*		NULL if no match was found.
*
* @note		None.
*
******************************************************************************/
XTMR_Manager_Config *XTMR_Manager_LookupConfig(u16 DeviceId)
{
	XTMR_Manager_Config *CfgPtr = NULL;
	u32 Index;

	for (Index=0; Index < XPAR_XTMR_MANAGER_NUM_INSTANCES; Index++) {
		if (XTMR_Manager_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XTMR_Manager_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}

/****************************************************************************/
/**
*
* Initialize a XTMR_Manager instance.  The receive and transmit FIFOs of the
* core are not flushed, so the user may want to flush them. The hardware
* device does not have any way to disable the receiver such that any valid
* data may be present in the receive FIFO. This function disables the core
* interrupt. The baudrate and format of the data are fixed in the hardware
* at hardware build time.
*
* @param	InstancePtr is a pointer to the XTMR_Manager instance.
* @param	DeviceId is the unique id of the device controlled by this
*		XTMR_Manager instance.  Passing in a device id associates the
*		generic XTMR_Manager instance to a specific device, as chosen by
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
int XTMR_Manager_Initialize(XTMR_Manager *InstancePtr, u16 DeviceId)
{
	XTMR_Manager_Config *ConfigPtr;

	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/*
	 * Lookup the device configuration in the configuration table. Use this
	 * configuration info when initializing this component.
	 */
	ConfigPtr = XTMR_Manager_LookupConfig(DeviceId);

	if (ConfigPtr == (XTMR_Manager_Config *)NULL) {
		return XST_DEVICE_NOT_FOUND;
	}
	return XTMR_Manager_CfgInitialize(InstancePtr, ConfigPtr,
					  ConfigPtr->RegBaseAddr);
}

/** @} */
