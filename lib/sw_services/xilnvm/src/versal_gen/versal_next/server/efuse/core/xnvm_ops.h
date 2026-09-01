/******************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
*
* @file versal_next/server/efuse/core/xnvm_ops.h
*
* Band-engine eFuse field validation and operation helpers.
*
* @cond xnvm_internal
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 3.8   vss  08/20/2026 Initial release
*
* </pre>
*
* @note
*
* @endcond
*
*******************************************************************************/
#ifndef XNVM_EFUSE_OPS_H_
#define XNVM_EFUSE_OPS_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xnvm_common_defs.h"
#include "xnvm_efuse_common_field.h"

/************************** Function Prototypes ******************************/

int XNvm_EfuseOpsValidateWriteField(const XNvm_EfuseDevice *Dev,
		XNvm_EfuseFieldId FieldId, const u32 *Data, u32 NumWords);
int XNvm_EfuseOpsValidateFieldWrLk(const XNvm_EfuseFieldDesc *Field);
void XNvm_EfuseOpsBindHooks(void);
int XNvm_EfuseOpsCacheOperations(void);
int XNvm_EfuseOpsValidateCrc(void);
int XNvm_EfuseOpsChangeEndianness(u8 *Dest, u8 *Src, u32 Size);
#ifndef XNVM_ACCESS_PUF_USER_DATA
int XNvm_EfuseOpsValidatePufHdAccess(void);
#endif
#ifdef __cplusplus
}
#endif

#endif /* XNVM_EFUSE_OPS_H_ */
