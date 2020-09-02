/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_metal.h
* @{
*
* This file contains the generic definitions for the AIE simulator interface.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Hyun    07/12/2018  Initial creation
* 1.1  Nishad  12/05/2018  Renamed ME attributes to AIE
* 1.2  Tejus   06/09/2020  Rename and import file from legacy driver.
* </pre>
*
******************************************************************************/
#ifndef XAIE_METAL_H
#define XAIE_METAL_H

/***************************** Include Files *********************************/
#include "xaiegbl.h"

/************************** Constant Definitions *****************************/
/************************** Function Prototypes  *****************************/

AieRC XAie_MetalIO_Finish(void *IOInst);
AieRC XAie_MetalIO_Init(XAie_DevInst *DevInst);
u32 XAie_MetalIO_Read32(void *IOInst, u64 RegOff);
void XAie_MetalIO_Write32(void *IOInst, u64 RegOff, u32 Data);
void XAie_MetalIO_MaskWrite32(void *IOInst, u64 RegOff, u32 Mask, u32 Data);
u32 XAie_MetalIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask, u32 Value,
		u32 TimeOutUs);
void XAie_MetalIO_BlockWrite32(void *IOInst, u64 RegOff, u32 *Data, u32 Size);
void XAie_MetalIO_BlockSet32(void *IOInst, u64 RegOff, u32 Data, u32 Size);
void XAie_MetalIO_CmdWrite(void *IOInst, u8 Col, u8 Row, u8 Command, u32 CmdWd0,
		u32 CmdWd1, const char *CmdStr);
AieRC XAie_MetalIO_RunOp(void *IOInst, XAie_DevInst *DevInst,
		XAie_BackendOpCode Op, void *Arg);
XAie_MemInst* XAie_MetalMemAllocate(XAie_DevInst *DevInst, u64 Size,
		XAie_MemCacheProp Cache);
AieRC XAie_MetalMemFree(XAie_MemInst *MemInst);
AieRC XAie_MetalMemSyncForCPU(XAie_MemInst *MemInst);
AieRC XAie_MetalMemSyncForDev(XAie_MemInst *MemInst);
AieRC XAie_MetalMemAttach(XAie_MemInst *MemInst, u64 MemHandle);
AieRC XAie_MetalMemDetach(XAie_MemInst *MemInst);

#endif		/* end of protection macro */

/** @} */
