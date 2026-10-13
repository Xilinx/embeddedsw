/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_provision_common.h
 *
 * This file contains the Provisioning function prototypes which are common across the client and
 * server.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   kp   06/27/26 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_provision_common_apis Provisioning Common APIs
 * @{
*/
#ifndef XASU_PROVISION_COMMON_H_
#define XASU_PROVISION_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasu_keymanagerinfo.h"
#include "xasu_provisioninfo.h"

/*********************************** Constant Definitions ****************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XAsu_ProvisionValidateUserEfuseParams(const XAsu_ProvisionUserEfuseParams *ParamsPtr);
s32 XAsu_ProvisionValidateUdeParams(const XAsu_ProvisionUdeParams *ParamsPtr);
s32 XAsu_ProvisionValidateDpcParams(const XAsu_DevProvisionCertParams *ParamsPtr);
u16 XAsu_ProvisionReadU16Le(const u8 *Ptr);
u32 XAsu_ProvisionReadU32Le(const u8 *Ptr);
void XAsu_ProvisionWriteU16Le(u8 *Ptr, u16 Value);
void XAsu_ProvisionWriteU32Le(u8 *Ptr, u32 Value);
s32 XAsu_ProvisionValidateUserVaultId(u32 VaultId);
s32 XAsu_ProvisionValidateUserKeyId(u32 KeyId);
s32 XAsu_ProvisionGetImageSize(u32 RecordBlobLen, u32 *ImageSizePtr);
s32 XAsu_ProvisionValidateWrapParams(const XAsu_WrapProvisionInstructionsParams *ParamsPtr);
s32 XAsu_ProvisionValidateUnwrapParams(
	const XAsu_UnwrapProvisionInstructionsParams *ParamsPtr);
s32 XAsu_ProvisionValidateCreateKeyVaultParams(
	const XAsu_KeyManagerSubVaultParams *ParamsPtr);
s32 XAsu_ProvisionValidateStoreKeyMetadata(
	const XAsu_KeyManagerKeyMetadata *MetadataPtr, u32 *DataLenPtr);
s32 XAsu_ProvisionValidateXorParams(const XAsu_KmXorKeyParams *ParamsPtr);
/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASU_PROVISION_COMMON_H_ */
/** @} */
