/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_status.h
 *
 * This file contains ASU client error codes
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   vns  08/06/24 Initial release
 *       ss   09/19/24 Added XASU_ASUFW_NOT_PRESENT error code
 *       yog  09/26/24 Added doxygen groupings.
 *       yog  03/25/25 Added XASU_INVALID_CURVEINFO error code.
 *       lp   07/10/25 Added XASU_CLIENT_CTX_NOT_CREATED and XASU_REQUEST_INPROGRESS error codes.
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_client_error_codes Client Error Codes
 * @{
*/
#ifndef XASU_STATUS_H_
#define XASU_STATUS_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/**
 * This contains the client error codes.
 */
enum {
	XASU_INVALID_ARGUMENT = 0x10,	/**< 0x10 - Invalid argument */
	XASU_QUEUE_FULL,		/**< 0x11 - Channel queue is full */
	XASU_ASUFW_NOT_PRESENT,		/**< 0x12 - ASU application FW is not present */
	XASU_IPI_CONFIG_NOT_FOUND,	/**< 0x13 - IPI configuration is not found */
	XASU_INVALID_UNIQUE_ID,		/**< 0x14 - Invalid unique ID */
	XASU_CLIENT_NOT_INITIALIZED,/**< 0x15 - Client is not initialized */
	XASU_INVALID_PRIORITY,		/**< 0x16 - Invalid Priority */
	XASU_INVALID_CALL_BACK_REF,	/**< 0x17 - Invalid callback reference */
	XASU_INVALID_CLIENT_PARAM,	/**< 0x18 - Invalid client parameter pointer */
	XASU_INVALID_CLIENT_CTX,	/**< 0x19 - Client context is not valid */
	XASU_FAIL_SAVE_CTX,		/**< 0x1A - Failed in saving the context */
	XASU_INVALID_CURVEINFO,		/**< 0x1B - Invalid curve type or curve length */
	XASU_CLIENT_CTX_NOT_CREATED,	/**< 0x1C - Client context is not created */
	XASU_REQUEST_INPROGRESS,	/**< 0x1D - Client should not allow more than one split
				request */
	XASU_INVALID_SECURE_FLAG,	/**< 0x1E - Invalid secure flag */
	XASU_KAT_EXEC_NOT_COMPLETED,	/**< 0x1F - KAT execution is not completed */
};

/**
 * This contains the non-zero success values returned from server for critical security
 * functionalities and these codes should not be used for failures in client
 */
enum {
	XASU_RSA_ECDH_SUCCESS = 0x3F9, /**< 0x3F9U - RSA ECDH operation is successful */
	XASU_RSA_PSS_SIGNATURE_VERIFIED, /**< 0x3FAU - RSA PSS decode and sign verify
						operation is successful */
	XASU_RSA_DECRYPTION_SUCCESS, /**< 0x3FBU - Successfully decrypted RSA provided
					message */
	XASU_ECC_SIGNATURE_VERIFIED, /**< 0x3FCU - Successfully verified ECC signature */
	XASU_AES_TAG_MATCHED, /**< 0x3FDU - Successfully verified AES tag */
	XASU_AES_TAG_READ, /**< 0x3FEU - Successfully read AES tag */
};

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASU_STATUS_H_ */
/** @} */
