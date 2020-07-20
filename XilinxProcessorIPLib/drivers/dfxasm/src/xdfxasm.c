/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfxasm.c
* @addtogroup dfxasm_v1_0
* @{
*
* This file contains the implementation of the interface functions for the
* XDfxasm driver. Refer xdfxsm.h for a detailed description of the driver.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver  Who    Date          Changes
* --- ----- ----------  -----------------------------------------------
* 1.0  dp    07/14/2020     First release
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdfxasm.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/****************************** Functions Definitions ************************/

/*****************************************************************************/
/**
*
* This function initializes a XDfxasm instance/driver.
*
* @param	InstancePtr is a pointer to the XDfxasm instance.
* @param	ConfigPtr points to the XDfxasm device configuration structure.
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
s32 XDfxasm_CfgInitialize(XDfxasm *InstancePtr, XDfxasm_Config *ConfigPtr,
			  UINTPTR EffectiveAddress)
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
	InstancePtr->Config.DeviceId = ConfigPtr->DeviceId;

	/* Indicate the component is now ready to use */
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is used to set shutdown manager in shutdown or pass through mode
*
* @param	InstancePtr is a pointer to the XDfxasm instance.
* @param	ShutdownValue is to set shutdown manager in shutdown or pass
*               through mode. Value can be either 0 or 1.
*		Values are
*			- XDFX_ASM_PASSTHROUGH_MODE(0) if in passthrough mode
*                       - XDFX_ASM_SHUTDOWN_MODE(1) if in shutdown mode
*
* @return	None
*
* @note		When Shutdown manager is in shutdown mode it blocks the axi traffic
*               thats going to the Reconfigurable partition from static logic
*               while Partial Reconfiguration occurs.
*
******************************************************************************/
void XDfxasm_SetState(XDfxasm *InstancePtr, XDfxasm_State ShutdownValue)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid((ShutdownValue == XDFX_ASM_SHUTDOWN_MODE) ||
			(ShutdownValue == XDFX_ASM_PASSTHROUGH_MODE));

	XDfxasm_WriteReg(((InstancePtr->Config.BaseAddress) + XDFX_ASM_CTRL_OFFSET),
			ShutdownValue);
}

/*****************************************************************************/
/**
*
* This function is used to read the state of the shutdown manager.
*
* @param	InstancePtr is a pointer to the XDfxasm instance.
*
* @return	Returns shutdown manager state and its bit significance is as
*               follows.
*               -BIT0 - 0 Entry to Pass Through mode has been requested
*                     - 1 Entry to Shutdown mode has been requested
*               -BIT1 - 0 The read or the write channel (or both) are in the Pass
*                         Through mode
*                     - 1 Both the read and write channels are in the Shutdown
*                         Mode
*               -BIT2 - 0 The write channel is in the Pass Through mode
*                     - 1 The write channel is in the Shutdown Mode
*               -BIT3 - 0 The read channel is in the Pass Through mode
*                     - 1 The read channel is in the Shutdown Mode
*
* @note		None.
*
******************************************************************************/
u32 XDfxasm_GetState(XDfxasm *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XDfxasm_ReadReg((InstancePtr->Config.BaseAddress) +
			XDFX_ASM_CTRL_OFFSET) & XDFX_ASM_CTRL_SHUTDOWN_MASK;

	return Data;
}
/** @} */
