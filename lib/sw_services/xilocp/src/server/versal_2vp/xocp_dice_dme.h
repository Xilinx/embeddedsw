/***************************************************************************************************
* Copyright (c) 2025, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xocp_dice_dme.h
* @addtogroup xil_ocpapis APIs
* @{
*
* This file contains the xilocp DME challenge signature and DICE CDI declarations for versal_2vp.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.5   tvp  06/05/25 Initial release
*
* </pre>
*
* @note
*
* @endcond
***************************************************************************************************/

#ifndef XOCP_DICE_DME_H
#define XOCP_DICE_DME_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files ********************************************/

#include "xplmi_config.h"
#ifdef PLM_OCP
#include "xocp.h"
#include "xsecure_elliptic.h"
#include "xsecure_sha.h"

/************************************ Constant Definitions ****************************************/
#define XOCP_DME_IV_INC					(2U)
#define XOCP_UDS_IV_INC					(1U)
#define XOCP_SECURE_IV_LEN_IN_BYTES			(12U)
#define XOCP_SECURE_IV_LEN_IN_WORDS			(3U)
#define XOCP_AES_OCP_DATA_LEN_IN_BYTES			(48U)
#define XOCP_AES_OCP_DATA_LEN_IN_WORDS			(12U)
#define XOCP_AES_GCM_TAG_SIZE				(16U)
#define XOCP_WORD_SIZE					(4U)

#define XOCP_DME_INVALID_PRIVATE_KEY			(0xFFU)

#define XOCP_EFUSE_CACHE_MISC_CTRL_CRYPTO_KAT_EN_MASK	(0X00008000U) /**< Mask to enable running of
								       * KAT for Crypto engines */
/************************************** Type Definitions ******************************************/

/************************************ Function Prototypes *****************************************/
int XOcp_GenerateDiceCdi(void);
int XOcp_GenerateDmeResponseImpl(u64 NonceAddr, u64 DmeStructResAddr);

#endif /* PLM_OCP */

#ifdef __cplusplus
}
#endif
#endif /* XOCP_DICE_DME_H_ */
