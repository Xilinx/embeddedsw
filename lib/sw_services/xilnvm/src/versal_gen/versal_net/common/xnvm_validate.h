/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file versal_net/common/xnvm_validate.h
* @addtogroup xnvm_versal_net_validate_api_ids XilNvm Validate APIs
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
* 3.6   vss  08/08/25 Added DME OCP support for versal_2ve_2vm.
* 3.7   nik  01/29/26 Added support to allow use of PUF Helper Data eFUSEs for general purpose.
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
#ifndef VERSAL_2VE_2VM
int XNvm_IsDmeModeEn(void);
#endif
int XNvm_EfuseIsPufHelperDataEmpty(void);
#ifdef XNVM_ACCESS_PUF_USER_DATA
int XNvm_EfuseValidatePufUserFuseAccess(void);
#endif

#ifdef __cplusplus
}
#endif

#endif  /* XNVM_VALIDATE_H */
/** @} */
