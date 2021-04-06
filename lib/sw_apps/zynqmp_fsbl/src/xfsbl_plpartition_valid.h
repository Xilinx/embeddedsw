/******************************************************************************
* Copyright (c) 2017 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xfsbl_plpartition_valid.h
*
* Contains constant definitions for bitstream authentication.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   vns     01/28/17 First release
* 2.0   vns     11/09/17 In structure XFsblPs_PlPartition added member
*                        (SecureHdr) to store partial secure header when
*                        single secure header is in two chunks, also added
*                        another member(Hdr) to store size of data stored.
* 3.0   vns     03/12/19 Added instance to XSecure_Sss structure in
*                        XFsblPs_PlPartition structure.
* 4.0   har     06/17/20 Removed references to unused algorithms
* 5.0   har     01/16/20 Added checks for Status of SSS configuration when AES
*                        engine is the Resource
* 6.0   bsv     04/01/21 Added TPM support
*
* </pre>
*
******************************************************************************/
#ifndef XFSBL_PLPARTITION_VALID_H_
#define XFSBL_PLPARTITION_VALID_H_	/**< Prevent circular inclusions
					  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xfsbl_authentication.h"
#include "xfsbl_main.h"

/************************** Constant Definitions *****************************/

#define XFSBL_CSU_SSS_SRC_SRC_DMA    0x5U

/**************************** Type Definitions *******************************/
#if defined(XFSBL_SECURE) && defined(XFSBL_BS)
/** @name XFsblPs_PlAuthentication
 * @{
 */
typedef struct {
	u8 AuthType;	/**< Type of Authentication used */
	u8 *AuthCertBuf;/**< Buffer to store authentication certificate */
	u32 AcOfset;	/**< Offset of first authentication certificate
			  *  of bitstream */
	u8 *HashsOfChunks;/** To store hashes of all chunks of block */
	u32 NoOfHashs;	/**< HashsOfChunks buffer size provided */
	u32 BlockSize;	/**< Block size of bitstream */
} XFsblPs_PlAuthentication;
/*@}*/

/** @name XFsblPs_PlEncryption
 * @{
 */
typedef struct {
	XSecure_Aes *SecureAes; /**< AES initialized structure */
	u32 NextBlkLen; /**< Not required for user, used for storing
			  *  next block size */
} XFsblPs_PlEncryption;
/*@}*/

/** @name XFsblPs_PlPartition
 * @{
 */
typedef struct {
	u8 IsAuthenticated;	/**< Authentication flag */
	u8 IsEncrypted;		/**< Encryption flag */
	u64 StartAddress;	/** Start address of the partition */
	u32 UnEncryptLen;	/**< un encrypted length of bitstream */
	u32 TotalLen;		/**< Total partition length */
	u32 ChunkSize;		/**< Chunk size */
	u8 *ChunkBuffer;	/**< Buffer for storing chunk of data */
	XCsuDma *CsuDmaPtr;	/**< Initialized CSUDMA driver's instance */
	u32 (*DeviceCopy) (u32 SrcAddress, UINTPTR DestAddress, u32 Length);
				/**< Device copy for DDR less system */
	XFsblPs_PlEncryption PlEncrypt;	/**< Encryption parameters */
	XFsblPs_PlAuthentication PlAuth;/**< Authentication parameters */
	u8 SecureHdr[XSECURE_SECURE_HDR_SIZE + XSECURE_SECURE_GCM_TAG_SIZE];
	u8 Hdr;
	XSecure_Sss SssInstance;
	u8 *Hash; /**< Pointer to store calculated hash */
} XFsblPs_PlPartition;
/*@}*/

/***************************** Function Prototypes ***************************/
u32 XFsbl_SecPlPartition(XFsblPs * FsblInstancePtr,
			XFsblPs_PlPartition *PartitionParams);

#endif
/******************************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
