/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xprd.c
* @addtogroup prd_v1_1
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
*
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
	InstancePtr->Config.DeviceId = ConfigPtr->DeviceId;

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
