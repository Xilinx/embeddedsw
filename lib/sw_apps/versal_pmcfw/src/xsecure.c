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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
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
*       vnc 08/21/18 Added support for PMCFW
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xsecure.h"
#include "xpmcfw_debug.h"

void XPmcFw_PrintArray (u32 DebugType, const u8 Buf[], u32 Len,
						 const char *Str);

#define XPMCFW_PMCRAM_BASEADDR			(0xF2000000U)
#define XPMCFW_CHUNK_SIZE				(0x10000U)

extern u64 AcBuffer[XSECURE_AUTH_CERT_MIN_SIZE/8];

XSecure_Aes SecureAes;
XSecure_Rsa Secure_Rsa;
XSecure_Sha3 Sha3Instance;

static inline u32 XSecure_VerifySpk(u8 *Acptr, u32 EfuseRsaenable);
extern XCsuDma CsuDma0;
extern u32 Left;

/************************** Function Prototypes ******************************/

static inline u32 XSecure_SpkAuthentication(XCsuDma *CsuDmaInstPtr, u8 *AuthCert, u8 *Ppk);
static inline u32 XSecure_PartitionAuthentication(XCsuDma *CsuDmaInstPtr, u8 *Data,
		u32 Size, u8 *AuthCertPtr);
static inline u32 XSecure_DataAuth(u8 *Signature, XSecure_RsaKey *Key, u8 *Hash);
/************************** Function Definitions *****************************/


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
static inline u32 XSecure_DataAuth(u8 *Signature, XSecure_RsaKey *Key, u8 *Hash)
{

	u32 Status;
	XSecure_Rsa SecureRsa;

	/* Assert conditions */
	Xil_AssertNonvoid(Signature != NULL);
	Xil_AssertNonvoid(Key != NULL);
	Xil_AssertNonvoid((Key->Modulus != NULL) && (Key->Exponent != NULL));
	Xil_AssertNonvoid(Hash != NULL);

	/* Initialize RSA instance */
	Status = XSecure_RsaInitialize(&SecureRsa,
			Key->Modulus, Key->Exponentiation, Key->Exponent);
	if (Status != XST_SUCCESS) {
		Status = XSECURE_RSA_INIT_ERR;
		goto END;
	}

	XSecure_Sha3Initialize(&Sha3Instance, &CsuDma0);
	/* Compare encrypted signature with sha3 hash calculated on data */
	Status = XSecure_RsaPssSignatureverification(&SecureRsa, &Sha3Instance,
							Signature, Hash);
	if (Status != XST_SUCCESS) {
		goto END;
		Status = XSECURE_VERIFY_ERR;
	}

END:
	if (Status != XST_SUCCESS) {
		Status = Status | XSECURE_AUTH_FAILURE;
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
static inline u32 XSecure_PartitionAuthentication(XCsuDma *CsuDmaInstPtr, u8 *Data,
				u32 Size, u8 *AuthCertPtr)
{
	u32 Status;
	u8 *Signature = (AuthCertPtr + XSECURE_AUTH_CERT_PARTSIG_OFFSET);
	XSecure_RsaKey Key;
	u8 *AcPtr = AuthCertPtr;
	u8 Hash[XSECURE_HASH_TYPE_SHA3]__attribute__ ((aligned(32)));

	/* Assert conditions */
	Xil_AssertNonvoid(CsuDmaInstPtr != NULL);
	Xil_AssertNonvoid(Size != 0x00);
	Xil_AssertNonvoid(AuthCertPtr != NULL);

	/* Initialize Sha and RSA instances */
	Status = XSecure_Sha3Initialize(&Sha3Instance, CsuDmaInstPtr);
	if (Status != XST_SUCCESS) {
		Status = XSECURE_SHA3_INIT_FAIL;
		goto END;
	}

	/* Calculate Hash on Data to be authenticated */
	XSecure_Sha3Start(&Sha3Instance);
	XSecure_Sha3Update(&Sha3Instance, AuthCertPtr,
		(XSECURE_AUTH_CERT_MIN_SIZE - XSECURE_PARTITION_SIG_SIZE), 0);
	XSecure_Sha3Update(&Sha3Instance, Data, Size, 1);
	XSecure_Sha3Finish(&Sha3Instance, (u8 *)Hash);

	AcPtr += (XSECURE_RSA_AC_ALIGN + XSECURE_PPK_SIZE);
	Key.Modulus = AcPtr;

	AcPtr += XSECURE_SPK_MOD_SIZE;
	Key.Exponentiation = AcPtr;

	AcPtr += XSECURE_SPK_MOD_EXT_SIZE;
	Key.Exponent = AcPtr;

	Status = XSecure_DataAuth(Signature, &Key, (u8 *)Hash);
END:
	if (Status != XST_SUCCESS) {
		Status = Status | XSECURE_AUTH_FAILURE;
	}

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
static inline u32 XSecure_VerifySpk(u8 *AcPtr, u32 EfuseRsaenable)
{
	u32 Status = XST_SUCCESS;

	if (EfuseRsaenable != 0x00) {
		/* Verify SPK with verified PPK */
		Status = XSecure_SpkAuthentication(&CsuDma0, AcPtr, EfusePpk);
		if (Status != XST_SUCCESS) {
			goto END;
		}

	} else {
		/* Verify SPK with PPK in authentication certificate */
		Status = XSecure_SpkAuthentication(&CsuDma0, AcPtr, NULL);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

END:

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
u32 XSecure_IsRsaEnabled()
{
	if ((Xil_In32(XSECURE_EFUSE_SEC_CTRL) &
			XSECURE_EFUSE_SEC_CTRL_RSA_ENABLE) != 0x00) {

		return XSECURE_ENABLED;
	}

	return XSECURE_NOTENABLED;
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
u32 XSecure_IsEncOnlyEnabled()
{
	if ((Xil_In32(XSECURE_EFUSE_SEC_CTRL) &
			XSECURE_EFUSE_SEC_CTRL_ENC_ONLY) != 0x00U) {

		return XSECURE_ENABLED;
	}

	return XSECURE_NOTENABLED;
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
static inline u32 XSecure_SpkAuthentication(XCsuDma *CsuDmaInstPtr,
			u8 *AuthCert, u8 *Ppk)
{

	u8 Hash[XSECURE_HASH_TYPE_SHA3]__attribute__ ((aligned(32)));
	u8 *SpkHash = Hash;
	u8* PpkModular;
	u8* PpkModularEx;
	u8* PpkExpPtr;
	u32 Status;
	u8 *PpkPtr;
	u8 *AcPtr = (u8 *)AuthCert;

	if (Ppk == NULL) {
		PpkPtr = (AcPtr + XSECURE_RSA_AC_ALIGN);
	}
	else {
		PpkPtr = Ppk;
	}

	/* Initialize sha3 */
	XSecure_Sha3Initialize(&Sha3Instance, CsuDmaInstPtr);



	XSecure_Sha3Start(&Sha3Instance);


	/* Hash the PPK + SPK choice */
	XSecure_Sha3Update(&Sha3Instance, AcPtr, XSECURE_AUTH_HEADER_SIZE, 0);


	/* Set PPK pointer */
	PpkModular = (u8 *)PpkPtr;
	PpkPtr += XSECURE_PPK_MOD_SIZE;
	PpkModularEx = PpkPtr;
	PpkPtr += XSECURE_PPK_MOD_EXT_SIZE;
	PpkExpPtr = PpkPtr;
	AcPtr += (XSECURE_PPK_SIZE + 64);


	/* Calculate SPK + Auth header Hash */
	XSecure_Sha3Update(&Sha3Instance, (u8 *)AcPtr, XSECURE_SPK_SIZE, 1);
	XSecure_Sha3Finish(&Sha3Instance, (u8 *)SpkHash);

	/* Set SPK Signature pointer */
	AcPtr += XSECURE_SPK_SIZE + 8;

	Status = (u32)XSecure_RsaInitialize(&Secure_Rsa, PpkModular,
			PpkModularEx, PpkExpPtr);
	if (Status != XST_SUCCESS) {
		Status = XSECURE_RSA_INIT_ERR;
		goto END;
	}

	/* Initialize sha3 */
	XSecure_Sha3Initialize(&Sha3Instance, CsuDmaInstPtr);

	Status = XSecure_RsaPssSignatureverification(&Secure_Rsa, &Sha3Instance, AcPtr, (u8 *)SpkHash);

	if (Status != XST_SUCCESS) {
		XPmcFw_Printf(DEBUG_INFO, "\nFailed at RSA PSS verification\n\r");
		Status = XSECURE_RSA_ENCRYPT_ERR;
		goto END;
	}
	XPmcFw_Printf(DEBUG_DETAILED,
		"Spk authentication is successful\n\r");


END:
	if (Status != XST_SUCCESS) {
		Status = Status | XSECURE_SPK_ERR;
	}

	return Status;

}

/*****************************************************************************/
/**
 * @brief
 * This function authenticates partition
 *
 * @param	PrtnAddr	Address of the partition to be authenticated
 * @param	AcBuffer	Pointer to authentication certificate.
 * @param	PrtnSize	Size of the partition to be authenticated.
 *
 * @return	Returns Status
 * 		- XST_SUCCESS on successful verification.
 * 		- Error code on failure.
 *
 *****************************************************************************/
u32 XSecure_PrtnAuth(u64 PrtnAddr, u64 *AcBuffer, u64 PrtnSize)
{

	u8 *AcPtr = (u8 *)AcBuffer;
	u32 Status;

	/* Verify SPK */
	Status = XSecure_VerifySpk(AcPtr, 0);
	if (Status != XST_SUCCESS) {
		XPmcFw_Printf(DEBUG_INFO,
		"\nSPK authetnication is failed\n\r");
		return Status;
	}

	/* Authenticate partition */
	Status = XSecure_PartitionAuthentication(&CsuDma0,
			(u8 *)(UINTPTR)PrtnAddr, (u32)PrtnSize, AcPtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function Decrypts partition
 *
 * @param	Iv		Pointer to the IV
 * @param	PrtnAddr	Address of the partition to be authenticated
  * @param	PrtnSize	Size of the partition to be authenticated.
 *
 * @return	Returns Status
 * 		- XST_SUCCESS on successful verification.
 * 		- Error code on failure.
 *
 *****************************************************************************/
u32 XSecure_PrtnDec(u8 *Iv, u64 PrtnAddr, u64 Size, u32 KeySrc)
{
	XSecure_Aes Secure_Aes;
	u32 Status;

	u8 *Addr = (u8 *)(INTPTR)PrtnAddr;

	XSecure_AesInit(&Secure_Aes, KeySrc, Iv);

	Status = XSecure_AesDecrypt(&Secure_Aes, Addr, Addr, Size);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return  XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief
 * This function verifies the checksum of the partition
 *
 * @param	CopyFunc	This is a function pointer of device copy
 * @param	PrtnAddr	Pointer to the data
 * @param	PrtnSize	Size of the partition to be verified.
 * @param	Hash		Pointer to the Hash
 *
 * @return	Returns Status
 * 		- XST_SUCCESS on successful verification.
 * 		- Error code on failure.
 *
 *****************************************************************************/
u32 XSecure_CheckSum(DeviceCopy CopyFunc, u64 PrtnAddr, u64 PrtnSize, u8 *Hash)
{

	u32 Status;
	u8 CalHash[XSECURE_SHA3_LEN];
	u64 Addr = PrtnAddr;
	u8 Index;
	u32 ChunkSize;
	u32 TotalSize = PrtnSize;
	u8 Last = 0;

	XSecure_Sha3Initialize(&Sha3Instance, &CsuDma0);
	/*LDRA_INSPECTED 128 D */
	XSecure_Sha3Start(&Sha3Instance);

	/**
	 * Calculate HASH for partition
	 */
	while ((TotalSize != 0x0) && (CopyFunc != NULL)) {
		if (TotalSize > XPMCFW_CHUNK_SIZE) {
			ChunkSize = XPMCFW_CHUNK_SIZE;
		}
		else {
			ChunkSize = TotalSize;
			Last = 1;
		}

		Status = CopyFunc(Addr, (u64 )XPMCFW_PMCRAM_BASEADDR,
					ChunkSize, 0);
		if (XST_SUCCESS != Status)
		{
			goto END;
		}
		Status = XSecure_Sha3Update(&Sha3Instance,
					(u8 *)XPMCFW_PMCRAM_BASEADDR,
					ChunkSize, Last);
		if (Status != (u32)XST_SUCCESS) {

			goto END;
		}
		Addr = Addr + ChunkSize;
		TotalSize = TotalSize - ChunkSize;
	}

	if (CopyFunc == NULL) {
		Status = XSecure_Sha3Update(&Sha3Instance,
				(u8 *)(UINTPTR)PrtnAddr,
				PrtnSize, 1);
		if (Status != (u32)XST_SUCCESS) {

			goto END;
		}
	}
	Status = XSecure_Sha3Finish(&Sha3Instance, CalHash);
	if (Status != (u32)XST_SUCCESS)	{
		goto END;
	}

	/**
	* Compare the calculated and builtin hash
	*/
	for (Index = 0; Index < XSECURE_SHA3_LEN; Index++) {
		if ((*(Hash + Index)) != CalHash[Index]) {
			XPmcFw_PrintArray(DEBUG_INFO, CalHash, XSECURE_SHA3_LEN,
						"Calculated Hash");
			XPmcFw_PrintArray(DEBUG_INFO, Hash, XSECURE_SHA3_LEN,
						"Expected Hash");
			Status = XST_FAILURE;
			goto END;
		}
	}
END:

	return  Status;
}

/******************************************************************************
*
* This function performs authentication and RSA signature verification for
* each block.
*
* @param	PartitionParams is a pointer to XSecure_Partition
* @param	BlockAdrs is the block start address
* @param	BlockSize is the block size
*
* @return
* 		Error code on failure
* 		XFSBL_SUCESS on success
*
* @note		None
*
******************************************************************************/
static u32 XFsbl_PlSignVer(XSecure_Partition *PartitionParams,
		UINTPTR BlockAdrs, u32 BlockSize)
{
	u32 Status;
	u32 HashDataLen = BlockSize;
	u8 *ChunksHash = PartitionParams->PlAuth.HashsOfChunks;
	u32 Len = PartitionParams->ChunkSize;
	u32 Offset = BlockAdrs;
	u32 Last =0;
	XSecure_RsaKey Key;
	u8 Hash[XSECURE_HASH_TYPE_SHA3]__attribute__ ((aligned(32)));
	u8 *AcOffset = (u8 *)AcBuffer;
	u8 * AcPtr = (u8*)AcOffset;
	u8 *Signature = (AcPtr + XSECURE_AUTH_CERT_PARTSIG_OFFSET);

	/* Copy Authentication certificate */
	Status = PartitionParams->DeviceCopy(PartitionParams->PlAuth.AcOfset,
			(u32)AcBuffer, XSECURE_AUTH_CERT_MIN_SIZE, 0U);

	/*Verify SPK */
	Status = XSecure_VerifySpk((u8 *)AcBuffer, 0);
	if (Status != XST_SUCCESS) {
		XPmcFw_Printf(DEBUG_INFO,
		"\nSPK verification is failed for block %0d \n\r",
				PartitionParams->PlAuth.BlockNum);
		goto END;
	}
	XPmcFw_Printf(DEBUG_DETAILED,
		"\nSPK Authentication of block %0d is Successful\n\r",
				PartitionParams->PlAuth.BlockNum);


	/* Authentication in chunking mode */

	/* Start the SHA engine */
	XSecure_Sha3Initialize(&Sha3Instance, &CsuDma0);
	XSecure_Sha3Start(&Sha3Instance);

	memset(Hash, 0, 48);

	/* Calculate hash for (AC - signature size) */
	XSecure_Sha3Update(&Sha3Instance, (u8 *)AcOffset,
			(XSECURE_AUTH_CERT_MIN_SIZE - XSECURE_PARTITION_SIG_SIZE), 0);
	do {

		if (PartitionParams->IsEncrypted == TRUE) {
			/* Only Secure header is processed in first chunk of first block */
			if ((BlockAdrs == PartitionParams->StartAddress) &&
					(HashDataLen == BlockSize)) {
				Len = 64;
			}
			else if(HashDataLen > PartitionParams->ChunkSize) {
				Len = PartitionParams->ChunkSize + 64;
			}
			else {
				Len = HashDataLen;
				Last = 1;
			}
		}
		else {
			 if (HashDataLen > PartitionParams->ChunkSize) {
				Len = PartitionParams->ChunkSize;
			 }
			 else {
				Len = HashDataLen;
				Last = 1;
			}
		}

		HashDataLen = HashDataLen - Len;


		Status = PartitionParams->DeviceCopy((u32)Offset,
			(UINTPTR)PartitionParams->ChunkBuffer, Len, 0);
		if (Status != XST_SUCCESS) {
			return Status;
		}

		XSecure_Sha3Update(&Sha3Instance,
			PartitionParams->ChunkBuffer, Len , Last);
		XSecure_Sha3_ReadHash(&Sha3Instance,
				(u8 *)ChunksHash);

		ChunksHash = ChunksHash + 48;
		Offset = Offset + Len;

	}while (HashDataLen != 0x00);

	XSecure_Sha3Finish(&Sha3Instance, (u8 *)Hash);

	AcPtr += (XSECURE_RSA_AC_ALIGN + XSECURE_PPK_SIZE);
	Key.Modulus = AcPtr;

	AcPtr += XSECURE_SPK_MOD_SIZE;
	Key.Exponentiation = AcPtr;

	AcPtr += XSECURE_SPK_MOD_EXT_SIZE;
	Key.Exponent = AcPtr;

	Status = XSecure_DataAuth(Signature, &Key, (u8 *)Hash);
	if (Status != XST_SUCCESS) {
		XPmcFw_Printf(DEBUG_INFO,
			"\r\nAuthentication of block %d is failed\n\r",
			PartitionParams->PlAuth.BlockNum);
		Status = Status | XSECURE_AUTH_FAILURE;
		goto END;
	}
	XPmcFw_Printf(DEBUG_INFO,
		"\r\nAuthentication of block %d is success\n\r",
		PartitionParams->PlAuth.BlockNum);

END:
	return Status;
}

/******************************************************************************
*
* This function re-authenticates the each chunk of the block and compares
* with the stored hash and sends the data AES engine if encryption exists
* and to PCAP directly in encryption is not existing.
*
* @param	PartitionParams is a pointer to XSecure_Partition
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
static u32 XFsbl_ReAuthenticationBlock(XSecure_Partition *PartitionParams,
				u8 *AcOffset)
{
	u32 Status;
	u32 Len;
	UINTPTR Offset;
	u8 ChunksHash[48];
	u8 *HashStored;
	u32 Address;
	u8 Last = 0;

	Address = PartitionParams->StartAddress +
		(PartitionParams->PlAuth.BlockSize *
			(PartitionParams->PlAuth.BlockNum));

	(void)memset(ChunksHash,0U,sizeof(ChunksHash));


	if (PartitionParams->PlAuth.ChunkNum == 0x00) {
		Status = XSecure_Sha3Initialize(&Sha3Instance, &CsuDma0);
		if (Status != XST_SUCCESS) {
			return Status;
		}
		XSecure_Sha3Start(&Sha3Instance);
		/* Calculate hash for (AC - signature size) */
		/* As Authentication buffer has AC of this block */
		XSecure_Sha3Update(&Sha3Instance, (u8 *)AcOffset,
		(XSECURE_AUTH_CERT_MIN_SIZE - XSECURE_PARTITION_SIG_SIZE), 0);
	}
	else {
		/* Configure secure stream switch */
		Xil_Out32(0xF1110500,0xC << 16);
	}

	/* calculating hashes for all chunks copies to AES/PCAP */
	if (PartitionParams->IsEncrypted == TRUE) {
		if ((PartitionParams->PlAuth.BlockNum == 0x00) &&
			(PartitionParams->PlAuth.ChunkNum != 0x00)) {
			Offset = (UINTPTR)Address + 64 +
				(u64)((PartitionParams->ChunkSize + 64) *
				(PartitionParams->PlAuth.ChunkNum - 1));
		}
		else {
			Offset = (UINTPTR)Address;
		}
	}
	else {
		Offset = (UINTPTR)Address +
			(u64)(PartitionParams->ChunkSize *
			PartitionParams->PlAuth.ChunkNum);
	}

	HashStored = PartitionParams->PlAuth.HashsOfChunks +
			(PartitionParams->PlAuth.ChunkNum)*48;

	if (PartitionParams->IsEncrypted == TRUE) {
	/* Only Secure header is processed in first chunk of first block */
		if ((PartitionParams->PlAuth.ChunkNum == 0x00) &&
			(PartitionParams->PlAuth.BlockNum == 0x00)) {
			Len = 64;
		}
		else if(PartitionParams->PlAuth.CurBlockSize >
			PartitionParams->ChunkSize) {
			Len = PartitionParams->ChunkSize + 64;
		}
		else {
			Len = PartitionParams->PlAuth.CurBlockSize;
		}
	}
	else {
	/* Only Secure header is processed in first chunk of first block */
		 if(PartitionParams->PlAuth.CurBlockSize >
			PartitionParams->ChunkSize) {
			Len = PartitionParams->ChunkSize;
		}
		else {
			Len = PartitionParams->PlAuth.CurBlockSize;
		}
	}
	PartitionParams->PlAuth.CurBlockSize =
		PartitionParams->PlAuth.CurBlockSize - Len;
	PartitionParams->PlAuth.ChunkNum =
		PartitionParams->PlAuth.ChunkNum + 1;

	if (PartitionParams->PlAuth.CurBlockSize == 0x00) {
		Last = 1;
		PartitionParams->PlAuth.ChunkNum = 0;
		PartitionParams->PlAuth.BlockNum =
			PartitionParams->PlAuth.BlockNum + 1;
		PartitionParams->PlAuth.ChunkNum = 0;
	}

	/* Copy from DDR or flash to Buffer */
	Status = PartitionParams->DeviceCopy(
		(u32)Offset,(UINTPTR)PartitionParams->ChunkBuffer, Len, 0);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/* Calculating hash for each chunk */
	XSecure_Sha3Update(&Sha3Instance,
		PartitionParams->ChunkBuffer, Len, Last);
	XSecure_Sha3_ReadHash(&Sha3Instance, (u8 *)ChunksHash);

	/* Comparing with stored Hashes */
	Status = memcmp(HashStored, ChunksHash, 48);
	if (Status != XST_SUCCESS) {
		XPmcFw_Printf(DEBUG_INFO,"\nError at hash comparison\n\r");
		Status = XST_FAILURE;
		goto END;
	}

END:
	return Status;

}

/*****************************************************************************/
/**
 * @brief
 * This function performs decryption of partition where the partition is
 * accessed in chunks. Mainly used for npi/cdo or cfi
 * partitions.
 *
 * @param	PartitionParams	Pointer to the XSecure_Partition
 *
 * @return	Returns Status
 * 		- XST_SUCCESS on success
 * 		- Error code on failure
 *
 *****************************************************************************/
static inline u32 XSecure_DecryptChunks(XSecure_Partition *PartitionParams)
{
	XStatus Status = XST_SUCCESS;
	u8 *GcmTagAddr = 0x0U;
	u8 *SrcAddr = (u8 *)PartitionParams->ChunkBuffer;
	u32 NextBlkLen = 0x0U;
	XCsuDma_Configure ConfigurValues = {0U};
	XSecure_Aes *SecureAesPtr = PartitionParams->PlEncrypt.SecureAes;
	u32 SssDma;
	u32 SssAes;
	u32 SssCfg;

	/* Configure secure stream switch */
	/* Configure the SSS for AES. */
	if (SecureAesPtr->CsuDmaPtr->Config.DeviceId == 0U) {
		SssDma =  0x6U;
		SssAes = ((u32)0xE << 12);
		SssCfg = SssDma | SssAes;
	}
	else {
		SssDma = ((u32)0x7 << 4);
		SssAes = ((u32)0x5 << 12);
		SssCfg = SssDma | SssAes;
	}

	Xil_Out32(XSECURE_CSU_SSS_BASE, SssCfg);

	if (PartitionParams->IsEncrypted != TRUE) {
		return XST_SUCCESS;
	}
	if (PartitionParams->PlEncrypt.DataDecrypted == 0) {
		if (PartitionParams->IsAuthenticated != TRUE) {
		/* only Secure header should be decrypted */
			Status = PartitionParams->DeviceCopy(
				PartitionParams->StartAddress,
					(u32)SrcAddr, 64, 0U);
		}

		SecureAesPtr->DstEndinaness = 1;
		GcmTagAddr = SrcAddr + XSECURE_SECURE_HDR_SIZE;

		/* Clear AES_KEY_CLEAR bits to avoid clearing of key */
		XSecure_WriteReg(SecureAesPtr->BaseAddress,
			XSECURE_CSU_AES_KEY_CLR_OFFSET, (u32)0x0U);
		Status = XSecure_AesKeySelNLoad(SecureAesPtr);
		if (Status != (u32)XST_SUCCESS) {
			goto ENDF;
		}
		 XSecure_WriteReg(SecureAesPtr->BaseAddress,
			   XSECURE_AES_ENIDANNESS_SWAP_OFFSET, 0x1U);
		/* Enable CSU DMA Src channel for byte swapping.*/
		XCsuDma_GetConfig(SecureAesPtr->CsuDmaPtr,
			XCSUDMA_SRC_CHANNEL, &ConfigurValues);
		ConfigurValues.EndianType = 1U;
		XCsuDma_SetConfig(SecureAesPtr->CsuDmaPtr,
			XCSUDMA_SRC_CHANNEL, &ConfigurValues);

		/* Decrypt secure header */
		Status = XSecure_AesDecryptBlk(SecureAesPtr, SrcAddr,
					(const u8 *)SrcAddr,
				((const u8 *)GcmTagAddr),
					0, 0);

		/* If decryption failed then return error. */
		if(Status != (u32)XST_SUCCESS)
		{
			goto ENDF;
		}
		PartitionParams->PlEncrypt.DataDecrypted =
			PartitionParams->PlEncrypt.DataDecrypted + 64;
		if (PartitionParams->IsAuthenticated == TRUE) {
			goto ENDF;
		}
	}


	/* Decrypting the data other than Secure header */
	/*
	 * Find the size of next block to be decrypted.
	 * Size is in 32-bit words so mul it with 4
	 */
	NextBlkLen = Xil_Htonl(XSecure_ReadReg(SecureAesPtr->BaseAddress,
		XSECURE_CSU_AES_IV_3_OFFSET)) * 4U;
	if (NextBlkLen > PartitionParams->ChunkSize) {
		XPmcFw_Printf(DEBUG_INFO,"Block length is exceeded\n\r");
		return XST_FAILURE;
	}

	SecureAesPtr->KeySel = XSECURE_AES_KUP_KEY;
	Status = XSecure_AesKeySelNLoad(SecureAesPtr);
	if (Status != (u32)XST_SUCCESS) {
		goto ENDF;
	}


	/* Copy Next block + GCM tag of that block */
	if (PartitionParams->IsAuthenticated != TRUE) {
		Status = PartitionParams->DeviceCopy(
			PartitionParams->StartAddress +
		PartitionParams->PlEncrypt.DataDecrypted,
		(u32)SrcAddr, NextBlkLen + 64, 0U);
		if (Status != XST_SUCCESS) {
			goto ENDF;
		}
	}
	 /* Point IV to the CSU IV register. */
	SecureAesPtr->Iv = (u32 *)(SecureAesPtr->BaseAddress +
	(UINTPTR)XSECURE_CSU_AES_IV_0_OFFSET);


	/* Update the GcmTagAddr to get GCM-TAG for next block. */
	GcmTagAddr = SrcAddr + NextBlkLen + 48;
	/* Decrypt secure header */
	Status = XSecure_AesDecryptBlk(SecureAesPtr, SrcAddr,
				(const u8 *)SrcAddr,
				((const u8 *)GcmTagAddr),
				NextBlkLen, 1);

	/* If decryption failed then return error. */
	if(Status != (u32)XST_SUCCESS)
	{
		goto ENDF;
	}
	PartitionParams->PlEncrypt.DataDecrypted = PartitionParams->PlEncrypt.DataDecrypted + 64 + NextBlkLen;
	Status = XST_SUCCESS;
	XPmcFw_Printf(DEBUG_DETAILED,"Decryption is success\n\r");
ENDF:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function performs authentication and/or decryption of partition
 * where the partition is accessed in chunks. Mainly used for npi/cdo or cfi
 * partitions.
 *
 * @param	PartitionParams	Pointer to the XSecure_Partition
 *
 * @return	Returns Status
 * 		- XST_SUCCESS on success
 * 		- Error code on failure
 *
 *****************************************************************************/
u32 XPmcFw_ProcessSecurePrtn(XSecure_Partition *PartitionParams)
{

	XStatus Status = XST_SUCCESS;
	u64 Data;

	/* If partition is authenticated */
	if (PartitionParams->IsAuthenticated == TRUE) {
		if (PartitionParams->PlAuth.BlockNum == 0) {
			if (PartitionParams->IsEncrypted != TRUE) {
				PartitionParams->PlAuth.HashsOfChunks =
				(u8 *)(PartitionParams->ChunkBuffer +
					PartitionParams->ChunkSize);
			}
			else {
				PartitionParams->PlAuth.HashsOfChunks =
				(u8 *)(PartitionParams->ChunkBuffer +
					PartitionParams->ChunkSize +
					XSECURE_SECURE_HDR_SIZE +
					XSECURE_SECURE_HDR_SIZE);
			}
		}

		/* Authentication of blocks */
		if (PartitionParams->PlAuth.ChunkNum == 0) {
			XPmcFw_Printf(DEBUG_INFO,
				"Authenticaion of block %0d \n\r",
				PartitionParams->PlAuth.BlockNum);
			Data = PartitionParams->StartAddress +
				(PartitionParams->PlAuth.BlockSize *
				(PartitionParams->PlAuth.BlockNum));

			/*
			 * If the block start address + block size exceeds
			 * first AC address it is last block
			 */
			if (Data + PartitionParams->PlAuth.BlockSize >
				PartitionParams->PlAuth.AcOfset) {
				/*
				 * Data placed in last block might not be full block size
				 * TotalLen -
				 * (NoofBlocks)*(AC size) - (NoOfBlocks - 1)*BlockSize
				 */
				PartitionParams->PlAuth.CurBlockSize =
						(PartitionParams->TotalSize) -
					(PartitionParams->PlAuth.BlockNum + 1) *
						(XSECURE_AUTH_CERT_MIN_SIZE) -
					(PartitionParams->PlAuth.BlockNum) *
					(PartitionParams->PlAuth.BlockSize);
			}
			else {
				PartitionParams->PlAuth.CurBlockSize =
					PartitionParams->PlAuth.BlockSize;
			}


			Status = XFsbl_PlSignVer(PartitionParams, Data,
				PartitionParams->PlAuth.CurBlockSize);
			if (Status != XST_SUCCESS) {
				goto ENDF;
			}

			/* Re-Authenticate the function */
			Status = XFsbl_ReAuthenticationBlock(PartitionParams,
				(u8 *)AcBuffer);
			if (Status != XST_SUCCESS) {
				goto ENDF;
			}

		}
		/* Re authentication of blocks */
		else {
			Status = XFsbl_ReAuthenticationBlock(PartitionParams,
				(u8 *)AcBuffer);
			if (Status != XST_SUCCESS) {
				goto ENDF;
			}
		}

	}

	/* If partition is encrypted */
	if (PartitionParams->IsEncrypted == TRUE) {
		Status = XSecure_DecryptChunks(PartitionParams);
		if (Status != XST_SUCCESS) {
			goto ENDF;
		}
		/*
		 * Re-Authentication after secure header decryption as first
		 * chunk contains only secure header, when authentication is
		 * enabled
		 */
		if ((PartitionParams->IsAuthenticated == TRUE) &&
			(PartitionParams->PlAuth.BlockNum == 0) &&
			(PartitionParams->PlAuth.ChunkNum == 1)) {
			Status = XFsbl_ReAuthenticationBlock(PartitionParams,
					(u8 *)AcBuffer);
			if (Status != XST_SUCCESS) {
				goto ENDF;
			}
			Status = XSecure_DecryptChunks(PartitionParams);
			if (Status != XST_SUCCESS) {
				goto ENDF;
			}
		}
	}

ENDF:

	return Status;

}

/*****************************************************************************/
/**
 * @brief
 * This initializes AES's InstancePtr with key and IV
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes
 * @param	KeySel		Key source
 * @param	Iv			Pointer to the IV
 *
 * @return	Returns Status
 * 		- XST_SUCCESS on success
 * 		- Error code on failure
 *
 *****************************************************************************/
void XSecure_AesInit(XSecure_Aes *InstancePtr, u32 KeySel, u8 *Iv)
{

	XPmcFw_Printf(DEBUG_INFO,
					"\nDecrypting the partition \n\r");
	if (KeySel == 0x3A5C3C5A) {
		(void)XSecure_AesInitialize(InstancePtr, &CsuDma0,
				XSECURE_AES_BBRAM_KEY,
						Iv);
	}
	else if (KeySel == 0xA5C3C5A3) {
		(void)XSecure_AesInitialize(InstancePtr, &CsuDma0,
				XSECURE_AES_EFUSE_KEY,
						Iv);
	}
	XSecure_AesReset(InstancePtr);

}
