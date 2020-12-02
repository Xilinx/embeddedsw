/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaielib.h
* @{
*
* This file contains the generic definitions for the AIE drivers.
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
* 1.5  Hyun    11/14/2018  Move platform dependent code to xaielib.c
* 1.6  Nishad  12/05/2018  Renamed ME attributes to AIE
* 1.7  Hyun    01/08/2019  Add XAieLib_MaskPoll()
* 1.8  Tejus   10/14/2019  Enable assertion for linux and simulation
* 1.9  Wendy   02/25/2020  Add Logging API
* </pre>
*
******************************************************************************/
#ifndef XAIELIB_H
#define XAIELIB_H

/***************************** Include Files *********************************/
#include "xaieconfig.h"
#include <stdint.h>

/************************** Constant Definitions *****************************/
typedef uint8_t			u8;
typedef uint16_t		u16;
typedef int32_t			s32;
typedef uint32_t		u32;
typedef uint64_t		u64;

/* Don't mix with equivalent baremetal macros, ex, XST_SUCCESS */
#define XAIELIB_SUCCESS			0U
#define XAIELIB_FAILURE			1U
#define XAIELIB_COMPONENT_IS_READY	1U

#define XAIELIB_CMDIO_COMMAND_SETSTACK	0U
#define XAIELIB_CMDIO_COMMAND_LOADSYM	1U

/* Enable cache for memory mapping */
#define XAIELIB_MEM_ATTR_CACHE		0x1U

typedef enum {
	XAIELIB_LOGINFO,
	XAIELIB_LOGERROR
} XAieLib_LogLevel;

/************************** Variable Definitions *****************************/

/************************** Function Prototypes  *****************************/
u32 XAieLib_Read32(u64 Addr);
void XAieLib_Read128(u64 Addr, u32 *Data);
void XAieLib_Write32(u64 Addr, u32 Data);
void XAieLib_MaskWrite32(u64 Addr, u32 Mask, u32 Data);
void XAieLib_Write128(u64 Addr, u32 *Data);
void XAieLib_WriteCmd(u8 Command, u8 ColId, u8 RowId, u32 CmdWd0, u32 CmdWd1, u8 *CmdStr);
u32 XAieLib_MaskPoll(u64 Addr, u32 Mask, u32 Value, u32 TimeOutUs);

u32 XAieLib_NPIRead32(u64 Addr);
void XAieLib_NPIWrite32(u64 Addr, u32 Data);
u32 XAieLib_NPIMaskPoll(u64 Addr, u32 Mask, u32 Value, u32 TimeOutUs);

u32 XAieLib_AssertNonvoid(u8 Cond, const char *func, const u32 line);
void XAieLib_AssertVoid(u8 Cond, const char *func, const u32 line);
int XAieLib_usleep(u64 Usec);

struct XAieGbl_Tile;
typedef struct XAieGbl_Tile XAieGbl_Tile;

u32 XAieLib_LoadElf(XAieGbl_Tile *TileInstPtr, u8 *ElfPtr, u8 LoadSym);
u32 XAieLib_LoadElfMem(XAieGbl_Tile *TileInstPtr, u8 *ElfPtr, u8 LoadSym);

void XAieLib_InitDev(void);
u32 XAieLib_InitTile(XAieGbl_Tile *TileInstPtr);

void XAieLib_InterruptUnregisterIsr(int Offset);
int XAieLib_InterruptRegisterIsr(int Offset, int (*Handler) (void *Data), void *Data);
void XAieLib_InterruptEnable(void);
void XAieLib_InterruptDisable(void);

void XAieLib_IntPrint(const char *Format, ...);

#ifdef XAIE_DEBUG
#define XAieLib_print		XAieLib_IntPrint
#else
#define XAieLib_print(...)	{}
#endif

void XAieLib_log(XAieLib_LogLevel Level, const char *Format, ...);
u32 XAieLib_OpenLogFile(const char *File);
void XAieLib_CloseLogFile(void);

struct XAieLib_MemInst;
typedef struct XAieLib_MemInst XAieLib_MemInst;

void XAieLib_MemFinish(XAieLib_MemInst *XAieLib_MemInstPtr);
XAieLib_MemInst *XAieLib_MemInit(u8 idx);
void XAieLib_MemDetach(XAieLib_MemInst *XAieLib_MemInstPtr);
XAieLib_MemInst *XAieLib_MemAttach(u64 Vaddr, u64 Paddr, u64 Size, u64 MemHandle);
void XAieLib_MemFree(XAieLib_MemInst *XAieLib_MemInstPtr);
XAieLib_MemInst *XAieLib_MemAllocate(u64 Size, u32 Attr);
u8 XAieLib_MemSyncForCPU(XAieLib_MemInst *XAieLib_MemInstPtr);
u8 XAieLib_MemSyncForDev(XAieLib_MemInst *XAieLib_MemInstPtr);

u64 XAieLib_MemGetSize(XAieLib_MemInst *XAieLib_MemInstPtr);
u64 XAieLib_MemGetVaddr(XAieLib_MemInst *XAieLib_MemInstPtr);
u64 XAieLib_MemGetPaddr(XAieLib_MemInst *XAieLib_MemInstPtr);
void XAieLib_MemWrite32(XAieLib_MemInst *XAieLib_MemInstPtr, u64 Addr, u32 Data);
u32 XAieLib_MemRead32(XAieLib_MemInst *XAieLib_MemInstPtr, u64 Addr);

#endif		/* end of protection macro */
/** @} */


