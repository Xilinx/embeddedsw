/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_debug.c
* @{
*
* This file contains the data structures and routines for low level IO
* operations for debug.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   06/29/2020 Initial creation.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include <stdio.h>
#include <stdlib.h>

#include "xaie_debug.h"
#include "xaie_helper.h"
#include "xaie_io.h"
#include "xaie_npi.h"

/****************************** Type Definitions *****************************/
typedef struct {
	u64 BaseAddr;
	u64 NpiBaseAddr;
} XAie_DebugIO;

/************************** Variable Definitions *****************************/
const XAie_Backend DebugBackend =
{
	.Type = XAIE_IO_BACKEND_DEBUG,
	.Ops.Init = XAie_DebugIO_Init,
	.Ops.Finish = XAie_DebugIO_Finish,
	.Ops.Write32 = XAie_DebugIO_Write32,
	.Ops.Read32 = XAie_DebugIO_Read32,
	.Ops.MaskWrite32 = XAie_DebugIO_MaskWrite32,
	.Ops.MaskPoll = XAie_DebugIO_MaskPoll,
	.Ops.BlockWrite32 = XAie_DebugIO_BlockWrite32,
	.Ops.BlockSet32 = XAie_DebugIO_BlockSet32,
	.Ops.CmdWrite = XAie_DebugIO_CmdWrite,
	.Ops.RunOp = XAie_DebugIO_RunOp,
	.Ops.MemAllocate = XAie_DebugMemAllocate,
	.Ops.MemFree = XAie_DebugMemFree,
	.Ops.MemSyncForCPU = XAie_DebugMemSyncForCPU,
	.Ops.MemSyncForDev = XAie_DebugMemSyncForDev,
	.Ops.MemAttach = XAie_DebugMemAttach,
	.Ops.MemDetach = XAie_DebugMemDetach,
};

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This is the memory IO function to free the global IO instance
*
* @param	IOInst: IO Instance pointer.
*
* @return	None.
*
* @note		The global IO instance is a singleton and freed when
* the reference count reaches a zero.
*
*******************************************************************************/
AieRC XAie_DebugIO_Finish(void *IOInst)
{
	free(IOInst);
	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to initialize the global IO instance
*
* @param	DevInst: Device instance pointer.
*
* @return	XAIE_OK on success. Error code on failure.
*
* @note		None.
*
*******************************************************************************/
AieRC XAie_DebugIO_Init(XAie_DevInst *DevInst)
{
	XAie_DebugIO *IOInst;

	IOInst = (XAie_DebugIO *)malloc(sizeof(*IOInst));
	if(IOInst == NULL) {
		XAIE_ERROR("Memory allocation failed\n");
		return XAIE_ERR;
	}

	IOInst->BaseAddr = DevInst->BaseAddr;
	IOInst->NpiBaseAddr = XAIE_NPI_BASEADDR;
	DevInst->IOInst = IOInst;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to write 32bit data to the specified address.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Value: 32-bit data to be written.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAie_DebugIO_Write32(void *IOInst, u64 RegOff, u32 Value)
{
	XAie_DebugIO *DebugIOInst = (XAie_DebugIO *)IOInst;

	printf("W: 0x%lx, 0x%x\n", DebugIOInst->BaseAddr + RegOff, Value);
}

/*****************************************************************************/
/**
*
* This is the memory IO function to read 32bit data from the specified address.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
*
* @return	32-bit read value.
*
* @note		None.
*
*******************************************************************************/
u32 XAie_DebugIO_Read32(void *IOInst, u64 RegOff)
{
	XAie_DebugIO *DebugIOInst = (XAie_DebugIO *)IOInst;

	printf("R: 0x%lx, 0x%x\n", DebugIOInst->BaseAddr + RegOff, 0);

	return 0;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to write masked 32bit data to the specified
* address.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Mask: Mask to be applied to Data.
* @param	Value: 32-bit data to be written.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAie_DebugIO_MaskWrite32(void *IOInst, u64 RegOff, u32 Mask, u32 Value)
{
	XAie_DebugIO *DebugIOInst = (XAie_DebugIO *)IOInst;

	printf("MW: 0x%lx, 0x%x, 0x%x\n", DebugIOInst->BaseAddr + RegOff, Mask,
			Value);
}

/*****************************************************************************/
/**
*
* This is the memory IO function to mask poll an address for a value.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Mask: Mask to be applied to Data.
* @param	Value: 32-bit value to poll for
* @param	TimeOutUs: Timeout in micro seconds.
*
* @return	XAIE_SUCCESS or XAIE_FAILURE.
*
* @note		None.
*
*******************************************************************************/
u32 XAie_DebugIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask, u32 Value,
		u32 TimeOutUs)
{
	XAie_DebugIO *DebugIOInst = (XAie_DebugIO *)IOInst;

	printf("MP: 0x%lx, 0x%x, 0x%x, 0x%d\n", DebugIOInst->BaseAddr + RegOff,
			Mask, Value, TimeOutUs);

	return XAIE_FAILURE;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to write a block of data to aie.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Data: Pointer to the data buffer.
* @param	Size: Number of 32-bit words.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAie_DebugIO_BlockWrite32(void *IOInst, u64 RegOff, u32 *Data, u32 Size)
{
	for(u32 i = 0U; i < Size; i ++) {
		XAie_DebugIO_Write32(IOInst, RegOff + i * 4U, *Data);
		Data++;
	}
}

/*****************************************************************************/
/**
*
* This is the memory IO function to initialize a chunk of aie address space with
* a specified value.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Data: Data to initialize a chunk of aie address space..
* @param	Size: Number of 32-bit words.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAie_DebugIO_BlockSet32(void *IOInst, u64 RegOff, u32 Data, u32 Size)
{
	for(u32 i = 0U; i < Size; i++)
		XAie_DebugIO_Write32(IOInst, RegOff+ i * 4U, Data);
}

void XAie_DebugIO_CmdWrite(void *IOInst, u8 Col, u8 Row, u8 Command, u32 CmdWd0,
		u32 CmdWd1, const char *CmdStr)
{
	/* no-op */
	(void)IOInst;
	(void)Col;
	(void)Row;
	(void)Command;
	(void)CmdWd0;
	(void)CmdWd1;
	(void)CmdStr;
}

/*****************************************************************************/
/**
*
* This is the function to write to AI engine NPI registers
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to write.
* @param	RegVal: Register value to write
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
static void _XAie_DebugIO_NpiWrite32(void *IOInst, u32 RegOff, u32 RegVal)
{
	XAie_DebugIO *DebugIOInst = (XAie_DebugIO *)IOInst;
	u64 RegAddr;

	RegAddr = DebugIOInst->NpiBaseAddr + RegOff;
	printf("NPIMW: 0x%lx, 0x%x\n", RegAddr, RegVal);
}

/*****************************************************************************/
/**
*
* This is the function to run backend operations
*
* @param	IOInst: IO instance pointer
* @param	DevInst: AI engine partition device instance
* @param	Op: Backend operation code
* @param	Arg: Backend operation argument
*
* @return	XAIE_OK for success and error code for failure.
*
* @note		None.
*
*******************************************************************************/
AieRC XAie_DebugIO_RunOp(void *IOInst, XAie_DevInst *DevInst,
		     XAie_BackendOpCode Op, void *Arg)
{
	AieRC RC = XAIE_OK;
	(void)IOInst;

	switch(Op) {
		case XAIE_BACKEND_OP_NPIWR32:
		{
			XAie_BackendNpiWrReq *Req = Arg;

			_XAie_DebugIO_NpiWrite32(IOInst, Req->NpiRegOff,
					Req->Val);
			break;
		}
		case XAIE_BACKEND_OP_ASSERT_SHIMRST:
		{
			u8 RstEnable = (u8)((uintptr_t)Arg & 0xFF);

			_XAie_NpiSetShimReset(DevInst, RstEnable);
			break;
		}
		case XAIE_BACKEND_OP_SET_PROTREG:
		{
			RC = _XAie_NpiSetProtectedRegEnable(DevInst, Arg);
			break;
		}
		case XAIE_BACKEND_OP_CONFIG_SHIMDMABD:
		{
			XAie_ShimDmaBdArgs *BdArgs = (XAie_ShimDmaBdArgs *)Arg;
			for(u8 i = 0; i < BdArgs->NumBdWords; i++) {
				XAie_DebugIO_Write32(IOInst,
						BdArgs->Addr + i * 4,
						BdArgs->BdWords[i]);
			}
			break;
		}
		case XAIE_BACKEND_OP_REQUEST_TILES:
		{
			XAIE_DBG("Backend doesn't support Op %u.\n", Op);
			return XAIE_FEATURE_NOT_SUPPORTED;
		}
		default:
			XAIE_ERROR("Backend doesn't support Op %u.\n", Op);
			RC = XAIE_FEATURE_NOT_SUPPORTED;
			break;
	}

	return RC;
}

/*****************************************************************************/
/**
*
* This is the memory function to allocate a memory
*
* @param	DevInst: Device Instance
* @param	Size: Size of the memory
* @param	Cache: Buffer to be cacheable or not
*
* @return	Pointer to the allocated memory instance.
*
* @note		Internal only.
*
*******************************************************************************/
XAie_MemInst* XAie_DebugMemAllocate(XAie_DevInst *DevInst, u64 Size,
		XAie_MemCacheProp Cache)
{
	XAie_MemInst *MemInst;

	MemInst = (XAie_MemInst *)malloc(sizeof(*MemInst));
	if(MemInst == NULL) {
		XAIE_ERROR("memory allocation failed\n");
		return NULL;
	}

	MemInst->VAddr = (void *)malloc(Size);
	if(MemInst->VAddr == NULL) {
		XAIE_ERROR("malloc failed\n");
		free(MemInst);
		return NULL;
	}
	MemInst->DevAddr = (u64)MemInst->VAddr;
	MemInst->Size = Size;
	MemInst->DevInst = DevInst;

	(void)Cache;
	XAIE_DBG("Cache attribute is ignored\n");

	return MemInst;
}

/*****************************************************************************/
/**
*
* This is the memory function to free the memory
*
* @param	MemInst: Memory instance pointer.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*
*******************************************************************************/
AieRC XAie_DebugMemFree(XAie_MemInst *MemInst)
{
	free(MemInst->VAddr);
	free(MemInst);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory function to sync the memory for CPU
*
* @param	MemInst: Memory instance pointer.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*
*******************************************************************************/
AieRC XAie_DebugMemSyncForCPU(XAie_MemInst *MemInst)
{
	(void)MemInst;
	XAIE_DBG("Sync for CPU is no-op in debug mode\n");

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory function to sync the memory for CPU
*
* @param	MemInst: Memory instance pointer.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*
*******************************************************************************/
AieRC XAie_DebugMemSyncForDev(XAie_MemInst *MemInst)
{
	(void)MemInst;
	XAIE_DBG("Sync for Dev is no-op in debug mode\n");

	return XAIE_OK;
}

AieRC XAie_DebugMemAttach(XAie_MemInst *MemInst, u64 MemHandle)
{
	(void)MemInst;
	(void)MemHandle;
	XAIE_DBG("Mem attach is no-op in debug mode\n");

	return XAIE_OK;
}

AieRC XAie_DebugMemDetach(XAie_MemInst *MemInst)
{
	(void)MemInst;
	XAIE_DBG("Mem detach is no-op in debug mode\n");

	return XAIE_OK;
}

/** @} */
