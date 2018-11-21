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
* @file xmegbl_defs.h
* @{
*
* This file contains the generic definitions for the ME drivers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Naresh  03/23/2018  Initial creation
* 1.1  Naresh  07/11/2018  Updated copyright info
* 1.2  Hyun    10/10/2018  Added the mask write API
* </pre>
*
******************************************************************************/
#ifndef XMEGBL_DEFS_H
#define XMEGBL_DEFS_H

/***************************** Include Files *********************************/
#include "xmelib.h"

/************************** Constant Definitions *****************************/
#define XME_SUCCESS			XMELIB_SUCCESS
#define XME_FAILURE			XMELIB_FAILURE
#define XME_COMPONENT_IS_READY		XMELIB_COMPONENT_IS_READY

#define XME_NULL			(void *)0U
#define XME_ENABLE			1U
#define XME_DISABLE			0U
#define XME_RESETENABLE			1U
#define XME_RESETDISABLE		0U

#define XMEGBL_CMDIO_COMMAND_SETSTACK	XMELIB_CMDIO_COMMAND_SETSTACK
#define XMEGBL_CMDIO_COMMAND_LOADSYM	XMELIB_CMDIO_COMMAND_LOADSYM
#define XMEGBL_TILE_BASE_ADDRESS        XMELIB_TILE_BASE_ADDRESS

#define XMe_print			XMeLib_print
#define XMe_usleep			XMeLib_usleep
#define XMe_AssertNonvoid		XMeLib_AssertNonvoid
#define XMe_AssertVoid			XMeLib_AssertVoid

#define XMe_SetField(Val, Lsb, Mask)	(((u32)Val << Lsb) & Mask)
#define XMe_GetField(Val, Lsb, Mask)	(((u32)Val & Mask) >> Lsb)

#define XMeGbl_Read32                   XMeLib_Read32
#define XMeGbl_Read128                  XMeLib_Read128
#define XMeGbl_Write32                  XMeLib_Write32
#define XMeGbl_MaskWrite32              XMeLib_MaskWrite32
#define XMeGbl_Write128                 XMeLib_Write128
#define XMeGbl_WriteCmd                 XMeLib_WriteCmd
#define XMeGbl_LoadElf                  XMeLib_LoadElf

#define XMeGbl_MemInst                  XMeLib_MemInst
#define XMeGbl_MemInit                  XMeLib_MemInit
#define XMeGbl_MemFinish                XMeLib_MemFinish
#define XMeGbl_MemGetSize               XMeLib_MemGetSize
#define XMeGbl_MemGetVaddr              XMeLib_MemGetVaddr
#define XMeGbl_MemGetPaddr              XMeLib_MemGetPaddr
#define XMeGbl_MemRead32                XMeLib_MemRead32
#define XMeGbl_MemWrite32               XMeLib_MemWrite32

/************************** Variable Definitions *****************************/

/************************** Function Prototypes  *****************************/

#endif		/* end of protection macro */
/** @} */


