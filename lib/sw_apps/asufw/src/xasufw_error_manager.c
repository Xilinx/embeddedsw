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
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
/*************************************** Include Files *******************************************/
#include "xasufw_error_manager.h"
#include "xasufw_hw.h"
#include "xasufw_status.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function initializes the error management functionality by enabling the required
 * 		errors.
 *
 * @return
 *		- XASUFW_SUCCESS, if error  management initialization is successful.
 *		- XASUFW_FAILURE, if error  management initialization fails.
 *
 *************************************************************************************************/
s32 XAsufw_ErrorManagerInit(void)
{
	s32 Status = XASUFW_FAILURE;

	/** Enable ASU SW Fatal and Non-Fatal Errors. */
	XAsufw_WriteReg(ASU_GLOBAL_ASU_FATAL_ERROR_ENABLE,
			ASU_GLOBAL_ASU_FATAL_ERROR_ENABLE_ASU_SW_ERROR_MASK);
	if((XAsufw_ReadReg(ASU_GLOBAL_ASU_FATAL_ERROR_MASK) &
		ASU_GLOBAL_ASU_FATAL_ERROR_ENABLE_ASU_SW_ERROR_MASK) != 0U) {
		Status = XASUFW_SW_ERR_INIT_FAIL;
		goto END;
	}

	XAsufw_WriteReg(ASU_GLOBAL_ASU_NON_FATAL_ERROR_ENABLE,
			ASU_GLOBAL_ASU_NON_FATAL_ERROR_ENABLE_ASU_SW_ERROR_MASK);
	if((XAsufw_ReadReg(ASU_GLOBAL_ASU_NON_FATAL_ERROR_MASK) &
		ASU_GLOBAL_ASU_NON_FATAL_ERROR_ENABLE_ASU_SW_ERROR_MASK) != 0U) {
		Status = XASUFW_SW_ERR_INIT_FAIL;
		goto END;
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
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
