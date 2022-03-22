/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
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
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xpuf_client.h"

/************************** Constant Definitions *****************************/

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
* 	- XST_SUCCESS	On successful initialization
* 	- XST_FAILURE	On failure
*
******************************************************************************/
int XPuf_ClientInit(XPuf_ClientInstance* const InstancePtr, XMailbox* const MailboxPtr) {
	int Status = XST_FAILURE;

	if (InstancePtr != NULL) {
			InstancePtr->MailboxPtr = MailboxPtr;
			Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function sends IPI request for PUF registration
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	DataAddr	Address of the data structure which includes
 * 				        options to configure PUF
 *
 * @return	- XST_SUCCESS - If the PUF registration is successful
 * 		    - XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XPuf_Registration(XPuf_ClientInstance *InstancePtr, const u64 DataAddr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XPUF_PAYLOAD_LEN_3U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	Payload[0U] = PufHeader(0, (u32)XPUF_PUF_REGISTRATION);
	Payload[1U] = (u32)DataAddr;
	Payload[2U] = (u32)(DataAddr >> 32U);

	Status = XPuf_ProcessMailbox(InstancePtr->MailboxPtr, Payload,
				sizeof(Payload)/sizeof(u32));
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
 * @param	InstancePtr Pointer to the client instance
 * @param	DataAddr	Address of the data structure which includes
 * 				        options to configure PUF
 *
 * @return	- XST_SUCCESS - If the PUF regeneration is successful
 * 		    - XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XPuf_Regeneration(XPuf_ClientInstance *InstancePtr, const u64 DataAddr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XPUF_PAYLOAD_LEN_3U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	Payload[0U] = PufHeader(0, (u32)XPUF_PUF_REGENERATION);
	Payload[1U] = (u32)DataAddr;
	Payload[2U] = (u32)(DataAddr >> 32U);

	Status = XPuf_ProcessMailbox(InstancePtr->MailboxPtr, Payload,
				sizeof(Payload)/sizeof(u32));
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
 * @return	- XST_SUCCESS - If the PUF clear ID is successful
 * 		    - XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XPuf_ClearPufID(XPuf_ClientInstance *InstancePtr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XPUF_PAYLOAD_LEN_1U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	Payload[0U] = PufHeader(0, (u32)XPUF_PUF_CLEAR_PUF_ID);

	Status = XPuf_ProcessMailbox(InstancePtr->MailboxPtr, Payload,
						sizeof(Payload)/sizeof(u32));
	if (Status != XST_SUCCESS) {
		XPuf_Printf(XPUF_DEBUG_GENERAL, "Clear PUF ID Failed \r\n");
	}

END:
	return Status;
}