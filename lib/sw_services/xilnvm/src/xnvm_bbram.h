/*******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc. All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*******************************************************************************/

/******************************************************************************/
/**
*
* @file xnvm_bbram.h
*
* This file contains NVM library BBRAM API's declaration
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- --------------------------------------------------------
* 1.0   mmd  04/01/2019 Initial release
*
* </pre>
*
* @note
*
*******************************************************************************/
#ifndef XNVM_BBRAM_H
#define XNVM_BBRAM_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************** Include Files *********************************/
#include "xil_types.h"
#include "xil_io.h"
#include "xstatus.h"
#include "xnvm_utils.h"

/*************************** Constant Definitions *****************************/

/* AES Key size in bytes */
#define XNVM_BBRAM_AES_KEY_SIZE          XNVM_256_BITS_AES_KEY_LEN_IN_BYTES
#define XNVM_BBRAM_AES_KEY_SIZE_IN_WORDS (XNVM_BBRAM_AES_KEY_SIZE / sizeof(u32))

/* BBRAM API error codes */
#define XNVM_EFUSE_BBRAM_TAG                       (0x8100)
#define XNVM_BBRAM_ERROR_PGM_MODE_ENABLE_TIMEOUT   (XNVM_EFUSE_BBRAM_TAG | 0x00)
#define XNVM_BBRAM_ERROR_PGM_MODE_DISABLE_TIMEOUT  (XNVM_EFUSE_BBRAM_TAG | 0x01)
#define XNVM_BBRAM_ERROR_AES_CRC_DONE_TIMEOUT      (XNVM_EFUSE_BBRAM_TAG | 0x02)
#define XNVM_BBRAM_ERROR_AES_CRC_MISMATCH          (XNVM_EFUSE_BBRAM_TAG | 0x03)
#define XNVM_BBRAM_ERROR_LOCK_USR_DATA_WRITE       (XNVM_EFUSE_BBRAM_TAG | 0x04)
#define XNVM_BBRAM_ERROR_USR_DATA_WRITE_LOCKED     (XNVM_EFUSE_BBRAM_TAG | 0x05)
#define XNVM_BBRAM_ERROR_ZEROIZE_TIMEOUT           (XNVM_EFUSE_BBRAM_TAG | 0x06)

/***************************** Type Definitions *******************************/

/*************************** Function Prototypes ******************************/

/* Writes AES key to BBRAM */
u32 XNvm_BbramWriteAesKey(const u8* Key, u16 KeyLen);

/* Locks user data and prevent writes */
u32 XNvm_BbramLockUsrDataWrite();

/* Write 32-bit user data */
u32 XNvm_BbramWriteUsrData(u32 UsrData);

/* Read 32-bit user data */
u32 XNvm_BbramReadUsrData();

/* Zeroize BBRAM memory */
u32 XNvm_BbramZeroize();

#ifdef __cplusplus
}
#endif

#endif
