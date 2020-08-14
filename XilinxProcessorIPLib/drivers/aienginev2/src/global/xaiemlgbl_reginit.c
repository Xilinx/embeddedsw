/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie2gbl_reginit.c
* @{
*
* This file contains the instances of the register bit field definitions for the
* Core, Memory, NoC and PL module registers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   07/30/2019  Initial creation
* 1.1   Tejus   10/21/2019  Optimize stream switch data structures
* 1.2   Tejus   10/28/2019  Add pl interface register properties
* 1.3   Tejus   12/09/2019  Include correct header file to avoid cyclic
*			    dependancy
* 1.4   Tejus   03/16/2020  Seperate PlIf Module for SHIMPL and SHIMNOC Tiles
* 1.5   Tejus   03/16/2020  Add register properties for Mux/Demux registers
* 1.6   Tejus   03/17/2020  Add lock module properties
* 1.7   Tejus   03/21/2020  Add structure fields to stream switch module
*			    definition
* 1.8   Tejus   03/21/2020  Add register properties for stream switch slot
*			    registers
* 1.9   Tejus   03/23/2020  Organize header files in alphabetical order
* 2.0   Tejus   03/23/2020  Re-organize dma data structures
* 2.1   Dishita 03/24/2020  Add performance counter properties
* 2.2   Dishita 04/16/2020  Fix compiler warnings
* 2.3   Dishita 04/20/2020  Add timer properties
* 2.4   Tejus   05/26/2020  Restructure and optimize core module.
* 2.5   Tejus   06/01/2020  Add data structure for core debug register.
* 2.6   Nishad  06/02/2020  Rename included header file xaie2gbl_events to
*			    xaie_events_aie2
* 2.7   Nishad  06/03/2020  Rename XAIEGBL_<MODULE>_EVENT_* macros to
*			    XAIE_EVENTS_<MODULE>_*
* 2.8   Nishad  06/09/2020  Fix typo in *_MEMORY_* event macros
* 2.9   Tejus   06/05/2020  Populate fifo mode availability in data structure.
* 3.0   Nishad  06/16/2020  Add trace module properties
* 3.1   Nishad  06/25/2020  Fix typo in Aie2MemTileTraceMod structure
* 3.2   Nishad  06/28/2020  Populate stream switch port event selection, event
*			    generation and combo event properties
* 3.3   Nishad  07/01/2020  Populate MstrConfigBaseAddr stream switch property
* 3.4   Nishad  07/12/2020  Populate event broadcast, PC event, and group event
*			    register properties
* 3.5   Nishad  07/21/2020  Populate interrupt controller data structure.
* 3.4   Nishad  07/24/2020  Populate value of default group error mask.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xaie_dma_aieml.h"
#include "xaie_events.h"
#include "xaie_events_aieml.h"
#include "xaie_locks_aieml.h"
#include "xaie_reset_aieml.h"
#include "xaiegbl_regdef.h"
#include "xaiemlgbl_params.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/**************************** Macro Definitions ******************************/

/************************** Variable Definitions *****************************/
/*
 * Global instance for Core module Core_Control register.
 */
static const  XAie_RegCoreCtrl Aie2CoreCtrlReg =
{
	XAIE2GBL_CORE_MODULE_CORE_CONTROL,
	{XAIE2GBL_CORE_MODULE_CORE_CONTROL_ENABLE_LSB, XAIE2GBL_CORE_MODULE_CORE_CONTROL_ENABLE_MASK},
	{XAIE2GBL_CORE_MODULE_CORE_CONTROL_RESET_LSB, XAIE2GBL_CORE_MODULE_CORE_CONTROL_RESET_MASK}
};

/*
 * Global instance for Core module Core_Status register.
 */
static const  XAie_RegCoreSts Aie2CoreStsReg =
{
	XAIE2GBL_CORE_MODULE_CORE_STATUS,
	{XAIE2GBL_CORE_MODULE_CORE_STATUS_CORE_DONE_LSB, XAIE2GBL_CORE_MODULE_CORE_STATUS_CORE_DONE_MASK},
	{XAIE2GBL_CORE_MODULE_CORE_STATUS_RESET_LSB, XAIE2GBL_CORE_MODULE_CORE_STATUS_RESET_MASK},
	{XAIE2GBL_CORE_MODULE_CORE_STATUS_ENABLE_LSB, XAIE2GBL_CORE_MODULE_CORE_STATUS_ENABLE_MASK}
};

/*
 * Global instance for Core module for core debug registers.
 */
static const XAie_RegCoreDebug Aie2CoreDebugReg =
{
	.RegOff = XAIE2GBL_CORE_MODULE_DEBUG_CONTROL0,
	.DebugHalt.Lsb = XAIE2GBL_CORE_MODULE_DEBUG_CONTROL0_DEBUG_HALT_BIT_LSB,
	.DebugHalt.Mask = XAIE2GBL_CORE_MODULE_DEBUG_CONTROL0_DEBUG_HALT_BIT_MASK
};

static const  XAie_DmaBdEnProp Aie2MemTileDmaBdEnProp =
{
	.NxtBd.Idx = 1U,
	.NxtBd.Lsb = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_1_NEXT_BD_LSB,
	.NxtBd.Mask = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_1_NEXT_BD_MASK,
	.UseNxtBd.Idx = 1U,
	.UseNxtBd.Lsb = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_1_USE_NEXT_BD_LSB,
	.UseNxtBd.Mask = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_1_USE_NEXT_BD_MASK,
	.ValidBd.Idx = 7U,
	.ValidBd.Lsb = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_7_VALID_BD_LSB,
	.ValidBd.Mask = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_7_VALID_BD_MASK,
	.OutofOrderBdId.Idx = 0U,
	.OutofOrderBdId.Lsb = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_0_OUT_OF_ORDER_BD_ID_LSB,
	.OutofOrderBdId.Mask = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_0_OUT_OF_ORDER_BD_ID_MASK,
};

static const  XAie_DmaBdPkt Aie2MemTileDmaBdPktProp =
{
	.EnPkt.Idx = 0U,
	.EnPkt.Lsb = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_0_ENABLE_PACKET_LSB,
	.EnPkt.Mask = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_0_ENABLE_PACKET_MASK,
	.PktId.Idx = 0U,
	.PktId.Lsb = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_0_PACKET_ID_LSB,
	.PktId.Mask = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_0_PACKET_ID_MASK,
	.PktType.Idx = 0U,
	.PktType.Lsb = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_0_PACKET_TYPE_LSB,
	.PktType.Mask = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_0_PACKET_TYPE_MASK,
};

static const  XAie_DmaBdLock Aie2MemTileDmaLockProp =
{
	.Aie2DmaLock.LckRelVal.Idx = 7U,
	.Aie2DmaLock.LckRelVal.Lsb = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_REL_VALUE_LSB,
	.Aie2DmaLock.LckRelVal.Mask = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_REL_VALUE_MASK,
	.Aie2DmaLock.LckRelId.Idx = 7U,
	.Aie2DmaLock.LckRelId.Lsb = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_REL_ID_LSB,
	.Aie2DmaLock.LckRelId.Mask = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_REL_ID_MASK,
	.Aie2DmaLock.LckAcqEn.Idx = 7U,
	.Aie2DmaLock.LckAcqEn.Lsb = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_ACQ_ENABLE_LSB,
	.Aie2DmaLock.LckAcqEn.Mask = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_ACQ_ENABLE_MASK,
	.Aie2DmaLock.LckAcqVal.Idx = 7U,
	.Aie2DmaLock.LckAcqVal.Lsb = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_ACQ_VALUE_LSB,
	.Aie2DmaLock.LckAcqVal.Mask = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_ACQ_VALUE_MASK,
	.Aie2DmaLock.LckAcqId.Idx = 7U,
	.Aie2DmaLock.LckAcqId.Lsb = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_ACQ_ID_LSB,
	.Aie2DmaLock.LckAcqId.Mask = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_ACQ_ID_MASK,
};

static const  XAie_DmaBdBuffer Aie2MemTileBufferProp =
{
	.TileDmaBuff.BaseAddr.Idx = 1U,
	.TileDmaBuff.BaseAddr.Lsb = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_1_BASE_ADDRESS_LSB,
	.TileDmaBuff.BaseAddr.Mask = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_1_BASE_ADDRESS_MASK,
	.TileDmaBuff.BufferLen.Idx = 0U,
	.TileDmaBuff.BufferLen.Lsb = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_0_BUFFER_LENGTH_LSB,
	.TileDmaBuff.BufferLen.Mask = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_0_BUFFER_LENGTH_MASK,
};

static const XAie_DmaBdDoubleBuffer Aie2MemTileDoubleBufferProp =
{
	.EnDoubleBuff = {0U},
	.BaseAddr_B = {0U},
	.FifoMode = {0U},
	.EnIntrleaved = {0U},
	.IntrleaveCnt = {0U},
	.BuffSelect = {0U},
};

static const  XAie_DmaBdMultiDimAddr Aie2MemTileMultiDimProp =
{
	.Aie2MultiDimAddr.DmaDimProp[1U].StepSize.Idx = 3U,
	.Aie2MultiDimAddr.DmaDimProp[1U].StepSize.Lsb = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_3_D1_STEPSIZE_LSB,
	.Aie2MultiDimAddr.DmaDimProp[1U].StepSize.Mask = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_3_D1_STEPSIZE_MASK,
	.Aie2MultiDimAddr.DmaDimProp[0U].StepSize.Idx = 2U,
	.Aie2MultiDimAddr.DmaDimProp[0U].StepSize.Lsb = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_2_D0_STEPSIZE_LSB,
	.Aie2MultiDimAddr.DmaDimProp[0U].StepSize.Mask = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_2_D0_STEPSIZE_MASK,
	.Aie2MultiDimAddr.DmaDimProp[1U].Wrap.Idx = 3U,
	.Aie2MultiDimAddr.DmaDimProp[1U].Wrap.Lsb = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_3_D1_WRAP_LSB,
	.Aie2MultiDimAddr.DmaDimProp[1U].Wrap.Mask = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_3_D1_WRAP_MASK,
	.Aie2MultiDimAddr.DmaDimProp[0U].Wrap.Idx = 2U,
	.Aie2MultiDimAddr.DmaDimProp[0U].Wrap.Lsb = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_2_D0_WRAP_LSB,
	.Aie2MultiDimAddr.DmaDimProp[0U].Wrap.Mask = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_2_D0_WRAP_MASK,
	.Aie2MultiDimAddr.DmaDimProp[2U].StepSize.Idx = 4U,
	.Aie2MultiDimAddr.DmaDimProp[2U].StepSize.Lsb = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_4_D2_STEPSIZE_LSB,
	.Aie2MultiDimAddr.DmaDimProp[2U].StepSize.Mask = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_4_D2_STEPSIZE_MASK,
	.Aie2MultiDimAddr.DmaDimProp[2U].Wrap.Idx = 4U,
	.Aie2MultiDimAddr.DmaDimProp[2U].Wrap.Lsb = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_4_D2_WRAP_LSB,
	.Aie2MultiDimAddr.DmaDimProp[2U].Wrap.Mask = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_4_D2_WRAP_MASK,
	.Aie2MultiDimAddr.IterCurr.Idx = 6U,
	.Aie2MultiDimAddr.IterCurr.Lsb = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_6_ITERATION_CURRENT_LSB,
	.Aie2MultiDimAddr.IterCurr.Mask = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_6_ITERATION_CURRENT_MASK,
	.Aie2MultiDimAddr.Iter.Wrap.Idx = 6U,
	.Aie2MultiDimAddr.Iter.Wrap.Lsb = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_6_ITERATION_WRAP_LSB,
	.Aie2MultiDimAddr.Iter.Wrap.Mask = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_6_ITERATION_WRAP_MASK,
	.Aie2MultiDimAddr.Iter.StepSize.Idx = 6U,
	.Aie2MultiDimAddr.Iter.StepSize.Lsb = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_6_ITERATION_STEPSIZE_LSB,
	.Aie2MultiDimAddr.Iter.StepSize.Mask = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_6_ITERATION_STEPSIZE_MASK,
	.Aie2MultiDimAddr.DmaDimProp[3U].StepSize.Idx = 5U,
	.Aie2MultiDimAddr.DmaDimProp[3U].StepSize.Lsb = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_5_D3_STEPSIZE_LSB,
	.Aie2MultiDimAddr.DmaDimProp[3U].StepSize.Mask = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_5_D3_STEPSIZE_MASK,
};

static const  XAie_DmaBdZeroPad Aie2MemTileZeroPadProp =
{
	.D0_ZeroBefore.Idx = 1U,
	.D0_ZeroBefore.Lsb = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_1_D0_ZERO_BEFORE_LSB,
	.D0_ZeroBefore.Mask = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_1_D0_ZERO_BEFORE_MASK,
	.D1_ZeroBefore.Idx = 3U,
	.D1_ZeroBefore.Lsb = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_3_D1_ZERO_BEFORE_LSB,
	.D1_ZeroBefore.Mask = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_3_D1_ZERO_BEFORE_MASK,
	.D2_ZeroBefore.Idx = 4U,
	.D2_ZeroBefore.Lsb = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_4_D2_ZERO_BEFORE_LSB,
	.D2_ZeroBefore.Mask = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_4_D2_ZERO_BEFORE_MASK,
	.D0_ZeroAfter.Idx = 5U,
	.D0_ZeroAfter.Lsb = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_5_D0_ZERO_AFTER_LSB,
	.D0_ZeroAfter.Mask = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_5_D0_ZERO_AFTER_MASK,
	.D1_ZeroAfter.Idx = 5U,
	.D1_ZeroAfter.Lsb = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_5_D1_ZERO_AFTER_LSB,
	.D1_ZeroAfter.Mask = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_5_D1_ZERO_AFTER_MASK,
	.D2_ZeroAfter.Idx = 5U,
	.D2_ZeroAfter.Lsb = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_5_D2_ZERO_AFTER_LSB,
	.D2_ZeroAfter.Mask = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_5_D2_ZERO_AFTER_MASK,
};

static const  XAie_DmaBdCompression Aie2MemTileCompressionProp =
{
	.EnCompression.Idx = 4U,
	.EnCompression.Lsb = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_4_ENABLE_COMPRESSION_LSB,
	.EnCompression.Mask = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_4_ENABLE_COMPRESSION_MASK,
};

/* Data structure to capture register offsets and masks for Mem Tile Dma */
static const  XAie_DmaBdProp Aie2MemTileDmaProp =
{
	.AddrAlignMask = 0x3,
	.AddrAlignShift = 0x2,
	.AddrMask = (1U << 20U) - 1U,
	.LenActualOffset = 0U,
	.Buffer = &Aie2MemTileBufferProp,
	.DoubleBuffer = &Aie2MemTileDoubleBufferProp,
	.Lock = &Aie2MemTileDmaLockProp,
	.Pkt = &Aie2MemTileDmaBdPktProp,
	.BdEn = &Aie2MemTileDmaBdEnProp,
	.AddrMode = &Aie2MemTileMultiDimProp,
	.ZeroPad = &Aie2MemTileZeroPadProp,
	.Compression = &Aie2MemTileCompressionProp,
	.SysProp = NULL
};

static const  XAie_DmaChProp Aie2MemTileDmaChProp =
{
	.CtrlId.Idx = 0,
	.CtrlId.Lsb = XAIE2GBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_CONTROLLER_ID_LSB,
	.CtrlId.Mask =XAIE2GBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_CONTROLLER_ID_MASK ,
	.EnCompression.Idx = 0,
	.EnCompression.Lsb = XAIE2GBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_DECOMPRESSION_ENABLE_LSB,
	.EnCompression.Mask = XAIE2GBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_DECOMPRESSION_ENABLE_MASK,
	.EnOutofOrder.Idx = 0,
	.EnOutofOrder.Lsb = XAIE2GBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_ENABLE_OUT_OF_ORDER_LSB,
	.EnOutofOrder.Mask = XAIE2GBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_ENABLE_OUT_OF_ORDER_MASK,
	.Reset.Idx = 0,
	.Reset.Lsb = XAIE2GBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_RESET_LSB,
	.Reset.Mask = XAIE2GBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_RESET_MASK,
	.EnToken.Idx = 1,
	.EnToken.Lsb = XAIE2GBL_MEM_TILE_MODULE_DMA_S2MM_0_START_QUEUE_ENABLE_TOKEN_ISSUE_LSB,
	.EnToken.Mask = XAIE2GBL_MEM_TILE_MODULE_DMA_S2MM_0_START_QUEUE_ENABLE_TOKEN_ISSUE_MASK,
	.RptCount.Idx = 1,
	.RptCount.Lsb = XAIE2GBL_MEM_TILE_MODULE_DMA_S2MM_0_START_QUEUE_REPEAT_COUNT_LSB,
	.RptCount.Mask = XAIE2GBL_MEM_TILE_MODULE_DMA_S2MM_0_START_QUEUE_REPEAT_COUNT_MASK,
	.StartBd.Idx = 1,
	.StartBd.Lsb = XAIE2GBL_MEM_TILE_MODULE_DMA_S2MM_0_START_QUEUE_START_BD_ID_LSB,
	.StartBd.Mask = XAIE2GBL_MEM_TILE_MODULE_DMA_S2MM_0_START_QUEUE_START_BD_ID_MASK,
	.PauseStream = {0U},
	.PauseMem = {0U},
	.Enable = {0U},
};

/* Mem Tile Dma Module */
static const  XAie_DmaMod Aie2MemTileDmaMod =
{
	.BaseAddr = XAIE2GBL_MEM_TILE_MODULE_DMA_BD0_0,
	.IdxOffset = 0x20,  /* This is the offset between each BD */
	.NumBds = 48,	   /* Number of BDs for AIE2 Tile DMA */
	.NumLocks = 64,
	.NumAddrDim = 4U,
	.DoubleBuffering = XAIE_FEATURE_UNAVAILABLE,
	.Compression = XAIE_FEATURE_UNAVAILABLE,
	.ZeroPadding = XAIE_FEATURE_UNAVAILABLE,
	.OutofOrderBdId = XAIE_FEATURE_AVAILABLE,
	.InterleaveMode = XAIE_FEATURE_UNAVAILABLE,
	.FifoMode = XAIE_FEATURE_UNAVAILABLE,
	.ChCtrlBase = XAIE2GBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL,
	.NumChannels = 6,  /* number of s2mm/mm2s channels */
	.ChIdxOffset = 0x8,  /* This is the offset between each channel */
	.BdProp = &Aie2MemTileDmaProp,
	.ChProp = &Aie2MemTileDmaChProp,
	.DmaBdInit = &_XAieMl_MemTileDmaInit,
	.SetLock = &_XAieMl_DmaSetLock,
	.SetIntrleave = NULL,
	.SetMultiDim = &_XAieMl_DmaSetMultiDim,
	.WriteBd = &_XAieMl_MemTileDmaWriteBd,
	.PendingBd = &_XAieMl_DmaGetPendingBdCount,
	.WaitforDone = &_XAieMl_DmaWaitForDone,
};

static const  XAie_DmaBdEnProp Aie2TileDmaBdEnProp =
{
	.NxtBd.Idx = 5U,
	.NxtBd.Lsb = XAIE2GBL_MEMORY_MODULE_DMA_BD0_5_NEXT_BD_LSB,
	.NxtBd.Mask = XAIE2GBL_MEMORY_MODULE_DMA_BD0_5_NEXT_BD_MASK,
	.UseNxtBd.Idx = 5U,
	.UseNxtBd.Lsb = XAIE2GBL_MEMORY_MODULE_DMA_BD0_5_USE_NEXT_BD_LSB,
	.UseNxtBd.Mask = XAIE2GBL_MEMORY_MODULE_DMA_BD0_5_USE_NEXT_BD_MASK,
	.ValidBd.Idx = 5U,
	.ValidBd.Lsb = XAIE2GBL_MEMORY_MODULE_DMA_BD0_5_VALID_BD_LSB,
	.ValidBd.Mask = XAIE2GBL_MEMORY_MODULE_DMA_BD0_5_VALID_BD_MASK,
	.OutofOrderBdId.Idx = 1U,
	.OutofOrderBdId.Lsb = XAIE2GBL_MEMORY_MODULE_DMA_BD0_1_OUT_OF_ORDER_BD_ID_LSB,
	.OutofOrderBdId.Mask = XAIE2GBL_MEMORY_MODULE_DMA_BD0_1_OUT_OF_ORDER_BD_ID_MASK,
};

static const  XAie_DmaBdPkt Aie2TileDmaBdPktProp =
{
	.EnPkt.Idx = 1U,
	.EnPkt.Lsb = XAIE2GBL_MEMORY_MODULE_DMA_BD0_1_ENABLE_PACKET_LSB,
	.EnPkt.Mask = XAIE2GBL_MEMORY_MODULE_DMA_BD0_1_ENABLE_PACKET_MASK,
	.PktId.Idx = 1U,
	.PktId.Lsb = XAIE2GBL_MEMORY_MODULE_DMA_BD0_1_PACKET_ID_LSB,
	.PktId.Mask = XAIE2GBL_MEMORY_MODULE_DMA_BD0_1_PACKET_ID_MASK,
	.PktType.Idx = 1U,
	.PktType.Lsb = XAIE2GBL_MEMORY_MODULE_DMA_BD0_1_PACKET_TYPE_LSB,
	.PktType.Mask = XAIE2GBL_MEMORY_MODULE_DMA_BD0_1_PACKET_TYPE_MASK,
};

static const  XAie_DmaBdLock Aie2TileDmaLockProp =
{
	.Aie2DmaLock.LckRelVal.Idx = 5U,
	.Aie2DmaLock.LckRelVal.Lsb = XAIE2GBL_MEMORY_MODULE_DMA_BD0_5_LOCK_REL_VALUE_LSB,
	.Aie2DmaLock.LckRelVal.Mask = XAIE2GBL_MEMORY_MODULE_DMA_BD0_5_LOCK_REL_VALUE_MASK,
	.Aie2DmaLock.LckRelId.Idx = 5U,
	.Aie2DmaLock.LckRelId.Lsb = XAIE2GBL_MEMORY_MODULE_DMA_BD0_5_LOCK_REL_ID_LSB,
	.Aie2DmaLock.LckRelId.Mask = XAIE2GBL_MEMORY_MODULE_DMA_BD0_5_LOCK_REL_ID_MASK,
	.Aie2DmaLock.LckAcqEn.Idx = 5U,
	.Aie2DmaLock.LckAcqEn.Lsb = XAIE2GBL_MEMORY_MODULE_DMA_BD0_5_LOCK_ACQ_ENABLE_LSB,
	.Aie2DmaLock.LckAcqEn.Mask = XAIE2GBL_MEMORY_MODULE_DMA_BD0_5_LOCK_ACQ_ENABLE_MASK,
	.Aie2DmaLock.LckAcqVal.Idx = 5U,
	.Aie2DmaLock.LckAcqVal.Lsb = XAIE2GBL_MEMORY_MODULE_DMA_BD0_5_LOCK_ACQ_VALUE_LSB,
	.Aie2DmaLock.LckAcqVal.Mask = XAIE2GBL_MEMORY_MODULE_DMA_BD0_5_LOCK_ACQ_VALUE_MASK,
	.Aie2DmaLock.LckAcqId.Idx = 5U,
	.Aie2DmaLock.LckAcqId.Lsb = XAIE2GBL_MEMORY_MODULE_DMA_BD0_5_LOCK_ACQ_ID_LSB,
	.Aie2DmaLock.LckAcqId.Mask = XAIE2GBL_MEMORY_MODULE_DMA_BD0_5_LOCK_ACQ_ID_MASK,
};

static const  XAie_DmaBdBuffer Aie2TileDmaBufferProp =
{
	.TileDmaBuff.BaseAddr.Idx = 0U,
	.TileDmaBuff.BaseAddr.Lsb = XAIE2GBL_MEMORY_MODULE_DMA_BD0_0_BASE_ADDRESS_LSB,
	.TileDmaBuff.BaseAddr.Mask = XAIE2GBL_MEMORY_MODULE_DMA_BD0_0_BASE_ADDRESS_MASK,
	.TileDmaBuff.BufferLen.Idx = 0U,
	.TileDmaBuff.BufferLen.Lsb = XAIE2GBL_MEMORY_MODULE_DMA_BD0_0_BUFFER_LENGTH_LSB,
	.TileDmaBuff.BufferLen.Mask = XAIE2GBL_MEMORY_MODULE_DMA_BD0_0_BUFFER_LENGTH_MASK,
};

static const XAie_DmaBdDoubleBuffer Aie2TileDmaDoubleBufferProp =
{
	.EnDoubleBuff = {0U},
	.BaseAddr_B = {0U},
	.FifoMode = {0U},
	.EnIntrleaved = {0U},
	.IntrleaveCnt = {0U},
	.BuffSelect = {0U},
};

static const  XAie_DmaBdMultiDimAddr Aie2TileDmaMultiDimProp =
{
	.Aie2MultiDimAddr.DmaDimProp[0U].StepSize.Idx = 2U,
	.Aie2MultiDimAddr.DmaDimProp[0U].StepSize.Lsb = XAIE2GBL_MEMORY_MODULE_DMA_BD0_2_D0_STEPSIZE_LSB,
	.Aie2MultiDimAddr.DmaDimProp[0U].StepSize.Mask = XAIE2GBL_MEMORY_MODULE_DMA_BD0_2_D0_STEPSIZE_MASK,
	.Aie2MultiDimAddr.DmaDimProp[0U].Wrap.Idx = 3U,
	.Aie2MultiDimAddr.DmaDimProp[0U].Wrap.Lsb = XAIE2GBL_MEMORY_MODULE_DMA_BD0_3_D0_WRAP_LSB,
	.Aie2MultiDimAddr.DmaDimProp[0U].Wrap.Mask = XAIE2GBL_MEMORY_MODULE_DMA_BD0_3_D0_WRAP_MASK,
	.Aie2MultiDimAddr.DmaDimProp[1U].StepSize.Idx = 2U,
	.Aie2MultiDimAddr.DmaDimProp[1U].StepSize.Lsb = XAIE2GBL_MEMORY_MODULE_DMA_BD0_2_D1_STEPSIZE_LSB,
	.Aie2MultiDimAddr.DmaDimProp[1U].StepSize.Mask = XAIE2GBL_MEMORY_MODULE_DMA_BD0_2_D1_STEPSIZE_MASK,
	.Aie2MultiDimAddr.DmaDimProp[1U].Wrap.Idx = 3U,
	.Aie2MultiDimAddr.DmaDimProp[1U].Wrap.Lsb = XAIE2GBL_MEMORY_MODULE_DMA_BD0_3_D1_WRAP_LSB,
	.Aie2MultiDimAddr.DmaDimProp[1U].Wrap.Mask = XAIE2GBL_MEMORY_MODULE_DMA_BD0_3_D1_WRAP_MASK,
	.Aie2MultiDimAddr.DmaDimProp[2U].StepSize.Idx = 3U,
	.Aie2MultiDimAddr.DmaDimProp[2U].StepSize.Lsb = XAIE2GBL_MEMORY_MODULE_DMA_BD0_3_D2_STEPSIZE_LSB,
	.Aie2MultiDimAddr.DmaDimProp[2U].StepSize.Mask = XAIE2GBL_MEMORY_MODULE_DMA_BD0_3_D2_STEPSIZE_MASK,
	.Aie2MultiDimAddr.IterCurr.Idx = 4U,
	.Aie2MultiDimAddr.IterCurr.Lsb = XAIE2GBL_MEMORY_MODULE_DMA_BD0_4_ITERATION_CURRENT_LSB,
	.Aie2MultiDimAddr.IterCurr.Mask = XAIE2GBL_MEMORY_MODULE_DMA_BD0_4_ITERATION_CURRENT_MASK,
	.Aie2MultiDimAddr.Iter.Wrap.Idx = 4U,
	.Aie2MultiDimAddr.Iter.Wrap.Lsb = XAIE2GBL_MEMORY_MODULE_DMA_BD0_4_ITERATION_WRAP_LSB,
	.Aie2MultiDimAddr.Iter.Wrap.Mask = XAIE2GBL_MEMORY_MODULE_DMA_BD0_4_ITERATION_WRAP_MASK,
	.Aie2MultiDimAddr.Iter.StepSize.Idx = 4U,
	.Aie2MultiDimAddr.Iter.StepSize.Lsb = XAIE2GBL_MEMORY_MODULE_DMA_BD0_4_ITERATION_STEPSIZE_LSB,
	.Aie2MultiDimAddr.Iter.StepSize.Mask = XAIE2GBL_MEMORY_MODULE_DMA_BD0_4_ITERATION_STEPSIZE_MASK,
	.Aie2MultiDimAddr.DmaDimProp[2U].Wrap = {0U},
	.Aie2MultiDimAddr.DmaDimProp[3U].Wrap = {0U},
	.Aie2MultiDimAddr.DmaDimProp[3U].StepSize = {0U}
};

static const  XAie_DmaBdCompression Aie2TileDmaCompressionProp =
{
	.EnCompression.Idx = 1U,
	.EnCompression.Lsb = XAIE2GBL_MEMORY_MODULE_DMA_BD0_1_ENABLE_COMPRESSION_LSB,
	.EnCompression.Mask = XAIE2GBL_MEMORY_MODULE_DMA_BD0_1_ENABLE_COMPRESSION_MASK,
};

/* Data structure to capture register offsets and masks for Tile Dma */
static const  XAie_DmaBdProp Aie2TileDmaProp =
{
	.AddrAlignMask = 0x3,
	.AddrAlignShift = 0x2,
	.AddrMask = (1U << 17U) - 1U,
	.LenActualOffset = 0U,
	.Buffer = &Aie2TileDmaBufferProp,
	.DoubleBuffer = &Aie2TileDmaDoubleBufferProp,
	.Lock = &Aie2TileDmaLockProp,
	.Pkt = &Aie2TileDmaBdPktProp,
	.BdEn = &Aie2TileDmaBdEnProp,
	.AddrMode = &Aie2TileDmaMultiDimProp,
	.ZeroPad = NULL,
	.Compression = &Aie2TileDmaCompressionProp,
	.SysProp = NULL
};

/* Data structure to capture register offsets and masks for Mem Tile and
 * Tile Dma Channels
 */
static const  XAie_DmaChProp Aie2DmaChProp =
{
	.CtrlId.Idx = 0,
	.CtrlId.Lsb = XAIE2GBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_CONTROLLER_ID_LSB,
	.CtrlId.Mask =XAIE2GBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_CONTROLLER_ID_MASK ,
	.EnCompression.Idx = 0,
	.EnCompression.Lsb = XAIE2GBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_DECOMPRESSION_ENABLE_LSB,
	.EnCompression.Mask = XAIE2GBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_DECOMPRESSION_ENABLE_MASK,
	.EnOutofOrder.Idx = 0,
	.EnOutofOrder.Lsb = XAIE2GBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_ENABLE_OUT_OF_ORDER_LSB,
	.EnOutofOrder.Mask = XAIE2GBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_ENABLE_OUT_OF_ORDER_MASK,
	.Reset.Idx = 0,
	.Reset.Lsb = XAIE2GBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_RESET_LSB,
	.Reset.Mask = XAIE2GBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_RESET_MASK,
	.EnToken.Idx = 1,
	.EnToken.Lsb = XAIE2GBL_MEMORY_MODULE_DMA_S2MM_0_START_QUEUE_ENABLE_TOKEN_ISSUE_LSB,
	.EnToken.Mask = XAIE2GBL_MEMORY_MODULE_DMA_S2MM_0_START_QUEUE_ENABLE_TOKEN_ISSUE_MASK,
	.RptCount.Idx = 1,
	.RptCount.Lsb = XAIE2GBL_MEMORY_MODULE_DMA_S2MM_0_START_QUEUE_REPEAT_COUNT_LSB,
	.RptCount.Mask = XAIE2GBL_MEMORY_MODULE_DMA_S2MM_0_START_QUEUE_REPEAT_COUNT_MASK,
	.StartBd.Idx = 1,
	.StartBd.Lsb = XAIE2GBL_MEMORY_MODULE_DMA_S2MM_0_START_QUEUE_START_BD_ID_LSB,
	.StartBd.Mask = XAIE2GBL_MEMORY_MODULE_DMA_S2MM_0_START_QUEUE_START_BD_ID_MASK,
	.PauseStream = {0U},
	.PauseMem = {0U},
	.Enable = {0U},
};

/* Tile Dma Module */
static const  XAie_DmaMod Aie2TileDmaMod =
{
	.BaseAddr = XAIE2GBL_MEMORY_MODULE_DMA_BD0_0,
	.IdxOffset = 0x20,  	/* This is the offset between each BD */
	.NumBds = 16U,	   	/* Number of BDs for AIE2 Tile DMA */
	.NumLocks = 16U,
	.NumAddrDim = 3U,
	.DoubleBuffering = XAIE_FEATURE_UNAVAILABLE,
	.Compression = XAIE_FEATURE_AVAILABLE,
	.ZeroPadding = XAIE_FEATURE_UNAVAILABLE,
	.OutofOrderBdId = XAIE_FEATURE_AVAILABLE,
	.InterleaveMode = XAIE_FEATURE_UNAVAILABLE,
	.FifoMode = XAIE_FEATURE_UNAVAILABLE,
	.ChCtrlBase = XAIE2GBL_MEMORY_MODULE_DMA_S2MM_0_CTRL,
	.NumChannels = 2U,  /* Number of s2mm/mm2s channels */
	.ChIdxOffset = 0x8,  /* This is the offset between each channel */
	.BdProp = &Aie2TileDmaProp,
	.ChProp = &Aie2DmaChProp,
	.DmaBdInit = &_XAieMl_TileDmaInit,
	.SetLock = &_XAieMl_DmaSetLock,
	.SetIntrleave = NULL,
	.SetMultiDim = &_XAieMl_DmaSetMultiDim,
	.WriteBd = &_XAieMl_TileDmaWriteBd,
	.PendingBd = &_XAieMl_DmaGetPendingBdCount,
	.WaitforDone = &_XAieMl_DmaWaitForDone,
};

static const  XAie_DmaBdEnProp Aie2ShimDmaBdEnProp =
{
	.NxtBd.Idx = 7U,
	.NxtBd.Lsb = XAIE2GBL_NOC_MODULE_DMA_BD0_7_NEXT_BD_LSB,
	.NxtBd.Mask = XAIE2GBL_NOC_MODULE_DMA_BD0_7_NEXT_BD_MASK,
	.UseNxtBd.Idx = 7U,
	.UseNxtBd.Lsb = XAIE2GBL_NOC_MODULE_DMA_BD0_7_USE_NEXT_BD_LSB,
	.UseNxtBd.Mask = XAIE2GBL_NOC_MODULE_DMA_BD0_7_USE_NEXT_BD_MASK,
	.ValidBd.Idx = 7U,
	.ValidBd.Lsb = XAIE2GBL_NOC_MODULE_DMA_BD0_7_VALID_BD_LSB,
	.ValidBd.Mask = XAIE2GBL_NOC_MODULE_DMA_BD0_7_VALID_BD_MASK,
	.OutofOrderBdId.Idx = 2U,
	.OutofOrderBdId.Lsb = XAIE2GBL_NOC_MODULE_DMA_BD0_2_OUT_OF_ORDER_BD_ID_LSB,
	.OutofOrderBdId.Mask = XAIE2GBL_NOC_MODULE_DMA_BD0_2_OUT_OF_ORDER_BD_ID_MASK,
};

static const  XAie_DmaBdPkt Aie2ShimDmaBdPktProp =
{
	.EnPkt.Idx = 2U,
	.EnPkt.Lsb = XAIE2GBL_NOC_MODULE_DMA_BD0_2_ENABLE_PACKET_LSB,
	.EnPkt.Mask = XAIE2GBL_NOC_MODULE_DMA_BD0_2_ENABLE_PACKET_MASK,
	.PktId.Idx = 2U,
	.PktId.Lsb = XAIE2GBL_NOC_MODULE_DMA_BD0_2_PACKET_ID_LSB,
	.PktId.Mask = XAIE2GBL_NOC_MODULE_DMA_BD0_2_PACKET_ID_MASK,
	.PktType.Idx = 2U,
	.PktType.Lsb = XAIE2GBL_NOC_MODULE_DMA_BD0_2_PACKET_TYPE_LSB,
	.PktType.Mask = XAIE2GBL_NOC_MODULE_DMA_BD0_2_PACKET_TYPE_MASK,
};

static const  XAie_DmaBdLock Aie2ShimDmaLockProp =
{
	.Aie2DmaLock.LckRelVal.Idx = 7U,
	.Aie2DmaLock.LckRelVal.Lsb = XAIE2GBL_NOC_MODULE_DMA_BD0_7_LOCK_REL_VALUE_LSB,
	.Aie2DmaLock.LckRelVal.Mask = XAIE2GBL_NOC_MODULE_DMA_BD0_7_LOCK_REL_VALUE_MASK,
	.Aie2DmaLock.LckRelId.Idx = 7U,
	.Aie2DmaLock.LckRelId.Lsb = XAIE2GBL_NOC_MODULE_DMA_BD0_7_LOCK_REL_ID_LSB,
	.Aie2DmaLock.LckRelId.Mask = XAIE2GBL_NOC_MODULE_DMA_BD0_7_LOCK_REL_ID_MASK,
	.Aie2DmaLock.LckAcqEn.Idx = 7U,
	.Aie2DmaLock.LckAcqEn.Lsb = XAIE2GBL_NOC_MODULE_DMA_BD0_7_LOCK_ACQ_ENABLE_LSB,
	.Aie2DmaLock.LckAcqEn.Mask = XAIE2GBL_NOC_MODULE_DMA_BD0_7_LOCK_ACQ_ENABLE_MASK,
	.Aie2DmaLock.LckAcqVal.Idx = 7U,
	.Aie2DmaLock.LckAcqVal.Lsb = XAIE2GBL_NOC_MODULE_DMA_BD0_7_LOCK_ACQ_VALUE_LSB,
	.Aie2DmaLock.LckAcqVal.Mask = XAIE2GBL_NOC_MODULE_DMA_BD0_7_LOCK_ACQ_VALUE_MASK,
	.Aie2DmaLock.LckAcqId.Idx = 7U,
	.Aie2DmaLock.LckAcqId.Lsb = XAIE2GBL_NOC_MODULE_DMA_BD0_7_LOCK_ACQ_ID_LSB,
	.Aie2DmaLock.LckAcqId.Mask = XAIE2GBL_NOC_MODULE_DMA_BD0_7_LOCK_ACQ_ID_MASK,
};

static const  XAie_DmaBdBuffer Aie2ShimDmaBufferProp =
{
	.ShimDmaBuff.AddrLow.Idx = 1U,
	.ShimDmaBuff.AddrLow.Lsb = XAIE2GBL_NOC_MODULE_DMA_BD0_1_BASE_ADDRESS_LOW_LSB,
	.ShimDmaBuff.AddrLow.Mask = XAIE2GBL_NOC_MODULE_DMA_BD0_1_BASE_ADDRESS_LOW_MASK,
	.ShimDmaBuff.AddrHigh.Idx = 2U,
	.ShimDmaBuff.AddrHigh.Lsb = XAIE2GBL_NOC_MODULE_DMA_BD0_2_BASE_ADDRESS_HIGH_LSB,
	.ShimDmaBuff.AddrHigh.Mask = XAIE2GBL_NOC_MODULE_DMA_BD0_2_BASE_ADDRESS_HIGH_MASK,
	.ShimDmaBuff.BufferLen.Idx = 0U,
	.ShimDmaBuff.BufferLen.Lsb = XAIE2GBL_NOC_MODULE_DMA_BD0_0_BUFFER_LENGTH_LSB,
	.ShimDmaBuff.BufferLen.Mask = XAIE2GBL_NOC_MODULE_DMA_BD0_0_BUFFER_LENGTH_MASK,
};

static const  XAie_DmaBdDoubleBuffer Aie2ShimDmaDoubleBufferProp =
{
	.EnDoubleBuff = {0U},
	.BaseAddr_B = {0U},
	.FifoMode = {0U},
	.EnIntrleaved = {0U},
	.IntrleaveCnt = {0U},
	.BuffSelect = {0U},
};

static const  XAie_DmaBdMultiDimAddr Aie2ShimDmaMultiDimProp =
{
	.Aie2MultiDimAddr.DmaDimProp[0U].StepSize.Idx = 3U,
	.Aie2MultiDimAddr.DmaDimProp[0U].StepSize.Lsb = XAIE2GBL_NOC_MODULE_DMA_BD0_3_D0_STEPSIZE_LSB,
	.Aie2MultiDimAddr.DmaDimProp[0U].StepSize.Mask = XAIE2GBL_NOC_MODULE_DMA_BD0_3_D0_STEPSIZE_MASK,
	.Aie2MultiDimAddr.DmaDimProp[0U].Wrap.Idx = 3U,
	.Aie2MultiDimAddr.DmaDimProp[0U].Wrap.Lsb = XAIE2GBL_NOC_MODULE_DMA_BD0_3_D0_WRAP_LSB,
	.Aie2MultiDimAddr.DmaDimProp[0U].Wrap.Mask = XAIE2GBL_NOC_MODULE_DMA_BD0_3_D0_WRAP_MASK,
	.Aie2MultiDimAddr.DmaDimProp[1U].StepSize.Idx =3U ,
	.Aie2MultiDimAddr.DmaDimProp[1U].StepSize.Lsb = XAIE2GBL_NOC_MODULE_DMA_BD0_4_D1_STEPSIZE_LSB,
	.Aie2MultiDimAddr.DmaDimProp[1U].StepSize.Mask = XAIE2GBL_NOC_MODULE_DMA_BD0_4_D1_STEPSIZE_MASK,
	.Aie2MultiDimAddr.DmaDimProp[1U].Wrap.Idx = 3U,
	.Aie2MultiDimAddr.DmaDimProp[1U].Wrap.Lsb = XAIE2GBL_NOC_MODULE_DMA_BD0_4_D1_WRAP_LSB,
	.Aie2MultiDimAddr.DmaDimProp[1U].Wrap.Mask = XAIE2GBL_NOC_MODULE_DMA_BD0_4_D1_WRAP_MASK,
	.Aie2MultiDimAddr.DmaDimProp[2U].StepSize.Idx = 5U,
	.Aie2MultiDimAddr.DmaDimProp[2U].StepSize.Lsb = XAIE2GBL_NOC_MODULE_DMA_BD0_5_D2_STEPSIZE_LSB,
	.Aie2MultiDimAddr.DmaDimProp[2U].StepSize.Mask = XAIE2GBL_NOC_MODULE_DMA_BD0_5_D2_STEPSIZE_MASK,
	.Aie2MultiDimAddr.IterCurr.Idx = 6U,
	.Aie2MultiDimAddr.IterCurr.Lsb = XAIE2GBL_NOC_MODULE_DMA_BD0_6_ITERATION_CURRENT_LSB,
	.Aie2MultiDimAddr.IterCurr.Mask = XAIE2GBL_NOC_MODULE_DMA_BD0_6_ITERATION_CURRENT_MASK,
	.Aie2MultiDimAddr.Iter.Wrap.Idx = 6U,
	.Aie2MultiDimAddr.Iter.Wrap.Lsb = XAIE2GBL_NOC_MODULE_DMA_BD0_6_ITERATION_WRAP_LSB,
	.Aie2MultiDimAddr.Iter.Wrap.Mask = XAIE2GBL_NOC_MODULE_DMA_BD0_6_ITERATION_WRAP_MASK,
	.Aie2MultiDimAddr.Iter.StepSize.Idx = 6U,
	.Aie2MultiDimAddr.Iter.StepSize.Lsb = XAIE2GBL_NOC_MODULE_DMA_BD0_6_ITERATION_STEPSIZE_LSB,
	.Aie2MultiDimAddr.Iter.StepSize.Mask = XAIE2GBL_NOC_MODULE_DMA_BD0_6_ITERATION_STEPSIZE_MASK,
	.Aie2MultiDimAddr.DmaDimProp[2U].Wrap = {0U},
	.Aie2MultiDimAddr.DmaDimProp[3U].Wrap = {0U},
	.Aie2MultiDimAddr.DmaDimProp[3U].StepSize = {0U}
};

static const  XAie_DmaSysProp Aie2ShimDmaSysProp =
{
	.SMID.Idx = 5U,
	.SMID.Lsb = XAIE2GBL_NOC_MODULE_DMA_BD0_5_SMID_LSB,
	.SMID.Mask = XAIE2GBL_NOC_MODULE_DMA_BD0_5_SMID_MASK,
	.BurstLen.Idx = 4U,
	.BurstLen.Lsb = XAIE2GBL_NOC_MODULE_DMA_BD0_4_BURST_LENGTH_LSB,
	.BurstLen.Mask = XAIE2GBL_NOC_MODULE_DMA_BD0_4_BURST_LENGTH_MASK,
	.AxQos.Idx = 5U,
	.AxQos.Lsb = XAIE2GBL_NOC_MODULE_DMA_BD0_5_AXQOS_LSB,
	.AxQos.Mask = XAIE2GBL_NOC_MODULE_DMA_BD0_5_AXQOS_MASK,
	.SecureAccess.Idx = 3U,
	.SecureAccess.Lsb = XAIE2GBL_NOC_MODULE_DMA_BD0_3_SECURE_ACCESS_LSB,
	.SecureAccess.Mask = XAIE2GBL_NOC_MODULE_DMA_BD0_3_SECURE_ACCESS_MASK,
	.AxCache.Idx = 5U,
	.AxCache.Lsb = XAIE2GBL_NOC_MODULE_DMA_BD0_5_AXCACHE_LSB,
	.AxCache.Mask = XAIE2GBL_NOC_MODULE_DMA_BD0_5_AXCACHE_MASK,
};

/* Data structure to capture register offsets and masks for Tile Dma */
static const  XAie_DmaBdProp Aie2ShimDmaProp =
{
	.AddrAlignMask = 0xF,
	.AddrAlignShift = 0x0,
	.AddrMask = (1UL << 48U) - 1U,
	.LenActualOffset = 0U,
	.Buffer = &Aie2ShimDmaBufferProp,
	.DoubleBuffer = &Aie2ShimDmaDoubleBufferProp,
	.Lock = &Aie2ShimDmaLockProp,
	.Pkt = &Aie2ShimDmaBdPktProp,
	.BdEn = &Aie2ShimDmaBdEnProp,
	.AddrMode = &Aie2ShimDmaMultiDimProp,
	.ZeroPad = NULL,
	.Compression = NULL,
	.SysProp = &Aie2ShimDmaSysProp
};

/* Data structure to capture register offsets and masks for Mem Tile and
 * Tile Dma Channels
 */
static const  XAie_DmaChProp Aie2ShimDmaChProp =
{
	.CtrlId.Idx = 0U,
	.CtrlId.Lsb = XAIE2GBL_NOC_MODULE_DMA_S2MM_0_CTRL_CONTROLLER_ID_LSB ,
	.CtrlId.Mask = XAIE2GBL_NOC_MODULE_DMA_S2MM_0_CTRL_CONTROLLER_ID_MASK ,
	.EnCompression.Idx = 0U,
	.EnCompression.Lsb = 0U,
	.EnCompression.Mask = 0U,
	.EnOutofOrder.Idx = 0U,
	.EnOutofOrder.Lsb = XAIE2GBL_NOC_MODULE_DMA_S2MM_0_CTRL_ENABLE_OUT_OF_ORDER_LSB,
	.EnOutofOrder.Mask = XAIE2GBL_NOC_MODULE_DMA_S2MM_0_CTRL_ENABLE_OUT_OF_ORDER_MASK,
	.Reset.Idx = 0U,
	.Reset.Lsb = 0U,
	.Reset.Mask = 0U,
	.EnToken.Idx = 1U,
	.EnToken.Lsb = XAIE2GBL_NOC_MODULE_DMA_S2MM_0_TASK_QUEUE_ENABLE_TOKEN_ISSUE_LSB,
	.EnToken.Mask = XAIE2GBL_NOC_MODULE_DMA_S2MM_0_TASK_QUEUE_ENABLE_TOKEN_ISSUE_MASK,
	.RptCount.Idx = 1U,
	.RptCount.Lsb = XAIE2GBL_NOC_MODULE_DMA_S2MM_0_TASK_QUEUE_REPEAT_COUNT_LSB,
	.RptCount.Mask = XAIE2GBL_NOC_MODULE_DMA_S2MM_0_TASK_QUEUE_REPEAT_COUNT_MASK,
	.StartBd.Idx = 1U,
	.StartBd.Lsb = XAIE2GBL_NOC_MODULE_DMA_S2MM_0_TASK_QUEUE_START_BD_ID_LSB,
	.StartBd.Mask = XAIE2GBL_NOC_MODULE_DMA_S2MM_0_TASK_QUEUE_START_BD_ID_MASK,
	.PauseStream = {0U},
	.PauseMem = {0U},
	.Enable = {0U},
};

/* Tile Dma Module */
static const  XAie_DmaMod Aie2ShimDmaMod =
{
	.BaseAddr = XAIE2GBL_NOC_MODULE_DMA_BD0_0,
	.IdxOffset = 0x20,  	/* This is the offset between each BD */
	.NumBds = 16U,	   	/* Number of BDs for AIE2 Tile DMA */
	.NumLocks = 16U,
	.NumAddrDim = 3U,
	.DoubleBuffering = XAIE_FEATURE_UNAVAILABLE,
	.Compression = XAIE_FEATURE_UNAVAILABLE,
	.ZeroPadding = XAIE_FEATURE_UNAVAILABLE,
	.OutofOrderBdId = XAIE_FEATURE_AVAILABLE,
	.InterleaveMode = XAIE_FEATURE_UNAVAILABLE,
	.FifoMode = XAIE_FEATURE_UNAVAILABLE,
	.ChCtrlBase = XAIE2GBL_NOC_MODULE_DMA_S2MM_0_CTRL,
	.NumChannels = 2U,  /* Number of s2mm/mm2s channels */
	.ChIdxOffset = 0x8,  /* This is the offset between each channel */
	.BdProp = &Aie2ShimDmaProp,
	.ChProp = &Aie2ShimDmaChProp,
	.DmaBdInit = &_XAieMl_ShimDmaInit,
	.SetLock = &_XAieMl_DmaSetLock,
	.SetIntrleave = NULL,
	.SetMultiDim = &_XAieMl_DmaSetMultiDim,
	.WriteBd = &_XAieMl_ShimDmaWriteBd,
	.PendingBd = &_XAieMl_DmaGetPendingBdCount,
	.WaitforDone = &_XAieMl_DmaWaitForDone,
};

/*
 * Array of all Tile Stream Switch Master Config registers
 * The data structure contains number of ports and the register offsets
 */
static const  XAie_StrmPort Aie2TileStrmMstr[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2GBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0,
	},
	{	/* DMA */
		.NumPorts = 2,
		.PortBaseAddr = XAIE2GBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_DMA0,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2GBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_TILE_CTRL,
	},
	{	/* Fifo */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2GBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_FIFO0,
	},
	{	/* South */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2GBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_SOUTH0,
	},
	{	/* West */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2GBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_WEST0,
	},
	{	/* North */
		.NumPorts = 6,
		.PortBaseAddr = XAIE2GBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_NORTH0,
	},
	{	/* East */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2GBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_EAST0,
	},
	{	/* Trace */
		.NumPorts = 0,
		.PortBaseAddr = 0
	}
};

/*
 * Array of all Tile Stream Switch Slave Config registers
 * The data structure contains number of ports and the register offsets
 */
static const  XAie_StrmPort Aie2TileStrmSlv[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0,
	},
	{	/* DMA */
		.NumPorts = 2,
		.PortBaseAddr = XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_DMA_0,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_TILE_CTRL,
	},
	{	/* Fifo */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_FIFO_0,
	},
	{	/* South */
		.NumPorts = 6,
		.PortBaseAddr = XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_SOUTH_0,
	},
	{	/* West */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_WEST_0,
	},
	{	/* North */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_NORTH_0,
	},
	{	/* East */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_EAST_0,
	},
	{	/* Trace */
		.NumPorts = 2,
		.PortBaseAddr = XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_MEM_TRACE
	}
};

/*
 * Array of all Shim NOC/PL Stream Switch Master Config registers
 * The data structure contains number of ports and the register offsets
 */
static const  XAie_StrmPort Aie2ShimStrmMstr[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* DMA */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2GBL_PL_MODULE_STREAM_SWITCH_MASTER_CONFIG_TILE_CTRL,
	},
	{	/* Fifo */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2GBL_PL_MODULE_STREAM_SWITCH_MASTER_CONFIG_FIFO0,
	},
	{	/* South */
		.NumPorts = 6,
		.PortBaseAddr = XAIE2GBL_PL_MODULE_STREAM_SWITCH_MASTER_CONFIG_SOUTH0,
	},
	{	/* West */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2GBL_PL_MODULE_STREAM_SWITCH_MASTER_CONFIG_WEST0,
	},
	{	/* North */
		.NumPorts = 6,
		.PortBaseAddr = XAIE2GBL_PL_MODULE_STREAM_SWITCH_MASTER_CONFIG_NORTH0,
	},
	{	/* East */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2GBL_PL_MODULE_STREAM_SWITCH_MASTER_CONFIG_EAST0,
	},
	{	/* Trace */
		.NumPorts = 0,
		.PortBaseAddr = 0
	}
};

/*
 * Array of all Shim NOC/PL Stream Switch Slave Config registers
 * The data structure contains number of ports and the register offsets
 */
static const  XAie_StrmPort Aie2ShimStrmSlv[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* DMA */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2GBL_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_TILE_CTRL,
	},
	{	/* Fifo */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2GBL_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_FIFO_0,
	},
	{	/* South */
		.NumPorts = 8,
		.PortBaseAddr = XAIE2GBL_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_SOUTH_0,
	},
	{	/* West */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2GBL_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_WEST_0,
	},
	{	/* North */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2GBL_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_NORTH_0,
	},
	{	/* East */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2GBL_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_EAST_0,
	},
	{	/* Trace */
		.NumPorts = 2,
		.PortBaseAddr = XAIE2GBL_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_TRACE
	}
};

/*
 * Array of all Mem Tile Stream Switch Master Config registers
 * The data structure contains number of ports and the register offsets
 */
static const  XAie_StrmPort Aie2MemTileStrmMstr[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* DMA */
		.NumPorts = 6,
		.PortBaseAddr = XAIE2GBL_MEM_TILE_MODULE_STREAM_SWITCH_MASTER_CONFIG_DMA0,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2GBL_MEM_TILE_MODULE_STREAM_SWITCH_MASTER_CONFIG_TILE_CTRL,
	},
	{	/* Fifo */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* South */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2GBL_MEM_TILE_MODULE_STREAM_SWITCH_MASTER_CONFIG_SOUTH0,
	},
	{	/* West */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* North */
		.NumPorts = 6,
		.PortBaseAddr = XAIE2GBL_MEM_TILE_MODULE_STREAM_SWITCH_MASTER_CONFIG_NORTH0,
	},
	{	/* East */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* Trace */
		.NumPorts = 0,
		.PortBaseAddr = 0
	}
};

/*
 * Array of all Mem Tile Stream Switch Slave Config registers
 * The data structure contains number of ports and the register offsets
 */
static const  XAie_StrmPort Aie2MemTileStrmSlv[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* DMA */
		.NumPorts = 6,
		.PortBaseAddr = XAIE2GBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_DMA_0,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2GBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_TILE_CTRL,
	},
	{	/* Fifo */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* South */
		.NumPorts = 6,
		.PortBaseAddr = XAIE2GBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_SOUTH_0,
	},
	{	/* West */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* North */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2GBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_NORTH_0,
	},
	{	/* East */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* Trace */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2GBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_TRACE
	}
};

/*
 * Array of all Shim NOC/PL Stream Switch Slave Slot Config registers of AIE2.
 * The data structure contains number of ports and the register base address.
 */
static const  XAie_StrmPort Aie2ShimStrmSlaveSlot[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* DMA */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2GBL_PL_MODULE_STREAM_SWITCH_SLAVE_TILE_CTRL_SLOT0,
	},
	{	/* Fifo */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2GBL_PL_MODULE_STREAM_SWITCH_SLAVE_FIFO_0_SLOT0,
	},
	{	/* South */
		.NumPorts = 8,
		.PortBaseAddr = XAIE2GBL_PL_MODULE_STREAM_SWITCH_SLAVE_SOUTH_0_SLOT0,
	},
	{	/* West */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2GBL_PL_MODULE_STREAM_SWITCH_SLAVE_WEST_0_SLOT0,
	},
	{	/* North */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2GBL_PL_MODULE_STREAM_SWITCH_SLAVE_NORTH_0_SLOT0,
	},
	{	/* East */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2GBL_PL_MODULE_STREAM_SWITCH_SLAVE_EAST_0_SLOT0,
	},
	{	/* Trace */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2GBL_PL_MODULE_STREAM_SWITCH_SLAVE_TRACE_SLOT0
	}
};

/*
 * Array of all AIE2 Tile Stream Switch Slave Slot Config registers.
 * The data structure contains number of ports and the register offsets
 */
static const  XAie_StrmPort Aie2TileStrmSlaveSlot[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0,
	},
	{	/* DMA */
		.NumPorts = 2,
		.PortBaseAddr = XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_DMA_0_SLOT0,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_TILE_CTRL_SLOT0,
	},
	{	/* Fifo */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_FIFO_0_SLOT0,
	},
	{	/* South */
		.NumPorts = 6,
		.PortBaseAddr = XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_SOUTH_0_SLOT0,
	},
	{	/* West */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_WEST_0_SLOT0,
	},
	{	/* North */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_NORTH_0_SLOT0,
	},
	{	/* East */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_EAST_0_SLOT0,
	},
	{	/* Trace */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_TRACE_SLOT0
	}
};

/*
 * Array of all AIE2 Mem Tile Stream Switch Slave Slot Config registers
 * The data structure contains number of ports and the register offsets
 */
static const  XAie_StrmPort Aie2MemTileStrmSlaveSlot[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* DMA */
		.NumPorts = 6,
		.PortBaseAddr = XAIE2GBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_DMA_0_SLOT0,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2GBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_TILE_CTRL_SLOT0,
	},
	{	/* Fifo */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* South */
		.NumPorts = 6,
		.PortBaseAddr = XAIE2GBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_SOUTH_0_SLOT0,
	},
	{	/* West */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* North */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2GBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_NORTH_0_SLOT0,
	},
	{	/* East */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* Trace */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2GBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_TRACE_SLOT0,
	}
};

/*
 * Data structure to capture all stream configs for XAIEGBL_TILE_TYPE_AIETILE
 */
static const  XAie_StrmMod Aie2TileStrmSw =
{
	.SlvConfigBaseAddr = XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0,
	.MstrConfigBaseAddr = XAIE2GBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0,
	.PortOffset = 0x4,
	.NumSlaveSlots = 4U,
	.SlotOffsetPerPort = 0x10,
	.SlotOffset = 0x4,
	.MstrEn = {XAIE2GBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_MASTER_ENABLE_LSB, XAIE2GBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_MASTER_ENABLE_MASK},
	.MstrPktEn = {XAIE2GBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_PACKET_ENABLE_LSB, XAIE2GBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_PACKET_ENABLE_MASK},
	.DrpHdr = {XAIE2GBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_DROP_HEADER_LSB, XAIE2GBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_DROP_HEADER_MASK},
	.Config = {XAIE2GBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_CONFIGURATION_LSB, XAIE2GBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_CONFIGURATION_MASK},
	.SlvEn = {XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_SLAVE_ENABLE_LSB, XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_SLAVE_ENABLE_MASK},
	.SlvPktEn = {XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_PACKET_ENABLE_LSB, XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_PACKET_ENABLE_MASK},
	.SlotPktId = {XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ID_LSB, XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ID_MASK},
	.SlotMask = {XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MASK_LSB, XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MASK_MASK},
	.SlotEn = {XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ENABLE_LSB, XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ENABLE_MASK},
	.SlotMsel = {XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MSEL_LSB, XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MSEL_MASK},
	.SlotArbitor = {XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ARBIT_LSB, XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ARBIT_MASK},
	.MstrConfig = Aie2TileStrmMstr,
	.SlvConfig = Aie2TileStrmSlv,
	.SlvSlotConfig = Aie2TileStrmSlaveSlot
};

/*
 * Data structure to capture all stream configs for XAIEGBL_TILE_TYPE_SHIMNOC/PL
 */
static const  XAie_StrmMod Aie2ShimStrmSw =
{
	.SlvConfigBaseAddr = XAIE2GBL_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_TILE_CTRL,
	.MstrConfigBaseAddr = XAIE2GBL_PL_MODULE_STREAM_SWITCH_MASTER_CONFIG_TILE_CTRL,
	.PortOffset = 0x4,
	.NumSlaveSlots = 4U,
	.SlotOffsetPerPort = 0x10,
	.SlotOffset = 0x4,
	.MstrEn = {XAIE2GBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_MASTER_ENABLE_LSB, XAIE2GBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_MASTER_ENABLE_MASK},
	.MstrPktEn = {XAIE2GBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_PACKET_ENABLE_LSB, XAIE2GBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_PACKET_ENABLE_MASK},
	.DrpHdr = {XAIE2GBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_DROP_HEADER_LSB, XAIE2GBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_DROP_HEADER_MASK},
	.Config = {XAIE2GBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_CONFIGURATION_LSB, XAIE2GBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_CONFIGURATION_MASK},
	.SlvEn = {XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_SLAVE_ENABLE_LSB, XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_SLAVE_ENABLE_MASK},
	.SlvPktEn = {XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_PACKET_ENABLE_LSB, XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_PACKET_ENABLE_MASK},
	.SlotPktId = {XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ID_LSB, XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ID_MASK},
	.SlotMask = {XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MASK_LSB, XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MASK_MASK},
	.SlotEn = {XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ENABLE_LSB, XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ENABLE_MASK},
	.SlotMsel = {XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MSEL_LSB, XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MSEL_MASK},
	.SlotArbitor = {XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ARBIT_LSB, XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ARBIT_MASK},
	.MstrConfig = Aie2ShimStrmMstr,
	.SlvConfig = Aie2ShimStrmSlv,
	.SlvSlotConfig = Aie2ShimStrmSlaveSlot
};

/*
 * Data structure to capture all stream configs for XAIEGBL_TILE_TYPE_MEMTILE
 */
static const  XAie_StrmMod Aie2MemTileStrmSw =
{
	.SlvConfigBaseAddr = XAIE2GBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_DMA_0,
	.MstrConfigBaseAddr = XAIE2GBL_MEM_TILE_MODULE_STREAM_SWITCH_MASTER_CONFIG_DMA0,
	.PortOffset = 0x4,
	.NumSlaveSlots = 4U,
	.SlotOffsetPerPort = 0x10,
	.SlotOffset = 0x4,
	.MstrEn = {XAIE2GBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_MASTER_ENABLE_LSB, XAIE2GBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_MASTER_ENABLE_MASK},
	.MstrPktEn = {XAIE2GBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_PACKET_ENABLE_LSB, XAIE2GBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_PACKET_ENABLE_MASK},
	.DrpHdr = {XAIE2GBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_DROP_HEADER_LSB, XAIE2GBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_DROP_HEADER_MASK},
	.Config = {XAIE2GBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_CONFIGURATION_LSB, XAIE2GBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_CONFIGURATION_MASK},
	.SlvEn = {XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_SLAVE_ENABLE_LSB, XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_SLAVE_ENABLE_MASK},
	.SlvPktEn = {XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_PACKET_ENABLE_LSB, XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_PACKET_ENABLE_MASK},
	.SlotPktId = {XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ID_LSB, XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ID_MASK},
	.SlotMask = {XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MASK_LSB, XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MASK_MASK},
	.SlotEn = {XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ENABLE_LSB, XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ENABLE_MASK},
	.SlotMsel = {XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MSEL_LSB, XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MSEL_MASK},
	.SlotArbitor = {XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ARBIT_LSB, XAIE2GBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ARBIT_MASK},
	.MstrConfig = Aie2MemTileStrmMstr,
	.SlvConfig = Aie2MemTileStrmSlv,
	.SlvSlotConfig = Aie2MemTileStrmSlaveSlot
};

/* Register field attributes for PL interface down sizer for 32 and 64 bits */
static const  XAie_RegFldAttr Aie2DownSzr32_64Bit[] =
{
	{XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH0_LSB, XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH0_MASK},
	{XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH1_LSB, XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH1_MASK},
	{XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH2_LSB, XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH2_MASK},
	{XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH3_LSB, XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH3_MASK},
	{XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH4_LSB, XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH4_MASK},
	{XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH5_LSB, XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH5_MASK},
	{XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH6_LSB, XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH6_MASK},
	{XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH7_LSB, XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH7_MASK}
};

/* Register field attributes for PL interface down sizer for 128 bits */
static const  XAie_RegFldAttr Aie2DownSzr128Bit[] =
{
	{XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH0_SOUTH1_128_COMBINE_LSB, XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH0_SOUTH1_128_COMBINE_MASK},
	{XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH2_SOUTH3_128_COMBINE_LSB, XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH2_SOUTH3_128_COMBINE_MASK},
	{XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH4_SOUTH5_128_COMBINE_LSB, XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH4_SOUTH5_128_COMBINE_MASK},
	{XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH6_SOUTH7_128_COMBINE_LSB, XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH6_SOUTH7_128_COMBINE_MASK}
};

/* Register field attributes for PL interface up sizer */
static const  XAie_RegFldAttr Aie2UpSzr32_64Bit[] =
{
	{XAIE2GBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH0_LSB, XAIE2GBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH0_MASK},
	{XAIE2GBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH1_LSB, XAIE2GBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH1_MASK},
	{XAIE2GBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH2_LSB, XAIE2GBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH2_MASK},
	{XAIE2GBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH3_LSB, XAIE2GBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH3_MASK},
	{XAIE2GBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH4_LSB, XAIE2GBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH4_MASK},
	{XAIE2GBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH5_LSB, XAIE2GBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH5_MASK}
};

/* Register field attributes for PL interface up sizer for 128 bits */
static const  XAie_RegFldAttr Aie2UpSzr128Bit[] =
{
	{XAIE2GBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH0_SOUTH1_128_COMBINE_LSB, XAIE2GBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH0_SOUTH1_128_COMBINE_MASK},
	{XAIE2GBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH2_SOUTH3_128_COMBINE_LSB, XAIE2GBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH2_SOUTH3_128_COMBINE_MASK},
	{XAIE2GBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH4_SOUTH5_128_COMBINE_LSB, XAIE2GBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH4_SOUTH5_128_COMBINE_MASK}
};

/* Register field attributes for PL interface down sizer bypass */
static const  XAie_RegFldAttr Aie2DownSzrByPass[] =
{
	{XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH0_LSB, XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH0_MASK},
	{XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH1_LSB, XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH1_MASK},
	{XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH2_LSB, XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH2_MASK},
	{XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH4_LSB, XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH4_MASK},
	{XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH5_LSB, XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH5_MASK},
	{XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH6_LSB, XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH6_MASK}
};

/* Register field attributes for PL interface down sizer enable */
static const  XAie_RegFldAttr Aie2DownSzrEnable[] =
{
	{XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH0_LSB, XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH0_MASK},
	{XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH1_LSB, XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH1_MASK},
	{XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH2_LSB, XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH2_MASK},
	{XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH3_LSB, XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH3_MASK},
	{XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH4_LSB, XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH4_MASK},
	{XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH5_LSB, XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH5_MASK},
	{XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH6_LSB, XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH6_MASK},
	{XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH7_LSB, XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH7_MASK}
};

/* Register field attributes for SHIMNOC Mux configuration */
static const  XAie_RegFldAttr Aie2ShimMuxConfig[] =
{
	{XAIE2GBL_NOC_MODULE_MUX_CONFIG_SOUTH2_LSB, XAIE2GBL_NOC_MODULE_MUX_CONFIG_SOUTH2_MASK},
	{XAIE2GBL_NOC_MODULE_MUX_CONFIG_SOUTH3_LSB, XAIE2GBL_NOC_MODULE_MUX_CONFIG_SOUTH3_MASK},
	{XAIE2GBL_NOC_MODULE_MUX_CONFIG_SOUTH6_LSB, XAIE2GBL_NOC_MODULE_MUX_CONFIG_SOUTH6_MASK},
	{XAIE2GBL_NOC_MODULE_MUX_CONFIG_SOUTH7_LSB, XAIE2GBL_NOC_MODULE_MUX_CONFIG_SOUTH7_MASK},
};

/* Register field attributes for SHIMNOC DeMux configuration */
static const  XAie_RegFldAttr Aie2ShimDeMuxConfig[] =
{
	{XAIE2GBL_NOC_MODULE_DEMUX_CONFIG_SOUTH2_LSB, XAIE2GBL_NOC_MODULE_DEMUX_CONFIG_SOUTH2_MASK},
	{XAIE2GBL_NOC_MODULE_DEMUX_CONFIG_SOUTH3_LSB, XAIE2GBL_NOC_MODULE_DEMUX_CONFIG_SOUTH3_MASK},
	{XAIE2GBL_NOC_MODULE_DEMUX_CONFIG_SOUTH4_LSB, XAIE2GBL_NOC_MODULE_DEMUX_CONFIG_SOUTH4_MASK},
	{XAIE2GBL_NOC_MODULE_DEMUX_CONFIG_SOUTH5_LSB, XAIE2GBL_NOC_MODULE_DEMUX_CONFIG_SOUTH5_MASK}
};

/* Register to set SHIM clock buffer control */
static const XAie_ShimClkBufCntr Aie2ShimClkBufCntr =
{
	.RegOff = 0xFFF20,
	.RstEnable = XAIE_DISABLE,
	.ClkBufEnable = {0, 0x1}
};

static const XAie_ShimRstMod Aie2ShimTileRst =
{
	.RegOff = 0,
	.RstCntr = {0},
	.RstShims = _XAieMl_RstShims,
};

/* Register feild attributes for Shim AXI MM config for NSU Errors */
static const XAie_ShimNocAxiMMConfig Aie2ShimNocAxiMMConfig =
{
	.RegOff = XAIE2GBL_NOC_MODULE_ME_AXIMM_CONFIG,
	.NsuSlvErr = {XAIE2GBL_NOC_MODULE_ME_AXIMM_CONFIG_SLVERR_BLOCK_LSB, XAIE2GBL_NOC_MODULE_ME_AXIMM_CONFIG_SLVERR_BLOCK_MASK},
	.NsuDecErr = {XAIE2GBL_NOC_MODULE_ME_AXIMM_CONFIG_DECERR_BLOCK_LSB, XAIE2GBL_NOC_MODULE_ME_AXIMM_CONFIG_DECERR_BLOCK_MASK}
};

/* Core Module */
static const  XAie_CoreMod Aie2CoreMod =
{
	.IsCheckerBoard = 0U,
	.ProgMemAddr = 0x0,
	.ProgMemSize = 16 * 1024,
	.DataMemAddr = 0x40000,
	.ProgMemHostOffset = XAIE2GBL_CORE_MODULE_PROGRAM_MEMORY,
	.DataMemSize = 64 * 1024,		/* AIE2 Tile Memory is 64kB */
	.DataMemShift = 16,
	.CoreCtrl = &Aie2CoreCtrlReg,
	.CoreSts = &Aie2CoreStsReg,
	.CoreDebug = &Aie2CoreDebugReg
};

/* Data Memory Module for Tile data memory*/
static const  XAie_MemMod Aie2TileMemMod =
{
	.Size = 0x10000,
	.MemAddr = XAIE2GBL_MEMORY_MODULE_DATAMEMORY
};

/* Data Memory Module for Mem Tile data memory*/
static const  XAie_MemMod Aie2MemTileMemMod =
{
	.Size = 0x80000,
	.MemAddr = XAIE2GBL_MEM_TILE_MODULE_DATAMEMORY
};

/* PL Interface module for SHIMPL Tiles */
static const  XAie_PlIfMod Aie2PlIfMod =
{
	.UpSzrOff = XAIE2GBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG,
	.DownSzrOff = XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG,
	.DownSzrEnOff = XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE,
	.DownSzrByPassOff = XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS,
	.ColRstOff = 0xFFF28,
	.NumUpSzrPorts = 0x6,
	.MaxByPassPortNum = 0x6,
	.NumDownSzrPorts = 0x8,
	.UpSzr32_64Bit = Aie2UpSzr32_64Bit,
	.UpSzr128Bit = Aie2UpSzr128Bit,
	.DownSzr32_64Bit = Aie2DownSzr32_64Bit,
	.DownSzr128Bit = Aie2DownSzr128Bit,
	.DownSzrEn = Aie2DownSzrEnable,
	.DownSzrByPass = Aie2DownSzrByPass,
	.ShimNocMuxOff = 0x0,
	.ShimNocDeMuxOff = 0x0,
	.ShimNocMux = NULL,
	.ShimNocDeMux = NULL,
	.ClkBufCntr = &Aie2ShimClkBufCntr,
	.ColRst = {0, 0x1},
	.ShimTileRst = &Aie2ShimTileRst,
	.ShimNocAxiMM = NULL,
};

/* PL Interface module for SHIMNOC Tiles */
static const  XAie_PlIfMod Aie2ShimTilePlIfMod =
{
	.UpSzrOff = XAIE2GBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG,
	.DownSzrOff = XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG,
	.DownSzrEnOff = XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE,
	.DownSzrByPassOff = XAIE2GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS,
	.ColRstOff = 0xFFF28,
	.NumUpSzrPorts = 0x6,
	.MaxByPassPortNum = 0x6,
	.NumDownSzrPorts = 0x8,
	.UpSzr32_64Bit = Aie2UpSzr32_64Bit,
	.UpSzr128Bit = Aie2UpSzr128Bit,
	.DownSzr32_64Bit = Aie2DownSzr32_64Bit,
	.DownSzr128Bit = Aie2DownSzr128Bit,
	.DownSzrEn = Aie2DownSzrEnable,
	.DownSzrByPass = Aie2DownSzrByPass,
	.ShimNocMuxOff = XAIE2GBL_NOC_MODULE_MUX_CONFIG,
	.ShimNocDeMuxOff = XAIE2GBL_NOC_MODULE_DEMUX_CONFIG,
	.ShimNocMux = Aie2ShimMuxConfig,
	.ShimNocDeMux = Aie2ShimDeMuxConfig,
	.ClkBufCntr = &Aie2ShimClkBufCntr,
	.ColRst = {0, 0x1},
	.ShimTileRst = &Aie2ShimTileRst,
	.ShimNocAxiMM = &Aie2ShimNocAxiMMConfig,
};

/* Lock Module for AIE Tiles  */
static const  XAie_LockMod Aie2TileLockMod =
{
	.BaseAddr = XAIE2GBL_MEMORY_MODULE_LOCK_REQUEST,
	.NumLocks = 16U,
	.LockIdOff = 0x400,
	.RelAcqOff = 0x200,
	.LockValOff = 0x4,
	.LockValUpperBound = 63,
	.LockValLowerBound = -64,
	.Acquire = &_XAieMl_LockAcquire,
	.Release = &_XAieMl_LockRelease
};

/* Lock Module for SHIM NOC Tiles  */
static const  XAie_LockMod Aie2ShimNocLockMod =
{
	.BaseAddr = XAIE2GBL_NOC_MODULE_LOCK_REQUEST,
	.NumLocks = 16U,
	.LockIdOff = 0x400,
	.RelAcqOff = 0x200,
	.LockValOff = 0x4,
	.LockValUpperBound = 63,
	.LockValLowerBound = -64,
	.Acquire = &_XAieMl_LockAcquire,
	.Release = &_XAieMl_LockRelease
};

/* Lock Module for Mem Tiles  */
static const  XAie_LockMod Aie2MemTileLockMod =
{
	.BaseAddr = XAIE2GBL_MEM_TILE_MODULE_LOCK_REQUEST,
	.NumLocks = 64U,
	.LockIdOff = 0x400,
	.RelAcqOff = 0x200,
	.LockValOff = 0x4,
	.LockValUpperBound = 63,
	.LockValLowerBound = -64,
	.Acquire = &_XAieMl_LockAcquire,
	.Release = &_XAieMl_LockRelease
};

/* Enum to event number mapping of all events of AIE2 Core Mod of aie tile */
static const u8 Aie2CoreModEventMapping[] =
{
	XAIE2_EVENTS_CORE_NONE,
	XAIE2_EVENTS_CORE_TRUE,
	XAIE2_EVENTS_CORE_GROUP_0,
	XAIE2_EVENTS_CORE_TIMER_SYNC,
	XAIE2_EVENTS_CORE_TIMER_VALUE_REACHED,
	XAIE2_EVENTS_CORE_PERF_CNT_0,
	XAIE2_EVENTS_CORE_PERF_CNT_1,
	XAIE2_EVENTS_CORE_PERF_CNT_2,
	XAIE2_EVENTS_CORE_PERF_CNT_3,
	XAIE2_EVENTS_CORE_COMBO_EVENT_0,
	XAIE2_EVENTS_CORE_COMBO_EVENT_1,
	XAIE2_EVENTS_CORE_COMBO_EVENT_2,
	XAIE2_EVENTS_CORE_COMBO_EVENT_3,
	XAIE2_EVENTS_CORE_GROUP_PC_EVENT,
	XAIE2_EVENTS_CORE_PC_0,
	XAIE2_EVENTS_CORE_PC_1,
	XAIE2_EVENTS_CORE_PC_2,
	XAIE2_EVENTS_CORE_PC_3,
	XAIE2_EVENTS_CORE_PC_RANGE_0_1,
	XAIE2_EVENTS_CORE_PC_RANGE_2_3,
	XAIE2_EVENTS_CORE_GROUP_STALL,
	XAIE2_EVENTS_CORE_MEMORY_STALL,
	XAIE2_EVENTS_CORE_STREAM_STALL,
	XAIE2_EVENTS_CORE_CASCADE_STALL,
	XAIE2_EVENTS_CORE_LOCK_STALL,
	XAIE2_EVENTS_CORE_DEBUG_HALTED,
	XAIE2_EVENTS_CORE_ACTIVE,
	XAIE2_EVENTS_CORE_DISABLED,
	XAIE2_EVENTS_CORE_ECC_ERROR_STALL,
	XAIE2_EVENTS_CORE_ECC_SCRUBBING_STALL,
	XAIE2_EVENTS_CORE_GROUP_PROGRAM_FLOW,
	XAIE2_EVENTS_CORE_INSTR_EVENT_0,
	XAIE2_EVENTS_CORE_INSTR_EVENT_1,
	XAIE2_EVENTS_CORE_INSTR_CALL,
	XAIE2_EVENTS_CORE_INSTR_RETURN,
	XAIE2_EVENTS_CORE_INSTR_VECTOR,
	XAIE2_EVENTS_CORE_INSTR_LOAD,
	XAIE2_EVENTS_CORE_INSTR_STORE,
	XAIE2_EVENTS_CORE_INSTR_STREAM_GET,
	XAIE2_EVENTS_CORE_INSTR_STREAM_PUT,
	XAIE2_EVENTS_CORE_INSTR_CASCADE_GET,
	XAIE2_EVENTS_CORE_INSTR_CASCADE_PUT,
	XAIE2_EVENTS_CORE_INSTR_LOCK_ACQUIRE_REQ,
	XAIE2_EVENTS_CORE_INSTR_LOCK_RELEASE_REQ,
	XAIE2_EVENTS_CORE_GROUP_ERRORS_0,
	XAIE2_EVENTS_CORE_GROUP_ERRORS_1,
	XAIE2_EVENTS_CORE_SRS_OVERFLOW,
	XAIE2_EVENTS_CORE_UPS_OVERFLOW,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE2_EVENTS_CORE_FP_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE2_EVENTS_CORE_PM_REG_ACCESS_FAILURE,
	XAIE2_EVENTS_CORE_STREAM_PKT_PARITY_ERROR,
	XAIE2_EVENTS_CORE_CONTROL_PKT_ERROR,
	XAIE2_EVENTS_CORE_AXI_MM_SLAVE_ERROR,
	XAIE2_EVENTS_CORE_INSTR_DECOMPRSN_ERROR,
	XAIE2_EVENTS_CORE_DM_ADDRESS_OUT_OF_RANGE,
	XAIE2_EVENTS_CORE_PM_ECC_ERROR_SCRUB_CORRECTED,
	XAIE2_EVENTS_CORE_PM_ECC_ERROR_SCRUB_2BIT,
	XAIE2_EVENTS_CORE_PM_ECC_ERROR_1BIT,
	XAIE2_EVENTS_CORE_PM_ECC_ERROR_2BIT,
	XAIE2_EVENTS_CORE_PM_ADDRESS_OUT_OF_RANGE,
	XAIE2_EVENTS_CORE_DM_ACCESS_TO_UNAVAILABLE,
	XAIE2_EVENTS_CORE_LOCK_ACCESS_TO_UNAVAILABLE,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE2_EVENTS_CORE_GROUP_STREAM_SWITCH,
	XAIE2_EVENTS_CORE_PORT_IDLE_0,
	XAIE2_EVENTS_CORE_PORT_RUNNING_0,
	XAIE2_EVENTS_CORE_PORT_STALLED_0,
	XAIE2_EVENTS_CORE_PORT_TLAST_0,
	XAIE2_EVENTS_CORE_PORT_IDLE_1,
	XAIE2_EVENTS_CORE_PORT_RUNNING_1,
	XAIE2_EVENTS_CORE_PORT_STALLED_1,
	XAIE2_EVENTS_CORE_PORT_TLAST_1,
	XAIE2_EVENTS_CORE_PORT_IDLE_2,
	XAIE2_EVENTS_CORE_PORT_RUNNING_2,
	XAIE2_EVENTS_CORE_PORT_STALLED_2,
	XAIE2_EVENTS_CORE_PORT_TLAST_2,
	XAIE2_EVENTS_CORE_PORT_IDLE_3,
	XAIE2_EVENTS_CORE_PORT_RUNNING_3,
	XAIE2_EVENTS_CORE_PORT_STALLED_3,
	XAIE2_EVENTS_CORE_PORT_TLAST_3,
	XAIE2_EVENTS_CORE_PORT_IDLE_4,
	XAIE2_EVENTS_CORE_PORT_RUNNING_4,
	XAIE2_EVENTS_CORE_PORT_STALLED_4,
	XAIE2_EVENTS_CORE_PORT_TLAST_4,
	XAIE2_EVENTS_CORE_PORT_IDLE_5,
	XAIE2_EVENTS_CORE_PORT_RUNNING_5,
	XAIE2_EVENTS_CORE_PORT_STALLED_5,
	XAIE2_EVENTS_CORE_PORT_TLAST_5,
	XAIE2_EVENTS_CORE_PORT_IDLE_6,
	XAIE2_EVENTS_CORE_PORT_RUNNING_6,
	XAIE2_EVENTS_CORE_PORT_STALLED_6,
	XAIE2_EVENTS_CORE_PORT_TLAST_6,
	XAIE2_EVENTS_CORE_PORT_IDLE_7,
	XAIE2_EVENTS_CORE_PORT_RUNNING_7,
	XAIE2_EVENTS_CORE_PORT_STALLED_7,
	XAIE2_EVENTS_CORE_PORT_TLAST_7,
	XAIE2_EVENTS_CORE_GROUP_BROADCAST,
	XAIE2_EVENTS_CORE_BROADCAST_0,
	XAIE2_EVENTS_CORE_BROADCAST_1,
	XAIE2_EVENTS_CORE_BROADCAST_2,
	XAIE2_EVENTS_CORE_BROADCAST_3,
	XAIE2_EVENTS_CORE_BROADCAST_4,
	XAIE2_EVENTS_CORE_BROADCAST_5,
	XAIE2_EVENTS_CORE_BROADCAST_6,
	XAIE2_EVENTS_CORE_BROADCAST_7,
	XAIE2_EVENTS_CORE_BROADCAST_8,
	XAIE2_EVENTS_CORE_BROADCAST_9,
	XAIE2_EVENTS_CORE_BROADCAST_10,
	XAIE2_EVENTS_CORE_BROADCAST_11,
	XAIE2_EVENTS_CORE_BROADCAST_12,
	XAIE2_EVENTS_CORE_BROADCAST_13,
	XAIE2_EVENTS_CORE_BROADCAST_14,
	XAIE2_EVENTS_CORE_BROADCAST_15,
	XAIE2_EVENTS_CORE_GROUP_USER_EVENT,
	XAIE2_EVENTS_CORE_USER_EVENT_0,
	XAIE2_EVENTS_CORE_USER_EVENT_1,
	XAIE2_EVENTS_CORE_USER_EVENT_2,
	XAIE2_EVENTS_CORE_USER_EVENT_3,
	XAIE2_EVENTS_CORE_EDGE_DETECTION_EVENT_0,
	XAIE2_EVENTS_CORE_EDGE_DETECTION_EVENT_1,
	XAIE2_EVENTS_CORE_FP_HUGE,
	XAIE2_EVENTS_CORE_INT_FP_0,
	XAIE2_EVENTS_CORE_FP_INF,
	XAIE2_EVENTS_CORE_INSTR_WARNING,
	XAIE2_EVENTS_CORE_INSTR_ERROR,
	XAIE2_EVENTS_CORE_DECOMPRESSION_UNDERFLOW,
	XAIE2_EVENTS_CORE_STREAM_SWITCH_PORT_PARITY_ERROR,
	XAIE2_EVENTS_CORE_PROCESSOR_BUS_ERROR,
};

/* Enum to event number mapping of all events of AIE2 Mem Mod of aie tile */
static const u8 Aie2MemModEventMapping[] =
{
	XAIE2_EVENTS_MEM_NONE,
	XAIE2_EVENTS_MEM_TRUE,
	XAIE2_EVENTS_MEM_GROUP_0,
	XAIE2_EVENTS_MEM_TIMER_SYNC,
	XAIE2_EVENTS_MEM_TIMER_VALUE_REACHED,
	XAIE2_EVENTS_MEM_PERF_CNT_0,
	XAIE2_EVENTS_MEM_PERF_CNT_1,
	XAIE2_EVENTS_MEM_COMBO_EVENT_0,
	XAIE2_EVENTS_MEM_COMBO_EVENT_1,
	XAIE2_EVENTS_MEM_COMBO_EVENT_2,
	XAIE2_EVENTS_MEM_COMBO_EVENT_3,
	XAIE2_EVENTS_MEM_GROUP_WATCHPOINT,
	XAIE2_EVENTS_MEM_WATCHPOINT_0,
	XAIE2_EVENTS_MEM_WATCHPOINT_1,
	XAIE2_EVENTS_MEM_GROUP_DMA_ACTIVITY,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE2_EVENTS_MEM_DMA_S2MM_0_FINISHED_BD,
	XAIE2_EVENTS_MEM_DMA_S2MM_1_FINISHED_BD,
	XAIE2_EVENTS_MEM_DMA_MM2S_0_FINISHED_BD,
	XAIE2_EVENTS_MEM_DMA_MM2S_1_FINISHED_BD,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE2_EVENTS_MEM_GROUP_LOCK,
	XAIE_EVENT_INVALID,
	XAIE2_EVENTS_MEM_LOCK_0_REL,
	XAIE_EVENT_INVALID,
	XAIE2_EVENTS_MEM_LOCK_1_REL,
	XAIE_EVENT_INVALID,
	XAIE2_EVENTS_MEM_LOCK_2_REL,
	XAIE_EVENT_INVALID,
	XAIE2_EVENTS_MEM_LOCK_3_REL,
	XAIE_EVENT_INVALID,
	XAIE2_EVENTS_MEM_LOCK_4_REL,
	XAIE_EVENT_INVALID,
	XAIE2_EVENTS_MEM_LOCK_5_REL,
	XAIE_EVENT_INVALID,
	XAIE2_EVENTS_MEM_LOCK_6_REL,
	XAIE_EVENT_INVALID,
	XAIE2_EVENTS_MEM_LOCK_7_REL,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE2_EVENTS_MEM_GROUP_MEMORY_CONFLICT,
	XAIE2_EVENTS_MEM_CONFLICT_DM_BANK_0,
	XAIE2_EVENTS_MEM_CONFLICT_DM_BANK_1,
	XAIE2_EVENTS_MEM_CONFLICT_DM_BANK_2,
	XAIE2_EVENTS_MEM_CONFLICT_DM_BANK_3,
	XAIE2_EVENTS_MEM_CONFLICT_DM_BANK_4,
	XAIE2_EVENTS_MEM_CONFLICT_DM_BANK_5,
	XAIE2_EVENTS_MEM_CONFLICT_DM_BANK_6,
	XAIE2_EVENTS_MEM_CONFLICT_DM_BANK_7,
	XAIE2_EVENTS_MEM_GROUP_ERRORS,
	XAIE2_EVENTS_MEM_DM_ECC_ERROR_SCRUB_CORRECTED,
	XAIE2_EVENTS_MEM_DM_ECC_ERROR_SCRUB_2BIT,
	XAIE2_EVENTS_MEM_DM_ECC_ERROR_1BIT,
	XAIE2_EVENTS_MEM_DM_ECC_ERROR_2BIT,
	XAIE2_EVENTS_MEM_DM_PARITY_ERROR_BANK_2,
	XAIE2_EVENTS_MEM_DM_PARITY_ERROR_BANK_3,
	XAIE2_EVENTS_MEM_DM_PARITY_ERROR_BANK_4,
	XAIE2_EVENTS_MEM_DM_PARITY_ERROR_BANK_5,
	XAIE2_EVENTS_MEM_DM_PARITY_ERROR_BANK_6,
	XAIE2_EVENTS_MEM_DM_PARITY_ERROR_BANK_7,
	XAIE2_EVENTS_MEM_DMA_S2MM_0_ERROR,
	XAIE2_EVENTS_MEM_DMA_S2MM_1_ERROR,
	XAIE2_EVENTS_MEM_DMA_MM2S_0_ERROR,
	XAIE2_EVENTS_MEM_DMA_MM2S_1_ERROR,
	XAIE2_EVENTS_MEM_GROUP_BROADCAST,
	XAIE2_EVENTS_MEM_BROADCAST_0,
	XAIE2_EVENTS_MEM_BROADCAST_1,
	XAIE2_EVENTS_MEM_BROADCAST_2,
	XAIE2_EVENTS_MEM_BROADCAST_3,
	XAIE2_EVENTS_MEM_BROADCAST_4,
	XAIE2_EVENTS_MEM_BROADCAST_5,
	XAIE2_EVENTS_MEM_BROADCAST_6,
	XAIE2_EVENTS_MEM_BROADCAST_7,
	XAIE2_EVENTS_MEM_BROADCAST_8,
	XAIE2_EVENTS_MEM_BROADCAST_9,
	XAIE2_EVENTS_MEM_BROADCAST_10,
	XAIE2_EVENTS_MEM_BROADCAST_11,
	XAIE2_EVENTS_MEM_BROADCAST_12,
	XAIE2_EVENTS_MEM_BROADCAST_13,
	XAIE2_EVENTS_MEM_BROADCAST_14,
	XAIE2_EVENTS_MEM_BROADCAST_15,
	XAIE2_EVENTS_MEM_GROUP_USER_EVENT,
	XAIE2_EVENTS_MEM_USER_EVENT_0,
	XAIE2_EVENTS_MEM_USER_EVENT_1,
	XAIE2_EVENTS_MEM_USER_EVENT_2,
	XAIE2_EVENTS_MEM_USER_EVENT_3,
	XAIE2_EVENTS_MEM_EDGE_DETECTION_EVENT_0,
	XAIE2_EVENTS_MEM_EDGE_DETECTION_EVENT_1,
	XAIE2_EVENTS_MEM_DMA_S2MM_0_START_TASK,
	XAIE2_EVENTS_MEM_DMA_S2MM_1_START_TASK,
	XAIE2_EVENTS_MEM_DMA_MM2S_0_START_TASK,
	XAIE2_EVENTS_MEM_DMA_MM2S_1_START_TASK,
	XAIE2_EVENTS_MEM_DMA_S2MM_0_FINISHED_TASK,
	XAIE2_EVENTS_MEM_DMA_S2MM_1_FINISHED_TASK,
	XAIE2_EVENTS_MEM_DMA_MM2S_0_FINISHED_TASK,
	XAIE2_EVENTS_MEM_DMA_MM2S_1_FINISHED_TASK,
	XAIE2_EVENTS_MEM_DMA_S2MM_0_STALLED_LOCK,
	XAIE2_EVENTS_MEM_DMA_S2MM_1_STALLED_LOCK,
	XAIE2_EVENTS_MEM_DMA_MM2S_0_STALLED_LOCK,
	XAIE2_EVENTS_MEM_DMA_MM2S_1_STALLED_LOCK,
	XAIE2_EVENTS_MEM_DMA_S2MM_0_STREAM_STARVATION,
	XAIE2_EVENTS_MEM_DMA_S2MM_1_STREAM_STARVATION,
	XAIE2_EVENTS_MEM_DMA_MM2S_0_STREAM_BACKPRESSURE,
	XAIE2_EVENTS_MEM_DMA_MM2S_1_STREAM_BACKPRESSURE,
	XAIE2_EVENTS_MEM_DMA_S2MM_0_MEMORY_BACKPRESSURE,
	XAIE2_EVENTS_MEM_DMA_S2MM_1_MEMORY_BACKPRESSURE,
	XAIE2_EVENTS_MEM_DMA_MM2S_0_MEMORY_STARVATION,
	XAIE2_EVENTS_MEM_DMA_MM2S_1_MEMORY_STARVATION,
	XAIE2_EVENTS_MEM_LOCK_SEL0_ACQ_EQ,
	XAIE2_EVENTS_MEM_LOCK_SEL0_ACQ_GE,
	XAIE2_EVENTS_MEM_LOCK_SEL0_EQUAL_TO_VALUE,
	XAIE2_EVENTS_MEM_LOCK_SEL1_ACQ_EQ,
	XAIE2_EVENTS_MEM_LOCK_SEL1_ACQ_GE,
	XAIE2_EVENTS_MEM_LOCK_SEL1_EQUAL_TO_VALUE,
	XAIE2_EVENTS_MEM_LOCK_SEL2_ACQ_EQ,
	XAIE2_EVENTS_MEM_LOCK_SEL2_ACQ_GE,
	XAIE2_EVENTS_MEM_LOCK_SEL2_EQUAL_TO_VALUE,
	XAIE2_EVENTS_MEM_LOCK_SEL3_ACQ_EQ,
	XAIE2_EVENTS_MEM_LOCK_SEL3_ACQ_GE,
	XAIE2_EVENTS_MEM_LOCK_SEL3_EQUAL_TO_VALUE,
	XAIE2_EVENTS_MEM_LOCK_SEL4_ACQ_EQ,
	XAIE2_EVENTS_MEM_LOCK_SEL4_ACQ_GE,
	XAIE2_EVENTS_MEM_LOCK_SEL4_EQUAL_TO_VALUE,
	XAIE2_EVENTS_MEM_LOCK_SEL5_ACQ_EQ,
	XAIE2_EVENTS_MEM_LOCK_SEL5_ACQ_GE,
	XAIE2_EVENTS_MEM_LOCK_SEL5_EQUAL_TO_VALUE,
	XAIE2_EVENTS_MEM_LOCK_SEL6_ACQ_EQ,
	XAIE2_EVENTS_MEM_LOCK_SEL6_ACQ_GE,
	XAIE2_EVENTS_MEM_LOCK_SEL6_EQUAL_TO_VALUE,
	XAIE2_EVENTS_MEM_LOCK_SEL7_ACQ_EQ,
	XAIE2_EVENTS_MEM_LOCK_SEL7_ACQ_GE,
	XAIE2_EVENTS_MEM_LOCK_SEL7_EQUAL_TO_VALUE,
	XAIE2_EVENTS_MEM_LOCK_ERROR,
	XAIE2_EVENTS_MEM_DMA_TASK_TOKEN_STALL,
};

/* Enum to event number mapping of all events of AIE2 NOC tile */
static const u8 Aie2NocModEventMapping[] =
{
	XAIE2_EVENTS_PL_NONE,
	XAIE2_EVENTS_PL_TRUE,
	XAIE2_EVENTS_PL_GROUP_0,
	XAIE2_EVENTS_PL_TIMER_SYNC,
	XAIE2_EVENTS_PL_TIMER_VALUE_REACHED,
	XAIE2_EVENTS_PL_PERF_CNT_0,
	XAIE2_EVENTS_PL_PERF_CNT_1,
	XAIE2_EVENTS_PL_COMBO_EVENT_0,
	XAIE2_EVENTS_PL_COMBO_EVENT_1,
	XAIE2_EVENTS_PL_COMBO_EVENT_2,
	XAIE2_EVENTS_PL_COMBO_EVENT_3,
	XAIE2_EVENTS_PL_GROUP_DMA_ACTIVITY,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE2_EVENTS_PL_DMA_S2MM_0_FINISHED_BD,
	XAIE2_EVENTS_PL_DMA_S2MM_1_FINISHED_BD,
	XAIE2_EVENTS_PL_DMA_MM2S_0_FINISHED_BD,
	XAIE2_EVENTS_PL_DMA_MM2S_1_FINISHED_BD,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE2_EVENTS_PL_GROUP_LOCK,
	XAIE_EVENT_INVALID,
	XAIE2_EVENTS_PL_LOCK_0_REL,
	XAIE_EVENT_INVALID,
	XAIE2_EVENTS_PL_LOCK_1_REL,
	XAIE_EVENT_INVALID,
	XAIE2_EVENTS_PL_LOCK_2_REL,
	XAIE_EVENT_INVALID,
	XAIE2_EVENTS_PL_LOCK_3_REL,
	XAIE_EVENT_INVALID,
	XAIE2_EVENTS_PL_LOCK_4_REL,
	XAIE_EVENT_INVALID,
	XAIE2_EVENTS_PL_LOCK_5_REL,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE2_EVENTS_PL_GROUP_ERRORS,
	XAIE2_EVENTS_PL_AXI_MM_SLAVE_ERROR,
	XAIE2_EVENTS_PL_CONTROL_PKT_ERROR,
	XAIE2_EVENTS_PL_AXI_MM_DECODE_NSU_ERROR,
	XAIE2_EVENTS_PL_AXI_MM_SLAVE_NSU_ERROR,
	XAIE2_EVENTS_PL_AXI_MM_UNSUPPORTED_TRAFFIC,
	XAIE2_EVENTS_PL_AXI_MM_UNSECURE_ACCESS_IN_SECURE_MODE,
	XAIE2_EVENTS_PL_AXI_MM_BYTE_STROBE_ERROR,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE2_EVENTS_PL_GROUP_STREAM_SWITCH,
	XAIE2_EVENTS_PL_PORT_IDLE_0,
	XAIE2_EVENTS_PL_PORT_RUNNING_0,
	XAIE2_EVENTS_PL_PORT_STALLED_0,
	XAIE2_EVENTS_PL_PORT_TLAST_0,
	XAIE2_EVENTS_PL_PORT_IDLE_1,
	XAIE2_EVENTS_PL_PORT_RUNNING_1,
	XAIE2_EVENTS_PL_PORT_STALLED_1,
	XAIE2_EVENTS_PL_PORT_TLAST_1,
	XAIE2_EVENTS_PL_PORT_IDLE_2,
	XAIE2_EVENTS_PL_PORT_RUNNING_2,
	XAIE2_EVENTS_PL_PORT_STALLED_2,
	XAIE2_EVENTS_PL_PORT_TLAST_2,
	XAIE2_EVENTS_PL_PORT_IDLE_3,
	XAIE2_EVENTS_PL_PORT_RUNNING_3,
	XAIE2_EVENTS_PL_PORT_STALLED_3,
	XAIE2_EVENTS_PL_PORT_TLAST_3,
	XAIE2_EVENTS_PL_PORT_IDLE_4,
	XAIE2_EVENTS_PL_PORT_RUNNING_4,
	XAIE2_EVENTS_PL_PORT_STALLED_4,
	XAIE2_EVENTS_PL_PORT_TLAST_4,
	XAIE2_EVENTS_PL_PORT_IDLE_5,
	XAIE2_EVENTS_PL_PORT_RUNNING_5,
	XAIE2_EVENTS_PL_PORT_STALLED_5,
	XAIE2_EVENTS_PL_PORT_TLAST_5,
	XAIE2_EVENTS_PL_PORT_IDLE_6,
	XAIE2_EVENTS_PL_PORT_RUNNING_6,
	XAIE2_EVENTS_PL_PORT_STALLED_6,
	XAIE2_EVENTS_PL_PORT_TLAST_6,
	XAIE2_EVENTS_PL_PORT_IDLE_7,
	XAIE2_EVENTS_PL_PORT_RUNNING_7,
	XAIE2_EVENTS_PL_PORT_STALLED_7,
	XAIE2_EVENTS_PL_PORT_TLAST_7,
	XAIE2_EVENTS_PL_GROUP_BROADCAST_A,
	XAIE2_EVENTS_PL_BROADCAST_A_0,
	XAIE2_EVENTS_PL_BROADCAST_A_1,
	XAIE2_EVENTS_PL_BROADCAST_A_2,
	XAIE2_EVENTS_PL_BROADCAST_A_3,
	XAIE2_EVENTS_PL_BROADCAST_A_4,
	XAIE2_EVENTS_PL_BROADCAST_A_5,
	XAIE2_EVENTS_PL_BROADCAST_A_6,
	XAIE2_EVENTS_PL_BROADCAST_A_7,
	XAIE2_EVENTS_PL_BROADCAST_A_8,
	XAIE2_EVENTS_PL_BROADCAST_A_9,
	XAIE2_EVENTS_PL_BROADCAST_A_10,
	XAIE2_EVENTS_PL_BROADCAST_A_11,
	XAIE2_EVENTS_PL_BROADCAST_A_12,
	XAIE2_EVENTS_PL_BROADCAST_A_13,
	XAIE2_EVENTS_PL_BROADCAST_A_14,
	XAIE2_EVENTS_PL_BROADCAST_A_15,
	XAIE_EVENT_INVALID,
	XAIE2_EVENTS_PL_USER_EVENT_0,
	XAIE2_EVENTS_PL_USER_EVENT_1,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE2_EVENTS_PL_EDGE_DETECTION_EVENT_0,
	XAIE2_EVENTS_PL_EDGE_DETECTION_EVENT_1,
	XAIE2_EVENTS_PL_DMA_S2MM_0_START_TASK,
	XAIE2_EVENTS_PL_DMA_S2MM_1_START_TASK,
	XAIE2_EVENTS_PL_DMA_MM2S_0_START_TASK,
	XAIE2_EVENTS_PL_DMA_MM2S_1_START_TASK,
	XAIE2_EVENTS_PL_DMA_S2MM_0_FINISHED_TASK,
	XAIE2_EVENTS_PL_DMA_S2MM_1_FINISHED_TASK,
	XAIE2_EVENTS_PL_DMA_MM2S_0_FINISHED_TASK,
	XAIE2_EVENTS_PL_DMA_MM2S_1_FINISHED_TASK,
	XAIE2_EVENTS_PL_DMA_S2MM_0_STALLED_LOCK,
	XAIE2_EVENTS_PL_DMA_S2MM_1_STALLED_LOCK,
	XAIE2_EVENTS_PL_DMA_MM2S_0_STALLED_LOCK,
	XAIE2_EVENTS_PL_DMA_MM2S_1_STALLED_LOCK,
	XAIE2_EVENTS_PL_DMA_S2MM_0_STREAM_STARVATION,
	XAIE2_EVENTS_PL_DMA_S2MM_1_STREAM_STARVATION,
	XAIE2_EVENTS_PL_DMA_MM2S_0_STREAM_BACKPRESSURE,
	XAIE2_EVENTS_PL_DMA_MM2S_1_STREAM_BACKPRESSURE,
	XAIE2_EVENTS_PL_DMA_S2MM_0_MEMORY_BACKPRESSURE,
	XAIE2_EVENTS_PL_DMA_S2MM_1_MEMORY_BACKPRESSURE,
	XAIE2_EVENTS_PL_DMA_MM2S_0_MEMORY_STARVATION,
	XAIE2_EVENTS_PL_DMA_MM2S_1_MEMORY_STARVATION,
	XAIE2_EVENTS_PL_LOCK_0_ACQ_EQ,
	XAIE2_EVENTS_PL_LOCK_0_ACQ_GE,
	XAIE2_EVENTS_PL_LOCK_0_EQUAL_TO_VALUE,
	XAIE2_EVENTS_PL_LOCK_1_ACQ_EQ,
	XAIE2_EVENTS_PL_LOCK_1_ACQ_GE,
	XAIE2_EVENTS_PL_LOCK_1_EQUAL_TO_VALUE,
	XAIE2_EVENTS_PL_LOCK_2_ACQ_EQ,
	XAIE2_EVENTS_PL_LOCK_2_ACQ_GE,
	XAIE2_EVENTS_PL_LOCK_2_EQUAL_TO_VALUE,
	XAIE2_EVENTS_PL_LOCK_3_ACQ_EQ,
	XAIE2_EVENTS_PL_LOCK_3_ACQ_GE,
	XAIE2_EVENTS_PL_LOCK_3_EQUAL_TO_VALUE,
	XAIE2_EVENTS_PL_LOCK_4_ACQ_EQ,
	XAIE2_EVENTS_PL_LOCK_4_ACQ_GE,
	XAIE2_EVENTS_PL_LOCK_4_EQUAL_TO_VALUE,
	XAIE2_EVENTS_PL_LOCK_5_ACQ_EQ,
	XAIE2_EVENTS_PL_LOCK_5_ACQ_GE,
	XAIE2_EVENTS_PL_LOCK_5_EQUAL_TO_VALUE,
	XAIE2_EVENTS_PL_STREAM_SWITCH_PARITY_ERROR,
	XAIE2_EVENTS_PL_DMA_S2MM_ERROR,
	XAIE2_EVENTS_PL_DMA_MM2S_ERROR,
	XAIE2_EVENTS_PL_LOCK_ERROR,
	XAIE2_EVENTS_PL_DMA_TASK_TOKEN_STALL,
};

/* Enum to event number mapping of all events of AIE2 PL Module */
static const u8 Aie2PlModEventMapping[] =
{
	XAIE2_EVENTS_PL_NONE,
	XAIE2_EVENTS_PL_TRUE,
	XAIE2_EVENTS_PL_GROUP_0,
	XAIE2_EVENTS_PL_TIMER_SYNC,
	XAIE2_EVENTS_PL_TIMER_VALUE_REACHED,
	XAIE2_EVENTS_PL_PERF_CNT_0,
	XAIE2_EVENTS_PL_PERF_CNT_1,
	XAIE2_EVENTS_PL_COMBO_EVENT_0,
	XAIE2_EVENTS_PL_COMBO_EVENT_1,
	XAIE2_EVENTS_PL_COMBO_EVENT_2,
	XAIE2_EVENTS_PL_COMBO_EVENT_3,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE2_EVENTS_PL_GROUP_ERRORS,
	XAIE2_EVENTS_PL_AXI_MM_SLAVE_ERROR,
	XAIE2_EVENTS_PL_CONTROL_PKT_ERROR,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE2_EVENTS_PL_GROUP_STREAM_SWITCH,
	XAIE2_EVENTS_PL_PORT_IDLE_0,
	XAIE2_EVENTS_PL_PORT_RUNNING_0,
	XAIE2_EVENTS_PL_PORT_STALLED_0,
	XAIE2_EVENTS_PL_PORT_TLAST_0,
	XAIE2_EVENTS_PL_PORT_IDLE_1,
	XAIE2_EVENTS_PL_PORT_RUNNING_1,
	XAIE2_EVENTS_PL_PORT_STALLED_1,
	XAIE2_EVENTS_PL_PORT_TLAST_1,
	XAIE2_EVENTS_PL_PORT_IDLE_2,
	XAIE2_EVENTS_PL_PORT_RUNNING_2,
	XAIE2_EVENTS_PL_PORT_STALLED_2,
	XAIE2_EVENTS_PL_PORT_TLAST_2,
	XAIE2_EVENTS_PL_PORT_IDLE_3,
	XAIE2_EVENTS_PL_PORT_RUNNING_3,
	XAIE2_EVENTS_PL_PORT_STALLED_3,
	XAIE2_EVENTS_PL_PORT_TLAST_3,
	XAIE2_EVENTS_PL_PORT_IDLE_4,
	XAIE2_EVENTS_PL_PORT_RUNNING_4,
	XAIE2_EVENTS_PL_PORT_STALLED_4,
	XAIE2_EVENTS_PL_PORT_TLAST_4,
	XAIE2_EVENTS_PL_PORT_IDLE_5,
	XAIE2_EVENTS_PL_PORT_RUNNING_5,
	XAIE2_EVENTS_PL_PORT_STALLED_5,
	XAIE2_EVENTS_PL_PORT_TLAST_5,
	XAIE2_EVENTS_PL_PORT_IDLE_6,
	XAIE2_EVENTS_PL_PORT_RUNNING_6,
	XAIE2_EVENTS_PL_PORT_STALLED_6,
	XAIE2_EVENTS_PL_PORT_TLAST_6,
	XAIE2_EVENTS_PL_PORT_IDLE_7,
	XAIE2_EVENTS_PL_PORT_RUNNING_7,
	XAIE2_EVENTS_PL_PORT_STALLED_7,
	XAIE2_EVENTS_PL_PORT_TLAST_7,
	XAIE2_EVENTS_PL_GROUP_BROADCAST_A,
	XAIE2_EVENTS_PL_BROADCAST_A_0,
	XAIE2_EVENTS_PL_BROADCAST_A_1,
	XAIE2_EVENTS_PL_BROADCAST_A_2,
	XAIE2_EVENTS_PL_BROADCAST_A_3,
	XAIE2_EVENTS_PL_BROADCAST_A_4,
	XAIE2_EVENTS_PL_BROADCAST_A_5,
	XAIE2_EVENTS_PL_BROADCAST_A_6,
	XAIE2_EVENTS_PL_BROADCAST_A_7,
	XAIE2_EVENTS_PL_BROADCAST_A_8,
	XAIE2_EVENTS_PL_BROADCAST_A_9,
	XAIE2_EVENTS_PL_BROADCAST_A_10,
	XAIE2_EVENTS_PL_BROADCAST_A_11,
	XAIE2_EVENTS_PL_BROADCAST_A_12,
	XAIE2_EVENTS_PL_BROADCAST_A_13,
	XAIE2_EVENTS_PL_BROADCAST_A_14,
	XAIE2_EVENTS_PL_BROADCAST_A_15,
	XAIE_EVENT_INVALID,
	XAIE2_EVENTS_PL_USER_EVENT_0,
	XAIE2_EVENTS_PL_USER_EVENT_1,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE2_EVENTS_PL_EDGE_DETECTION_EVENT_0,
	XAIE2_EVENTS_PL_EDGE_DETECTION_EVENT_1,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE2_EVENTS_PL_STREAM_SWITCH_PARITY_ERROR,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
};

/* Enum to event number mapping of all events of AIE2 Mem Tile Module */
static const u8 Aie2MemTileModEventMapping[] =
{
	XAIE2_EVENTS_MEM_TILE_NONE,
	XAIE2_EVENTS_MEM_TILE_TRUE,
	XAIE2_EVENTS_MEM_TILE_GROUP_0,
	XAIE2_EVENTS_MEM_TILE_TIMER_SYNC,
	XAIE2_EVENTS_MEM_TILE_TIMER_VALUE_REACHED,
	XAIE2_EVENTS_MEM_TILE_PERF_CNT0_EVENT,
	XAIE2_EVENTS_MEM_TILE_PERF_CNT1_EVENT,
	XAIE2_EVENTS_MEM_TILE_PERF_CNT2_EVENT,
	XAIE2_EVENTS_MEM_TILE_PERF_CNT3_EVENT,
	XAIE2_EVENTS_MEM_TILE_COMBO_EVENT_0,
	XAIE2_EVENTS_MEM_TILE_COMBO_EVENT_1,
	XAIE2_EVENTS_MEM_TILE_COMBO_EVENT_2,
	XAIE2_EVENTS_MEM_TILE_COMBO_EVENT_3,
	XAIE2_EVENTS_MEM_TILE_EDGE_DETECTION_EVENT_0,
	XAIE2_EVENTS_MEM_TILE_EDGE_DETECTION_EVENT_1,
	XAIE2_EVENTS_MEM_TILE_GROUP_WATCHPOINT,
	XAIE2_EVENTS_MEM_TILE_WATCHPOINT_0,
	XAIE2_EVENTS_MEM_TILE_WATCHPOINT_1,
	XAIE2_EVENTS_MEM_TILE_WATCHPOINT_2,
	XAIE2_EVENTS_MEM_TILE_WATCHPOINT_3,
	XAIE2_EVENTS_MEM_TILE_GROUP_DMA_ACTIVITY,
	XAIE2_EVENTS_MEM_TILE_DMA_S2MM_SEL0_START_TASK,
	XAIE2_EVENTS_MEM_TILE_DMA_S2MM_SEL1_START_TASK,
	XAIE2_EVENTS_MEM_TILE_DMA_MM2S_SEL0_START_TASK,
	XAIE2_EVENTS_MEM_TILE_DMA_MM2S_SEL1_START_TASK,
	XAIE2_EVENTS_MEM_TILE_DMA_S2MM_SEL0_FINISHED_BD,
	XAIE2_EVENTS_MEM_TILE_DMA_S2MM_SEL1_FINISHED_BD,
	XAIE2_EVENTS_MEM_TILE_DMA_MM2S_SEL0_FINISHED_BD,
	XAIE2_EVENTS_MEM_TILE_DMA_MM2S_SEL1_FINISHED_BD,
	XAIE2_EVENTS_MEM_TILE_DMA_S2MM_SEL0_FINISHED_TASK,
	XAIE2_EVENTS_MEM_TILE_DMA_S2MM_SEL1_FINISHED_TASK,
	XAIE2_EVENTS_MEM_TILE_DMA_MM2S_SEL0_FINISHED_TASK,
	XAIE2_EVENTS_MEM_TILE_DMA_MM2S_SEL1_FINISHED_TASK,
	XAIE2_EVENTS_MEM_TILE_DMA_S2MM_SEL0_STALLED_LOCK,
	XAIE2_EVENTS_MEM_TILE_DMA_S2MM_SEL1_STALLED_LOCK,
	XAIE2_EVENTS_MEM_TILE_DMA_MM2S_SEL0_STALLED_LOCK,
	XAIE2_EVENTS_MEM_TILE_DMA_MM2S_SEL1_STALLED_LOCK,
	XAIE2_EVENTS_MEM_TILE_DMA_S2MM_SEL0_STREAM_STARVATION,
	XAIE2_EVENTS_MEM_TILE_DMA_S2MM_SEL1_STREAM_STARVATION,
	XAIE2_EVENTS_MEM_TILE_DMA_MM2S_SEL0_STREAM_BACKPRESSURE,
	XAIE2_EVENTS_MEM_TILE_DMA_MM2S_SEL1_STREAM_BACKPRESSURE,
	XAIE2_EVENTS_MEM_TILE_DMA_S2MM_SEL0_MEMORY_BACKPRESSURE,
	XAIE2_EVENTS_MEM_TILE_DMA_S2MM_SEL1_MEMORY_BACKPRESSURE,
	XAIE2_EVENTS_MEM_TILE_DMA_MM2S_SEL0_MEMORY_STARVATION,
	XAIE2_EVENTS_MEM_TILE_DMA_MM2S_SEL1_MEMORY_STARVATION,
	XAIE2_EVENTS_MEM_TILE_GROUP_LOCK,
	XAIE2_EVENTS_MEM_TILE_LOCK_SEL0_ACQ_EQ,
	XAIE2_EVENTS_MEM_TILE_LOCK_SEL0_ACQ_GE,
	XAIE2_EVENTS_MEM_TILE_LOCK_SEL0_REL,
	XAIE2_EVENTS_MEM_TILE_LOCK_SEL0_EQUAL_TO_VALUE,
	XAIE2_EVENTS_MEM_TILE_LOCK_SEL1_ACQ_EQ,
	XAIE2_EVENTS_MEM_TILE_LOCK_SEL1_ACQ_GE,
	XAIE2_EVENTS_MEM_TILE_LOCK_SEL1_REL,
	XAIE2_EVENTS_MEM_TILE_LOCK_SEL1_EQUAL_TO_VALUE,
	XAIE2_EVENTS_MEM_TILE_LOCK_SEL2_ACQ_EQ,
	XAIE2_EVENTS_MEM_TILE_LOCK_SEL2_ACQ_GE,
	XAIE2_EVENTS_MEM_TILE_LOCK_SEL2_REL,
	XAIE2_EVENTS_MEM_TILE_LOCK_SEL2_EQUAL_TO_VALUE,
	XAIE2_EVENTS_MEM_TILE_LOCK_SEL3_ACQ_EQ,
	XAIE2_EVENTS_MEM_TILE_LOCK_SEL3_ACQ_GE,
	XAIE2_EVENTS_MEM_TILE_LOCK_SEL3_REL,
	XAIE2_EVENTS_MEM_TILE_LOCK_SEL3_EQUAL_TO_VALUE,
	XAIE2_EVENTS_MEM_TILE_LOCK_SEL4_ACQ_EQ,
	XAIE2_EVENTS_MEM_TILE_LOCK_SEL4_ACQ_GE,
	XAIE2_EVENTS_MEM_TILE_LOCK_SEL4_REL,
	XAIE2_EVENTS_MEM_TILE_LOCK_SEL4_EQUAL_TO_VALUE,
	XAIE2_EVENTS_MEM_TILE_LOCK_SEL5_ACQ_EQ,
	XAIE2_EVENTS_MEM_TILE_LOCK_SEL5_ACQ_GE,
	XAIE2_EVENTS_MEM_TILE_LOCK_SEL5_REL,
	XAIE2_EVENTS_MEM_TILE_LOCK_SEL5_EQUAL_TO_VALUE,
	XAIE2_EVENTS_MEM_TILE_LOCK_SEL6_ACQ_EQ,
	XAIE2_EVENTS_MEM_TILE_LOCK_SEL6_ACQ_GE,
	XAIE2_EVENTS_MEM_TILE_LOCK_SEL6_REL,
	XAIE2_EVENTS_MEM_TILE_LOCK_SEL6_EQUAL_TO_VALUE,
	XAIE2_EVENTS_MEM_TILE_LOCK_SEL7_ACQ_EQ,
	XAIE2_EVENTS_MEM_TILE_LOCK_SEL7_ACQ_GE,
	XAIE2_EVENTS_MEM_TILE_LOCK_SEL7_REL,
	XAIE2_EVENTS_MEM_TILE_LOCK_SEL7_EQUAL_TO_VALUE,
	XAIE2_EVENTS_MEM_TILE_GROUP_STREAM_SWITCH,
	XAIE2_EVENTS_MEM_TILE_PORT_IDLE_0,
	XAIE2_EVENTS_MEM_TILE_PORT_RUNNING_0,
	XAIE2_EVENTS_MEM_TILE_PORT_STALLED_0,
	XAIE2_EVENTS_MEM_TILE_PORT_TLAST_0,
	XAIE2_EVENTS_MEM_TILE_PORT_IDLE_1,
	XAIE2_EVENTS_MEM_TILE_PORT_RUNNING_1,
	XAIE2_EVENTS_MEM_TILE_PORT_STALLED_1,
	XAIE2_EVENTS_MEM_TILE_PORT_TLAST_1,
	XAIE2_EVENTS_MEM_TILE_PORT_IDLE_2,
	XAIE2_EVENTS_MEM_TILE_PORT_RUNNING_2,
	XAIE2_EVENTS_MEM_TILE_PORT_STALLED_2,
	XAIE2_EVENTS_MEM_TILE_PORT_TLAST_2,
	XAIE2_EVENTS_MEM_TILE_PORT_IDLE_3,
	XAIE2_EVENTS_MEM_TILE_PORT_RUNNING_3,
	XAIE2_EVENTS_MEM_TILE_PORT_STALLED_3,
	XAIE2_EVENTS_MEM_TILE_PORT_TLAST_3,
	XAIE2_EVENTS_MEM_TILE_PORT_IDLE_4,
	XAIE2_EVENTS_MEM_TILE_PORT_RUNNING_4,
	XAIE2_EVENTS_MEM_TILE_PORT_STALLED_4,
	XAIE2_EVENTS_MEM_TILE_PORT_TLAST_4,
	XAIE2_EVENTS_MEM_TILE_PORT_IDLE_5,
	XAIE2_EVENTS_MEM_TILE_PORT_RUNNING_5,
	XAIE2_EVENTS_MEM_TILE_PORT_STALLED_5,
	XAIE2_EVENTS_MEM_TILE_PORT_TLAST_5,
	XAIE2_EVENTS_MEM_TILE_PORT_IDLE_6,
	XAIE2_EVENTS_MEM_TILE_PORT_RUNNING_6,
	XAIE2_EVENTS_MEM_TILE_PORT_STALLED_6,
	XAIE2_EVENTS_MEM_TILE_PORT_TLAST_6,
	XAIE2_EVENTS_MEM_TILE_PORT_IDLE_7,
	XAIE2_EVENTS_MEM_TILE_PORT_RUNNING_7,
	XAIE2_EVENTS_MEM_TILE_PORT_STALLED_7,
	XAIE2_EVENTS_MEM_TILE_PORT_TLAST_7,
	XAIE2_EVENTS_MEM_TILE_GROUP_MEMORY_CONFLICT,
	XAIE2_EVENTS_MEM_TILE_CONFLICT_DM_BANK_0,
	XAIE2_EVENTS_MEM_TILE_CONFLICT_DM_BANK_1,
	XAIE2_EVENTS_MEM_TILE_CONFLICT_DM_BANK_2,
	XAIE2_EVENTS_MEM_TILE_CONFLICT_DM_BANK_3,
	XAIE2_EVENTS_MEM_TILE_CONFLICT_DM_BANK_4,
	XAIE2_EVENTS_MEM_TILE_CONFLICT_DM_BANK_5,
	XAIE2_EVENTS_MEM_TILE_CONFLICT_DM_BANK_6,
	XAIE2_EVENTS_MEM_TILE_CONFLICT_DM_BANK_7,
	XAIE2_EVENTS_MEM_TILE_CONFLICT_DM_BANK_8,
	XAIE2_EVENTS_MEM_TILE_CONFLICT_DM_BANK_9,
	XAIE2_EVENTS_MEM_TILE_CONFLICT_DM_BANK_10,
	XAIE2_EVENTS_MEM_TILE_CONFLICT_DM_BANK_11,
	XAIE2_EVENTS_MEM_TILE_CONFLICT_DM_BANK_12,
	XAIE2_EVENTS_MEM_TILE_CONFLICT_DM_BANK_13,
	XAIE2_EVENTS_MEM_TILE_CONFLICT_DM_BANK_14,
	XAIE2_EVENTS_MEM_TILE_CONFLICT_DM_BANK_15,
	XAIE2_EVENTS_MEM_TILE_GROUP_ERRORS,
	XAIE2_EVENTS_MEM_TILE_DM_ECC_ERROR_SCRUB_CORRECTED,
	XAIE2_EVENTS_MEM_TILE_DM_ECC_ERROR_SCRUB_2BIT,
	XAIE2_EVENTS_MEM_TILE_DM_ECC_ERROR_1BIT,
	XAIE2_EVENTS_MEM_TILE_DM_ECC_ERROR_2BIT,
	XAIE2_EVENTS_MEM_TILE_DMA_S2MM_ERROR,
	XAIE2_EVENTS_MEM_TILE_DMA_MM2S_ERROR,
	XAIE2_EVENTS_MEM_TILE_STREAM_SWITCH_PARITY_ERROR,
	XAIE2_EVENTS_MEM_TILE_STREAM_PKT_ERROR,
	XAIE2_EVENTS_MEM_TILE_CONTROL_PKT_ERROR,
	XAIE2_EVENTS_MEM_TILE_AXI_MM_SLAVE_ERROR,
	XAIE2_EVENTS_MEM_TILE_LOCK_ERROR,
	XAIE2_EVENTS_MEM_TILE_DMA_TASK_TOKEN_STALL,
	XAIE2_EVENTS_MEM_TILE_GROUP_BROADCAST,
	XAIE2_EVENTS_MEM_TILE_BROADCAST_0,
	XAIE2_EVENTS_MEM_TILE_BROADCAST_1,
	XAIE2_EVENTS_MEM_TILE_BROADCAST_2,
	XAIE2_EVENTS_MEM_TILE_BROADCAST_3,
	XAIE2_EVENTS_MEM_TILE_BROADCAST_4,
	XAIE2_EVENTS_MEM_TILE_BROADCAST_5,
	XAIE2_EVENTS_MEM_TILE_BROADCAST_6,
	XAIE2_EVENTS_MEM_TILE_BROADCAST_7,
	XAIE2_EVENTS_MEM_TILE_BROADCAST_8,
	XAIE2_EVENTS_MEM_TILE_BROADCAST_9,
	XAIE2_EVENTS_MEM_TILE_BROADCAST_10,
	XAIE2_EVENTS_MEM_TILE_BROADCAST_11,
	XAIE2_EVENTS_MEM_TILE_BROADCAST_12,
	XAIE2_EVENTS_MEM_TILE_BROADCAST_13,
	XAIE2_EVENTS_MEM_TILE_BROADCAST_14,
	XAIE2_EVENTS_MEM_TILE_BROADCAST_15,
	XAIE2_EVENTS_MEM_TILE_GROUP_USER_EVENT,
	XAIE2_EVENTS_MEM_TILE_USER_EVENT_0,
	XAIE2_EVENTS_MEM_TILE_USER_EVENT_1,
};

/*
 * Data structure to capture registers & offsets for Core and memory Module of
 * performance counter.
 */
static const XAie_PerfMod Aie2TilePerfCnt[] =
{
	{	.MaxCounterVal = 2U,
		.StartStopShift = 16U,
		.ResetShift = 8U,
		.PerfCounterOffsetAdd = 0X4,
		.PerfCtrlBaseAddr = XAIE2GBL_MEMORY_MODULE_PERFORMANCE_CONTROL0,
		.PerfCtrlOffsetAdd = 0x4,
		.PerfCtrlResetBaseAddr = XAIE2GBL_MEMORY_MODULE_PERFORMANCE_CONTROL1,
		.PerfCounterBaseAddr = XAIE2GBL_MEMORY_MODULE_PERFORMANCE_COUNTER0,
		.PerfCounterEvtValBaseAddr = XAIE2GBL_MEMORY_MODULE_PERFORMANCE_COUNTER0_EVENT_VALUE,
		{XAIE2GBL_MEMORY_MODULE_PERFORMANCE_CONTROL0_CNT0_START_EVENT_LSB, XAIE2GBL_MEMORY_MODULE_PERFORMANCE_CONTROL0_CNT0_START_EVENT_MASK},
		{XAIE2GBL_MEMORY_MODULE_PERFORMANCE_CONTROL0_CNT0_STOP_EVENT_LSB, XAIE2GBL_MEMORY_MODULE_PERFORMANCE_CONTROL0_CNT0_STOP_EVENT_MASK},
		{XAIE2GBL_MEMORY_MODULE_PERFORMANCE_CONTROL1_CNT0_RESET_EVENT_LSB,XAIE2GBL_MEMORY_MODULE_PERFORMANCE_CONTROL1_CNT0_RESET_EVENT_MASK},
	},
	{	.MaxCounterVal = 4U,
		.StartStopShift = 16U,
		.ResetShift = 8U,
		.PerfCounterOffsetAdd = 0X4,
		.PerfCtrlBaseAddr = XAIE2GBL_CORE_MODULE_PERFORMANCE_CONTROL0,
		.PerfCtrlOffsetAdd = 0x4,
		.PerfCtrlResetBaseAddr = XAIE2GBL_CORE_MODULE_PERFORMANCE_CONTROL2,
		.PerfCounterBaseAddr = XAIE2GBL_CORE_MODULE_PERFORMANCE_COUNTER0,
		.PerfCounterEvtValBaseAddr = XAIE2GBL_CORE_MODULE_PERFORMANCE_COUNTER0_EVENT_VALUE,
		{XAIE2GBL_CORE_MODULE_PERFORMANCE_CONTROL0_CNT0_START_EVENT_LSB, XAIE2GBL_CORE_MODULE_PERFORMANCE_CONTROL0_CNT0_START_EVENT_MASK},
		{XAIE2GBL_CORE_MODULE_PERFORMANCE_CONTROL0_CNT0_STOP_EVENT_LSB, XAIE2GBL_CORE_MODULE_PERFORMANCE_CONTROL0_CNT0_STOP_EVENT_MASK},
		{XAIE2GBL_CORE_MODULE_PERFORMANCE_CONTROL2_CNT0_RESET_EVENT_LSB, XAIE2GBL_CORE_MODULE_PERFORMANCE_CONTROL2_CNT0_RESET_EVENT_MASK},
	}
};

/*
 * Data structure to capture registers & offsets for PL Module of performance
 * counter.
 */
static const XAie_PerfMod Aie2PlPerfCnt =
{
	.MaxCounterVal = 2U,
	.StartStopShift = 16U,
	.ResetShift = 8U,
	.PerfCounterOffsetAdd = 0x4,
	.PerfCtrlBaseAddr = XAIE2GBL_PL_MODULE_PERFORMANCE_CTRL0,
	.PerfCtrlOffsetAdd = 0x0,
	.PerfCtrlResetBaseAddr = XAIE2GBL_PL_MODULE_PERFORMANCE_CTRL1,
	.PerfCounterBaseAddr = XAIE2GBL_PL_MODULE_PERFORMANCE_COUNTER0,
	.PerfCounterEvtValBaseAddr = XAIE2GBL_PL_MODULE_PERFORMANCE_COUNTER0_EVENT_VALUE,
	{XAIE2GBL_PL_MODULE_PERFORMANCE_CTRL0_CNT0_START_EVENT_LSB, XAIE2GBL_PL_MODULE_PERFORMANCE_CTRL0_CNT0_START_EVENT_MASK},
	{XAIE2GBL_PL_MODULE_PERFORMANCE_CTRL0_CNT0_STOP_EVENT_LSB, XAIE2GBL_PL_MODULE_PERFORMANCE_CTRL0_CNT0_STOP_EVENT_MASK},
	{XAIE2GBL_PL_MODULE_PERFORMANCE_CTRL1_CNT0_RESET_EVENT_LSB,XAIE2GBL_PL_MODULE_PERFORMANCE_CTRL1_CNT0_RESET_EVENT_MASK},};

/*
 * Data structure to capture registers & offsets for Mem tile Module of
 * performance counter.
 */
static const XAie_PerfMod Aie2MemTilePerfCnt =
{
	.MaxCounterVal = 4U,
	.StartStopShift = 16U,
	.ResetShift = 8U,
	.PerfCounterOffsetAdd = 0X4,
	.PerfCtrlBaseAddr = XAIE2GBL_MEM_TILE_MODULE_PERFORMANCE_CONTROL0,
	.PerfCtrlOffsetAdd = 0x4,
	.PerfCtrlResetBaseAddr = XAIE2GBL_MEM_TILE_MODULE_PERFORMANCE_CONTROL2,
	.PerfCounterBaseAddr = XAIE2GBL_MEM_TILE_MODULE_PERFORMANCE_COUNTER0,
	.PerfCounterEvtValBaseAddr = XAIE2GBL_MEM_TILE_MODULE_PERFORMANCE_COUNTER0_EVENT_VALUE,
	{XAIE2GBL_MEM_TILE_MODULE_PERFORMANCE_CONTROL0_CNT0_START_EVENT_LSB, XAIE2GBL_MEM_TILE_MODULE_PERFORMANCE_CONTROL0_CNT0_START_EVENT_MASK},
	{XAIE2GBL_MEM_TILE_MODULE_PERFORMANCE_CONTROL0_CNT0_STOP_EVENT_LSB, XAIE2GBL_MEM_TILE_MODULE_PERFORMANCE_CONTROL0_CNT0_STOP_EVENT_MASK},
	{XAIE2GBL_MEM_TILE_MODULE_PERFORMANCE_CONTROL2_CNT0_RESET_EVENT_LSB, XAIE2GBL_MEM_TILE_MODULE_PERFORMANCE_CONTROL2_CNT0_RESET_EVENT_MASK},
};

static const XAie_EventGroup Aie2MemGroupEvent[] =
{
	{
		.GroupEvent = XAIE_EVENT_GROUP_0_MEM,
		.GroupOff = 0U,
		.GroupMask = XAIE2GBL_MEMORY_MODULE_EVENT_GROUP_0_ENABLE_MASK,
		.ResetValue = XAIE2GBL_MEMORY_MODULE_EVENT_GROUP_0_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_WATCHPOINT_MEM,
		.GroupOff = 1U,
		.GroupMask = XAIE2GBL_MEMORY_MODULE_EVENT_GROUP_WATCHPOINT_ENABLE_MASK,
		.ResetValue = XAIE2GBL_MEMORY_MODULE_EVENT_GROUP_WATCHPOINT_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_DMA_ACTIVITY_MEM,
		.GroupOff = 2U,
		.GroupMask = XAIE2GBL_MEMORY_MODULE_EVENT_GROUP_DMA_ENABLE_MASK,
		.ResetValue = XAIE2GBL_MEMORY_MODULE_EVENT_GROUP_DMA_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_LOCK_MEM,
		.GroupOff = 3U,
		.GroupMask = XAIE2GBL_MEMORY_MODULE_EVENT_GROUP_LOCK_ENABLE_MASK,
		.ResetValue = XAIE2GBL_MEMORY_MODULE_EVENT_GROUP_LOCK_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_MEMORY_CONFLICT_MEM,
		.GroupOff = 4U,
		.GroupMask = XAIE2GBL_MEMORY_MODULE_EVENT_GROUP_MEMORY_CONFLICT_ENABLE_MASK,
		.ResetValue = XAIE2GBL_MEMORY_MODULE_EVENT_GROUP_MEMORY_CONFLICT_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_ERRORS_MEM,
		.GroupOff = 5U,
		.GroupMask = XAIE2GBL_MEMORY_MODULE_EVENT_GROUP_ERROR_ENABLE_MASK,
		.ResetValue = XAIE2GBL_MEMORY_MODULE_EVENT_GROUP_ERROR_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_BROADCAST_MEM,
		.GroupOff = 6U,
		.GroupMask = XAIE2GBL_MEMORY_MODULE_EVENT_GROUP_BROADCAST_ENABLE_MASK,
		.ResetValue = XAIE2GBL_MEMORY_MODULE_EVENT_GROUP_BROADCAST_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_USER_EVENT_MEM,
		.GroupOff = 7U,
		.GroupMask = XAIE2GBL_MEMORY_MODULE_EVENT_GROUP_USER_EVENT_ENABLE_MASK,
		.ResetValue = XAIE2GBL_MEMORY_MODULE_EVENT_GROUP_USER_EVENT_ENABLE_MASK,
	},
};

static const XAie_EventGroup Aie2CoreGroupEvent[] =
{
	{
		.GroupEvent = XAIE_EVENT_GROUP_0_CORE,
		.GroupOff = 0U,
		.GroupMask = XAIE2GBL_CORE_MODULE_EVENT_GROUP_0_ENABLE_MASK,
		.ResetValue = XAIE2GBL_CORE_MODULE_EVENT_GROUP_0_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_PC_EVENT_CORE,
		.GroupOff = 1U,
		.GroupMask = XAIE2GBL_CORE_MODULE_EVENT_GROUP_PC_ENABLE_MASK,
		.ResetValue = XAIE2GBL_CORE_MODULE_EVENT_GROUP_PC_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_CORE_STALL_CORE,
		.GroupOff = 2U,
		.GroupMask = XAIE2GBL_CORE_MODULE_EVENT_GROUP_CORE_STALL_ENABLE_MASK,
		.ResetValue = XAIE2GBL_CORE_MODULE_EVENT_GROUP_CORE_STALL_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_CORE_PROGRAM_FLOW_CORE,
		.GroupOff = 3U,
		.GroupMask = XAIE2GBL_CORE_MODULE_EVENT_GROUP_CORE_PROGRAM_FLOW_ENABLE_MASK,
		.ResetValue = XAIE2GBL_CORE_MODULE_EVENT_GROUP_CORE_PROGRAM_FLOW_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_ERRORS_0_CORE,
		.GroupOff = 4U,
		.GroupMask = XAIE2GBL_CORE_MODULE_EVENT_GROUP_ERRORS0_ENABLE_MASK,
		.ResetValue = XAIE2GBL_CORE_MODULE_EVENT_GROUP_ERRORS0_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_ERRORS_1_CORE,
		.GroupOff = 5U,
		.GroupMask = XAIE2GBL_CORE_MODULE_EVENT_GROUP_ERRORS1_ENABLE_MASK,
		.ResetValue = XAIE2GBL_CORE_MODULE_EVENT_GROUP_ERRORS1_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_STREAM_SWITCH_CORE,
		.GroupOff = 6U,
		.GroupMask = XAIE2GBL_CORE_MODULE_EVENT_GROUP_STREAM_SWITCH_ENABLE_MASK,
		.ResetValue = XAIE2GBL_CORE_MODULE_EVENT_GROUP_STREAM_SWITCH_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_BROADCAST_CORE,
		.GroupOff = 7U,
		.GroupMask = XAIE2GBL_CORE_MODULE_EVENT_GROUP_BROADCAST_ENABLE_MASK,
		.ResetValue = XAIE2GBL_CORE_MODULE_EVENT_GROUP_BROADCAST_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_USER_EVENT_CORE,
		.GroupOff = 8U,
		.GroupMask = XAIE2GBL_CORE_MODULE_EVENT_GROUP_USER_EVENT_ENABLE_MASK,
		.ResetValue = XAIE2GBL_CORE_MODULE_EVENT_GROUP_USER_EVENT_ENABLE_MASK,
	},
};

static const XAie_EventGroup Aie2PlGroupEvent[] =
{
	{
		.GroupEvent = XAIE_EVENT_GROUP_0_PL,
		.GroupOff = 0U,
		.GroupMask = XAIE2GBL_PL_MODULE_EVENT_GROUP_0_ENABLE_MASK,
		.ResetValue = XAIE2GBL_PL_MODULE_EVENT_GROUP_0_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_DMA_ACTIVITY_PL,
		.GroupOff = 1U,
		.GroupMask = XAIE2GBL_PL_MODULE_EVENT_GROUP_DMA_ACTIVITY_ENABLE_MASK,
		.ResetValue = XAIE2GBL_PL_MODULE_EVENT_GROUP_DMA_ACTIVITY_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_LOCK_PL,
		.GroupOff = 2U,
		.GroupMask = XAIE2GBL_PL_MODULE_EVENT_GROUP_LOCK_ENABLE_MASK,
		.ResetValue = XAIE2GBL_PL_MODULE_EVENT_GROUP_LOCK_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_ERRORS_PL,
		.GroupOff = 3U,
		.GroupMask = XAIE2GBL_PL_MODULE_EVENT_GROUP_ERRORS_ENABLE_MASK,
		.ResetValue = XAIE2GBL_PL_MODULE_EVENT_GROUP_ERRORS_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_STREAM_SWITCH_PL,
		.GroupOff = 4U,
		.GroupMask = XAIE2GBL_PL_MODULE_EVENT_GROUP_STREAM_SWITCH_ENABLE_MASK,
		.ResetValue = XAIE2GBL_PL_MODULE_EVENT_GROUP_STREAM_SWITCH_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_BROADCAST_A_PL,
		.GroupOff = 5U,
		.GroupMask = XAIE2GBL_PL_MODULE_EVENT_GROUP_BROADCAST_A_ENABLE_MASK,
		.ResetValue = XAIE2GBL_PL_MODULE_EVENT_GROUP_BROADCAST_A_ENABLE_MASK,
	},
};

static const XAie_EventGroup Aie2MemTileGroupEvent[] = {
	{
		.GroupEvent = XAIE_EVENT_GROUP_0_MEM_TILE,
		.GroupOff = 0U,
		.GroupMask = XAIE2GBL_MEM_TILE_MODULE_EVENT_GROUP_0_ENABLE_MASK,
		.ResetValue = XAIE2GBL_MEM_TILE_MODULE_EVENT_GROUP_0_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_WATCHPOINT_MEM_TILE,
		.GroupOff = 1U,
		.GroupMask = XAIE2GBL_MEM_TILE_MODULE_EVENT_GROUP_WATCHPOINT_ENABLE_MASK,
		.ResetValue = XAIE2GBL_MEM_TILE_MODULE_EVENT_GROUP_WATCHPOINT_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_DMA_ACTIVITY_MEM_TILE,
		.GroupOff = 2U,
		.GroupMask = XAIE2GBL_MEM_TILE_MODULE_EVENT_GROUP_DMA_ENABLE_MASK,
		.ResetValue = XAIE2GBL_MEM_TILE_MODULE_EVENT_GROUP_DMA_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_LOCK_MEM_TILE,
		.GroupOff = 3U,
		.GroupMask = XAIE2GBL_MEM_TILE_MODULE_EVENT_GROUP_LOCK_ENABLE_MASK,
		.ResetValue = XAIE2GBL_MEM_TILE_MODULE_EVENT_GROUP_LOCK_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_STREAM_SWITCH_MEM_TILE,
		.GroupOff = 4U,
		.GroupMask = XAIE2GBL_MEM_TILE_MODULE_EVENT_GROUP_STREAM_SWITCH_ENABLE_MASK,
		.ResetValue = XAIE2GBL_MEM_TILE_MODULE_EVENT_GROUP_STREAM_SWITCH_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_MEMORY_CONFLICT_MEM_TILE,
		.GroupOff = 5U,
		.GroupMask = XAIE2GBL_MEM_TILE_MODULE_EVENT_GROUP_MEMORY_CONFLICT_ENABLE_MASK,
		.ResetValue = XAIE2GBL_MEM_TILE_MODULE_EVENT_GROUP_MEMORY_CONFLICT_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_ERRORS_MEM_TILE,
		.GroupOff = 6U,
		.GroupMask = XAIE2GBL_MEM_TILE_MODULE_EVENT_GROUP_ERROR_ENABLE_MASK,
		.ResetValue = XAIE2GBL_MEM_TILE_MODULE_EVENT_GROUP_ERROR_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_BROADCAST_MEM_TILE,
		.GroupOff = 7U,
		.GroupMask = XAIE2GBL_MEM_TILE_MODULE_EVENT_GROUP_BROADCAST_ENABLE_MASK,
		.ResetValue = XAIE2GBL_MEM_TILE_MODULE_EVENT_GROUP_BROADCAST_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_USER_EVENT_MEM_TILE,
		.GroupOff = 8U,
		.GroupMask = XAIE2GBL_MEM_TILE_MODULE_EVENT_GROUP_USER_EVENT_ENABLE_MASK,
		.ResetValue = XAIE2GBL_MEM_TILE_MODULE_EVENT_GROUP_USER_EVENT_ENABLE_MASK,
	},
};

/*
 * Data structure to capture core and memory module events properties.
 * For memory module default error group mask enables,
 *	DM_ECC_Error_Scrub_2bit,
 *	DM_ECC_Error_2bit,
 *	DM_Parity_Error_Bank_2,
 *	DM_Parity_Error_Bank_3,
 *	DM_Parity_Error_Bank_4,
 *	DM_Parity_Error_Bank_5,
 *	DM_Parity_Error_Bank_6,
 *	DM_Parity_Error_Bank_7,
 *	DMA_S2MM_0_Error,
 *	DMA_S2MM_1_Error,
 *	DMA_MM2S_0_Error,
 *	DMA_MM2S_1_Error,
 *	Lock_Error.
 * For core module default error group mask enables,
 *	PM_Reg_Access_Failure,
 *	Stream_Pkt_Parity_Error,
 *	Control_Pkt_Error,
 *	AXI_MM_Slave_Error,
 *	Instruction_Decompression_Error,
 *	DM_address_out_of_range,
 *	PM_ECC_Error_Scrub_2bit,
 *	PM_ECC_Error_2bit,
 *	PM_address_out_of_range,
 *	DM_access_to_Unavailable,
 *	Lock_Access_to_Unavailable,
 *	Decompression_underflow,
 *	Stream_Switch_Port_Parity_Error,
 *	Processor_Bus_Error.
 */
static const XAie_EvntMod Aie2TileEvntMod[] =
{
	{
		.XAie_EventNumber = Aie2MemModEventMapping,
		.EventMin = XAIE_EVENT_NONE_MEM,
		.EventMax = XAIE_EVENT_DMA_TASK_TOKEN_STALL_MEM,
		.GenEventRegOff = XAIE2GBL_MEMORY_MODULE_EVENT_GENERATE,
		.GenEvent = {XAIE2GBL_MEMORY_MODULE_EVENT_GENERATE_EVENT_LSB, XAIE2GBL_MEMORY_MODULE_EVENT_GENERATE_EVENT_MASK},
		.ComboInputRegOff = XAIE2GBL_MEMORY_MODULE_COMBO_EVENT_INPUTS,
		.ComboEventMask = XAIE2GBL_MEMORY_MODULE_COMBO_EVENT_INPUTS_EVENTA_MASK,
		.ComboEventOff = 8U,
		.ComboCtrlRegOff = XAIE2GBL_MEMORY_MODULE_COMBO_EVENT_CONTROL,
		.ComboConfigMask = XAIE2GBL_MEMORY_MODULE_COMBO_EVENT_CONTROL_COMBO0_MASK,
		.ComboConfigOff = 8U,
		.BaseStrmPortSelectRegOff = XAIE_FEATURE_UNAVAILABLE,
		.NumStrmPortSelectIds = XAIE_FEATURE_UNAVAILABLE,
		.StrmPortSelectIdsPerReg = XAIE_FEATURE_UNAVAILABLE,
		.PortIdMask = XAIE_FEATURE_UNAVAILABLE,
		.PortIdOff = XAIE_FEATURE_UNAVAILABLE,
		.PortMstrSlvMask = XAIE_FEATURE_UNAVAILABLE,
		.PortMstrSlvOff = XAIE_FEATURE_UNAVAILABLE,
		.BaseBroadcastRegOff = XAIE2GBL_MEMORY_MODULE_EVENT_BROADCAST0,
		.NumBroadcastIds = 16U,
		.BaseBroadcastSwBlockRegOff = XAIE2GBL_MEMORY_MODULE_EVENT_BROADCAST_BLOCK_SOUTH_SET,
		.BaseBroadcastSwUnblockRegOff = XAIE2GBL_MEMORY_MODULE_EVENT_BROADCAST_BLOCK_SOUTH_CLR,
		.BroadcastSwOff = 0U,
		.BroadcastSwBlockOff = 16U,
		.BroadcastSwUnblockOff = 16U,
		.NumSwitches = 1U,
		.BaseGroupEventRegOff = XAIE2GBL_MEMORY_MODULE_EVENT_GROUP_0_ENABLE,
		.NumGroupEvents = 8U,
		.DefaultGroupErrorMask = 0x7FFAU,
		.Group = Aie2MemGroupEvent,
		.BasePCEventRegOff = XAIE_FEATURE_UNAVAILABLE,
		.NumPCEvents = XAIE_FEATURE_UNAVAILABLE,
		.PCAddr = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
		.PCValid = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
	},
	{
		.XAie_EventNumber = Aie2CoreModEventMapping,
		.EventMin = XAIE_EVENT_NONE_CORE,
		.EventMax = XAIE_EVENT_PROCESSOR_BUS_ERROR_CORE,
		.GenEventRegOff = XAIE2GBL_CORE_MODULE_EVENT_GENERATE,
		.GenEvent = {XAIE2GBL_CORE_MODULE_EVENT_GENERATE_EVENT_LSB, XAIE2GBL_CORE_MODULE_EVENT_GENERATE_EVENT_MASK},
		.ComboInputRegOff = XAIE2GBL_CORE_MODULE_COMBO_EVENT_INPUTS,
		.ComboEventMask = XAIE2GBL_CORE_MODULE_COMBO_EVENT_INPUTS_EVENTA_MASK,
		.ComboEventOff = 8U,
		.ComboCtrlRegOff = XAIE2GBL_CORE_MODULE_COMBO_EVENT_CONTROL,
		.ComboConfigMask = XAIE2GBL_CORE_MODULE_COMBO_EVENT_CONTROL_COMBO0_MASK,
		.ComboConfigOff = 8U,
		.BaseStrmPortSelectRegOff = XAIE2GBL_CORE_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_0,
		.NumStrmPortSelectIds = 8U,
		.StrmPortSelectIdsPerReg = 4U,
		.PortIdMask = XAIE2GBL_CORE_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_0_PORT_0_ID_MASK,
		.PortIdOff = 8U,
		.PortMstrSlvMask = XAIE2GBL_CORE_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_0_PORT_0_MASTER_SLAVE_MASK,
		.PortMstrSlvOff = 8U,
		.BaseBroadcastRegOff = XAIE2GBL_CORE_MODULE_EVENT_BROADCAST0,
		.NumBroadcastIds = 16U,
		.BaseBroadcastSwBlockRegOff = XAIE2GBL_CORE_MODULE_EVENT_BROADCAST_BLOCK_SOUTH_SET,
		.BaseBroadcastSwUnblockRegOff = XAIE2GBL_CORE_MODULE_EVENT_BROADCAST_BLOCK_SOUTH_CLR,
		.BroadcastSwOff = 0U,
		.BroadcastSwBlockOff = 16U,
		.BroadcastSwUnblockOff = 16U,
		.NumSwitches = 1U,
		.BaseGroupEventRegOff = XAIE2GBL_CORE_MODULE_EVENT_GROUP_0_ENABLE,
		.NumGroupEvents = 9U,
		.DefaultGroupErrorMask = 0x1CF5F80U,
		.Group = Aie2CoreGroupEvent,
		.BasePCEventRegOff = XAIE2GBL_CORE_MODULE_PC_EVENT0,
		.NumPCEvents = 4U,
		.PCAddr = {XAIE2GBL_CORE_MODULE_PC_EVENT0_PC_ADDRESS_LSB, XAIE2GBL_CORE_MODULE_PC_EVENT0_PC_ADDRESS_MASK},
		.PCValid = {XAIE2GBL_CORE_MODULE_PC_EVENT0_VALID_LSB, XAIE2GBL_CORE_MODULE_PC_EVENT0_VALID_MASK},
	}
};

/*
 * Data structure to capture NOC tile events properties.
 * For PL module default error group mask enables,
 *	AXI_MM_Slave_Tile_Error,
 *	Control_Pkt_Error,
 *	Stream_Switch_Parity_Error,
 *	AXI_MM_Decode_NSU_Error,
 *	AXI_MM_Slave_NSU_Error,
 *	AXI_MM_Unsupported_Traffic,
 *	AXI_MM_Unsecure_Access_in_Secure_Mode,
 *	AXI_MM_Byte_Strobe_Error,
 *	DMA_S2MM_Error,
 *	DMA_MM2S_Error,
 *	Lock_Error.
 */
static const XAie_EvntMod Aie2NocEvntMod =
{
	.XAie_EventNumber = Aie2NocModEventMapping,
	.EventMin = XAIE_EVENT_NONE_PL,
	.EventMax = XAIE_EVENT_DMA_TASK_TOKEN_STALL_PL,
	.GenEventRegOff = XAIE2GBL_PL_MODULE_EVENT_GENERATE,
	.GenEvent = {XAIE2GBL_PL_MODULE_EVENT_GENERATE_EVENT_LSB, XAIE2GBL_PL_MODULE_EVENT_GENERATE_EVENT_MASK},
	.ComboInputRegOff = XAIE2GBL_PL_MODULE_COMBO_EVENT_INPUTS,
	.ComboEventMask = XAIE2GBL_PL_MODULE_COMBO_EVENT_INPUTS_EVENTA_MASK,
	.ComboEventOff = 8U,
	.ComboCtrlRegOff = XAIE2GBL_PL_MODULE_COMBO_EVENT_CONTROL,
	.ComboConfigMask = XAIE2GBL_PL_MODULE_COMBO_EVENT_CONTROL_COMBO0_MASK,
	.ComboConfigOff = 8U,
	.BaseStrmPortSelectRegOff = XAIE2GBL_PL_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_0,
	.NumStrmPortSelectIds = 8U,
	.StrmPortSelectIdsPerReg = 4U,
	.PortIdMask = XAIE2GBL_PL_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_0_PORT_0_ID_MASK,
	.PortIdOff = 8U,
	.PortMstrSlvMask = XAIE2GBL_PL_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_0_PORT_0_MASTER_SLAVE_MASK,
	.PortMstrSlvOff = 8U,
	.BaseBroadcastRegOff = XAIE2GBL_PL_MODULE_EVENT_BROADCAST0_A,
	.NumBroadcastIds = 15U,
	.BaseBroadcastSwBlockRegOff = XAIE2GBL_PL_MODULE_EVENT_BROADCAST_A_BLOCK_SOUTH_SET,
	.BaseBroadcastSwUnblockRegOff = XAIE2GBL_PL_MODULE_EVENT_BROADCAST_A_BLOCK_SOUTH_CLR,
	.BroadcastSwOff = 64U,
	.BroadcastSwBlockOff = 16U,
	.BroadcastSwUnblockOff = 16U,
	.NumSwitches = 2U,
	.BaseGroupEventRegOff = XAIE2GBL_PL_MODULE_EVENT_GROUP_0_ENABLE,
	.NumGroupEvents = 6U,
	.DefaultGroupErrorMask = 0x7FFU,
	.Group = Aie2PlGroupEvent,
	.BasePCEventRegOff = XAIE_FEATURE_UNAVAILABLE,
	.NumPCEvents = XAIE_FEATURE_UNAVAILABLE,
	.PCAddr = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
	.PCValid = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
};

/*
 * Data structure to capture PL module events properties.
 * For PL module default error group mask enables,
 *	AXI_MM_Slave_Tile_Error,
 *	Control_Pkt_Error,
 *	Stream_Switch_Parity_Error,
 *	AXI_MM_Decode_NSU_Error,
 *	AXI_MM_Slave_NSU_Error,
 *	AXI_MM_Unsupported_Traffic,
 *	AXI_MM_Unsecure_Access_in_Secure_Mode,
 *	AXI_MM_Byte_Strobe_Error,
 *	DMA_S2MM_Error,
 *	DMA_MM2S_Error,
 *	Lock_Error.
 */
static const XAie_EvntMod Aie2PlEvntMod =
{
	.XAie_EventNumber = Aie2PlModEventMapping,
	.EventMin = XAIE_EVENT_NONE_PL,
	.EventMax = XAIE_EVENT_DMA_TASK_TOKEN_STALL_PL,
	.GenEventRegOff = XAIE2GBL_PL_MODULE_EVENT_GENERATE,
	.GenEvent = {XAIE2GBL_PL_MODULE_EVENT_GENERATE_EVENT_LSB, XAIE2GBL_PL_MODULE_EVENT_GENERATE_EVENT_MASK},
	.ComboInputRegOff = XAIE2GBL_PL_MODULE_COMBO_EVENT_INPUTS,
	.ComboEventMask = XAIE2GBL_PL_MODULE_COMBO_EVENT_INPUTS_EVENTA_MASK,
	.ComboEventOff = 8U,
	.ComboCtrlRegOff = XAIE2GBL_PL_MODULE_COMBO_EVENT_CONTROL,
	.ComboConfigMask = XAIE2GBL_PL_MODULE_COMBO_EVENT_CONTROL_COMBO0_MASK,
	.ComboConfigOff = 8U,
	.BaseStrmPortSelectRegOff = XAIE2GBL_PL_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_0,
	.NumStrmPortSelectIds = 8U,
	.StrmPortSelectIdsPerReg = 4U,
	.PortIdMask = XAIE2GBL_PL_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_0_PORT_0_ID_MASK,
	.PortIdOff = 8U,
	.PortMstrSlvMask = XAIE2GBL_PL_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_0_PORT_0_MASTER_SLAVE_MASK,
	.PortMstrSlvOff = 8U,
	.BaseBroadcastRegOff = XAIE2GBL_PL_MODULE_EVENT_BROADCAST0_A,
	.NumBroadcastIds = 15U,
	.BaseBroadcastSwBlockRegOff = XAIE2GBL_PL_MODULE_EVENT_BROADCAST_A_BLOCK_SOUTH_SET,
	.BaseBroadcastSwUnblockRegOff = XAIE2GBL_PL_MODULE_EVENT_BROADCAST_A_BLOCK_SOUTH_CLR,
	.BroadcastSwOff = 64U,
	.BroadcastSwBlockOff = 16U,
	.BroadcastSwUnblockOff = 16U,
	.NumSwitches = 2U,
	.BaseGroupEventRegOff = XAIE2GBL_PL_MODULE_EVENT_GROUP_0_ENABLE,
	.NumGroupEvents = 6U,
	.DefaultGroupErrorMask = 0x7FFU,
	.Group = Aie2PlGroupEvent,
	.BasePCEventRegOff = XAIE_FEATURE_UNAVAILABLE,
	.NumPCEvents = XAIE_FEATURE_UNAVAILABLE,
	.PCAddr = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
	.PCValid = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
};

/*
 * Data structure to capture mem tile module events properties.
 * For mem tile default error group mask enables,
 *	DM_ECC_Error_Scrub_2bit,
 *	DM_ECC_Error_2bit,
 *	DMA_S2MM_Error,
 *	DMA_MM2S_Error,
 *	Stream_Switch_Parity_Error,
 *	Stream_Pkt_Parity_Error,
 *	Control_Pkt_Error,
 *	AXI-MM_Slave_Error,
 *	Lock_Error.
 */
static const XAie_EvntMod Aie2MemTileEvntMod =
{
	.XAie_EventNumber = Aie2MemTileModEventMapping,
	.EventMin = XAIE_EVENT_NONE_MEM_TILE,
	.EventMax = XAIE_EVENT_USER_EVENT_1_MEM_TILE,
	.GenEventRegOff = XAIE2GBL_MEM_TILE_MODULE_EVENT_GENERATE,
	.GenEvent = {XAIE2GBL_MEM_TILE_MODULE_EVENT_GENERATE_EVENT_LSB, XAIE2GBL_MEM_TILE_MODULE_EVENT_GENERATE_EVENT_MASK},
	.ComboInputRegOff = XAIE2GBL_MEM_TILE_MODULE_COMBO_EVENT_INPUTS,
	.ComboEventMask = XAIE2GBL_MEM_TILE_MODULE_COMBO_EVENT_INPUTS_EVENTA_MASK,
	.ComboEventOff = 8U,
	.ComboCtrlRegOff = XAIE2GBL_MEM_TILE_MODULE_COMBO_EVENT_CONTROL,
	.ComboConfigMask = XAIE2GBL_MEM_TILE_MODULE_COMBO_EVENT_CONTROL_COMBO0_MASK,
	.ComboConfigOff = 8U,
	.BaseStrmPortSelectRegOff = XAIE2GBL_MEM_TILE_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_0,
	.NumStrmPortSelectIds = 8U,
	.StrmPortSelectIdsPerReg = 4U,
	.PortIdMask = XAIE2GBL_MEM_TILE_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_0_PORT_0_ID_MASK,
	.PortIdOff = 8U,
	.PortMstrSlvMask = XAIE2GBL_MEM_TILE_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_0_PORT_0_MASTER_SLAVE_MASK,
	.PortMstrSlvOff = 8U,
	.BaseBroadcastRegOff = XAIE2GBL_MEM_TILE_MODULE_EVENT_BROADCAST0,
	.NumBroadcastIds = 16U,
	.BaseBroadcastSwBlockRegOff = XAIE2GBL_MEM_TILE_MODULE_EVENT_BROADCAST_A_BLOCK_SOUTH_SET,
	.BaseBroadcastSwUnblockRegOff = XAIE2GBL_MEM_TILE_MODULE_EVENT_BROADCAST_A_BLOCK_SOUTH_CLR,
	.BroadcastSwOff = 64U,
	.BroadcastSwBlockOff = 16U,
	.BroadcastSwUnblockOff = 16U,
	.NumSwitches = 2U,
	.BaseGroupEventRegOff = XAIE2GBL_MEM_TILE_MODULE_EVENT_GROUP_0_ENABLE,
	.NumGroupEvents = 9U,
	.DefaultGroupErrorMask = 0x7FAU,
	.Group = Aie2MemTileGroupEvent,
	.BasePCEventRegOff = XAIE_FEATURE_UNAVAILABLE,
	.NumPCEvents = XAIE_FEATURE_UNAVAILABLE,
	.PCAddr = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
	.PCValid = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
};

static const XAie_TimerMod Aie2TileTimerMod[] =
{
	 {
		.TrigEventLowValOff = XAIE2GBL_MEMORY_MODULE_TIMER_TRIG_EVENT_LOW_VALUE,
		.TrigEventHighValOff = XAIE2GBL_MEMORY_MODULE_TIMER_TRIG_EVENT_HIGH_VALUE,
		.LowOff = XAIE2GBL_MEMORY_MODULE_TIMER_LOW,
		.HighOff = XAIE2GBL_MEMORY_MODULE_TIMER_HIGH,
		.CtrlOff = XAIE2GBL_MEMORY_MODULE_TIMER_CONTROL,
		{XAIE2GBL_MEMORY_MODULE_TIMER_CONTROL_RESET_LSB, XAIE2GBL_MEMORY_MODULE_TIMER_CONTROL_RESET_MASK},
		{XAIE2GBL_MEMORY_MODULE_TIMER_CONTROL_RESET_EVENT_LSB, XAIE2GBL_MEMORY_MODULE_TIMER_CONTROL_RESET_EVENT_MASK},
	},
	{
		.TrigEventLowValOff = XAIE2GBL_CORE_MODULE_TIMER_TRIG_EVENT_LOW_VALUE,
		.TrigEventHighValOff = XAIE2GBL_CORE_MODULE_TIMER_TRIG_EVENT_HIGH_VALUE,
		.LowOff = XAIE2GBL_CORE_MODULE_TIMER_LOW,
		.HighOff = XAIE2GBL_CORE_MODULE_TIMER_HIGH,
		.CtrlOff = XAIE2GBL_CORE_MODULE_TIMER_CONTROL,
		{XAIE2GBL_CORE_MODULE_TIMER_CONTROL_RESET_LSB, XAIE2GBL_CORE_MODULE_TIMER_CONTROL_RESET_MASK},
		{XAIE2GBL_CORE_MODULE_TIMER_CONTROL_RESET_EVENT_LSB, XAIE2GBL_CORE_MODULE_TIMER_CONTROL_RESET_EVENT_MASK},
	}
};

static const XAie_TimerMod Aie2PlTimerMod =
{
	.TrigEventLowValOff = XAIE2GBL_PL_MODULE_TIMER_TRIG_EVENT_LOW_VALUE,
	.TrigEventHighValOff = XAIE2GBL_PL_MODULE_TIMER_TRIG_EVENT_HIGH_VALUE,
	.LowOff = XAIE2GBL_PL_MODULE_TIMER_LOW,
	.HighOff = XAIE2GBL_PL_MODULE_TIMER_HIGH,
	.CtrlOff = XAIE2GBL_PL_MODULE_TIMER_CONTROL,
	{XAIE2GBL_PL_MODULE_TIMER_CONTROL_RESET_LSB, XAIE2GBL_PL_MODULE_TIMER_CONTROL_RESET_MASK},
	{XAIE2GBL_PL_MODULE_TIMER_CONTROL_RESET_EVENT_LSB, XAIE2GBL_PL_MODULE_TIMER_CONTROL_RESET_EVENT_MASK}
};

static const XAie_TimerMod Aie2MemTileTimerMod =
{
	.TrigEventLowValOff = XAIE2GBL_MEM_TILE_MODULE_TIMER_TRIG_EVENT_LOW_VALUE ,
	.TrigEventHighValOff = XAIE2GBL_MEM_TILE_MODULE_TIMER_TRIG_EVENT_HIGH_VALUE,
	.LowOff = XAIE2GBL_MEM_TILE_MODULE_TIMER_LOW,
	.HighOff = XAIE2GBL_MEM_TILE_MODULE_TIMER_HIGH,
	.CtrlOff = XAIE2GBL_MEM_TILE_MODULE_TIMER_CONTROL,
	{XAIE2GBL_MEM_TILE_MODULE_TIMER_CONTROL_RESET_LSB, XAIE2GBL_MEM_TILE_MODULE_TIMER_CONTROL_RESET_MASK},
	{XAIE2GBL_MEM_TILE_MODULE_TIMER_CONTROL_RESET_EVENT_LSB, XAIE2GBL_MEM_TILE_MODULE_TIMER_CONTROL_RESET_EVENT_MASK}
};

/*
 * Data structure to configure trace event register for XAIE_MEM_MOD module
 * type
 */
static const XAie_RegFldAttr Aie2MemTraceEvent[] =
{
	{XAIE2GBL_MEMORY_MODULE_TRACE_EVENT0_TRACE_EVENT0_LSB, XAIE2GBL_MEMORY_MODULE_TRACE_EVENT0_TRACE_EVENT0_MASK},
	{XAIE2GBL_MEMORY_MODULE_TRACE_EVENT0_TRACE_EVENT1_LSB, XAIE2GBL_MEMORY_MODULE_TRACE_EVENT0_TRACE_EVENT1_MASK},
	{XAIE2GBL_MEMORY_MODULE_TRACE_EVENT0_TRACE_EVENT2_LSB, XAIE2GBL_MEMORY_MODULE_TRACE_EVENT0_TRACE_EVENT2_MASK},
	{XAIE2GBL_MEMORY_MODULE_TRACE_EVENT0_TRACE_EVENT3_LSB, XAIE2GBL_MEMORY_MODULE_TRACE_EVENT0_TRACE_EVENT3_MASK},
	{XAIE2GBL_MEMORY_MODULE_TRACE_EVENT1_TRACE_EVENT4_LSB, XAIE2GBL_MEMORY_MODULE_TRACE_EVENT1_TRACE_EVENT4_MASK},
	{XAIE2GBL_MEMORY_MODULE_TRACE_EVENT1_TRACE_EVENT5_LSB, XAIE2GBL_MEMORY_MODULE_TRACE_EVENT1_TRACE_EVENT5_MASK},
	{XAIE2GBL_MEMORY_MODULE_TRACE_EVENT1_TRACE_EVENT6_LSB, XAIE2GBL_MEMORY_MODULE_TRACE_EVENT1_TRACE_EVENT6_MASK},
	{XAIE2GBL_MEMORY_MODULE_TRACE_EVENT1_TRACE_EVENT7_LSB, XAIE2GBL_MEMORY_MODULE_TRACE_EVENT1_TRACE_EVENT7_MASK}
};

/*
 * Data structure to configure trace event register for XAIE_CORE_MOD module
 * type
 */
static const XAie_RegFldAttr Aie2CoreTraceEvent[] =
{
	{XAIE2GBL_CORE_MODULE_TRACE_EVENT0_TRACE_EVENT0_LSB, XAIE2GBL_CORE_MODULE_TRACE_EVENT0_TRACE_EVENT0_MASK},
	{XAIE2GBL_CORE_MODULE_TRACE_EVENT0_TRACE_EVENT1_LSB, XAIE2GBL_CORE_MODULE_TRACE_EVENT0_TRACE_EVENT1_MASK},
	{XAIE2GBL_CORE_MODULE_TRACE_EVENT0_TRACE_EVENT2_LSB, XAIE2GBL_CORE_MODULE_TRACE_EVENT0_TRACE_EVENT2_MASK},
	{XAIE2GBL_CORE_MODULE_TRACE_EVENT0_TRACE_EVENT3_LSB, XAIE2GBL_CORE_MODULE_TRACE_EVENT0_TRACE_EVENT3_MASK},
	{XAIE2GBL_CORE_MODULE_TRACE_EVENT1_TRACE_EVENT4_LSB, XAIE2GBL_CORE_MODULE_TRACE_EVENT1_TRACE_EVENT4_MASK},
	{XAIE2GBL_CORE_MODULE_TRACE_EVENT1_TRACE_EVENT5_LSB, XAIE2GBL_CORE_MODULE_TRACE_EVENT1_TRACE_EVENT5_MASK},
	{XAIE2GBL_CORE_MODULE_TRACE_EVENT1_TRACE_EVENT6_LSB, XAIE2GBL_CORE_MODULE_TRACE_EVENT1_TRACE_EVENT6_MASK},
	{XAIE2GBL_CORE_MODULE_TRACE_EVENT1_TRACE_EVENT7_LSB, XAIE2GBL_CORE_MODULE_TRACE_EVENT1_TRACE_EVENT7_MASK}
};

/*
 * Data structure to configure trace event register for XAIE_PL_MOD module
 * type
 */
static const XAie_RegFldAttr Aie2PlTraceEvent[] =
{
	{XAIE2GBL_PL_MODULE_TRACE_EVENT0_TRACE_EVENT0_LSB, XAIE2GBL_PL_MODULE_TRACE_EVENT0_TRACE_EVENT0_MASK},
	{XAIE2GBL_PL_MODULE_TRACE_EVENT0_TRACE_EVENT1_LSB, XAIE2GBL_PL_MODULE_TRACE_EVENT0_TRACE_EVENT1_MASK},
	{XAIE2GBL_PL_MODULE_TRACE_EVENT0_TRACE_EVENT2_LSB, XAIE2GBL_PL_MODULE_TRACE_EVENT0_TRACE_EVENT2_MASK},
	{XAIE2GBL_PL_MODULE_TRACE_EVENT0_TRACE_EVENT3_LSB, XAIE2GBL_PL_MODULE_TRACE_EVENT0_TRACE_EVENT3_MASK},
	{XAIE2GBL_PL_MODULE_TRACE_EVENT1_TRACE_EVENT4_LSB, XAIE2GBL_PL_MODULE_TRACE_EVENT1_TRACE_EVENT4_MASK},
	{XAIE2GBL_PL_MODULE_TRACE_EVENT1_TRACE_EVENT5_LSB, XAIE2GBL_PL_MODULE_TRACE_EVENT1_TRACE_EVENT5_MASK},
	{XAIE2GBL_PL_MODULE_TRACE_EVENT1_TRACE_EVENT6_LSB, XAIE2GBL_PL_MODULE_TRACE_EVENT1_TRACE_EVENT6_MASK},
	{XAIE2GBL_PL_MODULE_TRACE_EVENT1_TRACE_EVENT7_LSB, XAIE2GBL_PL_MODULE_TRACE_EVENT1_TRACE_EVENT7_MASK}
};

/*
 * Data structure to configure trace event register for
 * XAIEGBL_TILE_TYPE_MEMTILE type
 */
static const XAie_RegFldAttr Aie2MemTileTraceEvent[] =
{
	{XAIE2GBL_MEM_TILE_MODULE_TRACE_EVENT0_TRACE_EVENT0_LSB, XAIE2GBL_MEM_TILE_MODULE_TRACE_EVENT0_TRACE_EVENT0_MASK},
	{XAIE2GBL_MEM_TILE_MODULE_TRACE_EVENT0_TRACE_EVENT1_LSB, XAIE2GBL_MEM_TILE_MODULE_TRACE_EVENT0_TRACE_EVENT1_MASK},
	{XAIE2GBL_MEM_TILE_MODULE_TRACE_EVENT0_TRACE_EVENT2_LSB, XAIE2GBL_MEM_TILE_MODULE_TRACE_EVENT0_TRACE_EVENT2_MASK},
	{XAIE2GBL_MEM_TILE_MODULE_TRACE_EVENT0_TRACE_EVENT3_LSB, XAIE2GBL_MEM_TILE_MODULE_TRACE_EVENT0_TRACE_EVENT3_MASK},
	{XAIE2GBL_MEM_TILE_MODULE_TRACE_EVENT1_TRACE_EVENT4_LSB, XAIE2GBL_MEM_TILE_MODULE_TRACE_EVENT1_TRACE_EVENT4_MASK},
	{XAIE2GBL_MEM_TILE_MODULE_TRACE_EVENT1_TRACE_EVENT5_LSB, XAIE2GBL_MEM_TILE_MODULE_TRACE_EVENT1_TRACE_EVENT5_MASK},
	{XAIE2GBL_MEM_TILE_MODULE_TRACE_EVENT1_TRACE_EVENT6_LSB, XAIE2GBL_MEM_TILE_MODULE_TRACE_EVENT1_TRACE_EVENT6_MASK},
	{XAIE2GBL_MEM_TILE_MODULE_TRACE_EVENT1_TRACE_EVENT7_LSB, XAIE2GBL_MEM_TILE_MODULE_TRACE_EVENT1_TRACE_EVENT7_MASK}
};

/*
 * Data structure to configure trace module for XAIEGBL_TILE_TYPE_AIETILE tile
 * type
 */
static const XAie_TraceMod Aie2TileTraceMod[] =
{
	{
		.CtrlRegOff = XAIE2GBL_MEMORY_MODULE_TRACE_CONTROL0,
		.PktConfigRegOff = XAIE2GBL_MEMORY_MODULE_TRACE_CONTROL1,
		.StatusRegOff = XAIE2GBL_MEMORY_MODULE_TRACE_STATUS,
		.EventRegOffs = (u32 []){XAIE2GBL_MEMORY_MODULE_TRACE_EVENT0, XAIE2GBL_MEMORY_MODULE_TRACE_EVENT1},
		.NumTraceSlotIds = 8U,
		.NumEventsPerSlot = 4U,
		.StopEvent = {XAIE2GBL_MEMORY_MODULE_TRACE_CONTROL0_TRACE_STOP_EVENT_LSB, XAIE2GBL_MEMORY_MODULE_TRACE_CONTROL0_TRACE_STOP_EVENT_MASK},
		.StartEvent = {XAIE2GBL_MEMORY_MODULE_TRACE_CONTROL0_TRACE_START_EVENT_LSB, XAIE2GBL_MEMORY_MODULE_TRACE_CONTROL0_TRACE_START_EVENT_MASK},
		.ModeConfig = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
		.PktType = {XAIE2GBL_MEMORY_MODULE_TRACE_CONTROL1_PACKET_TYPE_LSB, XAIE2GBL_MEMORY_MODULE_TRACE_CONTROL1_PACKET_TYPE_MASK},
		.PktId = {XAIE2GBL_MEMORY_MODULE_TRACE_CONTROL1_ID_LSB, XAIE2GBL_MEMORY_MODULE_TRACE_CONTROL1_ID_MASK},
		.State = {XAIE2GBL_MEMORY_MODULE_TRACE_STATUS_STATE_LSB, XAIE2GBL_MEMORY_MODULE_TRACE_STATUS_STATE_MASK},
		.ModeSts = {XAIE2GBL_MEMORY_MODULE_TRACE_STATUS_MODE_LSB, XAIE2GBL_MEMORY_MODULE_TRACE_STATUS_MODE_MASK},
		.Event = Aie2MemTraceEvent
	},
	{
		.CtrlRegOff = XAIE2GBL_CORE_MODULE_TRACE_CONTROL0,
		.PktConfigRegOff = XAIE2GBL_CORE_MODULE_TRACE_CONTROL1,
		.StatusRegOff = XAIE2GBL_CORE_MODULE_TRACE_STATUS,
		.EventRegOffs = (u32 []){XAIE2GBL_CORE_MODULE_TRACE_EVENT0, XAIE2GBL_CORE_MODULE_TRACE_EVENT1},
		.NumTraceSlotIds = 8U,
		.NumEventsPerSlot = 4U,
		.StopEvent = {XAIE2GBL_CORE_MODULE_TRACE_CONTROL0_TRACE_STOP_EVENT_LSB, XAIE2GBL_CORE_MODULE_TRACE_CONTROL0_TRACE_STOP_EVENT_MASK},
		.StartEvent = {XAIE2GBL_CORE_MODULE_TRACE_CONTROL0_TRACE_START_EVENT_LSB, XAIE2GBL_CORE_MODULE_TRACE_CONTROL0_TRACE_START_EVENT_MASK},
		.ModeConfig = {XAIE2GBL_CORE_MODULE_TRACE_CONTROL0_MODE_LSB, XAIE2GBL_CORE_MODULE_TRACE_CONTROL0_MODE_MASK},
		.PktType = {XAIE2GBL_CORE_MODULE_TRACE_CONTROL1_PACKET_TYPE_LSB, XAIE2GBL_CORE_MODULE_TRACE_CONTROL1_PACKET_TYPE_MASK},
		.PktId = {XAIE2GBL_CORE_MODULE_TRACE_CONTROL1_ID_LSB, XAIE2GBL_CORE_MODULE_TRACE_CONTROL1_ID_MASK},
		.State = {XAIE2GBL_CORE_MODULE_TRACE_STATUS_STATE_LSB, XAIE2GBL_CORE_MODULE_TRACE_STATUS_STATE_MASK},
		.ModeSts = {XAIE2GBL_CORE_MODULE_TRACE_STATUS_MODE_LSB, XAIE2GBL_CORE_MODULE_TRACE_STATUS_MODE_MASK},
		.Event = Aie2CoreTraceEvent
	}
};

/*
 * Data structure to configure trace module for XAIEGBL_TILE_TYPE_SHIMNOC/PL
 * tile type
 */
static const XAie_TraceMod Aie2PlTraceMod =
{
	.CtrlRegOff = XAIE2GBL_PL_MODULE_TRACE_CONTROL0,
	.PktConfigRegOff = XAIE2GBL_PL_MODULE_TRACE_CONTROL1,
	.StatusRegOff = XAIE2GBL_PL_MODULE_TRACE_STATUS,
	.EventRegOffs = (u32 []){XAIE2GBL_PL_MODULE_TRACE_EVENT0, XAIE2GBL_PL_MODULE_TRACE_EVENT1},
	.NumTraceSlotIds = 8U,
	.NumEventsPerSlot = 4U,
	.StopEvent = {XAIE2GBL_PL_MODULE_TRACE_CONTROL0_TRACE_STOP_EVENT_LSB, XAIE2GBL_PL_MODULE_TRACE_CONTROL0_TRACE_STOP_EVENT_MASK},
	.StartEvent = {XAIE2GBL_PL_MODULE_TRACE_CONTROL0_TRACE_START_EVENT_LSB, XAIE2GBL_PL_MODULE_TRACE_CONTROL0_TRACE_START_EVENT_MASK},
	.ModeConfig = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
	.PktType = {XAIE2GBL_PL_MODULE_TRACE_CONTROL1_PACKET_TYPE_LSB, XAIE2GBL_PL_MODULE_TRACE_CONTROL1_PACKET_TYPE_MASK},
	.PktId = {XAIE2GBL_PL_MODULE_TRACE_CONTROL1_ID_LSB, XAIE2GBL_PL_MODULE_TRACE_CONTROL1_ID_MASK},
	.State = {XAIE2GBL_PL_MODULE_TRACE_STATUS_STATE_LSB, XAIE2GBL_PL_MODULE_TRACE_STATUS_STATE_MASK},
	.ModeSts = {XAIE2GBL_PL_MODULE_TRACE_STATUS_MODE_LSB, XAIE2GBL_PL_MODULE_TRACE_STATUS_MODE_MASK},
	.Event = Aie2PlTraceEvent
};

/*
 * Data structure to configure trace module for XAIEGBL_TILE_TYPE_MEMTILE
 * tile type
 */
static const XAie_TraceMod Aie2MemTileTraceMod =
{
	.CtrlRegOff = XAIE2GBL_MEM_TILE_MODULE_TRACE_CONTROL0,
	.PktConfigRegOff = XAIE2GBL_MEM_TILE_MODULE_TRACE_CONTROL1,
	.StatusRegOff = XAIE2GBL_MEM_TILE_MODULE_TRACE_STATUS,
	.EventRegOffs = (u32 []){XAIE2GBL_MEM_TILE_MODULE_TRACE_EVENT0, XAIE2GBL_MEM_TILE_MODULE_TRACE_EVENT1},
	.NumTraceSlotIds = 8U,
	.NumEventsPerSlot = 4U,
	.StopEvent = {XAIE2GBL_MEM_TILE_MODULE_TRACE_CONTROL0_TRACE_STOP_EVENT_LSB, XAIE2GBL_MEM_TILE_MODULE_TRACE_CONTROL0_TRACE_STOP_EVENT_MASK},
	.StartEvent = {XAIE2GBL_MEM_TILE_MODULE_TRACE_CONTROL0_TRACE_START_EVENT_LSB, XAIE2GBL_MEM_TILE_MODULE_TRACE_CONTROL0_TRACE_START_EVENT_MASK},
	.ModeConfig = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
	.PktType = {XAIE2GBL_MEM_TILE_MODULE_TRACE_CONTROL1_PACKET_TYPE_LSB, XAIE2GBL_MEM_TILE_MODULE_TRACE_CONTROL1_PACKET_TYPE_MASK},
	.PktId = {XAIE2GBL_MEM_TILE_MODULE_TRACE_CONTROL1_ID_LSB, XAIE2GBL_MEM_TILE_MODULE_TRACE_CONTROL1_ID_MASK},
	.State = {XAIE2GBL_MEM_TILE_MODULE_TRACE_STATUS_STATE_LSB, XAIE2GBL_MEM_TILE_MODULE_TRACE_STATUS_STATE_MASK},
	.ModeSts = {XAIE2GBL_MEM_TILE_MODULE_TRACE_STATUS_MODE_LSB, XAIE2GBL_MEM_TILE_MODULE_TRACE_STATUS_MODE_MASK},
	.Event = Aie2MemTileTraceEvent
};

/*
 * Data structure to configures first level interrupt controller for
 * XAIEGBL_TILE_TYPE_SHIMPL tile type
 */
static const XAie_L1IntrMod Aie2PlL1IntrMod =
{
	.BaseEnableRegOff = XAIE2GBL_PL_MODULE_INTERRUPT_CONTROLLER_1ST_LEVEL_ENABLE_A,
	.BaseDisableRegOff = XAIE2GBL_PL_MODULE_INTERRUPT_CONTROLLER_1ST_LEVEL_DISABLE_A,
	.BaseIrqRegOff = XAIE2GBL_PL_MODULE_INTERRUPT_CONTROLLER_1ST_LEVEL_IRQ_NO_A,
	.BaseIrqEventRegOff = XAIE2GBL_PL_MODULE_INTERRUPT_CONTROLLER_1ST_LEVEL_IRQ_EVENT_A,
	.BaseIrqEventMask = XAIE2GBL_PL_MODULE_INTERRUPT_CONTROLLER_1ST_LEVEL_IRQ_EVENT_A_IRQ_EVENT0_MASK,
	.BaseBroadcastBlockRegOff = XAIE2GBL_PL_MODULE_INTERRUPT_CONTROLLER_1ST_LEVEL_BLOCK_NORTH_IN_A_SET,
	.BaseBroadcastUnblockRegOff = XAIE2GBL_PL_MODULE_INTERRUPT_CONTROLLER_1ST_LEVEL_BLOCK_NORTH_IN_A_CLEAR,
	.SwOff = 0x30U,
	.NumIntrIds = 20U,
	.NumIrqEvents = 4U,
	.IrqEventOff = 8U,
	.NumBroadcastIds = 16U,
};

/*
 * Data structure to configures second level interrupt controller for
 * XAIEGBL_TILE_TYPE_SHIMNOC tile type
 */
static const XAie_L2IntrMod Aie2NoCL2IntrMod =
{
	.EnableRegOff = XAIE2GBL_NOC_MODULE_INTERRUPT_CONTROLLER_2ND_LEVEL_ENABLE,
	.DisableRegOff = XAIE2GBL_NOC_MODULE_INTERRUPT_CONTROLLER_2ND_LEVEL_DISABLE,
	.IrqRegOff = XAIE2GBL_NOC_MODULE_INTERRUPT_CONTROLLER_2ND_LEVEL_INTERRUPT,
	.NumBroadcastIds = 16U,
	.NumNoCIntr = 4U,
};

/*
 * AIE2 Module
 * This data structure captures all the modules for each tile type.
 * Depending on the tile type, this data strcuture can be used to access all
 * hardware properties of individual modules.
 */
XAie_TileMod Aie2Mod[] =
{
	{
		/*
		 * AIE2 Tile Module indexed using XAIEGBL_TILE_TYPE_AIETILE
		 */
		.CoreMod = &Aie2CoreMod,
		.StrmSw  = &Aie2TileStrmSw,
		.DmaMod  = &Aie2TileDmaMod,
		.MemMod  = &Aie2TileMemMod,
		.PlIfMod = NULL,
		.LockMod = &Aie2TileLockMod,
		.PerfMod = Aie2TilePerfCnt,
		.EvntMod = Aie2TileEvntMod,
		.TimerMod = Aie2TileTimerMod,
		.TraceMod = Aie2TileTraceMod,
		.L1IntrMod = NULL,
		.L2IntrMod = NULL,
	},
	{
		/*
		 * AIE2 Shim Noc Module indexed using XAIEGBL_TILE_TYPE_SHIMNOC
		 */
		.CoreMod = NULL,
		.StrmSw  = &Aie2ShimStrmSw,
		.DmaMod  = &Aie2ShimDmaMod,
		.MemMod  = NULL,
		.PlIfMod = &Aie2ShimTilePlIfMod,
		.LockMod = &Aie2ShimNocLockMod,
		.PerfMod = &Aie2PlPerfCnt,
		.EvntMod = &Aie2NocEvntMod,
		.TimerMod = &Aie2PlTimerMod,
		.TraceMod = &Aie2PlTraceMod,
		.L1IntrMod = &Aie2PlL1IntrMod,
		.L2IntrMod = &Aie2NoCL2IntrMod,
	},
	{
		/*
		 * AIE2 Shim PL Module indexed using XAIEGBL_TILE_TYPE_SHIMPL
		 */
		.CoreMod = NULL,
		.StrmSw  = &Aie2ShimStrmSw,
		.DmaMod  = NULL,
		.MemMod  = NULL,
		.PlIfMod = &Aie2PlIfMod,
		.LockMod = NULL,
		.PerfMod = &Aie2PlPerfCnt,
		.EvntMod = &Aie2PlEvntMod,
		.TimerMod = &Aie2PlTimerMod,
		.TraceMod = &Aie2PlTraceMod,
		.L1IntrMod = &Aie2PlL1IntrMod,
		.L2IntrMod = NULL,
	},
	{
		/*
		 * AIE2 MemTile Module indexed using XAIEGBL_TILE_TYPE_MEMTILE
		 */
		.CoreMod = NULL,
		.StrmSw  = &Aie2MemTileStrmSw,
		.DmaMod  = &Aie2MemTileDmaMod,
		.MemMod  = &Aie2MemTileMemMod,
		.PlIfMod = NULL,
		.LockMod = &Aie2MemTileLockMod,
		.PerfMod = &Aie2MemTilePerfCnt,
		.EvntMod = &Aie2MemTileEvntMod,
		.TimerMod = &Aie2MemTileTimerMod,
		.TraceMod = &Aie2MemTileTraceMod,
		.L1IntrMod = NULL,
		.L2IntrMod = NULL,
	}
};

/** @} */
