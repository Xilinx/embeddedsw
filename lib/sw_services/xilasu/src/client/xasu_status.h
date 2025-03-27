/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_status.h
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
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_client_info Client APIs AND Error Codes
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
	XASU_INVALID_CALL_BACK_REF,	/**< 0x17 - Invalid call back reference */
	XASU_INVALID_CLIENT_PARAM,	/**< 0x18 - Invalid client parameter pointer */
	XASU_INVALID_CLIENT_CTX,	/**< 0x19 - Client context is not valid */
	XASU_FAIL_SAVE_CTX,		/**< 0x1A - Failed in saving the context */
	XASU_INVALID_CURVEINFO,		/**< 0x1B - Invalid curve type or curve length */
};
/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASU_STATUS_H_ */
/** @} */
