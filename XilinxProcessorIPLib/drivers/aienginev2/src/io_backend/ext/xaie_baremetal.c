/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_baremetal.c
* @{
*
* This file contains the data structures and routines for low level IO
* operations for baremetal backend.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   07/04/2020 Initial creation.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include <stdlib.h>

#ifdef __AIEBAREMETAL__

#include "sleep.h"
#include "xil_cache.h"
#include "xil_io.h"
#include "xil_types.h"
#include "xstatus.h"

#endif

#include "xaie_helper.h"
#include "xaie_baremetal.h"
#include "xaie_io.h"

/****************************** Type Definitions *****************************/
typedef struct {
	u64 BaseAddr;
} XAie_BaremetalIO;

/************************** Variable Definitions *****************************/
static XAie_BaremetalIO BaremetalIO;

const XAie_Backend BaremetalBackend =
{
	.Type = XAIE_IO_BACKEND_BAREMETAL,
	.Ops.Init = XAie_BaremetalIO_Init,
	.Ops.Finish = XAie_BaremetalIO_Finish,
	.Ops.Write32 = XAie_BaremetalIO_Write32,
	.Ops.Read32 = XAie_BaremetalIO_Read32,
	.Ops.MaskWrite32 = XAie_BaremetalIO_MaskWrite32,
	.Ops.MaskPoll = XAie_BaremetalIO_MaskPoll,
	.Ops.BlockWrite32 = XAie_BaremetalIO_BlockWrite32,
	.Ops.BlockSet32 = XAie_BaremetalIO_BlockSet32,
	.Ops.CmdWrite = XAie_BaremetalIO_CmdWrite,
	.Ops.RunOp = XAie_BaremetalIO_RunOp,
	.Ops.MemAllocate = XAie_BaremetalMemAllocate,
	.Ops.MemFree = XAie_BaremetalMemFree,
	.Ops.MemSyncForCPU = XAie_BaremetalMemSyncForCPU,
	.Ops.MemSyncForDev = XAie_BaremetalMemSyncForDev,
	.Ops.MemAttach = XAie_BaremetalMemAttach,
	.Ops.MemDetach = XAie_BaremetalMemDetach,
};

/************************** Function Definitions *****************************/
#ifdef __AIEBAREMETAL__

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
* the reference count reaches a zero. Internal only.
*
*******************************************************************************/
AieRC XAie_BaremetalIO_Finish(void *IOInst)
{
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
* @note		Internal only.
*
*******************************************************************************/
AieRC XAie_BaremetalIO_Init(XAie_DevInst *DevInst)
{
	XAie_BaremetalIO *IOInst = &BaremetalIO;

	IOInst->BaseAddr = DevInst->BaseAddr;
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
* @param	Data: 32-bit data to be written.
*
* @return	None.
*
* @note		Internal only.
*
*******************************************************************************/
void XAie_BaremetalIO_Write32(void *IOInst, u64 RegOff, u32 Value)
{
	XAie_BaremetalIO *BaremetalIOInst = (XAie_BaremetalIO *)IOInst;

	Xil_Out32(BaremetalIOInst->BaseAddr + RegOff, Value);
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
* @note		Internal only.
*
*******************************************************************************/
u32 XAie_BaremetalIO_Read32(void *IOInst, u64 RegOff)
{
	XAie_BaremetalIO *BaremetalIOInst = (XAie_BaremetalIO *)IOInst;

	return Xil_In32(BaremetalIOInst->BaseAddr + RegOff);
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
* @note		Internal only.
*
*******************************************************************************/
void XAie_BaremetalIO_MaskWrite32(void *IOInst, u64 RegOff, u32 Mask, u32 Value)
{
	u32 RegVal = XAie_BaremetalIO_Read32(IOInst, RegOff);

	RegVal &= ~Mask;
	RegVal |= Value;

	XAie_BaremetalIO_Write32(IOInst, RegOff, RegVal);
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
* @note		Internal only.
*
*******************************************************************************/
u32 XAie_BaremetalIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask, u32 Value,
		u32 TimeOutUs)
{
	u32 Ret = XAIE_FAILURE;
	u32 Count, MinTimeOutUs;

	/*
	 * Any value less than 200 us becomes noticable overhead. This is based
	 * on some profiling, and it may vary between platforms.
	 */
	MinTimeOutUs = 200;
	Count = ((u64)TimeOutUs + MinTimeOutUs - 1) / MinTimeOutUs;

	while (Count > 0U) {
		if((XAie_BaremetalIO_Read32(IOInst, RegOff) & Mask) == Value) {
			Ret = XAIE_SUCCESS;
			break;
		}
		usleep(MinTimeOutUs);
		Count--;
	}

	/* Check for the break from timed-out loop */
	if((Ret == XAIE_FAILURE) &&
			((XAie_BaremetalIO_Read32(IOInst, RegOff) & Mask) ==
			 Value)) {
		Ret = XAIE_SUCCESS;
	}

	return Ret;
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
* @note		Internal only.
*
*******************************************************************************/
void XAie_BaremetalIO_BlockWrite32(void *IOInst, u64 RegOff, u32 *Data,
		u32 Size)
{
	for(u32 i = 0U; i < Size; i++) {
		XAie_BaremetalIO_Write32(IOInst, RegOff + i * 4U, *Data);
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
* @note		Internal only.
*
*******************************************************************************/
void XAie_BaremetalIO_BlockSet32(void *IOInst, u64 RegOff, u32 Data, u32 Size)
{
	for(u32 i = 0U; i < Size; i++)
		XAie_BaremetalIO_Write32(IOInst, RegOff+ i * 4U, Data);
}

/*****************************************************************************/
/**
*
* This is the memory function to allocate a memory
*
* @param	DevInst: Device Instance
* @param	Size: Size of the memory
* @param	Cache: Value from XAie_MemCacheProp enum
*
* @return	Pointer to the allocated memory instance.
*
* @note		Internal only.
*
*******************************************************************************/
XAie_MemInst* XAie_BaremetalMemAllocate(XAie_DevInst *DevInst, u64 Size,
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
	/*
	 * TODO: Cache is not handled at the moment for baremetal. The allocated
	 * memory is always cached.
	 */

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
AieRC XAie_BaremetalMemFree(XAie_MemInst *MemInst)
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
AieRC XAie_BaremetalMemSyncForCPU(XAie_MemInst *MemInst)
{
	Xil_DCacheInvalidateRange((u64)MemInst->VAddr, MemInst->Size);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory function to sync the memory for device
*
* @param	MemInst: Memory instance pointer.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*
*******************************************************************************/
AieRC XAie_BaremetalMemSyncForDev(XAie_MemInst *MemInst)
{
	Xil_DCacheFlushRange((u64)MemInst->VAddr, MemInst->Size);

	return XAIE_OK;
}

AieRC XAie_BaremetalMemAttach(XAie_MemInst *MemInst, u64 MemHandle)
{
	(void)MemInst;
	(void)MemHandle;
	return XAIE_OK;
}

AieRC XAie_BaremetalMemDetach(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_OK;
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
* @note		Internal only.
*
*******************************************************************************/
AieRC XAie_BaremetalIO_RunOp(void *IOInst, XAie_DevInst *DevInst,
		XAie_BackendOpCode Op, void *Arg)
{
	(void)DevInst;
	switch(Op) {
		case XAIE_BACKEND_OP_CONFIG_SHIMDMABD:
		{
			XAie_ShimDmaBdArgs *BdArgs =
				(XAie_ShimDmaBdArgs *)Arg;
			for(u8 i = 0; i < BdArgs->NumBdWords; i++) {
				XAie_BaremetalIO_Write32(IOInst,
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
			XAIE_ERROR("Linux backend does not support operation "
					"%d\n", Op);
			return XAIE_FEATURE_NOT_SUPPORTED;
	}

	return XAIE_OK;
}

#else

AieRC XAie_BaremetalIO_Finish(void *IOInst)
{
	/* no-op */
	(void)IOInst;
	return XAIE_OK;
}

AieRC XAie_BaremetalIO_Init(XAie_DevInst *DevInst)
{
	/* no-op */
	(void)DevInst;
	XAIE_ERROR("Driver is not compiled with baremetal "
			"backend (__AIEBAREMETAL__)\n");
	return XAIE_INVALID_BACKEND;
}

void XAie_BaremetalIO_Write32(void *IOInst, u64 RegOff, u32 Value)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Value;
}

u32 XAie_BaremetalIO_Read32(void *IOInst, u64 RegOff)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	return 0;
}

void XAie_BaremetalIO_MaskWrite32(void *IOInst, u64 RegOff, u32 Mask, u32 Value)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Mask;
	(void)Value;
}

u32 XAie_BaremetalIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask, u32 Value,
		u32 TimeOutUs)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Mask;
	(void)Value;
	(void)TimeOutUs;
	return XAIE_FAILURE;
}

void XAie_BaremetalIO_BlockWrite32(void *IOInst, u64 RegOff, u32 *Data,
		u32 Size)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Data;
	(void)Size;
}

void XAie_BaremetalIO_BlockSet32(void *IOInst, u64 RegOff, u32 Data, u32 Size)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Data;
	(void)Size;
}

XAie_MemInst* XAie_BaremetalMemAllocate(XAie_DevInst *DevInst, u64 Size,
		XAie_MemCacheProp Cache)
{
	(void)DevInst;
	(void)Size;
	(void)Cache;
	return NULL;
}

AieRC XAie_BaremetalMemFree(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

AieRC XAie_BaremetalMemSyncForCPU(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

AieRC XAie_BaremetalMemSyncForDev(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

AieRC XAie_BaremetalMemAttach(XAie_MemInst *MemInst, u64 MemHandle)
{
	(void)MemInst;
	(void)MemHandle;
	return XAIE_ERR;
}

AieRC XAie_BaremetalMemDetach(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

AieRC XAie_BaremetalIO_RunOp(void *IOInst, XAie_DevInst *DevInst,
		XAie_BackendOpCode Op, void *Arg)
{
	(void)IOInst;
	(void)DevInst;
	(void)Op;
	(void)Arg;
	return XAIE_FEATURE_NOT_SUPPORTED;
}

#endif /* __AIEBAREMETAL__ */

void XAie_BaremetalIO_CmdWrite(void *IOInst, u8 Col, u8 Row, u8 Command,
		u32 CmdWd0, u32 CmdWd1, const char *CmdStr)
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

/** @} */
