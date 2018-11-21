/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*****************************************************************************/
/**
* @file xmesim_elfload.h
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
* 1.0  Naresh  03/27/2018  Initial creation
* 1.1  Naresh  05/07/2018  Updated code to fix CR#1001944
* 1.2  Naresh  06/13/2018  Fixed CR#1003905
* 1.3  Naresh  07/11/2018  Updated copyright info
* </pre>
*
******************************************************************************/
#ifndef XMESIM_ELFLOAD_H
#define XMESIM_ELFLOAD_H

/***************************** Include Files *********************************/
#include <elf.h>
#include "xmegbl.h"
#include "xmegbl_params.h"

/************************** Constant Definitions *****************************/
#define XMESIM_ELF_SECTION_NUMMAX		100
#define XMESIM_ELF_SECTION_NAMEMAXLEN		100

#define XMESIM_ELF_TILEADDR_DMB_MASK            0x7FFFU	/* 32 KB */
#define XMESIM_ELF_TILEADDR_DMB_CARD_OFF        0x18000U
#define XMESIM_ELF_TILEADDR_DMB_CARD_SHIFT      15U

#define XMESIM_ELF_TILEBASE_ADDRMASK            XMEGBL_TILE_BASE_ADDRMASK
#define XMESIM_ELF_TILEADDR_COL_SHIFT           XMEGBL_TILE_ADDR_COL_SHIFT
#define XMESIM_ELF_TILEADDR_ROW_SHIFT           XMEGBL_TILE_ADDR_ROW_SHIFT

#define XMESIM_ELF_TILECORE_PRGMEM              XMEGBL_CORE_PRGMEM
#define XMESIM_ELF_TILECORE_DATMEM              XMEGBL_MEM_DATMEM

#define XMeSim_Tile                             XMeGbl_Tile

/**************************** Type Definitions *******************************/
/**
 * This typedef contains all the stack range addresses derived from the map file.
 */
typedef struct {
	uint32 start;	/**< Stack start address */
	uint32 end;	/**< Stack end address */
} XMeSim_StackSz;

/***************************** Macro Definitions *****************************/

/************************** Function Prototypes  *****************************/
uint32 XMeSim_LoadElf(XMeGbl_Tile *TileInstPtr, uint8 *ElfPtr, uint8 LoadSym);
uint32 XMeSim_GetStackRange(uint8 *MapPtr, XMeSim_StackSz *StackSzPtr);
void XMeSim_LoadSymbols(XMeGbl_Tile *TileInstPtr, uint8 *ElfPtr);
void XMeSim_WriteSection(XMeGbl_Tile *TileInstPtr, uint8 *SectName, Elf32_Shdr *SectPtr, FILE *Fd);
uint64_t XMeSim_GetTargetTileAddr(XMeGbl_Tile *TileInstPtr, uint32 ShAddr);

#endif		/* end of protection macro */
/** @} */

