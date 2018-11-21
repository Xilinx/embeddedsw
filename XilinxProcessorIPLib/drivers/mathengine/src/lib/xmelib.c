/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*****************************************************************************/
/**
* @file xmelib.c
* @{
*
* This file contains the low level layer interface of the ME driver with the
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
* 1.5  Hyun    10/11/2018  Don't include the xmeio header for sim build
* 1.6  Hyun    10/16/2018  Added the baremetal compile switch everywhere
*                          it's needed
* 1.7  Hyun    11/14/2018  Move platform dependent code to xmelib.c
* </pre>
*
******************************************************************************/
#include "xmelib.h"

#ifdef __MESIM__ /* ME simulator */

#include "xmesim.h"

#elif defined __MEBAREMTL__ /* Bare-metal application */

#include "xil_types.h"
#include "xil_io.h"
#include "xil_assert.h"
#include "xil_printf.h"
#include "xstatus.h"
#include "sleep.h"

#else /* Non-baremetal application, ex Linux */

#include <stdio.h>
#include <unistd.h>

#include "xmeio.h"
#include "xmetile_proc.h"

#endif

/***************************** Include Files *********************************/

/***************************** Macro Definitions *****************************/

/************************** Variable Definitions *****************************/

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
u32 XMeLib_AssertNonvoid(u8 Cond)
{
#ifdef __MESIM__
	XMeSim_AssertNonvoid(Cond);
#elif defined __MEBAREMTL__
	Xil_AssertNonvoid(Cond);
#else
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
void XMeLib_AssertVoid(u8 Cond)
{
#ifdef __MESIM__
	XMeSim_AssertVoid(Cond);
#elif defined __MEBAREMTL__
	Xil_AssertVoid(Cond);
#else
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
int XMeLib_usleep(u64 Usec)
{
#ifdef __MESIM__
	return XMeSim_usleep(Usec);
#elif defined __MEBAREMTL__
	return usleep_A53(Usec);
#else
	return usleep(Usec);
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
* @return	XMELIB_SUCCESS on success, otherwise XMELIB_FAILURE
*
* @note		None.
*
*******************************************************************************/
u32 XMeLib_LoadElf(XMeGbl_Tile *TileInstPtr, u8 *ElfPtr, u8 LoadSym)
{
#ifdef __MESIM__
	return XMeSim_LoadElf(TileInstPtr, ElfPtr, LoadSym);
#elif defined __MEBAREMTL__
	return XMELIB_FAILURE;
#else
	return XMeTileProc_LoadElfFile(TileInstPtr, ElfPtr, LoadSym);
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
void XMeLib_InitDev(void)
{
#ifdef __MESIM__
#elif defined __MEBAREMTL__
#else
	XMeIO_Init();
#endif
}

/*****************************************************************************/
/**
*
* This API initializes the platform specific tile instance if needed
*
* @param	TileInstPtr: Tile instance to be initialized
*
* @return	XMELIB_SUCCESS on success, otherwise XMELIB_FAILURE.
*
* @note		If there's no platform specific initialization,
* return XMELIB_SUCCESS.
*
*******************************************************************************/
u32 XMeLib_InitTile(XMeGbl_Tile *TileInstPtr)
{
#ifdef __MESIM__
	return XMELIB_SUCCESS;
#elif defined __MEBAREMTL__
	return XMELIB_SUCCESS;
#else
	return XMeTileProc_Init(TileInstPtr);
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
void XMeLib_IntPrint(const char *Format, ...)
{
#ifdef __MESIM__
	/*
	 * If XMeSim_print() is used, the driver should be built with
	 * XME_DEBUG. Use print directly instead.
	 */
	printf(Format);
#elif defined __MEBAREMTL__
	xil_printf(Format);
#else
	printf(Format);
#endif
}

/*****************************************************************************/
/**
*
* This is the memory function to free the platform specific memory instance
*
* @param	XMeLib_MemInstPtr: Memory instance pointer.
*
* @return	None.
*
* @note		@IO_MemInstPtr is freed and invalid after this function.
*
*******************************************************************************/
void XMeLib_MemFinish(XMeLib_MemInst *XMeLib_MemInstPtr)
{
#ifdef __MESIM__
#elif defined __MEBAREMTL__
#else
	XMeIO_Mem *MemInstPtr = (XMeIO_Mem *)XMeLib_MemInstPtr;
	XMeIO_MemFinish(MemInstPtr);
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
XMeLib_MemInst *XMeLib_MemInit(u8 idx)
{
#ifdef __MESIM__
	return 0;
#elif defined __MEBAREMTL__
	return NULL;
#else
	return (XMeLib_MemInst *)XMeIO_MemInit(idx);
#endif
}

/*****************************************************************************/
/**
*
* This is the memory function to return the size of the memory instance
*
* @param	XMeLib_MemInstPtr: Memory instance pointer.
*
* @return	size of the memory instance. 0 if not supported.
*
* @note		None.
*
*******************************************************************************/
u64 XMeLib_MemGetSize(XMeLib_MemInst *XMeLib_MemInstPtr)
{
#ifdef __MESIM__
	return 0;
#elif defined __MEBAREMTL__
	return 0;
#else
	XMeIO_Mem *MemInstPtr = (XMeIO_Mem *)XMeLib_MemInstPtr;
	return XMeIO_MemGetSize(MemInstPtr);
#endif
}

/*****************************************************************************/
/**
*
* This is the memory function to return the virtual address of
* the memory instance
*
* @param	XMeLib_MemInstPtr: Memory instance pointer.
*
* @return	Mapped virtual address of the memory instance.
* 0 if not supported.
*
* @note		None.
*
*******************************************************************************/
u64 XMeLib_MemGetVaddr(XMeLib_MemInst *XMeLib_MemInstPtr)
{
#ifdef __MESIM__
	return 0;
#elif defined __MEBAREMTL__
	return 0;
#else
	XMeIO_Mem *MemInstPtr = (XMeIO_Mem *)XMeLib_MemInstPtr;
	return XMeIO_MemGetVaddr(MemInstPtr);
#endif
}

/*****************************************************************************/
/**
*
* This is the memory function to return the virtual address of
* the memory instance
*
* @param	XMeLib_MemInstPtr: Memory instance pointer.
*
* @return	Physical address of the memory instance. 0 if not supported.
*
* @note		None.
*
*******************************************************************************/
u64 XMeLib_MemGetPaddr(XMeLib_MemInst *XMeLib_MemInstPtr)
{
#ifdef __MESIM__
	return 0;
#elif defined __MEBAREMTL__
	return 0;
#else
	XMeIO_Mem *MemInstPtr = (XMeIO_Mem *)XMeLib_MemInstPtr;
	return XMeIO_MemGetPaddr(MemInstPtr);
#endif
}

/*****************************************************************************/
/**
*
* This is the memory function to write to the physical address.
*
* @param	XMeLib_MemInstPtr: Memory instance pointer.
* @param	Addr: Absolute physical address to write.
* @param	Data: A 32 bit data to write.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XMeLib_MemWrite32(XMeLib_MemInst *XMeLib_MemInstPtr, u64 Addr, u32 Data)
{
#ifdef __MESIM__
#elif defined __MEBAREMTL__
#else
	XMeIO_Mem *MemInstPtr = (XMeIO_Mem *)XMeLib_MemInstPtr;
	XMeIO_MemWrite32(MemInstPtr, Addr, Data);
#endif
}

/*****************************************************************************/
/**
*
* This is the memory function to read from the physical address.
*
* @param	XMeLib_MemInstPtr: Memory instance pointer.
* @param	Addr: Absolute physical address to write.
*
* @return	A read 32 bit data. 0 if not supported.
*
* @note		None.
*
*******************************************************************************/
u32 XMeLib_MemRead32(XMeLib_MemInst *XMeLib_MemInstPtr, u64 Addr)
{
#ifdef __MESIM__
	return 0;
#elif defined __MEBAREMTL__
	return 0;
#else
	XMeIO_Mem *MemInstPtr = (XMeIO_Mem *)XMeLib_MemInstPtr;
	return XMeIO_MemRead32(MemInstPtr, Addr);
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
u32 XMeLib_Read32(u64 Addr)
{
#ifdef __MESIM__
	return(XMeSim_Read32(Addr));
#elif defined __MEBAREMTL__
        return(Xil_In32(Addr));
#else
	return(XMeIO_Read32(Addr));
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
void XMeLib_Read128(u64 Addr, u32 *Data)
{
	u8 Idx;

	for(Idx = 0U; Idx < 4U; Idx++) {
#ifdef __MESIM__
		Data[Idx] = XMeSim_Read32(Addr + Idx*4U);
#elif defined __MEBAREMTL__
#else
		Data[Idx] = XMeIO_Read32(Addr + Idx*4U);
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
void XMeLib_Write32(u64 Addr, u32 Data)
{
#ifdef __MESIM__
	XMeSim_Write32(Addr, Data);
#elif defined __MEBAREMTL__
        Xil_Out32(Addr, Data);
#else
	XMeIO_Write32(Addr, Data);
#endif
}

void XMeLib_MaskWrite32(u64 Addr, u32 Mask, u32 Data)
{
	u32 RegVal;

#ifdef __MESIM__
	XMeSim_MaskWrite32(Addr, Mask, Data);
#elif defined __MEBAREMTL__
        RegVal = Xil_In32(Addr);
	RegVal &= ~Mask;
	RegVal |= Data;
        Xil_Out32(Addr, RegVal);
#else
	RegVal = XMeIO_Read32(Addr);
	RegVal &= ~Mask;
	RegVal |= Data;
	XMeIO_Write32(Addr, RegVal);
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
void XMeLib_Write128(u64 Addr, u32 *Data)
{
#ifdef __MESIM__
	XMeSim_Write128(Addr, Data);
#elif defined __MEBAREMTL__
#else
	XMeIO_Write128(Addr, Data);
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
void XMeLib_WriteCmd(u8 Command, u8 ColId, u8 RowId, u32 CmdWd0,
						u32 CmdWd1, u8 *CmdStr)
{
#ifdef __MESIM__
	XMeSim_WriteCmd(Command, ColId, RowId, CmdWd0, CmdWd1, CmdStr);
#elif defined __MEBAREMTL__
#endif
}

/** @} */

