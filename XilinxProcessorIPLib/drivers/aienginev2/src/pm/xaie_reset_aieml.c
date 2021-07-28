/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_reset_aieml.c
* @{
*
* This file contains routines for AI engine resets for AIEML
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_feature_config.h"
#include "xaie_helper.h"
#include "xaie_npi.h"
#include "xaiegbl.h"

#ifdef XAIE_FEATURE_PRIVILEGED_ENABLE

/*****************************************************************************/
/***************************** Macro Definitions *****************************/
/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API reset the SHIM for the specified columns
*
* @param	DevInst: Device Instance
* @param	StartCol: Start column
* @param	NumCols: Number of columns
*
* @return	XAIE_OK for success, and error value for failure
*
* @note		It is not required to check the DevInst as the caller function
*		should provide the correct value. As we only supports full
*		partition SHIMs reset, if the @StartCol and @NumCols are not
*		aligned with the full partition, it will return error.
*		This function does the following steps:
*		 * Enable protect registers
*		 * Assert SHIM reset
*		 * Deassert SHIM reset
*		 * Disable protect registers
*
******************************************************************************/
AieRC _XAieMl_RstShims(XAie_DevInst *DevInst, u32 StartCol, u32 NumCols)
{
	XAie_NpiProtRegReq ProtRegReq;

	if(StartCol > 0 || NumCols != DevInst->NumCols) {
		XAIE_ERROR("AIE shim reset, not supported columns.\n");
		return XAIE_INVALID_ARGS;
	}

	ProtRegReq.StartCol = StartCol;
	ProtRegReq.NumCols = NumCols;
	ProtRegReq.Enable = XAIE_ENABLE;
	XAie_RunOp(DevInst, XAIE_BACKEND_OP_SET_PROTREG, (void *)&ProtRegReq);

	XAie_RunOp(DevInst, XAIE_BACKEND_OP_ASSERT_SHIMRST,
			(void *)(uintptr_t)XAIE_ENABLE);

	XAie_RunOp(DevInst, XAIE_BACKEND_OP_ASSERT_SHIMRST,
			(void *)(uintptr_t)XAIE_DISABLE);

	ProtRegReq.Enable = XAIE_DISABLE;
	XAie_RunOp(DevInst, XAIE_BACKEND_OP_SET_PROTREG, (void *)&ProtRegReq);

	return XAIE_OK;
}

#endif /* XAIE_FEATURE_PRIVILEGED_ENABLE */
/** @} */
