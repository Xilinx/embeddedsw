/******************************************************************************
* Copyright (c) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
*       skg  10/23/22 Added In body comments for APIs
* 3.2   am   03/09/23 Replaced xnvm payload lengths with xmailbox payload lengths
* 3.4   har  08/22/24 Added support for provisioning configuration limiter
* 3.4   ng   09/05/24 Fixed doxygen grouping
*
* </pre>
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
 * @return
 *		 	- XST_SUCCESS  If the BBRAM programming is successful
 *		 	- XST_INVALID_PARAM  If there is a input validation failure
 *			- XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XNvm_BbramWriteAesKey(const XNvm_ClientInstance *InstancePtr, const u64 KeyAddr,
						const u32 KeyLen)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_4U];

        /**
	 *  Performs input parameters validation.
	 *  Return XST_INVALID_PARAM if input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Payload[0U] = Header(0, (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT)|(u32)XNVM_API_ID_BBRAM_WRITE_AES_KEY));
	Payload[1U] = KeyLen;
	Payload[2U] = (u32)KeyAddr;
	Payload[3U] = (u32)(KeyAddr >> 32U);

        /**
	 *  @{ Sends BBRAM Write CDO command to PLM through IPI.
	 *     Waits for the response from PLM.
	 *     Returns the response of BBRAM aes key write status of PLM.
	 */
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
 * @return
 *		- XST_SUCCESS  If the BBRAM zeroize is successful
 *		- XST_INVALID_PARAM  If there is a input validation failure
 *		- XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XNvm_BbramZeroize(const XNvm_ClientInstance *InstancePtr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_1U];

	/**
	 *  Performs input parameters validation.
	 *  Return XST_INVALID_PARAM if input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Payload[0U] = Header(0, (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT)|(u32)XNVM_API_ID_BBRAM_ZEROIZE));

	/**
	 *  Sends BBRAM Zeroize CDO command to PLM through IPI.
	 *  Returns the response of the BBRAM Zeroize operation from the PLM
	 */
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
 * @return
 *		- XST_SUCCESS  If the BBRAM user data write successful
 *		- XST_INVALID_PARAM  If there is a input validation failure.
 *		- XST_FAILURE  If there is a failure
 *
 *****************************************************************************/
int XNvm_BbramWriteUsrData(const XNvm_ClientInstance *InstancePtr, const u32 UsrData)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_2U];

	/**
	 *  Performs input parameters validation.
	 *  Return XST_INVALID_PARAM if input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Payload[0U] = Header(0, (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT)|(u32)XNVM_API_ID_BBRAM_WRITE_USER_DATA));
	Payload[1U] = UsrData;

	/**
	 *  Sends BBRAM WRITE USER DATA CDO command to PLM IPI.
	 *  Returns the response of Bbram write user data status from the PLM.
	 */
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
 * @return
 *		- XST_SUCCESS  If the read is successful
 *		- XST_INVALID_PARAM  If there is a input validation failure
 *		- XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XNvm_BbramReadUsrData(const XNvm_ClientInstance *InstancePtr, const u64 OutDataAddr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_3U];

	/**
	 *  Performs input parameters validation.
	 *  Return XST_INVALID_PARAM if input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Payload[0U] = Header(0, (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT)|(u32)XNVM_API_ID_BBRAM_READ_USER_DATA));
	Payload[1U] = (u32)OutDataAddr;
	Payload[2U] = (u32)(OutDataAddr >> 32U);

	/**
	 *  Sends BBRAM READ USER DATA CDO command to PLM through IPI.
	 *  Returns the response of Bbram Read user data status from the PLM.
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));
END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to lock the updates to
 * 		user data written to BBRAM_8 register.
 *
 * @param	InstancePtr Pointer to the client instance
 *
 * @return
 *		- XST_SUCCESS  If the Locking is successful
 *		- XST_INVALID_PARAM  If there is a input validation failure
 *		- XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XNvm_BbramLockUsrDataWrite(const XNvm_ClientInstance *InstancePtr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_1U];

	/**
	 *  Performs input parameters validation.
	 *  Return XST_INVALID_PARAM, if input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Payload[0U] = Header(0, (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT)|(u32)XNVM_API_ID_BBRAM_LOCK_WRITE_USER_DATA));

	/**
	 *  Sends BBRAM LOCK USER DATA WRITE CDO command to PLM through IPI.
	 *  Returns the response of BBRAM lock user data write status from the PLM.
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));
END:
	return Status;
}

#ifdef VERSAL_2VE_2VM
/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to provision the configuration limiter parameters
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	ClEnFlag - Flag to indicate if the configuration limiter feature is enabled/disabled
 * @param	ClMode - Flag to indicate if the counter maintains the count of failed/total
 * 		configurations.
 * @param	MaxNumOfConfigs - Value of maximum number of configurations(failed/total) which are
 * 		allowed
 *
 * @return
 *		- XST_SUCCESS  If the provisioning is successful
 *		- XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XNvm_BbramWriteConfigLimiterParams(const XNvm_ClientInstance *InstancePtr, const u32 ClEnFlag,
	const u32 ClMode, const u32 MaxNumOfConfigs)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_4U];

	/**
	 *  Performs input parameters validation. Return error code if input parameters are invalid
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	Payload[0U] = Header(0, (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT) |
		(u32)XNVM_API_ID_BBRAM_WRITE_CFG_LMT_PARAMS));
	Payload[1U] = ClEnFlag;
	Payload[2U] = ClMode;
	Payload[3U] = MaxNumOfConfigs;

	/**
	 *  Sends BBRAM_WRITE_CFG_LMT_PARAMS CDO command to PLM through IPI. Returns the response of BBRAM_WRITE_CFG_LMT_PARAMS status in PLM.
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}
#endif
/*@}*/
