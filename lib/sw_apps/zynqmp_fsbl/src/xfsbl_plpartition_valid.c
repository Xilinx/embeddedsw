/******************************************************************************
*
* Copyright (C) 2017 - 18 Xilinx, Inc.  All rights reserved.
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
*******************************************************************************/
/*****************************************************************************/
/**
*
* @file xfsbl_plpartition_valid.c
*
* Contains the function definitions of bitstream authentication in blocks.
* As we need to decrypt the authenticated data only and for secure reasons data
* should not be on external memory authenticated data bitstream of MBs cannot
* be stored in OCM as we have limited OCM memory.
* In boot image bitstream is placed in blocks of size 8MB so bitstream will be
* authenticated in blocks by copying each time 32KB chunk to OCM, and each
* chunk's hash will be stored in OCM buffer for the current block.
* Once authentication of the block is completed successfully, again each chunk
* of the block will be read from external memory and calculates hash on chunk
* and compares with the stored hash. If matched each chunk will be sent to AES
* if decryption exists or to PCAP throught CSUDMA.
* The above process will be repeated for all the blocks of bitstream.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   vns     01/28/17 First release
* 2.0   vns     11/09/17 Modified recursive function call to while loop,
*                        added code for handling the decryption of bitstream
*                        when secure header of block is divided in two chunks,
*                        by copying partial secure header to a buffer and then
*                        processing it along with next chunk of data where it
*                        holds remaining secure header.
* 3.0   vns     01/03/18 Modified XFsbl_DecrptPlChunks() API, to use key IV
*                        from secure header to decrypt the secure bitstream.
*       vns     01/23/18 Removed SSS switch configuring for every SHA3 update
*                        as now library is configuring switch before every DMA
*                        transfer.
*       vns     03/07/18 Removed PPK verification for bitstream partition
*                        as APIs are modified to verify SPK with already
*                        verified PPK
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xfsbl_plpartition_valid.h"
#include "xtime_l.h"

/************************** Constant Definitions ****************************/

#define XFSBL_AES_TAG_SIZE	(XSECURE_SECURE_HDR_SIZE + \
		XSECURE_SECURE_GCM_TAG_SIZE) /**< AES block decryption tag size */

/************************** Function Prototypes ******************************/
#if defined (XFSBL_BS) && defined (XFSBL_SECURE)
extern u32 XFsbl_SpkVer(u64 AcOffset, u32 HashLen);
extern u32 XFsbl_AdmaCopy(void * DestPtr, void * SrcPtr, u32 Size);
extern u32 XFsbl_PcapWaitForDone(void);
static u32 XFsbl_DecrptPl(XFsblPs_PlPartition *PartitionParams,
				u64 ChunkAdrs, u32 ChunkSize);
static u32 XFsbl_DecrptPlChunks(XFsblPs_PlPartition *PartitionParams,
				u64 ChunkAdrs, u32 ChunkSize);
static u32 XFsbl_PlBlockAuthentication(XFsblPs * FsblInstancePtr,
			XFsblPs_PlPartition *PartitionParams,
			UINTPTR SrcAddress, u32 Length, u8 *AuthCer);
static void XFsbl_DmaPlCopy(XCsuDma *InstancePtr, UINTPTR Src,
		u32 Size, u8 EnLast);
static u32 XFsbl_CopyData(XFsblPs_PlPartition *PartitionPtr,
		u8 *DstPtr, u8 *SrcPtr, u32 Size);
static u32 XFsbl_DecrypSecureHdr(XSecure_Aes *InstancePtr, u64 SrcAddr,
		u32 Size);
static u32 XFsbl_ReAuthenticationBlock(XFsblPs_PlPartition *PartitionParams,
				UINTPTR Address, u32 BlockLen, u32 NoOfChunks);
static u32 XFsbl_PlSignVer(XFsblPs_PlPartition *PartitionParams,
		UINTPTR BlockAdrs, u32 BlockSize, u8 *AcOffset, u32 NoOfChunks);
static u32 XFsbl_DecrptSetUpNextBlk(XFsblPs_PlPartition *PartitionParams,
		UINTPTR ChunkAdrs, u32 ChunkSize);

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* This function authenticates the bitstream in blocks. Sends the data to PCAP
* in blocks via AES engine if encryption exists or directly to PCAP by CSUDMA
* if an encryption is not enabled.
*
* @param	PartitionParams is pointer to XFsblPs_PlPartition structure
* 		which has to be initialized by required parameters.
*
* @return
* 		- XFSBL_SUCCESS on success
* 		- Returns error code on failure
*
* @note		Currently SHA2 is not been supported but gave option in
* 		structure and will be used later
*
******************************************************************************/
u32 XFsbl_SecPlPartition(XFsblPs * FsblInstancePtr,
			XFsblPs_PlPartition *PartitionParams)
{
	u32 Status = XFSBL_SUCCESS;
	u8 Index;
	u32 Len;
	UINTPTR SrcAddress = (u64)PartitionParams->StartAddress;
	UINTPTR CurrentAcOffset = PartitionParams->PlAuth.AcOfset;
	u8 IsLastBlock = FALSE;
	u32 RegVal;

	if (PartitionParams->IsAuthenticated != TRUE) {
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_SECURE_NOT_ENABLED"
				" for PL partition\r\n");
		Status = XFSBL_ERROR_SECURE_NOT_ENABLED;
		goto END;
	}

	/* AES initialization expects IV in required format */
	if (PartitionParams->IsEncrypted == TRUE) {
		PartitionParams->PlEncrypt.NextBlkLen = 0;
		if (PartitionParams->PlEncrypt.SecureAes == NULL) {
			XFsbl_Printf(DEBUG_GENERAL,
				"XFSBL_ERROR_SECURE_NOT_ENABLED"
					" for PL partition \r\n");
			Status = XFSBL_ERROR_SECURE_NOT_ENABLED;
			goto END;
		}
		if ((PartitionParams->PlEncrypt.SecureAes->KeySel
				== XSECURE_CSU_AES_KEY_SRC_KUP) &&
			(PartitionParams->PlEncrypt.SecureAes->Key == NULL)) {
			XFsbl_Printf(DEBUG_GENERAL,
				"KUP key is not been provided"
					" for PL partition\r\n");
			Status = XFSBL_FAILURE;
			goto END;
		}
	}

	/* Enable Simple DMA Mode for ADMA channel 0 */
	RegVal = XFsbl_In32(ADMA_CH0_ZDMA_CH_CTRL0);
	RegVal &= (ADMA_CH0_ZDMA_CH_CTRL0_POINT_TYPE_MASK |
			ADMA_CH0_ZDMA_CH_CTRL0_MODE_MASK);
	XFsbl_Out32(ADMA_CH0_ZDMA_CH_CTRL0, RegVal);

	Xil_DCacheDisable();
	/* Loop for traversing all blocks */
	for (Len = PartitionParams->PlAuth.BlockSize, Index = 1;
			SrcAddress < PartitionParams->PlAuth.AcOfset; Index++) {
		Status = XFsbl_CopyData(PartitionParams,
			PartitionParams->PlAuth.AuthCertBuf,
			(u8 *)CurrentAcOffset, XFSBL_AUTH_CERT_MIN_SIZE);
		if (Status != XFSBL_SUCCESS) {
			XFsbl_Printf(DEBUG_GENERAL,
			"Copy of chunk from flash/DDR to OCM failed \r\n");
			return Status;
		}

		/*
		 * If the block start address + block size exceeds
		 * first AC address it is last block
		 */
		if (SrcAddress + PartitionParams->PlAuth.BlockSize >
					PartitionParams->PlAuth.AcOfset) {
			/*
			 * Data placed in last block might not be full block size
			 * TotalLen -
			 * (NoofBlocks)*(AC size) - (NoOfBlocks - 1)*BlockSize
			 */
			Len = (PartitionParams->TotalLen) -
				(Index) * (XFSBL_AUTH_CERT_MIN_SIZE) -
				(Index -1)*(PartitionParams->PlAuth.BlockSize);
			IsLastBlock = TRUE;
		}

		Status = XFsbl_PlBlockAuthentication(FsblInstancePtr,
				PartitionParams, SrcAddress, Len,
				(u8 *)PartitionParams->PlAuth.AuthCertBuf);
		if (Status != XFSBL_SUCCESS) {
			return Status;
		}

		if (IsLastBlock == FALSE) {
			CurrentAcOffset =
				CurrentAcOffset + XFSBL_AUTH_CERT_MIN_SIZE;
			SrcAddress =
				SrcAddress + PartitionParams->PlAuth.BlockSize;
		}
		else {
			/* Completed last block of bitstream */
			break;
		}
	}
	Xil_DCacheEnable();
#ifdef XFSBL_PS_DDR
	/* Restore reset values for the DMA registers used */
	XFsbl_Out32(ADMA_CH0_ZDMA_CH_CTRL0, 0x00000080U);
	XFsbl_Out32(ADMA_CH0_ZDMA_CH_DST_DSCR_WORD0, 0x00000000U);
	XFsbl_Out32(ADMA_CH0_ZDMA_CH_DST_DSCR_WORD1, 0x00000000U);
	XFsbl_Out32(ADMA_CH0_ZDMA_CH_DST_DSCR_WORD2, 0x00000000U);
	XFsbl_Out32(ADMA_CH0_ZDMA_CH_DST_DSCR_WORD3, 0x00000000U);
	XFsbl_Out32(ADMA_CH0_ZDMA_CH_SRC_DSCR_WORD0, 0x00000000U);
	XFsbl_Out32(ADMA_CH0_ZDMA_CH_SRC_DSCR_WORD1, 0x00000000U);
	XFsbl_Out32(ADMA_CH0_ZDMA_CH_SRC_DSCR_WORD2, 0x00000000U);
	XFsbl_Out32(ADMA_CH0_ZDMA_CH_SRC_DSCR_WORD3, 0x00000000U);
#endif

END:

	return Status;

}

/******************************************************************************
*
* This function performs authentication and reauthentication of block
*
* @param	PartitionParams is a pointer to XFsblPs_PlPartition
* @param	SrcAddress holds the start address of block
* @param	Length of the block
* @param	AuthCer holds authentication certificate.
*
* @return
* 		- Error code on failure
* 		- Success on success
*
* @note		None.
*
******************************************************************************/
static u32 XFsbl_PlBlockAuthentication(XFsblPs * FsblInstancePtr,
		XFsblPs_PlPartition *PartitionParams,
		UINTPTR SrcAddress, u32 Length, u8 *AuthCer)
{
	u32 Status;
	u32 NoOfChunks;

	if (Length > PartitionParams->ChunkSize) {
		NoOfChunks = Length/PartitionParams->ChunkSize;
		if (Length%PartitionParams->ChunkSize > 0) {
			NoOfChunks++;
		}
	}
	else {
		/* If length of block equal to Chunk Size */
		NoOfChunks = 1;
	}

	/* Check for Hash storage buffer availability */
	if (NoOfChunks > PartitionParams->PlAuth.NoOfHashs) {
		XFsbl_Printf(DEBUG_GENERAL,
			"XFsbl_PlPartition:"
			"XFSBL_ERROR_PROVIDED_BUF_HASH_STORE Required "
			"hashs = %d \t provided = %d\r\n", NoOfChunks,
			PartitionParams->PlAuth.NoOfHashs);
		return XFSBL_ERROR_PROVIDED_BUF_HASH_STORE;
	}

	/* Do SPK Signature verification using PPK */
	Status = XFsbl_SpkVer((UINTPTR)AuthCer, PartitionParams->PlAuth.AuthType);
	if (XFSBL_SUCCESS != Status) {
		goto END;
	}

	/*
	 * Do Partition Signature verification
	 * of block in chunks and store each chunk's hash
	 */
	Status = XFsbl_PlSignVer(PartitionParams, SrcAddress,
				Length, AuthCer, NoOfChunks);
	if (XFSBL_SUCCESS != Status) {
	 goto END;
	}

	/*
	 * Re-Authentication of block calculates the hash on
	 * chunks and compares with stored hashs
	 * If decryption is enabled data is been sent to AES
	 * and if decryption is disabled
	 * data is written to PCAP with CSU DMA
	 */
	Status = XFsbl_ReAuthenticationBlock(PartitionParams, SrcAddress,
				Length, NoOfChunks);
	if (Status != XFSBL_SUCCESS) {
		goto END;
	}

END:

	return Status;

}

/******************************************************************************
*
* This function re-authenticates the each chunk of the block and compares
* with the stored hash and sends the data AES engine if encryption exists
* and to PCAP directly in encryption is not existing.
*
* @param	PartitionParams is a pointer to XFsblPs_PlPartition
* @param	Address start address of the authentication block.
* @param	BlockLen size of the authentication block.
* @param	NoOfChunks holds the no of chunks for the provided block
*
* @return
* 		Error code on failure
* 		XFSBL_SUCESS on success
*
* @note		None.
*
******************************************************************************/
static u32 XFsbl_ReAuthenticationBlock(XFsblPs_PlPartition *PartitionParams,
				UINTPTR Address, u32 BlockLen, u32 NoOfChunks)
{
	u32 Status;
	u32 Index;
	u32 Len = PartitionParams->ChunkSize;;
	UINTPTR Offset;
	u8 ChunksHash[48];
	XSecure_Sha3 SecureSha3;
	u32 HashDataLen = BlockLen;
	u8 *HashStored = PartitionParams->PlAuth.HashsOfChunks;
	(void)memset(ChunksHash,0U,sizeof(ChunksHash));

	Status = XSecure_Sha3Initialize(&SecureSha3,
			PartitionParams->CsuDmaPtr);
	if (Status != XFSBL_SUCCESS) {
		return Status;
	}
	XSecure_Sha3Start(&SecureSha3);

	/* calculating hashs for all chunks copies to AES/PCAP */
	for (Index = 0; Index < NoOfChunks; Index++) {
			/* Last chunk */
			if (Index == NoOfChunks -1) {
				Len = HashDataLen;
			}
			Offset = (UINTPTR)Address +
				(u64)(PartitionParams->ChunkSize * Index);

			/* Copy from DDR or flash to Buffer */
			Status = XFsbl_CopyData(PartitionParams,
						PartitionParams->ChunkBuffer,
							(u8 *)Offset, Len);
			if (Status != XFSBL_SUCCESS) {
				return Status;
			}
			/* Calculating hash for each chunk */
			XSecure_Sha3Update(&SecureSha3,
				PartitionParams->ChunkBuffer, Len);
			XSecure_Sha3_ReadHash(&SecureSha3, (u8 *)ChunksHash);

			/* Comparing with stored Hashs */
			Status = XFsbl_CompareHashs(HashStored, ChunksHash,
					PartitionParams->PlAuth.AuthType);
			if (Status != XFSBL_SUCCESS) {
				XFsbl_Printf(DEBUG_GENERAL,
					"XFsbl_PlReAuth:"
				" XFSBL_ERROR_CHUNK_HASH_COMPARISON\r\n");
				XFsbl_PrintArray(DEBUG_INFO, HashStored,
					PartitionParams->PlAuth.AuthType,
					"Stored Chunk Hash");
				XFsbl_PrintArray(DEBUG_INFO, ChunksHash,
					PartitionParams->PlAuth.AuthType,
					"Calculated chunk Hash");
				Status = XFSBL_ERROR_CHUNK_HASH_COMPARISON;
				goto END;
			}
			/* Remaining block size will be in HashDataLen */
			HashDataLen = HashDataLen - PartitionParams->ChunkSize;
			if (Index+1 <= NoOfChunks - 1) {
				HashStored = HashStored +
					PartitionParams->PlAuth.AuthType;
			}

			/* If image is not encrypted */
			if (PartitionParams->IsEncrypted == FALSE) {
				/* Configure Secure stream swith to PCAP */
				XFsbl_Out32(CSU_CSU_SSS_CFG,
					XFSBL_CSU_SSS_SRC_SRC_DMA <<
					CSU_CSU_SSS_CFG_PCAP_SSS_SHIFT);

				/* Copy bitstream to PCAP */
				XFsbl_DmaPlCopy(PartitionParams->CsuDmaPtr,
					(UINTPTR)PartitionParams->ChunkBuffer,
						Len/4, 0);

				Status = XFsbl_PcapWaitForDone();
				if (Status != XFSBL_SUCCESS) {
					goto END;
				}
			}
			/* If image is encrypted */
			else {
				Status = XFsbl_DecrptPlChunks(PartitionParams,
					(UINTPTR)PartitionParams->ChunkBuffer, Len);
				if (Status != XFSBL_SUCCESS) {
					goto END;
				}
			}
		}

END:
	return Status;

}

/******************************************************************************
*
* This function performs authentication and RSA signature verification for
* each block.
*
* @param	PartitionParams is a pointer to XFsblPs_PlPartition
* @param	BlockAdrs is the block start address
* @param	BlockSize is the block size
* @param	AcOffset holds authentication certificate's address
* @param	NoOfChunks for the provided block
*
* @return
* 		Error code on failure
* 		XFSBL_SUCESS on success
*
* @note		None
*
******************************************************************************/
static u32 XFsbl_PlSignVer(XFsblPs_PlPartition *PartitionParams,
		UINTPTR BlockAdrs, u32 BlockSize, u8 *AcOffset, u32 NoOfChunks)
{

	u8 PartitionHash[XFSBL_HASH_TYPE_SHA3]={0U};
	u8 * SpkModular;
	u8* SpkModularEx;
	u32 SpkExp;
	u8 * AcPtr = (u8*)AcOffset;
	u32 Status;
	u32 HashDataLen = BlockSize;
	u8 XFsbl_RsaSha3Array[512] = {0U};
	s32 SStatus;
	u8 *ChunksHash = PartitionParams->PlAuth.HashsOfChunks;
	XSecure_Sha3 SecureSha3={0U};
	XSecure_Rsa SecureRsa={0U};
	u32 Index;
	u32 Len = PartitionParams->ChunkSize;
	u64 Offset;

	/* Start the SHA engine */
	if (XSECURE_HASH_TYPE_SHA3 == PartitionParams->PlAuth.AuthType) {
		XCsuDma_Configure ConfigurValues = {0};
		/* this needs to be modified at encryption */
		XCsuDma_GetConfig(PartitionParams->CsuDmaPtr,
			XCSUDMA_SRC_CHANNEL, &ConfigurValues);

		ConfigurValues.EndianType = 0U;

		XCsuDma_SetConfig(PartitionParams->CsuDmaPtr,
			XCSUDMA_SRC_CHANNEL, &ConfigurValues);
		(void)XSecure_Sha3Initialize(&SecureSha3,
			PartitionParams->CsuDmaPtr);

		XSecure_Sha3Start(&SecureSha3);
	}

	/* SHA calculation */
	for (Index = 0; Index < NoOfChunks; Index++) {
		/*
		 * If the block is the last it may not be complete block size
		 * mentioned so will break if it is less than chunk size
		 */
		if ((Index == NoOfChunks-1) ||
			(HashDataLen < PartitionParams->ChunkSize)) {
			Len = HashDataLen;
		}
		Offset = (u64)BlockAdrs +
			(u64)(PartitionParams->ChunkSize * Index);

		Status = XFsbl_CopyData(PartitionParams,
			PartitionParams->ChunkBuffer, (u8 *)(UINTPTR)Offset, Len);
		if (Status != XFSBL_SUCCESS) {
			return Status;
		}

		XSecure_Sha3Update(&SecureSha3,
			PartitionParams->ChunkBuffer, Len);
		XSecure_Sha3_ReadHash(&SecureSha3,
				(u8 *)ChunksHash);

		if (Index+1 < NoOfChunks) {
			HashDataLen =
				HashDataLen - PartitionParams->ChunkSize;
			ChunksHash =
				ChunksHash + PartitionParams->PlAuth.AuthType;
		}

	}

	/* Calculate hash for (AC - signature size) */
	XSecure_Sha3Update(&SecureSha3, (u8 *)AcOffset,
		(XFSBL_AUTH_CERT_MIN_SIZE - XFSBL_FSBL_SIG_SIZE));
	XSecure_Sha3Finish(&SecureSha3, (u8 *)PartitionHash);

	/* Set SPK pointer */
	AcPtr += (XFSBL_RSA_AC_ALIGN + XFSBL_PPK_SIZE);
	SpkModular = AcPtr;
	AcPtr += XFSBL_SPK_MOD_SIZE;
	SpkModularEx = AcPtr;
	AcPtr += XFSBL_SPK_MOD_EXT_SIZE;
	SpkExp = *((u32 *)AcPtr);
	AcPtr += XFSBL_RSA_AC_ALIGN;

	/* Increment by  SPK Signature pointer */
	AcPtr += XFSBL_SPK_SIG_SIZE;
	/* Increment by  BHDR Signature pointer */
	AcPtr += XFSBL_BHDR_SIG_SIZE;
	if((SpkModular != NULL) && (SpkModularEx != NULL)) {
		XFsbl_Printf(DEBUG_DETAILED,
		"XFsbl_PartVer: Spk Mod %0x, Spk Mod Ex %0x, Spk Exp %0x\r\n",
		SpkModular, SpkModularEx, SpkExp);
	}

	SStatus = XSecure_RsaInitialize(&SecureRsa, SpkModular,
				SpkModularEx, (u8 *)&SpkExp);
	if (SStatus != XFSBL_SUCCESS) {
		Status = XFSBL_ERROR_RSA_INITIALIZE;
		XFsbl_Printf(DEBUG_INFO,
		"XFSBL_ERROR_RSA_INITIALIZE at PL verification\r\n");
		goto END;
	}
	/* Decrypt Partition Signature. */
	if(XFSBL_SUCCESS !=
		XSecure_RsaDecrypt(&SecureRsa, AcPtr, XFsbl_RsaSha3Array)) {
		XFsbl_Printf(DEBUG_INFO, "XFsbl_SpkVer: "
			"XFSBL_ERROR_PART_RSA_DECRYPT at PL verification\r\n");
		Status = XFSBL_ERROR_PART_RSA_DECRYPT;
		goto END;
	}


	/* Authenticate Partition Signature */
	if(XFSBL_SUCCESS != XSecure_RsaSignVerification(XFsbl_RsaSha3Array,
			PartitionHash, PartitionParams->PlAuth.AuthType))
	{
		XFsbl_PrintArray(DEBUG_INFO, PartitionHash,
			PartitionParams->PlAuth.AuthType,
			"Calculated Partition Hash at PL verification");
		XFsbl_PrintArray(DEBUG_INFO, XFsbl_RsaSha3Array,
			512, "RSA decrypted Hash at PL verification");
		Status = XFSBL_FAILURE;
		goto END;
	}

	Status = XFSBL_SUCCESS;

END:
	return Status;
}

/******************************************************************************
*
* This function is used to copy data to AES/PL.
*
* @param	InstancePtr is an instance of CSUDMA
* @param	Src holds the source Address
* @param	Size of the data
* @param	EnLast - 0 or 1
*
* @return	None
*
* @note		None
*
******************************************************************************/
static void XFsbl_DmaPlCopy(XCsuDma *InstancePtr, UINTPTR Src, u32 Size,
			u8 EnLast)
{

	/* Data transfer */
	XCsuDma_Transfer(InstancePtr, XCSUDMA_SRC_CHANNEL, (UINTPTR)Src,
							Size, EnLast);
	/* Polling for transfer to be done */
	XCsuDma_WaitForDone(InstancePtr, XCSUDMA_SRC_CHANNEL);
	/* To acknowledge the transfer has completed */
	XCsuDma_IntrClear(InstancePtr, XCSUDMA_SRC_CHANNEL,
					XCSUDMA_IXR_DONE_MASK);

}

/*****************************************************************************
*
* This function copies the data from DDR/flash to OCM.
* For DDR systems uses ADMA and for DDR-less uses devices DMA copy
*
* @param	PartitionPtr is the pointer to XFsblPs_PlPartition
* @param	DstPtr holds destination address.
* @param	SrcPtr holds source address
* @param	Size of the data to be copied
*
* @return
* 		- Success on successful data transfer
* 		- Error on failure
*
* @note		None
*
******************************************************************************/
static u32 XFsbl_CopyData(XFsblPs_PlPartition *PartitionPtr,
					u8 *DstPtr, u8 *SrcPtr, u32 Size)
{
	u32 *Dst = (u32 *)DstPtr;
	u32 *Src = (u32 *)SrcPtr;
	u32 Status;

	if (PartitionPtr->DeviceCopy == NULL) {
		Status = XFsbl_AdmaCopy(Dst, Src, Size);
		if (Status != XFSBL_SUCCESS) {
			goto END;
		}

	}
	else {
		Status = PartitionPtr->DeviceCopy((UINTPTR)Src,
				(UINTPTR)(Dst), Size);
		if (Status != XFSBL_SUCCESS) {
			goto END;
		}
	}
END:
	return Status;
}

/******************************************************************************
*
* This function decrypts the secure header when key rolling is enabled
*
* @param	InstancePtr is an instance AES engine.
* @param	SrcAddr holds the address of secure header
* @param	Size holds size
*
* @return
* 		Error code on failure
* 		XFSBL_SUCESS on success
*
* @note		None
*
******************************************************************************/
static u32 XFsbl_DecrypSecureHdr(XSecure_Aes *InstancePtr, u64 SrcAddr,
								u32 Size)
{
	XCsuDma_Configure ConfigurValues = {0};
	u32 GcmStatus;

	XCsuDma_GetConfig(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
							&ConfigurValues);
	ConfigurValues.EndianType = 1U;
	XCsuDma_SetConfig(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
							&ConfigurValues);

	/*
	 * Push secure header before that configure to
	 * push IV and key to csu engine
	 */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_CSU_AES_KUP_WR_OFFSET,
			XSECURE_CSU_AES_IV_WR | XSECURE_CSU_AES_KUP_WR);
	XCsuDma_IntrClear(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
						XCSUDMA_IXR_DONE_MASK);
	/* PUSH Secure hdr */
	XFsbl_DmaPlCopy(InstancePtr->CsuDmaPtr, SrcAddr,
			XSECURE_SECURE_HDR_SIZE/4, 1);

	/* Restore Key write register to 0. */
	XSecure_WriteReg(InstancePtr->BaseAddress,
					XSECURE_CSU_AES_KUP_WR_OFFSET, 0x0);
	/* Push the GCM tag. */
	XFsbl_DmaPlCopy(InstancePtr->CsuDmaPtr,
		SrcAddr + XSECURE_SECURE_HDR_SIZE,
		XSECURE_SECURE_GCM_TAG_SIZE/4, 1);

	/* Disable CSU DMA Src channel for byte swapping. */
	XCsuDma_GetConfig(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
							&ConfigurValues);
	ConfigurValues.EndianType = 0U;
	XCsuDma_SetConfig(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
							&ConfigurValues);

	XSecure_PcapWaitForDone();

	XSecure_AesWaitForDone(InstancePtr);
	/* Get the AES status to know if GCM check passed. */
	GcmStatus = XSecure_ReadReg(InstancePtr->BaseAddress,
				XSECURE_CSU_AES_STS_OFFSET) &
				XSECURE_CSU_AES_STS_GCM_TAG_OK;

	if (GcmStatus == 0) {
		XFsbl_Printf(DEBUG_GENERAL,
			"XFSBL_DECRYPT:"
			"XFSBL_ERROR_BITSTREAM_GCM_TAG_MISMATCH\r\n");
		return XFSBL_ERROR_BITSTREAM_GCM_TAG_MISMATCH;
	}

	return XFSBL_SUCCESS;
}

/******************************************************************************
*
* This function calculates the next block size and updates the required
* parameters.
*
* @param	PartitionParams is a pointer to XFsblPs_PlPartition
* @param	ChunkAdrs is a pointer to the data location
* @param	ChunkSize is the remaining chunk size
*
* @return
* 		Error code on failure
* 		XFSBL_SUCESS on success
*
* @note		None
*
******************************************************************************/
static u32 XFsbl_DecrptSetUpNextBlk(XFsblPs_PlPartition *PartitionParams,
		UINTPTR ChunkAdrs, u32 ChunkSize)
{
	u32 Status = XFSBL_SUCCESS;
	u32 SssAes;
	u32 SssCfg;

	/* Length of next block */
	PartitionParams->PlEncrypt.NextBlkLen =
			Xil_Htonl(XSecure_ReadReg(
			PartitionParams->PlEncrypt.SecureAes->BaseAddress,
				XSECURE_CSU_AES_IV_3_OFFSET)) * 4;
	PartitionParams->PlEncrypt.SecureAes->Iv =
		(u32 *)(PartitionParams->PlEncrypt.SecureAes->BaseAddress +
			(UINTPTR)XSECURE_CSU_AES_IV_0_OFFSET);

	/* Configure the SSS for AES. */
	SssAes = XSecure_SssInputAes(XSECURE_CSU_SSS_SRC_SRC_DMA);
	SssCfg = SssAes | XSecure_SssInputPcap(XSECURE_CSU_SSS_SRC_AES);
	XSecure_SssSetup(SssCfg);

	/* Start the message. */
	XSecure_WriteReg(PartitionParams->PlEncrypt.SecureAes->BaseAddress,
				XSECURE_CSU_AES_START_MSG_OFFSET,
				XSECURE_CSU_AES_START_MSG);

	/* Transfer IV of the next block */
	XFsbl_DmaPlCopy(PartitionParams->PlEncrypt.SecureAes->CsuDmaPtr,
			(UINTPTR)PartitionParams->PlEncrypt.SecureAes->Iv,
					XSECURE_SECURE_GCM_TAG_SIZE/4, 0);

	PartitionParams->PlEncrypt.SecureAes->SizeofData =
				PartitionParams->PlEncrypt.NextBlkLen;

	XSecure_WriteReg(PartitionParams->PlEncrypt.SecureAes->BaseAddress,
					XSECURE_CSU_AES_KUP_WR_OFFSET, 0x0);


	return Status;

}

/******************************************************************************
*
* This function sends data to AES engine which needs to be decrypted till the
* end of the encryption block.
*
* @param	PartitionParams is a pointer to XFsblPs_PlPartition
* @param	ChunkAdrs is a pointer to the data location
* @param	ChunkSize is the remaining chunk size
*
* @return
* 			Error code on failure
* 			XFSBL_SUCESS on success
*
* @note     None
*
******************************************************************************/
static u32 XFsbl_DecrptPl(XFsblPs_PlPartition *PartitionParams,
					u64 ChunkAdrs, u32 ChunkSize)
{

	u32 Size = ChunkSize;
	u32 Status = XFSBL_SUCCESS;
	u64 SrcAddr = (u64)ChunkAdrs;
	XCsuDma_Configure ConfigurValues = {0};
	UINTPTR NextBlkAddr = 0;
	u32 SssAes;
	u32 SssCfg;

	do {

		/* Enable byte swapping */
		XCsuDma_GetConfig(
			PartitionParams->PlEncrypt.SecureAes->CsuDmaPtr,
					XCSUDMA_SRC_CHANNEL, &ConfigurValues);
		ConfigurValues.EndianType = 1U;
		XCsuDma_SetConfig(
			PartitionParams->PlEncrypt.SecureAes->CsuDmaPtr,
				XCSUDMA_SRC_CHANNEL, &ConfigurValues);

		/* Configure AES engine */
		SssAes = XSecure_SssInputAes(XSECURE_CSU_SSS_SRC_SRC_DMA);
		SssCfg = SssAes | XSecure_SssInputPcap(XSECURE_CSU_SSS_SRC_AES);
		XSecure_SssSetup(SssCfg);

		/* Send whole chunk of data to AES */
		if ((Size <=
			(PartitionParams->PlEncrypt.SecureAes->SizeofData)) &&
		   (PartitionParams->PlEncrypt.SecureAes->SizeofData != 0)) {
			XFsbl_DmaPlCopy(
				PartitionParams->PlEncrypt.SecureAes->CsuDmaPtr,
					(UINTPTR)SrcAddr, Size/4, 0);
			PartitionParams->PlEncrypt.SecureAes->SizeofData =
			PartitionParams->PlEncrypt.SecureAes->SizeofData - Size;
			Size = 0;
		}
		/*
		 * If data to be processed is not zero
		 * and chunk of data is greater
		 */
		else if (PartitionParams->PlEncrypt.SecureAes->SizeofData != 0) {
			/* First transfer whole data other than secure header */
			XFsbl_DmaPlCopy(
				PartitionParams->PlEncrypt.SecureAes->CsuDmaPtr,
				(UINTPTR)SrcAddr,
			PartitionParams->PlEncrypt.SecureAes->SizeofData/4, 0);
			SrcAddr = SrcAddr +
				PartitionParams->PlEncrypt.SecureAes->SizeofData;
			Size = Size -
				PartitionParams->PlEncrypt.SecureAes->SizeofData;
			PartitionParams->PlEncrypt.SecureAes->SizeofData = 0;
			/*
			 * when data to be processed is greater than
			 * remaining data of the encrypted block
			 * and part of GCM tag and secure header of next block
			 * also exists with chunk, copy that portion for
			 * proceessing along with next chunk of data
			 */
			if (Size <
			 (XSECURE_SECURE_HDR_SIZE +
				XSECURE_SECURE_GCM_TAG_SIZE)) {
				XFsbl_CopyData(PartitionParams,
					PartitionParams->SecureHdr,
					(u8 *)(UINTPTR)SrcAddr, Size);
				PartitionParams->Hdr = Size;
				Size = 0;
			}
		}

		/* Wait PCAP done */
		Status = XFsbl_PcapWaitForDone();
		if (Status != XFSBL_SUCCESS) {
			return Status;
		}

		XCsuDma_GetConfig(
			PartitionParams->PlEncrypt.SecureAes->CsuDmaPtr,
				XCSUDMA_SRC_CHANNEL, &ConfigurValues);
		ConfigurValues.EndianType = 0U;
		XCsuDma_SetConfig(
			PartitionParams->PlEncrypt.SecureAes->CsuDmaPtr,
				XCSUDMA_SRC_CHANNEL, &ConfigurValues);

		/* Decrypting secure header and GCM tag address */
		if ((PartitionParams->PlEncrypt.SecureAes->SizeofData == 0) &&
						(Size != 0)) {
			Status = XFsbl_DecrypSecureHdr(
				PartitionParams->PlEncrypt.SecureAes,
					SrcAddr, 0);
			if (Status != XFSBL_SUCCESS) {
				return Status;
			}
			Size = Size - (XSECURE_SECURE_HDR_SIZE +
					XSECURE_SECURE_GCM_TAG_SIZE);
			if (Size != 0x00) {
				NextBlkAddr = SrcAddr +
					XSECURE_SECURE_HDR_SIZE +
					XSECURE_SECURE_GCM_TAG_SIZE;
			}
			/*
			 * This means we are done with Secure header and Block 0
			 * And now we can change the AES key source to KUP.
			 */
			PartitionParams->PlEncrypt.SecureAes->KeySel =
					XSECURE_CSU_AES_KEY_SRC_KUP;
			XSecure_AesKeySelNLoad(
				PartitionParams->PlEncrypt.SecureAes);
			Status = XFsbl_DecrptSetUpNextBlk(PartitionParams,
					NextBlkAddr, Size);
			if (Status != XFSBL_SUCCESS) {
				return Status;
			}
			if ((NextBlkAddr != 0x00U) &&
			(PartitionParams->PlEncrypt.SecureAes->SizeofData != 0)) {
				SrcAddr = NextBlkAddr;
			}
			else {
				break;
			}


		}

	} while (Size != 0x00);

	return Status;

}

/******************************************************************************
*
* This API decrypts the chunks of data
*
* @param	PartitionParams is a pointer to XFsblPs_PlPartition
* @param	ChunkAdrs holds the address of chunk address
* @param	ChunkSize holds the size of chunk
*
* @return
* 		Error code on failure
* 		XFSBL_SUCESS on success
*
* @note		None.
*
******************************************************************************/
static u32 XFsbl_DecrptPlChunks(XFsblPs_PlPartition *PartitionParams,
		u64 ChunkAdrs, u32 ChunkSize)
{
	u32 Status = XFSBL_SUCCESS;
	UINTPTR SrcAddr = (u64)ChunkAdrs;
	u32 Size = ChunkSize;
	u64 NextBlkAddr = 0;
	u32 SssAes;
	u32 SssCfg;

	/* If this is the first block to be decrypted it is the secure header */
	if (PartitionParams->PlEncrypt.NextBlkLen == 0x00) {
		XSecure_AesDecryptInit(PartitionParams->PlEncrypt.SecureAes,
		(u8 *)XSECURE_DESTINATION_PCAP_ADDR, XSECURE_SECURE_HDR_SIZE,
			(u8 *)(SrcAddr + XSECURE_SECURE_HDR_SIZE));

		/*
		 * Configure AES engine to push decrypted Key and IV in the
		 * block to the CSU KEY and IV registers.
		 */
		XSecure_WriteReg(
			PartitionParams->PlEncrypt.SecureAes->BaseAddress,
				XSECURE_CSU_AES_KUP_WR_OFFSET,
				XSECURE_CSU_AES_IV_WR | XSECURE_CSU_AES_KUP_WR);
		/* Decrypting the Secure header */
		Status = XSecure_AesDecryptUpdate(
			PartitionParams->PlEncrypt.SecureAes,
			(u8 *)(SrcAddr), XSECURE_SECURE_HDR_SIZE);
		if (Status != XFSBL_SUCCESS) {
			XFsbl_Printf(DEBUG_GENERAL,
				"XFSBL_DECRYPT_SECURE_HEADER:"
				"XFSBL_ERROR_BITSTREAM_GCM_TAG_MISMATCH\r\n");
			Status = XFSBL_ERROR_BITSTREAM_GCM_TAG_MISMATCH;
			goto END;
		}
		PartitionParams->PlEncrypt.SecureAes->KeySel =
				XSECURE_CSU_AES_KEY_SRC_KUP;
		XSecure_AesKeySelNLoad(PartitionParams->PlEncrypt.SecureAes);
		/* Point IV to the CSU IV register. */
		PartitionParams->PlEncrypt.SecureAes->Iv =
		(u32 *)(PartitionParams->PlEncrypt.SecureAes->BaseAddress +
					(UINTPTR)XSECURE_CSU_AES_IV_0_OFFSET);
		/*
		 * Remaining size and source address
		 * of the data to be processed
		 */
		Size = ChunkSize -
			XSECURE_SECURE_HDR_SIZE - XSECURE_SECURE_GCM_TAG_SIZE;
		SrcAddr = ChunkAdrs +
			XSECURE_SECURE_HDR_SIZE+XSECURE_SECURE_GCM_TAG_SIZE;

		/*
		 * Decrypt next block after Secure header and
		 * update the required fields
		 */
		Status = XFsbl_DecrptSetUpNextBlk(PartitionParams,
						SrcAddr, Size);
		if (Status != XFSBL_SUCCESS) {
			goto END;
		}

		Status = XFsbl_DecrptPl(PartitionParams,
					(UINTPTR)SrcAddr, Size);
		if (Status != XFSBL_SUCCESS) {
			goto END;
		}
		/*
		 * If status is true or false also goto END
		 * As remaining data also processed in above API
		 */
		goto END;

	}
	/*
	 * If previous chunk has portion of left header,
	 * which needs to be processed along with this chunk
	 */
	else  if (PartitionParams->Hdr != 0x00) {

		/* Configure AES engine */
		SssAes = XSecure_SssInputAes(XSECURE_CSU_SSS_SRC_SRC_DMA);
		SssCfg = SssAes | XSecure_SssInputPcap(XSECURE_CSU_SSS_SRC_AES);
		XSecure_SssSetup(SssCfg);

		XFsbl_CopyData(PartitionParams,
		(u8 *)(PartitionParams->SecureHdr + PartitionParams->Hdr),
		(u8 *)(UINTPTR)SrcAddr, XFSBL_AES_TAG_SIZE - PartitionParams->Hdr);

		Status = XFsbl_DecrypSecureHdr(
			PartitionParams->PlEncrypt.SecureAes,
			(u64)(UINTPTR)PartitionParams->SecureHdr, 0);
		if (Status != XFSBL_SUCCESS) {
			return Status;
		}

		Size = Size - (XFSBL_AES_TAG_SIZE - PartitionParams->Hdr);
		if (Size != 0x00) {
			NextBlkAddr = SrcAddr +
				(XFSBL_AES_TAG_SIZE - PartitionParams->Hdr);
		}
		PartitionParams->Hdr = 0;
		memset(PartitionParams->SecureHdr, 0, XFSBL_AES_TAG_SIZE);
		/*
		 * This means we are done with Secure header and Block 0
		 * And now we can change the AES key source to KUP.
		 */
		PartitionParams->PlEncrypt.SecureAes->KeySel =
				XSECURE_CSU_AES_KEY_SRC_KUP;

		XSecure_AesKeySelNLoad(PartitionParams->PlEncrypt.SecureAes);
		Status = XFsbl_DecrptSetUpNextBlk(PartitionParams,
						NextBlkAddr, Size);
		if (Status != XFSBL_SUCCESS) {
			return Status;
		}

		if ((NextBlkAddr != 0x00U) &&
			(PartitionParams->PlEncrypt.SecureAes->SizeofData != 0)) {
			Status = XFsbl_DecrptPl(PartitionParams,
					(UINTPTR)NextBlkAddr, Size);
			if (Status != XFSBL_SUCCESS) {
				return Status;
			}
		}
	}
	else {
		Status = XFsbl_DecrptPl(PartitionParams, SrcAddr, Size);
		goto END;
	}

END:

	return Status;

}

#endif
