/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_npi_aie2.c
* @{
*
* This file contains routines for operations for accessing AI engine v2
* specific NPI.
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_helper.h"
#include "xaie_npi.h"
#include "xaiegbl.h"

/************************** Constant Definitions *****************************/
#define XAIE2_NPI_PCSR_UNLOCK_CODE			0xF9E8D7C6U
#define XAIE2_NPI_PCSR_MASK				0x00000000U
#define XAIE2_NPI_PCSR_MASK_SHIM_RESET_MSK		0x08000000U
#define XAIE2_NPI_PCSR_MASK_SHIM_RESET_LSB		27U

#define XAIE2_NPI_PCSR_CONTROL				0X00000004U
#define XAIE2_NPI_PCSR_CONTROL_SHIM_RESET_MSK		0x08000000U
#define XAIE2_NPI_PCSR_CONTROL_SHIM_RESET_LSB		27U

#define XAIE2_NPI_PCSR_LOCK				0X0000000CU
#define XAIE2_NPI_PCSR_LOCK_STATE_UNLOCK_CODE		0xF9E8D7C6U

#define XAIE2_NPI_PROT_REG_CNTR				0x00000200U
#define XAIE2_NPI_PROT_REG_CNTR_EN_MSK			0x00000001U
#define XAIE2_NPI_PROT_REG_CNTR_EN_LSB			0U
#define XAIE2_NPI_PROT_REG_CNTR_FIRSTCOL_MSK		0x000000FEU
#define XAIE2_NPI_PROT_REG_CNTR_FIRSTCOL_LSB		1U
#define XAIE2_NPI_PROT_REG_CNTR_LASTCOL_MSK		0x00007F00U
#define XAIE2_NPI_PROT_REG_CNTR_LASTCOL_LSB		8U

#define XAIE2_COL_MASK	(0x7FU << 25)
/****************************** Type Definitions *****************************/

/************************** Variable Definitions *****************************/
static AieRC _XAie2_NpiSetProtectedRegField(XAie_DevInst *DevInst,
		XAie_NpiProtRegReq *Req, u32 *RegVal);

const XAie_NpiMod _XAie2NpiMod =
{
	.PcsrMaskOff = XAIE2_NPI_PCSR_MASK,
	.PcsrCntrOff = XAIE2_NPI_PCSR_CONTROL,
	.PcsrLockOff = XAIE2_NPI_PCSR_LOCK,
	.ProtRegOff = XAIE2_NPI_PROT_REG_CNTR,
	.PcsrUnlockCode = XAIE2_NPI_PCSR_UNLOCK_CODE,
	.ShimReset = {XAIE2_NPI_PCSR_CONTROL_SHIM_RESET_LSB, XAIE2_NPI_PCSR_CONTROL_SHIM_RESET_MSK},
	.ProtRegEnable = {XAIE2_NPI_PROT_REG_CNTR_EN_LSB, XAIE2_NPI_PROT_REG_CNTR_EN_MSK},
	.ProtRegFirstCol = {XAIE2_NPI_PROT_REG_CNTR_FIRSTCOL_LSB, XAIE2_NPI_PROT_REG_CNTR_FIRSTCOL_MSK},
	.ProtRegLastCol = {XAIE2_NPI_PROT_REG_CNTR_LASTCOL_LSB, XAIE2_NPI_PROT_REG_CNTR_LASTCOL_MSK},
	.SetProtectedRegField = _XAie2_NpiSetProtectedRegField,
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
static AieRC _XAie2_NpiSetProtectedRegField(XAie_DevInst *DevInst,
		XAie_NpiProtRegReq *Req, u32 *RegVal)
{
	u32 CFirst, CLast, NumCols;

	if ((Req->StartCol + Req->NumCols) > DevInst->NumCols ||
	    (Req->StartCol != 0  && Req->NumCols == 0)) {
		XAIE_ERROR("Invalid columns (%u, %u) for protected regs.\n",
				Req->StartCol, Req->NumCols);
		return XAIE_INVALID_ARGS;
	}

	*RegVal = XAie_SetField(Req->Enable, _XAie2NpiMod.ProtRegEnable.Lsb,
			       _XAie2NpiMod.ProtRegEnable.Mask);

	NumCols = Req->NumCols;
	if (NumCols == 0) {
		/* It is for the whole partition instance */
		NumCols = DevInst->NumCols;
		CFirst = 0;
	} else {
		CFirst = Req->StartCol;
	}

	CFirst += (DevInst->BaseAddr & XAIE2_COL_MASK) >>
		DevInst->DevProp.ColShift;
	CLast = CFirst + NumCols - 1;

	*RegVal |= XAie_SetField(CFirst, _XAie2NpiMod.ProtRegFirstCol.Lsb,
				_XAie2NpiMod.ProtRegFirstCol.Mask);
	*RegVal |= XAie_SetField(CLast, _XAie2NpiMod.ProtRegLastCol.Lsb,
				_XAie2NpiMod.ProtRegLastCol.Mask);

	return XAIE_OK;
}

/** @} */
