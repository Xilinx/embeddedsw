/******************************************************************************
* Copyright (c) 2023 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2024, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_utils.c
* This file contains common functionalities required for xilsecure Versalnet library
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 5.1   kpt     01/12/23 Initial release
*       pre     08/16/24 Removed XSecure_MemCpy64 Function
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_generic_server_apis XilSecure Generic Server APIs
* @{
*/
/***************************** Include Files *********************************/
#include "xsecure_utils.h"
#include "xsecure_plat.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes ******************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function takes the hardware core out of reset
 *
 * @param	BaseAddress	Base address of the core
 * @param	Offset		Offset of the reset register
 *
 *****************************************************************************/
void XSecure_ReleaseReset(UINTPTR BaseAddress, u32 Offset)
{
	XSecure_WriteReg(BaseAddress, Offset, XSECURE_RESET_SET);
	XSecure_WriteReg(BaseAddress, Offset, XSECURE_RESET_UNSET);

	/* Set bit when crypto is in use */
	XSecure_UpdateCryptoStatus(BaseAddress, XSECURE_SET_BIT);
}

/*****************************************************************************/
/**
 * @brief	This function places the hardware core into the reset
 *
 * @param	BaseAddress	Base address of the core
 * @param	Offset		Offset of the reset register
 *
 *****************************************************************************/
void XSecure_SetReset(UINTPTR BaseAddress, u32 Offset)
{
	XSecure_WriteReg(BaseAddress, Offset, XSECURE_RESET_SET);

	/* Clear bit when crypto is not in use */
	XSecure_UpdateCryptoStatus(BaseAddress, XSECURE_CLEAR_BIT);
}
/** @} */
