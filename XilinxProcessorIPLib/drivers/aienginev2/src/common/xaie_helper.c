/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_helper.c
* @{
*
* This file contains inline helper functions for AIE drivers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   09/24/2019  Initial creation
* 1.1   Tejus   09/24/2019  Fix range check logic for shim row
* 1.2   Tejus   01/04/2020  Cleanup error messages
* 1.3   Tejus   04/13/2020  Add api to get tile type from Loc
* 1.4   Tejus   04/13/2020  Remove helper functions for range apis
* 1.5   Dishita 04/29/2020  Add api to check module & tile type combination
* 1.6   Nishad  07/06/2020  Add _XAie_GetMstrIdx() helper API and move
*			    _XAie_GetSlaveIdx() API.
* 1.7   Nishad  07/24/2020  Add _XAie_GetFatalGroupErrors() helper function.
* 1.8   Dishita 08/10/2020  Add api to get bit position from tile location
* 1.9   Nishad  08/26/2020  Fix tiletype check in _XAie_CheckModule()
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "xaie_helper.h"

/************************** Constant Definitions *****************************/
#define XAIE_DEFAULT_NUM_CMDS 1024U

#define XAIE_TXN_INSTANCE_EXPORTED	0b10
#define XAIE_TXN_INST_EXPORTED_MASK XAIE_TXN_INSTANCE_EXPORTED
#define XAIE_TXN_AUTO_FLUSH_MASK XAIE_TRANSACTION_ENABLE_AUTO_FLUSH

/************************** Variable Definitions *****************************/
/***************************** Macro Definitions *****************************/
/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This is the function used to get the tile type for a given device instance
* and tile location.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the AIE tile.
* @return	TileType (AIETILE/RESERVED/SHIMPL/SHIMNOC on success and MAX on
*		error)
*
* @note		Internal API only.
*
******************************************************************************/
u8 _XAie_GetTileTypefromLoc(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	u8 ColType;

	if(Loc.Col >= DevInst->NumCols) {
		XAIE_ERROR("Invalid column: %d\n", Loc.Col);
		return XAIEGBL_TILE_TYPE_MAX;
	}

	if(Loc.Row == 0U) {
		ColType = Loc.Col % 4U;
		if((ColType == 0U) || (ColType == 1U)) {
			return XAIEGBL_TILE_TYPE_SHIMPL;
		}

		return XAIEGBL_TILE_TYPE_SHIMNOC;

	} else if(Loc.Row >= DevInst->ReservedRowStart &&
			(Loc.Row < (DevInst->ReservedRowStart +
				     DevInst->ReservedNumRows))) {
		return XAIEGBL_TILE_TYPE_RESERVED;
	} else if (Loc.Row >= DevInst->AieTileRowStart &&
			(Loc.Row < (DevInst->AieTileRowStart +
				     DevInst->AieTileNumRows))) {
		return XAIEGBL_TILE_TYPE_AIETILE;
	}

	XAIE_ERROR("Cannot find Tile Type\n");

	return XAIEGBL_TILE_TYPE_MAX;
}

/*****************************************************************************/
/**
* This function is used to check for module and tiletype combination.
*
* @param        DevInst: Device Instance
* @param        Loc: Location of the AIE tile.
* @param	Module:	XAIE_MEM_MOD - memory module
* 			XAIE_CORE_MOD - core module
* 			XAIE_PL_MOD - pl module
* @return       XAIE_OK for correct combination of Module and tile type
* 		XAIE_INVALID_ARGS for incorrect combination of module and tile
* 		type
*
* @note         Internal API only.
*
*******************************************************************************/
AieRC _XAie_CheckModule(XAie_DevInst *DevInst,
		XAie_LocType Loc, XAie_ModuleType Module)
{
	u8 TileType;

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_AIETILE && Module > XAIE_CORE_MOD) {
		XAIE_ERROR("Invalid Module\n");
		return XAIE_INVALID_ARGS;
	}

	if((TileType == XAIEGBL_TILE_TYPE_SHIMPL ||
	    TileType == XAIEGBL_TILE_TYPE_SHIMNOC) && Module != XAIE_PL_MOD) {
		XAIE_ERROR("Invalid Module\n");
		return XAIE_INVALID_ARGS;
	}

	if(TileType == XAIEGBL_TILE_TYPE_RESERVED &&
		Module != XAIE_MEM_MOD) {
		XAIE_ERROR("Invalid Module\n");
		return XAIE_INVALID_ARGS;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This function is used to get no. of rows for the given tiletype.
*
* @param        DevInst: Device Instance
* @param        TileType: Type of tile
*
* @return       BitmapNumRows: Number of rows for given tiletype
*
* @note         Internal API only.
*
*******************************************************************************/
u32 _XAie_GetNumRows(XAie_DevInst *DevInst, u8 TileType)
{
	u32 NumRows;

	switch(TileType) {
	case XAIEGBL_TILE_TYPE_SHIMNOC:
	case XAIEGBL_TILE_TYPE_SHIMPL:
	{       NumRows = 1U;
		break;
	}
	case XAIEGBL_TILE_TYPE_AIETILE:
	{       NumRows = DevInst->AieTileNumRows;
		break;
	}
	case XAIEGBL_TILE_TYPE_RESERVED:
	{
		NumRows = 0U;
		break;
	}
	default:
	{
		XAIE_ERROR("Invalid Tiletype\n");
		return 0;
	}
	}

	return NumRows;
}

/*****************************************************************************/
/**
* This function is used to get start row for the given tiletype.
*
* @param        DevInst: Device Instance
* @param        TileType: Type of tile
*
* @return       StartRow: Start row for given tiletype
*
* @note         Internal API only.
*
*******************************************************************************/
u32 _XAie_GetStartRow(XAie_DevInst *DevInst, u8 TileType)
{
	u32 StartRow;

	switch(TileType) {
	case XAIEGBL_TILE_TYPE_SHIMNOC:
	case XAIEGBL_TILE_TYPE_SHIMPL:
	{       StartRow = DevInst->ShimRow;
		break;
	}
	case XAIEGBL_TILE_TYPE_AIETILE:
	{	StartRow = DevInst->AieTileRowStart;
		break;
	}
	default:
	{
		XAIE_ERROR("Invalid Tiletype\n");
		return 0;
	}
	}

	return StartRow;
}

/*****************************************************************************/
/**
*
* To configure stream switch master registers, slave index has to be calculated
* from the internal data structure. The routine calculates the slave index for
* any tile type.
*
* @param	StrmMod: Stream Module pointer
* @param	Slave: Stream switch port type
* @param	PortNum: Slave port number
* @param	SlaveIdx: Place holder for the routine to store the slave idx
*
* @return	XAIE_OK on success and XAIE_INVALID_RANGE on failure
*
* @note		Internal API only.
*
******************************************************************************/
AieRC _XAie_GetSlaveIdx(const XAie_StrmMod *StrmMod, StrmSwPortType Slave,
		u8 PortNum, u8 *SlaveIdx)
{
	u32 BaseAddr;
	u32 RegAddr;
	const XAie_StrmPort *PortPtr;

	/* Get Base Addr of the slave tile from Stream Switch Module */
	BaseAddr = StrmMod->SlvConfigBaseAddr;

	PortPtr = &StrmMod->SlvConfig[Slave];

	/* Return error if the Slave Port Type is not valid */
	if((PortPtr->NumPorts == 0) || (PortNum >= PortPtr->NumPorts)) {
		XAIE_ERROR("Invalid Slave Port\n");
		return XAIE_ERR_STREAM_PORT;
	}

	RegAddr = PortPtr->PortBaseAddr + StrmMod->PortOffset * PortNum;
	*SlaveIdx = (RegAddr - BaseAddr) / 4;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* The routine calculates the master index for any tile type.
*
* @param	StrmMod: Stream Module pointer
* @param	Master: Stream switch port type
* @param	PortNum: Master port number
* @param	MasterIdx: Place holder for the routine to store the master idx
*
* @return	XAIE_OK on success and XAIE_INVALID_RANGE on failure
*
* @note		Internal API only.
*
******************************************************************************/
AieRC _XAie_GetMstrIdx(const XAie_StrmMod *StrmMod, StrmSwPortType Master,
		u8 PortNum, u8 *MasterIdx)
{
	u32 BaseAddr;
	u32 RegAddr;
	const XAie_StrmPort *PortPtr;

	/* Get Base Addr of the master tile from Stream Switch Module */
	BaseAddr = StrmMod->MstrConfigBaseAddr;

	PortPtr = &StrmMod->MstrConfig[Master];

	/* Return error if the Master Port Type is not valid */
	if((PortPtr->NumPorts == 0) || (PortNum >= PortPtr->NumPorts)) {
		XAIE_ERROR("Invalid Master Port\n");
		return XAIE_ERR_STREAM_PORT;
	}

	RegAddr = PortPtr->PortBaseAddr + StrmMod->PortOffset * PortNum;
	*MasterIdx = (RegAddr - BaseAddr) / 4;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API returns the default value of group errors marked as fatal.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the AIE tile.
* @param	Module: Module of tile.
*			for AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			for Shim tile - XAIE_PL_MOD,
*
* @return	Default value of group fatal errors.
*
* @note		Internal API only.
*
******************************************************************************/
u32 _XAie_GetFatalGroupErrors(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module)
{
	u8 TileType;
	const XAie_EvntMod *EvntMod;

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(Module == XAIE_PL_MOD)
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[0U];
	else
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[Module];

	return EvntMod->DefaultGroupErrorMask;
}

void XAie_Log(FILE *Fd, const char *prefix, const char *Format, ...)
{
	va_list ArgPtr;
	va_start(ArgPtr, Format);
	fprintf(Fd, "%s", prefix);
	vfprintf(Fd, Format, ArgPtr);
	va_end(ArgPtr);
}

/*****************************************************************************/
/**
* This is an internal API to get bit position corresponding to tile location in
* bitmap. This bitmap does not represent Shim tile so this API
* only accepts AIE tile.
*
* @param        DevInst: Device Instance
* @param        Loc: Location of AIE tile
* @return       Bit position in the TilesInUse bitmap
*
* @note         None
*
******************************************************************************/
u32 _XAie_GetTileBitPosFromLoc(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	return Loc.Col * (DevInst->NumRows - 1U) + Loc.Row - 1U;
}

/*****************************************************************************/
/**
* This API sets given number of bits from given start bit in the given bitmap.
*
* @param        Bitmap: bitmap to be set
* @param        StartSetBit: Bit position in the bitmap
* @param        NumSetBit: Number of bits to be set.
*
* @return       none
*
* @note         This API is internal, hence all the argument checks are taken
*               care of in the caller API.
*
******************************************************************************/
void _XAie_SetBitInBitmap(u32 *Bitmap, u32 StartSetBit,
		u32 NumSetBit)
{
	for(u32 i = StartSetBit; i < StartSetBit + NumSetBit; i++) {
		Bitmap[i / (sizeof(Bitmap[0]) * 8U)] |=
			1U << (i % (sizeof(Bitmap[0]) * 8U));
	}
}

/*****************************************************************************/
/**
** This API clears number of bits from given start bit in the given bitmap.
*
* @param        Bitmap: bitmap to be set
* @param        StartBit: Bit position in the bitmap
* @param        NumBit: Number of bits to be set.
*
* @return       None
*
* @note         This API is internal, hence all the argument checks are taken
*               care of in the caller API.
*
******************************************************************************/
void _XAie_ClrBitInBitmap(u32 *Bitmap, u32 StartBit, u32 NumBit)
{
	for(u32 i = StartBit; i < StartBit + NumBit; i++) {
		Bitmap[i / (sizeof(Bitmap[0]) * 8U)] &=
			~(1U << (i % (sizeof(Bitmap[0]) * 8U)));
	}
}

/*****************************************************************************/
/**
* This API inserts a transaction node to the linked list.
*
* @param        DevInst: Device Instance
* @param        TxnNode: Pointer to the transaction node to be inserted
*
* @return       None
*
* @note         Internal only.
*
******************************************************************************/
static void _XAie_AppendTxnInstToList(XAie_DevInst *DevInst, XAie_TxnInst *Inst)
{
	XAie_List *Node = &DevInst->TxnList;

	while(Node->Next != NULL) {
		Node = Node->Next;
	}

	Node->Next = &Inst->Node;
	Inst->Node.Next = NULL;
}

/*****************************************************************************/
/**
* This API returns the transaction list from the linked list based on the thread
* id.
*
* @param        DevInst: Device instance pointer
* @param	Tid: Thread id.
*
* @return       Pointer to transaction instance on success and NULL on failure
*
* @note         Internal only.
*
******************************************************************************/
static XAie_TxnInst *_XAie_GetTxnInst(XAie_DevInst *DevInst, u64 Tid)
{
	XAie_List *NodePtr = DevInst->TxnList.Next;
	XAie_TxnInst *TxnInst;

	while(NodePtr != NULL) {
		TxnInst = XAIE_CONTAINER_OF(NodePtr, XAie_TxnInst, Node);
		if(TxnInst->Tid == Tid) {
			return TxnInst;
		}

		NodePtr = NodePtr->Next;
	}

	return NULL;
}

/*****************************************************************************/
/**
* This API removes a node from the linked list if the thread id is found.
*
* @param        DevInst: Device instance pointer
* @param	Tid: Thread id.
*
* @return       XAIE_OK on success and error code on failure.
*
* @note         Internal only.
*
******************************************************************************/
static AieRC _XAie_RemoveTxnInstFromList(XAie_DevInst *DevInst, u64 Tid)
{
	XAie_List *NodePtr = DevInst->TxnList.Next;
	XAie_List *Prev = &DevInst->TxnList;
	XAie_TxnInst *Inst;

	while(NodePtr != NULL) {
		Inst = (XAie_TxnInst *)XAIE_CONTAINER_OF(NodePtr, XAie_TxnInst,
				Node);
		if(Inst->Tid == Tid) {
			break;
		}

		Prev = NodePtr;
		NodePtr = NodePtr->Next;
	}

	if(NodePtr == NULL) {
		XAIE_ERROR("Cannot find node to delete from list\n");
		return XAIE_ERR;
	} else {
		Prev->Next = NodePtr->Next;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API rellaocates the command buffer associated with the given transaction
* instance. The command buffer is extended to accomodate an additional
* XAIE_DEFAULT_NUM_CMDS number of commands.
*
* @param        TxnInst: Pointer to the transaction instance
*
* @return       XAIE_OK on success and XAIE_ERR on failure
*
* @note         Internal only.
*
******************************************************************************/
static AieRC _XAie_ReallocCmdBuf(XAie_TxnInst *TxnInst)
{
	TxnInst->CmdBuf = (XAie_TxnCmd *)realloc((void *)TxnInst->CmdBuf,
			sizeof(XAie_TxnCmd) *
			(TxnInst->MaxCmds + XAIE_DEFAULT_NUM_CMDS));
	if(TxnInst->CmdBuf == NULL) {
		XAIE_ERROR("Failed reallocate memory for transaction buffer "
				"with id: %d\n", TxnInst->Tid);
		return XAIE_ERR;
	}

	TxnInst->MaxCmds += XAIE_DEFAULT_NUM_CMDS;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This api allocates the memory to store the IO transaction commands
* when the driver is configured to execute in transaction mode.
*
* @param	DevInst - Device instance pointer.
* @param	Flags - Flags passed by the user.
*
* @return	Pointer to transaction instance on success and NULL on error.
*
* @note		Internal Only.
*
******************************************************************************/
AieRC _XAie_Txn_Start(XAie_DevInst *DevInst, u32 Flags)
{
	XAie_TxnInst *Inst;
	const XAie_Backend *Backend = DevInst->Backend;

	Inst = (XAie_TxnInst*)malloc(sizeof(*Inst));
	if(Inst == NULL) {
		XAIE_ERROR("Failed to allocate memory for txn instance\n");
		return XAIE_ERR;
	}

	Inst->CmdBuf = (XAie_TxnCmd*)calloc(XAIE_DEFAULT_NUM_CMDS,
			sizeof(*Inst->CmdBuf));
	if(Inst->CmdBuf == NULL) {
		XAIE_ERROR("Failed to allocate memory for command buffer\n");
		free(Inst);
		return XAIE_ERR;
	}

	Inst->NumCmds = 0U;
	Inst->MaxCmds = XAIE_DEFAULT_NUM_CMDS;
	Inst->Tid = Backend->Ops.GetTid();

	XAIE_DBG("Transaction buffer allocated with id: %ld\n", Inst->Tid);
	Inst->Flags = Flags;
	if(Flags & XAIE_TXN_AUTO_FLUSH_MASK) {
		XAIE_DBG("Auto flush is enabled for transaction buffer with "
				"id: %ld\n", Inst->Tid);
	} else {
		XAIE_DBG("Auto flush is disabled for transaction buffer with "
				"id: %ld\n", Inst->Tid);
	}

	_XAie_AppendTxnInstToList(DevInst, Inst);

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API decodes the command type and executes the IO operation.
*
* @param        DevInst: Device instance pointer
* @param        Cmd: Pointer to the transaction command structure
* @param	Flags: Transaction instance flags
*
* @return       XAIE_OK on success and XAIE_ERR on failure.
*
* @note         Internal only.
*
******************************************************************************/
static AieRC _XAie_ExecuteCmd(XAie_DevInst *DevInst, XAie_TxnCmd *Cmd,
		u32 Flags)
{
	AieRC RC;
	const XAie_Backend *Backend = DevInst->Backend;

	switch(Cmd->Opcode)
	{
		case XAIE_IO_WRITE:
			if(!Cmd->Mask) {
				RC = Backend->Ops.Write32((void*)DevInst->IOInst,
						Cmd->RegOff, Cmd->Value);
			} else {

				RC = Backend->Ops.MaskWrite32((void*)DevInst->IOInst,
							Cmd->RegOff, Cmd->Mask,
							Cmd->Value);
			}
			if(RC != XAIE_OK) {
				XAIE_ERROR("Wr failed. Addr: 0x%lx, Mask: 0x%x,"
						"Value: 0x%x\n", Cmd->RegOff,
						Cmd->Mask, Cmd->Value);
				return RC;
			}
			break;
		case XAIE_IO_BLOCKWRITE:
			RC = Backend->Ops.BlockWrite32((void *)DevInst->IOInst,
					Cmd->RegOff,
					(u32 *)(uintptr_t)Cmd->DataPtr,
					Cmd->Size);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Block Wr failed. Addr: 0x%lx\n",
						Cmd->RegOff);
				return RC;
			}

			if(!(Flags & XAIE_TXN_INST_EXPORTED_MASK)) {
				free((void *)Cmd->DataPtr);
			}
			break;
		case XAIE_IO_BLOCKSET:
			RC = Backend->Ops.BlockSet32((void *)DevInst->IOInst,
					Cmd->RegOff, Cmd->Value,
					Cmd->Size);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Block Wr failed. Addr: 0x%lx\n",
						Cmd->RegOff);
				return RC;
			}
			break;
		default:
			XAIE_ERROR("Invalid transaction opcode\n");
			return XAIE_ERR;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API executes all the commands in the command buffer and resets the number
* of commands.
*
* @param        DevInst: Device instance pointer
* @param        TxnInst: Pointer to the transaction instance
*
* @return       XAIE_OK on success and XAIE_ERR on failure
*
* @note         Internal only. This API does not allocate, reallocate or free
*		any buffer.
*
******************************************************************************/
static AieRC _XAie_Txn_FlushCmdBuf(XAie_DevInst *DevInst, XAie_TxnInst *TxnInst)
{
	AieRC RC;
	const XAie_Backend *Backend = DevInst->Backend;

	XAIE_DBG("Flushing %d commands from transaction buffer\n",
			TxnInst->NumCmds);

	if(Backend->Ops.SubmitTxn != NULL) {
		return Backend->Ops.SubmitTxn(DevInst->IOInst, TxnInst);
	}

	for(u32 i = 0U; i < TxnInst->NumCmds; i++) {
		RC = _XAie_ExecuteCmd(DevInst, &TxnInst->CmdBuf[i],
				TxnInst->Flags);
		if(RC != XAIE_OK) return RC;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API executes all the commands in the command buffer and frees the
* transaction instance unless it is exported to the user.
*
* @param        DevInst: Device instance pointer
* @param        TxnInst: Pointer to the transaction instance
*
* @return       XAIE_OK on success and XAIE_ERR on failure
*
* @note         Internal only.
*
******************************************************************************/
AieRC _XAie_Txn_Submit(XAie_DevInst *DevInst, XAie_TxnInst *TxnInst)
{
	AieRC RC;
	u64 Tid;
	XAie_TxnInst *Inst;
	const XAie_Backend *Backend = DevInst->Backend;

	if(TxnInst == NULL) {
		Tid = Backend->Ops.GetTid();
		Inst = _XAie_GetTxnInst(DevInst, Tid);
		if(Inst == NULL) {
			XAIE_ERROR("Failed to get the correct transaction "
					"instance\n");
			return XAIE_ERR;
		}
	} else {
		if(TxnInst->Flags & XAIE_TXN_INST_EXPORTED_MASK) {
			Inst = TxnInst;
		} else {
			XAIE_ERROR("Transaction instance was not exported.\n");
			return XAIE_ERR;
		}
	}

	RC =  _XAie_Txn_FlushCmdBuf(DevInst, Inst);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Flushing the buffer failed\n");
		return RC;
	}

	/* Do not free resources if transaction is exported */
	if(Inst->Flags & XAIE_TXN_INST_EXPORTED_MASK) {
		return XAIE_OK;
	}

	RC = _XAie_RemoveTxnInstFromList(DevInst, Tid);
	if(RC != XAIE_OK) {
		return RC;
	}

	free(Inst->CmdBuf);
	free(Inst);
	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This api copies an existing transaction instance and returns a copy of the
* instance with all the commands for users to save the commands and use them
* at a later point.
*
* @param	DevInst - Device instance pointer.
*
* @return	Pointer to copy of transaction instance on success and NULL
*		on error.
*
* @note		Internal only.
*
******************************************************************************/
XAie_TxnInst* _XAie_TxnExport(XAie_DevInst *DevInst)
{
	XAie_TxnInst *Inst, *TmpInst;
	const XAie_Backend *Backend = DevInst->Backend;

	TmpInst = _XAie_GetTxnInst(DevInst, Backend->Ops.GetTid());
	if(TmpInst == NULL) {
		XAIE_ERROR("Failed to get the correct transaction instance "
				"from internal list\n");
		return NULL;
	}

	Inst = (XAie_TxnInst *)malloc(sizeof(*Inst));
	if(Inst == NULL) {
		XAIE_ERROR("Failed to allocate memory for txn instance\n");
		return NULL;
	}

	Inst->CmdBuf = (XAie_TxnCmd *)calloc(TmpInst->NumCmds,
			sizeof(*Inst->CmdBuf));
	if(Inst->CmdBuf == NULL) {
		XAIE_ERROR("Failed to allocate memory for command buffer\n");
		free(Inst);
		return NULL;
	}

	Inst->CmdBuf = (XAie_TxnCmd *)memcpy((void *)Inst->CmdBuf,
			(void *)TmpInst->CmdBuf,
			TmpInst->NumCmds * sizeof(*Inst->CmdBuf));

	for(u32 i = 0U; i < TmpInst->NumCmds; i++) {
		XAie_TxnCmd *TmpCmd = &TmpInst->CmdBuf[i];
		XAie_TxnCmd *Cmd = &Inst->CmdBuf[i];
		if(TmpCmd->Opcode == XAIE_IO_BLOCKWRITE) {
			Cmd->DataPtr = (u64)(uintptr_t)malloc(
					sizeof(u32) * TmpCmd->Size);
			if((void *)(uintptr_t)Cmd->DataPtr == NULL) {
				XAIE_ERROR("Failed to allocate memory to copy "
						"command %d\n", i);
				free(Inst->CmdBuf);
				free(Inst);
				return NULL;
			}

			Cmd->DataPtr = (u64)(uintptr_t)memcpy(
					(void *)Cmd->DataPtr,
					(void *)TmpCmd->DataPtr,
					sizeof(u32) * TmpCmd->Size);
		}
	}

	Inst->Tid = TmpInst->Tid;
	Inst->Flags = TmpInst->Flags;
	Inst->Flags |= XAIE_TXN_INSTANCE_EXPORTED;
	Inst->NumCmds = TmpInst->NumCmds;
	Inst->MaxCmds = TmpInst->MaxCmds;
	Inst->Node.Next = NULL;

	return Inst;
}

/*****************************************************************************/
/**
*
* This api releases the memory resources used by exported transaction instance.
*
* @param	TxnInst - Existing Transaction instance
*
* @return	XAIE_OK on success or error code on failure.
*
* @note		Internal only.
*
******************************************************************************/
AieRC _XAie_TxnFree(XAie_TxnInst *Inst)
{
	if(!(Inst->Flags & XAIE_TXN_INST_EXPORTED_MASK)) {
		XAIE_ERROR("The transaction instance was not exported, it's "
				"resources cannot be released\n");
		return XAIE_ERR;
	}

	for(u32 i = 0; i < Inst->NumCmds; i++) {
		XAie_TxnCmd *Cmd = &Inst->CmdBuf[i];
		if((Cmd->Opcode == XAIE_IO_BLOCKWRITE) &&
				((void *)(uintptr_t)Cmd->DataPtr != NULL)) {
			free((void *)(uintptr_t)Cmd->DataPtr);
		}
	}

	free(Inst->CmdBuf);
	free(Inst);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This api cleansup all the resources when the device instance is closed.
*
* @param	DevInst - Device instance pointer.
*
* @return	None.
*
* @note		Internal only.
*
******************************************************************************/
void _XAie_TxnResourceCleanup(XAie_DevInst *DevInst)
{
	XAie_List *NodePtr = DevInst->TxnList.Next;
	XAie_TxnInst *TxnInst;

	while(NodePtr != NULL) {
		TxnInst = XAIE_CONTAINER_OF(NodePtr, XAie_TxnInst, Node);

		if(TxnInst == NULL) continue;

		for(u32 i = 0; i < TxnInst->NumCmds; i++) {
			XAie_TxnCmd *Cmd = &TxnInst->CmdBuf[i];
			if((Cmd->Opcode == XAIE_IO_BLOCKWRITE) &&
					((void *)(uintptr_t)Cmd->DataPtr != NULL)) {
				free((void *)(uintptr_t)Cmd->DataPtr);
			}
		}

		NodePtr = NodePtr->Next;
		free(TxnInst->CmdBuf);
		free(TxnInst);
	}
}

AieRC XAie_Write32(XAie_DevInst *DevInst, u64 RegOff, u32 Value)
{
	u64 Tid;
	AieRC RC;
	XAie_TxnInst *TxnInst;
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL) {
		Tid = Backend->Ops.GetTid();
		TxnInst = _XAie_GetTxnInst(DevInst, Tid);
		if(TxnInst == NULL) {
			XAIE_DBG("Could not find transaction instance "
					"associated with thread. Mask writing "
					"to register\n");
			goto write;
		}

		if(TxnInst->NumCmds + 1U == TxnInst->MaxCmds) {
			RC = _XAie_ReallocCmdBuf(TxnInst);
			if(RC != XAIE_OK) return RC;
		}

		TxnInst->CmdBuf[TxnInst->NumCmds].Opcode = XAIE_IO_WRITE;
		TxnInst->CmdBuf[TxnInst->NumCmds].RegOff = RegOff;
		TxnInst->CmdBuf[TxnInst->NumCmds].Value = Value;
		TxnInst->CmdBuf[TxnInst->NumCmds].Mask = 0U;
		TxnInst->NumCmds++;

		return XAIE_OK;
	}

write:
	return Backend->Ops.Write32((void*)(DevInst->IOInst), RegOff, Value);
}

AieRC XAie_Read32(XAie_DevInst *DevInst, u64 RegOff, u32 *Data)
{
	u64 Tid;
	AieRC RC;
	XAie_TxnInst *TxnInst;
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL) {
		Tid = Backend->Ops.GetTid();
		TxnInst = _XAie_GetTxnInst(DevInst, Tid);
		if(TxnInst == NULL) {
			XAIE_DBG("Could not find transaction instance "
					"associated with thread. Reading "
					"from register\n");
			goto read;
		}

		if((TxnInst->Flags & XAIE_TXN_AUTO_FLUSH_MASK) &&
				(TxnInst->NumCmds > 0)) {
			/* Flush command buffer */
			XAIE_DBG("Auto flushing contents of the transaction "
					"buffer.\n");
			RC = _XAie_Txn_FlushCmdBuf(DevInst, TxnInst);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to flush cmd buffer\n");
				return RC;
			}

			TxnInst->NumCmds = 0;
			goto read;
		} else if(TxnInst->NumCmds == 0) {
			goto read;
		} else {
			XAIE_ERROR("Read operation is not supported "
					"when auto flush is disabled\n");
			return XAIE_ERR;
		}
	}

read:
	return Backend->Ops.Read32((void*)(DevInst->IOInst), RegOff, Data);
}

AieRC XAie_MaskWrite32(XAie_DevInst *DevInst, u64 RegOff, u32 Mask, u32 Value)
{
	AieRC RC;
	u64 Tid;
	XAie_TxnInst *TxnInst;
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL) {
		Tid = Backend->Ops.GetTid();
		TxnInst = _XAie_GetTxnInst(DevInst, Tid);
		if(TxnInst == NULL) {
			XAIE_DBG("Could not find transaction instance "
					"associated with thread. Writing "
					"to register\n");
			goto maskwrite;
		}

		if(TxnInst->NumCmds + 1U == TxnInst->MaxCmds) {
			RC = _XAie_ReallocCmdBuf(TxnInst);
			if(RC != XAIE_OK) return RC;
		}

		TxnInst->CmdBuf[TxnInst->NumCmds].Opcode = XAIE_IO_WRITE;
		TxnInst->CmdBuf[TxnInst->NumCmds].RegOff = RegOff;
		TxnInst->CmdBuf[TxnInst->NumCmds].Mask = Mask;
		TxnInst->CmdBuf[TxnInst->NumCmds].Value = Value;
		TxnInst->NumCmds++;

		return XAIE_OK;
	}

maskwrite:
	return Backend->Ops.MaskWrite32((void *)(DevInst->IOInst), RegOff, Mask,
			Value);
}

AieRC XAie_MaskPoll(XAie_DevInst *DevInst, u64 RegOff, u32 Mask, u32 Value,
		u32 TimeOutUs)
{
	AieRC RC;
	u64 Tid;
	XAie_TxnInst *TxnInst;
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL) {
		Tid = Backend->Ops.GetTid();
		TxnInst = _XAie_GetTxnInst(DevInst, Tid);
		if(TxnInst == NULL) {
			XAIE_DBG("Could not find transaction instance "
					"associated with thread. Polling "
					"from register\n");
			goto maskpoll;
		}

		if((TxnInst->Flags & XAIE_TXN_AUTO_FLUSH_MASK) &&
				(TxnInst->NumCmds > 0)) {
			/* Flush command buffer */
			XAIE_DBG("Auto flushing contents of the transaction "
					"buffer.\n");
			RC = _XAie_Txn_FlushCmdBuf(DevInst, TxnInst);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to flush cmd buffer\n");
				return RC;
			}

			TxnInst->NumCmds = 0;
			goto maskpoll;
		} else if(TxnInst->NumCmds == 0) {
			goto maskpoll;
		} else {
			XAIE_ERROR("MaskPoll operation is not supported "
					"when auto flush is disabled\n");
			return XAIE_ERR;
		}
	}

maskpoll:
	return Backend->Ops.MaskPoll((void*)(DevInst->IOInst), RegOff, Mask,
			Value, TimeOutUs);
}

AieRC XAie_BlockWrite32(XAie_DevInst *DevInst, u64 RegOff, u32 *Data, u32 Size)
{
	AieRC RC;
	u64 Tid;
	u32 *Buf;
	XAie_TxnInst *TxnInst;
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL) {
		Tid = Backend->Ops.GetTid();
		TxnInst = _XAie_GetTxnInst(DevInst, Tid);
		if(TxnInst == NULL) {
			XAIE_DBG("Could not find transaction instance "
					"associated with thread. Block write "
					"to register\n");
			goto blockwrite;
		}

		if(TxnInst->Flags & XAIE_TXN_AUTO_FLUSH_MASK) {
			/* Flush command buffer */
			XAIE_DBG("Auto flushing contents of the transaction "
					"buffer.\n");
			if(TxnInst->NumCmds > 0) {
				RC = _XAie_Txn_FlushCmdBuf(DevInst, TxnInst);
				if(RC != XAIE_OK) {
					XAIE_ERROR("Failed to flush cmd buffer\n");
					return RC;
				}
			}

			TxnInst->NumCmds = 0;
			goto blockwrite;
		}

		if(TxnInst->NumCmds + 1U == TxnInst->MaxCmds) {
			RC = _XAie_ReallocCmdBuf(TxnInst);
			if(RC != XAIE_OK) return RC;
		}

		Buf = (u32 *)malloc(sizeof(u32) * Size);
		if(Buf == NULL) {
			XAIE_ERROR("Memory allocation for block write failed\n");
			return XAIE_ERR;
		}

		Buf = memcpy((void *)Buf, (void *)Data, sizeof(u32) * Size);
		TxnInst->CmdBuf[TxnInst->NumCmds].Opcode = XAIE_IO_BLOCKWRITE;
		TxnInst->CmdBuf[TxnInst->NumCmds].RegOff = RegOff;
		TxnInst->CmdBuf[TxnInst->NumCmds].DataPtr = (u64)(uintptr_t)Buf;
		TxnInst->CmdBuf[TxnInst->NumCmds].Size = Size;
		TxnInst->CmdBuf[TxnInst->NumCmds].Mask = 0U;
		TxnInst->NumCmds++;

		return XAIE_OK;
	}

blockwrite:
	return Backend->Ops.BlockWrite32((void *)(DevInst->IOInst), RegOff,
			Data, Size);
}

AieRC XAie_BlockSet32(XAie_DevInst *DevInst, u64 RegOff, u32 Data, u32 Size)
{
	AieRC RC;
	u64 Tid;
	XAie_TxnInst *TxnInst;
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL) {
		Tid = Backend->Ops.GetTid();
		TxnInst = _XAie_GetTxnInst(DevInst, Tid);
		if(TxnInst == NULL) {
			XAIE_DBG("Could not find transaction instance "
					"associated with thread. Block set "
					"to register\n");
			goto blockset;
		}

		if(TxnInst->Flags & XAIE_TXN_AUTO_FLUSH_MASK) {
			/* Flush command buffer */
			XAIE_DBG("Auto flushing contents of the transaction "
					"buffer.\n");
			if(TxnInst->NumCmds > 0) {
				RC = _XAie_Txn_FlushCmdBuf(DevInst, TxnInst);
				if(RC != XAIE_OK) {
					XAIE_ERROR("Failed to flush cmd buffer\n");
					return RC;
				}
			}

			TxnInst->NumCmds = 0;
			goto blockset;
		}

		if(TxnInst->NumCmds + 1U == TxnInst->MaxCmds) {
			RC = _XAie_ReallocCmdBuf(TxnInst);
			if(RC != XAIE_OK) return RC;
		}

		TxnInst->CmdBuf[TxnInst->NumCmds].Opcode = XAIE_IO_BLOCKSET;
		TxnInst->CmdBuf[TxnInst->NumCmds].RegOff = RegOff;
		TxnInst->CmdBuf[TxnInst->NumCmds].Value = Data;
		TxnInst->CmdBuf[TxnInst->NumCmds].Size = Size;
		TxnInst->CmdBuf[TxnInst->NumCmds].Mask = 0U;
		TxnInst->NumCmds++;

		return XAIE_OK;
	}

blockset:
	return Backend->Ops.BlockSet32((void *)(DevInst->IOInst), RegOff, Data,
			Size);
}

AieRC XAie_CmdWrite(XAie_DevInst *DevInst, u8 Col, u8 Row, u8 Command,
		u32 CmdWd0, u32 CmdWd1, const char *CmdStr)
{
	AieRC RC;
	u64 Tid;
	XAie_TxnInst *TxnInst;
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL) {
		Tid = Backend->Ops.GetTid();
		TxnInst = _XAie_GetTxnInst(DevInst, Tid);
		if(TxnInst == NULL) {
			XAIE_DBG("Could not find transaction instance "
					"associated with thread. Writing cmd "
					"to register\n");
			goto cmdwrite;
		}

		if((TxnInst->Flags & XAIE_TXN_AUTO_FLUSH_MASK) &&
				(TxnInst->NumCmds > 0)) {
			/* Flush command buffer */
			XAIE_DBG("Auto flushing contents of the transaction "
					"buffer.\n");
			RC = _XAie_Txn_FlushCmdBuf(DevInst, TxnInst);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to flush cmd buffer\n");
				return RC;
			}

			TxnInst->NumCmds = 0;
			goto cmdwrite;
		} else if(TxnInst->NumCmds == 0) {
			goto cmdwrite;
		} else {
			XAIE_ERROR("Cmd Write operation is not supported "
					"when auto flush is disabled\n");
			return XAIE_ERR;
		}
	}

cmdwrite:
	return Backend->Ops.CmdWrite((void *)(DevInst->IOInst), Col, Row,
			Command, CmdWd0, CmdWd1, CmdStr);
}

AieRC XAie_RunOp(XAie_DevInst *DevInst, XAie_BackendOpCode Op, void *Arg)
{
	AieRC RC;
	u64 Tid;
	XAie_TxnInst *TxnInst;
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL) {
		Tid = Backend->Ops.GetTid();
		TxnInst = _XAie_GetTxnInst(DevInst, Tid);
		if(TxnInst == NULL) {
			XAIE_DBG("Could not find transaction instance "
					"associated with thread. Running Op.\n");
			goto runop;
		}

		if((TxnInst->Flags & XAIE_TXN_AUTO_FLUSH_MASK) &&
				(TxnInst->NumCmds > 0)) {
			/* Flush command buffer */
			RC = _XAie_Txn_FlushCmdBuf(DevInst, TxnInst);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to flush cmd buffer\n");
				return RC;
			}

			TxnInst->NumCmds = 0;
			goto runop;
		} else if(TxnInst->NumCmds == 0) {
			goto runop;
		} else {
			XAIE_ERROR("Cmd Write operation is not supported "
					"when auto flush is disabled\n");
			return XAIE_ERR;
		}
	}

runop:
	return Backend->Ops.RunOp(DevInst->IOInst, DevInst, Op, Arg);
}

/** @} */
