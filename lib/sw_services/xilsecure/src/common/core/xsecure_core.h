/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file common/core/xsecure_core.h
*
* This file contains the common definitions for versalgen core
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kpt  08/17/23 Initial release
* 5.7   tbk  04/07/26 Added combined KUP and expanded key clear mask and enum
*
* </pre>
*
******************************************************************************/

/**
 * @addtogroup xsecure_common_apis XilSecure Common Apis
 * @{
 */
#ifndef XSECURE_CORE_H
#define XSECURE_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xsecure_cryptochk.h"
#include "xsecure_defs.h"

/************************** Constant Definitions ****************************/

#define XSECURE_AES_BASEADDR				(0xF11E0000U)
						/**< AES Base Address */

/**
 * @name AES_KEY_SEL values
 * @{
 */
/**< Key select values */
#define XSECURE_AES_KEY_SEL_BBRAM_KEY			(0xBBDE6600U)
#define XSECURE_AES_KEY_SEL_BBRAM_RD_KEY		(0xBBDE8200U)
#define XSECURE_AES_KEY_SEL_BH_KEY			(0xBDB06600U)
#define XSECURE_AES_KEY_SEL_BH_RD_KEY			(0xBDB08200U)
#define XSECURE_AES_KEY_SEL_EFUSE_KEY			(0xEFDE6600U)
#define XSECURE_AES_KEY_SEL_EFUSE_RED_KEY		(0xEFDE8200U)
#define XSECURE_AES_KEY_SEL_EFUSE_USR_KEY0		(0xEF856601U)
#define XSECURE_AES_KEY_SEL_EFUSE_USR_KEY1		(0xEF856602U)
#define XSECURE_AES_KEY_SEL_EFUSE_USR_RD_KEY0		(0xEF858201U)
#define XSECURE_AES_KEY_SEL_EFUSE_USR_RD_KEY1		(0xEF858202U)
#define XSECURE_AES_KEY_SEL_KUP_KEY			(0xBDC98200U)
#define XSECURE_AES_KEY_SEL_PUF_KEY			(0xDBDE8200U)
#define XSECURE_AES_KEY_SEL_USR_KEY_0			(0xBD858201U)
#define XSECURE_AES_KEY_SEL_USR_KEY_1			(0xBD858202U)
#define XSECURE_AES_KEY_SEL_USR_KEY_2			(0xBD858204U)
#define XSECURE_AES_KEY_SEL_USR_KEY_3			(0xBD858208U)
#define XSECURE_AES_KEY_SEL_USR_KEY_4			(0xBD858210U)
#define XSECURE_AES_KEY_SEL_USR_KEY_5			(0xBD858220U)
#define XSECURE_AES_KEY_SEL_USR_KEY_6			(0xBD858240U)
#define XSECURE_AES_KEY_SEL_USR_KEY_7			(0xBD858280U)
/** @} */

/**
 * @name  AES_KEY_CLEAR register
 * @{
 */
/**< AES_KEY_CLEAR register offset and definitions */
#define XSECURE_AES_KEY_CLEAR_OFFSET			(0x00000014U)
							/**< AES Key Clear register offset */

#define XSECURE_AES_KEY_CLEAR_PUF_KEY_MASK		(0x00200000U)
							/**< PUF key clear mask */

#define XSECURE_AES_KEY_CLEAR_BBRAM_RED_KEY_MASK	(0x00100000U)
							/**< BBRAM Red key clear mask */

#define XSECURE_AES_KEY_CLEAR_BH_RED_KEY_MASK		(0x00080000U)
							/**< BH Red key clear mask */

#define XSECURE_AES_KEY_CLEAR_BH_KEY_MASK		(0x00040000U)
							/**< BH key clear mask */

#define XSECURE_AES_KEY_CLEAR_EFUSE_USER_RED_KEY_1_MASK	(0x00020000U)
							/**< eFUSE User Red key 1 clear mask */

#define XSECURE_AES_KEY_CLEAR_EFUSE_USER_RED_KEY_0_MASK	(0x00010000U)
							/**< eFUSE User Red key 0 clear mask */

#define XSECURE_AES_KEY_CLEAR_EFUSE_RED_KEY_MASK	(0x00008000U)
							/**< eFUSE Red key clear mask */

#define XSECURE_AES_KEY_CLEAR_EFUSE_USER_KEY_1_MASK	(0x00004000U)
							/**< eFUSE User key 1 clear mask */

#define XSECURE_AES_KEY_CLEAR_EFUSE_USER_KEY_0_MASK	(0x00002000U)
							/**< eFUSE User key 0 clear mask */

#define XSECURE_AES_KEY_CLEAR_EFUSE_KEY_MASK		(0x00001000U)
							/**< eFUSE key clear mask */

#define XSECURE_AES_KEY_CLEAR_USER_KEY_7_MASK		(0x00000800U)
							/**< User key 7 clear mask */

#define XSECURE_AES_KEY_CLEAR_USER_KEY_6_MASK		(0x00000400U)
							/**< User key 6 clear mask */

#define XSECURE_AES_KEY_CLEAR_USER_KEY_5_MASK		(0x00000200U)
							/**< User key 5 clear mask */

#define XSECURE_AES_KEY_CLEAR_USER_KEY_4_MASK		(0x00000100U)
							/**< User key 4 clear mask */

#define XSECURE_AES_KEY_CLEAR_USER_KEY_3_MASK		(0x00000080U)
							/**< User key 3 clear mask */

#define XSECURE_AES_KEY_CLEAR_USER_KEY_2_MASK		(0x00000040U)
							/**< User key 2 clear mask */

#define XSECURE_AES_KEY_CLEAR_USER_KEY_1_MASK		(0x00000020U)
							/**< User key 1 clear mask */

#define XSECURE_AES_KEY_CLEAR_USER_KEY_0_MASK		(0x00000010U)
							/**< User key 0 clear mask */

#define XSECURE_AES_KEY_CLEAR_KUP_KEY_MASK		(0x00000002U)
							/**< KUP key clear mask */

#define XSECURE_AES_KEY_CLEAR_AES_KEY_ZEROIZE_MASK	(0x00000001U)
							/**< AES key zeroize mask */

#define XSECURE_AES_KEY_CLR_REG_CLR_MASK		(0x00000000U)
							/**< Key clear register clear mask */

#define XSECURE_AES_KEY_CLEAR_ALL_KEYS_MASK		(0x003FFFF3U)
							/**< Clear all keys mask */

#define XSECURE_AES_KEY_CLEAR_PUF_RED_EXPANDED_KEYS_MASK	(0x003B8003U)
							/**< PUF Red expanded keys clear mask */

/** Combined mask to clear KUP key and expanded (zeroize) key simultaneously */
#define XSECURE_AES_KEY_CLEAR_KUP_AND_EXP_KEYS_MASK	(XSECURE_AES_KEY_CLEAR_KUP_KEY_MASK | \
							 XSECURE_AES_KEY_CLEAR_AES_KEY_ZEROIZE_MASK)
/** @} */

#define XSECURE_AES_KEY_DEC_SEL_BBRAM_RED		(0x0U)	/**< BBRAM Red Key */
#define XSECURE_AES_KEY_DEC_SEL_BH_RED			(0x1U)	/**< BH Red Key */
#define XSECURE_AES_KEY_DEC_SEL_EFUSE_RED		(0x2U)	/**< eFUSE Red Key */
#define XSECURE_AES_KEY_DEC_SEL_EFUSE_USR0_RED		(0x3U)	/**< eFUSE User Red Key 0 */
#define XSECURE_AES_KEY_DEC_SEL_EFUSE_USR1_RED		(0x4U)	/**< eFUSE User Red Key 1 */

#define XSECURE_AES_INVALID_CFG		(0xFFFFFFFFU)	/**< Invalid Configuration */

#define XSECURE_MAX_KEY_SOURCES		XSECURE_AES_EXPANDED_KEYS
					/**< Max key source value */

/************************** Variable Definitions *****************************/

/**************************** Type Definitions *******************************/
/** Used for selecting the Key source of AES Core. */
typedef enum {
	XSECURE_AES_BBRAM_KEY = 0,		/**< BBRAM Key */
	XSECURE_AES_BBRAM_RED_KEY,		/**< BBRAM Red Key */
	XSECURE_AES_BH_KEY,			/**< BH Key */
	XSECURE_AES_BH_RED_KEY,			/**< BH Red Key */
	XSECURE_AES_EFUSE_KEY,			/**< eFUSE Key */
	XSECURE_AES_EFUSE_RED_KEY,		/**< eFUSE Red Key */
	XSECURE_AES_EFUSE_USER_KEY_0,		/**< eFUSE User Key 0 */
	XSECURE_AES_EFUSE_USER_KEY_1,		/**< eFUSE User Key 1 */
	XSECURE_AES_EFUSE_USER_RED_KEY_0,	/**< eFUSE User Red Key 0 */
	XSECURE_AES_EFUSE_USER_RED_KEY_1,	/**< eFUSE User Red Key 1 */
	XSECURE_AES_KUP_KEY,			/**< KUP key */
	XSECURE_AES_PUF_KEY,			/**< PUF key */
	XSECURE_AES_USER_KEY_0,			/**< User Key 0 */
	XSECURE_AES_USER_KEY_1,			/**< User Key 1 */
	XSECURE_AES_USER_KEY_2,			/**< User Key 2 */
	XSECURE_AES_USER_KEY_3,			/**< User Key 3 */
	XSECURE_AES_USER_KEY_4,			/**< User Key 4 */
	XSECURE_AES_USER_KEY_5,			/**< User Key 5 */
	XSECURE_AES_USER_KEY_6,			/**< User Key 6 */
	XSECURE_AES_USER_KEY_7,			/**< User Key 7 */
	XSECURE_AES_EXPANDED_KEYS,		/**< Expanded keys */
	XSECURE_AES_PUF_RED_EXPANDED_KEYS,	/**< AES PUF,RED,KUP keys */
	XSECURE_AES_KUP_AND_EXPANDED_KEYS,	/**< KUP and Expanded keys */
	XSECURE_AES_ALL_KEYS,			/**< AES All keys */
	XSECURE_AES_INVALID_KEY,		/**< AES Invalid Key */
} XSecure_AesKeySrc;

/***************************** Function Prototypes ***************************/

/** @} */
#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_CORE_H */
