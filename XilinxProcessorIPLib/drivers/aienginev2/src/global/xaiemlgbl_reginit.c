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
#include "xaie_core_aieml.h"
#include "xaie_dma_aieml.h"
#include "xaie_events.h"
#include "xaie_events_aieml.h"
#include "xaie_locks_aieml.h"
#include "xaie_reset_aieml.h"
#include "xaie_ss_aieml.h"
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
	XAIEMLGBL_CORE_MODULE_CORE_CONTROL,
	{XAIEMLGBL_CORE_MODULE_CORE_CONTROL_ENABLE_LSB, XAIEMLGBL_CORE_MODULE_CORE_CONTROL_ENABLE_MASK},
	{XAIEMLGBL_CORE_MODULE_CORE_CONTROL_RESET_LSB, XAIEMLGBL_CORE_MODULE_CORE_CONTROL_RESET_MASK}
};

/*
 * Global instance for Core module Core_Status register.
 */
static const  XAie_RegCoreSts Aie2CoreStsReg =
{
	XAIEMLGBL_CORE_MODULE_CORE_STATUS,
	{XAIEMLGBL_CORE_MODULE_CORE_STATUS_CORE_DONE_LSB, XAIEMLGBL_CORE_MODULE_CORE_STATUS_CORE_DONE_MASK},
	{XAIEMLGBL_CORE_MODULE_CORE_STATUS_RESET_LSB, XAIEMLGBL_CORE_MODULE_CORE_STATUS_RESET_MASK},
	{XAIEMLGBL_CORE_MODULE_CORE_STATUS_ENABLE_LSB, XAIEMLGBL_CORE_MODULE_CORE_STATUS_ENABLE_MASK}
};

/*
 * Global instance for Core module for core debug registers.
 */
static const XAie_RegCoreDebug Aie2CoreDebugReg =
{
	.RegOff = XAIEMLGBL_CORE_MODULE_DEBUG_CONTROL0,
	.DebugCtrl1Offset = XAIEMLGBL_CORE_MODULE_DEBUG_CONTROL1,
	.DebugHaltCoreEvent1.Lsb = XAIEMLGBL_CORE_MODULE_DEBUG_CONTROL1_DEBUG_HALT_CORE_EVENT1_LSB,
	.DebugHaltCoreEvent1.Mask = XAIEMLGBL_CORE_MODULE_DEBUG_CONTROL1_DEBUG_HALT_CORE_EVENT1_MASK,
	.DebugHaltCoreEvent0.Lsb = XAIEMLGBL_CORE_MODULE_DEBUG_CONTROL1_DEBUG_HALT_CORE_EVENT0_LSB,
	.DebugHaltCoreEvent0.Mask = XAIEMLGBL_CORE_MODULE_DEBUG_CONTROL1_DEBUG_HALT_CORE_EVENT0_MASK,
	.DebugSStepCoreEvent.Lsb = XAIEMLGBL_CORE_MODULE_DEBUG_CONTROL1_DEBUG_SINGLESTEP_CORE_EVENT_LSB,
	.DebugSStepCoreEvent.Mask = XAIEMLGBL_CORE_MODULE_DEBUG_CONTROL1_DEBUG_SINGLESTEP_CORE_EVENT_MASK,
	.DebugResumeCoreEvent.Lsb = XAIEMLGBL_CORE_MODULE_DEBUG_CONTROL1_DEBUG_RESUME_CORE_EVENT_LSB,
	.DebugResumeCoreEvent.Mask = XAIEMLGBL_CORE_MODULE_DEBUG_CONTROL1_DEBUG_RESUME_CORE_EVENT_MASK,
	.DebugHalt.Lsb = XAIEMLGBL_CORE_MODULE_DEBUG_CONTROL0_DEBUG_HALT_BIT_LSB,
	.DebugHalt.Mask = XAIEMLGBL_CORE_MODULE_DEBUG_CONTROL0_DEBUG_HALT_BIT_MASK
};

/*
 * Global instance for core event registers in the core module.
 */
static const XAie_RegCoreEvents Aie2CoreEventReg =
{
	.EnableEventOff = XAIEMLGBL_CORE_MODULE_ENABLE_EVENTS,
	.DisableEventOccurred.Lsb = XAIEMLGBL_CORE_MODULE_ENABLE_EVENTS_DISABLE_EVENT_OCCURRED_LSB,
	.DisableEventOccurred.Mask = XAIEMLGBL_CORE_MODULE_ENABLE_EVENTS_DISABLE_EVENT_OCCURRED_MASK,
	.DisableEvent.Lsb = XAIEMLGBL_CORE_MODULE_ENABLE_EVENTS_DISABLE_EVENT_LSB,
	.DisableEvent.Mask = XAIEMLGBL_CORE_MODULE_ENABLE_EVENTS_DISABLE_EVENT_MASK,
};

/*
 * Global instance for core accumulator control register.
 */
static const XAie_RegCoreAccumCtrl AieMlCoreAccumCtrlReg =
{
	.RegOff = XAIEMLGBL_CORE_MODULE_ACCUMULATOR_CONTROL,
	.CascadeInput.Lsb = XAIEMLGBL_CORE_MODULE_ACCUMULATOR_CONTROL_INPUT_LSB,
	.CascadeInput.Mask = XAIEMLGBL_CORE_MODULE_ACCUMULATOR_CONTROL_INPUT_MASK,
	.CascadeOutput.Lsb = XAIEMLGBL_CORE_MODULE_ACCUMULATOR_CONTROL_OUTPUT_LSB,
	.CascadeOutput.Mask = XAIEMLGBL_CORE_MODULE_ACCUMULATOR_CONTROL_OUTPUT_MASK,
};

static const  XAie_DmaBdEnProp Aie2MemTileDmaBdEnProp =
{
	.NxtBd.Idx = 1U,
	.NxtBd.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_1_NEXT_BD_LSB,
	.NxtBd.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_1_NEXT_BD_MASK,
	.UseNxtBd.Idx = 1U,
	.UseNxtBd.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_1_USE_NEXT_BD_LSB,
	.UseNxtBd.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_1_USE_NEXT_BD_MASK,
	.ValidBd.Idx = 7U,
	.ValidBd.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_7_VALID_BD_LSB,
	.ValidBd.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_7_VALID_BD_MASK,
	.OutofOrderBdId.Idx = 0U,
	.OutofOrderBdId.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_0_OUT_OF_ORDER_BD_ID_LSB,
	.OutofOrderBdId.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_0_OUT_OF_ORDER_BD_ID_MASK,
	.TlastSuppress.Idx = 2U,
	.TlastSuppress.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_2_TLAST_SUPPRESS_LSB,
	.TlastSuppress.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_2_TLAST_SUPPRESS_MASK,
};

static const  XAie_DmaBdPkt Aie2MemTileDmaBdPktProp =
{
	.EnPkt.Idx = 0U,
	.EnPkt.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_0_ENABLE_PACKET_LSB,
	.EnPkt.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_0_ENABLE_PACKET_MASK,
	.PktId.Idx = 0U,
	.PktId.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_0_PACKET_ID_LSB,
	.PktId.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_0_PACKET_ID_MASK,
	.PktType.Idx = 0U,
	.PktType.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_0_PACKET_TYPE_LSB,
	.PktType.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_0_PACKET_TYPE_MASK,
};

static const  XAie_DmaBdLock Aie2MemTileDmaLockProp =
{
	.Aie2DmaLock.LckRelVal.Idx = 7U,
	.Aie2DmaLock.LckRelVal.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_REL_VALUE_LSB,
	.Aie2DmaLock.LckRelVal.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_REL_VALUE_MASK,
	.Aie2DmaLock.LckRelId.Idx = 7U,
	.Aie2DmaLock.LckRelId.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_REL_ID_LSB,
	.Aie2DmaLock.LckRelId.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_REL_ID_MASK,
	.Aie2DmaLock.LckAcqEn.Idx = 7U,
	.Aie2DmaLock.LckAcqEn.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_ACQ_ENABLE_LSB,
	.Aie2DmaLock.LckAcqEn.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_ACQ_ENABLE_MASK,
	.Aie2DmaLock.LckAcqVal.Idx = 7U,
	.Aie2DmaLock.LckAcqVal.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_ACQ_VALUE_LSB,
	.Aie2DmaLock.LckAcqVal.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_ACQ_VALUE_MASK,
	.Aie2DmaLock.LckAcqId.Idx = 7U,
	.Aie2DmaLock.LckAcqId.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_ACQ_ID_LSB,
	.Aie2DmaLock.LckAcqId.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_ACQ_ID_MASK,
};

static const  XAie_DmaBdBuffer Aie2MemTileBufferProp =
{
	.TileDmaBuff.BaseAddr.Idx = 1U,
	.TileDmaBuff.BaseAddr.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_1_BASE_ADDRESS_LSB,
	.TileDmaBuff.BaseAddr.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_1_BASE_ADDRESS_MASK,
	.TileDmaBuff.BufferLen.Idx = 0U,
	.TileDmaBuff.BufferLen.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_0_BUFFER_LENGTH_LSB,
	.TileDmaBuff.BufferLen.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_0_BUFFER_LENGTH_MASK,
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
	.Aie2MultiDimAddr.DmaDimProp[1U].StepSize.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_3_D1_STEPSIZE_LSB,
	.Aie2MultiDimAddr.DmaDimProp[1U].StepSize.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_3_D1_STEPSIZE_MASK,
	.Aie2MultiDimAddr.DmaDimProp[0U].StepSize.Idx = 2U,
	.Aie2MultiDimAddr.DmaDimProp[0U].StepSize.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_2_D0_STEPSIZE_LSB,
	.Aie2MultiDimAddr.DmaDimProp[0U].StepSize.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_2_D0_STEPSIZE_MASK,
	.Aie2MultiDimAddr.DmaDimProp[1U].Wrap.Idx = 3U,
	.Aie2MultiDimAddr.DmaDimProp[1U].Wrap.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_3_D1_WRAP_LSB,
	.Aie2MultiDimAddr.DmaDimProp[1U].Wrap.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_3_D1_WRAP_MASK,
	.Aie2MultiDimAddr.DmaDimProp[0U].Wrap.Idx = 2U,
	.Aie2MultiDimAddr.DmaDimProp[0U].Wrap.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_2_D0_WRAP_LSB,
	.Aie2MultiDimAddr.DmaDimProp[0U].Wrap.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_2_D0_WRAP_MASK,
	.Aie2MultiDimAddr.DmaDimProp[2U].StepSize.Idx = 4U,
	.Aie2MultiDimAddr.DmaDimProp[2U].StepSize.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_4_D2_STEPSIZE_LSB,
	.Aie2MultiDimAddr.DmaDimProp[2U].StepSize.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_4_D2_STEPSIZE_MASK,
	.Aie2MultiDimAddr.DmaDimProp[2U].Wrap.Idx = 4U,
	.Aie2MultiDimAddr.DmaDimProp[2U].Wrap.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_4_D2_WRAP_LSB,
	.Aie2MultiDimAddr.DmaDimProp[2U].Wrap.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_4_D2_WRAP_MASK,
	.Aie2MultiDimAddr.IterCurr.Idx = 6U,
	.Aie2MultiDimAddr.IterCurr.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_6_ITERATION_CURRENT_LSB,
	.Aie2MultiDimAddr.IterCurr.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_6_ITERATION_CURRENT_MASK,
	.Aie2MultiDimAddr.Iter.Wrap.Idx = 6U,
	.Aie2MultiDimAddr.Iter.Wrap.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_6_ITERATION_WRAP_LSB,
	.Aie2MultiDimAddr.Iter.Wrap.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_6_ITERATION_WRAP_MASK,
	.Aie2MultiDimAddr.Iter.StepSize.Idx = 6U,
	.Aie2MultiDimAddr.Iter.StepSize.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_6_ITERATION_STEPSIZE_LSB,
	.Aie2MultiDimAddr.Iter.StepSize.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_6_ITERATION_STEPSIZE_MASK,
	.Aie2MultiDimAddr.DmaDimProp[3U].StepSize.Idx = 5U,
	.Aie2MultiDimAddr.DmaDimProp[3U].StepSize.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_5_D3_STEPSIZE_LSB,
	.Aie2MultiDimAddr.DmaDimProp[3U].StepSize.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_5_D3_STEPSIZE_MASK,
};

static const  XAie_DmaBdZeroPad Aie2MemTileZeroPadProp =
{
	.D0_ZeroBefore.Idx = 1U,
	.D0_ZeroBefore.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_1_D0_ZERO_BEFORE_LSB,
	.D0_ZeroBefore.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_1_D0_ZERO_BEFORE_MASK,
	.D1_ZeroBefore.Idx = 3U,
	.D1_ZeroBefore.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_3_D1_ZERO_BEFORE_LSB,
	.D1_ZeroBefore.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_3_D1_ZERO_BEFORE_MASK,
	.D2_ZeroBefore.Idx = 4U,
	.D2_ZeroBefore.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_4_D2_ZERO_BEFORE_LSB,
	.D2_ZeroBefore.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_4_D2_ZERO_BEFORE_MASK,
	.D0_ZeroAfter.Idx = 5U,
	.D0_ZeroAfter.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_5_D0_ZERO_AFTER_LSB,
	.D0_ZeroAfter.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_5_D0_ZERO_AFTER_MASK,
	.D1_ZeroAfter.Idx = 5U,
	.D1_ZeroAfter.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_5_D1_ZERO_AFTER_LSB,
	.D1_ZeroAfter.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_5_D1_ZERO_AFTER_MASK,
	.D2_ZeroAfter.Idx = 5U,
	.D2_ZeroAfter.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_5_D2_ZERO_AFTER_LSB,
	.D2_ZeroAfter.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_5_D2_ZERO_AFTER_MASK,
};

static const  XAie_DmaBdCompression Aie2MemTileCompressionProp =
{
	.EnCompression.Idx = 4U,
	.EnCompression.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_4_ENABLE_COMPRESSION_LSB,
	.EnCompression.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_4_ENABLE_COMPRESSION_MASK,
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

static const XAie_DmaChStatus Aie2MemTileDmaChStatus =
{
	/* This database is common for mm2s and s2mm channels */
	.Aie2DmaChStatus.Status.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STATUS_LSB,
	.Aie2DmaChStatus.Status.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STATUS_MASK,
	.Aie2DmaChStatus.TaskQSize.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_TASK_QUEUE_SIZE_LSB,
	.Aie2DmaChStatus.TaskQSize.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_TASK_QUEUE_SIZE_MASK,
	.Aie2DmaChStatus.StalledLockAcq.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_ACQ_LSB,
	.Aie2DmaChStatus.StalledLockAcq.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_ACQ_MASK,
	.Aie2DmaChStatus.StalledLockRel.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_REL_LSB,
	.Aie2DmaChStatus.StalledLockRel.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_REL_MASK,
	.Aie2DmaChStatus.StalledStreamStarve.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STALLED_STREAM_STARVATION_LSB,
	.Aie2DmaChStatus.StalledStreamStarve.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STALLED_STREAM_STARVATION_MASK,
	.Aie2DmaChStatus.StalledTCT.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STALLED_TCT_OR_COUNT_FIFO_FULL_LSB,
	.Aie2DmaChStatus.StalledTCT.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STALLED_TCT_OR_COUNT_FIFO_FULL_MASK,
};

static const  XAie_DmaChProp Aie2MemTileDmaChProp =
{
	.HasFoTMode = XAIE_FEATURE_AVAILABLE,
	.HasControllerId = XAIE_FEATURE_AVAILABLE,
	.HasEnCompression = XAIE_FEATURE_AVAILABLE,
	.HasEnOutOfOrder = XAIE_FEATURE_AVAILABLE,
	.MaxFoTMode = DMA_FoT_COUNTS_FROM_MM_REG,
	.MaxRepeatCount = 256U,
	.ControllerId.Idx = 0,
	.ControllerId.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_CONTROLLER_ID_LSB,
	.ControllerId.Mask =XAIEMLGBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_CONTROLLER_ID_MASK ,
	.EnCompression.Idx = 0,
	.EnCompression.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_DECOMPRESSION_ENABLE_LSB,
	.EnCompression.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_DECOMPRESSION_ENABLE_MASK,
	.EnOutofOrder.Idx = 0,
	.EnOutofOrder.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_ENABLE_OUT_OF_ORDER_LSB,
	.EnOutofOrder.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_ENABLE_OUT_OF_ORDER_MASK,
	.FoTMode.Idx = 0,
	.FoTMode.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_FOT_MODE_LSB,
	.FoTMode.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_FOT_MODE_MASK ,
	.Reset.Idx = 0,
	.Reset.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_RESET_LSB,
	.Reset.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_RESET_MASK,
	.EnToken.Idx = 1,
	.EnToken.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_S2MM_0_START_QUEUE_ENABLE_TOKEN_ISSUE_LSB,
	.EnToken.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_S2MM_0_START_QUEUE_ENABLE_TOKEN_ISSUE_MASK,
	.RptCount.Idx = 1,
	.RptCount.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_S2MM_0_START_QUEUE_REPEAT_COUNT_LSB,
	.RptCount.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_S2MM_0_START_QUEUE_REPEAT_COUNT_MASK,
	.StartBd.Idx = 1,
	.StartBd.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_S2MM_0_START_QUEUE_START_BD_ID_LSB,
	.StartBd.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_S2MM_0_START_QUEUE_START_BD_ID_MASK,
	.PauseStream = {0U},
	.PauseMem = {0U},
	.Enable = {0U},
	.StartQSizeMax = 4U,
	.DmaChStatus = &Aie2MemTileDmaChStatus,
};

/* Mem Tile Dma Module */
static const  XAie_DmaMod Aie2MemTileDmaMod =
{
	.BaseAddr = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_0,
	.IdxOffset = 0x20,  /* This is the offset between each BD */
	.NumBds = 48,	   /* Number of BDs for AIE2 Tile DMA */
	.NumLocks = 192U,
	.NumAddrDim = 4U,
	.DoubleBuffering = XAIE_FEATURE_UNAVAILABLE,
	.Compression = XAIE_FEATURE_AVAILABLE,
	.ZeroPadding = XAIE_FEATURE_AVAILABLE,
	.OutofOrderBdId = XAIE_FEATURE_AVAILABLE,
	.InterleaveMode = XAIE_FEATURE_UNAVAILABLE,
	.FifoMode = XAIE_FEATURE_UNAVAILABLE,
	.EnTokenIssue = XAIE_FEATURE_AVAILABLE,
	.RepeatCount = XAIE_FEATURE_AVAILABLE,
	.TlastSuppress = XAIE_FEATURE_AVAILABLE,
	.StartQueueBase = XAIEMLGBL_MEM_TILE_MODULE_DMA_S2MM_0_START_QUEUE,
	.ChCtrlBase = XAIEMLGBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL,
	.NumChannels = 6,  /* number of s2mm/mm2s channels */
	.ChIdxOffset = 0x8,  /* This is the offset between each channel */
	.ChStatusBase = XAIEMLGBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0,
	.ChStatusOffset = 0x20,
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
	.NxtBd.Lsb = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_5_NEXT_BD_LSB,
	.NxtBd.Mask = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_5_NEXT_BD_MASK,
	.UseNxtBd.Idx = 5U,
	.UseNxtBd.Lsb = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_5_USE_NEXT_BD_LSB,
	.UseNxtBd.Mask = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_5_USE_NEXT_BD_MASK,
	.ValidBd.Idx = 5U,
	.ValidBd.Lsb = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_5_VALID_BD_LSB,
	.ValidBd.Mask = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_5_VALID_BD_MASK,
	.OutofOrderBdId.Idx = 1U,
	.OutofOrderBdId.Lsb = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_1_OUT_OF_ORDER_BD_ID_LSB,
	.OutofOrderBdId.Mask = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_1_OUT_OF_ORDER_BD_ID_MASK,
	.TlastSuppress.Idx = 5U,
	.TlastSuppress.Lsb = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_5_TLAST_SUPPRESS_LSB,
	.TlastSuppress.Mask = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_5_TLAST_SUPPRESS_MASK,
};

static const  XAie_DmaBdPkt Aie2TileDmaBdPktProp =
{
	.EnPkt.Idx = 1U,
	.EnPkt.Lsb = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_1_ENABLE_PACKET_LSB,
	.EnPkt.Mask = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_1_ENABLE_PACKET_MASK,
	.PktId.Idx = 1U,
	.PktId.Lsb = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_1_PACKET_ID_LSB,
	.PktId.Mask = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_1_PACKET_ID_MASK,
	.PktType.Idx = 1U,
	.PktType.Lsb = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_1_PACKET_TYPE_LSB,
	.PktType.Mask = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_1_PACKET_TYPE_MASK,
};

static const  XAie_DmaBdLock Aie2TileDmaLockProp =
{
	.Aie2DmaLock.LckRelVal.Idx = 5U,
	.Aie2DmaLock.LckRelVal.Lsb = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_5_LOCK_REL_VALUE_LSB,
	.Aie2DmaLock.LckRelVal.Mask = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_5_LOCK_REL_VALUE_MASK,
	.Aie2DmaLock.LckRelId.Idx = 5U,
	.Aie2DmaLock.LckRelId.Lsb = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_5_LOCK_REL_ID_LSB,
	.Aie2DmaLock.LckRelId.Mask = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_5_LOCK_REL_ID_MASK,
	.Aie2DmaLock.LckAcqEn.Idx = 5U,
	.Aie2DmaLock.LckAcqEn.Lsb = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_5_LOCK_ACQ_ENABLE_LSB,
	.Aie2DmaLock.LckAcqEn.Mask = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_5_LOCK_ACQ_ENABLE_MASK,
	.Aie2DmaLock.LckAcqVal.Idx = 5U,
	.Aie2DmaLock.LckAcqVal.Lsb = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_5_LOCK_ACQ_VALUE_LSB,
	.Aie2DmaLock.LckAcqVal.Mask = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_5_LOCK_ACQ_VALUE_MASK,
	.Aie2DmaLock.LckAcqId.Idx = 5U,
	.Aie2DmaLock.LckAcqId.Lsb = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_5_LOCK_ACQ_ID_LSB,
	.Aie2DmaLock.LckAcqId.Mask = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_5_LOCK_ACQ_ID_MASK,
};

static const  XAie_DmaBdBuffer Aie2TileDmaBufferProp =
{
	.TileDmaBuff.BaseAddr.Idx = 0U,
	.TileDmaBuff.BaseAddr.Lsb = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_0_BASE_ADDRESS_LSB,
	.TileDmaBuff.BaseAddr.Mask = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_0_BASE_ADDRESS_MASK,
	.TileDmaBuff.BufferLen.Idx = 0U,
	.TileDmaBuff.BufferLen.Lsb = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_0_BUFFER_LENGTH_LSB,
	.TileDmaBuff.BufferLen.Mask = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_0_BUFFER_LENGTH_MASK,
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
	.Aie2MultiDimAddr.DmaDimProp[0U].StepSize.Lsb = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_2_D0_STEPSIZE_LSB,
	.Aie2MultiDimAddr.DmaDimProp[0U].StepSize.Mask = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_2_D0_STEPSIZE_MASK,
	.Aie2MultiDimAddr.DmaDimProp[0U].Wrap.Idx = 3U,
	.Aie2MultiDimAddr.DmaDimProp[0U].Wrap.Lsb = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_3_D0_WRAP_LSB,
	.Aie2MultiDimAddr.DmaDimProp[0U].Wrap.Mask = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_3_D0_WRAP_MASK,
	.Aie2MultiDimAddr.DmaDimProp[1U].StepSize.Idx = 2U,
	.Aie2MultiDimAddr.DmaDimProp[1U].StepSize.Lsb = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_2_D1_STEPSIZE_LSB,
	.Aie2MultiDimAddr.DmaDimProp[1U].StepSize.Mask = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_2_D1_STEPSIZE_MASK,
	.Aie2MultiDimAddr.DmaDimProp[1U].Wrap.Idx = 3U,
	.Aie2MultiDimAddr.DmaDimProp[1U].Wrap.Lsb = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_3_D1_WRAP_LSB,
	.Aie2MultiDimAddr.DmaDimProp[1U].Wrap.Mask = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_3_D1_WRAP_MASK,
	.Aie2MultiDimAddr.DmaDimProp[2U].StepSize.Idx = 3U,
	.Aie2MultiDimAddr.DmaDimProp[2U].StepSize.Lsb = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_3_D2_STEPSIZE_LSB,
	.Aie2MultiDimAddr.DmaDimProp[2U].StepSize.Mask = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_3_D2_STEPSIZE_MASK,
	.Aie2MultiDimAddr.IterCurr.Idx = 4U,
	.Aie2MultiDimAddr.IterCurr.Lsb = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_4_ITERATION_CURRENT_LSB,
	.Aie2MultiDimAddr.IterCurr.Mask = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_4_ITERATION_CURRENT_MASK,
	.Aie2MultiDimAddr.Iter.Wrap.Idx = 4U,
	.Aie2MultiDimAddr.Iter.Wrap.Lsb = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_4_ITERATION_WRAP_LSB,
	.Aie2MultiDimAddr.Iter.Wrap.Mask = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_4_ITERATION_WRAP_MASK,
	.Aie2MultiDimAddr.Iter.StepSize.Idx = 4U,
	.Aie2MultiDimAddr.Iter.StepSize.Lsb = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_4_ITERATION_STEPSIZE_LSB,
	.Aie2MultiDimAddr.Iter.StepSize.Mask = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_4_ITERATION_STEPSIZE_MASK,
	.Aie2MultiDimAddr.DmaDimProp[2U].Wrap = {0U},
	.Aie2MultiDimAddr.DmaDimProp[3U].Wrap = {0U},
	.Aie2MultiDimAddr.DmaDimProp[3U].StepSize = {0U}
};

static const  XAie_DmaBdCompression Aie2TileDmaCompressionProp =
{
	.EnCompression.Idx = 1U,
	.EnCompression.Lsb = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_1_ENABLE_COMPRESSION_LSB,
	.EnCompression.Mask = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_1_ENABLE_COMPRESSION_MASK,
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

static const XAie_DmaChStatus Aie2TileDmaChStatus =
{
	/* This database is common for mm2s and s2mm channels */
	.Aie2DmaChStatus.Status.Lsb = XAIEMLGBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STATUS_LSB,
	.Aie2DmaChStatus.Status.Mask = XAIEMLGBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STATUS_MASK,
	.Aie2DmaChStatus.TaskQSize.Lsb = XAIEMLGBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_TASK_QUEUE_SIZE_LSB,
	.Aie2DmaChStatus.TaskQSize.Mask = XAIEMLGBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_TASK_QUEUE_SIZE_MASK,
	.Aie2DmaChStatus.StalledLockAcq.Lsb = XAIEMLGBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_ACQ_LSB,
	.Aie2DmaChStatus.StalledLockAcq.Mask = XAIEMLGBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_ACQ_MASK,
	.Aie2DmaChStatus.StalledLockRel.Lsb = XAIEMLGBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_REL_LSB,
	.Aie2DmaChStatus.StalledLockRel.Mask = XAIEMLGBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_REL_MASK,
	.Aie2DmaChStatus.StalledStreamStarve.Lsb = XAIEMLGBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STALLED_STREAM_STARVATION_LSB,
	.Aie2DmaChStatus.StalledStreamStarve.Mask = XAIEMLGBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STALLED_STREAM_STARVATION_MASK,
	.Aie2DmaChStatus.StalledTCT.Lsb = XAIEMLGBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STALLED_TCT_OR_COUNT_FIFO_FULL_LSB,
	.Aie2DmaChStatus.StalledTCT.Mask = XAIEMLGBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STALLED_TCT_OR_COUNT_FIFO_FULL_MASK,
};

/* Data structure to capture register offsets and masks for Mem Tile and
 * Tile Dma Channels
 */
static const  XAie_DmaChProp Aie2DmaChProp =
{
	.HasFoTMode = XAIE_FEATURE_AVAILABLE,
	.HasControllerId = XAIE_FEATURE_AVAILABLE,
	.HasEnCompression = XAIE_FEATURE_AVAILABLE,
	.HasEnOutOfOrder = XAIE_FEATURE_AVAILABLE,
	.MaxFoTMode = DMA_FoT_COUNTS_FROM_MM_REG,
	.MaxRepeatCount = 256U,
	.ControllerId.Idx = 0,
	.ControllerId.Lsb = XAIEMLGBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_CONTROLLER_ID_LSB,
	.ControllerId.Mask =XAIEMLGBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_CONTROLLER_ID_MASK ,
	.EnCompression.Idx = 0,
	.EnCompression.Lsb = XAIEMLGBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_DECOMPRESSION_ENABLE_LSB,
	.EnCompression.Mask = XAIEMLGBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_DECOMPRESSION_ENABLE_MASK,
	.EnOutofOrder.Idx = 0,
	.EnOutofOrder.Lsb = XAIEMLGBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_ENABLE_OUT_OF_ORDER_LSB,
	.EnOutofOrder.Mask = XAIEMLGBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_ENABLE_OUT_OF_ORDER_MASK,
	.FoTMode.Idx = 0,
	.FoTMode.Lsb = XAIEMLGBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_FOT_MODE_LSB,
	.FoTMode.Mask = XAIEMLGBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_FOT_MODE_MASK ,
	.Reset.Idx = 0,
	.Reset.Lsb = XAIEMLGBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_RESET_LSB,
	.Reset.Mask = XAIEMLGBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_RESET_MASK,
	.EnToken.Idx = 1,
	.EnToken.Lsb = XAIEMLGBL_MEMORY_MODULE_DMA_S2MM_0_START_QUEUE_ENABLE_TOKEN_ISSUE_LSB,
	.EnToken.Mask = XAIEMLGBL_MEMORY_MODULE_DMA_S2MM_0_START_QUEUE_ENABLE_TOKEN_ISSUE_MASK,
	.RptCount.Idx = 1,
	.RptCount.Lsb = XAIEMLGBL_MEMORY_MODULE_DMA_S2MM_0_START_QUEUE_REPEAT_COUNT_LSB,
	.RptCount.Mask = XAIEMLGBL_MEMORY_MODULE_DMA_S2MM_0_START_QUEUE_REPEAT_COUNT_MASK,
	.StartBd.Idx = 1,
	.StartBd.Lsb = XAIEMLGBL_MEMORY_MODULE_DMA_S2MM_0_START_QUEUE_START_BD_ID_LSB,
	.StartBd.Mask = XAIEMLGBL_MEMORY_MODULE_DMA_S2MM_0_START_QUEUE_START_BD_ID_MASK,
	.PauseStream = {0U},
	.PauseMem = {0U},
	.Enable = {0U},
	.StartQSizeMax = 4U,
	.DmaChStatus = &Aie2TileDmaChStatus,
};

/* Tile Dma Module */
static const  XAie_DmaMod Aie2TileDmaMod =
{
	.BaseAddr = XAIEMLGBL_MEMORY_MODULE_DMA_BD0_0,
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
	.EnTokenIssue = XAIE_FEATURE_AVAILABLE,
	.RepeatCount = XAIE_FEATURE_AVAILABLE,
	.TlastSuppress = XAIE_FEATURE_AVAILABLE,
	.StartQueueBase = XAIEMLGBL_MEMORY_MODULE_DMA_S2MM_0_START_QUEUE,
	.ChCtrlBase = XAIEMLGBL_MEMORY_MODULE_DMA_S2MM_0_CTRL,
	.NumChannels = 2U,  /* Number of s2mm/mm2s channels */
	.ChIdxOffset = 0x8,  /* This is the offset between each channel */
	.ChStatusBase = XAIEMLGBL_MEMORY_MODULE_DMA_S2MM_STATUS_0,
	.ChStatusOffset = 0x10,
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
	.NxtBd.Lsb = XAIEMLGBL_NOC_MODULE_DMA_BD0_7_NEXT_BD_LSB,
	.NxtBd.Mask = XAIEMLGBL_NOC_MODULE_DMA_BD0_7_NEXT_BD_MASK,
	.UseNxtBd.Idx = 7U,
	.UseNxtBd.Lsb = XAIEMLGBL_NOC_MODULE_DMA_BD0_7_USE_NEXT_BD_LSB,
	.UseNxtBd.Mask = XAIEMLGBL_NOC_MODULE_DMA_BD0_7_USE_NEXT_BD_MASK,
	.ValidBd.Idx = 7U,
	.ValidBd.Lsb = XAIEMLGBL_NOC_MODULE_DMA_BD0_7_VALID_BD_LSB,
	.ValidBd.Mask = XAIEMLGBL_NOC_MODULE_DMA_BD0_7_VALID_BD_MASK,
	.OutofOrderBdId.Idx = 2U,
	.OutofOrderBdId.Lsb = XAIEMLGBL_NOC_MODULE_DMA_BD0_2_OUT_OF_ORDER_BD_ID_LSB,
	.OutofOrderBdId.Mask = XAIEMLGBL_NOC_MODULE_DMA_BD0_2_OUT_OF_ORDER_BD_ID_MASK,
	.TlastSuppress.Idx = 7U,
	.TlastSuppress.Lsb = XAIEMLGBL_NOC_MODULE_DMA_BD0_7_TLAST_SUPPRESS_LSB,
	.TlastSuppress.Mask = XAIEMLGBL_NOC_MODULE_DMA_BD0_7_TLAST_SUPPRESS_MASK,
};

static const  XAie_DmaBdPkt Aie2ShimDmaBdPktProp =
{
	.EnPkt.Idx = 2U,
	.EnPkt.Lsb = XAIEMLGBL_NOC_MODULE_DMA_BD0_2_ENABLE_PACKET_LSB,
	.EnPkt.Mask = XAIEMLGBL_NOC_MODULE_DMA_BD0_2_ENABLE_PACKET_MASK,
	.PktId.Idx = 2U,
	.PktId.Lsb = XAIEMLGBL_NOC_MODULE_DMA_BD0_2_PACKET_ID_LSB,
	.PktId.Mask = XAIEMLGBL_NOC_MODULE_DMA_BD0_2_PACKET_ID_MASK,
	.PktType.Idx = 2U,
	.PktType.Lsb = XAIEMLGBL_NOC_MODULE_DMA_BD0_2_PACKET_TYPE_LSB,
	.PktType.Mask = XAIEMLGBL_NOC_MODULE_DMA_BD0_2_PACKET_TYPE_MASK,
};

static const  XAie_DmaBdLock Aie2ShimDmaLockProp =
{
	.Aie2DmaLock.LckRelVal.Idx = 7U,
	.Aie2DmaLock.LckRelVal.Lsb = XAIEMLGBL_NOC_MODULE_DMA_BD0_7_LOCK_REL_VALUE_LSB,
	.Aie2DmaLock.LckRelVal.Mask = XAIEMLGBL_NOC_MODULE_DMA_BD0_7_LOCK_REL_VALUE_MASK,
	.Aie2DmaLock.LckRelId.Idx = 7U,
	.Aie2DmaLock.LckRelId.Lsb = XAIEMLGBL_NOC_MODULE_DMA_BD0_7_LOCK_REL_ID_LSB,
	.Aie2DmaLock.LckRelId.Mask = XAIEMLGBL_NOC_MODULE_DMA_BD0_7_LOCK_REL_ID_MASK,
	.Aie2DmaLock.LckAcqEn.Idx = 7U,
	.Aie2DmaLock.LckAcqEn.Lsb = XAIEMLGBL_NOC_MODULE_DMA_BD0_7_LOCK_ACQ_ENABLE_LSB,
	.Aie2DmaLock.LckAcqEn.Mask = XAIEMLGBL_NOC_MODULE_DMA_BD0_7_LOCK_ACQ_ENABLE_MASK,
	.Aie2DmaLock.LckAcqVal.Idx = 7U,
	.Aie2DmaLock.LckAcqVal.Lsb = XAIEMLGBL_NOC_MODULE_DMA_BD0_7_LOCK_ACQ_VALUE_LSB,
	.Aie2DmaLock.LckAcqVal.Mask = XAIEMLGBL_NOC_MODULE_DMA_BD0_7_LOCK_ACQ_VALUE_MASK,
	.Aie2DmaLock.LckAcqId.Idx = 7U,
	.Aie2DmaLock.LckAcqId.Lsb = XAIEMLGBL_NOC_MODULE_DMA_BD0_7_LOCK_ACQ_ID_LSB,
	.Aie2DmaLock.LckAcqId.Mask = XAIEMLGBL_NOC_MODULE_DMA_BD0_7_LOCK_ACQ_ID_MASK,
};

static const  XAie_DmaBdBuffer Aie2ShimDmaBufferProp =
{
	.ShimDmaBuff.AddrLow.Idx = 1U,
	.ShimDmaBuff.AddrLow.Lsb = XAIEMLGBL_NOC_MODULE_DMA_BD0_1_BASE_ADDRESS_LOW_LSB,
	.ShimDmaBuff.AddrLow.Mask = XAIEMLGBL_NOC_MODULE_DMA_BD0_1_BASE_ADDRESS_LOW_MASK,
	.ShimDmaBuff.AddrHigh.Idx = 2U,
	.ShimDmaBuff.AddrHigh.Lsb = XAIEMLGBL_NOC_MODULE_DMA_BD0_2_BASE_ADDRESS_HIGH_LSB,
	.ShimDmaBuff.AddrHigh.Mask = XAIEMLGBL_NOC_MODULE_DMA_BD0_2_BASE_ADDRESS_HIGH_MASK,
	.ShimDmaBuff.BufferLen.Idx = 0U,
	.ShimDmaBuff.BufferLen.Lsb = XAIEMLGBL_NOC_MODULE_DMA_BD0_0_BUFFER_LENGTH_LSB,
	.ShimDmaBuff.BufferLen.Mask = XAIEMLGBL_NOC_MODULE_DMA_BD0_0_BUFFER_LENGTH_MASK,
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
	.Aie2MultiDimAddr.DmaDimProp[0U].StepSize.Lsb = XAIEMLGBL_NOC_MODULE_DMA_BD0_3_D0_STEPSIZE_LSB,
	.Aie2MultiDimAddr.DmaDimProp[0U].StepSize.Mask = XAIEMLGBL_NOC_MODULE_DMA_BD0_3_D0_STEPSIZE_MASK,
	.Aie2MultiDimAddr.DmaDimProp[0U].Wrap.Idx = 3U,
	.Aie2MultiDimAddr.DmaDimProp[0U].Wrap.Lsb = XAIEMLGBL_NOC_MODULE_DMA_BD0_3_D0_WRAP_LSB,
	.Aie2MultiDimAddr.DmaDimProp[0U].Wrap.Mask = XAIEMLGBL_NOC_MODULE_DMA_BD0_3_D0_WRAP_MASK,
	.Aie2MultiDimAddr.DmaDimProp[1U].StepSize.Idx =3U ,
	.Aie2MultiDimAddr.DmaDimProp[1U].StepSize.Lsb = XAIEMLGBL_NOC_MODULE_DMA_BD0_4_D1_STEPSIZE_LSB,
	.Aie2MultiDimAddr.DmaDimProp[1U].StepSize.Mask = XAIEMLGBL_NOC_MODULE_DMA_BD0_4_D1_STEPSIZE_MASK,
	.Aie2MultiDimAddr.DmaDimProp[1U].Wrap.Idx = 3U,
	.Aie2MultiDimAddr.DmaDimProp[1U].Wrap.Lsb = XAIEMLGBL_NOC_MODULE_DMA_BD0_4_D1_WRAP_LSB,
	.Aie2MultiDimAddr.DmaDimProp[1U].Wrap.Mask = XAIEMLGBL_NOC_MODULE_DMA_BD0_4_D1_WRAP_MASK,
	.Aie2MultiDimAddr.DmaDimProp[2U].StepSize.Idx = 5U,
	.Aie2MultiDimAddr.DmaDimProp[2U].StepSize.Lsb = XAIEMLGBL_NOC_MODULE_DMA_BD0_5_D2_STEPSIZE_LSB,
	.Aie2MultiDimAddr.DmaDimProp[2U].StepSize.Mask = XAIEMLGBL_NOC_MODULE_DMA_BD0_5_D2_STEPSIZE_MASK,
	.Aie2MultiDimAddr.IterCurr.Idx = 6U,
	.Aie2MultiDimAddr.IterCurr.Lsb = XAIEMLGBL_NOC_MODULE_DMA_BD0_6_ITERATION_CURRENT_LSB,
	.Aie2MultiDimAddr.IterCurr.Mask = XAIEMLGBL_NOC_MODULE_DMA_BD0_6_ITERATION_CURRENT_MASK,
	.Aie2MultiDimAddr.Iter.Wrap.Idx = 6U,
	.Aie2MultiDimAddr.Iter.Wrap.Lsb = XAIEMLGBL_NOC_MODULE_DMA_BD0_6_ITERATION_WRAP_LSB,
	.Aie2MultiDimAddr.Iter.Wrap.Mask = XAIEMLGBL_NOC_MODULE_DMA_BD0_6_ITERATION_WRAP_MASK,
	.Aie2MultiDimAddr.Iter.StepSize.Idx = 6U,
	.Aie2MultiDimAddr.Iter.StepSize.Lsb = XAIEMLGBL_NOC_MODULE_DMA_BD0_6_ITERATION_STEPSIZE_LSB,
	.Aie2MultiDimAddr.Iter.StepSize.Mask = XAIEMLGBL_NOC_MODULE_DMA_BD0_6_ITERATION_STEPSIZE_MASK,
	.Aie2MultiDimAddr.DmaDimProp[2U].Wrap = {0U},
	.Aie2MultiDimAddr.DmaDimProp[3U].Wrap = {0U},
	.Aie2MultiDimAddr.DmaDimProp[3U].StepSize = {0U}
};

static const  XAie_DmaSysProp Aie2ShimDmaSysProp =
{
	.SMID.Idx = 5U,
	.SMID.Lsb = XAIEMLGBL_NOC_MODULE_DMA_BD0_5_SMID_LSB,
	.SMID.Mask = XAIEMLGBL_NOC_MODULE_DMA_BD0_5_SMID_MASK,
	.BurstLen.Idx = 4U,
	.BurstLen.Lsb = XAIEMLGBL_NOC_MODULE_DMA_BD0_4_BURST_LENGTH_LSB,
	.BurstLen.Mask = XAIEMLGBL_NOC_MODULE_DMA_BD0_4_BURST_LENGTH_MASK,
	.AxQos.Idx = 5U,
	.AxQos.Lsb = XAIEMLGBL_NOC_MODULE_DMA_BD0_5_AXQOS_LSB,
	.AxQos.Mask = XAIEMLGBL_NOC_MODULE_DMA_BD0_5_AXQOS_MASK,
	.SecureAccess.Idx = 3U,
	.SecureAccess.Lsb = XAIEMLGBL_NOC_MODULE_DMA_BD0_3_SECURE_ACCESS_LSB,
	.SecureAccess.Mask = XAIEMLGBL_NOC_MODULE_DMA_BD0_3_SECURE_ACCESS_MASK,
	.AxCache.Idx = 5U,
	.AxCache.Lsb = XAIEMLGBL_NOC_MODULE_DMA_BD0_5_AXCACHE_LSB,
	.AxCache.Mask = XAIEMLGBL_NOC_MODULE_DMA_BD0_5_AXCACHE_MASK,
};

/* Data structure to capture register offsets and masks for Tile Dma */
static const  XAie_DmaBdProp Aie2ShimDmaProp =
{
	.AddrAlignMask = 0xF,
	.AddrAlignShift = 2U,
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

static const XAie_DmaChStatus Aie2ShimDmaChStatus =
{
	/* This database is common for mm2s and s2mm channels */
	.Aie2DmaChStatus.Status.Lsb = XAIEMLGBL_NOC_MODULE_DMA_S2MM_STATUS_0_STATUS_LSB,
	.Aie2DmaChStatus.Status.Mask = XAIEMLGBL_NOC_MODULE_DMA_S2MM_STATUS_0_STATUS_MASK,
	.Aie2DmaChStatus.TaskQSize.Lsb = XAIEMLGBL_NOC_MODULE_DMA_S2MM_STATUS_0_TASK_QUEUE_SIZE_LSB,
	.Aie2DmaChStatus.TaskQSize.Mask = XAIEMLGBL_NOC_MODULE_DMA_S2MM_STATUS_0_TASK_QUEUE_SIZE_MASK,
	.Aie2DmaChStatus.StalledLockAcq.Lsb = XAIEMLGBL_NOC_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_ACQ_LSB,
	.Aie2DmaChStatus.StalledLockAcq.Mask = XAIEMLGBL_NOC_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_ACQ_MASK,
	.Aie2DmaChStatus.StalledLockRel.Lsb = XAIEMLGBL_NOC_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_REL_LSB,
	.Aie2DmaChStatus.StalledLockRel.Mask = XAIEMLGBL_NOC_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_REL_MASK,
	.Aie2DmaChStatus.StalledStreamStarve.Lsb = XAIEMLGBL_NOC_MODULE_DMA_S2MM_STATUS_0_STALLED_STREAM_STARVATION_LSB,
	.Aie2DmaChStatus.StalledStreamStarve.Mask = XAIEMLGBL_NOC_MODULE_DMA_S2MM_STATUS_0_STALLED_STREAM_STARVATION_MASK,
	.Aie2DmaChStatus.StalledTCT.Lsb = XAIEMLGBL_NOC_MODULE_DMA_S2MM_STATUS_0_STALLED_TCT_OR_COUNT_FIFO_FULL_LSB,
	.Aie2DmaChStatus.StalledTCT.Mask = XAIEMLGBL_NOC_MODULE_DMA_S2MM_STATUS_0_STALLED_TCT_OR_COUNT_FIFO_FULL_MASK,
};

/* Data structure to capture register offsets and masks for Mem Tile and
 * Tile Dma Channels
 */
static const  XAie_DmaChProp Aie2ShimDmaChProp =
{
	.HasFoTMode = XAIE_FEATURE_AVAILABLE,
	.HasControllerId = XAIE_FEATURE_AVAILABLE,
	.HasEnCompression = XAIE_FEATURE_AVAILABLE,
	.HasEnOutOfOrder = XAIE_FEATURE_AVAILABLE,
	.MaxFoTMode = DMA_FoT_COUNTS_FROM_MM_REG,
	.MaxRepeatCount = 256U,
	.ControllerId.Idx = 0U,
	.ControllerId.Lsb = XAIEMLGBL_NOC_MODULE_DMA_S2MM_0_CTRL_CONTROLLER_ID_LSB ,
	.ControllerId.Mask = XAIEMLGBL_NOC_MODULE_DMA_S2MM_0_CTRL_CONTROLLER_ID_MASK ,
	.EnCompression.Idx = 0U,
	.EnCompression.Lsb = 0U,
	.EnCompression.Mask = 0U,
	.EnOutofOrder.Idx = 0U,
	.EnOutofOrder.Lsb = XAIEMLGBL_NOC_MODULE_DMA_S2MM_0_CTRL_ENABLE_OUT_OF_ORDER_LSB,
	.EnOutofOrder.Mask = XAIEMLGBL_NOC_MODULE_DMA_S2MM_0_CTRL_ENABLE_OUT_OF_ORDER_MASK,
	.FoTMode.Idx = 0,
	.FoTMode.Lsb = XAIEMLGBL_NOC_MODULE_DMA_S2MM_0_CTRL_FOT_MODE_LSB,
	.FoTMode.Mask = XAIEMLGBL_NOC_MODULE_DMA_S2MM_0_CTRL_FOT_MODE_MASK ,
	.Reset.Idx = 0U,
	.Reset.Lsb = 0U,
	.Reset.Mask = 0U,
	.EnToken.Idx = 1U,
	.EnToken.Lsb = XAIEMLGBL_NOC_MODULE_DMA_S2MM_0_TASK_QUEUE_ENABLE_TOKEN_ISSUE_LSB,
	.EnToken.Mask = XAIEMLGBL_NOC_MODULE_DMA_S2MM_0_TASK_QUEUE_ENABLE_TOKEN_ISSUE_MASK,
	.RptCount.Idx = 1U,
	.RptCount.Lsb = XAIEMLGBL_NOC_MODULE_DMA_S2MM_0_TASK_QUEUE_REPEAT_COUNT_LSB,
	.RptCount.Mask = XAIEMLGBL_NOC_MODULE_DMA_S2MM_0_TASK_QUEUE_REPEAT_COUNT_MASK,
	.StartBd.Idx = 1U,
	.StartBd.Lsb = XAIEMLGBL_NOC_MODULE_DMA_S2MM_0_TASK_QUEUE_START_BD_ID_LSB,
	.StartBd.Mask = XAIEMLGBL_NOC_MODULE_DMA_S2MM_0_TASK_QUEUE_START_BD_ID_MASK,
	.PauseStream = {0U},
	.PauseMem = {0U},
	.Enable = {0U},
	.StartQSizeMax = 4U,
	.DmaChStatus = &Aie2ShimDmaChStatus,
};

/* Tile Dma Module */
static const  XAie_DmaMod Aie2ShimDmaMod =
{
	.BaseAddr = XAIEMLGBL_NOC_MODULE_DMA_BD0_0,
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
	.EnTokenIssue = XAIE_FEATURE_AVAILABLE,
	.RepeatCount = XAIE_FEATURE_AVAILABLE,
	.TlastSuppress = XAIE_FEATURE_AVAILABLE,
	.StartQueueBase = XAIEMLGBL_NOC_MODULE_DMA_S2MM_0_TASK_QUEUE,
	.ChCtrlBase = XAIEMLGBL_NOC_MODULE_DMA_S2MM_0_CTRL,
	.NumChannels = 2U,  /* Number of s2mm/mm2s channels */
	.ChIdxOffset = 0x8,  /* This is the offset between each channel */
	.ChStatusBase = XAIEMLGBL_NOC_MODULE_DMA_S2MM_STATUS_0,
	.ChStatusOffset = 0x8,
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
		.PortBaseAddr = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0,
	},
	{	/* DMA */
		.NumPorts = 2,
		.PortBaseAddr = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_DMA0,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_TILE_CTRL,
	},
	{	/* Fifo */
		.NumPorts = 1,
		.PortBaseAddr = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_FIFO0,
	},
	{	/* South */
		.NumPorts = 4,
		.PortBaseAddr = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_SOUTH0,
	},
	{	/* West */
		.NumPorts = 4,
		.PortBaseAddr = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_WEST0,
	},
	{	/* North */
		.NumPorts = 6,
		.PortBaseAddr = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_NORTH0,
	},
	{	/* East */
		.NumPorts = 4,
		.PortBaseAddr = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_EAST0,
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
		.PortBaseAddr = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0,
	},
	{	/* DMA */
		.NumPorts = 2,
		.PortBaseAddr = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_DMA_0,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_TILE_CTRL,
	},
	{	/* Fifo */
		.NumPorts = 1,
		.PortBaseAddr = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_FIFO_0,
	},
	{	/* South */
		.NumPorts = 6,
		.PortBaseAddr = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_SOUTH_0,
	},
	{	/* West */
		.NumPorts = 4,
		.PortBaseAddr = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_WEST_0,
	},
	{	/* North */
		.NumPorts = 4,
		.PortBaseAddr = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_NORTH_0,
	},
	{	/* East */
		.NumPorts = 4,
		.PortBaseAddr = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_EAST_0,
	},
	{	/* Trace */
		.NumPorts = 2,
		.PortBaseAddr = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_MEM_TRACE
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
		.PortBaseAddr = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_MASTER_CONFIG_TILE_CTRL,
	},
	{	/* Fifo */
		.NumPorts = 1,
		.PortBaseAddr = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_MASTER_CONFIG_FIFO0,
	},
	{	/* South */
		.NumPorts = 6,
		.PortBaseAddr = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_MASTER_CONFIG_SOUTH0,
	},
	{	/* West */
		.NumPorts = 4,
		.PortBaseAddr = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_MASTER_CONFIG_WEST0,
	},
	{	/* North */
		.NumPorts = 6,
		.PortBaseAddr = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_MASTER_CONFIG_NORTH0,
	},
	{	/* East */
		.NumPorts = 4,
		.PortBaseAddr = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_MASTER_CONFIG_EAST0,
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
		.PortBaseAddr = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_TILE_CTRL,
	},
	{	/* Fifo */
		.NumPorts = 1,
		.PortBaseAddr = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_FIFO_0,
	},
	{	/* South */
		.NumPorts = 8,
		.PortBaseAddr = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_SOUTH_0,
	},
	{	/* West */
		.NumPorts = 4,
		.PortBaseAddr = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_WEST_0,
	},
	{	/* North */
		.NumPorts = 4,
		.PortBaseAddr = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_NORTH_0,
	},
	{	/* East */
		.NumPorts = 4,
		.PortBaseAddr = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_EAST_0,
	},
	{	/* Trace */
		.NumPorts = 2,
		.PortBaseAddr = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_TRACE
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
		.PortBaseAddr = XAIEMLGBL_MEM_TILE_MODULE_STREAM_SWITCH_MASTER_CONFIG_DMA0,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIEMLGBL_MEM_TILE_MODULE_STREAM_SWITCH_MASTER_CONFIG_TILE_CTRL,
	},
	{	/* Fifo */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* South */
		.NumPorts = 4,
		.PortBaseAddr = XAIEMLGBL_MEM_TILE_MODULE_STREAM_SWITCH_MASTER_CONFIG_SOUTH0,
	},
	{	/* West */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* North */
		.NumPorts = 6,
		.PortBaseAddr = XAIEMLGBL_MEM_TILE_MODULE_STREAM_SWITCH_MASTER_CONFIG_NORTH0,
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
		.PortBaseAddr = XAIEMLGBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_DMA_0,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIEMLGBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_TILE_CTRL,
	},
	{	/* Fifo */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* South */
		.NumPorts = 6,
		.PortBaseAddr = XAIEMLGBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_SOUTH_0,
	},
	{	/* West */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* North */
		.NumPorts = 4,
		.PortBaseAddr = XAIEMLGBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_NORTH_0,
	},
	{	/* East */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* Trace */
		.NumPorts = 1,
		.PortBaseAddr = XAIEMLGBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_TRACE
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
		.PortBaseAddr = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_SLAVE_TILE_CTRL_SLOT0,
	},
	{	/* Fifo */
		.NumPorts = 1,
		.PortBaseAddr = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_SLAVE_FIFO_0_SLOT0,
	},
	{	/* South */
		.NumPorts = 8,
		.PortBaseAddr = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_SLAVE_SOUTH_0_SLOT0,
	},
	{	/* West */
		.NumPorts = 4,
		.PortBaseAddr = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_SLAVE_WEST_0_SLOT0,
	},
	{	/* North */
		.NumPorts = 4,
		.PortBaseAddr = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_SLAVE_NORTH_0_SLOT0,
	},
	{	/* East */
		.NumPorts = 4,
		.PortBaseAddr = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_SLAVE_EAST_0_SLOT0,
	},
	{	/* Trace */
		.NumPorts = 1,
		.PortBaseAddr = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_SLAVE_TRACE_SLOT0
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
		.PortBaseAddr = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0,
	},
	{	/* DMA */
		.NumPorts = 2,
		.PortBaseAddr = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_DMA_0_SLOT0,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_TILE_CTRL_SLOT0,
	},
	{	/* Fifo */
		.NumPorts = 1,
		.PortBaseAddr = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_FIFO_0_SLOT0,
	},
	{	/* South */
		.NumPorts = 6,
		.PortBaseAddr = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_SOUTH_0_SLOT0,
	},
	{	/* West */
		.NumPorts = 4,
		.PortBaseAddr = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_WEST_0_SLOT0,
	},
	{	/* North */
		.NumPorts = 4,
		.PortBaseAddr = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_NORTH_0_SLOT0,
	},
	{	/* East */
		.NumPorts = 4,
		.PortBaseAddr = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_EAST_0_SLOT0,
	},
	{	/* Trace */
		.NumPorts = 1,
		.PortBaseAddr = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_TRACE_SLOT0
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
		.PortBaseAddr = XAIEMLGBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_DMA_0_SLOT0,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIEMLGBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_TILE_CTRL_SLOT0,
	},
	{	/* Fifo */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* South */
		.NumPorts = 6,
		.PortBaseAddr = XAIEMLGBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_SOUTH_0_SLOT0,
	},
	{	/* West */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* North */
		.NumPorts = 4,
		.PortBaseAddr = XAIEMLGBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_NORTH_0_SLOT0,
	},
	{	/* East */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* Trace */
		.NumPorts = 1,
		.PortBaseAddr = XAIEMLGBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_TRACE_SLOT0,
	}
};

static const XAie_StrmSwPortMap Aie2TileStrmSwMasterPortMap[] =
{
	{
		/* PhyPort 0 */
		.PortType = CORE,
		.PortNum = 0,
	},
	{
		/* PhyPort 1 */
		.PortType = DMA,
		.PortNum = 0,
	},
	{
		/* PhyPort 2 */
		.PortType = DMA,
		.PortNum = 1,
	},
	{
		/* PhyPort 3 */
		.PortType = CTRL,
		.PortNum = 0,
	},
	{
		/* PhyPort 4 */
		.PortType = FIFO,
		.PortNum = 0,
	},
	{
		/* PhyPort 5 */
		.PortType = SOUTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 6 */
		.PortType = SOUTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 7 */
		.PortType = SOUTH,
		.PortNum = 2,
	},
	{
		/* PhyPort 8 */
		.PortType = SOUTH,
		.PortNum = 3,
	},
	{
		/* PhyPort 9 */
		.PortType = WEST,
		.PortNum = 0,
	},
	{
		/* PhyPort 10 */
		.PortType = WEST,
		.PortNum = 1,
	},
	{
		/* PhyPort 11 */
		.PortType = WEST,
		.PortNum = 2,
	},
	{
		/* PhyPort 12 */
		.PortType = WEST,
		.PortNum = 3,
	},
	{
		/* PhyPort 13 */
		.PortType = NORTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 14 */
		.PortType = NORTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 15 */
		.PortType = NORTH,
		.PortNum = 2,
	},
	{
		/* PhyPort 16 */
		.PortType = NORTH,
		.PortNum = 3,
	},
	{
		/* PhyPort 17 */
		.PortType = NORTH,
		.PortNum = 4,
	},
	{
		/* PhyPort 18 */
		.PortType = NORTH,
		.PortNum = 5,
	},
	{
		/* PhyPort 19 */
		.PortType = EAST,
		.PortNum = 0,
	},
	{
		/* PhyPort 20 */
		.PortType = EAST,
		.PortNum = 1,
	},
	{
		/* PhyPort 21 */
		.PortType = EAST,
		.PortNum = 2,
	},
	{
		/* PhyPort 22 */
		.PortType = EAST,
		.PortNum = 3,
	},
};

static const XAie_StrmSwPortMap Aie2TileStrmSwSlavePortMap[] =
{
	{
		/* PhyPort 0 */
		.PortType = CORE,
		.PortNum = 0,
	},
	{
		/* PhyPort 1 */
		.PortType = DMA,
		.PortNum = 0,
	},
	{
		/* PhyPort 2 */
		.PortType = DMA,
		.PortNum = 1,
	},
	{
		/* PhyPort 3 */
		.PortType = CTRL,
		.PortNum = 0,
	},
	{
		/* PhyPort 4 */
		.PortType = FIFO,
		.PortNum = 0,
	},
	{
		/* PhyPort 5 */
		.PortType = SOUTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 6 */
		.PortType = SOUTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 7 */
		.PortType = SOUTH,
		.PortNum = 2,
	},
	{
		/* PhyPort 8 */
		.PortType = SOUTH,
		.PortNum = 3,
	},
	{
		/* PhyPort 9 */
		.PortType = SOUTH,
		.PortNum = 4,
	},
	{
		/* PhyPort 10 */
		.PortType = SOUTH,
		.PortNum = 5,
	},
	{
		/* PhyPort 11 */
		.PortType = WEST,
		.PortNum = 0,
	},
	{
		/* PhyPort 12 */
		.PortType = WEST,
		.PortNum = 1,
	},
	{
		/* PhyPort 13 */
		.PortType = WEST,
		.PortNum = 2,
	},
	{
		/* PhyPort 14 */
		.PortType = WEST,
		.PortNum = 3,
	},
	{
		/* PhyPort 15 */
		.PortType = NORTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 16 */
		.PortType = NORTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 17 */
		.PortType = NORTH,
		.PortNum = 2,
	},
	{
		/* PhyPort 18 */
		.PortType = NORTH,
		.PortNum = 3,
	},
	{
		/* PhyPort 19 */
		.PortType = EAST,
		.PortNum = 0,
	},
	{
		/* PhyPort 20 */
		.PortType = EAST,
		.PortNum = 1,
	},
	{
		/* PhyPort 21 */
		.PortType = EAST,
		.PortNum = 2,
	},
	{
		/* PhyPort 22 */
		.PortType = EAST,
		.PortNum = 3,
	},
	{
		/* PhyPort 23 */
		.PortType = TRACE,
		.PortNum = 0,
	},
	{
		/* PhyPort 24 */
		.PortType = TRACE,
		.PortNum = 1,
	},
};

static const XAie_StrmSwPortMap Aie2ShimStrmSwMasterPortMap[] =
{
	{
		/* PhyPort 0 */
		.PortType = CTRL,
		.PortNum = 0,
	},
	{
		/* PhyPort 1 */
		.PortType = FIFO,
		.PortNum = 0,
	},
	{
		/* PhyPort 2 */
		.PortType = SOUTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 3 */
		.PortType = SOUTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 4 */
		.PortType = SOUTH,
		.PortNum = 2,
	},
	{
		/* PhyPort 5 */
		.PortType = SOUTH,
		.PortNum = 3,
	},
	{
		/* PhyPort 6 */
		.PortType = SOUTH,
		.PortNum = 4,
	},
	{
		/* PhyPort 7 */
		.PortType = SOUTH,
		.PortNum = 5,
	},
	{
		/* PhyPort 8 */
		.PortType = WEST,
		.PortNum = 0,
	},
	{
		/* PhyPort 9 */
		.PortType = WEST,
		.PortNum = 1,
	},
	{
		/* PhyPort 10 */
		.PortType = WEST,
		.PortNum = 2,
	},
	{
		/* PhyPort 11 */
		.PortType = WEST,
		.PortNum = 3,
	},
	{
		/* PhyPort 12 */
		.PortType = NORTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 13 */
		.PortType = NORTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 14 */
		.PortType = NORTH,
		.PortNum = 2,
	},
	{
		/* PhyPort 15 */
		.PortType = NORTH,
		.PortNum = 3,
	},
	{
		/* PhyPort 16 */
		.PortType = NORTH,
		.PortNum = 4,
	},
	{
		/* PhyPort 17 */
		.PortType = NORTH,
		.PortNum = 5,
	},
	{
		/* PhyPort 18 */
		.PortType = EAST,
		.PortNum = 0,
	},
	{
		/* PhyPort 19 */
		.PortType = EAST,
		.PortNum = 1,
	},
	{
		/* PhyPort 20 */
		.PortType = EAST,
		.PortNum = 2,
	},
	{
		/* PhyPort 21 */
		.PortType = EAST,
		.PortNum = 3,
	},
};

static const XAie_StrmSwPortMap Aie2ShimStrmSwSlavePortMap[] =
{
	{
		/* PhyPort 0 */
		.PortType = CTRL,
		.PortNum = 0,
	},
	{
		/* PhyPort 1 */
		.PortType = FIFO,
		.PortNum = 0,
	},
	{
		/* PhyPort 2 */
		.PortType = SOUTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 3 */
		.PortType = SOUTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 4 */
		.PortType = SOUTH,
		.PortNum = 2,
	},
	{
		/* PhyPort 5 */
		.PortType = SOUTH,
		.PortNum = 3,
	},
	{
		/* PhyPort 6 */
		.PortType = SOUTH,
		.PortNum = 4,
	},
	{
		/* PhyPort 7 */
		.PortType = SOUTH,
		.PortNum = 5,
	},
	{
		/* PhyPort 8 */
		.PortType = SOUTH,
		.PortNum = 6,
	},
	{
		/* PhyPort 9 */
		.PortType = SOUTH,
		.PortNum = 7,
	},
	{
		/* PhyPort 10 */
		.PortType = WEST,
		.PortNum = 0,
	},
	{
		/* PhyPort 11 */
		.PortType = WEST,
		.PortNum = 1,
	},
	{
		/* PhyPort 12 */
		.PortType = WEST,
		.PortNum = 2,
	},
	{
		/* PhyPort 13 */
		.PortType = WEST,
		.PortNum = 3,
	},
	{
		/* PhyPort 14 */
		.PortType = NORTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 15 */
		.PortType = NORTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 16 */
		.PortType = NORTH,
		.PortNum = 2,
	},
	{
		/* PhyPort 17 */
		.PortType = NORTH,
		.PortNum = 3,
	},
	{
		/* PhyPort 18 */
		.PortType = EAST,
		.PortNum = 0,
	},
	{
		/* PhyPort 19 */
		.PortType = EAST,
		.PortNum = 1,
	},
	{
		/* PhyPort 20 */
		.PortType = EAST,
		.PortNum = 2,
	},
	{
		/* PhyPort 21 */
		.PortType = EAST,
		.PortNum = 3,
	},
	{
		/* PhyPort 22 */
		.PortType = TRACE,
		.PortNum = 0,
	},
};

static const XAie_StrmSwPortMap Aie2MemTileStrmSwMasterPortMap[] =
{
	{
		/* PhyPort 0 */
		.PortType = DMA,
		.PortNum = 0,
	},
	{
		/* PhyPort 1 */
		.PortType = DMA,
		.PortNum = 1,
	},
	{
		/* PhyPort 2 */
		.PortType = DMA,
		.PortNum = 2,
	},
	{
		/* PhyPort 3 */
		.PortType = DMA,
		.PortNum = 3,
	},
	{
		/* PhyPort 4 */
		.PortType = DMA,
		.PortNum = 4,
	},
	{
		/* PhyPort 5 */
		.PortType = DMA,
		.PortNum = 5,
	},
	{
		/* PhyPort 6 */
		.PortType = CTRL,
		.PortNum = 0,
	},
	{
		/* PhyPort 7 */
		.PortType = SOUTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 8 */
		.PortType = SOUTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 9 */
		.PortType = SOUTH,
		.PortNum = 2,
	},
	{
		/* PhyPort 10 */
		.PortType = SOUTH,
		.PortNum = 3,
	},
	{
		/* PhyPort 11 */
		.PortType = NORTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 12 */
		.PortType = NORTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 13 */
		.PortType = NORTH,
		.PortNum = 2,
	},
	{
		/* PhyPort 14 */
		.PortType = NORTH,
		.PortNum = 3,
	},
	{
		/* PhyPort 15 */
		.PortType = NORTH,
		.PortNum = 4,
	},
	{
		/* PhyPort 16 */
		.PortType = NORTH,
		.PortNum = 5,
	},
};

static const XAie_StrmSwPortMap Aie2MemTileStrmSwSlavePortMap[] =
{
	{
		/* PhyPort 0 */
		.PortType = DMA,
		.PortNum = 0,
	},
	{
		/* PhyPort 1 */
		.PortType = DMA,
		.PortNum = 1,
	},
	{
		/* PhyPort 2 */
		.PortType = DMA,
		.PortNum = 2,
	},
	{
		/* PhyPort 3 */
		.PortType = DMA,
		.PortNum = 3,
	},
	{
		/* PhyPort 4 */
		.PortType = DMA,
		.PortNum = 4,
	},
	{
		/* PhyPort 5 */
		.PortType = DMA,
		.PortNum = 5,
	},
	{
		/* PhyPort 6 */
		.PortType = CTRL,
		.PortNum = 0,
	},
	{
		/* PhyPort 7 */
		.PortType = SOUTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 8 */
		.PortType = SOUTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 9 */
		.PortType = SOUTH,
		.PortNum = 2,
	},
	{
		/* PhyPort 10 */
		.PortType = SOUTH,
		.PortNum = 3,
	},
	{
		/* PhyPort 11 */
		.PortType = NORTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 12 */
		.PortType = NORTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 13 */
		.PortType = NORTH,
		.PortNum = 2,
	},
	{
		/* PhyPort 14 */
		.PortType = NORTH,
		.PortNum = 3,
	},
	{
		/* PhyPort 15 */
		.PortType = NORTH,
		.PortNum = 4,
	},
	{
		/* PhyPort 16 */
		.PortType = NORTH,
		.PortNum = 5,
	},
	{
		/* PhyPort 17 */
		.PortType = TRACE,
		.PortNum = 0,
	},
};

/*
 * Data structure to capture stream switch deterministic merge properties for
 * AIEML Tiles.
 */
static const XAie_StrmSwDetMerge AieMlAieTileStrmSwDetMerge = {
	.NumArbitors = 2U,
	.NumPositions = 4U,
	.ArbConfigOffset = 0x10,
	.ConfigBase = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1,
	.EnableBase = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_CTRL,
	.SlvId0.Lsb = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_SLAVE_ID_0_LSB,
	.SlvId0.Mask = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_SLAVE_ID_0_MASK,
	.SlvId1.Lsb = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_SLAVE_ID_1_LSB,
	.SlvId1.Mask = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_SLAVE_ID_1_MASK,
	.PktCount0.Lsb = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_PACKET_COUNT_0_LSB,
	.PktCount0.Mask = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_PACKET_COUNT_0_MASK,
	.PktCount1.Lsb = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_PACKET_COUNT_1_LSB,
	.PktCount1.Mask = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_PACKET_COUNT_1_MASK,
	.Enable.Lsb = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_CTRL_ENABLE_LSB,
	.Enable.Mask = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_CTRL_ENABLE_MASK,
};

/*
 * Data structure to capture stream switch deterministic merge properties for
 * AIEML Mem Tiles.
 */
static const XAie_StrmSwDetMerge AieMlMemTileStrmSwDetMerge = {
	.NumArbitors = 2U,
	.NumPositions = 4U,
	.ArbConfigOffset = 0x10,
	.ConfigBase = XAIEMLGBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1,
	.EnableBase = XAIEMLGBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_CTRL,
	.SlvId0.Lsb = XAIEMLGBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_SLAVE_ID_0_LSB,
	.SlvId0.Mask = XAIEMLGBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_SLAVE_ID_0_MASK,
	.SlvId1.Lsb = XAIEMLGBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_SLAVE_ID_1_LSB,
	.SlvId1.Mask = XAIEMLGBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_SLAVE_ID_1_MASK,
	.PktCount0.Lsb = XAIEMLGBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_PACKET_COUNT_0_LSB,
	.PktCount0.Mask = XAIEMLGBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_PACKET_COUNT_0_MASK,
	.PktCount1.Lsb = XAIEMLGBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_PACKET_COUNT_1_LSB,
	.PktCount1.Mask = XAIEMLGBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_PACKET_COUNT_1_MASK,
	.Enable.Lsb = XAIEMLGBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_CTRL_ENABLE_LSB,
	.Enable.Mask = XAIEMLGBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_CTRL_ENABLE_MASK,
};

/*
 * Data structure to capture stream switch deterministic merge properties for
 * AIEML SHIM PL Tiles.
 */
static const XAie_StrmSwDetMerge AieMlShimTileStrmSwDetMerge = {
	.NumArbitors = 2U,
	.NumPositions = 4U,
	.ArbConfigOffset = 0x10,
	.ConfigBase = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1,
	.EnableBase = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_CTRL,
	.SlvId0.Lsb = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_SLAVE_ID_0_LSB,
	.SlvId0.Mask = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_SLAVE_ID_0_MASK,
	.SlvId1.Lsb = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_SLAVE_ID_1_LSB,
	.SlvId1.Mask = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_SLAVE_ID_1_MASK,
	.PktCount0.Lsb = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_PACKET_COUNT_0_LSB,
	.PktCount0.Mask = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_PACKET_COUNT_0_MASK,
	.PktCount1.Lsb = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_PACKET_COUNT_1_LSB,
	.PktCount1.Mask = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_PACKET_COUNT_1_MASK,
	.Enable.Lsb = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_CTRL_ENABLE_LSB,
	.Enable.Mask = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_CTRL_ENABLE_MASK,
};

/*
 * Data structure to capture all stream configs for XAIEGBL_TILE_TYPE_AIETILE
 */
static const  XAie_StrmMod Aie2TileStrmSw =
{
	.SlvConfigBaseAddr = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0,
	.MstrConfigBaseAddr = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0,
	.PortOffset = 0x4,
	.NumSlaveSlots = 4U,
	.SlotOffsetPerPort = 0x10,
	.SlotOffset = 0x4,
	.DetMergeFeature = XAIE_FEATURE_AVAILABLE,
	.MstrEn = {XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_MASTER_ENABLE_LSB, XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_MASTER_ENABLE_MASK},
	.MstrPktEn = {XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_PACKET_ENABLE_LSB, XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_PACKET_ENABLE_MASK},
	.DrpHdr = {XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_DROP_HEADER_LSB, XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_DROP_HEADER_MASK},
	.Config = {XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_CONFIGURATION_LSB, XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_CONFIGURATION_MASK},
	.SlvEn = {XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_SLAVE_ENABLE_LSB, XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_SLAVE_ENABLE_MASK},
	.SlvPktEn = {XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_PACKET_ENABLE_LSB, XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_PACKET_ENABLE_MASK},
	.SlotPktId = {XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ID_LSB, XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ID_MASK},
	.SlotMask = {XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MASK_LSB, XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MASK_MASK},
	.SlotEn = {XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ENABLE_LSB, XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ENABLE_MASK},
	.SlotMsel = {XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MSEL_LSB, XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MSEL_MASK},
	.SlotArbitor = {XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ARBIT_LSB, XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ARBIT_MASK},
	.MstrConfig = Aie2TileStrmMstr,
	.SlvConfig = Aie2TileStrmSlv,
	.SlvSlotConfig = Aie2TileStrmSlaveSlot,
	.MaxMasterPhyPortId = 22U,
	.MaxSlavePhyPortId = 24U,
	.MasterPortMap = Aie2TileStrmSwMasterPortMap,
	.SlavePortMap = Aie2TileStrmSwSlavePortMap,
	.DetMerge = &AieMlAieTileStrmSwDetMerge,
	.PortVerify = _XAieMl_AieTile_StrmSwCheckPortValidity,
};

/*
 * Data structure to capture all stream configs for XAIEGBL_TILE_TYPE_SHIMNOC/PL
 */
static const  XAie_StrmMod Aie2ShimStrmSw =
{
	.SlvConfigBaseAddr = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_TILE_CTRL,
	.MstrConfigBaseAddr = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_MASTER_CONFIG_TILE_CTRL,
	.PortOffset = 0x4,
	.NumSlaveSlots = 4U,
	.SlotOffsetPerPort = 0x10,
	.SlotOffset = 0x4,
	.DetMergeFeature = XAIE_FEATURE_AVAILABLE,
	.MstrEn = {XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_MASTER_ENABLE_LSB, XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_MASTER_ENABLE_MASK},
	.MstrPktEn = {XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_PACKET_ENABLE_LSB, XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_PACKET_ENABLE_MASK},
	.DrpHdr = {XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_DROP_HEADER_LSB, XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_DROP_HEADER_MASK},
	.Config = {XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_CONFIGURATION_LSB, XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_CONFIGURATION_MASK},
	.SlvEn = {XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_SLAVE_ENABLE_LSB, XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_SLAVE_ENABLE_MASK},
	.SlvPktEn = {XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_PACKET_ENABLE_LSB, XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_PACKET_ENABLE_MASK},
	.SlotPktId = {XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ID_LSB, XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ID_MASK},
	.SlotMask = {XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MASK_LSB, XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MASK_MASK},
	.SlotEn = {XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ENABLE_LSB, XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ENABLE_MASK},
	.SlotMsel = {XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MSEL_LSB, XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MSEL_MASK},
	.SlotArbitor = {XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ARBIT_LSB, XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ARBIT_MASK},
	.MstrConfig = Aie2ShimStrmMstr,
	.SlvConfig = Aie2ShimStrmSlv,
	.SlvSlotConfig = Aie2ShimStrmSlaveSlot,
	.MaxMasterPhyPortId = 21U,
	.MaxSlavePhyPortId = 22U,
	.MasterPortMap = Aie2ShimStrmSwMasterPortMap,
	.SlavePortMap = Aie2ShimStrmSwSlavePortMap,
	.DetMerge = &AieMlShimTileStrmSwDetMerge,
	.PortVerify = _XAieMl_ShimTile_StrmSwCheckPortValidity,
};

/*
 * Data structure to capture all stream configs for XAIEGBL_TILE_TYPE_MEMTILE
 */
static const  XAie_StrmMod Aie2MemTileStrmSw =
{
	.SlvConfigBaseAddr = XAIEMLGBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_DMA_0,
	.MstrConfigBaseAddr = XAIEMLGBL_MEM_TILE_MODULE_STREAM_SWITCH_MASTER_CONFIG_DMA0,
	.PortOffset = 0x4,
	.NumSlaveSlots = 4U,
	.SlotOffsetPerPort = 0x10,
	.SlotOffset = 0x4,
	.DetMergeFeature = XAIE_FEATURE_AVAILABLE,
	.MstrEn = {XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_MASTER_ENABLE_LSB, XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_MASTER_ENABLE_MASK},
	.MstrPktEn = {XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_PACKET_ENABLE_LSB, XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_PACKET_ENABLE_MASK},
	.DrpHdr = {XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_DROP_HEADER_LSB, XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_DROP_HEADER_MASK},
	.Config = {XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_CONFIGURATION_LSB, XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_CONFIGURATION_MASK},
	.SlvEn = {XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_SLAVE_ENABLE_LSB, XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_SLAVE_ENABLE_MASK},
	.SlvPktEn = {XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_PACKET_ENABLE_LSB, XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_PACKET_ENABLE_MASK},
	.SlotPktId = {XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ID_LSB, XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ID_MASK},
	.SlotMask = {XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MASK_LSB, XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MASK_MASK},
	.SlotEn = {XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ENABLE_LSB, XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ENABLE_MASK},
	.SlotMsel = {XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MSEL_LSB, XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MSEL_MASK},
	.SlotArbitor = {XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ARBIT_LSB, XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ARBIT_MASK},
	.MstrConfig = Aie2MemTileStrmMstr,
	.SlvConfig = Aie2MemTileStrmSlv,
	.SlvSlotConfig = Aie2MemTileStrmSlaveSlot,
	.MaxMasterPhyPortId = 16U,
	.MaxSlavePhyPortId = 17U,
	.MasterPortMap = Aie2MemTileStrmSwMasterPortMap,
	.SlavePortMap = Aie2MemTileStrmSwSlavePortMap,
	.DetMerge = &AieMlMemTileStrmSwDetMerge,
	.PortVerify = _XAieMl_MemTile_StrmSwCheckPortValidity,
};

/* Register field attributes for PL interface down sizer for 32 and 64 bits */
static const  XAie_RegFldAttr Aie2DownSzr32_64Bit[] =
{
	{XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH0_LSB, XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH0_MASK},
	{XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH1_LSB, XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH1_MASK},
	{XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH2_LSB, XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH2_MASK},
	{XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH3_LSB, XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH3_MASK},
	{XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH4_LSB, XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH4_MASK},
	{XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH5_LSB, XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH5_MASK},
	{XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH6_LSB, XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH6_MASK},
	{XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH7_LSB, XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH7_MASK}
};

/* Register field attributes for PL interface down sizer for 128 bits */
static const  XAie_RegFldAttr Aie2DownSzr128Bit[] =
{
	{XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH0_SOUTH1_128_COMBINE_LSB, XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH0_SOUTH1_128_COMBINE_MASK},
	{XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH2_SOUTH3_128_COMBINE_LSB, XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH2_SOUTH3_128_COMBINE_MASK},
	{XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH4_SOUTH5_128_COMBINE_LSB, XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH4_SOUTH5_128_COMBINE_MASK},
	{XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH6_SOUTH7_128_COMBINE_LSB, XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH6_SOUTH7_128_COMBINE_MASK}
};

/* Register field attributes for PL interface up sizer */
static const  XAie_RegFldAttr Aie2UpSzr32_64Bit[] =
{
	{XAIEMLGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH0_LSB, XAIEMLGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH0_MASK},
	{XAIEMLGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH1_LSB, XAIEMLGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH1_MASK},
	{XAIEMLGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH2_LSB, XAIEMLGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH2_MASK},
	{XAIEMLGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH3_LSB, XAIEMLGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH3_MASK},
	{XAIEMLGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH4_LSB, XAIEMLGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH4_MASK},
	{XAIEMLGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH5_LSB, XAIEMLGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH5_MASK}
};

/* Register field attributes for PL interface up sizer for 128 bits */
static const  XAie_RegFldAttr Aie2UpSzr128Bit[] =
{
	{XAIEMLGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH0_SOUTH1_128_COMBINE_LSB, XAIEMLGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH0_SOUTH1_128_COMBINE_MASK},
	{XAIEMLGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH2_SOUTH3_128_COMBINE_LSB, XAIEMLGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH2_SOUTH3_128_COMBINE_MASK},
	{XAIEMLGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH4_SOUTH5_128_COMBINE_LSB, XAIEMLGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH4_SOUTH5_128_COMBINE_MASK}
};

/* Register field attributes for PL interface down sizer bypass */
static const  XAie_RegFldAttr Aie2DownSzrByPass[] =
{
	{XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH0_LSB, XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH0_MASK},
	{XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH1_LSB, XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH1_MASK},
	{XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH2_LSB, XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH2_MASK},
	{XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH4_LSB, XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH4_MASK},
	{XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH5_LSB, XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH5_MASK},
	{XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH6_LSB, XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH6_MASK}
};

/* Register field attributes for PL interface down sizer enable */
static const  XAie_RegFldAttr Aie2DownSzrEnable[] =
{
	{XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH0_LSB, XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH0_MASK},
	{XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH1_LSB, XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH1_MASK},
	{XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH2_LSB, XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH2_MASK},
	{XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH3_LSB, XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH3_MASK},
	{XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH4_LSB, XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH4_MASK},
	{XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH5_LSB, XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH5_MASK},
	{XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH6_LSB, XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH6_MASK},
	{XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH7_LSB, XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH7_MASK}
};

/* Register field attributes for SHIMNOC Mux configuration */
static const  XAie_RegFldAttr Aie2ShimMuxConfig[] =
{
	{XAIEMLGBL_NOC_MODULE_MUX_CONFIG_SOUTH2_LSB, XAIEMLGBL_NOC_MODULE_MUX_CONFIG_SOUTH2_MASK},
	{XAIEMLGBL_NOC_MODULE_MUX_CONFIG_SOUTH3_LSB, XAIEMLGBL_NOC_MODULE_MUX_CONFIG_SOUTH3_MASK},
	{XAIEMLGBL_NOC_MODULE_MUX_CONFIG_SOUTH6_LSB, XAIEMLGBL_NOC_MODULE_MUX_CONFIG_SOUTH6_MASK},
	{XAIEMLGBL_NOC_MODULE_MUX_CONFIG_SOUTH7_LSB, XAIEMLGBL_NOC_MODULE_MUX_CONFIG_SOUTH7_MASK},
};

/* Register field attributes for SHIMNOC DeMux configuration */
static const  XAie_RegFldAttr Aie2ShimDeMuxConfig[] =
{
	{XAIEMLGBL_NOC_MODULE_DEMUX_CONFIG_SOUTH2_LSB, XAIEMLGBL_NOC_MODULE_DEMUX_CONFIG_SOUTH2_MASK},
	{XAIEMLGBL_NOC_MODULE_DEMUX_CONFIG_SOUTH3_LSB, XAIEMLGBL_NOC_MODULE_DEMUX_CONFIG_SOUTH3_MASK},
	{XAIEMLGBL_NOC_MODULE_DEMUX_CONFIG_SOUTH4_LSB, XAIEMLGBL_NOC_MODULE_DEMUX_CONFIG_SOUTH4_MASK},
	{XAIEMLGBL_NOC_MODULE_DEMUX_CONFIG_SOUTH5_LSB, XAIEMLGBL_NOC_MODULE_DEMUX_CONFIG_SOUTH5_MASK}
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
	.RegOff = XAIEMLGBL_NOC_MODULE_ME_AXIMM_CONFIG,
	.NsuSlvErr = {XAIEMLGBL_NOC_MODULE_ME_AXIMM_CONFIG_SLVERR_BLOCK_LSB, XAIEMLGBL_NOC_MODULE_ME_AXIMM_CONFIG_SLVERR_BLOCK_MASK},
	.NsuDecErr = {XAIEMLGBL_NOC_MODULE_ME_AXIMM_CONFIG_DECERR_BLOCK_LSB, XAIEMLGBL_NOC_MODULE_ME_AXIMM_CONFIG_DECERR_BLOCK_MASK}
};

/* Core Module */
static const  XAie_CoreMod Aie2CoreMod =
{
	.IsCheckerBoard = 0U,
	.ProgMemAddr = 0x0,
	.ProgMemSize = 16 * 1024,
	.DataMemAddr = 0x40000,
	.ProgMemHostOffset = XAIEMLGBL_CORE_MODULE_PROGRAM_MEMORY,
	.DataMemSize = 64 * 1024,		/* AIE2 Tile Memory is 64kB */
	.DataMemShift = 16,
	.EccEvntRegOff = XAIEMLGBL_CORE_MODULE_ECC_SCRUBBING_EVENT,
	.CoreCtrl = &Aie2CoreCtrlReg,
	.CoreSts = &Aie2CoreStsReg,
	.CoreDebug = &Aie2CoreDebugReg,
	.CoreEvent = &Aie2CoreEventReg,
	.CoreAccumCtrl = &AieMlCoreAccumCtrlReg,
	.ConfigureDone = &_XAieMl_CoreConfigureDone,
	.Enable = &_XAieMl_CoreEnable,
	.WaitForDone = &_XAieMl_CoreWaitForDone,
	.ReadDoneBit = &_XAieMl_CoreReadDoneBit,
};

/* Data Memory Module for Tile data memory*/
static const  XAie_MemMod Aie2TileMemMod =
{
	.Size = 0x10000,
	.MemAddr = XAIEMLGBL_MEMORY_MODULE_DATAMEMORY,
	.EccEvntRegOff = XAIEMLGBL_MEMORY_MODULE_ECC_SCRUBBING_EVENT,
};

/* Data Memory Module for Mem Tile data memory*/
static const  XAie_MemMod Aie2MemTileMemMod =
{
	.Size = 0x80000,
	.MemAddr = XAIEMLGBL_MEM_TILE_MODULE_DATAMEMORY,
	.EccEvntRegOff = XAIEMLGBL_MEM_TILE_MODULE_ECC_SCRUBBING_EVENT,
};

/* PL Interface module for SHIMPL Tiles */
static const  XAie_PlIfMod Aie2PlIfMod =
{
	.UpSzrOff = XAIEMLGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG,
	.DownSzrOff = XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG,
	.DownSzrEnOff = XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE,
	.DownSzrByPassOff = XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS,
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
	.UpSzrOff = XAIEMLGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG,
	.DownSzrOff = XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG,
	.DownSzrEnOff = XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE,
	.DownSzrByPassOff = XAIEMLGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS,
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
	.ShimNocMuxOff = XAIEMLGBL_NOC_MODULE_MUX_CONFIG,
	.ShimNocDeMuxOff = XAIEMLGBL_NOC_MODULE_DEMUX_CONFIG,
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
	.BaseAddr = XAIEMLGBL_MEMORY_MODULE_LOCK_REQUEST,
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
	.BaseAddr = XAIEMLGBL_NOC_MODULE_LOCK_REQUEST,
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
	.BaseAddr = XAIEMLGBL_MEM_TILE_MODULE_LOCK_REQUEST,
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
		.PerfCtrlBaseAddr = XAIEMLGBL_MEMORY_MODULE_PERFORMANCE_CONTROL0,
		.PerfCtrlOffsetAdd = 0x4,
		.PerfCtrlResetBaseAddr = XAIEMLGBL_MEMORY_MODULE_PERFORMANCE_CONTROL1,
		.PerfCounterBaseAddr = XAIEMLGBL_MEMORY_MODULE_PERFORMANCE_COUNTER0,
		.PerfCounterEvtValBaseAddr = XAIEMLGBL_MEMORY_MODULE_PERFORMANCE_COUNTER0_EVENT_VALUE,
		{XAIEMLGBL_MEMORY_MODULE_PERFORMANCE_CONTROL0_CNT0_START_EVENT_LSB, XAIEMLGBL_MEMORY_MODULE_PERFORMANCE_CONTROL0_CNT0_START_EVENT_MASK},
		{XAIEMLGBL_MEMORY_MODULE_PERFORMANCE_CONTROL0_CNT0_STOP_EVENT_LSB, XAIEMLGBL_MEMORY_MODULE_PERFORMANCE_CONTROL0_CNT0_STOP_EVENT_MASK},
		{XAIEMLGBL_MEMORY_MODULE_PERFORMANCE_CONTROL1_CNT0_RESET_EVENT_LSB,XAIEMLGBL_MEMORY_MODULE_PERFORMANCE_CONTROL1_CNT0_RESET_EVENT_MASK},
	},
	{	.MaxCounterVal = 4U,
		.StartStopShift = 16U,
		.ResetShift = 8U,
		.PerfCounterOffsetAdd = 0X4,
		.PerfCtrlBaseAddr = XAIEMLGBL_CORE_MODULE_PERFORMANCE_CONTROL0,
		.PerfCtrlOffsetAdd = 0x4,
		.PerfCtrlResetBaseAddr = XAIEMLGBL_CORE_MODULE_PERFORMANCE_CONTROL2,
		.PerfCounterBaseAddr = XAIEMLGBL_CORE_MODULE_PERFORMANCE_COUNTER0,
		.PerfCounterEvtValBaseAddr = XAIEMLGBL_CORE_MODULE_PERFORMANCE_COUNTER0_EVENT_VALUE,
		{XAIEMLGBL_CORE_MODULE_PERFORMANCE_CONTROL0_CNT0_START_EVENT_LSB, XAIEMLGBL_CORE_MODULE_PERFORMANCE_CONTROL0_CNT0_START_EVENT_MASK},
		{XAIEMLGBL_CORE_MODULE_PERFORMANCE_CONTROL0_CNT0_STOP_EVENT_LSB, XAIEMLGBL_CORE_MODULE_PERFORMANCE_CONTROL0_CNT0_STOP_EVENT_MASK},
		{XAIEMLGBL_CORE_MODULE_PERFORMANCE_CONTROL2_CNT0_RESET_EVENT_LSB, XAIEMLGBL_CORE_MODULE_PERFORMANCE_CONTROL2_CNT0_RESET_EVENT_MASK},
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
	.PerfCtrlBaseAddr = XAIEMLGBL_PL_MODULE_PERFORMANCE_CTRL0,
	.PerfCtrlOffsetAdd = 0x0,
	.PerfCtrlResetBaseAddr = XAIEMLGBL_PL_MODULE_PERFORMANCE_CTRL1,
	.PerfCounterBaseAddr = XAIEMLGBL_PL_MODULE_PERFORMANCE_COUNTER0,
	.PerfCounterEvtValBaseAddr = XAIEMLGBL_PL_MODULE_PERFORMANCE_COUNTER0_EVENT_VALUE,
	{XAIEMLGBL_PL_MODULE_PERFORMANCE_CTRL0_CNT0_START_EVENT_LSB, XAIEMLGBL_PL_MODULE_PERFORMANCE_CTRL0_CNT0_START_EVENT_MASK},
	{XAIEMLGBL_PL_MODULE_PERFORMANCE_CTRL0_CNT0_STOP_EVENT_LSB, XAIEMLGBL_PL_MODULE_PERFORMANCE_CTRL0_CNT0_STOP_EVENT_MASK},
	{XAIEMLGBL_PL_MODULE_PERFORMANCE_CTRL1_CNT0_RESET_EVENT_LSB,XAIEMLGBL_PL_MODULE_PERFORMANCE_CTRL1_CNT0_RESET_EVENT_MASK},};

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
	.PerfCtrlBaseAddr = XAIEMLGBL_MEM_TILE_MODULE_PERFORMANCE_CONTROL0,
	.PerfCtrlOffsetAdd = 0x4,
	.PerfCtrlResetBaseAddr = XAIEMLGBL_MEM_TILE_MODULE_PERFORMANCE_CONTROL2,
	.PerfCounterBaseAddr = XAIEMLGBL_MEM_TILE_MODULE_PERFORMANCE_COUNTER0,
	.PerfCounterEvtValBaseAddr = XAIEMLGBL_MEM_TILE_MODULE_PERFORMANCE_COUNTER0_EVENT_VALUE,
	{XAIEMLGBL_MEM_TILE_MODULE_PERFORMANCE_CONTROL0_CNT0_START_EVENT_LSB, XAIEMLGBL_MEM_TILE_MODULE_PERFORMANCE_CONTROL0_CNT0_START_EVENT_MASK},
	{XAIEMLGBL_MEM_TILE_MODULE_PERFORMANCE_CONTROL0_CNT0_STOP_EVENT_LSB, XAIEMLGBL_MEM_TILE_MODULE_PERFORMANCE_CONTROL0_CNT0_STOP_EVENT_MASK},
	{XAIEMLGBL_MEM_TILE_MODULE_PERFORMANCE_CONTROL2_CNT0_RESET_EVENT_LSB, XAIEMLGBL_MEM_TILE_MODULE_PERFORMANCE_CONTROL2_CNT0_RESET_EVENT_MASK},
};

static const XAie_EventGroup Aie2MemGroupEvent[] =
{
	{
		.GroupEvent = XAIE_EVENT_GROUP_0_MEM,
		.GroupOff = 0U,
		.GroupMask = XAIEMLGBL_MEMORY_MODULE_EVENT_GROUP_0_ENABLE_MASK,
		.ResetValue = XAIEMLGBL_MEMORY_MODULE_EVENT_GROUP_0_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_WATCHPOINT_MEM,
		.GroupOff = 1U,
		.GroupMask = XAIEMLGBL_MEMORY_MODULE_EVENT_GROUP_WATCHPOINT_ENABLE_MASK,
		.ResetValue = XAIEMLGBL_MEMORY_MODULE_EVENT_GROUP_WATCHPOINT_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_DMA_ACTIVITY_MEM,
		.GroupOff = 2U,
		.GroupMask = XAIEMLGBL_MEMORY_MODULE_EVENT_GROUP_DMA_ENABLE_MASK,
		.ResetValue = XAIEMLGBL_MEMORY_MODULE_EVENT_GROUP_DMA_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_LOCK_MEM,
		.GroupOff = 3U,
		.GroupMask = XAIEMLGBL_MEMORY_MODULE_EVENT_GROUP_LOCK_ENABLE_MASK,
		.ResetValue = XAIEMLGBL_MEMORY_MODULE_EVENT_GROUP_LOCK_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_MEMORY_CONFLICT_MEM,
		.GroupOff = 4U,
		.GroupMask = XAIEMLGBL_MEMORY_MODULE_EVENT_GROUP_MEMORY_CONFLICT_ENABLE_MASK,
		.ResetValue = XAIEMLGBL_MEMORY_MODULE_EVENT_GROUP_MEMORY_CONFLICT_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_ERRORS_MEM,
		.GroupOff = 5U,
		.GroupMask = XAIEMLGBL_MEMORY_MODULE_EVENT_GROUP_ERROR_ENABLE_MASK,
		.ResetValue = XAIEMLGBL_MEMORY_MODULE_EVENT_GROUP_ERROR_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_BROADCAST_MEM,
		.GroupOff = 6U,
		.GroupMask = XAIEMLGBL_MEMORY_MODULE_EVENT_GROUP_BROADCAST_ENABLE_MASK,
		.ResetValue = XAIEMLGBL_MEMORY_MODULE_EVENT_GROUP_BROADCAST_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_USER_EVENT_MEM,
		.GroupOff = 7U,
		.GroupMask = XAIEMLGBL_MEMORY_MODULE_EVENT_GROUP_USER_EVENT_ENABLE_MASK,
		.ResetValue = XAIEMLGBL_MEMORY_MODULE_EVENT_GROUP_USER_EVENT_ENABLE_MASK,
	},
};

static const XAie_EventGroup Aie2CoreGroupEvent[] =
{
	{
		.GroupEvent = XAIE_EVENT_GROUP_0_CORE,
		.GroupOff = 0U,
		.GroupMask = XAIEMLGBL_CORE_MODULE_EVENT_GROUP_0_ENABLE_MASK,
		.ResetValue = XAIEMLGBL_CORE_MODULE_EVENT_GROUP_0_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_PC_EVENT_CORE,
		.GroupOff = 1U,
		.GroupMask = XAIEMLGBL_CORE_MODULE_EVENT_GROUP_PC_ENABLE_MASK,
		.ResetValue = XAIEMLGBL_CORE_MODULE_EVENT_GROUP_PC_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_CORE_STALL_CORE,
		.GroupOff = 2U,
		.GroupMask = XAIEMLGBL_CORE_MODULE_EVENT_GROUP_CORE_STALL_ENABLE_MASK,
		.ResetValue = XAIEMLGBL_CORE_MODULE_EVENT_GROUP_CORE_STALL_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_CORE_PROGRAM_FLOW_CORE,
		.GroupOff = 3U,
		.GroupMask = XAIEMLGBL_CORE_MODULE_EVENT_GROUP_CORE_PROGRAM_FLOW_ENABLE_MASK,
		.ResetValue = XAIEMLGBL_CORE_MODULE_EVENT_GROUP_CORE_PROGRAM_FLOW_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_ERRORS_0_CORE,
		.GroupOff = 4U,
		.GroupMask = XAIEMLGBL_CORE_MODULE_EVENT_GROUP_ERRORS0_ENABLE_MASK,
		.ResetValue = XAIEMLGBL_CORE_MODULE_EVENT_GROUP_ERRORS0_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_ERRORS_1_CORE,
		.GroupOff = 5U,
		.GroupMask = XAIEMLGBL_CORE_MODULE_EVENT_GROUP_ERRORS1_ENABLE_MASK,
		.ResetValue = XAIEMLGBL_CORE_MODULE_EVENT_GROUP_ERRORS1_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_STREAM_SWITCH_CORE,
		.GroupOff = 6U,
		.GroupMask = XAIEMLGBL_CORE_MODULE_EVENT_GROUP_STREAM_SWITCH_ENABLE_MASK,
		.ResetValue = XAIEMLGBL_CORE_MODULE_EVENT_GROUP_STREAM_SWITCH_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_BROADCAST_CORE,
		.GroupOff = 7U,
		.GroupMask = XAIEMLGBL_CORE_MODULE_EVENT_GROUP_BROADCAST_ENABLE_MASK,
		.ResetValue = XAIEMLGBL_CORE_MODULE_EVENT_GROUP_BROADCAST_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_USER_EVENT_CORE,
		.GroupOff = 8U,
		.GroupMask = XAIEMLGBL_CORE_MODULE_EVENT_GROUP_USER_EVENT_ENABLE_MASK,
		.ResetValue = XAIEMLGBL_CORE_MODULE_EVENT_GROUP_USER_EVENT_ENABLE_MASK,
	},
};

static const XAie_EventGroup Aie2PlGroupEvent[] =
{
	{
		.GroupEvent = XAIE_EVENT_GROUP_0_PL,
		.GroupOff = 0U,
		.GroupMask = XAIEMLGBL_PL_MODULE_EVENT_GROUP_0_ENABLE_MASK,
		.ResetValue = XAIEMLGBL_PL_MODULE_EVENT_GROUP_0_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_DMA_ACTIVITY_PL,
		.GroupOff = 1U,
		.GroupMask = XAIEMLGBL_PL_MODULE_EVENT_GROUP_DMA_ACTIVITY_ENABLE_MASK,
		.ResetValue = XAIEMLGBL_PL_MODULE_EVENT_GROUP_DMA_ACTIVITY_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_LOCK_PL,
		.GroupOff = 2U,
		.GroupMask = XAIEMLGBL_PL_MODULE_EVENT_GROUP_LOCK_ENABLE_MASK,
		.ResetValue = XAIEMLGBL_PL_MODULE_EVENT_GROUP_LOCK_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_ERRORS_PL,
		.GroupOff = 3U,
		.GroupMask = XAIEMLGBL_PL_MODULE_EVENT_GROUP_ERRORS_ENABLE_MASK,
		.ResetValue = XAIEMLGBL_PL_MODULE_EVENT_GROUP_ERRORS_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_STREAM_SWITCH_PL,
		.GroupOff = 4U,
		.GroupMask = XAIEMLGBL_PL_MODULE_EVENT_GROUP_STREAM_SWITCH_ENABLE_MASK,
		.ResetValue = XAIEMLGBL_PL_MODULE_EVENT_GROUP_STREAM_SWITCH_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_BROADCAST_A_PL,
		.GroupOff = 5U,
		.GroupMask = XAIEMLGBL_PL_MODULE_EVENT_GROUP_BROADCAST_A_ENABLE_MASK,
		.ResetValue = XAIEMLGBL_PL_MODULE_EVENT_GROUP_BROADCAST_A_ENABLE_MASK,
	},
};

static const XAie_EventGroup Aie2MemTileGroupEvent[] = {
	{
		.GroupEvent = XAIE_EVENT_GROUP_0_MEM_TILE,
		.GroupOff = 0U,
		.GroupMask = XAIEMLGBL_MEM_TILE_MODULE_EVENT_GROUP_0_ENABLE_MASK,
		.ResetValue = XAIEMLGBL_MEM_TILE_MODULE_EVENT_GROUP_0_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_WATCHPOINT_MEM_TILE,
		.GroupOff = 1U,
		.GroupMask = XAIEMLGBL_MEM_TILE_MODULE_EVENT_GROUP_WATCHPOINT_ENABLE_MASK,
		.ResetValue = XAIEMLGBL_MEM_TILE_MODULE_EVENT_GROUP_WATCHPOINT_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_DMA_ACTIVITY_MEM_TILE,
		.GroupOff = 2U,
		.GroupMask = XAIEMLGBL_MEM_TILE_MODULE_EVENT_GROUP_DMA_ENABLE_MASK,
		.ResetValue = XAIEMLGBL_MEM_TILE_MODULE_EVENT_GROUP_DMA_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_LOCK_MEM_TILE,
		.GroupOff = 3U,
		.GroupMask = XAIEMLGBL_MEM_TILE_MODULE_EVENT_GROUP_LOCK_ENABLE_MASK,
		.ResetValue = XAIEMLGBL_MEM_TILE_MODULE_EVENT_GROUP_LOCK_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_STREAM_SWITCH_MEM_TILE,
		.GroupOff = 4U,
		.GroupMask = XAIEMLGBL_MEM_TILE_MODULE_EVENT_GROUP_STREAM_SWITCH_ENABLE_MASK,
		.ResetValue = XAIEMLGBL_MEM_TILE_MODULE_EVENT_GROUP_STREAM_SWITCH_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_MEMORY_CONFLICT_MEM_TILE,
		.GroupOff = 5U,
		.GroupMask = XAIEMLGBL_MEM_TILE_MODULE_EVENT_GROUP_MEMORY_CONFLICT_ENABLE_MASK,
		.ResetValue = XAIEMLGBL_MEM_TILE_MODULE_EVENT_GROUP_MEMORY_CONFLICT_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_ERRORS_MEM_TILE,
		.GroupOff = 6U,
		.GroupMask = XAIEMLGBL_MEM_TILE_MODULE_EVENT_GROUP_ERROR_ENABLE_MASK,
		.ResetValue = XAIEMLGBL_MEM_TILE_MODULE_EVENT_GROUP_ERROR_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_BROADCAST_MEM_TILE,
		.GroupOff = 7U,
		.GroupMask = XAIEMLGBL_MEM_TILE_MODULE_EVENT_GROUP_BROADCAST_ENABLE_MASK,
		.ResetValue = XAIEMLGBL_MEM_TILE_MODULE_EVENT_GROUP_BROADCAST_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_USER_EVENT_MEM_TILE,
		.GroupOff = 8U,
		.GroupMask = XAIEMLGBL_MEM_TILE_MODULE_EVENT_GROUP_USER_EVENT_ENABLE_MASK,
		.ResetValue = XAIEMLGBL_MEM_TILE_MODULE_EVENT_GROUP_USER_EVENT_ENABLE_MASK,
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
		.GenEventRegOff = XAIEMLGBL_MEMORY_MODULE_EVENT_GENERATE,
		.GenEvent = {XAIEMLGBL_MEMORY_MODULE_EVENT_GENERATE_EVENT_LSB, XAIEMLGBL_MEMORY_MODULE_EVENT_GENERATE_EVENT_MASK},
		.ComboInputRegOff = XAIEMLGBL_MEMORY_MODULE_COMBO_EVENT_INPUTS,
		.ComboEventMask = XAIEMLGBL_MEMORY_MODULE_COMBO_EVENT_INPUTS_EVENTA_MASK,
		.ComboEventOff = 8U,
		.ComboCtrlRegOff = XAIEMLGBL_MEMORY_MODULE_COMBO_EVENT_CONTROL,
		.ComboConfigMask = XAIEMLGBL_MEMORY_MODULE_COMBO_EVENT_CONTROL_COMBO0_MASK,
		.ComboConfigOff = 8U,
		.BaseStrmPortSelectRegOff = XAIE_FEATURE_UNAVAILABLE,
		.NumStrmPortSelectIds = XAIE_FEATURE_UNAVAILABLE,
		.StrmPortSelectIdsPerReg = XAIE_FEATURE_UNAVAILABLE,
		.PortIdMask = XAIE_FEATURE_UNAVAILABLE,
		.PortIdOff = XAIE_FEATURE_UNAVAILABLE,
		.PortMstrSlvMask = XAIE_FEATURE_UNAVAILABLE,
		.PortMstrSlvOff = XAIE_FEATURE_UNAVAILABLE,
		.BaseBroadcastRegOff = XAIEMLGBL_MEMORY_MODULE_EVENT_BROADCAST0,
		.NumBroadcastIds = 16U,
		.BaseBroadcastSwBlockRegOff = XAIEMLGBL_MEMORY_MODULE_EVENT_BROADCAST_BLOCK_SOUTH_SET,
		.BaseBroadcastSwUnblockRegOff = XAIEMLGBL_MEMORY_MODULE_EVENT_BROADCAST_BLOCK_SOUTH_CLR,
		.BroadcastSwOff = 0U,
		.BroadcastSwBlockOff = 16U,
		.BroadcastSwUnblockOff = 16U,
		.NumSwitches = 1U,
		.BaseGroupEventRegOff = XAIEMLGBL_MEMORY_MODULE_EVENT_GROUP_0_ENABLE,
		.NumGroupEvents = 8U,
		.DefaultGroupErrorMask = 0x7FFAU,
		.Group = Aie2MemGroupEvent,
		.BasePCEventRegOff = XAIE_FEATURE_UNAVAILABLE,
		.NumPCEvents = XAIE_FEATURE_UNAVAILABLE,
		.PCAddr = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
		.PCValid = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
		.BaseStatusRegOff = XAIEMLGBL_MEMORY_MODULE_EVENT_STATUS0,
	},
	{
		.XAie_EventNumber = Aie2CoreModEventMapping,
		.EventMin = XAIE_EVENT_NONE_CORE,
		.EventMax = XAIE_EVENT_PROCESSOR_BUS_ERROR_CORE,
		.GenEventRegOff = XAIEMLGBL_CORE_MODULE_EVENT_GENERATE,
		.GenEvent = {XAIEMLGBL_CORE_MODULE_EVENT_GENERATE_EVENT_LSB, XAIEMLGBL_CORE_MODULE_EVENT_GENERATE_EVENT_MASK},
		.ComboInputRegOff = XAIEMLGBL_CORE_MODULE_COMBO_EVENT_INPUTS,
		.ComboEventMask = XAIEMLGBL_CORE_MODULE_COMBO_EVENT_INPUTS_EVENTA_MASK,
		.ComboEventOff = 8U,
		.ComboCtrlRegOff = XAIEMLGBL_CORE_MODULE_COMBO_EVENT_CONTROL,
		.ComboConfigMask = XAIEMLGBL_CORE_MODULE_COMBO_EVENT_CONTROL_COMBO0_MASK,
		.ComboConfigOff = 8U,
		.BaseStrmPortSelectRegOff = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_0,
		.NumStrmPortSelectIds = 8U,
		.StrmPortSelectIdsPerReg = 4U,
		.PortIdMask = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_0_PORT_0_ID_MASK,
		.PortIdOff = 8U,
		.PortMstrSlvMask = XAIEMLGBL_CORE_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_0_PORT_0_MASTER_SLAVE_MASK,
		.PortMstrSlvOff = 5U,
		.BaseBroadcastRegOff = XAIEMLGBL_CORE_MODULE_EVENT_BROADCAST0,
		.NumBroadcastIds = 16U,
		.BaseBroadcastSwBlockRegOff = XAIEMLGBL_CORE_MODULE_EVENT_BROADCAST_BLOCK_SOUTH_SET,
		.BaseBroadcastSwUnblockRegOff = XAIEMLGBL_CORE_MODULE_EVENT_BROADCAST_BLOCK_SOUTH_CLR,
		.BroadcastSwOff = 0U,
		.BroadcastSwBlockOff = 16U,
		.BroadcastSwUnblockOff = 16U,
		.NumSwitches = 1U,
		.BaseGroupEventRegOff = XAIEMLGBL_CORE_MODULE_EVENT_GROUP_0_ENABLE,
		.NumGroupEvents = 9U,
		.DefaultGroupErrorMask = 0x1CF5F80U,
		.Group = Aie2CoreGroupEvent,
		.BasePCEventRegOff = XAIEMLGBL_CORE_MODULE_PC_EVENT0,
		.NumPCEvents = 4U,
		.PCAddr = {XAIEMLGBL_CORE_MODULE_PC_EVENT0_PC_ADDRESS_LSB, XAIEMLGBL_CORE_MODULE_PC_EVENT0_PC_ADDRESS_MASK},
		.PCValid = {XAIEMLGBL_CORE_MODULE_PC_EVENT0_VALID_LSB, XAIEMLGBL_CORE_MODULE_PC_EVENT0_VALID_MASK},
		.BaseStatusRegOff = XAIEMLGBL_CORE_MODULE_EVENT_STATUS0,
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
	.GenEventRegOff = XAIEMLGBL_PL_MODULE_EVENT_GENERATE,
	.GenEvent = {XAIEMLGBL_PL_MODULE_EVENT_GENERATE_EVENT_LSB, XAIEMLGBL_PL_MODULE_EVENT_GENERATE_EVENT_MASK},
	.ComboInputRegOff = XAIEMLGBL_PL_MODULE_COMBO_EVENT_INPUTS,
	.ComboEventMask = XAIEMLGBL_PL_MODULE_COMBO_EVENT_INPUTS_EVENTA_MASK,
	.ComboEventOff = 8U,
	.ComboCtrlRegOff = XAIEMLGBL_PL_MODULE_COMBO_EVENT_CONTROL,
	.ComboConfigMask = XAIEMLGBL_PL_MODULE_COMBO_EVENT_CONTROL_COMBO0_MASK,
	.ComboConfigOff = 8U,
	.BaseStrmPortSelectRegOff = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_0,
	.NumStrmPortSelectIds = 8U,
	.StrmPortSelectIdsPerReg = 4U,
	.PortIdMask = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_0_PORT_0_ID_MASK,
	.PortIdOff = 8U,
	.PortMstrSlvMask = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_0_PORT_0_MASTER_SLAVE_MASK,
	.PortMstrSlvOff = 5U,
	.BaseBroadcastRegOff = XAIEMLGBL_PL_MODULE_EVENT_BROADCAST0_A,
	.NumBroadcastIds = 15U,
	.BaseBroadcastSwBlockRegOff = XAIEMLGBL_PL_MODULE_EVENT_BROADCAST_A_BLOCK_SOUTH_SET,
	.BaseBroadcastSwUnblockRegOff = XAIEMLGBL_PL_MODULE_EVENT_BROADCAST_A_BLOCK_SOUTH_CLR,
	.BroadcastSwOff = 64U,
	.BroadcastSwBlockOff = 16U,
	.BroadcastSwUnblockOff = 16U,
	.NumSwitches = 2U,
	.BaseGroupEventRegOff = XAIEMLGBL_PL_MODULE_EVENT_GROUP_0_ENABLE,
	.NumGroupEvents = 6U,
	.DefaultGroupErrorMask = 0x7FFU,
	.Group = Aie2PlGroupEvent,
	.BasePCEventRegOff = XAIE_FEATURE_UNAVAILABLE,
	.NumPCEvents = XAIE_FEATURE_UNAVAILABLE,
	.PCAddr = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
	.PCValid = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
	.BaseStatusRegOff = XAIEMLGBL_PL_MODULE_EVENT_STATUS0,
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
	.GenEventRegOff = XAIEMLGBL_PL_MODULE_EVENT_GENERATE,
	.GenEvent = {XAIEMLGBL_PL_MODULE_EVENT_GENERATE_EVENT_LSB, XAIEMLGBL_PL_MODULE_EVENT_GENERATE_EVENT_MASK},
	.ComboInputRegOff = XAIEMLGBL_PL_MODULE_COMBO_EVENT_INPUTS,
	.ComboEventMask = XAIEMLGBL_PL_MODULE_COMBO_EVENT_INPUTS_EVENTA_MASK,
	.ComboEventOff = 8U,
	.ComboCtrlRegOff = XAIEMLGBL_PL_MODULE_COMBO_EVENT_CONTROL,
	.ComboConfigMask = XAIEMLGBL_PL_MODULE_COMBO_EVENT_CONTROL_COMBO0_MASK,
	.ComboConfigOff = 8U,
	.BaseStrmPortSelectRegOff = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_0,
	.NumStrmPortSelectIds = 8U,
	.StrmPortSelectIdsPerReg = 4U,
	.PortIdMask = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_0_PORT_0_ID_MASK,
	.PortIdOff = 8U,
	.PortMstrSlvMask = XAIEMLGBL_PL_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_0_PORT_0_MASTER_SLAVE_MASK,
	.PortMstrSlvOff = 5U,
	.BaseBroadcastRegOff = XAIEMLGBL_PL_MODULE_EVENT_BROADCAST0_A,
	.NumBroadcastIds = 15U,
	.BaseBroadcastSwBlockRegOff = XAIEMLGBL_PL_MODULE_EVENT_BROADCAST_A_BLOCK_SOUTH_SET,
	.BaseBroadcastSwUnblockRegOff = XAIEMLGBL_PL_MODULE_EVENT_BROADCAST_A_BLOCK_SOUTH_CLR,
	.BroadcastSwOff = 64U,
	.BroadcastSwBlockOff = 16U,
	.BroadcastSwUnblockOff = 16U,
	.NumSwitches = 2U,
	.BaseGroupEventRegOff = XAIEMLGBL_PL_MODULE_EVENT_GROUP_0_ENABLE,
	.NumGroupEvents = 6U,
	.DefaultGroupErrorMask = 0x7FFU,
	.Group = Aie2PlGroupEvent,
	.BasePCEventRegOff = XAIE_FEATURE_UNAVAILABLE,
	.NumPCEvents = XAIE_FEATURE_UNAVAILABLE,
	.PCAddr = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
	.PCValid = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
	.BaseStatusRegOff = XAIEMLGBL_PL_MODULE_EVENT_STATUS0,
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
	.GenEventRegOff = XAIEMLGBL_MEM_TILE_MODULE_EVENT_GENERATE,
	.GenEvent = {XAIEMLGBL_MEM_TILE_MODULE_EVENT_GENERATE_EVENT_LSB, XAIEMLGBL_MEM_TILE_MODULE_EVENT_GENERATE_EVENT_MASK},
	.ComboInputRegOff = XAIEMLGBL_MEM_TILE_MODULE_COMBO_EVENT_INPUTS,
	.ComboEventMask = XAIEMLGBL_MEM_TILE_MODULE_COMBO_EVENT_INPUTS_EVENTA_MASK,
	.ComboEventOff = 8U,
	.ComboCtrlRegOff = XAIEMLGBL_MEM_TILE_MODULE_COMBO_EVENT_CONTROL,
	.ComboConfigMask = XAIEMLGBL_MEM_TILE_MODULE_COMBO_EVENT_CONTROL_COMBO0_MASK,
	.ComboConfigOff = 8U,
	.BaseStrmPortSelectRegOff = XAIEMLGBL_MEM_TILE_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_0,
	.NumStrmPortSelectIds = 8U,
	.StrmPortSelectIdsPerReg = 4U,
	.PortIdMask = XAIEMLGBL_MEM_TILE_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_0_PORT_0_ID_MASK,
	.PortIdOff = 8U,
	.PortMstrSlvMask = XAIEMLGBL_MEM_TILE_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_0_PORT_0_MASTER_SLAVE_MASK,
	.PortMstrSlvOff = 5U,
	.BaseBroadcastRegOff = XAIEMLGBL_MEM_TILE_MODULE_EVENT_BROADCAST0,
	.NumBroadcastIds = 16U,
	.BaseBroadcastSwBlockRegOff = XAIEMLGBL_MEM_TILE_MODULE_EVENT_BROADCAST_A_BLOCK_SOUTH_SET,
	.BaseBroadcastSwUnblockRegOff = XAIEMLGBL_MEM_TILE_MODULE_EVENT_BROADCAST_A_BLOCK_SOUTH_CLR,
	.BroadcastSwOff = 64U,
	.BroadcastSwBlockOff = 16U,
	.BroadcastSwUnblockOff = 16U,
	.NumSwitches = 2U,
	.BaseGroupEventRegOff = XAIEMLGBL_MEM_TILE_MODULE_EVENT_GROUP_0_ENABLE,
	.NumGroupEvents = 9U,
	.DefaultGroupErrorMask = 0x7FAU,
	.Group = Aie2MemTileGroupEvent,
	.BasePCEventRegOff = XAIE_FEATURE_UNAVAILABLE,
	.NumPCEvents = XAIE_FEATURE_UNAVAILABLE,
	.PCAddr = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
	.PCValid = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
	.BaseStatusRegOff = XAIEMLGBL_MEM_TILE_MODULE_EVENT_STATUS0,
};

static const XAie_TimerMod Aie2TileTimerMod[] =
{
	 {
		.TrigEventLowValOff = XAIEMLGBL_MEMORY_MODULE_TIMER_TRIG_EVENT_LOW_VALUE,
		.TrigEventHighValOff = XAIEMLGBL_MEMORY_MODULE_TIMER_TRIG_EVENT_HIGH_VALUE,
		.LowOff = XAIEMLGBL_MEMORY_MODULE_TIMER_LOW,
		.HighOff = XAIEMLGBL_MEMORY_MODULE_TIMER_HIGH,
		.CtrlOff = XAIEMLGBL_MEMORY_MODULE_TIMER_CONTROL,
		{XAIEMLGBL_MEMORY_MODULE_TIMER_CONTROL_RESET_LSB, XAIEMLGBL_MEMORY_MODULE_TIMER_CONTROL_RESET_MASK},
		{XAIEMLGBL_MEMORY_MODULE_TIMER_CONTROL_RESET_EVENT_LSB, XAIEMLGBL_MEMORY_MODULE_TIMER_CONTROL_RESET_EVENT_MASK},
	},
	{
		.TrigEventLowValOff = XAIEMLGBL_CORE_MODULE_TIMER_TRIG_EVENT_LOW_VALUE,
		.TrigEventHighValOff = XAIEMLGBL_CORE_MODULE_TIMER_TRIG_EVENT_HIGH_VALUE,
		.LowOff = XAIEMLGBL_CORE_MODULE_TIMER_LOW,
		.HighOff = XAIEMLGBL_CORE_MODULE_TIMER_HIGH,
		.CtrlOff = XAIEMLGBL_CORE_MODULE_TIMER_CONTROL,
		{XAIEMLGBL_CORE_MODULE_TIMER_CONTROL_RESET_LSB, XAIEMLGBL_CORE_MODULE_TIMER_CONTROL_RESET_MASK},
		{XAIEMLGBL_CORE_MODULE_TIMER_CONTROL_RESET_EVENT_LSB, XAIEMLGBL_CORE_MODULE_TIMER_CONTROL_RESET_EVENT_MASK},
	}
};

static const XAie_TimerMod Aie2PlTimerMod =
{
	.TrigEventLowValOff = XAIEMLGBL_PL_MODULE_TIMER_TRIG_EVENT_LOW_VALUE,
	.TrigEventHighValOff = XAIEMLGBL_PL_MODULE_TIMER_TRIG_EVENT_HIGH_VALUE,
	.LowOff = XAIEMLGBL_PL_MODULE_TIMER_LOW,
	.HighOff = XAIEMLGBL_PL_MODULE_TIMER_HIGH,
	.CtrlOff = XAIEMLGBL_PL_MODULE_TIMER_CONTROL,
	{XAIEMLGBL_PL_MODULE_TIMER_CONTROL_RESET_LSB, XAIEMLGBL_PL_MODULE_TIMER_CONTROL_RESET_MASK},
	{XAIEMLGBL_PL_MODULE_TIMER_CONTROL_RESET_EVENT_LSB, XAIEMLGBL_PL_MODULE_TIMER_CONTROL_RESET_EVENT_MASK}
};

static const XAie_TimerMod Aie2MemTileTimerMod =
{
	.TrigEventLowValOff = XAIEMLGBL_MEM_TILE_MODULE_TIMER_TRIG_EVENT_LOW_VALUE ,
	.TrigEventHighValOff = XAIEMLGBL_MEM_TILE_MODULE_TIMER_TRIG_EVENT_HIGH_VALUE,
	.LowOff = XAIEMLGBL_MEM_TILE_MODULE_TIMER_LOW,
	.HighOff = XAIEMLGBL_MEM_TILE_MODULE_TIMER_HIGH,
	.CtrlOff = XAIEMLGBL_MEM_TILE_MODULE_TIMER_CONTROL,
	{XAIEMLGBL_MEM_TILE_MODULE_TIMER_CONTROL_RESET_LSB, XAIEMLGBL_MEM_TILE_MODULE_TIMER_CONTROL_RESET_MASK},
	{XAIEMLGBL_MEM_TILE_MODULE_TIMER_CONTROL_RESET_EVENT_LSB, XAIEMLGBL_MEM_TILE_MODULE_TIMER_CONTROL_RESET_EVENT_MASK}
};

/*
 * Data structure to configure trace event register for XAIE_MEM_MOD module
 * type
 */
static const XAie_RegFldAttr Aie2MemTraceEvent[] =
{
	{XAIEMLGBL_MEMORY_MODULE_TRACE_EVENT0_TRACE_EVENT0_LSB, XAIEMLGBL_MEMORY_MODULE_TRACE_EVENT0_TRACE_EVENT0_MASK},
	{XAIEMLGBL_MEMORY_MODULE_TRACE_EVENT0_TRACE_EVENT1_LSB, XAIEMLGBL_MEMORY_MODULE_TRACE_EVENT0_TRACE_EVENT1_MASK},
	{XAIEMLGBL_MEMORY_MODULE_TRACE_EVENT0_TRACE_EVENT2_LSB, XAIEMLGBL_MEMORY_MODULE_TRACE_EVENT0_TRACE_EVENT2_MASK},
	{XAIEMLGBL_MEMORY_MODULE_TRACE_EVENT0_TRACE_EVENT3_LSB, XAIEMLGBL_MEMORY_MODULE_TRACE_EVENT0_TRACE_EVENT3_MASK},
	{XAIEMLGBL_MEMORY_MODULE_TRACE_EVENT1_TRACE_EVENT4_LSB, XAIEMLGBL_MEMORY_MODULE_TRACE_EVENT1_TRACE_EVENT4_MASK},
	{XAIEMLGBL_MEMORY_MODULE_TRACE_EVENT1_TRACE_EVENT5_LSB, XAIEMLGBL_MEMORY_MODULE_TRACE_EVENT1_TRACE_EVENT5_MASK},
	{XAIEMLGBL_MEMORY_MODULE_TRACE_EVENT1_TRACE_EVENT6_LSB, XAIEMLGBL_MEMORY_MODULE_TRACE_EVENT1_TRACE_EVENT6_MASK},
	{XAIEMLGBL_MEMORY_MODULE_TRACE_EVENT1_TRACE_EVENT7_LSB, XAIEMLGBL_MEMORY_MODULE_TRACE_EVENT1_TRACE_EVENT7_MASK}
};

/*
 * Data structure to configure trace event register for XAIE_CORE_MOD module
 * type
 */
static const XAie_RegFldAttr Aie2CoreTraceEvent[] =
{
	{XAIEMLGBL_CORE_MODULE_TRACE_EVENT0_TRACE_EVENT0_LSB, XAIEMLGBL_CORE_MODULE_TRACE_EVENT0_TRACE_EVENT0_MASK},
	{XAIEMLGBL_CORE_MODULE_TRACE_EVENT0_TRACE_EVENT1_LSB, XAIEMLGBL_CORE_MODULE_TRACE_EVENT0_TRACE_EVENT1_MASK},
	{XAIEMLGBL_CORE_MODULE_TRACE_EVENT0_TRACE_EVENT2_LSB, XAIEMLGBL_CORE_MODULE_TRACE_EVENT0_TRACE_EVENT2_MASK},
	{XAIEMLGBL_CORE_MODULE_TRACE_EVENT0_TRACE_EVENT3_LSB, XAIEMLGBL_CORE_MODULE_TRACE_EVENT0_TRACE_EVENT3_MASK},
	{XAIEMLGBL_CORE_MODULE_TRACE_EVENT1_TRACE_EVENT4_LSB, XAIEMLGBL_CORE_MODULE_TRACE_EVENT1_TRACE_EVENT4_MASK},
	{XAIEMLGBL_CORE_MODULE_TRACE_EVENT1_TRACE_EVENT5_LSB, XAIEMLGBL_CORE_MODULE_TRACE_EVENT1_TRACE_EVENT5_MASK},
	{XAIEMLGBL_CORE_MODULE_TRACE_EVENT1_TRACE_EVENT6_LSB, XAIEMLGBL_CORE_MODULE_TRACE_EVENT1_TRACE_EVENT6_MASK},
	{XAIEMLGBL_CORE_MODULE_TRACE_EVENT1_TRACE_EVENT7_LSB, XAIEMLGBL_CORE_MODULE_TRACE_EVENT1_TRACE_EVENT7_MASK}
};

/*
 * Data structure to configure trace event register for XAIE_PL_MOD module
 * type
 */
static const XAie_RegFldAttr Aie2PlTraceEvent[] =
{
	{XAIEMLGBL_PL_MODULE_TRACE_EVENT0_TRACE_EVENT0_LSB, XAIEMLGBL_PL_MODULE_TRACE_EVENT0_TRACE_EVENT0_MASK},
	{XAIEMLGBL_PL_MODULE_TRACE_EVENT0_TRACE_EVENT1_LSB, XAIEMLGBL_PL_MODULE_TRACE_EVENT0_TRACE_EVENT1_MASK},
	{XAIEMLGBL_PL_MODULE_TRACE_EVENT0_TRACE_EVENT2_LSB, XAIEMLGBL_PL_MODULE_TRACE_EVENT0_TRACE_EVENT2_MASK},
	{XAIEMLGBL_PL_MODULE_TRACE_EVENT0_TRACE_EVENT3_LSB, XAIEMLGBL_PL_MODULE_TRACE_EVENT0_TRACE_EVENT3_MASK},
	{XAIEMLGBL_PL_MODULE_TRACE_EVENT1_TRACE_EVENT4_LSB, XAIEMLGBL_PL_MODULE_TRACE_EVENT1_TRACE_EVENT4_MASK},
	{XAIEMLGBL_PL_MODULE_TRACE_EVENT1_TRACE_EVENT5_LSB, XAIEMLGBL_PL_MODULE_TRACE_EVENT1_TRACE_EVENT5_MASK},
	{XAIEMLGBL_PL_MODULE_TRACE_EVENT1_TRACE_EVENT6_LSB, XAIEMLGBL_PL_MODULE_TRACE_EVENT1_TRACE_EVENT6_MASK},
	{XAIEMLGBL_PL_MODULE_TRACE_EVENT1_TRACE_EVENT7_LSB, XAIEMLGBL_PL_MODULE_TRACE_EVENT1_TRACE_EVENT7_MASK}
};

/*
 * Data structure to configure trace event register for
 * XAIEGBL_TILE_TYPE_MEMTILE type
 */
static const XAie_RegFldAttr Aie2MemTileTraceEvent[] =
{
	{XAIEMLGBL_MEM_TILE_MODULE_TRACE_EVENT0_TRACE_EVENT0_LSB, XAIEMLGBL_MEM_TILE_MODULE_TRACE_EVENT0_TRACE_EVENT0_MASK},
	{XAIEMLGBL_MEM_TILE_MODULE_TRACE_EVENT0_TRACE_EVENT1_LSB, XAIEMLGBL_MEM_TILE_MODULE_TRACE_EVENT0_TRACE_EVENT1_MASK},
	{XAIEMLGBL_MEM_TILE_MODULE_TRACE_EVENT0_TRACE_EVENT2_LSB, XAIEMLGBL_MEM_TILE_MODULE_TRACE_EVENT0_TRACE_EVENT2_MASK},
	{XAIEMLGBL_MEM_TILE_MODULE_TRACE_EVENT0_TRACE_EVENT3_LSB, XAIEMLGBL_MEM_TILE_MODULE_TRACE_EVENT0_TRACE_EVENT3_MASK},
	{XAIEMLGBL_MEM_TILE_MODULE_TRACE_EVENT1_TRACE_EVENT4_LSB, XAIEMLGBL_MEM_TILE_MODULE_TRACE_EVENT1_TRACE_EVENT4_MASK},
	{XAIEMLGBL_MEM_TILE_MODULE_TRACE_EVENT1_TRACE_EVENT5_LSB, XAIEMLGBL_MEM_TILE_MODULE_TRACE_EVENT1_TRACE_EVENT5_MASK},
	{XAIEMLGBL_MEM_TILE_MODULE_TRACE_EVENT1_TRACE_EVENT6_LSB, XAIEMLGBL_MEM_TILE_MODULE_TRACE_EVENT1_TRACE_EVENT6_MASK},
	{XAIEMLGBL_MEM_TILE_MODULE_TRACE_EVENT1_TRACE_EVENT7_LSB, XAIEMLGBL_MEM_TILE_MODULE_TRACE_EVENT1_TRACE_EVENT7_MASK}
};

/*
 * Data structure to configure trace module for XAIEGBL_TILE_TYPE_AIETILE tile
 * type
 */
static const XAie_TraceMod Aie2TileTraceMod[] =
{
	{
		.CtrlRegOff = XAIEMLGBL_MEMORY_MODULE_TRACE_CONTROL0,
		.PktConfigRegOff = XAIEMLGBL_MEMORY_MODULE_TRACE_CONTROL1,
		.StatusRegOff = XAIEMLGBL_MEMORY_MODULE_TRACE_STATUS,
		.EventRegOffs = (u32 []){XAIEMLGBL_MEMORY_MODULE_TRACE_EVENT0, XAIEMLGBL_MEMORY_MODULE_TRACE_EVENT1},
		.NumTraceSlotIds = 8U,
		.NumEventsPerSlot = 4U,
		.StopEvent = {XAIEMLGBL_MEMORY_MODULE_TRACE_CONTROL0_TRACE_STOP_EVENT_LSB, XAIEMLGBL_MEMORY_MODULE_TRACE_CONTROL0_TRACE_STOP_EVENT_MASK},
		.StartEvent = {XAIEMLGBL_MEMORY_MODULE_TRACE_CONTROL0_TRACE_START_EVENT_LSB, XAIEMLGBL_MEMORY_MODULE_TRACE_CONTROL0_TRACE_START_EVENT_MASK},
		.ModeConfig = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
		.PktType = {XAIEMLGBL_MEMORY_MODULE_TRACE_CONTROL1_PACKET_TYPE_LSB, XAIEMLGBL_MEMORY_MODULE_TRACE_CONTROL1_PACKET_TYPE_MASK},
		.PktId = {XAIEMLGBL_MEMORY_MODULE_TRACE_CONTROL1_ID_LSB, XAIEMLGBL_MEMORY_MODULE_TRACE_CONTROL1_ID_MASK},
		.State = {XAIEMLGBL_MEMORY_MODULE_TRACE_STATUS_STATE_LSB, XAIEMLGBL_MEMORY_MODULE_TRACE_STATUS_STATE_MASK},
		.ModeSts = {XAIEMLGBL_MEMORY_MODULE_TRACE_STATUS_MODE_LSB, XAIEMLGBL_MEMORY_MODULE_TRACE_STATUS_MODE_MASK},
		.Event = Aie2MemTraceEvent
	},
	{
		.CtrlRegOff = XAIEMLGBL_CORE_MODULE_TRACE_CONTROL0,
		.PktConfigRegOff = XAIEMLGBL_CORE_MODULE_TRACE_CONTROL1,
		.StatusRegOff = XAIEMLGBL_CORE_MODULE_TRACE_STATUS,
		.EventRegOffs = (u32 []){XAIEMLGBL_CORE_MODULE_TRACE_EVENT0, XAIEMLGBL_CORE_MODULE_TRACE_EVENT1},
		.NumTraceSlotIds = 8U,
		.NumEventsPerSlot = 4U,
		.StopEvent = {XAIEMLGBL_CORE_MODULE_TRACE_CONTROL0_TRACE_STOP_EVENT_LSB, XAIEMLGBL_CORE_MODULE_TRACE_CONTROL0_TRACE_STOP_EVENT_MASK},
		.StartEvent = {XAIEMLGBL_CORE_MODULE_TRACE_CONTROL0_TRACE_START_EVENT_LSB, XAIEMLGBL_CORE_MODULE_TRACE_CONTROL0_TRACE_START_EVENT_MASK},
		.ModeConfig = {XAIEMLGBL_CORE_MODULE_TRACE_CONTROL0_MODE_LSB, XAIEMLGBL_CORE_MODULE_TRACE_CONTROL0_MODE_MASK},
		.PktType = {XAIEMLGBL_CORE_MODULE_TRACE_CONTROL1_PACKET_TYPE_LSB, XAIEMLGBL_CORE_MODULE_TRACE_CONTROL1_PACKET_TYPE_MASK},
		.PktId = {XAIEMLGBL_CORE_MODULE_TRACE_CONTROL1_ID_LSB, XAIEMLGBL_CORE_MODULE_TRACE_CONTROL1_ID_MASK},
		.State = {XAIEMLGBL_CORE_MODULE_TRACE_STATUS_STATE_LSB, XAIEMLGBL_CORE_MODULE_TRACE_STATUS_STATE_MASK},
		.ModeSts = {XAIEMLGBL_CORE_MODULE_TRACE_STATUS_MODE_LSB, XAIEMLGBL_CORE_MODULE_TRACE_STATUS_MODE_MASK},
		.Event = Aie2CoreTraceEvent
	}
};

/*
 * Data structure to configure trace module for XAIEGBL_TILE_TYPE_SHIMNOC/PL
 * tile type
 */
static const XAie_TraceMod Aie2PlTraceMod =
{
	.CtrlRegOff = XAIEMLGBL_PL_MODULE_TRACE_CONTROL0,
	.PktConfigRegOff = XAIEMLGBL_PL_MODULE_TRACE_CONTROL1,
	.StatusRegOff = XAIEMLGBL_PL_MODULE_TRACE_STATUS,
	.EventRegOffs = (u32 []){XAIEMLGBL_PL_MODULE_TRACE_EVENT0, XAIEMLGBL_PL_MODULE_TRACE_EVENT1},
	.NumTraceSlotIds = 8U,
	.NumEventsPerSlot = 4U,
	.StopEvent = {XAIEMLGBL_PL_MODULE_TRACE_CONTROL0_TRACE_STOP_EVENT_LSB, XAIEMLGBL_PL_MODULE_TRACE_CONTROL0_TRACE_STOP_EVENT_MASK},
	.StartEvent = {XAIEMLGBL_PL_MODULE_TRACE_CONTROL0_TRACE_START_EVENT_LSB, XAIEMLGBL_PL_MODULE_TRACE_CONTROL0_TRACE_START_EVENT_MASK},
	.ModeConfig = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
	.PktType = {XAIEMLGBL_PL_MODULE_TRACE_CONTROL1_PACKET_TYPE_LSB, XAIEMLGBL_PL_MODULE_TRACE_CONTROL1_PACKET_TYPE_MASK},
	.PktId = {XAIEMLGBL_PL_MODULE_TRACE_CONTROL1_ID_LSB, XAIEMLGBL_PL_MODULE_TRACE_CONTROL1_ID_MASK},
	.State = {XAIEMLGBL_PL_MODULE_TRACE_STATUS_STATE_LSB, XAIEMLGBL_PL_MODULE_TRACE_STATUS_STATE_MASK},
	.ModeSts = {XAIEMLGBL_PL_MODULE_TRACE_STATUS_MODE_LSB, XAIEMLGBL_PL_MODULE_TRACE_STATUS_MODE_MASK},
	.Event = Aie2PlTraceEvent
};

/*
 * Data structure to configure trace module for XAIEGBL_TILE_TYPE_MEMTILE
 * tile type
 */
static const XAie_TraceMod Aie2MemTileTraceMod =
{
	.CtrlRegOff = XAIEMLGBL_MEM_TILE_MODULE_TRACE_CONTROL0,
	.PktConfigRegOff = XAIEMLGBL_MEM_TILE_MODULE_TRACE_CONTROL1,
	.StatusRegOff = XAIEMLGBL_MEM_TILE_MODULE_TRACE_STATUS,
	.EventRegOffs = (u32 []){XAIEMLGBL_MEM_TILE_MODULE_TRACE_EVENT0, XAIEMLGBL_MEM_TILE_MODULE_TRACE_EVENT1},
	.NumTraceSlotIds = 8U,
	.NumEventsPerSlot = 4U,
	.StopEvent = {XAIEMLGBL_MEM_TILE_MODULE_TRACE_CONTROL0_TRACE_STOP_EVENT_LSB, XAIEMLGBL_MEM_TILE_MODULE_TRACE_CONTROL0_TRACE_STOP_EVENT_MASK},
	.StartEvent = {XAIEMLGBL_MEM_TILE_MODULE_TRACE_CONTROL0_TRACE_START_EVENT_LSB, XAIEMLGBL_MEM_TILE_MODULE_TRACE_CONTROL0_TRACE_START_EVENT_MASK},
	.ModeConfig = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
	.PktType = {XAIEMLGBL_MEM_TILE_MODULE_TRACE_CONTROL1_PACKET_TYPE_LSB, XAIEMLGBL_MEM_TILE_MODULE_TRACE_CONTROL1_PACKET_TYPE_MASK},
	.PktId = {XAIEMLGBL_MEM_TILE_MODULE_TRACE_CONTROL1_ID_LSB, XAIEMLGBL_MEM_TILE_MODULE_TRACE_CONTROL1_ID_MASK},
	.State = {XAIEMLGBL_MEM_TILE_MODULE_TRACE_STATUS_STATE_LSB, XAIEMLGBL_MEM_TILE_MODULE_TRACE_STATUS_STATE_MASK},
	.ModeSts = {XAIEMLGBL_MEM_TILE_MODULE_TRACE_STATUS_MODE_LSB, XAIEMLGBL_MEM_TILE_MODULE_TRACE_STATUS_MODE_MASK},
	.Event = Aie2MemTileTraceEvent
};

/*
 * Data structure to configures first level interrupt controller for
 * XAIEGBL_TILE_TYPE_SHIMPL tile type
 */
static const XAie_L1IntrMod Aie2PlL1IntrMod =
{
	.BaseEnableRegOff = XAIEMLGBL_PL_MODULE_INTERRUPT_CONTROLLER_1ST_LEVEL_ENABLE_A,
	.BaseDisableRegOff = XAIEMLGBL_PL_MODULE_INTERRUPT_CONTROLLER_1ST_LEVEL_DISABLE_A,
	.BaseIrqRegOff = XAIEMLGBL_PL_MODULE_INTERRUPT_CONTROLLER_1ST_LEVEL_IRQ_NO_A,
	.BaseIrqEventRegOff = XAIEMLGBL_PL_MODULE_INTERRUPT_CONTROLLER_1ST_LEVEL_IRQ_EVENT_A,
	.BaseIrqEventMask = XAIEMLGBL_PL_MODULE_INTERRUPT_CONTROLLER_1ST_LEVEL_IRQ_EVENT_A_IRQ_EVENT0_MASK,
	.BaseBroadcastBlockRegOff = XAIEMLGBL_PL_MODULE_INTERRUPT_CONTROLLER_1ST_LEVEL_BLOCK_NORTH_IN_A_SET,
	.BaseBroadcastUnblockRegOff = XAIEMLGBL_PL_MODULE_INTERRUPT_CONTROLLER_1ST_LEVEL_BLOCK_NORTH_IN_A_CLEAR,
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
	.EnableRegOff = XAIEMLGBL_NOC_MODULE_INTERRUPT_CONTROLLER_2ND_LEVEL_ENABLE,
	.DisableRegOff = XAIEMLGBL_NOC_MODULE_INTERRUPT_CONTROLLER_2ND_LEVEL_DISABLE,
	.IrqRegOff = XAIEMLGBL_NOC_MODULE_INTERRUPT_CONTROLLER_2ND_LEVEL_INTERRUPT,
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
		.NumModules = 2U,
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
		.NumModules = 1U,
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
		.NumModules = 1U,
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
		.NumModules = 1U,
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
