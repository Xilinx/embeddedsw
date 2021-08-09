/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaiegbl.h
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
* 1.0   Tejus   09/24/2019  Initial creation
* 1.1   Tejus   10/28/2019  Add error type for pl interface apis
* 1.2   Tejus   12/09/2019  Forward declaration of structures
* 1.3   Tejus   03/17/2020  Add error types and data structure for lock apis
* 1.4   Tejus   03/21/2020  Add data strcuture and initialization function for
*			    packets.
* 1.5   Tejus   03/22/2020  Remove data strcutures from initial dma api
*			    implementation
* 1.6   Tejus   03/23/2020  Add data structures for common dma apis.
* 1.7   Tejus   04/13/2020  Remove range structure and helper function.
* 1.8   Dishita 04/27/2020  Add enum for reset and modules.
* 1.9   Tejus   06/05/2020  Change name of FifoMode field.
* 2.0   Nishad  06/18/2020  Add macros for max value of packet Id and type.
* 2.1   Tejus   06/10/2020  Add IO backend data structures.
* 2.2   Tejus   06/10/2020  Add ess simulation backend.
* 2.3   Tejus   06/10/2020  Add api to change backend at runtime.
* </pre>
*
******************************************************************************/

#ifndef XAIEGBL_H /* prevent circular inclusions */
#define XAIEGBL_H /* by using protection macros */

/***************************** Include Files *********************************/
#include "xaiegbl_defs.h"

/************************** Constant Definitions *****************************/
#define XAIE_LOCK_WITH_NO_VALUE		(-1)
#define XAIE_PACKET_ID_MAX		0x1F
#define XAIE_PACKET_TYPE_MAX		0x7
#define XAIE_TILES_BITMAP_SIZE          32

#define XAIE_TRANSACTION_ENABLE_AUTO_FLUSH	0b1
#define XAIE_TRANSACTION_DISABLE_AUTO_FLUSH	0b0

#define XAIE_PART_INIT_OPT_COLUMN_RST		(1U << 0)
#define XAIE_PART_INIT_OPT_SHIM_RST		(1U << 1)
#define XAIE_PART_INIT_OPT_BLOCK_NOCAXIMMERR	(1U << 2)
#define XAIE_PART_INIT_OPT_ISOLATE		(1U << 3)
#define XAIE_PART_INIT_OPT_ZEROIZEMEM		(1U << 4)
#define XAIE_PART_INIT_OPT_DEFAULT	(XAIE_PART_INIT_OPT_COLUMN_RST | \
		XAIE_PART_INIT_OPT_SHIM_RST | \
		XAIE_PART_INIT_OPT_BLOCK_NOCAXIMMERR | \
		XAIE_PART_INIT_OPT_ISOLATE)

/**************************** Type Definitions *******************************/
typedef struct XAie_TileMod XAie_TileMod;
typedef struct XAie_DeviceOps XAie_DeviceOps;
typedef struct XAie_DmaMod XAie_DmaMod;
typedef struct XAie_LockMod XAie_LockMod;
typedef struct XAie_Backend XAie_Backend;
typedef struct XAie_TxnCmd XAie_TxnCmd;
typedef struct XAie_ResourceManager XAie_ResourceManager;

/*
 * This typedef captures all the properties of a AIE Device
 */
typedef struct XAie_DevProp {
	u8 DevGen;
	u8 RowShift;
	u8 ColShift;
	XAie_TileMod *DevMod;
} XAie_DevProp;

/*
 * This typedef captures all the IO Backends supported by the driver
 */
typedef enum {
	XAIE_IO_BACKEND_METAL, /* Linux backend. Default backend of driver. */
	XAIE_IO_BACKEND_SIM,   /* Ess simulation backend */
	XAIE_IO_BACKEND_CDO,   /* Cdo generation backend */
	XAIE_IO_BACKEND_BAREMETAL, /* Baremetal backend */
	XAIE_IO_BACKEND_DEBUG, /* IO debug backend */
	XAIE_IO_BACKEND_LINUX, /* Linux kernel backend */
	XAIE_IO_BACKEND_MAX
} XAie_BackendType;

/*
 * This typedef contains the attributes for an AIE partition properties.
 * It will contain the fields required to intialize the AIE partition software
 * instance.
 */
typedef struct {
	u64 Handle;	/* AI engine partition handle. If AI engine handle is
			 * specified, the NID, and the UID will be ignored. It
			 * is used in case of Linux Xilinx runtime stack. */
	u32 Nid;	/* Partition Node ID */
	u32 Uid;	/* UID of the image runs on the AI engine partition */
	u32 CntrFlag;	/* AI enigne parition control flag. E.g.to indicate
			 * if to reset and gate all the tiles when the parition
			 * is closed. */
} XAie_PartitionProp;

/* Generic linked list structure */
typedef struct XAie_List {
	struct XAie_List *Next;
} XAie_List;

/*
 * This typedef contains the attributes for an AIE partition. The structure is
 * setup during intialization.
 */
typedef struct {
	u64 BaseAddr; /* Base address of the partition*/
	u8 NumRows;   /* Number of rows allocated to the partition */
	u8 NumCols;   /* Number of cols allocated to the partition */
	u8 ShimRow;   /* ShimRow location */
	u8 MemTileRowStart; /* Mem tile starting row in the partition */
	u8 MemTileNumRows;  /* Number of memtile rows in the partition */
	u8 AieTileRowStart; /* Aie tile starting row in the partition */
	u8 AieTileNumRows;  /* Number of aie tile rows in the partition */
	u8 IsReady;
	u8 EccStatus;		/* Ecc On/Off status of the partition */
	u32 TilesInUse[XAIE_TILES_BITMAP_SIZE]; /* Bitmap for tiles in use */
	u32 MemInUse[XAIE_TILES_BITMAP_SIZE];/* Bitmap for ECC status of DM */
	u32 CoreInUse[XAIE_TILES_BITMAP_SIZE];/* Bitmap for ECC status of PM */
	const XAie_Backend *Backend; /* Backend IO properties */
	XAie_ResourceManager *RscMapping;
	void *IOInst;	       /* IO Instance for the backend */
	XAie_DevProp DevProp; /* Pointer to the device property. To be
				     setup to AIE prop during intialization*/
	XAie_DeviceOps *DevOps; /* Device level operations */
	XAie_PartitionProp PartProp; /* Partition property */
	XAie_List TxnList; /* Head of the list of txn buffers */
} XAie_DevInst;

/* typedef to capture transaction buffer data */
typedef struct {
	u64 Tid;
	u32 Flags;
	u32 NumCmds;
	u32 MaxCmds;
	XAie_TxnCmd *CmdBuf;
	XAie_List Node;
} XAie_TxnInst;

/* enum to capture cache property of allocate memory */
typedef enum {
	XAIE_MEM_CACHEABLE,
	XAIE_MEM_NONCACHEABLE
} XAie_MemCacheProp;

/* typedef to capture properties of an allcoated memory buffer */
typedef struct {
	u64 Size;
	void *VAddr;
	u64 DevAddr;
	XAie_MemCacheProp Cache;
	XAie_DevInst *DevInst;
	void *BackendHandle; /* Backend specific properties */
} XAie_MemInst;

typedef struct {
	u8 AieGen;
	u64 BaseAddr;
	u8 ColShift;
	u8 RowShift;
	u8 NumRows;
	u8 NumCols;
	u8 ShimRowNum;
	u8 MemTileRowStart;
	u8 MemTileNumRows;
	u8 AieTileRowStart;
	u8 AieTileNumRows;
	XAie_PartitionProp PartProp;
} XAie_Config;

/*
 * This typedef contains attributes for a tile coordinate.
 */
typedef struct {
	u8 Row;
	u8 Col;
} XAie_LocType;

/*
 * This typedef contains the attributes for an AIE partition initialization
 * options. The structure is used by the AI engine partition initialization
 * API.
 */
typedef struct XAie_PartInitOpts {
	XAie_LocType *Locs; /* Array of tiles locactions which will be used */
	u32 NumUseTiles; /* Number of tiles to use */
	u32 InitOpts; /* AI engine partition initialization options */
} XAie_PartInitOpts;

/*
 * This enum contains all the Stream Switch Port types. These enums are used to
 * access the base address of stream switch configuration registers.
 */
typedef enum{
	CORE,
	DMA,
	CTRL,
	FIFO,
	SOUTH,
	WEST,
	NORTH,
	EAST,
	TRACE,
	SS_PORT_TYPE_MAX
} StrmSwPortType;

/* Data structures to capture data shape for dmas */
typedef struct {
	u32 StepSize;
	u32 Wrap;
} XAie_AieMlDmaDimDesc;

typedef struct {
	u32 Offset;
	u32 Incr;
	u32 Wrap;
} XAie_AieDmaDimDesc;

typedef union {
	XAie_AieDmaDimDesc AieDimDesc;
	XAie_AieMlDmaDimDesc AieMlDimDesc;
} XAie_DmaDimDesc;

typedef struct {
	u8 NumDim;
	XAie_DmaDimDesc *Dim;
} XAie_DmaTensor;

typedef struct {
	u8 LockAcqId;
	u8 LockRelId;
	u8 LockAcqEn;
	s8 LockAcqVal;
	u8 LockAcqValEn;
	u8 LockRelEn;
	s8 LockRelVal;
	u8 LockRelValEn;
} XAie_LockDesc;

typedef struct {
	u8 PktId;
	u8 PktType;
	u8 PktEn;
} XAie_PktDesc;

typedef struct {
	u64 Address;
	u32 Length;
} XAie_AddrDesc;

typedef struct {
	u8 ValidBd;
	u8 NxtBd;
	u8 UseNxtBd;
	u8 OutofOrderBdId;
} XAie_BdEnDesc;

typedef struct {
	u8 SMID;
	u8 BurstLen;
	u8 AxQos;
	u8 SecureAccess;
	u8 AxCache;
} XAie_DmaAxiDesc;

typedef struct {
	u8 X_Incr;
	u8 X_Wrap;
	u16 X_Offset;
	u8 Y_Incr;
	u8 Y_Wrap;
	u16 Y_Offset;
	u8 IntrleaveBufSelect;
	u16 CurrPtr;
	u8 IntrleaveCount;
	u8 EnInterleaved;
} XAie_AieMultiDimDesc;

typedef struct {
	u16 Wrap;
	u32 StepSize;
} XAie_AieMlDimDesc;

typedef struct {
	u8 IterCurr;
	XAie_AieMlDimDesc IterDesc;
	XAie_AieMlDimDesc DimDesc[4U];	/* Max 4D addressing supported */
} XAie_AieMlMultiDimDesc;

typedef union {
	XAie_AieMultiDimDesc AieMultiDimDesc;
	XAie_AieMlMultiDimDesc AieMlMultiDimDesc;
} XAie_MultiDimDesc;

typedef struct {
	u8 Before;
	u8 After;
} XAie_PadDesc;

typedef struct {
	u8 NumDim;
	XAie_PadDesc *PadDesc;
} XAie_DmaPadTensor;

typedef struct {
	u8 EnOutofOrderId;
	u8 EnTokenIssue;
	u8 EnCompression;
	u8 FoTMode;
	u8 TileType;
	u8 IsReady;
	u32 ControllerId;
	const XAie_DmaMod *DmaMod;
} XAie_DmaChannelDesc;

typedef struct {
	XAie_PktDesc PktDesc;
	XAie_LockDesc LockDesc;
	XAie_AddrDesc AddrDesc;
	XAie_DmaAxiDesc AxiDesc;
	XAie_BdEnDesc BdEnDesc;
	XAie_LockDesc LockDesc_2;
	XAie_AddrDesc AddrDesc_2;
	XAie_MultiDimDesc MultiDimDesc;
	XAie_PadDesc PadDesc[3U];
	const XAie_DmaMod *DmaMod;
	const XAie_LockMod *LockMod;
	XAie_MemInst *MemInst;
	u8 EnDoubleBuff;
	u8 FifoMode;
	u8 EnCompression;
	u8 EnOutofOrderBdId;
	u8 TlastSuppress;
	u8 TileType;
	u8 IsReady;
} XAie_DmaDesc;

typedef struct {
	u32 RepeatCount;
	u8 StartBd;
	u8 EnTokenIssue;
	u8 OutOfOrder;
} XAie_DmaQueueDesc;

/*
 * This enum contains the dma channel reset for aie dmas.
 */
typedef enum {
	DMA_CHANNEL_UNRESET,
	DMA_CHANNEL_RESET
} XAie_DmaChReset;
/*
 * This enum contains the dma direction for aie dmas.
 */
typedef enum {
	DMA_S2MM,
	DMA_MM2S,
	DMA_MAX
} XAie_DmaDirection;

/*
 * This enum contains the FoT mode for aie Dma Channel.
 */
typedef enum {
	DMA_FoT_DISABLED,
	DMA_FoT_NO_COUNTS,
	DMA_FoT_COUNTS_WITH_TASK_TOKENS,
	DMA_FoT_COUNTS_FROM_MM_REG,
} XAie_DmaChannelFoTMode;

/*
 * This enum contains the positions for dma zero padding.
 */
typedef enum {
	DMA_ZERO_PADDING_BEFORE,
	DMA_ZERO_PADDING_AFTER,
} XAie_DmaZeroPaddingPos;
/*
 * This enum captures all the error codes from the driver
 */
typedef enum{
	XAIE_OK,
	XAIE_ERR,
	XAIE_INVALID_DEVICE,
	XAIE_INVALID_RANGE,
	XAIE_INVALID_ARGS,
	XAIE_INVALID_TILE,
	XAIE_ERR_STREAM_PORT,
	XAIE_INVALID_DMA_TILE,
	XAIE_INVALID_BD_NUM,
	XAIE_ERR_OUTOFBOUND,
	XAIE_INVALID_DATA_MEM_ADDR,
	XAIE_INVALID_ELF,
	XAIE_CORE_STATUS_TIMEOUT,
	XAIE_INVALID_CHANNEL_NUM,
	XAIE_INVALID_LOCK,
	XAIE_INVALID_DMA_DIRECTION,
	XAIE_INVALID_PLIF_WIDTH,
	XAIE_INVALID_LOCK_ID,
	XAIE_INVALID_LOCK_VALUE,
	XAIE_LOCK_RESULT_FAILED,
	XAIE_INVALID_DMA_DESC,
	XAIE_INVALID_ADDRESS,
	XAIE_FEATURE_NOT_SUPPORTED,
	XAIE_INVALID_BURST_LENGTH,
	XAIE_INVALID_BACKEND,
	XAIE_ERR_MAX
} AieRC;

/*
 * This enum is to identify different hardware modules within a tile type.
 * An AIE tile can have memory or core module. A PL or Shim tile will have
 * Pl module. A mem tile will have memory module. Any hardware module
 * addition in future generations of AIE needs to be appended to this enum.
 */
typedef enum{
	XAIE_MEM_MOD,
	XAIE_CORE_MOD,
	XAIE_PL_MOD,
} XAie_ModuleType;

/* This enum contains reset input values. */
typedef enum {
	XAIE_RESETDISABLE,
	XAIE_RESETENABLE,
} XAie_Reset;

/* Data structure to capture lock id and value */
typedef struct {
	u8 LockId;
	s8 LockVal;
} XAie_Lock;

/* Data structure to capture packet id and packet type */
typedef struct {
	u8 PktId;
	u8 PktType;
} XAie_Packet;

/**************************** Function prototypes ***************************/
AieRC XAie_CfgInitialize(XAie_DevInst *InstPtr, XAie_Config *ConfigPtr);
AieRC XAie_PartitionInitialize(XAie_DevInst *DevInst, XAie_PartInitOpts *Opts);
AieRC XAie_PartitionTeardown(XAie_DevInst *DevInst);
AieRC XAie_Finish(XAie_DevInst *DevInst);
AieRC XAie_SetIOBackend(XAie_DevInst *DevInst, XAie_BackendType Backend);
XAie_MemInst* XAie_MemAllocate(XAie_DevInst *DevInst, u64 Size,
		XAie_MemCacheProp Cache);
AieRC XAie_MemFree(XAie_MemInst *MemInst);
AieRC XAie_MemSyncForCPU(XAie_MemInst *MemInst);
AieRC XAie_MemSyncForDev(XAie_MemInst *MemInst);
void* XAie_MemGetVAddr(XAie_MemInst *MemInst);
u64 XAie_MemGetDevAddr(XAie_MemInst *MemInst);
AieRC XAie_MemAttach(XAie_DevInst *DevInst, XAie_MemInst *MemInst, u64 DAddr,
		u64 VAddr, u64 Size, XAie_MemCacheProp Cache, u64 MemHandle);
AieRC XAie_MemDetach(XAie_MemInst *MemInst);
AieRC XAie_TurnEccOff(XAie_DevInst *DevInst);
AieRC XAie_TurnEccOn(XAie_DevInst *DevInst);
AieRC XAie_StartTransaction(XAie_DevInst *DevInst, u32 Flags);
AieRC XAie_SubmitTransaction(XAie_DevInst *DevInst, XAie_TxnInst *TxnInst);
XAie_TxnInst* XAie_ExportTransactionInstance(XAie_DevInst *DevInst);
AieRC XAie_FreeTransactionInstance(XAie_TxnInst *TxnInst);
AieRC XAie_IsDeviceCheckerboard(XAie_DevInst *DevInst, u8 *IsCheckerBoard);
/*****************************************************************************/
/*
*
* This API returns a structure of type XAie_Loc which captures the lock id and
* lock value.
*
* @param	Id: Lock id
* @param	Value: Lock value.
*
* @return	Lock: Lock strcuture initialized with Id and Value.
*
* @note		None.
*
******************************************************************************/
static inline XAie_Lock XAie_LockInit(u8 Id, s8 Value)
{
	XAie_Lock Lock = {Id, Value};
	return Lock;
}

/*****************************************************************************/
/*
*
* This API returns a structure of type XAie_Packet which captures the packet id
* and packet type. XAie_Packet can be used to configure the packet properties
* of aie dmas and stream switches. Packet ID determins the route between ports,
* and packet type is used to differentiate packets from the same source.
*
* @param	PktId: Packet id(5 bits)
* @param	PktType: Packet type(3 bits)
*
* @return	Pkt: Packet strcuture initialized with Id and Type.
*
* @note		None.
*
******************************************************************************/
static inline XAie_Packet XAie_PacketInit(u8 PktId, u8 PktType)
{
	XAie_Packet Pkt = {PktId, PktType};
	return Pkt;
}

/*****************************************************************************/
/*
*
* This API returns a structure of type XAie_LocType given a col and row index of
* AIE. All APIs use this structure to identify the coordinates of AIE tiles.
*
* @param	col: column index
* @param	row: row index
*
* @return	Loc: strcuture containing row and col index.
*
* @note		None.
*
******************************************************************************/
static inline XAie_LocType XAie_TileLoc(u8 col, u8 row)
{
	XAie_LocType Loc = { row, col };
	return Loc;
}

/*****************************************************************************/
/**
*
* This API setups the AI engine partition property in AI engine config
*
* @param	Config: XAie_Config structure.
* @param	Nid: AI enigne partition node ID
* @param	Uid: AI enigne partition image UID
* @param	Handle: AI engine partition handle, in some OS such as Linux
*			the AI engine partition is presented as file descriptor.
*			In case of Xilinx runtime stack, the Xilinx runtime
*			module has requested the AI engine partition which will
*			have the handle can be passed to the userspace
*			application.
* @param	CntrFlag: AI engine partition control flag. E.g., it can be used
*			to indicate if the partition needs to cleanup when
*			application terminates.
*
* @return	None.
*
* @note		This function is to set the partition system design property to
*		the AI engine config. It needs to be called before intialize
*		AI engine partition.
*
*******************************************************************************/
static inline void XAie_SetupConfigPartProp(XAie_Config *ConfigPtr, u32 Nid,
		u32 Uid, u64 Handle, u32 CntrFlag)
{
	ConfigPtr->PartProp.Nid = Nid;
	ConfigPtr->PartProp.Uid = Uid;
	ConfigPtr->PartProp.Handle = Handle;
	ConfigPtr->PartProp.CntrFlag = CntrFlag;
}

/*****************************************************************************/
/**
*
* Macro to setup the configurate pointer data structure with hardware specific
* details.
*
* @param	Config: XAie_Config structure.
* @param	_AieGen: Aie device generation.
* @param	_BaseAddr: Base Address of the device.
* @param	_ColShift: Bit shift value for column.
* @param	_RowShift: Bit shift value for row.
* @param	_NumCols: Number of cols in the hardware.
* @param	_NumRows: Number of rows in the hardware.
* @param	_ShimRowNum: Row number of the shimrow.
* @param	_MemTileRowStart: Starting row number of the mem tile.
* @param	_MemTileNumRows: Number of mem tile rows.
* @param	_AieTileRowStart: Starting row number of the mem tile.
* @param	_AieTileNumRows: Number of mem tile rows.
*
* @return	None.
*
* @note		The macro declares it XAie_Config as a stack variable.
*
*******************************************************************************/
#define XAie_SetupConfig(Config, _AieGen, _BaseAddr, _ColShift, _RowShift,\
		_NumCols, _NumRows, _ShimRowNum, _MemTileRowStart, _MemTileNumRows,\
		_AieTileRowStart, _AieTileNumRows) \
		XAie_Config Config = {\
			.AieGen = _AieGen,\
			.BaseAddr = _BaseAddr,\
			.ColShift = _ColShift,\
			.RowShift = _RowShift,\
			.NumRows = _NumRows,\
			.NumCols = _NumCols,\
			.ShimRowNum = _ShimRowNum,\
			.MemTileRowStart = _MemTileRowStart,\
			.MemTileNumRows = _MemTileNumRows,\
			.AieTileRowStart = _AieTileRowStart,\
			.AieTileNumRows = _AieTileNumRows,\
			.PartProp = {0}, \
		}\

/*****************************************************************************/
/**
*
* Macro to declare device instance.
*
* @param	Inst: Name of the Device Instance variable.
* @param	ConfigPtr: Pointer to the XAie_Config structure containing the
*			   hardware details..
*
* @return	None.
*
* @note		The instance of a device must be always declared using this
*		macro. In future, the same macro will be expanded to allocate
*		more memory from the user application for resource management.
*
*******************************************************************************/
#define XAie_InstDeclare(Inst, ConfigPtr) XAie_DevInst Inst = { 0 }

#endif	/* end of protection macro */

/** @} */
