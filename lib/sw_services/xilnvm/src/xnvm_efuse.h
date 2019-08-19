/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/

/******************************************************************************/
/**
*
* @file xnvm_efuse.h
* @addtogroup xnvm_efuse_apis XilNvm eFuse APIs
* @{
*
* @cond xnvm_internal
* This file contains NVM library eFUSE APIs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- ---------- --------------------------------------------------------
* 1.0   kal  08/16/2019 Initial release
*
* </pre>
*
* @note
*
* @endcond
*******************************************************************************/
#ifndef XNVM_EFUSE_H
#define XNVM_EFUSE_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************** Include Files *********************************/
#include "xil_io.h"
#include "xil_types.h"
#include "xstatus.h"

/*************************** Constant Definitions *****************************/

#define XNvm_Printf(type,...)   if ((type) == (1U)) {xil_printf (__VA_ARGS__);}

/* Error codes for eFUSE APIs */
#define XNVM_EFUSE_ERROR_TAG			(0x8000)
#define XNVM_EFUSE_ERROR_INVALID_PARAM		(XNVM_EFUSE_ERROR_TAG | 0x80)
#define XNVM_EFUSE_ERROR_RD			(XNVM_EFUSE_ERROR_TAG | 0x81)
#define XNVM_EFUSE_ERROR_RD_TIMEOUT		(XNVM_EFUSE_ERROR_TAG | 0x82)
#define XNVM_EFUSE_ERROR_CACHE_PARITY		(XNVM_EFUSE_ERROR_TAG | 0x83)
#define XNVM_EFUSE_ERROR_LOCK			(XNVM_EFUSE_ERROR_TAG | 0x84)
#define XNVM_EFUSE_ERROR_UNLOCK			(XNVM_EFUSE_ERROR_TAG | 0x85)
#define XNVM_EFUSE_ERROR_PGM_VERIFY		(XNVM_EFUSE_ERROR_TAG | 0x90)
#define XNVM_EFUSE_ERROR_PGM			(XNVM_EFUSE_ERROR_TAG | 0x91)
#define XNVM_EFUSE_ERROR_PGM_TIMEOUT		(XNVM_EFUSE_ERROR_TAG | 0x92)
#define XNVM_EFUSE_ERROR_PGM_TBIT_PATTERN	(XNVM_EFUSE_ERROR_TAG |	0x93)
#define XNVM_EFUSE_ERROR_CACHE_LOAD		(XNVM_EFUSE_ERROR_TAG | 0x94)

/***************************** Type Definitions *******************************/

typedef enum {
	XNVM_EFUSE_RD_FROM_CACHE,
	XNVM_EFUSE_RD_FROM_EFUSE
} XNvm_EfuseRdOpt;

typedef enum {
	XNVM_EFUSE_PAGE_0,
	XNVM_EFUSE_PAGE_1,
	XNVM_EFUSE_PAGE_2
}XNvm_EfuseType;

/*************************** Function Prototypes ******************************/

/* This functions reads the specfied rows from eFUSE/Cache */
u32 XNvm_EfuseReadRows(XNvm_EfuseRdOpt ReadOption, u8 Row, u8 RowCount,
			XNvm_EfuseType EfuseType, u32* RowData);

/* This function sets the specified bits in the specified rows */
u32 XNvm_EfusePgmRows(u8 Row, u8 RowCount, XNvm_EfuseType EfuseType,
			const u32* RowData);

#ifdef __cplusplus
}
#endif

#endif	/* XNVM_EFUSE_H */

/* @} */
