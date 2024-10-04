/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
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
* 4.5   kal  03/23/20 Updated file version to sync with library version
*       kal  03/28/21 Added XSecure_Sha3Digest Client API
*       kpt  05/02/21 Updated XSecure_Sha3Update function to accept multiple
*                     data update requests from user and maintained sha driver
*                     state using XSecure_ShaState
* 4.6   kal  08/22/21 Updated doxygen comment description for
*                     XSecure_Sha3Initialize API
*       kpt  03/16/22 Removed IPI related code and added mailbox support
* 5.0   kpt  07/24/22 Moved XSecure_Sha3Kat into xsecure_katclient.c
* 5.2   am   03/09/23 Replaced xsecure payload lengths with xmailbox payload lengths
*	yog  05/03/23 Fixed MISRA C violation of Rule 12.2
*	yog  05/04/23 Fixed HIS COMF violations
* 5.4   yog  04/29/24 Fixed doxygen warnings
*       pre  08/29/24 APIs are updated for SSIT support
*
* </pre>
*
*
******************************************************************************/
/**
* @addtogroup xsecure_sha_client_apis XilSecure SHA Client APIs
* @{
*/
/***************************** Include Files *********************************/
#include "xsecure_shaclient.h"

/************************** Constant Definitions *****************************/
static XSecure_ShaState Sha3State = XSECURE_SHA_UNINITIALIZED;
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XSECURE_SHA_FIRST_PACKET_SHIFT		(30U)
#define XSECURE_SHA_UPDATE_CONTINUE_SHIFT	(31U)

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function stores the Sha3 initialize state as initialized
 *		if the current state is uninitialized.
 *
 * @return
 *		 - XST_SUCCESS  If the Sha3 state is changed to initialized state
 *		 - XST_FAILURE  If the Sha3 is not in uninitialized state
 *
 ******************************************************************************/
int XSecure_Sha3Initialize(void)
{
	volatile int Status = XST_FAILURE;

	if (Sha3State == XSECURE_SHA_UNINITIALIZED) {
		Sha3State = XSECURE_SHA_INITIALIZED;
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to update the SHA3 engine
 *		with the input data
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	InDataAddr	Address of the output buffer to store the
 * 				output hash
 * @param	Size		Size of the data to be updated to SHA3 engine
 *
 * @return
 *		 - XST_SUCCESS  If the update is successful
 *		 - XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XSecure_Sha3Update(XSecure_ClientInstance *InstancePtr, const u64 InDataAddr, u32 Size)
{
	volatile int Status = XST_FAILURE;
	u32 Sha3InitializeMask = 0U;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_6U];

	/**
	 * Perform input parameter validation on InstancePtr. Return XST_FAILURE if input parameters are invalid
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/**
	 * Validate Sha3state. Return XST_FAILURE if the state is not XSECURE_SHA_UNINITIALIZED
	 */
	if ((Sha3State != XSECURE_SHA_INITIALIZED) &&
		(Sha3State != XSECURE_SHA_UPDATE)) {
		XSecure_Printf(XSECURE_DEBUG_GENERAL, "Invalid SHA3 State \r\n");
		goto END;
	}

	if (Sha3State == XSECURE_SHA_INITIALIZED) {
		Sha3InitializeMask = ((u32)1U) << XSECURE_SHA_FIRST_PACKET_SHIFT;
	}

	/* Fill IPI Payload */
	Payload[0U] = HEADER(0, (InstancePtr->SlrIndex << XSECURE_SLR_INDEX_SHIFT)
	                    | XSECURE_API_SHA3_UPDATE);
	Payload[1U] = (u32)InDataAddr;
	Payload[2U] = (u32)(InDataAddr >> 32);
	Payload[3U] = (u32)(((u32)1U << XSECURE_SHA_UPDATE_CONTINUE_SHIFT)|
						(Sha3InitializeMask) | Size);
	Payload[4U] = XSECURE_IPI_UNUSED_PARAM;
	Payload[5U] = XSECURE_IPI_UNUSED_PARAM;

	/**
	 * Send an IPI request to the PLM by using the CDO command to call XSecure_ShaOperation
	 * API and returns the status of the IPI response.
	 */
	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));
	if (Status != XST_SUCCESS) {
		XSecure_Printf(XSECURE_DEBUG_GENERAL, "Sha3 Update Failed \r\n");
		goto END;
	}

	Sha3State = XSECURE_SHA_UPDATE;
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request final data to SHA3 engine
 * 		which includes SHA3 padding and reads final hash
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	OutDataAddr	Address of the output buffer to store the
 * 				output hash
 *
 * @return
 *		 - XST_SUCCESS  If finished without any errors
 *		 - XST_FAILURE  If there is a failure
 *
 *****************************************************************************/
int XSecure_Sha3Finish(XSecure_ClientInstance *InstancePtr, const u64 OutDataAddr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_6U];

	/**
	 * Perform input parameter validation on InstancePtr. Return XST_FAILURE if input parameters are invalid
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/**
	 * Validate Sha3state. Return XST_FAILURE if the state is XSECURE_SHA_UNINITIALIZED
	 */
	if ((Sha3State != XSECURE_SHA_INITIALIZED) &&
		(Sha3State != XSECURE_SHA_UPDATE)) {
		XSecure_Printf(XSECURE_DEBUG_GENERAL, "Invalid SHA3 State \r\n");
		goto END;
	}

	/* Fill IPI Payload */
	Payload[0U] = HEADER(0, (InstancePtr->SlrIndex << XSECURE_SLR_INDEX_SHIFT)
	                    | XSECURE_API_SHA3_UPDATE);
	Payload[1U] = XSECURE_IPI_UNUSED_PARAM;
	Payload[2U] = XSECURE_IPI_UNUSED_PARAM;
	Payload[3U] = XSECURE_IPI_UNUSED_PARAM;
	Payload[4U] = (u32)OutDataAddr;
	Payload[5U] = (u32)(OutDataAddr >> 32);

	/**
	 * Send an IPI request to the PLM by using the CDO command to call XSecure_ShaOperation
	 * API and it returns the status of the IPI response.
	 */
	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));
	if (Status != XST_SUCCESS) {
		XSecure_Printf(XSECURE_DEBUG_GENERAL, "Sha3 Finish Failed \r\n");
		goto END;
	}

	Sha3State = XSECURE_SHA_UNINITIALIZED;
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
 *		 - XST_SUCCESS  If the sha3 hash calculation is successful
 *		 - XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XSecure_Sha3Digest(XSecure_ClientInstance *InstancePtr, const u64 InDataAddr, const u64 OutDataAddr, u32 Size)
{
	volatile int Status = XST_FAILURE;
	u32 Sha3InitializeMask = 0U;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_6U];

	/**
	 * Perform input parameter validation on InstancePtr. Return XST_FAILURE if input parameters are invalid
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/**
	 * Initialize SHA3
	 */
	Status = XSecure_Sha3Initialize();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Sha3InitializeMask = ((u32)1U) << XSECURE_SHA_FIRST_PACKET_SHIFT;

	/* Fill IPI Payload */
	Payload[0U] = HEADER(0, (InstancePtr->SlrIndex << XSECURE_SLR_INDEX_SHIFT)
	                    | XSECURE_API_SHA3_UPDATE);
	Payload[1U] = (u32)InDataAddr;
	Payload[2U] = (u32)(InDataAddr >> XSECURE_ADDR_HIGH_SHIFT);
	Payload[3U] = (Sha3InitializeMask) | Size;
	Payload[4U] = (u32)OutDataAddr;
	Payload[5U] = (u32)(OutDataAddr >> XSECURE_ADDR_HIGH_SHIFT);

	/**
	 * Send an IPI request to the PLM by using the CDO command to call XSecure_ShaOperation
	 * API and returns the status of the IPI response.
	 */
	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload,
				sizeof(Payload)/sizeof(u32));

	if (Status != XST_SUCCESS) {
		XSecure_Printf(XSECURE_DEBUG_GENERAL, "Sha3 Digest Failed \r\n");
		goto END;
	}

	Sha3State = XSECURE_SHA_UNINITIALIZED;

END:
	return Status;
}
/** @} */
