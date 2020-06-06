/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
*       har  08/22/19 Fixed MISRA C violations
*       har  02/13/20 Added code to return error codes
* 		rpo  02/25/20 Added SHA, RSA, ECDSA, AES KAT support
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xloader_secure.h"
#include "xilpdi.h"
#include "xplmi_dma.h"
#include "xsecure_ecdsa_rsa_hw.h"
#include "xsecure_error.h"
#include "xsecure_aes_core_hw.h"
#include "xsecure_aes.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
* @brief	This function returns authentication type by reading
* authentication header.
*
* @param	AcPtr is pointer to the XLoader_AuthCertificate instance.
*
* @return	- XLOADER_AC_AH_PUB_ALG_RSA
*		- XLOADER_AC_AH_PUB_ALG_ECDSA
*
******************************************************************************/
static INLINE u32 XLoader_GetAuthType(const XLoader_AuthCertificate *AcPtr)
{
	return (AcPtr->AuthHdr & XLOADER_AC_AH_PUB_ALG_MASK);
}

/************************** Function Prototypes ******************************/

static u32 XLoader_VerifyHashNUpdateNext(XLoader_SecureParms *SecurePtr,
	u32 Size, u8 Last);
static u32 XLoader_SpkAuthentication(XLoader_SecureParms *SecurePtr);
static u32 XLoader_DataAuth(XLoader_SecureParms *SecurePtr, u8 *Hash,
	u8 *Signature);
static inline void XLoader_I2Osp(u32 Integer, u32 Size, u8 *Convert);
static u32 XLoader_EcdsaSignVerify(u32 *Hash, u32 *Key, u32 *Signature);
static u32 XLoader_RsaSignVerify(XLoader_SecureParms *SecurePtr,
	u8 *Hash, XLoader_RsaKey *Key, u8 *Signature);
static u32 XLoader_VerifySignature(XLoader_SecureParms *SecurePtr,
	u8 *Hash, XLoader_RsaKey *Key, u8 *Signature);
static u32 XLoader_AesDecryption(XLoader_SecureParms *SecurePtr,
	u64 SrcAddr, u64 DestAddr, u32 Size);
static u32 XLoader_AesKeySelect(XLoader_SecureParms *SecurePtr,
	XLoader_AesKekKey *KeyDetails, XSecure_AesKeySrc *KeySrc);
static u32 XLoader_CheckNonZeroPpk(void);
static u32 XLoader_PpkVerify(XLoader_SecureParms *SecurePtr);
static u32 XLoader_IsPpkValid(u8 PpkSelect, u8 *PpkHash);
static u32 XLoader_VerifyRevokeId(u32 RevokeId);
static u32 XLoader_PpkCompare(u32 EfusePpkOffset, u8 *PpkHash);
static u32 XLoader_AuthHdrs(XLoader_SecureParms *SecurePtr,
	XilPdi_MetaHdr *MetaHdr);
static u32 XLoader_ReadHdrs(XLoader_SecureParms *SecurePtr,
	XilPdi_MetaHdr *MetaHdr, u64 BufferAddr);
static u32 XLoader_DecHdrs(XLoader_SecureParms *SecurePtr,
	XilPdi_MetaHdr *MetaHdr, u64 BufferAddr);
static u32 XLoader_AuthNDecHdrs(XLoader_SecureParms *SecurePtr,
	XilPdi_MetaHdr *MetaHdr, u64 BufferAddr);
static u32 XLoader_SetAesDpaCm(XSecure_Aes *AesInstPtr, u32 DpaCmCfg);
static u32 XLoader_DecryptBlkKey(XSecure_Aes *AesInstPtr,
	XLoader_AesKekKey *KeyDetails);
static u32 XLoader_AesKatTest(XLoader_SecureParms *SecurePtr);

/************************** Variable Definitions *****************************/
static XLoader_AuthCertificate AuthCert;

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* @brief	This function initializes  XLoader_SecureParms's instance.
*
* @param	SecurePtr is pointer to the XLoader_SecureParms instance.
* @param	PdiPtr is pointer to the XilPdi instance
* @param	PrtnNum is the partition number to be processed
*
* @return	XLOADER_SUCCESS on success and error code on failure
*
******************************************************************************/
u32 XLoader_SecureInit(XLoader_SecureParms *SecurePtr, XilPdi *PdiPtr,
	u32 PrtnNum)
{
	u32 Status = XLOADER_FAILURE;
	XilPdi_PrtnHdr *PrtnHdr;
	u32 ChecksumOffset;
	u32 AcOffset;

	memset(SecurePtr, 0U, sizeof(XLoader_SecureParms));

	/* Assign the partition header to local variable */
	PrtnHdr = &(PdiPtr->MetaHdr.PrtnHdr[PrtnNum]);
	SecurePtr->PdiPtr = PdiPtr;
	SecurePtr->ChunkAddr = XPLMI_LOADER_CHUNK_MEMORY;
	SecurePtr->BlockNum = 0x00U;
	SecurePtr->PrtnHdr = PrtnHdr;

	/* Get DMA instance */
	SecurePtr->PmcDmaInstPtr = XPlmi_GetDmaInstance(PMCDMA_0_DEVICE_ID);
	if (SecurePtr->PmcDmaInstPtr == NULL) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_INIT_GET_DMA, 0x0U);
		goto END;
	}

	/* Check if checksum is enabled */
	if ((PrtnHdr->PrtnAttrb & XIH_PH_ATTRB_CHECKSUM_MASK) != 0x00U) {
		 XPlmi_Printf(DEBUG_INFO,
			 "Checksum verification is enabled\n\r");

		/* Check checksum type */
		if((PrtnHdr->PrtnAttrb & XIH_PH_ATTRB_CHECKSUM_MASK) ==
			XIH_PH_ATTRB_HASH_SHA3) {
			SecurePtr->IsCheckSumEnabled = TRUE;
			SecurePtr->SecureEn = TRUE;
		}
		else {
			/* Only SHA3 checksum is supported */
			Status = XPLMI_UPDATE_STATUS(
				XLOADER_ERR_INIT_INVALID_CHECKSUM_TYPE, 0x0U);
			goto END;
		}
		ChecksumOffset = SecurePtr->PdiPtr->MetaHdr.FlashOfstAddr +
				(SecurePtr->PrtnHdr->ChecksumWordOfst *
					XIH_PRTN_WORD_LEN);

		/* Copy checksum hash */
		Status = SecurePtr->PdiPtr->DeviceCopy(ChecksumOffset,
				(UINTPTR)SecurePtr->Sha3Hash, XLOADER_SHA3_LEN, 0U);
		if (Status != XLOADER_SUCCESS){
			Status = XPLMI_UPDATE_STATUS(
				XLOADER_ERR_INIT_CHECKSUM_COPY_FAIL, Status);
			goto END;
		}
	}
	/* Check if authentication is enabled */
	if (PrtnHdr->AuthCertificateOfst != 0x00U) {
		 XPlmi_Printf(DEBUG_INFO,
			 "Authentication is enabled\n\r");

		SecurePtr->IsAuthenticated = TRUE;
		SecurePtr->SecureEn = TRUE;
		/* Copy Authentication certificate */
		AcOffset = SecurePtr->PdiPtr->MetaHdr.FlashOfstAddr +
			(SecurePtr->PrtnHdr->AuthCertificateOfst *
				XIH_PRTN_WORD_LEN);
		SecurePtr->AcPtr = &AuthCert;
		Status = SecurePtr->PdiPtr->DeviceCopy(AcOffset,
				(UINTPTR)SecurePtr->AcPtr,
				XLOADER_AUTH_CERT_MIN_SIZE, 0U);
		if (Status != XLOADER_SUCCESS) {
			Status = XPLMI_UPDATE_STATUS(
					XLOADER_ERR_INIT_AC_COPY_FAIL, Status);
			goto END;
		}
	}
	/* Check if encryption is enabled */
	if (PrtnHdr->EncStatus != 0x00U) {
		 XPlmi_Printf(DEBUG_INFO,
			"Encryption is enabled\n\r");
		SecurePtr->IsEncrypted = TRUE;
		SecurePtr->SecureEn = TRUE;
	}

	/* Checksum could not be enabled with authentication or encryption */
	if ((SecurePtr->IsCheckSumEnabled == TRUE) &&
		((SecurePtr->IsAuthenticated == TRUE) ||
		 (SecurePtr->IsEncrypted == TRUE))) {
		Status = XPLMI_UPDATE_STATUS(
			XLOADER_ERR_INIT_CHECKSUM_INVLD_WITH_AUTHDEC, 0x0U);
		goto END;
	}

	Status = XLOADER_SUCCESS;

END:

	return Status;
}

/*****************************************************************************/
/**
* @brief	This function loads secure non-cdo partitions.
*
* @param	SecurePtr is pointer to the XLoader_SecureParms instance.
* @param	DestAddr is load address of the partition
* @param	Size is unencrypted size of the partition.
*
* @return	XLOADER_SUCCESS on success and error code on failure
*
******************************************************************************/
u32 XLoader_SecureCopy(XLoader_SecureParms *SecurePtr, u64 DestAddr, u32 Size)
{
	u32 Status = XLOADER_FAILURE;
	int ClrStatus = XST_FAILURE;
	u32 ChunkLen = XLOADER_CHUNK_SIZE;
	u32 Len = Size;
	u64 LoadAddr = DestAddr;
	u8 LastChunk = FALSE;

	while (Len > 0U) {
		/* Update the length for last chunk */
		if (Len <= ChunkLen) {
			LastChunk = TRUE;
			ChunkLen = Len;
		}

		/* Call security function */
		Status = XLoader_ProcessSecurePrtn(SecurePtr, LoadAddr,
					ChunkLen, LastChunk);
		if (Status != XLOADER_SUCCESS) {
			goto END;
		}

		if (SecurePtr->IsEncrypted != TRUE) {
			/* Copy to destination address */
			Status = XPlmi_DmaXfr((u64)SecurePtr->SecureData,
						(u64)LoadAddr,
						SecurePtr->SecureDataLen / XIH_PRTN_WORD_LEN,
						XPLMI_PMCDMA_0);
			if (Status != XST_SUCCESS) {
				Status = XPLMI_UPDATE_STATUS(
					XLOADER_ERR_DMA_TRANSFER, Status);
				goto END;
			}
		}

		/* Update variables for next chunk */
		LoadAddr = LoadAddr + SecurePtr->SecureDataLen;
		Len = Len - SecurePtr->SecureDataLen;
	}

END:
	if (Status != XLOADER_SUCCESS) {
		/* On failure clear data at destination address is cleared */
		ClrStatus = XPlmi_InitNVerifyMem(DestAddr, Size);
		if (ClrStatus != XST_SUCCESS) {
			Status = Status | XLOADER_SEC_BUF_CLEAR_ERR;
		}
		else {
			Status = Status | XLOADER_SEC_BUF_CLEAR_SUCCESS;
		}
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief	This function performs authentication, checksum and decryption
* of the partition.
*
* @param	SecurePtr is pointer to the XLoader_SecureParms instance
* @param	DestAddr is the address to which data is copied
* @param	BlockSize is size of the data block to be processed
*		which doesn't include padding lengths and hash.
* @param	Last notifies if the block to be processed is last or not
*
* @return	XLOADER_SUCCESS on success and error code on failure
*
******************************************************************************/

u32 XLoader_ProcessSecurePrtn(XLoader_SecureParms *SecurePtr, u64 DestAddr,
				u32 BlockSize, u8 Last)
{
	u32 Status = XLOADER_FAILURE;
	int ClrStatus = XST_FAILURE;
	u32 TotalSize = BlockSize;
	u32 SrcAddr;
	u64 OutAddr;

	XPlmi_Printf(DEBUG_DETAILED,
			"Processing Block %d \n\r", SecurePtr->BlockNum);
	/* 1st block */
	if (SecurePtr->BlockNum == 0x0U) {
		SrcAddr = SecurePtr->PdiPtr->MetaHdr.FlashOfstAddr +
				(SecurePtr->PrtnHdr->DataWordOfst * XIH_PRTN_WORD_LEN);
		if (SecurePtr->IsEncrypted == TRUE) {
			SecurePtr->RemainingEncLen =
					SecurePtr->PrtnHdr->EncDataWordLen * XIH_PRTN_WORD_LEN;
			/* Verify encrypted partition is revoked or not */
			Status = XLoader_VerifyRevokeId(SecurePtr->PrtnHdr->EncRevokeID);
			if (Status != XLOADER_SUCCESS) {
				XPlmi_Printf(DEBUG_GENERAL, "Partition is revoked\n\r");
				goto END;
			}
		}
	}
	else {
		SrcAddr = SecurePtr->NextBlkAddr;
	}

	if (SecurePtr->IsEncrypted == TRUE) {
		if (Last == TRUE) {
			TotalSize = SecurePtr->RemainingEncLen;
		}
		else {
			if (SecurePtr->BlockNum == 0U)  {
				/* To include Secure Header */
				TotalSize = TotalSize + XLOADER_SECURE_HDR_TOTAL_SIZE;
			}
		}
	}

	/*
	 * If authentication or checksum is enabled validate the data hash
	 * with expected hash
	 */
	if ((SecurePtr->IsAuthenticated == TRUE) ||
			(SecurePtr->IsCheckSumEnabled == TRUE)) {
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
		Status = XLoader_VerifyHashNUpdateNext(SecurePtr, TotalSize, Last);
		if (Status != XLOADER_SUCCESS) {
			goto END;
		}

	}

	/* If encryption is enabled */
	if (SecurePtr->IsEncrypted == TRUE) {
		if (SecurePtr->IsAuthenticated != TRUE) {
			SecurePtr->PdiPtr->DeviceCopy(SrcAddr,
				SecurePtr->ChunkAddr, TotalSize, 0U);
			SecurePtr->SecureData = SecurePtr->ChunkAddr;
			SecurePtr->SecureDataLen = TotalSize;
		}

		if (SecurePtr->IsCdo != TRUE) {
			OutAddr = DestAddr;
		}
		else {
			OutAddr = SecurePtr->SecureData;
		}
		Status = XLoader_AesDecryption(SecurePtr,
					SecurePtr->SecureData, OutAddr, SecurePtr->SecureDataLen);
		if (Status != XLOADER_SUCCESS) {
			Status = XPLMI_UPDATE_STATUS(
					XLOADER_ERR_PRTN_DECRYPT_FAIL, Status);
			goto END;
		}
	}

	SecurePtr->NextBlkAddr = SrcAddr + TotalSize;
	SecurePtr->BlockNum++;

END:
	/* Clears whole intermediate buffers on failure */
	if (Status != XLOADER_SUCCESS) {
		ClrStatus = XPlmi_InitNVerifyMem(SecurePtr->ChunkAddr, TotalSize);
		if (ClrStatus != XST_SUCCESS) {
			Status = Status | XLOADER_SEC_BUF_CLEAR_ERR;
		}
		else {
			Status = Status | XLOADER_SEC_BUF_CLEAR_SUCCESS;
		}
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief	This function checks if authentication/encryption is compulsory.
*
* @param	SecurePtr is pointer to the XLoader_SecureParms instance.
*
* @return	XLOADER_SUCCESS on success and error code on failure
*
******************************************************************************/
u32 XLoader_SecureValidations(XLoader_SecureParms *SecurePtr)
{
	u32 Status = XLOADER_FAILURE;
	XilPdi_BootHdr *BootHdr = &SecurePtr->PdiPtr->MetaHdr.BootHdr;

	XPlmi_Printf(DEBUG_INFO,
		"Performing security checks \n\r");
	/*
	 * Checking if authentication is compulsory
	 * If bits in PPK0/1/2 is programmed bh_auth is not allowed
	 */
	Status = XLoader_CheckNonZeroPpk();
	/* Authentication is compulsory */
	if (Status == XLOADER_SUCCESS) {
		if (SecurePtr->IsAuthenticated == FALSE) {
			XPlmi_Printf(DEBUG_INFO,
				"HWROT is enabled, non authenticated PDI is"
				" not allowed\n\r");
			Status = XPLMI_UPDATE_STATUS(
				XLOADER_ERR_HWROT_EFUSE_AUTH_COMPULSORY, 0x0U);
			goto END;
		}
		else {
			if (XilPdi_IsBhdrAuthEnable(BootHdr) != 0x00U) {
				XPlmi_Printf(DEBUG_INFO,
				"Boot header authentication is not allowed"
				 "when HWROT is enabled\n\r");
				Status = XPLMI_UPDATE_STATUS(
				XLOADER_ERR_HWROT_BH_AUTH_NOT_ALLOWED, 0x0U);
				goto END;
			}
			else {
				/*
				 * Authentication is true and BHDR
				 * authentication is not enabled
				 */
				XPlmi_Printf(DEBUG_DETAILED,
				"HWROT- Authentication is enabled\n\r");
			}
		}
	}
	else {
		if (SecurePtr->IsAuthenticated == TRUE) {
			if (XilPdi_IsBhdrAuthEnable(BootHdr) == 0x00U) {
				XPlmi_Printf(DEBUG_INFO,
				"eFUSE PPK(s) are zero and Boot header authentication"
				" is disabled, loading PDI with authentication"
				" enabled is not allowed\n\r");
				Status = XPLMI_UPDATE_STATUS(
				  XLOADER_ERR_AUTH_EN_PPK_HASH_ZERO, 0x0U);
				goto END;
			}
			else {
				/*
				 * BHDR authentication is
				 * enabled and PPK hash is not programmed
				 */
				XPlmi_Printf(DEBUG_INFO,
					"Authentication with BH enabled\n\r");
			}
		}
		else {
			/* Authentication is not compulsory */
			XPlmi_Printf(DEBUG_DETAILED,
				"Authentication is not enabled\n\r");
		}
	}

	/* Checking if encryption is compulsory */
	if ((XPlmi_In32(XLOADER_EFUSE_SEC_MISC0_OFFSET) &
		XLOADER_EFUSE_SEC_DEC_MASK) != 0x0U) {
		if (SecurePtr->IsEncrypted == FALSE) {
			XPlmi_Printf(DEBUG_INFO, "DEC_ONLY mode is set,"
			" non encrypted meta header is not allowed\n\r");
			Status = XPLMI_UPDATE_STATUS(
				XLOADER_ERR_ENCONLY_ENC_COMPULSORY, 0x0U);
			goto END;
		}
		else {
			XPlmi_Printf(DEBUG_INFO, "Encryption is enabled\n\r");
			Status = XLOADER_SUCCESS;
			/*
			 * When DEC only is set, meta header should be decrypted
			 * with only efuse black key
			 */
			if (SecurePtr->PdiPtr->MetaHdr.ImgHdrTbl.EncKeySrc !=
				XLOADER_EFUSE_BLK_KEY) {
				Status = XLOADER_FAILURE;
				goto END;
			}
		}
	}
	else {
		/* Header encryption is not compulsory */
		XPlmi_Printf(DEBUG_DETAILED, "Encryption is not enabled\n\r");
		Status = XLOADER_SUCCESS;
	}

	Status = XLOADER_SUCCESS;

END:
	return Status;

}

/*****************************************************************************/
/**
* @brief	This function authenticates the image header table
*
* @param	SecurePtr is pointer to the XLoader_SecureParms instance
*
* @return	XLOADER_SUCCESS on success and error code on failure
*
******************************************************************************/
u32 XLoader_ImgHdrTblAuth(XLoader_SecureParms *SecurePtr)
{
	u32 Status = XLOADER_FAILURE;
	int ClrStatus = XST_FAILURE;
	Sha3Hash HashVal;
	XSecure_Sha3 Sha3Instance;
	u32 AcOffset;
	XilPdi_ImgHdrTbl *ImgHdrTbl =
		&SecurePtr->PdiPtr->MetaHdr.ImgHdrTbl;

	XPlmi_Printf(DEBUG_DETAILED, "Authentication of"
			" Image header table\n\r");

	/* Get DMA instance */
	SecurePtr->PmcDmaInstPtr = XPlmi_GetDmaInstance(PMCDMA_0_DEVICE_ID);
	if (SecurePtr->PmcDmaInstPtr == NULL) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_IHT_GET_DMA, 0x0U);
		goto END;
	}

	/* Copy Authentication certificate */
	AcOffset = SecurePtr->PdiPtr->MetaHdr.FlashOfstAddr +
			(ImgHdrTbl->AcOffset * XIH_PRTN_WORD_LEN);

	SecurePtr->AcPtr = &AuthCert;
	Status = SecurePtr->PdiPtr->DeviceCopy(AcOffset,
		(UINTPTR)SecurePtr->AcPtr, XLOADER_AUTH_CERT_MIN_SIZE, 0U);
	if (Status != XLOADER_SUCCESS) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_IHT_COPY_FAIL,
				Status);
		goto END;
	}

	if ((SecurePtr->PdiPtr->PlmKatStatus & XLOADER_SHA3_KAT_MASK) == 0U) {
		/*
		 * Skip running the KAT for SHA3 if it is already run by ROM
		 * KAT will be run only when the CYRPTO_KAT_EN bits in eFUSE are set
		 */
		Status = XSecure_Sha3Initialize(&Sha3Instance, SecurePtr->PmcDmaInstPtr);
		if (Status != XLOADER_SUCCESS) {
			Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_PRTN_HASH_CALC_FAIL,
						Status);
			goto END;
		}
		Status = XSecure_Sha3Kat(&Sha3Instance);
		if(Status != XLOADER_SUCCESS) {
			XPlmi_Printf(DEBUG_GENERAL, "SHA3 KAT Failed\n\r");
			Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_KAT_FAILED, Status);
			goto END;
		}
		SecurePtr->PdiPtr->PlmKatStatus |= XLOADER_SHA3_KAT_MASK;
	}

	/* calculate hash of the image header table */
	Status = XSecure_Sha3Initialize(&Sha3Instance, SecurePtr->PmcDmaInstPtr);
	if (Status != XLOADER_SUCCESS) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_IHT_HASH_CALC_FAIL,
					Status);
		goto END;
	}
	Status = XSecure_Sha3Digest(&Sha3Instance, (u8 *)ImgHdrTbl, XIH_IHT_LEN,
								&HashVal);
	if (Status != XLOADER_SUCCESS) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_IHT_HASH_CALC_FAIL,
					Status);
		goto END;
	}

	XPlmi_PrintArray(DEBUG_INFO, (UINTPTR)HashVal.Hash,
		XLOADER_SHA3_LEN / XIH_PRTN_WORD_LEN, "IHT Hash");

	/* Authenticating Image header table */
	Status = XLoader_DataAuth(SecurePtr, HashVal.Hash,
			(u8 *)SecurePtr->AcPtr->BHSignature);
	if (Status != XLOADER_SUCCESS) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_IHT_AUTH_FAIL, Status);
		XPlmi_Printf(DEBUG_INFO, "Authentication of image header table "
					"is failed\n\r");
		goto END;
	}

	XPlmi_Printf(DEBUG_INFO, "Authentication of Image header table is "
				"successful\n\r");

END:
	if (Status != XLOADER_SUCCESS) {
		/* On failure clear IHT structure which has invalid data */
		ClrStatus = XPlmi_InitNVerifyMem((UINTPTR)ImgHdrTbl, XIH_IHT_LEN);
		if (ClrStatus != XST_SUCCESS) {
			Status = Status | XLOADER_SEC_BUF_CLEAR_ERR;
		}
		else {
			Status = Status | XLOADER_SEC_BUF_CLEAR_SUCCESS;
		}
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief	This function authenticates and/or decrypts the image headers
* and partition headers and copies the contents to the corresponding structures.
*
* @param	SecurePtr	Pointer to the XLoader_SecureParms instance.
* @param	MetaHdr		Pointer to the Meta header table.
*
* @return	XLOADER_SUCCESS on success and error code on failure
*
******************************************************************************/
u32 XLoader_ReadAndVerifySecureHdrs(XLoader_SecureParms *SecurePtr,
	XilPdi_MetaHdr *MetaHdr)
{
	u32 Status = XLOADER_FAILURE;
	u32 ClearIHs;
	u32 ClearPHs;
	u32 Ihs;

	XPlmi_Printf(DEBUG_DETAILED,
		"Loading secure image headers and partition headers\n\r");

	/*
	 * If headers are in encrypted format
	 * either authentication is enabled or not
	 */
	if (SecurePtr->IsEncrypted == TRUE) {

		/* Get DMA instance */
		SecurePtr->PmcDmaInstPtr = XPlmi_GetDmaInstance(PMCDMA_0_DEVICE_ID);
		if (SecurePtr->PmcDmaInstPtr == NULL) {
			Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_HDR_GET_DMA, 0x0U);
			goto END;
		}

		/* Initialize AES driver */
		Status = XSecure_AesInitialize(&SecurePtr->AesInstance, SecurePtr->PmcDmaInstPtr);
		if (Status != XLOADER_SUCCESS) {
			XPlmi_Printf(DEBUG_INFO,"Failed at XSecure_AesInitialize\n\r");
			Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_HDR_AES_OP_FAIL,
					Status);
			goto END;
		}

		Status = XLoader_AesKatTest(SecurePtr);
		if (Status != XLOADER_SUCCESS) {
			XPlmi_Printf(DEBUG_INFO, "Failed at AES KAT test\n\r");
			goto END;
		}

		XPlmi_Printf(DEBUG_INFO, "Headers are in encrypted format\n\r");
		SecurePtr->ChunkAddr = XPLMI_LOADER_CHUNK_MEMORY;

		/* Read headers to a buffer */
		Status = XLoader_ReadHdrs(SecurePtr, MetaHdr,
				SecurePtr->ChunkAddr);
		if (Status != XLOADER_SUCCESS) {
			goto END;
		}

		/* Authenticate headers and decrypt the headers */
		if (SecurePtr->IsAuthenticated == TRUE) {
			XPlmi_Printf(DEBUG_INFO, "Authentication enabled\n\r");
			Status = XLoader_AuthNDecHdrs(SecurePtr, MetaHdr,
						SecurePtr->ChunkAddr);
		}
		/* Decrypt the headers */
		else {
			Status = XLoader_DecHdrs(SecurePtr, MetaHdr,
						SecurePtr->ChunkAddr);
		}
		if (Status != XLOADER_SUCCESS) {
			goto END;
		}

		/* Read and verify headers to structures */
		MetaHdr->Flag = XILPDI_METAHDR_RD_HDRS_FROM_MEMBUF;
		MetaHdr->BufferAddr = SecurePtr->ChunkAddr;
		MetaHdr->XMemCpy = XPlmi_MemCpy;

		/* Read IHT and PHT to structures and verify checksum */
		Status = XilPdi_ReadAndVerifyImgHdr(MetaHdr);
		if (Status != XLOADER_SUCCESS) {
			Status = XPLMI_UPDATE_STATUS(
				XLOADER_ERR_SEC_IH_READ_VERIFY_FAIL, Status);
			goto END;
		}
		/* Verify Meta header is revoked or not */
		for (Ihs = 0U; Ihs < MetaHdr->ImgHdrTbl.NoOfImgs; Ihs++) {
			Status = XLoader_VerifyRevokeId(MetaHdr->ImgHdr[Ihs].EncRevokeID);
			if (Status != XLOADER_SUCCESS) {
				XPlmi_Printf(DEBUG_GENERAL, "Meta header is revoked\n\r");
				goto END;
			}
		}
		/* Update buffer address to point to PHs */
		MetaHdr->BufferAddr = SecurePtr->ChunkAddr +
					(MetaHdr->ImgHdrTbl.NoOfImgs * XIH_IH_LEN);
		Status = XilPdi_ReadAndVerifyPrtnHdr(MetaHdr);
		if(Status != XLOADER_SUCCESS) {
			Status = XPLMI_UPDATE_STATUS(
				XLOADER_ERR_SEC_PH_READ_VERIFY_FAIL, Status);
			goto END;
		}
	}
	/* If authentication is enabled */
	else if (SecurePtr->IsAuthenticated == TRUE) {
		XPlmi_Printf(DEBUG_INFO, "Headers are only authenticated\n\r");
		Status = XLoader_AuthHdrs(SecurePtr, MetaHdr);
		if (Status != XLOADER_SUCCESS) {
			goto END;
		}
	}
	else {
		XPlmi_Printf(DEBUG_INFO, "Headers are not secure\n\r");
		Status = XLOADER_ERR_HDR_NOT_SECURE;
		goto END;
	}
END:
	if ((Status != XLOADER_SUCCESS) &&
		(Status != XLOADER_ERR_HDR_NOT_SECURE)) {
		ClearIHs = XPlmi_InitNVerifyMem((UINTPTR)&MetaHdr->ImgHdr[0U],
			(MetaHdr->ImgHdrTbl.NoOfImgs * XIH_IH_LEN));
		ClearPHs = XPlmi_InitNVerifyMem((UINTPTR)&MetaHdr->PrtnHdr[0U],
			(MetaHdr->ImgHdrTbl.NoOfPrtns * XIH_PH_LEN));
		if ((ClearIHs != XLOADER_SUCCESS) || (ClearPHs != XLOADER_SUCCESS)) {
			Status = Status | XLOADER_SEC_BUF_CLEAR_ERR;
		}
		else {
			Status = Status | XLOADER_SEC_BUF_CLEAR_SUCCESS;
		}
	}
	else {
		if (Status == XLOADER_ERR_HDR_NOT_SECURE) {
			Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_HDR_NOT_SECURE, 0U);
		}
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function updates KEK red key availability status from
 * boot header.
 *
 * @param	PdiPtr is pointer to the XilPdi instance.
 *
 * @return	None.
 *
 ******************************************************************************/
void XLoader_UpdateKekRdKeyStatus(XilPdi *PdiPtr)
{
	PdiPtr->KekStatus = 0x0U;

	XPlmi_Printf(DEBUG_INFO, "Identifying KEK's corresponding RED "
			"key availability status\n\r");
	switch(PdiPtr->MetaHdr.BootHdr.EncStatus) {
	case XLOADER_BH_BLK_KEY:
	case XLOADER_BH_OBFUS_KEY:
		PdiPtr->KekStatus = XLOADER_BHDR_RED_KEY;
		break;
	case XLOADER_BBRAM_BLK_KEY:
	case XLOADER_BBRAM_OBFUS_KEY:
		PdiPtr->KekStatus = XLOADER_BBRAM_RED_KEY;
		break;
	case XLOADER_EFUSE_BLK_KEY:
	case XLOADER_EFUSE_OBFUS_KEY:
		PdiPtr->KekStatus = XLOADER_EFUSE_RED_KEY;
		break;
	default:
		/* No KEK is available for PLM */
		PdiPtr->KekStatus = 0x0U;
		break;
	}
	XPlmi_Printf(DEBUG_DETAILED, "KEK red key available after "
			"for PLM %x\n\r", PdiPtr->KekStatus);
}

/*****************************************************************************/
/**
* @brief	This function calculates hash and compares with expected hash.
* If authentication is enabled hash, is calculated on AC + Data for first block,
* encrypts the ECDSA/RSA signature and compares it with the expected hash.
* For checksum and authentication(after first block), hash is calculated on block
* of data and compared with the expected hash.
*
* @param	SecurePtr is pointer to the XLoader_SecureParms instance.
* @param	Size is size of the data block to be processed
*		which includes padding lengths and hash.
* @param	Last notifies if the block to be processed is last or not.
*
* @return	XLOADER_SUCCESS on success and error code on failure
*
******************************************************************************/
static u32 XLoader_VerifyHashNUpdateNext(XLoader_SecureParms *SecurePtr,
	u32 Size, u8 Last)
{
	u32 Status = XLOADER_FAILURE;
	XSecure_Sha3 Sha3Instance;
	u8 *Data = (u8 *)SecurePtr->ChunkAddr;
	Sha3Hash CalHash = {0U};
	u8 *ExpHash = (u8 *)SecurePtr->Sha3Hash;
	XLoader_AuthCertificate *AcPtr=
		(XLoader_AuthCertificate *)SecurePtr->AcPtr;
	if (SecurePtr->PmcDmaInstPtr == NULL) {
		goto END;
	}

	Status = XSecure_Sha3Initialize(&Sha3Instance, SecurePtr->PmcDmaInstPtr);
	if (Status != XLOADER_SUCCESS) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_PRTN_HASH_CALC_FAIL,
				Status);
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
		if (Status != XLOADER_SUCCESS) {
			Status = XPLMI_UPDATE_STATUS(
				XLOADER_ERR_PRTN_HASH_CALC_FAIL, Status);
			goto END;
		}
	}

	Status = XSecure_Sha3Update(&Sha3Instance, Data, Size);
	if (Status != XLOADER_SUCCESS) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_PRTN_HASH_CALC_FAIL,
							Status);
		goto END;
	}

	Status = XSecure_Sha3Finish(&Sha3Instance, &CalHash);
	if (Status != XLOADER_SUCCESS) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_PRTN_HASH_CALC_FAIL,
							Status);
		goto END;
	}

	/* Verify the hash */
	if ((SecurePtr->IsAuthenticated == TRUE) &&
			(SecurePtr->BlockNum == 0x00U)) {
		Status = XLoader_DataAuth(SecurePtr, CalHash.Hash,
					(u8 *)SecurePtr->AcPtr->ImgSignature);
		if (Status != XLOADER_SUCCESS) {
			Status = XPLMI_UPDATE_STATUS(
					XLOADER_ERR_PRTN_AUTH_FAIL, Status);
			goto END;
		}
	}
	else {
		Status = XPlmi_MemCmp(ExpHash, CalHash.Hash, XLOADER_SHA3_LEN);
		if (Status != XLOADER_SUCCESS) {
			XPlmi_Printf(DEBUG_INFO,"Hash mismatch error\n\r");
			XPlmi_PrintArray(DEBUG_INFO,
			(UINTPTR)CalHash.Hash, XLOADER_SHA3_LEN / XIH_PRTN_WORD_LEN,
				"Calculated Hash");
			XPlmi_PrintArray(DEBUG_INFO, (UINTPTR)ExpHash,
				XLOADER_SHA3_LEN / XIH_PRTN_WORD_LEN,
				"Expected Hash");
			Status = XPLMI_UPDATE_STATUS(
				XLOADER_ERR_PRTN_HASH_COMPARE_FAIL, Status);
			goto END;
		}
	}

	/* Update the next expected hash  and data location */
	if (Last == 0x00U) {
		(void *)XPlmi_MemCpy(ExpHash, Data, XLOADER_SHA3_LEN);
		/* Here Authentication overhead is removed in the chunk */
		SecurePtr->SecureData = (UINTPTR)Data + XLOADER_SHA3_LEN;
		SecurePtr->SecureDataLen = Size - XLOADER_SHA3_LEN;
	}
	else {
		/* This is the last block */
		SecurePtr->SecureData = (UINTPTR)Data;
		SecurePtr->SecureDataLen = Size;
	}

	Status = XLOADER_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function authenticates the data with SPK.
*
* @param	SecurePtr is pointer to the XLoader_SecureParms instance.
* @param	Hash is a Pointer to the expected hash buffer.
* @param	Signature pointer points to the signature buffer.
*
* @return	XLOADER_SUCCESS on success and error code on failure
*
******************************************************************************/
static u32 XLoader_DataAuth(XLoader_SecureParms *SecurePtr, u8 *Hash,
	u8 *Signature)
{
	u32 Status = XLOADER_FAILURE;
	XLoader_AuthCertificate *AcPtr =
		(XLoader_AuthCertificate *)SecurePtr->AcPtr;
	XilPdi_BootHdr *BootHdr = &SecurePtr->PdiPtr->MetaHdr.BootHdr;
	u8 IsEfuseAuth = TRUE;
	u32 AuthType;

	AuthType = XLoader_GetAuthType(AcPtr);
	/*
	 * Skip running the KAT for ECDSA or RSA if it is already run by ROM
	 * KAT will be run only when the CYRPTO_KAT_EN bits in eFUSE are set
	 */
	if(((SecurePtr->PdiPtr->PlmKatStatus & XLOADER_RSA_KAT_MASK) == 0U)
		&& (AuthType == XLOADER_AC_AH_PUB_ALG_RSA)) {
		Status = XSecure_RsaPublicEncryptKat();
		if(Status != XLOADER_SUCCESS) {
			XPlmi_Printf(DEBUG_GENERAL, "RSA KAT Failed\n\r");
			Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_KAT_FAILED, Status);
			goto END;
		}
		SecurePtr->PdiPtr->PlmKatStatus |= XLOADER_RSA_KAT_MASK;
	}
	if(((SecurePtr->PdiPtr->PlmKatStatus & XLOADER_ECDSA_KAT_MASK) == 0U)
		&& (AuthType == XLOADER_AC_AH_PUB_ALG_ECDSA)) {
		Status = XSecure_EcdsaKat();
		if(Status != XLOADER_SUCCESS) {
			XPlmi_Printf(DEBUG_GENERAL, "ECDSA KAT Failed\n\r");
			Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_KAT_FAILED, Status);
			goto END;
		}
		SecurePtr->PdiPtr->PlmKatStatus |= XLOADER_ECDSA_KAT_MASK;
	}

	/* If bits in PPK0/1/2 is programmed bh_auth is not allowed */
	Status = XLoader_CheckNonZeroPpk();
	/*
	 * Only boot header authentication is allowed when
	 * none of PPK hash bits are programmed
	 */
	if (Status != XLOADER_SUCCESS) {
		IsEfuseAuth = FALSE;
		/* If BHDR authentication is not enabled return error */
		if (XilPdi_IsBhdrAuthEnable(BootHdr) == 0x00U) {
			XPlmi_Printf(DEBUG_INFO,
			"None of the PPKs are programmed and also boot header"
				" authentication is not enabled\n\r");
			Status = XLOADER_UPDATE_MIN_ERR(
				XLOADER_SEC_AUTH_EN_PPK_HASH_NONZERO, 0x0U);
			goto END;
		}
	}
	/* Only efuse RSA authentication is allowed */
	else {
		IsEfuseAuth = TRUE;

		/* If BHDR authentication is enabled return error */
		if (XilPdi_IsBhdrAuthEnable(BootHdr) != 0x00U) {
			XPlmi_Printf(DEBUG_INFO, "Boot header authentication is not allowed"
					"when HWROT is enabled\n\r");
			Status = XLOADER_UPDATE_MIN_ERR(
				XLOADER_SEC_AUTH_EN_PPK_HASH_NONZERO, 0x0U);
			goto END;
		}
		/* Validate PPK hash */
		Status = XLoader_PpkVerify(SecurePtr);
		if (Status != XLOADER_SUCCESS) {
			goto END;
		}
	}

	/* Perform SPK Validation */
	Status = XLoader_SpkAuthentication(SecurePtr);
	if (Status != XLOADER_SUCCESS) {
		goto END;
	}

	/* Check for SPK ID revocation */
	if (IsEfuseAuth == TRUE) {
		Status = XLoader_VerifyRevokeId(AcPtr->SpkId);
		if (Status != XLOADER_SUCCESS) {
			goto END;
		}
	}

	Status = XLoader_VerifySignature(SecurePtr, Hash, &AcPtr->Spk,
					Signature);

END:
	return Status;

}

/*****************************************************************************/
/**
* @brief	This function encrypts the RSA/ECDSA signature provided and
* compares it with expected hash.
*
* @param	SecurePtr is pointer to the XLoader_SecureParms instance.
* @param	Hash is pointer to the expected hash
* @param	Key is pointer to the RSA/ECDSA public key to be used
* @param	Signature is pointer to the Signature
*
* @return	XLOADER_SUCCESS on success and error code on failure
*
******************************************************************************/
static u32 XLoader_VerifySignature(XLoader_SecureParms *SecurePtr,
		u8 *Hash, XLoader_RsaKey *Key, u8 *Signature)
{
	u32 Status = XLOADER_FAILURE;
	XLoader_AuthCertificate *AcPtr =
		(XLoader_AuthCertificate *)SecurePtr->AcPtr;
	u32 AuthType =  XLoader_GetAuthType(AcPtr);

	/* RSA authentication */
	if (AuthType ==	XLOADER_AC_AH_PUB_ALG_RSA) {
		Status = XLoader_RsaSignVerify(SecurePtr, Hash,
			Key, Signature);
		if (Status != XLOADER_SUCCESS) {
			goto END;
		}
	}
	else if (AuthType == XLOADER_AC_AH_PUB_ALG_ECDSA) {
		/* ECDSA authentication */
		Status = XLoader_EcdsaSignVerify((u32 *)Hash,
			(u32 *)Key->PubModulus, (u32 *)Signature);
		if (Status != XLOADER_SUCCESS) {
			goto END;
		}
	}
	else {
		/* Not supported */
		XPlmi_Printf(DEBUG_INFO, "Authentication type is invalid\n\r");
		Status = XLOADER_UPDATE_MIN_ERR(XLOADER_SEC_INVALID_AUTH, 0U);
	}

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function verifies SPK with PPK.
*
* @param	SecurePtr is pointer to the XLoader_SecureParms instance.
*
* @return	XLOADER_SUCCESS on success and error code on failure
*
******************************************************************************/
static u32 XLoader_SpkAuthentication(XLoader_SecureParms *SecurePtr)
{
	u32 Status = XLOADER_FAILURE;
	Sha3Hash SpkHash;
	XLoader_AuthCertificate *AcPtr = SecurePtr->AcPtr;
	XSecure_Sha3 Sha3Instance;

	XPlmi_Printf(DEBUG_DETAILED, "Performing SPK verification \n\r");
	/* Initialize sha3 */
	Status = XSecure_Sha3Initialize(&Sha3Instance, SecurePtr->PmcDmaInstPtr);
	if (Status != XLOADER_SUCCESS) {
		Status = XLOADER_UPDATE_MIN_ERR(
			XLOADER_SEC_SPK_HASH_CALCULATION_FAIL, Status);
		goto END;
	}

	XSecure_Sha3Start(&Sha3Instance);

	/* Hash the AH  and SPK*/
	/* Update AH */
	Status = XSecure_Sha3Update(&Sha3Instance,
			(u8 *)&AcPtr->AuthHdr, XLOADER_AUTH_HEADER_SIZE);
	if (Status != XLOADER_SUCCESS) {
		Status = XLOADER_UPDATE_MIN_ERR(
			XLOADER_SEC_SPK_HASH_CALCULATION_FAIL, Status);
		goto END;
	}
	Status = XSecure_Sha3LastUpdate(&Sha3Instance);
	if (Status != XLOADER_SUCCESS) {
		Status = XLOADER_UPDATE_MIN_ERR(
			XLOADER_SEC_SPK_HASH_CALCULATION_FAIL, Status);
		goto END;
	}

	/* Update SPK */
	Status = XSecure_Sha3Update(&Sha3Instance, (u8 *)&AcPtr->Spk,
						XLOADER_SPK_SIZE);
	if (Status != XLOADER_SUCCESS) {
		Status = XLOADER_UPDATE_MIN_ERR(
			XLOADER_SEC_SPK_HASH_CALCULATION_FAIL, Status);
		goto END;
	}

	Status = XSecure_Sha3Finish(&Sha3Instance, &SpkHash);
	if (Status != XLOADER_SUCCESS) {
		Status = XLOADER_UPDATE_MIN_ERR(
			XLOADER_SEC_SPK_HASH_CALCULATION_FAIL, Status);
		goto END;
	}

	Status = XLoader_VerifySignature(SecurePtr, SpkHash.Hash, &AcPtr->Ppk,
					(u8 *)&AcPtr->SPKSignature);

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function validates SPK by verifying if the given SPK ID
* has been revoked or not.
*
* @param	SpkId is ID of the SPK
*
* @return	XLOADER_SUCCESS on success and error code on failure
*
******************************************************************************/
static u32 XLoader_VerifyRevokeId(u32 RevokeId)
{
	u32 Status = XLOADER_FAILURE;
	u32 RevokeAll;
	u32 Quo;
	u32 Mod;
	u32 Value;

	/* TBD this API should ultilize XilNvm library */
	XPlmi_Printf(DEBUG_DETAILED, "Validating SPKID\n\r");
	RevokeAll = XPlmi_In32(XLOADER_EFUSE_REVOCATION_ID_0_OFFSET) &
			XPlmi_In32(XLOADER_EFUSE_REVOCATION_ID_1_OFFSET) &
			XPlmi_In32(XLOADER_EFUSE_REVOCATION_ID_2_OFFSET) &
			XPlmi_In32(XLOADER_EFUSE_REVOCATION_ID_3_OFFSET) &
			XPlmi_In32(XLOADER_EFUSE_REVOCATION_ID_4_OFFSET) &
			XPlmi_In32(XLOADER_EFUSE_REVOCATION_ID_5_OFFSET) &
			XPlmi_In32(XLOADER_EFUSE_REVOCATION_ID_6_OFFSET) &
			XPlmi_In32(XLOADER_EFUSE_REVOCATION_ID_7_OFFSET);
	/* If all bits of REVOCATION_ID are programmed */
	if(RevokeAll == MASK_ALL) {
		XPlmi_Printf(DEBUG_INFO, "All IDs are invalid\n\r");
		Status = XLOADER_UPDATE_MIN_ERR(
				XLOADER_SEC_ALL_IDS_REVOKED_ERR, 0x0U);
		goto END;
	}
	/* Verify range of provided revocation ID */
	if(RevokeId > XLOADER_REVOCATION_IDMAX) {
		XPlmi_Printf(DEBUG_INFO, "Revocation ID provided is"
				" out of range, valid range is 0 - 255\n\r");
		Status = XLOADER_UPDATE_MIN_ERR(
				XLOADER_SEC_REVOCATION_ID_OUTOFRANGE_ERR, 0x0U);
		goto END;
	}

	Quo = RevokeId / XLOADER_WORD_IN_BITS;
	Mod = RevokeId % XLOADER_WORD_IN_BITS;
	Value = (XPlmi_In32(XLOADER_EFUSE_REVOCATION_ID_0_OFFSET +
			(Quo * XIH_PRTN_WORD_LEN)) & (1U << Mod)) >> Mod;
	if(Value != 0x00U)	{
		Status = XLOADER_UPDATE_MIN_ERR(
				XLOADER_SEC_ID_REVOKED, 0x0U);
		goto END;
	}

	Status = XLOADER_SUCCESS;
	XPlmi_Printf(DEBUG_DETAILED, "Revocation ID is valid\r\n");

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function compares calculated PPK hash with the
* efuse PPK hash.
*
* @param	EfusePpkOffset is PPK hash address of efuse.
* @param	PpkHash is pointer to the PPK hash to be verified.
*
* @return	XLOADER_SUCCESS on success and error code on failure
*
******************************************************************************/
static u32 XLoader_PpkCompare(u32 EfusePpkOffset, u8 *PpkHash)
{
	u32 Status = XLOADER_FAILURE;
	int HashStatus = XST_FAILURE;

	HashStatus = XPlmi_MemCmp(PpkHash,
			(void *)EfusePpkOffset,
			XLOADER_EFUSE_PPK_HASH_LEN);
	if (HashStatus != XST_SUCCESS) {
		Status = XLOADER_UPDATE_MIN_ERR(
				XLOADER_SEC_PPK_HASH_COMPARE_FAIL, 0x0U);
	}
	else {
		Status = XLOADER_SUCCESS;
	}

	return Status;

}

/*****************************************************************************/
/**
* @brief	This function validates PPK, by checking selected PPK is valid
* or not by reading PPK invalid bits
* Efuse only stores 256 bits of hash.
*
* @param	PpkSelect	PPK selection of eFUSE.
* @param	PpkHash		Pointer to the PPK hash to be verified.
*
* @return	XLOADER_SUCCESS on success and error code on failure
*
******************************************************************************/
static u32 XLoader_IsPpkValid(u8 PpkSelect, u8 *PpkHash)
{
	u32 Status = XLOADER_FAILURE;
	int HashStatus;
	u8 HashZeros[XLOADER_EFUSE_PPK_HASH_LEN] = {0U};
	u32 ReadReg;
	u32 PpkOffset;
	u32 InvalidMask;

	switch (PpkSelect) {
		case XLOADER_PPK_SEL_0:
			InvalidMask = XLOADER_EFUSE_MISC_CTRL_PPK0_INVLD;
			PpkOffset = XLOADER_EFUSE_PPK0_START_OFFSET;
			break;
		case XLOADER_PPK_SEL_1:
			InvalidMask = XLOADER_EFUSE_MISC_CTRL_PPK1_INVLD;
			PpkOffset = XLOADER_EFUSE_PPK1_START_OFFSET;
			break;
		case XLOADER_PPK_SEL_2:
			InvalidMask = XLOADER_EFUSE_MISC_CTRL_PPK2_INVLD;
			PpkOffset = XLOADER_EFUSE_PPK2_START_OFFSET;
			break;
		default:
			goto END;
	}

	/* Read PPK invalid set bits */
	ReadReg = XPlmi_In32(XLOADER_EFUSE_MISC_CTRL_OFFSET);
	if ((ReadReg & InvalidMask) != 0x0U) {
		Status = XLOADER_SEC_PPK_INVALID_BIT_ERR;
		goto END;
	}
	Status = XLoader_PpkCompare(PpkOffset, PpkHash);
	if (Status == XLOADER_SUCCESS) {
		/* Check if valid PPK hash is all zeros */
		HashStatus = XPlmi_MemCmp(HashZeros, (void *)PpkOffset,
					XLOADER_EFUSE_PPK_HASH_LEN);
		if (HashStatus == XST_SUCCESS) {
			Status = XLOADER_UPDATE_MIN_ERR(
				XLOADER_SEC_PPK_HASH_ALLZERO_INVLD, 0x0U);
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function returns XLOADER_SUCCESS if any of the PPK hash
* bits are programmed.
*
* @param	None
*
* @return	XLOADER_SUCCESS on success and error code on failure
*
******************************************************************************/
static u32 XLoader_CheckNonZeroPpk()
{
	u32 Status = XLOADER_FAILURE;
	u32 Index;

	for (Index = XLOADER_EFUSE_PPK0_START_OFFSET;
			Index < XLOADER_EFUSE_PPK2_END_OFFSET;
			Index = Index + XIH_PRTN_WORD_LEN) {
		/* Any bit of PPK hash are non-zero break and return success */
		if (XPlmi_In32(Index) != 0x0U) {
			Status = XLOADER_SUCCESS;
			break;
		}
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief	This function verifies PPK.
*
* @param	SecurePtr is pointer to the XLoader_SecureParms instance.
*
* @return	XLOADER_SUCCESS on success and error code on failure
*
******************************************************************************/
static u32 XLoader_PpkVerify(XLoader_SecureParms *SecurePtr)
{
	u32 Status = XLOADER_FAILURE;
	Sha3Hash HashVal;
	XLoader_AuthCertificate *AcPtr = SecurePtr->AcPtr;
	XSecure_Sha3 Sha3Instance;
	u32 ReadReg;

	/* Check if all PPKs are revoked */
	ReadReg = XPlmi_In32(XLOADER_EFUSE_MISC_CTRL_OFFSET);
	if ((ReadReg & XLOADER_EFUSE_MISC_CTRL_ALL_PPK_INVLD) ==
				(XLOADER_EFUSE_MISC_CTRL_ALL_PPK_INVLD)) {
		XPlmi_Printf(DEBUG_INFO, "All PPKs are invalid\n\r");
		Status = XLOADER_UPDATE_MIN_ERR(
			XLOADER_SEC_ALL_PPK_REVOKED_ERR, 0x0U);
		goto END;
	}
		/* Calculate PPK hash */
	Status = XSecure_Sha3Initialize(&Sha3Instance, SecurePtr->PmcDmaInstPtr);
	if (Status != XLOADER_SUCCESS) {
		Status = XLOADER_UPDATE_MIN_ERR(
				XLOADER_SEC_PPK_HASH_CALCULATION_FAIL, Status);
		goto END;
	}

	XSecure_Sha3Start(&Sha3Instance);
	Status = XSecure_Sha3LastUpdate(&Sha3Instance);
	if (Status != XLOADER_SUCCESS) {
		Status = XLOADER_UPDATE_MIN_ERR(
				XLOADER_SEC_PPK_HASH_CALCULATION_FAIL, Status);
		goto END;
	}
	/* Update PPK  */
	Status = XSecure_Sha3Update(&Sha3Instance, (u8 *)&AcPtr->Ppk,
			XLOADER_PPK_SIZE);
	if (Status != XLOADER_SUCCESS) {
		Status = XLOADER_UPDATE_MIN_ERR(
				XLOADER_SEC_PPK_HASH_CALCULATION_FAIL, Status);
		goto END;
	}

	Status = XSecure_Sha3Finish(&Sha3Instance, &HashVal);
	if (Status != XLOADER_SUCCESS) {
		Status = XLOADER_UPDATE_MIN_ERR(
				XLOADER_SEC_PPK_HASH_CALCULATION_FAIL, Status);
		goto END;
	}

	Status = XLoader_IsPpkValid(XLOADER_PPK_SEL_0, HashVal.Hash);
	if (Status != XLOADER_SUCCESS) {
		Status = XLoader_IsPpkValid(XLOADER_PPK_SEL_1, HashVal.Hash);
		if(Status != XLOADER_SUCCESS) {
			Status = XLoader_IsPpkValid(XLOADER_PPK_SEL_2, HashVal.Hash);
			if (Status == XLOADER_SUCCESS) {
				/* Selection matched with PPK2 HASH */
				XPlmi_Printf(DEBUG_INFO, "PPK2 is valid\n\r");
			}
			else {
				/* No PPK is valid */
				XPlmi_Printf(DEBUG_INFO, "No PPK is valid\n\r");
				Status = XLOADER_UPDATE_MIN_ERR(
					XLOADER_SEC_ALL_PPK_INVALID_ERR, 0x0U);
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
 * @breif	This function converts a non-negative integer to an octet string of a
 * specified length.
 *
 * @param	Integer is the variable in which input should be provided.
 * @param	Size holds the required size.
 * @param	Convert is a pointer in which output will be updated.
 *
 * @return	None
 *
 ******************************************************************************/
static inline void XLoader_I2Osp(u32 Integer, u32 Size, u8 *Convert)
{
	if (Integer < XLOADER_I2OSP_INT_LIMIT) {
		Convert[Size - 1U] = (u8)Integer;
	}
}

/*****************************************************************************/
/**
 * @brief	Mask generation function with SHA3.
 *
 * @param	Sha3InstancePtr is pointer to the XSecure_Sha3 instance.
 * @param	Out is pointer in which output of this
		function will be stored.
 * @param	Outlen specifies the required length.
 * @param	Input is pointer which holds the input data for
 *		which mask should be calculated which should be 48 bytes length
 *
 * @return	XLOADER_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
static u32 XLoader_MaskGenFunc(XSecure_Sha3 *Sha3InstancePtr,
	u8 * Out, u32 OutLen, u8 *Input)
{
	u32 Status = XLOADER_FAILURE;
	u32 Counter = 0U;
	u32 HashLen = XLOADER_SHA3_LEN;
	Sha3Hash HashStore = {0U};
	u8 Convert[XIH_PRTN_WORD_LEN] = {0U};
	u32 Size = XLOADER_SHA3_LEN;

	while (Counter <= (OutLen / HashLen)) {
		XLoader_I2Osp(Counter, XIH_PRTN_WORD_LEN, Convert);

		XSecure_Sha3Start(Sha3InstancePtr);
		Status = XSecure_Sha3Update(Sha3InstancePtr, Input, HashLen);
		if (Status != XLOADER_SUCCESS) {
			goto END;
		}
		Status = XSecure_Sha3Update(Sha3InstancePtr, Convert,
					XIH_PRTN_WORD_LEN);
		if (Status != XLOADER_SUCCESS) {
			goto END;
		}
		Status = XSecure_Sha3Finish(Sha3InstancePtr, &HashStore);
		if (Status != XLOADER_SUCCESS) {
			goto END;
		}
		if (Counter == (OutLen / HashLen)) {
			  /*
			   * Only 463 bytes are required but the chunklen is 48 bytes.
			   * The extra bytes are discarded by the modulus operation below.
			   */
			 Size = (OutLen % HashLen);
		}
		(void)XPlmi_MemCpy(Out, HashStore.Hash, Size);
		Out = Out + XLOADER_SHA3_LEN;
		Counter = Counter + 1U;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function encrypts the RSA signature provided and performs
 * required PSS operations to extract salt and calculates M prime hash and
 * compares with hash obtained from EM.
 *
 * @param	SecurePtr is pointer to the XLoader_SecureParms instance.
 * @param	RsaInstancePtr is pointer to the XSecure_Rsa instance.
 * @param	Signature is pointer to RSA signature for data to be
 *		authenticated.
 * @param	Hash of the data to be authenticated.
 *
 * @return	XLOADER_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
static u32 XLoader_RsaSignVerify(XLoader_SecureParms *SecurePtr,
		u8 *MsgHash, XLoader_RsaKey *Key, u8 *Signature)
{

	u32 Status = XLOADER_FAILURE;
	Sha3Hash MPrimeHash = {0U};
	u8 XSecure_RsaSha3Array[XSECURE_RSA_4096_KEY_SIZE];
	XLoader_Vars Xsecure_Varsocm __attribute__ ((aligned(32U)));
	/* Buffer variable used to store HashMgf and DB */
	u8 Buffer[XLOADER_RSA_PSS_BUFFER_LEN]__attribute__ ((aligned(32U))) = {0U};
	u32 Index;
	XSecure_Sha3 Sha3Instance;
	XSecure_Rsa RsaInstance;
	u8 *DataHash = (u8 *)MsgHash;

	/* Initialize RSA instance */
	Status = (u32)XSecure_RsaInitialize(&RsaInstance, (u8 *)Key->PubModulus,
			(u8 *)Key->PubModulusExt, (u8 *)&Key->PubExponent);
	if (Status != XLOADER_SUCCESS) {
		Status = XLOADER_UPDATE_MIN_ERR(XLOADER_SEC_RSA_AUTH_FAIL,
					 Status);
		goto END;
	}

	(void)memset(XSecure_RsaSha3Array, 0U, XLOADER_PARTITION_SIG_SIZE);
	(void)memset(&Xsecure_Varsocm, 0U, sizeof(Xsecure_Varsocm));

	/* RSA signature encryption with public key components */
	Status = (u32)XSecure_RsaPublicEncrypt(&RsaInstance, Signature,
					XSECURE_RSA_4096_KEY_SIZE,
					XSecure_RsaSha3Array);
	if (Status != XLOADER_SUCCESS) {
		Status = XLOADER_UPDATE_MIN_ERR(XLOADER_SEC_RSA_PSS_SIGN_VERIFY_FAIL,
							 Status);
		goto END;
	}

	/* Checks for signature encrypted message */
	if (XSecure_RsaSha3Array[XSECURE_RSA_4096_KEY_SIZE - 1U] !=
			XLOADER_RSA_SIG_EXP_BYTE) {
		Status = XLOADER_UPDATE_MIN_ERR(
				XLOADER_SEC_RSA_PSS_ENC_BC_VALUE_NOT_MATCHED, 0U);
		goto END;
	}

	/* As PMCDMA can't accept unaligned addresses */
	(void)memcpy(Xsecure_Varsocm.EmHash, XSecure_RsaSha3Array +
				XLOADER_RSA_PSS_MASKED_DB_LEN, XLOADER_SHA3_LEN);
	Status = XSecure_Sha3Initialize(&Sha3Instance, SecurePtr->PmcDmaInstPtr);
	if (Status != XLOADER_SUCCESS) {
		Status = XLOADER_UPDATE_MIN_ERR(XLOADER_SEC_RSA_PSS_SIGN_VERIFY_FAIL,
									 Status);
		goto END;
	}

			/* Salt extraction */
	/* Generate DB from masked DB and Hash */
	Status = XLoader_MaskGenFunc(&Sha3Instance, Buffer,
								XLOADER_RSA_PSS_MASKED_DB_LEN,
								Xsecure_Varsocm.EmHash);
	if (Status != XLOADER_SUCCESS) {
		Status = XLOADER_UPDATE_MIN_ERR(XLOADER_SEC_RSA_PSS_SIGN_VERIFY_FAIL,
									 Status);
		goto END;
	}

	/* XOR MGF output with masked DB from EM to get DB */
	for (Index = 0U; Index < XLOADER_RSA_PSS_MASKED_DB_LEN; Index++) {
		Buffer[Index] = Buffer[Index] ^ XSecure_RsaSha3Array[Index];
	}

	/* As PMCDMA can't accept unaligned addresses */
	(void)memcpy(Xsecure_Varsocm.Salt, Buffer + XLOADER_RSA_PSS_DB_LEN,
				XLOADER_RSA_PSS_SALT_LEN);

	Status = XSecure_Sha3Initialize(&Sha3Instance, SecurePtr->PmcDmaInstPtr);
	if (Status != XLOADER_SUCCESS) {
		Status = XLOADER_UPDATE_MIN_ERR(XLOADER_SEC_RSA_PSS_SIGN_VERIFY_FAIL,
									 Status);
		goto END;
	}
	/* Hash on M prime */
	XSecure_Sha3Start(&Sha3Instance);

	 /* Padding 1 */
	Status = XSecure_Sha3Update(&Sha3Instance,
							Xsecure_Varsocm.Padding1,
							XLOADER_RSA_PSS_PADDING1);
	if (Status != XLOADER_SUCCESS) {
		Status = XLOADER_UPDATE_MIN_ERR(XLOADER_SEC_RSA_PSS_SIGN_VERIFY_FAIL,
											 Status);
		goto END;
	}

	 /* Message hash */
	Status = XSecure_Sha3Update(&Sha3Instance, DataHash, XLOADER_SHA3_LEN);
	if (Status != XLOADER_SUCCESS) {
		Status = XLOADER_UPDATE_MIN_ERR(XLOADER_SEC_RSA_PSS_SIGN_VERIFY_FAIL,
											 Status);
		goto END;
	}

	 /* Salt */
	Status = XSecure_Sha3Update(&Sha3Instance, Xsecure_Varsocm.Salt,
							XLOADER_RSA_PSS_SALT_LEN);
	if (Status != XLOADER_SUCCESS) {
		Status = XLOADER_UPDATE_MIN_ERR(XLOADER_SEC_RSA_PSS_SIGN_VERIFY_FAIL,
											 Status);
		goto END;
	}

	Status = XSecure_Sha3Finish(&Sha3Instance, &MPrimeHash);
	if (Status != XLOADER_SUCCESS) {
		Status = XLOADER_UPDATE_MIN_ERR(XLOADER_SEC_RSA_PSS_SIGN_VERIFY_FAIL,
											 Status);
		goto END;
	}

	/* Compare MPrime Hash with Hash from EM */
	for (Index = 0U; Index < XLOADER_SHA3_LEN; Index++) {
		if (MPrimeHash.Hash[Index] !=
			XSecure_RsaSha3Array[XLOADER_RSA_PSS_MASKED_DB_LEN + Index]) {
			XPlmi_Printf(DEBUG_INFO, "Failed at RSA PSS "
				"signature verification\n\r");
			XPlmi_PrintArray(DEBUG_INFO,
				(UINTPTR)MPrimeHash.Hash,
				XLOADER_SHA3_LEN / XIH_PRTN_WORD_LEN,
					"M prime Hash");
			XPlmi_PrintArray(DEBUG_INFO,
				(UINTPTR)(XSecure_RsaSha3Array + XLOADER_RSA_PSS_MASKED_DB_LEN),
				XLOADER_SHA3_LEN / XIH_PRTN_WORD_LEN,
				"RSA sha3 array Hash");
			Status = XLOADER_UPDATE_MIN_ERR(XLOADER_SEC_RSA_PSS_HASH_COMPARE_FAILURE,
					0U);
			goto END;
		}
	}
	XPlmi_Printf(DEBUG_DETAILED,
			"RSA PSS verification is success\n\r");

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function encrypts the ECDSA signature
 * provided with the key components.
 *
 * @param	Hash is pointer to the hash of the data to be authenticated.
 * @param	Key is pointer to the ECDSA key.
 * @param	Signature is pointer to the ECDSA signature.
 *
 * @return	XLOADER_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
static u32 XLoader_EcdsaSignVerify(u32 *DataHash, u32 *Key, u32 *Signature)
{
	u32 Status = XLOADER_FAILURE;
	u32 *XKey = (u32 *)Key;
	u32 *YKey = (u32 *)(Key + XLOADER_ECDSA_KEYSIZE);
	u32 *W = (u32 *)Signature;
	u32 *S= (u32 *)(Signature + XLOADER_ECDSA_KEYSIZE);
	u32 Qx[XLOADER_ECDSA_KEYSIZE] = {0U};
	u32 Qy[XLOADER_ECDSA_KEYSIZE] = {0U};
	u32 SIGR[XLOADER_ECDSA_KEYSIZE] = {0U};
	u32 SIGS[XLOADER_ECDSA_KEYSIZE] = {0U};
	u32 HashPtr[XLOADER_ECDSA_KEYSIZE] = {0U};
	u32 Index;

	/* Take the core out of reset */
	XSecure_ReleaseReset(XSECURE_ECDSA_RSA_BASEADDR,
			XSECURE_ECDSA_RSA_RESET_OFFSET);

	for (Index = 0U; Index < XLOADER_ECDSA_KEYSIZE; Index++) {
		Qx[Index] = Xil_Htonl(XKey[XLOADER_ECDSA_KEYSIZE - Index - 1U]);
		Qy[Index] = Xil_Htonl(YKey[XLOADER_ECDSA_KEYSIZE - Index - 1U]);
		SIGR[Index] =  Xil_Htonl(W[XLOADER_ECDSA_KEYSIZE - Index - 1U]);
		SIGS[Index] =  Xil_Htonl(S[XLOADER_ECDSA_KEYSIZE - Index - 1U]);
		HashPtr[Index] = Xil_Htonl(DataHash[XLOADER_ECDSA_KEYSIZE -
					Index - 1U]);
	}
	/* Validate point on the curve */
	Status = (u32)P384_validatekey((u8 *)Qx, (u8 *)Qy);
	if (Status != XLOADER_SUCCESS) {
		XPlmi_Printf(DEBUG_INFO, "\nFailed at "
			"ECDSA Key validation\n\r");
		Status = XLOADER_UPDATE_MIN_ERR(
			XLOADER_SEC_ECDSA_INVLD_KEY_COORDINATES, 0x0U);
	}
	else {
		Status = (u32)P384_ecdsaverify((u8 *)HashPtr, (u8 *)Qx,
					(u8 *)Qy, (u8 *)SIGR, (u8 *)SIGS);
		if (Status != XLOADER_SUCCESS) {
			Status = XLOADER_UPDATE_MIN_ERR(
					XLOADER_SEC_ECDSA_AUTH_FAIL, 0x0U);
			XPlmi_Printf(DEBUG_INFO, "Failed at ECDSA signature "
					"verification\n\r");
		}
		else {
			XPlmi_Printf(DEBUG_DETAILED, "Authentication of ECDSA "
						"is successful\n\r");
		}
	}

	XSecure_SetReset(XSECURE_ECDSA_RSA_BASEADDR,
		XSECURE_ECDSA_RSA_RESET_OFFSET);

	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function decrypts the secure header/footer.
 *
 * @param	SecurePtr	Pointer to the XLoader_SecureParms
 * @param	AesInstancePtr	Pointer to the AES instance
 * @param	SrcAddr		Pointer to the buffer where header/footer present
 *
 * @return	XLOADER_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
static u32 XLoader_DecryptSH(XLoader_SecureParms *SecurePtr,
			u64 SrcAddr)
{
	u32 Status = XLOADER_FAILURE;

	/* Configure AES engine to push Key and IV */
	XPlmi_Printf(DEBUG_DETAILED, "Decrypting Secure header\n\r");
	Status = XSecure_AesCfgKupIv(&SecurePtr->AesInstance, TRUE);
	if (Status != XLOADER_SUCCESS) {
		Status  = XLOADER_UPDATE_MIN_ERR(
				XLOADER_SEC_AES_OPERATION_FAILED, Status);
		goto END;
	}

	/* Push secure header */
	Status = XSecure_AesDecryptUpdate(&SecurePtr->AesInstance,
			SrcAddr, XSECURE_AES_NO_CFG_DST_DMA,
			XLOADER_SECURE_HDR_SIZE, TRUE);
	if (Status != XLOADER_SUCCESS) {
		Status  = XLOADER_UPDATE_MIN_ERR(
				XLOADER_SEC_AES_OPERATION_FAILED, Status);
		goto END;
	}

	/* Verify GCM Tag */
	Status = XSecure_AesDecryptFinal(&SecurePtr->AesInstance,
			SrcAddr + XLOADER_SECURE_HDR_SIZE);
	if (Status != XLOADER_SUCCESS) {
		XPlmi_Printf(DEBUG_INFO, "Decrypting Secure header failed in "
				"AesDecrypt Final\n\r");
		Status  = XLOADER_UPDATE_MIN_ERR(
				XLOADER_SEC_AES_OPERATION_FAILED, Status);
		goto END;
	}

	SecurePtr->RemainingEncLen = SecurePtr->RemainingEncLen -
				XLOADER_SECURE_HDR_TOTAL_SIZE;
	Status = XSecure_AesCfgKupIv(&SecurePtr->AesInstance, FALSE);
	if (Status != XLOADER_SUCCESS) {
		Status  = XLOADER_UPDATE_MIN_ERR(
				XLOADER_SEC_AES_OPERATION_FAILED, Status);
		goto END;
	}
	XPlmi_Printf(DEBUG_DETAILED,
	"Decryption NextBlkLen = %0x\n\r", SecurePtr->AesInstance.NextBlkLen);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function decrypts the data.
 *
 * @param	SecurePtr is pointer to the XLoader_SecureParms
 * @param	AesInstancePtr is pointer to the AES instance
 * @param	SrcAddr is pointer to the buffer where header/footer present
 * @param	DestAddr is pointer to the buffer where header/footer should
 * 		be copied
 * @param	Size is the number of byte sto be copied
 *
 * @return	XLOADER_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
static u32 XLoader_DataDecrypt(XLoader_SecureParms *SecurePtr,
		u64 SrcAddr, u64 DestAddr, u32 Size)
{
	u32 Status = XLOADER_FAILURE;
	u64 InAddr = SrcAddr;
	u64 OutAddr = DestAddr;
	u32 Iv[XLOADER_SECURE_IV_LEN];
	u32 ChunkSize = Size;
	u8 Index;
	u32 RegVal;

	do {
		for (Index = 0U; Index < XLOADER_SECURE_IV_LEN; Index++) {
			RegVal = XPlmi_In32(SecurePtr->AesInstance.BaseAddress +
					(XSECURE_AES_IV_0_OFFSET +
						(Index * XIH_PRTN_WORD_LEN)));
			Iv[Index] = Xil_Htonl(RegVal);
		}

		Status = XSecure_AesDecryptInit(&SecurePtr->AesInstance,
			XSECURE_AES_KUP_KEY, XSECURE_AES_KEY_SIZE_256,
			(UINTPTR)Iv);
		if (Status != XLOADER_SUCCESS) {
			Status  = XLOADER_UPDATE_MIN_ERR(
					XLOADER_SEC_AES_OPERATION_FAILED,
					Status);
			break;
		}

		/* Decrypt the data */
		Status = XSecure_AesDecryptUpdate(&SecurePtr->AesInstance,
				InAddr, OutAddr, SecurePtr->AesInstance.NextBlkLen, 0U);
		if (Status != XLOADER_SUCCESS) {
			Status  = XLOADER_UPDATE_MIN_ERR(
				XLOADER_SEC_AES_OPERATION_FAILED, Status);
			break;
		}

		InAddr = InAddr + SecurePtr->AesInstance.NextBlkLen;
		OutAddr = OutAddr + SecurePtr->AesInstance.NextBlkLen;
		SecurePtr->SecureDataLen = SecurePtr->SecureDataLen +
						 SecurePtr->AesInstance.NextBlkLen;
		ChunkSize = ChunkSize - SecurePtr->AesInstance.NextBlkLen;
		SecurePtr->RemainingEncLen = SecurePtr->RemainingEncLen -
						 SecurePtr->AesInstance.NextBlkLen;

		/* Decrypt Secure header */
		Status = XLoader_DecryptSH(SecurePtr, InAddr);
		if (Status != XLOADER_SUCCESS) {
			break;
		}
		ChunkSize = ChunkSize - XLOADER_SECURE_HDR_TOTAL_SIZE;
		InAddr = InAddr + XLOADER_SECURE_HDR_TOTAL_SIZE;

		if (ChunkSize == 0x00U) {
			break;
		}
		if (SecurePtr->AesInstance.NextBlkLen == 0x00U) {
			if (SecurePtr->RemainingEncLen == 0U) {
				Status = XLOADER_SUCCESS;
			}
			else {
			/* Still remaining data is there for decryption */
				Status = XLOADER_UPDATE_MIN_ERR(
				XLOADER_SEC_DATA_LEFT_FOR_DECRYPT_ERR, 0x0U);
			}
			break;
		}
		else {
			if (SecurePtr->RemainingEncLen < SecurePtr->AesInstance.NextBlkLen) {
				Status = XLOADER_UPDATE_MIN_ERR(
				XLOADER_SEC_DECRYPT_REM_DATA_SIZE_MISMATCH,
							 0x0U);
				break;
			}
			if (ChunkSize < SecurePtr->AesInstance.NextBlkLen) {
				Status = XLOADER_UPDATE_MIN_ERR(
				XLOADER_SEC_DECRYPT_REM_DATA_SIZE_MISMATCH,
							0x0U);
				break;
			}
		}
	} while (1U);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function decrypts the data.
 *
 * @param	SecurePtr is pointer to the XLoader_SecureParms
 * @param	AesInstacePtr is pointer to the Aes instance
 * @param	SrcAddr is address to the buffer where header/footer present
 * @param	DestAddr is the address to which header / footer is copied
 * @param	Size is the number of bytes to be copied
 *
 * @return	XLOADER_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
static u32 XLoader_AesDecryption(XLoader_SecureParms *SecurePtr,
		 u64 SrcAddr, u64 DstAddr, u32 Size)
{
	u32 Status = XLOADER_FAILURE;
	XSecure_AesKeySrc KeySrc;
	u32 ChunkSize = Size;
	u32 DpaCmCfg;
	XLoader_AesKekKey KeyDetails;

	/* To update decrypted data */
	SecurePtr->SecureDataLen = 0U;

	if (SecurePtr->BlockNum == 0x0U) {
		/* Initialize AES driver */
		Status = XSecure_AesInitialize(&SecurePtr->AesInstance,
							SecurePtr->PmcDmaInstPtr);
		if (Status != XLOADER_SUCCESS) {
			Status = XLOADER_UPDATE_MIN_ERR(XLOADER_SEC_AES_OPERATION_FAILED,
						Status);
			goto END;
		}

		KeyDetails.PufHdLocation = XilPdi_GetPufHdPh(SecurePtr->PrtnHdr)
					>> XIH_PH_ATTRB_PUFHD_SHIFT;
		KeyDetails.PdiKeySrc = SecurePtr->PrtnHdr->EncStatus;
		KeyDetails.KekIvAddr = (UINTPTR)SecurePtr->PrtnHdr->KekIv;
		Status = XLoader_AesKeySelect(SecurePtr,
					&KeyDetails, &KeySrc);
		if (Status != XLOADER_SUCCESS) {
			goto END;
		}
		/* Configure DPA counter measure */
		DpaCmCfg = XilPdi_IsDpaCmEnable(SecurePtr->PrtnHdr);
		Status = XLoader_SetAesDpaCm(&SecurePtr->AesInstance, DpaCmCfg);
		if (Status != XLOADER_SUCCESS) {
			Status  = XLOADER_UPDATE_MIN_ERR(
					XLOADER_SEC_DPA_CM_ERR, Status);
			goto END;
		}
		/* Decrypting SH */
		Status = XSecure_AesDecryptInit(&SecurePtr->AesInstance, KeySrc,
					XSECURE_AES_KEY_SIZE_256,
					(UINTPTR)SecurePtr->PrtnHdr->PrtnIv);
		if (Status != XLOADER_SUCCESS) {
			Status = XLOADER_UPDATE_MIN_ERR(
				XLOADER_SEC_AES_OPERATION_FAILED, Status);
			goto END;
		}
		/* Decrypt Secure header */
		Status = XLoader_DecryptSH(SecurePtr, SrcAddr);
		if (Status != XLOADER_SUCCESS) {
			goto END;
		}
		SrcAddr = SrcAddr + XLOADER_SECURE_HDR_TOTAL_SIZE;
		ChunkSize = ChunkSize - XLOADER_SECURE_HDR_TOTAL_SIZE;

	}
	Status = XLoader_DataDecrypt(SecurePtr,
			 SrcAddr, DstAddr, ChunkSize);
	if (Status != XLOADER_SUCCESS) {
		Status = XLOADER_UPDATE_MIN_ERR(
				XLOADER_SEC_AES_OPERATION_FAILED, Status);
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function helps in key selection.
 *
 * @param	SecurePtr is pointer to the XLoader_SecureParms
 * @param	AesInstancePtr is pointer to the AES instance
 * @param	KeyDetails is pointer to the key details.
 * @param	KeySrc is pointer to the key source to be updated as
 *		key source for decrypting. If key provided is KEK format, this API
 *		decrypts and provides the red key source.
 *
 * @return	XLOADER_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
static u32 XLoader_AesKeySelect(XLoader_SecureParms *SecurePtr,
						XLoader_AesKekKey *KeyDetails,
						XSecure_AesKeySrc *KeySrc)

{
	u32 Status = XLOADER_FAILURE;
	u32 *KekStatus = &SecurePtr->PdiPtr->KekStatus;
	XilPdi_BootHdr *BootHdr = &SecurePtr->PdiPtr->MetaHdr.BootHdr;

	XPlmi_Printf(DEBUG_INFO, "Key source is %0x\n\r", KeyDetails->PdiKeySrc);
	switch (KeyDetails->PdiKeySrc) {
	case XLOADER_EFUSE_KEY:
		*KeySrc = XSECURE_AES_EFUSE_KEY;
		Status = XLOADER_SUCCESS;
		break;
	case XLOADER_EFUSE_BLK_KEY:
		if (((*KekStatus) & XLOADER_EFUSE_RED_KEY) == 0x0U) {
			KeyDetails->KekType = XSECURE_BLACK_KEY;
			KeyDetails->KeySrc = XSECURE_AES_EFUSE_KEY;
			KeyDetails->KeyDst = XSECURE_AES_EFUSE_RED_KEY;

			Status = XLoader_DecryptBlkKey(&SecurePtr->AesInstance, KeyDetails);
			if (Status == XLOADER_SUCCESS) {
				*KekStatus = (*KekStatus) | XLOADER_EFUSE_RED_KEY;
				*KeySrc = XSECURE_AES_EFUSE_RED_KEY;
			}
			else {
				Status  = XLOADER_UPDATE_MIN_ERR(XLOADER_SEC_AES_KEK_DEC,
									Status);
			}
		}
		else {
			Status = XLOADER_SUCCESS;
			*KeySrc = XSECURE_AES_EFUSE_RED_KEY;
		}
		break;
	case XLOADER_EFUSE_OBFUS_KEY:
		if (((*KekStatus) & XLOADER_EFUSE_RED_KEY) == 0x0U) {
			/* If corresponding RED key is not available */
			XPlmi_Printf(DEBUG_INFO,
						"Decrypting EFUSE obfuscated key\n\r");

			Status = XSecure_AesKekDecrypt(&SecurePtr->AesInstance,
					XSECURE_OBFUSCATED_KEY, XSECURE_AES_EFUSE_KEY,
					XSECURE_AES_EFUSE_RED_KEY,
					(UINTPTR)KeyDetails->KekIvAddr,
					XSECURE_AES_KEY_SIZE_256);
			if (Status == XLOADER_SUCCESS) {
				/* Update Status */
				*KekStatus = (*KekStatus) | XLOADER_EFUSE_RED_KEY;
				*KeySrc = XSECURE_AES_EFUSE_RED_KEY;
			}
			else {
				Status  = XLOADER_UPDATE_MIN_ERR(XLOADER_SEC_AES_KEK_DEC,
									Status);
			}
		}
		else {
			Status = XLOADER_SUCCESS;
			*KeySrc = XSECURE_AES_EFUSE_RED_KEY;
		}
		break;
	case XLOADER_BBRAM_KEY:
		*KeySrc = XSECURE_AES_BBRAM_KEY;
		Status = XLOADER_SUCCESS;
		break;
	case XLOADER_BBRAM_BLK_KEY:
		if (((*KekStatus) & XLOADER_BBRAM_RED_KEY) == 0x0U) {
			KeyDetails->KekType = XSECURE_BLACK_KEY;
			KeyDetails->KeySrc = XSECURE_AES_BBRAM_KEY;
			KeyDetails->KeyDst = XSECURE_AES_BBRAM_RED_KEY;

			Status = XLoader_DecryptBlkKey(&SecurePtr->AesInstance, KeyDetails);
			if (Status == XLOADER_SUCCESS) {
				*KekStatus = (*KekStatus) | XLOADER_BBRAM_RED_KEY;
				*KeySrc = XSECURE_AES_BBRAM_RED_KEY;
			}
			else {
				Status  = XLOADER_UPDATE_MIN_ERR(XLOADER_SEC_AES_KEK_DEC,
									Status);
			}
		}
		else {
			Status = XLOADER_SUCCESS;
			*KeySrc = XSECURE_AES_BBRAM_RED_KEY;
		}
		break;
	case XLOADER_BBRAM_OBFUS_KEY:
		if (((*KekStatus) & XLOADER_BBRAM_RED_KEY) == 0x0U) {
			/* If corresponding RED key is not available */
			XPlmi_Printf(DEBUG_INFO,
						"Decrypting BBRAM obfuscated key\n\r");

			Status = XSecure_AesKekDecrypt(&SecurePtr->AesInstance,
					XSECURE_OBFUSCATED_KEY, XSECURE_AES_BBRAM_KEY,
					XSECURE_AES_BBRAM_RED_KEY,
					(UINTPTR)KeyDetails->KekIvAddr,
					XSECURE_AES_KEY_SIZE_256);
			if (Status == XLOADER_SUCCESS) {
				/* Update Status */
				*KekStatus = (*KekStatus) | XLOADER_BBRAM_RED_KEY;
				*KeySrc = XSECURE_AES_BBRAM_RED_KEY;
			}
			else {
				Status  = XLOADER_UPDATE_MIN_ERR(XLOADER_SEC_AES_KEK_DEC,
									Status);
			}
		}
		else {
			Status = XLOADER_SUCCESS;
			*KeySrc = XSECURE_AES_BBRAM_RED_KEY;
		}
		break;
	case XLOADER_BH_BLK_KEY:
		if (((*KekStatus) & XLOADER_BHDR_RED_KEY) == 0x0U) {
			KeyDetails->KekType = XSECURE_BLACK_KEY;
			KeyDetails->KeySrc = XSECURE_AES_BH_KEY;
			KeyDetails->KeyDst = XSECURE_AES_BH_RED_KEY;

			/* Write BH key into BH registers */
			Status = XSecure_AesWriteKey(&SecurePtr->AesInstance,
				XSECURE_AES_BH_KEY, XSECURE_AES_KEY_SIZE_256,
					(UINTPTR)BootHdr->Kek);
			if (Status != XLOADER_SUCCESS) {
				break;
			}
			Status = XLoader_DecryptBlkKey(&SecurePtr->AesInstance, KeyDetails);
			if (Status == XLOADER_SUCCESS) {
				*KekStatus = (*KekStatus) | XLOADER_BHDR_RED_KEY;
				*KeySrc = XSECURE_AES_BH_RED_KEY;
			}
			else {
				Status  = XLOADER_UPDATE_MIN_ERR(XLOADER_SEC_AES_KEK_DEC,
									Status);
			}
		}
		else {
			Status = XLOADER_SUCCESS;
			*KeySrc = XSECURE_AES_BH_RED_KEY;
		}
		break;
	case XLOADER_BH_OBFUS_KEY:
		if ((*KekStatus & XLOADER_BHDR_RED_KEY) == 0x0U) {
			/* If corresponding RED key is not available */
			XPlmi_Printf(DEBUG_INFO,
					"Decryting BH obfuscated key\n\r");
			/* Write BH key into BH registers */
			Status = XSecure_AesWriteKey(&SecurePtr->AesInstance,
				XSECURE_AES_BH_KEY, XSECURE_AES_KEY_SIZE_256,
					(UINTPTR)BootHdr->Kek);
			if (Status != XLOADER_SUCCESS) {
				break;
			}
			Status = XSecure_AesKekDecrypt(&SecurePtr->AesInstance,
					XSECURE_OBFUSCATED_KEY, XSECURE_AES_BH_KEY,
					XSECURE_AES_BH_RED_KEY,
				(UINTPTR)KeyDetails->KekIvAddr,
				XSECURE_AES_KEY_SIZE_256);
			if (Status == XLOADER_SUCCESS) {
					*KekStatus = (*KekStatus) | XLOADER_BHDR_RED_KEY;
					*KeySrc = XSECURE_AES_BH_RED_KEY;
			}
			else {
				Status  = XLOADER_UPDATE_MIN_ERR(XLOADER_SEC_AES_KEK_DEC,
									Status);
			}
		}
		else {
			Status = XLOADER_SUCCESS;
			*KeySrc = XSECURE_AES_BH_RED_KEY;
		}
		break;
	case XLOADER_EFUSE_USR_KEY0:
		*KeySrc = XSECURE_AES_EFUSE_USER_KEY_0;
		Status = XLOADER_SUCCESS;
		break;
	case XLOADER_EFUSE_USR_BLK_KEY0:
		if (((*KekStatus) & XLOADER_EFUSE_USR0_RED_KEY) == 0x0U) {
			KeyDetails->KekType = XSECURE_BLACK_KEY;
			KeyDetails->KeySrc = XSECURE_AES_EFUSE_USER_KEY_0;
			KeyDetails->KeyDst = XSECURE_AES_EFUSE_USER_RED_KEY_0;

			Status = XLoader_DecryptBlkKey(&SecurePtr->AesInstance, KeyDetails);
			if (Status == XLOADER_SUCCESS) {
				*KekStatus = (*KekStatus) | XLOADER_EFUSE_USR0_RED_KEY;
				*KeySrc = XSECURE_AES_EFUSE_USER_RED_KEY_0;
			}
			else {
				Status  = XLOADER_UPDATE_MIN_ERR(XLOADER_SEC_AES_KEK_DEC,
									Status);
			}
		}
		else {
			Status = XLOADER_SUCCESS;
			*KeySrc = XSECURE_AES_EFUSE_USER_RED_KEY_0;
		}
		break;
	case XLOADER_EFUSE_USR_OBFUS_KEY0:
		if (((*KekStatus) & XLOADER_EFUSE_USR0_RED_KEY) == 0x0U) {
			/* If corresponding RED key is not available */
			XPlmi_Printf(DEBUG_INFO,
						"Decrypting BBRAM obfuscated key\n\r");

			Status = XSecure_AesKekDecrypt(&SecurePtr->AesInstance,
					XSECURE_OBFUSCATED_KEY, XSECURE_AES_EFUSE_USER_KEY_0,
					XSECURE_AES_EFUSE_USER_RED_KEY_0,
					(UINTPTR)KeyDetails->KekIvAddr,
					XSECURE_AES_KEY_SIZE_256);
			if (Status == XLOADER_SUCCESS) {
				/* Update Status */
				*KekStatus = (*KekStatus) | XLOADER_EFUSE_USR0_RED_KEY;
				*KeySrc = XSECURE_AES_EFUSE_USER_RED_KEY_0;
			}
			else {
				Status  = XLOADER_UPDATE_MIN_ERR(XLOADER_SEC_AES_KEK_DEC,
									Status);
			}
		}
		else {
			Status = XLOADER_SUCCESS;
			*KeySrc = XSECURE_AES_EFUSE_USER_RED_KEY_0;
		}
		break;
	case XLOADER_EFUSE_USR_KEY1:
		*KeySrc = XSECURE_AES_EFUSE_USER_KEY_1;
		Status = XLOADER_SUCCESS;
		break;
	case XLOADER_EFUSE_USR_BLK_KEY1:
		if (((*KekStatus) & XLOADER_EFUSE_USR1_RED_KEY) == 0x0U) {
			KeyDetails->KekType = XSECURE_BLACK_KEY;
			KeyDetails->KeySrc = XSECURE_AES_EFUSE_USER_KEY_1;
			KeyDetails->KeyDst = XSECURE_AES_EFUSE_USER_RED_KEY_1;

			Status = XLoader_DecryptBlkKey(&SecurePtr->AesInstance, KeyDetails);
			if (Status == XLOADER_SUCCESS) {
				*KekStatus = (*KekStatus) | XLOADER_EFUSE_USR1_RED_KEY;
				*KeySrc = XSECURE_AES_EFUSE_USER_RED_KEY_1;
			}
			else {
				Status  = XLOADER_UPDATE_MIN_ERR(XLOADER_SEC_AES_KEK_DEC,
									Status);
			}
		}
		else {
			Status = XLOADER_SUCCESS;
			*KeySrc = XSECURE_AES_EFUSE_USER_RED_KEY_1;
		}
		break;
	case XLOADER_EFUSE_USR_OBFUS_KEY1:
		if (((*KekStatus) & XLOADER_EFUSE_USR1_RED_KEY) == 0x0U) {
			/* If corresponding RED key is not available */
			XPlmi_Printf(DEBUG_INFO,
						"Decrypting BBRAM obfuscated key\n\r");

			Status = XSecure_AesKekDecrypt(&SecurePtr->AesInstance,
					XSECURE_OBFUSCATED_KEY, XSECURE_AES_EFUSE_USER_KEY_1,
					XSECURE_AES_EFUSE_USER_RED_KEY_1,
					(UINTPTR)KeyDetails->KekIvAddr,
					XSECURE_AES_KEY_SIZE_256);
			if (Status == XLOADER_SUCCESS) {
				/* Update Status */
				*KekStatus = (*KekStatus) | XLOADER_EFUSE_USR1_RED_KEY;
				*KeySrc = XSECURE_AES_EFUSE_USER_RED_KEY_1;
			}
			else {
				Status  = XLOADER_UPDATE_MIN_ERR(XLOADER_SEC_AES_KEK_DEC,
									Status);
			}
		}
		else {
			Status = XLOADER_SUCCESS;
			*KeySrc = XSECURE_AES_EFUSE_USER_RED_KEY_1;
		}
		break;
	case XLOADER_USR_KEY0:
		*KeySrc = XSECURE_AES_USER_KEY_0;
		Status = XLOADER_SUCCESS;
		break;
	case XLOADER_USR_KEY1:
		*KeySrc = XSECURE_AES_USER_KEY_1;
		Status = XLOADER_SUCCESS;
		break;
	case XLOADER_USR_KEY2:
		*KeySrc = XSECURE_AES_USER_KEY_2;
		Status = XLOADER_SUCCESS;
		break;
	case XLOADER_USR_KEY3:
		*KeySrc = XSECURE_AES_USER_KEY_3;
		Status = XLOADER_SUCCESS;
		break;
	case XLOADER_USR_KEY4:
		*KeySrc = XSECURE_AES_USER_KEY_4;
		Status = XLOADER_SUCCESS;
		break;
	case XLOADER_USR_KEY5:
		*KeySrc = XSECURE_AES_USER_KEY_5;
		Status = XLOADER_SUCCESS;
		break;
	case XLOADER_USR_KEY6:
		*KeySrc = XSECURE_AES_USER_KEY_6;
		Status = XLOADER_SUCCESS;
		break;
	case XLOADER_USR_KEY7:
		*KeySrc = XSECURE_AES_USER_KEY_7;
		Status = XLOADER_SUCCESS;
		break;
	default:
		Status = XLOADER_UPDATE_MIN_ERR(
				XLOADER_SEC_DEC_INVALID_KEYSRC_SEL, 0x0U);
		break;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function authenticates image headers and partition headers
 * of image.
 *
 * @param	SecurePtr	Pointer to the XLoader_SecureParms
 * @param	MetaHdr		Pointer to the Meta header.
 *
 * @return	XLOADER_SUCCESS if verification was successful.
 *			Error code on failure
 *
 ******************************************************************************/
static u32 XLoader_AuthHdrs(XLoader_SecureParms *SecurePtr,
			XilPdi_MetaHdr *MetaHdr)
{
	u32 Status = XLOADER_FAILURE;
	int ClrIh = XST_FAILURE;
	int ClrPh = XST_FAILURE;
	int SStatus = XST_FAILURE;
	Sha3Hash HashVal;
	XSecure_Sha3 Sha3Instance;

	/* Get DMA instance */
	SecurePtr->PmcDmaInstPtr = XPlmi_GetDmaInstance(PMCDMA_0_DEVICE_ID);
	if (SecurePtr->PmcDmaInstPtr == NULL) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_HDR_GET_DMA, 0x0U);
		goto END;
	}

	MetaHdr->Flag = XILPDI_METAHDR_RD_HDRS_FROM_DEVICE;
	/* Read IHT and PHT to structures and verify checksum */
	SStatus = XilPdi_ReadAndVerifyImgHdr(MetaHdr);
	if (SStatus != XST_SUCCESS)
	{
		Status = XPLMI_UPDATE_STATUS(
				XLOADER_ERR_SEC_IH_READ_VERIFY_FAIL, SStatus);
		goto END;
	}

	SStatus = XilPdi_ReadAndVerifyPrtnHdr(MetaHdr);
	if(SStatus != XST_SUCCESS)
	{
		Status = XPLMI_UPDATE_STATUS(
				XLOADER_ERR_SEC_PH_READ_VERIFY_FAIL, SStatus);
		goto END;
	}

	/*
	 * As SPK and PPK are validated during authentication of IHT,
	 * using the same valid SPK to authenticate IHs and PHs.
	 * Calculate hash on the data
	 */
	Status = XSecure_Sha3Initialize(&Sha3Instance,
			SecurePtr->PmcDmaInstPtr);
	if (Status != XLOADER_SUCCESS) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_HDR_HASH_CALC_FAIL,
						 Status);
		goto END;
	}

	XSecure_Sha3Start(&Sha3Instance);
	Status = XSecure_Sha3Update(&Sha3Instance, (u8 *)SecurePtr->AcPtr,
		XLOADER_AUTH_CERT_MIN_SIZE - XLOADER_PARTITION_SIG_SIZE);
	if (Status != XLOADER_SUCCESS) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_HDR_HASH_CALC_FAIL,
						 Status);
		goto END;
	}

	/* Image headers */
	Status = XSecure_Sha3Update(&Sha3Instance, (u8 *)MetaHdr->ImgHdr,
				(MetaHdr->ImgHdrTbl.NoOfImgs * XIH_IH_LEN));
	if (Status != XLOADER_SUCCESS) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_HDR_HASH_CALC_FAIL,
						 Status);
		goto END;
	}

	/* Partition headers */
	Status = XSecure_Sha3Update(&Sha3Instance, (u8 *)MetaHdr->PrtnHdr,
				(MetaHdr->ImgHdrTbl.NoOfPrtns * XIH_PH_LEN));
	if (Status != XLOADER_SUCCESS) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_HDR_HASH_CALC_FAIL,
						 Status);
		goto END;
	}
	/* Read hash */
	Status = XSecure_Sha3Finish(&Sha3Instance, &HashVal);
	if (Status != XLOADER_SUCCESS) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_HDR_HASH_CALC_FAIL,
						 Status);
		goto END;
	}
	XPlmi_PrintArray(DEBUG_INFO,
		(UINTPTR)HashVal.Hash,
		XLOADER_SHA3_LEN / XIH_PRTN_WORD_LEN,
		"Headers Hash");

	/* Signature Verification */
	Status = XLoader_DataAuth(SecurePtr, HashVal.Hash,
					 (u8 *)SecurePtr->AcPtr->ImgSignature);
	if (Status != XLOADER_SUCCESS) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_HDR_AUTH_FAIL, Status);
		goto END;
	}

	XPlmi_Printf(DEBUG_INFO, "Authentication of"
				" partition and image headers is successful\n\r");

END:
	if (Status != XLOADER_SUCCESS) {
		XPlmi_Printf(DEBUG_INFO, "Clearing memory\n\r");
		/* Image and partition headers are cleared upon failure */
		ClrIh = XPlmi_InitNVerifyMem((UINTPTR)(MetaHdr->ImgHdr),
					(MetaHdr->ImgHdrTbl.NoOfImgs * XIH_IH_LEN));
		ClrPh = XPlmi_InitNVerifyMem((UINTPTR)(MetaHdr->PrtnHdr),
					(MetaHdr->ImgHdrTbl.NoOfPrtns * XIH_PH_LEN));
		if ((ClrPh != XST_SUCCESS) || (ClrIh != XST_SUCCESS)) {
			Status = Status | XLOADER_SEC_BUF_CLEAR_ERR;
		}
		else {
			Status = Status | XLOADER_SEC_BUF_CLEAR_SUCCESS;
		}
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function copies whole secure headers to the buffer.
 *
 * @param	SecurePtr	Pointer to the XLoader_SecureParms
 * @param	MetaHdr		Pointer to the Meta header.
 * @param	BufferAddr	Read whole headers to the mentioned buffer
 *		address
 *
 * @return	XLOADER_SUCCESS if verification was successful.
 *			Error code on failure
 *
 ******************************************************************************/
static u32 XLoader_ReadHdrs(XLoader_SecureParms *SecurePtr,
			XilPdi_MetaHdr *MetaHdr, u64 BufferAddr)
{
	u32 Status = XLOADER_FAILURE;
	u32 TotalSize = MetaHdr->ImgHdrTbl.TotalHdrLen * XIH_PRTN_WORD_LEN;
	u32 ImgHdrAddr;

	/* Update the first image header address */
	ImgHdrAddr = MetaHdr->ImgHdrTbl.ImgHdrAddr
				* XIH_PRTN_WORD_LEN;

	if (SecurePtr->IsAuthenticated == TRUE) {
		TotalSize = TotalSize - XLOADER_AUTH_CERT_MIN_SIZE;
	}

	/* Read IHT and PHT to buffers along with encryption overhead */
	Status = MetaHdr->DeviceCopy(MetaHdr->FlashOfstAddr +
					ImgHdrAddr, (u64 )BufferAddr,
					TotalSize, 0x0U);
	if (XLOADER_SUCCESS != Status) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_HDR_COPY_FAIL, Status);
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function authenticates and decrypts the headers.
 *
 * @param	SecurePtr	Pointer to the XLoader_SecureParms
 * @param	MetaHdr		Pointer to the Meta header.
 * @param	BufferAddr	Read whole headers to the mentioned buffer
 *		address
 *
 * @return	XLOADER_SUCCESS if verification was successful.
 *			Error code on failure
 *
 ******************************************************************************/
static u32 XLoader_AuthNDecHdrs(XLoader_SecureParms *SecurePtr,
		XilPdi_MetaHdr *MetaHdr,
		u64 BufferAddr)
{

	u32 Status = XLOADER_FAILURE;
	int ClrStatus = XST_FAILURE;
	Sha3Hash CalHash;
	XSecure_Sha3 Sha3Instance;
	u32 TotalSize = MetaHdr->ImgHdrTbl.TotalHdrLen * XIH_PRTN_WORD_LEN;

	if (SecurePtr->IsAuthenticated == TRUE) {
		TotalSize = TotalSize - XLOADER_AUTH_CERT_MIN_SIZE;
	}

	/* Authenticate the headers */
	Status = XSecure_Sha3Initialize(&Sha3Instance,
				SecurePtr->PmcDmaInstPtr);
	if (Status != XLOADER_SUCCESS) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_HDR_HASH_CALC_FAIL,
							 Status);
		goto END;
	}

	XSecure_Sha3Start(&Sha3Instance);
	Status = XSecure_Sha3Update(&Sha3Instance, (u8 *)SecurePtr->AcPtr,
		(XLOADER_AUTH_CERT_MIN_SIZE - XLOADER_PARTITION_SIG_SIZE));
	if (Status != XLOADER_SUCCESS) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_HDR_HASH_CALC_FAIL,
							 Status);
		goto END;
	}
	Status = XSecure_Sha3Update(&Sha3Instance, (u8 *)(UINTPTR)BufferAddr, TotalSize);
	if (Status != XLOADER_SUCCESS) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_HDR_HASH_CALC_FAIL,
							 Status);
		goto END;
	}

	Status = XSecure_Sha3Finish(&Sha3Instance, &CalHash);
	if (Status != XLOADER_SUCCESS) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_HDR_HASH_CALC_FAIL,
							 Status);
		goto END;
	}

	/* RSA PSS signature verification */
	Status = XLoader_DataAuth(SecurePtr, CalHash.Hash,
				(u8 *)SecurePtr->AcPtr->ImgSignature);
	if (Status != XLOADER_SUCCESS) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_HDR_AUTH_FAIL,
							 Status);
		goto END;
	}
	else {
		XPlmi_Printf(DEBUG_INFO,
			"Authentication of the headers is successful\n\r");
	}

	/* Decrypt the headers and copy to structures */
	Status = XLoader_DecHdrs(SecurePtr, MetaHdr, BufferAddr);
	if (Status != XLOADER_SUCCESS) {
		goto END;
	}

END:
	if (Status != XLOADER_SUCCESS) {
		/* Clear the buffer */
		ClrStatus = XPlmi_InitNVerifyMem(BufferAddr, TotalSize);
		if (ClrStatus != XST_SUCCESS) {
			Status = Status | XLOADER_SEC_BUF_CLEAR_ERR;
		}
		else {
			Status = Status | XLOADER_SEC_BUF_CLEAR_SUCCESS;
		}
	}
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function decrypts headers.
 *
 * @param	SecurePtr	Pointer to the XLoader_SecureParms
 * @param	MetaHdr		Pointer to the Meta header.
 * @param	BufferAddr	Read whole headers to the mentioned buffer
 *		address
 *
 * @return	XLOADER_SUCCESS if verification was successful.
 *			Error code on failure
 *
 ******************************************************************************/
static u32 XLoader_DecHdrs(XLoader_SecureParms *SecurePtr,
			XilPdi_MetaHdr *MetaHdr, u64 BufferAddr)
{
	u32 Status = XLOADER_FAILURE;
	u32 Iv[XLOADER_SECURE_IV_LEN];
	u8 Index;
	XSecure_AesKeySrc KeySrc = 0;
	u32 TotalSize = MetaHdr->ImgHdrTbl.TotalHdrLen * XIH_PRTN_WORD_LEN;
	u32 SrcAddr = BufferAddr;
	u32 DpaCmCfg = XilPdi_IsDpaCmEnableMetaHdr(&MetaHdr->ImgHdrTbl);
	XLoader_AesKekKey KeyDetails;

	if (SecurePtr->IsAuthenticated == TRUE) {
		TotalSize = TotalSize - XLOADER_AUTH_CERT_MIN_SIZE;
	}

	if (SecurePtr->IsEncrypted != TRUE) {
		XPlmi_Printf(DEBUG_INFO,"Headers are not encrypted\n\r");
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_HDR_NOT_ENCRYPTED,
							 0x0U);
		goto END;
	}

	/* Get DMA instance */
	SecurePtr->PmcDmaInstPtr = XPlmi_GetDmaInstance(PMCDMA_0_DEVICE_ID);
	if (SecurePtr->PmcDmaInstPtr == NULL) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_HDR_GET_DMA, 0x0U);
		goto END;
	}

	/* Initialize AES driver */
	Status = XSecure_AesInitialize(&SecurePtr->AesInstance, SecurePtr->PmcDmaInstPtr);
	if (Status != XLOADER_SUCCESS) {
		XPlmi_Printf(DEBUG_INFO,
			"Failed at XSecure_AesInitialize\n\r");
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_HDR_AES_OP_FAIL,
							 Status);
		goto END;
	}

	KeyDetails.PufHdLocation = XilPdi_GetPufHdMetaHdr(&MetaHdr->ImgHdrTbl) >>
					XIH_IHT_ATTR_PUFHD_SHIFT;
	KeyDetails.PdiKeySrc = MetaHdr->ImgHdrTbl.EncKeySrc;
	KeyDetails.KekIvAddr =
		(UINTPTR)SecurePtr->PdiPtr->MetaHdr.ImgHdrTbl.KekIv;
	/*
	 * Key source selection
	 */
	Status = XLoader_AesKeySelect(SecurePtr, &KeyDetails, &KeySrc);
	if (Status != XLOADER_SUCCESS) {
		XPlmi_Printf(DEBUG_INFO,"Failed at Key selection\n\r");
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_HDR_AES_OP_FAIL,
							 Status);
		goto END;
	}

	/* Configure DPA CM */
	Status = XLoader_SetAesDpaCm(&SecurePtr->AesInstance, DpaCmCfg);
	if (Status != XLOADER_SUCCESS) {
		Status = XLOADER_UPDATE_MIN_ERR(XLOADER_SEC_DPA_CM_ERR,
								Status);
		goto END;
	}

	/* Decrypting SH */
	Status = XSecure_AesDecryptInit(&SecurePtr->AesInstance, KeySrc,
			XSECURE_AES_KEY_SIZE_256,
			(UINTPTR)MetaHdr->ImgHdrTbl.IvMetaHdr);
	if (Status != XLOADER_SUCCESS) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_HDR_AES_OP_FAIL,
							 Status);
		goto END;
	}

	/* Decrypt Secure header */
	Status = XLoader_DecryptSH(SecurePtr, SrcAddr);
	if (Status != XLOADER_SUCCESS) {
		XPlmi_Printf(DEBUG_INFO,
		"SH decryption failed during header decryption\n\r");
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_HDR_DEC_FAIL,
							 Status);
		goto END;
	}

	SrcAddr = SrcAddr + XLOADER_SECURE_HDR_TOTAL_SIZE;
	TotalSize = TotalSize - XLOADER_SECURE_HDR_TOTAL_SIZE;


	for (Index = 0U; Index < XLOADER_SECURE_IV_LEN; Index++) {
		Iv[Index] = Xil_Htonl(XPlmi_In32(SecurePtr->AesInstance.BaseAddress +
		(XSECURE_AES_IV_0_OFFSET + (Index * XIH_PRTN_WORD_LEN))));
	}

	Status = XSecure_AesDecryptInit(&SecurePtr->AesInstance,
		XSECURE_AES_KUP_KEY, XSECURE_AES_KEY_SIZE_256, (UINTPTR)Iv);
	if (Status != XLOADER_SUCCESS) {
		XPlmi_Printf(DEBUG_INFO,"Failed at header decryption"
					" XSecure_AesDecryptInit\n\r");
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_HDR_AES_OP_FAIL,
							 Status);
		goto END;
	}
	Status = XLoader_DataDecrypt(SecurePtr,
			 (UINTPTR)SrcAddr,
			(UINTPTR)SecurePtr->ChunkAddr, TotalSize);
	if (Status != XLOADER_SUCCESS) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_HDR_DEC_FAIL, Status);
		XPlmi_Printf(DEBUG_INFO, "Failed at headers decryption\n\r");
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function enables or disables DPA counter measures.
 *
 * @param	AesInstPtr	Pointer to the XSecure_Aes instance.
 * @param	DpaCmCfg
 *				- TRUE - to enable AES DPA counter measure
 *				- FALSE -to disable AES DPA counter measure
 *
 * @return
 *			- XLOADER_SUCCESS if enable/disable of DPA was successful.
 *			- Error if device doesn't support DPA CM or configuration is
 *			not successful
 *
 ******************************************************************************/
static u32 XLoader_SetAesDpaCm(XSecure_Aes *AesInstPtr, u32 DpaCmCfg)
{
	u32 Status = XLOADER_FAILURE;

	/* Configure AES DPA CM */
	Status = XSecure_AesSetDpaCm(AesInstPtr, DpaCmCfg);

	/* If DPA CM request is to disable and device also not supports DPA CM */
	if ((Status == XSECURE_AES_DPA_CM_NOT_SUPPORTED) &&
			(DpaCmCfg == FALSE)) {
		Status = XLOADER_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function decrypts the black key with PUF key and stores in specified
 * destination AES red key register.
 *
 * @param	AesInstPtr is pointer to the XSecure_Aes instance.
 * @param	KeyDetails is pointer to the XLoader_AesKekKey instance.
 *
 * @return	XLOADER_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
static u32 XLoader_DecryptBlkKey(XSecure_Aes *AesInstPtr,
					XLoader_AesKekKey *KeyDetails)
{
	u32 Status = XLOADER_FAILURE;
	XPuf_Data PufData;

	XPlmi_Printf(DEBUG_INFO, "Decrypting PUF KEK\n\r");
	PufData.RegMode = XPUF_SYNDROME_MODE_4K;
	PufData.ShutterValue = XPUF_SHUTTER_VALUE;
	PufData.PufOperation = XPUF_REGEN_ON_DEMAND;

	if (KeyDetails->PufHdLocation == XLOADER_PUF_HD_BHDR) {
		PufData.ReadOption = XPUF_READ_FROM_RAM;
		PufData.SyndromeAddr = XIH_BH_PRAM_ADDR + XIH_BH_PUF_HD_OFFSET;
		PufData.Chash = *(u32 *)(XIH_BH_PRAM_ADDR + XIH_BH_PUF_CHASH_OFFSET);
		PufData.Aux = *(u32 *)(XIH_BH_PRAM_ADDR + XIH_BH_PUF_AUX_OFFSET);
		XPlmi_Printf(DEBUG_INFO,
				"BHDR PUF HELPER DATA with CHASH:%0x and AUX:%0x\n\r",
				PufData.Chash, PufData.Aux);
	}
	else {
		XPlmi_Printf(DEBUG_INFO, "EFUSE PUF HELPER DATA\n\r");
		PufData.ReadOption = XPUF_READ_FROM_EFUSE_CACHE;
	}

	Status = XPuf_Regeneration(&PufData);
	if (Status != XLOADER_SUCCESS) {
		XPlmi_Printf(DEBUG_GENERAL,
			"Failed at PUF regeneration with status %0x\n\r", Status);
		Status  = XLOADER_UPDATE_MIN_ERR(XLOADER_SEC_PUF_REGN_ERRR,
						 Status);
		goto END;
	}

	Status = XSecure_AesKekDecrypt(AesInstPtr,
				XSECURE_BLACK_KEY, KeyDetails->KeySrc,
				KeyDetails->KeyDst, (UINTPTR)KeyDetails->KekIvAddr,
				XSECURE_AES_KEY_SIZE_256);
	if (Status != XLOADER_SUCCESS) {
		XPlmi_Printf(DEBUG_GENERAL, "Failed during AES KEK decrypt\n\r");
		Status  = XLOADER_UPDATE_MIN_ERR(XLOADER_SEC_AES_KEK_DEC,
						 Status);
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief       This function performs KAT test on AES crypto Engine
*
* @param       SecurePtr       Pointer to the XLoader_SecureParms instance.
*
* @return      XLOADER_SUCCESS on success and error code on failure
*
******************************************************************************/
static u32 XLoader_AesKatTest(XLoader_SecureParms *SecurePtr)
{
	u32 Status = XLOADER_SUCCESS;
	u32 DpacmEfuseStatus;
	u32 PlmDpacmKatStatus;

	/*
	 * Skip running the KAT for AES DPACM or AES if it is already run by ROM
	 * KAT will be run only when the CYRPTO_KAT_EN bits in eFUSE are set
	 */
	DpacmEfuseStatus = XPlmi_In32(XLOADER_EFUSE_SEC_MISC1_OFFSET) &
		XLOADER_EFUSE_SEC_DPA_DIS_MASK;
	PlmDpacmKatStatus = SecurePtr->PdiPtr->PlmKatStatus &
		XLOADER_DPACM_KAT_MASK;

	if((DpacmEfuseStatus == 0U) && (PlmDpacmKatStatus == 0U)) {
		Status = XSecure_AesDecryptCmKat(&SecurePtr->AesInstance);
		if(Status != XLOADER_SUCCESS) {
			XPlmi_Printf(DEBUG_GENERAL, "DPACM KAT Failed\n\r");
			Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_KAT_FAILED, Status);
			goto END;
		}
		SecurePtr->PdiPtr->PlmKatStatus |= XLOADER_DPACM_KAT_MASK;
	}

	if((SecurePtr->PdiPtr->PlmKatStatus & XLOADER_AES_KAT_MASK) == 0U) {
		Status = XSecure_AesDecryptKat(&SecurePtr->AesInstance);
		if(Status != XLOADER_SUCCESS) {
			XPlmi_Printf(DEBUG_GENERAL, "AES KAT Failed\n\r");
			Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_KAT_FAILED, Status);
			goto END;
		}
		SecurePtr->PdiPtr->PlmKatStatus |= XLOADER_AES_KAT_MASK;
	}

END:
	return Status;
}
