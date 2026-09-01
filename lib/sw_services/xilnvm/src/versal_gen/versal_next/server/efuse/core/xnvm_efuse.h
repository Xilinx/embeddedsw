/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xnvm_efuse.h
 *
 * This file contains function declarations of eFuse APIs
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- ---------- -------------------------------------------------------
 * 3.0   kal  07/12/2022 Initial release
 * 3.2   har  02/21/2023 Added support for writing ROM Rsvd bits
 *	 kpt  07/26/2023 Removed XNvm_EfuseReadCacheRange
 *       vss  12/31/2023 Added support for Program the eFuse protection bits only once
 * 3.7   nik  01/06/2026 Added support to allow use of PUF Helper Data eFUSEs for general purpose.
 * 3.8   vss  08/20/2026 Redesigned efuse programming flow.
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
#include "xnvm_efuse_hw.h"
#include "xnvm_common_defs.h"
#include "xnvm_efuse_plat.h"

/** @cond xnvm_internal */
/************************** Constant Definitions *****************************/

#define XNVM_EFUSE_PROGRAM_VERIFY		(XIL_UNSIGNED_ZERO)
#define XNVM_EFUSE_SKIP_VERIFY			(XIL_UNSIGNED_ONE)
#define XNVM_EFUSE_BIT0_MASK			(0x01U)

/**************************** Type Definitions *******************************/

/** Rectangular fuse region for XNvm_EfusePgmAndVerifyData(). */
typedef struct {
	u32 StartRow;		/**< First fuse row within the page */
	u32 ColStart;		/**< First column (inclusive) */
	u32 ColEnd;		/**< Last column (inclusive) */
	u32 NumOfRows;		/**< Number of consecutive rows to program */
	u32 SkipVerify;		/**< XNVM_EFUSE_SKIP_VERIFY or XNVM_EFUSE_PROGRAM_VERIFY */
	XNvm_EfuseType EfuseType; /**< eFuse page select */
} XNvm_EfusePrgmInfo;

/** @endcond */

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
int XNvm_EfuseWriteCrc(u32 EnvDisFlag, u32 Crc);
int XNvm_EfuseWriteRomRsvdBits(u32 EnvDisFlag, u32 RomRsvdBits);
int XNvm_EfuseCloseController(void);
int XNvm_EfuseComputeProgrammableBits(const u32 *ReqData, u32 *PrgmData,
		u32 StartOffset, u32 EndOffset);
int XNvm_EfusePgmAndVerifyBit(u32 Page, u32 Row, u32 Col, u32 SkipVerify);
int XNvm_EfusePgmAndVerifyData(const XNvm_EfusePrgmInfo *PrgmInfo,
		const u32 *RowData, u32 NumWords);
#ifdef __cplusplus
}
#endif

#endif /* XNVM_EFUSE_H_ */
