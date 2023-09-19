/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file net/common/xnvm_validate.h
* @addtogroup xnvm_versal_net_api_ids XilNvm Versal Net API IDs
* @{
* @cond xnvm_internal
* This file contains the APIs used to validate write request for different eFUSEs.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 3.0   har  07/21/22 Initial release
* 3.2   yog  09/13/23 Added XNvm_IsDmeModeEn() API
*
* </pre>
* @note
*
* @endcond
******************************************************************************/

#ifndef XNVM_VALIDATE_H
#define XNVM_VALIDATE_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xnvm_common_defs.h"

/************************** Constant Definitions ****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
int XNvm_EfuseValidateAesKeyWriteReq(XNvm_AesKeyType KeyType);
int XNvm_EfuseValidatePpkHashWriteReq(XNvm_PpkType PpkType);
int XNvm_EfuseValidateIvWriteReq(XNvm_IvType IvType, XNvm_Iv *EfuseIv);
int XNvm_EfuseCheckZeros(u32 CacheOffset, u32 Count);
int XNvm_EfuseValidateDecOnlyRequest(void);
int XNvm_EfuseValidateFipsInfo(u32 FipsMode, u32 FipsVersion);
int XNvm_IsDmeModeEn(void);

/**
 * @}
 * @endcond
 */

#ifdef __cplusplus
}
#endif

#endif  /* XNVM_VALIDATE_H */
