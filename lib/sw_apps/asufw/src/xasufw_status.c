/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_status.c
 *
 * This file contains the definition of error code update functions in ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   vns  01/16/24 Initial release
 *       ma   03/16/24 Updated XST_SUCCESS with XASUFW_SUCCESS
 *       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_server_error_codes Server Error Codes
* @{
*/
/*************************************** Include Files *******************************************/
#include "xasufw_status.h"

/************************************ Constant Definitions ***************************************/
/**
 * Error status of ASUFW :
 * The error status of ASUFW is represented by a 32-bit value, which can be broken down as
 * outlined below. Each error code is referenced through the XAsufw_Status enum or respective
 * BSP error codes.
 * Bit 31-30    : Buffer clear Status, 31st bit high represents failure and 30th bit high
 * 		          represents success.
 * Bit 29-20    : Final error Code
 * Bit 19-10    : Second error code which is responsible for failure
 * Bit 9-0      : First error code which is responsible for failure
 */
#define XASUFW_FIRST_ERROR_CODE_MASK	        (0x000003FFU) /**< Mask of first error */
#define XASUFW_SECOND_ERROR_CODE_MASK           (0x000FFC00U) /**< Mask of second error */
#define XASUFW_SECOND_ERROR_CODE_SHIFT          (10U)		  /**< Second error shift value */
#define XASUFW_FINAL_ERROR_CODE_MASK            (0x3FF00000U) /**< Mask of final error */
#define XASUFW_FINAL_ERROR_CODE_SHIFT           (20U)		  /**< Shift value of final error */
#define XASUFW_BUF_CLEAR_STATUS_MASK            (0xC0000000U) /**< Buffer clear status mask */
#define XASUFW_BUF_CLEAR_STATUS_FAILURE_MASK    (0x80000000U) /**< Buffer clear status failure
									mask*/
#define XASUFW_BUF_CLEAR_STATUS_SUCCESS_MASK    (0x40000000U) /**< Buffer clear status success
									mask */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function builds an error status on the order of the error occurance.
 *
 * @param	ErrorStatus	The current error which needs an update with provided new error codes.
 * @param	Error		Latest error code to be updated to AsuFwErrorCode.
 *
 * @return	Updated error code will be returned by this function.
 *
 * @note
 * 		Refer xasufw_status.h for more details on the error status format.
 *
 *************************************************************************************************/
s32 XAsufw_UpdateErrorStatus(s32 ErrorStatus, s32 Error)
{
	u32 Status = (u32)ErrorStatus;
	u32 LatestCode = (u32)Error;

	if ((Status & XASUFW_FIRST_ERROR_CODE_MASK) == 0x0U) {
		/** If first error code is zero, update it with error code provided. */
		Status |= (LatestCode & XASUFW_FIRST_ERROR_CODE_MASK);
	} else if ((Status & XASUFW_SECOND_ERROR_CODE_MASK) == 0x0U) {
		/**
		* If already first error code is existing, update
		* the second error code with provided new error code.
		*/
		Status |= ((LatestCode << XASUFW_SECOND_ERROR_CODE_SHIFT) &
			   XASUFW_SECOND_ERROR_CODE_MASK);
	} else {
		Status &= ~(XASUFW_FINAL_ERROR_CODE_MASK);
		Status |= ((LatestCode << XASUFW_FINAL_ERROR_CODE_SHIFT) & XASUFW_FINAL_ERROR_CODE_MASK);
	}

	return (s32)Status;
}

/*************************************************************************************************/
/**
 * @brief	This function updates the error code with provided buffer clear status.
 *
 * @param	ErrorStatus	The current error which needs an update with buffer clear status.
 * @param	BufStatus	Buffer clear status
 *
 * @return	Updated error code will be returned by this function
 *
 *************************************************************************************************/
s32 XAsufw_UpdateBufStatus(s32 ErrorStatus, s32 BufStatus)
{
	u32 Status = (u32)ErrorStatus;

	if ((Status == 0x0U) && (BufStatus != XASUFW_SUCCESS)) {
		Status = Status | XASUFW_BUF_CLEAR_STATUS_FAILURE_MASK;
	}

	if ((Status != 0x0U) &&  ((Status & XASUFW_BUF_CLEAR_STATUS_FAILURE_MASK) == 0x0U)) {
		Status &= (~XASUFW_BUF_CLEAR_STATUS_MASK);
		Status |= (BufStatus != XASUFW_SUCCESS) ? XASUFW_BUF_CLEAR_STATUS_FAILURE_MASK
			  : XASUFW_BUF_CLEAR_STATUS_SUCCESS_MASK;
	}

	return (s32)Status;
}
/** @} */
