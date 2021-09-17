/******************************************************************************
* Copyright (c) 2017 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure.c
*
* This file contains the implementation of the interface functions for secure
* library.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -------------------------------------------------------
* 1.0   DP  02/15/17 Initial release
* 2.2   vns 09/18/17 Added APIs to support generic functionality
*                    for SHA3 and RSA hardware at linux level.
*                    Removed RSA-SHA2 authentication support
*                    for loading linux image and dtb from u-boot, as here it
*                    is using SHA2 hash and single RSA key pair authentication
* 3.0   vns 02/21/18 Added support for single partition image authentication
*                    and/or decryption.
* 3.1   vns 04/13/18 Added device key support even if authentication is not
*                    been enabled for single partition image, when PMUFW is
*                    compiled by enabling secure environment variable in bsp
*                    settings.
*       ka  04/10/18 Added support for user-efuse revocation
*       ka  04/18/18 Added support for Zeroization of the memory in case of
*                    Gcm-Tag mismatch
* 3.2   ka  04/04/18 Added support for Sha3_Update, if the payload is not
*        	     4 bytes aligned.
*       ka  08/03/18 Added XSecure_Aes Api's to encrypt or decrypt data-blobs.
*       ka  10/25/18 Added support to clear user key after use.
* 4.0   arc 18/12/18 Fixed MISRA-C violations.
*       arc 12/02/19 Added support for validate image format.
*       rama 18/03/19 Fixed IAR compiler errors and warnings
*       arc 03/20/19 Reading the return value of the functions to validate
* 4.0  Nava 19/03/19 In the current implementation, the SecureIv variable
*                    is sharing between xilfpga and Xilsecure libraries.
*                    To avoid data sharing conflicts removed SecureIV
*                    shared variable dependency.
*       mmd 03/15/19 Refactored the code
*       psl 03/26/19 Fixed MISRA-C violation
* 4.1   psl 07/02/19 Fixed Coverity warning.
*       mmd 07/05/19 Optimized the code
*       psl 07/31/19 Fixed MISRA-C violations.
* 4.2   kal 03/12/20 Authenticate SizeofImgHdr before use, in case of failure
*                    return XSECURE_IMAGE_HEADER_SIZE_ERR.
*                    Added support to hold Aes engine in reset after secure
*                    image processing.
*       vns 03/24/20 Cleared locally copied user key
*                    Corrected IV calculation
*       ana 04/03/20 Removed the support of storing key in global array
*       rpo 04/09/20 Aligned the buffers used by DMA to 64 bytes
*       kpt 04/10/20 Resolved coverity warnings
*       ana 04/24/20 Removed support of boot header RSA with 0x1 and 0x2
* 4.6   am  09/17/21 Resolved compiler warnings
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xsecure.h"
#include "xparameters.h"
#include "xil_util.h"

static XSecure_Aes SecureAes;
static XSecure_Rsa Secure_Rsa;
static XSecure_Sha3 Sha3Instance;

static XCsuDma CsuDma = {0U};
#if defined (__GNUC__)
#if defined (PSU_PMU)
u8 EfusePpk[XSECURE_PPK_SIZE]__attribute__ ((aligned (32)));
			/**< eFUSE verified PPK */
u8 AcBuf[XSECURE_AUTH_CERT_MIN_SIZE]__attribute__ ((aligned (32)));
			/**< Buffer to store authentication certificate */
u8 Buffer[XSECURE_BUFFER_SIZE] __attribute__ ((aligned (32)));
			/**< Buffer to store */
#else	/*!PSU_PMU*/
u8 EfusePpk[XSECURE_PPK_SIZE]__attribute__ ((aligned (64)));
			/**< eFUSE verified PPK */
u8 AcBuf[XSECURE_AUTH_CERT_MIN_SIZE]__attribute__ ((aligned (64)));
			/**< Buffer to store authentication certificate */
u8 Buffer[XSECURE_BUFFER_SIZE] __attribute__ ((aligned (64)));
			/**< Buffer to store */
#endif

#elif defined (__ICCARM__)
#if defined (PSU_PMU)
#pragma data_alignment = 32
u8 EfusePpk[XSECURE_PPK_SIZE];
			/**< eFUSE verified PPK */
#pragma data_alignment = 32
u8 AcBuf[XSECURE_AUTH_CERT_MIN_SIZE];
			/**< Buffer to store authentication certificate */
#pragma data_alignment = 32
u8 Buffer[XSECURE_BUFFER_SIZE];
			/**< Buffer to store */
#else	/*!PSU_PMU*/
#pragma data_alignment = 64
u8 EfusePpk[XSECURE_PPK_SIZE];
			/**< eFUSE verified PPK */
#pragma data_alignment = 64
u8 AcBuf[XSECURE_AUTH_CERT_MIN_SIZE];
			/**< Buffer to store authentication certificate */
#pragma data_alignment = 64
u8 Buffer[XSECURE_BUFFER_SIZE];
			/**< Buffer to store */
#endif
#endif
u32 XsecureIv[XSECURE_IV_LEN];

/**************************** Type Definitions *******************************/
typedef struct {
	XCsuDma *CsuDmaInstPtr;
	u8 *Data;
	u32 Size;
	u8 *AuthCertPtr;
	u32 SignatureOffset;
	XSecure_Sha3PadType PaddingType;
	u8 AuthIncludingCert;
} XSecure_AuthParam;

/************************** Function Prototypes ******************************/
static u32 XSecure_BhdrValidation(XSecure_ImageInfo *ImageInfo);
static u32 XSecure_DecryptPartition(XSecure_ImageInfo *ImageHdrInfo,
			u8 *SrcAddr, u8 *KupKey, XSecure_DataAddr *Addr);
static inline u32 XSecure_DataAuthentication(XSecure_AuthParam *AuthParam);

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * This function is used to initialize the DMA driver
 *
 * @param	None
 *
 * @return	returns the error code on any error
 *		returns XST_SUCCESS on success
 *
 *****************************************************************************/
u32 XSecure_CsuDmaInit(void)
{
	u32 Status = (u32)XST_FAILURE;
	s32 SStatus;
	XCsuDma_Config * CsuDmaConfig;

	CsuDmaConfig = XCsuDma_LookupConfig(XSECURE_CSUDMA_DEVICEID);
	if (NULL == CsuDmaConfig) {
		Status = XSECURE_ERROR_CSUDMA_INIT_FAIL;
		goto END;
	}

	SStatus = XCsuDma_CfgInitialize(&CsuDma, CsuDmaConfig,
			CsuDmaConfig->BaseAddress);
	if (SStatus != XST_SUCCESS) {
		Status = XSECURE_ERROR_CSUDMA_INIT_FAIL;
		goto END;
	}
	Status = (u32)XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to initialize CsuDma and return CsuDma pointer
 *
 * @param	None
 *
 *****************************************************************************/
XCsuDma* Xsecure_GetCsuDma(void)
{
	u32 Status = (u32)XST_FAILURE;

	Status = XSecure_CsuDmaInit();
	if (Status != (u32)XST_SUCCESS) {
		return NULL;
	}
	return &CsuDma;
}


/****************************************************************************/
 /**
 * This function access the xilsecure SHA3 hardware based on the flags provided
 * to calculate the SHA3 hash.
 *
 * @param	SrcAddrHigh  Higher 32-bit of the input or output address.
 * @param	SrcAddrLow   Lower 32-bit of the input or output address.
 * @param	SrcSize      Size of the data on which hash should be
 *		calculated.
 * @param	Flags: inputs for the operation requested
 *		BIT(0) - for initializing csudma driver and SHA3,
 *		(Here address and size inputs can be NULL)
 *		BIT(1) - To call Sha3_Update API which can be called multiple
 *		times when data is not contiguous.
 *		BIT(2) - to get final hash of the whole updated data.
 *		Hash will be overwritten at provided address with 48 bytes.
 *
 * @return	Returns Status XST_SUCCESS on success and error code on failure.
 *
 *****************************************************************************/
u32 XSecure_Sha3Hash(u32 SrcAddrHigh, u32 SrcAddrLow, u32 SrcSize, u32 Flags)
{
	u32 Status = XST_SUCCESS;
	u64 SrcAddr = ((u64)SrcAddrHigh << 32) | (u64)SrcAddrLow;

	switch (Flags & XSECURE_SHA3_MASK) {
	case XSECURE_SHA3_INIT:
		Status = XSecure_CsuDmaInit();
		if (Status != (u32)XST_SUCCESS) {
			Status = XSECURE_ERROR_CSUDMA_INIT_FAIL;
			goto END;
		}

		Status = (u32)XSecure_Sha3Initialize(&Sha3Instance, &CsuDma);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
		XSecure_Sha3Start(&Sha3Instance);
		break;
	case XSECURE_SHA3HASH_UPDATE:
		Status = XSecure_Sha3Update(&Sha3Instance, (u8 *)(UINTPTR)SrcAddr,
						SrcSize);
		if (Status != (u32)XST_SUCCESS) {
			Status = XSECURE_SHA3_UPDATE_FAIL;
			goto END;
		}
		break;
	case XSECURE_SHA3_FINAL:
		Status = XSecure_Sha3Finish(&Sha3Instance,
						(u8 *)(UINTPTR)SrcAddr);
		break;
	default:
		Status = XSECURE_INVALID_FLAG;
		break;
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * This is the function to RSA decrypt or encrypt the provided data and load back
 * to memory
 *
 * @param	SrcAddrHigh	Higher 32-bit Linear memory space from where
 *		CSUDMA will read the data
 * @param	SrcAddrLow	Lower 32-bit Linear memory space from where
 *		CSUDMA will read the data
 * @param	WrSize	Number of bytes to be encrypted or decrypted.
 * @param	Flags:
 *		BIT(0) - Encryption/Decryption
 *			 0 - Rsa Private key Decryption
 *			 1 - Rsa Public key Encryption
 *
 * @return	Returns Status XST_SUCCESS on success and error code on failure.
 *
 * @note	Data to be encrypted/Decrypted + Modulus + Exponent
 *		Modulus and Data should always be key size
 *		Exponent : private key's exponent is key size while decrypting
 *		the signature and 32 bit for public key for encrypting the
 *		signature
 *		In this API we are not taking exponentiation value.
 *
 *****************************************************************************/
u32 XSecure_RsaCore(u32 SrcAddrHigh, u32 SrcAddrLow, u32 SrcSize,
				u32 Flags)
{
	u32 Status = (u32)XST_FAILURE;
	u64 WrAddr = ((u64)SrcAddrHigh << 32) | (u64)SrcAddrLow;
	u8 *Modulus;
	u8 *Exponent;

	Xil_AssertNonvoid(SrcSize != 0x0U);

	Modulus = (u8 *)(UINTPTR)(WrAddr + SrcSize);
	Exponent = (u8 *)(UINTPTR)(WrAddr + SrcSize + SrcSize);

	Status = (u32)XSecure_RsaInitialize(&Secure_Rsa, Modulus, NULL, Exponent);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}
	if ((Flags & XSECURE_RSA_CORE_OPERATION) == (u32)XSECURE_DEC) {
		Status = (u32)XSecure_RsaPrivateDecrypt(&Secure_Rsa,
			(u8 *)(UINTPTR)WrAddr, SrcSize, (u8 *)(UINTPTR)WrAddr);
	}
	else {
		Status = (u32)XSecure_RsaPublicEncrypt(&Secure_Rsa,
			(u8 *)(UINTPTR)WrAddr, SrcSize, (u8 *)(UINTPTR)WrAddr);
	}

END:
	return Status;
}

/*****************************************************************************/
/*
 * @brief
 * This function initializes the AES-GCM engine with Key and Iv
 *
 * @param	AddrHigh	Higher 32 bit address of the XSecure_AesParams
 * 				structure.
 * @param	AddrLow		Lower 32 bit address of the XSecure_AesParams
 * 				structure.
 *
 * @return	Returns Status
 * 		- XST_SUCCESS on success
 * 		- Error code on failure
 *
 *****************************************************************************/
static u32 XSecure_InitAes(u32 AddrHigh, u32 AddrLow)
{
	u64 WrAddr = ((u64)AddrHigh << 32) | (u64)AddrLow;
	XSecure_AesParams *AesParams = (XSecure_AesParams *)(UINTPTR)WrAddr;
	u32 Status = (u32)XST_FAILURE;
	u8 Index;
	u64 IvAddr;
	u64 KeyAddr;
	u32 *IvPtr;
	u32 *KeyPtr;


	IvAddr = AesParams->Iv;
	KeyAddr = AesParams->Key;

	IvPtr = (u32 *)(UINTPTR)IvAddr;
	KeyPtr =(u32 *)(UINTPTR)KeyAddr;

	if(IvPtr == NULL){
		Status = (u32)XST_FAILURE;
		goto END;
	}
	if ((AesParams->KeySrc == XSECURE_AES_KUP_KEY) && (KeyPtr == NULL)) {
		Status = (u32)XST_FAILURE;
		goto END;
	}
	Status = XSecure_CsuDmaInit();
	if (Status != (u32)XST_SUCCESS) {
		Status = XSECURE_ERROR_CSUDMA_INIT_FAIL;
		goto END;
	}
	for (Index = 0U; Index < XSECURE_IV_LEN; Index++) {
		XsecureIv[Index] = *IvPtr;
		IvPtr++;
	}
	/* Initialize the Aes driver so that it's ready to use */
	if (AesParams->KeySrc == XSECURE_AES_KUP_KEY) {
		Status = (u32)XSecure_AesInitialize(&SecureAes, &CsuDma,
				XSECURE_CSU_AES_KEY_SRC_KUP,
				XsecureIv, KeyPtr);
	}
	else {
		Status = (u32)XSecure_AesInitialize(&SecureAes, &CsuDma,
				XSECURE_CSU_AES_KEY_SRC_DEV,
				XsecureIv, NULL);
	}

END:
	return Status;

}

/*****************************************************************************/
/**
 * @brief
 * This function performs the AES decryption of the data-blob.
 *
 * @param	AddrHigh	Higher 32 bit address of the XSecure_AesParams
 * 				structure.
 * @param	AddrLow		Lower 32 bit address of the XSecure_AesParams
 * 				structure.
 *
 * @return	Returns Status
 *		- XST_SUCCESS on success
 *		- Error code on failure
 *
 *****************************************************************************/
static u32 XSecure_DecryptData(u32 AddrHigh, u32 AddrLow)
{
	u64 WrAddr = ((u64)AddrHigh << 32) | (u64)AddrLow;
	XSecure_AesParams *AesParams = (XSecure_AesParams *)(UINTPTR)WrAddr;
	u32 Status = (u32)XST_FAILURE;
	u32 KeyClearStatus;
	u64 SrcAddr;
	u64 DstAddr;
	u64 GcmTagAddr;

	Status = XSecure_InitAes(AddrHigh,AddrLow);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}
	SrcAddr = AesParams->Src;
	DstAddr = AesParams->Dst;

	GcmTagAddr = (u64)(UINTPTR)((u32 *)(UINTPTR)SrcAddr +
			(AesParams->Size/4U));

	Status = (u32)XSecure_AesDecryptData(&SecureAes,
			(u8 *)(UINTPTR)DstAddr,
			(u8 *)(UINTPTR)SrcAddr,
			(u32)AesParams->Size,
			(u8 *)(UINTPTR)GcmTagAddr);

	KeyClearStatus = XSecure_AesKeyZero(&SecureAes);
	if (KeyClearStatus != (u32)XST_SUCCESS) {
		Status = KeyClearStatus;
		goto END;
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function performs the AES encryption of the data-blob.
 *
 * @param	AddrHigh	Higher 32 bit address of the XSecure_AesParams
 * 				structure.
 * @param	AddrLow		Lower 32 bit address of the XSecure_AesParams
 * 				structure.
 *
 * @return	Returns Status
 * 		- XST_SUCCESS on success
 * 		- Error code on failure
 *
 *****************************************************************************/
static u32 XSecure_EncryptData(u32 AddrHigh, u32 AddrLow)
{
	u64 WrAddr = ((u64)AddrHigh << 32) | (u64)AddrLow;
	XSecure_AesParams *AesParams = (XSecure_AesParams *)(UINTPTR)WrAddr;
	u32 Status = (u32)XST_FAILURE;
	u32 KeyClearStatus;
	u64 SrcAddr;
	u64 DstAddr;

	Status = XSecure_InitAes(AddrHigh,AddrLow);
	if (Status != (u32)XST_SUCCESS) {
		 goto END;
	}
	SrcAddr = AesParams->Src;
	DstAddr = AesParams->Dst;

	Status = XSecure_AesEncryptData(&SecureAes,
			(u8 *)(UINTPTR)DstAddr,
			(u8 *)(UINTPTR)SrcAddr,
			(u32)AesParams->Size);

	KeyClearStatus = XSecure_AesKeyZero(&SecureAes);
        if (KeyClearStatus != (u32)XST_SUCCESS) {
                Status =  KeyClearStatus;
		goto END;
        }
END:
	return Status;
}

/*****************************************************************************/
/*
 * @brief
 * This function checks for flags field of the XSecure_AesParams struct
 * and resolves weather the request is encryption or decryption.
 *
 * @param	AddrHigh	Higher 32 bit address of the XSecure_AesParams
 * 				structure.
 * @param	AddrLow		Lower 32 bit address of the XSecure_AesParams
 * 				structure.
 *
 * @return      Returns Status
 * 		- XST_SUCCESS on success
 * 		- Error code on failure
 *
 ******************************************************************************/
u32 XSecure_AesOperation(u32 AddrHigh, u32 AddrLow)
{
	u64 WrAddr = ((u64)AddrHigh << 32) | (u64)AddrLow;
	XSecure_AesParams *AesParams = (XSecure_AesParams *)(UINTPTR)WrAddr;
	u32 Status = (u32)XST_FAILURE;
	u32 Flags = (u32)(AesParams->AesOp);

#ifndef XSECURE_TRUSTED_ENVIRONMENT
	if (AesParams->KeySrc != XSECURE_AES_KUP_KEY) {
		Status = XSECURE_DEC_WRONG_KEY_SOURCE;
		goto END;
	}
#endif
	if ((AesParams->Size % XSECURE_WORD_LEN) != 0x00U) {
		Status =  XSECURE_SIZE_ERR;
		goto END;
	}
	switch (Flags) {
		case XSECURE_DEC:
			Status = XSecure_DecryptData(AddrHigh, AddrLow);
			break;
		case XSECURE_ENC:
			Status = XSecure_EncryptData(AddrHigh, AddrLow);
			break;
		default:
			Status = XSECURE_INVALID_FLAG;
			break;
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function performs authentication of data by encrypting the signature
 * with provided key and compares with hash of the data and returns success or
 * failure.
 *
 * @param	Signature	Pointer to the RSA signature of the data
 * @param	Key		Pointer to XSecure_RsaKey structure.
 * @param	Hash		Pointer to the hash of the data to be
 *		authenticated.
 *
 * @return	Returns Status
 * 		- XST_SUCCESS on success
 * 		- Error code on failure
 *
 *****************************************************************************/
u32 XSecure_DataAuth(u8 *Signature, XSecure_RsaKey *KeyInst, u8 *Hash)
{

	u32 Status = (u32)XST_FAILURE;
	XSecure_Rsa SecureRsaInst = {0U};
	u8 EncSignature[XSECURE_MOD_LEN]  = {0U};

	/* Assert conditions */
	Xil_AssertNonvoid(Signature != NULL);
	Xil_AssertNonvoid(KeyInst != NULL);
	Xil_AssertNonvoid((KeyInst->Modulus != NULL) && (KeyInst->Exponent != NULL));
	Xil_AssertNonvoid(Hash != NULL);

	/* Initialize RSA instance */
	Status = (u32)XSecure_RsaInitialize(&SecureRsaInst,
			KeyInst->Modulus, KeyInst->Exponentiation, KeyInst->Exponent);
	if (Status != (u32)XST_SUCCESS) {
		Status = XSECURE_RSA_INIT_ERR;
		goto END;
	}

	/* Encrypt signature with RSA key */
	Status = (u32)XSecure_RsaPublicEncrypt(&SecureRsaInst, Signature,
			XSECURE_MOD_LEN, EncSignature);
	if (Status != (u32)XST_SUCCESS) {
		Status = XSECURE_RSA_ENCRYPT_ERR;
		goto END;
	}


	/* Compare encrypted signature with sha3 hash calculated on data */
	Status = (u32)XSecure_RsaSignVerification(EncSignature, Hash,
					XSECURE_HASH_TYPE_SHA3);
	if (Status != (u32)XST_SUCCESS) {
		Status = XSECURE_VERIFY_ERR;
		goto END;
	}

END:
	if (Status != (u32)XST_SUCCESS) {
		Status = Status | (u32)XSECURE_AUTH_FAILURE;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function performs authentication of a partition of an image.
 *
 * @param	CsuDmaInstPtr	Pointer to the CSU DMA instance.
 * @param	Data		Pointer to partition to be authenticated.
 * @param	Size		Represents the size of the partition.
 * @param	AuthCertPtr	Pointer to authentication certificate of the
 *		partition.
 *
 * @return	Returns Status
 *		- XST_SUCCESS on success
 *		- Error code on failure
 *
 *****************************************************************************/
u32 XSecure_PartitionAuthentication(XCsuDma *CsuDmaInstPtr, u8 *Data,
				u32 Size, u8 *AuthCertPtr)
{
	u32 Status = (u32)XST_FAILURE;
	XSecure_AuthParam AuthParam;

	/* Assert conditions */
	Xil_AssertNonvoid(CsuDmaInstPtr != NULL);
	Xil_AssertNonvoid(Data != NULL);
	Xil_AssertNonvoid(Size != 0x00U);
	Xil_AssertNonvoid(AuthCertPtr != NULL);

	AuthParam.CsuDmaInstPtr = CsuDmaInstPtr;
	AuthParam.Data = Data;
	AuthParam.Size = Size;
	AuthParam.AuthCertPtr = AuthCertPtr;
	AuthParam.SignatureOffset = XSECURE_AUTH_CERT_PARTSIG_OFFSET;
	AuthParam.PaddingType = XSECURE_CSU_NIST_SHA3;
	AuthParam.AuthIncludingCert = 1U;

	Status = XSecure_DataAuthentication(&AuthParam);
	if (Status != (u32)XST_SUCCESS) {
		Status = Status | (u32)XSECURE_AUTH_FAILURE;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function performs authentication of a boot header of the image.
 *
 * @param	CsuDmaInstPtr	Pointer to the CSU DMA instance.
 * @param	Data		pointer to boot header of the image.
 * @param	Size 		Size of the boot header.
 * @param	AuthCertPtr	Pointer to authentication certificate.
 *
 * @return	Returns Status
 * 		- XST_SUCCESS on success
 * 		- XST_FAILURE on failure
 *
 *****************************************************************************/
static inline u32 XSecure_BhdrAuthentication(XCsuDma *CsuDmaInstPtr,
				u8 *Data, u32 Size, u8 *AuthCertPtr)
{

	u32 Status = (u32)XST_FAILURE;
	XSecure_AuthParam AuthParam;

	/* Assert conditions */
	Xil_AssertNonvoid(CsuDmaInstPtr != NULL);
	Xil_AssertNonvoid(Data != NULL);
	Xil_AssertNonvoid(Size != 0x00U);
	Xil_AssertNonvoid(AuthCertPtr != NULL);

	AuthParam.CsuDmaInstPtr = CsuDmaInstPtr;
	AuthParam.Data = Data;
	AuthParam.Size = Size;
	AuthParam.AuthCertPtr = AuthCertPtr;
	AuthParam.SignatureOffset = XSECURE_AUTH_CERT_BHDRSIG_OFFSET;
	AuthParam.PaddingType = XSECURE_CSU_KECCAK_SHA3;
	AuthParam.AuthIncludingCert = 0U;

	Status = XSecure_DataAuthentication(&AuthParam);
	if (Status != (u32)XST_SUCCESS) {
		Status = XSECURE_BOOT_HDR_FAIL | Status;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function copies the data from specified source to destination
 * using CSU DMA.
 *
 * @param	DestPtr		Pointer to the destination address.
 * @param	SrcPtr		Pointer to the source address.
 * @param	Size 		Data size to be copied.
 *
 * @return	Returns Status
 * 		- XST_SUCCESS on success
 * 		- XST_FAILURE on failure
 *
 *****************************************************************************/
u32 XSecure_MemCopy(void * DestPtr, void * SrcPtr, u32 Size)
{
	u32 Status = (u32)XST_SUCCESS;

	XSecure_Sss SssInstance;

	Xil_AssertNonvoid(Size != 0x0U);

	XSecure_SssInitialize(&SssInstance);
	Status = XSecure_SssDmaLoopBack(&SssInstance, CsuDma.Config.DeviceId);
	if(Status != (u32)XST_SUCCESS){
		goto ENDF;
	}
	/* Data transfer in loop back mode */
	XCsuDma_Transfer(&CsuDma, XCSUDMA_DST_CHANNEL,
				(UINTPTR)DestPtr, Size, 1);
	XCsuDma_Transfer(&CsuDma, XCSUDMA_SRC_CHANNEL,
				(UINTPTR)SrcPtr, Size, 1);

	/* Polling for transfer to be done */
	Status = XCsuDma_WaitForDoneTimeout(&CsuDma, XCSUDMA_DST_CHANNEL);
	if(Status != (u32)XST_SUCCESS) {
		goto ENDF;
	}

	/* To acknowledge the transfer has completed */
	XCsuDma_IntrClear(&CsuDma, XCSUDMA_SRC_CHANNEL, XCSUDMA_IXR_DONE_MASK);
	XCsuDma_IntrClear(&CsuDma, XCSUDMA_DST_CHANNEL, XCSUDMA_IXR_DONE_MASK);

ENDF:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function verifies SPK by authenticating SPK with PPK, also checks either
 * SPK is revoked or not.if it is not boot header authentication.
 *
 * @param	AcPtr		Pointer to the authentication certificate.
 * @param	EfuseRsaenable	Input variable which holds
 *		efuse RSA authentication or boot header authentication.
 *		0 - Boot header authentication
 *		1 - RSA authentication
 *
 * @return	Returns Status
 * 		- XST_SUCCESS on success
 * 		- Error code on failure
 *
 *****************************************************************************/
u32 XSecure_VerifySpk(u8 *AcPtr, u32 EfuseRsaenable)
{
	u32 Status = XST_FAILURE;
	u32 IsRsaenabled = XSecure_IsRsaEnabled();

	/* Check the cache value with the value passed */
	if (IsRsaenabled != EfuseRsaenable) {
		goto END;
	}
	if (IsRsaenabled != 0x00U) {
		/* Verify SPK with verified PPK */
		Status = XSecure_SpkAuthentication(&CsuDma, AcPtr, (u8 *)EfusePpk);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
		/* SPK revocation */
		Status = XSecure_SpkRevokeCheck(AcPtr);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
	} else {
		/* Verify SPK with PPK in authentication certificate */
		Status = XSecure_SpkAuthentication(&CsuDma, AcPtr, NULL);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
	}

END:

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function authenticates the single partition image's boot header and
 * image header, also copies all the required details to the ImageInfo pointer.
 *
 * @param	StartAddr	Pointer to the single partition image.
 * @param	ImageInfo	Pointer to XSecure_ImageInfo structure.
 *
 * @return	Returns Status
 * 		- XST_SUCCESS on success
 * 		- Error code on failure
 *		- XSECURE_AUTH_NOT_ENABLED - represents image is not
 *		authenticated.
 *
 * @note	Copies the header and authentication certificate to internal
 *		buffer.
 *
 *****************************************************************************/
u32 XSecure_AuthenticationHeaders(u8 *StartAddr, XSecure_ImageInfo *ImageInfo)
{
	u32 Status = (u32)XST_FAILURE;
	u32 ImgHdrToffset;
	u32 AuthCertOffset;
	u32 SizeofImgHdr;
	u32 PhOffset;
	u8 NoAuth = 0U;
	u8 IsEncrypted = 0U;
	u8 EncOnly = 0U;
	XSecure_PartitionHeader *Ph;
	u8 *IvPtr = (u8 *)(ImageInfo->Iv + (XSECURE_IV_LEN - 1));
	u8 BootgenBinFormat[] = {0x66, 0x55, 0x99, 0xAA,
				 0x58, 0x4E, 0x4C, 0x58};/* Sync Word */

	/* validate the  image */
	if((memcmp((StartAddr + XSECURE_IMAGE_SYNC_WORD_OFFSET),
				BootgenBinFormat,
				XSECURE_ARRAY_LENGTH(BootgenBinFormat))) != 0){
		Status = XSECURE_INVALID_IMAGE_ERROR;
		goto END;
	}

	/* Pointer IV available */
	ImageInfo->Iv = XsecureIv;

	(void)memset(Buffer, 0, XSECURE_BUFFER_SIZE);
	(void)memset(AcBuf, 0, XSECURE_AUTH_CERT_MIN_SIZE);

	ImageInfo->EfuseRsaenable = XSecure_IsRsaEnabled();

	/* Copy boot header to internal buffer */
	(void)XSecure_MemCopy((void *)(UINTPTR)Buffer, StartAddr,
		XSECURE_BOOT_HDR_MAX_SIZE/XSECURE_WORD_LEN);

	/* Know image header's authentication certificate */
	ImgHdrToffset = Xil_In32((UINTPTR)Buffer + XSECURE_IMAGE_HDR_OFFSET);
	AuthCertOffset = Xil_In32((UINTPTR)(StartAddr +
			ImgHdrToffset + XSECURE_AC_IMAGE_HDR_OFFSET)) *
					XSECURE_WORD_LEN;
	if (AuthCertOffset == 0x00U) {
		if (ImageInfo->EfuseRsaenable != 0x00U) {
			Status = XSECURE_AUTH_ISCOMPULSORY;
			goto END;
		}
		Status = XSECURE_AUTH_NOT_ENABLED;
		NoAuth = 1U;
		goto UPDATE;
	}

	/* Copy Image header authentication certificate to local memory */
	(void)XSecure_MemCopy((void *)(UINTPTR)AcBuf, (u8 *)(StartAddr + AuthCertOffset),
			XSECURE_AUTH_CERT_MIN_SIZE/XSECURE_WORD_LEN);

	Status = XSecure_BhdrValidation(ImageInfo);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/* Copy boot header parameters */
	ImageInfo->KeySrc =
		Xil_In32((UINTPTR)(Buffer + XSECURE_KEY_SOURCE_OFFSET));
	/* Copy secure header IV */
	if (ImageInfo->KeySrc != 0x00U) {
		(void)XSecure_MemCopy(ImageInfo->Iv, (Buffer + XSECURE_IV_OFFSET),
						XSECURE_IV_LEN);
	}

	/* Image header Authentication */

	/* Copy image header to internal memory */
	SizeofImgHdr = AuthCertOffset - ImgHdrToffset;

	if (SizeofImgHdr > sizeof(Buffer)) {
		Status = XSECURE_IMAGE_HEADER_SIZE_ERR;
		goto END;
	}
	(void)XSecure_MemCopy((void *)(UINTPTR)Buffer, (u8 *)(StartAddr + ImgHdrToffset),
					SizeofImgHdr/XSECURE_WORD_LEN);

	/* Authenticate image header */
	Status = XSecure_PartitionAuthentication(&CsuDma,
			(u8 *)(UINTPTR)Buffer, SizeofImgHdr, (u8 *)(UINTPTR)AcBuf);
	if (Status != (u32)XST_SUCCESS) {
		Status = Status | (u32)XSECURE_IMG_HDR_FAIL;
		goto END;
	}
	 /*
	  * After image header authentication just making sure we
	  * have used proper AC for authenticating
	  */
	if ((Xil_In32((UINTPTR)(Buffer +
		XSECURE_AC_IMAGE_HDR_OFFSET)) * XSECURE_WORD_LEN) !=
						AuthCertOffset) {
		Status = XSECURE_IMG_HDR_FAIL;
		goto END;
	}

UPDATE:
	if (NoAuth != 0U) {
		Ph = (XSecure_PartitionHeader *)((UINTPTR)
				(StartAddr + Xil_In32((UINTPTR)Buffer +
						XSECURE_PH_TABLE_OFFSET)));
		/* Copy boot header parameters */
		ImageInfo->KeySrc = Xil_In32((UINTPTR)(Buffer +
					XSECURE_KEY_SOURCE_OFFSET));
		/* Copy secure header IV */
		if (ImageInfo->KeySrc != 0x00U) {
			(void)XSecure_MemCopy(ImageInfo->Iv,
						(Buffer + XSECURE_IV_OFFSET),
						XSECURE_IV_LEN);
		}
	} else {
		/* Partition header */
		PhOffset = Xil_In32((UINTPTR)(Buffer + XSECURE_PH_OFFSET));
		/* Partition header offset is w.r.t to image start address */
		Ph = (XSecure_PartitionHeader *)((UINTPTR)
				(Buffer + (u32)((PhOffset * (u32)XSECURE_WORD_LEN)
				- ImgHdrToffset)));
	}

	ImageInfo->PartitionHdr = Ph;

	if ((ImageInfo->PartitionHdr->PartitionAttributes &
		XSECURE_PH_ATTR_ENC_ENABLE) != 0U) {
		IsEncrypted = 1U;
	}

	EncOnly = (u8)XSecure_IsEncOnlyEnabled();
	if ((EncOnly != 0U) && (IsEncrypted == 0U)) {
		Status = XSECURE_ENC_ISCOMPULSORY;
		goto END;
	}

	/*
	 * When authentication exists and requesting
	 * for device key other than eFUSE and KUP key
	 * when ENC_ONLY bit is blown
	 */

	if (EncOnly != 0U) {
		if ((ImageInfo->KeySrc == XSECURE_KEY_SRC_BBRAM) ||
			(ImageInfo->KeySrc == XSECURE_KEY_SRC_GREY_BH) ||
			(ImageInfo->KeySrc == XSECURE_KEY_SRC_BLACK_BH)) {
			Status = XSECURE_DEC_WRONG_KEY_SOURCE;
			goto END;
		}
	 }

	if (Ph->NextPartitionOffset != 0x00U) {
		Status = XSECURE_IMAGE_WITH_MUL_PARTITIONS;
		goto END;
	}

	/* Add partition header IV to boot header IV */
	 if (ImageInfo->KeySrc != 0x00U) {
		 *(IvPtr + XSECURE_IV_LEN) =
			 (*(IvPtr + XSECURE_IV_LEN)) +
			 (ImageInfo->PartitionHdr->Iv & XSECURE_PH_IV_MASK);
	}

END:

	return Status;

}

/*****************************************************************************/
/**
 * @brief
 * This function process the secure image of single partition created by
 * bootgen.
 *
 * @param	AddrHigh	Higher 32-bit linear memory space of single
 *		partition non-bitstream image.
 * @param	AddrLow		Lower 32-bit linear memory space of single
 *		partition non-bitstream image.
 * @param	KupAddrHigh	Higher 32-bit linear memory space of KUP key.
 * @param	KupAddrLow	Ligher 32-bit linear memory space of KUP key.
 * @param	Addr		Location of the secure image loaded after validation.
 *
 * @return	Returns Status
 * 		- XST_SUCCESS on success
 * 		- Error code on failure
 *
 *****************************************************************************/
u32 XSecure_SecureImage(u32 AddrHigh, u32 AddrLow,
		u32 KupAddrHigh, u32 KupAddrLow, XSecure_DataAddr *Addr)
{
	u8* SrcAddr = (u8 *)(UINTPTR)(((u64)AddrHigh << XSECURE_WORD_SHIFT) |
								AddrLow);
	u32 Status = (u32)XST_FAILURE;
	XSecure_ImageInfo ImageHdrInfo = {0};
	u8 *KupKey = (u8 *)(UINTPTR)(((u64)KupAddrHigh << XSECURE_WORD_SHIFT) |
							KupAddrLow);
	u8 NoAuth = 0;
	u32 EncOnly;
	u8 IsEncrypted;
	u8 *IvPtr = (u8 *)(UINTPTR)&XsecureIv[XSECURE_IV_LEN - 1];
	u32 Offset;

	/* Address provided is null */
	if (SrcAddr == NULL) {
		Status = (u32)XST_FAILURE;
		goto END;
	}
	/* Initialize CSU DMA driver */
	Status = XSecure_CsuDmaInit();
	if (Status != (u32)XST_SUCCESS) {
		Status = XSECURE_ERROR_CSUDMA_INIT_FAIL;
		goto END;
	}
	/* Headers authentication */
	Status = XSecure_AuthenticationHeaders(SrcAddr, &ImageHdrInfo);
	if (Status != (u32)XST_SUCCESS) {
	/* Error other than XSECURE_AUTH_NOT_ENABLED error will be an error */
		if (Status != XSECURE_AUTH_NOT_ENABLED) {
			goto END;
		}
		else {
			/* Here Buffer still contains Boot header */
			NoAuth = 1;
		}
	}
	else {
		/*
		 * In case of partition is not authenticated and
		 * headers are only authenticated
		 */
		Addr->AddrHigh = (u32)((((u64)(UINTPTR)SrcAddr) +
				(ImageHdrInfo.PartitionHdr->DataWordOffset *
						XSECURE_WORD_LEN)) >> 32);
		Addr->AddrLow = (u32)((UINTPTR)SrcAddr +
					(ImageHdrInfo.PartitionHdr->DataWordOffset *
							XSECURE_WORD_LEN));
	}

	/* If authentication is enabled and authentication of partition is enabled */
	if ((NoAuth == 0U) &&
		((ImageHdrInfo.PartitionHdr->PartitionAttributes &
			XSECURE_PH_ATTR_AUTH_ENABLE) != 0x00U)) {
		/* Copy authentication certificate to internal memory */
		(void)XSecure_MemCopy((void *)(UINTPTR)AcBuf, (u8 *)(SrcAddr +
			(ImageHdrInfo.PartitionHdr->AuthCertificateOffset *
					XSECURE_WORD_LEN)),
			XSECURE_AUTH_CERT_MIN_SIZE/XSECURE_WORD_LEN);

		Status = XSecure_VerifySpk((u8 *)(UINTPTR)AcBuf, ImageHdrInfo.EfuseRsaenable);
		if (Status != (u32)XST_SUCCESS) {
			Status = XSECURE_PARTITION_FAIL | Status;
			goto END;
		}
		Offset = ImageHdrInfo.PartitionHdr->DataWordOffset * XSECURE_WORD_LEN;
		/* Authenticate Partition */
		Status = XSecure_PartitionAuthentication(&CsuDma,
				(u8 *)(SrcAddr + Offset),
			(u32)((ImageHdrInfo.PartitionHdr->TotalDataWordLength *
					XSECURE_WORD_LEN) -
			XSECURE_AUTH_CERT_MIN_SIZE),
			(u8 *)((UINTPTR)AcBuf));
		if (Status != (u32)XST_SUCCESS) {
			Status = Status | (u32)XSECURE_PARTITION_FAIL;
			goto END;
		}
	}

	if (NoAuth != 0x0U) {
		XSecure_PartitionHeader *Ph =
			(XSecure_PartitionHeader *)((UINTPTR)
			(SrcAddr + Xil_In32((UINTPTR)Buffer +
					XSECURE_PH_TABLE_OFFSET)));
		ImageHdrInfo.PartitionHdr = Ph;
		if ((ImageHdrInfo.PartitionHdr->PartitionAttributes &
				XSECURE_PH_ATTR_AUTH_ENABLE) != 0x00U) {
			Status = XSECURE_HDR_NOAUTH_PART_AUTH;
			goto END;
		}
	}

	/* Decrypt the partition if encryption is enabled */
	EncOnly = XSecure_IsEncOnlyEnabled();
	IsEncrypted = (u8)(ImageHdrInfo.PartitionHdr->PartitionAttributes &
						XSECURE_PH_ATTR_ENC_ENABLE);
	if (IsEncrypted != 0x00U) {
		/* key selection */
		if (NoAuth != 0x00U) {
			ImageHdrInfo.KeySrc = Xil_In32((UINTPTR)Buffer +
						XSECURE_KEY_SOURCE_OFFSET);
#ifndef XSECURE_TRUSTED_ENVIRONMENT
			if (ImageHdrInfo.KeySrc != XSECURE_KEY_SRC_KUP) {
				Status = XSECURE_DEC_WRONG_KEY_SOURCE;
				goto END;
			}
#endif
			(void)XSecure_MemCopy(ImageHdrInfo.Iv,
				(Buffer + XSECURE_IV_OFFSET), XSECURE_IV_LEN);
			/* Add partition header IV to boot header IV */
			*(IvPtr + XSECURE_IV_LEN) =
				(*(IvPtr + XSECURE_IV_LEN)) +
				(ImageHdrInfo.PartitionHdr->Iv & XSECURE_PH_IV_MASK);
		}
		/*
		 * When authentication exists and requesting
		 * for device key other than eFUSE and KUP key
		 * when ENC_ONLY bit is blown
		 */
		if (EncOnly != 0x00U) {
			if ((ImageHdrInfo.KeySrc == XSECURE_KEY_SRC_BBRAM) ||
			(ImageHdrInfo.KeySrc == XSECURE_KEY_SRC_GREY_BH) ||
			(ImageHdrInfo.KeySrc == XSECURE_KEY_SRC_BLACK_BH)) {
				Status = XSECURE_DEC_WRONG_KEY_SOURCE;
				goto END;
			}
		}
	}
	else {
		/* when image is not encrypted */
		if (EncOnly != 0x00U) {
			Status = XSECURE_ENC_ISCOMPULSORY;
			goto END;
		}
		if (NoAuth != 0x00U) {
			Status = XSECURE_ISNOT_SECURE_IMAGE;
			goto END;
		}
		else {
			Status = (u32)XST_SUCCESS;
			goto END;
		}
	}

	Status = XSecure_DecryptPartition(&ImageHdrInfo, SrcAddr, KupKey, Addr);

END:
	/* Clear internal buffers */
	(void)memset(Buffer, 0, XSECURE_BUFFER_SIZE);
	(void)memset(AcBuf, 0, XSECURE_AUTH_CERT_MIN_SIZE);
	(void)memset(EfusePpk, 0, XSECURE_PPK_SIZE);

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function tells whether RSA authentication is enabled or not.
 *
 * @return	Returns Status
 * 		- XSECURE_ENABLED if RSA bit of efuse is programmed
 * 		- XSECURE_NOTENABLED if RSA bit of efuse is not programmed.
 *
 *****************************************************************************/
u32 XSecure_IsRsaEnabled(void)
{
	u32 Status = XSECURE_ENABLED;

	if ((Xil_In32(XSECURE_EFUSE_SEC_CTRL) &
			XSECURE_EFUSE_SEC_CTRL_RSA_ENABLE) != 0x00U) {

		Status = XSECURE_ENABLED;
		goto END;
	}
	Status = XSECURE_NOTENABLED;
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function tells whether encrypt only is enabled or not.
 *
 * @return	Returns Status
 * 		- XSECURE_ENABLED if enc_only bit of efuse is programmed
 * 		- XSECURE_NOTENABLED if enc_only bit of efuse is not programmed
 *
 *****************************************************************************/
u32 XSecure_IsEncOnlyEnabled(void)
{
	u32 Status = XSECURE_ENABLED;

	if ((Xil_In32(XSECURE_EFUSE_SEC_CTRL) &
			XSECURE_EFUSE_SEC_CTRL_ENC_ONLY) != 0x00U) {

		Status = XSECURE_ENABLED;
		goto END;
	}
	Status = XSECURE_NOTENABLED;
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function verifies the PPK hash with PPK programmed on efuse.
 *
 * @param	CsuDmaInstPtr	Pointer to CSU DMA instance.
 * @param	AuthCert	Pointer to authentication certificate.
 *
 * @return	Returns Status
 * 		- XST_SUCCESS on successful verification.
 * 		- Error code on failure.
 *
 *****************************************************************************/
u32 XSecure_PpkVerify(XCsuDma *CsuDmaInstPtr, u8 *AuthCert)
{
	u8 PpkSel = (*(u32 *)AuthCert & XSECURE_AH_ATTR_PPK_SEL_MASK) >>
					XSECURE_AH_ATTR_PPK_SEL_SHIFT;
	u32 Status = (u32)XST_FAILURE;
	u32 Hash[XSECURE_HASH_TYPE_SHA3/XSECURE_WORD_LEN] = {0U};
	XSecure_Sha3 Sha3Inst = {0U};
	u8 Index;
	u32 EfusePpkAddr;

	/* Check if PPK selection is correct */
	if (PpkSel > 1U) {
		Status = XSECURE_SEL_ERR;
		goto END;
	}

	/* Calculate PPK hash */
	Status = (u32)XSecure_Sha3Initialize(&Sha3Inst, CsuDmaInstPtr);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}
	Status = (u32)XSecure_Sha3PadSelection(&Sha3Inst,
				XSECURE_CSU_KECCAK_SHA3);
	if (Status != (u32)XST_SUCCESS) {
		Status = XSECURE_SHA3_PADSELECT_ERR;
		goto END;
	}
	Status = XSecure_Sha3Digest(&Sha3Inst, AuthCert + XSECURE_AC_PPK_OFFSET,
					XSECURE_KEY_SIZE, (u8 *)Hash);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/* Check selected is PPK revoked */
	if (PpkSel == 0x0U) {
		EfusePpkAddr = XSECURE_EFUSE_PPK0;
		if ((Xil_In32(XSECURE_EFUSE_SEC_CTRL) &
				XSECURE_EFUSE_SEC_CTRL_PPK0_REVOKE) != 0x00U) {
			Status = XSECURE_REVOKE_ERR;
			goto END;
		}
	}
	else {
		EfusePpkAddr = XSECURE_EFUSE_PPK1;
		if ((Xil_In32(XSECURE_EFUSE_SEC_CTRL) &
				XSECURE_EFUSE_SEC_CTRL_PPK1_REVOKE) != 0x00U) {
			Status = XSECURE_REVOKE_ERR;
			goto END;
		}
	}
	/* Verify PPK hash */
	for (Index = 0U;
		Index < (XSECURE_HASH_TYPE_SHA3 / XSECURE_WORD_LEN); Index++) {
		if (Hash[Index] != Xil_In32(
				EfusePpkAddr + ((u32)Index * (u32)XSECURE_WORD_LEN))) {
			Status = XSECURE_VERIFY_ERR;
			goto END;
		}
	}

END:
	if (Status != (u32)XST_SUCCESS) {
		Status = Status | (u32)XSECURE_PPK_ERR;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function authenticates SPK with provided PPK or PPK from authentication
 * certificate.
 *
 * @param	CsuDmaInstPtr	Pointer to CSU DMA instance.
 * @param	AuthCert	Pointer to authentication certificate.
 * @param	Ppk		Pointer to the PPK key.
 *		- If NULL uses PPK from provided authentication certificate.
 *
 * @return	Returns Status
 * 		- XST_SUCCESS on successful verification.
 * 		- Error code on failure.
 *
 *****************************************************************************/
u32 XSecure_SpkAuthentication(XCsuDma *CsuDmaInstPtr, u8 *AuthCert, u8 *Ppk)
{
	u8 SpkHash[XSECURE_HASH_TYPE_SHA3] = {0U};
	u8 SpkIdFuseSel = (*(u32 *)AuthCert & XSECURE_AH_ATTR_SPK_ID_FUSE_SEL_MASK) >>
                                        XSECURE_AH_ATTR_SPKID_FUSESEL_SHIFT;

	u8* PpkModular;
	u8* PpkModularEx;
	u8* PpkExpPtr;
	u32 Status = (u32)XST_FAILURE;
	u8 RsaSha3Array[512] = {0};
	u8 *PpkPtr;
	u8 *AcPtr = (u8 *)AuthCert;

	if (Ppk == NULL) {
		PpkPtr = (AcPtr + XSECURE_RSA_AC_ALIGN);
	}
	else {
		PpkPtr = Ppk;
	}

	/* Initialize sha3 */
	Status = (u32)XSecure_Sha3Initialize(&Sha3Instance, CsuDmaInstPtr);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}
	if (SpkIdFuseSel == XSECURE_SPKID_EFUSE) {
                (void)XSecure_Sha3PadSelection(&Sha3Instance, XSECURE_CSU_KECCAK_SHA3);
        }
        else if (SpkIdFuseSel != XSECURE_USER_EFUSE) {
		Status = XSECURE_INVALID_EFUSE_SELECT;
		goto END;
	}
	else {
		Status = (u32)XST_SUCCESS;
	}

	(void)XSecure_Sha3Start(&Sha3Instance);


	/* Hash the PPK + SPK choice */
	Status = XSecure_Sha3Update(&Sha3Instance, AcPtr, XSECURE_AUTH_HEADER_SIZE);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/* Set PPK pointer */
	PpkModular = (u8 *)PpkPtr;
	PpkPtr += XSECURE_PPK_MOD_SIZE;
	PpkModularEx = PpkPtr;
	PpkPtr += XSECURE_PPK_MOD_EXT_SIZE;
	PpkExpPtr = PpkPtr;
	AcPtr += ((u32)XSECURE_RSA_AC_ALIGN + (u32)XSECURE_PPK_SIZE);

	/* Calculate SPK + Auth header Hash */
	Status = XSecure_Sha3Update(&Sha3Instance, (u8 *)AcPtr, XSECURE_SPK_SIZE);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_Sha3Finish(&Sha3Instance, (u8 *)SpkHash);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/* Set SPK Signature pointer */
	AcPtr += XSECURE_SPK_SIZE;


	Status = (u32)XSecure_RsaInitialize(&Secure_Rsa, PpkModular,
			PpkModularEx, PpkExpPtr);
	if (Status != (u32)XST_SUCCESS) {
		Status = XSECURE_RSA_INIT_ERR;
		goto END;
	}

	/* Decrypt SPK Signature */
	Status = (u32)XSecure_RsaPublicEncrypt(&Secure_Rsa, AcPtr,
				XSECURE_SPK_SIG_SIZE, RsaSha3Array);
	if (Status != (u32)XST_SUCCESS) {
		Status = XSECURE_RSA_ENCRYPT_ERR;
		goto END;
	}

	/* Authenticate SPK Signature */
	Status = (u32)XSecure_RsaSignVerification(RsaSha3Array,
				SpkHash, XSECURE_HASH_TYPE_SHA3);
	if (Status != (u32)XST_SUCCESS) {
		Status = XSECURE_VERIFY_ERR;
		goto END;
	}


END:
	if (Status != (u32)XST_SUCCESS) {
		Status = Status | (u32)XSECURE_SPK_ERR;
	}

	return Status;

}

/*****************************************************************************/
/**
 * @brief
 * This function checks whether SPK is been revoked or not.
 *
 * @param	AuthCert	Pointer to authentication certificate.
 *
 * @return	Returns Status
 * 		- XST_SUCCESS on successful verification.
 * 		- Error code on failure.
 *
 *****************************************************************************/
u32 XSecure_SpkRevokeCheck(u8 *AuthCert)
{
	u32 Status = (u32)XST_FAILURE;
	u32 *SpkID = (u32 *)(UINTPTR)(AuthCert + XSECURE_AC_SPKID_OFFSET);
	u8 SpkIdFuseSel = (*(u32 *)AuthCert & XSECURE_AH_ATTR_SPK_ID_FUSE_SEL_MASK)
							>>	XSECURE_AH_ATTR_SPKID_FUSESEL_SHIFT;
	u32 UserFuseAddr;
	u32 UserFuseVal;

	/* If SPKID Efuse is selected , Verifies SPKID with Efuse SPKID*/
	if (SpkIdFuseSel == XSECURE_SPKID_EFUSE) {
		if (*SpkID != Xil_In32(XSECURE_EFUSE_SPKID)) {
			Status = (u32)XSECURE_SPK_ERR | (u32)XSECURE_REVOKE_ERR;
		} else {
			Status = (u32)XST_SUCCESS;
		}
	}
	/*
	 *	If User EFUSE is selected, checks the corresponding User-Efuse bit
	 *	programmed or not. If Programmed (indicates that key is revocated)
	 *	throws an error
	 */
	else if (SpkIdFuseSel == XSECURE_USER_EFUSE) {
		if ((*SpkID >= XSECURE_USER_EFUSE_MIN_VALUE) &&
			(*SpkID <= XSECURE_USER_EFUSE_MAX_VALUE)) {
			UserFuseAddr = XSECURE_USER_EFUSE_START_ADDR +
						(((*SpkID-1)/XSECURE_WORD_SHIFT)*XSECURE_WORD_LEN);
			UserFuseVal = Xil_In32(UserFuseAddr);
			if ((UserFuseVal & (0x1U << ((*SpkID - 1) %
								XSECURE_WORD_SHIFT))) != 0x0U) {
				Status = (u32)XSECURE_SPK_ERR | (u32)XSECURE_REVOKE_ERR;
			} else {
				Status = (u32)XST_SUCCESS;
			}
		}
		/*
		 *	Maximum 256 keys can be revocated, if exceeds throws
		 *	out of range error
		 */
		else {
			Status = XSECURE_OUT_OF_RANGE_USER_EFUSE_ERROR;
		}
	}
	else {
		Status = XSECURE_INVALID_EFUSE_SELECT;
	}

	return Status;
}


/*****************************************************************************/
/**
 * @brief
 * This function validates boot header.
 *
 * @param		ImageInfo	Pointer to XSecure_ImageInfo structure.
 *
 * @return	Returns Status
 * 		- XST_SUCCESS on successful verification.
 * 		- Error code on failure.
 *
 *****************************************************************************/
static u32 XSecure_BhdrValidation(XSecure_ImageInfo *ImageInfo)
{
	u32 Status = (u32)XST_FAILURE;
	u32 ImgAttributes;
	u32 SizeofBH;

	ImgAttributes = Xil_In32((UINTPTR)(Buffer +
				XSECURE_IMAGE_ATTR_OFFSET));

	if ((ImgAttributes & XSECURE_IMG_ATTR_BHDR_MASK) ==
				XSECURE_IMG_ATTR_BHDR_MASK) {
		ImageInfo->BhdrAuth = XSECURE_ENABLED;
	}

	/* If PUF helper data exists in boot header */
	if ((ImgAttributes & XSECURE_IMG_ATTR_PUFHD_MASK) != 0x00U) {
		SizeofBH = XSECURE_BOOT_HDR_MAX_SIZE;
	}
	else {
		SizeofBH = (u32)XSECURE_BOOT_HDR_MIN_SIZE;
	}

	/* Authenticate keys */
	if (ImageInfo->EfuseRsaenable != 0x00U) {
		if (ImageInfo->BhdrAuth != 0x00U) {
			Status = XSECURE_BHDR_AUTH_NOT_ALLOWED;
			goto END;
		}
	}
	else {
		if (ImageInfo->BhdrAuth == 0x00U) {
			Status = XSECURE_ONLY_BHDR_AUTH_ALLOWED;
			goto END;
		}
	}

	if (ImageInfo->EfuseRsaenable != 0x0U) {
		/* Verify PPK hash with eFUSE */
		Status = XSecure_PpkVerify(&CsuDma, (u8 *)(UINTPTR)AcBuf);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
		/* Copy PPK to global variable for future use */
		(void)XSecure_MemCopy((void *)(UINTPTR)EfusePpk, AcBuf + XSECURE_AC_PPK_OFFSET,
				XSECURE_PPK_SIZE/XSECURE_WORD_LEN);
	}
	/* SPK authentication */
	Status = XSecure_SpkAuthentication(&CsuDma, (u8 *)(UINTPTR)AcBuf, NULL);
	if (Status != (u32)XST_SUCCESS) {
		Status = Status | (u32)XSECURE_BOOT_HDR_FAIL;
		goto END;
	}
	if (ImageInfo->EfuseRsaenable != 0x00U) {
		/* SPK revocation check */
		Status = XSecure_SpkRevokeCheck((u8 *)(UINTPTR)AcBuf);
		if (Status != (u32)XST_SUCCESS) {
			Status = Status | (u32)XSECURE_BOOT_HDR_FAIL;
			goto END;
		}
	}
	/* Authenticated boot header */
	Status = XSecure_BhdrAuthentication(&CsuDma, Buffer, SizeofBH, (u8 *)(UINTPTR)AcBuf);

END:
	return Status;
}


/*****************************************************************************/
/**
 * @brief
 * This function decrypts the image partition.
 *
 * @param	ImageHdrInfo	Pointer to XSecure_ImageInfo structure
 * @param	SrcAddr			Pointer to single partition non-bitstream image
 * @param	KupKey			Pointer to User Key
 * @param	Addr			Location of the secure image loaded after validation
 *
 * @return	Returns Status
 * 		- XST_SUCCESS on successful verification.
 * 		- Error code on failure.
 *
 *****************************************************************************/
static u32 XSecure_DecryptPartition(XSecure_ImageInfo *ImageHdrInfo,
	u8 *SrcAddr, u8 *KupKey,	XSecure_DataAddr *Addr)
{
	u32 Status = (u32)XST_FAILURE;
	u32 Index;
	u8 *EncSrc;
	u8 *DecDst;
	u32 AesKupKey[XSECURE_KEY_LEN];

	if (ImageHdrInfo->KeySrc == XSECURE_KEY_SRC_KUP) {
		if (KupKey != 0x00) {
			/* Linux or U-boot stores Key in the form of String
			 * So this conversion is required here.
			 */
			Status = Xil_ConvertStringToHex((char *)(UINTPTR)KupKey,
			                                   AesKupKey, XSECURE_KEY_STR_LEN);
			if (Status != (u32)XST_SUCCESS) {
				goto END;
			}

			/* XilSecure expects Key in big endian form */
			for (Index = 0U; Index < XSECURE_ARRAY_LENGTH(AesKupKey);
							Index++) {
				AesKupKey[Index] = Xil_Htonl(AesKupKey[Index]);
			}
		}
		else {
			Status = XSECURE_KUP_KEY_NOT_PROVIDED;
			goto END;
		}
	 }
	else {
		if (KupKey != 0x00) {
			Status = XSECURE_KUP_KEY_NOT_REQUIRED;
			goto END;
		}
	}

	/* Initialize the AES driver so that it's ready to use */
	if (ImageHdrInfo->KeySrc == XSECURE_KEY_SRC_KUP) {
		Status = (u32)XSecure_AesInitialize(&SecureAes, &CsuDma,
				XSECURE_CSU_AES_KEY_SRC_KUP,
				(u32 *)ImageHdrInfo->Iv, AesKupKey);
	}
	else {
		Status = (u32)XSecure_AesInitialize(&SecureAes, &CsuDma,
					XSECURE_CSU_AES_KEY_SRC_DEV,
					(u32 *)ImageHdrInfo->Iv, NULL);
	}

	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	if (ImageHdrInfo->PartitionHdr->DestinationLoadAddress == 0x00U) {
		EncSrc = (u8 *)(UINTPTR)(SrcAddr +
				(ImageHdrInfo->PartitionHdr->DataWordOffset *
					XSECURE_WORD_LEN));
		DecDst = (u8 *)(UINTPTR)(SrcAddr +
				(ImageHdrInfo->PartitionHdr->DataWordOffset *
					XSECURE_WORD_LEN));
	}
	else {
		EncSrc = (u8 *)(UINTPTR)(SrcAddr +
				(ImageHdrInfo->PartitionHdr->DataWordOffset *
					XSECURE_WORD_LEN));
		DecDst = (u8 *)(UINTPTR)
			(ImageHdrInfo->PartitionHdr->DestinationLoadAddress);
	}

	Status = (u32)XSecure_AesDecrypt(&SecureAes,
				DecDst, EncSrc,
		(ImageHdrInfo->PartitionHdr->UnEncryptedDataWordLength *
				XSECURE_WORD_LEN));

	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)((u32)XSECURE_PARTITION_FAIL |
				(u32)XSECURE_AES_DECRYPTION_FAILURE |
				(u32)XSECURE_AES_ERROR | (u32)Status);
		Addr->AddrHigh = 0x00U;
		Addr->AddrLow = 0x00U;
	}
	else {
		Addr->AddrHigh = (u32)((u64)(UINTPTR)(DecDst) >> 32);
		Addr->AddrLow = (u32)(UINTPTR)(DecDst);
	}

	XSecure_SetReset(SecureAes.BaseAddress, XSECURE_CSU_AES_RESET_OFFSET);

END:
	if (KupKey != 0x00) {
		/* Clear local user key */
		(void)memset(AesKupKey, 0, XSECURE_KEY_LEN * XSECURE_WORD_LEN);
	}
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function performs authentication of a given data.
 *
 * @param	AuthParam	Pointer to auhentication parameter structure.
 *
 * @return	Returns Status
 * 		- XST_SUCCESS on success
 * 		- XST_FAILURE on failure
 *
 *****************************************************************************/
static inline u32 XSecure_DataAuthentication(XSecure_AuthParam *AuthParam)
{
	u32 Status = (u32)XST_FAILURE;
	XSecure_Sha3 SecureSha3 = {0U};
	u8 Hash[XSECURE_HASH_TYPE_SHA3] = {0U};
	u8 *Signature = (AuthParam->AuthCertPtr + AuthParam->SignatureOffset);
	XSecure_RsaKey KeyInst;
	u8 *AcPtr = AuthParam->AuthCertPtr;

	/* Initialize Sha and RSA instances */
	Status = (u32)XSecure_Sha3Initialize(&SecureSha3, AuthParam->CsuDmaInstPtr);
	if (Status != (u32)XST_SUCCESS) {
		Status = XSECURE_SHA3_INIT_FAIL;
		goto END;
	}

	if (XSECURE_CSU_KECCAK_SHA3 == AuthParam->PaddingType) {
		Status = (u32)XSecure_Sha3PadSelection(&SecureSha3,
		                                      XSECURE_CSU_KECCAK_SHA3);
		if (Status != (u32)XST_SUCCESS) {
			Status = XSECURE_SHA3_PADSELECT_ERR;
			goto END;
		}
	}

	/* Calculate Hash on Data to be authenticated */
	(void)XSecure_Sha3Start(&SecureSha3);
	Status = XSecure_Sha3Update(&SecureSha3, AuthParam->Data, AuthParam->Size);
	if (Status != (u32)XST_SUCCESS) {
		Status = XSECURE_SHA3_UPDATE_FAIL;
		goto END;
	}
	if (1U == AuthParam->AuthIncludingCert) {
		Status = XSecure_Sha3Update(&SecureSha3, AuthParam->AuthCertPtr,
					(XSECURE_AUTH_CERT_MIN_SIZE - XSECURE_PARTITION_SIG_SIZE));
		if (Status != (u32)XST_SUCCESS) {
			Status = XSECURE_SHA3_UPDATE_FAIL;
			goto END;
		}
	}

	Status = XSecure_Sha3Finish(&SecureSha3, Hash);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	AcPtr += ((u32)XSECURE_RSA_AC_ALIGN + (u32)XSECURE_PPK_SIZE);
	KeyInst.Modulus = AcPtr;

	AcPtr += XSECURE_SPK_MOD_SIZE;
	KeyInst.Exponentiation = AcPtr;

	AcPtr += XSECURE_SPK_MOD_EXT_SIZE;
	KeyInst.Exponent = AcPtr;

	Status = XSecure_DataAuth(Signature, &KeyInst, Hash);

END:
	return Status;
}
