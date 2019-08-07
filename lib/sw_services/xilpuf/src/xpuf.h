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

/*****************************************************************************/
/**
*
* @file xpuf.h
* @addtogroup xpuf_apis XilPuf APIs
* @{
* @cond xpuf_internal
* This file contains PUF interface APIs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.0   kal  08/01/2019 Initial release
*
* </pre>
*
* @note
*
* @endcond
*
******************************************************************************/

#ifndef XPUF_H
#define XPUF_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************** Include Files *********************************/
#include "xil_types.h"
#include "xil_io.h"
#include "xstatus.h"
#include "xpuf_hw.h"
#include "xil_printf.h"

/*************************** Constant Definitions *****************************/
/** @cond xpuf_internal
@{
*/
#if defined XPUF_DEBUG
#define XPUF_DEBUG_GENERAL (1U)
#else
#define XPUF_DEBUG_GENERAL (0U)
#endif

#define xPuf_printf(type,...)	if ((type) == (1U)) {xil_printf (__VA_ARGS__);}

#define XPUF_MAX_SYNDROME_DATA_LEN_IN_WORDS		(350U)
#define XPUF_FORMATTED_4K_SYNDROME_DATA_LEN_IN_WORDS	(140U)
#define XPUF_DBG2_DATA_LEN_IN_BYTES			(36U)
#define XPUF_AES_KEY_LEN_IN_BYTES			(32U)
#define XPUF_AES_KEY_IV_LEN_IN_BYTES			(12U)
#define XPUF_4K_PUF_SYN_LEN_IN_WORDS			(140U)
#define XPUF_4K_PUF_SYN_LEN_IN_BYTES			(560U)
#define XPUF_12K_PUF_SYN_LEN_IN_WORDS			(350U)
#define XPUF_12K_PUF_SYN_LEN_IN_BYTES			(1400U)
#define XPUF_SHUTTER_VALUE				(0x1000040U)
#define XPUF_ID_LENGTH					(0x8U)
#define XPUF_WORD_LENGTH				(0x4U)

#define XPUF_READ_FROM_CACHE				(0x00)
#define XPUF_READ_FROM_EFUSE				(0x01)
#define XPUF_READ_FROM_RAM				(0x02)

#define XPUF_SYNDROME_MODE_4K				(0x0U)
#define XPUF_SYNDROME_MODE_12K				(0x1U)


/* XilPuf API's error codes */
#define XPUF_ERROR_TAG					(0x8200)

/* Key provisioning time error codes */
#define XPUF_ERROR_INVALID_PARAM			(XPUF_ERROR_TAG | 0x01)
#define XPUF_ERROR_INVALID_SYNDROME_MODE		(XPUF_ERROR_TAG | 0x02)
#define XPUF_ERROR_SYNDROME_WORD_WAIT_TIMEOUT		(XPUF_ERROR_TAG | 0x03)
#define XPUF_ERROR_SYNDROME_DATA_OVERFLOW		(XPUF_ERROR_TAG | 0x04)
#define XPUF_ERROR_SYNDROME_DATA_UNDERFLOW		(XPUF_ERROR_TAG | 0x05)
#define XPUF_ERROR_PUF_DONE_WAIT_TIMEOUT		(XPUF_ERROR_TAG | 0x06)

/* Key regeneration time error codes */
#define XPUF_ERROR_CHASH_NOT_PROGRAMMED			(XPUF_ERROR_TAG | 0x10)
#define XPUF_ERROR_PUF_STATUS_DONE_TIMEOUT		(XPUF_ERROR_TAG | 0x11)
#define	XPUF_ERROR_PUF_EFUSE_REGEN_NOT_IMPLEMENTED	(XPUF_ERROR_TAG | 0x12)

/***************************** Type Definitions *******************************/

typedef struct {
	u32 RegMode;		/* PUF Registration Mode 4K/12K*/
	u32 ReadOption;		/* Read Syndrome data from eFuse/Cache/DDR */
	u32 ShutterValue;
	u32 SyndromeData[XPUF_MAX_SYNDROME_DATA_LEN_IN_WORDS];
	u32 Chash;
	u32 Aux;
	u32 PufID[XPUF_ID_LENGTH];
	u32 SyndromeAddr;
}XPuf_Data;

/** @}
@endcond */

/*************************** Function Prototypes ******************************/

u32 XPuf_Puf_Registration(XPuf_Data *PufData);
u32 XPuf_Puf_Regeneration(XPuf_Data *PufData);

#ifdef __cplusplus
}
#endif

#endif  /* XPUF_HW_H */
/**@}*/
