/******************************************************************************
* Copyright (c) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
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
* 4.7   kpt  11/29/21 Replaced Xil_DCacheFlushRange with
*                     XSecure_DCacheFlushRange
*       kpt  01/13/21 Allocated CDO structure's in shared memory set by the
*                     user
*       am   03/08/22 Fixed MISRA C violations
*       kpt  03/16/22 Removed IPI related code and added mailbox support
* 5.0   kpt  07/24/22 Moved XSecure_AesDecryptKat and XSecure_AesDecryptCMKat
*                     into XSecure_Katclient.c
*       kpt  08/19/22 Added GMAC support
* 5.1   skg  12/14/22 Added SSIT Provisioning support
*       am   03/09/23 Replaced xsecure payload lengths with xmailbox payload lengths
*
* </pre>
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_aesclient.h"

/************************** Constant Definitions *****************************/
#define XSECURE_SLR_INDEX_SHIFT (6U)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief       This function sends IPI request to initialize the AES engine
 *
 * @param	InstancePtr	Pointer to the client instance
 * @return
 *	-	XST_SUCCESS - If the initialization is successful
 * 	-	XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XSecure_AesInitialize(XSecure_ClientInstance *InstancePtr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_1U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/* Fill IPI Payload */
	Payload[0U] = HEADER(0U, (InstancePtr->SlrIndex << XSECURE_SLR_INDEX_SHIFT) | XSECURE_API_AES_INIT);

	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr ,Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to EncryptInit the AES engine
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	KeySrc		Type of the Key
 * @param	Size		Size of the Key
 * @param	IvAddr		Address of the IV
 *
 * @return
 *	-	XST_SUCCESS - If the Encrypt init is successful
 *	-	XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XSecure_AesEncryptInit(XSecure_ClientInstance *InstancePtr, XSecure_AesKeySource KeySrc, u32 Size, u64 IvAddr)
{
	volatile int Status = XST_FAILURE;
	XSecure_AesInitOps *AesParams = NULL;
	u64 Buffer;
	u32 MemSize;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_3U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	MemSize = XMailbox_GetSharedMem(InstancePtr->MailboxPtr, (u64**)(UINTPTR)&AesParams);
	if ((AesParams == NULL) || (MemSize < sizeof(XSecure_AesInitOps))) {
		goto END;
	}

	AesParams->IvAddr = IvAddr;
	AesParams->OperationId = (u32)XSECURE_ENCRYPT;
	AesParams->KeySrc = (u32)KeySrc;
	AesParams->KeySize = Size;
	Buffer = (u64)(UINTPTR)AesParams;

	XSecure_DCacheFlushRange(AesParams, sizeof(XSecure_AesInitOps));

	/* Fill IPI Payload */
	Payload[0U] = HEADER(0U, (InstancePtr->SlrIndex << XSECURE_SLR_INDEX_SHIFT) | XSECURE_API_AES_OP_INIT);
	Payload[1U] = (u32)Buffer;
	Payload[2U] = (u32)(Buffer >> 32U);

	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to DecryptInit the AES engine
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	KeySrc		Type of the Key
 * @param	Size		Size of the Key
 * @param	IvAddr		Address of the IV
 *
 * @return
 *	-	XST_SUCCESS - If the Decrypt init is successful
 *	-	XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XSecure_AesDecryptInit(XSecure_ClientInstance *InstancePtr, XSecure_AesKeySource KeySrc, u32 Size, u64 IvAddr)
{
	volatile int Status = XST_FAILURE;
	XSecure_AesInitOps *AesParams = NULL;
	u64 Buffer;
	u32 MemSize;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_3U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	MemSize = XMailbox_GetSharedMem(InstancePtr->MailboxPtr, (u64**)(UINTPTR)&AesParams);
	if ((AesParams == NULL) || (MemSize < sizeof(XSecure_AesInitOps))) {
		goto END;
	}

	AesParams->IvAddr = IvAddr;
	AesParams->OperationId = (u32)XSECURE_DECRYPT;
	AesParams->KeySrc = (u32)KeySrc;
	AesParams->KeySize = Size;
	Buffer = (u64)(UINTPTR)AesParams;

	XSecure_DCacheFlushRange(AesParams, sizeof(XSecure_AesInitOps));

	/* Fill IPI Payload */
	Payload[0U] = HEADER(0U, (InstancePtr->SlrIndex << XSECURE_SLR_INDEX_SHIFT) | XSECURE_API_AES_OP_INIT);
	Payload[1U] = (u32)Buffer;
	Payload[2U] = (u32)(Buffer >> 32U);

	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to update AAD to AES engine
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	AadAddr		Address of the Aad
 * @param	AadSize		Size of the Aad data
 *
 * @return
 *	-	XST_SUCCESS - If the Aad update is successful
 *	-	XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XSecure_AesUpdateAad(XSecure_ClientInstance *InstancePtr, u64 AadAddr, u32 AadSize)
{
	volatile int Status = XST_FAILURE;

	Status = XSecure_AesGmacUpdateAad(InstancePtr, AadAddr, AadSize, FALSE);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to update AAD data to AES engine
 *
 * @param	InstancePtr		Pointer to the client instance
 * @param	AadAddr			Address of the Aad
 * @param	AadSize			Size of the Aad data
 * @param   IsLastChunkSrc	If this is the last update of data, this parameter
 *                          should be set to TRUE otherwise FALSE
 *
 * @return
 *	-	XST_SUCCESS - If the Aad update is successful
 *	-	XST_FAILURE - If there is a failure
 *
 * @note
 *      To generate GMAC, this API must be called by setting IsLastChunkSrc as TRUE for
 *      the last update followed by XSecure_AesEncryptFinal or XSecure_AesDecryptFinal API
 *      call to generate or validate GMAC tag
 *
 ******************************************************************************/
int XSecure_AesGmacUpdateAad(XSecure_ClientInstance *InstancePtr, u64 AadAddr, u32 AadSize, u32 IsLastChunkSrc)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_5U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/* Fill IPI Payload */
	Payload[0U] = HEADER(0U, (InstancePtr->SlrIndex << XSECURE_SLR_INDEX_SHIFT) | XSECURE_API_AES_UPDATE_AAD);
	Payload[1U] = (u32)AadAddr;
	Payload[2U] = (u32)(AadAddr >> 32U);
	Payload[3U] = (u32)AadSize;
	Payload[4U] = IsLastChunkSrc;

	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to update the input data to
 * 		AES engine for encryption
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	InDataAddr	Address of the input data which needs to be
 * 				encrypted
 * @param	OutDataAddr	Address of the buffer where the encrypted data
 * 				to be updated
 * @param	Size		Size of the input data to be encrypted
 * @param	IsLast		If this is the last update of data to be
 * 				encrypted, this parameter should be set to TRUE
 * 				otherwise FALSE
 *
 * @return
 *	-	XST_SUCCESS - On successful encryption of the data
 *	-	XSECURE_AES_INVALID_PARAM - On invalid parameter
 *	-	XSECURE_AES_STATE_MISMATCH_ERROR - If there is state mismatch
 *	-	XST_FAILURE - On failure
 *
 *****************************************************************************/
int XSecure_AesEncryptUpdate(XSecure_ClientInstance *InstancePtr, u64 InDataAddr,
				u64 OutDataAddr, u32 Size, u32 IsLast)
{
	volatile int Status = XST_FAILURE;
	XSecure_AesInParams *EncInAddr = NULL;
	u64 SrcAddr;
	u32 MemSize;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_5U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	MemSize = XMailbox_GetSharedMem(InstancePtr->MailboxPtr, (u64**)(UINTPTR)&EncInAddr);

	if ((EncInAddr == NULL) || (MemSize < sizeof(XSecure_AesInParams))) {
		goto END;
	}

	EncInAddr->InDataAddr = InDataAddr;
	EncInAddr->Size = Size;
	EncInAddr->IsLast = IsLast;
	SrcAddr = (u64)(UINTPTR)EncInAddr;

	XSecure_DCacheFlushRange(EncInAddr, sizeof(XSecure_AesInParams));

	/* Fill IPI Payload */
	Payload[0U] = HEADER(0U, (InstancePtr->SlrIndex << XSECURE_SLR_INDEX_SHIFT) | XSECURE_API_AES_ENCRYPT_UPDATE);
	Payload[1U] = (u32)SrcAddr;
	Payload[2U] = (u32)(SrcAddr >> 32U);
	Payload[3U] = (u32)(OutDataAddr);
	Payload[4U] = (u32)(OutDataAddr >> 32U);

	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to update the GcmTag Addr to
 * 		AES engine
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	GcmTagAddr	Address to the buffer of GCM tag size,
 * 				where the API updates GCM tag
 *
 * @return
 *	-	XST_SUCCESS - On successful encryption of the data
 *	-	XSECURE_AES_INVALID_PARAM - On invalid parameter
 *	-	XSECURE_AES_STATE_MISMATCH_ERROR - If there is state mismatch
 *	-	XST_FAILURE - On failure
 *
 *****************************************************************************/
int XSecure_AesEncryptFinal(XSecure_ClientInstance *InstancePtr, u64 GcmTagAddr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_3U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	Payload[0U] = HEADER(0U, (InstancePtr->SlrIndex << XSECURE_SLR_INDEX_SHIFT) | XSECURE_API_AES_ENCRYPT_FINAL);
	Payload[1U] = (u32)GcmTagAddr;
	Payload[2U] = (u32)(GcmTagAddr >> 32);

	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to update the encrypted data to
 * 		AES engine for decryption
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	InDataAddr	Address of the encryped data which needs to be
 * 				decrypted
 * @param	OutDataAddr	Address of the buffer where the decrypted data
 * 				to be updated
 * @param	Size		Size of the input data to be decrypted
 * @param	IsLast		If this is the last update of data to be
 * 				decrypted, this parameter should be set to TRUE
 * 				otherwise FALSE
 *
 * @return
 *	-	XST_SUCCESS - On successful decryption of the data
 *	-	XSECURE_AES_INVALID_PARAM - On invalid parameter
 *	-	XSECURE_AES_STATE_MISMATCH_ERROR - If there is state mismatch
 *	-	XST_FAILURE - On failure
 *
 *****************************************************************************/
int XSecure_AesDecryptUpdate(XSecure_ClientInstance *InstancePtr, u64 InDataAddr,
				u64 OutDataAddr, u32 Size, u32 IsLast)
{
	volatile int Status = XST_FAILURE;
	XSecure_AesInParams *DecInParams = NULL;
	u64 SrcAddr;
	u32 MemSize;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_5U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	MemSize = XMailbox_GetSharedMem(InstancePtr->MailboxPtr, (u64**)(UINTPTR)&DecInParams);

	if ((DecInParams == NULL) || (MemSize < sizeof(XSecure_AesInParams))) {
		goto END;
	}

	DecInParams->InDataAddr = InDataAddr;
	DecInParams->Size = Size;
	DecInParams->IsLast = IsLast;
	SrcAddr = (u64)(UINTPTR)DecInParams;

	XSecure_DCacheFlushRange(DecInParams, sizeof(XSecure_AesInParams));

	/* Fill IPI Payload */
	Payload[0U] = HEADER(0U, (InstancePtr->SlrIndex << XSECURE_SLR_INDEX_SHIFT) | XSECURE_API_AES_DECRYPT_UPDATE);
	Payload[1U] = (u32)SrcAddr;
	Payload[2U] = (u32)(SrcAddr >> 32);
	Payload[3U] = (u32)(OutDataAddr);
	Payload[4U] = (u32)(OutDataAddr >> 32);

	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function sends IPI request to verify the GcmTag provided
 * 		for the data decrypted till the point
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	GcmTagAddr	Address of a buffer which should holds GCM Tag
 *
 * @return
 *	-	XST_SUCCESS - On successful encryption of the data
 *	-	XSECURE_AES_GCM_TAG_MISMATCH - User provided GCM tag does not
 *	 				match calculated tag
 *	-	XSECURE_AES_INVALID_PARAM - On invalid parameter
 *	-	XSECURE_AES_STATE_MISMATCH_ERROR - If there is state mismatch
 *	-	XST_FAILURE - On failure
 *
 *****************************************************************************/
int XSecure_AesDecryptFinal(XSecure_ClientInstance *InstancePtr, u64 GcmTagAddr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_3U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	Payload[0U] = HEADER(0U, (InstancePtr->SlrIndex << XSECURE_SLR_INDEX_SHIFT) | XSECURE_API_AES_DECRYPT_FINAL);
	Payload[1U] = (u32)GcmTagAddr;
	Payload[2U] = (u32)(GcmTagAddr >> 32);

	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to zeroize selected AES
 * 		key storage register
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	KeySrc		Select the key source which needs to be zeroized
 *
 * @return
 *	-	XST_SUCCESS -  When key zeroization is success
 *	-	XSECURE_AES_INVALID_PARAM - On invalid parameter
 *	-	XSECURE_AES_STATE_MISMATCH_ERROR - If there is state mismatch
 *	-	XST_FAILURE - On failure
 *
 ******************************************************************************/
int XSecure_AesKeyZero(XSecure_ClientInstance *InstancePtr, XSecure_AesKeySource KeySrc)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_2U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	Payload[0U] = HEADER(0U, (InstancePtr->SlrIndex << XSECURE_SLR_INDEX_SHIFT) | XSECURE_API_AES_KEY_ZERO);
	Payload[1U] = KeySrc;

	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to write the key provided
 * 		into the specified AES key registers
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	KeySrc		Key Source to be selected to which provided
 * 						key should be updated
 * @param	Size		Size of the input key to be written
 * @param	KeyAddr		Address of a buffer which should contain the key
 * 						to be written
 *
 * @return
 *	-	XST_SUCCESS - On successful key written on AES registers
 *	-	XSECURE_AES_INVALID_PARAM - On invalid parameter
 *	-	XSECURE_AES_STATE_MISMATCH_ERROR - If there is state mismatch
 *	-	XST_FAILURE - On failure
 *
 ******************************************************************************/
int XSecure_AesWriteKey(XSecure_ClientInstance *InstancePtr, XSecure_AesKeySource KeySrc,
								u32 Size, u64 KeyAddr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_5U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/* Fill IPI Payload */
	Payload[0U] = HEADER(0U, (InstancePtr->SlrIndex << XSECURE_SLR_INDEX_SHIFT) | XSECURE_API_AES_WRITE_KEY);
	Payload[1U] = Size;
	Payload[2U] = KeySrc;
	Payload[3U] = (u32)KeyAddr;
	Payload[4U] = (u32)(KeyAddr >> 32U);

	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to decrypt the key in
 * 		KEK key form
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	IVAddr		Address of IV holding buffer for decryption
 *				of the key
 * @param	DecKeySrc	Select key source which holds KEK and
 * 				needs to be decrypted
 * @param	DstKeySrc	Select the key in which decrypted red key
 * 				should be updated
 * @param	Size		Size of the key
 *
 * @return
 *	-	XST_SUCCESS - On successful key decryption
 *	-	XSECURE_AES_INVALID_PARAM - On invalid parameter
 *	-	XST_FAILURE - If timeout has occurred
 *
 ******************************************************************************/
int XSecure_AesKekDecrypt(XSecure_ClientInstance *InstancePtr, u64 IvAddr,
				XSecure_AesKeySource DstKeySrc, XSecure_AesKeySource DecKeySrc,
				XSecure_AesKeySize Size)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_4U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	if ((Size != XSECURE_AES_KEY_SIZE_128) &&
		 (Size != XSECURE_AES_KEY_SIZE_256)) {
		Status = XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	/* Fill IPI Payload */
	Payload[0U] = HEADER(0U, (InstancePtr->SlrIndex << XSECURE_SLR_INDEX_SHIFT) | XSECURE_API_AES_KEK_DECRYPT);
	Payload[1U] = (((u32)Size << 16) | ((u32)DstKeySrc << 8) | DecKeySrc);
	Payload[2U] = (u32)IvAddr;
	Payload[3U] = (u32)(IvAddr >> 32);

	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}
/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to enable/disable DpaCm in AES
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	DpaCmCfg    User choice to enable/disable DPA CM
 *
 * @return
 *	-	XST_SUCCESS - If configuration is success
 *	-	XSECURE_AES_INVALID_PARAM	- For invalid parameter
 *	-	XSECURE_AES_STATE_MISMATCH_ERROR - If there is state mismatch
 *	-	XSECURE_AES_DPA_CM_NOT_SUPPORTED - If DPA CM is disabled on chip
 * 		(Enabling/Disabling in AES engine does not impact functionality)
 *
 *
 ******************************************************************************/
int XSecure_AesSetDpaCm(XSecure_ClientInstance *InstancePtr, u8 DpaCmCfg)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_2U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/* Fill IPI Payload */
	Payload[0U] = HEADER(0U, (InstancePtr->SlrIndex << XSECURE_SLR_INDEX_SHIFT) | XSECURE_API_AES_SET_DPA_CM);
	Payload[1U] = DpaCmCfg;

	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function calls IPI request to encrypt a single block of data.
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	KeySrc 		Type of the key
 * @param	KeySize 	Size of the key
 * @param	IvAddr  	Address of the IV
 * @param	InDataAddr  Address of the data which needs to be encrypted
 * @param	OutDataAddr	Address of output buffer where the encrypted data
 *						to be updated
 * @param	Size		Size of data to be encrypted in bytes where number of
 * 						bytes provided should be multiples of 4
 * @param	GcmTagAddr  Address to the buffer of GCM tag
 *
 * @return
 *	-	XST_SUCCESS - On success
 *	-	XSECURE_AES_INVALID_PARAM - On invalid parameter
 *	-	XSECURE_AES_STATE_MISMATCH_ERROR - If State mismatch is occurred
 *	-	XST_FAILURE - On failure
 *
 ******************************************************************************/
int XSecure_AesEncryptData(XSecure_ClientInstance *InstancePtr, XSecure_AesKeySource KeySrc, u32 KeySize, u64 IvAddr,
	u64 InDataAddr, u64 OutDataAddr, u32 Size, u64 GcmTagAddr)
{
	volatile int Status = XST_FAILURE;
	XSecure_AesDataBlockParams *AesParams = NULL;
	u64 Buffer;
	u32 MemSize;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_3U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	MemSize = XMailbox_GetSharedMem(InstancePtr->MailboxPtr, (u64**)(UINTPTR)&AesParams);
	if ((AesParams == NULL) || (MemSize < sizeof(XSecure_AesDataBlockParams))) {
		goto END;
	}

	/**<AES Init operation*/
	AesParams->IvAddr = IvAddr;
	AesParams->OperationId = (u32)XSECURE_ENCRYPT;
	AesParams->KeySrc = (u32)KeySrc;
	AesParams->KeySize = KeySize;

	/**<AES Encrypt Update*/
	AesParams->InDataAddr = InDataAddr;
	AesParams->Size = Size;
	AesParams->IsLast = TRUE;
	AesParams->OutDataAddr = OutDataAddr;

	/**<AES Encrypt Final*/
	AesParams->GcmTagAddr = GcmTagAddr;

	Buffer = (u64)(UINTPTR)AesParams;

	XSecure_DCacheFlushRange(AesParams, sizeof(XSecure_AesDataBlockParams));

	/* Fill IPI Payload */
	Payload[0U] = HEADER(0U, (InstancePtr->SlrIndex << XSECURE_SLR_INDEX_SHIFT) | XSECURE_API_AES_PERFORM_OPERATION);
	Payload[1U] = (u32)Buffer;
	Payload[2U] = (u32)(Buffer >> 32U);

	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function calls IPI request to decrypt a single block of data.
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	KeySrc  	Type of the key
 * @param	KeySize  	Size of the key
 * @param	IvAddr  	Address of the IV
 * @param	InDataAddr  Address of the encrypted data which needs to be
 *						decrypted
 * @param	OutDataAddr	Address of buffer where the decrypted data to be
 *						updated
 * @param	Size  		Size of input data to be decrypted
 * @param	GcmTagAddr  Address to the buffer of GCM tag
 *
 * @return
 *	-	XST_SUCCESS - On success
 *	-	XSECURE_AES_INVALID_PARAM - On invalid parameter
 *	-	XSECURE_AES_STATE_MISMATCH_ERROR - If State mismatch is occurred
 *	-	XST_FAILURE - On failure
 *
 ******************************************************************************/
int XSecure_AesDecryptData(XSecure_ClientInstance *InstancePtr, XSecure_AesKeySource KeySrc, u32 KeySize, u64 IvAddr,
	u64 InDataAddr, u64 OutDataAddr, u32 Size, u64 GcmTagAddr)
{
	volatile int Status = XST_FAILURE;
	XSecure_AesDataBlockParams *AesParams = NULL;
	u64 Buffer;
	u32 MemSize;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_3U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	MemSize = XMailbox_GetSharedMem(InstancePtr->MailboxPtr, (u64**)(UINTPTR)&AesParams);
	if ((AesParams == NULL) || (MemSize < sizeof(XSecure_AesDataBlockParams))) {
		goto END;
	}

	/**<AES Decrypt Init operation*/
	AesParams->IvAddr = IvAddr;
	AesParams->OperationId = (u32)XSECURE_DECRYPT;
	AesParams->KeySrc = (u32)KeySrc;
	AesParams->KeySize = KeySize;

	/**<AES Decrypt Update*/
	AesParams->InDataAddr = InDataAddr;
	AesParams->Size = Size;
	AesParams->IsLast = TRUE;
	AesParams->OutDataAddr = OutDataAddr;

	/**<AES Decrypt Final*/
	AesParams->GcmTagAddr = GcmTagAddr;

	Buffer = (u64)(UINTPTR)AesParams;

	XSecure_DCacheFlushRange(AesParams, sizeof(XSecure_AesDataBlockParams));

	/* Fill IPI Payload */
	Payload[0U] = HEADER(0U, (InstancePtr->SlrIndex << XSECURE_SLR_INDEX_SHIFT) | XSECURE_API_AES_PERFORM_OPERATION);
	Payload[1U] = (u32)Buffer;
	Payload[2U] = (u32)(Buffer >> 32U);

	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}
