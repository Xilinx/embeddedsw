/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file server/core/generic/xsecure_cryptochk.h
*
* This file contains macros and functions common to AES, SHA ,RSA and ECDSA
* for Versal.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 1.0   har     09/16/21 Initial Release
* 4.6   har     09/16/21 Updated release version to 4.6
* 5.1   har     01/23/23 Corrected the value of XSECURE_CRP_RST_PS_PS_SRST_MASK
* 5.7   mb      04/17/26 Update XSecure_CryptoCheck API definitions
*
* </pre>
*
******************************************************************************/

/**
 * @addtogroup xsecure_generic_server_apis XilSecure Generic Server APIs
 * @{
 */
#ifndef XSECURE_CRYPTOCHK_H
#define XSECURE_CRYPTOCHK_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions ****************************/

/***************************** Type Definitions******************************/
/** Sources to be selected the core to check sticky bits for AES, SHA, RSA */
typedef enum {
	XSECURE_CORE_AES = 0,	/**< AES Core */
	XSECURE_CORE_SHA,	/**< SHA Core */
	XSECURE_CORE_RSA_ECC,	/**< RSA/ECC Core */
	XSECURE_CORE_ALL	/**< All Cores */
} XSecure_CoreSrc;

/**
 * @name  IP_DISABLE_0 register in EFUSE_CACHE module
 * @{
 */
/**< IP_DISABLE_0 register address and definitions */
#define XSECURE_EFUSE_CACHE_IP_DISABLE0			(0xF1250018U)
#define XSECURE_EFUSE_CACHE_IP_DISABLE0_EXPORT_MASK	(0x20000000U)
/** @} */

/**
 * @name  CFU_FGCR register in CFU_APB module
 * @{
 */
/**< CFU_FGCR register address and definitions */
#define XSECURE_CFU_APB_CFU_FGCR			(0xF12B0018U)
#define XSECURE_CFU_APB_CFU_FGCR_EOS_MASK		(0x00000002U)
/** @} */

/**
 * @name  RST_PS register in CRP module
 * @{
 */
/**< RST_PS register address and definitions */
#define XSECURE_CRP_RST_PS				(0xF126031CU)
#define XSECURE_CRP_RST_PS_PS_SRST_MASK			(0x00000004U)
/** @} */

/***************** Macros (Inline Functions) Definitions *********************/


/*****************************************************************************/

/************************** Function Prototypes ******************************/
int XSecure_CryptoCheck(XSecure_CoreSrc CoreSrc);

/** @} */
#ifdef __cplusplus
}
#endif

#endif /* XSECURE_CRYPTOCHK_H_ */
