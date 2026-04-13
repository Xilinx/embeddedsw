/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file net/server/xnvm_efuse_cdohandler.h
* This file contains the Versal_Net XilNvm eFUSE Cdo handler declaration.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 3.0  kal  07/12/2021 Initial release
* 3.2  kum  05/03/2023 Added macros and structures to handle cdo chunk boundary
* 3.3  kpt  01/22/2024 Added support to extend secure state into SWPCR
* 3.4  har  04/26/2025 Updated value of XNVM_PPK_HASH_CDO_PAYLOAD_LEN_IN_WORDS macro
*
* </pre>
*
* @note
*
******************************************************************************/

/**
* @addtogroup xnvm_server_apis XilNvm Plat Server APIs
* @{
*/

#ifndef XNVM_EFUSE_CDOHANDLER_H_
#define XNVM_EFUSE_CDOHANDLER_H_

#ifdef __cplusplus
extern "c" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifdef PLM_NVM
#include "xplmi_cmd.h"


/************************** Variable Definitions *****************************/
/** 16-bits used for Environmental monitoring, 16-bits used for Key_type and 8 words are for AES Key */
#define XNVM_AES_KEY_CDO_PAYLOAD_LEN_IN_WORDS          (9U)

#if defined(VERSAL_2VE_2VM)
/** 16-bits used for Env monitoring, 16-bits used for ppk_type and 12 words are for PPK Hash */
#define XNVM_PPK_HASH_CDO_PAYLOAD_LEN_IN_WORDS (13U)
#elif defined(VERSAL_NET)
/** 16-bits used for Env monitoring, 16-bits used for ppk_type and 8 words are for PPK Hash */
#define XNVM_PPK_HASH_CDO_PAYLOAD_LEN_IN_WORDS (9U)
#else
/*< Define XNVM_PPK_HASH_CDO_PAYLOAD_LEN_IN_WORDS for other devices here */
#endif


/** 16-bits used for Environmental monitoring, 16-bits are reserved and 12 words are for DICE UDS */
#define XNVM_UDS_CDO_PAYLOAD_LEN_IN_WORDS              (13U)

/** 16-bits used for Environmental monitoring, 16-bits are key_type and 12 words are for DME User Key */
#define XNVM_DME_KEY_CDO_PAYLOAD_LEN_IN_WORDS          (13U)

/** 16-bits used for Environmental monitoring, 16-bits are reserved,1-word is for c-hash,
 * 1-word is for Auxiliary, 1-word is for Ro swap and 127 words are for PUF SYN */
#define XNVM_PUF_CFG_CDO_PAYLOAD_LEN_IN_WORDS          (131U)


/**************************** Type Definitions *******************************/

/**
 * OCP handler function pointer
 */
typedef int (*XNvm_OcpHandler)(void);

/**
 * Structure to store eFUSE keys for CDO operations
 */
typedef struct {
	u32 AesKey[XNVM_AES_KEY_CDO_PAYLOAD_LEN_IN_WORDS];	/**< AES key array */
	u32 PpkHash[XNVM_PPK_HASH_CDO_PAYLOAD_LEN_IN_WORDS];	/**< PPK hash array */
	u32 UdsKey[XNVM_UDS_CDO_PAYLOAD_LEN_IN_WORDS];		/**< UDS key array */
	u32 DmeKey[XNVM_DME_KEY_CDO_PAYLOAD_LEN_IN_WORDS];	/**< DME key array */
	u32 PufCfg[XNVM_PUF_CFG_CDO_PAYLOAD_LEN_IN_WORDS];	/**< PUF configuration array */
} XNvm_CdoEfuseKeys;

/**
 * Structure to store CDO chunk data with keys and memory clear flag
 */
typedef struct {
	XNvm_CdoEfuseKeys Keys;		/**< eFUSE keys structure */
	u8 MemClear;			/**< Memory clear flag */
} XNvm_CdoChunk;

/************************** Constant Definitions *****************************/
int XNvm_EfuseCdoHandler(XPlmi_Cmd *Cmd);
XNvm_OcpHandler XNvm_ManageOcpHandler(XNvm_OcpHandler OcpHandler);

#endif /* PLM_NVM */

#ifdef __cplusplus
}
#endif

#endif /* XNVM_EFUSE_CDOHANDLER_H_ */
/** @} */
