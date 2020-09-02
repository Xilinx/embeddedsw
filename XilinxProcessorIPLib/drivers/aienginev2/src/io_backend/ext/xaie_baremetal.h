/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_baremetal.h
* @{
*
* This file contains the data structures and routines for low level IO
* operations for baremetal backend.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   07/04/2020 Initial creation.
* </pre>
*
******************************************************************************/
#ifndef XAIE_BAREMETAL_H
#define XAIE_BAREMETAL_H
/***************************** Include Files *********************************/
#include "xaie_io.h"
#include "xaiegbl.h"

/************************** Function Prototypes  *****************************/
AieRC XAie_BaremetalIO_Init(XAie_DevInst *DevInst);
AieRC XAie_BaremetalIO_Finish(void *IOInst);
void XAie_BaremetalIO_Write32(void *IOInst, u64 RegOff, u32 Value);
u32 XAie_BaremetalIO_Read32(void *IOInst, u64 RegOff);
void XAie_BaremetalIO_MaskWrite32(void *IOInst, u64 RegOff, u32 Mask,
		u32 Value);
u32 XAie_BaremetalIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask, u32 Value,
		u32 TimeOutUs);
void XAie_BaremetalIO_BlockWrite32(void *IOInst, u64 RegOff, u32 *Data,
		u32 Size);
void XAie_BaremetalIO_BlockSet32(void *IOInst, u64 RegOff, u32 Data, u32 Size);
void XAie_BaremetalIO_CmdWrite(void *IOInst, u8 Col, u8 Row, u8 Command,
		u32 CmdWd0, u32 CmdWd1, const char *CmdStr);
AieRC XAie_BaremetalIO_RunOp(void *IOInst, XAie_DevInst *DevInst,
		XAie_BackendOpCode Op, void *Arg);
XAie_MemInst* XAie_BaremetalMemAllocate(XAie_DevInst *DevInst, u64 Size,
		XAie_MemCacheProp Cache);
AieRC XAie_BaremetalMemFree(XAie_MemInst *MemInst);
AieRC XAie_BaremetalMemSyncForCPU(XAie_MemInst *MemInst);
AieRC XAie_BaremetalMemSyncForDev(XAie_MemInst *MemInst);
AieRC XAie_BaremetalMemAttach(XAie_MemInst *MemInst, u64 MemHandle);
AieRC XAie_BaremetalMemDetach(XAie_MemInst *MemInst);

#endif	/* End of protection macro */

/** @} */
