/******************************************************************************
*
* Copyright (C) 2018-2019 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/

/*****************************************************************************/
/**
* @file xaielib.c
* @{
*
* This file contains the low level layer interface of the AIE driver with the
* definitions for the memory write and read operations.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Naresh  03/23/2018  Initial creation
* 1.1  Naresh  05/23/2018  Added bare-metal BSP support
* 1.2  Naresh  06/18/2018  Updated code as per standalone driver framework
* 1.3  Naresh  07/11/2018  Updated copyright info
* 1.4  Hyun    10/10/2018  Added the mask write API
* 1.5  Hyun    10/11/2018  Don't include the xaieio header for sim build
* 1.6  Hyun    10/16/2018  Added the baremetal compile switch everywhere
*                          it's needed
* 1.7  Hyun    11/14/2018  Move platform dependent code to xaielib.c
* 1.8  Nishad  12/05/2018  Renamed ME attributes to AIE
* 1.9  Hyun    01/08/2019  Implement 128bit IO operations for baremetal
* 2.0  Hyun    01/08/2019  Add XAieLib_MaskPoll()
* 2.1  Hyun    04/05/2019  NPI support for simulation
* 2.2  Nishad  05/16/2019  Fix deallocation of pointer not on heap MISRA-c
* 				mandatory violation
* 2.3  Nishad  08/07/2019  Remove OS specific gaurd from XAieLib_usleep API
* 2.4  Hyun    09/13/2019  Use the simulation elf loader function
* 2.5  Hyun    09/13/2019  Use XAieSim_LoadElfMem()
* 2.6  Tejus   10/14/2019  Enable assertion for linux and simulation
* </pre>
*
******************************************************************************/
#include "xaielib.h"

#ifdef __AIESIM__ /* AIE simulator */

#include <assert.h>
#include "xaiesim.h"

#elif defined __AIEBAREMTL__ /* Bare-metal application */

#include "xil_types.h"
#include "xil_io.h"
#include "xil_assert.h"
#include "xil_printf.h"
#include "xil_cache.h"
#include "xstatus.h"
#include "sleep.h"

#include <stdlib.h>

#else /* Non-baremetal application, ex Linux */

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#include "xaieio.h"
#include "xaietile_proc.h"

#include "xaiesim.h"
#include "xaiesim_elfload.h"

#endif

/***************************** Include Files *********************************/

/***************************** Macro Definitions *****************************/
/*
 * Default heap size in baremetal bsp is 0x2000. Use half of it unless
 * specified.
 */
#ifndef XAIELIB_BAREMTL_DEF_MEM_SIZE
#define XAIELIB_BAREMTL_DEF_MEM_SIZE	(0x1000)
#endif

/* Address should be aligned at 128 bit / 16 bytes */
#define XAIELIB_SHIM_MEM_ALIGN		16

/************************** Variable Definitions *****************************/
typedef struct XAieLib_MemInst
{
	u64 Size;	/**< Size */
	u64 Vaddr;	/**< Virtual address */
	u64 Paddr;	/**< Device / physical address */
	void *Platform;	/**< Platform specific data */
} XAieLib_MemInst;

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This asserts if the condition doesn't meet.
*
* @param	Cond: Condition to meet. Should be 0 or 1.
*
* @return	0
*
* @note		None.
*
*******************************************************************************/
u32 XAieLib_AssertNonvoid(u8 Cond, const char *func, const u32 line)
{
	if(!Cond)
		XAieLib_print("Assert: %s, line %d\n", func, line);
#ifdef __AIEBAREMTL__
	Xil_AssertNonvoid(Cond);
#else
	assert(Cond);
#endif
	return 0;
}

/*****************************************************************************/
/**
*
* This asserts if the condition doesn't meet. Can be used for void return
* function.
*
* @param	Cond: Condition to meet. Should be 0 or 1.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieLib_AssertVoid(u8 Cond, const char *func, const u32 line)
{
	if(!Cond)
		XAieLib_print("Assert: %s, line %d\n", func, line);
#ifdef __AIEBAREMTL__
	Xil_AssertVoid(Cond);
#else
	assert(Cond);
#endif
}

/*****************************************************************************/
/**
*
* This provides to sleep in micro seconds.
*
* @param	Usec: Micro seconds to sleep
*
* @return	0 for success, and -1 for error..
*
* @note		None.
*
*******************************************************************************/
int XAieLib_usleep(u64 Usec)
{
#ifdef __AIESIM__
	return XAieSim_usleep(Usec);
#else
	/**
	 * FIXME: Platform implementation of usleep() API, returns void when it
	 * is expected to return SUCCESS/FAILURE code. Instead of returning a
	 * zero, this API must return SUCCESS/FAILURE code when usleep API is
	 * fixed.
	 */
	usleep(Usec);
	return 0;
#endif
}

/*****************************************************************************/
/**
*
* This API loads the elf to corresponding tile
*
* @param	TileInstPtr: Tile instance for the elf to be loaded
* @param	ElfPtr: path to the elf file
*
* @return	XAIELIB_SUCCESS on success, otherwise XAIELIB_FAILURE
*
* @note		None.
*
*******************************************************************************/
u32 XAieLib_LoadElf(XAieGbl_Tile *TileInstPtr, u8 *ElfPtr, u8 LoadSym)
{
#ifdef __AIESIM__
	return XAieSim_LoadElf(TileInstPtr, ElfPtr, LoadSym);
#elif defined __AIEBAREMTL__
	return XAIELIB_FAILURE;
#else
	/* Use the simulation elf load to workaround the elf loader issue. */
	return XAieSim_LoadElf(TileInstPtr, ElfPtr, LoadSym);
#endif
}

/*****************************************************************************/
/**
*
* This API loads the elf to corresponding tile
*
* @param	TileInstPtr: Tile instance for the elf to be loaded
* @param	ElfPtr: pointer to the elf in memory
*
* @return	XAIELIB_SUCCESS on success, otherwise XAIELIB_FAILURE
*
* @note		None.
*
*******************************************************************************/
u32 XAieLib_LoadElfMem(XAieGbl_Tile *TileInstPtr, u8 *ElfPtr, u8 LoadSym)
{
#ifdef __AIESIM__
	return XAIELIB_FAILURE;
#elif defined __AIEBAREMTL__
	return XAIELIB_FAILURE;
#else
	return XAieSim_LoadElfMem(TileInstPtr, ElfPtr, LoadSym);
#endif
}

/*****************************************************************************/
/**
*
* This API initializes the platform specific device instance if needed
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieLib_InitDev(void)
{
#ifdef __AIESIM__
#elif defined __AIEBAREMTL__
#else
	XAieIO_Init();
#endif
}

/*****************************************************************************/
/**
*
* This API initializes the platform specific tile instance if needed
*
* @param	TileInstPtr: Tile instance to be initialized
*
* @return	XAIELIB_SUCCESS on success, otherwise XAIELIB_FAILURE.
*
* @note		If there's no platform specific initialization,
* return XAIELIB_SUCCESS.
*
*******************************************************************************/
u32 XAieLib_InitTile(XAieGbl_Tile *TileInstPtr)
{
#ifdef __AIESIM__
	return XAIELIB_SUCCESS;
#elif defined __AIEBAREMTL__
	return XAIELIB_SUCCESS;
#else
	return XAieTileProc_Init(TileInstPtr);
#endif
}

/*****************************************************************************/
/**
*
* This API unregisters the interrupt.
*
* @param	Offest: Should be 1 - 3.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieLib_InterruptUnregisterIsr(int Offset)
{
#ifdef __AIESIM__
	return;
#elif defined __AIEBAREMTL__
	return;
#else
	XAieIO_IntrUnregisterIsr(Offset);
#endif
}

/*****************************************************************************/
/**
*
* This API registers the handler for interrupt
*
* @param	Offset: The value should one of 1 - 3.
* @param	Handler: the callback to be called upon interrupt.
* @param	Data: the data to be used with the handler.
*
* @return	XAIELIB_SUCCESS on success, otherwise XAIELIB_FAILURE.
*
* @note		None.
*
*******************************************************************************/
int XAieLib_InterruptRegisterIsr(int Offset, int (*Handler) (void *Data), void *Data)
{
#ifdef __AIESIM__
	return XAIELIB_FAILURE;
#elif defined __AIEBAREMTL__
	return XAIELIB_FAILURE;
#else
	return XAieIO_IntrRegisterIsr(Offset, Handler, Data);
#endif
}

/*****************************************************************************/
/**
*
* This API re-routes to platform print function
*
* @param	format strings
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieLib_IntPrint(const char *Format, ...)
{
#ifdef __AIESIM__
	/*
	 * If XAieSim_print() is used, the driver should be built with
	 * XAIE_DEBUG. Use print directly instead.
	 */
	printf(Format);
#elif defined __AIEBAREMTL__
	xil_printf(Format);
#else
	va_list argptr;
	va_start(argptr, Format);
	vprintf(Format, argptr);
	va_end(argptr);
#endif
}

/*****************************************************************************/
/**
*
* This is the memory function to initialize the platform specific memory
* instance.
*
* @param	idx: Index of the memory to initialize.
*
* @return	Pointer to the initialized memory instance. Null or 0 if not
* supported.
*
* @note		None.
*
*******************************************************************************/
XAieLib_MemInst *XAieLib_MemInit(u8 idx)
{
#ifdef __AIESIM__
	return 0;
#elif defined __AIEBAREMTL__
	XAieLib_MemInst *XAieLib_MemInstPtr;

	XAieLib_MemInstPtr = malloc(sizeof(*XAieLib_MemInstPtr));
	if (!XAieLib_MemInstPtr)
		return NULL;
	XAieLib_MemInstPtr->Vaddr = (u64)malloc(XAIELIB_BAREMTL_DEF_MEM_SIZE);
	if (!XAieLib_MemInstPtr->Vaddr) {
		free(XAieLib_MemInstPtr);
		return NULL;
	}
	XAieLib_MemInstPtr->Paddr = XAieLib_MemInstPtr->Vaddr;
	XAieLib_MemInstPtr->Size = XAIELIB_BAREMTL_DEF_MEM_SIZE;

	return XAieLib_MemInstPtr;
#else
	return (XAieLib_MemInst *)XAieIO_MemInit(idx);
#endif
}

/*****************************************************************************/
/**
*
* This is the memory function to free the platform specific memory instance
*
* @param	XAieLib_MemInstPtr: Memory instance pointer.
*
* @return	None.
*
* @note		@IO_MemInstPtr is freed and invalid after this function.
*
*******************************************************************************/
void XAieLib_MemFinish(XAieLib_MemInst *XAieLib_MemInstPtr)
{
#ifdef __AIESIM__
#elif defined __AIEBAREMTL__
	free((void *)XAieLib_MemInstPtr->Vaddr);
	free(XAieLib_MemInstPtr);
#else
	XAieIO_Mem *MemInstPtr = (XAieIO_Mem *)XAieLib_MemInstPtr;
	XAieIO_MemFinish(MemInstPtr);
#endif
}

/*****************************************************************************/
/**
*
* This is the memory function to attach the external memory to device
*
* @param	Vaddr: Vaddr of the memory
* @param	Paddr: Paddr of the memory
* @param	Size: Size of the memory
* @param	MemHandle: Handle of the memory. For linux, dmabuf fd
*
* @return	Pointer to the attached memory instance.
*
* @note		Some arguments are not required for some backend platforms.
* This is determined by platform implementation.
*
*******************************************************************************/
XAieLib_MemInst *XAieLib_MemAttach(u64 Vaddr, u64 Paddr, u64 Size, u64 MemHandle)
{
#ifdef __AIESIM__
	return 0;
#elif defined __AIEBAREMTL__
	XAieLib_MemInst *XAieLib_MemInstPtr;

	XAieLib_MemInstPtr = malloc(sizeof(*XAieLib_MemInstPtr));
	if (!XAieLib_MemInstPtr)
		return NULL;
	/* In baremetal, the handle doesn't exist */
	XAieLib_MemInstPtr->Vaddr = Vaddr;
	XAieLib_MemInstPtr->Paddr = Paddr;
	XAieLib_MemInstPtr->Size = Size;

	return XAieLib_MemInstPtr;
#else
	return (XAieLib_MemInst *)XAieIO_MemAttach(Vaddr, Paddr, Size, MemHandle);
#endif
}

/*****************************************************************************/
/**
*
* This is the memory function to detach the memory from device
*
* @param	XAieLib_MemInstPtr: Memory instance pointer.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieLib_MemDetach(XAieLib_MemInst *XAieLib_MemInstPtr)
{
#ifdef __AIESIM__
#elif defined __AIEBAREMTL__
	/* In baremetal expect the handle to be the paddr / vaddr */
	free(XAieLib_MemInstPtr);
#else
	XAieIO_MemDetach((XAieIO_Mem *)XAieLib_MemInstPtr);
#endif
}

/*****************************************************************************/
/**
*
* This is the memory function to allocate a memory
*
* @param	Size: Size of the memory
* @param	Attr: Any of XAIELIB_MEM_ATTR_*
*
* @return	Pointer to the allocated IO memory instance.
*
* @note		None.
*
*******************************************************************************/
XAieLib_MemInst *XAieLib_MemAllocate(u64 Size, u32 Attr)
{
#ifdef __AIESIM__
	return 0;
#elif defined __AIEBAREMTL__
	XAieLib_MemInst *XAieLib_MemInstPtr;

	XAieLib_MemInstPtr = malloc(sizeof(*XAieLib_MemInstPtr));
	if (!XAieLib_MemInstPtr)
		return NULL;
	XAieLib_MemInstPtr->Vaddr = (u64)malloc(Size);
	XAieLib_MemInstPtr->Paddr = XAieLib_MemInstPtr->Vaddr;
	XAieLib_MemInstPtr->Size = Size;
	/*
	 * 'Attr' is not handled at the moment. So it's always cached, and
	 * the sync or accessor function takes care of it.
	 */

	return XAieLib_MemInstPtr;
#else
	return (XAieLib_MemInst *)XAieIO_MemAllocate(Size, Attr);
#endif
}

/*****************************************************************************/
/**
*
* This is the memory function to free the memory
*
* @param	XAieLib_MemInstPtr: IO Memory instance pointer.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieLib_MemFree(XAieLib_MemInst *XAieLib_MemInstPtr)
{
#ifdef __AIESIM__
#elif defined __AIEBAREMTL__
	free((void *)XAieLib_MemInstPtr->Vaddr);
	free(XAieLib_MemInstPtr);
#else
	XAieIO_MemFree((XAieIO_Mem *)XAieLib_MemInstPtr);
#endif
}

/*****************************************************************************/
/**
*
* This is the memory function to sync the memory for CPU
*
* @param	XAieLib_MemInstPtr: IO Memory instance pointer.
*
* @return	XAIELIB_SUCCESS if successful.
*
* @note		None.
*
*******************************************************************************/
u8 XAieLib_MemSyncForCPU(XAieLib_MemInst *XAieLib_MemInstPtr)
{
#ifdef __AIESIM__
	return 0;
#elif defined __AIEBAREMTL__
	Xil_DCacheInvalidateRange(XAieLib_MemInstPtr->Vaddr, XAieLib_MemInstPtr->Size);
	return 0;
#else
	return XAieIO_MemSyncForCPU((XAieIO_Mem *)XAieLib_MemInstPtr);
#endif
}

/*****************************************************************************/
/**
*
* This is the memory function to sync the memory for device
*
* @param	XAieLib_MemInstPtr: IO Memory instance pointer.
*
* @return	XAIELIB_SUCCESS if successful.
*
* @note		None.
*
*******************************************************************************/
u8 XAieLib_MemSyncForDev(XAieLib_MemInst *XAieLib_MemInstPtr)
{
#ifdef __AIESIM__
	return 0;
#elif defined __AIEBAREMTL__
	Xil_DCacheFlushRange(XAieLib_MemInstPtr->Vaddr, XAieLib_MemInstPtr->Size);
	return 0;
#else
	return XAieIO_MemSyncForDev((XAieIO_Mem *)XAieLib_MemInstPtr);
#endif

}

/*****************************************************************************/
/**
*
* This is the memory function to return the size of the memory instance
*
* @param	XAieLib_MemInstPtr: Memory instance pointer.
*
* @return	size of the memory instance. 0 if not supported.
*
* @note		None.
*
*******************************************************************************/
u64 XAieLib_MemGetSize(XAieLib_MemInst *XAieLib_MemInstPtr)
{
#ifdef __AIESIM__
	return 0;
#elif defined __AIEBAREMTL__
	return XAieLib_MemInstPtr->Size -
		((u64)XAieLib_MemInstPtr->Vaddr % XAIELIB_SHIM_MEM_ALIGN);
#else
	XAieIO_Mem *MemInstPtr = (XAieIO_Mem *)XAieLib_MemInstPtr;
	return XAieIO_MemGetSize(MemInstPtr);
#endif
}

/*****************************************************************************/
/**
*
* This is the memory function to return the virtual address of
* the memory instance
*
* @param	XAieLib_MemInstPtr: Memory instance pointer.
*
* @return	Mapped virtual address of the memory instance.
* 0 if not supported.
*
* @note		None.
*
*******************************************************************************/
u64 XAieLib_MemGetVaddr(XAieLib_MemInst *XAieLib_MemInstPtr)
{
#ifdef __AIESIM__
	return 0;
#elif defined __AIEBAREMTL__
	return ((u64)XAieLib_MemInstPtr->Vaddr + XAIELIB_SHIM_MEM_ALIGN - 1) &
		~(XAIELIB_SHIM_MEM_ALIGN - 1);
#else
	XAieIO_Mem *MemInstPtr = (XAieIO_Mem *)XAieLib_MemInstPtr;
	return XAieIO_MemGetVaddr(MemInstPtr);
#endif
}

/*****************************************************************************/
/**
*
* This is the memory function to return the virtual address of
* the memory instance
*
* @param	XAieLib_MemInstPtr: Memory instance pointer.
*
* @return	Physical address of the memory instance. 0 if not supported.
*
* @note		None.
*
*******************************************************************************/
u64 XAieLib_MemGetPaddr(XAieLib_MemInst *XAieLib_MemInstPtr)
{
#ifdef __AIESIM__
	return 0;
#elif defined __AIEBAREMTL__
	return ((u64)XAieLib_MemInstPtr->Vaddr + XAIELIB_SHIM_MEM_ALIGN - 1) &
		~(XAIELIB_SHIM_MEM_ALIGN - 1);
#else
	XAieIO_Mem *MemInstPtr = (XAieIO_Mem *)XAieLib_MemInstPtr;
	return XAieIO_MemGetPaddr(MemInstPtr);
#endif
}

/*****************************************************************************/
/**
*
* This is the memory function to write to the physical address.
*
* @param	XAieLib_MemInstPtr: Memory instance pointer.
* @param	Addr: Absolute physical address to write.
* @param	Data: A 32 bit data to write.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieLib_MemWrite32(XAieLib_MemInst *XAieLib_MemInstPtr, u64 Addr, u32 Data)
{
#ifdef __AIESIM__
#elif defined __AIEBAREMTL__
	Xil_Out32(Addr, Data);
	Xil_DCacheFlushRange(Addr, 4);
#else
	XAieIO_Mem *MemInstPtr = (XAieIO_Mem *)XAieLib_MemInstPtr;
	XAieIO_MemWrite32(MemInstPtr, Addr, Data);
#endif
}

/*****************************************************************************/
/**
*
* This is the memory function to read from the physical address.
*
* @param	XAieLib_MemInstPtr: Memory instance pointer.
* @param	Addr: Absolute physical address to write.
*
* @return	A read 32 bit data. 0 if not supported.
*
* @note		None.
*
*******************************************************************************/
u32 XAieLib_MemRead32(XAieLib_MemInst *XAieLib_MemInstPtr, u64 Addr)
{
#ifdef __AIESIM__
	return 0;
#elif defined __AIEBAREMTL__
	Xil_DCacheInvalidateRange(Addr, 4);
	return Xil_In32(Addr);
#else
	XAieIO_Mem *MemInstPtr = (XAieIO_Mem *)XAieLib_MemInstPtr;
	return XAieIO_MemRead32(MemInstPtr, Addr);
#endif
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
u32 XAieLib_Read32(u64 Addr)
{
#ifdef __AIESIM__
	return(XAieSim_Read32(Addr));
#elif defined __AIEBAREMTL__
        return(Xil_In32(Addr));
#else
	return(XAieIO_Read32(Addr));
#endif
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
void XAieLib_Read128(u64 Addr, u32 *Data)
{
	u8 Idx;

	for(Idx = 0U; Idx < 4U; Idx++) {
#ifdef __AIESIM__
		Data[Idx] = XAieSim_Read32(Addr + Idx*4U);
#elif defined __AIEBAREMTL__
		Data[Idx] = Xil_In32(Addr + Idx*4U);
#else
		Data[Idx] = XAieIO_Read32(Addr + Idx*4U);
#endif
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
void XAieLib_Write32(u64 Addr, u32 Data)
{
#ifdef __AIESIM__
	XAieSim_Write32(Addr, Data);
#elif defined __AIEBAREMTL__
        Xil_Out32(Addr, Data);
#else
	XAieIO_Write32(Addr, Data);
#endif
}

/*****************************************************************************/
/**
*
* This is the memory IO function to write a masked 32bit data to
* the specified address.
*
* @param	Addr: Address to write to.
* @param	Mask: Mask to be applied to Data.
* @param	Data: 32-bit data to be written.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieLib_MaskWrite32(u64 Addr, u32 Mask, u32 Data)
{
	u32 RegVal;

#ifdef __AIESIM__
	XAieSim_MaskWrite32(Addr, Mask, Data);
#elif defined __AIEBAREMTL__
        RegVal = Xil_In32(Addr);
	RegVal &= ~Mask;
	RegVal |= Data;
        Xil_Out32(Addr, RegVal);
#else
	RegVal = XAieIO_Read32(Addr);
	RegVal &= ~Mask;
	RegVal |= Data;
	XAieIO_Write32(Addr, RegVal);
#endif
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
void XAieLib_Write128(u64 Addr, u32 *Data)
{
#ifdef __AIESIM__
	XAieSim_Write128(Addr, Data);
#elif defined __AIEBAREMTL__
	u8 Idx;

	for(Idx = 0U; Idx < 4U; Idx++) {
		Xil_Out32((u32)Addr + Idx * 4U, Data[Idx]);
	}
#else
	XAieIO_Write128(Addr, Data);
#endif
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
void XAieLib_WriteCmd(u8 Command, u8 ColId, u8 RowId, u32 CmdWd0,
						u32 CmdWd1, u8 *CmdStr)
{
#ifdef __AIESIM__
	XAieSim_WriteCmd(Command, ColId, RowId, CmdWd0, CmdWd1, CmdStr);
#elif defined __AIEBAREMTL__
#endif
}

/*****************************************************************************/
/**
*
* This is the IO function to poll until the value at the address to be given
* masked value.
*
* @param	Addr: Address to write to.
* @param	Mask: Mask to be applied to read data.
* @param	Value: The expected value
* @param	TimeOutUs: Minimum timeout in usec.
*
* @return	XAIELIB_SUCCESS on success, otherwise XAIELIB_FAILURE
*
* @note		None.
*
*******************************************************************************/
u32 XAieLib_MaskPoll(u64 Addr, u32 Mask, u32 Value, u32 TimeOutUs)
{
	u32 Ret = XAIELIB_FAILURE;

#ifdef __AIESIM__
	if (XAieSim_MaskPoll(Addr, Mask, Value, TimeOutUs) == XAIESIM_SUCCESS) {
		Ret = XAIELIB_SUCCESS;
	}
#else
	u32 Count, MinTimeOutUs;

	/*
	 * Any value less than 200 us becomes noticable overhead. This is based
	 * on some profiling, and it may vary between platforms.
	 */
	MinTimeOutUs = 200;
	Count = (TimeOutUs + MinTimeOutUs - 1) / MinTimeOutUs;

	while (Count > 0U) {
		if ((XAieLib_Read32(Addr) & Mask) == Value) {
			Ret = XAIELIB_SUCCESS;
			break;
		}
		XAieLib_usleep(MinTimeOutUs);
		Count--;
	}

	/* Check for the break from timed-out loop */
	if ((Ret == XAIELIB_FAILURE) &&
			((XAieLib_Read32(Addr) & Mask) == Value)) {
		Ret = XAIELIB_SUCCESS;
	}
#endif
	return Ret;
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
u32 XAieLib_NPIRead32(u64 Addr)
{
#ifdef __AIESIM__
	return XAieSim_NPIRead32(Addr);
#elif defined __AIEBAREMTL__
        return Xil_In32(Addr);
#else
	return XAieIO_NPIRead32(Addr);
#endif
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
void XAieLib_NPIWrite32(u64 Addr, u32 Data)
{
#ifdef __AIESIM__
	XAieSim_NPIWrite32(Addr, Data);
#elif defined __AIEBAREMTL__
        Xil_Out32(Addr, Data);
#else
	XAieIO_NPIWrite32(Addr, Data);
#endif
}

/*****************************************************************************/
/**
*
* This is the NPI IO function to write a masked 32bit data to
* the specified address.
*
* @param	Addr: Address to write to.
* @param	Mask: Mask to be applied to Data.
* @param	Data: 32-bit data to be written.
*
* @return	None.
*
* @note		This only work if NPI is accessble.
*
*******************************************************************************/
void XAieLib_NPIMaskWrite32(u64 Addr, u32 Mask, u32 Data)
{
	u32 RegVal;

#ifdef __AIESIM__
	XAieSim_NPIMaskWrite32(Addr, Mask, Data);
#elif defined __AIEBAREMTL__
        RegVal = Xil_In32(Addr);
	RegVal &= ~Mask;
	RegVal |= Data;
        Xil_Out32(Addr, RegVal);
#else
	RegVal = XAieIO_NPIRead32(Addr);
	RegVal &= ~Mask;
	RegVal |= Data;
	XAieIO_NPIWrite32(Addr, RegVal);
#endif
}
/*****************************************************************************/
/**
*
* This is the NPI IO function to poll until the value at the address to be given
* masked value.
*
* @param	Addr: Address to write to.
* @param	Mask: Mask to be applied to read data.
* @param	Value: The expected value
* @param	TimeOutUs: Minimum timeout in usec.
*
* @return	XAIELIB_SUCCESS on success, otherwise XAIELIB_FAILURE
*
* @note		This only work if NPI is accessble.
*
*******************************************************************************/
u32 XAieLib_NPIMaskPoll(u64 Addr, u32 Mask, u32 Value, u32 TimeOutUs)
{
	u32 Ret = XAIELIB_FAILURE;

#ifdef __AIESIM__
	if (XAieSim_NPIMaskPoll(Addr, Mask, Value, TimeOutUs) == XAIESIM_SUCCESS) {
		Ret = XAIELIB_SUCCESS;
	}
#else
	u32 Count, MinTimeOutUs;

	/*
	 * Any value less than 200 us becomes noticable overhead. This is based
	 * on some profiling, and it may vary between platforms.
	 */
	MinTimeOutUs = 200;
	Count = (TimeOutUs + MinTimeOutUs - 1) / MinTimeOutUs;

	while (Count > 0U) {
		if ((XAieLib_NPIRead32(Addr) & Mask) == Value) {
			Ret = XAIELIB_SUCCESS;
			break;
		}
		XAieLib_usleep(MinTimeOutUs);
		Count--;
	}

	/* Check for the break from timed-out loop */
	if ((Ret == XAIELIB_FAILURE) &&
			((XAieLib_NPIRead32(Addr) & Mask) == Value)) {
		Ret = XAIELIB_SUCCESS;
	}
#endif
	return Ret;
}

/** @} */
