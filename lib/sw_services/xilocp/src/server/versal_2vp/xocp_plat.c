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
#endif /* PLM OCP */
