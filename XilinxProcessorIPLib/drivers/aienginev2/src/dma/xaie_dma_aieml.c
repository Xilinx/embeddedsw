/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_dma_aieml.c
* @{
*
* This file contains routines for AIEML DMA configuration and controls.
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
#include "xaie_helper.h"
#include "xaiegbl_regdef.h"

/************************** Constant Definitions *****************************/
#define XAIEML_TILEDMA_NUM_BD_WORDS			6U
#define XAIEML_SHIMDMA_NUM_BD_WORDS			8U
#define XAIEML_MEMTILEDMA_NUM_BD_WORDS			8U
#define XAIEML_DMA_STEPSIZE_DEFAULT			1U
#define XAIEML_DMA_ITERWRAP_DEFAULT			1U

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API initializes the Dma Descriptor for AIEML Shim Dma.
*
* @param	Desc: Dma Descriptor
*
* @return	None.
*
* @note		Internal Only.
*
******************************************************************************/
void _XAieMl_ShimDmaInit(XAie_DmaDesc *Desc)
{
	for(u8 i = 0U; i < 3U; i++) {
		Desc->MultiDimDesc.Gen2MultiDimDesc.DimDesc[i].StepSize =
			XAIEML_DMA_STEPSIZE_DEFAULT;
	}

	Desc->MultiDimDesc.Gen2MultiDimDesc.IterDesc.Wrap =
		XAIEML_DMA_ITERWRAP_DEFAULT;
	Desc->MultiDimDesc.Gen2MultiDimDesc.IterDesc.StepSize =
		XAIEML_DMA_STEPSIZE_DEFAULT;
}

/*****************************************************************************/
/**
*
* This API initializes the Dma Descriptor for AIEML Tile Dma.
*
* @param	Desc: Dma Descriptor
*
* @return	None.
*
* @note		Internal Only.
*
******************************************************************************/
void _XAieMl_TileDmaInit(XAie_DmaDesc *Desc)
{
	for(u8 i = 0U; i < 3U; i++) {
		Desc->MultiDimDesc.Gen2MultiDimDesc.DimDesc[i].StepSize =
			XAIEML_DMA_STEPSIZE_DEFAULT;
	}

	Desc->MultiDimDesc.Gen2MultiDimDesc.IterDesc.Wrap =
		XAIEML_DMA_ITERWRAP_DEFAULT;
	Desc->MultiDimDesc.Gen2MultiDimDesc.IterDesc.StepSize =
		XAIEML_DMA_STEPSIZE_DEFAULT;
}

/*****************************************************************************/
/**
*
* This API initializes the Dma Descriptor for AIE 2 Mem Tile Dma.
*
* @param	Desc: Dma Descriptor
*
* @return	None.
*
* @note		Internal Only.
*
******************************************************************************/
void _XAieMl_MemTileDmaInit(XAie_DmaDesc *Desc)
{
	for(u8 i = 0U; i < 4U; i++) {
		Desc->MultiDimDesc.Gen2MultiDimDesc.DimDesc[i].StepSize =
			XAIEML_DMA_STEPSIZE_DEFAULT;
	}

	Desc->MultiDimDesc.Gen2MultiDimDesc.IterDesc.Wrap =
		XAIEML_DMA_ITERWRAP_DEFAULT;
	Desc->MultiDimDesc.Gen2MultiDimDesc.IterDesc.StepSize =
		XAIEML_DMA_STEPSIZE_DEFAULT;
}

/*****************************************************************************/
/**
*
* This API initializes the Acquire and Release Locks for a a Dma for AIEML
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
AieRC _XAieMl_DmaSetLock(XAie_DmaDesc *DmaDesc, XAie_Lock Acq, XAie_Lock Rel,
		u8 AcqEn, u8 RelEn)
{
	DmaDesc->LockDesc.LockAcqId = Acq.LockId;
	DmaDesc->LockDesc.LockRelId = Rel.LockId;
	DmaDesc->LockDesc.LockAcqEn = AcqEn;
	DmaDesc->LockDesc.LockRelEn = RelEn;
	DmaDesc->LockDesc.LockRelVal = Rel.LockVal;
	DmaDesc->LockDesc.LockAcqVal = Acq.LockVal;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API setups the DmaDesc with the register fields required for the dma
* addressing mode of AIEML.
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	Tensor: Dma Tensor describing the address mode of dma.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal API only.
*
******************************************************************************/
AieRC _XAieMl_DmaSetMultiDim(XAie_DmaDesc *DmaDesc, XAie_DmaTensor *Tensor)
{
	for(u8 i = 0U; i < Tensor->NumDim; i++) {

		DmaDesc->MultiDimDesc.Gen2MultiDimDesc.DimDesc[i].StepSize =
			Tensor->Dim[i].Aie2DimDesc.StepSize;
		DmaDesc->MultiDimDesc.Gen2MultiDimDesc.DimDesc[i].Wrap =
			Tensor->Dim[i].Aie2DimDesc.Wrap;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API writes a Dma Descriptor which is initialized and setup by other APIs
* into the corresponding registers and register fields in the hardware. This API
* is specific to AIEML Memory Tiles only.
*
* @param	DevInst: Device Instance
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	Loc: Location of AIE Tile
* @param	BdNum: Hardware BD number to be written to.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only. For AIEML Mem Tiles only.
*
******************************************************************************/
AieRC _XAieMl_MemTileDmaWriteBd(XAie_DevInst *DevInst , XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u8 BdNum)
{
	u64 Addr;
	u64 BdBaseAddr;
	u32 BdWord[XAIEML_MEMTILEDMA_NUM_BD_WORDS];
	const XAie_DmaMod *DmaMod;
	const XAie_DmaBdProp *BdProp;

	DmaMod = DevInst->DevProp.DevMod[DmaDesc->TileType].DmaMod;
	BdProp = DmaMod->BdProp;

	BdBaseAddr = DmaMod->BaseAddr + BdNum * DmaMod->IdxOffset;

	/* Setup BdWord with the right values from DmaDesc */
	BdWord[0U] = XAie_SetField(DmaDesc->PktDesc.PktEn,
			BdProp->Pkt->EnPkt.Lsb, BdProp->Pkt->EnPkt.Mask) |
		XAie_SetField(DmaDesc->PktDesc.PktType,
				BdProp->Pkt->PktType.Lsb,
				BdProp->Pkt->PktType.Mask) |
		XAie_SetField(DmaDesc->PktDesc.PktId, BdProp->Pkt->PktId.Lsb,
				BdProp->Pkt->PktId.Mask) |
		XAie_SetField(DmaDesc->BdEnDesc.OutofOrderBdId,
				BdProp->BdEn->OutofOrderBdId.Lsb,
				BdProp->BdEn->OutofOrderBdId.Mask) |
		XAie_SetField(DmaDesc->AddrDesc.Length,
				BdProp->Buffer->TileDmaBuff.BufferLen.Lsb,
				BdProp->Buffer->TileDmaBuff.BufferLen.Mask);

	BdWord[1U] = XAie_SetField(DmaDesc->ZeroPadDesc.D0_ZeroBefore,
			BdProp->ZeroPad->D0_ZeroBefore.Lsb,
			BdProp->ZeroPad->D0_ZeroBefore.Mask) |
		XAie_SetField(DmaDesc->BdEnDesc.NxtBd, BdProp->BdEn->NxtBd.Lsb,
				BdProp->BdEn->NxtBd.Mask) |
		XAie_SetField(DmaDesc->BdEnDesc.UseNxtBd,
				BdProp->BdEn->UseNxtBd.Lsb,
				BdProp->BdEn->UseNxtBd.Mask) |
		XAie_SetField(DmaDesc->AddrDesc.Address,
				BdProp->Buffer->TileDmaBuff.BaseAddr.Lsb,
				BdProp->Buffer->TileDmaBuff.BaseAddr.Mask);

	BdWord[2U] = XAie_SetField(DmaDesc->MultiDimDesc.Gen2MultiDimDesc.DimDesc[0U].Wrap,
			BdProp->AddrMode->Aie2MultiDimAddr.DmaDimProp[0U].Wrap.Lsb,
			BdProp->AddrMode->Aie2MultiDimAddr.DmaDimProp[0U].Wrap.Mask) |
		 XAie_SetField((DmaDesc->MultiDimDesc.Gen2MultiDimDesc.DimDesc[0U].StepSize - 1U),
				BdProp->AddrMode->Aie2MultiDimAddr.DmaDimProp[0U].StepSize.Lsb,
				BdProp->AddrMode->Aie2MultiDimAddr.DmaDimProp[0U].StepSize.Mask);

	BdWord[3U] = XAie_SetField(DmaDesc->ZeroPadDesc.D1_ZeroBefore,
			BdProp->ZeroPad->D1_ZeroBefore.Lsb,
			BdProp->ZeroPad->D1_ZeroBefore.Mask) |
		XAie_SetField(DmaDesc->MultiDimDesc.Gen2MultiDimDesc.DimDesc[1U].Wrap,
			BdProp->AddrMode->Aie2MultiDimAddr.DmaDimProp[1U].Wrap.Lsb,
			BdProp->AddrMode->Aie2MultiDimAddr.DmaDimProp[1U].Wrap.Mask) |
		XAie_SetField((DmaDesc->MultiDimDesc.Gen2MultiDimDesc.DimDesc[1U].StepSize - 1U),
				BdProp->AddrMode->Aie2MultiDimAddr.DmaDimProp[1U].StepSize.Lsb,
				BdProp->AddrMode->Aie2MultiDimAddr.DmaDimProp[1U].StepSize.Mask);

	BdWord[4U] = XAie_SetField(DmaDesc->ZeroPadDesc.D2_ZeroBefore,
			BdProp->ZeroPad->D2_ZeroBefore.Lsb,
			BdProp->ZeroPad->D2_ZeroBefore.Mask) |
		XAie_SetField(DmaDesc->MultiDimDesc.Gen2MultiDimDesc.DimDesc[2U].Wrap,
			BdProp->AddrMode->Aie2MultiDimAddr.DmaDimProp[2U].Wrap.Lsb,
			BdProp->AddrMode->Aie2MultiDimAddr.DmaDimProp[2U].Wrap.Mask) |
		XAie_SetField((DmaDesc->MultiDimDesc.Gen2MultiDimDesc.DimDesc[2U].StepSize - 1U),
				BdProp->AddrMode->Aie2MultiDimAddr.DmaDimProp[2U].StepSize.Lsb,
				BdProp->AddrMode->Aie2MultiDimAddr.DmaDimProp[2U].StepSize.Mask) |
		XAie_SetField(DmaDesc->EnCompression,
				BdProp->Compression->EnCompression.Lsb,
				BdProp->Compression->EnCompression.Mask);

	BdWord[5U] = XAie_SetField(DmaDesc->ZeroPadDesc.D2_ZeroAfter,
			BdProp->ZeroPad->D2_ZeroAfter.Lsb,
			BdProp->ZeroPad->D2_ZeroAfter.Mask) |
		XAie_SetField(DmaDesc->ZeroPadDesc.D1_ZeroAfter,
			BdProp->ZeroPad->D1_ZeroAfter.Lsb,
			BdProp->ZeroPad->D1_ZeroAfter.Mask) |
		XAie_SetField(DmaDesc->ZeroPadDesc.D0_ZeroAfter,
			BdProp->ZeroPad->D0_ZeroAfter.Lsb,
			BdProp->ZeroPad->D0_ZeroAfter.Mask) |
		XAie_SetField((DmaDesc->MultiDimDesc.Gen2MultiDimDesc.DimDesc[3U].StepSize - 1U),
				BdProp->AddrMode->Aie2MultiDimAddr.DmaDimProp[3U].StepSize.Lsb,
				BdProp->AddrMode->Aie2MultiDimAddr.DmaDimProp[3U].StepSize.Mask);

	BdWord[6U] = XAie_SetField(DmaDesc->MultiDimDesc.Gen2MultiDimDesc.IterCurr,
			BdProp->AddrMode->Aie2MultiDimAddr.IterCurr.Lsb,
			BdProp->AddrMode->Aie2MultiDimAddr.IterCurr.Mask) |
		XAie_SetField((DmaDesc->MultiDimDesc.Gen2MultiDimDesc.IterDesc.Wrap - 1U),
				BdProp->AddrMode->Aie2MultiDimAddr.Iter.Wrap.Lsb,
				BdProp->AddrMode->Aie2MultiDimAddr.Iter.Wrap.Mask) |
		XAie_SetField((DmaDesc->MultiDimDesc.Gen2MultiDimDesc.IterDesc.StepSize - 1U),
				BdProp->AddrMode->Aie2MultiDimAddr.Iter.StepSize.Lsb,
				BdProp->AddrMode->Aie2MultiDimAddr.Iter.StepSize.Mask);

	BdWord[7U] = XAie_SetField(DmaDesc->BdEnDesc.ValidBd,
			BdProp->BdEn->ValidBd.Lsb, BdProp->BdEn->ValidBd.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockRelVal,
				BdProp->Lock->Aie2DmaLock.LckRelVal.Lsb,
				BdProp->Lock->Aie2DmaLock.LckRelVal.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockRelId,
				BdProp->Lock->Aie2DmaLock.LckRelId.Lsb,
				BdProp->Lock->Aie2DmaLock.LckRelId.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqId,
				BdProp->Lock->Aie2DmaLock.LckAcqId.Lsb,
				BdProp->Lock->Aie2DmaLock.LckAcqId.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqVal,
				BdProp->Lock->Aie2DmaLock.LckAcqVal.Lsb,
				BdProp->Lock->Aie2DmaLock.LckAcqVal.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqEn,
				BdProp->Lock->Aie2DmaLock.LckAcqEn.Lsb,
				BdProp->Lock->Aie2DmaLock.LckAcqEn.Mask);

	Addr = BdBaseAddr + _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	for(u8 i = 0U; i < XAIEML_MEMTILEDMA_NUM_BD_WORDS; i++) {
		XAie_Write32(DevInst, Addr + i * 4U, BdWord[i]);
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API writes a Dma Descriptor which is initialized and setup by other APIs
* into the corresponding registers and register fields in the hardware. This API
* is specific to AIEML Tiles only.
*
* @param	DevInst: Device Instance
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	Loc: Location of AIE Tile
* @param	BdNum: Hardware BD number to be written to.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only. For AIEML Tiles only.
*
******************************************************************************/
AieRC _XAieMl_TileDmaWriteBd(XAie_DevInst *DevInst , XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u8 BdNum)
{
	u64 Addr;
	u64 BdBaseAddr;
	u32 BdWord[XAIEML_TILEDMA_NUM_BD_WORDS];
	const XAie_DmaMod *DmaMod;
	const XAie_DmaBdProp *BdProp;

	DmaMod = DevInst->DevProp.DevMod[DmaDesc->TileType].DmaMod;
	BdProp = DmaMod->BdProp;

	BdBaseAddr = DmaMod->BaseAddr + BdNum * DmaMod->IdxOffset;

	/* Setup BdWord with the right values from DmaDesc */
	BdWord[0U] = XAie_SetField(DmaDesc->AddrDesc.Address,
				BdProp->Buffer->TileDmaBuff.BaseAddr.Lsb,
				BdProp->Buffer->TileDmaBuff.BaseAddr.Mask) |
		XAie_SetField(DmaDesc->AddrDesc.Length,
				BdProp->Buffer->TileDmaBuff.BufferLen.Lsb,
				BdProp->Buffer->TileDmaBuff.BufferLen.Mask);

	BdWord[1U] = XAie_SetField(DmaDesc->EnCompression,
				BdProp->Compression->EnCompression.Lsb,
				BdProp->Compression->EnCompression.Mask) |
		XAie_SetField(DmaDesc->PktDesc.PktEn,
				BdProp->Pkt->EnPkt.Lsb,
				BdProp->Pkt->EnPkt.Mask) |
		XAie_SetField(DmaDesc->PktDesc.PktType,
				BdProp->Pkt->PktType.Lsb,
				BdProp->Pkt->PktType.Mask) |
		XAie_SetField(DmaDesc->PktDesc.PktId, BdProp->Pkt->PktId.Lsb,
				BdProp->Pkt->PktId.Mask) |
		XAie_SetField(DmaDesc->BdEnDesc.OutofOrderBdId,
				BdProp->BdEn->OutofOrderBdId.Lsb,
				BdProp->BdEn->OutofOrderBdId.Mask);

	BdWord[2U] = XAie_SetField((DmaDesc->MultiDimDesc.Gen2MultiDimDesc.DimDesc[1U].StepSize - 1U),
			BdProp->AddrMode->Aie2MultiDimAddr.DmaDimProp[1U].StepSize.Lsb,
			BdProp->AddrMode->Aie2MultiDimAddr.DmaDimProp[1U].StepSize.Mask) |
		XAie_SetField((DmaDesc->MultiDimDesc.Gen2MultiDimDesc.DimDesc[0U].StepSize - 1U),
				BdProp->AddrMode->Aie2MultiDimAddr.DmaDimProp[0U].StepSize.Lsb,
				BdProp->AddrMode->Aie2MultiDimAddr.DmaDimProp[0U].StepSize.Mask);

	BdWord[3U] = XAie_SetField(DmaDesc->MultiDimDesc.Gen2MultiDimDesc.DimDesc[1U].Wrap,
			BdProp->AddrMode->Aie2MultiDimAddr.DmaDimProp[1U].Wrap.Lsb,
			BdProp->AddrMode->Aie2MultiDimAddr.DmaDimProp[1U].Wrap.Mask) |
		XAie_SetField(DmaDesc->MultiDimDesc.Gen2MultiDimDesc.DimDesc[0U].Wrap,
				BdProp->AddrMode->Aie2MultiDimAddr.DmaDimProp[0U].Wrap.Lsb,
				BdProp->AddrMode->Aie2MultiDimAddr.DmaDimProp[0U].Wrap.Mask) |
		XAie_SetField((DmaDesc->MultiDimDesc.Gen2MultiDimDesc.DimDesc[2U].StepSize - 1U),
				BdProp->AddrMode->Aie2MultiDimAddr.DmaDimProp[2U].StepSize.Lsb,
				BdProp->AddrMode->Aie2MultiDimAddr.DmaDimProp[2U].StepSize.Mask);

	BdWord[4U] = XAie_SetField(DmaDesc->MultiDimDesc.Gen2MultiDimDesc.IterCurr,
			BdProp->AddrMode->Aie2MultiDimAddr.IterCurr.Lsb,
			BdProp->AddrMode->Aie2MultiDimAddr.IterCurr.Mask) |
		XAie_SetField((DmaDesc->MultiDimDesc.Gen2MultiDimDesc.IterDesc.Wrap - 1U),
				BdProp->AddrMode->Aie2MultiDimAddr.Iter.Wrap.Lsb,
				BdProp->AddrMode->Aie2MultiDimAddr.Iter.Wrap.Mask) |
		XAie_SetField((DmaDesc->MultiDimDesc.Gen2MultiDimDesc.IterDesc.StepSize - 1U),
				BdProp->AddrMode->Aie2MultiDimAddr.Iter.StepSize.Lsb,
				BdProp->AddrMode->Aie2MultiDimAddr.Iter.StepSize.Mask);

	BdWord[5U] = XAie_SetField(DmaDesc->BdEnDesc.ValidBd,
			BdProp->BdEn->ValidBd.Lsb, BdProp->BdEn->ValidBd.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockRelVal,
				BdProp->Lock->Aie2DmaLock.LckRelVal.Lsb,
				BdProp->Lock->Aie2DmaLock.LckRelVal.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockRelId,
				BdProp->Lock->Aie2DmaLock.LckRelId.Lsb,
				BdProp->Lock->Aie2DmaLock.LckRelId.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqId,
				BdProp->Lock->Aie2DmaLock.LckAcqId.Lsb,
				BdProp->Lock->Aie2DmaLock.LckAcqId.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqVal,
				BdProp->Lock->Aie2DmaLock.LckAcqVal.Lsb,
				BdProp->Lock->Aie2DmaLock.LckAcqVal.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqEn,
				BdProp->Lock->Aie2DmaLock.LckAcqEn.Lsb,
				BdProp->Lock->Aie2DmaLock.LckAcqEn.Mask) |
		XAie_SetField(DmaDesc->BdEnDesc.UseNxtBd,
				BdProp->BdEn->UseNxtBd.Lsb,
				BdProp->BdEn->UseNxtBd.Mask) |
		XAie_SetField(DmaDesc->BdEnDesc.NxtBd, BdProp->BdEn->NxtBd.Lsb,
				BdProp->BdEn->NxtBd.Mask);

	Addr = BdBaseAddr + _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	for(u8 i = 0U; i < XAIEML_TILEDMA_NUM_BD_WORDS; i++) {
		XAie_Write32(DevInst, Addr + i * 4U, BdWord[i]);
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API writes a Dma Descriptor which is initialized and setup by other APIs
* into the corresponding registers and register fields in the hardware. This API
* is specific to AIEML Shim Tiles only.
*
* @param	DevInst: Device Instance
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	Loc: Location of AIE Tile
* @param	BdNum: Hardware BD number to be written to.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only. For AIEML Shim Tiles only.
*
******************************************************************************/
AieRC _XAieMl_ShimDmaWriteBd(XAie_DevInst *DevInst , XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u8 BdNum)
{
	u64 Addr;
	u64 BdBaseAddr;
	u32 BdWord[XAIEML_SHIMDMA_NUM_BD_WORDS];
	const XAie_DmaMod *DmaMod;
	const XAie_DmaBdProp *BdProp;

	DmaMod = DevInst->DevProp.DevMod[DmaDesc->TileType].DmaMod;
	BdProp = DmaMod->BdProp;

	BdBaseAddr = DmaMod->BaseAddr + BdNum * DmaMod->IdxOffset;

	/* Setup BdWord with the right values from DmaDesc */
	BdWord[0U] = XAie_SetField(DmaDesc->AddrDesc.Length,
			BdProp->Buffer->ShimDmaBuff.BufferLen.Lsb,
			BdProp->Buffer->ShimDmaBuff.BufferLen.Mask);

	BdWord[1U] = XAie_SetField(DmaDesc->AddrDesc.Address,
			BdProp->Buffer->ShimDmaBuff.AddrLow.Lsb,
			BdProp->Buffer->ShimDmaBuff.AddrLow.Mask);

	BdWord[2U] = XAie_SetField((DmaDesc->AddrDesc.Address >> 32U),
			BdProp->Buffer->ShimDmaBuff.AddrHigh.Lsb,
			BdProp->Buffer->ShimDmaBuff.AddrHigh.Mask) |
		XAie_SetField(DmaDesc->PktDesc.PktEn,
				BdProp->Pkt->EnPkt.Lsb,
				BdProp->Pkt->EnPkt.Mask) |
		XAie_SetField(DmaDesc->PktDesc.PktType,
				BdProp->Pkt->PktType.Lsb,
				BdProp->Pkt->PktType.Mask) |
		XAie_SetField(DmaDesc->PktDesc.PktId, BdProp->Pkt->PktId.Lsb,
				BdProp->Pkt->PktId.Mask) |
		XAie_SetField(DmaDesc->BdEnDesc.OutofOrderBdId,
				BdProp->BdEn->OutofOrderBdId.Lsb,
				BdProp->BdEn->OutofOrderBdId.Mask);

	BdWord[3U] = XAie_SetField(DmaDesc->MultiDimDesc.Gen2MultiDimDesc.DimDesc[0U].Wrap,
			BdProp->AddrMode->Aie2MultiDimAddr.DmaDimProp[0U].Wrap.Lsb,
			BdProp->AddrMode->Aie2MultiDimAddr.DmaDimProp[0U].Wrap.Mask) |
		XAie_SetField((DmaDesc->MultiDimDesc.Gen2MultiDimDesc.DimDesc[0U].StepSize - 1U),
				BdProp->AddrMode->Aie2MultiDimAddr.DmaDimProp[0U].StepSize.Lsb,
				BdProp->AddrMode->Aie2MultiDimAddr.DmaDimProp[0U].StepSize.Mask) |
		XAie_SetField(DmaDesc->AxiDesc.SecureAccess,
				BdProp->SysProp->SecureAccess.Lsb,
				BdProp->SysProp->SecureAccess.Mask);
	BdWord[4U] = XAie_SetField(DmaDesc->MultiDimDesc.Gen2MultiDimDesc.DimDesc[1U].Wrap,
			BdProp->AddrMode->Aie2MultiDimAddr.DmaDimProp[1U].Wrap.Lsb,
			BdProp->AddrMode->Aie2MultiDimAddr.DmaDimProp[1U].Wrap.Mask) |
		XAie_SetField((DmaDesc->MultiDimDesc.Gen2MultiDimDesc.DimDesc[1U].StepSize - 1U),
				BdProp->AddrMode->Aie2MultiDimAddr.DmaDimProp[1U].StepSize.Lsb,
				BdProp->AddrMode->Aie2MultiDimAddr.DmaDimProp[1U].StepSize.Mask) |
		XAie_SetField(DmaDesc->AxiDesc.BurstLen,
				BdProp->SysProp->BurstLen.Lsb,
				BdProp->SysProp->BurstLen.Mask);

	BdWord[5U] = XAie_SetField(DmaDesc->AxiDesc.SMID,
			BdProp->SysProp->SMID.Lsb, BdProp->SysProp->SMID.Mask) |
		XAie_SetField(DmaDesc->AxiDesc.AxQos,
				BdProp->SysProp->AxQos.Lsb,
				BdProp->SysProp->AxQos.Mask) |
		XAie_SetField(DmaDesc->AxiDesc.AxCache,
				BdProp->SysProp->AxCache.Lsb,
				BdProp->SysProp->AxCache.Mask) |
		XAie_SetField((DmaDesc->MultiDimDesc.Gen2MultiDimDesc.DimDesc[2U].StepSize - 1U),
				BdProp->AddrMode->Aie2MultiDimAddr.DmaDimProp[2U].StepSize.Lsb,
				BdProp->AddrMode->Aie2MultiDimAddr.DmaDimProp[2U].StepSize.Mask);

	BdWord[6U] = XAie_SetField(DmaDesc->MultiDimDesc.Gen2MultiDimDesc.IterCurr,
			BdProp->AddrMode->Aie2MultiDimAddr.IterCurr.Lsb,
			BdProp->AddrMode->Aie2MultiDimAddr.IterCurr.Mask) |
		XAie_SetField((DmaDesc->MultiDimDesc.Gen2MultiDimDesc.IterDesc.Wrap - 1U),
				BdProp->AddrMode->Aie2MultiDimAddr.Iter.Wrap.Lsb,
				BdProp->AddrMode->Aie2MultiDimAddr.Iter.Wrap.Mask) |
		XAie_SetField((DmaDesc->MultiDimDesc.Gen2MultiDimDesc.IterDesc.StepSize - 1U),
				BdProp->AddrMode->Aie2MultiDimAddr.Iter.StepSize.Lsb,
				BdProp->AddrMode->Aie2MultiDimAddr.Iter.StepSize.Mask);

	BdWord[7U] = XAie_SetField(DmaDesc->BdEnDesc.ValidBd,
			BdProp->BdEn->ValidBd.Lsb, BdProp->BdEn->ValidBd.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockRelVal,
				BdProp->Lock->Aie2DmaLock.LckRelVal.Lsb,
				BdProp->Lock->Aie2DmaLock.LckRelVal.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockRelId,
				BdProp->Lock->Aie2DmaLock.LckRelId.Lsb,
				BdProp->Lock->Aie2DmaLock.LckRelId.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqId,
				BdProp->Lock->Aie2DmaLock.LckAcqId.Lsb,
				BdProp->Lock->Aie2DmaLock.LckAcqId.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqVal,
				BdProp->Lock->Aie2DmaLock.LckAcqVal.Lsb,
				BdProp->Lock->Aie2DmaLock.LckAcqVal.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqEn,
				BdProp->Lock->Aie2DmaLock.LckAcqEn.Lsb,
				BdProp->Lock->Aie2DmaLock.LckAcqEn.Mask) |
		XAie_SetField(DmaDesc->BdEnDesc.UseNxtBd,
				BdProp->BdEn->UseNxtBd.Lsb,
				BdProp->BdEn->UseNxtBd.Mask) |
		XAie_SetField(DmaDesc->BdEnDesc.NxtBd, BdProp->BdEn->NxtBd.Lsb,
				BdProp->BdEn->NxtBd.Mask);

	Addr = BdBaseAddr + _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	for(u8 i = 0U; i < XAIEML_SHIMDMA_NUM_BD_WORDS; i++) {
		XAie_Write32(DevInst, Addr + i * 4U, BdWord[i]);
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
* @note		Internal only. For AIEML Tiles only.
*
******************************************************************************/
AieRC _XAieMl_DmaGetPendingBdCount(XAie_DevInst *DevInst, XAie_LocType Loc,
		const XAie_DmaMod *DmaMod, u8 ChNum, XAie_DmaDirection Dir,
		u8 *PendingBd)
{
	/*
	 * TODO: The register database required for this api has changed b/w
	 * r0p6 and r0p13. Implement this once the register database if freezed.
	 */

	return XAIE_FEATURE_NOT_SUPPORTED;
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
* @note		Internal only. For AIEML Tiles only.
*
******************************************************************************/
AieRC _XAieMl_DmaWaitForDone(XAie_DevInst *DevInst, XAie_LocType Loc,
		const XAie_DmaMod *DmaMod, u8 ChNum, XAie_DmaDirection Dir,
		u32 TimeOutUs)
{
	/*
	 * TODO: The register database required for this api has changed b/w
	 * r0p6 and r0p13. Implement this once the register database if freezed.
	 */

	return XAIE_FEATURE_NOT_SUPPORTED;
}

/** @} */
