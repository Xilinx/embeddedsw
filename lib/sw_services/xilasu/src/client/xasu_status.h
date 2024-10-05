/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
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
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_client_info Client APIs AND Error Codes
 * @{
*/
#ifndef XASU_STATUS_H
#define XASU_STATUS_H

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
enum XAsu_Status {
	XASU_INVALID_ARGUMENT = 0x10,	/**< 0x10 - Invalid argument */
	XASU_QUEUE_FULL,		/**< 0x11 - Channel queue is full */
	XASU_ASUFW_NOT_PRESENT,		/**< 0x12 - ASU application FW is not present */
	XASU_IPI_CONFIG_NOT_FOUND,	/**< 0x13 - IPI configuration is not found */
};
/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASU_STATUS_H */
/** @} */
