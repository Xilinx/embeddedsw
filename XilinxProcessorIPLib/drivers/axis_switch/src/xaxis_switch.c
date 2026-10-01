/******************************************************************************
* Copyright (C) 2015 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xaxis_switch.c
*
* @brief Main file for the AXI4-Stream Switch Control Router core.
*
* See xaxis_switch.h for more details of the driver.
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
* 1.5   asa 08/25/22 Remove the incorrect check in the API
*                    XAxisScr_IsMiPortEnabled. Because of the incorrect check
*                    the API can return a false TRUE for an failure case.
*                    As an example, if a MI is configured for SI[7] and the
*                    API is called to check if the MI is configured for SI[3],
*                    the API will return TRUE.
* 1.9   vn  07/30/26 Fix Doxygen warnings
* 1.9  vn   09/08/26 Align Doxygen comments with AMD standards
* 1.9  vn   09/30/26 Update Doxygen comments to AMD standards
* </pre>
*
******************************************************************************/

/** @addtogroup axis_switch Overview
 * @{
 */

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
* @brief Initializes the AXI4-Stream Switch core.
*
* Call this function prior to using the core. Sets up the instance data
* and ensures the hardware is in a quiescent state.
*
* @param[out]   InstancePtr Pointer to the XAxis_Switch core instance.
* @param[in]    CfgPtr Pointer to the configuration structure associated with
*		the AXI4-Stream Switch core.
* @param[in]    EffectiveAddr Base address of the device. If address
*		translation is being used, then this parameter must reflect the
*		virtual base address. Otherwise, the physical address should be
*		used.
*
* @return	XST_SUCCESS if initialization was successful.
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
* @brief Enables the MUX port specified by a MI index and links it with
*        the desired SI.
*
* @param[in]    InstancePtr Pointer to the XAxis_Switch core instance.
* @param[in]    MiIndex Index for enabling the MUX.
*		Range is [0 to 15].
* @param[in]    SiIndex Index used as a source to the MUX.
*		Range is [0 to 15].
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
* @brief Disables the MUX port specified by a MI index and unlinks the
*        previously set SI.
*
* @param[in]    InstancePtr Pointer to the XAxis_Switch core instance.
* @param[in]    MiIndex Index for disabling the MUX.
*		Range is [0 to 15].
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
* @brief Checks whether the MUX is sourced from the desired SI.
*
* @param[in]    InstancePtr Pointer to the XAxis_Switch core instance.
* @param[in]    MiIndex Index for addressing the MUX.
*		Range is [0 to 15].
* @param[in]    SiIndex SI index to check as the source.
*		Range is [0 to 15].
*
* @return	TRUE if MUX is sourced from SI, FALSE otherwise.
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

	if ((RegValue == SiIndex) && (!Enable)) {
		RegValue = TRUE;
	} else {
		RegValue = FALSE;
	}

	return RegValue;
}

/*****************************************************************************/
/**
* @brief Checks whether the MUX port is disabled.
*
* @param[in]    InstancePtr Pointer to the XAxis_Switch core instance.
* @param[in]    MiIndex Index for addressing the MUX.
*		Range is [0 to 15].
*
* @return	TRUE if MUX is disabled, FALSE otherwise.
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
* @brief Disables all MUX ports and removes association with the SI.
*
* @param[in]    InstancePtr Pointer to the XAxis_Switch core instance.
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
