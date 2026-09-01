/******************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/******************************************************************************/
/**
*
* @file versal_next/server/efuse/core/xnvm_efuse_engine.h
*
* Band-engine APIs (server/PLM builds).
*
* @cond xnvm_internal
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- --------   -------------------------------------------------------
* 3.8   vss  08/20/2026 Initial release
*
* </pre>
*
* @note
*
* @endcond
*
*******************************************************************************/
#ifndef XNVM_EFUSE_ENGINE_H_
#define XNVM_EFUSE_ENGINE_H_

#ifdef __cplusplus
extern "C" {
#endif

/****************************** Include Files *********************************/
#include "xnvm_efuse_common_field.h"
#include "xnvm_efuse_common.h"
#include "xnvm_efuse.h"

/************************** Function Prototypes ******************************/

int XNvm_EfuseEngineWriteField(const XNvm_EfuseDevice *Dev,
		XNvm_EfuseFieldId FieldId, u32 EnvDisFlag, const u32 *Data,
		u32 NumWords);
int XNvm_EfuseEnginePrgmProtectionBits(const XNvm_EfuseDevice *Dev);
int XNvm_EfuseEngineOpen(u32 EnvDisFlag);
int XNvm_EfuseEngineClose(int Status);
int XNvm_EfuseEngineProgramBand(const XNvm_EfuseBand *Band, const u32 *Data,
		u32 NumWords);

#ifdef __cplusplus
}
#endif

#endif /* XNVM_EFUSE_ENGINE_H_ */
