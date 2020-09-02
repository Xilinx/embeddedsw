/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_debug.h
* @{
*
* This file contains the data structures and routines for low level IO
* operations for debug backend.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   06/29/2020 Initial creation.
* </pre>
*
******************************************************************************/
#ifndef XAIE_DEBUG_H
#define XAIE_DEBUG_H
/***************************** Include Files *********************************/
#include "xaie_io.h"
#include "xaiegbl.h"

/************************** Function Prototypes  *****************************/
AieRC XAie_DebugIO_Init(XAie_DevInst *DevInst);
AieRC XAie_DebugIO_Finish(void *IOInst);
void XAie_DebugIO_Write32(void *IOInst, u64 RegOff, u32 Value);
u32 XAie_DebugIO_Read32(void *IOInst, u64 RegOff);
void XAie_DebugIO_MaskWrite32(void *IOInst, u64 RegOff, u32 Mask, u32 Value);
u32 XAie_DebugIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask, u32 Value,
		u32 TimeOutUs);
void XAie_DebugIO_BlockWrite32(void *IOInst, u64 RegOff, u32 *Data, u32 Size);
void XAie_DebugIO_BlockSet32(void *IOInst, u64 RegOff, u32 Data, u32 Size);
void XAie_DebugIO_CmdWrite(void *IOInst, u8 Col, u8 Row, u8 Command, u32 CmdWd0,
		u32 CmdWd1, const char *CmdStr);
AieRC XAie_DebugIO_SetShimResetAssert(void *IOInst, u8 RstEnable);
AieRC XAie_DebugIO_SetProtectedReg(void *IOInst, XAie_DevInst *DevInst,
			u8 Enable);
AieRC XAie_DebugIO_RunOp(void *IOInst, XAie_DevInst *DevInst,
		XAie_BackendOpCode Op, void *Arg);
XAie_MemInst* XAie_DebugMemAllocate(XAie_DevInst *DevInst, u64 Size,
		XAie_MemCacheProp Cache);
AieRC XAie_DebugMemFree(XAie_MemInst *MemInst);
AieRC XAie_DebugMemSyncForCPU(XAie_MemInst *MemInst);
AieRC XAie_DebugMemSyncForDev(XAie_MemInst *MemInst);
AieRC XAie_DebugMemAttach(XAie_MemInst *MemInst, u64 MemHandle);
AieRC XAie_DebugMemDetach(XAie_MemInst *MemInst);

#endif	/* End of protection macro */

/** @} */
