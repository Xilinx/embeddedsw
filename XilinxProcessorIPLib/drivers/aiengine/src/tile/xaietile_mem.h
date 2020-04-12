/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaietile_mem.h
* @{
*
*  Header file for Tile memory control functions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Naresh  04/18/2018  Initial creation to fix CR#1000217
* 1.1  Naresh  07/11/2018  Updated copyright info and addressed CR#1006589
* 1.2  Nishad  12/05/2018  Renamed ME attributes to AIE
* 1.3  Hyun    06/27/2018  Add XAieTile_MemReadTimer()
* </pre>
*
******************************************************************************/
#ifndef XAIETILE_MEM_H
#define XAIETILE_MEM_H

/***************************** Include Files *********************************/

/***************************** Constant Definitions **************************/

/***************************** Type Definitions ******************************/

/***************************** Macro Definitions *****************************/

/************************** Function Prototypes  *****************************/
void XAieTile_DmWriteWord(XAieGbl_Tile *TileInstPtr, u32 DmOffset, u32 DmVal);
u32 XAieTile_DmReadWord(XAieGbl_Tile *TileInstPtr, u32 DmOffset);
u64 XAieTile_MemReadTimer(XAieGbl_Tile *TileInstPtr);

#endif		/* end of protection macro */
/** @} */

