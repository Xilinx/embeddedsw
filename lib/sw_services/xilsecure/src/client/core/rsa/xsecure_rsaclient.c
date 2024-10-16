/******************************************************************************
* Copyright (c) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
* @file xsecure_rsaclient.c
*
* This file contains the implementation of the client interface functions for
* RSA driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kal  03/23/21 Initial release
* 4.5   kal  03/23/20 Updated file version to sync with library version
* 4.6   kpt  09/27/21 Fixed compilation warnings
* 4.7   kpt  11/29/21 Replaced Xil_DCacheFlushRange with
*                      XSecure_DCacheFlushRange
*       kpt  01/13/21 Allocated CDO structure's in shared memory set by the
*                     user
*       am   03/08/22 Fixed MISRA C violations
*       kpt  03/16/22 Removed IPI related code and added mailbox support
* 5.0   kpt  07/24/22 Moved XSecure_RsaKat into xsecure_katclient.c
* 5.2   am   03/09/23 Replaced xsecure payload lengths with xmailbox payload lengths
*	yog  05/04/23 Fixed HIS COMF violations
* 5.4   yog  04/29/24 Fixed doxygen warnings.
*       pre  08/29/24 APIs are updated for SSIT support
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_rsa_client_apis XilSecure RSA Client APIs
* @{
*/
/***************************** Include Files *********************************/
#include "xsecure_rsaclient.h"

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to Perform RSA decryption with
 * 		private key.
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	KeyAddr		Address of the Key
 * @param	InDataAddr	Address of the data which has to be decrypted
 * @param	Size		Key size in bytes, Input size also should be
 * 				same as key size mentioned. Inputs supported are
 *				- XSECURE_RSA_4096_KEY_SIZE,
 *				- XSECURE_RSA_2048_KEY_SIZE
 *				- XSECURE_RSA_3072_KEY_SIZE
 * @param	OutDataAddr	Address of the buffer where resultant decrypted
 *				data to be stored
 *
 * @return
 *		 - XST_SUCCESS  If the update is successful
 *		 - XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XSecure_RsaPrivateDecrypt(XSecure_ClientInstance *InstancePtr, const u64 KeyAddr,
				const u64 InDataAddr, const u32 Size, const u64 OutDataAddr)
{
	volatile int Status = XST_FAILURE;
	XSecure_RsaInParam *RsaParams = NULL;
	u64 BufferAddr;
	u32 MemSize;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_5U];

	/**
	 * Perform input parameter validation on InstancePtr. Return XST_FAILURE if input parameters are invalid
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/**
	 * Link shared memory of size RsaParams to RsaParams structure for IPI usage.
	 * Validates the size of the shared memory whether the required size is available or not.
	 */
	MemSize = XMailbox_GetSharedMem(InstancePtr->MailboxPtr, (u64**)(UINTPTR)&RsaParams);

	if ((RsaParams == NULL) || (MemSize < sizeof(XSecure_RsaInParam))) {
		goto END;
	}

	RsaParams->KeyAddr = KeyAddr;
	RsaParams->DataAddr = InDataAddr;
	RsaParams->Size = Size;
	BufferAddr = (u64)(UINTPTR)RsaParams;

	XSecure_DCacheFlushRange(RsaParams, sizeof(XSecure_RsaInParam));

	/* Fill IPI Payload */
	Payload[0U] = HEADER(0, (InstancePtr->SlrIndex << XSECURE_SLR_INDEX_SHIFT)
				| XSECURE_API_RSA_PRIVATE_DECRYPT);
	Payload[1U] = (u32)BufferAddr;
	Payload[2U] = (u32)(BufferAddr >> 32);
	Payload[3U] = (u32)(OutDataAddr);
	Payload[4U] = (u32)(OutDataAddr >> 32);

	/**
	 * Send an IPI request to the PLM by using the CDO command to call XSecure_RsaDecrypt
	 * API and returns the status of the IPI response.
	 */
	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to Perform RSA encryption with
 * 		public key.
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	KeyAddr		Address of the Key
 * @param	InDataAddr	Address of the data which has to be encrypted
 * 				with public key
 * @param	Size		Key size in bytes, Input size also should be
 * 				same as key size mentioned. Inputs supported are
 *				- XSECURE_RSA_4096_KEY_SIZE,
 *				- XSECURE_RSA_2048_KEY_SIZE
 *				- XSECURE_RSA_3072_KEY_SIZE
 * @param	OutDataAddr	Address of the buffer where resultant decrypted
 *							data to be stored
 *
 * @return
 *		 - XST_SUCCESS  If encryption was successful
 *		 - XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XSecure_RsaPublicEncrypt(XSecure_ClientInstance *InstancePtr, const u64 KeyAddr, const u64 InDataAddr,
				const u32 Size, const u64 OutDataAddr)
{
	volatile int Status = XST_FAILURE;
	XSecure_RsaInParam *RsaParams = NULL;
	u64 BufferAddr;
	u32 MemSize;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_5U];

	/**
	 * Perform input parameter validation on InstancePtr. Return XST_FAILURE if input parameters are invalid
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/**
	 * Link shared memory of size RsaParams to RsaParams structure for IPI usage.
	 * Validates the size of the shared memory whether the required size is available or not.
	 */
	MemSize = XMailbox_GetSharedMem(InstancePtr->MailboxPtr, (u64**)(UINTPTR)&RsaParams);

	if ((RsaParams == NULL) || (MemSize < sizeof(XSecure_RsaInParam))) {
		goto END;
	}

	RsaParams->KeyAddr = KeyAddr;
	RsaParams->DataAddr = InDataAddr;
	RsaParams->Size = Size;
	BufferAddr = (u64)(UINTPTR)RsaParams;

	XSecure_DCacheFlushRange(RsaParams, sizeof(XSecure_RsaInParam));

	/* Fill IPI Payload */
	Payload[0U] = HEADER(0, (InstancePtr->SlrIndex << XSECURE_SLR_INDEX_SHIFT)
				| XSECURE_API_RSA_PUBLIC_ENCRYPT);
	Payload[1U] = (u32)BufferAddr;
	Payload[2U] = (u32)(BufferAddr >> 32);
	Payload[3U] = (u32)(OutDataAddr);
	Payload[4U] = (u32)(OutDataAddr >> 32);

	/**
	 * Send an IPI request to the PLM by using the CDO command to call XSecure_RsaEncrypt
	 * API and returns the status of the IPI response.
	 */
	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to Perform RSA sign verification
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	SignAddr 	Address of the buffer which holds the
 * 				decrypted RSA signature.
 * @param	HashAddr	Address of the HashAddr which has the
 * 				hash calculated on the data to be authenticated
 * @param	Size		Length of Hash used
 * 				For SHA3 it should be 48 bytes
 *
 * @return
 *		 - XST_SUCCESS  If decryption was successful
 *		 - XST_FAILURE  In case of mismatch
 *
 *****************************************************************************/
int XSecure_RsaSignVerification(XSecure_ClientInstance *InstancePtr, const u64 SignAddr, const u64 HashAddr,
								const u32 Size)
{
	volatile int Status = XST_FAILURE;
	XSecure_RsaSignParams *SignParams = NULL;
	u64 BufferAddr;
	u32 MemSize;
	volatile u32 Payload[XMAILBOX_PAYLOAD_LEN_3U] = {0U};

	/**
	 * Perform input parameter validation on InstancePtr. Return XST_FAILURE if input parameters are invalid
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/**
	 * Link shared memory of size SignParams to SignParams structure for IPI usage.
	 * Validates the size of the shared memory whether the required size is available or not.
	 */
	MemSize = XMailbox_GetSharedMem(InstancePtr->MailboxPtr, (u64**)(UINTPTR)&SignParams);

	if ((SignParams == NULL) || (MemSize < sizeof(XSecure_RsaSignParams))) {
		goto END;
	}

	SignParams->SignAddr = SignAddr;
	SignParams->HashAddr = HashAddr;
	SignParams->Size = Size;
	BufferAddr = (u64)(UINTPTR)SignParams;

	XSecure_DCacheFlushRange(SignParams, sizeof(XSecure_RsaSignParams));

	/* Fill IPI Payload */
	Payload[0U] = HEADER(0, (InstancePtr->SlrIndex << XSECURE_SLR_INDEX_SHIFT)
				| XSECURE_API_RSA_SIGN_VERIFY);
	Payload[1U] = (u32)BufferAddr;
	Payload[2U] = (u32)(BufferAddr >> 32);

	/**
	 * Send an IPI request to the PLM by using the CDO command to call XSecure_RsaSignVerify
	 * API and returns the status of the IPI response.
	 */
	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}
/** @} */
