/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_cdo.c
* @{
*
* This file contains the data structures and routines for low level IO
* operations for cdo backend.
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

#ifdef __AIECDO__ /* AIE simulator */

#include "main_rts.h"
#include "cdo_rts.h"

#endif

#include "xaie_cdo.h"
#include "xaie_helper.h"
#include "xaie_io.h"
#include "xaie_npi.h"

/************************** Constant Definitions *****************************/
/****************************** Type Definitions *****************************/
typedef struct {
	u64 BaseAddr;
	u64 NpiBaseAddr;
} XAie_CdoIO;

/************************** Variable Definitions *****************************/
const XAie_Backend CdoBackend =
{
	.Type = XAIE_IO_BACKEND_CDO,
	.Ops.Init = XAie_CdoIO_Init,
	.Ops.Finish = XAie_CdoIO_Finish,
	.Ops.Write32 = XAie_CdoIO_Write32,
	.Ops.Read32 = XAie_CdoIO_Read32,
	.Ops.MaskWrite32 = XAie_CdoIO_MaskWrite32,
	.Ops.MaskPoll = XAie_CdoIO_MaskPoll,
	.Ops.BlockWrite32 = XAie_CdoIO_BlockWrite32,
	.Ops.BlockSet32 = XAie_CdoIO_BlockSet32,
	.Ops.CmdWrite = XAie_CdoIO_CmdWrite,
	.Ops.RunOp = XAie_CdoIO_RunOp,
	.Ops.MemAllocate = XAie_CdoMemAllocate,
	.Ops.MemFree = XAie_CdoMemFree,
	.Ops.MemSyncForCPU = XAie_CdoMemSyncForCPU,
	.Ops.MemSyncForDev = XAie_CdoMemSyncForDev,
	.Ops.MemAttach = XAie_CdoMemAttach,
	.Ops.MemDetach = XAie_CdoMemDetach,
};

/************************** Function Definitions *****************************/
#ifdef __AIECDO__

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
AieRC XAie_CdoIO_Finish(void *IOInst)
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
AieRC XAie_CdoIO_Init(XAie_DevInst *DevInst)
{
	XAie_CdoIO *IOInst;

	IOInst = (XAie_CdoIO *)malloc(sizeof(*IOInst));
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
* @param	Data: 32-bit data to be written.
*
* @return	None.
*
* @note		None.
* @note		Internal only.
*
*******************************************************************************/
void XAie_CdoIO_Write32(void *IOInst, u64 RegOff, u32 Value)
{
	XAie_CdoIO *CdoIOInst = (XAie_CdoIO *)IOInst;

	cdo_Write32(CdoIOInst->BaseAddr + RegOff, Value);
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
* @note		Internal only.
*
*******************************************************************************/
u32 XAie_CdoIO_Read32(void *IOInst, u64 RegOff)
{
	/* no-op */
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
* @note		Internal only.
*
*******************************************************************************/
void XAie_CdoIO_MaskWrite32(void *IOInst, u64 RegOff, u32 Mask, u32 Value)
{
	XAie_CdoIO *CdoIOInst = (XAie_CdoIO *)IOInst;
	cdo_MaskWrite32(CdoIOInst->BaseAddr + RegOff, Mask, Value);
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
* @note		Internal only.
*
*******************************************************************************/
u32 XAie_CdoIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask, u32 Value,
		u32 TimeOutUs)
{
	XAie_CdoIO *CdoIOInst = (XAie_CdoIO *)IOInst;
	/* Round up to msec */
	cdo_MaskPoll(CdoIOInst->BaseAddr + RegOff, Mask, Value,
			(TimeOutUs + 999) / 1000);
	return XAIE_SUCCESS;
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
* @note		Internal only.
*
*******************************************************************************/
void XAie_CdoIO_BlockWrite32(void *IOInst, u64 RegOff, u32 *Data, u32 Size)
{
	XAie_CdoIO *CdoIOInst = (XAie_CdoIO *)IOInst;

	cdo_BlockWrite32(CdoIOInst->BaseAddr + RegOff, Data, Size);
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
* @note		Internal only.
*
*******************************************************************************/
void XAie_CdoIO_BlockSet32(void *IOInst, u64 RegOff, u32 Data, u32 Size)
{
	XAie_CdoIO *CdoIOInst = (XAie_CdoIO *)IOInst;

	cdo_BlockSet32(CdoIOInst->BaseAddr + RegOff, Data, Size);
}

/*****************************************************************************/
/**
*
* This is the function to write 32 bit value to NPI register address.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: NPI register offset
* @param	RegVal: Value to write to register
*
* @return	None.
*
* @note		None.
* @note		Internal only.
*
*******************************************************************************/
static void _XAie_CdoIO_NpiWrite32(void *IOInst, u32 RegOff, u32 RegVal)
{
	XAie_CdoIO *CdoIOInst = (XAie_CdoIO *)IOInst;
	u64 RegAddr;

	RegAddr = CdoIOInst->NpiBaseAddr + RegOff;
	cdo_Write32(RegAddr, RegVal);
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
AieRC XAie_CdoIO_RunOp(void *IOInst, XAie_DevInst *DevInst,
		     XAie_BackendOpCode Op, void *Arg)
{
	AieRC RC = XAIE_OK;
	(void)IOInst;

	switch(Op) {
		case XAIE_BACKEND_OP_NPIWR32:
		{
			XAie_BackendNpiWrReq *Req = Arg;

			_XAie_CdoIO_NpiWrite32(IOInst, Req->NpiRegOff,
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
				XAie_CdoIO_Write32(IOInst, BdArgs->Addr + i * 4,
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

#else

AieRC XAie_CdoIO_Finish(void *IOInst)
{
	/* no-op */
	(void)IOInst;
	return XAIE_OK;
}

AieRC XAie_CdoIO_Init(XAie_DevInst *DevInst)
{
	/* no-op */
	(void)DevInst;
	XAIE_ERROR("Driver is not compiled with cdo generation "
			"backend (__AIECDO__)\n");
	return XAIE_INVALID_BACKEND;
}

void XAie_CdoIO_Write32(void *IOInst, u64 RegOff, u32 Value)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Value;
}

u32 XAie_CdoIO_Read32(void *IOInst, u64 RegOff)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	return 0;
}

void XAie_CdoIO_MaskWrite32(void *IOInst, u64 RegOff, u32 Mask, u32 Value)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Mask;
	(void)Value;
}

u32 XAie_CdoIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask, u32 Value,
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

void XAie_CdoIO_BlockWrite32(void *IOInst, u64 RegOff, u32 *Data, u32 Size)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Data;
	(void)Size;
}

void XAie_CdoIO_BlockSet32(void *IOInst, u64 RegOff, u32 Data, u32 Size)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Data;
	(void)Size;
}

AieRC XAie_CdoIO_RunOp(void *IOInst, XAie_DevInst *DevInst,
		     XAie_BackendOpCode Op, void *Arg)
{
	(void)IOInst;
	(void)DevInst;
	(void)Op;
	(void)Arg;
	return XAIE_FEATURE_NOT_SUPPORTED;
}

#endif /* __AIECDO__ */

void XAie_CdoIO_CmdWrite(void *IOInst, u8 Col, u8 Row, u8 Command, u32 CmdWd0,
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

XAie_MemInst* XAie_CdoMemAllocate(XAie_DevInst *DevInst, u64 Size,
		XAie_MemCacheProp Cache)
{
	(void)DevInst;
	(void)Size;
	(void)Cache;
	return NULL;
}

AieRC XAie_CdoMemFree(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

AieRC XAie_CdoMemSyncForCPU(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

AieRC XAie_CdoMemSyncForDev(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

AieRC XAie_CdoMemAttach(XAie_MemInst *MemInst, u64 MemHandle)
{
	(void)MemInst;
	(void)MemHandle;
	return XAIE_ERR;
}

AieRC XAie_CdoMemDetach(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

/** @} */
