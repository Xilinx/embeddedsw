/******************************************************************************
*
* Copyright (C) 2015 - 2016 Xilinx, Inc.  All rights reserved.
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
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file xhdcp22_common.h
*
* This file contains common functions shared between HDCP22 drivers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00  MH   10/30/15 First Release.
* 1.01  MH   01/15/16 Added prefix to function names.
*</pre>
*
*****************************************************************************/

#ifndef XHDCP22_COMMON_H_
#define XHDCP22_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/
#include "bigdigits.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/

/* Cryptographic functions */
void XHdcp22Cmn_Sha256Hash(const u8 *Data, u32 DataSize, u8 *HashedData);
int  XHdcp22Cmn_HmacSha256Hash(const u8 *Data, int DataSize, const u8 *Key, int KeySize, u8  *HashedData);
void XHdcp22Cmn_Aes128Encrypt(const u8 *Data, const u8 *Key, u8 *Output);
void XHdcp22Cmn_Aes128Decrypt(const u8 *Data, const u8 *Key, u8 *Output);

#ifdef __cplusplus
}
#endif

#endif /* XHDCP22_COMMON_H_ */
