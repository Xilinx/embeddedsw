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
* @file xmelib.h
* @{
*
* This file contains the generic definitions for the ME drivers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Naresh  03/27/2018  Initial creation
* 1.1  Naresh  05/23/2018  Added support for bare-metal BSP
* 1.2  Naresh  06/18/2018  Updated code as per standalone driver framework
* 1.3  Naresh  07/11/2018  Updated copyright info
* 1.4  Hyun    10/10/2018  Added the mask write API
* 1.5  Hyun    11/14/2018  Move platform dependent code to xmelib.c
* </pre>
*
******************************************************************************/
#ifndef XMELIB_H
#define XMELIB_H

/***************************** Include Files *********************************/
#include "xmeconfig.h"

/************************** Constant Definitions *****************************/
typedef unsigned char			u8;
typedef unsigned short int		u16;
typedef signed int			s32;
typedef unsigned int			u32;
typedef unsigned long int		u64;

/* Don't mix with equivalent baremetal macros, ex, XST_SUCCESS */
#define XMELIB_SUCCESS			0U
#define XMELIB_FAILURE			1U
#define XMELIB_COMPONENT_IS_READY	1U

#define XMELIB_CMDIO_COMMAND_SETSTACK	0U
#define XMELIB_CMDIO_COMMAND_LOADSYM	1U

/************************** Variable Definitions *****************************/

/************************** Function Prototypes  *****************************/
u32 XMeLib_Read32(u64 Addr);
void XMeLib_Read128(u64 Addr, u32 *Data);
void XMeLib_Write32(u64 Addr, u32 Data);
void XMeLib_MaskWrite32(u64 Addr, u32 Mask, u32 Data);
void XMeLib_Write128(u64 Addr, u32 *Data);
void XMeLib_WriteCmd(u8 Command, u8 ColId, u8 RowId, u32 CmdWd0, u32 CmdWd1, u8 *CmdStr);

u32 XMeLib_AssertNonvoid(u8 Cond);
void XMeLib_AssertVoid(u8 Cond);
int XMeLib_usleep(u64 Usec);

struct XMeGbl_Tile;
typedef struct XMeGbl_Tile XMeGbl_Tile;

u32 XMeLib_LoadElf(XMeGbl_Tile *TileInstPtr, u8 *ElfPtr, u8 LoadSym);

void XMeLib_InitDev(void);
u32 XMeLib_InitTile(XMeGbl_Tile *TileInstPtr);

void XMeLib_IntPrint(const char *Format, ...);

#ifdef XME_DEBUG
#define XMeLib_print		XMeLib_IntPrint
#else
#define XMeLib_print(...)	{}
#endif

struct XMeLib_MemInst;
typedef struct XMeLib_MemInst XMeLib_MemInst;

void XMeLib_MemFinish(XMeLib_MemInst *XMeLib_MemInstPtr);
XMeLib_MemInst *XMeLib_MemInit(u8 idx);
u64 XMeLib_MemGetSize(XMeLib_MemInst *XMeLib_MemInstPtr);
u64 XMeLib_MemGetVaddr(XMeLib_MemInst *XMeLib_MemInstPtr);
u64 XMeLib_MemGetPaddr(XMeLib_MemInst *XMeLib_MemInstPtr);
void XMeLib_MemWrite32(XMeLib_MemInst *XMeLib_MemInstPtr, u64 Addr, u32 Data);
u32 XMeLib_MemRead32(XMeLib_MemInst *XMeLib_MemInstPtr, u64 Addr);

#endif		/* end of protection macro */
/** @} */


