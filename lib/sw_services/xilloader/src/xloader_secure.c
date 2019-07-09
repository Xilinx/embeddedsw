/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xloader_secure.c
*
* This file contains all security functionality operations.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   vns  04/23/19 First release
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xloader_secure.h"
#include "xilpdi.h"
#include "xplmi_dma.h"
/************************** Constant Definitions ****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

static u32 XLoader_VerifyHash(XLoader_SecureParms *SecurePtr, u32 Size,
						u8 Last);
static u32 XLoader_RsaPssSignatureverification(
		XLoader_SecureParms *SecurePtr,
		XSecure_Rsa *RsaInstancePtr,
		u8 *Signature, u8 *MsgHash);
static u32 XLoader_SpkAuthentication(XLoader_SecureParms *SecurePtr);
static u32 XLoader_DataAuth(XLoader_SecureParms *SecurePtr, u8 *Hash);
static inline void XLoader_I2Osp(u32 Integer, u32 Size, u8 *Convert);
static u32 XLoader_EcdsaSignVerify(u8 *Hash, u8 *Key, u8 *Signature);
static u32 XLoader_RsaSignVerify(XLoader_SecureParms *SecurePtr,
	u8 *Hash, XLoader_RsaKey *Key, u8 *Signature);
static u32 XLoader_VerifySignature(XLoader_SecureParms *SecurePtr,
		u8 *Hash, XLoader_RsaKey *Key, u8 *signature);
static u32 XLoader_AesDecryption(XLoader_SecureParms *SecurePtr, u64 SrcAddr,
			u64 DstAddr, u32 Size);
static u32 XLoader_AesKeySelct(XLoader_SecureParms *SecurePtr,
		XSecure_Aes *AesInstancePtr,
		XSecure_AesKeySrc *KeySrc);
static u32 XLoader_CheckNonZeroPpk();
static u32 XLoader_PpkVerify(XLoader_SecureParms *SecurePtr);
static u32 XLoader_IsPpkValid(u8 PpkSelect, u8 *PpkHash);
static u32 XLoader_VerifySpkId(u32 SpkId);
static u32 XLoader_PpkCompare(u32 EfusePpkOffset, u8 *PpkHash);

/************************** Variable Definitions *****************************/

XLoader_AuthCertificate AuthCert;
XLoader_Vars Xsecure_Varsocm __attribute__ ((aligned(32)));

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* @brief
* This function initializes  XLoader_SecureParms's instance.
*
* @param	SecurePtr	Pointer to the XLoader_SecureParms instance.
* @param	PdiPtr		Pointer to the XilPdi.
* @param	PrtnNum		Partition number to be processed.
*
* @return	XST_SUCCESS on success.
*
******************************************************************************/
u32 XLoader_SecureInit(XLoader_SecureParms *SecurePtr, XilPdi *PdiPtr,
						u32 PrtnNum)
{
	XCsuDma *CsuDmaPtr;
	u32 Status;
	XilPdi_PrtnHdr *PrtnHdr;
	u32 ChecksumOffset;
	u32 AcOffset;

	memset(SecurePtr, 0, sizeof(XLoader_SecureParms));

	/* Assign the partition header to local variable */
	PrtnHdr = &(PdiPtr->MetaHdr.PrtnHdr[PrtnNum]);

	SecurePtr->PdiPtr = PdiPtr;
	SecurePtr->ChunkAddr = XLOADER_CHUNK_MEMORY;
	SecurePtr->BlockNum = 0x00U;
	SecurePtr->PrtnHdr = PrtnHdr;

	/* Get DMA instance */
	CsuDmaPtr = XPlmi_GetDmaInstance(CSUDMA_0_DEVICE_ID);
	if (CsuDmaPtr == NULL) {
		Status = XST_FAILURE;
		goto END;
	}
	SecurePtr->CsuDmaInstPtr = CsuDmaPtr;

	/* Check if checksum is enabled */
	if ((PrtnHdr->PrtnAttrb & XIH_PH_ATTRB_CHECKSUM_MASK) != 0x00) {
		 XPlmi_Printf(DEBUG_INFO,
			 "Verifying checksum of the partition\n\r");

		/* Check checksum type */
		if((PrtnHdr->PrtnAttrb & XIH_PH_ATTRB_CHECKSUM_MASK) ==
				XIH_PH_ATTRB_HASH_SHA3) {
			SecurePtr->IsCheckSumEnabled = TRUE;
			SecurePtr->SecureEn = TRUE;
		}
		else {
			/* only SHA3 checksum is supported */
			Status = XST_FAILURE;
			goto END;
		}
		ChecksumOffset = SecurePtr->PdiPtr->MetaHdr.FlashOfstAddr +
				((SecurePtr->PrtnHdr->ChecksumWordOfst) *
					XIH_PRTN_WORD_LEN);

		/* Copy checksum hash */
		SecurePtr->PdiPtr->DeviceCopy(ChecksumOffset,
					(UINTPTR)SecurePtr->Sha3Hash,
						XLOADER_SHA3_LEN, 0U);
	}
	/* check if authentication is enabled */
	if ((PrtnHdr->PrtnAttrb & XIH_PH_ATTRB_RSA_SIGNATURE_MASK) != 0x00) {
		 XPlmi_Printf(DEBUG_INFO,
                         "Authenticating the partition\n\r");

		SecurePtr->IsAuthenticated = TRUE;
		SecurePtr->SecureEn = TRUE;
		/* Copy Authentication certificate */
		AcOffset = SecurePtr->PdiPtr->MetaHdr.FlashOfstAddr +
			((SecurePtr->PrtnHdr->AuthCertificateOfst) *
				XIH_PRTN_WORD_LEN);
		SecurePtr->AcPtr = &AuthCert;
		SecurePtr->PdiPtr->DeviceCopy(AcOffset,
					(UINTPTR)SecurePtr->AcPtr,
					XLOADER_AUTH_CERT_MIN_SIZE,
						0U);
	}
	/* Check if encryption is enabled */
	if ((PrtnHdr->PrtnAttrb & XIH_PH_ATTRB_ENCRYPTION_MASK) != 0x00) {
		 XPlmi_Printf(DEBUG_INFO,
                         "Partition is in encrypted format\n\r");

		SecurePtr->IsEncrypted = TRUE;
		SecurePtr->SecureEn = TRUE;
	}

	/* Checksum could not be enabled with authentication or ecnryption */
	if ((SecurePtr->IsCheckSumEnabled == TRUE) &&
			((SecurePtr->IsAuthenticated == TRUE) ||
				(SecurePtr->IsEncrypted == TRUE))) {
		Status = XST_FAILURE;
		goto END;
	}

	Status = XST_SUCCESS;

END:

	return Status;
}

/*****************************************************************************/
/**
* @brief
* This function loads secure non-cdo partitions.
*
* @param	SecurePtr	Pointer to the XLoader_SecureParms instance.
* @param	DestAddr	Load address of the partition
* @param	Size		un encrypted size of the partition.
*
* @return	XST_SUCCESS on success.
*
******************************************************************************/
u32 XLoader_SecureCopy(XLoader_SecureParms *SecurePtr, u64 DestAddr, u32 Size)
{

	u32	ChunkLen;
	u32 Len = Size;
	u64 LoadAddr = DestAddr;
	u8 LastChunk = FALSE;
	u32 Status = XST_FAILURE;

	if ((SecurePtr->IsAuthenticated == TRUE)
			|| (SecurePtr->IsCheckSumEnabled == TRUE)) {
		ChunkLen = Len % XLOADER_CHUNK_SIZE;
	}
	else {
		ChunkLen = XLOADER_CHUNK_SIZE;
	}

	while (Len > 0U)
	{
		/** Update the length for last chunk */
		if (Len <= ChunkLen)
		{
			LastChunk = TRUE;
			ChunkLen = Len;
		}

		/* Call security function */
		Status = XLoader_SecurePrtn(SecurePtr, LoadAddr,
				ChunkLen, LastChunk);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		if (SecurePtr->IsEncrypted != TRUE) {
			/* copy to destination address */
			Status = XPlmi_DmaXfr((u64)SecurePtr->SecureData,
				(u64)LoadAddr, ChunkLen/4, XPLMI_PMCDMA_0);
		}
		LoadAddr = LoadAddr + ChunkLen;

		/** Update variables for next chunk */
		Len -= ChunkLen;
		if (LastChunk == FALSE) {
			ChunkLen = XLOADER_CHUNK_SIZE;
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* This function performs authentication, checksum and decryption of the
* partition.
*
* @param	SecurePtr	Pointer to the XLoader_SecureParms instance.
* @param	BlockSize	Size of the data block to be processed
*		which doesn't include padding lengths and hash.
* @param	Last		Notifies if the block to be processed is
*		last or not.
*
* @return	XST_SUCCESS on success.
*
* @note
* 		If Size < chunk size it is first block
* 		If last = TRUE then last chunk
*
******************************************************************************/

u32 XLoader_SecurePrtn(XLoader_SecureParms *SecurePtr, u64 DstAddr,
				u32 BlockSize, u8 Last)
{
	u32 Status = XST_SUCCESS;
	u32 TotalSize = BlockSize;
	u32 SrcAddr;
	u64 OutAddr;

	/* 1st block */
	if (SecurePtr->BlockNum == 0x0) {
		SrcAddr = SecurePtr->PdiPtr->MetaHdr.FlashOfstAddr +
		((SecurePtr->PrtnHdr->DataWordOfst) * XIH_PRTN_WORD_LEN);
	}
	else {
		SrcAddr = SecurePtr->NextBlkAddr;
	}

	/*
	 * If authentication or checksum is enabled validate the data hash
	 * with expected hash
	 */
	if ((SecurePtr->IsAuthenticated == TRUE) ||
			(SecurePtr->IsCheckSumEnabled == TRUE)) {
		 if (SecurePtr->IsEncrypted == TRUE) {
			if (SecurePtr->BlockNum == 0) {
				TotalSize = TotalSize + 64;
			}
			TotalSize = TotalSize + 64;
		 }
		 /*
		 * Except for the last block of data,
		 * SHA3 hash(48 bytes) of next block should
		 * be added for block size
		 */
		if (Last != TRUE) {
			TotalSize = TotalSize + XLOADER_SHA3_LEN;
		}

		/* Copy to total data to the buffer */
		SecurePtr->PdiPtr->DeviceCopy(SrcAddr,
				SecurePtr->ChunkAddr, TotalSize, 0U);

		/* Verify hash */
		Status = XLoader_VerifyHash(SecurePtr, TotalSize, Last);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		if (Last != TRUE) {
			SecurePtr->NextBlkAddr = SrcAddr + TotalSize;
		}
	}

	/* If encryption is enabled */
	if (SecurePtr->IsEncrypted == TRUE) {

		if (SecurePtr->IsAuthenticated != TRUE) {
			/* Copy to total data to the buffer */
			if (SecurePtr->BlockNum == 0) {
				/* Secure header is extra here */
				TotalSize = BlockSize + 64;
			}
			TotalSize = TotalSize + 64;

			SecurePtr->PdiPtr->DeviceCopy(SrcAddr,
				SecurePtr->ChunkAddr, TotalSize, 0U);
			SecurePtr->SecureData = SecurePtr->ChunkAddr;
			SecurePtr->NextBlkAddr = SrcAddr + TotalSize;
		}
		if (SecurePtr->IsCdo != TRUE) {
			OutAddr = DstAddr;
		}
		else {
			OutAddr = SecurePtr->ChunkAddr;
		}
		Status = XLoader_AesDecryption(SecurePtr,
				SecurePtr->SecureData, OutAddr, BlockSize);
		if (Status != XST_SUCCESS) {
			goto END;
		}

	}

	SecurePtr->BlockNum++;

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* This function calculates hash and compares with expected hash.
* If authentication is enabled hash is calculated on AC + Data for first block,
* encrypts the ECDSA/RSA signature and compares it with the expected hash.
* For checksum and authentication(after first block), hash is calculated on block
* of data and compared with the expected hash.
*
* @param	SecurePtr	Pointer to the XLoader_SecureParms instance.
* @param	Size		Size of the data block to be processed
*		which includes padding lengths and hash.
* @param	Last		Notifies if the block to be processed is
*		last or not.
*
* @return	XST_SUCCESS on success.
*
******************************************************************************/
static u32 XLoader_VerifyHash(XLoader_SecureParms *SecurePtr,
					u32 Size, u8 Last)
{
	s32 RetStatus;
	u32 Status;
	XSecure_Sha3 Sha3Instance;
	u8 *Data = (u8 *)SecurePtr->ChunkAddr;
	u8 CalHash[XLOADER_SHA3_LEN] = {0};
	u8 *ExpHash = (u8 *)SecurePtr->Sha3Hash;
	XCsuDma *CsuDmaPtr = SecurePtr->CsuDmaInstPtr;
	XLoader_AuthCertificate *AcPtr=
		(XLoader_AuthCertificate *)SecurePtr->AcPtr;
	if (CsuDmaPtr == NULL) {
		Status = XST_FAILURE;
		goto END;
	}

	RetStatus = XSecure_Sha3Initialize(&Sha3Instance, CsuDmaPtr);
	if (RetStatus != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto END;
	}

	XSecure_Sha3Start(&Sha3Instance);

	/* Hash should be calculated on AC + first chunk */
	if ((SecurePtr->IsAuthenticated == TRUE) &&
		(SecurePtr->BlockNum == 0x00U)) {
		Status = XSecure_Sha3Update(&Sha3Instance,
					(u8 *)AcPtr,
					XLOADER_AUTH_CERT_MIN_SIZE -
					XLOADER_PARTITION_SIG_SIZE);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	Status = XSecure_Sha3Update(&Sha3Instance, Data, Size);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_Sha3Finish(&Sha3Instance, CalHash);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Verify the hash */
	if ((SecurePtr->IsAuthenticated == TRUE) &&
			(SecurePtr->BlockNum == 0x00U)) {
		Status = XLoader_DataAuth(SecurePtr, CalHash);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	else {
		Status = XPlmi_MemCmp(ExpHash, CalHash, XLOADER_SHA3_LEN);
		if (Status != XST_SUCCESS) {
			XPlmi_PrintArray(DEBUG_INFO,
			(UINTPTR)CalHash, XLOADER_SHA3_LEN/XIH_PRTN_WORD_LEN,
				"Calculated Hash");
			XPlmi_PrintArray(DEBUG_INFO, (UINTPTR)ExpHash,
				XLOADER_SHA3_LEN/XIH_PRTN_WORD_LEN,
				"Expected Hash");
			Status = XST_FAILURE;
			goto END;
		}
	}

	/* Update the next expected hash  and data location */
	if (Last == 0x00U) {
		memcpy(ExpHash, Data, XLOADER_SHA3_LEN);
		SecurePtr->SecureData = (UINTPTR)Data + XLOADER_SHA3_LEN;
	}
	else {
		/* this is the last block */
		SecurePtr->SecureData = (UINTPTR)Data;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* This function authenticates the data with SPK.
*
* @param	SecurePtr	Pointer to the XLoader_SecureParms instance.
* @param	Hash		Pointer to the expected hash
*
* @return	XST_SUCCESS on success.
*
******************************************************************************/
static u32 XLoader_DataAuth(XLoader_SecureParms *SecurePtr, u8 *Hash)
{
	u32 Status = XST_FAILURE;
	XLoader_AuthCertificate *AcPtr =
		(XLoader_AuthCertificate *)SecurePtr->AcPtr;
	u32 IsEfuseAuth = TRUE;

	/* If bits in PPK0/1/2 is programmed bh_auth is not allowed */
	Status = XLoader_CheckNonZeroPpk();
	/*
	 * only boot header authentication is allowed when
	 * none of PPK hash bits are programmed
	 */
	if (Status != XST_SUCCESS) {
		IsEfuseAuth = FALSE;
		/* If BHDR authentication is not enabled return error */
		if ((SecurePtr->PdiPtr->MetaHdr.BootHdr.ImgAttrb &
			XIH_BH_IMG_ATTRB_BH_AUTH_MASK) == 0x00U) {
			XPlmi_Printf(DEBUG_INFO,
			"None of the PPKs are programmed and also boot header"
				" authentication is not enabled\n\r");
			Status = XST_FAILURE;
			goto END;
		}
	}
	/* Only efuse RSA authentication is allowed */
	else {
		IsEfuseAuth = TRUE;

		/* If BHDR authentication is enabled return error */
		if ((SecurePtr->PdiPtr->MetaHdr.BootHdr.ImgAttrb &
			XIH_BH_IMG_ATTRB_BH_AUTH_MASK) != 0x00U) {
			XPlmi_Printf(DEBUG_INFO,
			"As PPK was programmed and boot header"
				" authentication is also enabled\n\r");
			Status = XST_FAILURE;
			goto END;
		}
		/* Validate PPK hash */
		Status = XLoader_PpkVerify(SecurePtr);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	/* Perform SPK Validation */
	Status = XLoader_SpkAuthentication(SecurePtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Check for SPK ID revocation */
	if (IsEfuseAuth == TRUE) {
		Status = XLoader_VerifySpkId(AcPtr->SpkId);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	/* Data Authentication of first block */
	Status = XLoader_VerifySignature(SecurePtr, Hash, &AcPtr->Spk,
			(u8 *)&AcPtr->ImageSignature);


END:
	return Status;

}

/*****************************************************************************/
/**
* @brief
* This function encrypts the RSA/ECDSA signature provided and compares it
* with expected hash.
*
* @param	SecurePtr	Pointer to the XLoader_SecureParms instance.
* @param	Hash		Pointer to the expected hash
* @param	Key			Pointer to the RSa/ECDSA public key to be used
* @param	Signature	Pointer to the Signature
*
* @return	XST_SUCCESS on success.
*
******************************************************************************/
static u32 XLoader_VerifySignature(XLoader_SecureParms *SecurePtr,
		u8 *Hash, XLoader_RsaKey *Key, u8 *Signature)
{
	u32 Status = XST_FAILURE;
	XLoader_AuthCertificate *AcPtr =
		(XLoader_AuthCertificate *)SecurePtr->AcPtr;

	/* RSA authentication */
	if ((AcPtr->AuthHeader & XLOADER_AC_AH_PUB_ALG_MASK) ==
			XLOADER_AC_AH_PUB_ALG_RSA) {
		Status = XLoader_RsaSignVerify(SecurePtr, Hash,
			Key, Signature);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	/* ECDSA authentication */
	else if ((AcPtr->AuthHeader & XLOADER_AC_AH_PUB_ALG_MASK) ==
			XLOADER_AC_AH_PUB_ALG_ECDSA) {
		Status = XLoader_EcdsaSignVerify(Hash,
			(u8 *)Key->PubModulus, Signature);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	/* Not supported */
	else {
		Status = XST_FAILURE;
	}

END:
	return Status;

}

/*****************************************************************************/
/**
* @brief
* This function verifies SPK with PPK.
*
* @param	SecurePtr	Pointer to the XLoader_SecureParms instance.
*
* @return	XST_SUCCESS on success.
*
******************************************************************************/
static u32 XLoader_SpkAuthentication(XLoader_SecureParms *SecurePtr)
{

	u8 Hash[XSECURE_HASH_TYPE_SHA3]__attribute__ ((aligned(32)));
	u8 *SpkHash = Hash;
	u32 Status;
	XLoader_AuthCertificate *AcPtr = SecurePtr->AcPtr;
	XSecure_Sha3 Sha3Instance;

	XPlmi_Printf(DEBUG_DETAILED, "Performing SPK verification \n\r");
	/* Initialize sha3 */
	XSecure_Sha3Initialize(&Sha3Instance, SecurePtr->CsuDmaInstPtr);
	XSecure_Sha3Start(&Sha3Instance);

	/* Hash the AH  and SPK*/
	/* Update AH */
	XSecure_Sha3Update(&Sha3Instance,
			(u8 *)&AcPtr->AuthHeader, XLOADER_AUTH_HEADER_SIZE);
	XSecure_Sha3LastUpdate(&Sha3Instance);
	/* Update SPK */
	XSecure_Sha3Update(&Sha3Instance, (u8 *)&AcPtr->Spk, XLOADER_SPK_SIZE);

	XSecure_Sha3WaitForDone(&Sha3Instance);
	XSecure_Sha3_ReadHash(&Sha3Instance, (u8 *)SpkHash);

	Status = XLoader_VerifySignature(SecurePtr, SpkHash, &AcPtr->Ppk,
					(u8 *)&AcPtr->SPKSignature);

	return Status;

}

/*****************************************************************************/
/**
* @brief
* This function validates SPK by verifying providing SPK ID is been revoked
* or not.
*
* @param	SpkId	ID of the SPK
*
* @return	XST_SUCCESS on success.
*
******************************************************************************/
static u32 XLoader_VerifySpkId(u32 SpkId)
{
	u32 Status;
	u32 SpkRevokeAll;
	u32 Quo;
	u32 Mod;
	u32 Value;

	/* TBD this API should ultilize XilNvm library */
	XPlmi_Printf(DEBUG_DETAILED, "Validating SPKID\n\r");
	SpkRevokeAll = XPlmi_In32(XLOADER_EFUSE_SPKID_0_OFFSET) &
			XPlmi_In32(XLOADER_EFUSE_SPKID_1_OFFSET) &
			XPlmi_In32(XLOADER_EFUSE_SPKID_2_OFFSET) &
			XPlmi_In32(XLOADER_EFUSE_SPKID_3_OFFSET) &
			XPlmi_In32(XLOADER_EFUSE_SPKID_4_OFFSET) &
			XPlmi_In32(XLOADER_EFUSE_SPKID_5_OFFSET) &
			XPlmi_In32(XLOADER_EFUSE_SPKID_6_OFFSET) &
			XPlmi_In32(XLOADER_EFUSE_SPKID_7_OFFSET);
	/* If all bits of SPKID are programmed */
	if(SpkRevokeAll == MASK_ALL) {
		XPlmi_Printf(DEBUG_INFO, "All SPK IDs are invalid\n\r");
		Status = XST_FAILURE;
		goto END;
	}
	/* Verify range of provided SPK ID */
	if(SpkId > XLOADER_SPKID_MAX)
	{
		XPlmi_Printf(DEBUG_INFO, "SPK ID provided is out of range,"
				" valid range is 0 - 255\n\r");
		Status = XST_FAILURE;
		goto END;
	}
	else
	{
		Quo = SpkId / XLOADER_WORD_IN_BITS;
		Mod = SpkId % XLOADER_WORD_IN_BITS;
		Value = (XPlmi_In32(XLOADER_EFUSE_SPKID_0_OFFSET +
				(Quo * XIH_PRTN_WORD_LEN))
				& (1U << Mod)) >> Mod;
		if(Value != 0x00U)	{
			Status = XST_FAILURE;
			goto END;
		}
	}

	Status = (u32)XST_SUCCESS;
	XPlmi_Printf(DEBUG_DETAILED, "SPK Status - GOOD\r\n");

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* This function compares calculated PPK hash with the efuse PPK hash
*
* @param	EfusePpkOffset	PPK hash address of efuse.
* @param	PpkHash		Pointer to the PPK hash to be verified.
*
* @return	XST_SUCCESS on success.
*
******************************************************************************/
static u32 XLoader_PpkCompare(u32 EfusePpkOffset, u8 *PpkHash)
{
	s32 HashStatus;
	u8 Index;
	u32 Status;
	u8 HashZeros[XLOADER_EFUSE_PPK_HASH_LEN];

	/* Fill HashZeros array with all zeros */
	for (Index = 0; Index < 32; Index++) {
		HashZeros[Index] = 0x00U;
	}

	HashStatus = XPlmi_MemCmp(PpkHash,
			(void *)EfusePpkOffset,
			XLOADER_EFUSE_PPK_HASH_LEN);
	if (HashStatus == XST_SUCCESS) {
		/* Check if PPK hash is all zeros */
		HashStatus = XPlmi_MemCmp(HashZeros,
			(void *)EfusePpkOffset,
			XLOADER_EFUSE_PPK_HASH_LEN);
		if (HashStatus == XST_SUCCESS) {
			Status = (u32)XST_FAILURE;
		}
		else {
			Status = (u32)XST_SUCCESS;
		}
	}
	else {
		Status = (u32)XST_FAILURE;
	}

	return Status;

}

/*****************************************************************************/
/**
* @brief
* This function validates PPK, by checking selected PPK is valid or not
* by reading PPK invalid bits and verifying the hash provided with programmed.
* Efuse only stores 256 bits of hash.
*
* @param	PpkSelect	PPK selection of eFUSE.
* @param	PpkHash		Pointer to the PPK hash to be verified.
*
* @return	XST_SUCCESS on success.
*
******************************************************************************/
static u32 XLoader_IsPpkValid(u8 PpkSelect, u8 *PpkHash)
{
	u32 ReadReg;
	u32 Status;

	/* Read PPK invalid set bits */
	ReadReg = XPlmi_In32(XLOADER_EFUSE_MISC_CTRL_OFFSET);

	switch (PpkSelect) {
		case XLOADER_PPK_SEL_0:
			if ((ReadReg & XLOADER_EFUSE_MISC_CTRL_PPK0_INVLD) !=
				0x0U) {
				Status = XST_FAILURE;
				break;
			}
			Status = XLoader_PpkCompare(XLOADER_EFUSE_PPK0_START_OFFSET,
					PpkHash);
			break;
		case XLOADER_PPK_SEL_1:
			if ((ReadReg & XLOADER_EFUSE_MISC_CTRL_PPK1_INVLD) !=
						0x0U) {
				Status = (u32)XST_FAILURE;
				break;
			}
			Status = XLoader_PpkCompare(XLOADER_EFUSE_PPK1_START_OFFSET,
							PpkHash);
			break;
		case XLOADER_PPK_SEL_2:
			if ((ReadReg & XLOADER_EFUSE_MISC_CTRL_PPK2_INVLD) !=
								0x0U) {
				Status = (u32)XST_FAILURE;
				break;
			}
			Status = XLoader_PpkCompare(XLOADER_EFUSE_PPK2_START_OFFSET,
							PpkHash);
			break;
		default:
			Status = (u32)XST_FAILURE;
			break;
	}

	return Status;

}

/*****************************************************************************/
/**
* @brief
* This function returns XST_SUCCESS if any of the PPK hash bits are programmed.
*
* @return	XST_SUCCESS on success.
*
******************************************************************************/
static u32 XLoader_CheckNonZeroPpk()
{
	u32 Index;
	u32 Status;

	for (Index = XLOADER_EFUSE_PPK0_START_OFFSET;
			Index < XLOADER_EFUSE_PPK2_END_OFFSET;
			Index = Index + XIH_PRTN_WORD_LEN) {
		/* Any bit of PPK hash are non-zero break and return success */
		if (XPlmi_In32(Index) != 0x0) {
			Status = (u32)XST_SUCCESS;
			goto END;
		}
	}
	Status = (u32)XST_FAILURE;
END:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* This function verifies PPK.
*
* @param	SecurePtr	Pointer to the XLoader_SecureParms instance.
*
* @return	XST_SUCCESS on success.
*
******************************************************************************/
static u32 XLoader_PpkVerify(XLoader_SecureParms *SecurePtr)
{

	u8 Hash[XSECURE_HASH_TYPE_SHA3]__attribute__ ((aligned(32)));
	u32 Status;
	XLoader_AuthCertificate *AcPtr = SecurePtr->AcPtr;
	XSecure_Sha3 Sha3Instance;
	u32 ReadReg;

	/* Check if all PPKs are revoked */
	ReadReg = XPlmi_In32(XLOADER_EFUSE_MISC_CTRL_OFFSET);
	if ((ReadReg & XLOADER_EFUSE_MISC_CTRL_ALL_PPK_INVLD) ==
				(XLOADER_EFUSE_MISC_CTRL_ALL_PPK_INVLD)) {
		XPlmi_Printf(DEBUG_INFO, "All PPKs are invalid\n\r");
		Status = XST_FAILURE;
		goto END;
	}
		/* Calculate PPK hash */
	XSecure_Sha3Initialize(&Sha3Instance, SecurePtr->CsuDmaInstPtr);
	XSecure_Sha3Start(&Sha3Instance);
	XSecure_Sha3LastUpdate(&Sha3Instance);
	/* Update PPK  */
	XSecure_Sha3Update(&Sha3Instance, (u8 *)&AcPtr->Ppk, XLOADER_PPK_SIZE);

	XSecure_Sha3WaitForDone(&Sha3Instance);
	XSecure_Sha3_ReadHash(&Sha3Instance, (u8 *)Hash);

	Status = XLoader_IsPpkValid(XLOADER_PPK_SEL_0, Hash);
	if(Status != (u32)XST_SUCCESS)
	{
		Status = XLoader_IsPpkValid(XLOADER_PPK_SEL_1, Hash);
		if(Status != (u32)XST_SUCCESS) {
			Status = XLoader_IsPpkValid(XLOADER_PPK_SEL_2, Hash);
			if(Status == (u32)XST_SUCCESS) {
				/* Selection matched with PPK2 HASH */
				XPlmi_Printf(DEBUG_INFO, "PPK2 is valid\n\r");
			}
			else {
				/* No PPK is valid */
				XPlmi_Printf(DEBUG_INFO, "No PPK is valid\n\r");
				Status = XST_FAILURE;
			}
		}
		else {
			/* Selection matched with PPK1 HASH */
			XPlmi_Printf(DEBUG_INFO, "PPK1 is valid\n\r");
		}
	}
	else {
		/* Selection matched with PPK0 HASH */
		XPlmi_Printf(DEBUG_INFO, "PPK0 is valid\n\r");

	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * This function converts the integer provided to secified length.
 *
 * @param	Integer is the variable in which input should be provided.
 * @param	Size holds the required size.
 * @param	Convert is a pointer in which output will be updated.
 *
 * @return	None.
 *
 * @note	None
 *
 ******************************************************************************/
static inline void XLoader_I2Osp(u32 Integer, u32 Size, u8 *Convert)
{
	if (Integer < 256U) {
		Convert[Size - 1] = (u8)Integer;
	}
}

/*****************************************************************************/
/**
 * Mask generation function with SHA3.
 *
 * @param	Sha3InstancePtr Pointer to the XSecure_Sha3 instance.
 * @param	Out		Pointer in which output of this
 		function will be stored.
 * @param	Outlen		Specify the required length.
 * @param	Input 		Pointer which holds the input data for
 *		which mask should be calculated which should be 48 bytes length.
 *
 * @return	None.
 *
 * @note	None
 *
 ******************************************************************************/
static u32 XLoader_MaskGenFunc(XSecure_Sha3 *Sha3InstancePtr,
					u8 * Out, u32 OutLen,
					u8 *Input)
{
	u32 Counter = 0;
	u32 HashLen = 48;
	u8 Hashstore[48]= {0};
	u32 Index1 = 0;
	u32 Size = 48;
	u32 Status = XST_SUCCESS;

	if(Hashstore != 0U){;}
	while (Counter <= (OutLen/HashLen)) {
		XLoader_I2Osp(Counter, 4, Xsecure_Varsocm.Convert);
		XSecure_Sha3Start(Sha3InstancePtr);
		(void)XSecure_Sha3Update(Sha3InstancePtr, Input, HashLen);
		(void)XSecure_Sha3Update(Sha3InstancePtr,
			Xsecure_Varsocm.Convert, 4);
		/* Padding for SHA3 */
		/* 01 and 10*1 padding */
		Status = XSecure_Sha3Finish(Sha3InstancePtr, Hashstore);
		if ((Counter + 1U) > (OutLen/HashLen)) {
			 /*
			  * By the time this if loop is
			  * true counter value is > 1
			  * OutLen is fixed here to 463,
			  * HashLen is 48
			  */
			 Size = (OutLen % HashLen);
		}
		(void)memcpy(Out + Index1, Hashstore, Size);
		Index1 = Index1 + 48U;
		Counter = Counter + 1U;
		(void)XSecure_Sha3Initialize(Sha3InstancePtr,
			Sha3InstancePtr->CsuDmaPtr);
		if(Status != (u32)XST_SUCCESS){;}
	}
	return Status;

}

/*****************************************************************************/
/**
 *
 * This function encrypts the RSA signature provided and performs required
 * PSS operations to extract salt and calculates M prime hash and compares
 * with hash obtained from EM.
 *
 * @param	SecurePtr	Pointer to the XLoader_SecureParms instance.
 * @param	RsaInstancePtr	Pointer to the XSecure_Rsa instance.
 * @param	Sha3InstancePtr is a pointer to the XSecure_Sha3 instance.
 * @param	Signature is the pointer to RSA signature for data to be
 *		authenticated
 * @param	Hash		Hash of the data to be authenticated.
 *
 * @return	XST_SUCCESS if verification was successful.
 *
 * @note	Prior to this API, XSecure_RsaInitialize() API should be called.
 *
 ******************************************************************************/
static u32 XLoader_RsaPssSignatureverification(XLoader_SecureParms *SecurePtr,
		XSecure_Rsa *RsaInstancePtr, u8 *Signature, u8 *MsgHash)
{

	u32 Status;
	u8 MPrimeHash[48] = {0};
	u8 XSecure_RsaSha3Array[XLOADER_RSA_SIZE];
	u8 HashMgf[480]__attribute__ ((aligned(32))) = {0};
	u8 Db[463]__attribute__ ((aligned(32))); // make use of HashMgf only
	u32 Index;
	Status = XST_FAILURE;
	XSecure_Sha3 Sha3Instance;
	u8 *DataHash = (u8 *)MsgHash;

	(void)memset(XSecure_RsaSha3Array, 0U, XLOADER_PARTITION_SIG_SIZE);
	(void)memset(&Xsecure_Varsocm, 0U, sizeof(XLoader_Vars));

	/* RSA signature encryption with public key components */
	Status = (u32)XSecure_RsaPublicEncrypt(RsaInstancePtr, Signature,
					XSECURE_RSA_4096_KEY_SIZE,
					XSecure_RsaSha3Array);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/* Checks for signature encrypted message */
	if (XSecure_RsaSha3Array[XLOADER_RSA_SIZE - 1] !=
			XLOADER_RSA_SIG_EXP_BYTE) {
		Status = XST_FAILURE;
		goto END;
	}

	/* As CSUDMA can't accept unaligned addresses */
	(void)memcpy(Xsecure_Varsocm.EmHash, XSecure_RsaSha3Array + 463, 48);
	XSecure_Sha3Initialize(&Sha3Instance, SecurePtr->CsuDmaInstPtr);

			/* Salt extraction */
	/* Generate DB from masked DB and Hash */
	(void)XLoader_MaskGenFunc(&Sha3Instance, HashMgf, 463,
		Xsecure_Varsocm.EmHash);


	/* XOR MGF output with masked DB from EM to get DB */
	for (Index = 0U; Index < 463U; Index++) {
		Db[Index] = HashMgf[Index] ^ XSecure_RsaSha3Array[Index];
	}

	/* As CSUDMA can't accept unaligned addresses */
	(void)memcpy(Xsecure_Varsocm.Salt, Db+415, 48);

	XSecure_Sha3Initialize(&Sha3Instance, SecurePtr->CsuDmaInstPtr);
	/* Hash on M prime */
	XSecure_Sha3Start(&Sha3Instance);

	 /* Padding 1 */
	(void)XSecure_Sha3Update(&Sha3Instance, Xsecure_Varsocm.Padding1, 8);

	 /* Message hash */
	(void)XSecure_Sha3Update(&Sha3Instance, DataHash, 48);

	 /* salt */
	(void)XSecure_Sha3Update(&Sha3Instance, Xsecure_Varsocm.Salt, 48);

	(void)XSecure_Sha3Finish(&Sha3Instance, MPrimeHash);

	/* Compare MPrime Hash with Hash from EM */
	for (Index = 0U; Index < 48U; Index++) {
		if (MPrimeHash[Index] !=
			XSecure_RsaSha3Array[463+Index]) {
			XPlmi_Printf(DEBUG_INFO, "Failed at RSA PSS "
				"signature verification \n\r");
			Status = XST_FAILURE;
			XPlmi_PrintArray(DEBUG_INFO,
				(UINTPTR)MPrimeHash,
				XLOADER_SHA3_LEN/XIH_PRTN_WORD_LEN,
					"M prime Hash");
			XPlmi_PrintArray(DEBUG_INFO,
				(UINTPTR)(XSecure_RsaSha3Array + 463),
				XLOADER_SHA3_LEN/XIH_PRTN_WORD_LEN,
				"RSA sha3 array Hash");
				goto END;
		}
	}
	XPlmi_Printf(DEBUG_DETAILED,
			"RSA PSS verification is success\n\r");

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * This function verifies the RSA signature with expected hash.
 *
 * @param	SecurePtr	Pointer to the XLoader_SecureParms instance.
 * @param	Hash		Pointer to the hash of the data to be
 *		authenticated.
 * @param	Key		Pointer to the ECDSA key.
 * @param	Signature	Pointer to the ECDSA signature
 *
 * @return	XST_SUCCESS if verification was successful.
 *
 ******************************************************************************/
static u32 XLoader_RsaSignVerify(XLoader_SecureParms *SecurePtr,
		u8 *Hash, XLoader_RsaKey *Key, u8 *Signature)
{
	XSecure_Rsa RsaInstance;
	u32 Status;

	/* Initialize RSA instance */
	Status = (u32)XSecure_RsaInitialize(&RsaInstance, (u8 *)Key->PubModulus,
			(u8 *)Key->PubModulusExt, (u8 *)&Key->PubExponent);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	/* Perform RSA PSS verification */
	Status = XLoader_RsaPssSignatureverification(SecurePtr,
			&RsaInstance, Signature, Hash);
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_INFO, "\nFailed at RSA PSS"
			"verification for partition\n\r");
		goto END;
	}
	XPlmi_Printf(DEBUG_DETAILED,
		"partition's RSA authentication is successful\n\r");
END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * This function encrypts the ECDSA signature provided with the key components.
 *
 * @param	Hash		Pointer to the hash of the data to be
 *		authenticated.
 * @param	Key		Pointer to the ECDSA key.
 * @param	Signature	Pointer to the ECDSA signature
 *
 * @return	XST_SUCCESS if verification was successful.
 *
 ******************************************************************************/
static u32 XLoader_EcdsaSignVerify(u8 *Hash, u8 *Key, u8 *Signature)
{
	u32 *XKey = (u32 *)Key;
	u8 * KeyTemp = Key + (XLOADER_ECDSA_KEYSIZE * 4U);
	u32 *YKey = (u32 *)KeyTemp;
	u32 *W = (u32 *)Signature;
	u8 * Sign = Signature + (XLOADER_ECDSA_KEYSIZE * 4U);
	u32 *S= (u32 *)Sign;
	u32 *DataHash = (u32 *)Hash;
	u32 Qx[XLOADER_ECDSA_KEYSIZE] = {0};
	u32 Qy[XLOADER_ECDSA_KEYSIZE] = {0};
	u32 SIGR[XLOADER_ECDSA_KEYSIZE] = {0};
	u32 SIGS[XLOADER_ECDSA_KEYSIZE] = {0};
	u32 HashPtr[XLOADER_ECDSA_KEYSIZE] = {0};
	u32 Index;
	u32 Status;

	/**
	 * Take the core out of reset
	 */
	Xil_Out32(0xF1200040, 1);
	Xil_Out32(0xF1200040, 0);

	for (Index = 0; Index < XLOADER_ECDSA_KEYSIZE; Index++) {
		Qx[Index] = Xil_Htonl(XKey[XLOADER_ECDSA_INDEXVAL - Index]);
		Qy[Index] = Xil_Htonl(YKey[XLOADER_ECDSA_INDEXVAL - Index]);
		SIGR[Index] =  Xil_Htonl(W[XLOADER_ECDSA_INDEXVAL - Index]);
		SIGS[Index] =  Xil_Htonl(S[XLOADER_ECDSA_INDEXVAL - Index]);
		HashPtr[Index] =
			Xil_Htonl(DataHash[XLOADER_ECDSA_INDEXVAL - Index]);
	}
	/* Validate point on the curve */
	Status = (u32)P384_validatekey((u8 *)Qx, (u8 *)Qy);
	if (Status != (u32)XST_SUCCESS) {
		Status = XST_FAILURE;
	} else {
		Status = (u32)P384_ecdsaverify((u8 *)HashPtr, (u8 *)Qx,
					(u8 *)Qy, (u8 *)SIGR, (u8 *)SIGS);
		if (Status != (u32)XST_SUCCESS) {
			XPlmi_Printf(DEBUG_INFO, "\nFailed at "
			"ECDSA signature verification for partition\n\r");
		}
	}

	XPlmi_Printf(DEBUG_DETAILED,
		"\n Authentication of ECDSA is successful \n\r");

	return Status;
}

/*****************************************************************************/
/**
 *
 * This function decrypts the secure header/footer.
 *
 * @param	SecurePtr	Pointer to the XLoader_SecureParms
 * @param	AesInstancePtr	Pointer to the AES instance
 * @param	SrcAddr		Pointer to the buffer where header/footer present
 *
 * @return	XST_SUCCESS if verification was successful.
 *
 ******************************************************************************/
static u32 XLoader_DecryptSH(XLoader_SecureParms *SecurePtr,
			XSecure_Aes *AesInstancePtr, u64 SrcAddr)
{
	u32 Status;
	/* decrypt header/footer */

	/* configure AES engine to push Key and IV */
	XPlmi_Printf(DEBUG_DETAILED, "Decrypting Secure header\n\r");
	Status = XSecure_AesCfgKupIv(AesInstancePtr, 1);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Push secure header */
	Status = XSecure_AesDecryptUpdate(AesInstancePtr,
			SrcAddr, XSECURE_AES_NO_CFG_DST_DMA,
			XLOADER_SECURE_HDR_SIZE, 1);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Verify GCM Tag */
	Status = XSecure_AesDecryptFinal(AesInstancePtr,
			SrcAddr + XLOADER_SECURE_HDR_SIZE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XSecure_AesCfgKupIv(AesInstancePtr, 0);
	/* Get next block length */
	Status = XSecure_AesGetNxtBlkLen(AesInstancePtr,
			&SecurePtr->EncNextBlkSize);
	XPlmi_Printf(DEBUG_DETAILED,
	"Decryption NextBlkLen = %x\n\r", SecurePtr->EncNextBlkSize);

END:

	return Status;
}

/*****************************************************************************/
/**
 *
 * This function decrypts the data
 *
 * @param	SecurePtr	Pointer to the XLoader_SecureParms
 * @param	AesInstancePtr	Pointer to the AES instance
 * @param	SrcAddr		Pointer to the buffer where header/footer present
 *
 * @return	XST_SUCCESS if verification was successful.
 *
 ******************************************************************************/
static u32 XLoader_DataDecrypt(XLoader_SecureParms *SecurePtr,
		XSecure_Aes *AesInstancePtr, u64 SrcAddr, u64 DstAddr, u32 Size)
{
	u32 Status = XST_SUCCESS;
	u32 TotalLen = Size;
	u64 InAddr = SrcAddr;
	u64 OutAddr = DstAddr;

	do {
		/* decrypt the data */
		Status = XSecure_AesDecryptUpdate(AesInstancePtr,
				InAddr, OutAddr, SecurePtr->EncNextBlkSize, 0);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		InAddr = InAddr + SecurePtr->EncNextBlkSize;
		OutAddr = OutAddr + SecurePtr->EncNextBlkSize;
		TotalLen = TotalLen - SecurePtr->EncNextBlkSize;


		/* decrypt Secure header */
		Status = XLoader_DecryptSH(SecurePtr, AesInstancePtr, InAddr);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		InAddr = InAddr + 64;
		if (SecurePtr->EncNextBlkSize == 0x00) {
			if (TotalLen == 0) {
				Status = XST_SUCCESS;
			}
			else {
			/* still remaining data is there for decryption */
				Status = XST_FAILURE;
			}
			goto END;
		}
		/* If still there is pending length to be
		 * decrypted and length is less than next blk length error out
		 */
		if (TotalLen < SecurePtr->EncNextBlkSize) {
			if (TotalLen == 0) {
				Status = XST_SUCCESS;
			}
			else {
				Status = XST_FAILURE;
			}
			goto END;
		}



	} while (1);



END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * This function decrypts the data
 *
 * @param	SecurePtr	Pointer to the XLoader_SecureParms
 * @param	SrcAddr		Address to the buffer where header/footer present
 *
 * @return	XST_SUCCESS if verification was successful.
 *
 ******************************************************************************/
/* Size we get is un encrypted length */
static u32 XLoader_AesDecryption(XLoader_SecureParms *SecurePtr,
			u64 SrcAddr, u64 DstAddr, u32 Size)
{
	XSecure_Aes AesInstance;
	u32 Status;
	u32 Iv[4];
	u8 Index;
	XSecure_AesKeySrc KeySrc = 0;

	/* Initialize AES driver */
	Status = XSecure_AesInitialize(&AesInstance, SecurePtr->CsuDmaInstPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (SecurePtr->BlockNum == 0x0) {
		XSecure_ReleaseReset(AesInstance.BaseAddress,
					XSECURE_AES_SOFT_RST_OFFSET);
		/* Clear all key zeroization register */
		XPlmi_Out32((AesInstance.BaseAddress +
			XSECURE_AES_KEY_CLEAR_OFFSET), 0x00000000U);
		Status = XLoader_AesKeySelct(SecurePtr, &AesInstance, &KeySrc);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		/* Decrypting SH */
		Status = XSecure_AesDecryptInit(&AesInstance, KeySrc,
					XSECURE_AES_KEY_SIZE_256,
					(UINTPTR)SecurePtr->PrtnHdr->PrtnIv);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		/* Decrypt Secure header */
		Status = XLoader_DecryptSH(SecurePtr, &AesInstance, SrcAddr);
		SrcAddr = SrcAddr + 64;

	}

	for (Index = 0; Index < 4; Index++) {
		Iv[Index] = Xil_Htonl(XSecure_ReadReg(AesInstance.BaseAddress,
				(0x40 + (Index *4))));
	}

	Status = XSecure_AesDecryptInit(&AesInstance,
		XSECURE_AES_KUP_KEY, XSECURE_AES_KEY_SIZE_256, (UINTPTR)Iv);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XLoader_DataDecrypt(SecurePtr,
			&AesInstance, SrcAddr, DstAddr, Size);
	if (Status != XST_SUCCESS) {
		goto END;
	}



END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * This function helps in key selection.
 *
 * @param	SecurePtr	Pointer to the XLoader_SecureParms
 *
 * @return	XST_SUCCESS if verification was successful.
 *
 ******************************************************************************/
static u32 XLoader_AesKeySelct(XLoader_SecureParms *SecurePtr,
				XSecure_Aes *AesInstancePtr,
				XSecure_AesKeySrc *KeySrc)
{
	u32 Status = XST_SUCCESS;
	/* TBD can use different IV other than one used by ROM */
	u32 *KekIv = SecurePtr->PdiPtr->MetaHdr.BootHdr.KekIv;
	u32 *BhKey = SecurePtr->PdiPtr->MetaHdr.BootHdr.Kek;

	switch (SecurePtr->PrtnHdr->EncStatus) {
	case XLOADER_BBRAM_KEY:
			*KeySrc = XSECURE_AES_BBRAM_KEY;
			break;
	case XLOADER_EFUSE_KEY:
			*KeySrc = XSECURE_AES_EFUSE_KEY;
			break;
	case XLOADER_BBRAM_OBFUS_KEY:
		XPlmi_Printf(DEBUG_DETAILED,
			"Decryting BBRAM obfuscated key\n\r");
		Status = XSecure_AesKekDecrypt(AesInstancePtr,
				XSECURE_OBFUSCATED_KEY, XSECURE_AES_BBRAM_KEY,
				XSECURE_AES_BBRAM_RED_KEY, (UINTPTR)KekIv,
			XSECURE_AES_KEY_SIZE_256);
		*KeySrc = XSECURE_AES_BBRAM_RED_KEY;
		break;
	case XLOADER_EFUSE_OBFUS_KEY:
		XPlmi_Printf(DEBUG_DETAILED,
			"Decryting efuse obfuscated key\n\r");
		Status = XSecure_AesKekDecrypt(AesInstancePtr,
				XSECURE_OBFUSCATED_KEY, XSECURE_AES_EFUSE_KEY,
				XSECURE_AES_EFUSE_RED_KEY, (UINTPTR)KekIv,
			XSECURE_AES_KEY_SIZE_256);
		*KeySrc = XSECURE_AES_EFUSE_RED_KEY;
		break;
	case XLOADER_BH_OBFUS_KEY:
		XPlmi_Printf(DEBUG_DETAILED,
			"Decryting BH obfuscated key\n\r");
		/* Write BH key into BH registers */
		Status = XSecure_AesWriteKey(AesInstancePtr,
				XSECURE_AES_BH_KEY, XSECURE_AES_KEY_SIZE_256,
					(UINTPTR)BhKey);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XSecure_AesKekDecrypt(AesInstancePtr,
				XSECURE_OBFUSCATED_KEY, XSECURE_AES_BH_KEY,
				XSECURE_AES_BH_RED_KEY, (UINTPTR)KekIv,
			XSECURE_AES_KEY_SIZE_256);
		*KeySrc = XSECURE_AES_BH_RED_KEY;
		break;
	default: Status = XST_FAILURE;
			break;

	}

END:
	return Status;
}

