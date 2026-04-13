/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_sha_hw.h
* This file contains SHA3 core hardware definitions for spartan ultrascale plus.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 5.5   kpt  08/16/24 Initial release
*       tri  10/25/24 Added maximum supported hash size macro
* 5.6   aa   07/21/25 Removed unused macros
* 5.7   tvp  02/23/26 Move SHA mode and auto padding offset from common file
*
* </pre>
*
* @endcond
******************************************************************************/

#ifndef XSECURE_SHA_HW_H
#define XSECURE_SHA_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xsecure_utils.h"

/************************** Constant Definitions ****************************/
#define XSECURE_SHA_NUM_OF_INSTANCES            (1)	/**< Number of SHA instances*/

#define XSECURE_SHA3_DEVICE_ID			(0U) /**< SHA3 device id */

#define XSECURE_SHA_BASE_ADDRESS		(0x04100000U) /**< SHA3 base address */

#define XSECURE_SHA_DEVICE_ID           	(0U)	/**< SHA device id */

#define XSECURE_SHA_0_DEVICE_ID			(XSECURE_SHA3_DEVICE_ID)
							/**< SHA 0 device id */
#ifdef SPARTANUPLUSAES1
#define	XSECURE_MAX_HASH_SIZE_IN_BYTES		(48U)	/**< Maximum hash size in bytes supported by the hardware */
#else
#define	XSECURE_MAX_HASH_SIZE_IN_BYTES		(32U)	/**< Maximum hash size in bytes supported by the hardware */
#endif

#define XSECURE_SHA2_MODE_OFFSET		(0xA0U) /**< SHA2 Mode Register offset */
#define XSECURE_SHA3_MODE_OFFSET		(0xA0U) /**< SHA3 Mode Register offset */

#define XSECURE_SHA2_AUTO_PADDING_OFFSET	(0xA4U) /**< SHA2 Auto Padding Register offset */
#define XSECURE_SHA3_AUTO_PADDING_OFFSET	(0xA4U) /**< SHA3 Auto Padding Register offset */

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_SHA_HW_H */
