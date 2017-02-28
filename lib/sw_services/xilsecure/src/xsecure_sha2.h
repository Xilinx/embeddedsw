/******************************************************************************
*
* Copyright (C) 2016 - 17 Xilinx, Inc.  All rights reserved.
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
* @file xsecure_sha2.h
*
* This file contains the RSA algorithm functions
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who	Date		Changes
* ----- ---- -------- -------------------------------------------------------
* 1.2   vns  23/08/16 First release
*
* </pre>
*
* @note
*
******************************************************************************/
#ifndef ___XSECURE_H___
#define ___XSECURE_H___

#ifdef __cplusplus
extern "C" {
#endif
/***************************** Include Files *********************************/

#define SHA_BLKSIZE		512
#define SHA_BLKBYTES	(SHA_BLKSIZE/8)
#define SHA_BLKWORDS	(SHA_BLKBYTES/4)

#define SHA_VALSIZE		256
#define SHA_VALBYTES	(SHA_VALSIZE/8)
#define SHA_VALWORDS	(SHA_VALBYTES/4)

/*
 * SHA-256 context structure
 * Includes SHA-256 state, coalescing buffer to collect the processed strings, and
 * total byte length counter (used both to manage the buffer and for padding)
 */
typedef struct
{
	unsigned int state[8];
	unsigned char buffer[SHA_BLKBYTES];
	unsigned long long bytes;
} sha2_context;

/*
 * SHA-256 user interfaces
 */
void sha_256(const unsigned char *in, const unsigned int size, unsigned char *out);
void sha2_starts(sha2_context *ctx);
void sha2_update(sha2_context *ctx, unsigned char* input, unsigned int ilen);
void sha2_finish(sha2_context *ctx, unsigned char* output);
void sha2_hash(sha2_context *ctx, unsigned char *output);


#ifdef __cplusplus
}
#endif

#endif /* ___XSECURE_H___ */
