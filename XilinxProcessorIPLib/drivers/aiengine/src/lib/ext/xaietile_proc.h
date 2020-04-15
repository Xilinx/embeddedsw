/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/



/*****************************************************************************/
/**
* @file xaietile_proc.h
* @{
*
* This file contains the variable and function prototypes for ELF loading.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Hyun    08/17/2018  Initial creation
* 1.1  Nishad  12/05/2018  Renamed ME attributes to AIE
* </pre>
*
******************************************************************************/
#ifndef XAIETILEPROC_H
#define XAIETILEPROC_H

/***************************** Include Files *********************************/
#include <elf.h>
#include "xaiegbl.h"
#include "xaiegbl_params.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
struct XAieGbl_Tile;
typedef struct XAieGbl_Tile XAieGbl_Tile;

/***************************** Macro Definitions *****************************/

/************************** Function Prototypes  *****************************/
u32 XAieTileProc_LoadElfFile(XAieGbl_Tile *TileInstPtr, u8 *ElfPtr, u8 LoadSym);
u32 XAieTileProc_LoadElfMem(XAieGbl_Tile *TileInstPtr, u8 *ElfPtr, u8 LoadSym);

u32 XAieTileProc_Init(XAieGbl_Tile *TileInstPtr);
u32 XAieTileProc_Finish(XAieGbl_Tile *TileInstPtr);

#endif		/* end of protection macro */
/** @} */
