/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_dma_aieml.h
* @{
*
* This file contains routines for AIEML DMA configuration and controls. This
* header file is not exposed to the user.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   03/23/2020  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XAIE_DMA_AIEML_H
#define XAIE_DMA_AIEML_H

/***************************** Include Files *********************************/
#include "xaiegbl.h"

/************************** Function Prototypes  *****************************/
void _XAieMl_ShimDmaInit(XAie_DmaDesc *Desc);
void _XAieMl_TileDmaInit(XAie_DmaDesc *Desc);
void _XAieMl_MemTileDmaInit(XAie_DmaDesc *Desc);
AieRC _XAieMl_DmaSetLock(XAie_DmaDesc *DmaDesc, XAie_Lock Acq, XAie_Lock Rel,
		u8 AcqEn, u8 RelEn);
AieRC _XAieMl_MemTileDmaWriteBd(XAie_DevInst *DevInst , XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u8 BdNum);
AieRC _XAieMl_TileDmaWriteBd(XAie_DevInst *DevInst , XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u8 BdNum);
AieRC _XAieMl_ShimDmaWriteBd(XAie_DevInst *DevInst , XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u8 BdNum);
AieRC _XAieMl_DmaSetMultiDim(XAie_DmaDesc *DmaDesc, XAie_DmaTensor *Tensor);
AieRC _XAieMl_DmaGetPendingBdCount(XAie_DevInst *DevInst, XAie_LocType Loc,
		const XAie_DmaMod *DmaMod, u8 ChNum, XAie_DmaDirection Dir,
		u8 *PendingBd);
AieRC _XAieMl_DmaWaitForDone(XAie_DevInst *DevInst, XAie_LocType Loc,
		const XAie_DmaMod *DmaMod, u8 ChNum, XAie_DmaDirection Dir,
		u32 TimeOutUs);
AieRC _XAieMl_DmaCheckBdChValidity(u8 BdNum, u8 ChNum);
AieRC _XAieMl_MemTileDmaCheckBdChValidity(u8 BdNum, u8 ChNum);
AieRC _XAieMl_DmaUpdateBdLen(XAie_DevInst *DevInst, const XAie_DmaMod *DmaMod,
		XAie_LocType Loc, u32 Len, u8 BdNum);
AieRC _XAieMl_ShimDmaUpdateBdLen(XAie_DevInst *DevInst,
		const XAie_DmaMod *DmaMod, XAie_LocType Loc, u32 Len, u8 BdNum);
AieRC _XAieMl_DmaUpdateBdAddr(XAie_DevInst *DevInst, const XAie_DmaMod *DmaMod,
		XAie_LocType Loc, u64 Addr, u8 BdNum);
AieRC _XAieMl_ShimDmaUpdateBdAddr(XAie_DevInst *DevInst,
		const XAie_DmaMod *DmaMod, XAie_LocType Loc, u64 Addr,
		u8 BdNum);
AieRC _XAieMl_DmaSetBdIteration(XAie_DmaDesc *DmaDesc, u32 StepSize, u8 Wrap,
		u8 IterCurr);

#endif /* XAIE_DMA_AIEML_H */
/** @} */
