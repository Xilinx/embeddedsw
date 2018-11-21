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
* @file xmesim.h
* @{
*
* This file contains the generic definitions for the ME simulator interface.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Naresh  04/06/2018  Initial creation
* 1.1  Naresh  04/10/2018  Added the API declaration for XMeSim_Init
* 1.2  Naresh  04/10/2018  Added macro XMESIM_DEVCFG_SET_CONFIG
* 1.3  Naresh  04/18/2018  Modified workaround for CRVO#1696/CR#999680 to extend
*                          it to all types of registers and also for all tiles
* 1.4  Naresh  05/07/2018  Fixed CR#1001816
* 1.5  Naresh  05/23/2018  Updated code to fix CR#999693
* 1.6  Naresh  07/11/2018  Updated copyright info
* 1.7  Hyun    09/12/2018  Fixed CR#1006669
* 1.8  Hyun    10/16/2018  Added XMeSim_SetIOMode and mask write APIs
* </pre>
*
******************************************************************************/
#ifndef XMESIM_H
#define XMESIM_H

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/
#define XMESIM_TILE_BASE_ADDRESS	0x80000000U
#define XMESIM_TILE_BASE_ADDRMASK	0x3FFFFFFFU

#define XMESIM_TILE_ADDR_COL_SHIFT      23U
#define XMESIM_TILE_ADDR_ROW_SHIFT      18U

#define XMESIM_SAVE_REGSTATE_NUMADDRS   3U

#define XMESIM_CMDIO_CMD_SETSTACK       0U
#define XMESIM_CMDIO_CMD_LOADSYM        1U

#define XMESIM_SUCCESS			0U
#define XMESIM_FAILURE			1U

#ifdef XME_DEBUG
#define XMeSim_print                    printf
#else
#define XMeSim_print                    {}
#endif
#define XMeSim_usleep                   usleep

#define XMeSim_AssertNonvoid(arg)	{}
#define XMeSim_AssertVoid(arg)          {}

#define XMESIM_IO_MODE_ESS		0U
#define XMESIM_IO_MODE_SOCK		1U
#define XMESIM_IO_MODE_CDO		2U

/****************************** Type Definitions *****************************/
typedef unsigned char			uint8;
typedef unsigned int			uint32;
typedef signed int                      sint32;
typedef unsigned long int               uint64_t;

typedef struct {
        uint32 RegAddr;
        uint32 RegVal;
} XMeSim_RegState;

/************************** Function Prototypes  *****************************/
uint32 XMeSim_Read32(uint64_t Addr);
void XMeSim_Read128(uint64_t Addr, uint32 *Data);
void XMeSim_Write32(uint64_t Addr, uint32 Data);
void XMeSim_MaskWrite32(uint64_t Addr, uint32 Mask, uint32 Data);
void XMeSim_Write128(uint64_t Addr, uint32 *Data);
void XMeSim_WriteCmd(uint8 Command, uint8 ColId, uint8 RowId, uint32 CmdWd0, uint32 CmdWd1, uint8 *CmdStr);
void XMeSim_Init(uint8 NumCols, uint8 NumRows);

uint8 XMeSim_SetIOMode(uint8 Mode);

#endif		/* end of protection macro */
/** @} */


