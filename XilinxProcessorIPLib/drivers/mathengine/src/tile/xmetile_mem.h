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
* @file xmetile_mem.h
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
* </pre>
*
******************************************************************************/
#ifndef XMETILE_MEM_H
#define XMETILE_MEM_H

/***************************** Include Files *********************************/

/***************************** Constant Definitions **************************/

/***************************** Type Definitions ******************************/

/***************************** Macro Definitions *****************************/

/************************** Function Prototypes  *****************************/
void XMeTile_DmWriteWord(XMeGbl_Tile *TileInstPtr, u32 DmOffset, u32 DmVal);
u32 XMeTile_DmReadWord(XMeGbl_Tile *TileInstPtr, u32 DmOffset);

#endif		/* end of protection macro */
/** @} */

