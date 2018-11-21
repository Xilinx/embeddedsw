/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*****************************************************************************/
/**
* @file xmedma_shim.c
* @{
*
* This file contains the routines to initialize and configure the Shim DMA.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Naresh  03/14/2018  Initial creation
* 1.1  Naresh  07/11/2018  Updated copyright info
* </pre>
*
******************************************************************************/
#include "xmegbl_defs.h"
#include "xmegbl.h"
#include "xmedma_shim.h"

/***************************** Include Files *********************************/

/***************************** Macro Definitions *****************************/
#define XMEDMA_SHIM_DONE_DEF_WAIT_USECS		50 * 1000 * 1000U
#define XMEDMA_SHIM_DONE_MIN_WAIT_USECS		1U

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API intializes the Shim DMA BDs to their default values and followed by
* disable of all the 4 channels (S2MM0, S2MM1, MM2S0, MM2S1).
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	DmaInstPtr - Pointer to the Shim DMA instance.
*
* @return	None.
*
* @note		This API is required to be invoked first in the sequence,
*		before any of the other Shim DMA driver APIs are used.
*
*******************************************************************************/
void XMeDma_ShimInitialize(XMeGbl_Tile *TileInstPtr, XMeDma_Shim *DmaInstPtr)
{
	u8 BdIdx;
	u8 ChNum;

	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	XMe_AssertNonvoid(DmaInstPtr != XME_NULL);
	XMe_AssertNonvoid(TileInstPtr->TileType == XMEGBL_TILE_TYPE_SHIMNOC);

	DmaInstPtr->BaseAddress = TileInstPtr->TileAddr;

	/* Clear the BD entries in the DMA instance structure */
	for(BdIdx = 0U; BdIdx < XMEDMA_SHIM_MAX_NUM_DESCRS; BdIdx++) {
		XMeDma_ShimBdClear(DmaInstPtr, BdIdx);
	}

	/* Disable all the channels */
	for(ChNum = 0U; ChNum < XMEDMA_SHIM_MAX_NUM_CHANNELS; ChNum++) {
		/* Disable the channel */
		XMeDma_ShimChControl(DmaInstPtr, ChNum, XME_DISABLE,
						XME_DISABLE, XME_DISABLE);

		/* Set Start BD to the reset value */
		XMeDma_ShimSetStartBd(DmaInstPtr, ChNum,
						XMEDMA_SHIM_STARTBD_RESET);
	}
}

/*****************************************************************************/
/**
*
* This API is to configure the Lock release and acquire attributes for the
* selected BD entry in the Shim DMA instance which is later written to the
* the physical BD location.
*
* @param	DmaInstPtr - Pointer to the Shim DMA instance.
* @param	BdNum - BD index whose attributes need to be set to.
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
void XMeDma_ShimBdSetLock(XMeDma_Shim *DmaInstPtr, u8 BdNum, u8 LockId,
		u8 LockRelEn, u8 LockRelVal, u8 LockAcqEn, u8 LockAcqVal)
{
	XMeDma_ShimBd *DescrPtr;

	XMe_AssertNonvoid(DmaInstPtr != XME_NULL);
	XMe_AssertNonvoid(BdNum < XMEDMA_SHIM_MAX_NUM_DESCRS);
	XMe_AssertNonvoid(LockId < XMEDMA_SHIM_MAX_NUM_LOCKS);

	DescrPtr = (XMeDma_ShimBd *)&(DmaInstPtr->Descrs[BdNum]);

	DescrPtr->Lock.LockId = LockId;

	DescrPtr->Lock.LkRelEn = LockRelEn;
	if(LockRelVal != XMEDMA_SHIM_LKACQRELVAL_INVALID) {
		DescrPtr->Lock.LkRelValEn = XME_ENABLE;
		DescrPtr->Lock.LkRelVal = LockRelVal;
	}

	DescrPtr->Lock.LkAcqEn = LockAcqEn;
	if(LockAcqVal != XMEDMA_SHIM_LKACQRELVAL_INVALID) {
		DescrPtr->Lock.LkAcqValEn = XME_ENABLE;
		DescrPtr->Lock.LkAcqVal = LockAcqVal;
	}
}

/*****************************************************************************/
/**
*
* This API is to configure the AXI attributes for the selected BD entry in the
* Shim DMA instance.
*
* @param	DmaInstPtr - Pointer to the Shim DMA instance.
* @param	BdNum - BD index whose attributes need to be set to.
* @param	Smid - SMID value for the AXI-MM transfer.
* @param	BurstLen - Burst length for the AXI-MM transfer (4 or 8 or 16).
* @param	Qos - AXI Qos bits (AxQOS) for the AXI-MM transfer.
* @param	Cache - AxCACHE bits for the AXI-MM transfer.
* @param	Secure - Secure staus of the transfer (1-Secure,0-Non secure).
*
* @return	None.
*
* @note		This doesn't set the values in the BD physical location.
*
*******************************************************************************/
void XMeDma_ShimBdSetAxi(XMeDma_Shim *DmaInstPtr, u8 BdNum, u8 Smid,
				u8 BurstLen, u8 Qos, u8 Cache, u8 Secure)
{
	XMeDma_ShimBd *DescrPtr;

	XMe_AssertNonvoid(DmaInstPtr != XME_NULL);
	XMe_AssertNonvoid(BdNum < XMEDMA_SHIM_MAX_NUM_DESCRS);
	XMe_AssertNonvoid((BurstLen == 4U) || (BurstLen == 8U) ||
						(BurstLen == 16U));

	DescrPtr = (XMeDma_ShimBd *)&(DmaInstPtr->Descrs[BdNum]);

	DescrPtr->Axi.Smid = Smid;
	DescrPtr->Axi.BrstLen = (BurstLen >> XMEDMA_SHIM_BD_AXIBLEN_MASK);
	DescrPtr->Axi.Qos = Qos;
	DescrPtr->Axi.Cache = Cache;
	DescrPtr->Axi.Secure = Secure;
}

/*****************************************************************************/
/**
*
* This API configures the Packet attributes of the selected BD entry in the
* Shim DMA instance.
*
* @param	DmaInstPtr - Pointer to the Shim DMA instance.
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
void XMeDma_ShimBdSetPkt(XMeDma_Shim *DmaInstPtr, u8 BdNum, u8 PktEn,
							u8 PktType, u8 PktId)
{
	XMeDma_ShimBd *DescrPtr;

	XMe_AssertNonvoid(DmaInstPtr != XME_NULL);
	XMe_AssertNonvoid(BdNum < XMEDMA_SHIM_MAX_NUM_DESCRS);

	DescrPtr = (XMeDma_ShimBd *)&(DmaInstPtr->Descrs[BdNum]);

	DescrPtr->PktEn = PktEn;
	DescrPtr->PktType = PktType;
	DescrPtr->PktId = PktId;
}

/*****************************************************************************/
/**
*
* This API configures the Next BD attribute for the selected BD entry in the
* Shim DMA instance.
*
* @param	DmaInstPtr - Pointer to the Shim DMA instance.
* @param	BdNum - BD index whose attributes need to be set to.
* @param	NextBd - Next BD to use, ranging from 0-15. If NextBd==0xFF,
*		it is deemed invalid and UseNextBd is set to 0.
*
* @return	None.
*
* @note		This doesn't set the values in the BD physical location.
*
*******************************************************************************/
void XMeDma_ShimBdSetNext(XMeDma_Shim *DmaInstPtr, u8 BdNum, u8 NextBd)
{
	XMeDma_ShimBd *DescrPtr;

	XMe_AssertNonvoid(DmaInstPtr != XME_NULL);
	XMe_AssertNonvoid(BdNum < XMEDMA_SHIM_MAX_NUM_DESCRS);

	DescrPtr = (XMeDma_ShimBd *)&(DmaInstPtr->Descrs[BdNum]);

	DescrPtr->NextBd = NextBd;

	/* Use next BD only if the Next BD value is not invaliud */
	if(NextBd != XMEDMA_SHIM_BD_NEXTBD_INVALID) {
		DescrPtr->NextBdEn = XME_ENABLE;
	} else {
		DescrPtr->NextBdEn = XME_DISABLE;
	}
}

/*****************************************************************************/
/**
*
* This API configures the address and length attributes for the selected BD
* entry in the Shim DMA instance.
*
* @param	DmaInstPtr - Pointer to the Shim DMA instance.
* @param	BdNum - BD index whose attributes need to be set to.
* @param	AddrHigh - Upper 16-bits base address bits.
* @param	AddrLow - Lower 32-bits base address bits.
* @param	Length - Transfer length in bytes (min length = 4 bytes).
*
* @return	None.
*
* @note		This doesn't set the values in the BD physical location.
*
*******************************************************************************/
void XMeDma_ShimBdSetAddr(XMeDma_Shim *DmaInstPtr, u8 BdNum, u16 AddrHigh,
						u32 AddrLow, u32 Length)
{
	XMeDma_ShimBd *DescrPtr;

	XMe_AssertNonvoid(DmaInstPtr != XME_NULL);
	XMe_AssertNonvoid(BdNum < XMEDMA_SHIM_MAX_NUM_DESCRS);

	/* Address lower to be 128-bit word aligned */
	XMe_AssertNonvoid((AddrLow & XMEDMA_SHIM_ADDRLOW_ALIGN_MASK) == 0U);

	/* Length to be aligned to 32-bit words */
	XMe_AssertNonvoid((Length & XMEDMA_SHIM_TXFER_LEN32_MASK) == 0U);

	DescrPtr = (XMeDma_ShimBd *)&(DmaInstPtr->Descrs[BdNum]);

	DescrPtr->AddrL = AddrLow;
	DescrPtr->AddrH = AddrHigh;

	/* Length in 32-bit words */
	DescrPtr->Length = Length >> XMEDMA_SHIM_TXFER_LEN32_OFFSET;
}

/*****************************************************************************/
/**
*
* This API writes all the attributes of the selected BD entry in the Shim DMA
* instance to the actual physical BD location.
*
* @param	DmaInstPtr - Pointer to the Shim DMA instance.
* @param	BdNum - BD index whose attributes need to be written to the
*		corresponding physical location.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XMeDma_ShimBdWrite(XMeDma_Shim *DmaInstPtr, u8 BdNum)
{
	u64 BdAddr;
	u32 BdWord[XMEDMA_SHIM_NUM_BD_WORDS];
	u32 Idx;
	XMeDma_ShimBd *DescrPtr;

	XMe_AssertNonvoid(DmaInstPtr != XME_NULL);
	XMe_AssertNonvoid(BdNum < XMEDMA_SHIM_MAX_NUM_DESCRS);

	DescrPtr = (XMeDma_ShimBd *)&(DmaInstPtr->Descrs[BdNum]);

	BdWord[0U] = XMe_SetField(DescrPtr->AddrL, ShimBd[BdNum].Addl.Lsb,
                                                ShimBd[BdNum].Addl.Mask);

	BdWord[1U] = XMe_SetField(DescrPtr->Length, ShimBd[BdNum].Len.Lsb,
                                                ShimBd[BdNum].Len.Mask);

	BdWord[2U] = (XMe_SetField(DescrPtr->AddrH, ShimBd[BdNum].Ctrl.Addh.Lsb,
                                        ShimBd[BdNum].Ctrl.Addh.Mask) |
                XMe_SetField(DescrPtr->NextBdEn, ShimBd[BdNum].Ctrl.NexEn.Lsb,
                                        ShimBd[BdNum].Ctrl.NexEn.Mask) |
                XMe_SetField(DescrPtr->NextBd, ShimBd[BdNum].Ctrl.NexBd.Lsb,
                                        ShimBd[BdNum].Ctrl.NexBd.Mask) |
                XMe_SetField(DescrPtr->Lock.LockId, ShimBd[BdNum].Ctrl.Lock.Lsb,
                                        ShimBd[BdNum].Ctrl.Lock.Mask) |
                XMe_SetField(DescrPtr->Lock.LkRelEn,
                                        ShimBd[BdNum].Ctrl.RelEn.Lsb,
                                        ShimBd[BdNum].Ctrl.RelEn.Mask) |
                XMe_SetField(DescrPtr->Lock.LkRelVal,
                                        ShimBd[BdNum].Ctrl.RelVal.Lsb,
                                        ShimBd[BdNum].Ctrl.RelVal.Mask) |
                XMe_SetField(DescrPtr->Lock.LkRelValEn,
                                        ShimBd[BdNum].Ctrl.RelValEn.Lsb,
                                        ShimBd[BdNum].Ctrl.RelValEn.Mask) |
                XMe_SetField(DescrPtr->Lock.LkAcqEn,
                                        ShimBd[BdNum].Ctrl.AcqEn.Lsb,
                                        ShimBd[BdNum].Ctrl.AcqEn.Mask) |
                XMe_SetField(DescrPtr->Lock.LkAcqVal,
                                        ShimBd[BdNum].Ctrl.AcqVal.Lsb,
                                        ShimBd[BdNum].Ctrl.AcqVal.Mask) |
                XMe_SetField(DescrPtr->Lock.LkAcqValEn,
                                        ShimBd[BdNum].Ctrl.AcqValEn.Lsb,
                                        ShimBd[BdNum].Ctrl.AcqValEn.Mask) |
                XMe_SetField(XMEDMA_SHIM_BD_VALID, ShimBd[BdNum].Ctrl.Valid.Lsb,
                                        ShimBd[BdNum].Ctrl.Valid.Mask));

        BdWord[3U] = (XMe_SetField(DescrPtr->Axi.Smid,
                                        ShimBd[BdNum].Axi.Smid.Lsb,
                                        ShimBd[BdNum].Axi.Smid.Mask) |
		XMe_SetField(DescrPtr->Axi.BrstLen, ShimBd[BdNum].Axi.Blen.Lsb,
                                        ShimBd[BdNum].Axi.Blen.Mask) |
                XMe_SetField(DescrPtr->Axi.Qos, ShimBd[BdNum].Axi.Qos.Lsb,
                                         ShimBd[BdNum].Axi.Qos.Mask) |
                XMe_SetField(DescrPtr->Axi.Secure, ShimBd[BdNum].Axi.Sec.Lsb,
                                        ShimBd[BdNum].Axi.Sec.Mask) |
                XMe_SetField(DescrPtr->Axi.Cache, ShimBd[BdNum].Axi.Cache.Lsb,
                                        ShimBd[BdNum].Axi.Cache.Mask));

	BdWord[4U] = (XMe_SetField(DescrPtr->PktEn, ShimBd[BdNum].Pkt.En.Lsb,
                                        ShimBd[BdNum].Pkt.En.Mask) |
                XMe_SetField(DescrPtr->PktType, ShimBd[BdNum].Pkt.Type.Lsb,
                                        ShimBd[BdNum].Pkt.Type.Mask) |
                XMe_SetField(DescrPtr->PktId, ShimBd[BdNum].Pkt.Id.Lsb,
                                        ShimBd[BdNum].Pkt.Id.Mask));

	for(Idx = 0U; Idx < XMEDMA_SHIM_NUM_BD_WORDS; Idx++) {
		BdAddr = DmaInstPtr->BaseAddress + ShimBd[BdNum].RegOff[Idx];
                XMeGbl_Write32(BdAddr, BdWord[Idx]);
	}
}

/*****************************************************************************/
/**
*
* This API is used to clear the selected BD entry in the Shim DMA instance.
*
* @param	DmaInstPtr - Pointer to the Shim DMA instance.
* @param	BdNum - BD index which needs to be cleared.
*
* @return	None.
*
* @note		This doesn't clear the values in the BD physical location.
*
*******************************************************************************/
void XMeDma_ShimBdClear(XMeDma_Shim *DmaInstPtr, u8 BdNum)
{
	XMeDma_ShimBd *DescrPtr;

	XMe_AssertNonvoid(DmaInstPtr != XME_NULL);
	XMe_AssertNonvoid(BdNum < XMEDMA_SHIM_MAX_NUM_DESCRS);

	DescrPtr = (XMeDma_ShimBd *)&(DmaInstPtr->Descrs[BdNum]);

	DescrPtr->Lock.LockId = 0U;
	DescrPtr->Lock.LkRelEn = 0U;
	DescrPtr->Lock.LkRelVal = 0U;
	DescrPtr->Lock.LkRelValEn = 0U;
	DescrPtr->Lock.LkAcqEn = 0U;
	DescrPtr->Lock.LkAcqVal = 0U;
	DescrPtr->Lock.LkAcqValEn = 0U;

	DescrPtr->Axi.Smid = 0U;
	DescrPtr->Axi.BrstLen = 0U;
	DescrPtr->Axi.Qos = 0U;
	DescrPtr->Axi.Cache = 0U;
	DescrPtr->Axi.Secure = 0U;

	DescrPtr->AddrL = 0U;
	DescrPtr->AddrH = 0U;
	DescrPtr->Length = 0U;
	DescrPtr->PktEn = 0U;
	DescrPtr->PktType = 0U;
	DescrPtr->PktId = 0U;
	DescrPtr->NextBdEn = 0U;
	DescrPtr->NextBd = 0U;
}

/*****************************************************************************/
/**
*
* This API is used to wait on Shim DMA channel to be completed.
*
* @param	DmaInstPtr - Pointer to the Shim DMA instance.
* @param	ChNum - Should be one of XMEDMA_SHIM_CHNUM_S2MM0,
* XMEDMA_SHIM_CHNUM_S2MM1, XMEDMA_SHIM_CHNUM_MM2S0, or XMEDMA_SHIM_CHNUM_MM2S1.
* @param	TimeOut - Minimum timeout value in micro seconds.
*
* @return	1 if completed or 0 for timedout.
*
* @note		None.
*
*******************************************************************************/
u8 XMeDma_ShimWaitDone(XMeDma_Shim *DmaInstPtr, u32 ChNum, u32 TimeOut)
{
	u64 RegAddr;
	u32 RegVal;
	u32 LoopCnt;
	u32 Status, Stalled, StartQSize;

	XMe_AssertNonvoid(DmaInstPtr != XME_NULL);

	if (TimeOut == 0U) {
		TimeOut = XMEDMA_SHIM_DONE_DEF_WAIT_USECS;
	}

	LoopCnt = (TimeOut + XMEDMA_SHIM_DONE_MIN_WAIT_USECS - 1U) /
		XMEDMA_SHIM_DONE_MIN_WAIT_USECS;

	RegAddr = DmaInstPtr->BaseAddress + ShimDmaSts[ChNum].RegOff;

	while (LoopCnt > 0U) {
		RegVal = XMeGbl_Read32(RegAddr);

		Status = XMe_GetField(RegVal, ShimDmaSts[ChNum].Sts.Lsb,
				ShimDmaSts[ChNum].Sts.Mask);

		if (Status == XMEGBL_NOC_DMASTA_STA_IDLE) {
			/*
			 * Check the channel is not stalled by lock, and nothing
			 * is in the queue.
			 */
			Stalled = XMe_GetField(RegVal,
					ShimDmaSts[ChNum].Stalled.Lsb,
					ShimDmaSts[ChNum].Stalled.Mask);
			StartQSize = XMe_GetField(RegVal,
					ShimDmaSts[ChNum].StartQSize.Lsb,
					ShimDmaSts[ChNum].StartQSize.Mask);
			if (!Stalled && !StartQSize) {
				break;
			}
			Status = 1;
		}

		XMe_usleep(XMEDMA_SHIM_DONE_MIN_WAIT_USECS);
		LoopCnt--;
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This API is used to get the count of scheduled BDs in pending
*
* @param	DmaInstPtr - Pointer to the Shim DMA instance.
* @param	ChNum - Should be one of XMEDMA_SHIM_CHNUM_S2MM0,
* XMEDMA_SHIM_CHNUM_S2MM1, XMEDMA_SHIM_CHNUM_MM2S0, or XMEDMA_SHIM_CHNUM_MM2S1.
*
* @return	Number of scheduled BDs in pending
*
* @note		This function checks the number of pending BDs in the queue
* as well as if there's any BD that the channel is currently operating on.
* If multiple BDs are chained, it's counted as one BD.
*
*******************************************************************************/
u8 XMeDma_ShimPendingBdCount(XMeDma_Shim *DmaInstPtr, u32 ChNum)
{
	u64 RegAddr;
	u32 RegVal;
	u32 Status, Stalled, StartQSize;

	XMe_AssertNonvoid(DmaInstPtr != XME_NULL);

	RegAddr = DmaInstPtr->BaseAddress + ShimDmaSts[ChNum].RegOff;
	RegVal = XMeGbl_Read32(RegAddr);
	StartQSize = XMe_GetField(RegVal,
			ShimDmaSts[ChNum].StartQSize.Lsb,
			ShimDmaSts[ChNum].StartQSize.Mask);
	XMe_AssertNonvoid(StartQSize <= XMEGBL_NOC_DMASTA_STARTQ_MAX);
	Status = XMe_GetField(RegVal, ShimDmaSts[ChNum].Sts.Lsb,
			ShimDmaSts[ChNum].Sts.Mask);
	Stalled = XMe_GetField(RegVal,
			ShimDmaSts[ChNum].Stalled.Lsb,
			ShimDmaSts[ChNum].Stalled.Mask);

	/* Check if a BD is being used by the channel */
	if (Status != XMEGBL_NOC_DMASTA_STA_IDLE || Stalled) {
		StartQSize++;
	}

	return StartQSize;
}

/** @} */


