/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_dma.c
* @{
*
* This file contains routines for AIE DMA Controls.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   09/24/2019  Initial creation
* 1.1   Tejus	01/04/2020  Cleanup error messages
* 1.2   Tejus   03/22/2020  Remove initial dma api implementation
* 1.3   Tejus   03/22/2020  Dma api implementation
* 1.4   Tejus   04/09/2020  Remove unused argument from interleave enable api
* 1.5   Tejus   04/13/2020  Remove use of range in apis
* 1.6   Tejus   06/05/2020  Add api to enable fifo mode.
* 1.7   Tejus   06/10/2020  Switch to new io backend apis.
* 1.8   Nishad  09/15/2020  Add checks to validate XAie_DmaChReset, and
*			    XAie_DmaFifoCounter values in
*			    XAie_DmaChannelResetAll, and XAie_DmaConfigFifoMode,
*			    respectively.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include <string.h>
#include "xaie_dma.h"
#include "xaie_feature_config.h"
#include "xaie_helper.h"
#include "xaiegbl_regdef.h"

#ifdef XAIE_FEATURE_DMA_ENABLE

/************************** Constant Definitions *****************************/
#define XAIE_DMA_32BIT_TXFER_LEN			2U
#define XAIE_DMA_MAX_QUEUE_SIZE				4U

#define XAIE_SHIM_BLEN_SHIFT				0x3
#define XAIE_DMA_CHCTRL_NUM_WORDS			2U
#define XAIE_DMA_WAITFORDONE_DEF_WAIT_TIME_US		1000000U
/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API initializes the Dma Descriptor for AIE DMAs for a given range of
* tiles.
*
* @param	DevInst: Device Instance.
* @param	DmaDesc: Pointer to the user allocated dma descriptor.
* @param	Loc: Location of AIE Tile
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_DmaDescInit(XAie_DevInst *DevInst, XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc)
{
	u8 TileType;
	const XAie_DmaMod *DmaMod;

	if((DevInst == XAIE_NULL) || (DmaDesc == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;

	memset((void *)DmaDesc, 0U, sizeof(XAie_DmaDesc));

	DmaMod->DmaBdInit(DmaDesc);
	DmaDesc->TileType = TileType;
	DmaDesc->IsReady = XAIE_COMPONENT_IS_READY;
	DmaDesc->DmaMod = DmaMod;
	DmaDesc->LockMod = DevInst->DevProp.DevMod[TileType].LockMod;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API initializes the Acquire and Release Locks for a a Dma descriptor.
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
* @note		Internal Only.
*
******************************************************************************/
static AieRC _XAie_DmaLockConfig(XAie_DmaDesc *DmaDesc, XAie_Lock Acq,
		XAie_Lock Rel, u8 AcqEn, u8 RelEn)
{
	const XAie_DmaMod *DmaMod;
	const XAie_LockMod *LockMod;

	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaDesc->DmaMod;
	LockMod = DmaDesc->LockMod;
	if((Acq.LockId > DmaMod->NumLocks) ||
			(Acq.LockVal > LockMod->LockValUpperBound) ||
			(Rel.LockVal > LockMod->LockValUpperBound)) {
		XAIE_ERROR("Invalid Lock\n");
		return XAIE_INVALID_LOCK_ID;
	}

	return DmaMod->SetLock(DmaDesc, Acq, Rel, AcqEn, RelEn);
}

/*****************************************************************************/
/**
*
* This API initializes the Acquire and Release Locks for a a Dma descriptor and
* enables the lock in the dma descriptor.
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	Acq: Lock object with acquire lock ID and lock value.
* @param	Rel: Lock object with release lock ID and lock value.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		The API sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware.
*
******************************************************************************/
AieRC XAie_DmaSetLock(XAie_DmaDesc *DmaDesc, XAie_Lock Acq, XAie_Lock Rel)
{
	return _XAie_DmaLockConfig(DmaDesc, Acq, Rel, XAIE_ENABLE, XAIE_ENABLE);
}

/*****************************************************************************/
/**
*
* This api sets up the packet id and packet type for a dma descriptor.
*
* @param	DmaDesc: Initialized dma descriptor.
* @param	Pkt: Pkt object with acquire packet id and packet type.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		The API sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware.
*
******************************************************************************/
AieRC XAie_DmaSetPkt(XAie_DmaDesc *DmaDesc, XAie_Packet Pkt)
{
	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaDesc->PktDesc.PktEn = XAIE_ENABLE;
	DmaDesc->PktDesc.PktId = Pkt.PktId;
	DmaDesc->PktDesc.PktType = Pkt.PktType;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This api sets up the value of inserted out of order ID field in packet header.
*
* @param	DmaDesc: Initialized dma descriptor.
* @param	OutofOrderBdId: Value of out of order ID field in packet header.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		The api sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware. This API
*		works only for AIEML and has no effect on AIE. The buffer
*		descriptor with this feature enabled can work only for MM2S
*		channels.
*
******************************************************************************/
AieRC XAie_DmaSetOutofOrderBdId(XAie_DmaDesc *DmaDesc, u8 OutofOrderBdId)
{
	const XAie_DmaMod *DmaMod;

	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaDesc->DmaMod;
	if(DmaMod->OutofOrderBdId == XAIE_FEATURE_UNAVAILABLE) {
		XAIE_ERROR("Feature unavailable\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	DmaDesc->BdEnDesc.OutofOrderBdId = OutofOrderBdId;
	DmaDesc->EnOutofOrderBdId = XAIE_ENABLE;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API setups the Buffer starting address, Acquire lock and release lock for
* DMA Descriptor and enables the double buffering mode. This API is supported
* for AIE only.
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	Addr: Buffer Address for the 2nd buffer in double buffer mode.
* @param	Acq: Lock object with acquire lock ID and lock value.
* @param	Rel: Lock object with release lock ID and lock value.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		The Acquire and release locks for the double buffers are enabled
*		by default. The API sets up the value in the dma descriptor and
*		does not configure the buffer descriptor field in the hardware.
*
******************************************************************************/
AieRC XAie_DmaSetDoubleBuffer(XAie_DmaDesc *DmaDesc, u64 Addr, XAie_Lock Acq,
		XAie_Lock Rel)
{
	const XAie_DmaMod *DmaMod;
	const XAie_LockMod *LockMod;

	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaDesc->DmaMod;
	if(DmaMod->DoubleBuffering == XAIE_FEATURE_UNAVAILABLE) {
		XAIE_ERROR("Feature not supported\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	if(((Addr & DmaMod->BdProp->AddrAlignMask) != 0U) ||
			((Addr + DmaDesc->AddrDesc.Length) >
			 DmaMod->BdProp->AddrMax)) {
		XAIE_ERROR("Invalid Address\n");
		return XAIE_INVALID_ADDRESS;
	}

	LockMod = DmaDesc->LockMod;
	if((Acq.LockId > LockMod->NumLocks) || (Acq.LockId != Rel.LockId)) {
		XAIE_ERROR("Invalid Lock\n");
		return XAIE_INVALID_LOCK_ID;
	}

	DmaDesc->AddrDesc_2.Address = Addr >> DmaMod->BdProp->AddrAlignShift;
	DmaDesc->EnDoubleBuff = XAIE_ENABLE;

	/* For AIE, Acquire and Release Lock IDs must be the same */
	DmaDesc->LockDesc_2.LockAcqId = Acq.LockId;
	DmaDesc->LockDesc_2.LockRelId = Rel.LockId;
	DmaDesc->LockDesc_2.LockAcqEn = XAIE_ENABLE;
	DmaDesc->LockDesc_2.LockRelEn = XAIE_ENABLE;

	if(Rel.LockVal != XAIE_LOCK_WITH_NO_VALUE) {
		DmaDesc->LockDesc_2.LockRelValEn = XAIE_ENABLE;
		DmaDesc->LockDesc_2.LockRelVal = Rel.LockVal;
	}

	if(Acq.LockVal != XAIE_LOCK_WITH_NO_VALUE) {
		DmaDesc->LockDesc_2.LockAcqValEn = XAIE_ENABLE;
		DmaDesc->LockDesc_2.LockAcqVal = Acq.LockVal;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API setups the Buffer starting address and the buffer length of the DMA
* Descriptor.
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	Addr: Buffer address.
* @param	Len: Length of the buffer in bytes.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		If the dma is being used in double buffer mode, the Len argument
*		passed to this API should include the size for both buffers.
*		The API sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware.
*
******************************************************************************/
AieRC XAie_DmaSetAddrLen(XAie_DmaDesc *DmaDesc, u64 Addr, u32 Len)
{
	const XAie_DmaMod *DmaMod;

	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaDesc->DmaMod;
	if(((Addr & DmaMod->BdProp->AddrAlignMask) != 0U) ||
			((Addr + Len) > DmaMod->BdProp->AddrMax)) {
		XAIE_ERROR("Invalid Address\n");
		return XAIE_INVALID_ADDRESS;
	}

	DmaDesc->AddrDesc.Address = Addr >> DmaMod->BdProp->AddrAlignShift;
	DmaDesc->AddrDesc.Length = (Len >> XAIE_DMA_32BIT_TXFER_LEN) -
		DmaMod->BdProp->LenActualOffset;

	/*
	 * Make sure the MemInst is not set as the Addr is not the offset
	 * to the memory object
	 */
	DmaDesc->MemInst = XAIE_NULL;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API setups the Buffer starting offset to a memory object and the buffer
* length of the DMA Descriptor.
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	MemInst: Memory object instance
* @param	Offset: Buffer address offset to the specified memory object.
* @param	Len: Length of the buffer in bytes.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		The memory object is the shared memory object between the
*		appication and the AI engine partition, this API is used to
*		setup the DMA descriptor which be used for SHIM DMA as it is
*		the one to transfer memory to/from external external memory.
******************************************************************************/
AieRC XAie_DmaSetAddrOffsetLen(XAie_DmaDesc *DmaDesc, XAie_MemInst *MemInst,
		u64 Offset, u32 Len)
{
	const XAie_DmaMod *DmaMod;
	u64 Addr;

	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("DMA set address offset failed, Invalid DmaDesc\n");
		return XAIE_INVALID_ARGS;
	}

	if (MemInst == XAIE_NULL) {
		XAIE_ERROR("DMA set address offset failed, Invalid MemInst\n");
		return XAIE_INVALID_ARGS;
	}

	if (Offset >= MemInst->Size || Offset + Len > MemInst->Size) {
		XAIE_ERROR("DMA set address offset failed, Invalid Offset, Len\n");
		return XAIE_INVALID_ARGS;
	}

	Addr = Offset + MemInst->DevAddr;
	DmaMod = DmaDesc->DmaMod;
	if(((Addr & DmaMod->BdProp->AddrAlignMask) != 0U) ||
			((Offset + Len) > DmaMod->BdProp->AddrMax)) {
		XAIE_ERROR("DMA Set Address Offset failed, Invalid Address Offset\n");
		return XAIE_INVALID_ADDRESS;
	}

	DmaDesc->AddrDesc.Address = Addr >> DmaMod->BdProp->AddrAlignShift;
	DmaDesc->AddrDesc.Length = (Len >> XAIE_DMA_32BIT_TXFER_LEN) -
		DmaMod->BdProp->LenActualOffset;
	DmaDesc->MemInst = MemInst;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API setups the Buffer starting address, the buffer length and the address
* mode of the DMA Descriptor.
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	Tensor: Dma Tensor describing the address mode of dma. The user
*		should setup the tensor based on the hardware generation.
* @param	Addr: Buffer address.
* @param	Len: Length of the buffer in bytes.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None. The stepsize and wrap parameters operate at 32 bit
*		granularity. The address is the absolute address of the buffer
*		which is 32 bit aligned. The driver will configure the BD
*		register with necessary bits(<32 bits) as required by the
*		hardware.
*
******************************************************************************/
AieRC XAie_DmaSetMultiDimAddr(XAie_DmaDesc *DmaDesc, XAie_DmaTensor *Tensor,
		u64 Addr, u32 Len)
{
	const XAie_DmaMod *DmaMod;

	if((DmaDesc == XAIE_NULL) || (Tensor == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaDesc->DmaMod;
	if(((Addr & DmaMod->BdProp->AddrAlignMask) != 0U) ||
			((Addr + Len) > DmaMod->BdProp->AddrMax)) {
		XAIE_ERROR("Invalid Address\n");
		return XAIE_INVALID_ADDRESS;
	}

	if(Tensor->NumDim > DmaMod->NumAddrDim) {
		XAIE_ERROR("Tensor dimension not supported\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	DmaDesc->AddrDesc.Address = Addr >> DmaMod->BdProp->AddrAlignShift;
	DmaDesc->AddrDesc.Length = (Len >> XAIE_DMA_32BIT_TXFER_LEN) -
		DmaMod->BdProp->LenActualOffset;

	return	DmaMod->SetMultiDim(DmaDesc, Tensor);
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
* @note		None. The stepsize and wrap parameters operate at 32 bit
*		granularity. The address is the absolute address of the buffer
*		which is 32 bit aligned. The driver will configure the BD
*		register with necessary bits(<32 bits) as required by the
*		hardware.
*
******************************************************************************/
AieRC XAie_DmaSetBdIteration(XAie_DmaDesc *DmaDesc, u32 StepSize, u8 Wrap,
		u8 IterCurr)
{
	const XAie_DmaMod *DmaMod;

	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaDesc->DmaMod;

	return DmaMod->SetBdIter(DmaDesc, StepSize, Wrap, IterCurr);
}

/*****************************************************************************/
/**
*
* This API enables the compression bit in the DMA Descriptor.
*
* @param	DmaDesc: Initialized Dma Descriptor.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		The API sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware.
*
******************************************************************************/
AieRC XAie_DmaEnableCompression(XAie_DmaDesc *DmaDesc)
{
	const XAie_DmaMod *DmaMod;

	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaDesc->DmaMod;
	if(DmaMod->Compression == XAIE_FEATURE_UNAVAILABLE) {
		XAIE_ERROR("Feature not supported\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	DmaDesc->EnCompression = XAIE_ENABLE;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API configures the FIFO mode of the DMA Descriptor.
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	Counter: Fifo counter to use. XAIE_DMA_FIFO_COUNTER_NONE to
* 		disable.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		The API sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware.
*
******************************************************************************/
AieRC XAie_DmaConfigFifoMode(XAie_DmaDesc *DmaDesc, XAie_DmaFifoCounter Counter)
{
	const XAie_DmaMod *DmaMod;

	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaDesc->DmaMod;
	if(DmaMod->FifoMode == XAIE_FEATURE_UNAVAILABLE) {
		XAIE_ERROR("Feature not supported\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	if(Counter > XAIE_DMA_FIFO_COUNTER_1) {
		XAIE_ERROR("Invalid DMA FIFO counter\n");
		return XAIE_INVALID_ARGS;
	}

	DmaDesc->FifoMode = Counter;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API setups the Enable Bd, Next Bd and UseNxtBd fields in the DMA
* Descriptor.
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	NextBd: NextBd to run after the current Bd finishes execution..
* @param	EnableNextBd: XAIE_ENABLE/XAIE_DISABLE to enable or disable
*		the next BD in DMA Descriptor.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		The API sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware.
*
******************************************************************************/
AieRC XAie_DmaSetNextBd(XAie_DmaDesc *DmaDesc, u8 NextBd, u8 EnableNextBd)
{
	const XAie_DmaMod *DmaMod;

	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaDesc->DmaMod;
	if(NextBd > DmaMod->NumBds) {
		XAIE_ERROR("Invalid Next Bd\n");
		return XAIE_INVALID_BD_NUM;
	}

	DmaDesc->BdEnDesc.NxtBd = NextBd;
	DmaDesc->BdEnDesc.UseNxtBd = EnableNextBd;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API enables the Enable Bd bit in the DMA Descriptor.
*
* @param	DmaDesc: Initialized Dma Descriptor.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		The API sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware.
*
******************************************************************************/
AieRC XAie_DmaEnableBd(XAie_DmaDesc *DmaDesc)
{
	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaDesc->BdEnDesc.ValidBd = XAIE_ENABLE;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API disables the Enable Bd bit in the DMA Descriptor.
*
* @param	DmaDesc: Initialized Dma Descriptor.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		The API sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware.
*
******************************************************************************/
AieRC XAie_DmaDisableBd(XAie_DmaDesc *DmaDesc)
{
	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaDesc->BdEnDesc.ValidBd = XAIE_DISABLE;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API setups the Enable Bd, Next Bd and UseNxtBd fields in the DMA
* Descriptor.
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	Smid: SMID for the buffer descriptor.
* @param	BurstLen: BurstLength for the buffer descriptor (4, 8 or 16).
* @param	Qos: AXI Qos bits for the AXI-MM transfer.
* @param	Cache: AxCACHE bits for AXI-MM transfer.
* @param	Secure: Secure status of the transfer(1-Secure, 0-Non Secure).
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		The API sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware.
*
******************************************************************************/
AieRC XAie_DmaSetAxi(XAie_DmaDesc *DmaDesc, u8 Smid, u8 BurstLen, u8 Qos,
		u8 Cache, u8 Secure)
{
	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	if(DmaDesc->TileType != XAIEGBL_TILE_TYPE_SHIMNOC) {
		XAIE_ERROR("No Axi properties for tile type\n");
		return XAIE_INVALID_TILE;
	}

	if((BurstLen != 4U) && (BurstLen != 8U) && (BurstLen != 16U)) {
		XAIE_ERROR("Invalid Burst length\n");
		return XAIE_INVALID_BURST_LENGTH;
	}

	DmaDesc->AxiDesc.SMID = Smid;
	DmaDesc->AxiDesc.BurstLen = BurstLen >> XAIE_SHIM_BLEN_SHIFT;
	DmaDesc->AxiDesc.AxQos = Qos;
	DmaDesc->AxiDesc.AxCache = Cache;
	DmaDesc->AxiDesc.SecureAccess = Secure;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API setups DmaDesc with parameters to run dma in interleave mode.
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	DoubleBuff: Double buffer to use(0 - A, 1-B)
* @param	IntrleaveCount: Interleaved count to use(to be 32b word aligned)
* @param	IntrleaveCurr: Interleave current pointer.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		The API sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware.
*
******************************************************************************/
AieRC XAie_DmaSetInterleaveEnable(XAie_DmaDesc *DmaDesc, u8 DoubleBuff,
		u8 IntrleaveCount, u16 IntrleaveCurr)
{
	const XAie_DmaMod *DmaMod;

	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaDesc->DmaMod;
	if(DmaMod->InterleaveMode == XAIE_FEATURE_UNAVAILABLE) {
		XAIE_ERROR("Feature unavailable\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	return DmaMod->SetIntrleave(DmaDesc, DoubleBuff, IntrleaveCount,
			IntrleaveCurr);
}

/*****************************************************************************/
/**
*
* This API writes a Dma Descriptor which is initialized and setup by other APIs
* into the corresponding registers and register fields in the hardware.
*
* @param	DevInst: Device Instance
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	Loc: Location of AIE Tile
* @param	BdNum: Hardware BD number to be written to.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_DmaWriteBd(XAie_DevInst *DevInst, XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u8 BdNum)
{
	const XAie_DmaMod *DmaMod;

	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	if(DmaDesc->TileType != DevInst->DevOps->GetTTypefromLoc(DevInst, Loc)) {
		XAIE_ERROR("Tile type mismatch\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DmaDesc->DmaMod;
	if(BdNum > DmaMod->NumBds) {
		XAIE_ERROR("Invalid BD number\n");
		return XAIE_INVALID_BD_NUM;
	}

	return DmaMod->WriteBd(DevInst, DmaDesc, Loc, BdNum);
}

/*****************************************************************************/
/**
*
* This API resets a DMA Channel for a given AIE Tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
* @param	Reset: DMA_CHANNEL_UNRESET/RESET
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Returns error for SHIMNOC tiles.
*
******************************************************************************/
AieRC XAie_DmaChannelReset(XAie_DevInst *DevInst, XAie_LocType Loc, u8 ChNum,
		XAie_DmaDirection Dir, XAie_DmaChReset Reset)
{
	u8 TileType;
	u64 Addr;
	u32 Val;
	const XAie_DmaMod *DmaMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(Dir >= DMA_MAX) {
		XAIE_ERROR("Invalid DMA direction\n");
		return XAIE_INVALID_ARGS;
	}

	if(Reset > DMA_CHANNEL_RESET) {
		XAIE_ERROR("Invalid DMA channel reset value\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if((TileType == XAIEGBL_TILE_TYPE_SHIMPL) ||
			(TileType == XAIEGBL_TILE_TYPE_SHIMNOC)) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;
	if(ChNum > DmaMod->NumChannels) {
		XAIE_ERROR("Invalid Channel number\n");
		return XAIE_INVALID_CHANNEL_NUM;
	}

	Addr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		DmaMod->ChCtrlBase + ChNum * DmaMod->ChIdxOffset +
		Dir * DmaMod->ChIdxOffset * DmaMod->NumChannels;

	Val = XAie_SetField(Reset, DmaMod->ChProp->Reset.Lsb,
			DmaMod->ChProp->Reset.Mask);

	return XAie_MaskWrite32(DevInst, Addr, Val, DmaMod->ChProp->Reset.Mask);
}

/*****************************************************************************/
/**
*
* This API resets all the Dma Channels of an AIE Tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Reset: DMA_CHANNEL_UNRESET/RESET
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Returns error for SHIMNOC tiles.
*
******************************************************************************/
AieRC XAie_DmaChannelResetAll(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_DmaChReset Reset)
{
	u8 TileType;
	AieRC RC;
	const XAie_DmaMod *DmaMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(Reset > DMA_CHANNEL_RESET) {
		XAIE_ERROR("Invalid DMA channel reset value\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;

	/* Reset MM2S */
	for(u8 i = 0U; i < DmaMod->NumChannels; i++) {
		RC = XAie_DmaChannelReset(DevInst, Loc, i, DMA_MM2S, Reset);
		if(RC != XAIE_OK) return RC;
	}

	/* Reset S2MM */
	for(u8 i = 0U; i < DmaMod->NumChannels; i++) {
		RC = XAie_DmaChannelReset(DevInst, Loc, i, DMA_S2MM, Reset);
		if(RC != XAIE_OK) return RC;
	}

	return  XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API pauses the stream via SHIM Dma for Shim NoC tiles.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the SHIM DMA Channel. (MM2S or S2MM)
* @param	Pause: XAIE_ENABLE to Pause or XAIE_DISABLE to un-pause.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		For AIE Shim Noc Tiles only.
*
******************************************************************************/
AieRC XAie_DmaChannelPauseStream(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 ChNum, XAie_DmaDirection Dir, u8 Pause)
{
	u8 TileType;
	u32 Value;
	u64 Addr;
	const XAie_DmaMod *DmaMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(Dir >= DMA_MAX) {
		XAIE_ERROR("Invalid DMA direction\n");
		return XAIE_INVALID_ARGS;
	}

	if(DevInst->DevProp.DevGen != XAIE_DEV_GEN_AIE) {
		XAIE_ERROR("Shim stream pause not supported\n");
		return XAIE_ERR;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType != XAIEGBL_TILE_TYPE_SHIMNOC) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;
	if(ChNum > DmaMod->NumChannels) {
		XAIE_ERROR("Invalid Channel number\n");
		return XAIE_INVALID_CHANNEL_NUM;
	}

	Value = XAie_SetField(Pause, DmaMod->ChProp->PauseStream.Lsb,
			DmaMod->ChProp->PauseStream.Mask);

	Addr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		DmaMod->ChCtrlBase + ChNum * DmaMod->ChIdxOffset +
		Dir * DmaMod->ChIdxOffset * DmaMod->NumChannels;

	return XAie_MaskWrite32(DevInst, Addr, DmaMod->ChProp->PauseStream.Mask,
			Value);
}

/*****************************************************************************/
/**
*
* This API pauses the AXI-MM transactions on SHIM Dma for Shim NoC tiles.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the SHIM DMA Channel. (MM2S or S2MM)
* @param	Pause: XAIE_ENABLE to Pause or XAIE_DISABLE to un-pause.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		For AIE Shim Noc Tiles only.
*
******************************************************************************/
AieRC XAie_DmaChannelPauseMem(XAie_DevInst *DevInst, XAie_LocType Loc, u8 ChNum,
		XAie_DmaDirection Dir, u8 Pause)
{
	u8 TileType;
	u32 Value;
	u64 Addr;
	const XAie_DmaMod *DmaMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(Dir >= DMA_MAX) {
		XAIE_ERROR("Invalid DMA direction\n");
		return XAIE_INVALID_ARGS;
	}

	if(DevInst->DevProp.DevGen != XAIE_DEV_GEN_AIE) {
		XAIE_ERROR("Shim stream pause not supported\n");
		return XAIE_ERR;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType != XAIEGBL_TILE_TYPE_SHIMNOC) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;
	if(ChNum > DmaMod->NumChannels) {
		XAIE_ERROR("Invalid Channel number\n");
		return XAIE_INVALID_CHANNEL_NUM;
	}

	Value = XAie_SetField(Pause, DmaMod->ChProp->PauseMem.Lsb,
			DmaMod->ChProp->PauseMem.Mask);
	Addr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		DmaMod->ChCtrlBase + ChNum * DmaMod->ChIdxOffset +
		Dir * DmaMod->ChIdxOffset * DmaMod->NumChannels;

	return XAie_MaskWrite32(DevInst, Addr, DmaMod->ChProp->PauseMem.Mask,
			Value);
}

/*****************************************************************************/
/**
*
* This API pushes a Buffer Descriptor onto the MM2S or S2MM Channel queue.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE Tile
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
* @param	BdNum: Bd number to be pushed to the queue.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		If there is no bit to enable the channel in AIE DMAs,
*		pushing a Buffer descriptor number onto the queue starts the
*		channel.
*
******************************************************************************/
AieRC XAie_DmaChannelPushBdToQueue(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 ChNum, XAie_DmaDirection Dir, u8 BdNum)
{
	AieRC RC;
	u8 TileType;
	u64 Addr;
	const XAie_DmaMod *DmaMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(Dir >= DMA_MAX) {
		XAIE_ERROR("Invalid DMA direction\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;
	if(ChNum > DmaMod->NumChannels) {
		XAIE_ERROR("Invalid Channel number\n");
		return XAIE_INVALID_CHANNEL_NUM;
	}

	if(BdNum > DmaMod->NumBds) {
		XAIE_ERROR("Invalid BD number\n");
		return XAIE_INVALID_BD_NUM;
	}

	RC = DmaMod->BdChValidity(BdNum, ChNum);
	if(RC != XAIE_OK)
		return RC;

	Addr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		DmaMod->ChCtrlBase + ChNum * DmaMod->ChIdxOffset +
		Dir * DmaMod->ChIdxOffset * DmaMod->NumChannels;

	return XAie_Write32(DevInst, Addr + (DmaMod->ChProp->StartBd.Idx * 4U),
			BdNum);
}

/*****************************************************************************/
/**
*
* This API Enables or Disables a S2MM or MM2S channel of AIE DMAs.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE Tile
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
* @param	Enable: XAIE_ENABLE/XAIE_DISABLE to enable/disable.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*
******************************************************************************/
static AieRC _XAie_DmaChannelControl(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 ChNum, XAie_DmaDirection Dir, u8 Enable)
{
	u8 TileType;
	u64 Addr;
	const XAie_DmaMod *DmaMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(Dir >= DMA_MAX) {
		XAIE_ERROR("Invalid DMA direction\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;
	if(ChNum > DmaMod->NumChannels) {
		XAIE_ERROR("Invalid Channel number\n");
		return XAIE_INVALID_CHANNEL_NUM;
	}

	Addr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		DmaMod->ChCtrlBase + ChNum * DmaMod->ChIdxOffset +
		Dir * DmaMod->ChIdxOffset * DmaMod->NumChannels;

	return XAie_MaskWrite32(DevInst,
			Addr + (DmaMod->ChProp->Enable.Idx * 4U),
			DmaMod->ChProp->Enable.Mask, Enable);
}

/*****************************************************************************/
/**
*
* This API Enables a S2MM or MM2S channel of AIE DMAs.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE Tile
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_DmaChannelEnable(XAie_DevInst *DevInst, XAie_LocType Loc, u8 ChNum,
		XAie_DmaDirection Dir)
{
	return _XAie_DmaChannelControl(DevInst, Loc, ChNum, Dir, XAIE_ENABLE);
}

/*****************************************************************************/
/**
*
* This API Disables a S2MM or MM2S channel of AIE DMAs.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE Tile
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_DmaChannelDisable(XAie_DevInst *DevInst, XAie_LocType Loc, u8 ChNum,
		XAie_DmaDirection Dir)
{
	return _XAie_DmaChannelControl(DevInst, Loc, ChNum, Dir, XAIE_DISABLE);
}

/*****************************************************************************/
/**
*
* This API is used to get the count of scheduled BDs in pending.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
* @param	PendingBd: Pointer to store the number of pending BDs.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		This function checks the number of pending BDs in the queue
* as well as if there's any BD that the channel is currently operating on.
* If multiple BDs are chained, it's counted as one BD.
*
******************************************************************************/
AieRC XAie_DmaGetPendingBdCount(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 ChNum, XAie_DmaDirection Dir, u8 *PendingBd)
{
	u8 TileType;
	const XAie_DmaMod *DmaMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(Dir >= DMA_MAX) {
		XAIE_ERROR("Invalid DMA direction\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;
	if(ChNum > DmaMod->NumChannels) {
		XAIE_ERROR("Invalid Channel number\n");
		return XAIE_INVALID_CHANNEL_NUM;
	}

	return DmaMod->PendingBd(DevInst, Loc, DmaMod, ChNum, Dir, PendingBd);
}

/*****************************************************************************/
/**
*
* This API is used to wait on Shim DMA channel to be completed.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
* @param        TimeOutUs - Minimum timeout value in micro seconds.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_DmaWaitForDone(XAie_DevInst *DevInst, XAie_LocType Loc, u8 ChNum,
		XAie_DmaDirection Dir, u32 TimeOutUs)
{
	u8 TileType;
	const XAie_DmaMod *DmaMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(Dir >= DMA_MAX) {
		XAIE_ERROR("Invalid DMA direction\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;
	if(ChNum > DmaMod->NumChannels) {
		XAIE_ERROR("Invalid Channel number\n");
		return XAIE_INVALID_CHANNEL_NUM;
	}

	if(TimeOutUs == 0U) {
		TimeOutUs = XAIE_DMA_WAITFORDONE_DEF_WAIT_TIME_US;
	}

	return DmaMod->WaitforDone(DevInst, Loc, DmaMod, ChNum, Dir, TimeOutUs);
}

/*****************************************************************************/
/**
*
* This API return the maximum queue size of the dma given a tile location.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE Tile
* @param	QueueSize: Pointer to store the queue size.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_DmaGetMaxQueueSize(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 *QueueSize)
{
	u8 TileType;

	if((DevInst == XAIE_NULL) || (QueueSize == NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	*QueueSize = XAIE_DMA_MAX_QUEUE_SIZE;
	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API pushes a Buffer Descriptor number, configures repeat count and token
* status to start channel queue.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE Tile
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
* @param	BdNum: Bd number to be pushed to the queue.
* 		RepeatCount: Number of times the task is to be repeated.
* 		EnTokenIssue: Determines whether or not issue a token when task
* 			     is completed
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		This feature is not supported for AIE. For AIE-ML the enable
*		token issue can be XAIE_ENABLE or XAIE_DISABLE.
*		This API doesn't support out of order.
*
******************************************************************************/
AieRC XAie_DmaChannelSetStartQueue(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 ChNum, XAie_DmaDirection Dir, u8 BdNum, u32 RepeatCount,
		u8 EnTokenIssue)
{
	XAie_DmaDeclareQueueConfig(DmaQueueDesc, BdNum, RepeatCount,
			EnTokenIssue, XAIE_DISABLE);

	return XAie_DmaChannelSetStartQueueGeneric(DevInst, Loc, ChNum, Dir,
			&DmaQueueDesc);
}

/*****************************************************************************/
/**
*
* This API pushes a Buffer Descriptor number, configures repeat count and token
* status to start channel queue.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE Tile
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
* @param	DmaQueueDesc: Pointer of DMA queue descriptor
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		This feature is not supported for AIE.
*
******************************************************************************/
AieRC XAie_DmaChannelSetStartQueueGeneric(XAie_DevInst *DevInst,
		XAie_LocType Loc, u8 ChNum, XAie_DmaDirection Dir,
		XAie_DmaQueueDesc *DmaQueueDesc)
{
	AieRC RC;
	u8 TileType;
	u8 StartBd;
	u32 Val = 0;
	u64 Addr;
	const XAie_DmaMod *DmaMod;

	if((DevInst == XAIE_NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance to start queue\n");
		return XAIE_INVALID_ARGS;
	}

	if(DmaQueueDesc == XAIE_NULL) {
		XAIE_ERROR("Invalid Dma queue description pointer to start queue.\n");
		return XAIE_INVALID_ARGS;
	}

	if(Dir >= DMA_MAX) {
		XAIE_ERROR("Invalid DMA direction\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_SHIMPL ||
		TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid Tile Type to start queue\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;
	if(DmaMod->RepeatCount == XAIE_FEATURE_UNAVAILABLE) {
		XAIE_ERROR("Repeat count feature in start queue is not supported for this device generation\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	if(DmaMod->EnTokenIssue == XAIE_FEATURE_UNAVAILABLE) {
		XAIE_ERROR("Enable token issue feature in start queue is not supported for this device generation\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	if(ChNum > DmaMod->NumChannels) {
		XAIE_ERROR("Invalid Channel number\n");
		return XAIE_INVALID_CHANNEL_NUM;
	}

	if((DmaQueueDesc->RepeatCount < 1U) ||
		(DmaQueueDesc->RepeatCount > DmaMod->ChProp->MaxRepeatCount)) {
		XAIE_ERROR("Invalid Repeat Count: %d\n",
			DmaQueueDesc->RepeatCount);
		return XAIE_INVALID_ARGS;
	}

	if(DmaQueueDesc->OutOfOrder != XAIE_ENABLE) {
		StartBd = DmaQueueDesc->StartBd;
		if(StartBd > DmaMod->NumBds) {
			XAIE_ERROR("Invalid BD number\n");
			return XAIE_INVALID_BD_NUM;
		}
		RC = DmaMod->BdChValidity(StartBd, ChNum);
		if(RC != XAIE_OK) {
			return RC;
		}
	} else {
		StartBd = 0;
	}

	Addr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		DmaMod->StartQueueBase + ChNum * DmaMod->ChIdxOffset +
		Dir * DmaMod->ChIdxOffset * DmaMod->NumChannels;

	Val = XAie_SetField(StartBd, DmaMod->ChProp->StartBd.Lsb,
			DmaMod->ChProp->StartBd.Mask) |
		XAie_SetField((DmaQueueDesc->RepeatCount - 1),
			DmaMod->ChProp->RptCount.Lsb,
			DmaMod->ChProp->RptCount.Mask) |
		XAie_SetField(DmaQueueDesc->EnTokenIssue,
			DmaMod->ChProp->EnToken.Lsb,
			DmaMod->ChProp->EnToken.Mask);

	return XAie_Write32(DevInst, Addr, Val);
}

/*****************************************************************************/
/**
*
* This API updates the length of the buffer descriptor in the dma module.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE Tile
* @param	Len: Length of BD in bytes.
* @param	BdNum: Hardware BD number to be written to.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		This API accesses the hardware directly and does not operate
*		on software descriptor.
******************************************************************************/
AieRC XAie_DmaUpdateBdLen(XAie_DevInst *DevInst, XAie_LocType Loc, u32 Len,
		u8 BdNum)
{
	const XAie_DmaMod *DmaMod;
	u32 AdjustedLen;
	u8 TileType;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;
	if(BdNum > DmaMod->NumBds) {
		XAIE_ERROR("Invalid BD number\n");
		return XAIE_INVALID_BD_NUM;
	}

	AdjustedLen = (Len >> XAIE_DMA_32BIT_TXFER_LEN) -
		DmaMod->BdProp->LenActualOffset;


	return DmaMod->UpdateBdLen(DevInst, DmaMod, Loc, AdjustedLen, BdNum);
}

/*****************************************************************************/
/**
*
* This API updates the address of the buffer descriptor in the dma module.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE Tile
* @param	Addr: Buffer address
* @param	BdNum: Hardware BD number to be written to.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		This API accesses the hardware directly and does not operate
*		on software descriptor.
******************************************************************************/
AieRC XAie_DmaUpdateBdAddr(XAie_DevInst *DevInst, XAie_LocType Loc, u64 Addr,
		u8 BdNum)
{
	const XAie_DmaMod *DmaMod;
	u8 TileType;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;
	if(BdNum > DmaMod->NumBds) {
		XAIE_ERROR("Invalid BD number\n");
		return XAIE_INVALID_BD_NUM;
	}

	if(((Addr & DmaMod->BdProp->AddrAlignMask) != 0U) ||
			(Addr > DmaMod->BdProp->AddrMax)) {
		XAIE_ERROR("Invalid Address\n");
		return XAIE_INVALID_ADDRESS;
	}

	return DmaMod->UpdateBdAddr(DevInst, DmaMod, Loc, Addr, BdNum);
}

/*****************************************************************************/
/**
*
* This API initializes the Dma Channel Descriptor for AIE DMAs for a given
* location.
*
* @param	DevInst: Device Instance.
* @param	DmaChannelDesc: Pointer to user allocated dma channel descriptor
* @param	Loc: Location of AIE-ML Tile
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		This API should be called before setting individual fields for
* 		Dma Channel Descriptor. This API works only for AIE-ML and has
* 		no effect on AIE.
*
******************************************************************************/
AieRC XAie_DmaChannelDescInit(XAie_DevInst *DevInst,
		XAie_DmaChannelDesc *DmaChannelDesc, XAie_LocType Loc)
{
	u8 TileType;
	const XAie_DmaMod *DmaMod;

	if((DevInst == XAIE_NULL) || (DmaChannelDesc == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;

	memset((void *)DmaChannelDesc, 0U, sizeof(XAie_DmaChannelDesc));

	DmaChannelDesc->TileType = TileType;
	DmaChannelDesc->IsReady = XAIE_COMPONENT_IS_READY;
	DmaChannelDesc->DmaMod = DmaMod;

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API enables/disables the Encompression bit in the DMA Channel Descriptor.
*
* @param	DmaChannelDesc: Initialized Dma channel Descriptor.
* @param	EnCompression: XAIE_ENABLE or XAIE_DISABLE for
* 				Compression / Decompression.
* @return	XAIE_OK on success, Error code on failure.
*
* @note		The API sets the enable/disable choice for
* 		Compression / Decompression in dma channel descriptor and does
* 		not configure the descriptor field in the hardware.
* 		This API works only for AIE-ML and has no effect on AIE.
*
******************************************************************************/
AieRC XAie_DmaChannelEnCompression(XAie_DmaChannelDesc *DmaChannelDesc,
		u8 EnCompression)
{
	const XAie_DmaMod *DmaMod;

	if((DmaChannelDesc == XAIE_NULL) ||
			(DmaChannelDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaChannelDesc->DmaMod;
	if(DmaMod->ChProp->HasEnCompression == XAIE_FEATURE_UNAVAILABLE) {
		XAIE_ERROR("Feature not supported\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	DmaChannelDesc->EnCompression = EnCompression;

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API enables/disables out of order mode in the DMA Channel Descriptor.
*
* @param	DmaChannelDesc: Initialized Dma Channel Descriptor.
* @param	EnOutofOrder: XAIE_ENABLE or XAIE_DISABLE for out of order mode.
* 				XAIE_DISABLE = in-order mode.
* @return	XAIE_OK on success, Error code on failure.
*
* @note		The API sets the enable/disable choice for
* 		out of order mode in dma channel descriptor and does
* 		not configure the descriptor field in the hardware.
* 		This API works only for AIE-ML and has no effect on AIE.
*
******************************************************************************/
AieRC XAie_DmaChannelEnOutofOrder(XAie_DmaChannelDesc *DmaChannelDesc,
		u8 EnOutofOrder)
{
	const XAie_DmaMod *DmaMod;

	if((DmaChannelDesc == XAIE_NULL) ||
			(DmaChannelDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaChannelDesc->DmaMod;
	if(DmaMod->ChProp->HasEnOutOfOrder == XAIE_FEATURE_UNAVAILABLE) {
		XAIE_ERROR("Feature not supported\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	DmaChannelDesc->EnOutofOrderId = EnOutofOrder;

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This api sets the task complete token controller id field in the DMA Channel
* Descriptor.
*
* @param	DmaChannelDesc: Initialized dma channel descriptor.
* @param	ControllerId: Task-complete-token controller ID field
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		The api sets the controller id in the dma channel descriptor and
*		does not configure the descriptor field in the hardware. This
*		API works only for AIE-ML and has no effect on AIE.
*
******************************************************************************/
AieRC XAie_DmaChannelSetControllerId(XAie_DmaChannelDesc *DmaChannelDesc,
		u32 ControllerId)
{
	const XAie_DmaMod *DmaMod;

	if((DmaChannelDesc == XAIE_NULL) ||
			(DmaChannelDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaChannelDesc->DmaMod;
	if(DmaMod->ChProp->HasControllerId == XAIE_FEATURE_UNAVAILABLE) {
		XAIE_ERROR("Feature unavailable\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	if(ControllerId > DmaMod->ChProp->ControllerId.Mask >>
			DmaMod->ChProp->ControllerId.Lsb) {
		XAIE_ERROR("Invalid ControllerId: %d\n", ControllerId);
		return XAIE_INVALID_ARGS;
	}

	DmaChannelDesc->ControllerId = ControllerId;

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This api sets the task complete token controller id field in the DMA Channel
* Descriptor.
*
* @param	DmaChannelDesc: Initialized dma channel descriptor.
* @param	FoTMode: Finish on TLAST mode (DMA_FoT_DISABLED,
* 			DMA_FoT_NO_COUNTS, DMA_FoT_COUNTS_WITH_TASK_TOKENS or
*			DMA_FoT_COUNTS_FROM_MM_REG)
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		The api sets the FoT Mode in the dma channel descriptor and
*		does not configure the descriptor field in the hardware. This
*		API works only for AIE-ML and has no effect on AIE.
*
******************************************************************************/
AieRC XAie_DmaChannelSetFoTMode(XAie_DmaChannelDesc *DmaChannelDesc,
		XAie_DmaChannelFoTMode FoTMode)
{
	const XAie_DmaMod *DmaMod;

	if((DmaChannelDesc == XAIE_NULL) ||
			(DmaChannelDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaChannelDesc->DmaMod;
	if(DmaMod->ChProp->HasFoTMode == XAIE_FEATURE_UNAVAILABLE) {
		XAIE_ERROR("Feature unavailable\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	if(FoTMode > DmaMod->ChProp->MaxFoTMode) {
		XAIE_ERROR("Invalid FoTMode: %d\n", FoTMode);
		return XAIE_INVALID_ARGS;
	}

	DmaChannelDesc->FoTMode = FoTMode;

	return XAIE_OK;
}

/*****************************************************************************/
/**
=======
* This API configures the Dma channel descriptor fields in the hardware for a
* particular tile location. This includes FoT mode, Controller id, out of order
* and Compression/Decompression.
*
*
* @param	DevInst: Device Instance.
* @param	DmaChannelDesc: Initialized Dma Channel Descriptor.
* @param	Loc: Location of AIE Tile
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		This API works only for AIE-ML and has no effect on AIE.
*
******************************************************************************/
AieRC XAie_DmaWriteChannel(XAie_DevInst *DevInst,
		XAie_DmaChannelDesc *DmaChannelDesc, XAie_LocType Loc,
		u8 ChNum, XAie_DmaDirection Dir)
{
	u64 Addr;
	u32 Val;
	const XAie_DmaMod *DmaMod;

	if((DevInst == XAIE_NULL) || (DmaChannelDesc == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	if(DevInst->DevProp.DevGen == XAIE_DEV_GEN_AIE) {
		XAIE_ERROR("Feature not supported\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	if(Dir >= DMA_MAX) {
		XAIE_ERROR("Invalid DMA direction\n");
		return XAIE_INVALID_ARGS;
	}

	if((DmaChannelDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Dma Channel descriptor not initilized\n");
		return XAIE_INVALID_DMA_DESC;
	}

	if(DmaChannelDesc->TileType != DevInst->DevOps->GetTTypefromLoc(DevInst, Loc)) {
		XAIE_ERROR("Tile type mismatch\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[DmaChannelDesc->TileType].DmaMod;
	if(ChNum > DmaMod->NumChannels) {
		XAIE_ERROR("Invalid Channel number\n");
		return XAIE_INVALID_CHANNEL_NUM;
	}

	Addr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		DmaMod->ChCtrlBase + ChNum * DmaMod->ChIdxOffset +
		Dir * DmaMod->ChIdxOffset * DmaMod->NumChannels;

	Val = XAie_SetField(DmaChannelDesc->EnOutofOrderId, (DmaMod->ChProp->EnOutofOrder.Lsb),
			(DmaMod->ChProp->EnOutofOrder.Mask)) |
		XAie_SetField(DmaChannelDesc->EnCompression, (DmaMod->ChProp->EnCompression.Lsb),
			(DmaMod->ChProp->EnCompression.Mask)) |
		XAie_SetField(DmaChannelDesc->ControllerId, (DmaMod->ChProp->ControllerId.Lsb),
			(DmaMod->ChProp->ControllerId.Mask)) |
		XAie_SetField(DmaChannelDesc->FoTMode, (DmaMod->ChProp->FoTMode.Lsb),
			(DmaMod->ChProp->FoTMode.Mask));

	return XAie_Write32(DevInst, Addr, Val);
}

/******************************************************************************/
/**
* This api sets the number of 32 bit words to be added before and after each
* dimenstion for a DMA BD descriptor.
*
* @param	DmaDesc: Initialized dma descriptor.
* @param	PadTensor: Padding tensor with After and Before values for each
*		dimension.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		The API sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware.
*
*******************************************************************************/
AieRC XAie_DmaSetPadding(XAie_DmaDesc *DmaDesc, XAie_DmaPadTensor *PadTensor)
{
	const XAie_DmaMod *DmaMod;
	if((DmaDesc == XAIE_NULL) || (PadTensor == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaDesc->DmaMod;
	if(DmaMod->ZeroPadding == XAIE_FEATURE_UNAVAILABLE) {
		XAIE_ERROR("Feature unavailable\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	for(u8 i = 0U; i < PadTensor->NumDim; i++) {
		DmaDesc->PadDesc[i].After = PadTensor->PadDesc[i].After;
		DmaDesc->PadDesc[i].Before = PadTensor->PadDesc[i].Before;
	}

	return XAIE_OK;
}

/******************************************************************************/
/**
* This api sets the number of zeros to be added before or after  the given
* dimension for a dma descriptor.
*
* @param	DmaDesc: Initialized dma descriptor.
* @param	NumZeros: No. of zeros to be added
* @param	Dim: Dimension - 0, 1 or 2.
* @param	Pos: Position for zeros padding i.e.
* 			DMA_ZERO_PADDING_BEFORE or DMA_ZERO_PADDING_AFTER
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		The API sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware.
*
*******************************************************************************/
AieRC XAie_DmaSetZeroPadding(XAie_DmaDesc *DmaDesc, u8 Dim,
		XAie_DmaZeroPaddingPos Pos, u8 NumZeros)
{
	const XAie_DmaMod *DmaMod;
	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaDesc->DmaMod;
	if(DmaMod->ZeroPadding == XAIE_FEATURE_UNAVAILABLE) {
		XAIE_ERROR("Feature unavailable\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	switch(Pos){
	case DMA_ZERO_PADDING_BEFORE: {
		switch(Dim){
		case 0U:
		case 1U:
		case 2U:
		{
			DmaDesc->PadDesc[Dim].Before = NumZeros;
			break;
		}
		default:
			XAIE_ERROR("Invalid Dimension: %d\n", Dim);
			return XAIE_INVALID_ARGS;
		}
		break;
	}

	case DMA_ZERO_PADDING_AFTER: {
		switch(Dim){
		case 0U:
		case 1U:
		case 2U:
		{
			DmaDesc->PadDesc[Dim].After = NumZeros;
			break;
		}
		default:
			XAIE_ERROR("Invalid Dimension: %d\n", Dim);
			return XAIE_INVALID_ARGS;
		}
		break;
	}
	default:
		XAIE_ERROR("Invalid Position: %d\n", Pos);
		return XAIE_INVALID_ARGS;
	}

	return XAIE_OK;
}

/******************************************************************************/
/**
* This api enables the DMA to assert TLAST signal after the DMA transfer is
* complete. By default, the TLAST assertion is enabled in the DmaDesc during
* dma descriptor initialization.
*
* @param	DmaDesc: Initialized dma descriptor.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		The API sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware.
*
*******************************************************************************/
AieRC XAie_DmaTlastEnable(XAie_DmaDesc *DmaDesc)
{
	const XAie_DmaMod *DmaMod;
	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaDesc->DmaMod;
	if(DmaMod->TlastSuppress == XAIE_FEATURE_UNAVAILABLE) {
		XAIE_ERROR("Feature unavailable\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	DmaDesc->TlastSuppress = XAIE_DISABLE;

	return XAIE_OK;
}

/******************************************************************************/
/**
* This api configures the DMA descriptor to not assert the TLAST signal after
* the DMA transfer is complete. By default, the TLAST assertion is enabled in
* the DmaDesc during dma descriptor initialization.
*
* @param	DmaDesc: Initialized dma descriptor.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		The API sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware.
*
*******************************************************************************/
AieRC XAie_DmaTlastDisable(XAie_DmaDesc *DmaDesc)
{
	const XAie_DmaMod *DmaMod;
	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaDesc->DmaMod;
	if(DmaMod->TlastSuppress == XAIE_FEATURE_UNAVAILABLE) {
		XAIE_ERROR("Feature unavailable\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	DmaDesc->TlastSuppress = XAIE_ENABLE;

	return XAIE_OK;
}

#endif /* XAIE_FEATURE_DMA_ENABLE */
/** @} */
