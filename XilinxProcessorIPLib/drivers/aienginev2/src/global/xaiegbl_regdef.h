/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaiegbl_regdef.h
* @{
*
* Header to include type definitions for the register bit field definitions
* of Core, Memory, NoC and PL module registers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus  09/26/2019  Initial creation
* 1.1   Tejus  10/21/2019  Optimize stream switch data structures
* 1.2   Tejus  10/28/2019  Add data structures for pl interface module
* 1.3   Tejus  12/09/2019  Forward declaration of structures
* 1.4	Tejus  03/16/2020  Add register properties for Mux/Demux registers
* 1.5   Tejus  03/17/2020  Add data structures for lock module
* 1.6   Tejus  03/21/2020  Add data structures for stream switch slot registers
* 1.7   Tejus  03/23/2020  Re-organize data structure to capture aie dmas
* 1.8  Dishita 03/24/2020  Add data structure for performance counter
* 1.9  Dishita 04/16/2020  Fix compiler warnings related to performance counter
* 2.0  Dishita 04/20/2020  Add data structure for timer
* 2.1   Tejus   05/26/2020  Restructure and optimize core module.
* 2.2   Tejus  06/01/2020  Add data structure for debug halt register.
* 2.3   Tejus  06/05/2020  Add field in data structure for dma fifo mode.
* 2.4   Nishad 06/16/2020  Add data structures for trace module
* 2.5   Nishad 06/28/2020  Add data structures for event selection and combo
*			   event registers
* 2.6   Nishad 07/01/2020  Add MstConfigBaseAddr property to stream switch data
*			   structure
* 2.7   Nishad 07/12/2020  Add data structure and register properties to support
*			   event broadcast, PC event, and group events.
* 2.8   Nishad 07/21/2020  Add data structure for interrupt controller.
* 2.9   Nishad 07/24/2020  Add event property to capture default group error
*			   mask.
* </pre>
*
******************************************************************************/
#ifndef XAIEGBL_REGDEF_H /* prevent circular inclusions */
#define XAIEGBL_REGDEF_H /* by using protection macros */

/***************************** Include Files *********************************/
#include "xaie_events.h"
#include "xaiegbl.h"
#include "xaiegbl_defs.h"
/**************************** Macro Definitions ******************************/
#define XAIE_FEATURE_AVAILABLE 	 1U
#define XAIE_FEATURE_UNAVAILABLE 0U

/************************** Constant Definitions *****************************/
/**
 * This typedef contains the attributes for the register bit fields.
 */
typedef struct {
	u32 Lsb;	/**< Bit position of the register bit-field in the 32-bit value */
	u32 Mask;	/**< Bit mask of the register bit-field in the 32-bit value */
} XAie_RegFldAttr;

/**
 * This typedef contains the attributes for the register bit fields of buffer descriptors.
 */
typedef struct {
	u8  Idx;
	u32 Lsb;	/**< Bit position of the register bit-field in the 32-bit value */
	u32 Mask;	/**< Bit mask of the register bit-field in the 32-bit value */
} XAie_RegBdFldAttr;

/**
 * This typedef contains the attributes for the Core control register.
 */
typedef struct {
	u32 RegOff;			/**< Register offset */
	XAie_RegFldAttr CtrlEn;	/**< Enable field attributes */
	XAie_RegFldAttr CtrlRst;	/**< Reset field attributes */
} XAie_RegCoreCtrl;

/**
 * This typedef contains the attributes for the Core status register.
 */
typedef struct {
	u32 RegOff;			/**< Register offset */
	XAie_RegFldAttr Done;	/**< Done value field attributes */
	XAie_RegFldAttr Rst;	/**< Reset value field attributes */
	XAie_RegFldAttr En;		/**< Enable value field attributes */
} XAie_RegCoreSts;

/*
 * This typedef contains the attributes for core debug halt register
 */
typedef struct {
	u32 RegOff;
	u32 DebugCtrl1Offset;
	XAie_RegFldAttr DebugHalt;
	XAie_RegFldAttr DebugHaltCoreEvent1;
	XAie_RegFldAttr DebugHaltCoreEvent0;
	XAie_RegFldAttr DebugSStepCoreEvent;
	XAie_RegFldAttr DebugResumeCoreEvent;
} XAie_RegCoreDebug;

/*
 * This typedef contains the attributes for enable events register
 */
typedef struct {
	u32 EnableEventOff;
	XAie_RegFldAttr DisableEventOccurred;
	XAie_RegFldAttr EnableEventOccurred;
	XAie_RegFldAttr DisableEvent;
	XAie_RegFldAttr EnableEvent;
} XAie_RegCoreEvents;

/*
 * This typedef captures port base address and number of slave ports available
 * for stream switch master and salve ports
 */
typedef struct {
	u8 NumPorts;
	u32 PortBaseAddr;
} XAie_StrmPort;

/*
 * This typedef captures physical port to logical port mapping for stream
 * switch modules
 */
typedef struct {
	StrmSwPortType PortType;
	u8 PortNum;
} XAie_StrmSwPortMap;

/*
 * This typedef contains the attributes for Stream Switch Module
 */
typedef struct {
	u8 NumSlaveSlots;
	u8 MaxMasterPhyPortId;
	u8 MaxSlavePhyPortId;
	u32 SlvConfigBaseAddr;
	u32 MstrConfigBaseAddr;
	u32 PortOffset;		  /**< Offset between ports */
	u32 SlotOffsetPerPort;
	u32 SlotOffset;
	XAie_RegFldAttr MstrEn;	  /**< Enable bit field attributes */
	XAie_RegFldAttr MstrPktEn;/**< Packet enable bit field attributes */
	XAie_RegFldAttr DrpHdr;   /**< Drop header bit field attributes */
	XAie_RegFldAttr Config;	  /**< Configuration bit field attributes */
	XAie_RegFldAttr SlvEn;	  /**< Enable bit field attributes */
	XAie_RegFldAttr SlvPktEn; /**< Packet enable bit field attributes */
	XAie_RegFldAttr SlotPktId;
	XAie_RegFldAttr SlotMask;
	XAie_RegFldAttr SlotEn;
	XAie_RegFldAttr SlotMsel;
	XAie_RegFldAttr SlotArbitor;
	const XAie_StrmPort *MstrConfig;
	const XAie_StrmPort *SlvConfig;
	const XAie_StrmPort *SlvSlotConfig;
	const XAie_StrmSwPortMap *MasterPortMap;
	const XAie_StrmSwPortMap *SlavePortMap;
} XAie_StrmMod;

/*
 * The typedef contains the attributes of Core Modules
 */
typedef struct XAie_CoreMod {
	u8 IsCheckerBoard;
	u32 ProgMemAddr;
	u32 ProgMemSize;
	u32 ProgMemHostOffset;
	u32 DataMemAddr;
	u32 DataMemSize;
	u32 DataMemShift;
	u32 EccEvntRegOff;
	const XAie_RegCoreSts *CoreSts;
	const XAie_RegCoreCtrl *CoreCtrl;
	const XAie_RegCoreDebug *CoreDebug;
	const XAie_RegCoreEvents *CoreEvent;
	AieRC (*ConfigureDone)(XAie_DevInst *DevInst, XAie_LocType Loc,
			const struct XAie_CoreMod *CoreMod);
	AieRC (*WaitForDone)(XAie_DevInst *DevInst, XAie_LocType Loc,
			u32 TimeOut, const struct XAie_CoreMod *CoreMod);
	AieRC (*ReadDoneBit)(XAie_DevInst *DevInst, XAie_LocType Loc,
			u8 *DoneBit, const struct XAie_CoreMod *CoreMod);
	AieRC (*Enable)(XAie_DevInst *DevInst, XAie_LocType Loc,
			const struct XAie_CoreMod *CoreMod);
} XAie_CoreMod;

/*
 * The typedef captures the Buffer descriptor validity properties
 */
typedef struct {
	XAie_RegBdFldAttr ValidBd;
	XAie_RegBdFldAttr NxtBd;
	XAie_RegBdFldAttr UseNxtBd;
	XAie_RegBdFldAttr OutofOrderBdId;
} XAie_DmaBdEnProp;

/*
 * The typedef captures the buffer descriptor packet properties
 */
typedef struct {
	XAie_RegBdFldAttr EnPkt;
	XAie_RegBdFldAttr PktId;
	XAie_RegBdFldAttr PktType;
} XAie_DmaBdPkt;

/*
 * The typedef captures the buffer descriptor lock properties of aie
 */
typedef struct {
	XAie_RegBdFldAttr LckId_A;
	XAie_RegBdFldAttr LckRelEn_A;
	XAie_RegBdFldAttr LckRelVal_A;
	XAie_RegBdFldAttr LckRelUseVal_A;
	XAie_RegBdFldAttr LckAcqEn_A;
	XAie_RegBdFldAttr LckAcqVal_A;
	XAie_RegBdFldAttr LckAcqUseVal_A;
	XAie_RegBdFldAttr LckId_B;
	XAie_RegBdFldAttr LckRelEn_B;
	XAie_RegBdFldAttr LckRelVal_B;
	XAie_RegBdFldAttr LckRelUseVal_B;
	XAie_RegBdFldAttr LckAcqEn_B;
	XAie_RegBdFldAttr LckAcqVal_B;
	XAie_RegBdFldAttr LckAcqUseVal_B;
} XAie_AieDmaLock;

/*
 * union to capture lock properties of dma
 */
typedef union {
	XAie_AieDmaLock AieDmaLock;
} XAie_DmaBdLock;

/*
 * typedef to capture Buffer properties of tile dma
 */
typedef struct {
	XAie_RegBdFldAttr BaseAddr;
	XAie_RegBdFldAttr BufferLen;
} XAie_TileDmaBuffer;

/*
 * The typedef captures the buffer properties of shim dma
 */
typedef struct {
	XAie_RegBdFldAttr AddrLow;
	XAie_RegBdFldAttr AddrHigh;
	XAie_RegBdFldAttr BufferLen;
} XAie_ShimDmaBuffer;

/*
 * union to capture buffer address and length properties
 */
typedef union {
	XAie_TileDmaBuffer TileDmaBuff;
	XAie_ShimDmaBuffer ShimDmaBuff;
} XAie_DmaBdBuffer;

/*
 * The typedef captures DoubleBuffer properties
 */
typedef struct {
	XAie_RegBdFldAttr EnDoubleBuff;
	XAie_RegBdFldAttr BaseAddr_B;
	XAie_RegBdFldAttr FifoMode;
	XAie_RegBdFldAttr EnIntrleaved;
	XAie_RegBdFldAttr IntrleaveCnt;
	XAie_RegBdFldAttr BuffSelect;
} XAie_DmaBdDoubleBuffer;

/*
 * The typedef captures buffer descriptor fields of aie 2D Mode
 */
typedef struct {
	XAie_RegBdFldAttr X_Incr;
	XAie_RegBdFldAttr X_Wrap;
	XAie_RegBdFldAttr X_Offset;
	XAie_RegBdFldAttr Y_Incr;
	XAie_RegBdFldAttr Y_Wrap;
	XAie_RegBdFldAttr Y_Offset;
	XAie_RegBdFldAttr CurrPtr;
} XAie_AieAddressMode;

/*
 * union captures multi dimension address generation properties between hardware
 * generations
 */
typedef union {
	XAie_AieAddressMode AieMultiDimAddr;
} XAie_DmaBdMultiDimAddr;

/*
 * The typedef captures system level properties of DMA. This is applicable only
 * for SHIM DMA
 */
typedef struct {
	XAie_RegBdFldAttr SMID;
	XAie_RegBdFldAttr BurstLen;
	XAie_RegBdFldAttr AxQos;
	XAie_RegBdFldAttr SecureAccess;
	XAie_RegBdFldAttr AxCache;
} XAie_DmaSysProp;

/*
 * The typedef captures all the buffer descriptor properties for AIE DMAs
 */
typedef struct {
	u64 AddrMax;
	u8 AddrAlignMask;
	u8 AddrAlignShift;
	u8 LenActualOffset;
	const XAie_DmaBdBuffer *Buffer;
	const XAie_DmaBdDoubleBuffer *DoubleBuffer;
	const XAie_DmaBdLock *Lock;
	const XAie_DmaBdPkt *Pkt;
	const XAie_DmaBdEnProp *BdEn;
	const XAie_DmaBdMultiDimAddr *AddrMode;
	const XAie_DmaSysProp *SysProp;
} XAie_DmaBdProp;

typedef struct {
	XAie_RegFldAttr Status;
	XAie_RegFldAttr StartQSize;
	XAie_RegFldAttr Stalled;
} XAie_AieDmaChStatus;

typedef union {
	XAie_AieDmaChStatus AieDmaChStatus;
} XAie_DmaChStatus;

/*
 * The typedef contains the attributes of the Dma Channels
 */
typedef struct {
	u8 StartQSizeMax;
	XAie_RegBdFldAttr StartBd;
	XAie_RegBdFldAttr Reset;
	XAie_RegBdFldAttr Enable;
	XAie_RegBdFldAttr PauseMem;
	XAie_RegBdFldAttr PauseStream;
	const XAie_DmaChStatus *DmaChStatus;
} XAie_DmaChProp;

/*
 * The typedef contains attributes of Dma Modules for AIE Tiles and Mem Tiles
 */
typedef struct XAie_DmaMod {
	u8  NumBds;
	u8  NumLocks;
	u8  ChIdxOffset;
	u8  NumAddrDim;
	u8  DoubleBuffering;
	u8  InterleaveMode;
	u8  FifoMode;
	u32 BaseAddr;
	u32 IdxOffset;
	u32 ChCtrlBase;
	u32 NumChannels;
	u32 ChStatusBase;
	u32 ChStatusOffset;
	const XAie_DmaBdProp *BdProp;
	const XAie_DmaChProp *ChProp;
	void (*DmaBdInit)(XAie_DmaDesc *Desc);
	AieRC (*SetLock) (XAie_DmaDesc *Desc, XAie_Lock Acq,
			XAie_Lock Rel, u8 AcqEn, u8 RelEn);
	AieRC (*SetIntrleave) (XAie_DmaDesc *Desc, u8 DoubleBuff,
			u8 IntrleaveCount, u16 IntrleaveCurr);
	AieRC (*SetMultiDim) (XAie_DmaDesc *Desc, XAie_DmaTensor *Tensor);
	AieRC (*WriteBd)(XAie_DevInst *DevInst, XAie_DmaDesc *Desc,
			XAie_LocType Loc, u8 BdNum);
	AieRC (*PendingBd)(XAie_DevInst *DevInst, XAie_LocType Loc,
			const XAie_DmaMod *DmaMod, u8 ChNum,
			XAie_DmaDirection Dir, u8 *PendingBd);
	AieRC (*WaitforDone)(XAie_DevInst *DevINst, XAie_LocType Loc,
			const XAie_DmaMod *DmaMod, u8 ChNum,
			XAie_DmaDirection Dir, u32 TimeOutUs);
} XAie_DmaMod;

/*
 * The typedef contains the attributes of Memory Module
 */
typedef struct {
	u32 Size;
	u32 MemAddr;
	u32 EccEvntRegOff;
} XAie_MemMod;

/*
 * The typedef contains the attributes of reset configuration
 */
typedef struct {
	u32 RegOff;
	XAie_RegFldAttr RstCntr;
	AieRC (*RstShims)(XAie_DevInst *DevInst, u32 StartCol, u32 NumCols);
} XAie_ShimRstMod;

/*
 * The typedef contains the attributes of SHIM NOC AXI MM configuration
 */
typedef struct {
	u32 RegOff;
	XAie_RegFldAttr NsuSlvErr;
	XAie_RegFldAttr NsuDecErr;
} XAie_ShimNocAxiMMConfig;

/*
 * The typedef contains the attributes of SHIM clock buffer configuration
 */
typedef struct {
	u32 RegOff;
	u32 RstEnable;			/* Reset value of enable bit */
	XAie_RegFldAttr ClkBufEnable;
} XAie_ShimClkBufCntr;

/*
 * The typedef contains attributes of PL interface module
 */
typedef struct {
	u32 UpSzrOff;
	u32 DownSzrOff;
	u32 DownSzrEnOff;
	u32 DownSzrByPassOff;
	u32 ShimNocMuxOff;
	u32 ShimNocDeMuxOff;
	u32 ColRstOff;
	u8  NumUpSzrPorts;
	u8  MaxByPassPortNum;
	u8  NumDownSzrPorts;
	const XAie_RegFldAttr	*UpSzr32_64Bit;
	const XAie_RegFldAttr *UpSzr128Bit;
	const XAie_RegFldAttr	*DownSzr32_64Bit;
	const XAie_RegFldAttr *DownSzr128Bit;
	const XAie_RegFldAttr *DownSzrEn;
	const XAie_RegFldAttr *DownSzrByPass;
	const XAie_RegFldAttr *ShimNocMux;
	const XAie_RegFldAttr *ShimNocDeMux;
	const XAie_ShimClkBufCntr *ClkBufCntr; /* Shim clock buffer control configuration */
	XAie_RegFldAttr ColRst; /* Tile column reset configuration */
	const XAie_ShimRstMod *ShimTileRst; /* SHIM tile reset enable configuration */
	const XAie_ShimNocAxiMMConfig *ShimNocAxiMM; /* SHIM NOC AXI MM configuration */
} XAie_PlIfMod;

/*
 * The typdef contains attributes of Lock modules.
 * The lock acquire and release mechanism for lock module are different across
 * hardware generations. In the first generation, the lock is acquired by
 * reading a register via AXI-MM path. The register address is specific to
 * the lock number and whether the lock is being acquired or released with
 * value. However, in subsequent generations, the lock access mechanism is
 * different. The register address to read from is computed differently.
 * To hide this change in the architecture, the information captured in the
 * below data strcuture does not use the register database directly. For more
 * details, please refer to the AI Engine hardware architecture specification
 * document.
 */
typedef struct XAie_LockMod {
	u8  NumLocks;		/* Number of lock in the module */
	s8  LockValUpperBound; 	/* Upper bound of the lock value */
	s8  LockValLowerBound; 	/* Lower bound of the lock value */
	u32 BaseAddr;		/* Base address of the lock module */
	u32 LockIdOff;		/* Offset between conseccutive locks */
	u32 RelAcqOff;  	/* Offset between Release and Acquire locks */
	u32 LockValOff; 	/* Offset thats added to the lock address for a value. */
	AieRC (*Acquire)(XAie_DevInst *DevInst,
			const struct XAie_LockMod *LockMod, XAie_LocType Loc,
			XAie_Lock Lock, u32 TimeOut);
	AieRC (*Release)(XAie_DevInst *DevInst,
			const struct XAie_LockMod *LockMod, XAie_LocType Loc,
			XAie_Lock Lock, u32 TimeOut);
} XAie_LockMod;

/* This typedef contains attributes of Performace Counter module */
typedef struct XAie_PerfMod {
	u8 MaxCounterVal;       /* Maximum counter value per module */
	u8 StartStopShift;      /* Shift for start stop perf ctrl reg */
	u8 ResetShift;          /* Shift for reset perf ctrl reg */
	u8 PerfCounterOffsetAdd;/* Add to calc perf cntrl offset for counter */
	u32 PerfCtrlBaseAddr;   /* Perf counter ctrl register offset address */
	u32 PerfCtrlOffsetAdd;  /* Add this val for next Perf counter ctrl reg*/
	u32 PerfCtrlResetBaseAddr;/* Perf counter ctrl offset addr for reset */
	u32 PerfCounterBaseAddr; /* Offset addr for perf counter 0 */
	u32 PerfCounterEvtValBaseAddr; /* Offset addr for perf counter evnt val*/
	const XAie_RegFldAttr Start; /* lsb and mask for start event for ctr0 */
	const XAie_RegFldAttr Stop; /* lsb and mask for stop event for ctr0 */
	const XAie_RegFldAttr Reset; /* lsb and mask for reset event for ctr0 */
} XAie_PerfMod;

typedef struct XAie_EventGroup {
	XAie_Events GroupEvent;
	u8 GroupOff;
	u32 GroupMask;
	u32 ResetValue;
} XAie_EventGroup;

/* structure to capture RscId to Events mapping */
typedef struct {
	u8 RscId;
	XAie_Events Event;
} XAie_EventMap;

/* This typedef contains attributes of Events module */
typedef struct XAie_EvntMod {
	const u8 *XAie_EventNumber;	/* Array of event numbers with true event val */
	u32 EventMin;		/* number corresponding to evt 0 in the enum */
	u32 EventMax;		/* number corresponding to last evt in enum */
	u32 GenEventRegOff;
	XAie_RegFldAttr GenEvent;
	u32 ComboInputRegOff;
	u32 ComboEventMask;
	u8 ComboEventOff;
	u32 ComboCtrlRegOff;
	u32 ComboConfigMask;
	u8 ComboConfigOff;
	u32 BaseStrmPortSelectRegOff;
	u32 NumStrmPortSelectIds;
	u32 StrmPortSelectIdsPerReg;
	u32 PortIdMask;
	u8 PortIdOff;
	u32 PortMstrSlvMask;
	u8 PortMstrSlvOff;
	u32 BaseBroadcastRegOff;
	u8 NumBroadcastIds;
	u32 BaseBroadcastSwBlockRegOff;
	u32 BaseBroadcastSwUnblockRegOff;
	u8 BroadcastSwOff;
	u8 BroadcastSwBlockOff;
	u8 BroadcastSwUnblockOff;
	u8 NumSwitches;
	u32 BaseGroupEventRegOff;
	u8 NumGroupEvents;
	u32 DefaultGroupErrorMask;
	const XAie_EventGroup *Group;
	u32 BasePCEventRegOff;
	u8 NumPCEvents;
	XAie_RegFldAttr PCAddr;
	XAie_RegFldAttr PCValid;
	u32 BaseStatusRegOff;
	u8 NumUserEvents;
	const XAie_EventMap *UserEventMap;
	const XAie_EventMap *PCEventMap;
	const XAie_EventMap *BroadcastEventMap;
} XAie_EvntMod;

/* This typedef contains attributes of timer module */
typedef struct XAie_TimerMod {
	u32 TrigEventLowValOff;  /* Timer trigger evel low val register offset */
	u32 TrigEventHighValOff; /* Timer trigger evel high val register offset */
	u32 LowOff;              /* Timer low value Register offset */
	u32 HighOff;             /* Timer high value Register offset */
	u32 CtrlOff;             /* Timer control Register offset */
	const XAie_RegFldAttr CtrlReset; /* Timer control reset field */
	const XAie_RegFldAttr CtrlResetEvent; /* Timer control reset event field */
} XAie_TimerMod;

/* This structure captures all the attributes relevant to trace module */
typedef struct {
	u32 CtrlRegOff;
	u32 PktConfigRegOff;
	u32 StatusRegOff;
	u32 *EventRegOffs;
	u8 NumTraceSlotIds;
	u8 NumEventsPerSlot;
	XAie_RegFldAttr StopEvent;
	XAie_RegFldAttr StartEvent;
	XAie_RegFldAttr ModeConfig;
	XAie_RegFldAttr PktType;
	XAie_RegFldAttr PktId;
	XAie_RegFldAttr State;
	XAie_RegFldAttr ModeSts;
	const XAie_RegFldAttr *Event;
} XAie_TraceMod;

/* This structure captures all attributes related to Clock Module */
typedef struct XAie_ClockMod {
	u32 ClockRegOff;
	const XAie_RegFldAttr NextTileClockCntrl;
} XAie_ClockMod;

/*
 * This structure captures all attributes related to first level interrupt
 * controller.
 */
typedef struct XAie_L1IntrMod {
	u32 BaseEnableRegOff;
	u32 BaseDisableRegOff;
	u32 BaseIrqRegOff;
	u32 BaseIrqEventRegOff;
	u32 BaseIrqEventMask;
	u32 BaseBroadcastBlockRegOff;
	u32 BaseBroadcastUnblockRegOff;
	u8 SwOff;
	u8 NumIntrIds;
	u8 NumIrqEvents;
	u8 IrqEventOff;
	u8 NumBroadcastIds;
} XAie_L1IntrMod;

/*
 * This structure captures all attributes related to second level interrupt
 * controller.
 */
typedef struct XAie_L2IntrMod {
	u32 EnableRegOff;
	u32 DisableRegOff;
	u32 IrqRegOff;
	u8 NumBroadcastIds;
	u8 NumNoCIntr;
} XAie_L2IntrMod;

/*
 * This structure captures all attributes related to resource manager.
 */
typedef struct XAie_ResourceManager {
	u32 **Bitmaps;
} XAie_ResourceManager;

/*
 * This typedef contains all the modules for a Tile type
 */
typedef struct XAie_TileMod {
	 const u8 NumModules;
	const XAie_CoreMod *CoreMod;
	const XAie_StrmMod *StrmSw;
	const XAie_DmaMod  *DmaMod;
	const XAie_MemMod  *MemMod;
	const XAie_PlIfMod *PlIfMod;
	const XAie_LockMod *LockMod;
	const XAie_PerfMod *PerfMod;
	const XAie_EvntMod *EvntMod;
	const XAie_TimerMod *TimerMod;
	const XAie_TraceMod *TraceMod;
	const XAie_ClockMod *ClockMod;
	const XAie_L1IntrMod *L1IntrMod;
	const XAie_L2IntrMod *L2IntrMod;
} XAie_TileMod;

#endif

/** @} */
