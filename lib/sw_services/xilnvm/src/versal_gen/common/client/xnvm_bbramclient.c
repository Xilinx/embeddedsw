/******************************************************************************
* Copyright (c) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
* 3.7   tbk  03/20/26 Added SMC support for client applications
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xnvm_bbramclient.h"
#include "xnvm_generic.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief       This function sends IPI request to program BBRAM AES key.
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	KeyAddr		Address of the key buffer where the key to
 * 				be programmed is stored
 *
 * @param	KeyLen		Size of the Aes key
 *
 * @return
 * 		- XST_SUCCESS  If the BBRAM programming is successful
 * 		- XST_INVALID_PARAM  If there is a input validation failure
 * 		- XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XNvm_BbramWriteAesKey(const XNvm_ClientInstance *InstancePtr, const u64 KeyAddr,
						const u32 KeyLen)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * - Performs input parameters validation.
	 * Return XST_INVALID_PARAM if input parameters are invalid.
	 */
	if (InstancePtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** - Fill Payload */
	XNVM_PACK_PAYLOAD3(Payload, (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT)
				| (u32)XNVM_API_ID_BBRAM_WRITE_AES_KEY),
				KeyLen,
				KeyAddr,
				(KeyAddr >> XNVM_ADDR_HIGH_SHIFT));

	/** - Send request using generic API */
	Status = XNvm_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);
	if (Status != XST_SUCCESS) {
		XNvm_Printf(XNVM_DEBUG_GENERAL, "BBRAM programming Failed \r\n");
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function sends IPI request to zeroize the BBRAM.
 *
 * @param	InstancePtr Pointer to the client instance
 *
 * @return
 * 		- XST_SUCCESS  If the BBRAM zeroize is successful
 * 		- XST_INVALID_PARAM  If there is a input validation failure
 * 		- XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XNvm_BbramZeroize(const XNvm_ClientInstance *InstancePtr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * - Performs input parameters validation.
	 * Return XST_INVALID_PARAM if input parameters are invalid.
	 */
	if (InstancePtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** - Fill Payload */
	XNVM_PACK_PAYLOAD0(Payload, (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT)
				| (u32)XNVM_API_ID_BBRAM_ZEROIZE));

	/** - Send request using generic API */
	Status = XNvm_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to write the user data into
 * 		BBRAM user data registers.
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	UsrData		User data to be written to BBRAM
 *
 * @return
 * 		- XST_SUCCESS  If the BBRAM user data write successful
 * 		- XST_INVALID_PARAM  If there is a input validation failure.
 * 		- XST_FAILURE  If there is a failure
 *
 *****************************************************************************/
int XNvm_BbramWriteUsrData(const XNvm_ClientInstance *InstancePtr, const u32 UsrData)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * - Performs input parameters validation.
	 * Return XST_INVALID_PARAM if input parameters are invalid.
	 */
	if (InstancePtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** - Fill Payload */
	XNVM_PACK_PAYLOAD1(Payload, (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT)
				| (u32)XNVM_API_ID_BBRAM_WRITE_USER_DATA),
				UsrData);

	/** - Send request using generic API */
	Status = XNvm_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read the BBRAM user data.
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	OutDataAddr	Address of the output buffer to store the
 * 				BBRAM user data
 *
 * @return
 * 		- XST_SUCCESS  If the read is successful
 * 		- XST_INVALID_PARAM  If there is a input validation failure
 * 		- XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XNvm_BbramReadUsrData(const XNvm_ClientInstance *InstancePtr, const u64 OutDataAddr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * - Performs input parameters validation.
	 * Return XST_INVALID_PARAM if input parameters are invalid.
	 */
	if (InstancePtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** - Fill Payload */
	XNVM_PACK_PAYLOAD2(Payload, (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT)
				| (u32)XNVM_API_ID_BBRAM_READ_USER_DATA),
				OutDataAddr,
				(OutDataAddr >> XNVM_ADDR_HIGH_SHIFT));

	/** - Send request using generic API */
	Status = XNvm_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);

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
 * 		- XST_SUCCESS  If the Locking is successful
 * 		- XST_INVALID_PARAM  If there is a input validation failure
 * 		- XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XNvm_BbramLockUsrDataWrite(const XNvm_ClientInstance *InstancePtr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * - Performs input parameters validation.
	 * Return XST_INVALID_PARAM, if input parameters are invalid.
	 */
	if (InstancePtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** - Fill Payload */
	XNVM_PACK_PAYLOAD0(Payload, (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT)
				| (u32)XNVM_API_ID_BBRAM_LOCK_WRITE_USER_DATA));

	/** - Send request using generic API */
	Status = XNvm_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);

END:
	return Status;
}

#ifdef VERSAL_2VE_2VM
/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to provision the configuration limiter parameters.
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	ClEnFlag - Flag to indicate if the configuration limiter feature is enabled/disabled
 * @param	ClMode - Flag to indicate if the counter maintains the count of failed/total
 * 		configurations.
 * @param	MaxNumOfConfigs - Value of maximum number of configurations(failed/total) which are
 * 		allowed
 *
 * @return
 * 		- XST_SUCCESS  If the provisioning is successful
 * 		- XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XNvm_BbramWriteConfigLimiterParams(const XNvm_ClientInstance *InstancePtr, const u32 ClEnFlag,
	const u32 ClMode, const u32 MaxNumOfConfigs)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * - Performs input parameters validation. Return error code if input parameters are invalid
	 */
	if (InstancePtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** - Fill IPI Payload */
	XNVM_PACK_PAYLOAD3(Payload, (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT) |
				(u32)XNVM_API_ID_BBRAM_WRITE_CFG_LMT_PARAMS),
				ClEnFlag,
				ClMode,
				MaxNumOfConfigs);

	/** - Send request using generic API */
	Status = XNvm_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);

END:
	return Status;
}
#endif
