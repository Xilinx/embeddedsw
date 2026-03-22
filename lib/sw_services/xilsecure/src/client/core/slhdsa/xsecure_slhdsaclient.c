/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xsecure_slhdsaclient.c
*
* This file contains the implementation of the client interface functions for SLHDSA driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 5.7   tvp  03/20/26 Initial release
*
* </pre>
*
***************************************************************************************************/
/**
* @addtogroup xsecure_slhdsa_client_apis XilSecure SLHDSA Client APIs
* @{
*/
/**************************************** Include Files *******************************************/
#include "xsecure_slhdsaclient.h"

/**************************************************************************************************/
/**
 * @brief	This function sends IPI request to verify SLHDSA signature for a given data
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	SlhdsaParamAddr	Address of XSecure_SlhdsaInputParams structure
 *
 * @return
 *		 - XST_SUCCESS  On success
 *		 - XST_FAILURE  On failure
 *
 **************************************************************************************************/
int XSecure_SlhdsaSignVerifyClient(XSecure_ClientInstance *InstancePtr, u64 SlhdsaParamAddr)
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
				| XSECURE_API_SLHDSA_SIGN_VERIFY),
				SlhdsaParamAddr,
				(SlhdsaParamAddr >> XSECURE_ADDR_HIGH_SHIFT));

	/**
	 * Send an IPI request to the PLM by using the CDO command to call XSecure_SlhdsaVerify
	 * API and returns the status of the IPI response.
	 */
	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, PAYLOAD_ARG_CNT);

END:
	return Status;
}
/** @} */
