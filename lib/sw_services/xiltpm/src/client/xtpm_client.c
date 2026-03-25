/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xtpm_client.c
*
* This file contains the implementation of the client interface functions for
* TPM driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   pre  03/09/26 Initial release
*
* </pre>
*
*
******************************************************************************/
/**
* @addtogroup xtpm_client_apis XilTPM Client APIs
* @{
*/
/***************************** Include Files *********************************/
#include "xtpm_client.h"


/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function sends TPM initialization command to PLM and initializes
 * 			the TPM for further operations.
 *
 * @param	InstancePtr Pointer to the client instance
 * @return
 * 		- XST_SUCCESS  If TPM gets initialized successfully
 * 		- XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XTpm_Init(XTpm_ClientInstance *InstancePtr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * Perform input parameter validation on InstancePtr. Return XST_FAILURE if input parameters are invalid
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/* Fill IPI Payload */
	XTPM_PACK_PAYLOAD0(Payload, ((InstancePtr->SlrIndex << XTPM_SLR_INDEX_SHIFT) | XTPM_API_ID_INIT));

	/**
	 * Send an IPI request to the PLM by using the CDO command to call TPM initialization API and
	 * returns the status of the IPI response.
	 */
	Status = XTpm_ProcessMailbox(InstancePtr->MailboxPtr, Payload,
				sizeof(Payload)/sizeof(u32));

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends TPM startup command to PLM to start the TPM.
 *
 * @param	InstancePtr Pointer to the client instance
 *
 * @return	- XST_SUCCESS  If TPM startup command is successful
 * 			- XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XTpm_Startup(XTpm_ClientInstance *InstancePtr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * Perform input parameter validation on InstancePtr. Return XST_FAILURE if input parameters are invalid
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/* Fill IPI Payload */
	XTPM_PACK_PAYLOAD0(Payload, ((InstancePtr->SlrIndex << XTPM_SLR_INDEX_SHIFT) | XTPM_API_ID_STARTUP));

	/**
	 * Send an IPI request to the PLM by using the CDO command to call TPM startup API and
	 * returns the status of the IPI response.
	 */
	Status = XTpm_ProcessMailbox(InstancePtr->MailboxPtr, Payload,
				sizeof(Payload)/sizeof(u32));

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends TPM self-test command to PLM to perform a self-test on the TPM.
 *
 * @param	InstancePtr Pointer to the client instance
 *
 * @return	- XST_SUCCESS  If TPM self-test command is successful
 * 			- XST_FAILURE  If there is a failure
 *
 *************************************************************************************************/
int XTpm_SelfTest(XTpm_ClientInstance *InstancePtr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * Perform input parameter validation on InstancePtr. Return XST_FAILURE if input parameters are invalid
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/* Fill IPI Payload */
	XTPM_PACK_PAYLOAD0(Payload, ((InstancePtr->SlrIndex << XTPM_SLR_INDEX_SHIFT) | XTPM_API_ID_SELFTEST));

	/**
	 * Send an IPI request to the PLM by using the CDO command to call TPM self-test API and
	 * returns the status of the IPI response.
	 */
	Status = XTpm_ProcessMailbox(InstancePtr->MailboxPtr, Payload,
				sizeof(Payload)/sizeof(u32));

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends PCR event command to PLM to extend the
 * 			data to PCR specified in the input.
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	DataAddr Address of the data to be extended to PCR
 * @param	Length Length of the data to be extended to PCR
 * @param	PcrNumber PCR number to which the data will be extended
 *
 * @return
 * 			- XST_SUCCESS if successful
 * 			- Error code on failure
 *
 *************************************************************************************************/
int XTpm_PcrEvent(XTpm_ClientInstance *InstancePtr, u64 DataAddr, u32 Length, u32 PcrNumber)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * Perform input parameter validation on InstancePtr. Return XST_FAILURE if input parameters are invalid
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/* Fill IPI Payload */
	XTPM_PACK_PAYLOAD4(Payload, ((InstancePtr->SlrIndex << XTPM_SLR_INDEX_SHIFT) | XTPM_API_ID_PCR_EVENT),
						  DataAddr,
						 (DataAddr >> XTPM_ADDR_HIGH_SHIFT),
						  Length,
						  PcrNumber);
	/**
	 * Send an IPI request to the PLM by using the CDO command to call TPM PCR event API and
	 * returns the status of the IPI response.
	 */
	Status = XTpm_ProcessMailbox(InstancePtr->MailboxPtr, Payload,
				sizeof(Payload)/sizeof(u32));

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends PCR read command to PLM to read the PCR value for the specified PCR
 *          index and hash algorithm.
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	PcrIndex PCR index to be read
 * @param	HashAlgo Hash algorithm identifier for which PCR value needs to be read
 * @param	RespBufferAddr Address of the buffer to store the PCR read response (PCR value)
 *
 * @return
 * 			- XST_SUCCESS if successful
 * 			- Error code on failure
 *
 *************************************************************************************************/
int XTpm_PcrRead(XTpm_ClientInstance *InstancePtr, u32 PcrIndex, u8 HashAlgo, u64 RespBufferAddr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * Perform input parameter validation on InstancePtr. Return XST_FAILURE if input parameters are invalid
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/* Fill IPI Payload */
	XTPM_PACK_PAYLOAD4(Payload, ((InstancePtr->SlrIndex << XTPM_SLR_INDEX_SHIFT) | XTPM_API_ID_PCR_READ),
						  PcrIndex,
						  HashAlgo,
						  RespBufferAddr,
						  (RespBufferAddr >> XTPM_ADDR_HIGH_SHIFT));
	/**
	 * Send an IPI request to the PLM by using the CDO command to call TPM PCR read API and
	 * returns the status of the IPI response.
	 */
	Status = XTpm_ProcessMailbox(InstancePtr->MailboxPtr, Payload,
				sizeof(Payload)/sizeof(u32));

END:
	return Status;
}
/** @} End of xtpm_client_apis group */
