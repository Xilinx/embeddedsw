/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_io_privilege.c
* @{
*
* This file contains privilege routines for io backends.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date        Changes
* ----- ------  --------    ---------------------------------------------------
* 1.0   Wendy 05/17/2021  Initial creation
*
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include <stdlib.h>

#include "xaie_clock.h"
#include "xaie_feature_config.h"
#include "xaie_helper.h"
#include "xaie_io_privilege.h"
#include "xaie_npi.h"

#ifdef XAIE_FEATURE_PRIVILEGED_ENABLE

/*****************************************************************************/
/***************************** Macro Definitions *****************************/

#define XAIE_ISOLATE_EAST_MASK	(1U << 3)
#define XAIE_ISOLATE_NORTH_MASK	(1U << 2)
#define XAIE_ISOLATE_WEST_MASK	(1U << 1)
#define XAIE_ISOLATE_SOUTH_MASK	(1U << 0)
#define XAIE_ISOLATE_ALL_MASK	((1U << 4) - 1)

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API set the tile column reset
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE SHIM tile
* @param	RstEnable: XAIE_ENABLE to assert reset, XAIE_DISABLE to
*			   deassert reset.
*
* @return       XAIE_OK on success, error code on failure
*
* @note		It is not required to check the DevInst and the Loc tile type
*		as the caller function should provide the correct value.
*		This function is internal to this file.
*
******************************************************************************/
static AieRC _XAie_PrivilegeSetColReset(XAie_DevInst *DevInst,
		XAie_LocType Loc, u8 RstEnable)
{
	u8 TileType;
	u32 FldVal;
	u64 RegAddr;
	const XAie_PlIfMod *PlIfMod;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	PlIfMod = DevInst->DevProp.DevMod[TileType].PlIfMod;
	RegAddr = PlIfMod->ColRstOff +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);
	FldVal = XAie_SetField(RstEnable,
			PlIfMod->ColRst.Lsb,
			PlIfMod->ColRst.Mask);

	return XAie_Write32(DevInst, RegAddr, FldVal);
}

/*****************************************************************************/
/**
*
* This API set the tile columns reset for every column in the partition
*
* @param	DevInst: Device Instance
* @param	RstEnable: XAIE_ENABLE to assert reset, XAIE_DISABLE to
*			   deassert reset.
*
* @return       XAIE_OK on success, error code on failure
*
* @note		It is not required to check the DevInst as the caller function
*		should provide the correct value.
*		This function is internal to this file.
*
******************************************************************************/
static AieRC _XAie_PrivilegeSetPartColReset(XAie_DevInst *DevInst,
		u8 RstEnable)
{
	AieRC RC = XAIE_OK;

	for(u32 C = 0; C < DevInst->NumCols; C++) {
		XAie_LocType Loc = XAie_TileLoc(C, 0);

		RC = _XAie_PrivilegeSetColReset(DevInst, Loc, RstEnable);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to reset columns.\n");
			break;
		}
	}

	return RC;
}
/*****************************************************************************/
/**
*
* This API reset all SHIMs in the AI engine partition
*
* @param	DevInst: Device Instance
*
* @return	XAIE_OK for success, and error value for failure
*
* @note		This function asserts reset, and then deassert it.
*		It is not required to check the DevInst as the caller function
*		should provide the correct value.
*		This function is internal to this file.
*
******************************************************************************/
static AieRC _XAie_PrivilegeRstPartShims(XAie_DevInst *DevInst)
{
	AieRC RC;

	RC = DevInst->DevOps->SetPartColShimReset(DevInst, XAIE_ENABLE);
	if(RC != XAIE_OK) {
		return RC;
	}

	RC = _XAie_NpiSetShimReset(DevInst, XAIE_ENABLE);
	if(RC != XAIE_OK) {
		return RC;
	}

	return _XAie_NpiSetShimReset(DevInst, XAIE_DISABLE);
}

/*****************************************************************************/
/**
*
* This API sets to block NSU AXI MM slave error and decode error based on user
* inputs. If NSU errors is blocked, it will only generate error events.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE SHIM tile
* @param	BlockSlvEnable: XAIE_ENABLE to block NSU AXI MM Slave errors,
*				or XAIE_DISABLE to unblock.
* @param	BlockDecEnable: XAIE_ENABLE to block NSU AXI MM Decode errors,
*				or XAIE_DISABLE to unblock.
*
* @return       XAIE_OK on success, error code on failure
*
* @note		It is not required to check the DevInst and the Loc tile type
*		as the caller function should provide the correct value.
*		This function is internal to this file.
*
******************************************************************************/
static AieRC _XAie_PrivilegeSetBlockAxiMmNsuErr(XAie_DevInst *DevInst,
		XAie_LocType Loc, u8 BlockSlvEnable, u8 BlockDecEnable)
{
	u8 TileType;
	u32 FldVal;
	u64 RegAddr;
	const XAie_PlIfMod *PlIfMod;
	const XAie_ShimNocAxiMMConfig *ShimNocAxiMM;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	PlIfMod = DevInst->DevProp.DevMod[TileType].PlIfMod;
	ShimNocAxiMM = PlIfMod->ShimNocAxiMM;
	RegAddr = ShimNocAxiMM->RegOff +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);
	FldVal = XAie_SetField(BlockSlvEnable,
			ShimNocAxiMM->NsuSlvErr.Lsb,
			ShimNocAxiMM->NsuSlvErr.Mask);
	FldVal |= XAie_SetField(BlockDecEnable,
			ShimNocAxiMM->NsuDecErr.Lsb,
			ShimNocAxiMM->NsuDecErr.Mask);

	return XAie_Write32(DevInst, RegAddr, FldVal);
}

/*****************************************************************************/
/**
*
* This API sets to block the NSU AXI MM slave error and decode error config
* for all the SHIM NOCs in the full partition based on user inputs.
*
* @param	DevInst: Device Instance
* @param	BlockSlvEnable: XAIE_ENABLE to block NSU AXI MM Slave errors,
*				or XAIE_DISABLE to unblock.
* @param	BlockDecEnable: XAIE_ENABLE to block NSU AXI MM Decode errors,
*				or XAIE_DISABLE to unblock.
*
* @return       XAIE_OK on success, error code on failure
*
* @note		It is not required to check the DevInst as the caller function
*		should provide the correct value.
*		This function will do the following steps:
*		 * set AXI MM registers NSU errors fields in all SHIM NOC tiles
*		This function is internal to this file.
*
******************************************************************************/
static
AieRC _XAie_PrivilegeSetPartBlockAxiMmNsuErr(XAie_DevInst *DevInst,
		u8 BlockSlvEnable, u8 BlockDecEnable)
{
	AieRC RC = XAIE_OK;

	for(u32 C = 0; C < DevInst->NumCols; C++) {
		XAie_LocType Loc = XAie_TileLoc(C, 0);
		u8 TileType;

		TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
		if(TileType != XAIEGBL_TILE_TYPE_SHIMNOC) {
			continue;
		}
		RC = _XAie_PrivilegeSetBlockAxiMmNsuErr(DevInst, Loc,
				BlockSlvEnable, BlockDecEnable);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to set SHIM NOC AXI MM Errors.");
			return RC;
		}
	}

	return RC;
}

/*****************************************************************************/
/**
* This API sets partition NPI protected register enabling
*
* @param	DevInst: AI engine partition device instance pointer
* @param	Enable: XAIE_ENABLE to enable access to protected register.
*			XAIE_DISABLE to disable access.
*
* @return       XAIE_OK on success, error code on failure
*
* @note		This function is internal to this file.
*
*******************************************************************************/
static AieRC _XAie_PrivilegeSetPartProtectedRegs(XAie_DevInst *DevInst,
		u8 Enable)
{
	AieRC RC;
	XAie_NpiProtRegReq NpiProtReq = {0};

	NpiProtReq.NumCols = DevInst->NumCols;
	NpiProtReq.Enable = Enable;
	RC = _XAie_NpiSetProtectedRegEnable(DevInst, &NpiProtReq);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Failed to set protected registers.\n");
	}

	return RC;
}

/*****************************************************************************/
/**
* This API initializes the AI engine partition
*
* @param	DevInst: AI engine partition device instance pointer
* @param	Opts: Initialization options
*
* @return       XAIE_OK on success, error code on failure
*
* @note		This operation does the following steps to initialize an AI
*		engine partition:
*		- Clock gate all columns
*		- Reset Columns
*		- Reset shims
*		- Setup AXI MM not to return errors for AXI decode or slave
*		  errors, raise events instead.
*		- ungate all columns
*		- Setup partition isolation.
*		- zeroize memory if it is requested
*
*******************************************************************************/
AieRC _XAie_PrivilegeInitPart(XAie_DevInst *DevInst, XAie_PartInitOpts *Opts)
{
	u32 OptFlags;
	AieRC RC;

	if(Opts != NULL) {
		OptFlags = Opts->InitOpts;
	} else {
		OptFlags = XAIE_PART_INIT_OPT_DEFAULT;
	}

	RC = _XAie_PrivilegeSetPartProtectedRegs(DevInst, XAIE_ENABLE);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Failed to initialize partition, enable protected registers failed.\n");
		return RC;
	}

	if((OptFlags & XAIE_PART_INIT_OPT_COLUMN_RST) != 0) {
		/* Always gate all tiles before resetting columns */
		RC = _XAie_PmSetPartitionClock(DevInst, XAIE_DISABLE);
		if(RC != XAIE_OK) {
			_XAie_PrivilegeSetPartProtectedRegs(DevInst, XAIE_DISABLE);
			return RC;
		}

		RC = _XAie_PrivilegeSetPartColReset(DevInst, XAIE_ENABLE);
		if(RC != XAIE_OK) {
			_XAie_PrivilegeSetPartProtectedRegs(DevInst, XAIE_DISABLE);
			return RC;
		}

		RC = _XAie_PrivilegeSetPartColReset(DevInst, XAIE_DISABLE);
		if(RC != XAIE_OK) {
			_XAie_PrivilegeSetPartProtectedRegs(DevInst, XAIE_DISABLE);
			return RC;
		}

	}

	if((OptFlags & XAIE_PART_INIT_OPT_SHIM_RST) != 0) {
		RC = _XAie_PrivilegeRstPartShims(DevInst);
		if(RC != XAIE_OK) {
			_XAie_PrivilegeSetPartProtectedRegs(DevInst, XAIE_DISABLE);
			return RC;
		}
	}

	if((OptFlags & XAIE_PART_INIT_OPT_BLOCK_NOCAXIMMERR) != 0) {
		RC = _XAie_PrivilegeSetPartBlockAxiMmNsuErr(DevInst,
			XAIE_ENABLE, XAIE_ENABLE);
		if(RC != XAIE_OK) {
			_XAie_PrivilegeSetPartProtectedRegs(DevInst, XAIE_DISABLE);
			return RC;
		}
	}

	RC = DevInst->DevOps->SetPartColClockAfterRst(DevInst, XAIE_ENABLE);
	if(RC != XAIE_OK) {
		_XAie_PrivilegeSetPartProtectedRegs(DevInst, XAIE_DISABLE);
		return RC;
	}

	if ((OptFlags & XAIE_PART_INIT_OPT_ISOLATE) != 0) {
		RC = DevInst->DevOps->SetPartIsolationAfterRst(DevInst);
		if(RC != XAIE_OK) {
			_XAie_PrivilegeSetPartProtectedRegs(DevInst, XAIE_DISABLE);
		}
	}

	if ((OptFlags & XAIE_PART_INIT_OPT_ZEROIZEMEM) != 0) {
		RC = DevInst->DevOps->PartMemZeroInit(DevInst);
		if(RC != XAIE_OK) {
			_XAie_PrivilegeSetPartProtectedRegs(DevInst, XAIE_DISABLE);
			return RC;
		}
	}

	return _XAie_PrivilegeSetPartProtectedRegs(DevInst, XAIE_DISABLE);
}

/*****************************************************************************/
/**
* This API tears down the AI engine partition
*
* @param	DevInst: AI engine partition device instance pointer
* @param	Opts: Initialization options
*
* @return       XAIE_OK on success, error code on failure
*
* @note		This operation does the following steps to initialize an AI
*		engine partition:
*		- Clock gate all columns
*		- Reset Columns
*		- Reset shims
*		- zeroize memories
*		- Clock gate all columns
*
*******************************************************************************/
AieRC _XAie_PrivilegeTeardownPart(XAie_DevInst *DevInst)
{
	AieRC RC;

	RC = _XAie_PrivilegeSetPartProtectedRegs(DevInst, XAIE_ENABLE);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Failed to teardown partition, enable protected registers failed.\n");
		return RC;
	}

	RC = _XAie_PmSetPartitionClock(DevInst, XAIE_DISABLE);
	if(RC != XAIE_OK) {
		_XAie_PrivilegeSetPartProtectedRegs(DevInst, XAIE_DISABLE);
		return RC;
	}

	RC = _XAie_PrivilegeSetPartColReset(DevInst, XAIE_ENABLE);
	if(RC != XAIE_OK) {
		_XAie_PrivilegeSetPartProtectedRegs(DevInst, XAIE_DISABLE);
		return RC;
	}

	RC = _XAie_PrivilegeSetPartColReset(DevInst, XAIE_DISABLE);
	if(RC != XAIE_OK) {
		_XAie_PrivilegeSetPartProtectedRegs(DevInst, XAIE_DISABLE);
		return RC;
	}

	RC = _XAie_PrivilegeRstPartShims(DevInst);
	if(RC != XAIE_OK) {
		_XAie_PrivilegeSetPartProtectedRegs(DevInst, XAIE_DISABLE);
		return RC;
	}

	RC = DevInst->DevOps->SetPartColClockAfterRst(DevInst, XAIE_ENABLE);
	if(RC != XAIE_OK) {
		_XAie_PrivilegeSetPartProtectedRegs(DevInst, XAIE_DISABLE);
		return RC;
	}

	RC = DevInst->DevOps->PartMemZeroInit(DevInst);
	if (RC != XAIE_OK) {
		_XAie_PrivilegeSetPartProtectedRegs(DevInst, XAIE_DISABLE);
		return RC;
	}

	RC = _XAie_PmSetPartitionClock(DevInst, XAIE_DISABLE);
	if(RC != XAIE_OK) {
		_XAie_PrivilegeSetPartProtectedRegs(DevInst, XAIE_DISABLE);
		return RC;
	}

	return _XAie_PrivilegeSetPartProtectedRegs(DevInst, XAIE_DISABLE);
}

/*****************************************************************************/
/**
* This API enables clock for all the tiles passed as argument to this API.
*
* @param	DevInst: AI engine partition device instance pointer
* @param	Args: Backend tile args
*
* @return       XAIE_OK on success, error code on failure
*
* @note		Internal only.
*
*******************************************************************************/
AieRC _XAie_PrivilegeRequestTiles(XAie_DevInst *DevInst,
		XAie_BackendTilesArray *Args)
{
	AieRC RC;
	/* TODO: Configure previlege registers only for non-AIE devices. */
	if(DevInst->DevProp.DevGen != XAIE_DEV_GEN_AIE) {
		RC = _XAie_PrivilegeSetPartProtectedRegs(DevInst, XAIE_ENABLE);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to initialize partition, enable"
					" protected registers failed.\n");
			 return RC;
		}
	}

	RC = DevInst->DevOps->RequestTiles(DevInst, Args);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Request tiles failed\n");
	}

	if(DevInst->DevProp.DevGen != XAIE_DEV_GEN_AIE)
		_XAie_PrivilegeSetPartProtectedRegs(DevInst, XAIE_DISABLE);

	return RC;
}

#else /* XAIE_FEATURE_PRIVILEGED_ENABLE */
AieRC _XAie_PrivilegeInitPart(XAie_DevInst *DevInst, XAie_PartInitOpts *Opts)
{
	(void)DevInst;
	(void)Opts;
	return XAIE_FEATURE_NOT_SUPPORTED;
}

AieRC _XAie_PrivilegeTeardownPart(XAie_DevInst *DevInst)
{
	(void)DevInst;
	return XAIE_FEATURE_NOT_SUPPORTED;
}

AieRC _XAie_PrivilegeRequestTiles(XAie_DevInst *DevInst,
		XAie_BackendTilesArray *Args)
{
	(void)DevInst;
	(void)Args;
	return XAIE_FEATURE_NOT_SUPPORTED;
}
#endif /* XAIE_FEATURE_PRIVILEGED_ENABLE */
/** @} */
