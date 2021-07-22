/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_dma.h
* @{
*
* Header file for dma functions
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   09/24/2019  Initial creation
* 1.1   Tejus   03/22/2020  Remove initial dma implemenatation
* 1.2   Tejus   03/22/2020  Dma apis for aie
* 1.3   Tejus   04/09/2020  Remove unused argument from interleave enable api
* 1.4   Tejus   06/05/2020  Add api to enable fifo mode.
* </pre>
*
******************************************************************************/
#ifndef XAIEDMA_H
#define XAIEDMA_H
/***************************** Include Files *********************************/
#include "xaiegbl.h"
#include "xaiegbl_defs.h"

/**************************** Type Definitions *******************************/
/*
 * This enum captures the DMA Fifo Counters
 */
typedef enum {
	XAIE_DMA_FIFO_COUNTER_NONE = 0U,
	XAIE_DMA_FIFO_COUNTER_0 = 2U,
	XAIE_DMA_FIFO_COUNTER_1 = 3U,
} XAie_DmaFifoCounter;

/************************** Function Prototypes  *****************************/

/*****************************************************************************/
/**
*
* Macro to declare DMA queue configuration variable.
*
* @param	_Config: XAie DMA Queue configuration variable name
* @param	_StartBD: Start BD of the Queue
* @param	_RepeatCount: Repeat count
* @param	_EnTokenIssue: XAIE_ENABLE to issue token when completes,
*			otherwise disable.
* @param	_OutOfOrder: XAIE_ENABLE to indicate it is out of order mode,
*			otherwise it is not out of order. If out of order mode
*			is enabled, it will ignore the _StartBd setting.
* @return	None.
*
* @note		The macro declares @_Config as an XAie_DmaQueueDesc stack
*		variable.
*
*******************************************************************************/
#define XAie_DmaDeclareQueueConfig(_Config, _StartBD, _RepeatCount, \
		_EnTokenIssue, _OutOfOrder) \
	XAie_DmaQueueDesc _Config; \
	{ \
		_Config.StartBd = (_StartBD); \
		_Config.RepeatCount = (_RepeatCount); \
		_Config.EnTokenIssue = (_EnTokenIssue); \
		_Config.OutOfOrder = (_OutOfOrder); \
	}

AieRC XAie_DmaDescInit(XAie_DevInst *DevInst, XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc);
AieRC XAie_DmaSetLock(XAie_DmaDesc *DmaDesc, XAie_Lock Acq, XAie_Lock Rel);
AieRC XAie_DmaSetPkt(XAie_DmaDesc *DmaDesc, XAie_Packet Pkt);
AieRC XAie_DmaSetOutofOrderBdId(XAie_DmaDesc *DmaDesc, u8 OutofOrderBdId);
AieRC XAie_DmaSetDoubleBuffer(XAie_DmaDesc *DmaDesc, u64 Addr, XAie_Lock Acq,
		XAie_Lock Rel);
AieRC XAie_DmaSetAddrLen(XAie_DmaDesc *DmaDesc, u64 Addr, u32 Len);
AieRC XAie_DmaSetMemInst(XAie_DmaDesc *DmaDesc, XAie_MemInst *MemInst);
AieRC XAie_DmaSetAddrOffsetLen(XAie_DmaDesc *DmaDesc, XAie_MemInst *MemInst,
			       u64 Offset, u32 Len);
AieRC XAie_DmaSetMultiDimAddr(XAie_DmaDesc *DmaDesc, XAie_DmaTensor *Tensor,
		u64 Addr, u32 Len);
AieRC XAie_DmaSetBdIteration(XAie_DmaDesc *DmaDesc, u32 StepSize, u8 Wrap,
		u8 IterCurr);
AieRC XAie_DmaSetPadding(XAie_DmaDesc *DmaDesc, XAie_DmaPadTensor *PadTensor);
AieRC XAie_DmaEnableCompression(XAie_DmaDesc *DmaDesc);
AieRC XAie_DmaConfigFifoMode(XAie_DmaDesc *DmaDesc,
		XAie_DmaFifoCounter Counter);
AieRC XAie_DmaSetNextBd(XAie_DmaDesc *DmaDesc, u8 NextBd, u8 EnableNextBd);
AieRC XAie_DmaEnableBd(XAie_DmaDesc *DmaDesc);
AieRC XAie_DmaDisableBd(XAie_DmaDesc *DmaDesc);
AieRC XAie_DmaSetAxi(XAie_DmaDesc *DmaDesc, u8 Smid, u8 BurstLen, u8 Qos,
		u8 Cache, u8 Secure);
AieRC XAie_DmaSetInterleaveEnable(XAie_DmaDesc *DmaDesc, u8 DoubleBuff,
		u8 IntrleaveCount, u16 IntrleaveCurr);
AieRC XAie_DmaWriteBd(XAie_DevInst *DevInst, XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u8 BdNum);
AieRC XAie_DmaChannelResetAll(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_DmaChReset Reset);
AieRC XAie_DmaChannelReset(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 ChNum, XAie_DmaDirection Dir, XAie_DmaChReset Reset);
AieRC XAie_DmaChannelPauseStream(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 ChNum, XAie_DmaDirection Dir, u8 Pause);
AieRC XAie_DmaChannelPauseMem(XAie_DevInst *DevInst, XAie_LocType Loc, u8 ChNum,
		XAie_DmaDirection Dir, u8 Pause);
AieRC XAie_DmaChannelPushBdToQueue(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 ChNum, XAie_DmaDirection Dir, u8 BdNum);
AieRC XAie_DmaChannelEnable(XAie_DevInst *DevInst, XAie_LocType Loc, u8 ChNum,
		XAie_DmaDirection Dir);
AieRC XAie_DmaChannelDisable(XAie_DevInst *DevInst, XAie_LocType Loc, u8 ChNum,
		XAie_DmaDirection Dir);
AieRC XAie_DmaWaitForDone(XAie_DevInst *DevInst, XAie_LocType Loc, u8 ChNum,
		XAie_DmaDirection Dir, u32 TimeOutUs);
AieRC XAie_DmaGetPendingBdCount(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 ChNum, XAie_DmaDirection Dir, u8 *PendingBd);
AieRC XAie_DmaGetMaxQueueSize(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 *QueueSize);
AieRC XAie_DmaChannelSetStartQueue(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 ChNum, XAie_DmaDirection Dir, u8 BdNum, u32 RepeatCount,
		u8 EnTokenIssue);
AieRC XAie_DmaChannelSetStartQueueGeneric(XAie_DevInst *DevInst,
		XAie_LocType Loc, u8 ChNum, XAie_DmaDirection Dir,
		XAie_DmaQueueDesc *DmaQueueDesc);
AieRC XAie_DmaWriteChannel(XAie_DevInst *DevInst,
		XAie_DmaChannelDesc *DmaChannelDesc, XAie_LocType Loc,
		u8 ChNum, XAie_DmaDirection Dir);
AieRC XAie_DmaChannelSetFoTMode(XAie_DmaChannelDesc *DmaChannelDesc,
		XAie_DmaChannelFoTMode FoTMode);
AieRC XAie_DmaChannelSetControllerId(XAie_DmaChannelDesc *DmaChannelDesc,
		u32 ControllerId);
AieRC XAie_DmaChannelEnOutofOrder(XAie_DmaChannelDesc *DmaChannelDesc,
		u8 EnOutofOrder);
AieRC XAie_DmaChannelEnCompression(XAie_DmaChannelDesc *DmaChannelDesc,
		u8 EnCompression);
AieRC XAie_DmaChannelDescInit(XAie_DevInst *DevInst,
		XAie_DmaChannelDesc *DmaChannelDesc, XAie_LocType Loc);
AieRC XAie_DmaSetZeroPadding(XAie_DmaDesc *DmaDesc, u8 Dim,
		XAie_DmaZeroPaddingPos Pos, u8 NumZeros);
AieRC XAie_DmaTlastEnable(XAie_DmaDesc *DmaDesc);
AieRC XAie_DmaTlastDisable(XAie_DmaDesc *DmaDesc);
AieRC XAie_DmaUpdateBdLen(XAie_DevInst *DevInst, XAie_LocType Loc, u32 Len,
		u8 BdNum);
AieRC XAie_DmaUpdateBdAddr(XAie_DevInst *DevInst, XAie_LocType Loc, u64 Addr,
		u8 BdNum);

#endif		/* end of protection macro */
