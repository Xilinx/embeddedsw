/******************************************************************************
* (c) Copyright 2018 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information
* of Xilinx, Inc. and is protected under U.S. and
* international copyright and other intellectual property
* laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any
* rights to the materials distributed herewith. Except as
* otherwise provided in a valid license issued to you by
* Xilinx, and to the maximum extent permitted by applicable
* law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
* WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
* AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
* BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
* INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
* (2) Xilinx shall not be liable (whether in contract or tort,
* including negligence, or under any other theory of
* liability) for any loss or damage of any kind or nature
* related to, arising under or in connection with these
* materials, including for any direct, or any indirect,
* special, incidental, or consequential loss or damage
* (including loss of data, profits, goodwill, or any type of
* loss or damage suffered as a result of any action brought
* by a third party) even if such damage or loss was
* reasonably foreseeable or Xilinx had been advised of the
* possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-
* safe, or for use in any application requiring fail-safe
* performance, such as life-support or safety devices or
* systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any
* other applications that could lead to death, personal
* injury, or severe property or environmental damage
* (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and
* liability of any use of Xilinx products in Critical
* Applications, subject only to applicable laws and
* regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
* PART OF THIS FILE AT ALL TIMES.
******************************************************************************/

/*****************************************************************************/
/**
* @file xaieio.c
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
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xaiegbl.h"

#include <metal/alloc.h>
#include <metal/device.h>
#include <metal/io.h>
#include <metal/irq.h>

#include <metal/list.h>
#include <metal/mutex.h>
#include <metal/shmem.h>
#include <metal/shmem-provider.h>
#include <metal/utilities.h>

/***************************** Macro Definitions *****************************/
#define SHM_NUM_ULONG	8					/**< number of ulong for bitmap */
#define SHM_MAX_IDS	(sizeof(unsigned long) * SHM_NUM_ULONG)	/**< max number of IDs = 64 * 16 = 512 */

/************************** Variable Definitions *****************************/
typedef struct XAieIO_IntrIsr
{
	int (*handler) (void *data);	/**< Interrupt handler */
	void *data;			/**< Data to be used by the handler */
} XAieIO_IntrIsr;

typedef struct
{
	u32 RefCnt;			/**< RefCnt. Will not work perfectly */
	struct metal_device *device;	/**< libmetal device */
	struct metal_device *npi;	/**< device for aie npi mapping */
	struct metal_io_region *io;	/**< libmetal io region */
	u64 io_base;			/**< libmetal io region base */
	struct metal_io_region *npi_io;	/**< io region for aie npi */
	u64 npi_io_base;		/**< io region base for aie npi */
	unsigned long shm_ids[SHM_NUM_ULONG];	/**< bitmap for shm name space */
	XAieIO_IntrIsr isr;		/**< Interrupt server routine */
} XAieIO;

typedef struct XAieIO_Mem
{
	struct metal_io_region *io;	/**< libmetal io region */
	u64 io_base;			/**< libmetal io region base */
	struct metal_generic_shmem *shm;/**< attached metal shm memory */
	struct metal_scatter_list *sg;	/**< shm sg list */
	unsigned int id;		/**< shm id. only for allocated one */
} XAieIO_Mem;

static XAieIO IOInst;

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This is the memory IO function to free the global IO instance
*
* @param	None.
*
* @return	None.
*
* @note		The global IO instance is a singleton and freed when
* the reference count reaches a zero.
*
*******************************************************************************/
void XAieIO_Finish(void)
{
	if (!IOInst.RefCnt) {
		XAieLib_print("XAieIO is not ready\n");
		return;
	}

	IOInst.RefCnt--;
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
* to initialize just increments the reference count.
*
*******************************************************************************/
void XAieIO_Init(void)
{
	struct metal_init_params init_param = METAL_INIT_DEFAULTS;
	int ret;

	if (IOInst.RefCnt) {
		IOInst.RefCnt++;
		return;
	}

	ret = metal_init(&init_param);
	if (ret) {
		XAieLib_print("failed to metal_init %d\n", ret);
		return;
	}

	ret = metal_device_open("platform", "xilinx-aiengine",
			&IOInst.device);
	if (ret) {
		XAieLib_print("failed to metal_device_open\n");
		return;
	}

	IOInst.io = metal_device_io_region(IOInst.device, 0);
	if (!IOInst.io) {
		XAieLib_print("failed to metal_device_io_region\n");
		return;
	}
	IOInst.io_base = metal_io_phys(IOInst.io, 0);
	IOInst.RefCnt++;

	ret = metal_device_open("platform", "f70a0000.aie-npi", &IOInst.npi);
	if (ret) {
		return;
	}

	IOInst.npi_io = metal_device_io_region(IOInst.npi, 0);
	if (!IOInst.npi_io) {
		return;
	}
	IOInst.npi_io_base = metal_io_phys(IOInst.npi_io, 0);
	XAieLib_print("AIE NPI space is mapped\n");
}

/*****************************************************************************/
/**
*
* This is the memory IO function to get a low level IO instance.
*
* @param	None.
*
* @return	A void pointer that points to the low level IO instance.
*
* @note		This function is for internal use only, and not meant to be
* used by applications.
*
*******************************************************************************/
void *_XAieIO_GetIO(void)
{
	if (!IOInst.RefCnt) {
		XAieLib_print("XAieIO is not ready\n");
		return NULL;
	}

	return (void *)IOInst.io;
}

/*****************************************************************************/
/**
*
* This function unregisters the interrupt handler for given irq offset.
*
* @param	Offset: The value should one of 1 - 3. Current it's not used.
*
* @return	None.
*
* @note		The driver doesn't support all 4 interrupts yet, so the offset
* is not used.
*
*******************************************************************************/
void XAieIO_IntrUnregisterIsr(int Offset)
{
	metal_irq_disable((int)IOInst.device->irq_info);
	metal_irq_unregister((int)IOInst.device->irq_info);
	IOInst.isr.handler = NULL;
	IOInst.isr.data = NULL;
	XAieIO_Finish();
}

/*****************************************************************************/
/**
*
* This is an internal wrapper that calls the user registered callback in
* the libmetal irq handler.
*
* @param	irq: the irq number
* @param	data: the data given at registration.
*
* @return	METAL_IRQ_HANDLED for success.
*
* @note		None.
*
*******************************************************************************/
static int XAieIO_IntrHandler(int irq, void *data)
{
	if (IOInst.isr.handler != data) {
		/* Return as handled to avoid interrupt overflow */
		return METAL_IRQ_HANDLED;
	}

	IOInst.isr.handler(IOInst.isr.data);

	return METAL_IRQ_HANDLED;
}

/*****************************************************************************/
/**
*
* This function registers the interrupt handler for given irq offset.
*
* @param	Offset: The value should one of 1 - 3. Current it's not used.
* @param	Handler: the callback to be called upon interrupt.
* @param	Data: the data to be used with the handler.
*
* @return	XAIELIB_SUCCESS for success, XAIELIB_FAILURE otherwise.
*
* @note		The driver doesn't support all 4 interrupts yet, so the offset
* is not used. The handler is registerd as global one, so it isn't allowed to
* register more than one handlers.
*
*******************************************************************************/
int XAieIO_IntrRegisterIsr(int Offset, int (*Handler) (void *Data), void *Data)
{
	int ret;

	XAieIO_Init();

	/* Only one interrupt is supported at the moment */
	if (Offset == 0 || IOInst.isr.handler) {
		XAieIO_Finish();
		return XAIELIB_FAILURE;
	}

	/*
	 * Register the handler with global instance. This limits that only one
	 * handler can be registered.
	 */
	IOInst.isr.handler = Handler;
	IOInst.isr.data = Data;
	ret = metal_irq_register((int)IOInst.device->irq_info,
			XAieIO_IntrHandler, Handler);
	if (ret) {
		XAieIO_Finish();
		return XAIELIB_FAILURE;
	}

	metal_irq_enable((int)IOInst.device->irq_info);

	return XAIELIB_SUCCESS;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to read 32bit data from the specified address.
*
* @param	Addr: Address to read from.
*
* @return	32-bit read value.
*
* @note		None.
*
*******************************************************************************/
u32 XAieIO_Read32(u64 Addr)
{
	return metal_io_read32(IOInst.io, Addr - IOInst.io_base);
}

/*****************************************************************************/
/**
*
* This is the memory IO function to read 128b data from the specified address.
*
* @param	Addr: Address to read from.
* @param	Data: Pointer to the 128-bit buffer to store the read data.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieIO_Read128(u64 Addr, u32 *Data)
{
	u8 Idx;

	/* AIE 128-bit no support yet, so do 32-bit reads for now */
	for(Idx = 0U; Idx < 4U; Idx++) {
		Data[Idx] = XAieIO_Read32((u32)Addr + Idx*4U);
	}
}

/*****************************************************************************/
/**
*
* This is the memory IO function to write 32bit data to the specified address.
*
* @param	Addr: Address to write to.
* @param	Data: 32-bit data to be written.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieIO_Write32(u64 Addr, u32 Data)
{
	metal_io_write32(IOInst.io, Addr - IOInst.io_base, Data);
}

/*****************************************************************************/
/**
*
* This is the memory IO function to write 128bit data to the specified address.
*
* @param	Addr: Address to write to.
* @param	Data: Pointer to the 128-bit data buffer.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieIO_Write128(u64 Addr, u32 *Data)
{
	u8 Idx;

	for(Idx = 0U; Idx < 4U; Idx++) {
		XAieIO_Write32((u32)Addr + Idx * 4U, Data[Idx]);
	}
}

/*****************************************************************************/
/**
*
* This is the IO memory function to free the memory instance
*
* @param	IO_MemInstPtr: IO Memory instance pointer.
*
* @return	None.
*
* @note		@IO_MemInstPtr is freed and invalid after this function.
*
*******************************************************************************/
void XAieIO_MemFinish(XAieIO_Mem *IO_MemInstPtr)
{
	if (!IO_MemInstPtr) {
		XAieLib_print("XAieIO_Mem pointer is invalid\n");
		return;
	}

	metal_free_memory(IO_MemInstPtr);
	XAieIO_Finish();
}

/*****************************************************************************/
/**
*
* This is the IO memory function to initialize the IO memory instance.
*
* @param	idx: Index of the memory to initialize.
*
* @return	Pointer to the initialized IO memory instance.
*
* @note		None.
*
*******************************************************************************/
XAieIO_Mem *XAieIO_MemInit(u8 idx)
{
	XAieIO_Mem *IO_MemInstPtr;
	struct metal_io_region *io;

	XAieIO_Init();

	/* First io region is for register. Start from 1 */
	io = metal_device_io_region(IOInst.device, 1 + idx);
	if (!io)
		return NULL;

	IO_MemInstPtr = metal_allocate_memory(sizeof(*IO_MemInstPtr));
	IO_MemInstPtr->io = io;
	IO_MemInstPtr->io_base = metal_io_phys(io, 0);
	/* Set NULL to differentiate this memory */
	IO_MemInstPtr->shm = NULL;

	return IO_MemInstPtr;
}

/*****************************************************************************/
/**
*
* This is the IO memory function to detach the memory from device
*
* @param	IO_MemInstPtr: IO Memory instance pointer.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieIO_MemDetach(XAieIO_Mem *IO_MemInstPtr)
{
	metal_shm_detach(IO_MemInstPtr->shm, IOInst.device);
	metal_free_memory(IO_MemInstPtr->shm);
	metal_free_memory(IO_MemInstPtr);
	XAieIO_Finish();
}

/*****************************************************************************/
/**
*
* This is the IO memory function to attach the external memory to device
*
* @param	Vaddr: Optional. Vaddr of the memory
* @param	Paddr: Optional. Paddr of the memory
* @param	Size: Required. Size of the memory
* @param	MemHandle: Required. Handle of the memory. For linux, dmabuf fd
*
* @return	Pointer to the attached IO memory instance.
*
* @note		None.
*
*******************************************************************************/
XAieIO_Mem *XAieIO_MemAttach(uint64_t Vaddr, uint64_t Paddr, uint64_t Size,
		uint64_t MemHandle)
{
	XAieIO_Mem *IO_MemInstPtr;

	XAieIO_Init();

	IO_MemInstPtr = metal_allocate_memory(sizeof(*IO_MemInstPtr));
	if (!IO_MemInstPtr) {
		goto io_finish;
	}

	IO_MemInstPtr->shm = metal_allocate_memory(sizeof(*IO_MemInstPtr->shm));
	if (!IO_MemInstPtr->shm) {
		goto free_mem_inst;
	}
	metal_list_init(&IO_MemInstPtr->shm->refs);
	metal_mutex_init(&IO_MemInstPtr->shm->lock);

	/*
	 * FIXME: dummy allocate to retreive the dmabuf ops. This can be
	 * removed if libemtal allows to attach the dmabuf ops to shm object
	 * allocated outside.
	 */
	IO_MemInstPtr->shm->provider = metal_shmem_get_provider("ion.reserved");
	IO_MemInstPtr->shm->provider->alloc(IO_MemInstPtr->shm->provider,
			IO_MemInstPtr->shm, 1);
	IO_MemInstPtr->shm->provider->free(IO_MemInstPtr->shm->provider,
			IO_MemInstPtr->shm);

	/* Expect the dmabuf fd as a handle*/
	IO_MemInstPtr->shm->id = MemHandle;
	IO_MemInstPtr->shm->size = Size;
	IO_MemInstPtr->sg = metal_shm_attach(IO_MemInstPtr->shm, IOInst.device,
			METAL_SHM_DIR_DEV_RW);
	if (!IO_MemInstPtr->sg) {
		goto free_shm;
	}

	/* This can be cross-checked if it matches as given argument */
	IO_MemInstPtr->io_base = metal_io_phys(IO_MemInstPtr->sg->ios, 0);
	IO_MemInstPtr->io = IO_MemInstPtr->sg->ios;

	return IO_MemInstPtr;

free_shm:
	metal_free_memory(IO_MemInstPtr->shm);
free_mem_inst:
	metal_free_memory(IO_MemInstPtr);
io_finish:
	XAieIO_Finish();
	return NULL;
}

/*****************************************************************************/
/**
*
* This is the IO memory function to free the memory
*
* @param	IO_MemInstPtr: IO Memory instance pointer.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieIO_MemFree(XAieIO_Mem *IO_MemInstPtr)
{
	metal_bitmap_clear_bit(IOInst.shm_ids, IO_MemInstPtr->id);
	metal_shm_detach(IO_MemInstPtr->shm, IOInst.device);
	metal_shmem_close(IO_MemInstPtr->shm);
	metal_free_memory(IO_MemInstPtr);
	XAieIO_Finish();
}

/*****************************************************************************/
/**
*
* This is the IO memory function to allocate a memory
*
* @param	Size: Size of the memory
* @param	Attr: Any of XAIELIB_MEM_ATTR_*
*
* @return	Pointer to the allocated IO memory instance.
*
* @note		None.
*
*******************************************************************************/
XAieIO_Mem *XAieIO_MemAllocate(uint64_t Size, u32 Attr)
{
	XAieIO_Mem *IO_MemInstPtr;
	char shm_name[32];
	unsigned int id, flag = 0;
	int ret;

	id = metal_bitmap_next_clear_bit(IOInst.shm_ids, 0, SHM_MAX_IDS);
	if (id >= SHM_MAX_IDS) {
		goto err_out;
	}

	XAieIO_Init();

	IO_MemInstPtr = metal_allocate_memory(sizeof(*IO_MemInstPtr));
	if (!IO_MemInstPtr) {
		goto io_finish;
	}

	/* The name, ion.reserved, and snprintf() are linux specific */
	snprintf(shm_name, sizeof(shm_name), "ion.reserved/shm%d", id);
	if (!(Attr & XAIELIB_MEM_ATTR_CACHE)) {
		flag = METAL_SHM_NOTCACHED;
	}
	ret = metal_shmem_open(shm_name, Size, flag, &IO_MemInstPtr->shm);
	if (ret) {
		goto free_mem_inst;
	}

	IO_MemInstPtr->sg = metal_shm_attach(IO_MemInstPtr->shm, IOInst.device,
			METAL_SHM_DIR_DEV_RW);
	if (!IO_MemInstPtr->sg) {
		goto shm_close;
	}

	IO_MemInstPtr->io = IO_MemInstPtr->sg->ios;
	IO_MemInstPtr->io_base = metal_io_phys(IO_MemInstPtr->sg->ios, 0);
	IO_MemInstPtr->id = id;
	metal_bitmap_set_bit(IOInst.shm_ids, id);

	return IO_MemInstPtr;

shm_close:
	metal_shmem_close(IO_MemInstPtr->shm);
free_mem_inst:
	metal_free_memory(IO_MemInstPtr);
io_finish:
	XAieIO_Finish();
err_out:
	return NULL;
}

/*****************************************************************************/
/**
*
* This is the IO memory function to sync the memory for CPU
*
* @param	IO_MemInstPtr: IO Memory instance pointer.
*
* @return	None.
*
* @note		This only works with imported or allocated memory. This doesn't
* do anything with memory from the own memory pool, ex XAieIO_MemInit().
*
*******************************************************************************/
u8 XAieIO_MemSyncForCPU(XAieIO_Mem *IO_MemInstPtr)
{
	int ret;

	if (!IO_MemInstPtr->shm) {
		return XAIELIB_SUCCESS;
	}

	ret = metal_shm_sync_for_cpu(IO_MemInstPtr->shm, METAL_SHM_DIR_DEV_RW);
	if (ret) {
		return XAIELIB_FAILURE;
	}
	return XAIELIB_SUCCESS;
}

/*****************************************************************************/
/**
*
* This is the IO memory function to sync the memory for device
*
* @param	IO_MemInstPtr: IO Memory instance pointer.
*
* @return	None.
*
* @note		This only works with imported or allocated memory. This doesn't
* do anything with memory from the own memory pool, ex XAieIO_MemInit().
*
*******************************************************************************/
u8 XAieIO_MemSyncForDev(XAieIO_Mem *IO_MemInstPtr)
{
	int ret;

	if (!IO_MemInstPtr->shm) {
		return XAIELIB_SUCCESS;
	}

	ret = metal_shm_sync_for_device(IO_MemInstPtr->shm, IOInst.device,
			METAL_SHM_DIR_DEV_RW);
	if (ret) {
		return XAIELIB_FAILURE;
	}
	return XAIELIB_SUCCESS;
}

/*****************************************************************************/
/**
*
* This is the IO memory function to return the size of the memory instance
*
* @param	IO_MemInstPtr: IO Memory instance pointer.
*
* @return	size of the memory instance
*
* @note		None.
*
*******************************************************************************/
uint64_t XAieIO_MemGetSize(XAieIO_Mem *IO_MemInstPtr)
{
	return (uint64_t)metal_io_region_size(IO_MemInstPtr->io);
}

/*****************************************************************************/
/**
*
* This is the IO memory function to return the mapped virtual address
* of the memory instance
*
* @param	IO_MemInstPtr: IO Memory instance pointer.
*
* @return	Mapped virtual address.
*
* @note		None.
*
*******************************************************************************/
uint64_t XAieIO_MemGetVaddr(XAieIO_Mem *IO_MemInstPtr)
{
	return (uint64_t)metal_io_virt(IO_MemInstPtr->io, 0);
}

/*****************************************************************************/
/**
*
* This is the IO memory function to return the physical address
* of the memory instance
*
* @param	IO_MemInstPtr: IO Memory instance pointer.
*
* @return	Physical address of the memory instance
*
* @note		None.
*
*******************************************************************************/
uint64_t XAieIO_MemGetPaddr(XAieIO_Mem *IO_MemInstPtr)
{
	return (uint64_t)metal_io_phys(IO_MemInstPtr->io, 0);
}

/*****************************************************************************/
/**
*
* This is the IO memory function to write to the physical address.
*
* @param	IO_MemInstPtr: IO Memory instance pointer.
* @param	Addr: Absolute physical address to write.
* @param	Data: A 32 bit data to write.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieIO_MemWrite32(XAieIO_Mem *IO_MemInstPtr, uint64_t Addr, u32 Data)
{
	metal_io_write32(IO_MemInstPtr->io,
			Addr - IO_MemInstPtr->io_base, Data);
}

/*****************************************************************************/
/**
*
* This is the IO memory function to read from the physical address.
*
* @param	IO_MemInstPtr: IO Memory instance pointer.
* @param	Addr: Absolute physical address to read from.
*
* @return	A read 32 bit data.
*
* @note		None.
*
*******************************************************************************/
u32 XAieIO_MemRead32(XAieIO_Mem *IO_MemInstPtr, uint64_t Addr)
{
	return  (u32)metal_io_read32(IO_MemInstPtr->io,
			Addr - IO_MemInstPtr->io_base);
}

/*****************************************************************************/
/**
*
* This is the NPI IO function to read 32bit data from the specified address.
*
* @param	Addr: Address to read from.
*
* @return	32-bit read value.
*
* @note		This only work if NPI is accessble.
*
*******************************************************************************/
u32 XAieIO_NPIRead32(u64 Addr)
{
	if (!IOInst.npi_io)
		return 0;
	return metal_io_read32(IOInst.npi_io, Addr - IOInst.npi_io_base);
}

/*****************************************************************************/
/**
*
* This is the NPI IO function to write 32bit data to the specified address.
*
* @param	Addr: Address to write to.
* @param	Data: 32-bit data to be written.
*
* @return	None.
*
* @note		This only work if NPI is accessble.
*
*******************************************************************************/
void XAieIO_NPIWrite32(u64 Addr, u32 Data)
{
	if (!IOInst.npi_io)
		return;
	metal_io_write32(IOInst.npi_io, Addr - IOInst.npi_io_base, Data);
}

/** @} */
