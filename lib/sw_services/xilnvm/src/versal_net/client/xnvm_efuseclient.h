/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file net/client/xnvm_efuseclient.h
* @addtogroup xnvm_efuse_versal_net_client_apis XilNvm eFUSE Versal Net Client APIs
* @{
* @cond xnvm_internal
* This file Contains the client function prototypes, defines and macros for
* the programming and reading eFUSEs.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 3.0   har  07/06/22 Initial release
* 3.2   har  02/21/23 Added support for writing Misc Ctrl bits and ROM Rsvd bits
*   	vek  05/31/23 Added support for Programming PUF secure control bits
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
#include "xnvm_mailbox.h"
#include "xnvm_defs.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XNvm_EfuseWrite(XNvm_ClientInstance *InstancePtr, const u64 DataAddr);
int XNvm_EfuseWriteIVs(XNvm_ClientInstance *InstancePtr, const u64 IvAddr, const u32 EnvDisFlag);
int XNvm_EfuseWriteDiceUds(XNvm_ClientInstance *InstancePtr, const u64 UdsAddr, const u32 EnvDisFlag);
int XNvm_WriteDmePrivateKey(XNvm_ClientInstance *InstancePtr, u32 DmeKeyType, const u64 DmeKeyAddr, const u32 EnvDisFlag);
int XNvm_EfuseWriteDmeMode(XNvm_ClientInstance *InstancePtr, u32 DmeMode, const u32 EnvDisFlag);
int XNvm_EfuseWriteSecCtrlBits(XNvm_ClientInstance *InstancePtr, u32 SecCtrlBits, const u32 EnvDisFlag);
int XNvm_EfuseWritePufCtrlBits(XNvm_ClientInstance *InstancePtr, u32 PufCtrlBits, const u32 EnvDisFlag);
int XNvm_EfuseWriteMiscCtrlBits(XNvm_ClientInstance *InstancePtr, u32 MiscCtrlBits, const u32 EnvDisFlag);
int XNvm_EfuseWriteBootModeDis(XNvm_ClientInstance *InstancePtr, u32 BootModeDisBits, const u32 EnvDisFlag);
int XNvm_EfuseWriteSecMisc1Bits(XNvm_ClientInstance *InstancePtr, u32 SecMisc1Bits, const u32 EnvDisFlag);
int XNvm_EfuseWriteBootEnvCtrlBits(XNvm_ClientInstance *InstancePtr, u32 BootEnvCtrlBits, const u32 EnvDisFlag);
int XNvm_EfuseWriteRomRsvdBits(XNvm_ClientInstance *InstancePtr, u32 RomRsvdBits, const u32 EnvDisFlag);
int XNvm_EfuseWriteGlitchConfigBits(XNvm_ClientInstance *InstancePtr, u32 GlitchCfgBits, const u32 EnvDisFlag);
int XNvm_EfuseWritePlmUpdate(XNvm_ClientInstance *InstancePtr, const u32 EnvDisFlag);
int XNvm_EfuseWriteDecOnly(XNvm_ClientInstance *InstancePtr, const u32 EnvDisFlag);
int XNvm_EfuseWriteFipsInfo(XNvm_ClientInstance *InstancePtr, const u16 FipsMode, const u16 FipsVersion, const u32 EnvDisFlag);
int XNvm_EfuseWriteRevocationId(XNvm_ClientInstance *InstancePtr, const u32 RevokeIdNum, const u32 EnvDisFlag);
int XNvm_EfuseWriteOffChipRevocationId(XNvm_ClientInstance *InstancePtr, const u32 OffChipRevokeIdNum, const u32 EnvDisFlag);
int XNvm_EfuseWritePuf(XNvm_ClientInstance *InstancePtr, const u64 PufHdAddr);
int XNvm_EfuseReadPuf(XNvm_ClientInstance *InstancePtr, u64 PufHdAddr);
int XNvm_EfuseReadIv(XNvm_ClientInstance *InstancePtr, const u64 IvAddr, const XNvm_IvType IvType);
int XNvm_EfuseReadRevocationId(XNvm_ClientInstance *InstancePtr, const u64 RevokeIdAddr, const XNvm_RevocationId RevokeIdNum);
int XNvm_EfuseReadUserFuses(XNvm_ClientInstance *InstancePtr, u64 UserFuseAddr);
int XNvm_EfuseReadMiscCtrlBits(XNvm_ClientInstance *InstancePtr, const u64 MiscCtrlBits);
int XNvm_EfuseReadSecCtrlBits(XNvm_ClientInstance *InstancePtr, const u64 SecCtrlBits);
int XNvm_EfuseReadSecMisc1Bits(XNvm_ClientInstance *InstancePtr, const u64 SecMisc1Bits);
int XNvm_EfuseReadBootEnvCtrlBits(XNvm_ClientInstance *InstancePtr, const u64 BootEnvCtrlBits);
int XNvm_EfuseReadPufSecCtrlBits(XNvm_ClientInstance *InstancePtr, const u64 PufSecCtrlBits);
int XNvm_EfuseReadOffchipRevokeId(XNvm_ClientInstance *InstancePtr, const u64 OffChipIdAddr, const XNvm_OffchipId OffChipIdNum);
int XNvm_EfuseReadPpkHash(XNvm_ClientInstance *InstancePtr, const u64 PpkHashAddr, const XNvm_PpkType PpkHashType);
int XNvm_EfuseReadDecOnly(XNvm_ClientInstance *InstancePtr, const u64 DecOnlyAddr);
int XNvm_EfuseReadDna(XNvm_ClientInstance *InstancePtr, const u64 DnaAddr);
int XNvm_EfuseReadBootModeDis(XNvm_ClientInstance *InstancePtr, const u64 BootModeDisAddr);
int XNvm_EfuseReadRomRsvdBits(XNvm_ClientInstance *InstancePtr, const u64 RomRsvdBits);
int XNvm_EfuseReadDmeMode(XNvm_ClientInstance *InstancePtr, const u64 DmeModeAddr);
int XNvm_EfuseReadFipsInfoBits(XNvm_ClientInstance *InstancePtr, const u64 FipsInfoBits);

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XNVM_EFUSECLIENT_H */
