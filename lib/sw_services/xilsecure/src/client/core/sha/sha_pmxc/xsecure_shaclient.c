/******************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_shaclient.c
*
* This file contains the implementation of the client interface functions for
* SHA driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kal  03/23/21 Initial release
*       kal  04/06/25 Updated SHA client API's
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_shaclient.h"

/************************** Constant Definitions *****************************/
#define XSECURE_ADDR_HIGH_SHIFT		(32U) /**< Shift to get the higher 32 bit
						address of a 64 bit address.
						*/

/**************************** Type Definitions *******************************/
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*************************************************************************************************/
/**
 * @brief	This function sends IPI request to generate SHA3 digest.
 *
 * @param	InstancePtr		Pointer to the XSecure_ClientInstance structure.
 * @param	Sha3Params		Pointer to the XSecure_ShaOpParams structure which holds
 * 					parameters of SHA input arguments.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to server is sent successfully.
 * 		- XST_INVALID_PARAM, if any argument is invalid.
 * 		- XST_FAILURE, if sending IPI request to server fails.
 *
 *************************************************************************************************/
int XSecure_Sha3Operation(XSecure_ClientInstance *InstancePtr, XSecure_ShaOpParams *Sha3Params)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_3U];
	u64 Sha3ParamsAddr = (u64)(UINTPTR)Sha3Params;

	/**
	 * Perform input parameter validation on InstancePtr. Return XST_FAILURE if input parameters are invalid
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL) || (Sha3Params == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/* Fill IPI Payload */
	Payload[XSECURE_IPI_PAYLOAD_IDX_0] = HEADER(0U, XSECURE_API_SHA3_OPERATION);
	Payload[XSECURE_IPI_PAYLOAD_IDX_1] = (u32)(UINTPTR)Sha3ParamsAddr;
	Payload[XSECURE_IPI_PAYLOAD_IDX_2] = (u32)(UINTPTR)(Sha3ParamsAddr >> XSECURE_ADDR_HIGH_SHIFT);
	/**
	 * Send an IPI request to the PLM by using the CDO command to call XSecure_ShaOperation api.
	 * Wait for IPI response from PLM with a timeout.
	 * If the timeout exceeds then error is returned otherwise it returns the status of the IPI response
	 */
	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, XMAILBOX_PAYLOAD_LEN_3U);
	if (Status != XST_SUCCESS) {
		XSecure_Printf(XSECURE_DEBUG_GENERAL, "Sha3 Operation Failed \r\n");
		goto END;
	}

END:
        return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends IPI request to generate SHA2 digest.
 *
 * @param	InstancePtr		Pointer to the XSecure_ClientInstance structure.
 * @param	Sha2Params		Pointer to the XSecure_ShaOpParams structure which holds
 * 					parameters of SHA input arguments.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to server is sent successfully.
 * 		- XST_INVALID_PARAM, if any argument is invalid.
 * 		- XST_FAILURE, if sending IPI request to server fails.
 *
 *************************************************************************************************/
int XSecure_Sha2Operation(XSecure_ClientInstance *InstancePtr, XSecure_ShaOpParams *Sha2Params)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_3U];
	u64 Sha2ParamsAddr = (u64)(UINTPTR)Sha2Params;

	/**
	 * Perform input parameter validation on InstancePtr. Return XST_FAILURE if input parameters are invalid
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL) || (Sha2Params == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/* Fill IPI Payload */
	Payload[XSECURE_IPI_PAYLOAD_IDX_0] = HEADER(0U, XSECURE_API_SHA2_OPERATION);
	Payload[XSECURE_IPI_PAYLOAD_IDX_1] = (u32)Sha2ParamsAddr;
        Payload[XSECURE_IPI_PAYLOAD_IDX_2] = (u32)(Sha2ParamsAddr >> XSECURE_ADDR_HIGH_SHIFT);
	/**
	 * Send an IPI request to the PLM by using the CDO command to call XSecure_ShaOperation api.
	 * Wait for IPI response from PLM with a timeout.
	 * If the timeout exceeds then error is returned otherwise it returns the status of the IPI response
	 */
	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, XMAILBOX_PAYLOAD_LEN_3U);
	if (Status != XST_SUCCESS) {
		XSecure_Printf(XSECURE_DEBUG_GENERAL, "Sha2 Operation Failed \r\n");
		goto END;
	}

END:
        return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to PLM to perform SHA2 KAT
 *
 * @param	InstancePtr	Pointer to the client instance
 *
 * @return
 *		 - XST_SUCCESS  When KAT Pass
 *		 - XST_INVALID_PARAM  If input parameters are invalid
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
int XSecure_Sha2Kat(XSecure_ClientInstance *InstancePtr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_2U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/* Fill IPI Payload */
	Payload[XSECURE_IPI_PAYLOAD_IDX_0] = HEADER(0U, (InstancePtr->SlrIndex << XSECURE_SLR_INDEX_SHIFT) | XSECURE_API_KAT);
	Payload[XSECURE_IPI_PAYLOAD_IDX_1] = (u32)XSECURE_API_SHA2_KAT;

	/**
	 * Send an IPI request to the PLM by using the CDO command to call XSecure_ShaKat
	 * API and returns the status of the IPI response.
	 */
	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, XMAILBOX_PAYLOAD_LEN_2U);

END:
	return Status;
}
