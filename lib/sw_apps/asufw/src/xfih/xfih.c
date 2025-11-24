/**************************************************************************************************
* Copyright (c) 2023 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/******************************************************************************/
/**
*
* @file xfih.c
*
* This file gives interface to fault injection hardening macros and
* functions
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- ----------------------------------------------------------
* 1.0   mmd  06/10/23  Initial release
* 1.1   sak  06/19/23  Added code comments
* 1.2   mmd  09/04/23  Fixed review comments
*
* </pre>
*
*******************************************************************************/
/**
* @addtogroup xfih_library_apis FIH Library APIs for ASUFW
* @{
*/

#include "xil_printf.h"
#include "xfih.h"

/**
 * @brief Variable holding the success value. Should be used wherever
 *        temporal redundancy is required. The variable type used for holding
 *        success value is decided by XFIH_ENABLE_SECURE_CHECK.
 */
XFih_Var XFIH_SUCCESS;
/**
 * @brief Variable holding the failure value. Should be used wherever
 *        temporal redundancy is required. The variable type used for holding
 *        failure value is decided by XFIH_ENABLE_SECURE_CHECK.
 */
XFih_Var XFIH_FAILURE;

#if (XFIH_ENABLE_VAR_GLITCH_DETECTION == TRUE)

/******************************************************************************/
/**
 *
 * @brief Function to check integrity of fault injection hardened variable.
 * If the validation fails, it executes an illegal trap instruction to generate
 * processor exception.
 *
 * @param FihVar - Variable to be validated of type XFih_Var
 ******************************************************************************/
void XFih_Validate(XFih_Var FihVar)
{
	if (FihVar.Val != (FihVar.TransformedVal ^ XFIH_MASK)) {
		XFIH_TRIGGER_LOCKDOWN;
	}
}

#endif	/* (XFIH_ENABLE_VAR_GLITCH_DETECTION == TRUE) */
/** @} */
