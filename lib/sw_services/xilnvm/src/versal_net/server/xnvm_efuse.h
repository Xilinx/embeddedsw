/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file net/server/xnvm_efuse.h
 * @addtogroup xnvm_versal_net_efuse_apis XilNvm Versal Net eFuse APIs
 * @{
 * Header file for xnvm_efuse.c
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- ---------- -------------------------------------------------------
 * 3.0   kal  07/12/2022 Initial release
 * 3.2   har  02/21/2023 Added support for writing ROM Rsvd bits
 *	 kpt  07/26/2023 Removed XNvm_EfuseReadCacheRange
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/
#ifndef XNVM_EFUSE_H_
#define XNVM_EFUSE_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xnvm_defs.h"
#include "xnvm_efuse_common.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
int XNvm_EfuseWriteAesKey(u32 EnvDisFlag, XNvm_AesKeyType KeyType, XNvm_AesKey *EfuseKey);
int XNvm_EfuseWritePpkHash(u32 EnvDisFlag, XNvm_PpkType PpkType, XNvm_PpkHash *EfuseHash);
int XNvm_EfuseWriteIv(u32 EnvDisFlag, XNvm_IvType IvType, XNvm_Iv *EfuseIv);
int XNvm_EfuseCacheLoadNPrgmProtectionBits(void);
int XNvm_EfuseWriteGlitchConfigBits(u32 EnvDisFlag, u32 GlitchConfig);
int XNvm_EfuseWriteDecOnly(u32 EnvDisFlag);
int XNvm_EfuseWriteRevocationID(u32 EnvDisFlag, u32 RevokeIdNum);
int XNvm_EfuseWriteOffChipRevokeID(u32 EnvDisFlag, u32 OffchipIdNum);
int XNvm_EfuseWriteMiscCtrlBits(u32 EnvDisFlag, u32 MiscCtrlBits);
int XNvm_EfuseWriteSecCtrlBits(u32 EnvDisFlag, u32 SecCtrlBits);
int XNvm_EfuseWriteMisc1Bits(u32 EnvDisFlag, u32 Misc1Bits);
int XNvm_EfuseWriteBootEnvCtrlBits(u32 EnvDisFlag, u32 BootEnvCtrlBits);
int XNvm_EfuseWriteFipsInfo(u32 EnvDisFlag, u32 FipsMode, u32 FipsVersion);
int XNvm_EfuseWriteUds(u32 EnvDisFlag, XNvm_Uds *EfuseUds);
int XNvm_EfuseWriteDmeUserKey(u32 EnvDisFlag, XNvm_DmeKeyType KeyType, XNvm_DmeKey *EfuseKey);
int XNvm_EfuseWriteDmeRevoke(u32 EnvDisFlag, XNvm_DmeRevoke RevokeNum);
int XNvm_EfuseWriteDisableInplacePlmUpdate(u32 EnvDisFlag);
int XNvm_EfuseWriteBootModeDisable(u32 EnvDisFlag, u32 BootModeMask);
int XNvm_EfuseWriteDmeMode(u32 EnvDisFlag, u32 DmeMode);
int XNvm_EfuseWritePuf(const XNvm_EfusePufHdAddr *PufHelperData);
int XNvm_EfuseWriteCrc(u32 EnvDisFlag, u32 Crc);
int XNvm_EfuseWriteRomRsvdBits(u32 EnvDisFlag, u32 RomRsvdBits);
int XNvm_EfuseWritePufSecCtrl(u32 EnvDisFlag,u32 PufCtrlBits);

#ifdef __cplusplus
}
#endif

#endif /* XNVM_EFUSE_H_ */
