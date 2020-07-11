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
static u32 _XAie_NpiSetProtectedRegField(XAie_DevInst *DevInst, u8 Enable);

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
	.SetProtectedRegField = _XAie_NpiSetProtectedRegField,
};

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This is function to setup the protected register configuration value.
*
* @param	DevInst : AI engine partition device pointer
* @param	Enable : XAIE_ENABLE to enable access to protected register,
*			 XAIE_DISABLE to disable access to protected register
*
* @return	32bit Value used by caller function to set to the register
*
* @note		None
*
*******************************************************************************/
static u32 _XAie_NpiSetProtectedRegField(XAie_DevInst *DevInst, u8 Enable)
{
	u32 RegVal, CFirst, CLast;

	RegVal = XAie_SetField(Enable, _XAie2NpiMod.ProtRegEnable.Lsb,
			       _XAie2NpiMod.ProtRegEnable.Mask);

	CFirst = (DevInst->BaseAddr & XAIE2_COL_MASK) >>
		DevInst->DevProp.ColShift;
	CLast = CFirst + DevInst->NumCols - 1;

	RegVal |= XAie_SetField(CFirst, _XAie2NpiMod.ProtRegFirstCol.Lsb,
				_XAie2NpiMod.ProtRegFirstCol.Mask);
	RegVal |= XAie_SetField(CLast, _XAie2NpiMod.ProtRegLastCol.Lsb,
				_XAie2NpiMod.ProtRegLastCol.Mask);
	return RegVal;
}

/** @} */
