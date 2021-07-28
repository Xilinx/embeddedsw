/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_dma_aie.c
* @{
*
* This file contains routines for AIE DMA configuration and controls.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   03/23/2020  Initial creation
* 1.1   Tejus   06/10/2020  Switch to new io backend apis.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_feature_config.h"
#include "xaie_helper.h"
#include "xaie_io.h"
#include "xaiegbl.h"
#include "xaiegbl_regdef.h"

#ifdef XAIE_FEATURE_DMA_ENABLE

/************************** Constant Definitions *****************************/
#define XAIE_DMA_TILEDMA_2DX_DEFAULT_INCR		0U
#define XAIE_DMA_TILEDMA_2DX_DEFAULT_WRAP 		255U
#define XAIE_DMA_TILEDMA_2DX_DEFAULT_OFFSET		1U
#define XAIE_DMA_TILEDMA_2DY_DEFAULT_INCR 		255U
#define XAIE_DMA_TILEDMA_2DY_DEFAULT_WRAP 		255U
#define XAIE_DMA_TILEDMA_2DY_DEFAULT_OFFSET		256U

#define XAIE_TILEDMA_NUM_BD_WORDS			7U
#define XAIE_SHIMDMA_NUM_BD_WORDS			5U

#define XAIE_TILE_DMA_NUM_DIMS_MAX			2U
#define XAIE_DMA_STATUS_IDLE				0x0U
/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API initializes the Dma Descriptor for AIE 1 Tile Dma.
*
* @param	Desc: Dma Descriptor
*
* @return	None.
*
* @note		Internal Only.
*
******************************************************************************/
void _XAie_TileDmaInit(XAie_DmaDesc *Desc)
{
	Desc->MultiDimDesc.AieMultiDimDesc.X_Incr = XAIE_DMA_TILEDMA_2DX_DEFAULT_INCR;
	Desc->MultiDimDesc.AieMultiDimDesc.X_Wrap = XAIE_DMA_TILEDMA_2DX_DEFAULT_WRAP;
	Desc->MultiDimDesc.AieMultiDimDesc.X_Offset = XAIE_DMA_TILEDMA_2DX_DEFAULT_OFFSET;
	Desc->MultiDimDesc.AieMultiDimDesc.Y_Incr = XAIE_DMA_TILEDMA_2DY_DEFAULT_INCR;
	Desc->MultiDimDesc.AieMultiDimDesc.Y_Wrap = XAIE_DMA_TILEDMA_2DY_DEFAULT_WRAP;
	Desc->MultiDimDesc.AieMultiDimDesc.Y_Offset = XAIE_DMA_TILEDMA_2DY_DEFAULT_OFFSET;

	return;
}

/*****************************************************************************/
/**
*
* This API initializes the Dma Descriptor for AIE 1 Shim Dma.
*
* @param	Desc: Dma Descriptor
*
* @return	None.
*
* @note		Internal Only.
*
******************************************************************************/
void _XAie_ShimDmaInit(XAie_DmaDesc *Desc)
{
	(void)Desc;
	return;
}

/*****************************************************************************/
/**
*
* This API initializes the Acquire and Release Locks for a a Dma for AIE
* descriptor.
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	Acq: Lock object with acquire lock ID and lock value.
* @param	Rel: Lock object with release lock ID and lock value.
* @param 	AcqEn: XAIE_ENABLE/XAIE_DISABLE for enable or disable acquire
*		lock.
* @param 	RelEn: XAIE_ENABLE/XAIE_DISABLE for enable or disable release
*		lock.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal Only. Should not be called directly. This function is
*		called from the internal Dma Module data structure.
*
******************************************************************************/
AieRC _XAie_DmaSetLock(XAie_DmaDesc *DmaDesc, XAie_Lock Acq, XAie_Lock Rel,
		u8 AcqEn, u8 RelEn)
{
	/* For AIE, Acquire and Release Lock IDs must be the same */
	if((Acq.LockId != Rel.LockId)) {
		XAIE_ERROR("Lock ID is invalid\n");
		return XAIE_INVALID_LOCK_ID;
	}

	DmaDesc->LockDesc.LockAcqId = Acq.LockId;
	DmaDesc->LockDesc.LockRelId = Rel.LockId;
	DmaDesc->LockDesc.LockAcqEn = AcqEn;
	DmaDesc->LockDesc.LockRelEn = RelEn;
	DmaDesc->LockDesc.LockRelValEn = 0U;
	DmaDesc->LockDesc.LockAcqValEn = 0U;

	/* If lock release value is invalid,then lock released with no value */
	if(Rel.LockVal != XAIE_LOCK_WITH_NO_VALUE) {
		DmaDesc->LockDesc.LockRelValEn = XAIE_ENABLE;
		DmaDesc->LockDesc.LockRelVal = Rel.LockVal;
	}

	/* If lock acquire value is invalid,then lock acquired with no value */
	if(Acq.LockVal != XAIE_LOCK_WITH_NO_VALUE) {
		DmaDesc->LockDesc.LockAcqValEn = XAIE_ENABLE;
		DmaDesc->LockDesc.LockAcqVal = Acq.LockVal;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API setups the DmaDesc with the register fields required for the dma
* addressing mode of AIE.
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	Tensor: Dma Tensor describing the address mode of dma.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal API only.
*
******************************************************************************/
AieRC _XAie_DmaSetMultiDim(XAie_DmaDesc *DmaDesc, XAie_DmaTensor *Tensor)
{
	DmaDesc->MultiDimDesc.AieMultiDimDesc.X_Offset =
		Tensor->Dim[0].AieDimDesc.Offset;
	DmaDesc->MultiDimDesc.AieMultiDimDesc.X_Wrap =
		Tensor->Dim[0].AieDimDesc.Wrap - 1U;
	DmaDesc->MultiDimDesc.AieMultiDimDesc.X_Incr =
		Tensor->Dim[0].AieDimDesc.Incr - 1U;
	DmaDesc->MultiDimDesc.AieMultiDimDesc.Y_Offset =
		Tensor->Dim[1].AieDimDesc.Offset;
	DmaDesc->MultiDimDesc.AieMultiDimDesc.Y_Wrap =
		Tensor->Dim[1].AieDimDesc.Wrap - 1U;
	DmaDesc->MultiDimDesc.AieMultiDimDesc.Y_Incr =
		Tensor->Dim[1].AieDimDesc.Incr - 1U;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API setups DmaDesc with parameters to run dma in interleave mode for AIE
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	DoubleBuff: Double buffer to use(0 - A, 1-B)
* @param	IntrleaveCount: Interleaved count to use(to be 32b word aligned)
* @param	IntrleaveCurr: Interleave current pointer.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only. The API sets up the value in the dma descriptor
*		and does not configure the buffer descriptor field in the
*		hardware.
*
******************************************************************************/
AieRC _XAie_DmaSetInterleaveEnable(XAie_DmaDesc *DmaDesc, u8 DoubleBuff,
		u8 IntrleaveCount, u16 IntrleaveCurr)
{
	DmaDesc->MultiDimDesc.AieMultiDimDesc.EnInterleaved = XAIE_ENABLE;
	DmaDesc->MultiDimDesc.AieMultiDimDesc.IntrleaveBufSelect = DoubleBuff;
	DmaDesc->MultiDimDesc.AieMultiDimDesc.IntrleaveCount = IntrleaveCount;
	DmaDesc->MultiDimDesc.AieMultiDimDesc.CurrPtr = IntrleaveCurr;

	return XAIE_OK;
}
/*****************************************************************************/
/**
*
* This API writes a Dma Descriptor which is initialized and setup by other APIs
* into the corresponding registers and register fields in the hardware. This API
* is specific to AIE Shim Tiles only.
*
* @param	DevInst: Device Instance
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	Loc: Location of AIE Tile
* @param	BdNum: Hardware BD number to be written to.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only. For AIE Shim Tiles only.
*
******************************************************************************/
AieRC _XAie_ShimDmaWriteBd(XAie_DevInst *DevInst , XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u8 BdNum)
{
	u64 Addr;
	u64 BdBaseAddr;
	u32 BdWord[XAIE_SHIMDMA_NUM_BD_WORDS];
	XAie_ShimDmaBdArgs Args;
	const XAie_DmaMod *DmaMod;
	const XAie_DmaBdProp *BdProp;

	DmaMod = DevInst->DevProp.DevMod[DmaDesc->TileType].DmaMod;
	BdProp = DmaMod->BdProp;

	BdBaseAddr = DmaMod->BaseAddr + BdNum * DmaMod->IdxOffset;

	BdWord[0U] = XAie_SetField(DmaDesc->AddrDesc.Address,
			BdProp->Buffer->ShimDmaBuff.AddrLow.Lsb,
			BdProp->Buffer->ShimDmaBuff.AddrLow.Mask);

	BdWord[1U] = XAie_SetField(DmaDesc->AddrDesc.Length,
			BdProp->Buffer->ShimDmaBuff.BufferLen.Lsb,
			BdProp->Buffer->ShimDmaBuff.BufferLen.Mask);

	BdWord[2U] = XAie_SetField((DmaDesc->AddrDesc.Address >> 32U),
			BdProp->Buffer->ShimDmaBuff.AddrHigh.Lsb,
			BdProp->Buffer->ShimDmaBuff.AddrHigh.Mask) |
		XAie_SetField(DmaDesc->BdEnDesc.UseNxtBd,
				BdProp->BdEn->UseNxtBd.Lsb,
				BdProp->BdEn->UseNxtBd.Mask) |
		XAie_SetField(DmaDesc->BdEnDesc.NxtBd,
				BdProp->BdEn->NxtBd.Lsb,
				BdProp->BdEn->NxtBd.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqId,
				BdProp->Lock->AieDmaLock.LckId_A.Lsb,
				BdProp->Lock->AieDmaLock.LckId_A.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockRelEn,
				BdProp->Lock->AieDmaLock.LckRelEn_A.Lsb,
				BdProp->Lock->AieDmaLock.LckRelEn_A.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockRelVal,
				BdProp->Lock->AieDmaLock.LckRelVal_A.Lsb,
				BdProp->Lock->AieDmaLock.LckRelVal_A.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockRelValEn,
				BdProp->Lock->AieDmaLock.LckRelUseVal_A.Lsb,
				BdProp->Lock->AieDmaLock.LckRelUseVal_A.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqEn,
				BdProp->Lock->AieDmaLock.LckAcqEn_A.Lsb,
				BdProp->Lock->AieDmaLock.LckAcqEn_A.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqVal,
				BdProp->Lock->AieDmaLock.LckAcqVal_A.Lsb,
				BdProp->Lock->AieDmaLock.LckAcqVal_A.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqValEn,
				BdProp->Lock->AieDmaLock.LckAcqUseVal_A.Lsb,
				BdProp->Lock->AieDmaLock.LckAcqUseVal_A.Mask) |
		XAie_SetField(DmaDesc->BdEnDesc.ValidBd,
				BdProp->BdEn->ValidBd.Lsb,
				BdProp->BdEn->ValidBd.Mask);

	BdWord[3U] = XAie_SetField(DmaDesc->AxiDesc.SMID,
			BdProp->SysProp->SMID.Lsb, BdProp->SysProp->SMID.Mask) |
		XAie_SetField(DmaDesc->AxiDesc.BurstLen,
				BdProp->SysProp->BurstLen.Lsb,
				BdProp->SysProp->BurstLen.Mask) |
		XAie_SetField(DmaDesc->AxiDesc.AxQos,
				BdProp->SysProp->AxQos.Lsb,
				BdProp->SysProp->AxQos.Mask) |
		XAie_SetField(DmaDesc->AxiDesc.SecureAccess,
				BdProp->SysProp->SecureAccess.Lsb,
				BdProp->SysProp->SecureAccess.Mask) |
		XAie_SetField(DmaDesc->AxiDesc.AxCache,
				BdProp->SysProp->AxCache.Lsb,
				BdProp->SysProp->AxCache.Mask);

	BdWord[4U] = XAie_SetField(DmaDesc->PktDesc.PktId,
			BdProp->Pkt->PktId.Lsb, BdProp->Pkt->PktId.Mask) |
		XAie_SetField(DmaDesc->PktDesc.PktType,
				BdProp->Pkt->PktType.Lsb,
				BdProp->Pkt->PktType.Mask) |
		XAie_SetField(DmaDesc->PktDesc.PktEn,
				BdProp->Pkt->EnPkt.Lsb,
				BdProp->Pkt->EnPkt.Mask);

	Addr = BdBaseAddr + _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	Args.NumBdWords = XAIE_SHIMDMA_NUM_BD_WORDS;
	Args.BdWords = &BdWord[0U];
	Args.Loc = Loc;
	Args.VAddr = DmaDesc->AddrDesc.Address;
	Args.BdNum = BdNum;
	Args.Addr = Addr;
	Args.MemInst = DmaDesc->MemInst;

	XAie_RunOp(DevInst, XAIE_BACKEND_OP_CONFIG_SHIMDMABD, (void *)&Args);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API writes a Dma Descriptor which is initialized and setup by other APIs
* into the corresponding registers and register fields in the hardware. This API
* is specific to AIE Tiles only.
*
* @param	DevInst: Device Instance
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	Loc: Location of AIE Tile
* @param	BdNum: Hardware BD number to be written to.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only. For AIE Tiles only.
*
******************************************************************************/
AieRC _XAie_TileDmaWriteBd(XAie_DevInst *DevInst , XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u8 BdNum)
{
	AieRC RC;
	u64 Addr;
	u64 BdBaseAddr;
	u32 BdWord[XAIE_TILEDMA_NUM_BD_WORDS];
	const XAie_DmaMod *DmaMod;
	const XAie_DmaBdProp *BdProp;

	DmaMod = DevInst->DevProp.DevMod[DmaDesc->TileType].DmaMod;
	BdProp = DmaMod->BdProp;

	BdBaseAddr = DmaMod->BaseAddr + BdNum * DmaMod->IdxOffset;

	/* AcqLockId and RelLockId are the same in AIE */
	BdWord[0U] = XAie_SetField(DmaDesc->LockDesc.LockAcqId,
			BdProp->Lock->AieDmaLock.LckId_A.Lsb,
			BdProp->Lock->AieDmaLock.LckId_A.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockRelEn,
				BdProp->Lock->AieDmaLock.LckRelEn_A.Lsb,
				BdProp->Lock->AieDmaLock.LckRelEn_A.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockRelVal,
				BdProp->Lock->AieDmaLock.LckRelVal_A.Lsb,
				BdProp->Lock->AieDmaLock.LckRelVal_A.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockRelValEn,
				BdProp->Lock->AieDmaLock.LckRelUseVal_A.Lsb,
				BdProp->Lock->AieDmaLock.LckRelUseVal_A.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqEn,
				BdProp->Lock->AieDmaLock.LckAcqEn_A.Lsb,
				BdProp->Lock->AieDmaLock.LckAcqEn_A.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqVal,
				BdProp->Lock->AieDmaLock.LckAcqVal_A.Lsb,
				BdProp->Lock->AieDmaLock.LckAcqVal_A.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqValEn,
				BdProp->Lock->AieDmaLock.LckAcqUseVal_A.Lsb,
				BdProp->Lock->AieDmaLock.LckAcqUseVal_A.Mask) |
		XAie_SetField(DmaDesc->AddrDesc.Address,
				BdProp->Buffer->TileDmaBuff.BaseAddr.Lsb,
				BdProp->Buffer->TileDmaBuff.BaseAddr.Mask);

	BdWord[1U] = XAie_SetField(DmaDesc->LockDesc_2.LockAcqId,
			BdProp->Lock->AieDmaLock.LckId_B.Lsb,
			BdProp->Lock->AieDmaLock.LckId_B.Mask) |
		XAie_SetField(DmaDesc->LockDesc_2.LockRelEn,
				BdProp->Lock->AieDmaLock.LckRelEn_B.Lsb,
				BdProp->Lock->AieDmaLock.LckRelEn_B.Mask) |
		XAie_SetField(DmaDesc->LockDesc_2.LockRelVal,
				BdProp->Lock->AieDmaLock.LckRelVal_B.Lsb,
				BdProp->Lock->AieDmaLock.LckRelVal_B.Mask) |
		XAie_SetField(DmaDesc->LockDesc_2.LockRelValEn,
				BdProp->Lock->AieDmaLock.LckRelUseVal_B.Lsb,
				BdProp->Lock->AieDmaLock.LckRelUseVal_B.Mask) |
		XAie_SetField(DmaDesc->LockDesc_2.LockAcqEn,
				BdProp->Lock->AieDmaLock.LckAcqEn_B.Lsb,
				BdProp->Lock->AieDmaLock.LckAcqEn_B.Mask) |
		XAie_SetField(DmaDesc->LockDesc_2.LockAcqVal,
				BdProp->Lock->AieDmaLock.LckAcqVal_B.Lsb,
				BdProp->Lock->AieDmaLock.LckAcqVal_B.Mask) |
		XAie_SetField(DmaDesc->LockDesc_2.LockAcqValEn,
				BdProp->Lock->AieDmaLock.LckAcqUseVal_B.Lsb,
				BdProp->Lock->AieDmaLock.LckAcqUseVal_B.Mask) |
		XAie_SetField(DmaDesc->AddrDesc_2.Address,
				BdProp->DoubleBuffer->BaseAddr_B.Lsb,
				BdProp->DoubleBuffer->BaseAddr_B.Mask);

	BdWord[2U] = XAie_SetField(
			DmaDesc->MultiDimDesc.AieMultiDimDesc.X_Incr,
			BdProp->AddrMode->AieMultiDimAddr.X_Incr.Lsb,
			BdProp->AddrMode->AieMultiDimAddr.X_Incr.Mask) |
		XAie_SetField(DmaDesc->MultiDimDesc.AieMultiDimDesc.X_Wrap,
				BdProp->AddrMode->AieMultiDimAddr.X_Wrap.Lsb,
				BdProp->AddrMode->AieMultiDimAddr.X_Wrap.Mask) |
		XAie_SetField(DmaDesc->MultiDimDesc.AieMultiDimDesc.X_Offset,
				BdProp->AddrMode->AieMultiDimAddr.X_Offset.Lsb,
				BdProp->AddrMode->AieMultiDimAddr.X_Offset.Mask);

	BdWord[3U] = XAie_SetField(
			DmaDesc->MultiDimDesc.AieMultiDimDesc.Y_Incr,
			BdProp->AddrMode->AieMultiDimAddr.Y_Incr.Lsb,
			BdProp->AddrMode->AieMultiDimAddr.Y_Incr.Mask) |
		XAie_SetField(DmaDesc->MultiDimDesc.AieMultiDimDesc.X_Wrap,
				BdProp->AddrMode->AieMultiDimAddr.Y_Wrap.Lsb,
				BdProp->AddrMode->AieMultiDimAddr.Y_Wrap.Mask) |
		XAie_SetField(DmaDesc->MultiDimDesc.AieMultiDimDesc.Y_Offset,
				BdProp->AddrMode->AieMultiDimAddr.Y_Offset.Lsb,
				BdProp->AddrMode->AieMultiDimAddr.Y_Offset.Mask);

	BdWord[4U] = XAie_SetField(DmaDesc->PktDesc.PktId,
			BdProp->Pkt->PktId.Lsb, BdProp->Pkt->PktId.Mask) |
		XAie_SetField(DmaDesc->PktDesc.PktType,
				BdProp->Pkt->PktType.Lsb,
				BdProp->Pkt->PktType.Mask);

	BdWord[5U] = XAie_SetField(
			DmaDesc->MultiDimDesc.AieMultiDimDesc.IntrleaveBufSelect,
			BdProp->DoubleBuffer->BuffSelect.Lsb,
			BdProp->DoubleBuffer->BuffSelect.Mask) |
		XAie_SetField(DmaDesc->MultiDimDesc.AieMultiDimDesc.CurrPtr,
				BdProp->AddrMode->AieMultiDimAddr.CurrPtr.Lsb,
				BdProp->AddrMode->AieMultiDimAddr.CurrPtr.Mask);

	BdWord[6U] = XAie_SetField(DmaDesc->BdEnDesc.ValidBd,
			BdProp->BdEn->ValidBd.Lsb, BdProp->BdEn->ValidBd.Mask) |
		XAie_SetField(DmaDesc->BdEnDesc.UseNxtBd,
				BdProp->BdEn->UseNxtBd.Lsb,
				BdProp->BdEn->UseNxtBd.Mask) |
		XAie_SetField(DmaDesc->EnDoubleBuff,
				BdProp->DoubleBuffer->EnDoubleBuff.Lsb,
				BdProp->DoubleBuffer->EnDoubleBuff.Mask) |
		XAie_SetField(DmaDesc->FifoMode,
				BdProp->DoubleBuffer->FifoMode.Lsb,
				BdProp->DoubleBuffer->FifoMode.Mask) |
		XAie_SetField(DmaDesc->PktDesc.PktEn, BdProp->Pkt->EnPkt.Lsb,
				BdProp->Pkt->EnPkt.Mask) |
		XAie_SetField(DmaDesc->MultiDimDesc.AieMultiDimDesc.EnInterleaved,
				BdProp->DoubleBuffer->EnIntrleaved.Lsb,
				BdProp->DoubleBuffer->EnIntrleaved.Mask) |
		XAie_SetField((DmaDesc->MultiDimDesc.AieMultiDimDesc.IntrleaveCount - 1U),
				BdProp->DoubleBuffer->IntrleaveCnt.Lsb,
				BdProp->DoubleBuffer->IntrleaveCnt.Mask) |
		XAie_SetField(DmaDesc->BdEnDesc.NxtBd,
				BdProp->BdEn->NxtBd.Lsb,
				BdProp->BdEn->NxtBd.Mask) |
		XAie_SetField(DmaDesc->AddrDesc.Length,
				BdProp->Buffer->TileDmaBuff.BufferLen.Lsb,
				BdProp->Buffer->TileDmaBuff.BufferLen.Mask);

	Addr = BdBaseAddr + _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	for(u8 i = 0U; i < XAIE_TILEDMA_NUM_BD_WORDS; i++) {
		RC = XAie_Write32(DevInst, Addr + i * 4U, BdWord[i]);
		if(RC != XAIE_OK) {
			return RC;
		}
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API is used to get the count of scheduled BDs in pending.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	DmaMod: Dma module pointer
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
* @param	PendingBd: Pointer to store the number of pending BDs.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only. For AIE Tiles only.
*
******************************************************************************/
AieRC _XAie_DmaGetPendingBdCount(XAie_DevInst *DevInst, XAie_LocType Loc,
		const XAie_DmaMod *DmaMod, u8 ChNum, XAie_DmaDirection Dir,
		u8 *PendingBd)
{
	AieRC RC;
	u64 Addr;
	u32 StatusReg, StartQSize, Stalled, Status;

	Addr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		DmaMod->ChStatusBase + Dir * DmaMod->ChStatusOffset;

	RC = XAie_Read32(DevInst, Addr, &StatusReg);
	if(RC != XAIE_OK) {
		return RC;
	}

	StartQSize = XAie_GetField(StatusReg,
			DmaMod->ChProp->DmaChStatus[ChNum].AieDmaChStatus.StartQSize.Lsb,
			DmaMod->ChProp->DmaChStatus[ChNum].AieDmaChStatus.StartQSize.Mask);

	if(StartQSize > DmaMod->ChProp->StartQSizeMax) {
		XAIE_ERROR("Invalid start queue size from register\n");
		return XAIE_ERR;
	}

	Status = XAie_GetField(StatusReg,
			DmaMod->ChProp->DmaChStatus[ChNum].AieDmaChStatus.Status.Lsb,
			DmaMod->ChProp->DmaChStatus[ChNum].AieDmaChStatus.Status.Mask);
	Stalled = XAie_GetField(StatusReg,
			DmaMod->ChProp->DmaChStatus[ChNum].AieDmaChStatus.Stalled.Lsb,
			DmaMod->ChProp->DmaChStatus[ChNum].AieDmaChStatus.Stalled.Mask);

	/* Check if BD is being used by a channel */
	if((Status != XAIE_DMA_STATUS_IDLE) || Stalled) {
		StartQSize++;
	}

	*PendingBd = StartQSize;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API is used to wait on Shim DMA channel to be completed.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	DmaMod: Dma module pointer
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
* @param        TimeOutUs - Minimum timeout value in micro seconds.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only. For AIE Tiles only.
*
******************************************************************************/
AieRC _XAie_DmaWaitForDone(XAie_DevInst *DevInst, XAie_LocType Loc,
		const XAie_DmaMod *DmaMod, u8 ChNum, XAie_DmaDirection Dir,
		u32 TimeOutUs)
{
	u64 Addr;
	u32 Mask, Value;

	Addr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		DmaMod->ChStatusBase + Dir * DmaMod->ChStatusOffset;
	Mask = DmaMod->ChProp->DmaChStatus[ChNum].AieDmaChStatus.Status.Mask |
		DmaMod->ChProp->DmaChStatus[ChNum].AieDmaChStatus.StartQSize.Mask |
		DmaMod->ChProp->DmaChStatus[ChNum].AieDmaChStatus.Stalled.Mask;

	/* This will check the stalled and start queue size bits to be zero */
	Value = XAIE_DMA_STATUS_IDLE <<
		DmaMod->ChProp->DmaChStatus[ChNum].AieDmaChStatus.Status.Lsb;

	if(XAie_MaskPoll(DevInst, Addr, Mask, Value, TimeOutUs) !=
			XAIE_OK) {
		XAIE_DBG("Wait for done timed out\n");
		return XAIE_ERR;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API is used to check the validity of Bd number and Channel number
* combination.
*
* @param	BdNum: Buffer descriptor number.
* @param	ChNum: Channel number of the DMA.
* @param        TimeOutUs - Minimum timeout value in micro seconds.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only. For AIE Tiles only. This is a dummy function as
*		there is no restriction in AIE.
*
******************************************************************************/
AieRC _XAie_DmaCheckBdChValidity(u8 BdNum, u8 ChNum)
{
	(void)BdNum;
	(void)ChNum;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API updates the length of the buffer descriptor in the tile dma module.
*
* @param	DevInst: Device Instance.
* @param	DmaMod: Dma module pointer
* @param	Loc: Location of AIE Tile
* @param	Len: Length of BD in bytes.
* @param	BdNum: Hardware BD number to be written to.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only. This API accesses the hardware directly and does
*		not operate on software descriptor.
******************************************************************************/
AieRC _XAie_DmaUpdateBdLen(XAie_DevInst *DevInst, const XAie_DmaMod *DmaMod,
		XAie_LocType Loc, u32 Len, u8 BdNum)
{
	u64 RegAddr;
	u32 RegVal, Mask;

	RegAddr = DmaMod->BaseAddr + BdNum * DmaMod->IdxOffset +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
	DmaMod->BdProp->Buffer->TileDmaBuff.BufferLen.Idx * 4U;

	Mask = DmaMod->BdProp->Buffer->TileDmaBuff.BufferLen.Mask;
	RegVal = XAie_SetField(Len,
			DmaMod->BdProp->Buffer->TileDmaBuff.BufferLen.Lsb,
			Mask);

	return XAie_MaskWrite32(DevInst, RegAddr, Mask, RegVal);
}

/*****************************************************************************/
/**
*
* This API updates the length of the buffer descriptor in the shim dma module.
*
* @param	DevInst: Device Instance.
* @param	DmaMod: Dma module pointer
* @param	Loc: Location of AIE Tile
* @param	Len: Length of BD in bytes.
* @param	BdNum: Hardware BD number to be written to.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only. This API accesses the hardware directly and does
*		not operate on software descriptor.
******************************************************************************/
AieRC _XAie_ShimDmaUpdateBdLen(XAie_DevInst *DevInst, const XAie_DmaMod *DmaMod,
		XAie_LocType Loc, u32 Len, u8 BdNum)
{
	u64 RegAddr;
	u32 RegVal;

	RegAddr = DmaMod->BaseAddr + BdNum * DmaMod->IdxOffset +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		DmaMod->BdProp->Buffer->ShimDmaBuff.BufferLen.Idx * 4U;

	RegVal = XAie_SetField(Len,
			DmaMod->BdProp->Buffer->ShimDmaBuff.BufferLen.Lsb,
			DmaMod->BdProp->Buffer->ShimDmaBuff.BufferLen.Mask);

	/* BD length register does not have other parameters */
	return XAie_Write32(DevInst, RegAddr, RegVal);
}

/*****************************************************************************/
/**
*
* This API updates the address of the buffer descriptor in the dma module.
*
* @param	DevInst: Device Instance.
* @param	DmaMod: Dma module pointer
* @param	Loc: Location of AIE Tile
* @param	Addr: Buffer address
* @param	BdNum: Hardware BD number to be written to.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only. This API accesses the hardware directly and does
*		not operate on software descriptor.
******************************************************************************/
AieRC _XAie_DmaUpdateBdAddr(XAie_DevInst *DevInst, const XAie_DmaMod *DmaMod,
		XAie_LocType Loc, u64 Addr, u8 BdNum)
{
	u64 RegAddr;
	u32 RegVal, Mask;

	RegAddr = DmaMod->BaseAddr + BdNum * DmaMod->IdxOffset +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		DmaMod->BdProp->Buffer->TileDmaBuff.BaseAddr.Idx * 4U;

	Mask = DmaMod->BdProp->Buffer->TileDmaBuff.BaseAddr.Mask;
	RegVal = XAie_SetField(Addr,
			DmaMod->BdProp->Buffer->TileDmaBuff.BaseAddr.Lsb, Mask);

	return XAie_MaskWrite32(DevInst, RegAddr, Mask, RegVal);
}

/*****************************************************************************/
/**
*
* This API updates the address of the buffer descriptor in the dma module.
*
* @param	DevInst: Device Instance.
* @param	DmaMod: Dma module pointer
* @param	Loc: Location of AIE Tile
* @param	Addr: Buffer address
* @param	BdNum: Hardware BD number to be written to.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only. This API accesses the hardware directly and does
*		not operate on software descriptor.
******************************************************************************/
AieRC _XAie_ShimDmaUpdateBdAddr(XAie_DevInst *DevInst,
		const XAie_DmaMod *DmaMod, XAie_LocType Loc, u64 Addr, u8 BdNum)
{
	AieRC RC;
	u64 RegAddr;
	u32 RegVal, Mask;

	RegAddr = DmaMod->BaseAddr + BdNum * DmaMod->IdxOffset +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		DmaMod->BdProp->Buffer->ShimDmaBuff.AddrLow.Idx * 4U;

	Mask = DmaMod->BdProp->Buffer->ShimDmaBuff.AddrLow.Mask;
	RegVal = XAie_SetField(Addr,
			DmaMod->BdProp->Buffer->ShimDmaBuff.AddrLow.Lsb, Mask);

	/* Addrlow maps to a single register without other fields. */
	RC =  XAie_Write32(DevInst, RegAddr, RegVal);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Failed to update lower 32 bits of address\n");
		return RC;
	}

	RegAddr = DmaMod->BaseAddr + BdNum * DmaMod->IdxOffset +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		DmaMod->BdProp->Buffer->ShimDmaBuff.AddrHigh.Idx * 4U;

	Mask = DmaMod->BdProp->Buffer->ShimDmaBuff.AddrHigh.Mask;
	RegVal = XAie_SetField(Addr,
			DmaMod->BdProp->Buffer->ShimDmaBuff.AddrHigh.Lsb, Mask);

	return XAie_MaskWrite32(DevInst, RegAddr, Mask, RegVal);
}

/*****************************************************************************/
/**
*
* This API setups the iteration parameters for a Buffer descriptor.
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	StepSize: Offset applied at each execution of the BD.
* @param	Wrap: Iteration Wrap.
* @param	IterCurr: Current iteration step. This field is incremented by
*		the hardware after BD is loaded.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*
******************************************************************************/
AieRC _XAie_DmaSetBdIteration(XAie_DmaDesc *DmaDesc, u32 StepSize, u8 Wrap,
		u8 IterCurr)
{
	(void)DmaDesc;
	(void)StepSize;
	(void)Wrap;
	(void)IterCurr;

	return XAIE_FEATURE_NOT_SUPPORTED;
}

#endif /* XAIE_FEATURE_DMA_ENABLE */

/** @} */
