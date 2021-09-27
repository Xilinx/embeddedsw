/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_aesclient.c
*
* This file contains the implementation of the client interface functions for
* AES driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kal  03/23/21 Initial release
* 4.5   kal  03/23/20 Updated file version to sync with library version
*       har  04/14/21 Added XSecure_AesEncryptData and XSecure_AesDecryptData
* 4.6   har  08/31/21 Updated check for Size in XSecure_AesKekDecrypt
*       kpt  09/27/21 Fixed compilation warnings
*
* </pre>
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_cache.h"
#include "xsecure_aesclient.h"
#include "xsecure_defs.h"
#include "xsecure_ipi.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief       This function sends IPI request to initialize the AES engine
 *
 * @return	- XST_SUCCESS - If the initialization is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XSecure_AesInitialize(void)
{
	volatile int Status = XST_FAILURE;

	Status = XSecure_ProcessIpiWithPayload0(XSECURE_API_AES_INIT);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to EncryptInit the AES engine
 *
 * @param	KeySrc	- Type of the Key
 * @param	Size	- Size of the Key
 * @param	IvAddr	- Address of the IV
 *
 * @return	- XST_SUCCESS - If the Encryptinit is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XSecure_AesEncryptInit(XSecure_AesKeySource KeySrc, u32 Size, u64 IvAddr)
{
	volatile int Status = XST_FAILURE;
	XSecure_AesInitOps AesParams __attribute__ ((aligned (64)));
	u64 Buffer;

	AesParams.IvAddr = IvAddr;
	AesParams.OperationId = XSECURE_ENCRYPT;
	AesParams.KeySrc = KeySrc;
	AesParams.KeySize = Size;
	Buffer = (u64)(UINTPTR)&AesParams;

	Xil_DCacheFlushRange((INTPTR)Buffer, sizeof(AesParams));

	Status = XSecure_ProcessIpiWithPayload2(XSECURE_API_AES_OP_INIT,
			(u32)Buffer, (u32)(Buffer >> 32));

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to DecryptInit the AES engine
 *
 * @param	KeySrc	- Type of the Key
 * @param	Size	- Size of the Key
 * @param	IvAddr	- Address of the IV
 *
 * @return	- XST_SUCCESS - If the Decrypt init is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XSecure_AesDecryptInit(XSecure_AesKeySource KeySrc, u32 Size, u64 IvAddr)
{
	volatile int Status = XST_FAILURE;
	XSecure_AesInitOps AesParams __attribute__ ((aligned (64)));
	u64 Buffer;

	AesParams.IvAddr = IvAddr;
	AesParams.OperationId = XSECURE_DECRYPT;
	AesParams.KeySrc = KeySrc;
	AesParams.KeySize = Size;
	Buffer = (u64)(UINTPTR)&AesParams;

	Xil_DCacheFlushRange((INTPTR)Buffer, sizeof(AesParams));

	Status = XSecure_ProcessIpiWithPayload2(XSECURE_API_AES_OP_INIT,
			(u32)Buffer, (u32)(Buffer >> 32));

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to update AAD to AES engine
 *
 * @param	AadSize	- Size of the Aad data
 * @param	AadAddr	- Address of the Aad
 *
 * @return	- XST_SUCCESS - If the Aad update is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XSecure_AesUpdateAad(u64 AadAddr, u32 AadSize)
{
	volatile int Status = XST_FAILURE;

	Status = XSecure_ProcessIpiWithPayload3(XSECURE_API_AES_UPDATE_AAD,
			(u32)AadAddr, (u32)(AadAddr >> 32), AadSize);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to update the input data to
 * 		AES engine for encryption
 *
 * @param	InDataAddr	- Address of the input data which needs to be
 * 				encrypted
 * @param	OutDataAddr	- Address of the buffer where the encrypted data
 * 				to be updated
 * @param	Size		- Size of the input data to be encrypted
 * @param	IsLast		- If this is the last update of data to be
 * 				encrypted, this parameter should be set to TRUE
 * 				otherwise FALSE
 *
 * @return	- XST_SUCCESS - On successful encryption of the data
 *		- XSECURE_AES_INVALID_PARAM - On invalid parameter
 *		- XSECURE_AES_STATE_MISMATCH_ERROR - If there is state mismatch
 *		- XST_FAILURE - On failure
 *
 *****************************************************************************/
int XSecure_AesEncryptUpdate(u64 InDataAddr, u64 OutDataAddr,
				u32 Size, u32 IsLast)
{
	volatile int Status = XST_FAILURE;
	XSecure_AesInParams EncInAddr __attribute__ ((aligned (64)));
	u64 SrcAddr;

	EncInAddr.InDataAddr = InDataAddr;
	EncInAddr.Size = Size;
	EncInAddr.IsLast = IsLast;
	SrcAddr = (u64)(UINTPTR)&EncInAddr;

	Xil_DCacheFlushRange((INTPTR)SrcAddr, sizeof(EncInAddr));

	Status = XSecure_ProcessIpiWithPayload4(XSECURE_API_AES_ENCRYPT_UPDATE,
			(u32)SrcAddr, (u32)(SrcAddr >> 32), (u32)OutDataAddr,
			(u32)(OutDataAddr >> 32));

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to update the GcmTag Addr to
 * 		AES engine
 *
 * @param	GcmTagAddr	- Address to the buffer of GCM tag size,
 * 				where the API updates GCM tag
 *
 * @return	- XST_SUCCESS - On successful encryption of the data
 *		- XSECURE_AES_INVALID_PARAM - On invalid parameter
 *		- XSECURE_AES_STATE_MISMATCH_ERROR - If there is state mismatch
 *		- XST_FAILURE - On failure
 *
 *****************************************************************************/
int XSecure_AesEncryptFinal(u64 GcmTagAddr)
{
	volatile int Status = XST_FAILURE;

	Status = XSecure_ProcessIpiWithPayload2(XSECURE_API_AES_ENCRYPT_FINAL,
			(u32)GcmTagAddr, (u32)(GcmTagAddr >> 32));

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to update the encrypted data to
 * 		AES engine for decryption
 *
 * @param	InDataAddr	- Address of the encryped data which needs to be
 * 				decrypted
 * @param	OutDataAddr	- Address of the buffer where the decrypted data
 * 				to be updated
 * @param	Size		- Size of the input data to be decrypted
 * @param	IsLast		- If this is the last update of data to be
 * 				decrypted, this parameter should be set to TRUE
 * 				otherwise FALSE
 *
 * @return	- XST_SUCCESS - On successful decryption of the data
 *		- XSECURE_AES_INVALID_PARAM - On invalid parameter
 *		- XSECURE_AES_STATE_MISMATCH_ERROR - If there is state mismatch
 *		- XST_FAILURE - On failure
 *
 *****************************************************************************/
int XSecure_AesDecryptUpdate(u64 InDataAddr, u64 OutDataAddr,
				u32 Size, u32 IsLast)
{
	volatile int Status = XST_FAILURE;
	XSecure_AesInParams DecInParams __attribute__ ((aligned (64)));
	u64 SrcAddr;

	DecInParams.InDataAddr = InDataAddr;
	DecInParams.Size = Size;
	DecInParams.IsLast = IsLast;
	SrcAddr = (u64)(UINTPTR)&DecInParams;

	Xil_DCacheFlushRange((INTPTR)SrcAddr, sizeof(DecInParams));

	Status = XSecure_ProcessIpiWithPayload4(XSECURE_API_AES_DECRYPT_UPDATE,
			(u32)SrcAddr, (u32)(SrcAddr >> 32),
			(u32)OutDataAddr, (u32)(OutDataAddr >> 32));

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function sends IPI request to verify the GcmTag provided
 * 		for the data decrypted till the point
 *
 * @param	GcmTagAddr	- Address of a buffer which should holds GCM Tag
 *
 * @return	- XST_SUCCESS - On successful encryption of the data
 * 		- XSECURE_AES_GCM_TAG_MISMATCH - User provided GCM tag does not
 *	 				match calculated tag
 *		- XSECURE_AES_INVALID_PARAM - On invalid parameter
 *		- XSECURE_AES_STATE_MISMATCH_ERROR - If there is state mismatch
 *		- XST_FAILURE - On failure
 *
 *****************************************************************************/
int XSecure_AesDecryptFinal(u64 GcmTagAddr)
{
	volatile int Status = XST_FAILURE;

	Status = XSecure_ProcessIpiWithPayload2(XSECURE_API_AES_DECRYPT_FINAL,
			(u32)GcmTagAddr, (u32)(GcmTagAddr >> 32));

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to zeroize selected AES
 * 		key storage register
 *
 * @param	KeySrc	- Select the key source which needs to be zeroized
 *
 * @return	- XST_SUCCESS -  When key zeroization is success
 *		- XSECURE_AES_INVALID_PARAM - On invalid parameter
 *		- XSECURE_AES_STATE_MISMATCH_ERROR - If there is state mismatch
 *		- XST_FAILURE - On failure
 *
 ******************************************************************************/
int XSecure_AesKeyZero(XSecure_AesKeySource KeySrc)
{
	volatile int Status = XST_FAILURE;

	Status = XSecure_ProcessIpiWithPayload1(XSECURE_API_AES_KEY_ZERO,
			KeySrc);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to write the key provided
 * 		into the specified AES key registers
 *
 * @param	KeySrc	- Key Source to be selected to which provided
 * 			key should be updated
 * 		Size	- Size of the input key to be written
 * 		KeyAddr	- Address of a buffer which should contain the key
 * 			to be written
 *
 * @return	- XST_SUCCESS - On successful key written on AES registers
 *		- XSECURE_AES_INVALID_PARAM - On invalid parameter
 *		- XSECURE_AES_STATE_MISMATCH_ERROR - If there is state mismatch
 *		- XST_FAILURE - On failure
 *
 ******************************************************************************/
int XSecure_AesWriteKey(XSecure_AesKeySource KeySrc, u32 Size, u64 KeyAddr)
{
	volatile int Status = XST_FAILURE;

	Status = XSecure_ProcessIpiWithPayload4(XSECURE_API_AES_WRITE_KEY, Size,
			KeySrc, (u32)KeyAddr, (u32)(KeyAddr >> 32));

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to decrypt the key in
 * 		KEK key form
 *
 * @param	IVAddr		- Address of IV holding buffer for decryption
 *				of the key
 * @param	DecKeySrc	- Select key source which holds KEK and
 * 				needs to be decrypted
 * @param	DstKeySrc	- Select the key in which decrypted red key
 * 				should be updated
 * @param	Size		- Size of the key
 *
 * @return	- XST_SUCCESS - On successful key decryption
 * 		- XSECURE_AES_INVALID_PARAM - On invalid parameter
 * 		- XST_FAILURE - If timeout has occurred
 *
 ******************************************************************************/
int XSecure_AesKekDecrypt(u64 IvAddr, XSecure_AesKeySource DstKeySrc,
				XSecure_AesKeySource DecKeySrc, XSecure_AesKeySize Size)
{
	volatile int Status = XST_FAILURE;

	if ((Size != XSECURE_AES_KEY_SIZE_128) &&
		 (Size != XSECURE_AES_KEY_SIZE_256)) {
		Status = XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	Status = XSecure_ProcessIpiWithPayload3(XSECURE_API_AES_KEK_DECRYPT,
			(((u32)Size << 16) | ((u32)DstKeySrc << 8) | DecKeySrc),
			(u32)IvAddr, (u32)(IvAddr >> 32));

END:
	return Status;
}
/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to enable/disable DpaCm in AES
 *
 * @param	DpaCmCfg    - User choice to enable/disable DPA CM
 *
 * @return	- XST_SUCCESS - If configuration is success
 * 		- XSECURE_AES_INVALID_PARAM	- For invalid parameter
 * 		- XSECURE_AES_STATE_MISMATCH_ERROR - If there is state mismatch
 * 		- XSECURE_AES_DPA_CM_NOT_SUPPORTED - If DPA CM is disabled
 * 						on chip
 * 		(Enabling/Disabling in AES engine does not impact functionality)
 *
 *
 ******************************************************************************/
int XSecure_AesSetDpaCm(u8 DpaCmCfg)
{
	volatile int Status = XST_FAILURE;

	Status = XSecure_ProcessIpiWithPayload1(XSECURE_API_AES_SET_DPA_CM,
		DpaCmCfg);

	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to KAT on AES engine
 *
 * @return	- XST_SUCCESS - When KAT Pass
 * 		- XSECURE_AESKAT_INVALID_PARAM	 - On invalid argument
 * 		- XSECURE_AES_KAT_WRITE_KEY_FAILED_ERROR - Error when AES key
 *							write fails
 * 		- XSECURE_AES_KAT_DECRYPT_INIT_FAILED_ERROR - Error when AES
 * 							decrypt init fails
 * 		- XSECURE_AES_KAT_GCM_TAG_MISMATCH_ERROR - Error when GCM tag
 * 					not matched with user provided tag
 * 		- XSECURE_AES_KAT_DATA_MISMATCH_ERROR - Error when AES data
 * 					not matched with expected data
 *
 ******************************************************************************/
int XSecure_AesDecryptKat(void)
{
	volatile int Status = XST_FAILURE;

	Status = XSecure_ProcessIpiWithPayload0(XSECURE_API_AES_DECRYPT_KAT);

	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to perform KAT on AES engine
 * 		to confirm DPA counter measures is working fine
 *
 * @return 	- XST_SUCCESS - On success
 * 		- XSECURE_AESKAT_INVALID_PARAM	- Invalid Argument
 * 		- XSECURE_AESDPACM_KAT_WRITE_KEY_FAILED_ERROR - Error when
 * 						AESDPACM key write fails
 * 		- XSECURE_AESDPACM_KAT_KEYLOAD_FAILED_ERROR - Error when
 * 						AESDPACM key load fails
 * 		- XSECURE_AESDPACM_SSS_CFG_FAILED_ERROR - Error when
 * 						AESDPACM sss configuration fails
 * 		- XSECURE_AESDPACM_KAT_FAILED_ERROR - Error when
 * 						AESDPACM KAT fails
 * 		- XST_FAILURE - On failure
 *
 ******************************************************************************/
int XSecure_AesDecryptCmKat(void)
{
	volatile int Status = XST_FAILURE;

	Status = XSecure_ProcessIpiWithPayload0(XSECURE_API_AES_DECRYPT_CM_KAT);

	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function calls IPI request to encrypt a single block of data.
 *
 * @param	KeySrc - Type of the key
 * @param	KeySize - Size of the key
 * @param	IvAddr - Address of the IV
 * @param	InDataAddr - Address of the data which needs to be encrypted
 * @param	OutDataAddr - Address of output buffer where the encrypted data
 *		to be updated
 * @param	Size - Size of data to be encrypted in bytes where number of
 * 		bytes provided should be multiples of 4
 * @param	GcmTagAddr - Address to the buffer of GCM tag
 *
 * @return 	- XST_SUCCESS - On success
 *		- XSECURE_AES_INVALID_PARAM - On invalid parameter
 *		- XSECURE_AES_STATE_MISMATCH_ERROR - If State mismatch is occurred
 *		- XST_FAILURE - On failure
 *
 ******************************************************************************/
int XSecure_AesEncryptData(XSecure_AesKeySource KeySrc, u32 KeySize, u64 IvAddr,
	u64 InDataAddr, u64 OutDataAddr, u32 Size, u64 GcmTagAddr)
{
	volatile int Status = XST_FAILURE;

	Status = XSecure_AesEncryptInit(KeySrc, KeySize, IvAddr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_AesEncryptUpdate(InDataAddr, OutDataAddr, Size, TRUE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_AesEncryptFinal(GcmTagAddr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function calls IPI request to decrypt a single block of data.
 *
 * @param	KeySrc - Type of the key
 * @param	KeySize - Size of the key
 * @param	IvAddr - Address of the IV
 * @param	InDataAddr - Address of the encrypted data which needs to be
 *		decrypted
 * @param	OutDataAddr - Address of buffer where the decrypted data to be
 *		updated
 * @param	Size - Size of input data to be decrypted
 * @param	GcmTagAddr - Address to the buffer of GCM tag
 *
 *
 * @return 	- XST_SUCCESS - On success
 *		- XSECURE_AES_INVALID_PARAM - On invalid parameter
 *		- XSECURE_AES_STATE_MISMATCH_ERROR - If State mismatch is occurred
 *		- XST_FAILURE - On failure
 *
 ******************************************************************************/
int XSecure_AesDecryptData(XSecure_AesKeySource KeySrc, u32 KeySize, u64 IvAddr,
	u64 InDataAddr, u64 OutDataAddr, u32 Size, u64 GcmTagAddr)
{
	volatile int Status = XST_FAILURE;

	Status = XSecure_AesDecryptInit(KeySrc, KeySize, IvAddr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_AesDecryptUpdate(InDataAddr, OutDataAddr, Size, TRUE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_AesDecryptFinal(GcmTagAddr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	return Status;
}
