/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_cryptochk.c
* This file contains functions common for AES, SHA and RSA for ZynqMP.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 4.6   kal     08/18/21 Initial release
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_cryptochk.h"
#include "xsecure_utils.h"

/************************** Constant Definitions *****************************/

#define XSECURE_IPDISABLE_EFUSE_BASE	(0xFFCC0000U)
#define XSECURE_IPDISABLE_EFUSE_OFFSET	(0x00001018U)
#define XSECURE_EXPORT_CONTROL_BIT_MASK	(0x8000U)

/************************** Function Prototypes ******************************/

/************************** Function Definitions *****************************/

/***************************************************************************/
/**
 * @brief	This function checks if the EXPORT control eFuse is
 * 		programmed
 *
 * @return	XST_SUCCESS - When EXPORT control eFUSE is not programmed
 *		XSECURE_CRYPTO_DISABLED_ERROR - When EXPORT control eFUSE is
 * 				programmed
 *
 ******************************************************************************/
u32 XSecure_CryptoCheck(void)
{
	u32 Status = XST_FAILURE;
	u32 ExportValue;

	ExportValue = XSecure_ReadReg(XSECURE_IPDISABLE_EFUSE_BASE,
			XSECURE_IPDISABLE_EFUSE_OFFSET);
	if ((ExportValue & XSECURE_EXPORT_CONTROL_BIT_MASK) == 0U) {
		Status = XST_SUCCESS;
	}
	else {
		Status = XSECURE_CRYPTO_DISABLED_ERROR;
	}

	return Status;
}
