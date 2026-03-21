/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xsecure_mldsaclient.c
*
* This file contains the implementation of the client interface functions for MLDSA driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 5.7   tvp  03/03/26 Initial release
*
* </pre>
*
***************************************************************************************************/
/**
* @addtogroup xsecure_mldsa_client_apis XilSecure MLDSA Client APIs
* @{
*/
/**************************************** Include Files *******************************************/
#include "xsecure_mldsaclient.h"

/**************************************************************************************************/
/**
 * @brief	This function sends IPI request to verify MLDSA signature for a given data
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	MldsaParamAddr	Address of XSecure_MldsaSignVerifyParams structure
 *
 * @return
 *		 - XST_SUCCESS  On success
 *		 - XST_FAILURE  On failure
 *
 **************************************************************************************************/
int XSecure_MldsaSignVerifyClient(XSecure_ClientInstance *InstancePtr, u64 MldsaParamAddr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT] = {0U};

	/**
	 * Perform input parameter validation on InstancePtr. Return XST_FAILURE if input parameters
	 * are invalid
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/* Fill IPI Payload */
	XSECURE_PACK_PAYLOAD2(Payload, ((InstancePtr->SlrIndex << XSECURE_SLR_INDEX_SHIFT)
				| XSECURE_API_MLDSA_SIGN_VERIFY),
				MldsaParamAddr,
				(MldsaParamAddr >> XSECURE_ADDR_HIGH_SHIFT));

	/**
	 * Send an IPI request to the PLM by using the CDO command to call XSecure_MldsaSignVerify
	 * API and returns the status of the IPI response.
	 */
	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)Payload, PAYLOAD_ARG_CNT);

END:
	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function sends IPI request to generate MLDSA signature for a given data.
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	MldsaParamAddr	Address of XSecure_MldsaSignGenParams structure
 *
 * @return
 *		 - XST_SUCCESS  On success
 *		 - XST_FAILURE  On failure
 *
 **************************************************************************************************/
int XSecure_MldsaSignGenerateClient(XSecure_ClientInstance *InstancePtr, u64 MldsaParamAddr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT] = {0U};

	/**
	 * Perform input parameter validation on InstancePtr. Return XST_FAILURE if input parameters
	 * are invalid
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/* Fill IPI Payload */
	XSECURE_PACK_PAYLOAD2(Payload, ((InstancePtr->SlrIndex << XSECURE_SLR_INDEX_SHIFT)
				| XSECURE_API_MLDSA_SIGN_GENERATE),
				MldsaParamAddr,
				(MldsaParamAddr >> XSECURE_ADDR_HIGH_SHIFT));

	/**
	 * Send an IPI request to the PLM by using the CDO command to call XSecure_MldsaSignGenerate
	 * API and returns the status of the IPI response.
	 */
	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, PAYLOAD_ARG_CNT);

END:
	return Status;
}
/** @} */
