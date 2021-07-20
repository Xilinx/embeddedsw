/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_metal.c
* @{
*
* This file contains the low level layer IO interface
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Hyun    07/12/2018  Initial creation
* 1.1  Hyun    10/11/2018  Initialize the IO device for mem instance
* 1.2  Nishad  12/05/2018  Renamed ME attributes to AIE
* 1.3  Tejus   06/09/2020  Rename and import file from legacy driver.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#ifdef __AIEMETAL__

#include <metal/alloc.h>
#include <metal/device.h>
#include <metal/io.h>
#include <metal/irq.h>
#include <metal/list.h>
#include <metal/mutex.h>
#include <metal/shmem.h>
#include <metal/shmem-provider.h>
#include <metal/utilities.h>
#include <unistd.h>

#endif

#include "xaie_helper.h"
#include "xaie_io.h"
#include "xaie_io_common.h"
#include "xaie_npi.h"

/***************************** Macro Definitions *****************************/
#define SHM_NUM_ULONG	8					/**< number of ulong for bitmap */
#define SHM_MAX_IDS	(sizeof(unsigned long) * SHM_NUM_ULONG)	/**< max number of IDs = 64 * 16 = 512 */

/****************************** Type Definitions *****************************/
#ifdef __AIEMETAL__

typedef struct XAie_MetalIO {
	struct metal_device *device;	/**< libmetal device */
	struct metal_io_region *io;	/**< libmetal io region */
	u64 io_base;			/**< libmetal io region base */
	struct metal_device *npi_device;	/**< libmetal NPI device */
	struct metal_io_region *npi_io;	/**< libmetal NPI io region */
	unsigned long shm_ids[SHM_NUM_ULONG];	/**< bitmap for shm name space */
} XAie_MetalIO;

#endif /* __AIEMETAL__ */

/************************** Function Definitions *****************************/
#ifdef __AIEMETAL__

/*****************************************************************************/
/**
*
* This is the memory IO function to free the global IO instance
*
* @param	IOInst: IO Instance pointer.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		The global IO instance is a singleton and freed when
* the reference count reaches a zero. Internal only.
*
*******************************************************************************/
static AieRC XAie_MetalIO_Finish(void *IOInst)
{
	XAie_MetalIO *MetalIOInst = (XAie_MetalIO *)IOInst;

	metal_device_close(MetalIOInst->device);
	if (MetalIOInst->npi_device) {
		metal_device_close(MetalIOInst->npi_device);
	}
	metal_free_memory(IOInst);
	metal_finish();

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to initialize the global IO instance
*
* @param	None.
*
* @return	None.
*
* @note		The global IO instance is a singleton and any further attempt
* to initialize just increments the reference count. Internal only.
*
*******************************************************************************/
static AieRC XAie_MetalIO_Init(XAie_DevInst *DevInst)
{
	XAie_MetalIO *MetalIOInst;
	struct metal_init_params init_param = METAL_INIT_DEFAULTS;
	int ret;

	ret = metal_init(&init_param);
	if(ret) {
		XAIE_ERROR("failed to metal_init %d\n", ret);
		return XAIE_ERR;
	}

	MetalIOInst = (XAie_MetalIO *)metal_allocate_memory(
			sizeof(*MetalIOInst));
	if(MetalIOInst == NULL) {
		XAIE_ERROR("Memory allocation failed\n");
		goto finish;
	}


	ret = metal_device_open("platform", "xilinx-aiengine",
			&MetalIOInst->device);
	if(ret) {
		XAIE_ERROR("failed to metal_device_open\n");
		goto free_mem;
	}

	MetalIOInst->io = metal_device_io_region(MetalIOInst->device, 0);
	if(!MetalIOInst->io) {
		XAIE_ERROR("failed to metal_device_io_region\n");
		goto close;
	}

	MetalIOInst->io_base = metal_io_phys(MetalIOInst->io, 0);

	ret = metal_device_open("platform", "f70a0000.aie-npi",
			&MetalIOInst->npi_device);
	if (ret == 0) {
		MetalIOInst->npi_io = metal_device_io_region(MetalIOInst->npi_device, 0);
	}

	DevInst->IOInst = (void *)MetalIOInst;

	return XAIE_OK;

close:
	metal_device_close(MetalIOInst->device);

free_mem:
	metal_free_memory(MetalIOInst);

finish:
	metal_finish();
	return XAIE_ERR;
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
static AieRC XAie_MetalIO_Read32(void *IOInst, u64 RegOff, u32 *Data)
{
	XAie_MetalIO *MetalIOInst = (XAie_MetalIO *)IOInst;

	*Data = metal_io_read32(MetalIOInst->io, RegOff);

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
static AieRC XAie_MetalIO_Write32(void *IOInst, u64 RegOff, u32 Data)
{
	XAie_MetalIO *MetalIOInst = (XAie_MetalIO *)IOInst;

	metal_io_write32(MetalIOInst->io, RegOff, Data);

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
* @param	Data: 32-bit data to be written.
*
* @return	None.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC XAie_MetalIO_MaskWrite32(void *IOInst, u64 RegOff, u32 Mask,
		u32 Data)
{
	AieRC RC;
	u32 RegVal;

	RC = XAie_MetalIO_Read32(IOInst, RegOff, &RegVal);
	if(RC != XAIE_OK) {
		return RC;
	}

	RegVal &= ~Mask;
	RegVal |= Data;
	XAie_MetalIO_Write32(IOInst, RegOff, RegVal);

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
static AieRC XAie_MetalIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask, u32 Value,
		u32 TimeOutUs)
{
	AieRC Ret = XAIE_ERR;
	u32 Count, MinTimeOutUs, RegVal;

	/*
	 * Any value less than 200 us becomes noticable overhead. This is based
	 * on some profiling, and it may vary between platforms.
	 */
	MinTimeOutUs = 200;
	Count = ((u64)TimeOutUs + MinTimeOutUs - 1) / MinTimeOutUs;

	while (Count > 0U) {
		XAie_MetalIO_Read32(IOInst, RegOff, &RegVal);
		if((RegVal & Mask) == Value) {
			return XAIE_OK;
		}
		usleep(MinTimeOutUs);
		Count--;
	}

	/* Check for the break from timed-out loop */
	XAie_MetalIO_Read32(IOInst, RegOff, &RegVal);
	if((RegVal & Mask) == Value) {
		Ret = XAIE_OK;
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
static AieRC XAie_MetalIO_BlockWrite32(void *IOInst, u64 RegOff, u32 *Data,
		u32 Size)
{
	for(u32 i = 0; i < Size; i++) {
		XAie_MetalIO_Write32(IOInst, RegOff + i * 4U, *Data);
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
static AieRC XAie_MetalIO_BlockSet32(void *IOInst, u64 RegOff, u32 Data,
		u32 Size)
{
	for(u32 i = 0; i < Size; i++) {
		XAie_MetalIO_Write32(IOInst, RegOff + i * 4U, Data);
	}

	return XAIE_OK;
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
* @note		Internal only.
*
*******************************************************************************/
static void _XAie_MetalIO_NpiWrite32(void *IOInst, u32 RegOff, u32 RegVal)
{
	XAie_MetalIO *MetalIOInst = (XAie_MetalIO *)IOInst;

	metal_io_write32(MetalIOInst->npi_io, RegOff, RegVal);
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
static AieRC XAie_MetalIO_RunOp(void *IOInst, XAie_DevInst *DevInst,
		     XAie_BackendOpCode Op, void *Arg)
{
	AieRC RC = XAIE_FEATURE_NOT_SUPPORTED;
	XAie_MetalIO *MetalIOInst = (XAie_MetalIO *)IOInst;

	switch(Op) {
		case XAIE_BACKEND_OP_NPIWR32:
		{
			XAie_BackendNpiWrReq *Req = Arg;

			if (MetalIOInst->npi_io != NULL) {
				_XAie_MetalIO_NpiWrite32(IOInst,
						Req->NpiRegOff, Req->Val);
				RC = XAIE_OK;
			}
			break;
		}
		case XAIE_BACKEND_OP_ASSERT_SHIMRST:
		{
			u8 RstEnable = (u8)((uintptr_t)Arg & 0xFF);

			if (MetalIOInst->npi_io != NULL) {
				_XAie_NpiSetShimReset(DevInst, RstEnable);
				RC = XAIE_OK;
			}
			break;
		}
		case XAIE_BACKEND_OP_SET_PROTREG:
		{
			if (MetalIOInst->npi_io != NULL) {
				RC = _XAie_NpiSetProtectedRegEnable(DevInst,
						Arg);
			}
			break;
		}
		case XAIE_BACKEND_OP_CONFIG_SHIMDMABD:
		{
			XAie_ShimDmaBdArgs *BdArgs = (XAie_ShimDmaBdArgs *)Arg;
			for(u8 i = 0; i < BdArgs->NumBdWords; i++) {
				XAie_MetalIO_Write32(IOInst,
						BdArgs->Addr + i * 4,
						BdArgs->BdWords[i]);
			}
			RC = XAIE_OK;
			break;
		}
		case XAIE_BACKEND_OP_REQUEST_TILES:
			return _XAie_PrivilegeRequestTiles(DevInst,
					(XAie_BackendTilesArray *)Arg);
		case XAIE_BACKEND_OP_REQUEST_RESOURCE:
			return _XAie_RequestRscCommon(DevInst, Arg);
		case XAIE_BACKEND_OP_RELEASE_RESOURCE:
			return _XAie_ReleaseRscCommon(Arg);
		case XAIE_BACKEND_OP_FREE_RESOURCE:
			return _XAie_FreeRscCommon(Arg);
		case XAIE_BACKEND_OP_REQUEST_ALLOCATED_RESOURCE:
			return _XAie_RequestAllocatedRscCommon(DevInst, Arg);
		case XAIE_BACKEND_OP_PARTITION_INITIALIZE:
			return _XAie_PrivilegeInitPart(DevInst,
					(XAie_PartInitOpts *)Arg);
		case XAIE_BACKEND_OP_PARTITION_TEARDOWN:
			return _XAie_PrivilegeTeardownPart(DevInst);
		case XAIE_BACKEND_OP_GET_RSC_STAT:
			return _XAie_GetRscStatCommon(DevInst, Arg);
		default:
			RC = XAIE_FEATURE_NOT_SUPPORTED;
			break;
	}

	if (RC == XAIE_FEATURE_NOT_SUPPORTED) {
		XAIE_ERROR("Backend doesn't support Op %u.\n", Op);
	}
	return RC;
}

#else

static AieRC XAie_MetalIO_Finish(void *IOInst)
{
	/* no-op */
	(void)IOInst;
	return XAIE_OK;
}

static AieRC XAie_MetalIO_Init(XAie_DevInst *DevInst)
{
	/* no-op */
	(void)DevInst;
	XAIE_ERROR("Driver is not compiled with Libmetal backend "
			"(__AIEMETAL__)\n");
	return XAIE_INVALID_BACKEND;
}

static AieRC XAie_MetalIO_Read32(void *IOInst, u64 RegOff, u32 *Data)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Data;
	return XAIE_ERR;
}

static AieRC XAie_MetalIO_Write32(void *IOInst, u64 RegOff, u32 Data)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Data;

	return XAIE_ERR;
}

static AieRC XAie_MetalIO_MaskWrite32(void *IOInst, u64 RegOff, u32 Mask,
		u32 Data)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Mask;
	(void)Data;

	return XAIE_ERR;
}

static AieRC XAie_MetalIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask, u32 Value,
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

static AieRC XAie_MetalIO_BlockWrite32(void *IOInst, u64 RegOff, u32 *Data,
		u32 Size)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Data;
	(void)Size;

	return XAIE_ERR;
}

static AieRC XAie_MetalIO_BlockSet32(void *IOInst, u64 RegOff, u32 Data,
		u32 Size)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Data;
	(void)Size;

	return XAIE_ERR;
}

static AieRC XAie_MetalIO_RunOp(void *IOInst, XAie_DevInst *DevInst,
		XAie_BackendOpCode Op, void *Arg)
{
	(void)IOInst;
	(void)DevInst;
	(void)Op;
	(void)Arg;
	return XAIE_FEATURE_NOT_SUPPORTED;
}

#endif /* __AIEMETAL__ */

static XAie_MemInst* XAie_MetalMemAllocate(XAie_DevInst *DevInst, u64 Size,
		XAie_MemCacheProp Cache)
{
	(void)DevInst;
	(void)Size;
	(void)Cache;
	return NULL;
}

static AieRC XAie_MetalMemFree(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

static AieRC XAie_MetalMemSyncForCPU(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

static AieRC XAie_MetalMemSyncForDev(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

static AieRC XAie_MetalMemAttach(XAie_MemInst *MemInst, u64 MemHandle)
{
	(void)MemInst;
	(void)MemHandle;
	return XAIE_ERR;
}

static AieRC XAie_MetalMemDetach(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

static AieRC XAie_MetalIO_CmdWrite(void *IOInst, u8 Col, u8 Row, u8 Command,
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

const XAie_Backend MetalBackend =
{
	.Type = XAIE_IO_BACKEND_METAL,
	.Ops.Init = XAie_MetalIO_Init,
	.Ops.Finish = XAie_MetalIO_Finish,
	.Ops.Write32 = XAie_MetalIO_Write32,
	.Ops.Read32 = XAie_MetalIO_Read32,
	.Ops.MaskWrite32 = XAie_MetalIO_MaskWrite32,
	.Ops.MaskPoll = XAie_MetalIO_MaskPoll,
	.Ops.BlockWrite32 = XAie_MetalIO_BlockWrite32,
	.Ops.BlockSet32 = XAie_MetalIO_BlockSet32,
	.Ops.CmdWrite = XAie_MetalIO_CmdWrite,
	.Ops.RunOp = XAie_MetalIO_RunOp,
	.Ops.MemAllocate = XAie_MetalMemAllocate,
	.Ops.MemFree = XAie_MetalMemFree,
	.Ops.MemSyncForCPU = XAie_MetalMemSyncForCPU,
	.Ops.MemSyncForDev = XAie_MetalMemSyncForDev,
	.Ops.MemAttach = XAie_MetalMemAttach,
	.Ops.MemDetach = XAie_MetalMemDetach,
	.Ops.GetTid = XAie_IODummyGetTid,
	.Ops.SubmitTxn = NULL,
};

/** @} */
