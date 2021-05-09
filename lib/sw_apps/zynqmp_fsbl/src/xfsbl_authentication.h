
/******************************************************************************
* Copyright (c) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
*       vns  02/17/17 Added API prototype to compare hashes
* 3.0   vns  01/23/18 Added prototype for XFsbl_Sha3PadSelect()
*       vns  03/07/18 Added PPK/SPK offsets w.r.t to AC, modified
*                     prototype of XFsbl_CompareHashs()
* 4.0   ka   04/10/18 Added support for user-efuse revocation
* 5.0   bsv  03/11/21 Fixed build issues
*       bsv  04/01/21 Added TPM support
*       bsv  05/03/21 Add provision to load bitstream from OCM with DDR
*                     present in design
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
#include "xsecure_sha.h"
#ifdef XFSBL_SECURE
#include "xsecure_rsa.h"
#include "xsecure_aes.h"
#endif
#include "xcsudma.h"
#include "xparameters.h"
/***************************** Type defines *********************************/
#ifndef XFSBL_HASH_TYPE_SHA3
#define XFSBL_HASH_TYPE_SHA3					(48U)
#endif

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

#define XFSBL_AUTH_CERT_PPK_OFFSET 		0x40U
#define XFSBL_AUTH_CERT_SPK_OFFSET		0x480U
#define XFSBL_AUTH_CERT_SPK_SIG_OFFSET  	0x8C0U

#define XFSBL_AH_ATTR_SPK_ID_FUSE_SEL_MASK		0xC0000U
#define XFSBL_AH_ATTR_SPK_ID_FUSE_SEL_SHIFT		18
#define XFSBL_SPKID_EFUSE						1U /**< SPK ID eFUSE */
#define XFSBL_USER_EFUSE						2U /**< User eFUSE */
#define XFSBL_USER_EFUSE_MIN_VALUE				1U
#define XFSBL_USER_EFUSE_MAX_VALUE				256U
#define XFSBL_WORD_LEN_IN_BYTES					4U
#define XFSBL_WORD_SHIFT						32U
/*User eFuse 0 */
#define XFSBL_USER_EFUSE_ADDR					0xFFCC1020U


/**
* CSU RSA Register Map
*/

void XFsbl_ShaDigest(const u8 *In, const u32 Size, u8 *Out, u32 HashLen);
void XFsbl_ShaFinish(void * Ctx, u8 * Hash, u32 HashLen);
void XFsbl_ShaStart(void * Ctx, u32 HashLen);
void XFsbl_ShaUpdate(void * Ctx, u8 * Data, u32 Size, u32 HashLen);
#ifdef XFSBL_PL_LOAD_FROM_OCM
#ifdef XFSBL_BS
u32 XFsbl_ShaUpdate_DdrLess(const XFsblPs *FsblInstancePtr, void *Ctx,
		u64 PartitionOffset, u32 PartitionLen,
		u32 HashLen, u8 *PartitionHash);
#endif
#endif
#ifdef XFSBL_SECURE
u32 XFsbl_Authentication(const XFsblPs * FsblInstancePtr, u64 PartitionOffset,
				u32 PartitionLen, u64 AcOffset,
				u32 PartitionNum);
u32 XFsbl_CompareHashs(u8 *Hash1, u8 *Hash2, u32 HashLen);
u32 XFsbl_Sha3PadSelect(XSecure_Sha3PadType PadType);
u32 XFsbl_BhAuthentication(const XFsblPs * FsblInstancePtr, u8 *Data,
					u64 AcOffset, u8 IsEfuseRsa);
#endif

extern XCsuDma CsuDma;  /* CSU DMA instance */

#ifdef __cplusplus
}
#endif

#endif /** XFSBL_AUTHENTICATION_H */
