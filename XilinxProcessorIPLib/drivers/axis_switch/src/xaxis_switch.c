/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xaxis_switch.c
* @addtogroup axis_switch_v1_4
* @{
*
* This is the main file for Xilinx AXI4-Stream Switch Control Router core.
* Please see xaxis_switch.h for more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- --------------------------------------------------
* 1.00  sha 01/28/15 Initial release.
* 1.1   sk  08/16/16 Used UINTPTR instead of u32 for Baseaddress as part of
*                    adding 64 bit support. CR# 867425.
*                    Changed the prototype of XAxisScr_CfgInitialize API.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xaxis_switch.h"
#include "string.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function initializes the AXI4-Stream Switch core. This function must
* be called prior to using the core. Initialization of the core includes
* setting up the instance data and ensuring the hardware is in a quiescent
* state.
*
* @param	InstancePtr is a pointer to the XAxis_Switch core instance.
* @param	CfgPtr points to the configuration structure associated with
*		the AXI4-Stream Switch core.
* @param	EffectiveAddr is the base address of the device. If address
*		translation is being used, then this parameter must reflect the
*		virtual base address. Otherwise, the physical address should be
*		used.
*
* @return
*		- XST_SUCCESS if XAxisScr_CfgInitialize was successful.
*
* @note		Axis_Switch core connects up to 16 masters to 16 slaves.
*
******************************************************************************/
s32 XAxisScr_CfgInitialize(XAxis_Switch *InstancePtr,
			XAxis_Switch_Config *CfgPtr, UINTPTR EffectiveAddr)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddr != (u32)0x0);

	/* Setup the instance */
	(void)memset((void *)InstancePtr, 0, sizeof(XAxis_Switch));
	(void)memcpy((void *)&(InstancePtr->Config), (const void *)CfgPtr,
					sizeof(XAxis_Switch_Config));
	InstancePtr->Config.BaseAddress = EffectiveAddr;

	/* Make sure that all MI ports are disabled */
	XAxisScr_MiPortDisableAll(InstancePtr);

	/* Set the flag to indicate the driver is ready */
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function enables the MUX port specified by a MI index and links with
* desired SI, indicated by a SiIndex.
*
* @param	InstancePtr is a pointer to the XAxis_Switch core instance.
* @param	MiIndex specifies an index that will be used for enabling MUX
*		which is within the range [0 to 15].
* @param	SiIndex specifies an index that will be used as a source to
*		MUX. The range is [0 to 15].
*
* @return	None.
*
* @note		MUX[MiIndex] is sourced from SI[SiIndex]. Axis_Switch core
*		connects up to 16 masters to 16 slaves.
*
******************************************************************************/
void XAxisScr_MiPortEnable(XAxis_Switch *InstancePtr, u8 MiIndex, u8 SiIndex)
{
	u32 MiPortAddr;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(MiIndex < InstancePtr->Config.MaxNumMI);
	Xil_AssertVoid(SiIndex < InstancePtr->Config.MaxNumSI);

	/* Calculate MI port address to be enabled */
	MiPortAddr = XAXIS_SCR_MI_MUX_START_OFFSET + 4 * MiIndex;

	/* MUX[MiIndex] is sourced from SI[SiIndex] */
	XAxisScr_WriteReg(InstancePtr->Config.BaseAddress, MiPortAddr,
				SiIndex);
}

/*****************************************************************************/
/**
*
* This function disables the MUX port specified by a MI index and un-link
* SI, set previously.
*
* @param	InstancePtr is a pointer to the XAxis_Switch core instance.
* @param	MiIndex specifies an index that will be used for disabling MUX
*		which is within the range [0 to 15].
*
* @return	None.
*
* @note		MUX[MiIndex] is removed for SI[SiIndex]. Axis_Switch core
*		connects up to 16 masters to 16 slaves.
*
******************************************************************************/
void XAxisScr_MiPortDisable(XAxis_Switch *InstancePtr, u8 MiIndex)
{
	u32 MiPortAddr;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(MiIndex < InstancePtr->Config.MaxNumMI);


	/* Calculate MI port address of which to be enabled */
	MiPortAddr = XAXIS_SCR_MI_MUX_START_OFFSET + 4 * MiIndex;

	XAxisScr_WriteReg(InstancePtr->Config.BaseAddress, MiPortAddr,
				XAXIS_SCR_MI_X_DISABLE_MASK);
}

/*****************************************************************************/
/**
*
* This function returns the status of whether MUX is sourced from desired SI.
*
* @param	InstancePtr is a pointer to the XAxis_Switch core instance.
* @param	MiIndex specifies an index that will be used for addressing
*		the MUX which is within the range [0 to 15].
* @param	SiIndex specifies an index that will be used to identify,
*		whether it is sourced. The range is [0 to 15].
*
* @return
*		- TRUE, if MUX is sourced from SI.
*		- FALSE, if MUX is not sourced from SI.
*
* @note		Axis_Switch core connects up to 16 masters to 16 slaves.
*
******************************************************************************/
s32 XAxisScr_IsMiPortEnabled(XAxis_Switch *InstancePtr, u8 MiIndex,
				u8 SiIndex)
{
	u32 MiPortAddr;
	u32 RegValue;
	u8 Enable;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(MiIndex < InstancePtr->Config.MaxNumMI);
	Xil_AssertNonvoid(SiIndex < InstancePtr->Config.MaxNumSI);

	/* Calculate MI port address to be enabled */
	MiPortAddr = XAXIS_SCR_MI_MUX_START_OFFSET + 4 * MiIndex;

	/* Read MI port data */
	RegValue = XAxisScr_ReadReg(InstancePtr->Config.BaseAddress,
			MiPortAddr);

	/* Fetch enable bit */
	Enable = RegValue >> XAXIS_SCR_MI_X_DISABLE_SHIFT;

	/* Fetch SI value */
	RegValue &= XAXIS_SCR_MI_X_MUX_MASK;


	/* When SiIndex is zero */
	if ((RegValue == SiIndex) && (!Enable)) {
		RegValue = TRUE;
	}
	else if((RegValue & SiIndex) && (!Enable)) {
		RegValue = TRUE;
	}
	else {
		RegValue = FALSE;
	}

	return RegValue;
}

/*****************************************************************************/
/**
*
* This function returns the status of whether MUX is disabled.
*
* @param	InstancePtr is a pointer to the XAxis_Switch core instance.
* @param	MiIndex specifies an index that will be used for addressing
*		the MUX which is within the range [0 to 15].
*
* @return
*		- TRUE, if MUX is disabled.
*		- FALSE, if MUX is enabled.
*
* @note		Axis_Switch core connects up to 16 masters to 16 slaves.
*
******************************************************************************/
s32 XAxisScr_IsMiPortDisabled(XAxis_Switch *InstancePtr, u8 MiIndex)
{
	u32 MiPortAddr;
	u8 Disable;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(MiIndex < InstancePtr->Config.MaxNumMI);

	/* Calculate MI port address to be enabled */
	MiPortAddr = XAXIS_SCR_MI_MUX_START_OFFSET + 4 * MiIndex;

	/* Read MI port data */
	Disable = XAxisScr_ReadReg(InstancePtr->Config.BaseAddress,
			MiPortAddr) >> XAXIS_SCR_MI_X_DISABLE_SHIFT;

	if (Disable) {
		Disable = TRUE;
	}

	return Disable;
}

/*****************************************************************************/
/**
*
* This function disables all the MUX port, in turn removes association with
* desired SI.
*
* @param	InstancePtr is a pointer to the XAxis_Switch core instance.
*
* @return	None.
*
* @note		Axis_Switch core connects up to 16 masters to 16 slaves.
*
******************************************************************************/
void XAxisScr_MiPortDisableAll(XAxis_Switch *InstancePtr)
{
	u32 MiPortAddr;
	u8 Index;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	for (Index = 0; Index < InstancePtr->Config.MaxNumMI; Index++) {

		/* Calculate MI port address of which to be enabled */
		MiPortAddr = XAXIS_SCR_MI_MUX_START_OFFSET + 4 * Index;

		XAxisScr_WriteReg(InstancePtr->Config.BaseAddress, MiPortAddr,
				XAXIS_SCR_MI_X_DISABLE_MASK);
	}
}
/** @} */
