/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_sim.c
* @{
*
* This file contains the data structures and routines for low level IO
* operations for simulation backend.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   06/09/2020 Initial creation.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __AIESIM__ /* AIE simulator */

#include "main_rts.h"

#endif

#include "xaie_helper.h"
#include "xaie_io.h"
#include "xaie_io_common.h"

/****************************** Type Definitions *****************************/
typedef struct {
	u64 BaseAddr;
} XAie_SimIO;

/************************** Function Definitions *****************************/
#ifdef __AIESIM__

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
static AieRC XAie_SimIO_Finish(void *IOInst)
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
* @note		Internal only.
*
*******************************************************************************/
static AieRC XAie_SimIO_Init(XAie_DevInst *DevInst)
{
	XAie_SimIO *IOInst;

	IOInst = (XAie_SimIO *)malloc(sizeof(*IOInst));
	if(IOInst == NULL) {
		XAIE_ERROR("Memory allocation failed\n");
		return XAIE_ERR;
	}

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
static AieRC XAie_SimIO_Write32(void *IOInst, u64 RegOff, u32 Value)
{
	XAie_SimIO *SimIOInst = (XAie_SimIO *)IOInst;

	ess_Write32(SimIOInst->BaseAddr + RegOff, Value);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to read 32bit data from the specified address.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Data: Pointer to store the 32 bit value
*
* @return	XAIE_OK on success.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC XAie_SimIO_Read32(void *IOInst, u64 RegOff, u32 *Data)
{
	XAie_SimIO *SimIOInst = (XAie_SimIO *)IOInst;

	*Data = ess_Read32(SimIOInst->BaseAddr + RegOff);

	return XAIE_OK;
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
static AieRC XAie_SimIO_MaskWrite32(void *IOInst, u64 RegOff, u32 Mask,
		u32 Value)
{
	u32 RegVal;

	XAie_SimIO_Read32(IOInst, RegOff, &RegVal);

	RegVal &= ~Mask;
	RegVal |= Value;

	XAie_SimIO_Write32(IOInst, RegOff, RegVal);

	return XAIE_OK;
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
* @return	XAIE_OK or XAIE_ERR.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC XAie_SimIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask, u32 Value,
		u32 TimeOutUs)
{
	AieRC Ret = XAIE_ERR;
	u32 RegVal;


	/* Increment Timeout value to 1 if user passed value is 1 */
	if(TimeOutUs == 0U)
		TimeOutUs++;

	while(TimeOutUs > 0U) {
		XAie_SimIO_Read32(IOInst, RegOff, &RegVal);
		if((RegVal & Mask) == Value) {
			Ret = XAIE_OK;
			break;
		}
		usleep(1);
		TimeOutUs--;
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
static AieRC XAie_SimIO_BlockWrite32(void *IOInst, u64 RegOff, u32 *Data,
		u32 Size)
{
	for(u32 i = 0U; i < Size; i++) {
		XAie_SimIO_Write32(IOInst, RegOff + i * 4U, *Data);
		Data++;
	}

	return XAIE_OK;
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
static AieRC XAie_SimIO_BlockSet32(void *IOInst, u64 RegOff, u32 Data, u32 Size)
{
	for(u32 i = 0U; i < Size; i++)
		XAie_SimIO_Write32(IOInst, RegOff+ i * 4U, Data);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to initialize a chunk of aie address space with
* a specified value.
*
* @param	IOInst: IO instance pointer
* @param	Col: Column number
* @param	Row: Row number
* @param	Command: Command to write
* @param	CmdWd0: Command word 0
* @param	CmdWd1: Command word 1
* @param	CmdStr: Command string
*
* @return	None.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC XAie_SimIO_CmdWrite(void *IOInst, u8 Col, u8 Row, u8 Command,
		u32 CmdWd0, u32 CmdWd1, const char *CmdStr)
{
	ess_WriteCmd(Command, Col, Row, CmdWd0, CmdWd1, CmdStr);

	return XAIE_OK;
}

static AieRC XAie_SimIO_RunOp(void *IOInst, XAie_DevInst *DevInst,
		XAie_BackendOpCode Op, void *Arg)
{
	(void)DevInst;
	switch(Op) {
	case XAIE_BACKEND_OP_CONFIG_SHIMDMABD:
	{
		XAie_ShimDmaBdArgs *BdArgs = (XAie_ShimDmaBdArgs *)Arg;
		for(u8 i = 0; i < BdArgs->NumBdWords; i++) {
			XAie_SimIO_Write32(IOInst, BdArgs->Addr + i * 4,
					BdArgs->BdWords[i]);
		}
		break;
	}
	case XAIE_BACKEND_OP_REQUEST_TILES:
	{
		XAIE_DBG("Backend doesn't support Op %u.\n", Op);
		return XAIE_FEATURE_NOT_SUPPORTED;
	}
	case XAIE_BACKEND_OP_REQUEST_RESOURCE:
		return _XAie_RequestRscCommon(DevInst, Arg);
	case XAIE_BACKEND_OP_RELEASE_RESOURCE:
		return _XAie_ReleaseRscCommon(Arg);
	case XAIE_BACKEND_OP_FREE_RESOURCE:
		return _XAie_FreeRscCommon(Arg);
	case XAIE_BACKEND_OP_REQUEST_ALLOCATED_RESOURCE:
		return _XAie_RequestAllocatedRscCommon(DevInst, Arg);
	default:
		XAIE_ERROR("Linux backend does not support operation %d\n", Op);
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	return XAIE_OK;
}

#else

static AieRC XAie_SimIO_Finish(void *IOInst)
{
	/* no-op */
	(void)IOInst;
	return XAIE_OK;
}

static AieRC XAie_SimIO_Init(XAie_DevInst *DevInst)
{
	/* no-op */
	(void)DevInst;
	XAIE_ERROR("Driver is not compiled with simulation backend "
			"(__AIESIM__)\n");
	return XAIE_INVALID_BACKEND;
}

static AieRC XAie_SimIO_Write32(void *IOInst, u64 RegOff, u32 Value)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Value;

	return XAIE_ERR;
}

static AieRC XAie_SimIO_Read32(void *IOInst, u64 RegOff, u32 *Data)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Data;
	return 0;
}

static AieRC XAie_SimIO_MaskWrite32(void *IOInst, u64 RegOff, u32 Mask,
		u32 Value)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Mask;
	(void)Value;

	return XAIE_ERR;
}

static AieRC XAie_SimIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask, u32 Value,
		u32 TimeOutUs)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Mask;
	(void)Value;
	(void)TimeOutUs;
	return XAIE_ERR;
}

static AieRC XAie_SimIO_BlockWrite32(void *IOInst, u64 RegOff, u32 *Data,
		u32 Size)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Data;
	(void)Size;

	return XAIE_ERR;
}

static AieRC XAie_SimIO_BlockSet32(void *IOInst, u64 RegOff, u32 Data, u32 Size)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Data;
	(void)Size;

	return XAIE_ERR;
}

static AieRC XAie_SimIO_CmdWrite(void *IOInst, u8 Col, u8 Row, u8 Command,
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

	return XAIE_ERR;
}

static AieRC XAie_SimIO_RunOp(void *IOInst, XAie_DevInst *DevInst,
		XAie_BackendOpCode Op, void *Arg)
{
	(void)IOInst;
	(void)DevInst;
	(void)Op;
	(void)Arg;
	return XAIE_FEATURE_NOT_SUPPORTED;
}

#endif /* __AIESIM__ */

static XAie_MemInst* XAie_SimMemAllocate(XAie_DevInst *DevInst, u64 Size,
		XAie_MemCacheProp Cache)
{
	(void)DevInst;
	(void)Size;
	(void)Cache;
	return NULL;
}

static AieRC XAie_SimMemFree(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

static AieRC XAie_SimMemSyncForCPU(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

static AieRC XAie_SimMemSyncForDev(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

static AieRC XAie_SimMemAttach(XAie_MemInst *MemInst, u64 MemHandle)
{
	(void)MemInst;
	(void)MemHandle;
	return XAIE_ERR;
}

static AieRC XAie_SimMemDetach(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

const XAie_Backend SimBackend =
{
	.Type = XAIE_IO_BACKEND_SIM,
	.Ops.Init = XAie_SimIO_Init,
	.Ops.Finish = XAie_SimIO_Finish,
	.Ops.Write32 = XAie_SimIO_Write32,
	.Ops.Read32 = XAie_SimIO_Read32,
	.Ops.MaskWrite32 = XAie_SimIO_MaskWrite32,
	.Ops.MaskPoll = XAie_SimIO_MaskPoll,
	.Ops.BlockWrite32 = XAie_SimIO_BlockWrite32,
	.Ops.BlockSet32 = XAie_SimIO_BlockSet32,
	.Ops.CmdWrite = XAie_SimIO_CmdWrite,
	.Ops.RunOp = XAie_SimIO_RunOp,
	.Ops.MemAllocate = XAie_SimMemAllocate,
	.Ops.MemFree = XAie_SimMemFree,
	.Ops.MemSyncForCPU = XAie_SimMemSyncForCPU,
	.Ops.MemSyncForDev = XAie_SimMemSyncForDev,
	.Ops.MemAttach = XAie_SimMemAttach,
	.Ops.MemDetach = XAie_SimMemDetach,
	.Ops.GetTid = XAie_IODummyGetTid,
	.Ops.SubmitTxn = NULL,
};

/** @} */
