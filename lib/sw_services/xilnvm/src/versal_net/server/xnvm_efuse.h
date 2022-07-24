/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xnvm_efuse.h
 *
 * Header file for xnvm_efuse.c
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- ---------- -------------------------------------------------------
 * 1.0   kal  12/07/2022 Initial release
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
/**
 * @addtogroup xilnvm_versal_common_error_codes XilNvm Error Codes
 * @{
 */

/**
 The following table lists the Versal common eFuse library error codes.
 */
typedef enum {
	XNVM_EFUSE_ERR_WRITE_MISC_CTRL_BITS = 0x8B00,/**< 0x8B00
						* Error in MiscCtrl Bits
						* programming */
	XNVM_EFUSE_ERR_WRITE_SEC_CTRL_BITS = 0x8C00, /**< 0x8C00
						* Error in SecCtrl Bits
						* programming */
	XNVM_EFUSE_ERR_WRITE_MISC1_CTRL_BITS = 0x8D00, /**< 0x8D00
						* Error in Misc1Ctrl Bits
						* programming */
	XNVM_EFUSE_ERR_WRITE_UDS = 0x8E00, /**< 0x8E00
						* Error in Uds
						* programming */
	XNVM_EFUSE_ERR_WRITE_PLM_UPDATE = 0x8F00, /**< 0x8F00
						* Error in PLM_UPDATE
						* programming */
	XNVM_EFUSE_ERR_WRITE_BOOT_MODE_DISABLE = 0xA000, /**< 0xA000
						* Error in BootModeDisable
						* programming */
	XNVM_EFUSE_ERR_WRITE_DME_MODE = 0xA100, /**< 0xA100
						* Error in DmeMode
						* programming */
	XNVM_EFUSE_ERR_WRITE_RO_SWAP = 0xA200, /**< 0xA200
						* Error in RO_SWAP_EN
						* programming */
	XNVM_EFUSE_ERR_WRITE_DME_REVOKE_0 = 0xA300, /**< 0xA300
						* Error in DME_REVOKE_0
						* programming */
	XNVM_EFUSE_ERR_WRITE_DME_REVOKE_1 = 0xA400, /**< 0xA400
						* Error in DME_REVOKE_1
						* programming */
	XNVM_EFUSE_ERR_WRITE_DME_REVOKE_2 = 0xA500, /**< 0xA500
						* Error in DME_REVOKE_2
						* programming */
	XNVM_EFUSE_ERR_WRITE_DME_REVOKE_3 = 0xA600, /**< 0xA600
						* Error in DME_REVOKE_3
						* programming */
	XNVM_EFUSE_ERR_DME_MODE_SET = 0xA700, /**< 0xA700
						* Error in DME_MODE
						* programming */

} XNvm_EfuseVersalNetErrorCodes;

/************************** Function Prototypes ******************************/
int XNvm_EfuseWriteAesKey(XNvm_AesKeyType KeyType, XNvm_AesKey *EfuseKey);
int XNvm_EfuseWritePpkHash(XNvm_PpkType PpkType, XNvm_PpkHash *EfuseHash);
int XNvm_EfuseWriteIv(XNvm_IvType IvType, XNvm_Iv *EfuseIv);
int XNvm_EfuseCacheLoadNPrgmProtectionBits(void);
int XNvm_EfuseReadCacheRange(u32 StartOffset, u8 RegCount, u32* Data);
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
int XNvm_EfuseWriteDmeUserKey(XNvm_DmeKeyType KeyType, XNvm_DmeKey *EfuseKey);
int XNvm_EfuseWriteDmeRevoke(u32 EnvDisFlag, XNvm_DmeRevoke RevokeNum);
int XNvm_EfuseWriteDisableInplacePlmUpdate(u32 EnvDisFlag);
int XNvm_EfuseWriteBootModeDisable(u32 EnvDisFlag, u32 BootModeMask);
int XNvm_EfuseWriteDmeMode(u32 EnvDisFlag, u32 DmeMode);
int XNvm_EfuseWritePuf(const XNvm_EfusePufHdAddr *PufHelperData);

#ifdef __cplusplus
}
#endif

#endif /* XNVM_EFUSE_H_ */
