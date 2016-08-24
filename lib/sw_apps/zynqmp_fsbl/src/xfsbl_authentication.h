/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
#include "xilrsa.h"
#endif
/***************************** Type defines *********************************/
#define XFSBL_HASH_TYPE_SHA3					(48U)
#define XFSBL_HASH_TYPE_SHA2					(32U)

#define XFSBL_PPK_SIZE						(512U+512U+64U)
#define XFSBL_SPK_SIZE						XFSBL_PPK_SIZE
#define XFSBL_PPK_MOD_SIZE					(512U)
#define XFSBL_PPK_MOD_EXT_SIZE				(512U)
#define XFSBL_SPK_MOD_SIZE				XFSBL_PPK_MOD_SIZE
#define XFSBL_SPK_MOD_EXT_SIZE				XFSBL_PPK_MOD_EXT_SIZE
#define XFSBL_SPK_SIG_SIZE					(512U)
#define XFSBL_BHDR_SIG_SIZE					(512U)
#define XFSBL_FSBL_SIG_SIZE					(512U)
#define XFSBL_RSA_KEY_LEN					(4096U)
#define XFSBL_RSA_BIG_ENDIAN					(0x1U)
#define XFSBL_RSA_AC_ALIGN					(64U)

#define XFSBL_AUTH_HEADER_SIZE				(8U)

#define	XFSBL_AUTH_CERT_USER_DATA		(64U - XFSBL_AUTH_HEADER_SIZE)

#define XFSBL_AUTH_CERT_MIN_SIZE	(XFSBL_AUTH_HEADER_SIZE 	\
					+ XFSBL_AUTH_CERT_USER_DATA 	\
					+ XFSBL_PPK_SIZE 		\
					+ XFSBL_SPK_SIZE 		\
					+ XFSBL_SPK_SIG_SIZE 		\
					+ XFSBL_BHDR_SIG_SIZE 		\
					+ XFSBL_FSBL_SIG_SIZE)

#define XFSBL_AUTH_CERT_MAX_SIZE	(XFSBL_AUTH_CERT_MIN_SIZE + 60)

#define XFSBL_PARTIAL_AC_SIZE  (XFSBL_AUTH_CERT_MIN_SIZE - XFSBL_FSBL_SIG_SIZE)

#define XFSBL_AUTH_BUFFER_SIZE	(XFSBL_AUTH_CERT_MIN_SIZE)

/**
* CSU RSA Register Map
*/

#define XFSBL_CSU_RSA_CONTROL_2048	     (0xA0U)
#define XFSBL_CSU_RSA_CONTROL_4096	     (0xC0U)
#define XFSBL_CSU_RSA_CONTROL_DCA         (0x08U)
#define XFSBL_CSU_RSA_CONTROL_NOP         (0x00U)
#define XFSBL_CSU_RSA_CONTROL_EXP         (0x01U)
#define XFSBL_CSU_RSA_CONTROL_EXP_PRE     (0x05U)
#define XFSBL_CSU_RSA_CONTROL_MASK		 (XFSBL_CSU_RSA_CONTROL_4096 \
						+ XFSBL_CSU_RSA_CONTROL_EXP_PRE)

#define XFSBL_CSU_RSA_RAM_EXPO			 (0)
#define XFSBL_CSU_RSA_RAM_MOD			 (1)
#define XFSBL_CSU_RSA_RAM_DIGEST		 (2)
#define XFSBL_CSU_RSA_RAM_SPAD			 (3)
#define XFSBL_CSU_RSA_RAM_RES_Y			 (4)
#define XFSBL_CSU_RSA_RAM_RES_Q			 (5)

#define XFSBL_CSU_RSA_RAM_WORDS			 (6)

/**
* CSU SHA3 Memory Map
*/

#define XFSBL_SHA3_BLOCK_LEN 			 (104)

#define	XFSBL_SHA3_LAST_PACKET			 (0x1)

#ifdef XFSBL_SECURE
u32 XFsbl_Authentication(XFsblPs * FsblInstancePtr, u64 PartitionOffset,
				u32 PartitionLen, u64 AcOffset, u32 HashLen,
				u32 PartitionNum);
u32 XFsbl_PartitionSignVer(XFsblPs * FsblInstancePtr, u64 PartitionOffset,
				u32 PartitionLen, u64 AcOffset, u32 HashLen,
				u32 PartitionNum);
u32 XFsbl_SpkVer(XFsblPs * FsblInstancePtr, u64 AcOffset, u32 HashLen);

u32 XSecure_RsaSignVerification(u8 *Signature, u8 *Hash, u32 HashLen);

void XFsbl_ShaDigest(const u8 *In, const u32 Size, u8 *Out, u32 HashLen);
void XFsbl_ShaStart(void * Ctx, u32 HashLen);
void XFsbl_ShaUpdate(void * Ctx, u8 * Data, u32 Size, u32 HashLen);
void XFsbl_ShaFinish(void * Ctx, u8 * Hash, u32 HashLen);
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
