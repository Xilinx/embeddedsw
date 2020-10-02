/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_core_aieml.c
* @{
*
* This file contains routines for AIEML core control.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   10/02/2020  Initial creation
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_core_aieml.h"

/************************** Constant Definitions *****************************/

/************************** Function Definitions *****************************/
/*****************************************************************************/
/*
*
* This API is not supported for AIE-ML.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the aie tile.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*
******************************************************************************/
AieRC _XAieMl_CoreConfigureDone(XAie_DevInst *DevInst, XAie_LocType Loc,
		const struct XAie_CoreMod *CoreMod)
{
	(void)DevInst;
	(void)Loc;
	(void)CoreMod;

	return XAIE_FEATURE_NOT_SUPPORTED;
}

/** @} */
