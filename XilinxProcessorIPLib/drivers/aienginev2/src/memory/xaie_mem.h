/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_mem.h
* @{
*
* Header file for data memory implementations.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   09/24/2019  Initial creation
* 1.1   Tejus   03/20/2020  Remove range apis
* 1.2   Nishad  07/30/2020  Add API to read and write block of data from tile
*			    data memory.
* </pre>
*
******************************************************************************/
#ifndef XAIEMEM_H
#define XAIEMEM_H

/***************************** Include Files *********************************/
#include "xaiegbl.h"

/***************************** Macro Definitions *****************************/
#define XAIE_MEM_WORD_ALIGN_SHIFT	2U
#define XAIE_MEM_WORD_ALIGN_MASK	((1 << XAIE_MEM_WORD_ALIGN_SHIFT) - 1)
#define XAIE_MEM_WORD_ALIGN_SIZE	(1 << XAIE_MEM_WORD_ALIGN_SHIFT)
#define XAIE_MEM_WORD_ROUND_UP(Addr)	(((Addr) + XAIE_MEM_WORD_ALIGN_MASK) & \
						~XAIE_MEM_WORD_ALIGN_MASK)
#define XAIE_MEM_WORD_ROUND_DOWN(Addr)	((Addr) & (~XAIE_MEM_WORD_ALIGN_MASK))

/************************** Function Prototypes  *****************************/
AieRC XAie_DataMemWrWord(XAie_DevInst *DevInst, XAie_LocType Loc,
		u32 Addr, u32 Data);
AieRC XAie_DataMemRdWord(XAie_DevInst *DevInst, XAie_LocType Loc,
		u32 Addr, u32 *Data);
AieRC XAie_DataMemBlockWrite(XAie_DevInst *DevInst, XAie_LocType Loc, u32 Addr,
		const void *Src, u32 Size);
AieRC XAie_DataMemBlockRead(XAie_DevInst *DevInst, XAie_LocType Loc, u32 Addr,
		void *Dst, u32 Size);

#endif		/* end of protection macro */

/** @} */
