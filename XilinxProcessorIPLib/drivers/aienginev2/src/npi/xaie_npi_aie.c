/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_npi_aie.c
* @{
*
* This file contains routines for operations for accessing AI engine
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
#define XAIE_NPI_PCSR_MASK				0x00000000U
#define XAIE_NPI_PCSR_MASK_SHIM_RESET_MSK		0x08000000U
#define XAIE_NPI_PCSR_MASK_SHIM_RESET_LSB		27U

#define XAIE_NPI_PCSR_CONTROL				0X00000004U
#define XAIE_NPI_PCSR_CONTROL_SHIM_RESET_MSK		0x08000000U
#define XAIE_NPI_PCSR_CONTROL_SHIM_RESET_LSB		27U

#define XAIE_NPI_PCSR_LOCK				0X0000000CU
#define XAIE_NPI_PCSR_UNLOCK_CODE			0xF9E8D7C6U

#define XAIE_NPI_SPARE_REG				0x00000200U
#define XAIE_NPI_PROT_REG_EN_MSK			0x00000001U
#define XAIE_NPI_PROT_REG_EN_LSB			0U

#define XAIE_NPI_IRQ_REG				0x00000030U

/****************************** Type Definitions *****************************/

/************************** Variable Definitions *****************************/
static AieRC _XAie_NpiSetProtectedRegField(XAie_DevInst *DevInst,
		XAie_NpiProtRegReq *Req, u32 *RegVal);

XAie_NpiMod _XAieNpiMod =
{
	.PcsrMaskOff = XAIE_NPI_PCSR_MASK,
	.PcsrCntrOff = XAIE_NPI_PCSR_CONTROL,
	.PcsrLockOff = XAIE_NPI_PCSR_LOCK,
	.ProtRegOff = XAIE_NPI_SPARE_REG,
	.PcsrUnlockCode = XAIE_NPI_PCSR_UNLOCK_CODE,
	.BaseIrqRegOff = XAIE_NPI_IRQ_REG,
	.AieIrqNum = 0x4,
	.NpiIrqNum = 0x4,
	.IrqEnableOff = 0xC,
	.IrqDisableOff = 0x10,
	.ShimReset = {XAIE_NPI_PCSR_CONTROL_SHIM_RESET_LSB, XAIE_NPI_PCSR_CONTROL_SHIM_RESET_MSK},
	.ProtRegEnable = {XAIE_NPI_PROT_REG_EN_LSB, XAIE_NPI_PROT_REG_EN_MSK},
	.ProtRegFirstCol = {0, 0},
	.ProtRegLastCol = {0, 0},
	.SetProtectedRegField = _XAie_NpiSetProtectedRegField,
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
* @return	XAIE_OK
*
* @note		For AIE, the StartCol and NumCols in the @Req will not be
*		used as it is not supported in hardware.
*
*******************************************************************************/
static AieRC _XAie_NpiSetProtectedRegField(XAie_DevInst *DevInst,
		XAie_NpiProtRegReq *Req, u32 *RegVal)
{
	(void) DevInst;

	*RegVal = XAie_SetField(Req->Enable, _XAieNpiMod.ProtRegEnable.Lsb,
			       _XAieNpiMod.ProtRegEnable.Mask);

	return XAIE_OK;
}

#endif /* XAIE_FEATURE_PRIVILEGED_ENABLE */
/** @} */
