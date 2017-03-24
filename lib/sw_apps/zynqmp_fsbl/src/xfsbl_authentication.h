/******************************************************************************
*
* Copyright (C) 2015 - 17 Xilinx, Inc.  All rights reserved.
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
*
*******************************************************************************/
/*****************************************************************************/
/**
*
* @file xcbr_authentication.h
*
* Contains the function prototypes, defines and macros for
* the RSA DSA functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   ssc  01/20/16 Initial release
* 2.0   bv   12/05/16 Made compliance to MISRAC 2012 guidelines.
*       vns  02/17/17 Added API prototype to compare hashs
*
* </pre>
*
******************************************************************************/

#ifndef XFSBL_AUTHENTICATION_H
#define XFSBL_AUTHENTICATION_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xfsbl_hw.h"
#include "xfsbl_main.h"
#ifdef XFSBL_SECURE
#include "xsecure_sha.h"
#include "xsecure_rsa.h"
#include "xsecure_aes.h"
#endif
#include "xcsudma.h"
#include "xparameters.h"
#ifdef XFSBL_SHA2
#include "xsecure_sha2.h"
#endif
/***************************** Type defines *********************************/
#define XFSBL_HASH_TYPE_SHA3					(48U)
#define XFSBL_HASH_TYPE_SHA2					(32U)

#define XFSBL_SPK_SIZE						(512U+512U+64U)
#define XFSBL_PPK_SIZE						(u32)(XFSBL_SPK_SIZE)
#define XFSBL_PPK_MOD_SIZE					(512U)
#define XFSBL_PPK_MOD_EXT_SIZE				(512U)
#define XFSBL_SPK_MOD_SIZE				XFSBL_PPK_MOD_SIZE
#define XFSBL_SPK_MOD_EXT_SIZE				XFSBL_PPK_MOD_EXT_SIZE
#define XFSBL_SPK_SIG_SIZE					(512U)
#define XFSBL_BHDR_SIG_SIZE					(512U)
#define XFSBL_FSBL_SIG_SIZE					(512U)
#define XFSBL_RSA_AC_ALIGN					(64U)
#define XFSBL_SPKID_AC_ALIGN					(4U)

#define XFSBL_AUTH_HEADER_SIZE				(u32)(8U)

#define	XFSBL_AUTH_CERT_USER_DATA		((u32)64U - XFSBL_AUTH_HEADER_SIZE)

#define XFSBL_AUTH_CERT_MIN_SIZE	(XFSBL_AUTH_HEADER_SIZE 	\
					+ XFSBL_AUTH_CERT_USER_DATA 	\
					+ XFSBL_PPK_SIZE 		\
					+ XFSBL_SPK_SIZE 		\
					+ XFSBL_SPK_SIG_SIZE 		\
					+ XFSBL_BHDR_SIG_SIZE 		\
					+ XFSBL_FSBL_SIG_SIZE)


#define XFSBL_AUTH_BUFFER_SIZE	(XFSBL_AUTH_CERT_MIN_SIZE)

/**
* CSU RSA Register Map
*/


#ifdef XFSBL_SECURE
u32 XFsbl_Authentication(const XFsblPs * FsblInstancePtr, u64 PartitionOffset,
				u32 PartitionLen, u64 AcOffset,
				u32 PartitionNum);
void XFsbl_ShaDigest(const u8 *In, const u32 Size, u8 *Out, u32 HashLen);
void XFsbl_ShaStart(void * Ctx, u32 HashLen);
void XFsbl_ShaUpdate(void * Ctx, u8 * Data, u32 Size, u32 HashLen);
void XFsbl_ShaFinish(void * Ctx, u8 * Hash, u32 HashLen);
u32 XFsbl_CompareHashs(u8 *Hash1, u8 *Hash2);
#endif


#ifndef XFSBL_PS_DDR
#ifdef XFSBL_BS
u32 XFsbl_ShaUpdate_DdrLess(XFsblPs *FsblInstancePtr, void *Ctx,
		u64 PartitionOffset, u32 PartitionLen,
		u32 HashLen, u8 *PartitionHash);
#endif
#endif
extern XCsuDma CsuDma;  /* CSU DMA instance */

#ifdef __cplusplus
extern "C" }
#endif

#endif /** XFSBL_AUTHENTICATION_H */
