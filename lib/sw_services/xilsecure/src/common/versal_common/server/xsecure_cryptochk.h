/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_cryptochk.h
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
* 4.6   har     09/16/21 Updated relase version to 4.6
*
* </pre>
* @endcond
******************************************************************************/
#ifndef XSECURE_CRYPTOCHK_H
#define XSECURE_CRYPTOCHK_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions ****************************/

/***************************** Type Definitions******************************/
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
#define XSECURE_CRP_RST_PS_PS_SRST_MASK			(0x00000002U)
/** @} */

/***************** Macros (Inline Functions) Definitions *********************/


/*****************************************************************************/

/************************** Function Prototypes ******************************/
int XSecure_CryptoCheck(void);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_CRYPTOCHK_H_ */
/**@}*/
