/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpuf_client.c
*
* This file contains the implementation of the client interface functions for
* PUF hardware interface API's.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kpt  01/04/22 Initial release
*       am   02/28/22 Fixed MISRA C violation rule 10.3
*       kpt  03/16/22 Removed IPI related code and added mailbox support
* 2.1   skg  10/29/22 Added In Body comments
*       skg  12/14/22 Added Slr index as part of payload
*       am   02/13/23 Fixed MISRA C violations
*       am   02/17/23 Fixed HIS_COMF violations
* 2.2   am   03/09/23 Replaced xpuf payload lengths with xmailbox payload lengths
*       vss  09/21/23 Fixed doxygen warnings
* 2.3   ng   11/22/23 Fixed doxygen grouping
* 2.4   ng   04/30/24 Fixed doxygen comments
* 2.7   tbk  03/20/26 Added SMC support for client applications
*
* </pre>
*
******************************************************************************/

/**
 * @addtogroup xpuf_client_api XilPuf Client API
 * @{
 */

/***************************** Include Files *********************************/
#include "xpuf_client.h"
#include "xpuf_generic.h"

/************************** Constant Definitions *****************************/
#define XPUF_SLR_INDEX_SHIFT (6U) /**< To shift puf SLR index */
#define XPUF_ADDR_HIGH_SHIFT (32U) /**< Shift value to get higher 32 bit address */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
*
* This function sets the instance of mailbox
*
* @param InstancePtr Pointer to the client instance
* @param MailboxPtr Pointer to the mailbox instance
*
* @return
* 		- XST_SUCCESS	On successful initialization
* 		- XST_FAILURE	On failure
*
******************************************************************************/
int XPuf_ClientInit(XPuf_ClientInstance* const InstancePtr, XMailbox* const MailboxPtr) {
	int Status = XST_FAILURE;

	/**
	 * Set XMailbox instance provided by the user to client library instance by validating whether provided instance is not NULL and initialized.
	 * Use XMailbox instance to initiate the communication between client and server.
	 */
	if (InstancePtr != NULL) {
			InstancePtr->MailboxPtr = MailboxPtr;
			InstancePtr->SlrIndex = 0U;
			Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function sends IPI request for PUF registration
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	DataAddr	Address of the data structure which includes
 * 				        options to configure PUF
 *
 * @return
 *		- XST_SUCCESS if the PUF registration is successful
 *		- XST_FAILURE if there is a failure
 *
 ******************************************************************************/
int XPuf_Registration(const XPuf_ClientInstance *InstancePtr, const u64 DataAddr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * Perform input parameter validation on InstancePtr. Return XST_FAILURE if input parameters
	 * are invalid
	 */
	if (InstancePtr == NULL) {
		goto END;
	}

	/** Fill IPI Payload */
	XPUF_PACK_PAYLOAD2(Payload, ((InstancePtr->SlrIndex << XPUF_SLR_INDEX_SHIFT)
				| XPUF_PUF_REGISTRATION),
				DataAddr,
				(DataAddr >> XPUF_ADDR_HIGH_SHIFT));

	/** Send request to PLM using generic API */
	Status = XPuf_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);
	if (Status != XST_SUCCESS) {
		XPuf_Printf(XPUF_DEBUG_GENERAL, "PUF registration Failed \r\n");
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function sends IPI request for PUF regeneration
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	DataAddr	Address of the data structure which includes
 * 				        options to configure PUF
 *
 * @return
 *		- XST_SUCCESS if the PUF regeneration is successful
 *		- XST_FAILURE if there is a failure
 *
 ******************************************************************************/
int XPuf_Regeneration(const XPuf_ClientInstance *InstancePtr, const u64 DataAddr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * Perform input parameter validation on InstancePtr. Return XST_FAILURE if input parameters
	 * are invalid
	 */
	if (InstancePtr == NULL) {
		goto END;
	}

	/** Fill IPI Payload */
	XPUF_PACK_PAYLOAD2(Payload, ((InstancePtr->SlrIndex << XPUF_SLR_INDEX_SHIFT)
				| XPUF_PUF_REGENERATION),
				DataAddr,
				(DataAddr >> XPUF_ADDR_HIGH_SHIFT));

	/** Send request to PLM using generic API */
	Status = XPuf_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);
	if (Status != XST_SUCCESS) {
		XPuf_Printf(XPUF_DEBUG_GENERAL, "PUF regeneration Failed \r\n");
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function sends IPI request for PUF clear ID
 *
 * @param	InstancePtr	Pointer to the client instance
 *
 * @return
 *		- XST_SUCCESS if the PUF clear ID is successful
 *		- XST_FAILURE if there is a failure
 *
 ******************************************************************************/
int XPuf_ClearPufID(const XPuf_ClientInstance *InstancePtr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * Perform input parameter validation on InstancePtr. Return XST_FAILURE if input parameters
	 * are invalid
	 */
	if (InstancePtr == NULL) {
		goto END;
	}

	/** Fill IPI Payload */
	XPUF_PACK_PAYLOAD0(Payload, ((InstancePtr->SlrIndex << XPUF_SLR_INDEX_SHIFT)
				| XPUF_PUF_CLEAR_PUF_ID));

	/** Send request to PLM using generic API */
	Status = XPuf_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);
	if (Status != XST_SUCCESS) {
		XPuf_Printf(XPUF_DEBUG_GENERAL, "Clear PUF ID Failed \r\n");
	}

END:
	return Status;
}
/** @} */
