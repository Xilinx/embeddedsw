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
	u8 AuthType;	/**< Type of Authentication used SHA2/SHA3 */
	u8 *AuthCertBuf;/**< Buffer to store authentication certificate */
	u32 AcOfset;	/**< Offset of first authentication certificate
			  *  of bitstream */
	u8 *HashsOfChunks;/** To store hashs of all chunks of block */
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
