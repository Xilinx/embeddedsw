/******************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
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

#define XSECURE_MAX_HASH_SIZE_IN_BYTES          (32U)	/**< SHA3 maximum supported hash size */

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_SHA_HW_H */
