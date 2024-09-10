/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/******************************************************************************/
/**
*
* @file xfih.c
* @addtogroup ROM_COMP_BOOT
* @{
* @details This file gives interface to fault injection hardening macros and
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
* @note
*
*******************************************************************************/

#include "xil_printf.h"
#include "xfih.h"

/*
 * Variables for holding the success and failure value for both FIH and non FIH
 * variables.
 */
XFih_Var XFIH_SUCCESS;
XFih_Var XFIH_FAILURE;

#if (XFIH_ENABLE_VAR_GLITCH_DETECTION == TRUE)

/******************************************************************************/
/**
 *
 * @brief Function to check integrity of fault injection hardened variable
 * TBD: If size is not constrain, we can make this function always inline
 *
 * @param[in] FihVar - Variable to be validated of type @ref XFih_Var
 *
 * @return	None
 ******************************************************************************/
void XFih_Validate(XFih_Var FihVar)
{
	if (FihVar.Val != (FihVar.TransformedVal ^ XFIH_MASK)) {
		XFIH_TRIGGER_LOCKDOWN;
	}
}

#endif	/* (XFIH_ENABLE_VAR_GLITCH_DETECTION == TRUE) */
