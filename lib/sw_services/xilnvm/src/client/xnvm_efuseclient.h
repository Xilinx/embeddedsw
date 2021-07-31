/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xnvm_efuseclient.h
* @addtogroup xnvm_efuse_client_apis XilNvm eFUSE Versal Client APIs
* @{
* @cond xnvm_internal
* This file Contains the client function prototypes, defines and macros for
* the eFUSE programming and read.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kal  07/29/21 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XNVM_EFUSECLIENT_H
#define XNVM_EFUSECLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xnvm_defs.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XNvm_EfuseWrite(const u64 DataAddr);
int XNvm_EfuseWriteIVs(const u64 IvAddr, const u32 EnvDisFlag);
int XNvm_EfuseRevokePpk(const XNvm_PpkType PpkRevoke, const u32 EnvDisFlag);
int XNvm_EfuseWriteRevocationId(const u32 RevokeId, const u32 EnvDisFlag);
int XNvm_EfuseWriteUserFuses(const u64 UserFuseAddr, const u32 EnvDisFlag);
int XNvm_EfuseReadIv(const u64 IvAddr, const XNvm_IvType IvType);
int XNvm_EfuseReadRevocationId(const u64 RevokeIdAddr, const XNvm_RevocationId RevokeIdNum);
int XNvm_EfuseReadUserFuses(const u64 UserFuseAddr);
int XNvm_EfuseReadMiscCtrlBits(const u64 MiscCtrlBits);
int XNvm_EfuseReadSecCtrlBits(const u64 SecCtrlBits);
int XNvm_EfuseReadSecMisc1Bits(const u64 SecMisc1Bits);
int XNvm_EfuseReadBootEnvCtrlBits(const u64 BootEnvCtrlBits);
int XNvm_EfuseReadPufSecCtrlBits(const u64 PufSecCtrlBits);
int XNvm_EfuseReadOffchipRevokeId(const u64 OffChidIdAddr, const XNvm_OffchipId OffChipIdNum);
int XNvm_EfuseReadPpkHash(const u64 PpkHashAddr, const XNvm_PpkType PpkHashType);
int XNvm_EfuseReadDecOnly(const u64 DecOnlyAddr);
int XNvm_EfuseReadDna(const u64 DnaAddr);
#ifdef XNVM_ACCESS_PUF_USER_DATA
int XNvm_EfuseWritePufAsUserFuses(const u64 PufUserFuseAddr);
int XNvm_EfuseReadPufAsUserFuses(const u64 PufUserFuseAddr);
#endif

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XNVM_EFUSECLIENT_H */
