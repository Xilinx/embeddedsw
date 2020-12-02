/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_cdo.h
* @{
*
* This file contains the data structures and routines for low level IO
* operations for cdo backend.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   06/09/2020 Initial creation.
* </pre>
*
******************************************************************************/
#ifndef XAIE_CDO_H
#define XAIE_CDO_H
/***************************** Include Files *********************************/
#include "xaie_io.h"
#include "xaiegbl.h"

/************************** Function Prototypes  *****************************/
AieRC XAie_CdoIO_Init(XAie_DevInst *DevInst);
AieRC XAie_CdoIO_Finish(void *IOInst);
void XAie_CdoIO_Write32(void *IOInst, u64 RegOff, u32 Value);
u32 XAie_CdoIO_Read32(void *IOInst, u64 RegOff);
void XAie_CdoIO_MaskWrite32(void *IOInst, u64 RegOff, u32 Mask, u32 Value);
u32 XAie_CdoIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask, u32 Value,
		u32 TimeOutUs);
void XAie_CdoIO_BlockWrite32(void *IOInst, u64 RegOff, u32 *Data, u32 Size);
void XAie_CdoIO_BlockSet32(void *IOInst, u64 RegOff, u32 Data, u32 Size);
void XAie_CdoIO_CmdWrite(void *IOInst, u8 Col, u8 Row, u8 Command, u32 CmdWd0,
		u32 CmdWd1, const char *CmdStr);
AieRC XAie_CdoIO_RunOp(void *IOInst, XAie_DevInst *DevInst,
		XAie_BackendOpCode Op, void *Arg);
XAie_MemInst* XAie_CdoMemAllocate(XAie_DevInst *DevInst, u64 Size,
		XAie_MemCacheProp Cache);
AieRC XAie_CdoMemFree(XAie_MemInst *MemInst);
AieRC XAie_CdoMemSyncForCPU(XAie_MemInst *MemInst);
AieRC XAie_CdoMemSyncForDev(XAie_MemInst *MemInst);
AieRC XAie_CdoMemAttach(XAie_MemInst *MemInst, u64 MemHandle);
AieRC XAie_CdoMemDetach(XAie_MemInst *MemInst);

#endif	/* End of protection macro */

/** @} */
