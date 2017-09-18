/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
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
*
* @file xsecure.h
*
* This is the header file which contains secure library interface function prototype
* for authentication and decryption of images.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  dp   02/15/17 Initial release
* 2.2   vns  09/18/17 Added APIs to support generic functionality
*                     for SHA3 and RSA hardware at linux level.
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XSECURE_H
#define XSECURE_H

/************************** Include Files ***********************************/

/************************** Constant Definitions *****************************/

#define XSECURE_AES		1
#define XSECURE_RSA		2
#define XSECURE_RSA_AES		3

#define XSECURE_SHA3_INIT	1
#define XSECURE_SHA3_UPDATE	2
#define XSECURE_SHA3_FINAL	4
#define XSECURE_SHA3_MASK	(XSECURE_SHA3_INIT | \
				XSECURE_SHA3_UPDATE | XSECURE_SHA3_FINAL)

/* 0th bit of flag tells about encryption or decryption */
#define XSECURE_ENC		1
#define XSECURE_DEC		0

#define XSECURE_RSA_OPERATION	1
#define XSECURE_RSA_KEY_SELECT  2

#define XSECURE_MASK		(XSECURE_AES | XSECURE_RSA)

#define XSECURE_KEY_STR_LEN		64	/* String length */
#define XSECURE_IV_STR_LEN		24	/* String length */
#define XSECURE_KEY_LEN			8
#define XSECURE_IV_LEN			3
#define XSECURE_GCM_TAG_LEN		128
#define XSECURE_WORD_LEN		4
#define XSECURE_MAX_NIBBLES		8

#define XSECURE_ARRAY_LENGTH(array) (sizeof((array))/sizeof((array)[0]))

#define XSECURE_ERROR_CSUDMA_INIT_FAIL	0x1
#define XSECURE_STRING_INVALID_ERROR	0x2
#define XSECURE_INVALID_FLAG		0x3
#define XSECURE_AUTH_FAIL		0x4
#define XSECURE_SHA3_INIT_FAIL		0x5
#define XSECURE_SIZE_ERR		0x6

#define XSECURE_CSUDMA_DEVICEID		0

#define XSECURE_MOD_LEN			512

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

u32 XSecure_RsaAes(u32 SrcAddrHigh, u32 SrcAddrLow, u32 WrSize, u32 flags);
u32 XSecure_Sha3Hash(u32 SrcAddrHigh, u32 SrcAddrLow, u32 SrcSize, u32 Flags);
u32 XSecure_RsaCore(u32 SrcAddrHigh, u32 SrcAddrLow, u32 SrcSize, u32 Flags);

/************************** Variable Definitions *****************************/

#endif  /* XSECURE_HW_H */
