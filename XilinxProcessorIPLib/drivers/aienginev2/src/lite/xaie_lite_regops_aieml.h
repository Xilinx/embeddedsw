/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_lite_ops_aieml.h
* @{
*
* This header file defines a lightweight version of AIEML specific register
* operations.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Wendy   09/06/2021  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XAIE_LITE_OPS_AIEML_H
#define XAIE_LITE_OPS_AIEML_H

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/
/********************** Variable Definitions *****************************/
/************************** Function Prototypes  *****************************/

#include "xaie_lite_io.h"
#include "xaie_lite_npi.h"

/*****************************************************************************/
/**
*
* This API set SHIM reset in the AI engine partition
*
* @param	DevInst: Device Instance
* @param	Loc: SHIM tile location
* @param	Reset: XAIE_ENABLE to enable reset,
* 			XAIE_DISABLE to disable reset
*
* @return	XAIE_OK for success, and error value for failure
*
* @note		This function is internal.
*		This function does nothing in AIEML.
*
******************************************************************************/
static inline void _XAie_LSetPartColShimReset(XAie_DevInst *DevInst,
		XAie_LocType Loc, u8 Reset)
{
	(void)DevInst;
	(void)Loc;
	(void)Reset;
}

/*****************************************************************************/
/**
*
* This API sets isolation boundry of an AI engine partition after reset
*
* @param	DevInst: Device Instance
*
* @note		Internal API only.
*
******************************************************************************/
static inline void _XAie_LSetPartIsolationAfterRst(XAie_DevInst *DevInst)
{
	for(u8 C = 0; C < DevInst->NumCols; C++) {
		u64 RegAddr;
		u32 RegVal = 0;

		if(C == 0) {
			RegVal = XAIE_TILE_CNTR_ISOLATE_WEST_MASK;
		} else if(C == (u8)(DevInst->NumCols - 1)) {
			RegVal = XAIE_TILE_CNTR_ISOLATE_EAST_MASK;
		}

		/* Isolate boundrary of SHIM tiles */
		RegAddr = _XAie_LGetTileAddr(0, C) +
			XAIE_PL_MOD_COL_CLKCNTR_REGOFF;
		_XAie_LPartWrite32(DevInst, RegAddr, RegVal);

		/* Isolate boundrary of MEM tiles */
		for (u8 R = XAIE_MEM_TILE_ROW_START;
			R < XAIE_AIE_TILE_ROW_START; R++) {
			RegAddr = _XAie_LGetTileAddr(R, C) +
				XAIE_MEM_TILE_MOD_TILE_CNTR_REGOFF;
			_XAie_LPartWrite32(DevInst, RegAddr, RegVal);
		}

		/* Isolate boundrary of CORE tiles */
		for (u8 R = XAIE_AIE_TILE_ROW_START;
			R < XAIE_NUM_ROWS; R++) {
			RegAddr = _XAie_LGetTileAddr(R, C) +
				XAIE_CORE_MOD_TILE_CNTR_REGOFF;
			_XAie_LPartWrite32(DevInst, RegAddr, RegVal);
		}
	}
}

/*****************************************************************************/
/**
*
* This API initialize the memories of the partition to zero.
*
* @param	DevInst: Device Instance
*
* @return       XAIE_OK on success, error code on failure
*
* @note		Internal API only.
*
******************************************************************************/
static inline void  _XAie_LPartMemZeroInit(XAie_DevInst *DevInst)
{
	u64 RegAddr;

	for(u8 C = 0; C < DevInst->NumCols; C++) {
		for (u8 R = XAIE_MEM_TILE_ROW_START;
			R < XAIE_AIE_TILE_ROW_START; R++) {
			RegAddr = _XAie_LGetTileAddr(R, C) +
				XAIE_MEM_TILE_MOD_MEM_CNTR_REGOFF;
			_XAie_LPartMaskWrite32(DevInst, RegAddr,
				XAIE_MEM_TILE_MEM_CNTR_ZEROISATION_MASK,
				XAIE_MEM_TILE_MEM_CNTR_ZEROISATION_MASK);
		}

		/* Isolate boundrary of CORE tiles */
		for (u8 R = XAIE_AIE_TILE_ROW_START;
			R < XAIE_NUM_ROWS; R++) {
			RegAddr = _XAie_LGetTileAddr(R, C) +
				XAIE_CORE_MOD_MEM_CNTR_REGOFF;
			_XAie_LPartMaskWrite32(DevInst, RegAddr,
				XAIE_CORE_MOD_MEM_CNTR_ZEROISATION_MASK,
				XAIE_CORE_MOD_MEM_CNTR_ZEROISATION_MASK);
			RegAddr = _XAie_LGetTileAddr(R, C) +
				XAIE_MEM_MOD_MEM_CNTR_REGOFF;
			_XAie_LPartMaskWrite32(DevInst, RegAddr,
				XAIE_MEM_MOD_MEM_CNTR_ZEROISATION_MASK,
				XAIE_MEM_MOD_MEM_CNTR_ZEROISATION_MASK);
		}
	}

	/* Poll last mem module and last mem tile mem module */
	RegAddr = _XAie_LGetTileAddr(XAIE_NUM_ROWS - 1,
			DevInst->NumCols - 1) +
			XAIE_MEM_MOD_MEM_CNTR_REGOFF;
	_XAie_LPartPoll32(DevInst, RegAddr,
			XAIE_MEM_MOD_MEM_CNTR_ZEROISATION_MASK, 0, 800);

	RegAddr = _XAie_LGetTileAddr(XAIE_AIE_TILE_ROW_START - 1,
			DevInst->NumCols - 1) +
			XAIE_MEM_TILE_MOD_MEM_CNTR_REGOFF;
	_XAie_LPartPoll32(DevInst, RegAddr,
			XAIE_MEM_TILE_MEM_CNTR_ZEROISATION_MASK, 0, 800);

}

/*****************************************************************************/
/**
*
* This is function to setup the protected register configuration value.
*
* @param	DevInst : AI engine partition device pointer
* @param	Enable: Enable partition
*
* @note		None
*
*******************************************************************************/
static inline void _XAie_LNpiSetPartProtectedReg(XAie_DevInst *DevInst,
		u8 Enable)
{
	u32 StartCol, EndCol, RegVal;

	/* TODO: Will need to use partition start column */
	StartCol = 0;
	EndCol = DevInst->NumCols + StartCol - 1;

	RegVal = _XAie_LSetRegField(Enable, XAIE_NPI_PROT_REG_CNTR_EN_LSB,
			       XAIE_NPI_PROT_REG_CNTR_EN_MSK);

	RegVal |= _XAie_LSetRegField(StartCol, XAIE_NPI_PROT_REG_CNTR_FIRSTCOL_LSB,
				XAIE_NPI_PROT_REG_CNTR_FIRSTCOL_MSK);
	RegVal |= _XAie_LSetRegField(EndCol, XAIE_NPI_PROT_REG_CNTR_LASTCOL_LSB,
				XAIE_NPI_PROT_REG_CNTR_LASTCOL_MSK);

	_XAie_LNpiWriteCheck32(XAIE_NPI_PROT_REG_CNTR_REG, RegVal);
}

#endif		/* end of protection macro */
/** @} */
