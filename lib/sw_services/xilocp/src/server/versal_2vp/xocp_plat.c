/***************************************************************************************************
* Copyright (c) 2025, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xocp_plat.c
*
* This file contains the implementation of the platform specific OCP functions for Versal_2vp.
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
	static XOcp_DmeSignature DmeSign = {0};
	static XOcp_DevPubIk DevPubIk = {0};
	static u32 DevIkPvt[XOCP_ECC_P384_SIZE_WORDS] = {0};
	static u32 DiceCdiSeed[XOCP_ECC_P384_SIZE_WORDS] = {0};
	static u32 DiceCdiValid = 0;
	static u32 DiceCdiParity = 0;
	static XOcp_RegSpace RegSpace;

	RegSpace.DmeSignRAddr = (u32)&DmeSign.DmeSignatureR;
	RegSpace.DmeSignSAddr = (u32)&DmeSign.DmeSignatureS;
	RegSpace.DiceCdiSeedAddr = (u32)&DiceCdiSeed;
	RegSpace.DiceCdiSeedValidAddr = (u32)&DiceCdiValid;
	RegSpace.DiceCdiSeedParityAddr = (u32)&DiceCdiParity;
	RegSpace.DevIkPvtAddr = (u32)&DevIkPvt;
	RegSpace.DevIkPubXAddr = (u32)&DevPubIk.DevIkPubX;
	RegSpace.DevIkPubYAddr = (u32)&DevPubIk.DevIkPubY;

	return &RegSpace;
}

/**************************************************************************************************/
/**
 * @brief	This function reads secure efuse configuration.
 *
 * @param	EfuseConfig Pointer to XOcp_SecureConfig.
 *
 * @return
 * 		- None.
 *
 **************************************************************************************************/
void XOcp_ReadSecureConfig(XOcp_SecureConfig* EfuseConfig)
{
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
	EfuseConfig->IPDisable0 = XPlmi_In32(XOCP_EFUSE_CACHE_IP_DISABLE_0) &
			XOCP_IP_DISABLE0_MEASURED_MASK;
	EfuseConfig->RomRsvd = XPlmi_In32(XOCP_EFUSE_CACHE_ROM_RSVD) &
			XOCP_ROM_RSVD_MEASURED_MASK;
}

#endif /* PLM OCP */
