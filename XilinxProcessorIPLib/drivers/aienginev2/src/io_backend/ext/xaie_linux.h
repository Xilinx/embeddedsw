/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_linux.h
* @{
*
* This file contains the low level layer IO interface for linux kernel backend.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus    07/29/2020  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XAIE_LINUX_H
#define XAIE_LINUX_H

/***************************** Include Files *********************************/
#include "xaiegbl.h"

/************************** Constant Definitions *****************************/
/************************** Function Prototypes  *****************************/

AieRC XAie_LinuxIO_Finish(void *IOInst);
AieRC XAie_LinuxIO_Init(XAie_DevInst *DevInst);
u32 XAie_LinuxIO_Read32(void *IOInst, u64 RegOff);
void XAie_LinuxIO_Write32(void *IOInst, u64 RegOff, u32 Data);
void XAie_LinuxIO_MaskWrite32(void *IOInst, u64 RegOff, u32 Mask, u32 Data);
u32 XAie_LinuxIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask, u32 Value,
		u32 TimeOutUs);
void XAie_LinuxIO_BlockWrite32(void *IOInst, u64 RegOff, u32 *Data, u32 Size);
void XAie_LinuxIO_BlockSet32(void *IOInst, u64 RegOff, u32 Data, u32 Size);
void XAie_LinuxIO_CmdWrite(void *IOInst, u8 Col, u8 Row, u8 Command, u32 CmdWd0,
		u32 CmdWd1, const char *CmdStr);
AieRC XAie_LinuxIO_OpSupported(void *IOInst, XAie_DevInst *DevInst,
		XAie_BackendOpCode Op);
AieRC XAie_LinuxIO_RunOp(void *IOInst, XAie_DevInst *DevInst,
		XAie_BackendOpCode Op, void *Arg);
XAie_MemInst* XAie_LinuxMemAllocate(XAie_DevInst *DevInst, u64 Size,
		XAie_MemCacheProp Cache);
AieRC XAie_LinuxMemFree(XAie_MemInst *MemInst);
AieRC XAie_LinuxMemSyncForCPU(XAie_MemInst *MemInst);
AieRC XAie_LinuxMemSyncForDev(XAie_MemInst *MemInst);
AieRC XAie_LinuxMemAttach(XAie_MemInst *MemInst, u64 MemHandle);
AieRC XAie_LinuxMemDetach(XAie_MemInst *MemInst);

#endif		/* end of protection macro */

/** @} */
