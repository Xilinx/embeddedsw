/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_plat_aes_client.c
*
* This file contains the implementation of AES client interface APIs for
* Versal Net.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 5.3   har  02/05/24 Initial release
*
* </pre>
* @note
*
******************************************************************************/
/**
* @addtogroup xsecure_aes_client_apis XilSecure AES Client APIs
* @{
*/
/***************************** Include Files *********************************/
#include "xsecure_plat_aes_client.h"
#include "xsecure_defs.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
 *
 * @brief	This function calls IPI request to perform below operation:
 *			- Write key into the key source provided by the user
 *			- Encrypt/decrypt a single block of data using the provided key
 *			- Zeroize the key once the AES operation is done
 *
 * @param	InstancePtr	Pointer to the XSecure_ClientInstance
 * @param	KeyAddr		Address of the key to be used for AES operation
 * @param 	AesDataParams	Pointer to the XSecure_AesDataBlockParams structure variable
 *
 * @return
 *		 - XST_SUCCESS  On success
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
int XSecure_AesPerformOperationAndZeroizeKey(XSecure_ClientInstance *InstancePtr, u64 KeyAddr, const XSecure_AesDataBlockParams *AesDataParams)
{
	volatile int Status = XST_FAILURE;
	XSecure_AesDataBlockParams *AesParams = NULL;
	u64 Buffer;
	u32 MemSize;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_5U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	MemSize = XMailbox_GetSharedMem(InstancePtr->MailboxPtr, (u64**)(UINTPTR)&AesParams);
	if ((AesParams == NULL) || (MemSize < sizeof(XSecure_AesDataBlockParams))) {
		goto END;
	}

	AesParams->IvAddr = AesDataParams->IvAddr;
	AesParams->OperationId = AesDataParams->OperationId;
	AesParams->KeySrc = AesDataParams->KeySrc;
	AesParams->KeySize = AesDataParams->KeySize;

	AesParams->AadAddr = AesDataParams->AadAddr;
	AesParams->AadSize = AesDataParams->AadSize;
	AesParams->IsUpdateAadEn = AesDataParams->IsUpdateAadEn;
	AesParams->IsLast = TRUE;

	if (AesDataParams->IsGmacEnable == TRUE) {
		AesParams->IsGmacEnable = TRUE;
	}
	else {
		AesParams->InDataAddr = AesDataParams->InDataAddr;
		AesParams->Size = AesDataParams->Size;
		AesParams->OutDataAddr = AesDataParams->OutDataAddr;
		AesParams->IsGmacEnable = FALSE;
	}

	AesParams->GcmTagAddr = AesDataParams->GcmTagAddr;

	Buffer = (u64)(UINTPTR)AesParams;

	XSecure_DCacheFlushRange(AesParams, sizeof(XSecure_AesDataBlockParams));

	/* Fill IPI Payload */
	Payload[0U] = HEADER(0U, XSECURE_API_AES_PERFORM_OPERATION_AND_ZEROIZE_KEY);
	Payload[1U] = (u32)Buffer;
	Payload[2U] = (u32)(Buffer >> XSECURE_ADDR_HIGH_SHIFT);
	Payload[3U] = (u32)KeyAddr;
	Payload[4U] = (u32)(KeyAddr >> XSECURE_ADDR_HIGH_SHIFT);

	/**
	 * Send an IPI request to the PLM by using the CDO command to call XSecure_AesOpNZeroizeKey
	 * API and returns the status of the IPI response.
	 */
	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}
/** @} */
