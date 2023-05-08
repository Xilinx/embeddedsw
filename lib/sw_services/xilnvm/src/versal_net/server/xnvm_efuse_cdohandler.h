/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file net/server/xnvm_efuse_cdohandler.h
* @addtogroup xnvm_versal_net_apis XilNvm Versal Net APIs
* @{
* This file contains the Versal_Net XilNvm eFUSE Cdo handler declaration.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 3.0  kal  07/12/2021 Initial release
* 3.2  kum  05/03/2023 Added macros and structures to handle cdo chunk boundary
*
* </pre>
*
* @note
*
******************************************************************************/

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
/*< 16-bits used for Environmental monitoring, 16-bits used for Key_type and 8 words are for AES Key> */
#define XNVM_AES_KEY_CDO_PAYLOAD_LEN_IN_WORDS          (9U)

/*< 16-bits used for Env monitoring, 16-bits used for ppk_type and 8 words are for PPK Hash> */
#define XNVM_PPK_HASH_CDO_PAYLOAD_LEN_IN_WORDS         (9U)

/*< 16-bits used for Environmental monitoring, 16-bits are reserved and 12 words are for DICE UDS> */
#define XNVM_UDS_CDO_PAYLOAD_LEN_IN_WORDS              (13U)

/*< 16-bits used for Environmental monitoring, 16-bits are key_type and 12 words are for DME User Key> */
#define XNVM_DME_KEY_CDO_PAYLOAD_LEN_IN_WORDS          (13U)

/*< 16-bits used for Environmental monitoring, 16-bits are reserved,1-word is for c-hash,
 * 1-word is for Auxiliary, 1-word is for Ro swap and 127 words are for PUF SYN> */
#define XNVM_PUF_CFG_CDO_PAYLOAD_LEN_IN_WORDS          (131U)


/**************************** Type Definitions *******************************/
typedef struct {
		u32 AesKey[XNVM_AES_KEY_CDO_PAYLOAD_LEN_IN_WORDS];
		u32 PpkHash[XNVM_PPK_HASH_CDO_PAYLOAD_LEN_IN_WORDS];
		u32 UdsKey[XNVM_UDS_CDO_PAYLOAD_LEN_IN_WORDS];
		u32 DmeKey[XNVM_DME_KEY_CDO_PAYLOAD_LEN_IN_WORDS];
		u32 PufCfg[XNVM_PUF_CFG_CDO_PAYLOAD_LEN_IN_WORDS];
} XNvm_CdoEfuseKeys;
typedef struct {
	XNvm_CdoEfuseKeys Keys;
	u8 MemClear;
} XNvm_CdoChunk;
/************************** Constant Definitions *****************************/
int XNvm_EfuseCdoHandler(XPlmi_Cmd *Cmd);

#endif /* PLM_NVM */

#ifdef __cplusplus
}
#endif

#endif /* XNVM_EFUSE_CDOHANDLER_H_ */
