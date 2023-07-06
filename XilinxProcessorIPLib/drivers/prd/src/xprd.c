/******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xprd.c
* @addtogroup prd Overview
* @{
*
* This file contains the implementation of the interface functions for the
* XPrd driver. Refer xprd.h for a detailed description of the driver.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver  Who    Date          Changes
* --- ----- ----------  -----------------------------------------------
* 1.0  ms    07/14/2016     First release
* 2.2  Nava  06/22/2023     Added support for system device-tree flow.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xprd.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/****************************** Functions Definitions ************************/

/*****************************************************************************/
/**
*
* This function initializes a XPrd instance/driver.
*
* @param	InstancePtr is a pointer to the XPrd instance.
* @param	ConfigPtr points to the XPrd device configuration structure.
* @param	EffectiveAddress is the device base address in the virtual
*		memory address space. If the address translation is not used
*		then the physical address is passed.
*
* @return
*		- XST_SUCCESS if initialization was successful.
*
* @note		None.
*
******************************************************************************/
s32 XPrd_CfgInitialize(XPrd *InstancePtr, XPrd_Config *ConfigPtr,
		       u32 EffectiveAddress)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ConfigPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddress != 0x00);

	/**
	 * Set some default values for instance data, don't indicate the
	 * device is ready to use until everything has been initialized
	 * successfully.
	 */
	InstancePtr->Config.BaseAddress = EffectiveAddress;
#ifndef SDT
	InstancePtr->Config.DeviceId = ConfigPtr->DeviceId;
#endif

	/* Indicate the component is now ready to use */
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is used to set Decoupler On or Off.
*
* @param	InstancePtr is a pointer to the XPrd instance.
* @param	DecouplerValue is to set Decoupler On or Off. Value can be
*		either 0 or 1.
*		Values are
*			- XPRD_DECOUPLER_OFF(0) if Decoupler is Off
*			- XPRD_DECOUPLER_ON(1) if Decoupler is On
*
* @return	None
*
* @note		When Decoupler is On then it separates the Reconfigurable
*		partition from static logic while Partial Reconfiguration
*		occurs.
*
******************************************************************************/
void XPrd_SetDecouplerState(XPrd *InstancePtr, XPrd_State DecouplerValue)
{
	u32 Data;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid((DecouplerValue == XPRD_DECOUPLER_ON) ||
		       (DecouplerValue == XPRD_DECOUPLER_OFF));

	XPrd_WriteReg(((InstancePtr->Config.BaseAddress) + XPRD_CTRL_OFFSET),
		      DecouplerValue);
}

/*****************************************************************************/
/**
*
* This function is used to read the state of the Decoupler.
*
* @param	InstancePtr is a pointer to the XPrd instance.
*
* @return	Returns decoupler state.
*		- XPRD_DECOUPLER_ON(1) if Decoupler is enabled.
*		- XPRD_DECOUPLER_OFF(0) if Decoupler is disabled.
*
* @note		None.
*
******************************************************************************/
XPrd_State XPrd_GetDecouplerState(XPrd *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XPrd_ReadReg((InstancePtr->Config.BaseAddress) +
			    XPRD_CTRL_OFFSET) & XPRD_CTRL_DECOUPLER_MASK;

	return (Data) ? XPRD_DECOUPLER_ON : XPRD_DECOUPLER_OFF;
}
/** @} */
