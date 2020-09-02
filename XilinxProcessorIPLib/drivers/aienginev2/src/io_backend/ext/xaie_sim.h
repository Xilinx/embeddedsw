/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_sim.h
* @{
*
* This file contains the data structures and routines for low level IO
* operations for simulation backend.
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
#ifndef XAIE_SIM_H
#define XAIE_SIM_H
/***************************** Include Files *********************************/
#include "xaiegbl.h"

/************************** Function Prototypes  *****************************/

AieRC XAie_SimIO_Init(XAie_DevInst *DevInst);
AieRC XAie_SimIO_Finish(void *IOInst);
void XAie_SimIO_Write32(void *IOInst, u64 RegOff, u32 Value);
u32 XAie_SimIO_Read32(void *IOInst, u64 RegOff);
void XAie_SimIO_MaskWrite32(void *IOInst, u64 RegOff, u32 Mask, u32 Value);
u32 XAie_SimIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask, u32 Value,
		u32 TimeOutUs);
void XAie_SimIO_BlockWrite32(void *IOInst, u64 RegOff, u32 *Data, u32 Size);
void XAie_SimIO_BlockSet32(void *IOInst, u64 RegOff, u32 Data, u32 Size);
void XAie_SimIO_CmdWrite(void *IOInst, u8 Col, u8 Row, u8 Command, u32 CmdWd0,
		u32 CmdWd1, const char *CmdStr);
AieRC XAie_SimIO_RunOp(void *IOInst, XAie_DevInst *DevInst,
		XAie_BackendOpCode Op, void *Arg);
XAie_MemInst* XAie_SimMemAllocate(XAie_DevInst *DevInst, u64 Size,
		XAie_MemCacheProp Cache);
AieRC XAie_SimMemFree(XAie_MemInst *MemInst);
AieRC XAie_SimMemSyncForCPU(XAie_MemInst *MemInst);
AieRC XAie_SimMemSyncForDev(XAie_MemInst *MemInst);
AieRC XAie_SimMemAttach(XAie_MemInst *MemInst, u64 MemHandle);
AieRC XAie_SimMemDetach(XAie_MemInst *MemInst);

#endif	/* End of protection macro */

/** @} */
