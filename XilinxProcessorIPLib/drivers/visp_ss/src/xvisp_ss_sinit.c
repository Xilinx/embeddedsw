/******************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xvisp_ss_sinit.c
* @addtogroup visp_ss_api VISP SS APIs
* @{
*
* This file contains static initialization methods for the XVisp_Ss driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ---------------------------------------------------
* 1.0   system  01/22/24 Initial release
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xvisp_ss.h"
#include "xstatus.h"
#include "xil_assert.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

extern xvisp_ss_Config xvisp_ss_ConfigTable[];

/*****************************************************************************/
/**
*
* This function looks for the device configuration based on the unique device
* ID. The table xvisp_ss_ConfigTable[] contains the configuration information
* for each instance of the device in the system.
*
* @param    DeviceId is the unique device ID of the device being looked up.
*
* @return   A pointer to the configuration table entry corresponding to the
*           given device ID, or NULL if no match is found.
*
* @note     None.
*
******************************************************************************/
#ifndef SDT
xvisp_ss_Config *XVisp_Ss_LookupConfig(u16 DeviceId)
{
	xvisp_ss_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0; Index < XPAR_XVISP_SS_NUM_INSTANCES; Index++) {
		if (xvisp_ss_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &xvisp_ss_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#else
xvisp_ss_Config *XVisp_Ss_LookupConfig(UINTPTR BaseAddress)
{
	xvisp_ss_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0; Index < XPAR_XVISP_SS_NUM_INSTANCES ; Index++) {
		if ((xvisp_ss_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &xvisp_ss_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#endif

/*****************************************************************************/
/**
*
* This function initializes a XVisp_Ss instance/driver. This function must be
* called prior to using the XVisp_Ss driver.
*
* @param    InstancePtr is a pointer to the XVisp_Ss instance.
* @param    DeviceId is the unique ID of the device controlled by this
*           XVisp_Ss instance. Passing in a device ID associates the
*           generic XVisp_Ss instance to a specific device, as chosen by
*           the caller or application developer.
*
* @return
*           - XST_SUCCESS if initialization was successful.
*           - XST_DEVICE_NOT_FOUND if the device configuration data was not
*             found for a device with the supplied device ID.
*
* @note     None.
*
******************************************************************************/
#ifndef SDT
int XVisp_Ss_Initialize(XVisp_Ss *InstancePtr, u16 DeviceId)
{
	xvisp_ss_Config *ConfigPtr;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Lookup configuration data in the table. */
	ConfigPtr = XVisp_Ss_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XVisp_Ss_CfgInitialize(InstancePtr, ConfigPtr,
				      ConfigPtr->BaseAddress);
}
#else
int XVisp_Ss_Initialize(XVisp_Ss *InstancePtr, UINTPTR BaseAddress)
{
	xvisp_ss_Config *ConfigPtr;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Lookup configuration data in the table. */
	ConfigPtr = XVisp_Ss_LookupConfig(BaseAddress);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XVisp_Ss_CfgInitialize(InstancePtr, ConfigPtr,
				      ConfigPtr->BaseAddress);
}
#endif

/** @} */
