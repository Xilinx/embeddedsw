/**************************************************************************************************
* Copyright (C) 2025 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xocp_plat.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.0   rmv  07/17/25 Initial release
* 1.6   tvp  05/16/25 Add XOcp_GetRegSpace function
*       tvp  09/13/25 Moved XOcp_ReadSecureConfig to platform specific file
* 1.7   rmv  01/30/26 Refactor OCP library
*
* </pre>
*
**************************************************************************************************/

/**
 * @addtogroup xilocp_plat_apis XilOcp Platform APIs
 * @{
 */

#ifndef XOCP_PLAT_H
#define XOCP_PLAT_H

#ifdef __cplusplus
extern "C" {
#endif

/************************************** Include Files ********************************************/
#include "xplmi_config.h"
#include "xil_types.h"

#ifdef PLM_OCP
#include "xocp_hw.h"

/********************************** Constant Definitions *****************************************/
/** @cond xocp_internal
 * @{
 */
/** eFuse number of rows */
#define XOCP_EFUSE_PPK_NUM_OF_BYTES			(32U)	/**< PPK Hash number of bytes */
#define XOCP_EFUSE_PPK_HASH_NO_OF_WORDS			(XOCP_EFUSE_PPK_NUM_OF_BYTES / XOCP_WORD_LEN)
								/**< PPK Hash number of words */
#define XOCP_EFUSE_REVOCATION_NO_OF_WORDS		(8U)	/**< Revocation ID number of words */
#define XOCP_EFUSE_REVOCATION_ID_NUM_OF_BYTES		(32U)	/**< Revocation ID number of bytes */
/** @}
 * @endcond
 */

/************************************ Type Definitions *******************************************/
/**
 * Data structure to hold HW register addresses required for OCP key management
 */
typedef struct {
	u32 DmeSignRAddr;		/**< Address of DME Challenge R */
	u32 DmeSignSAddr;		/**< Address of DME Challenge S */
	u32 DiceCdiSeedAddr;		/**< Address of DICE CDI SEED_0 */
	u32 DiceCdiSeedValidAddr;	/**< Address of DICE CDI SEED VALIDITY */
	u32 DiceCdiSeedParityAddr;	/**< Address of DICE CDI SEED PARITY */
	u32 DevIkPvtAddr;		/**< Address of DEV_IK_PVT_KEY_0 */
	u32 DevIkPubXAddr;		/**< Address of DEV_IK_PUBLIC_KEY_X_0 */
	u32 DevIkPubYAddr;		/**< Address of DEV_IK_PUBLIC_KEY_Y_0 */
} XOcp_RegSpace;

/**
 * Data structure to hold secure configs.
 */
typedef struct {
	u32 BootmodeDis;	/**< BOOT_MODE_DIS_15_0 */
	u32 MiscCtrl;		/**< MISC_CTRL */
	u32 AnlgTrim3; 		/**< ANLG_TRIM_3 */
	u32 BootEnvCtrl;	/**< BOOT_ENV_CTRL considering reserved bits */
	u32 IpDisable1;		/**< IP_DISABLE_1 */
	u32 Caher1;		/**< Caher_1 */
	u32 DecOnly;		/**< DEC_ONLY */
	u32 SecCtrl;		/**< Secure control */
	u32 SecMisc1;		/**< SEC_MISC 1 */
	u32 DmeFips;		/**< DME FIPS */
	u32 IPDisable0;		/**< IP_DISABLE_0 */
	u32 RomRsvd;		/**< ROM RSVD */
	u32 RoSwapEn;		/**< RO_SWAP_EN */
} XOcp_SecureConfig;

/**************************** Macros (Inline Functions) Definitions ******************************/

/************************************ Function Prototypes ****************************************/
XOcp_RegSpace* XOcp_GetRegSpace(void);
void XOcp_ReadSecureConfig(XOcp_SecureConfig* EfuseConfig);

/********************************** Variable Definitions *****************************************/

#endif /* PLM_OCP */

#ifdef __cplusplus
}
#endif
#endif /* XOCP_PLAT_H */
/** @} */
