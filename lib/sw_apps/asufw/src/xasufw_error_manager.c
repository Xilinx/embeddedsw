/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_error_manager.c
 *
 * This file contains the error management code for ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   02/21/25 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Functionality
* @{
*/
/*************************************** Include Files *******************************************/
#include "xasufw_error_manager.h"
#include "xasufw_hw.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function initializes the error management functionality by enabling the required
 * errors.
 *
 *************************************************************************************************/
void XAsufw_ErrorManagerInit(void)
{
	/** Enable ASU SW Fatal and Non-Fatal Errors. */
	XAsufw_WriteReg(ASU_GLOBAL_ASU_FATAL_ERROR_ENABLE,
			ASU_GLOBAL_ASU_FATAL_ERROR_ENABLE_ASU_SW_ERROR_MASK);

	XAsufw_WriteReg(ASU_GLOBAL_ASU_NON_FATAL_ERROR_ENABLE,
			ASU_GLOBAL_ASU_NON_FATAL_ERROR_ENABLE_ASU_SW_ERROR_MASK);
}

/*************************************************************************************************/
/**
 * @brief	This function triggers Fatal/Non-Fatal error to PLM to indicate to PLM that there is
 * a failure in ASUFW.
 *
 * @param	ErrorType   Fatal/Non-Fatal error
 * @param	ErrorCode	ASUFW Error code
 *
 *************************************************************************************************/
void XAsufw_SendErrorToPlm(XAsufw_ErrorType ErrorType, s32 ErrorCode)
{
	/** Log the error to the ASU_SW_ERROR register. */
	XAsufw_WriteReg(ASU_GLOBAL_ASU_SW_ERROR, (u32)ErrorCode);

	/** Trigger ASU Fatal/Non-Fatal asu_sw_error based on ErrorType. */
	if (ErrorType == XASUFW_FATAL_ERROR) {
		XAsufw_WriteReg(ASU_GLOBAL_ASU_FATAL_ERROR_TRIGGER,
			ASU_GLOBAL_ASU_FATAL_ERROR_TRIGGER_ASU_SW_ERROR_MASK);
	} else {
		XAsufw_WriteReg(ASU_GLOBAL_ASU_NON_FATAL_ERROR_TRIGGER,
			ASU_GLOBAL_ASU_NON_FATAL_ERROR_TRIGGER_ASU_SW_ERROR_MASK);
	}
}
/** @} */
