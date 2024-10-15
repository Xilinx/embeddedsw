/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
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
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_shaclient.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief       This function sends IPI request to initialize the SHA engine
 *
 * @return
 *	-	XST_SUCCESS - If the Sha engine initialization is successful
 * 	-	XST_FAILURE - If the Sha engine initialization is failed
 *
 ******************************************************************************/
int XSecure_ShaInitialize(XSecure_ClientInstance *InstancePtr, XSecure_ShaMode ShaMode)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_6U];

	/**
	 * Perform input parameter validation on InstancePtr. Return XST_FAILURE if input parameters are invalid
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/* Fill IPI Payload */
	Payload[0U] = HEADER(0U, XSECURE_API_SHA_INIT);
	Payload[1U] = (u32)ShaMode;
	Payload[2U] = XSECURE_IPI_UNUSED_PARAM;
        Payload[3U] = XSECURE_IPI_UNUSED_PARAM;
	Payload[4U] = XSECURE_IPI_UNUSED_PARAM;
        Payload[5U] = XSECURE_IPI_UNUSED_PARAM;
	/**
	 * Send an IPI request to the PLM by using the CDO command to call XSecure_ShaOperation api.
	 * Wait for IPI response from PLM with a timeout.
	 * If the timeout exceeds then error is returned otherwise it returns the status of the IPI response
	 */
	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));
	if (Status != XST_SUCCESS) {
		XSecure_Printf(XSECURE_DEBUG_GENERAL, "Sha Init Failed \r\n");
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function sends IPI request to update the SHA engine
 *		with the input data
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	InDataAddr	Address of the output buffer to store the
 * 				output hash
 * @param	Size		Size of the data to be updated to SHA3 engine
 *
 * @return
 *	-	XST_SUCCESS - If the update is successful
 * 	-	XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XSecure_ShaUpdate(XSecure_ClientInstance *InstancePtr, const u64 InDataAddr, u32 Size, u32 EndLast)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_6U];

	/**
	 * Perform input parameter validation on InstancePtr. Return XST_FAILURE if input parameters are invalid
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/* Fill IPI Payload */
	Payload[0U] = HEADER(0U, XSECURE_API_SHA_UPDATE);
	Payload[1U] = (u32)InDataAddr;
	Payload[2U] = (u32)(InDataAddr >> 32);
	Payload[3U] = Size;
	Payload[4U] = EndLast;
	Payload[5U] = XSECURE_IPI_UNUSED_PARAM;

	/**
	 * Send an IPI request to the PLM by using the CDO command to call XSecure_ShaOperation api.
	 * Wait for IPI response from PLM with a timeout.
	 * If the timeout exceeds then error is returned otherwise it returns the status of the IPI response
	 */
	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));
	if (Status != XST_SUCCESS) {
		XSecure_Printf(XSECURE_DEBUG_GENERAL, "Sha3 Update Failed \r\n");
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to send final data to SHA engine
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	OutDataAddr	Address of the output buffer to store the
 * 				output hash
 *
 * @return
 *	-	XST_SUCCESS - If finished without any errors
 *	-	XSECURE_SHA_INVALID_PARAM - On invalid parameter
 *	-	XSECURE_SHA_STATE_MISMATCH_ERROR - If State mismatch is occurred
 *	-	XST_FAILURE - If Sha3PadType is other than KECCAK or NIST
 *
 *****************************************************************************/
int XSecure_ShaFinish(XSecure_ClientInstance *InstancePtr, const u64 OutDataAddr, u32 Size)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_6U];

	/**
	 * Perform input parameter validation on InstancePtr. Return XST_FAILURE if input parameters are invalid
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/* Fill IPI Payload */
	Payload[0U] = HEADER(0U, XSECURE_API_SHA_FINISH);
	Payload[1U] = (u32)OutDataAddr;
	Payload[2U] = (u32)(OutDataAddr >> 32);
	Payload[3U] = Size;
	Payload[4U] = XSECURE_IPI_UNUSED_PARAM;
	Payload[5U] = XSECURE_IPI_UNUSED_PARAM;

	/**
	 * Send an IPI request to the PLM by using the CDO command to call XSecure_ShaOperation api.
	 * Wait for IPI response from PLM with a timeout.
	 * If the timeout exceeds then error is returned otherwise it returns the status of the IPI response
	 */
	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));
	if (Status != XST_SUCCESS) {
		XSecure_Printf(XSECURE_DEBUG_GENERAL, "Sha Finish Failed \r\n");
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to calculate hash on single
 *		block of data
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	InDataAddr	Address of the input buffer where the input
 * 				data is stored
 * @param	OutDataAddr	Address of the output buffer to store the
 * 				output hash
 * @param	Size		Size of the data to be updated to SHA3 engine
 *
 * @return
 *	-	XST_SUCCESS - If the sha3 hash calculation is successful
 *	-	XSECURE_SHA3_INVALID_PARAM - On invalid parameter
 *	-	XSECURE_SHA3_STATE_MISMATCH_ERROR - If there is State mismatch
 *	-	XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XSecure_ShaDigest(XSecure_ClientInstance *InstancePtr, XSecure_ShaMode ShaMode,
	const u64 InDataAddr, const u64 OutDataAddr, u32 DataSize, u32 HashSize)
{
	volatile int Status = XST_FAILURE;

	/**
	 * Perform input parameter validation on InstancePtr. Return XST_FAILURE if input parameters are invalid
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	Status = XSecure_ShaInitialize(InstancePtr, ShaMode);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_ShaUpdate(InstancePtr, InDataAddr, DataSize, TRUE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_ShaFinish(InstancePtr, OutDataAddr, HashSize);

END:
	return Status;
}
