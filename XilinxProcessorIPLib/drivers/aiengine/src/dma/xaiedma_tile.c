/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaiedma_tile.c
* @{
*
* This file contains the routines to initialize and configure the Tile DMA.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Naresh  04/06/2018  Initial creation
* 1.1  Naresh  04/12/2018  Added workaround for CRVO#1692
* 1.2  Naresh  06/20/2018  Fixed CR#1005445
* 1.3  Naresh  07/11/2018  Updated copyright info
* 1.4  Nishad  12/05/2018  Renamed ME attributes to AIE
* 1.5  Hyun    01/08/2019  Don't poll the status after control change
* 1.6  Nishad  03/20/2019  Fix the use of uninitialized pointer in
* 			   XAieDma_TileBdSetLock function
* 1.7  Hyun    06/20/2019  Remove the duplicate initialization on BD
* 1.8  Hyun    06/20/2019  Add XAieDma_TileBdClearAll() that resets all sw BDs
* 1.9  Hyun    06/20/2019  Added APIs for individual BD / Channel reset
* 2.0  Hyun    06/20/2019  Add XAieDma_TileSoftInitialize()
* </pre>
*
******************************************************************************/
#include "xaiegbl_defs.h"
#include "xaiegbl.h"
#include "xaiedma_tile.h"

/***************************** Include Files *********************************/

/***************************** Macro Definitions *****************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API intializes the Tile DMA instances without programming hardware
* resource. This allows to control individual resources independently.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	DmaInstPtr - Pointer to the Tile DMA instance.
*
* @return	XAIE_SUCCESS if successful, else XAIE_FAILURE.
*
* @note		This API is required to be invoked first in the sequence,
*		before any of the other Tile DMA driver APIs are used.
*
*******************************************************************************/
u32 XAieDma_TileSoftInitialize(XAieGbl_Tile *TileInstPtr, XAieDma_Tile *DmaInstPtr)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(DmaInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);

	DmaInstPtr->BaseAddress = TileInstPtr->TileAddr;
	DmaInstPtr->IsReady = XAIE_COMPONENT_IS_READY;
	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API intializes the Tile DMA BDs to their default values and followed by
* a reset and disable of all the 4 channels (S2MM0, S2MM1, MM2S0, MM2S1).
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	DmaInstPtr - Pointer to the Tile DMA instance.
*
* @return	XAIE_SUCCESS if successful, else XAIE_FAILURE.
*
* @note		This API is required to be invoked first in the sequence,
*		before any of the other Tile DMA driver APIs are used.
*
*******************************************************************************/
u32 XAieDma_TileInitialize(XAieGbl_Tile *TileInstPtr, XAieDma_Tile *DmaInstPtr)
{
	u32 Status = XAIE_FAILURE;
        u8 BdIdx;
	u8 ChNum;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(DmaInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);

#ifndef XAIE_WRKARND_CRVO1692
        if(DmaInstPtr->IsReady == XAIE_COMPONENT_IS_READY) {
                /* DMA already initialized */
                return XAIE_SUCCESS;
        }
#endif

	/* Get the BD base address */
	DmaInstPtr->BaseAddress = TileInstPtr->TileAddr;

	/* Clear the BD entries in the DMA instance structure */
	for(BdIdx = 0U; BdIdx < XAIEDMA_TILE_MAX_NUM_DESCRS; BdIdx++) {
		XAieDma_TileBdClear(DmaInstPtr, BdIdx);
	}

#ifndef XAIE_WRKARND_CRVO1692
	/* Reset and disable all the channels */
	for(ChNum = 0U; ChNum < XAIEDMA_TILE_MAX_NUM_CHANNELS; ChNum++) {
		/* Reset the channel */
		Status = XAieDma_TileChControl(DmaInstPtr, ChNum,
                                                XAIE_RESETENABLE, XAIE_DISABLE);
                if(Status == XAIE_SUCCESS) {
        		/* Unreset and Disable the channel */
        		Status = XAieDma_TileChControl(DmaInstPtr, ChNum,
						XAIE_RESETDISABLE, XAIE_DISABLE);

                        if(Status == XAIE_SUCCESS) {
                		/* Set Start BD to the reset value */
                		XAieDma_TileSetStartBd(DmaInstPtr, ChNum,
						XAIEDMA_TILE_STARTBD_RESET);
                        } else {
                                break;
                        }
                } else {
                        break;
                }
	}
#endif

        if(Status == XAIE_SUCCESS) {
                DmaInstPtr->IsReady = XAIE_COMPONENT_IS_READY;
        }

        return Status;
}


/*****************************************************************************/
/**
*
* This API is to configure the Lock release and acquire attributes for the
* selected BD entry in the Tile DMA instance which is later written to the
* the physical BD location.
*
* @param	DmaInstPtr - Pointer to the Tile DMA instance.
* @param	BdNum - BD index whose attributes need to be set to.
* @param	AbType - A or B type in double buffering (0-AddrA, 1-AddrB).
* @param	LockId - Lock index value, ranging from 0-15.
* @param	LockRelEn - Enable/Disable lock release (1-Enable,0-Disable).
* @param	LockRelVal - Lock release value (Valid:0/1 & Invalid:0xFF).
* @param	LockAcqEn - Enable/Disable lock acquire (1-Enable,0-Disable).
* @param	LockAcqVal - Lock acquire value (Valid:0/1 & Invalid:0xFF).
*
* @return	None.
*
* @note		This doesn't set the values in the BD physical location.
*
*******************************************************************************/
void XAieDma_TileBdSetLock(XAieDma_Tile *DmaInstPtr, u8 BdNum, u8 AbType,
	u8 LockId, u8 LockRelEn, u8 LockRelVal, u8 LockAcqEn, u8 LockAcqVal)
{
	XAieDma_TileBdLock *AddrPtr;

	XAie_AssertNonvoid(DmaInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(BdNum < XAIEDMA_TILE_MAX_NUM_DESCRS);
	XAie_AssertNonvoid(LockId < XAIEDMA_TILE_MAX_NUM_LOCKS);
	XAie_AssertNonvoid(AbType == XAIEDMA_TILE_BD_ADDRA ||
				AbType == XAIEDMA_TILE_BD_ADDRB);

	if(AbType == XAIEDMA_TILE_BD_ADDRA) {
		AddrPtr = (XAieDma_TileBdLock *)
					&(DmaInstPtr->Descrs[BdNum].AddrA);
	} else {
		AddrPtr = (XAieDma_TileBdLock *)
					&(DmaInstPtr->Descrs[BdNum].AddrB);
	}

	AddrPtr->LockId = LockId;

	AddrPtr->LkRelEn = LockRelEn;
	/* If lock release value is invalid,then lock released with no value */
	if(LockRelVal != XAIEDMA_TILE_LOCK_ACQRELVAL_INVALID) {
		AddrPtr->LkRelValEn = XAIE_ENABLE;
		AddrPtr->LkRelVal = LockRelVal;
	}

	AddrPtr->LkAcqEn = LockAcqEn;
	/* If lock acquire value is invalid,then lock acquired with no value */
	if(LockAcqVal != XAIEDMA_TILE_LOCK_ACQRELVAL_INVALID) {
		AddrPtr->LkAcqValEn = XAIE_ENABLE;
		AddrPtr->LkAcqVal = LockAcqVal;
	}
}

/*****************************************************************************/
/**
*
* This API configures the 2D X or Y attributes for the selected BD entry in
* the Tile DMA instance.
*
* @param	DmaInstPtr - Pointer to the Tile DMA instance.
* @param	BdNum - BD index whose attributes need to be set to.
* @param	XyType - X oy Y addressing attributes (0-X, 1-Y).
* @param	Incr - Increment to be used for X/Y addressing.
* @param	Wrap - Wrap to be used for X/Y addressing.
* @param	Offset - Offset to be used for X/Y addressing.
*
* @return	None.
*
* @note		This doesn't set the values in the BD physical location.
*
*******************************************************************************/
void XAieDma_TileBdSetXy2d(XAieDma_Tile *DmaInstPtr, u8 BdNum, u8 XyType, u16 Incr,
							u16 Wrap, u16 Offset)
{
	XAieDma_TileBd *DescrPtr;

	XAie_AssertNonvoid(DmaInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(BdNum < XAIEDMA_TILE_MAX_NUM_DESCRS);

	DescrPtr = (XAieDma_TileBd *)&(DmaInstPtr->Descrs[BdNum]);

	if(XyType == XAIEDMA_TILE_BD_2DDMA_X) {
		DescrPtr->X2dCfg.Incr = Incr - 1U; /* Act value - 1 */
		DescrPtr->X2dCfg.Wrap = Wrap - 1U; /* Act value - 1 */
		DescrPtr->X2dCfg.Offset = Offset;
	} else if(XyType == XAIEDMA_TILE_BD_2DDMA_Y) {
		DescrPtr->Y2dCfg.Incr = Incr - 1U; /* Act value - 1 */
		DescrPtr->Y2dCfg.Wrap = Wrap - 1U; /* Act value - 1 */
		DescrPtr->Y2dCfg.Offset = Offset;
	}
}

/*****************************************************************************/
/**
*
* This API configures the Interleave attributes for the selected BD entry in
* the Tile DMA instance.
*
* @param	DmaInstPtr - Pointer to the Tile DMA instance.
* @param	BdNum - BD index whose attributes need to be set to.
* @param	IntlvMode - Enable/Disable Interleave mode(1-Enable,0-Disable).
* @param	IntlvDb - Double buffer to use, A or B.
* @param	IntlvCnt - Interleaved count to use (to be 32b word aligned).
* @param	IntlvCur - Interleave current pointer.
*
* @return	None.
*
* @note		This doesn't set the values in the BD physical location.
*
*******************************************************************************/
void XAieDma_TileBdSetIntlv(XAieDma_Tile *DmaInstPtr, u8 BdNum, u8 IntlvMode,
					u8 IntlvDb, u8 IntlvCnt, u16 IntlvCur)
{
	XAieDma_TileBd *DescrPtr;

	XAie_AssertNonvoid(DmaInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(BdNum < XAIEDMA_TILE_MAX_NUM_DESCRS);

	/* 32 bit word aligned */
	XAie_AssertNonvoid((IntlvCnt & XAIEDMA_TILE_INTLVCNT_ALIGN_MASK) == 0U);

	DescrPtr = (XAieDma_TileBd *)&(DmaInstPtr->Descrs[BdNum]);

	DescrPtr->IntlvMode = IntlvMode;
	DescrPtr->IntlvDb = IntlvDb;
	DescrPtr->IntlvCnt = IntlvCnt;
	DescrPtr->IntlvCur = IntlvCur;
}

/*****************************************************************************/
/**
*
* This API configures the Packet attributes of the selected BD entry in the
* Tile DMA instance.
*
* @param	DmaInstPtr - Pointer to the Tile DMA instance.
* @param	BdNum - BD index whose attributes need to be set to.
* @param	PktEn - Enable/Disable Pkt switching mode (1-Enable,0-Disable).
* @param	PktType - Packet type.
* @param	PktId - ID value for the packet.
*
* @return	None.
*
* @note		This doesn't set the values in the BD physical location.
*
*******************************************************************************/
void XAieDma_TileBdSetPkt(XAieDma_Tile *DmaInstPtr, u8 BdNum, u8 PktEn,
						u8 PktType, u8 PktId)
{
	XAieDma_TileBd *DescrPtr;

	XAie_AssertNonvoid(DmaInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(BdNum < XAIEDMA_TILE_MAX_NUM_DESCRS);

	DescrPtr = (XAieDma_TileBd *)&(DmaInstPtr->Descrs[BdNum]);

	DescrPtr->PktEn = PktEn;
	DescrPtr->PktType = PktType;
	DescrPtr->PktId = PktId;
}

/*****************************************************************************/
/**
*
* This API configures the address, length and mode attributes for the
* selected BD entry in the Tile DMA instance.
*
* @param	DmaInstPtr - Pointer to the Tile DMA instance.
* @param	BdNum - BD index whose attributes need to be set to.
* @param	BaseAddrA - AddrA base address (to be in 32b words).
* @param	BaseAddrB - AddrB base address (to be in 32b words).
* @param	Length - Transfer length in bytes.
* @param	AbMode - Enable/disable double buffer mode(1-Enable,0-Disable).
* @param	FifoMode - 0 to disable.
* XAIEDMA_TILE_FIFO_CNT0 or XAIE_DMA_TILE_FIFO_CNT1 to enable.
*
* @return	None.
*
* @note		This doesn't set the values in the BD physical location.
*
*******************************************************************************/
void XAieDma_TileBdSetAdrLenMod(XAieDma_Tile *DmaInstPtr, u8 BdNum, u16 BaseAddrA,
			u16 BaseAddrB, u16 Length, u8 AbMode, u8 FifoMode)
{
	XAieDma_TileBd *DescrPtr;
	u32 LenMask = 0U;

	XAie_AssertNonvoid(DmaInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(BdNum < XAIEDMA_TILE_MAX_NUM_DESCRS);
	XAie_AssertNonvoid((FifoMode == 0) ||
			(FifoMode == XAIEDMA_TILE_FIFO_CNT0) ||
			(FifoMode == XAIEDMA_TILE_FIFO_CNT1));

	/* Base address A to be in 32 bit words */
	XAie_AssertNonvoid((BaseAddrA & XAIEDMA_TILE_ADDRAB_ALIGN_MASK) == 0U);

	/* Base address B to be in 32 bit words */
	XAie_AssertNonvoid((BaseAddrB & XAIEDMA_TILE_ADDRAB_ALIGN_MASK) == 0U);

	Length = Length >> XAIEDMA_TILE_LENGTH32_OFFSET;
	if(FifoMode != 0U) {
		LenMask = XAIEDMA_TILE_LENGTH128_MASK;
	}

	XAie_AssertNonvoid((Length & LenMask) == 0U);

	DescrPtr = (XAieDma_TileBd *)&(DmaInstPtr->Descrs[BdNum]);

	DescrPtr->AddrA.BaseAddr = BaseAddrA >>XAIEDMA_TILE_ADDRAB_ALIGN_OFFSET;
	DescrPtr->AddrB.BaseAddr = BaseAddrB >>XAIEDMA_TILE_ADDRAB_ALIGN_OFFSET;
	DescrPtr->Length = Length - 1U;
	DescrPtr->AbMode = AbMode;
	DescrPtr->FifoMode = FifoMode;
}

/*****************************************************************************/
/**
*
* This API configures the Next BD attribute for the selected BD entry in the
* Tile DMA instance.
*
* @param	DmaInstPtr - Pointer to the Tile DMA instance.
* @param	BdNum - BD index whose attributes need to be set to.
* @param	NextBd - Next BD to use, ranging from 0-15. If NextBd==0xFF,
*		it is deemed invalid and UseNextBd is set to 0.
*
* @return	None.
*
* @note		This doesn't set the values in the BD physical location.
*
*******************************************************************************/
void XAieDma_TileBdSetNext(XAieDma_Tile *DmaInstPtr, u8 BdNum, u8 NextBd)
{
	XAieDma_TileBd *DescrPtr;

	XAie_AssertNonvoid(DmaInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(BdNum < XAIEDMA_TILE_MAX_NUM_DESCRS);
	XAie_AssertNonvoid(NextBd < XAIEDMA_TILE_MAX_NUM_DESCRS);

	DescrPtr = (XAieDma_TileBd *)&(DmaInstPtr->Descrs[BdNum]);

	DescrPtr->NextBd = NextBd;

	/* Use next BD only if the Next BD value is not invaliud */
	if(NextBd != XAIEDMA_TILE_BD_NEXTBD_INVALID) {
		DescrPtr->NextBdEn = XAIE_ENABLE;
	} else {
		DescrPtr->NextBdEn = XAIE_DISABLE;
	}
}

/*****************************************************************************/
/**
*
* This API writes all the attributes of the selected BD entry in the Tile DMA
* instance to the actual physical BD location.
*
* @param	DmaInstPtr - Pointer to the Tile DMA instance.
* @param	BdNum - BD index whose attributes need to be written to the
*		corresponding physical location.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieDma_TileBdWrite(XAieDma_Tile *DmaInstPtr, u8 BdNum)
{
	u64 BdAddr;
	u32 BdWord[XAIEDMA_TILE_NUM_BD_WORDS];
	u32 Idx;
	XAieDma_TileBd *DescrPtr;

	DescrPtr = (XAieDma_TileBd *)&(DmaInstPtr->Descrs[BdNum]);

	BdWord[0U] = XAie_SetField(DescrPtr->AddrA.LockId,
				TileBd[BdNum].AddA.LkId.Lsb,
				TileBd[BdNum].AddA.LkId.Mask) |
			XAie_SetField(DescrPtr->AddrA.LkRelEn,
				TileBd[BdNum].AddA.RelEn.Lsb,
				TileBd[BdNum].AddA.RelEn.Mask) |
			XAie_SetField(DescrPtr->AddrA.LkRelVal,
				TileBd[BdNum].AddA.RelVal.Lsb,
				TileBd[BdNum].AddA.RelVal.Mask) |
			XAie_SetField(DescrPtr->AddrA.LkRelValEn,
				TileBd[BdNum].AddA.RelValEn.Lsb,
				TileBd[BdNum].AddA.RelValEn.Mask)|
			XAie_SetField(DescrPtr->AddrA.LkAcqEn,
				TileBd[BdNum].AddA.AcqEn.Lsb,
				TileBd[BdNum].AddA.AcqEn.Mask) |
			XAie_SetField(DescrPtr->AddrA.LkAcqVal,
				TileBd[BdNum].AddA.AcqVal.Lsb,
				TileBd[BdNum].AddA.AcqVal.Mask) |
			XAie_SetField(DescrPtr->AddrA.LkAcqValEn,
				TileBd[BdNum].AddA.AcqValEn.Lsb,
				TileBd[BdNum].AddA.AcqValEn.Mask)|
			XAie_SetField(DescrPtr->AddrA.BaseAddr,
				TileBd[BdNum].AddA.Base.Lsb,
				TileBd[BdNum].AddA.Base.Mask);

	BdWord[1U] = XAie_SetField(DescrPtr->AddrB.LockId,
				TileBd[BdNum].AddB.LkId.Lsb,
				TileBd[BdNum].AddB.LkId.Mask) |
			XAie_SetField(DescrPtr->AddrB.LkRelEn,
				TileBd[BdNum].AddB.RelEn.Lsb,
				TileBd[BdNum].AddB.RelEn.Mask) |
			XAie_SetField(DescrPtr->AddrB.LkRelVal,
				TileBd[BdNum].AddB.RelVal.Lsb,
				TileBd[BdNum].AddB.RelVal.Mask) |
			XAie_SetField(DescrPtr->AddrB.LkRelValEn,
				TileBd[BdNum].AddB.RelValEn.Lsb,
				TileBd[BdNum].AddB.RelValEn.Mask)|
			XAie_SetField(DescrPtr->AddrB.LkAcqEn,
				TileBd[BdNum].AddB.AcqEn.Lsb,
				TileBd[BdNum].AddB.AcqEn.Mask) |
			XAie_SetField(DescrPtr->AddrB.LkAcqVal,
				TileBd[BdNum].AddB.AcqVal.Lsb,
				TileBd[BdNum].AddB.AcqVal.Mask) |
			XAie_SetField(DescrPtr->AddrB.LkAcqValEn,
				TileBd[BdNum].AddB.AcqValEn.Lsb,
				TileBd[BdNum].AddB.AcqValEn.Mask)|
			XAie_SetField(DescrPtr->AddrB.BaseAddr,
				TileBd[BdNum].AddB.Base.Lsb,
				TileBd[BdNum].AddB.Base.Mask);

	BdWord[2U] = XAie_SetField(DescrPtr->X2dCfg.Incr,
				TileBd[BdNum].Xinc.Incr.Lsb,
				TileBd[BdNum].Xinc.Incr.Mask) |
			XAie_SetField(DescrPtr->X2dCfg.Wrap,
				TileBd[BdNum].Xinc.Wrap.Lsb,
				TileBd[BdNum].Xinc.Wrap.Mask) |
			XAie_SetField(DescrPtr->X2dCfg.Offset,
				TileBd[BdNum].Xinc.Off.Lsb,
				TileBd[BdNum].Xinc.Off.Mask);

	BdWord[3U] = XAie_SetField(DescrPtr->Y2dCfg.Incr,
				TileBd[BdNum].Yinc.Incr.Lsb,
				TileBd[BdNum].Yinc.Incr.Mask) |
			XAie_SetField(DescrPtr->Y2dCfg.Wrap,
				TileBd[BdNum].Yinc.Wrap.Lsb,
				TileBd[BdNum].Yinc.Wrap.Mask) |
			XAie_SetField(DescrPtr->Y2dCfg.Offset,
				TileBd[BdNum].Yinc.Off.Lsb,
				TileBd[BdNum].Yinc.Off.Mask);

	BdWord[4U] = XAie_SetField(DescrPtr->PktType,
				TileBd[BdNum].Pkt.Type.Lsb,
				TileBd[BdNum].Pkt.Type.Mask) |
			XAie_SetField(DescrPtr->PktId,
				TileBd[BdNum].Pkt.Id.Lsb,
				TileBd[BdNum].Pkt.Id.Mask);

	BdWord[5U] = XAie_SetField(DescrPtr->IntlvDb,
				TileBd[BdNum].Intlv.Sts.Lsb,
				TileBd[BdNum].Intlv.Sts.Mask) |
			XAie_SetField(DescrPtr->IntlvCur,
				TileBd[BdNum].Intlv.Curr.Lsb,
				TileBd[BdNum].Intlv.Curr.Mask);

	BdWord[6U] = XAie_SetField(XAIEDMA_TILE_BD_VALID,
				TileBd[BdNum].Ctrl.Valid.Lsb,
				TileBd[BdNum].Ctrl.Valid.Mask) |
			XAie_SetField(DescrPtr->AbMode,
				TileBd[BdNum].Ctrl.Ab.Lsb,
				TileBd[BdNum].Ctrl.Ab.Mask) |
			XAie_SetField(DescrPtr->FifoMode,
				TileBd[BdNum].Ctrl.Fifo.Lsb,
				TileBd[BdNum].Ctrl.Fifo.Mask) |
			XAie_SetField(DescrPtr->PktEn,
				TileBd[BdNum].Ctrl.Pkt.Lsb,
				TileBd[BdNum].Ctrl.Pkt.Mask) |
			XAie_SetField(DescrPtr->IntlvMode,
				TileBd[BdNum].Ctrl.Intlv.Lsb,
				TileBd[BdNum].Ctrl.Intlv.Mask) |
			XAie_SetField(DescrPtr->IntlvCnt,
				TileBd[BdNum].Ctrl.Cnt.Lsb,
				TileBd[BdNum].Ctrl.Cnt.Mask) |
			XAie_SetField(DescrPtr->NextBdEn,
				TileBd[BdNum].Ctrl.NexEn.Lsb,
				TileBd[BdNum].Ctrl.NexEn.Mask) |
			XAie_SetField(DescrPtr->NextBd,
				TileBd[BdNum].Ctrl.NexBd.Lsb,
				TileBd[BdNum].Ctrl.NexBd.Mask) |
			XAie_SetField(DescrPtr->Length,
				TileBd[BdNum].Ctrl.Len.Lsb,
				TileBd[BdNum].Ctrl.Len.Mask);

	for(Idx = 0U; Idx < XAIEDMA_TILE_NUM_BD_WORDS; Idx++) {
		BdAddr = DmaInstPtr->BaseAddress + TileBd[BdNum].RegOff[Idx];
		XAieGbl_Write32(BdAddr, BdWord[Idx]);
	}
}

/*****************************************************************************/
/**
*
* This API is used to clear the selected BD entry in the Tile DMA instance.
*
* @param	DmaInstPtr - Pointer to the Tile DMA instance.
* @param	BdNum - BD index which needs to be cleared.
*
* @return	None.
*
* @note		This doesn't clear the values in the BD physical location.
*
*******************************************************************************/
void XAieDma_TileBdClear(XAieDma_Tile *DmaInstPtr, u8 BdNum)
{
	XAieDma_TileBd *DescrPtr;

	XAie_AssertNonvoid(DmaInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(BdNum < XAIEDMA_TILE_MAX_NUM_DESCRS);

	DescrPtr = (XAieDma_TileBd *)&(DmaInstPtr->Descrs[BdNum]);

	DescrPtr->AddrA.LockId = 0U;
	DescrPtr->AddrA.LkRelEn = 0U;
	DescrPtr->AddrA.LkRelVal = 0U;
	DescrPtr->AddrA.LkRelValEn = 0U;
	DescrPtr->AddrA.LkAcqEn = 0U;
	DescrPtr->AddrA.LkAcqVal = 0U;
	DescrPtr->AddrA.LkAcqValEn = 0U;
	DescrPtr->AddrA.BaseAddr = 0U;

	DescrPtr->AddrB.LockId = 0U;
	DescrPtr->AddrB.LkRelEn = 0U;
	DescrPtr->AddrB.LkRelVal = 0U;
	DescrPtr->AddrB.LkRelValEn = 0U;
	DescrPtr->AddrB.LkAcqEn = 0U;
	DescrPtr->AddrB.LkAcqVal = 0U;
	DescrPtr->AddrB.LkAcqValEn = 0U;
	DescrPtr->AddrB.BaseAddr = 0U;

	/* Set the 2D X and Y parameters to their default */
	DescrPtr->X2dCfg.Incr = XAIEDMA_TILE_2DX_DEFAULT_INCR;
	DescrPtr->X2dCfg.Wrap = XAIEDMA_TILE_2DX_DEFAULT_WRAP;
	DescrPtr->X2dCfg.Offset = XAIEDMA_TILE_2DX_DEFAULT_OFFSET;
	DescrPtr->Y2dCfg.Incr = XAIEDMA_TILE_2DY_DEFAULT_INCR;
	DescrPtr->Y2dCfg.Wrap = XAIEDMA_TILE_2DY_DEFAULT_WRAP;
	DescrPtr->Y2dCfg.Offset = XAIEDMA_TILE_2DY_DEFAULT_OFFSET;

	DescrPtr->PktType = 0U;
	DescrPtr->PktId = 0U;

	DescrPtr->IntlvDb = 0U;
	DescrPtr->IntlvCur = 0U;

	DescrPtr->AbMode = 0U;
	DescrPtr->FifoMode = 0U;
	DescrPtr->PktEn = 0U;
	DescrPtr->IntlvMode = 0U;
	DescrPtr->IntlvCnt = 0U;
	DescrPtr->NextBdEn = 0U;
	DescrPtr->NextBd = 0U;
	DescrPtr->Length = 0U;
}

/*****************************************************************************/
/**
*
* This API is used to clear all BD entries in the Tile DMA instance.
*
* @param	DmaInstPtr - Pointer to the Tile DMA instance.
*
* @return	None.
*
* @note		This doesn't clear the values in the BD physical location.
*
*******************************************************************************/
void XAieDma_TileBdClearAll(XAieDma_Tile *DmaInstPtr)
{
	u8 BdIdx;

	XAie_AssertVoid(DmaInstPtr != XAIE_NULL);

	/* Clear the BD entries in the DMA instance structure */
	for (BdIdx = 0U; BdIdx < XAIEDMA_TILE_MAX_NUM_DESCRS; BdIdx++) {
		XAieDma_TileBdClear(DmaInstPtr, BdIdx);
	}
}

/*****************************************************************************/
/**
*
* This API is used to reset/enable the selected Tile DMA channel.
*
* @param	DmaInstPtr - Pointer to the Tile DMA instance.
* @param	ChNum - Channel number (0-S2MM0,1-S2MM1,2-MM2S0,3-MM2S1).
* @param	Reset - Reset/Unreset the channel (1-Reset,0-Unreset).
* @param	Enable - Enable/Disable the channel (1-Enable,0-Disable).
*
* @return	XAIE_SUCCESS if successful, else XAIE_FAILURE.
*
* @note		None.
*
*******************************************************************************/
u32 XAieDma_TileChControl(XAieDma_Tile *DmaInstPtr, u8 ChNum, u8 Reset, u8 Enable)
{
        u64 RegAddr;
        u32 RegVal;

        XAie_AssertNonvoid(DmaInstPtr != XAIE_NULL);
        XAie_AssertNonvoid(ChNum < XAIEDMA_TILE_MAX_NUM_CHANNELS);

        /* Get address of Channel control register */
        RegAddr = DmaInstPtr->BaseAddress + TileDmaCh[ChNum].CtrlOff;
        /* Frame the register value */
        RegVal = XAie_SetField(Reset, TileDmaCh[ChNum].Rst.Lsb,
                				TileDmaCh[ChNum].Rst.Mask) |
                XAie_SetField(Enable, TileDmaCh[ChNum].En.Lsb,
                				TileDmaCh[ChNum].En.Mask);

        /* Write to channel control register */
        XAieGbl_Write32(RegAddr, RegVal);

        return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API is used to reset the selected Tile DMA channel. The channel gets
* reset first, and then gets out of reset as disabled. The StartBd value
* is cleared in the instance, but not cleared in the register.
*
* @param	DmaInstPtr - Pointer to the Tile DMA instance.
* @param	ChNum - Channel number (0-S2MM0,1-S2MM1,2-MM2S0,3-MM2S1).
*
* @return	XAIE_SUCCESS if successful, else XAIE_FAILURE.
*
* @note		None.
*
*******************************************************************************/
u32 XAieDma_TileChReset(XAieDma_Tile *DmaInstPtr, u8 ChNum)
{
	u32 Status;

        XAie_AssertNonvoid(DmaInstPtr != XAIE_NULL);
        XAie_AssertNonvoid(ChNum < XAIEDMA_TILE_MAX_NUM_CHANNELS);

	/* Reset the channel */
	Status = XAieDma_TileChControl(DmaInstPtr, ChNum, XAIE_RESETENABLE,
			XAIE_DISABLE);
	if (Status == XAIE_FAILURE) {
		return Status;
	}

	/* Unreset and Disable the channel */
	Status = XAieDma_TileChControl(DmaInstPtr, ChNum, XAIE_RESETDISABLE,
			XAIE_DISABLE);
	if (Status == XAIE_FAILURE) {
		return Status;
	}

	/* Set Start BD to the reset value */
	XAieDma_TileSetStartBd(DmaInstPtr, ChNum, XAIEDMA_TILE_STARTBD_RESET);

	return Status;
}

/*****************************************************************************/
/**
*
* This API is used to reset all Tile DMA channel. The channel gets reset first,
* and then gets out of reset as disabled. The StartBd value is cleared in
* the instance, but not cleared in the register.
*
* @param	DmaInstPtr - Pointer to the Tile DMA instance.
*
* @return	XAIE_SUCCESS if successful, or XAIE_FAILURE if any fails.
*
* @note		None.
*
*******************************************************************************/
u32 XAieDma_TileChResetAll(XAieDma_Tile *DmaInstPtr)
{
	u8 ChNum;
	u32 Status;
	u32 Ret = XAIE_SUCCESS;;

        XAie_AssertNonvoid(DmaInstPtr != XAIE_NULL);

	/* Reset and disable all the channels */
	for (ChNum = 0U; ChNum < XAIEDMA_TILE_MAX_NUM_CHANNELS; ChNum++) {
		Status = XAieDma_TileChReset(DmaInstPtr, ChNum);
		if (Status == XAIE_FAILURE) {
			Ret = XAIE_FAILURE;
		}
	}

	return Ret;
}

/** @} */

