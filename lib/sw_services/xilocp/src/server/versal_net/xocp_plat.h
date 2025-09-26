/***************************************************************************************************
* Copyright (C) 2025, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xocp_plat.h
*
* This file contains the platform specific declarations for Versal_net.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.6   tvp  05/16/25 Initial release
*       tvp  09/13/25 Moved XOcp_ReadSecureConfig to platform file
*
* </pre>
*
***************************************************************************************************/
#ifndef XOCP_PLAT_H
#define XOCP_PLAT_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files ********************************************/
#include "xplmi_config.h"

#ifdef PLM_OCP
#include "xocp_hw.h"

/************************************ Constant Definitions ****************************************/

/************************************** Type Definitions ******************************************/
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
	u32 BootmodeDis;	/* BOOT_MODE_DIS_15_0 */
	u32 MiscCtrl;		/* MISC_CTRL */
	u32 AnlgTrim3; 		/* ANLG_TRIM_3 */
	u32 BootEnvCtrl;	/* BOOT_ENV_CTRL considering reserved bits */
	u32 IpDisable1;		/* IP_DISABLE_1 */
	u32 Caher1;		/* Caher_1 */
	u32 DecOnly;		/* DEC_ONLY */
	u32 SecCtrl;		/* Secure control */
	u32 SecMisc1;		/* SEC_MISC 1 */
	u32 DmeFips;		/* DME FIPS */
	u32 IPDisable0;		/* IP_DISABLE_0 */
	u32 RomRsvd;		/* ROM RSVD */
	u32 RoSwapEn;		/* RO_SWAP_EN */
} XOcp_SecureConfig;

/*************************** Macros (Inline Functions) Definitions ********************************/

/************************************ Function Prototypes *****************************************/
XOcp_RegSpace* XOcp_GetRegSpace(void);
void XOcp_ReadSecureConfig(XOcp_SecureConfig* EfuseConfig);

/************************************ Variable Definitions ****************************************/

#endif /* PLM_OCP */

#ifdef __cplusplus
}
#endif
#endif  /* XOCP_PLAT_H */
