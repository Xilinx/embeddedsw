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
#include "xaie_feature_config.h"
#include "xaie_helper.h"
#include "xaie_io.h"
#include "xaiegbl_regdef.h"

#ifdef XAIE_FEATURE_DMA_ENABLE

/************************** Constant Definitions *****************************/
#define XAIEML_TILEDMA_NUM_BD_WORDS			6U
#define XAIEML_SHIMDMA_NUM_BD_WORDS			8U
#define XAIEML_MEMTILEDMA_NUM_BD_WORDS			8U
#define XAIEML_DMA_STEPSIZE_DEFAULT			1U
#define XAIEML_DMA_ITERWRAP_DEFAULT			1U
#define XAIEML_DMA_PAD_NUM_DIMS				3U

#define XAIEML_DMA_STATUS_IDLE				0x0
#define XAIEML_DMA_STATUS_CHNUM_OFFSET			0x4

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
		Desc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[i].StepSize =
			XAIEML_DMA_STEPSIZE_DEFAULT;
	}

	Desc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.Wrap =
		XAIEML_DMA_ITERWRAP_DEFAULT;
	Desc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.StepSize =
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
		Desc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[i].StepSize =
			XAIEML_DMA_STEPSIZE_DEFAULT;
	}

	Desc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.Wrap =
		XAIEML_DMA_ITERWRAP_DEFAULT;
	Desc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.StepSize =
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
		Desc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[i].StepSize =
			XAIEML_DMA_STEPSIZE_DEFAULT;
	}

	Desc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.Wrap =
		XAIEML_DMA_ITERWRAP_DEFAULT;
	Desc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.StepSize =
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
		const XAie_DmaBdProp *BdProp = DmaDesc->DmaMod->BdProp;
		if(Tensor->Dim[i].AieMlDimDesc.StepSize == 0U) {
			XAIE_ERROR("Invalid stepsize for dimension %d\n", i);
			return XAIE_ERR;
		}
		if((Tensor->Dim[i].AieMlDimDesc.StepSize > BdProp->StepSizeMax) ||
				(Tensor->Dim[i].AieMlDimDesc.Wrap > BdProp->WrapMax)) {
			XAIE_ERROR("Invalid stepsize or wrap for dimension %d\n",
					i);
			return XAIE_ERR;
		}
	}

	for(u8 i = 0U; i < Tensor->NumDim; i++) {

		DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[i].StepSize =
			Tensor->Dim[i].AieMlDimDesc.StepSize;
		DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[i].Wrap =
			Tensor->Dim[i].AieMlDimDesc.Wrap;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API checks the validity of wrap and padding before and after fields of
* the Dma descriptor.
*
* @param	DmaDesc: Dma Descriptor
*
* @return	XAIE_OK on success, XAIE_INVALID_DMA_DESC on failure
*
* @note		Internal Only.
*		If D0_wrap == 0:
*			D1/D2 padding after/before must be 0
*			D0 padding after must be 0
*		If D1_wrap == 0:
*			D2 padding after/before must be 0
*			D1 padding after must be 0
*		If D2_wrap == 0:
*			D2 padding after must be 0
*
******************************************************************************/
static AieRC _XAieMl_DmaMemTileCheckPaddingConfig(XAie_DmaDesc *DmaDesc)
{
	XAie_AieMlDimDesc *DDesc = DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc;
	XAie_PadDesc *PDesc = DmaDesc->PadDesc;

	for(u8 Dim = 0U; Dim < XAIEML_DMA_PAD_NUM_DIMS; Dim++) {
		if(DDesc[Dim].Wrap == 0U) {

			if(PDesc[Dim].After != 0U) {
				XAIE_ERROR("Padding after for dimension %u must"
						" be 0 when wrap is 1\n", Dim);
				return XAIE_INVALID_DMA_DESC;
			}

			for(u8 PadDim = Dim + 1U;
					PadDim < XAIEML_DMA_PAD_NUM_DIMS;
					PadDim++) {
				if((PDesc[PadDim].After != 0U) ||
						(PDesc[PadDim].Before != 0U)) {
					XAIE_ERROR("After and Before pading "
							"for dimension %u must "
							"be 0 when wrap for "
							"dimension %u is 0\n",
							PadDim, Dim);
					return XAIE_ERR;
				}
			}
		}
	}

	XAIE_DBG("Zero padding and wrap configuration is correct\n");
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
	AieRC RC;
	u64 Addr;
	u64 BdBaseAddr;
	u32 BdWord[XAIEML_MEMTILEDMA_NUM_BD_WORDS];
	const XAie_DmaMod *DmaMod;
	const XAie_DmaBdProp *BdProp;

	RC = _XAieMl_DmaMemTileCheckPaddingConfig(DmaDesc);
	if(RC != XAIE_OK)
		return RC;

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

	BdWord[1U] = XAie_SetField(DmaDesc->PadDesc[0U].Before,
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

	BdWord[2U] = XAie_SetField(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[0U].Wrap,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Mask) |
		 XAie_SetField((DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[0U].StepSize - 1U),
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[0U].StepSize.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[0U].StepSize.Mask) |
		 XAie_SetField(DmaDesc->TlastSuppress,
				BdProp->BdEn->TlastSuppress.Lsb,
				BdProp->BdEn->TlastSuppress.Mask);

	BdWord[3U] = XAie_SetField(DmaDesc->PadDesc[1U].Before,
			BdProp->ZeroPad->D1_ZeroBefore.Lsb,
			BdProp->ZeroPad->D1_ZeroBefore.Mask) |
		XAie_SetField(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[1U].Wrap,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Mask) |
		XAie_SetField((DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[1U].StepSize - 1U),
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Mask);

	BdWord[4U] = XAie_SetField(DmaDesc->PadDesc[2U].Before,
			BdProp->ZeroPad->D2_ZeroBefore.Lsb,
			BdProp->ZeroPad->D2_ZeroBefore.Mask) |
		XAie_SetField(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[2U].Wrap,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].Wrap.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].Wrap.Mask) |
		XAie_SetField((DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[2U].StepSize - 1U),
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Mask) |
		XAie_SetField(DmaDesc->EnCompression,
				BdProp->Compression->EnCompression.Lsb,
				BdProp->Compression->EnCompression.Mask);

	BdWord[5U] = XAie_SetField(DmaDesc->PadDesc[2U].After,
			BdProp->ZeroPad->D2_ZeroAfter.Lsb,
			BdProp->ZeroPad->D2_ZeroAfter.Mask) |
		XAie_SetField(DmaDesc->PadDesc[1U].After,
			BdProp->ZeroPad->D1_ZeroAfter.Lsb,
			BdProp->ZeroPad->D1_ZeroAfter.Mask) |
		XAie_SetField(DmaDesc->PadDesc[0U].After,
			BdProp->ZeroPad->D0_ZeroAfter.Lsb,
			BdProp->ZeroPad->D0_ZeroAfter.Mask) |
		XAie_SetField((DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[3U].StepSize - 1U),
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[3U].StepSize.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[3U].StepSize.Mask);

	BdWord[6U] = XAie_SetField(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterCurr,
			BdProp->AddrMode->AieMlMultiDimAddr.IterCurr.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.IterCurr.Mask) |
		XAie_SetField((DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.Wrap - 1U),
				BdProp->AddrMode->AieMlMultiDimAddr.Iter.Wrap.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.Iter.Wrap.Mask) |
		XAie_SetField((DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.StepSize - 1U),
				BdProp->AddrMode->AieMlMultiDimAddr.Iter.StepSize.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.Iter.StepSize.Mask);

	BdWord[7U] = XAie_SetField(DmaDesc->BdEnDesc.ValidBd,
			BdProp->BdEn->ValidBd.Lsb, BdProp->BdEn->ValidBd.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockRelVal,
				BdProp->Lock->AieMlDmaLock.LckRelVal.Lsb,
				BdProp->Lock->AieMlDmaLock.LckRelVal.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockRelId,
				BdProp->Lock->AieMlDmaLock.LckRelId.Lsb,
				BdProp->Lock->AieMlDmaLock.LckRelId.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqId,
				BdProp->Lock->AieMlDmaLock.LckAcqId.Lsb,
				BdProp->Lock->AieMlDmaLock.LckAcqId.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqVal,
				BdProp->Lock->AieMlDmaLock.LckAcqVal.Lsb,
				BdProp->Lock->AieMlDmaLock.LckAcqVal.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqEn,
				BdProp->Lock->AieMlDmaLock.LckAcqEn.Lsb,
				BdProp->Lock->AieMlDmaLock.LckAcqEn.Mask);

	Addr = BdBaseAddr + _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	for(u8 i = 0U; i < XAIEML_MEMTILEDMA_NUM_BD_WORDS; i++) {
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
	AieRC RC;
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

	BdWord[2U] = XAie_SetField((DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[1U].StepSize - 1U),
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Mask) |
		XAie_SetField((DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[0U].StepSize - 1U),
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[0U].StepSize.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[0U].StepSize.Mask);

	BdWord[3U] = XAie_SetField(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[1U].Wrap,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Mask) |
		XAie_SetField(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[0U].Wrap,
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Mask) |
		XAie_SetField((DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[2U].StepSize - 1U),
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Mask);

	BdWord[4U] = XAie_SetField(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterCurr,
			BdProp->AddrMode->AieMlMultiDimAddr.IterCurr.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.IterCurr.Mask) |
		XAie_SetField((DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.Wrap - 1U),
				BdProp->AddrMode->AieMlMultiDimAddr.Iter.Wrap.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.Iter.Wrap.Mask) |
		XAie_SetField((DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.StepSize - 1U),
				BdProp->AddrMode->AieMlMultiDimAddr.Iter.StepSize.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.Iter.StepSize.Mask);

	BdWord[5U] = XAie_SetField(DmaDesc->BdEnDesc.ValidBd,
			BdProp->BdEn->ValidBd.Lsb, BdProp->BdEn->ValidBd.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockRelVal,
				BdProp->Lock->AieMlDmaLock.LckRelVal.Lsb,
				BdProp->Lock->AieMlDmaLock.LckRelVal.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockRelId,
				BdProp->Lock->AieMlDmaLock.LckRelId.Lsb,
				BdProp->Lock->AieMlDmaLock.LckRelId.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqId,
				BdProp->Lock->AieMlDmaLock.LckAcqId.Lsb,
				BdProp->Lock->AieMlDmaLock.LckAcqId.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqVal,
				BdProp->Lock->AieMlDmaLock.LckAcqVal.Lsb,
				BdProp->Lock->AieMlDmaLock.LckAcqVal.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqEn,
				BdProp->Lock->AieMlDmaLock.LckAcqEn.Lsb,
				BdProp->Lock->AieMlDmaLock.LckAcqEn.Mask) |
		XAie_SetField(DmaDesc->BdEnDesc.UseNxtBd,
				BdProp->BdEn->UseNxtBd.Lsb,
				BdProp->BdEn->UseNxtBd.Mask) |
		XAie_SetField(DmaDesc->BdEnDesc.NxtBd, BdProp->BdEn->NxtBd.Lsb,
				BdProp->BdEn->NxtBd.Mask) |
		XAie_SetField(DmaDesc->TlastSuppress,
				BdProp->BdEn->TlastSuppress.Lsb,
				BdProp->BdEn->TlastSuppress.Mask);

	Addr = BdBaseAddr + _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	for(u8 i = 0U; i < XAIEML_TILEDMA_NUM_BD_WORDS; i++) {
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
	XAie_ShimDmaBdArgs Args;
	const XAie_DmaMod *DmaMod;
	const XAie_DmaBdProp *BdProp;

	DmaMod = DevInst->DevProp.DevMod[DmaDesc->TileType].DmaMod;
	BdProp = DmaMod->BdProp;

	BdBaseAddr = DmaMod->BaseAddr + BdNum * DmaMod->IdxOffset;

	/* Setup BdWord with the right values from DmaDesc */
	BdWord[0U] = XAie_SetField(DmaDesc->AddrDesc.Length,
			BdProp->Buffer->ShimDmaBuff.BufferLen.Lsb,
			BdProp->Buffer->ShimDmaBuff.BufferLen.Mask);

	BdWord[1U] = XAie_SetField(DmaDesc->AddrDesc.Address >>
				BdProp->Buffer->ShimDmaBuff.AddrLow.Lsb,
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

	BdWord[3U] = XAie_SetField(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[0U].Wrap,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Mask) |
		XAie_SetField((DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[0U].StepSize - 1U),
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[0U].StepSize.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[0U].StepSize.Mask) |
		XAie_SetField(DmaDesc->AxiDesc.SecureAccess,
				BdProp->SysProp->SecureAccess.Lsb,
				BdProp->SysProp->SecureAccess.Mask);
	BdWord[4U] = XAie_SetField(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[1U].Wrap,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Mask) |
		XAie_SetField((DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[1U].StepSize - 1U),
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Mask) |
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
		XAie_SetField((DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[2U].StepSize - 1U),
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Mask);

	BdWord[6U] = XAie_SetField(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterCurr,
			BdProp->AddrMode->AieMlMultiDimAddr.IterCurr.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.IterCurr.Mask) |
		XAie_SetField((DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.Wrap - 1U),
				BdProp->AddrMode->AieMlMultiDimAddr.Iter.Wrap.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.Iter.Wrap.Mask) |
		XAie_SetField((DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.StepSize - 1U),
				BdProp->AddrMode->AieMlMultiDimAddr.Iter.StepSize.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.Iter.StepSize.Mask);

	BdWord[7U] = XAie_SetField(DmaDesc->BdEnDesc.ValidBd,
			BdProp->BdEn->ValidBd.Lsb, BdProp->BdEn->ValidBd.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockRelVal,
				BdProp->Lock->AieMlDmaLock.LckRelVal.Lsb,
				BdProp->Lock->AieMlDmaLock.LckRelVal.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockRelId,
				BdProp->Lock->AieMlDmaLock.LckRelId.Lsb,
				BdProp->Lock->AieMlDmaLock.LckRelId.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqId,
				BdProp->Lock->AieMlDmaLock.LckAcqId.Lsb,
				BdProp->Lock->AieMlDmaLock.LckAcqId.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqVal,
				BdProp->Lock->AieMlDmaLock.LckAcqVal.Lsb,
				BdProp->Lock->AieMlDmaLock.LckAcqVal.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqEn,
				BdProp->Lock->AieMlDmaLock.LckAcqEn.Lsb,
				BdProp->Lock->AieMlDmaLock.LckAcqEn.Mask) |
		XAie_SetField(DmaDesc->BdEnDesc.UseNxtBd,
				BdProp->BdEn->UseNxtBd.Lsb,
				BdProp->BdEn->UseNxtBd.Mask) |
		XAie_SetField(DmaDesc->BdEnDesc.NxtBd, BdProp->BdEn->NxtBd.Lsb,
				BdProp->BdEn->NxtBd.Mask) |
		XAie_SetField(DmaDesc->TlastSuppress,
				BdProp->BdEn->TlastSuppress.Lsb,
				BdProp->BdEn->TlastSuppress.Mask);

	Addr = BdBaseAddr + _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	Args.NumBdWords = XAIEML_SHIMDMA_NUM_BD_WORDS;
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
	AieRC RC;
	u64 Addr;
	u32 Mask, StatusReg, TaskQSize;

	Addr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		DmaMod->ChStatusBase + ChNum * XAIEML_DMA_STATUS_CHNUM_OFFSET +
		Dir * DmaMod->ChStatusOffset;

	RC = XAie_Read32(DevInst, Addr, &StatusReg);
	if(RC != XAIE_OK) {
		return RC;
	}

	TaskQSize = XAie_GetField(StatusReg,
			DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.TaskQSize.Lsb,
			DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.TaskQSize.Mask);
	if(TaskQSize > DmaMod->ChProp->StartQSizeMax) {
		XAIE_ERROR("Invalid start queue size from register\n");
		return XAIE_ERR;
	}

	Mask = DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.Status.Mask |
		DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.StalledLockAcq.Mask |
		DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.StalledLockRel.Mask |
		DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.StalledStreamStarve.Mask |
		DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.StalledTCT.Mask;

	/* Check if BD is being used by a channel */
	if(StatusReg & Mask) {
		TaskQSize++;
	}

	*PendingBd = TaskQSize;

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
* @note		Internal only. For AIEML Tiles only.
*
******************************************************************************/
AieRC _XAieMl_DmaWaitForDone(XAie_DevInst *DevInst, XAie_LocType Loc,
		const XAie_DmaMod *DmaMod, u8 ChNum, XAie_DmaDirection Dir,
		u32 TimeOutUs)
{
	u64 Addr;
	u32 Mask, Value;

	Addr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		DmaMod->ChStatusBase + ChNum * XAIEML_DMA_STATUS_CHNUM_OFFSET +
		Dir * DmaMod->ChStatusOffset;

	Mask = DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.Status.Mask |
		DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.TaskQSize.Mask |
		DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.StalledLockAcq.Mask |
		DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.StalledLockRel.Mask |
		DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.StalledStreamStarve.Mask |
		DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.StalledTCT.Mask;

	/* This will check the stalled and start queue size bits to be zero */
	Value = XAIEML_DMA_STATUS_IDLE <<
		DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.Status.Lsb;

	if(XAie_MaskPoll(DevInst, Addr, Mask, Value, TimeOutUs) !=
			XAIE_OK) {
		XAIE_DBG("Wait for done timed out\n");
		return XAIE_ERR;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API is used to check the validity of Bd number and Channel number
* combination.
*
* @param	BdNum: Buffer descriptor number.
* @param	ChNum: Channel number of the DMA.
* @param        TimeOutUs - Minimum timeout value in micro seconds.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only. For AIEML Tiles only. This is a dummy function as
*		there is no restriction in AIE tile dma and shim dma.
*
******************************************************************************/
AieRC _XAieMl_DmaCheckBdChValidity(u8 BdNum, u8 ChNum)
{
	(void)BdNum;
	(void)ChNum;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API is used to check the validity of Bd number and Channel number
* combination.
*
* @param	BdNum: Buffer descriptor number.
* @param	ChNum: Channel number of the DMA.
* @param        TimeOutUs - Minimum timeout value in micro seconds.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only. For AIEML Mem Tiles only.
*
******************************************************************************/
AieRC _XAieMl_MemTileDmaCheckBdChValidity(u8 BdNum, u8 ChNum)
{
	if((BdNum < 24U) && ((ChNum % 2U) == 0U))
		return XAIE_OK;
	else if((BdNum >= 24U) && ((ChNum % 2U) == 1U))
		return XAIE_OK;

	XAIE_ERROR("Invalid BdNum, ChNum combination\n");
	return XAIE_INVALID_ARGS;
}

/*****************************************************************************/
/**
*
* This API updates the length of the buffer descriptor in the tile dma or mem
* tile dma module.
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
AieRC _XAieMl_DmaUpdateBdLen(XAie_DevInst *DevInst, const XAie_DmaMod *DmaMod,
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
* This API updates the address of the buffer descriptor in the shim dma module.
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
AieRC _XAieMl_ShimDmaUpdateBdLen(XAie_DevInst *DevInst,
		const XAie_DmaMod *DmaMod, XAie_LocType Loc, u32 Len, u8 BdNum)
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
AieRC _XAieMl_DmaUpdateBdAddr(XAie_DevInst *DevInst, const XAie_DmaMod *DmaMod,
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
* This API updates the length of the buffer descriptor in the dma module.
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
AieRC _XAieMl_ShimDmaUpdateBdAddr(XAie_DevInst *DevInst,
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
* @note		Internal. The stepsize and wrap parameters operate at 32 bit
*		granularity. The address is the absolute address of the buffer
*		which is 32 bit aligned. The driver will configure the BD
*		register with necessary bits(<32 bits) as required by the
*		hardware.
*
******************************************************************************/
AieRC _XAieMl_DmaSetBdIteration(XAie_DmaDesc *DmaDesc, u32 StepSize, u8 Wrap,
		u8 IterCurr)
{
	const XAie_DmaBdProp *BdProp = DmaDesc->DmaMod->BdProp;

	if((StepSize > BdProp->IterStepSizeMax) || (Wrap > BdProp->IterWrapMax) ||
			(IterCurr > BdProp->IterCurrMax)) {
		XAIE_ERROR("Iteration parameters exceed max value.\n");
		return XAIE_ERR;
	}

	DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.StepSize = StepSize;
	DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.Wrap = Wrap;
	DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterCurr = IterCurr;

	return XAIE_OK;
}

#endif /* XAIE_FEATURE_DMA_ENABLE */

/** @} */
