/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaiegbl_reginit.c
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
* 1.0   Tejus   10/22/2019  Initial creation
* 1.1   Tejus   10/28/2019  Add pl interface register properties
* 1.2   Tejus   12/09/2019  Include correct header file to avoid cyclic
*			    dependancy
* 1.3   Tejus   03/16/2020  Seperate PlIf Module for SHIMPL and SHIMNOC Tiles
* 1.4   Tejus   03/16/2020  Add register properties for Mux/Demux registers
* 1.5   Tejus   03/17/2020  Add lock module properties
* 1.6   Tejus   03/21/2020  Add structure fields to stream switch module
*			    definition
* 1.7   Tejus   03/21/2020  Add register properties for stream switch slot
*			    registers
* 1.8   Tejus   03/23/2020  Organize header files in alphabetical order
* 1.9   Tejus   03/23/2020  Add register properties for dmas
* 2.0   Dishita 03/24/2020  Add performance counter properties
* 2.1   Dishita 04/16/2020  Fix compiler warnings
* 2.2   Dishita 04/20/2020  Add timer properties
* 2.3   Tejus   05/26/2020  Restructure and optimize core module.
* 2.4   Tejus   06/01/2020  Add data structure for core debug register.
* 2.5   Nishad  06/02/2020  Rename included header file xaiegbl_events to
*			    xaie_events_aie
* 2.6   Nishad  06/03/2020  Rename XAIEGBL_<MODULE>_EVENT_* macros to
*			    XAIE_EVENTS_<MODULE>_*
* 2.7   Nishad  06/09/2020  Fix typo in *_MEMORY_* event macros
* 2.8   Tejus   06/05/2020  Populate fifo mode availability in data structure.
* 2.9   Nishad  06/16/2020  Add trace module properties
* 3.0   Nishad  06/28/2020  Populate stream switch port event selection, event
* 			    generation and combo event properties
* 3.1   Nishad  07/01/2020  Populate MstrConfigBaseAddr stream switch property
* 3.2   Nishad  07/12/2020  Populate event broadcast, PC event, and group event
*			    register properties
* 3.3   Nishad  07/21/2020  Populate interrupt controller data structure.
* 3.4   Nishad  07/24/2020  Populate value of default group error mask.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_core_aie.h"
#include "xaie_dma_aie.h"
#include "xaie_events.h"
#include "xaie_events_aie.h"
#include "xaie_locks_aie.h"
#include "xaie_reset_aie.h"
#include "xaiegbl_regdef.h"
#include "xaiegbl_params.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/**************************** Macro Definitions ******************************/

/************************** Variable Definitions *****************************/
/*
 * Global instance for Core module Core_Control register.
 */
static const XAie_RegCoreCtrl AieCoreCtrlReg =
{
	XAIEGBL_CORE_CORECTRL,
	{XAIEGBL_CORE_CORECTRL_ENA_LSB, XAIEGBL_CORE_CORECTRL_ENA_MASK},
	{XAIEGBL_CORE_CORECTRL_RST_LSB, XAIEGBL_CORE_CORECTRL_RST_MASK}
};

/*
 * Global instance for Core module Core_Status register.
 */
static const XAie_RegCoreSts AieCoreStsReg =
{
	XAIEGBL_CORE_CORESTA,
	{XAIEGBL_CORE_CORESTA_COREDON_LSB, XAIEGBL_CORE_CORESTA_COREDON_MASK},
	{XAIEGBL_CORE_CORESTA_RST_LSB, XAIEGBL_CORE_CORESTA_RST_MASK},
	{XAIEGBL_CORE_CORESTA_ENA_LSB, XAIEGBL_CORE_CORESTA_ENA_MASK}
};

/*
 * Global instance for Core module for core debug registers.
 */
static const XAie_RegCoreDebug AieCoreDebugReg =
{
	.RegOff = XAIEGBL_CORE_DBGCTRL0,
	.DebugCtrl1Offset = XAIEGBL_CORE_DBGCTRL1,
	.DebugHaltCoreEvent1.Lsb = XAIEGBL_CORE_DBGCTRL1_DBGHLTCOREEVT1_LSB,
	.DebugHaltCoreEvent1.Mask = XAIEGBL_CORE_DBGCTRL1_DBGHLTCOREEVT1_MASK,
	.DebugHaltCoreEvent0.Lsb = XAIEGBL_CORE_DBGCTRL1_DBGHLTCOREEVT0_LSB,
	.DebugHaltCoreEvent0.Mask = XAIEGBL_CORE_DBGCTRL1_DBGHLTCOREEVT0_MASK,
	.DebugSStepCoreEvent.Lsb = XAIEGBL_CORE_DBGCTRL1_DBGSINCOREEVT_LSB,
	.DebugSStepCoreEvent.Mask = XAIEGBL_CORE_DBGCTRL1_DBGSINCOREEVT_MASK,
	.DebugResumeCoreEvent.Lsb = XAIEGBL_CORE_DBGCTRL1_DBGRESCOREEVT_LSB,
	.DebugResumeCoreEvent.Mask = XAIEGBL_CORE_DBGCTRL1_DBGRESCOREEVT_MASK,
	.DebugHalt.Lsb = XAIEGBL_CORE_DBGCTRL0_DBGHLTBIT_LSB,
	.DebugHalt.Mask = XAIEGBL_CORE_DBGCTRL0_DBGHLTBIT_MASK
};

/*
 * Global instance for core event registers in the core module.
 */
static const XAie_RegCoreEvents AieCoreEventReg =
{
	.EnableEventOff = XAIEGBL_CORE_ENAEVE,
	.DisableEventOccurred.Lsb = XAIEGBL_CORE_ENAEVE_DISEVTOCC_LSB,
	.DisableEventOccurred.Mask = XAIEGBL_CORE_ENAEVE_DISEVTOCC_MASK,
	.EnableEventOccurred.Lsb = XAIEGBL_CORE_ENAEVE_ENAEVTOCC_LSB,
	.EnableEventOccurred.Mask = XAIEGBL_CORE_ENAEVE_ENAEVTOCC_MASK,
	.DisableEvent.Lsb =  XAIEGBL_CORE_ENAEVE_DISEVT_LSB,
	.DisableEvent.Mask =  XAIEGBL_CORE_ENAEVE_DISEVT_MASK,
	.EnableEvent.Lsb = XAIEGBL_CORE_ENAEVE_ENAEVT_LSB,
	.EnableEvent.Mask = XAIEGBL_CORE_ENAEVE_ENAEVT_MASK,
};

/*
 * Array of all Tile Stream Switch Master Config registers
 * The data structure contains number of ports and the register offsets
 */
static const XAie_StrmPort AieTileStrmMstr[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		.NumPorts = 2,
		.PortBaseAddr = XAIEGBL_CORE_STRSWIMSTRCFGMECORE0,
	},
	{	/* DMA */
		.NumPorts = 2,
		.PortBaseAddr = XAIEGBL_CORE_STRSWIMSTRCFGDMA0,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIEGBL_CORE_STRSWIMSTRCFGTILCTR,
	},
	{	/* Fifo */
		.NumPorts = 2,
		.PortBaseAddr = XAIEGBL_CORE_STRSWIMSTRCFGFIF0,
	},
	{	/* South */
		.NumPorts = 4,
		.PortBaseAddr = XAIEGBL_CORE_STRSWIMSTRCFGSOU0,
	},
	{	/* West */
		.NumPorts = 4,
		.PortBaseAddr = XAIEGBL_CORE_STRSWIMSTRCFGWES0,
	},
	{	/* North */
		.NumPorts = 6,
		.PortBaseAddr = XAIEGBL_CORE_STRSWIMSTRCFGNOR0,
	},
	{	/* East */
		.NumPorts = 4,
		.PortBaseAddr = XAIEGBL_CORE_STRSWIMSTRCFGEAS0,
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
static const XAie_StrmPort AieTileStrmSlv[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		.NumPorts = 2,
		.PortBaseAddr = XAIEGBL_CORE_STRSWISLVMECORE0CFG,
	},
	{	/* DMA */
		.NumPorts = 2,
		.PortBaseAddr = XAIEGBL_CORE_STRSWISLVDMA0CFG,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIEGBL_CORE_STRSWISLVTILCTRCFG,
	},
	{	/* Fifo */
		.NumPorts = 2,
		.PortBaseAddr = XAIEGBL_CORE_STRSWISLVFIF0CFG,
	},
	{	/* South */
		.NumPorts = 6,
		.PortBaseAddr = XAIEGBL_CORE_STRSWISLVSOU0CFG,
	},
	{	/* West */
		.NumPorts = 4,
		.PortBaseAddr = XAIEGBL_CORE_STRSWISLVWES0CFG,
	},
	{	/* North */
		.NumPorts = 4,
		.PortBaseAddr = XAIEGBL_CORE_STRSWISLVNOR0CFG,
	},
	{	/* East */
		.NumPorts = 4,
		.PortBaseAddr = XAIEGBL_CORE_STRSWISLVEAS0CFG,
	},
	{	/* Trace */
		.NumPorts = 2,
		.PortBaseAddr = XAIEGBL_CORE_STRSWISLVMETRACFG
	}
};

/*
 * Array of all Shim NOC/PL Stream Switch Master Config registers
 * The data structure contains number of ports and the register offsets
 */
static const XAie_StrmPort AieShimStrmMstr[SS_PORT_TYPE_MAX] =
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
		.PortBaseAddr = XAIEGBL_PL_STRSWIMSTRCFGTILCTR,
	},
	{	/* Fifo */
		.NumPorts = 2,
		.PortBaseAddr = XAIEGBL_PL_STRSWIMSTRCFGFIF0,
	},
	{	/* South */
		.NumPorts = 6,
		.PortBaseAddr = XAIEGBL_PL_STRSWIMSTRCFGSOU0,
	},
	{	/* West */
		.NumPorts = 4,
		.PortBaseAddr = XAIEGBL_PL_STRSWIMSTRCFGWES0,
	},
	{	/* North */
		.NumPorts = 6,
		.PortBaseAddr = XAIEGBL_PL_STRSWIMSTRCFGNOR0,
	},
	{	/* East */
		.NumPorts = 4,
		.PortBaseAddr = XAIEGBL_PL_STRSWIMSTRCFGEAS0,
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
static const XAie_StrmPort AieShimStrmSlv[SS_PORT_TYPE_MAX] =
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
		.PortBaseAddr = XAIEGBL_PL_STRSWISLVTILCTRCFG,
	},
	{	/* Fifo */
		.NumPorts = 2,
		.PortBaseAddr = XAIEGBL_PL_STRSWISLVFIF0CFG,
	},
	{	/* South */
		.NumPorts = 8,
		.PortBaseAddr = XAIEGBL_PL_STRSWISLVSOU0CFG,
	},
	{	/* West */
		.NumPorts = 4,
		.PortBaseAddr = XAIEGBL_PL_STRSWISLVWES0CFG,
	},
	{	/* North */
		.NumPorts = 4,
		.PortBaseAddr = XAIEGBL_PL_STRSWISLVNOR0CFG,
	},
	{	/* East */
		.NumPorts = 4,
		.PortBaseAddr = XAIEGBL_PL_STRSWISLVEAS0CFG,
	},
	{	/* Trace */
		.NumPorts = 1,
		.PortBaseAddr = XAIEGBL_PL_STRSWISLVTRACFG,
	}
};

/*
 * Array of all Shim NOC/PL Stream Switch Slave Slot Config registers
 * The data structure contains number of ports and the register base address.
 */
static const XAie_StrmPort AieShimStrmSlaveSlot[SS_PORT_TYPE_MAX] =
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
		.PortBaseAddr = XAIEGBL_PL_STRSWISLVTILCTRSLO0,
	},
	{	/* Fifo */
		.NumPorts = 2,
		.PortBaseAddr = XAIEGBL_PL_STRSWISLVFIF0SLO0,
	},
	{	/* South */
		.NumPorts = 8,
		.PortBaseAddr = XAIEGBL_PL_STRSWISLVSOU0SLO0,
	},
	{	/* West */
		.NumPorts = 4,
		.PortBaseAddr = XAIEGBL_PL_STRSWISLVWES0SLO0,
	},
	{	/* North */
		.NumPorts = 4,
		.PortBaseAddr = XAIEGBL_PL_STRSWISLVNOR0SLO0,
	},
	{	/* East */
		.NumPorts = 4,
		.PortBaseAddr = XAIEGBL_PL_STRSWISLVEAS0SLO0,
	},
	{	/* Trace */
		.NumPorts = 1,
		.PortBaseAddr = XAIEGBL_PL_STRSWISLVTRASLO0
	}
};

/*
 * Data structure to capture stream switch slave slot register base address for
 * AIE Tiles
 */
static const XAie_StrmPort AieTileStrmSlaveSlot[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		.NumPorts = 2,
		.PortBaseAddr = XAIEGBL_CORE_STRSWISLVMECORE0SLO0,
	},
	{	/* DMA */
		.NumPorts = 2,
		.PortBaseAddr = XAIEGBL_CORE_STRSWISLVDMA0SLO0,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIEGBL_CORE_STRSWISLVTILCTRSLO0,
	},
	{	/* Fifo */
		.NumPorts = 2,
		.PortBaseAddr = XAIEGBL_CORE_STRSWISLVFIF0SLO0,
	},
	{	/* South */
		.NumPorts = 6,
		.PortBaseAddr = XAIEGBL_CORE_STRSWISLVSOU0SLO0,
	},
	{	/* West */
		.NumPorts = 4,
		.PortBaseAddr = XAIEGBL_CORE_STRSWISLVWES0SLO0,
	},
	{	/* North */
		.NumPorts = 4,
		.PortBaseAddr = XAIEGBL_CORE_STRSWISLVNOR0SLO0,
	},
	{	/* East */
		.NumPorts = 4,
		.PortBaseAddr = XAIEGBL_CORE_STRSWISLVEAS0SLO0,
	},
	{	/* Trace */
		.NumPorts = 2,
		.PortBaseAddr = XAIEGBL_CORE_STRSWISLVMETRASLO0
	}
};

static const XAie_StrmSwPortMap AieTileStrmSwMasterPortMap[] =
{
	{
		/* PhyPort 0 */
		.PortType = CORE,
		.PortNum = 0,
	},
	{
		/* PhyPort 1 */
		.PortType = CORE,
		.PortNum = 1,
	},
	{
		/* PhyPort 2 */
		.PortType = DMA,
		.PortNum = 0,
	},
	{
		/* PhyPort 3 */
		.PortType = DMA,
		.PortNum = 1,
	},
	{
		/* PhyPort 4 */
		.PortType = CTRL,
		.PortNum = 0,
	},
	{
		/* PhyPort 5 */
		.PortType = FIFO,
		.PortNum = 0,
	},
	{
		/* PhyPort 6 */
		.PortType = FIFO,
		.PortNum = 1,
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
		.PortType = NORTH,
		.PortNum = 4,
	},
	{
		/* PhyPort 20 */
		.PortType = NORTH,
		.PortNum = 5,
	},
	{
		/* PhyPort 21 */
		.PortType = EAST,
		.PortNum = 0,
	},
	{
		/* PhyPort 22 */
		.PortType = EAST,
		.PortNum = 1,
	},
	{
		/* PhyPort 23 */
		.PortType = EAST,
		.PortNum = 2,
	},
	{
		/* PhyPort 24 */
		.PortType = EAST,
		.PortNum = 3,
	},
};

static const XAie_StrmSwPortMap AieTileStrmSwSlavePortMap[] =
{
	{
		/* PhyPort 0 */
		.PortType = CORE,
		.PortNum = 0,
	},
	{
		/* PhyPort 1 */
		.PortType = CORE,
		.PortNum = 1,
	},
	{
		/* PhyPort 2 */
		.PortType = DMA,
		.PortNum = 0,
	},
	{
		/* PhyPort 3 */
		.PortType = DMA,
		.PortNum = 1,
	},
	{
		/* PhyPort 4 */
		.PortType = CTRL,
		.PortNum = 0,
	},
	{
		/* PhyPort 5 */
		.PortType = FIFO,
		.PortNum = 0,
	},
	{
		/* PhyPort 6 */
		.PortType = FIFO,
		.PortNum = 1,
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
		.PortType = SOUTH,
		.PortNum = 4,
	},
	{
		/* PhyPort 12 */
		.PortType = SOUTH,
		.PortNum = 5,
	},
	{
		/* PhyPort 13 */
		.PortType = WEST,
		.PortNum = 0,
	},
	{
		/* PhyPort 14 */
		.PortType = WEST,
		.PortNum = 1,
	},
	{
		/* PhyPort 15 */
		.PortType = WEST,
		.PortNum = 2,
	},
	{
		/* PhyPort 16 */
		.PortType = WEST,
		.PortNum = 3,
	},
	{
		/* PhyPort 17 */
		.PortType = NORTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 18 */
		.PortType = NORTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 19 */
		.PortType = NORTH,
		.PortNum = 2,
	},
	{
		/* PhyPort 20 */
		.PortType = NORTH,
		.PortNum = 3,
	},
	{
		/* PhyPort 21 */
		.PortType = EAST,
		.PortNum = 0,
	},
	{
		/* PhyPort 22 */
		.PortType = EAST,
		.PortNum = 1,
	},
	{
		/* PhyPort 23 */
		.PortType = EAST,
		.PortNum = 2,
	},
	{
		/* PhyPort 24 */
		.PortType = EAST,
		.PortNum = 3,
	},
	{
		/* PhyPort 25 */
		.PortType = TRACE,
		.PortNum = 0,
	},
	{
		/* PhyPort 26 */
		.PortType = TRACE,
		.PortNum = 1,
	},
};

static const XAie_StrmSwPortMap AieShimStrmSwMasterPortMap[] =
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
		.PortType = FIFO,
		.PortNum = 1,
	},
	{
		/* PhyPort 3 */
		.PortType = SOUTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 4 */
		.PortType = SOUTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 5 */
		.PortType = SOUTH,
		.PortNum = 2,
	},
	{
		/* PhyPort 6 */
		.PortType = SOUTH,
		.PortNum = 3,
	},
	{
		/* PhyPort 7 */
		.PortType = SOUTH,
		.PortNum = 4,
	},
	{
		/* PhyPort 8 */
		.PortType = SOUTH,
		.PortNum = 5,
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

static const XAie_StrmSwPortMap AieShimStrmSwSlavePortMap[] =
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
		.PortType = FIFO,
		.PortNum = 1,
	},
	{
		/* PhyPort 3 */
		.PortType = SOUTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 4 */
		.PortType = SOUTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 5 */
		.PortType = SOUTH,
		.PortNum = 2,
	},
	{
		/* PhyPort 6 */
		.PortType = SOUTH,
		.PortNum = 3,
	},
	{
		/* PhyPort 7 */
		.PortType = SOUTH,
		.PortNum = 4,
	},
	{
		/* PhyPort 8 */
		.PortType = SOUTH,
		.PortNum = 5,
	},
	{
		/* PhyPort 9 */
		.PortType = SOUTH,
		.PortNum = 6,
	},
	{
		/* PhyPort 10 */
		.PortType = SOUTH,
		.PortNum = 7,
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
};

/*
 * Data structure to capture all stream configs for XAIEGBL_TILE_TYPE_AIETILE
 */
static const XAie_StrmMod AieTileStrmSw =
{
	.SlvConfigBaseAddr = XAIEGBL_CORE_STRSWISLVMECORE0CFG,
	.MstrConfigBaseAddr = XAIEGBL_CORE_STRSWIMSTRCFGMECORE0,
	.PortOffset = 0x4,
	.NumSlaveSlots = 4U,
	.SlotOffsetPerPort = 0x10,
	.SlotOffset = 0x4,
	.MstrEn = {XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_MSTRENA_LSB, XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_MSTRENA_MASK},
	.MstrPktEn = {XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_PKTENA_LSB, XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_PKTENA_MASK},
	.DrpHdr = {XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_DROHEA_LSB, XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_DROHEA_MASK},
	.Config = {XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_CON_LSB, XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_CON_MASK},
	.SlvEn = {XAIEGBL_CORE_STRSWISLVMECORE0CFG_SLVENA_LSB, XAIEGBL_CORE_STRSWISLVMECORE0CFG_SLVENA_MASK},
	.SlvPktEn = {XAIEGBL_CORE_STRSWISLVMECORE0CFG_PKTENA_LSB, XAIEGBL_CORE_STRSWISLVMECORE0CFG_PKTENA_MASK},
	.SlotPktId = {XAIEGBL_CORE_STRSWISLVMECORE0SLO0_ID_LSB, XAIEGBL_CORE_STRSWISLVMECORE0SLO0_ID_MASK},
	.SlotMask = {XAIEGBL_CORE_STRSWISLVMECORE0SLO0_MSK_LSB, XAIEGBL_CORE_STRSWISLVMECORE0SLO0_MSK_MASK},
	.SlotEn = {XAIEGBL_CORE_STRSWISLVMECORE0SLO0_ENA_LSB, XAIEGBL_CORE_STRSWISLVMECORE0SLO0_ENA_MASK},
	.SlotMsel = {XAIEGBL_CORE_STRSWISLVMECORE0SLO0_MSE_LSB, XAIEGBL_CORE_STRSWISLVMECORE0SLO0_MSE_MASK},
	.SlotArbitor = {XAIEGBL_CORE_STRSWISLVMECORE0SLO0_ARB_LSB, XAIEGBL_CORE_STRSWISLVMECORE0SLO0_ARB_MASK},
	.MstrConfig = AieTileStrmMstr,
	.SlvConfig = AieTileStrmSlv,
	.SlvSlotConfig = AieTileStrmSlaveSlot,
	.MaxMasterPhyPortId = 24U,
	.MaxSlavePhyPortId = 26U,
	.MasterPortMap = AieTileStrmSwMasterPortMap,
	.SlavePortMap = AieTileStrmSwSlavePortMap,
};

/*
 * Data structure to capture all stream configs for XAIEGBL_TILE_TYPE_SHIMNOC/PL
 */
static const XAie_StrmMod AieShimStrmSw =
{
	.SlvConfigBaseAddr = XAIEGBL_PL_STRSWISLVTILCTRCFG,
	.MstrConfigBaseAddr = XAIEGBL_PL_STRSWIMSTRCFGTILCTR,
	.PortOffset = 0x4,
	.NumSlaveSlots = 4U,
	.SlotOffsetPerPort = 0x10,
	.SlotOffset = 0x4,
	.MstrEn = {XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_MSTRENA_LSB, XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_MSTRENA_MASK},
	.MstrPktEn = {XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_PKTENA_LSB, XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_PKTENA_MASK},
	.DrpHdr = {XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_DROHEA_LSB, XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_DROHEA_MASK},
	.Config = {XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_CON_LSB, XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_CON_MASK},
	.SlvEn = {XAIEGBL_CORE_STRSWISLVMECORE0CFG_SLVENA_LSB, XAIEGBL_CORE_STRSWISLVMECORE0CFG_SLVENA_MASK},
	.SlvPktEn = {XAIEGBL_CORE_STRSWISLVMECORE0CFG_PKTENA_LSB, XAIEGBL_CORE_STRSWISLVMECORE0CFG_PKTENA_MASK},
	.SlotPktId = {XAIEGBL_CORE_STRSWISLVMECORE0SLO0_ID_LSB, XAIEGBL_CORE_STRSWISLVMECORE0SLO0_ID_MASK},
	.SlotMask = {XAIEGBL_CORE_STRSWISLVMECORE0SLO0_MSK_LSB, XAIEGBL_CORE_STRSWISLVMECORE0SLO0_MSK_MASK},
	.SlotEn = {XAIEGBL_CORE_STRSWISLVMECORE0SLO0_ENA_LSB, XAIEGBL_CORE_STRSWISLVMECORE0SLO0_ENA_MASK},
	.SlotMsel = {XAIEGBL_CORE_STRSWISLVMECORE0SLO0_MSE_LSB, XAIEGBL_CORE_STRSWISLVMECORE0SLO0_MSE_MASK},
	.SlotArbitor = {XAIEGBL_CORE_STRSWISLVMECORE0SLO0_ARB_LSB, XAIEGBL_CORE_STRSWISLVMECORE0SLO0_ARB_MASK},
	.MstrConfig = AieShimStrmMstr,
	.SlvConfig = AieShimStrmSlv,
	.SlvSlotConfig = AieShimStrmSlaveSlot,
	.MaxMasterPhyPortId = 22U,
	.MaxSlavePhyPortId = 23U,
	.MasterPortMap = AieShimStrmSwMasterPortMap,
	.SlavePortMap = AieShimStrmSwSlavePortMap,
};

/* Register field attributes for PL interface down sizer for 32 and 64 bits */
static const XAie_RegFldAttr AieDownSzr32_64Bit[] =
{
	{XAIEGBL_PL_PLINTDOWCFG_SOU0_LSB, XAIEGBL_PL_PLINTDOWCFG_SOU0_MASK},
	{XAIEGBL_PL_PLINTDOWCFG_SOU1_LSB, XAIEGBL_PL_PLINTDOWCFG_SOU1_MASK},
	{XAIEGBL_PL_PLINTDOWCFG_SOU2_LSB, XAIEGBL_PL_PLINTDOWCFG_SOU2_MASK},
	{XAIEGBL_PL_PLINTDOWCFG_SOU3_LSB, XAIEGBL_PL_PLINTDOWCFG_SOU3_MASK},
	{XAIEGBL_PL_PLINTDOWCFG_SOU4_LSB, XAIEGBL_PL_PLINTDOWCFG_SOU4_MASK},
	{XAIEGBL_PL_PLINTDOWCFG_SOU5_LSB, XAIEGBL_PL_PLINTDOWCFG_SOU5_MASK},
	{XAIEGBL_PL_PLINTDOWCFG_SOU6_LSB, XAIEGBL_PL_PLINTDOWCFG_SOU6_MASK},
	{XAIEGBL_PL_PLINTDOWCFG_SOU7_LSB, XAIEGBL_PL_PLINTDOWCFG_SOU7_MASK}
};

/* Register field attributes for PL interface down sizer for 128 bits */
static const XAie_RegFldAttr AieDownSzr128Bit[] =
{
	{XAIEGBL_PL_PLINTDOWCFG_SOU0SOU1128COM_LSB, XAIEGBL_PL_PLINTDOWCFG_SOU0SOU1128COM_MASK},
	{XAIEGBL_PL_PLINTDOWCFG_SOU2SOU3128COM_LSB, XAIEGBL_PL_PLINTDOWCFG_SOU2SOU3128COM_MASK},
	{XAIEGBL_PL_PLINTDOWCFG_SOU4SOU5128COM_LSB, XAIEGBL_PL_PLINTDOWCFG_SOU4SOU5128COM_MASK},
	{XAIEGBL_PL_PLINTDOWCFG_SOU6SOU7128COM_LSB, XAIEGBL_PL_PLINTDOWCFG_SOU6SOU7128COM_MASK}
};

/* Register field attributes for PL interface up sizer */
static const XAie_RegFldAttr AieUpSzr32_64Bit[] =
{
	{XAIEGBL_PL_PLINTUPSCFG_SOU0_LSB, XAIEGBL_PL_PLINTUPSCFG_SOU0_MASK},
	{XAIEGBL_PL_PLINTUPSCFG_SOU1_LSB, XAIEGBL_PL_PLINTUPSCFG_SOU1_MASK},
	{XAIEGBL_PL_PLINTUPSCFG_SOU2_LSB, XAIEGBL_PL_PLINTUPSCFG_SOU2_MASK},
	{XAIEGBL_PL_PLINTUPSCFG_SOU3_LSB, XAIEGBL_PL_PLINTUPSCFG_SOU3_MASK},
	{XAIEGBL_PL_PLINTUPSCFG_SOU4_LSB, XAIEGBL_PL_PLINTUPSCFG_SOU4_MASK},
	{XAIEGBL_PL_PLINTUPSCFG_SOU5_LSB, XAIEGBL_PL_PLINTUPSCFG_SOU5_MASK},
};

/* Register field attributes for PL interface up sizer for 128 bits */
static const XAie_RegFldAttr AieUpSzr128Bit[] =
{
	{XAIEGBL_PL_PLINTUPSCFG_SOU0SOU1128COM_LSB, XAIEGBL_PL_PLINTUPSCFG_SOU0SOU1128COM_MASK},
	{XAIEGBL_PL_PLINTUPSCFG_SOU2SOU3128COM_LSB, XAIEGBL_PL_PLINTUPSCFG_SOU2SOU3128COM_MASK},
	{XAIEGBL_PL_PLINTUPSCFG_SOU4SOU5128COM_LSB, XAIEGBL_PL_PLINTUPSCFG_SOU4SOU5128COM_MASK},
};

/* Register field attributes for PL interface down sizer bypass */
static const XAie_RegFldAttr AieDownSzrByPass[] =
{
	{XAIEGBL_PL_PLINTDOWBYPASS_SOU0_LSB, XAIEGBL_PL_PLINTDOWBYPASS_SOU0_MASK},
	{XAIEGBL_PL_PLINTDOWBYPASS_SOU1_LSB, XAIEGBL_PL_PLINTDOWBYPASS_SOU1_MASK},
	{XAIEGBL_PL_PLINTDOWBYPASS_SOU2_LSB, XAIEGBL_PL_PLINTDOWBYPASS_SOU2_MASK},
	{XAIEGBL_PL_PLINTDOWBYPASS_SOU4_LSB, XAIEGBL_PL_PLINTDOWBYPASS_SOU4_MASK},
	{XAIEGBL_PL_PLINTDOWBYPASS_SOU5_LSB, XAIEGBL_PL_PLINTDOWBYPASS_SOU5_MASK},
	{XAIEGBL_PL_PLINTDOWBYPASS_SOU6_LSB, XAIEGBL_PL_PLINTDOWBYPASS_SOU6_MASK},
};

/* Register field attributes for PL interface down sizer enable */
static const XAie_RegFldAttr AieDownSzrEnable[] =
{
	{XAIEGBL_PL_PLINTDOWENA_SOU0_LSB, XAIEGBL_PL_PLINTDOWENA_SOU0_MASK},
	{XAIEGBL_PL_PLINTDOWENA_SOU1_LSB, XAIEGBL_PL_PLINTDOWENA_SOU1_MASK},
	{XAIEGBL_PL_PLINTDOWENA_SOU2_LSB, XAIEGBL_PL_PLINTDOWENA_SOU2_MASK},
	{XAIEGBL_PL_PLINTDOWENA_SOU3_LSB, XAIEGBL_PL_PLINTDOWENA_SOU3_MASK},
	{XAIEGBL_PL_PLINTDOWENA_SOU4_LSB, XAIEGBL_PL_PLINTDOWENA_SOU4_MASK},
	{XAIEGBL_PL_PLINTDOWENA_SOU5_LSB, XAIEGBL_PL_PLINTDOWENA_SOU5_MASK},
	{XAIEGBL_PL_PLINTDOWENA_SOU6_LSB, XAIEGBL_PL_PLINTDOWENA_SOU6_MASK},
	{XAIEGBL_PL_PLINTDOWENA_SOU7_LSB, XAIEGBL_PL_PLINTDOWENA_SOU7_MASK}
};

/* Register field attributes for SHIMNOC Mux configuration */
static const XAie_RegFldAttr AieShimMuxConfig[] =
{
	{XAIEGBL_NOC_MUXCFG_SOU2_LSB, XAIEGBL_NOC_MUXCFG_SOU2_MASK},
	{XAIEGBL_NOC_MUXCFG_SOU3_LSB, XAIEGBL_NOC_MUXCFG_SOU3_MASK},
	{XAIEGBL_NOC_MUXCFG_SOU6_LSB, XAIEGBL_NOC_MUXCFG_SOU6_MASK},
	{XAIEGBL_NOC_MUXCFG_SOU7_LSB, XAIEGBL_NOC_MUXCFG_SOU7_MASK},
};

/* Register field attributes for SHIMNOC DeMux configuration */
static const XAie_RegFldAttr AieShimDeMuxConfig[] =
{
	{XAIEGBL_NOC_DEMCFG_SOU2_LSB, XAIEGBL_NOC_DEMCFG_SOU2_MASK},
	{XAIEGBL_NOC_DEMCFG_SOU3_LSB, XAIEGBL_NOC_DEMCFG_SOU3_MASK},
	{XAIEGBL_NOC_DEMCFG_SOU4_LSB, XAIEGBL_NOC_DEMCFG_SOU4_MASK},
	{XAIEGBL_NOC_DEMCFG_SOU5_LSB, XAIEGBL_NOC_DEMCFG_SOU5_MASK},
};

/* Register to set SHIM clock buffer control */
static const XAie_ShimClkBufCntr AieShimClkBufCntr =
{
	.RegOff = XAIEGBL_PL_TILCLOCTRL,
	.RstEnable = XAIE_ENABLE,
	.ClkBufEnable = {XAIEGBL_PL_TILCLOCTRL_CLOBUFENA_LSB, XAIEGBL_PL_TILCLOCTRL_CLOBUFENA_MASK}
};

/* Register to reset SHIM tile */
static const XAie_ShimRstMod AieShimTileRst =
{
	.RegOff = XAIEGBL_PL_AIESHIRSTENA,
	.RstCntr = {XAIEGBL_PL_AIESHIRSTENA_RST_LSB, XAIEGBL_PL_AIESHIRSTENA_RST_MASK},
	.RstShims = _XAie_RstShims,
};

/* Register field attributes for SHIMNOC AXI MM configuration */
static const XAie_ShimNocAxiMMConfig AieShimNocAxiMMConfig =
{
	.RegOff = 0x1E020,
	.NsuSlvErr = {2, 0x4},
	.NsuDecErr = {3, 0x8}
};

/* Core Module */
static const XAie_CoreMod AieCoreMod =
{
	.IsCheckerBoard = 1U,
	.ProgMemAddr = 0x0,
	.ProgMemSize = 16 * 1024,
	.DataMemAddr = 0x20000,
	.ProgMemHostOffset = XAIEGBL_CORE_PRGMEM,
	.DataMemSize = 32 * 1024,
	.DataMemShift = 15U,
	.EccEvntRegOff = 0x00032110,
	.CoreCtrl = &AieCoreCtrlReg,
	.CoreSts = &AieCoreStsReg,
	.CoreDebug = &AieCoreDebugReg,
	.CoreEvent = &AieCoreEventReg,
	.ConfigureDone = &_XAie_CoreConfigureDone,
	.Enable = &_XAie_CoreEnable,
	.WaitForDone = &_XAie_CoreWaitForDone,
	.ReadDoneBit = &_XAie_CoreReadDoneBit,
};

/* Data Memory Module for Tile data memory*/
static const XAie_MemMod AieTileMemMod =
{
	.Size = 32 * 1024,
	.MemAddr = XAIEGBL_MEM_DATMEM,
	.EccEvntRegOff = 0x00012110,
};

/* PL Interface module for SHIMPL Tiles */
static const XAie_PlIfMod AiePlIfMod =
{
	.UpSzrOff = XAIEGBL_PL_PLINTUPSCFG,
	.DownSzrOff = XAIEGBL_PL_PLINTDOWCFG,
	.DownSzrEnOff = XAIEGBL_PL_PLINTDOWENA,
	.DownSzrByPassOff = XAIEGBL_PL_PLINTDOWBYPASS,
	.ColRstOff = XAIEGBL_PL_AIETILCOLRST,
	.NumUpSzrPorts = 0x6,
	.MaxByPassPortNum = 0x6,
	.NumDownSzrPorts = 0x8,
	.UpSzr32_64Bit = AieUpSzr32_64Bit,
	.UpSzr128Bit = AieUpSzr128Bit,
	.DownSzr32_64Bit = AieDownSzr32_64Bit,
	.DownSzr128Bit = AieDownSzr128Bit,
	.DownSzrEn = AieDownSzrEnable,
	.DownSzrByPass = AieDownSzrByPass,
	.ShimNocMuxOff = 0x0,
	.ShimNocDeMuxOff = 0x0,
	.ShimNocMux = NULL,
	.ShimNocDeMux = NULL,
	.ClkBufCntr = &AieShimClkBufCntr,
	.ColRst = {XAIEGBL_PL_AIETILCOLRST_RST_LSB, XAIEGBL_PL_AIETILCOLRST_RST_MASK},
	.ShimTileRst = &AieShimTileRst,
	.ShimNocAxiMM = NULL,
};

/* PL Interface module for SHIMNOC Tiles */
static const XAie_PlIfMod AieShimTilePlIfMod =
{
	.UpSzrOff = XAIEGBL_PL_PLINTUPSCFG,
	.DownSzrOff = XAIEGBL_PL_PLINTDOWCFG,
	.DownSzrEnOff = XAIEGBL_PL_PLINTDOWENA,
	.DownSzrByPassOff = XAIEGBL_PL_PLINTDOWBYPASS,
	.ColRstOff = XAIEGBL_PL_AIETILCOLRST,
	.NumUpSzrPorts = 0x6,
	.MaxByPassPortNum = 0x6,
	.NumDownSzrPorts = 0x8,
	.UpSzr32_64Bit = AieUpSzr32_64Bit,
	.UpSzr128Bit = AieUpSzr128Bit,
	.DownSzr32_64Bit = AieDownSzr32_64Bit,
	.DownSzr128Bit = AieDownSzr128Bit,
	.DownSzrEn = AieDownSzrEnable,
	.DownSzrByPass = AieDownSzrByPass,
	.ShimNocMuxOff = XAIEGBL_NOC_MUXCFG,
	.ShimNocDeMuxOff = XAIEGBL_NOC_DEMCFG,
	.ShimNocMux = AieShimMuxConfig,
	.ShimNocDeMux = AieShimDeMuxConfig,
	.ClkBufCntr = &AieShimClkBufCntr,
	.ColRst = {XAIEGBL_PL_AIETILCOLRST_RST_LSB, XAIEGBL_PL_AIETILCOLRST_RST_MASK},
	.ShimTileRst = &AieShimTileRst,
	.ShimNocAxiMM = &AieShimNocAxiMMConfig,
};

/* Lock Module for AIE Tiles  */
static const XAie_LockMod AieTileLockMod =
{
	.BaseAddr = XAIEGBL_MEM_LOCK0RELNV,
	.NumLocks = 16U,
	.LockIdOff = 0x80,
	.RelAcqOff = 0x40,
	.LockValOff = 0x10,
	.LockValUpperBound = 1,
	.LockValLowerBound = -1,
	.Acquire = &(_XAie_LockAcquire),
	.Release = &(_XAie_LockRelease)
};

/* Lock Module for SHIM NOC Tiles  */
static const XAie_LockMod AieShimNocLockMod =
{
	.BaseAddr = XAIEGBL_NOC_LOCK0RELNV,
	.NumLocks = 16U,
	.LockIdOff = 0x80,
	.RelAcqOff = 0x40,
	.LockValOff = 0x10,
	.LockValUpperBound = 1,
	.LockValLowerBound = -1,
	.Acquire = &(_XAie_LockAcquire),
	.Release = &(_XAie_LockRelease)
};

static const XAie_DmaBdEnProp AieTileDmaBdEnProp =
{
	.NxtBd.Idx = 6U,
	.NxtBd.Lsb = XAIEGBL_MEM_DMABD0CTRL_NEXBD_LSB,
	.NxtBd.Mask = XAIEGBL_MEM_DMABD0CTRL_NEXBD_MASK,
	.UseNxtBd.Idx = 6U,
	.UseNxtBd.Lsb = XAIEGBL_MEM_DMABD0CTRL_USENEXBD_LSB,
	.UseNxtBd.Mask = XAIEGBL_MEM_DMABD0CTRL_USENEXBD_MASK,
	.ValidBd.Idx = 6U,
	.ValidBd.Lsb = XAIEGBL_MEM_DMABD0CTRL_VALBD_LSB,
	.ValidBd.Mask = XAIEGBL_MEM_DMABD0CTRL_VALBD_MASK,
};

static const XAie_DmaBdPkt AieTileDmaBdPktProp =
{
	.EnPkt.Idx = 4U,
	.EnPkt.Lsb = XAIEGBL_MEM_DMABD0CTRL_ENAPKT_LSB,
	.EnPkt.Mask = XAIEGBL_MEM_DMABD0CTRL_ENAPKT_MASK,
	.PktType.Idx = 4U,
	.PktType.Lsb = XAIEGBL_MEM_DMABD0PKT_PKTTYP_LSB,
	.PktType.Mask = XAIEGBL_MEM_DMABD0PKT_PKTTYP_MASK,
	.PktId.Idx = 4U,
	.PktId.Lsb = XAIEGBL_MEM_DMABD0PKT_ID_LSB,
	.PktId.Mask = XAIEGBL_MEM_DMABD0PKT_ID_MASK
};

static const XAie_DmaBdLock AieTileDmaLockProp =
{
	.AieDmaLock.LckId_A.Idx = 0U,
	.AieDmaLock.LckId_A.Lsb = XAIEGBL_MEM_DMABD0ADDA_LOCKIDA_LSB,
	.AieDmaLock.LckId_A.Mask = XAIEGBL_MEM_DMABD0ADDA_LOCKIDA_MASK,
	.AieDmaLock.LckId_B.Idx = 1U,
	.AieDmaLock.LckId_B.Lsb = XAIEGBL_MEM_DMABD0ADDB_LOCKIDB_LSB,
	.AieDmaLock.LckId_B.Mask = XAIEGBL_MEM_DMABD0ADDB_LOCKIDB_MASK,
	.AieDmaLock.LckRelEn_A.Idx = 0U,
	.AieDmaLock.LckRelEn_A.Lsb = XAIEGBL_MEM_DMABD0ADDA_ENAREL_LSB,
	.AieDmaLock.LckRelEn_A.Mask = XAIEGBL_MEM_DMABD0ADDA_ENAREL_MASK,
	.AieDmaLock.LckRelVal_A.Idx = 0U,
	.AieDmaLock.LckRelVal_A.Lsb = XAIEGBL_MEM_DMABD0ADDA_RELVALA_LSB,
	.AieDmaLock.LckRelVal_A.Mask = XAIEGBL_MEM_DMABD0ADDA_RELVALA_MASK,
	.AieDmaLock.LckRelUseVal_A.Idx = 0U,
	.AieDmaLock.LckRelUseVal_A.Lsb = XAIEGBL_MEM_DMABD0ADDA_USERELVALA_LSB,
	.AieDmaLock.LckRelUseVal_A.Mask = XAIEGBL_MEM_DMABD0ADDA_USERELVALA_MASK,
	.AieDmaLock.LckAcqEn_A.Idx = 0U,
	.AieDmaLock.LckAcqEn_A.Lsb = XAIEGBL_MEM_DMABD0ADDA_ENAACQ_LSB,
	.AieDmaLock.LckAcqEn_A.Mask = XAIEGBL_MEM_DMABD0ADDA_ENAACQ_MASK,
	.AieDmaLock.LckAcqVal_A.Idx = 0U,
	.AieDmaLock.LckAcqVal_A.Lsb = XAIEGBL_MEM_DMABD0ADDA_ACQVALA_LSB,
	.AieDmaLock.LckAcqVal_A.Mask = XAIEGBL_MEM_DMABD0ADDA_ACQVALA_MASK,
	.AieDmaLock.LckAcqUseVal_A.Idx = 0U,
	.AieDmaLock.LckAcqUseVal_A.Lsb = XAIEGBL_MEM_DMABD0ADDA_USEACQVALA_LSB,
	.AieDmaLock.LckAcqUseVal_A.Mask = XAIEGBL_MEM_DMABD0ADDA_USEACQVALA_MASK,
	.AieDmaLock.LckRelEn_B.Idx = 1U,
	.AieDmaLock.LckRelEn_B.Lsb = XAIEGBL_MEM_DMABD0ADDB_ENAREL_LSB,
	.AieDmaLock.LckRelEn_B.Mask = XAIEGBL_MEM_DMABD0ADDB_ENAREL_MASK,
	.AieDmaLock.LckRelVal_B.Idx = 1U,
	.AieDmaLock.LckRelVal_B.Lsb = XAIEGBL_MEM_DMABD0ADDB_RELVALB_LSB,
	.AieDmaLock.LckRelVal_B.Mask = XAIEGBL_MEM_DMABD0ADDB_RELVALB_MASK,
	.AieDmaLock.LckRelUseVal_B.Idx = 1U,
	.AieDmaLock.LckRelUseVal_B.Lsb = XAIEGBL_MEM_DMABD0ADDB_USERELVALB_LSB,
	.AieDmaLock.LckRelUseVal_B.Mask = XAIEGBL_MEM_DMABD0ADDB_USERELVALB_MASK,
	.AieDmaLock.LckAcqEn_B.Idx = 1U,
	.AieDmaLock.LckAcqEn_B.Lsb = XAIEGBL_MEM_DMABD0ADDB_ENAACQ_LSB,
	.AieDmaLock.LckAcqEn_B.Mask = XAIEGBL_MEM_DMABD0ADDB_ENAACQ_MASK,
	.AieDmaLock.LckAcqVal_B.Idx = 1U,
	.AieDmaLock.LckAcqVal_B.Lsb = XAIEGBL_MEM_DMABD0ADDB_ACQVALB_LSB,
	.AieDmaLock.LckAcqVal_B.Mask = XAIEGBL_MEM_DMABD0ADDB_ACQVALB_MASK,
	.AieDmaLock.LckAcqUseVal_B.Idx = 1U,
	.AieDmaLock.LckAcqUseVal_B.Lsb = XAIEGBL_MEM_DMABD0ADDB_USEACQVALB_LSB,
	.AieDmaLock.LckAcqUseVal_B.Mask = XAIEGBL_MEM_DMABD0ADDB_USEACQVALB_MASK,
};

static const XAie_DmaBdBuffer AieTileDmaBufferProp =
{
	.TileDmaBuff.BaseAddr.Idx = 0U,
	.TileDmaBuff.BaseAddr.Lsb = XAIEGBL_MEM_DMABD0ADDA_BASADDA_LSB,
	.TileDmaBuff.BaseAddr.Mask = XAIEGBL_MEM_DMABD0ADDA_BASADDA_MASK,
	.TileDmaBuff.BufferLen.Idx = 6U,
	.TileDmaBuff.BufferLen.Lsb = XAIEGBL_MEM_DMABD0CTRL_LEN_LSB,
	.TileDmaBuff.BufferLen.Mask = XAIEGBL_MEM_DMABD0CTRL_LEN_MASK,
};

static const XAie_DmaBdDoubleBuffer AieTileDmaDoubleBufferProp =
{
	.EnDoubleBuff.Idx = 6U,
	.EnDoubleBuff.Lsb = XAIEGBL_MEM_DMABD0CTRL_ENAABMOD_LSB,
	.EnDoubleBuff.Mask = XAIEGBL_MEM_DMABD0CTRL_ENAABMOD_MASK,
	.BaseAddr_B.Idx = 1U,
	.BaseAddr_B.Lsb = XAIEGBL_MEM_DMABD0ADDB_BASADDB_LSB,
	.BaseAddr_B.Mask = XAIEGBL_MEM_DMABD0ADDB_BASADDB_MASK,
	.FifoMode.Idx = 6U,
	.FifoMode.Lsb = XAIEGBL_MEM_DMABD0CTRL_ENAFIFMOD_LSB,
	.FifoMode.Mask = XAIEGBL_MEM_DMABD0CTRL_ENAFIFMOD_MASK,
	.EnIntrleaved.Idx = 6U,
	.EnIntrleaved.Lsb = XAIEGBL_MEM_DMABD0CTRL_ENAINT_LSB,
	.EnIntrleaved.Mask = XAIEGBL_MEM_DMABD0CTRL_ENAINT_MASK,
	.IntrleaveCnt.Idx = 6U,
	.IntrleaveCnt.Lsb = XAIEGBL_MEM_DMABD0CTRL_INTCNT_LSB,
	.IntrleaveCnt.Mask = XAIEGBL_MEM_DMABD0CTRL_INTCNT_MASK,
	.BuffSelect.Idx = 5U,
	.BuffSelect.Lsb = XAIEGBL_MEM_DMABD0INTSTA_AB_LSB,
	.BuffSelect.Mask = XAIEGBL_MEM_DMABD0INTSTA_AB_MASK
};

static const XAie_DmaBdMultiDimAddr AieTileDmaMultiDimProp =
{
	.AieMultiDimAddr.X_Incr.Idx = 2U,
	.AieMultiDimAddr.X_Incr.Lsb = XAIEGBL_MEM_DMABD02DX_XINC_LSB,
	.AieMultiDimAddr.X_Incr.Mask = XAIEGBL_MEM_DMABD02DX_XINC_MASK,
	.AieMultiDimAddr.X_Wrap.Idx = 2U,
	.AieMultiDimAddr.X_Wrap.Lsb = XAIEGBL_MEM_DMABD02DX_XWRA_LSB,
	.AieMultiDimAddr.X_Wrap.Mask = XAIEGBL_MEM_DMABD02DX_XWRA_MASK,
	.AieMultiDimAddr.X_Offset.Idx = 2U,
	.AieMultiDimAddr.X_Offset.Lsb = XAIEGBL_MEM_DMABD02DX_XOFF_LSB,
	.AieMultiDimAddr.X_Offset.Mask = XAIEGBL_MEM_DMABD02DX_XOFF_MASK,
	.AieMultiDimAddr.Y_Incr.Idx = 3U,
	.AieMultiDimAddr.Y_Incr.Lsb = XAIEGBL_MEM_DMABD02DY_YINC_LSB,
	.AieMultiDimAddr.Y_Incr.Mask = XAIEGBL_MEM_DMABD02DY_YINC_MASK,
	.AieMultiDimAddr.Y_Wrap.Idx = 3U,
	.AieMultiDimAddr.Y_Wrap.Lsb = XAIEGBL_MEM_DMABD02DY_YWRA_LSB,
	.AieMultiDimAddr.Y_Wrap.Mask = XAIEGBL_MEM_DMABD02DY_YWRA_MASK,
	.AieMultiDimAddr.Y_Offset.Idx = 3U,
	.AieMultiDimAddr.Y_Offset.Lsb = XAIEGBL_MEM_DMABD02DY_YOFF_LSB,
	.AieMultiDimAddr.Y_Offset.Mask = XAIEGBL_MEM_DMABD02DY_YOFF_MASK,
	.AieMultiDimAddr.CurrPtr.Idx = 5U,
	.AieMultiDimAddr.CurrPtr.Lsb = XAIEGBL_MEM_DMABD0INTSTA_CURPTR_LSB,
	.AieMultiDimAddr.CurrPtr.Mask = XAIEGBL_MEM_DMABD0INTSTA_CURPTR_MASK
};

static const XAie_DmaBdProp AieTileDmaProp =
{
	.AddrAlignMask = 0x3,
	.AddrAlignShift = 0x2,
	.AddrMax = 0x10000,
	.LenActualOffset = 1U,
	.Buffer = &AieTileDmaBufferProp,
	.DoubleBuffer = &AieTileDmaDoubleBufferProp,
	.Lock = &AieTileDmaLockProp,
	.Pkt = &AieTileDmaBdPktProp,
	.BdEn = &AieTileDmaBdEnProp,
	.AddrMode = &AieTileDmaMultiDimProp,
	.SysProp = NULL
};

static const XAie_DmaChStatus AieTileDmaChStatus[] =
{
	/* This database is common for mm2s and s2mm channels */
	{
		/* For channel 0 */
		.AieDmaChStatus.Status.Lsb = XAIEGBL_MEM_DMAS2MMSTA_STA0_LSB,
		.AieDmaChStatus.Status.Mask = XAIEGBL_MEM_DMAS2MMSTA_STA0_MASK,
		.AieDmaChStatus.StartQSize.Lsb = XAIEGBL_MEM_DMAS2MMSTA_STAQUESIZ0_LSB,
		.AieDmaChStatus.StartQSize.Mask = XAIEGBL_MEM_DMAS2MMSTA_STAQUESIZ0_MASK,
		.AieDmaChStatus.Stalled.Lsb = XAIEGBL_MEM_DMAS2MMSTA_LOCKSTAL0_LSB,
		.AieDmaChStatus.Stalled.Mask = XAIEGBL_MEM_DMAS2MMSTA_LOCKSTAL0_MASK,
	},
	{
		/* For channel 1 */
		.AieDmaChStatus.Status.Lsb = XAIEGBL_MEM_DMAS2MMSTA_STA1_LSB,
		.AieDmaChStatus.Status.Mask = XAIEGBL_MEM_DMAS2MMSTA_STA1_MASK,
		.AieDmaChStatus.StartQSize.Lsb = XAIEGBL_MEM_DMAS2MMSTA_STAQUESIZ1_LSB,
		.AieDmaChStatus.StartQSize.Mask = XAIEGBL_MEM_DMAS2MMSTA_STAQUESIZ1_MASK,
		.AieDmaChStatus.Stalled.Lsb = XAIEGBL_MEM_DMAS2MMSTA_LOCKSTAL1_LSB,
		.AieDmaChStatus.Stalled.Mask = XAIEGBL_MEM_DMAS2MMSTA_LOCKSTAL1_MASK,
	},
};

static const XAie_DmaChProp AieTileDmaChProp =
{
	.PauseStream = {0U},
	.PauseMem = {0U},
	.Reset.Idx = 0U,
	.Reset.Lsb = XAIEGBL_MEM_DMAS2MM0CTR_RST_LSB,
	.Reset.Mask = XAIEGBL_MEM_DMAS2MM0CTR_RST_MASK,
	.Enable.Idx = 0U,
	.Enable.Lsb = XAIEGBL_MEM_DMAS2MM0CTR_ENA_LSB,
	.Enable.Mask = XAIEGBL_MEM_DMAS2MM0CTR_ENA_MASK,
	.StartBd.Idx = 1U,
	.StartBd.Lsb = XAIEGBL_MEM_DMAS2MM0STAQUE_STABD_LSB,
	.StartBd.Mask = XAIEGBL_MEM_DMAS2MM0STAQUE_STABD_MASK,
	.StartQSizeMax = 4U,
	.DmaChStatus = AieTileDmaChStatus,
};

/* Tile Dma Module */
static const XAie_DmaMod AieTileDmaMod =
{
	.BaseAddr = XAIEGBL_MEM_DMABD0ADDA,
	.IdxOffset = 0x20,  	/* This is the offset between each BD */
	.NumBds = 16U,	   	/* Number of BDs for AIE Tile DMA */
	.NumLocks = 16U,
	.NumAddrDim = 2U,
	.DoubleBuffering = XAIE_FEATURE_AVAILABLE,
	.InterleaveMode = XAIE_FEATURE_AVAILABLE,
	.FifoMode = XAIE_FEATURE_AVAILABLE,
	.ChCtrlBase = XAIEGBL_MEM_DMAS2MM0CTR,
	.NumChannels = 2U,  /* Number of s2mm/mm2s channels */
	.ChIdxOffset = 0x8,  /* This is the offset between each channel */
	.ChStatusBase = XAIEGBL_MEM_DMAS2MMSTA,
	.ChStatusOffset = 0x10,
	.BdProp = &AieTileDmaProp,
	.ChProp = &AieTileDmaChProp,
	.DmaBdInit = &_XAie_TileDmaInit,
	.SetLock = &_XAie_DmaSetLock,
	.SetIntrleave = &_XAie_DmaSetInterleaveEnable,
	.SetMultiDim = &_XAie_DmaSetMultiDim,
	.WriteBd = &_XAie_TileDmaWriteBd,
	.PendingBd = &_XAie_DmaGetPendingBdCount,
	.WaitforDone = &_XAie_DmaWaitForDone,
};

/* shim dma structures */
static const XAie_DmaBdEnProp AieShimDmaBdEnProp =
{
	.NxtBd.Idx = 2U,
	.NxtBd.Lsb = XAIEGBL_NOC_DMABD1BUFCTRL_NEXBD_LSB,
	.NxtBd.Mask = XAIEGBL_NOC_DMABD1BUFCTRL_NEXBD_MASK,
	.UseNxtBd.Idx = 2U,
	.UseNxtBd.Lsb = XAIEGBL_NOC_DMABD1BUFCTRL_USENEXBD_LSB,
	.UseNxtBd.Mask = XAIEGBL_NOC_DMABD1BUFCTRL_USENEXBD_MASK,
	.ValidBd.Idx = 2U,
	.ValidBd.Lsb = XAIEGBL_NOC_DMABD1BUFCTRL_VALBD_LSB,
	.ValidBd.Mask = XAIEGBL_NOC_DMABD1BUFCTRL_VALBD_MASK,
	.OutofOrderBdId = {0U}
};

static const XAie_DmaBdPkt AieShimDmaBdPktProp =
{
	.EnPkt.Idx = 4U,
	.EnPkt.Lsb = XAIEGBL_NOC_DMABD0PKT_ENAPKT_LSB,
	.EnPkt.Mask = XAIEGBL_NOC_DMABD0PKT_ENAPKT_MASK,
	.PktType.Idx = 4U,
	.PktType.Lsb = XAIEGBL_NOC_DMABD0PKT_PKTTYP_LSB,
	.PktType.Mask = XAIEGBL_NOC_DMABD0PKT_PKTTYP_MASK,
	.PktId.Idx = 4U,
	.PktId.Lsb = XAIEGBL_NOC_DMABD0PKT_ID_LSB,
	.PktId.Mask = XAIEGBL_NOC_DMABD0PKT_ID_MASK
};

static const XAie_DmaBdLock AieShimDmaLockProp =
{
	.AieDmaLock.LckId_A.Idx = 2U,
	.AieDmaLock.LckId_A.Lsb = XAIEGBL_NOC_DMABD0CTRL_LOCKID_LSB,
	.AieDmaLock.LckId_A.Mask = XAIEGBL_NOC_DMABD0CTRL_LOCKID_MASK,
	.AieDmaLock.LckRelEn_A.Idx = 2U,
	.AieDmaLock.LckRelEn_A.Lsb = XAIEGBL_NOC_DMABD0CTRL_ENAREL_LSB,
	.AieDmaLock.LckRelEn_A.Mask = XAIEGBL_NOC_DMABD0CTRL_ENAREL_MASK,
	.AieDmaLock.LckRelVal_A.Idx = 2U,
	.AieDmaLock.LckRelVal_A.Lsb = XAIEGBL_NOC_DMABD0CTRL_RELVAL_LSB,
	.AieDmaLock.LckRelVal_A.Mask = XAIEGBL_NOC_DMABD0CTRL_RELVAL_MASK,
	.AieDmaLock.LckRelUseVal_A.Idx = 2U,
	.AieDmaLock.LckRelUseVal_A.Lsb = XAIEGBL_NOC_DMABD0CTRL_USERELVAL_LSB,
	.AieDmaLock.LckRelUseVal_A.Mask = XAIEGBL_NOC_DMABD0CTRL_USERELVAL_MASK,
	.AieDmaLock.LckAcqEn_A.Idx = 2U,
	.AieDmaLock.LckAcqEn_A.Lsb = XAIEGBL_NOC_DMABD0CTRL_ENAACQ_LSB,
	.AieDmaLock.LckAcqEn_A.Mask = XAIEGBL_NOC_DMABD0CTRL_ENAACQ_MASK,
	.AieDmaLock.LckAcqVal_A.Idx = 2U,
	.AieDmaLock.LckAcqVal_A.Lsb = XAIEGBL_NOC_DMABD0CTRL_ACQVAL_LSB,
	.AieDmaLock.LckAcqVal_A.Mask = XAIEGBL_NOC_DMABD0CTRL_ACQVAL_MASK,
	.AieDmaLock.LckAcqUseVal_A.Idx = 2U,
	.AieDmaLock.LckAcqUseVal_A.Lsb = XAIEGBL_NOC_DMABD0CTRL_USEACQVAL_LSB,
	.AieDmaLock.LckAcqUseVal_A.Mask = XAIEGBL_NOC_DMABD0CTRL_USEACQVAL_MASK,
	.AieDmaLock.LckId_B = {0U},
	.AieDmaLock.LckRelEn_B = {0U},
	.AieDmaLock.LckRelVal_B = {0U},
	.AieDmaLock.LckRelUseVal_B = {0U},
	.AieDmaLock.LckAcqEn_B = {0U},
	.AieDmaLock.LckAcqVal_B = {0U},
	.AieDmaLock.LckAcqUseVal_B = {0U}
};

static const XAie_DmaBdBuffer AieShimDmaBufferProp =
{
	.ShimDmaBuff.AddrLow.Idx = 0U,
	.ShimDmaBuff.AddrLow.Lsb = XAIEGBL_NOC_DMABD0ADDLOW_ADDLOW_LSB,
	.ShimDmaBuff.AddrLow.Mask = XAIEGBL_NOC_DMABD0ADDLOW_ADDLOW_MASK,
	.ShimDmaBuff.AddrHigh.Idx = 2U,
	.ShimDmaBuff.AddrHigh.Lsb = XAIEGBL_NOC_DMABD0CTRL_ADDHIG_LSB,
	.ShimDmaBuff.AddrHigh.Mask = XAIEGBL_NOC_DMABD0CTRL_ADDHIG_MASK,
	.ShimDmaBuff.BufferLen.Idx = 1U,
	.ShimDmaBuff.BufferLen.Lsb = XAIEGBL_NOC_DMABD0BUFLEN_BUFLEN_LSB,
	.ShimDmaBuff.BufferLen.Mask = XAIEGBL_NOC_DMABD0BUFLEN_BUFLEN_MASK,
};

static const XAie_DmaSysProp AieShimDmaSysProp =
{
	.SMID.Idx = 3U,
	.SMID.Lsb = XAIEGBL_NOC_DMABD0AXICFG_SMI_LSB,
	.SMID.Mask = XAIEGBL_NOC_DMABD0AXICFG_SMI_MASK,
	.BurstLen.Idx = 3U,
	.BurstLen.Lsb = XAIEGBL_NOC_DMABD0AXICFG_BURLEN_LSB,
	.BurstLen.Mask = XAIEGBL_NOC_DMABD0AXICFG_BURLEN_MASK,
	.AxQos.Idx = 3U,
	.AxQos.Lsb = XAIEGBL_NOC_DMABD0AXICFG_AXQ_LSB,
	.AxQos.Mask = XAIEGBL_NOC_DMABD0AXICFG_AXQ_MASK,
	.SecureAccess.Idx = 3U,
	.SecureAccess.Lsb = XAIEGBL_NOC_DMABD0AXICFG_SECACC_LSB,
	.SecureAccess.Mask = XAIEGBL_NOC_DMABD0AXICFG_SECACC_MASK,
	.AxCache.Idx = 3U,
	.AxCache.Lsb = XAIEGBL_NOC_DMABD0AXICFG_AXC_LSB,
	.AxCache.Mask = XAIEGBL_NOC_DMABD0AXICFG_AXC_MASK,
};

static const XAie_DmaBdProp AieShimDmaProp =
{
	.AddrAlignMask = 0xF,
	.AddrAlignShift = 0x0,
	.AddrMax = 0x1000000000000,
	.LenActualOffset = 0U,
	.Buffer = &AieShimDmaBufferProp,
	.DoubleBuffer = NULL,
	.Lock = &AieShimDmaLockProp,
	.Pkt = &AieShimDmaBdPktProp,
	.BdEn = &AieShimDmaBdEnProp,
	.AddrMode = NULL,
	.SysProp = &AieShimDmaSysProp
};

static const XAie_DmaChStatus AieShimDmaChStatus[] =
{
	/* This database is common for mm2s and s2mm channels */
	{
		/* For channel 0 */
		.AieDmaChStatus.Status.Lsb = XAIEGBL_NOC_DMAS2MMSTA_STA0_LSB,
		.AieDmaChStatus.Status.Mask = XAIEGBL_NOC_DMAS2MMSTA_STA0_MASK,
		.AieDmaChStatus.StartQSize.Lsb = XAIEGBL_NOC_DMAS2MMSTA_STAQUESIZ0_LSB,
		.AieDmaChStatus.StartQSize.Mask = XAIEGBL_NOC_DMAS2MMSTA_STAQUESIZ0_MASK,
		.AieDmaChStatus.Stalled.Lsb = XAIEGBL_NOC_DMAS2MMSTA_STAL0_LSB,
		.AieDmaChStatus.Stalled.Mask = XAIEGBL_NOC_DMAS2MMSTA_STAL0_MASK,
	},
	{
		/* For channel 1 */
		.AieDmaChStatus.Status.Lsb = XAIEGBL_NOC_DMAS2MMSTA_STA1_LSB,
		.AieDmaChStatus.Status.Mask = XAIEGBL_NOC_DMAS2MMSTA_STA1_MASK,
		.AieDmaChStatus.StartQSize.Lsb = XAIEGBL_NOC_DMAS2MMSTA_STAQUESIZ1_LSB,
		.AieDmaChStatus.StartQSize.Mask = XAIEGBL_NOC_DMAS2MMSTA_STAQUESIZ1_MASK,
		.AieDmaChStatus.Stalled.Lsb = XAIEGBL_NOC_DMAS2MMSTA_STAL1_LSB,
		.AieDmaChStatus.Stalled.Mask = XAIEGBL_NOC_DMAS2MMSTA_STAL1_MASK,
	},
};

static const XAie_DmaChProp AieShimDmaChProp =
{
	.Reset = {0U},
	.Enable.Idx = 0U,
	.Enable.Lsb = XAIEGBL_NOC_DMAS2MM0CTR_ENA_LSB,
	.Enable.Mask = XAIEGBL_NOC_DMAS2MM0CTR_ENA_MASK,
	.PauseStream.Idx = 0U,
	.PauseStream.Lsb = XAIEGBL_NOC_DMAS2MM0CTR_PAUSTR_LSB,
	.PauseStream.Mask = XAIEGBL_NOC_DMAS2MM0CTR_PAUSTR_MASK,
	.PauseMem.Idx = 0U,
	.PauseMem.Lsb = XAIEGBL_NOC_DMAS2MM0CTR_PAUMEM_LSB,
	.PauseMem.Mask = XAIEGBL_NOC_DMAS2MM0CTR_PAUMEM_MASK,
	.StartBd.Idx = 1U,
	.StartBd.Lsb = XAIEGBL_NOC_DMAS2MM0STAQUE_STABD_LSB,
	.StartBd.Mask = XAIEGBL_NOC_DMAS2MM0STAQUE_STABD_MASK,
	.StartQSizeMax = 4U,
	.DmaChStatus = AieShimDmaChStatus,
};

/* Shim Dma Module */
static const XAie_DmaMod AieShimDmaMod =
{
	.BaseAddr = XAIEGBL_NOC_DMABD0ADDLOW,
	.IdxOffset = 0x14,  	/* This is the offset between each BD */
	.NumBds = 16U,	   	/* Number of BDs for AIE Tile DMA */
	.NumLocks = 16U,
	.NumAddrDim = 0U,
	.DoubleBuffering = XAIE_FEATURE_UNAVAILABLE,
	.InterleaveMode = XAIE_FEATURE_UNAVAILABLE,
	.FifoMode = XAIE_FEATURE_UNAVAILABLE,
	.ChCtrlBase = XAIEGBL_NOC_DMAS2MM0CTR,
	.NumChannels = 2U,  /* Number of s2mm/mm2s channels */
	.ChIdxOffset = 0x8,  /* This is the offset between each channel */
	.ChStatusBase = XAIEGBL_NOC_DMAS2MMSTA,
	.ChStatusOffset = 0x4,
	.BdProp = &AieShimDmaProp,
	.ChProp = &AieShimDmaChProp,
	.DmaBdInit = &_XAie_ShimDmaInit,
	.SetLock = &_XAie_DmaSetLock,
	.SetIntrleave = NULL,
	.SetMultiDim = NULL,
	.WriteBd = &_XAie_ShimDmaWriteBd,
	.PendingBd = &_XAie_DmaGetPendingBdCount,
	.WaitforDone = &_XAie_DmaWaitForDone,
};

/* Enum to Event Number mapping of all events of AIE Core module */
static const u8 AieTileCoreModEventMapping[] =
{
	XAIE_EVENTS_CORE_NONE,
	XAIE_EVENTS_CORE_TRUE,
	XAIE_EVENTS_CORE_GROUP_0,
	XAIE_EVENTS_CORE_TIMER_SYNC,
	XAIE_EVENTS_CORE_TIMER_VALUE_REACHED,
	XAIE_EVENTS_CORE_PERF_CNT_0,
	XAIE_EVENTS_CORE_PERF_CNT_1,
	XAIE_EVENTS_CORE_PERF_CNT_2,
	XAIE_EVENTS_CORE_PERF_CNT_3,
	XAIE_EVENTS_CORE_COMBO_EVENT_0,
	XAIE_EVENTS_CORE_COMBO_EVENT_1,
	XAIE_EVENTS_CORE_COMBO_EVENT_2,
	XAIE_EVENTS_CORE_COMBO_EVENT_3,
	XAIE_EVENTS_CORE_GROUP_PC_EVENT,
	XAIE_EVENTS_CORE_PC_0,
	XAIE_EVENTS_CORE_PC_1,
	XAIE_EVENTS_CORE_PC_2,
	XAIE_EVENTS_CORE_PC_3,
	XAIE_EVENTS_CORE_PC_RANGE_0_1,
	XAIE_EVENTS_CORE_PC_RANGE_2_3,
	XAIE_EVENTS_CORE_GROUP_STALL,
	XAIE_EVENTS_CORE_MEMORY_STALL,
	XAIE_EVENTS_CORE_STREAM_STALL,
	XAIE_EVENTS_CORE_CASCADE_STALL,
	XAIE_EVENTS_CORE_LOCK_STALL,
	XAIE_EVENTS_CORE_DEBUG_HALTED,
	XAIE_EVENTS_CORE_ACTIVE,
	XAIE_EVENTS_CORE_DISABLED,
	XAIE_EVENTS_CORE_ECC_ERROR_STALL,
	XAIE_EVENTS_CORE_ECC_SCRUBBING_STALL,
	XAIE_EVENTS_CORE_GROUP_PROGRAM_FLOW,
	XAIE_EVENTS_CORE_INSTR_EVENT_0,
	XAIE_EVENTS_CORE_INSTR_EVENT_1,
	XAIE_EVENTS_CORE_INSTR_CALL,
	XAIE_EVENTS_CORE_INSTR_RETURN,
	XAIE_EVENTS_CORE_INSTR_VECTOR,
	XAIE_EVENTS_CORE_INSTR_LOAD,
	XAIE_EVENTS_CORE_INSTR_STORE,
	XAIE_EVENTS_CORE_INSTR_STREAM_GET,
	XAIE_EVENTS_CORE_INSTR_STREAM_PUT,
	XAIE_EVENTS_CORE_INSTR_CASCADE_GET,
	XAIE_EVENTS_CORE_INSTR_CASCADE_PUT,
	XAIE_EVENTS_CORE_INSTR_LOCK_ACQUIRE_REQ,
	XAIE_EVENTS_CORE_INSTR_LOCK_RELEASE_REQ,
	XAIE_EVENTS_CORE_GROUP_ERRORS_0,
	XAIE_EVENTS_CORE_GROUP_ERRORS_1,
	XAIE_EVENTS_CORE_SRS_SATURATE,
	XAIE_EVENTS_CORE_UPS_SATURATE,
	XAIE_EVENTS_CORE_FP_OVERFLOW,
	XAIE_EVENTS_CORE_FP_UNDERFLOW,
	XAIE_EVENTS_CORE_FP_INVALID,
	XAIE_EVENTS_CORE_FP_DIV_BY_ZERO,
	XAIE_EVENTS_CORE_TLAST_IN_WSS_WORDS_0_2,
	XAIE_EVENTS_CORE_PM_REG_ACCESS_FAILURE,
	XAIE_EVENTS_CORE_STREAM_PKT_PARITY_ERROR,
	XAIE_EVENTS_CORE_CONTROL_PKT_ERROR,
	XAIE_EVENTS_CORE_AXI_MM_SLAVE_ERROR,
	XAIE_EVENTS_CORE_INSTR_DECOMPRSN_ERROR,
	XAIE_EVENTS_CORE_DM_ADDRESS_OUT_OF_RANGE,
	XAIE_EVENTS_CORE_PM_ECC_ERROR_SCRUB_CORRECTED,
	XAIE_EVENTS_CORE_PM_ECC_ERROR_SCRUB_2BIT,
	XAIE_EVENTS_CORE_PM_ECC_ERROR_1BIT,
	XAIE_EVENTS_CORE_PM_ECC_ERROR_2BIT,
	XAIE_EVENTS_CORE_PM_ADDRESS_OUT_OF_RANGE,
	XAIE_EVENTS_CORE_DM_ACCESS_TO_UNAVAILABLE,
	XAIE_EVENTS_CORE_LOCK_ACCESS_TO_UNAVAILABLE,
	XAIE_EVENT_INVALID,
	XAIE_EVENTS_CORE_INSTR_EVENT_3,
	XAIE_EVENTS_CORE_GROUP_STREAM_SWITCH,
	XAIE_EVENTS_CORE_PORT_IDLE_0,
	XAIE_EVENTS_CORE_PORT_RUNNING_0,
	XAIE_EVENTS_CORE_PORT_STALLED_0,
	XAIE_EVENTS_CORE_PORT_TLAST_0,
	XAIE_EVENTS_CORE_PORT_IDLE_1,
	XAIE_EVENTS_CORE_PORT_RUNNING_1,
	XAIE_EVENTS_CORE_PORT_STALLED_1,
	XAIE_EVENTS_CORE_PORT_TLAST_1,
	XAIE_EVENTS_CORE_PORT_IDLE_2,
	XAIE_EVENTS_CORE_PORT_RUNNING_2,
	XAIE_EVENTS_CORE_PORT_STALLED_2,
	XAIE_EVENTS_CORE_PORT_TLAST_2,
	XAIE_EVENTS_CORE_PORT_IDLE_3,
	XAIE_EVENTS_CORE_PORT_RUNNING_3,
	XAIE_EVENTS_CORE_PORT_STALLED_3,
	XAIE_EVENTS_CORE_PORT_TLAST_3,
	XAIE_EVENTS_CORE_PORT_IDLE_4,
	XAIE_EVENTS_CORE_PORT_RUNNING_4,
	XAIE_EVENTS_CORE_PORT_STALLED_4,
	XAIE_EVENTS_CORE_PORT_TLAST_4,
	XAIE_EVENTS_CORE_PORT_IDLE_5,
	XAIE_EVENTS_CORE_PORT_RUNNING_5,
	XAIE_EVENTS_CORE_PORT_STALLED_5,
	XAIE_EVENTS_CORE_PORT_TLAST_5,
	XAIE_EVENTS_CORE_PORT_IDLE_6,
	XAIE_EVENTS_CORE_PORT_RUNNING_6,
	XAIE_EVENTS_CORE_PORT_STALLED_6,
	XAIE_EVENTS_CORE_PORT_TLAST_6,
	XAIE_EVENTS_CORE_PORT_IDLE_7,
	XAIE_EVENTS_CORE_PORT_RUNNING_7,
	XAIE_EVENTS_CORE_PORT_STALLED_7,
	XAIE_EVENTS_CORE_PORT_TLAST_7,
	XAIE_EVENTS_CORE_GROUP_BROADCAST,
	XAIE_EVENTS_CORE_BROADCAST_0,
	XAIE_EVENTS_CORE_BROADCAST_1,
	XAIE_EVENTS_CORE_BROADCAST_2,
	XAIE_EVENTS_CORE_BROADCAST_3,
	XAIE_EVENTS_CORE_BROADCAST_4,
	XAIE_EVENTS_CORE_BROADCAST_5,
	XAIE_EVENTS_CORE_BROADCAST_6,
	XAIE_EVENTS_CORE_BROADCAST_7,
	XAIE_EVENTS_CORE_BROADCAST_8,
	XAIE_EVENTS_CORE_BROADCAST_9,
	XAIE_EVENTS_CORE_BROADCAST_10,
	XAIE_EVENTS_CORE_BROADCAST_11,
	XAIE_EVENTS_CORE_BROADCAST_12,
	XAIE_EVENTS_CORE_BROADCAST_13,
	XAIE_EVENTS_CORE_BROADCAST_14,
	XAIE_EVENTS_CORE_BROADCAST_15,
	XAIE_EVENTS_CORE_GROUP_USER_EVENT,
	XAIE_EVENTS_CORE_USER_EVENT_0,
	XAIE_EVENTS_CORE_USER_EVENT_1,
	XAIE_EVENTS_CORE_USER_EVENT_2,
	XAIE_EVENTS_CORE_USER_EVENT_3,
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
};
/* Enum to Event Number mapping of all events of AIE Mem module */
static const u8 AieTileMemModEventMapping[] =
{
	XAIE_EVENTS_MEM_NONE,
	XAIE_EVENTS_MEM_TRUE,
	XAIE_EVENTS_MEM_GROUP_0,
	XAIE_EVENTS_MEM_TIMER_SYNC,
	XAIE_EVENTS_MEM_TIMER_VALUE_REACHED,
	XAIE_EVENTS_MEM_PERF_CNT_0,
	XAIE_EVENTS_MEM_PERF_CNT_1,
	XAIE_EVENTS_MEM_COMBO_EVENT_0,
	XAIE_EVENTS_MEM_COMBO_EVENT_1,
	XAIE_EVENTS_MEM_COMBO_EVENT_2,
	XAIE_EVENTS_MEM_COMBO_EVENT_3,
	XAIE_EVENTS_MEM_GROUP_WATCHPOINT,
	XAIE_EVENTS_MEM_WATCHPOINT_0,
	XAIE_EVENTS_MEM_WATCHPOINT_1,
	XAIE_EVENTS_MEM_GROUP_DMA_ACTIVITY,
	XAIE_EVENTS_MEM_DMA_S2MM_0_START_BD,
	XAIE_EVENTS_MEM_DMA_S2MM_1_START_BD,
	XAIE_EVENTS_MEM_DMA_MM2S_0_START_BD,
	XAIE_EVENTS_MEM_DMA_MM2S_1_START_BD,
	XAIE_EVENTS_MEM_DMA_S2MM_0_FINISHED_BD,
	XAIE_EVENTS_MEM_DMA_S2MM_1_FINISHED_BD,
	XAIE_EVENTS_MEM_DMA_MM2S_0_FINISHED_BD,
	XAIE_EVENTS_MEM_DMA_MM2S_1_FINISHED_BD,
	XAIE_EVENTS_MEM_DMA_S2MM_0_GO_TO_IDLE,
	XAIE_EVENTS_MEM_DMA_S2MM_1_GO_TO_IDLE,
	XAIE_EVENTS_MEM_DMA_MM2S_0_GO_TO_IDLE,
	XAIE_EVENTS_MEM_DMA_MM2S_1_GO_TO_IDLE,
	XAIE_EVENTS_MEM_DMA_S2MM_0_STALLED_LOCK_ACQUIRE,
	XAIE_EVENTS_MEM_DMA_S2MM_1_STALLED_LOCK_ACQUIRE,
	XAIE_EVENTS_MEM_DMA_MM2S_0_STALLED_LOCK_ACQUIRE,
	XAIE_EVENTS_MEM_DMA_MM2S_1_STALLED_LOCK_ACQUIRE,
	XAIE_EVENTS_MEM_DMA_S2MM_0_MEMORY_CONFLICT,
	XAIE_EVENTS_MEM_DMA_S2MM_1_MEMORY_CONFLICT,
	XAIE_EVENTS_MEM_DMA_MM2S_0_MEMORY_CONFLICT,
	XAIE_EVENTS_MEM_DMA_MM2S_1_MEMORY_CONFLICT,
	XAIE_EVENTS_MEM_GROUP_LOCK,
	XAIE_EVENTS_MEM_LOCK_0_ACQ,
	XAIE_EVENTS_MEM_LOCK_0_REL,
	XAIE_EVENTS_MEM_LOCK_1_ACQ,
	XAIE_EVENTS_MEM_LOCK_1_REL,
	XAIE_EVENTS_MEM_LOCK_2_ACQ,
	XAIE_EVENTS_MEM_LOCK_2_REL,
	XAIE_EVENTS_MEM_LOCK_3_ACQ,
	XAIE_EVENTS_MEM_LOCK_3_REL,
	XAIE_EVENTS_MEM_LOCK_4_ACQ,
	XAIE_EVENTS_MEM_LOCK_4_REL,
	XAIE_EVENTS_MEM_LOCK_5_ACQ,
	XAIE_EVENTS_MEM_LOCK_5_REL,
	XAIE_EVENTS_MEM_LOCK_6_ACQ,
	XAIE_EVENTS_MEM_LOCK_6_REL,
	XAIE_EVENTS_MEM_LOCK_7_ACQ,
	XAIE_EVENTS_MEM_LOCK_7_REL,
	XAIE_EVENTS_MEM_LOCK_8_ACQ,
	XAIE_EVENTS_MEM_LOCK_8_REL,
	XAIE_EVENTS_MEM_LOCK_9_ACQ,
	XAIE_EVENTS_MEM_LOCK_9_REL,
	XAIE_EVENTS_MEM_LOCK_10_ACQ,
	XAIE_EVENTS_MEM_LOCK_10_REL,
	XAIE_EVENTS_MEM_LOCK_11_ACQ,
	XAIE_EVENTS_MEM_LOCK_11_REL,
	XAIE_EVENTS_MEM_LOCK_12_ACQ,
	XAIE_EVENTS_MEM_LOCK_12_REL,
	XAIE_EVENTS_MEM_LOCK_13_ACQ,
	XAIE_EVENTS_MEM_LOCK_13_REL,
	XAIE_EVENTS_MEM_LOCK_14_ACQ,
	XAIE_EVENTS_MEM_LOCK_14_REL,
	XAIE_EVENTS_MEM_LOCK_15_ACQ,
	XAIE_EVENTS_MEM_LOCK_15_REL,
	XAIE_EVENTS_MEM_GROUP_MEMORY_CONFLICT,
	XAIE_EVENTS_MEM_CONFLICT_DM_BANK_0,
	XAIE_EVENTS_MEM_CONFLICT_DM_BANK_1,
	XAIE_EVENTS_MEM_CONFLICT_DM_BANK_2,
	XAIE_EVENTS_MEM_CONFLICT_DM_BANK_3,
	XAIE_EVENTS_MEM_CONFLICT_DM_BANK_4,
	XAIE_EVENTS_MEM_CONFLICT_DM_BANK_5,
	XAIE_EVENTS_MEM_CONFLICT_DM_BANK_6,
	XAIE_EVENTS_MEM_CONFLICT_DM_BANK_7,
	XAIE_EVENTS_MEM_GROUP_ERRORS,
	XAIE_EVENTS_MEM_DM_ECC_ERROR_SCRUB_CORRECTED,
	XAIE_EVENTS_MEM_DM_ECC_ERROR_SCRUB_2BIT,
	XAIE_EVENTS_MEM_DM_ECC_ERROR_1BIT,
	XAIE_EVENTS_MEM_DM_ECC_ERROR_2BIT,
	XAIE_EVENTS_MEM_DM_PARITY_ERROR_BANK_2,
	XAIE_EVENTS_MEM_DM_PARITY_ERROR_BANK_3,
	XAIE_EVENTS_MEM_DM_PARITY_ERROR_BANK_4,
	XAIE_EVENTS_MEM_DM_PARITY_ERROR_BANK_5,
	XAIE_EVENTS_MEM_DM_PARITY_ERROR_BANK_6,
	XAIE_EVENTS_MEM_DM_PARITY_ERROR_BANK_7,
	XAIE_EVENTS_MEM_DMA_S2MM_0_ERROR,
	XAIE_EVENTS_MEM_DMA_S2MM_1_ERROR,
	XAIE_EVENTS_MEM_DMA_MM2S_0_ERROR,
	XAIE_EVENTS_MEM_DMA_MM2S_1_ERROR,
	XAIE_EVENTS_MEM_GROUP_BROADCAST,
	XAIE_EVENTS_MEM_BROADCAST_0,
	XAIE_EVENTS_MEM_BROADCAST_1,
	XAIE_EVENTS_MEM_BROADCAST_2,
	XAIE_EVENTS_MEM_BROADCAST_3,
	XAIE_EVENTS_MEM_BROADCAST_4,
	XAIE_EVENTS_MEM_BROADCAST_5,
	XAIE_EVENTS_MEM_BROADCAST_6,
	XAIE_EVENTS_MEM_BROADCAST_7,
	XAIE_EVENTS_MEM_BROADCAST_8,
	XAIE_EVENTS_MEM_BROADCAST_9,
	XAIE_EVENTS_MEM_BROADCAST_10,
	XAIE_EVENTS_MEM_BROADCAST_11,
	XAIE_EVENTS_MEM_BROADCAST_12,
	XAIE_EVENTS_MEM_BROADCAST_13,
	XAIE_EVENTS_MEM_BROADCAST_14,
	XAIE_EVENTS_MEM_BROADCAST_15,
	XAIE_EVENTS_MEM_GROUP_USER_EVENT,
	XAIE_EVENTS_MEM_USER_EVENT_0,
	XAIE_EVENTS_MEM_USER_EVENT_1,
	XAIE_EVENTS_MEM_USER_EVENT_2,
	XAIE_EVENTS_MEM_USER_EVENT_3,
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
};

/* Enum to Event Number mapping of all events of AIE NOC tile */
static const u8 AieTileNocModEventMapping[] =
{
	XAIE_EVENTS_PL_NONE,
	XAIE_EVENTS_PL_TRUE,
	XAIE_EVENTS_PL_GROUP_0,
	XAIE_EVENTS_PL_TIMER_SYNC,
	XAIE_EVENTS_PL_TIMER_VALUE_REACHED,
	XAIE_EVENTS_PL_PERF_CNT_0,
	XAIE_EVENTS_PL_PERF_CNT_1,
	XAIE_EVENTS_PL_COMBO_EVENT_0,
	XAIE_EVENTS_PL_COMBO_EVENT_1,
	XAIE_EVENTS_PL_COMBO_EVENT_2,
	XAIE_EVENTS_PL_COMBO_EVENT_3,
	XAIE_EVENTS_PL_GROUP_DMA_ACTIVITY,
	XAIE_EVENTS_PL_DMA_S2MM_0_START_BD,
	XAIE_EVENTS_PL_DMA_S2MM_1_START_BD,
	XAIE_EVENTS_PL_DMA_MM2S_0_START_BD,
	XAIE_EVENTS_PL_DMA_MM2S_1_START_BD,
	XAIE_EVENTS_PL_DMA_S2MM_0_FINISHED_BD,
	XAIE_EVENTS_PL_DMA_S2MM_1_FINISHED_BD,
	XAIE_EVENTS_PL_DMA_MM2S_0_FINISHED_BD,
	XAIE_EVENTS_PL_DMA_MM2S_1_FINISHED_BD,
	XAIE_EVENTS_PL_DMA_S2MM_0_GO_TO_IDLE,
	XAIE_EVENTS_PL_DMA_S2MM_1_GO_TO_IDLE,
	XAIE_EVENTS_PL_DMA_MM2S_0_GO_TO_IDLE,
	XAIE_EVENTS_PL_DMA_MM2S_1_GO_TO_IDLE,
	XAIE_EVENTS_PL_DMA_S2MM_0_STALLED_LOCK_ACQUIRE,
	XAIE_EVENTS_PL_DMA_S2MM_1_STALLED_LOCK_ACQUIRE,
	XAIE_EVENTS_PL_DMA_MM2S_0_STALLED_LOCK_ACQUIRE,
	XAIE_EVENTS_PL_DMA_MM2S_1_STALLED_LOCK_ACQUIRE,
	XAIE_EVENTS_PL_GROUP_LOCK,
	XAIE_EVENTS_PL_LOCK_0_ACQUIRED,
	XAIE_EVENTS_PL_LOCK_0_RELEASED,
	XAIE_EVENTS_PL_LOCK_1_ACQUIRED,
	XAIE_EVENTS_PL_LOCK_1_RELEASED,
	XAIE_EVENTS_PL_LOCK_2_ACQUIRED,
	XAIE_EVENTS_PL_LOCK_2_RELEASED,
	XAIE_EVENTS_PL_LOCK_3_ACQUIRED,
	XAIE_EVENTS_PL_LOCK_3_RELEASED,
	XAIE_EVENTS_PL_LOCK_4_ACQUIRED,
	XAIE_EVENTS_PL_LOCK_4_RELEASED,
	XAIE_EVENTS_PL_LOCK_5_ACQUIRED,
	XAIE_EVENTS_PL_LOCK_5_RELEASED,
	XAIE_EVENTS_PL_LOCK_6_ACQUIRED,
	XAIE_EVENTS_PL_LOCK_6_RELEASED,
	XAIE_EVENTS_PL_LOCK_7_ACQUIRED,
	XAIE_EVENTS_PL_LOCK_7_RELEASED,
	XAIE_EVENTS_PL_LOCK_8_ACQUIRED,
	XAIE_EVENTS_PL_LOCK_8_RELEASED,
	XAIE_EVENTS_PL_LOCK_9_ACQUIRED,
	XAIE_EVENTS_PL_LOCK_9_RELEASED,
	XAIE_EVENTS_PL_LOCK_10_ACQUIRED,
	XAIE_EVENTS_PL_LOCK_10_RELEASED,
	XAIE_EVENTS_PL_LOCK_11_ACQUIRED,
	XAIE_EVENTS_PL_LOCK_11_RELEASED,
	XAIE_EVENTS_PL_LOCK_12_ACQUIRED,
	XAIE_EVENTS_PL_LOCK_12_RELEASED,
	XAIE_EVENTS_PL_LOCK_13_ACQUIRED,
	XAIE_EVENTS_PL_LOCK_13_RELEASED,
	XAIE_EVENTS_PL_LOCK_14_ACQUIRED,
	XAIE_EVENTS_PL_LOCK_14_RELEASED,
	XAIE_EVENTS_PL_LOCK_15_ACQUIRED,
	XAIE_EVENTS_PL_LOCK_15_RELEASED,
	XAIE_EVENTS_PL_GROUP_ERRORS,
	XAIE_EVENTS_PL_AXI_MM_SLAVE_TILE_ERROR,
	XAIE_EVENTS_PL_CONTROL_PKT_ERROR,
	XAIE_EVENTS_PL_AXI_MM_DECODE_NSU_ERROR,
	XAIE_EVENTS_PL_AXI_MM_SLAVE_NSU_ERROR,
	XAIE_EVENTS_PL_AXI_MM_UNSUPPORTED_TRAFFIC,
	XAIE_EVENTS_PL_AXI_MM_UNSECURE_ACCESS_IN_SECURE_MODE,
	XAIE_EVENTS_PL_AXI_MM_BYTE_STROBE_ERROR,
	XAIE_EVENTS_PL_DMA_S2MM_0_ERROR,
	XAIE_EVENTS_PL_DMA_S2MM_1_ERROR,
	XAIE_EVENTS_PL_DMA_MM2S_0_ERROR,
	XAIE_EVENTS_PL_DMA_MM2S_1_ERROR,
	XAIE_EVENTS_PL_GROUP_STREAM_SWITCH,
	XAIE_EVENTS_PL_PORT_IDLE_0,
	XAIE_EVENTS_PL_PORT_RUNNING_0,
	XAIE_EVENTS_PL_PORT_STALLED_0,
	XAIE_EVENTS_PL_PORT_TLAST_0,
	XAIE_EVENTS_PL_PORT_IDLE_1,
	XAIE_EVENTS_PL_PORT_RUNNING_1,
	XAIE_EVENTS_PL_PORT_STALLED_1,
	XAIE_EVENTS_PL_PORT_TLAST_1,
	XAIE_EVENTS_PL_PORT_IDLE_2,
	XAIE_EVENTS_PL_PORT_RUNNING_2,
	XAIE_EVENTS_PL_PORT_STALLED_2,
	XAIE_EVENTS_PL_PORT_TLAST_2,
	XAIE_EVENTS_PL_PORT_IDLE_3,
	XAIE_EVENTS_PL_PORT_RUNNING_3,
	XAIE_EVENTS_PL_PORT_STALLED_3,
	XAIE_EVENTS_PL_PORT_TLAST_3,
	XAIE_EVENTS_PL_PORT_IDLE_4,
	XAIE_EVENTS_PL_PORT_RUNNING_4,
	XAIE_EVENTS_PL_PORT_STALLED_4,
	XAIE_EVENTS_PL_PORT_TLAST_4,
	XAIE_EVENTS_PL_PORT_IDLE_5,
	XAIE_EVENTS_PL_PORT_RUNNING_5,
	XAIE_EVENTS_PL_PORT_STALLED_5,
	XAIE_EVENTS_PL_PORT_TLAST_5,
	XAIE_EVENTS_PL_PORT_IDLE_6,
	XAIE_EVENTS_PL_PORT_RUNNING_6,
	XAIE_EVENTS_PL_PORT_STALLED_6,
	XAIE_EVENTS_PL_PORT_TLAST_6,
	XAIE_EVENTS_PL_PORT_IDLE_7,
	XAIE_EVENTS_PL_PORT_RUNNING_7,
	XAIE_EVENTS_PL_PORT_STALLED_7,
	XAIE_EVENTS_PL_PORT_TLAST_7,
	XAIE_EVENTS_PL_GROUP_BROADCAST_A,
	XAIE_EVENTS_PL_BROADCAST_A_0,
	XAIE_EVENTS_PL_BROADCAST_A_1,
	XAIE_EVENTS_PL_BROADCAST_A_2,
	XAIE_EVENTS_PL_BROADCAST_A_3,
	XAIE_EVENTS_PL_BROADCAST_A_4,
	XAIE_EVENTS_PL_BROADCAST_A_5,
	XAIE_EVENTS_PL_BROADCAST_A_6,
	XAIE_EVENTS_PL_BROADCAST_A_7,
	XAIE_EVENTS_PL_BROADCAST_A_8,
	XAIE_EVENTS_PL_BROADCAST_A_9,
	XAIE_EVENTS_PL_BROADCAST_A_10,
	XAIE_EVENTS_PL_BROADCAST_A_11,
	XAIE_EVENTS_PL_BROADCAST_A_12,
	XAIE_EVENTS_PL_BROADCAST_A_13,
	XAIE_EVENTS_PL_BROADCAST_A_14,
	XAIE_EVENTS_PL_BROADCAST_A_15,
	XAIE_EVENTS_PL_GROUP_USER_EVENT,
	XAIE_EVENTS_PL_USER_EVENT_0,
	XAIE_EVENTS_PL_USER_EVENT_1,
	XAIE_EVENTS_PL_USER_EVENT_2,
	XAIE_EVENTS_PL_USER_EVENT_3,
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
};

/* Enum to Event Number mapping of all events of AIE PL module */
static const u8 AieTilePlModEventMapping[] =
{
	XAIE_EVENTS_PL_NONE,
	XAIE_EVENTS_PL_TRUE,
	XAIE_EVENTS_PL_GROUP_0,
	XAIE_EVENTS_PL_TIMER_SYNC,
	XAIE_EVENTS_PL_TIMER_VALUE_REACHED,
	XAIE_EVENTS_PL_PERF_CNT_0,
	XAIE_EVENTS_PL_PERF_CNT_1,
	XAIE_EVENTS_PL_COMBO_EVENT_0,
	XAIE_EVENTS_PL_COMBO_EVENT_1,
	XAIE_EVENTS_PL_COMBO_EVENT_2,
	XAIE_EVENTS_PL_COMBO_EVENT_3,
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
	XAIE_EVENTS_PL_GROUP_ERRORS,
	XAIE_EVENTS_PL_AXI_MM_SLAVE_TILE_ERROR,
	XAIE_EVENTS_PL_CONTROL_PKT_ERROR,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENTS_PL_GROUP_STREAM_SWITCH,
	XAIE_EVENTS_PL_PORT_IDLE_0,
	XAIE_EVENTS_PL_PORT_RUNNING_0,
	XAIE_EVENTS_PL_PORT_STALLED_0,
	XAIE_EVENTS_PL_PORT_TLAST_0,
	XAIE_EVENTS_PL_PORT_IDLE_1,
	XAIE_EVENTS_PL_PORT_RUNNING_1,
	XAIE_EVENTS_PL_PORT_STALLED_1,
	XAIE_EVENTS_PL_PORT_TLAST_1,
	XAIE_EVENTS_PL_PORT_IDLE_2,
	XAIE_EVENTS_PL_PORT_RUNNING_2,
	XAIE_EVENTS_PL_PORT_STALLED_2,
	XAIE_EVENTS_PL_PORT_TLAST_2,
	XAIE_EVENTS_PL_PORT_IDLE_3,
	XAIE_EVENTS_PL_PORT_RUNNING_3,
	XAIE_EVENTS_PL_PORT_STALLED_3,
	XAIE_EVENTS_PL_PORT_TLAST_3,
	XAIE_EVENTS_PL_PORT_IDLE_4,
	XAIE_EVENTS_PL_PORT_RUNNING_4,
	XAIE_EVENTS_PL_PORT_STALLED_4,
	XAIE_EVENTS_PL_PORT_TLAST_4,
	XAIE_EVENTS_PL_PORT_IDLE_5,
	XAIE_EVENTS_PL_PORT_RUNNING_5,
	XAIE_EVENTS_PL_PORT_STALLED_5,
	XAIE_EVENTS_PL_PORT_TLAST_5,
	XAIE_EVENTS_PL_PORT_IDLE_6,
	XAIE_EVENTS_PL_PORT_RUNNING_6,
	XAIE_EVENTS_PL_PORT_STALLED_6,
	XAIE_EVENTS_PL_PORT_TLAST_6,
	XAIE_EVENTS_PL_PORT_IDLE_7,
	XAIE_EVENTS_PL_PORT_RUNNING_7,
	XAIE_EVENTS_PL_PORT_STALLED_7,
	XAIE_EVENTS_PL_PORT_TLAST_7,
	XAIE_EVENTS_PL_GROUP_BROADCAST_A,
	XAIE_EVENTS_PL_BROADCAST_A_0,
	XAIE_EVENTS_PL_BROADCAST_A_1,
	XAIE_EVENTS_PL_BROADCAST_A_2,
	XAIE_EVENTS_PL_BROADCAST_A_3,
	XAIE_EVENTS_PL_BROADCAST_A_4,
	XAIE_EVENTS_PL_BROADCAST_A_5,
	XAIE_EVENTS_PL_BROADCAST_A_6,
	XAIE_EVENTS_PL_BROADCAST_A_7,
	XAIE_EVENTS_PL_BROADCAST_A_8,
	XAIE_EVENTS_PL_BROADCAST_A_9,
	XAIE_EVENTS_PL_BROADCAST_A_10,
	XAIE_EVENTS_PL_BROADCAST_A_11,
	XAIE_EVENTS_PL_BROADCAST_A_12,
	XAIE_EVENTS_PL_BROADCAST_A_13,
	XAIE_EVENTS_PL_BROADCAST_A_14,
	XAIE_EVENTS_PL_BROADCAST_A_15,
	XAIE_EVENTS_PL_GROUP_USER_EVENT,
	XAIE_EVENTS_PL_USER_EVENT_0,
	XAIE_EVENTS_PL_USER_EVENT_1,
	XAIE_EVENTS_PL_USER_EVENT_2,
	XAIE_EVENTS_PL_USER_EVENT_3,
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
};

/*
 * Data Structure contains all registers and offset info for perf counter
 * of AIE Core and Memory Module.
 */
static const XAie_PerfMod AieTilePerfCnt[] =
{
	{
		.MaxCounterVal = 2U,
		.StartStopShift = 16U,
		.ResetShift = 8U,
		.PerfCounterOffsetAdd = 0x4,
		.PerfCtrlBaseAddr = XAIEGBL_MEM_PERCTRL0,
		.PerfCtrlOffsetAdd = 0x0,
		.PerfCtrlResetBaseAddr = XAIEGBL_MEM_PERCTRL1,
		.PerfCounterBaseAddr = XAIEGBL_MEM_PERCOU0,
		.PerfCounterEvtValBaseAddr = XAIEGBL_MEM_PERCOU0EVTVAL,
		{XAIEGBL_MEM_PERCTRL0_CNT0STAEVT_LSB, XAIEGBL_MEM_PERCTRL0_CNT0STAEVT_MASK},
		{XAIEGBL_MEM_PERCTRL0_CNT0STOPEVT_LSB, XAIEGBL_MEM_PERCTRL0_CNT0STOPEVT_MASK},
		{XAIEGBL_MEM_PERCTRL1_CNT0RSTEVT_LSB, XAIEGBL_MEM_PERCTRL1_CNT0RSTEVT_MASK},
	},
	{
		.MaxCounterVal = 4U,
		.StartStopShift = 16U,
		.ResetShift = 8U,
		.PerfCounterOffsetAdd = 0x4,
		.PerfCtrlBaseAddr = XAIEGBL_CORE_PERCTR0,
		.PerfCtrlOffsetAdd = 0x4,
		.PerfCtrlResetBaseAddr = XAIEGBL_CORE_PERCTR2,
		.PerfCounterBaseAddr = XAIEGBL_CORE_PERCOU0,
		.PerfCounterEvtValBaseAddr = XAIEGBL_CORE_PERCOU0EVTVAL,
		{XAIEGBL_CORE_PERCTR0_CNT0STAEVT_LSB, XAIEGBL_CORE_PERCTR0_CNT0STAEVT_MASK},
		{XAIEGBL_CORE_PERCTR0_CNT0STOPEVT_LSB, XAIEGBL_CORE_PERCTR0_CNT0STOPEVT_MASK},
		{XAIEGBL_CORE_PERCTR2_CNT0RSTEVT_LSB, XAIEGBL_CORE_PERCTR2_CNT0RSTEVT_MASK},
	}
};

/*
 * Data Structure contains all registers and offset info for perf counter
 * of AIE PL Module.
 */
static const XAie_PerfMod AiePlPerfCnt =
{
	.MaxCounterVal = 2U,
	.StartStopShift = 16U,
	.ResetShift = 8U,
	.PerfCounterOffsetAdd = 0x4,
	.PerfCtrlBaseAddr = XAIEGBL_PL_PERCTR0,
	.PerfCtrlOffsetAdd = 0x0,
	.PerfCtrlResetBaseAddr = XAIEGBL_PL_PERCTR1,
	.PerfCounterBaseAddr = XAIEGBL_PL_PERCOU0,
	.PerfCounterEvtValBaseAddr = XAIEGBL_PL_PERCOU0EVTVAL,
	{XAIEGBL_PL_PERCTR0_CNT0STAEVT_LSB, XAIEGBL_PL_PERCTR0_CNT0STAEVT_MASK},
	{XAIEGBL_PL_PERCTR0_CNT0STOPEVT_LSB, XAIEGBL_PL_PERCTR0_CNT0STOPEVT_MASK},
	{XAIEGBL_PL_PERCTR1_CNT0RSTEVT_LSB, XAIEGBL_PL_PERCTR1_CNT0RSTEVT_MASK},
};

static const XAie_EventGroup AieMemGroupEvent[] =
{
	{
		.GroupEvent = XAIE_EVENT_GROUP_0_MEM,
		.GroupOff = 0U,
		.GroupMask = XAIEGBL_MEM_EVTGRP0ENAMSK,
		.ResetValue = XAIEGBL_MEM_EVTGRP0ENAMSK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_WATCHPOINT_MEM,
		.GroupOff = 1U,
		.GroupMask = XAIEGBL_MEM_EVTGRPWTCHPTENAMSK,
		.ResetValue = XAIEGBL_MEM_EVTGRPWTCHPTENAMSK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_DMA_ACTIVITY_MEM,
		.GroupOff = 2U,
		.GroupMask = XAIEGBL_MEM_EVTGRPDMAENAMSK,
		.ResetValue = XAIEGBL_MEM_EVTGRPDMAENAMSK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_LOCK_MEM,
		.GroupOff = 3U,
		.GroupMask = XAIEGBL_MEM_EVTGRPLOCKENAMSK,
		.ResetValue = XAIEGBL_MEM_EVTGRPLOCKENAMSK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_MEMORY_CONFLICT_MEM,
		.GroupOff = 4U,
		.GroupMask = XAIEGBL_MEM_EVTGRPMEMCONENAMSK,
		.ResetValue = XAIEGBL_MEM_EVTGRPMEMCONENAMSK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_ERRORS_MEM,
		.GroupOff = 5U,
		.GroupMask = XAIEGBL_MEM_EVTGRPERRENAMSK,
		.ResetValue = XAIEGBL_MEM_EVTGRPERRENAMSK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_BROADCAST_MEM,
		.GroupOff = 6U,
		.GroupMask = XAIEGBL_MEM_EVTGRPBRDCASTENAMSK,
		.ResetValue = XAIEGBL_MEM_EVTGRPBRDCASTENAMSK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_USER_EVENT_MEM,
		.GroupOff = 7U,
		.GroupMask = XAIEGBL_MEM_EVTGRPUSREVTENAMSK,
		.ResetValue = XAIEGBL_MEM_EVTGRPUSREVTENAMSK,
	},
};

static const XAie_EventGroup AieCoreGroupEvent[] =
{
	{
		.GroupEvent = XAIE_EVENT_GROUP_0_CORE,
		.GroupOff = 0U,
		.GroupMask = XAIEGBL_CORE_EVTGRP0ENAMSK,
		.ResetValue = XAIEGBL_CORE_EVTGRP0ENAMSK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_PC_EVENT_CORE,
		.GroupOff = 1U,
		.GroupMask = XAIEGBL_CORE_EVTGRPPCENAMSK,
		.ResetValue = XAIEGBL_CORE_EVTGRPPCENAMSK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_CORE_STALL_CORE,
		.GroupOff = 2U,
		.GroupMask = XAIEGBL_CORE_EVTGRPCORESTALENAMSK,
		.ResetValue = XAIEGBL_CORE_EVTGRPCORESTALENAMSK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_CORE_PROGRAM_FLOW_CORE,
		.GroupOff = 3U,
		.GroupMask = XAIEGBL_CORE_EVTGRPCOREPRGFLOENAMSK,
		.ResetValue = XAIEGBL_CORE_EVTGRPCOREPRGFLOENAMSK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_ERRORS_0_CORE,
		.GroupOff = 4U,
		.GroupMask = XAIEGBL_CORE_EVTGRPERR0ENAMSK,
		.ResetValue = XAIEGBL_CORE_EVTGRPERR0ENAMSK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_ERRORS_1_CORE,
		.GroupOff = 5U,
		.GroupMask = XAIEGBL_CORE_EVTGRPERR1ENAMSK,
		.ResetValue = XAIEGBL_CORE_EVTGRPERR1ENAMSK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_STREAM_SWITCH_CORE,
		.GroupOff = 6U,
		.GroupMask = XAIEGBL_CORE_EVTGRPSTRSWIENAMSK,
		.ResetValue = XAIEGBL_CORE_EVTGRPSTRSWIENAMSK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_BROADCAST_CORE,
		.GroupOff = 7U,
		.GroupMask = XAIEGBL_CORE_EVTGRPBRDCASTENAMSK,
		.ResetValue = XAIEGBL_CORE_EVTGRPBRDCASTENAMSK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_USER_EVENT_CORE,
		.GroupOff = 8U,
		.GroupMask = XAIEGBL_CORE_EVTGRPUSREVTENAMSK,
		.ResetValue = XAIEGBL_CORE_EVTGRPUSREVTENAMSK,
	},
};

static const XAie_EventGroup AiePlGroupEvent[] =
{
	{
		.GroupEvent = XAIE_EVENT_GROUP_0_PL,
		.GroupOff = 0U,
		.GroupMask = XAIEGBL_PL_EVTGRP0ENAMSK,
		.ResetValue = XAIEGBL_PL_EVTGRP0ENAMSK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_DMA_ACTIVITY_PL,
		.GroupOff = 1U,
		.GroupMask = XAIEGBL_PL_EVTGRPDMAACTENAMSK,
		.ResetValue = XAIEGBL_PL_EVTGRPDMAACTENAMSK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_LOCK_PL,
		.GroupOff = 2U,
		.GroupMask = XAIEGBL_PL_EVTGRPLOCKENAMSK,
		.ResetValue = XAIEGBL_PL_EVTGRPLOCKENAMSK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_ERRORS_PL,
		.GroupOff = 3U,
		.GroupMask = XAIEGBL_PL_EVTGRPERRENAMSK,
		.ResetValue = XAIEGBL_PL_EVTGRPERRENAMSK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_STREAM_SWITCH_PL,
		.GroupOff = 4U,
		.GroupMask = XAIEGBL_PL_EVTGRPSTRSWIENAMSK,
		.ResetValue = XAIEGBL_PL_EVTGRPSTRSWIENAMSK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_BROADCAST_A_PL,
		.GroupOff = 5U,
		.GroupMask = XAIEGBL_PL_EVTGRPBRDCASTAENAMSK,
		.ResetValue = XAIEGBL_PL_EVTGRPBRDCASTAENAMSK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_USER_EVENT_PL,
		.GroupOff = 6U,
		.GroupMask = XAIEGBL_PL_EVTGRPUSRENAMSK,
		.ResetValue = XAIEGBL_PL_EVTGRPUSRENAMSK,
	},
};

/* mapping of user events for core module */
static const XAie_EventMap AieTileCoreModUserEventMap =
{
	.RscId = 0U,
	.Event = XAIE_EVENT_USER_EVENT_0_CORE,
};

/* mapping of user events for memory module */
static const XAie_EventMap AieTileMemModUserEventStart =
{
	.RscId = 0U,
	.Event = XAIE_EVENT_USER_EVENT_0_MEM,
};

/* mapping of user events for memory module */
static const XAie_EventMap ShimTilePlModUserEventStart =
{
	.RscId = 0U,
	.Event = XAIE_EVENT_USER_EVENT_0_PL,
};

/* mapping of broadcast events for core module */
static const XAie_EventMap AieTileCoreModBroadcastEventMap =
{
	.RscId = 0U,
	.Event = XAIE_EVENT_BROADCAST_0_CORE,
};

/* mapping of broadcast events for memory module */
static const XAie_EventMap AieTileMemModBroadcastEventStart =
{
	.RscId = 0U,
	.Event = XAIE_EVENT_BROADCAST_0_MEM,
};

/* mapping of broadcast events for Pl module */
static const XAie_EventMap ShimTilePlModBroadcastEventStart =
{
	.RscId = 0U,
	.Event = XAIE_EVENT_BROADCAST_A_0_PL,
};

static const XAie_EventMap AieTileCoreModPCEventMap =
{
	.RscId = 0U,
	.Event = XAIE_EVENT_PC_0_CORE,
};

/*
 * Data structure to capture core and memory module events properties
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
 *	DMA_MM2S_1_Error.
 * For core module default error group mask enables,
 *	TLAST_in_WSS_words_0_2,
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
 *	Lock_Access_to_Unavailable.
 */
static const XAie_EvntMod AieTileEvntMod[] =
{
	{
		.XAie_EventNumber = AieTileMemModEventMapping,
		.EventMin = XAIE_EVENT_NONE_MEM,
		.EventMax = XAIE_EVENT_USER_EVENT_3_MEM,
		.GenEventRegOff = XAIEGBL_MEM_EVTGEN,
		.GenEvent = {XAIEGBL_MEM_EVTGEN_EVT_LSB, XAIEGBL_MEM_EVTGEN_EVT_MASK},
		.ComboInputRegOff = XAIEGBL_MEM_COMEVTINP,
		.ComboEventMask = XAIEGBL_MEM_COMEVTINP_EVTA_MASK,
		.ComboEventOff = 8U,
		.ComboCtrlRegOff = XAIEGBL_MEM_COMEVTCTRL,
		.ComboConfigMask = XAIEGBL_MEM_COMEVTCTRL_COM0_MASK,
		.ComboConfigOff = 8U,
		.BaseStrmPortSelectRegOff = XAIE_FEATURE_UNAVAILABLE,
		.NumStrmPortSelectIds = XAIE_FEATURE_UNAVAILABLE,
		.StrmPortSelectIdsPerReg = XAIE_FEATURE_UNAVAILABLE,
		.PortIdMask = XAIE_FEATURE_UNAVAILABLE,
		.PortIdOff = XAIE_FEATURE_UNAVAILABLE,
		.PortMstrSlvMask = XAIE_FEATURE_UNAVAILABLE,
		.PortMstrSlvOff = XAIE_FEATURE_UNAVAILABLE,
		.BaseBroadcastRegOff = XAIEGBL_MEM_EVTBRDCAST0,
		.NumBroadcastIds = 16U,
		.BaseBroadcastSwBlockRegOff = XAIEGBL_MEM_EVTBRDCASTBLKSOUSET,
		.BaseBroadcastSwUnblockRegOff = XAIEGBL_MEM_EVTBRDCASTBLKSOUCLR,
		.BroadcastSwOff = 0U,
		.BroadcastSwBlockOff = 16U,
		.BroadcastSwUnblockOff = 16U,
		.NumSwitches = 1U,
		.BaseGroupEventRegOff = XAIEGBL_MEM_EVTGRP0ENA,
		.NumGroupEvents = 8U,
		.DefaultGroupErrorMask = 0x3FFAU,
		.Group = AieMemGroupEvent,
		.BasePCEventRegOff = XAIE_FEATURE_UNAVAILABLE,
		.NumPCEvents = XAIE_FEATURE_UNAVAILABLE,
		.PCAddr = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
		.PCValid = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
		.BaseStatusRegOff = XAIEGBL_MEM_EVTSTA0,
		.NumUserEvents = 4U,
		.UserEventMap = &AieTileMemModUserEventStart,
		.PCEventMap = NULL,
		.BroadcastEventMap = &AieTileMemModBroadcastEventStart,
	},
	{
		.XAie_EventNumber = AieTileCoreModEventMapping,
		.EventMin = XAIE_EVENT_NONE_CORE,
		.EventMax = XAIE_EVENT_USER_EVENT_3_CORE,
		.GenEventRegOff = XAIEGBL_CORE_EVTGEN,
		.GenEvent = {XAIEGBL_CORE_EVTGEN_EVT_LSB, XAIEGBL_CORE_EVTGEN_EVT_MASK},
		.ComboInputRegOff = XAIEGBL_CORE_COMEVTINP,
		.ComboEventMask = XAIEGBL_CORE_COMEVTINP_EVTA_MASK,
		.ComboEventOff = 8U,
		.ComboCtrlRegOff = XAIEGBL_CORE_COMEVTCTRL,
		.ComboConfigMask = XAIEGBL_CORE_COMEVTCTRL_COM0_MASK,
		.ComboConfigOff = 8U,
		.BaseStrmPortSelectRegOff = XAIEGBL_CORE_STRSWIEVTPORTSEL0,
		.NumStrmPortSelectIds = 8U,
		.StrmPortSelectIdsPerReg = 4U,
		.PortIdMask = XAIEGBL_CORE_STRSWIEVTPORTSEL0_PORT0ID_MASK,
		.PortIdOff = 8U,
		.PortMstrSlvMask = XAIEGBL_CORE_STRSWIEVTPORTSEL0_PORT0MSTRSLV_MASK,
		.PortMstrSlvOff = 5U,
		.BaseBroadcastRegOff = XAIEGBL_CORE_EVTBRDCAST0,
		.NumBroadcastIds = 16U,
		.BaseBroadcastSwBlockRegOff = XAIEGBL_CORE_EVTBRDCASTBLKSOUSET,
		.BaseBroadcastSwUnblockRegOff = XAIEGBL_CORE_EVTBRDCASTBLKSOUCLR,
		.BroadcastSwOff = 0U,
		.BroadcastSwBlockOff = 16U,
		.BroadcastSwUnblockOff = 16U,
		.NumSwitches = 1U,
		.BaseGroupEventRegOff = XAIEGBL_CORE_EVTGRP0ENA,
		.NumGroupEvents = 9U,
		.DefaultGroupErrorMask = 0xF5FC0U,
		.Group = AieCoreGroupEvent,
		.BasePCEventRegOff = XAIEGBL_CORE_PCEVT0,
		.NumPCEvents = 4U,
		.PCAddr = {XAIEGBL_CORE_PCEVT0_PCADD_LSB, XAIEGBL_CORE_PCEVT0_PCADD_MASK},
		.PCValid = {XAIEGBL_CORE_PCEVT0_VAL_LSB, XAIEGBL_CORE_PCEVT0_VAL_MASK},
		.BaseStatusRegOff = XAIEGBL_CORE_EVTSTA0,
		.NumUserEvents = 4U,
		.UserEventMap = &AieTileCoreModUserEventMap,
		.PCEventMap = &AieTileCoreModPCEventMap,
		.BroadcastEventMap = &AieTileCoreModBroadcastEventMap,
	},
};

/* Data structure to capture Noc tile events properties.
 * For PL module default error group mask enables,
 *	AXI_MM_Slave_Tile_Error,
 *	Control_Pkt_Error,
 *	AXI_MM_Decode_NSU_Error,
 *	AXI_MM_Slave_NSU_Error,
 *	AXI_MM_Unsupported_Traffic,
 *	AXI_MM_Unsecure_Access_in_Secure_Mode,
 *	AXI_MM_Byte_Strobe_Error,
 *	DMA_S2MM_0_Error,
 *	DMA_S2MM_1_Error,
 *	DMA_MM2S_0_Error,
 *	DMA_MM2S_1_Error.
 */
static const XAie_EvntMod AieNocEvntMod =
{
	.XAie_EventNumber = AieTileNocModEventMapping,
	.EventMin = XAIE_EVENT_NONE_PL,
	.EventMax = XAIE_EVENT_USER_EVENT_3_PL,
	.GenEventRegOff = XAIEGBL_PL_EVTGEN,
	.GenEvent = {XAIEGBL_PL_EVTGEN_EVT_LSB, XAIEGBL_PL_EVTGEN_EVT_MASK},
	.ComboInputRegOff = XAIEGBL_PL_COMEVTINP,
	.ComboEventMask = XAIEGBL_PL_COMEVTINP_EVTA_MASK,
	.ComboEventOff = 8U,
	.ComboCtrlRegOff = XAIEGBL_PL_COMEVTCTRL,
	.ComboConfigMask = XAIEGBL_PL_COMEVTCTRL_COM0_MASK,
	.ComboConfigOff = 8U,
	.BaseStrmPortSelectRegOff = XAIEGBL_PL_STRSWIEVTPORTSEL0,
	.NumStrmPortSelectIds = 8U,
	.StrmPortSelectIdsPerReg = 4U,
	.PortIdMask = XAIEGBL_PL_STRSWIEVTPORTSEL0_PORT0ID_MASK,
	.PortIdOff = 8U,
	.PortMstrSlvMask = XAIEGBL_PL_STRSWIEVTPORTSEL0_PORT0MSTRSLV_MASK,
	.PortMstrSlvOff = 5U,
	.BaseBroadcastRegOff = XAIEGBL_PL_EVTBRDCAST0A,
	.NumBroadcastIds = 16U,
	.BaseBroadcastSwBlockRegOff = XAIEGBL_PL_EVTBRDCASTABLKSOUSET,
	.BaseBroadcastSwUnblockRegOff = XAIEGBL_PL_EVTBRDCASTABLKSOUCLR,
	.BroadcastSwOff = 64U,
	.BroadcastSwBlockOff = 16U,
	.BroadcastSwUnblockOff = 16U,
	.NumSwitches = 2U,
	.BaseGroupEventRegOff = XAIEGBL_PL_EVTGRP0ENA,
	.NumGroupEvents = 7U,
	.DefaultGroupErrorMask = 0x7FFU,
	.Group = AiePlGroupEvent,
	.BasePCEventRegOff = XAIE_FEATURE_UNAVAILABLE,
	.NumPCEvents = XAIE_FEATURE_UNAVAILABLE,
	.PCAddr = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
	.PCValid = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
	.BaseStatusRegOff = XAIEGBL_PL_EVTSTA0,
	.NumUserEvents = 4U,
	.UserEventMap = &ShimTilePlModUserEventStart,
	.BroadcastEventMap = &ShimTilePlModBroadcastEventStart,
	.PCEventMap = NULL,
};

/* Data structure to capture PL module events properties.
 * For PL module default error group mask enables,
 *	AXI_MM_Slave_Tile_Error,
 *	Control_Pkt_Error,
 *	AXI_MM_Decode_NSU_Error,
 *	AXI_MM_Slave_NSU_Error,
 *	AXI_MM_Unsupported_Traffic,
 *	AXI_MM_Unsecure_Access_in_Secure_Mode,
 *	AXI_MM_Byte_Strobe_Error,
 *	DMA_S2MM_0_Error,
 *	DMA_S2MM_1_Error,
 *	DMA_MM2S_0_Error,
 *	DMA_MM2S_1_Error.
 */
static const XAie_EvntMod AiePlEvntMod =
{
	.XAie_EventNumber = AieTilePlModEventMapping,
	.EventMin = XAIE_EVENT_NONE_PL,
	.EventMax = XAIE_EVENT_USER_EVENT_3_PL,
	.GenEventRegOff = XAIEGBL_PL_EVTGEN,
	.GenEvent = {XAIEGBL_PL_EVTGEN_EVT_LSB, XAIEGBL_PL_EVTGEN_EVT_MASK},
	.ComboInputRegOff = XAIEGBL_PL_COMEVTINP,
	.ComboEventMask = XAIEGBL_PL_COMEVTINP_EVTA_MASK,
	.ComboEventOff = 8U,
	.ComboCtrlRegOff = XAIEGBL_PL_COMEVTCTRL,
	.ComboConfigMask = XAIEGBL_PL_COMEVTCTRL_COM0_MASK,
	.ComboConfigOff = 8U,
	.BaseStrmPortSelectRegOff = XAIEGBL_PL_STRSWIEVTPORTSEL0,
	.NumStrmPortSelectIds = 8U,
	.StrmPortSelectIdsPerReg = 4U,
	.PortIdMask = XAIEGBL_PL_STRSWIEVTPORTSEL0_PORT0ID_MASK,
	.PortIdOff = 8U,
	.PortMstrSlvMask = XAIEGBL_PL_STRSWIEVTPORTSEL0_PORT0MSTRSLV_MASK,
	.PortMstrSlvOff = 5U,
	.BaseBroadcastRegOff = XAIEGBL_PL_EVTBRDCAST0A,
	.NumBroadcastIds = 16U,
	.BaseBroadcastSwBlockRegOff = XAIEGBL_PL_EVTBRDCASTABLKSOUSET,
	.BaseBroadcastSwUnblockRegOff = XAIEGBL_PL_EVTBRDCASTABLKSOUCLR,
	.BroadcastSwOff = 64U,
	.BroadcastSwBlockOff = 16U,
	.BroadcastSwUnblockOff = 16U,
	.NumSwitches = 2U,
	.BaseGroupEventRegOff = XAIEGBL_PL_EVTGRP0ENA,
	.NumGroupEvents = 7U,
	.DefaultGroupErrorMask = 0x7FFU,
	.Group = AiePlGroupEvent,
	.BasePCEventRegOff = XAIE_FEATURE_UNAVAILABLE,
	.NumPCEvents = XAIE_FEATURE_UNAVAILABLE,
	.PCAddr = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
	.PCValid = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
	.BaseStatusRegOff = XAIEGBL_PL_EVTSTA0,
	.NumUserEvents = 4U,
	.UserEventMap = &ShimTilePlModUserEventStart,
	.BroadcastEventMap = &ShimTilePlModBroadcastEventStart,
};

/* Data structure to capture core and mem module timer properties */
static XAie_TimerMod AieTileTimerMod[] =
{
	{
		.TrigEventLowValOff = XAIEGBL_MEM_TIMTRIEVTLOWVAL,
		.TrigEventHighValOff = XAIEGBL_MEM_TIMTRIEVTHIGVAL,
		.LowOff = XAIEGBL_MEM_TIMLOW,
		.HighOff = XAIEGBL_MEM_TIMHIG,
		.CtrlOff = XAIEGBL_MEM_TIMCTRL,
		{XAIEGBL_MEM_TIMCTRL_RST_LSB, XAIEGBL_MEM_TIMCTRL_RST_MASK},
		{XAIEGBL_MEM_TIMCTRL_RSTEVT_LSB, XAIEGBL_MEM_TIMCTRL_RSTEVT_MASK},
	},
	{
		.TrigEventLowValOff = XAIEGBL_CORE_TIMTRIEVTLOWVAL,
		.TrigEventHighValOff = XAIEGBL_CORE_TIMTRIEVTHIGVAL,
		.LowOff = XAIEGBL_CORE_TIMLOW,
		.HighOff = XAIEGBL_CORE_TIMHIG,
		.CtrlOff = XAIEGBL_CORE_TIMCTRL,
		{XAIEGBL_CORE_TIMCTRL_RST_LSB, XAIEGBL_CORE_TIMCTRL_RST_MASK},
		{XAIEGBL_CORE_TIMCTRL_RSTEVT_LSB, XAIEGBL_CORE_TIMCTRL_RSTEVT_MASK},
	},
};

/* Data structure to capture PL module timer properties */
static XAie_TimerMod AiePlTimerMod =
{
	.TrigEventLowValOff = XAIEGBL_PL_TIMTRIEVTLOWVAL,
	.TrigEventHighValOff = XAIEGBL_PL_TIMTRIEVTHIGVAL,
	.LowOff = XAIEGBL_PL_TIMLOW,
	.HighOff = XAIEGBL_PL_TIMHIG,
	.CtrlOff = XAIEGBL_PL_TIMCTRL,
	{XAIEGBL_PL_TIMCTRL_RST_LSB, XAIEGBL_PL_TIMCTRL_RST_MASK},
	{XAIEGBL_PL_TIMCTRL_RSTEVT_LSB, XAIEGBL_PL_TIMCTRL_RSTEVT_MASK},
};

/*
 * Data structure to configure trace event register for XAIE_MEM_MOD module
 * type
 */
static const XAie_RegFldAttr AieMemTraceEvent[] =
{
	{XAIEGBL_MEM_TRAEVT0_TRAEVT0_LSB, XAIEGBL_MEM_TRAEVT0_TRAEVT0_MASK},
	{XAIEGBL_MEM_TRAEVT0_TRAEVT1_LSB, XAIEGBL_MEM_TRAEVT0_TRAEVT1_MASK},
	{XAIEGBL_MEM_TRAEVT0_TRAEVT2_LSB, XAIEGBL_MEM_TRAEVT0_TRAEVT2_MASK},
	{XAIEGBL_MEM_TRAEVT0_TRAEVT3_LSB, XAIEGBL_MEM_TRAEVT0_TRAEVT3_MASK},
	{XAIEGBL_MEM_TRAEVT1_TRAEVT4_LSB, XAIEGBL_MEM_TRAEVT1_TRAEVT4_MASK},
	{XAIEGBL_MEM_TRAEVT1_TRAEVT5_LSB, XAIEGBL_MEM_TRAEVT1_TRAEVT5_MASK},
	{XAIEGBL_MEM_TRAEVT1_TRAEVT6_LSB, XAIEGBL_MEM_TRAEVT1_TRAEVT6_MASK},
	{XAIEGBL_MEM_TRAEVT1_TRAEVT7_LSB, XAIEGBL_MEM_TRAEVT1_TRAEVT7_MASK}
};

/*
 * Data structure to configure trace event register for XAIE_CORE_MOD module
 * type
 */
static const XAie_RegFldAttr AieCoreTraceEvent[] =
{
	{XAIEGBL_CORE_TRAEVT0_TRAEVT0_LSB, XAIEGBL_CORE_TRAEVT0_TRAEVT0_MASK},
	{XAIEGBL_CORE_TRAEVT0_TRAEVT1_LSB, XAIEGBL_CORE_TRAEVT0_TRAEVT1_MASK},
	{XAIEGBL_CORE_TRAEVT0_TRAEVT2_LSB, XAIEGBL_CORE_TRAEVT0_TRAEVT2_MASK},
	{XAIEGBL_CORE_TRAEVT0_TRAEVT3_LSB, XAIEGBL_CORE_TRAEVT0_TRAEVT3_MASK},
	{XAIEGBL_CORE_TRAEVT1_TRAEVT4_LSB, XAIEGBL_CORE_TRAEVT1_TRAEVT4_MASK},
	{XAIEGBL_CORE_TRAEVT1_TRAEVT5_LSB, XAIEGBL_CORE_TRAEVT1_TRAEVT5_MASK},
	{XAIEGBL_CORE_TRAEVT1_TRAEVT6_LSB, XAIEGBL_CORE_TRAEVT1_TRAEVT6_MASK},
	{XAIEGBL_CORE_TRAEVT1_TRAEVT7_LSB, XAIEGBL_CORE_TRAEVT1_TRAEVT7_MASK}
};

/*
 * Data structure to configure trace event register for XAIE_PL_MOD module
 * type
 */
static const XAie_RegFldAttr AiePlTraceEvent[] =
{
	{XAIEGBL_PL_TRAEVT0_TRAEVT0_LSB, XAIEGBL_PL_TRAEVT0_TRAEVT0_MASK},
	{XAIEGBL_PL_TRAEVT0_TRAEVT1_LSB, XAIEGBL_PL_TRAEVT0_TRAEVT1_MASK},
	{XAIEGBL_PL_TRAEVT0_TRAEVT2_LSB, XAIEGBL_PL_TRAEVT0_TRAEVT2_MASK},
	{XAIEGBL_PL_TRAEVT0_TRAEVT3_LSB, XAIEGBL_PL_TRAEVT0_TRAEVT3_MASK},
	{XAIEGBL_PL_TRAEVT1_TRAEVT4_LSB, XAIEGBL_PL_TRAEVT1_TRAEVT4_MASK},
	{XAIEGBL_PL_TRAEVT1_TRAEVT5_LSB, XAIEGBL_PL_TRAEVT1_TRAEVT5_MASK},
	{XAIEGBL_PL_TRAEVT1_TRAEVT6_LSB, XAIEGBL_PL_TRAEVT1_TRAEVT6_MASK},
	{XAIEGBL_PL_TRAEVT1_TRAEVT7_LSB, XAIEGBL_PL_TRAEVT1_TRAEVT7_MASK}
};

/*
 * Data structure to configure trace module for XAIEGBL_TILE_TYPE_AIETILE tile
 * type
 */
static const XAie_TraceMod AieTileTraceMod[] =
{
	{
		.CtrlRegOff = XAIEGBL_MEM_TRACTRL0,
		.PktConfigRegOff = XAIEGBL_MEM_TRACTRL1,
		.StatusRegOff = XAIEGBL_MEM_TRASTA,
		.EventRegOffs = (u32 []){XAIEGBL_MEM_TRAEVT0, XAIEGBL_MEM_TRAEVT1},
		.NumTraceSlotIds = 8U,
		.NumEventsPerSlot = 4U,
		.StopEvent = {XAIEGBL_MEM_TRACTRL0_TRASTOPEVT_LSB, XAIEGBL_MEM_TRACTRL0_TRASTOPEVT_MASK},
		.StartEvent = {XAIEGBL_MEM_TRACTRL0_TRASTAEVT_LSB, XAIEGBL_MEM_TRACTRL0_TRASTAEVT_MASK},
		.ModeConfig = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
		.PktType = {XAIEGBL_MEM_TRACTRL1_PKTTYP_LSB, XAIEGBL_MEM_TRACTRL1_PKTTYP_MASK},
		.PktId = {XAIEGBL_MEM_TRACTRL1_ID_LSB, XAIEGBL_MEM_TRACTRL1_ID_MASK},
		.State =  {XAIEGBL_MEM_TRASTA_STA_LSB, XAIEGBL_MEM_TRASTA_STA_MASK},
		.ModeSts = {XAIEGBL_MEM_TRASTA_MOD_LSB, XAIEGBL_MEM_TRASTA_MOD_MASK},
		.Event = AieMemTraceEvent
	},
	{
		.CtrlRegOff = XAIEGBL_CORE_TRACTRL0,
		.PktConfigRegOff = XAIEGBL_CORE_TRACTRL1,
		.StatusRegOff = XAIEGBL_CORE_TRASTA,
		.EventRegOffs = (u32 []){XAIEGBL_CORE_TRAEVT0, XAIEGBL_CORE_TRAEVT1},
		.NumTraceSlotIds = 8U,
		.NumEventsPerSlot = 4U,
		.StopEvent = {XAIEGBL_CORE_TRACTRL0_TRASTOPEVT_LSB, XAIEGBL_CORE_TRACTRL0_TRASTOPEVT_MASK},
		.StartEvent = {XAIEGBL_CORE_TRACTRL0_TRASTAEVT_LSB, XAIEGBL_CORE_TRACTRL0_TRASTAEVT_MASK},
		.ModeConfig = {XAIEGBL_CORE_TRACTRL0_MOD_LSB, XAIEGBL_CORE_TRACTRL0_MOD_MASK},
		.PktType = {XAIEGBL_CORE_TRACTRL1_PKTTYP_LSB, XAIEGBL_CORE_TRACTRL1_PKTTYP_MASK},
		.PktId = {XAIEGBL_CORE_TRACTRL1_ID_LSB, XAIEGBL_CORE_TRACTRL1_ID_MASK},
		.State = {XAIEGBL_CORE_TRASTA_STA_LSB, XAIEGBL_CORE_TRASTA_STA_MASK},
		.ModeSts = {XAIEGBL_CORE_TRASTA_MOD_LSB, XAIEGBL_CORE_TRASTA_MOD_MASK},
		.Event = AieCoreTraceEvent
	}
};

/*
 * Data structure to configure trace module for XAIEGBL_TILE_TYPE_SHIMNOC/PL
 * tile type
 */
static const XAie_TraceMod AiePlTraceMod =
{
	.CtrlRegOff = XAIEGBL_PL_TRACTRL0,
	.PktConfigRegOff = XAIEGBL_PL_TRACTRL1,
	.StatusRegOff = XAIEGBL_PL_TRASTA,
	.EventRegOffs = (u32 []){XAIEGBL_PL_TRAEVT0, XAIEGBL_PL_TRAEVT1},
	.NumTraceSlotIds = 8U,
	.NumEventsPerSlot = 4U,
	.StopEvent = {XAIEGBL_PL_TRACTRL0_TRASTOPEVT_LSB, XAIEGBL_PL_TRACTRL0_TRASTOPEVT_MASK},
	.StartEvent = {XAIEGBL_PL_TRACTRL0_TRASTAEVT_LSB, XAIEGBL_PL_TRACTRL0_TRASTAEVT_MASK},
	.ModeConfig = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
	.PktType = {XAIEGBL_PL_TRACTRL1_PKTTYP_LSB, XAIEGBL_PL_TRACTRL1_PKTTYP_MASK},
	.PktId = {XAIEGBL_PL_TRACTRL1_ID_LSB, XAIEGBL_PL_TRACTRL1_ID_MASK},
	.State = {XAIEGBL_PL_TRASTA_STA_LSB, XAIEGBL_PL_TRASTA_STA_MASK},
	.ModeSts = {XAIEGBL_PL_TRASTA_MOD_LSB, XAIEGBL_PL_TRASTA_MOD_MASK},
	.Event = AiePlTraceEvent
};

/* Data Structure for clock control of PL/NOC tile. */
static XAie_ClockMod AiePlClockMod =
{
	.ClockRegOff = XAIEGBL_PL_TILCLOCTRL,
	{XAIEGBL_PL_TILCLOCTRL_NEXTILCLOENA_LSB, XAIEGBL_PL_TILCLOCTRL_NEXTILCLOENA_MASK},
};

/* Data structure for clock control of AIE tile. */
static XAie_ClockMod AieTileClockMod =
{
	.ClockRegOff = XAIEGBL_CORE_TILCLOCTRL,
	{XAIEGBL_CORE_TILCLOCTRL_NEXTILCLOENA_LSB, XAIEGBL_CORE_TILCLOCTRL_NEXTILCLOENA_MASK},
};

/*
 * Data structure to configures first level interrupt controller for
 * XAIEGBL_TILE_TYPE_SHIMPL tile type
 */
static const XAie_L1IntrMod AiePlL1IntrMod =
{
	.BaseEnableRegOff = XAIEGBL_PL_INTCON1STLEVENAA,
	.BaseDisableRegOff = XAIEGBL_PL_INTCON1STLEVDISA,
	.BaseIrqRegOff = XAIEGBL_PL_INTCON1STLEVIRQNOA,
	.BaseIrqEventRegOff = XAIEGBL_PL_INTCON1STLEVIRQEVTA,
	.BaseIrqEventMask = XAIEGBL_PL_INTCON1STLEVIRQEVTA_IRQEVT0_MASK,
	.BaseBroadcastBlockRegOff = XAIEGBL_PL_INTCON1STLEVBLKNORINASET,
	.BaseBroadcastUnblockRegOff = XAIEGBL_PL_INTCON1STLEVBLKNORINACLR,
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
static const XAie_L2IntrMod AieNoCL2IntrMod =
{
	.EnableRegOff = XAIEGBL_NOC_INTCON2NDLEVENA,
	.DisableRegOff = XAIEGBL_NOC_INTCON2NDLEVDIS,
	.IrqRegOff = XAIEGBL_NOC_INTCON2NDLEVINT,
	.NumBroadcastIds = 16U,
	.NumNoCIntr = 4U,
};

/*
 * AIE Module
 * This data structure captures all the modules for each tile type.
 * Depending on the tile type, this data strcuture can be used to access all
 * hardware properties of individual modules.
 */
XAie_TileMod AieMod[] =
{
	{
		/*
		 * AIE Tile Module indexed using XAIEGBL_TILE_TYPE_AIETILE
		 */
		.NumModules = 2U,
		.CoreMod = &AieCoreMod,
		.StrmSw  = &AieTileStrmSw,
		.DmaMod  = &AieTileDmaMod,
		.MemMod  = &AieTileMemMod,
		.PlIfMod = NULL,
		.LockMod = &AieTileLockMod,
		.PerfMod = AieTilePerfCnt,
		.EvntMod = AieTileEvntMod,
		.TimerMod = AieTileTimerMod,
		.TraceMod = AieTileTraceMod,
		.ClockMod = &AieTileClockMod,
		.L1IntrMod = NULL,
		.L2IntrMod = NULL,
	},
	{
		/*
		 * AIE Shim Noc Module indexed using XAIEGBL_TILE_TYPE_SHIMNOC
		 */
		.NumModules = 1U,
		.CoreMod = NULL,
		.StrmSw  = &AieShimStrmSw,
		.DmaMod  = &AieShimDmaMod,
		.MemMod  = NULL,
		.PlIfMod = &AieShimTilePlIfMod,
		.LockMod = &AieShimNocLockMod,
		.PerfMod = &AiePlPerfCnt,
		.EvntMod = &AieNocEvntMod,
		.TimerMod = &AiePlTimerMod,
		.TraceMod = &AiePlTraceMod,
		.ClockMod = &AiePlClockMod,
		.L1IntrMod = &AiePlL1IntrMod,
		.L2IntrMod = &AieNoCL2IntrMod,
	},
	{
		/*
		 * AIE Shim PL Module indexed using XAIEGBL_TILE_TYPE_SHIMPL
		 */
		.NumModules = 1U,
		.CoreMod = NULL,
		.StrmSw  = &AieShimStrmSw,
		.DmaMod  = NULL,
		.MemMod  = NULL,
		.PlIfMod = &AiePlIfMod,
		.LockMod = NULL,
		.PerfMod = &AiePlPerfCnt,
		.EvntMod = &AiePlEvntMod,
		.TimerMod = &AiePlTimerMod,
		.TraceMod = &AiePlTraceMod,
		.ClockMod = &AiePlClockMod,
		.L1IntrMod = &AiePlL1IntrMod,
		.L2IntrMod = NULL,
	},
	{
		/*
		 * AIE Reserved Module indexed using XAIEGBL_TILE_TYPE_RESERVED
		 */
		.NumModules = 0U,
		.CoreMod = NULL,
		.StrmSw  = NULL,
		.DmaMod  = NULL,
		.MemMod  = NULL,
		.PlIfMod = NULL,
		.LockMod = NULL,
		.PerfMod = NULL,
		.EvntMod = NULL,
		.TimerMod = NULL,
		.TraceMod = NULL,
		.ClockMod = NULL,
		.L1IntrMod = NULL,
		.L2IntrMod = NULL,
	}
};

/** @} */
