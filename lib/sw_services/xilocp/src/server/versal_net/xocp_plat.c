/***************************************************************************************************
* Copyright (c) 2025, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xocp_plat.c
*
* This file contains the implementation of the platform specific OCP functions for Versal_net.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.6   tvp  05/16/25 Initial release
*       tvp  09/13/25 Moved XOcp_ReadSecureConfig from xocp.c to platform file
*
* </pre>
*
***************************************************************************************************/

/*************************************** Include Files ********************************************/
#include "xplmi_config.h"

#ifdef PLM_OCP
#include "xocp.h"
#include "xocp_plat.h"
#include "xocp_hw.h"

/************************************ Constant Definitions ****************************************/

/************************************** Type Definitions ******************************************/

/*************************** Macros (Inline Functions) Definitions ********************************/

/************************************ Function Prototypes *****************************************/

/************************************ Variable Definitions ****************************************/

/************************************ Function Definitions ****************************************/

/**************************************************************************************************/
/**
 * @brief      This function provides OCP related register space.
 *
 * @return
 * 		- XOcp_RegSpace pointer to register space.
 *
 **************************************************************************************************/
XOcp_RegSpace* XOcp_GetRegSpace(void)
{
	static XOcp_RegSpace RegSpace = {
		.DmeSignRAddr = XOCP_PMC_GLOBAL_DME_CHALLENGE_SIGNATURE_R_0,
		.DmeSignSAddr = XOCP_PMC_GLOBAL_DME_CHALLENGE_SIGNATURE_S_0,
		.DiceCdiSeedAddr = XOCP_PMC_GLOBAL_DICE_CDI_SEED_0,
		.DiceCdiSeedValidAddr = XOCP_PMC_GLOBAL_DICE_CDI_SEED_VALID,
		.DiceCdiSeedParityAddr = XOCP_PMC_GLOBAL_DICE_CDI_SEED_PARITY,
		.DevIkPvtAddr = XOCP_PMC_GLOBAL_DEV_IK_PRIVATE_0,
		.DevIkPubXAddr = XOCP_PMC_GLOBAL_DEV_IK_PUBLIC_X_0,
		.DevIkPubYAddr = XOCP_PMC_GLOBAL_DEV_IK_PUBLIC_Y_0,
	};

	return &RegSpace;
}

void XOcp_ReadSecureConfig(XOcp_SecureConfig* EfuseConfig)
{
	EfuseConfig->BootmodeDis = XPlmi_In32(XOCP_PMC_LOCAL_BOOT_MODE_DIS) &
			XOCP_PMC_LOCAL_BOOT_MODE_DIS_FULLMASK;
	EfuseConfig->BootEnvCtrl = XPlmi_In32(XOCP_EFUSE_CACHE_BOOT_ENV_CTRL);
	EfuseConfig->IpDisable1 = XPlmi_In32(XOCP_EFUSE_CACHE_IP_DISABLE_1);
	EfuseConfig->SecMisc1 = XPlmi_In32(XOCP_EFUSE_CACHE_SECURITY_MISC_1);
	EfuseConfig->Caher1 = XPlmi_In32(XOCP_EFUSE_CACHE_CAHER_1) &
						XOCP_CAHER_1_MEASURED_MASK;
	EfuseConfig->DecOnly = XPlmi_In32(XOCP_EFUSE_CACHE_SECURITY_MISC_0) &
			XOCP_DEC_ONLY_MEASURED_MASK;
	EfuseConfig->SecCtrl = XPlmi_In32(XOCP_EFUSE_CACHE_SECURITY_CONTROL) &
			XOCP_SEC_CTRL_MEASURED_MASK;
	EfuseConfig->MiscCtrl = XPlmi_In32(XOCP_EFUSE_CACHE_MISC_CTRL) &
			XOCP_MISC_CTRL_MEASURED_MASK;
	EfuseConfig->AnlgTrim3 = XPlmi_In32(XOCP_EFUSE_CACHE_ANLG_TRIM_3);
	EfuseConfig->DmeFips = XPlmi_In32(XOCP_EFUSE_CACHE_DME_FIPS) &
			XOCP_DME_FIPS_MEASURED_MASK;
	EfuseConfig->IPDisable0 = XPlmi_In32(XOCP_EFUSE_CACHE_IP_DISABLE_0) &
			XOCP_IP_DISABLE0_MEASURED_MASK;
	EfuseConfig->RomRsvd = XPlmi_In32(XOCP_EFUSE_CACHE_ROM_RSVD) &
			XOCP_ROM_RSVD_MEASURED_MASK;
	EfuseConfig->RoSwapEn = XPlmi_In32(XOCP_EFUSE_CACHE_RO_SWAP_EN);
}

#endif /* PLM OCP */
