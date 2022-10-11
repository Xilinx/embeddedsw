/******************************************************************************
* Copyright (c) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xnvm_bbramclient.c
*
* This file contains the implementation of the client interface functions for
* BBRAM programming.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kal  07/05/21 Initial release
* 1.1   am   02/28/22 Fixed MISRA C violation rule 4.5
*       kpt  03/16/22 Removed IPI related code and added mailbox support
* 3.1   skg  10/04/22 Added SlrIndex as part of payload based on user input
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xnvm_bbramclient.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief       This function sends IPI request to program BBRAM AES key
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	KeyAddr		Address of the key buffer where the key to
 * 				be programmed is stored
 *
 * @param	KeyLen		Size of the Aes key
 *
 * @return	- XST_SUCCESS - If the BBRAM programming is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XNvm_BbramWriteAesKey(XNvm_ClientInstance *InstancePtr, const u64 KeyAddr,
						const u32 KeyLen)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XNVM_PAYLOAD_LEN_4U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	Payload[0U] = Header(0, (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT)|XNVM_API_ID_BBRAM_WRITE_AES_KEY));
	Payload[1U] = KeyLen;
	Payload[2U] = (u32)KeyAddr;
	Payload[3U] = (u32)(KeyAddr >> 32U);

	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));
	if (Status != XST_SUCCESS) {
		XNvm_Printf(XNVM_DEBUG_GENERAL, "BBRAM programming Failed \r\n");
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function sends IPI request to zeroize the BBRAM
 *
 * @param	InstancePtr Pointer to the client instance
 *
 * @return	- XST_SUCCESS - If the BBRAM zeroize is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XNvm_BbramZeroize(XNvm_ClientInstance *InstancePtr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XNVM_PAYLOAD_LEN_1U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	Payload[0U] = Header(0, (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT)|XNVM_API_ID_BBRAM_ZEROIZE));

	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to write the user data into
 * 		BBRAM user data registers
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	UsrData		User data to be written to BBRAM
 *
 * @return	- XST_SUCCESS - If the BBRAM user data write successful
 *		- XST_FAILURE - If there is a failure
 *
 *****************************************************************************/
int XNvm_BbramWriteUsrData(XNvm_ClientInstance *InstancePtr, const u32 UsrData)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XNVM_PAYLOAD_LEN_2U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	Payload[0U] = Header(0, (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT)|XNVM_API_ID_BBRAM_WRITE_USER_DATA));
	Payload[1U] = UsrData;

	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read the BBRAM user data
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	OutDataAddr	Address of the output buffer to store the
 * 				BBRAM user data
 *
 * @return	- XST_SUCCESS - If the read is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XNvm_BbramReadUsrData(XNvm_ClientInstance *InstancePtr, const u64 OutDataAddr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XNVM_PAYLOAD_LEN_3U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	Payload[0U] = Header(0, (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT)|XNVM_API_ID_BBRAM_READ_USER_DATA));
	Payload[1U] = (u32)OutDataAddr;
	Payload[2U] = (u32)(OutDataAddr >> 32U);

	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));
END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to lock the user data written
 * 		to BBRAM
 *
 * @param	InstancePtr Pointer to the client instance
 *
 * @return	- XST_SUCCESS - If the Locking is successful
 *		- XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XNvm_BbramLockUsrDataWrite(XNvm_ClientInstance *InstancePtr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XNVM_PAYLOAD_LEN_1U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	Payload[0U] = Header(0, (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT)|XNVM_API_ID_BBRAM_LOCK_WRITE_USER_DATA));

	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));
END:
	return Status;
}
