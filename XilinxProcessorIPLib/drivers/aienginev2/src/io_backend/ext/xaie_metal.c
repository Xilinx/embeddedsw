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
#include "xaie_metal.h"
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

typedef struct XAie_MetalMemInst {
	struct metal_io_region *io;	/**< libmetal io region */
	u64 io_base;			/**< libmetal io region base */
	struct metal_generic_shmem *shm;/**< attached metal shm memory */
	struct metal_scatter_list *sg;  /**< shm sg list */
	unsigned int id;                /**< shm id. only for allocated one */
	XAie_MetalIO *IOInst;		/**< Pointer to the IO Inst */
} XAie_MetalMemInst;

#endif /* __AIEMETAL__ */

/************************** Variable Definitions *****************************/
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
};

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
AieRC XAie_MetalIO_Finish(void *IOInst)
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
AieRC XAie_MetalIO_Init(XAie_DevInst *DevInst)
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
*
* @return	32-bit read value.
*
* @note		Internal only.
*
*******************************************************************************/
u32 XAie_MetalIO_Read32(void *IOInst, u64 RegOff)
{
	XAie_MetalIO *MetalIOInst = (XAie_MetalIO *)IOInst;

	return metal_io_read32(MetalIOInst->io, RegOff);
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
void XAie_MetalIO_Write32(void *IOInst, u64 RegOff, u32 Data)
{
	XAie_MetalIO *MetalIOInst = (XAie_MetalIO *)IOInst;

	metal_io_write32(MetalIOInst->io, RegOff, Data);
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
void XAie_MetalIO_MaskWrite32(void *IOInst, u64 RegOff, u32 Mask, u32 Data)
{
	u32 RegVal;

	RegVal = XAie_MetalIO_Read32(IOInst, RegOff);
	RegVal &= ~Mask;
	RegVal |= Data;
	XAie_MetalIO_Write32(IOInst, RegOff, RegVal);
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
u32 XAie_MetalIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask, u32 Value,
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
		if((XAie_MetalIO_Read32(IOInst, RegOff) & Mask) == Value) {
			Ret = XAIE_SUCCESS;
			break;
		}
		usleep(MinTimeOutUs);
		Count--;
	}

	/* Check for the break from timed-out loop */
	if((Ret == XAIE_FAILURE) &&
			((XAie_MetalIO_Read32(IOInst, RegOff) & Mask) ==
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
void XAie_MetalIO_BlockWrite32(void *IOInst, u64 RegOff, u32 *Data, u32 Size)
{
	for(u32 i = 0; i < Size; i++) {
		XAie_MetalIO_Write32(IOInst, RegOff + i * 4U, *Data);
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
void XAie_MetalIO_BlockSet32(void *IOInst, u64 RegOff, u32 Data, u32 Size)
{
	for(u32 i = 0; i < Size; i++) {
		XAie_MetalIO_Write32(IOInst, RegOff + i * 4U, Data);
	}
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
AieRC XAie_MetalIO_RunOp(void *IOInst, XAie_DevInst *DevInst,
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
		{
			XAIE_DBG("Backend doesn't support Op %u.\n", Op);
			return XAIE_FEATURE_NOT_SUPPORTED;
		}
		default:
			RC = XAIE_FEATURE_NOT_SUPPORTED;
			break;
	}

	if (RC == XAIE_FEATURE_NOT_SUPPORTED) {
		XAIE_ERROR("Backend doesn't support Op %u.\n", Op);
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
* @param	Cache: Value from XAie_MemCacheProp enum
*
* @return	Pointer to the allocated memory instance.
*
* @note		Internal only.
*
*******************************************************************************/
XAie_MemInst* XAie_MetalMemAllocate(XAie_DevInst *DevInst, u64 Size,
		XAie_MemCacheProp Cache)
{
	char shm_name[32U];
	unsigned int id;
	int ret;
	XAie_MemInst *MemInst;
	XAie_MetalMemInst *MetalMemInst;
	XAie_MetalIO *MetalIOInst = (XAie_MetalIO *)DevInst->IOInst;

	id = metal_bitmap_next_clear_bit(MetalIOInst->shm_ids, 0, SHM_MAX_IDS);
	if(id >= SHM_MAX_IDS) {
		goto err_out;
	}

	MetalMemInst = metal_allocate_memory(sizeof(*MetalMemInst));
	if(MetalMemInst == NULL) {
		XAIE_ERROR("memory allocation failed\n");
		goto err_out;
	}

	MemInst = metal_allocate_memory(sizeof(*MemInst));
	if(MemInst == NULL) {
		XAIE_ERROR("memory allocation failed\n");
		goto free_metal_mem_inst;
	}

	/* The name, ion.reserved, and snprintf() are linux specific */
	snprintf(shm_name, sizeof(shm_name), "ion.reserved/shm%d", id);

	ret = metal_shmem_open(shm_name, Size, Cache, &MetalMemInst->shm);
	if(ret) {
		XAIE_ERROR("failed to open shared memory region\n");
		goto free_all_mem;
	}

	MetalMemInst->sg = metal_shm_attach(MetalMemInst->shm,
			MetalIOInst->device, METAL_SHM_DIR_DEV_RW);
	if(MetalMemInst->sg == NULL) {
		XAIE_ERROR("Failed to attach shmem to device\n");
		goto close_shmem;
	}

	MetalMemInst->io = MetalMemInst->sg->ios;
	MetalMemInst->io_base = metal_io_phys(MetalMemInst->sg->ios, 0U);
	MetalMemInst->id = id;
	MetalMemInst->IOInst = MetalIOInst;
	metal_bitmap_set_bit(MetalIOInst->shm_ids, id);

	MemInst->BackendHandle = (void *)MetalMemInst;
	MemInst->DevInst = DevInst;
	MemInst->Cache = Cache;
	MemInst->Size = Size;
	MemInst->DevAddr = (u64)metal_io_phys(MetalMemInst->io, 0U);
	MemInst->VAddr = (void *)metal_io_virt(MetalMemInst->io, 0U);

	return MemInst;

close_shmem:
	metal_shmem_close(MetalMemInst->shm);
free_all_mem:
	metal_free_memory(MemInst);
free_metal_mem_inst:
	metal_free_memory(MetalMemInst);
err_out:
	return NULL;
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
AieRC XAie_MetalMemFree(XAie_MemInst *MemInst)
{
	XAie_MetalMemInst *MetalMemInst = (XAie_MetalMemInst *)
		MemInst->BackendHandle;
	metal_bitmap_clear_bit(MetalMemInst->IOInst->shm_ids, MetalMemInst->id);
	metal_shm_detach(MetalMemInst->shm, MetalMemInst->IOInst->device);
	metal_shmem_close(MetalMemInst->shm);
	metal_free_memory(MetalMemInst);
	metal_free_memory(MemInst);

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
AieRC XAie_MetalMemSyncForCPU(XAie_MemInst *MemInst)
{
	int ret;
	XAie_MetalMemInst *MetalMemInst = (XAie_MetalMemInst *)
		MemInst->BackendHandle;

	if(MetalMemInst->shm == NULL) {
		XAIE_ERROR("Invalid memory instance\n");
		return XAIE_ERR;
	}

	ret = metal_shm_sync_for_cpu(MetalMemInst->shm, METAL_SHM_DIR_DEV_RW);
	if(ret) {
		XAIE_ERROR("Failed to sync for cpu\n");
		return XAIE_ERR;
	}

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
AieRC XAie_MetalMemSyncForDev(XAie_MemInst *MemInst)
{
	int ret;
	XAie_MetalMemInst *MetalMemInst = (XAie_MetalMemInst *)
		MemInst->BackendHandle;

	if(MetalMemInst->shm == NULL) {
		XAIE_ERROR("Invalid memory instance\n");
		return XAIE_ERR;
	}

	ret = metal_shm_sync_for_device(MetalMemInst->shm,
			MetalMemInst->IOInst->device, METAL_SHM_DIR_DEV_RW);
	if(ret) {
		XAIE_ERROR("Failed to sync for device\n");
		return XAIE_ERR;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory function to attach the external memory to device
*
* @param	MemInst: Memory instance pointer.
* @param	MemHandle: dmabuf fd
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*
*******************************************************************************/
AieRC XAie_MetalMemAttach(XAie_MemInst *MemInst, u64 MemHandle)
{
	XAie_MetalMemInst *MetalMemInst;
	XAie_MetalIO *MetalIOInst = (XAie_MetalIO *)MemInst->DevInst->IOInst;

	MetalMemInst = metal_allocate_memory(sizeof(*MetalMemInst));
	if(MetalMemInst == NULL) {
		XAIE_ERROR("memory attach failed, memory allocation failed\n");
		return XAIE_ERR;
	}

	MetalMemInst->shm = metal_allocate_memory(sizeof(*MetalMemInst->shm));
	if (MetalMemInst->shm == NULL) {
		XAIE_ERROR("memory attach failed, memory allocation failed\n");
		metal_free_memory(MetalMemInst->shm);
		metal_free_memory(MetalMemInst);
		return XAIE_ERR;
	}

	metal_list_init(&MetalMemInst->shm->refs);
	metal_mutex_init(&MetalMemInst->shm->lock);

	/*
	 * FIXME: dummy allocate to retreive the dmabuf ops. This can be
	 * removed if libemtal allows to attach the dmabuf ops to shm object
	 * allocated outside.
	 */
	MetalMemInst->shm->provider = metal_shmem_get_provider("ion.reserved");
	MetalMemInst->shm->provider->alloc(MetalMemInst->shm->provider,
			MetalMemInst->shm, 1);
	MetalMemInst->shm->provider->free(MetalMemInst->shm->provider,
			MetalMemInst->shm);

	/* Expect the dmabuf fd as a handle*/
	MetalMemInst->shm->id = MemHandle;
	MetalMemInst->shm->size = MemInst->Size;
	MetalMemInst->sg = metal_shm_attach(MetalMemInst->shm,
			MetalIOInst->device, METAL_SHM_DIR_DEV_RW);
	if (MetalMemInst->sg == NULL) {
		XAIE_ERROR("libmetal memory attach failed\n");
		metal_free_memory(MetalMemInst->shm);
		metal_free_memory(MetalMemInst);
		return XAIE_ERR;
	}

	/* This can be cross-checked if it matches as given argument */
	MetalMemInst->io_base = metal_io_phys(MetalMemInst->sg->ios, 0);
	MetalMemInst->io = MetalMemInst->sg->ios;
	MetalMemInst->IOInst = MetalIOInst;

	MemInst->BackendHandle = (void *)MetalMemInst;
	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory function to detach the memory from device
*
* @param	MemInst: Memory instance pointer.
*
* @return	XAIE_OK for success
*
* @note		None.
*
*******************************************************************************/
AieRC XAie_MetalMemDetach(XAie_MemInst *MemInst)
{
	XAie_MetalMemInst *MetalMemInst = (XAie_MetalMemInst *)
		MemInst->BackendHandle;

	if(MetalMemInst->shm == NULL || MetalMemInst->IOInst == NULL ||
		MetalMemInst->IOInst->device == NULL) {
		XAIE_ERROR("mem detach failed, invalid memory instance\n");
		return XAIE_INVALID_ARGS;
	}

	metal_shm_detach(MetalMemInst->shm, MetalMemInst->IOInst->device);
	metal_free_memory(MetalMemInst->shm);
	metal_free_memory(MetalMemInst);

	return XAIE_OK;
}

#else

AieRC XAie_MetalIO_Finish(void *IOInst)
{
	/* no-op */
	(void)IOInst;
	return XAIE_OK;
}

AieRC XAie_MetalIO_Init(XAie_DevInst *DevInst)
{
	/* no-op */
	(void)DevInst;
	XAIE_ERROR("Driver is not compiled with Libmetal backend "
			"(__AIEMETAL__)\n");
	return XAIE_INVALID_BACKEND;
}

u32 XAie_MetalIO_Read32(void *IOInst, u64 RegOff)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	return 0;
}

void XAie_MetalIO_Write32(void *IOInst, u64 RegOff, u32 Data)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Data;
}

void XAie_MetalIO_MaskWrite32(void *IOInst, u64 RegOff, u32 Mask, u32 Data)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Mask;
	(void)Data;
}

u32 XAie_MetalIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask, u32 Value,
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

void XAie_MetalIO_BlockWrite32(void *IOInst, u64 RegOff, u32 *Data, u32 Size)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Data;
	(void)Size;
}

void XAie_MetalIO_BlockSet32(void *IOInst, u64 RegOff, u32 Data, u32 Size)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Data;
	(void)Size;
}

AieRC XAie_MetalIO_RunOp(void *IOInst, XAie_DevInst *DevInst,
		XAie_BackendOpCode Op, void *Arg)
{
	(void)IOInst;
	(void)DevInst;
	(void)Op;
	(void)Arg;
	return XAIE_FEATURE_NOT_SUPPORTED;
}

XAie_MemInst* XAie_MetalMemAllocate(XAie_DevInst *DevInst, u64 Size,
		XAie_MemCacheProp Cache)
{
	(void)DevInst;
	(void)Size;
	(void)Cache;
	return NULL;
}

AieRC XAie_MetalMemFree(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

AieRC XAie_MetalMemSyncForCPU(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

AieRC XAie_MetalMemSyncForDev(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

AieRC XAie_MetalMemAttach(XAie_MemInst *MemInst, u64 MemHandle)
{
	(void)MemInst;
	(void)MemHandle;
	return XAIE_ERR;
}

AieRC XAie_MetalMemDetach(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

#endif /* __AIEMETAL__ */

void XAie_MetalIO_CmdWrite(void *IOInst, u8 Col, u8 Row, u8 Command, u32 CmdWd0,
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

/** @} */
