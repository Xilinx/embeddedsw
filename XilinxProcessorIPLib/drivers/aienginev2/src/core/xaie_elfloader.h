/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_elfloader.h
* @{
*
* Header file for core elf loader functions
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   09/24/2019  Initial creation
* 1.1   Tejus   03/20/2020  Remove range apis
* 1.2   Tejus   05/26/2020  Add API to load elf from memory.
* </pre>
*
******************************************************************************/
#ifndef XAIELOADER_H
#define XAIELOADER_H

/***************************** Include Files *********************************/
#include <elf.h>
#include <stdlib.h>
#include <string.h>
#include "xaie_helper.h"
#include "xaiegbl.h"
#include "xaiegbl_defs.h"
#include "xaiegbl_defs.h"

/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/
typedef struct {
	u32 start;	/**< Stack start address */
	u32 end;	/**< Stack end address */
} XAieSim_StackSz;
/************************** Function Prototypes  *****************************/

AieRC XAie_LoadElf(XAie_DevInst *DevInst, XAie_LocType Loc, const char *ElfPtr,
		u8 LoadSym);
AieRC XAie_LoadElfMem(XAie_DevInst *DevInst, XAie_LocType Loc,
		const unsigned char* ElfMem);
#endif		/* end of protection macro */
/** @} */
