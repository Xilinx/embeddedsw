/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/******************************************************************************/
/**
*
* @file xfih_config.h
* @{
* @details This file allows user to configure and choose level of Fault Injection
*          Hardening required
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- ----------------------------------------------------------
* 1.0   mmd  06/16/23 Initial release
* 1.1   sak  06/19/23 Added comments and closed review comments
* 1.2   mmd  09/04/23 Updated comments and added callback macro for lockdown
* </pre>
*
* @note
*
*******************************************************************************/

#ifndef XFIH_CONFIG_H
#define XFIH_CONFIG_H

#include "xil_types.h"

/**
 * @brief When XFIH_ENABLE_SECURE_CHECK set to TRUE, the fault injection
 *        hardened variables are used. The fault injection hardened variable
 *        stores the value in original form and transformed form. And these
 *        two values stored later used for fault injection hardened comparison.
 *
 * @ref XFih_Var
 *
 */
#define XFIH_ENABLE_SECURE_CHECK    TRUE

/**
 * @brief When XFIH_ENABLE_VAR_GLITCH_DETECTION set to TRUE, the validation of
 *        fault injection hardened variables used in comparison will be done
 *        before they are used for comparison. ( @ref XFih_Validate)
 *
 * @note Make sure to enable @ref XFIH_ENABLE_SECURE_CHECK also to use this
 *       feature
 */
#define XFIH_ENABLE_VAR_GLITCH_DETECTION    FALSE

/**
 * @brief When XFIH_ENABLE_CFI set to TRUE, the control flow integrity will be
 *        enabled.
 */
#define XFIH_ENABLE_CFI    TRUE

/**
 * @brief This macro provides callback provision before going to secure lockdown.
 *        User can add function call or code to be executed just before lockdown.
 *
 * @ref XFIH_CALL_ON_ERROR_LOCKDOWN
 */
#define XFIH_CALLBACK_BEFORE_LOCKDOWN(Error)	MyErrorHandler(Error)

#endif	/* XFIH_CONFIG_H */
/** @} */
