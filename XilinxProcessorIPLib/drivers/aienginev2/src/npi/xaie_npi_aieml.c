/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_npi_aieml.c
* @{
*
* This file contains routines for operations for accessing AI engine v2
* specific NPI.
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_feature_config.h"
#include "xaie_helper.h"
#include "xaie_npi.h"
#include "xaiegbl.h"

#ifdef XAIE_FEATURE_PRIVILEGED_ENABLE

/************************** Constant Definitions *****************************/
#define XAIEML_NPI_PCSR_UNLOCK_CODE			0xF9E8D7C6U
#define XAIEML_NPI_PCSR_MASK				0x00000000U
#define XAIEML_NPI_PCSR_MASK_SHIM_RESET_MSK		0x08000000U
#define XAIEML_NPI_PCSR_MASK_SHIM_RESET_LSB		27U

#define XAIEML_NPI_PCSR_CONTROL				0X00000004U
#define XAIEML_NPI_PCSR_CONTROL_SHIM_RESET_MSK		0x08000000U
#define XAIEML_NPI_PCSR_CONTROL_SHIM_RESET_LSB		27U

#define XAIEML_NPI_PCSR_LOCK				0X0000000CU
#define XAIEML_NPI_PCSR_LOCK_STATE_UNLOCK_CODE		0xF9E8D7C6U

#define XAIEML_NPI_PROT_REG_CNTR				0x00000200U
#define XAIEML_NPI_PROT_REG_CNTR_EN_MSK			0x00000001U
#define XAIEML_NPI_PROT_REG_CNTR_EN_LSB			0U
#define XAIEML_NPI_PROT_REG_CNTR_FIRSTCOL_MSK		0x000000FEU
#define XAIEML_NPI_PROT_REG_CNTR_FIRSTCOL_LSB		1U
#define XAIEML_NPI_PROT_REG_CNTR_LASTCOL_MSK		0x00007F00U
#define XAIEML_NPI_PROT_REG_CNTR_LASTCOL_LSB		8U

#define XAIEML_COL_MASK	(0x7FU << 25)
/****************************** Type Definitions *****************************/

/************************** Variable Definitions *****************************/
static AieRC _XAieMl_NpiSetProtectedRegField(XAie_DevInst *DevInst,
		XAie_NpiProtRegReq *Req, u32 *RegVal);

const XAie_NpiMod _XAieMlNpiMod =
{
	.PcsrMaskOff = XAIEML_NPI_PCSR_MASK,
	.PcsrCntrOff = XAIEML_NPI_PCSR_CONTROL,
	.PcsrLockOff = XAIEML_NPI_PCSR_LOCK,
	.ProtRegOff = XAIEML_NPI_PROT_REG_CNTR,
	.PcsrUnlockCode = XAIEML_NPI_PCSR_UNLOCK_CODE,
	.BaseIrqRegOff = XAIE_FEATURE_UNAVAILABLE,
	.AieIrqNum = XAIE_FEATURE_UNAVAILABLE,
	.NpiIrqNum = XAIE_FEATURE_UNAVAILABLE,
	.IrqEnableOff = XAIE_FEATURE_UNAVAILABLE,
	.IrqDisableOff = XAIE_FEATURE_UNAVAILABLE,
	.ShimReset = {XAIEML_NPI_PCSR_CONTROL_SHIM_RESET_LSB, XAIEML_NPI_PCSR_CONTROL_SHIM_RESET_MSK},
	.ProtRegEnable = {XAIEML_NPI_PROT_REG_CNTR_EN_LSB, XAIEML_NPI_PROT_REG_CNTR_EN_MSK},
	.ProtRegFirstCol = {XAIEML_NPI_PROT_REG_CNTR_FIRSTCOL_LSB, XAIEML_NPI_PROT_REG_CNTR_FIRSTCOL_MSK},
	.ProtRegLastCol = {XAIEML_NPI_PROT_REG_CNTR_LASTCOL_LSB, XAIEML_NPI_PROT_REG_CNTR_LASTCOL_MSK},
	.SetProtectedRegField = _XAieMl_NpiSetProtectedRegField,
};

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This is function to setup the protected register configuration value.
*
* @param	DevInst : AI engine partition device pointer
* @param	Req: Protect Register Request
* @param	RegVal: pointer to return calculated register value
*
* @return	XAIE_OK for success, and negative value for failure
*
* @note		None
*
*******************************************************************************/
static AieRC _XAieMl_NpiSetProtectedRegField(XAie_DevInst *DevInst,
		XAie_NpiProtRegReq *Req, u32 *RegVal)
{
	u32 CFirst, CLast, NumCols;

	if ((Req->StartCol + Req->NumCols) > DevInst->NumCols ||
	    (Req->StartCol != 0  && Req->NumCols == 0)) {
		XAIE_ERROR("Invalid columns (%u, %u) for protected regs.\n",
				Req->StartCol, Req->NumCols);
		return XAIE_INVALID_ARGS;
	}

	*RegVal = XAie_SetField(Req->Enable, _XAieMlNpiMod.ProtRegEnable.Lsb,
			       _XAieMlNpiMod.ProtRegEnable.Mask);

	NumCols = Req->NumCols;
	if (NumCols == 0) {
		/* It is for the whole partition instance */
		NumCols = DevInst->NumCols;
		CFirst = 0;
	} else {
		CFirst = Req->StartCol;
	}

	CLast = CFirst + NumCols - 1;

	*RegVal |= XAie_SetField(CFirst, _XAieMlNpiMod.ProtRegFirstCol.Lsb,
				_XAieMlNpiMod.ProtRegFirstCol.Mask);
	*RegVal |= XAie_SetField(CLast, _XAieMlNpiMod.ProtRegLastCol.Lsb,
				_XAieMlNpiMod.ProtRegLastCol.Mask);

	return XAIE_OK;
}

#endif /* XAIE_FEATURE_PRIVILEGED_ENABLE */
/** @} */
