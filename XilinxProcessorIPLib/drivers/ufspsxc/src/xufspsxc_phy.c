/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xufspsxc_phy.c
* @addtogroup ufspsxc Overview
* @{
*
* This file implements the PHY calibration and high-speed series configuration
* helpers used by the functions in xufspsxc_hw.c and xufspsxc_init.c files.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.3   an  09/25/26 First release
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xufspsxc.h"
#include "xufspsxc_control.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
* @brief
* Program the valid (not 0x00/0xFF) PHY RX ATT/CTLE compensation values.
*
* @param	InstancePtr is a pointer to the XUfsPsxc instance.
* @param	UicCmdPtr is a pointer to the XUfsPsxc_UicCmd instance.
*
* @return	XUFSPSXC_SUCCESS or Error codes.
*
******************************************************************************/
u32 XUfsPsxc_ProgramRxCompValues(const XUfsPsxc *InstancePtr, XUfsPsxc_UicCmd *UicCmdPtr)
{
	volatile u32 Status = (u32)XUFSPSXC_SUCCESS;
	const u32 CompAddr[4] = {0x4000U, 0x4100U, 0x4001U, 0x4101U};
	u32 CompVal[4];
	u32 Index;

	CompVal[0] = InstancePtr->RxATTCompValL0;
	CompVal[1] = InstancePtr->RxATTCompValL1;
	CompVal[2] = InstancePtr->RxCTLECompValL0;
	CompVal[3] = InstancePtr->RxCTLECompValL1;

	for (Index = 0U; Index < 4U; Index++) {
		if ((CompVal[Index] != (u32)0x0U) && (CompVal[Index] != (u32)0xFFU)) {
			Status = XUfsPsxc_WritePhyReg(InstancePtr, UicCmdPtr, CompAddr[Index], CompVal[Index]);
			if (Status != (u32)XUFSPSXC_SUCCESS) {
				break;
			}
		}
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief
* Bypass RX-AFE auto-calibration on both RX lanes.
*
* @param	InstancePtr is a pointer to the XUfsPsxc instance.
* @param	UicCmdPtr is a pointer to the XUfsPsxc_UicCmd instance.
*
* @return	XUFSPSXC_SUCCESS or Error codes.
*
******************************************************************************/
u32 XUfsPsxc_BypassRxAfeCalib(const XUfsPsxc *InstancePtr, XUfsPsxc_UicCmd *UicCmdPtr)
{
	volatile u32 Status = (u32)XUFSPSXC_FAILURE;
	u32 ReadReg;

	Status = XUfsPsxc_ReadPhyReg(InstancePtr, UicCmdPtr, 0x401CU, &ReadReg);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}
	Status = XUfsPsxc_WritePhyReg(InstancePtr, UicCmdPtr, 0x401CU, (ReadReg | 4U));
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	Status = XUfsPsxc_ReadPhyReg(InstancePtr, UicCmdPtr, 0x411CU, &ReadReg);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}
	Status = XUfsPsxc_WritePhyReg(InstancePtr, UicCmdPtr, 0x411CU, (ReadReg | 4U));
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

ERROR:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* Program RAWAONLANE_DIG_FW_CALIB_CONFIG[8]=1 on both RX lanes.
*
* @param	InstancePtr is a pointer to the XUfsPsxc instance.
* @param	UicCmdPtr is a pointer to the XUfsPsxc_UicCmd instance.
*
* @return	XUFSPSXC_SUCCESS or Error codes.
*
******************************************************************************/
u32 XUfsPsxc_ProgramRxCalibConfig(const XUfsPsxc *InstancePtr, XUfsPsxc_UicCmd *UicCmdPtr)
{
	volatile u32 Status = (u32)XUFSPSXC_FAILURE;
	u32 ReadReg;

	Status = XUfsPsxc_ReadPhyReg(InstancePtr, UicCmdPtr, 0x404DU, &ReadReg);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}
	Status = XUfsPsxc_WritePhyReg(InstancePtr, UicCmdPtr, 0x404DU, (ReadReg | 0x100U));
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	Status = XUfsPsxc_ReadPhyReg(InstancePtr, UicCmdPtr, 0x414DU, &ReadReg);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}
	Status = XUfsPsxc_WritePhyReg(InstancePtr, UicCmdPtr, 0x414DU, (ReadReg | 0x100U));
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

ERROR:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* Configure the high-speed series settings and RX override handling.
*
* @param	InstancePtr is a pointer to the XUfsPsxc instance.
* @param	UicCmdPtr is a pointer to the XUfsPsxc_UicCmd instance.
* @param	Rate is the HS rate series value.
* @param	RxLanes is the number of connected RX data lanes.
* @param	tx_gear is the configured TX gear.
* @param	AdaptVal is a pointer to the adapt value that may be updated for G4.
*
* @return	XUFSPSXC_SUCCESS or Error codes.
*
******************************************************************************/
u32 XUfsPsxc_ConfigureHsSeries(const XUfsPsxc *InstancePtr, XUfsPsxc_UicCmd *UicCmdPtr,
				u32 Rate, u32 RxLanes, u32 tx_gear, u32 *AdaptVal)
{
	volatile u32 Status;
	u32 RateSel;

	XUfsPsxc_FillUICCmd(UicCmdPtr, ((u32)0x156AU << 16U), Rate, 0U, XUFSPSXC_DME_SET_OPCODE);	/*  PA_HSSERIES */
	Status = XUfsPsxc_SendUICCmd(InstancePtr, UicCmdPtr);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	if (Rate == XUFSPSXC_HSSERIES_B) {
		RateSel = 1U;
	} else {
		RateSel = 0U;
	}

	/* RMMI CBRATESEL */
	XUfsPsxc_FillUICCmd(UicCmdPtr, ((u32)0x8114U << 16U), RateSel, 0U, XUFSPSXC_DME_SET_OPCODE);
	Status = XUfsPsxc_SendUICCmd(InstancePtr, UicCmdPtr);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	/* L VS_MphyCfgUpdt */
	XUfsPsxc_FillUICCmd(UicCmdPtr, ((u32)0xD085U << 16U), 1U, 0U, XUFSPSXC_DME_SET_OPCODE);
	Status = XUfsPsxc_SendUICCmd(InstancePtr, UicCmdPtr);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	/* Override PHY rx_req to 1 */
	Status = XUfsPsxc_OverridePhyRxReq(InstancePtr, 1U, RxLanes);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	/* Override PHY rx_req to 0 */
	Status = XUfsPsxc_OverridePhyRxReq(InstancePtr, 0U, RxLanes);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	/* Remove PHY rx_req override (Connected lanes) */
	Status = XUfsPsxc_ClearPhyRxOverride(InstancePtr, UicCmdPtr, RxLanes);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	/* Program initial ADAPT for G4 */
	if ((tx_gear == XUFSPSXC_GEAR4) && (RxLanes == 2U)) {
		*AdaptVal = XUFSPSXC_PA_INITIAL_ADAPT;
	}

ERROR:
	return Status;
}

/** @} */
