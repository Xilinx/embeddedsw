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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/

/*****************************************************************************/
/**
* @file xaiedma_shim.c
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
* 1.2  Nishad  12/05/2018  Renamed ME attributes to AIE
* 1.3  Hyun    01/08/2019  Use the poll function
* 1.4  Hyun    06/20/2019  Added APIs for individual BD / Channel reset
* 1.5  Hyun    06/20/2019  Add XAieDma_ShimSoftInitialize()
* </pre>
*
******************************************************************************/
#include "xaiegbl_defs.h"
#include "xaiegbl.h"
#include "xaiedma_shim.h"

/***************************** Include Files *********************************/

/***************************** Macro Definitions *****************************/
#define XAIEDMA_SHIM_DONE_DEF_WAIT_USECS		1000000U

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API intializes the Shim DMA instances without programming any related
* resources such as bd and channel. This allows the resource to be managed
* independently.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	DmaInstPtr - Pointer to the Shim DMA instance.
*
* @return	XAIE_SUCCESS if successful, else XAIE_FAILURE.
*
* @note		This API is required to be invoked first in the sequence,
*		before any of the other Shim DMA driver APIs are used.
*
*******************************************************************************/
u32 XAieDma_ShimSoftInitialize(XAieGbl_Tile *TileInstPtr, XAieDma_Shim *DmaInstPtr)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(DmaInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_SHIMNOC);

	DmaInstPtr->BaseAddress = TileInstPtr->TileAddr;
	return XAIE_SUCCESS;
}

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
void XAieDma_ShimInitialize(XAieGbl_Tile *TileInstPtr, XAieDma_Shim *DmaInstPtr)
{
	u8 BdIdx;
	u8 ChNum;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(DmaInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_SHIMNOC);

	DmaInstPtr->BaseAddress = TileInstPtr->TileAddr;

	/* Clear the BD entries in the DMA instance structure */
	for(BdIdx = 0U; BdIdx < XAIEDMA_SHIM_MAX_NUM_DESCRS; BdIdx++) {
		XAieDma_ShimBdClear(DmaInstPtr, BdIdx);
	}

	/* Disable all the channels */
	for(ChNum = 0U; ChNum < XAIEDMA_SHIM_MAX_NUM_CHANNELS; ChNum++) {
		/* Disable the channel */
		XAieDma_ShimChControl(DmaInstPtr, ChNum, XAIE_DISABLE,
						XAIE_DISABLE, XAIE_DISABLE);

		/* Set Start BD to the reset value */
		XAieDma_ShimSetStartBd(DmaInstPtr, ChNum,
						XAIEDMA_SHIM_STARTBD_RESET);
	}
}

/*****************************************************************************/
/**
*
* This API resets the selected Shim DMA channel, followed by disabling
* the channel. The StartBd is cleared to default value.
*
* @param	DmaInstPtr - Pointer to the Shim DMA instance.
* @param	ChNum - Channel number (0-S2MM0,1-S2MM1,2-MM2S0,3-MM2S1).
*
* @return	XAIE_SUCCESS if successful, else XAIE_FAILURE.
*
* @note		None.
*
*******************************************************************************/
u32 XAieDma_ShimChReset(XAieDma_Shim *DmaInstPtr, u8 ChNum)
{
	XAie_AssertNonvoid(DmaInstPtr != XAIE_NULL);

	/* Disable the channel */
	XAieDma_ShimChControl(DmaInstPtr, ChNum, XAIE_DISABLE, XAIE_DISABLE,
			XAIE_DISABLE);

	/* Set Start BD to the reset value */
	XAieDma_ShimSetStartBd(DmaInstPtr, ChNum, XAIEDMA_SHIM_STARTBD_RESET);

        return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API resets all Shim DMA channel, followed by disabling the channel.
* The StartBd is cleared to default value.
*
* @param	DmaInstPtr - Pointer to the Shim DMA instance.
*
* @return	XAIE_SUCCESS if successful, else XAIE_FAILURE.
*
* @note		None.
*
*******************************************************************************/
u32 XAieDma_ShimChResetAll(XAieDma_Shim *DmaInstPtr)
{
	u8 ChNum;

	XAie_AssertNonvoid(DmaInstPtr != XAIE_NULL);

	/* Disable and reset all the channels */
	for (ChNum = 0U; ChNum < XAIEDMA_SHIM_MAX_NUM_CHANNELS; ChNum++) {
		XAieDma_ShimChReset(DmaInstPtr, ChNum);
	}

        return XAIE_SUCCESS;
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
void XAieDma_ShimBdSetLock(XAieDma_Shim *DmaInstPtr, u8 BdNum, u8 LockId,
		u8 LockRelEn, u8 LockRelVal, u8 LockAcqEn, u8 LockAcqVal)
{
	XAieDma_ShimBd *DescrPtr;

	XAie_AssertNonvoid(DmaInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(BdNum < XAIEDMA_SHIM_MAX_NUM_DESCRS);
	XAie_AssertNonvoid(LockId < XAIEDMA_SHIM_MAX_NUM_LOCKS);

	DescrPtr = (XAieDma_ShimBd *)&(DmaInstPtr->Descrs[BdNum]);

	DescrPtr->Lock.LockId = LockId;

	DescrPtr->Lock.LkRelEn = LockRelEn;
	if(LockRelVal != XAIEDMA_SHIM_LKACQRELVAL_INVALID) {
		DescrPtr->Lock.LkRelValEn = XAIE_ENABLE;
		DescrPtr->Lock.LkRelVal = LockRelVal;
	}

	DescrPtr->Lock.LkAcqEn = LockAcqEn;
	if(LockAcqVal != XAIEDMA_SHIM_LKACQRELVAL_INVALID) {
		DescrPtr->Lock.LkAcqValEn = XAIE_ENABLE;
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
void XAieDma_ShimBdSetAxi(XAieDma_Shim *DmaInstPtr, u8 BdNum, u8 Smid,
				u8 BurstLen, u8 Qos, u8 Cache, u8 Secure)
{
	XAieDma_ShimBd *DescrPtr;

	XAie_AssertNonvoid(DmaInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(BdNum < XAIEDMA_SHIM_MAX_NUM_DESCRS);
	XAie_AssertNonvoid((BurstLen == 4U) || (BurstLen == 8U) ||
						(BurstLen == 16U));

	DescrPtr = (XAieDma_ShimBd *)&(DmaInstPtr->Descrs[BdNum]);

	DescrPtr->Axi.Smid = Smid;
	DescrPtr->Axi.BrstLen = (BurstLen >> XAIEDMA_SHIM_BD_AXIBLEN_MASK);
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
void XAieDma_ShimBdSetPkt(XAieDma_Shim *DmaInstPtr, u8 BdNum, u8 PktEn,
							u8 PktType, u8 PktId)
{
	XAieDma_ShimBd *DescrPtr;

	XAie_AssertNonvoid(DmaInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(BdNum < XAIEDMA_SHIM_MAX_NUM_DESCRS);

	DescrPtr = (XAieDma_ShimBd *)&(DmaInstPtr->Descrs[BdNum]);

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
void XAieDma_ShimBdSetNext(XAieDma_Shim *DmaInstPtr, u8 BdNum, u8 NextBd)
{
	XAieDma_ShimBd *DescrPtr;

	XAie_AssertNonvoid(DmaInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(BdNum < XAIEDMA_SHIM_MAX_NUM_DESCRS);

	DescrPtr = (XAieDma_ShimBd *)&(DmaInstPtr->Descrs[BdNum]);

	DescrPtr->NextBd = NextBd;

	/* Use next BD only if the Next BD value is not invaliud */
	if(NextBd != XAIEDMA_SHIM_BD_NEXTBD_INVALID) {
		DescrPtr->NextBdEn = XAIE_ENABLE;
	} else {
		DescrPtr->NextBdEn = XAIE_DISABLE;
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
void XAieDma_ShimBdSetAddr(XAieDma_Shim *DmaInstPtr, u8 BdNum, u16 AddrHigh,
						u32 AddrLow, u32 Length)
{
	XAieDma_ShimBd *DescrPtr;

	XAie_AssertNonvoid(DmaInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(BdNum < XAIEDMA_SHIM_MAX_NUM_DESCRS);

	/* Address lower to be 128-bit word aligned */
	XAie_AssertNonvoid((AddrLow & XAIEDMA_SHIM_ADDRLOW_ALIGN_MASK) == 0U);

	/* Length to be aligned to 32-bit words */
	XAie_AssertNonvoid((Length & XAIEDMA_SHIM_TXFER_LEN32_MASK) == 0U);

	DescrPtr = (XAieDma_ShimBd *)&(DmaInstPtr->Descrs[BdNum]);

	DescrPtr->AddrL = AddrLow;
	DescrPtr->AddrH = AddrHigh;

	/* Length in 32-bit words */
	DescrPtr->Length = Length >> XAIEDMA_SHIM_TXFER_LEN32_OFFSET;
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
void XAieDma_ShimBdWrite(XAieDma_Shim *DmaInstPtr, u8 BdNum)
{
	u64 BdAddr;
	u32 BdWord[XAIEDMA_SHIM_NUM_BD_WORDS];
	u32 Idx;
	XAieDma_ShimBd *DescrPtr;

	XAie_AssertNonvoid(DmaInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(BdNum < XAIEDMA_SHIM_MAX_NUM_DESCRS);

	DescrPtr = (XAieDma_ShimBd *)&(DmaInstPtr->Descrs[BdNum]);

	BdWord[0U] = XAie_SetField(DescrPtr->AddrL, ShimBd[BdNum].Addl.Lsb,
                                                ShimBd[BdNum].Addl.Mask);

	BdWord[1U] = XAie_SetField(DescrPtr->Length, ShimBd[BdNum].Len.Lsb,
                                                ShimBd[BdNum].Len.Mask);

	BdWord[2U] = (XAie_SetField(DescrPtr->AddrH, ShimBd[BdNum].Ctrl.Addh.Lsb,
                                        ShimBd[BdNum].Ctrl.Addh.Mask) |
                XAie_SetField(DescrPtr->NextBdEn, ShimBd[BdNum].Ctrl.NexEn.Lsb,
                                        ShimBd[BdNum].Ctrl.NexEn.Mask) |
                XAie_SetField(DescrPtr->NextBd, ShimBd[BdNum].Ctrl.NexBd.Lsb,
                                        ShimBd[BdNum].Ctrl.NexBd.Mask) |
                XAie_SetField(DescrPtr->Lock.LockId, ShimBd[BdNum].Ctrl.Lock.Lsb,
                                        ShimBd[BdNum].Ctrl.Lock.Mask) |
                XAie_SetField(DescrPtr->Lock.LkRelEn,
                                        ShimBd[BdNum].Ctrl.RelEn.Lsb,
                                        ShimBd[BdNum].Ctrl.RelEn.Mask) |
                XAie_SetField(DescrPtr->Lock.LkRelVal,
                                        ShimBd[BdNum].Ctrl.RelVal.Lsb,
                                        ShimBd[BdNum].Ctrl.RelVal.Mask) |
                XAie_SetField(DescrPtr->Lock.LkRelValEn,
                                        ShimBd[BdNum].Ctrl.RelValEn.Lsb,
                                        ShimBd[BdNum].Ctrl.RelValEn.Mask) |
                XAie_SetField(DescrPtr->Lock.LkAcqEn,
                                        ShimBd[BdNum].Ctrl.AcqEn.Lsb,
                                        ShimBd[BdNum].Ctrl.AcqEn.Mask) |
                XAie_SetField(DescrPtr->Lock.LkAcqVal,
                                        ShimBd[BdNum].Ctrl.AcqVal.Lsb,
                                        ShimBd[BdNum].Ctrl.AcqVal.Mask) |
                XAie_SetField(DescrPtr->Lock.LkAcqValEn,
                                        ShimBd[BdNum].Ctrl.AcqValEn.Lsb,
                                        ShimBd[BdNum].Ctrl.AcqValEn.Mask) |
                XAie_SetField(XAIEDMA_SHIM_BD_VALID, ShimBd[BdNum].Ctrl.Valid.Lsb,
                                        ShimBd[BdNum].Ctrl.Valid.Mask));

        BdWord[3U] = (XAie_SetField(DescrPtr->Axi.Smid,
                                        ShimBd[BdNum].Axi.Smid.Lsb,
                                        ShimBd[BdNum].Axi.Smid.Mask) |
		XAie_SetField(DescrPtr->Axi.BrstLen, ShimBd[BdNum].Axi.Blen.Lsb,
                                        ShimBd[BdNum].Axi.Blen.Mask) |
                XAie_SetField(DescrPtr->Axi.Qos, ShimBd[BdNum].Axi.Qos.Lsb,
                                         ShimBd[BdNum].Axi.Qos.Mask) |
                XAie_SetField(DescrPtr->Axi.Secure, ShimBd[BdNum].Axi.Sec.Lsb,
                                        ShimBd[BdNum].Axi.Sec.Mask) |
                XAie_SetField(DescrPtr->Axi.Cache, ShimBd[BdNum].Axi.Cache.Lsb,
                                        ShimBd[BdNum].Axi.Cache.Mask));

	BdWord[4U] = (XAie_SetField(DescrPtr->PktEn, ShimBd[BdNum].Pkt.En.Lsb,
                                        ShimBd[BdNum].Pkt.En.Mask) |
                XAie_SetField(DescrPtr->PktType, ShimBd[BdNum].Pkt.Type.Lsb,
                                        ShimBd[BdNum].Pkt.Type.Mask) |
                XAie_SetField(DescrPtr->PktId, ShimBd[BdNum].Pkt.Id.Lsb,
                                        ShimBd[BdNum].Pkt.Id.Mask));

	for(Idx = 0U; Idx < XAIEDMA_SHIM_NUM_BD_WORDS; Idx++) {
		BdAddr = DmaInstPtr->BaseAddress + ShimBd[BdNum].RegOff[Idx];
                XAieGbl_Write32(BdAddr, BdWord[Idx]);
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
void XAieDma_ShimBdClear(XAieDma_Shim *DmaInstPtr, u8 BdNum)
{
	XAieDma_ShimBd *DescrPtr;

	XAie_AssertNonvoid(DmaInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(BdNum < XAIEDMA_SHIM_MAX_NUM_DESCRS);

	DescrPtr = (XAieDma_ShimBd *)&(DmaInstPtr->Descrs[BdNum]);

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
* This API is used to clear all BD entries in the Shim DMA instance.
*
* @param	DmaInstPtr - Pointer to the Shim DMA instance.
*
* @return	None.
*
* @note		This doesn't clear the values in the BD physical location.
*
*******************************************************************************/
void XAieDma_ShimBdClearAll(XAieDma_Shim *DmaInstPtr)
{
	u8 BdIdx;

	XAie_AssertVoid(DmaInstPtr != XAIE_NULL);

	/* Clear the BD entries in the DMA instance structure */
	for (BdIdx = 0U; BdIdx < XAIEDMA_SHIM_MAX_NUM_DESCRS; BdIdx++) {
		XAieDma_ShimBdClear(DmaInstPtr, BdIdx);
	}
}

/*****************************************************************************/
/**
*
* This API is used to wait on Shim DMA channel to be completed.
*
* @param	DmaInstPtr - Pointer to the Shim DMA instance.
* @param	ChNum - Should be one of XAIEDMA_SHIM_CHNUM_S2MM0,
* XAIEDMA_SHIM_CHNUM_S2MM1, XAIEDMA_SHIM_CHNUM_MM2S0, or XAIEDMA_SHIM_CHNUM_MM2S1.
* @param	TimeOut - Minimum timeout value in micro seconds.
*
* @return	0 if completed or 1 for timedout.
*
* @note		None.
*
*******************************************************************************/
u8 XAieDma_ShimWaitDone(XAieDma_Shim *DmaInstPtr, u32 ChNum, u32 TimeOut)
{
	u64 RegAddr;
	u32 Mask, Value;
	u32 Ret = 1;

	XAie_AssertNonvoid(DmaInstPtr != XAIE_NULL);

	RegAddr = DmaInstPtr->BaseAddress + ShimDmaSts[ChNum].RegOff;
	Mask = ShimDmaSts[ChNum].Sts.Mask | ShimDmaSts[ChNum].Stalled.Mask |
		ShimDmaSts[ChNum].StartQSize.Mask;
	/* This will check the stalled and start queue size bits to be zero */
	Value = XAIEGBL_NOC_DMASTA_STA_IDLE << ShimDmaSts[ChNum].Sts.Lsb;

	if (TimeOut == 0U) {
		TimeOut = XAIEDMA_SHIM_DONE_DEF_WAIT_USECS;
	}

	if (XAieGbl_MaskPoll(RegAddr, Mask, Value, TimeOut) == XAIE_SUCCESS) {
		Ret = 0;
	}

	return Ret;
}

/*****************************************************************************/
/**
*
* This API is used to get the count of scheduled BDs in pending
*
* @param	DmaInstPtr - Pointer to the Shim DMA instance.
* @param	ChNum - Should be one of XAIEDMA_SHIM_CHNUM_S2MM0,
* XAIEDMA_SHIM_CHNUM_S2MM1, XAIEDMA_SHIM_CHNUM_MM2S0, or XAIEDMA_SHIM_CHNUM_MM2S1.
*
* @return	Number of scheduled BDs in pending
*
* @note		This function checks the number of pending BDs in the queue
* as well as if there's any BD that the channel is currently operating on.
* If multiple BDs are chained, it's counted as one BD.
*
*******************************************************************************/
u8 XAieDma_ShimPendingBdCount(XAieDma_Shim *DmaInstPtr, u32 ChNum)
{
	u64 RegAddr;
	u32 RegVal;
	u32 Status, Stalled, StartQSize;

	XAie_AssertNonvoid(DmaInstPtr != XAIE_NULL);

	RegAddr = DmaInstPtr->BaseAddress + ShimDmaSts[ChNum].RegOff;
	RegVal = XAieGbl_Read32(RegAddr);
	StartQSize = XAie_GetField(RegVal,
			ShimDmaSts[ChNum].StartQSize.Lsb,
			ShimDmaSts[ChNum].StartQSize.Mask);
	XAie_AssertNonvoid(StartQSize <= XAIEGBL_NOC_DMASTA_STARTQ_MAX);
	Status = XAie_GetField(RegVal, ShimDmaSts[ChNum].Sts.Lsb,
			ShimDmaSts[ChNum].Sts.Mask);
	Stalled = XAie_GetField(RegVal,
			ShimDmaSts[ChNum].Stalled.Lsb,
			ShimDmaSts[ChNum].Stalled.Mask);

	/* Check if a BD is being used by the channel */
	if (Status != XAIEGBL_NOC_DMASTA_STA_IDLE || Stalled) {
		StartQSize++;
	}

	return StartQSize;
}

/** @} */


