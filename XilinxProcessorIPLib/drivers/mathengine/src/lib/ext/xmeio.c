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
* @file xmeio.c
* @{
*
* This file contains the low level layer IO interface
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Hyun  07/12/2018  Initial creation
* 1.1  Hyun  10/11/2018  Initialize the IO device for mem instance
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xmegbl.h"

#include <metal/alloc.h>
#include <metal/device.h>
#include <metal/io.h>

/***************************** Macro Definitions *****************************/

/************************** Variable Definitions *****************************/
typedef struct
{
	u32 RefCnt;			/**< RefCnt. Will not work perfectly */
	struct metal_device *device;	/**< libmetal device */
	struct metal_io_region *io;	/**< libmetal io region */
	u64 io_base;			/**< libmetal io region base */
} XMeIO;

typedef struct XMeIO_Mem
{
	struct metal_io_region *io;	/**< libmetal io region */
	u64 io_base;			/**< libmetal io region base */
} XMeIO_Mem;

static XMeIO IOInst;

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
void XMeIO_Finish(void)
{
	if (!IOInst.RefCnt) {
		XMeLib_print("XMeIO is not ready\n");
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
void XMeIO_Init(void)
{
	struct metal_init_params init_param = METAL_INIT_DEFAULTS;
	int ret;

	if (IOInst.RefCnt) {
		IOInst.RefCnt++;
		return;
	}

	ret = metal_init(&init_param);
	if (ret) {
		XMeLib_print("failed to metal_init %d\n", ret);
		return;
	}

	ret = metal_device_open("platform", "xilinx-mathengine",
			&IOInst.device);
	if (ret) {
		XMeLib_print("failed to metal_device_open\n");
		return;
	}

	IOInst.io = metal_device_io_region(IOInst.device, 0);
	if (!IOInst.io) {
		XMeLib_print("failed to metal_device_io_region\n");
		return;
	}
	IOInst.io_base = metal_io_phys(IOInst.io, 0);
	IOInst.RefCnt++;
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
void *_XMeIO_GetIO(void)
{
	if (!IOInst.RefCnt) {
		XMeLib_print("XMeIO is not ready\n");
		return NULL;
	}

	return (void *)IOInst.io;
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
u32 XMeIO_Read32(u64 Addr)
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
void XMeIO_Read128(u64 Addr, u32 *Data)
{
	u8 Idx;

	/* ME 128-bit no support yet, so do 32-bit reads for now */
	for(Idx = 0U; Idx < 4U; Idx++) {
		Data[Idx] = XMeIO_Read32((u32)Addr + Idx*4U);
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
void XMeIO_Write32(u64 Addr, u32 Data)
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
void XMeIO_Write128(u64 Addr, u32 *Data)
{
	u8 Idx;

	for(Idx = 0U; Idx < 4U; Idx++) {
		XMeIO_Write32((u32)Addr + Idx * 4U, Data[Idx]);
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
void XMeIO_MemFinish(XMeIO_Mem *IO_MemInstPtr)
{
	if (!IO_MemInstPtr) {
		XMeLib_print("XMeIO_Mem pointer is invalid\n");
		return;
	}

	metal_free_memory(IO_MemInstPtr);
	XMeIO_Finish();
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
XMeIO_Mem *XMeIO_MemInit(u8 idx)
{
	XMeIO_Mem *IO_MemInstPtr;
	struct metal_io_region *io;

	XMeIO_Init();

	/* First io region is for register. Start from 1 */
	io = metal_device_io_region(IOInst.device, 1 + idx);
	if (!io)
		return NULL;
	IO_MemInstPtr = metal_allocate_memory(sizeof(*IO_MemInstPtr));
	IO_MemInstPtr->io = io;
	IO_MemInstPtr->io_base = metal_io_phys(io, 0);

	return IO_MemInstPtr;
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
uint64_t XMeIO_MemGetSize(XMeIO_Mem *IO_MemInstPtr)
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
uint64_t XMeIO_MemGetVaddr(XMeIO_Mem *IO_MemInstPtr)
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
uint64_t XMeIO_MemGetPaddr(XMeIO_Mem *IO_MemInstPtr)
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
void XMeIO_MemWrite32(XMeIO_Mem *IO_MemInstPtr, uint64_t Addr, u32 Data)
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
u32 XMeIO_MemRead32(XMeIO_Mem *IO_MemInstPtr, uint64_t Addr)
{
	return  (u32)metal_io_read32(IO_MemInstPtr->io,
			Addr - IO_MemInstPtr->io_base);
}

/** @} */
