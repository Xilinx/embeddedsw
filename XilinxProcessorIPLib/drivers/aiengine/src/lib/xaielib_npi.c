/*******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/


/******************************************************************************/
/**
* @file xaielib_npi.c
* @{
*
* This file contains routines for NPI module
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Jubaer  03/08/2019  Initial creation
* 1.1  Hyun    04/04/2019  Add the unlock and lock sequences
* </pre>
*
*******************************************************************************/

/***************************** Include Files **********************************/
#include "xaiegbl.h"
#include "xaiegbl_defs.h"
#include "xaiegbl_reginit.h"
#include "xaielib_npi.h"

/***************************** Constant Definitions ***************************/
/***************************** Macro Definitions ******************************/
/************************** Variable Definitions ******************************/
/************************** Function Definitions ******************************/
/*****************************************************************************/
/**
*
* This API is used to assert shim reset from NPI.
*
* @param	Reset - Reset/Unreset the Shim Tile (1-Reset,0-Unreset).
*
* @return	XAIE_SUCCESS if successful, else XAIE_FAILURE.
*
* @note		None.
*
*******************************************************************************/
u8 XAieLib_NpiShimReset(u8 Reset)
{
	u32 RegVal;

	XAie_AssertNonvoid(Reset == 0 || Reset == 1);

	XAieGbl_NPIWrite32(XAIE_NPI_PCSR_MASK, 1U <<
			   XAIE_NPI_PCSR_MASK_SHIM_RESET_LSB);

	RegVal = XAie_SetField(Reset, XAIE_NPI_PCSR_CONTROL_SHIM_RESET_LSB,
			       XAIE_NPI_PCSR_CONTROL_SHIM_RESET_MSK);
	XAieGbl_NPIWrite32(XAIE_NPI_PCSR_CONTROL, RegVal);

	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API is used to assert AIE array reset from NPI.
*
* @param	Reset - Reset/Unreset the AIE array.
*
* @return	XAIE_SUCCESS if successful, else XAIE_FAILURE.
*
* @note		None.
*
*******************************************************************************/
u8 XAieLib_NpiAieArrayReset(u8 Reset)
{
	u32 RegVal;

	XAie_AssertNonvoid(Reset == 0 || Reset == 1);

	XAieGbl_NPIWrite32(XAIE_NPI_PCSR_MASK, 1U <<
			   XAIE_NPI_PCSR_MASK_AIE_ARRAY_RESET_LSB);

	RegVal = XAie_SetField(Reset, XAIE_NPI_PCSR_CONTROL_AIE_ARRAY_RESET_LSB,
			       XAIE_NPI_PCSR_CONTROL_AIE_ARRAY_RESET_MASK);
	XAieGbl_NPIWrite32(XAIE_NPI_PCSR_CONTROL, RegVal);

	return XAIE_SUCCESS;
}

/** @} */
