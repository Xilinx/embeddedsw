/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_cryptochk.c
* This file contains functions common for AES, SHA, RSA and ECDSA for Versal.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 1.0   har     09/16/21 Initial Release
* 4.6   har     09/16/21 Updated release version to 4.6
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_cryptochk.h"
#include "xsecure_utils.h"
#include "xsecure_error.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes ******************************/

/************************** Function Definitions *****************************/

/***************************************************************************/
/**
 * @brief	This function checks if the EXPORT control eFuse is
 * 		programmed and PL loading is done
 *
 * @return	XST_SUCCESS - When crypto accelerators are enabled
 *		XSECURE_ERR_CRYPTO_ACCELERATOR_DISABLED - When crypto accelerators
 * 			are disabled
 *
 ******************************************************************************/
int XSecure_CryptoCheck(void)
{
	int Status = XST_FAILURE;
	u32 ExportControl = XSecure_In32(XSECURE_EFUSE_CACHE_IP_DISABLE0) &
		XSECURE_EFUSE_CACHE_IP_DISABLE0_EXPORT_MASK;
	u32 CfuApbEos = XSecure_In32(XSECURE_CFU_APB_CFU_FGCR) &
		XSECURE_CFU_APB_CFU_FGCR_EOS_MASK;
	u32 PsSrst = XSecure_In32(XSECURE_CRP_RST_PS) &
		XSECURE_CRP_RST_PS_PS_SRST_MASK;

	if ((ExportControl == XSECURE_EFUSE_CACHE_IP_DISABLE0_EXPORT_MASK) &&
		((CfuApbEos == XSECURE_CFU_APB_CFU_FGCR_EOS_MASK) || (PsSrst == 0U))) {
		Status = XSECURE_ERR_CRYPTO_ACCELERATOR_DISABLED;
	}
	else {
		Status = XST_SUCCESS;
	}

	return Status;
}
