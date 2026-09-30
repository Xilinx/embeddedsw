/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
*
* @file versal_next/server/efuse/core/xnvm_efuse.c
*
* Server-side eFuse field programming entry points and low-level controller
* helpers used by the band engine.
*
* @cond xnvm_internal
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- --------   -------------------------------------------------------
* 3.0   kal  07/12/2022 Initial release
* 3.1   kal  11/01/2022 Make Revocation id number 0 as valid to align with ROM
*                       behaviour
*       skg  11/08/2022 Added In Body comments for APIs
* 3.2   har  02/22/2023 Added API to program ROM Rsvd eFUSEs.
*       vss  03/14/2023 Fixed compilation warning
*       kum  04/11/2023 Added Env monitor before efuse programming
*       kpt  07/26/2023 Removed XNvm_EfuseReadCacheRange
*       kpt  07/26/2023 Fix security review comments
*       kpt  08/28/2023 Fix SW-BP-REDUNDANCY while assigning Status to CloseStatus
*       kpt  09/05/2023 Fix SW-BP-REDUNDANCY in XNvm_EfusePrgmIv and XNvm_EfuseWriteDmeRevoke
*       kpt  09/09/2023 Avoid returning XST_SUCCESS in case of glitch
*       yog  09/13/2023 Used XNvm_IsDmeModeEn() API for reading DME Mode
*       vss  12/31/2023 Added support for Program the eFuse protection bits only once
*       kal  01/24/2024 Fixed doxygen warnings
* 3.3	kpt  02/01/2024 XNvm_EfuseWriteRoSwapEn only when RoSwap is non-zero
*	vss  04/01/2024 Fixed MISRA-C 12.1 violation and EXPRESSION_WITH_MAGIC_NUMBERS coverity warning
* 3.4   kal  05/07/2024 Fixed issue in all DME keys programming
*       vss  07/26/2024 Corrected offchipids to be programmed
*       kal  11/13/2024 Corrected logic in XNvm_EfuseWriteRevocationID function
*       obs   04/21/2025 Fixed GCC Warnings.
* 3.6   rpu  07/21/2025 Fixed GCC warnings
*       vss  08/08/2025 Added DME support for versal_2ve_2vm.
*       ng   09/19/2025 Replaced XNVM_EFUSE_XLNX_MANFAC_BITS_CRC_ERROR with XST_FAILURE
* 3.7   nik  01/06/2026 Added support to allow use of PUF Helper Data eFUSEs for general purpose.
*       obs  03/26/2026 Replaced XSysMonPsv types with XNvm_SysmonPsv
* 3.8   vss  08/20/2026 Redesigned efuse programming flow.
*       vss  08/22/2026 Inlined DME revoke programming in XNvm_EfuseWriteDmeRevoke.
*       mb   09/13/2026 Clear ISR RD_DONE after row read wait to avoid stale row data.
*       rap  09/22/2026 Fixed warnings caused due to new compiler flags added in makefile
* </pre>
*
* @note
*
* @endcond
*
*******************************************************************************/

/**
 * @addtogroup xnvm_efuse_server_apis XilNvm eFUSE Server APIs
 * @{
 */

/***************************** Include Files *********************************/
#include "xil_util.h"
#include "xnvm_efuse.h"
#include "xnvm_efuse_common_hw.h"
#include "xnvm_efuse_hw.h"
#include "xnvm_utils.h"
#include "xnvm_ops.h"
#include "xnvm_validate.h"
#include "xnvm_efuse_common.h"
#include "xnvm_efuse_engine.h"
#include "xnvm_efuse_error.h"
#include "xnvm_efuse_common_field.h"
#include "xnvm_defs.h"

#ifdef XNVM_EFUSE_HAS_2ND_AES_KEYS
#include "xnvm_efuse_plat.h"
#endif

/************************** Constant Definitions *****************************/
#define XNVM_EFUSE_MAX_FIPS_MODE		(0xFFU)
			/**< Max value of FIPS mode */

/**************************** Type Definitions *******************************/

/*************************** Function Prototypes ******************************/
static int XNvm_EfusePrgmFipsVersionBit(u32 EnvDisFlag, u32 BitIdx);
static int XNvm_EfusePgmBit(u32 Page, u32 Row, u32 Col);
static int XNvm_EfuseReadRow(u32 Page, u32 Row, u32 *RegData);
static int XNvm_EfuseVerifyBit(u32 Page, u32 Row, u32 Col);

/************************** Function Definitions *****************************/


/******************************************************************************/
/**
 * @brief	Programs an AES-class key (AES key or user key 0/1) into eFuses.
 *
 * @param	EnvDisFlag - When TRUE, skip temperature and voltage checks.
 * @param	KeyType - AES-class key type to program.
 * @param	EfuseKey - Key words to program.
 *
 * @return
 *		- XST_SUCCESS on successful write.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM on bad arguments or a key type the
 *		  platform does not provide.
 *		- XNVM_EFUSE_ERR_BEFORE_PROGRAMMING when validation fails.
 *		- XNVM_EFUSE_ERR_WRITE_AES_KEY on AES key program failure.
 *		- XNVM_EFUSE_ERR_WRITE_USER_KEY0 on user key 0 program failure.
 *		- XNVM_EFUSE_ERR_WRITE_USER_KEY1 on user key 1 program failure.
 *
 ******************************************************************************/
int XNvm_EfuseWriteAesKey(u32 EnvDisFlag, XNvm_AesKeyType KeyType, XNvm_AesKey *EfuseKey)
{
	XNvm_EfuseFieldId FieldId;
	int Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;

	if (EfuseKey == NULL) {
		goto END;
	}

#ifdef XNVM_EFUSE_HAS_2ND_AES_KEYS
	if ((KeyType >= XNVM_EFUSE_AES_KEY_2ND) &&
			(KeyType <= XNVM_EFUSE_USER_KEY_1_2ND)) {
		switch (KeyType) {
		case XNVM_EFUSE_AES_KEY_2ND:
			Status = XNvm_EfusePlatWriteAesKey2nd(EnvDisFlag,
					EfuseKey);
			break;
		case XNVM_EFUSE_USER_KEY_0_2ND:
			Status = XNvm_EfusePlatWriteUserKey0_2nd(EnvDisFlag,
					EfuseKey);
			break;
		case XNVM_EFUSE_USER_KEY_1_2ND:
			Status = XNvm_EfusePlatWriteUserKey1_2nd(EnvDisFlag,
					EfuseKey);
			break;
		default:
			Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
			break;
		}
		goto END;
	}
#endif

	Status = XNvm_EfuseOpsAesKeyTypeToField(KeyType, &FieldId);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseEngineWriteField(&XNvm_EfuseDeviceDesc, FieldId,
			EnvDisFlag, EfuseKey->Key, XNVM_EFUSE_AES_KEY_LEN_IN_WORDS);

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	Programs PPK0, PPK1, or PPK2 hash eFuses.
 *
 * @param	EnvDisFlag - When TRUE, skip temperature and voltage checks.
 * @param	PpkType - PPK hash selector (PPK0/PPK1/PPK2).
 * @param	EfuseHash - PPK hash words to program.
 *
 * @return
 *		- XST_SUCCESS on successful write.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM on bad arguments.
 *		- XNVM_EFUSE_ERR_WRITE_PPK0_HASH on PPK0 program failure.
 *		- XNVM_EFUSE_ERR_WRITE_PPK1_HASH on PPK1 program failure.
 *		- XNVM_EFUSE_ERR_WRITE_PPK2_HASH on PPK2 program failure.
 *
 ******************************************************************************/
int XNvm_EfuseWritePpkHash(u32 EnvDisFlag, XNvm_PpkType PpkType, XNvm_PpkHash *EfuseHash)
{
	int Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
	XNvm_EfuseFieldId FieldId;

	if ((PpkType <= XNVM_EFUSE_MAX_PPK_TYPE) && (EfuseHash != NULL)) {
		FieldId = (XNvm_EfuseFieldId)(XNVM_EFUSE_FIELD_PPK0_HASH + (u32)PpkType);
		Status = XNvm_EfuseEngineWriteField(&XNvm_EfuseDeviceDesc, FieldId,
				EnvDisFlag, EfuseHash->Hash,
				XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS);
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	Programs an IV-range eFuse field.
 *
 * @param	EnvDisFlag - When TRUE, skip temperature and voltage checks.
 * @param	IvType - Meta-header, black, PLM, or data-partition IV selector.
 * @param	EfuseIv - IV words to program.
 *
 * @return
 *		- XST_SUCCESS on successful write.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM on bad arguments.
 *		- XNVM_EFUSE_ERR_WRITE_META_HEADER_IV on meta-header IV failure.
 *		- XNVM_EFUSE_ERR_WRITE_BLK_OBFUS_IV on black IV failure.
 *		- XNVM_EFUSE_ERR_WRITE_PLM_IV on PLM IV failure.
 *		- XNVM_EFUSE_ERR_WRITE_DATA_PARTITION_IV on data-partition IV failure.
 *
 ******************************************************************************/
int XNvm_EfuseWriteIv(u32 EnvDisFlag, XNvm_IvType IvType, XNvm_Iv *EfuseIv)
{
	volatile int Status = XST_FAILURE;
	XNvm_EfuseFieldId FieldId;

	if ((IvType != XNVM_EFUSE_META_HEADER_IV_RANGE) &&
		(IvType != XNVM_EFUSE_BLACK_IV) &&
		(IvType != XNVM_EFUSE_PLM_IV_RANGE) &&
		(IvType != XNVM_EFUSE_DATA_PARTITION_IV_RANGE)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (EfuseIv == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	FieldId = XNVM_EFUSE_FIELD_IV_META_HEADER + (XNvm_EfuseFieldId)IvType;

	Status = XNvm_EfuseEngineWriteField(&XNvm_EfuseDeviceDesc, FieldId,
			EnvDisFlag, EfuseIv->Iv, XNVM_EFUSE_IV_NUM_OF_CACHE_ROWS);

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	Programs glitch-detector configuration and optional write lock.
 *
 * @param	EnvDisFlag - When TRUE, skip temperature and voltage checks.
 * @param	GlitchConfig - Config data in the low mask; write-lock request in
 *		  the upper unmasked bits.
 *
 * @return
 *		- XST_SUCCESS on successful write.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM when neither config nor lock is requested.
 *		- XNVM_EFUSE_ERR_WRITE_GLITCH_CFG on configuration program failure.
 *		- XNVM_EFUSE_ERR_WRITE_GLITCH_WRLK on write-lock program failure.
 *
 ******************************************************************************/
int XNvm_EfuseWriteGlitchConfigBits(u32 EnvDisFlag, u32 GlitchConfig)
{
	volatile int Status = XST_FAILURE;
	u32 GlitchCfgData = GlitchConfig & XNVM_EFUSE_GLITCH_CONFIG_DATA_MASK;
	u32 WrLkReq = GlitchConfig & (~XNVM_EFUSE_GLITCH_CONFIG_DATA_MASK);

	if ((GlitchCfgData == XIL_UNSIGNED_ZERO) && (WrLkReq == XIL_UNSIGNED_ZERO)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (GlitchCfgData != XIL_UNSIGNED_ZERO) {
		Status = XNvm_EfuseEngineWriteField(&XNvm_EfuseDeviceDesc,
				XNVM_EFUSE_FIELD_GLITCH_CONFIG, EnvDisFlag,
				&GlitchCfgData, XIL_UNSIGNED_ONE);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	if (WrLkReq != XIL_UNSIGNED_ZERO) {
		Status = XNvm_EfuseEngineWriteField(&XNvm_EfuseDeviceDesc,
				XNVM_EFUSE_FIELD_GLITCH_WR_LK, EnvDisFlag, NULL,
				XIL_UNSIGNED_ZERO);
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	Programs the DEC_EFUSE_ONLY eFuse.
 *
 * @param	EnvDisFlag - When TRUE, skip temperature and voltage checks.
 *
 * @return
 *		- XST_SUCCESS on successful write.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM on bad arguments.
 *		- XNVM_EFUSE_ERR_WRITE_DEC_EFUSE_ONLY on program failure.
 *		- XNVM_EFUSE_ERR_CACHE_PARITY on cache reload failure.
 *
 ******************************************************************************/
int XNvm_EfuseWriteDecOnly(u32 EnvDisFlag)
{
	return XNvm_EfuseEngineWriteField(&XNvm_EfuseDeviceDesc,
			XNVM_EFUSE_FIELD_DEC_ONLY, EnvDisFlag, NULL, XIL_UNSIGNED_ZERO);
}

/******************************************************************************/
/**
 * @brief	Programs one on-chip revocation ID eFuse bit.
 *
 * @param	EnvDisFlag - When TRUE, skip temperature and voltage checks.
 * @param	RevokeIdNum - Revocation ID index (0-based).
 *
 * @return
 *		- XST_SUCCESS on successful write.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM when RevokeIdNum is out of range.
 *		- XNVM_EFUSE_ERR_WRITE_REVOCATION_IDS on program failure.
 *
 * @note	Revoke ID 64 programs bit 0 of REVOCATION_ID row 2.
 *
 ******************************************************************************/
int XNvm_EfuseWriteRevocationID(u32 EnvDisFlag, u32 RevokeIdNum)
{
	u32 Data[XNVM_NUM_OF_REVOKE_ID_FUSES] = {XIL_UNSIGNED_ZERO};
	u32 RowIdx = XIL_UNSIGNED_ZERO;
	u32 BitIdx = XIL_UNSIGNED_ZERO;
	int Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;

	if (RevokeIdNum <= (XNVM_MAX_REVOKE_ID_FUSES - 1U)) {
		RowIdx = RevokeIdNum / XNVM_EFUSE_MAX_BITS_IN_ROW;
		BitIdx = RevokeIdNum % XNVM_EFUSE_MAX_BITS_IN_ROW;
		Data[RowIdx] = XIL_UNSIGNED_ONE << BitIdx;
		Status = XNvm_EfuseEngineWriteField(&XNvm_EfuseDeviceDesc,
				XNVM_EFUSE_FIELD_REVOKE_ID, EnvDisFlag,
				Data, XNVM_NUM_OF_REVOKE_ID_FUSES);
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	Programs one off-chip revocation ID eFuse bit.
 *
 * @param	EnvDisFlag - When TRUE, skip temperature and voltage checks.
 * @param	OffchipIdNum - Off-chip revocation ID index (0-based).
 *
 * @return
 *		- XST_SUCCESS on successful write.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM when OffchipIdNum is out of range.
 *		- XNVM_EFUSE_ERR_WRITE_OFFCHIP_REVOKE_IDS on program failure.
 *		- XNVM_EFUSE_ERR_CACHE_PARITY on cache reload failure.
 *
 ******************************************************************************/
int XNvm_EfuseWriteOffChipRevokeID(u32 EnvDisFlag, u32 OffchipIdNum)
{
	u32 Data[XNVM_NUM_OF_OFFCHIP_ID_FUSES] = {XIL_UNSIGNED_ZERO};
	u32 RowIdx = XIL_UNSIGNED_ZERO;
	u32 BitIdx = XIL_UNSIGNED_ZERO;
	int Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;

	if (OffchipIdNum <= (XNVM_MAX_REVOKE_ID_FUSES - 1U)) {
		RowIdx = OffchipIdNum / XNVM_EFUSE_MAX_BITS_IN_ROW;
		BitIdx = OffchipIdNum % XNVM_EFUSE_MAX_BITS_IN_ROW;
		Data[RowIdx] = XIL_UNSIGNED_ONE << BitIdx;
		Status = XNvm_EfuseEngineWriteField(&XNvm_EfuseDeviceDesc,
				XNVM_EFUSE_FIELD_OFFCHIP_REVOKE_ID, EnvDisFlag,
				Data, XNVM_NUM_OF_OFFCHIP_ID_FUSES);
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	Programs MISC_CTRL eFuses.
 *
 * @param	EnvDisFlag - When TRUE, skip temperature and voltage checks.
 * @param	MiscCtrlBits - MISC_CTRL cache word to program.
 *
 * @return
 *		- XST_SUCCESS on successful write.
 *		- XNVM_EFUSE_ERR_WRITE_MISC_CTRL_BITS on program failure.
 *		- XNVM_EFUSE_ERR_BEFORE_PROGRAMMING when validation fails.
 *
 ******************************************************************************/
int XNvm_EfuseWriteMiscCtrlBits(u32 EnvDisFlag, u32 MiscCtrlBits)
{
	return XNvm_EfuseEngineWriteField(&XNvm_EfuseDeviceDesc,
			XNVM_EFUSE_FIELD_MISC_CTRL, EnvDisFlag, &MiscCtrlBits,
			XIL_UNSIGNED_ONE);
}

/******************************************************************************/
/**
 * @brief	Programs SEC_CTRL eFuses.
 *
 * @param	EnvDisFlag - When TRUE, skip temperature and voltage checks.
 * @param	SecCtrlBits - SEC_CTRL cache word to program.
 *
 * @return
 *		- XST_SUCCESS on successful write.
 *		- XNVM_EFUSE_ERR_WRITE_SEC_CTRL_BITS on program failure.
 *		- XNVM_EFUSE_ERR_BEFORE_PROGRAMMING when validation fails.
 *
 ******************************************************************************/
int XNvm_EfuseWriteSecCtrlBits(u32 EnvDisFlag, u32 SecCtrlBits)
{
	return XNvm_EfuseEngineWriteField(&XNvm_EfuseDeviceDesc,
			XNVM_EFUSE_FIELD_SEC_CTRL, EnvDisFlag, &SecCtrlBits,
			XIL_UNSIGNED_ONE);
}

/******************************************************************************/
/**
 * @brief	Programs SECURITY_MISC_1 eFuses.
 *
 * @param	EnvDisFlag - When TRUE, skip temperature and voltage checks.
 * @param	Misc1Bits - SECURITY_MISC_1 cache word to program.
 *
 * @return
 *		- XST_SUCCESS on successful write.
 *		- XNVM_EFUSE_ERR_WRITE_MISC1_CTRL_BITS on program failure.
 *		- XNVM_EFUSE_ERR_BEFORE_PROGRAMMING when validation fails.
 *
 ******************************************************************************/
int XNvm_EfuseWriteMisc1Bits(u32 EnvDisFlag, u32 Misc1Bits)
{
	return XNvm_EfuseEngineWriteField(&XNvm_EfuseDeviceDesc,
			XNVM_EFUSE_FIELD_SEC_MISC1, EnvDisFlag, &Misc1Bits,
			XIL_UNSIGNED_ONE);
}

/******************************************************************************/
/**
 * @brief	Programs BOOT_ENV_CTRL eFuses.
 *
 * @param	EnvDisFlag - When TRUE, skip temperature and voltage checks.
 * @param	BootEnvCtrlBits - BOOT_ENV_CTRL cache word to program.
 *
 * @return
 *		- XST_SUCCESS on successful write.
 *		- XNVM_EFUSE_ERR_WRITE_BOOT_ENV_CTRL on program failure.
 *		- XNVM_EFUSE_ERR_BEFORE_PROGRAMMING when validation fails.
 *
 ******************************************************************************/
int XNvm_EfuseWriteBootEnvCtrlBits(u32 EnvDisFlag, u32 BootEnvCtrlBits)
{
	return XNvm_EfuseEngineWriteField(&XNvm_EfuseDeviceDesc,
			XNVM_EFUSE_FIELD_BOOT_ENV_CTRL, EnvDisFlag,
			&BootEnvCtrlBits, XIL_UNSIGNED_ONE);
}

/******************************************************************************/
/**
 * @brief	Programs FIPS mode and scattered FIPS version eFuses.
 *
 * @param	EnvDisFlag - When TRUE, skip temperature and voltage checks.
 * @param	FipsMode - FIPS mode value to program (may be zero to skip).
 * @param	FipsVersion - FIPS version value to program (may be zero to skip).
 *
 * @return
 *		- XST_SUCCESS on successful write.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM on bad arguments.
 *		- XNVM_EFUSE_ERR_BEFORE_PROGRAMMING when validation fails.
 *		- Field write error on program failure.
 *
 ******************************************************************************/
int XNvm_EfuseWriteFipsInfo(u32 EnvDisFlag, u32 FipsMode, u32 FipsVersion)
{
	volatile int Status = XST_FAILURE;
	u32 PrgmVer = XIL_UNSIGNED_ZERO;
	u32 BitIdx = XIL_UNSIGNED_ZERO;

	if ((XNvm_EfuseDeviceDesc.FipsVersion == NULL) ||
			(FipsVersion > XNvm_EfuseDeviceDesc.FipsVersion->MaxVersion)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	if (FipsMode > XNVM_EFUSE_MAX_FIPS_MODE) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfuseValidateFipsInfo(FipsMode, FipsVersion);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	if (FipsMode != XIL_UNSIGNED_ZERO) {
		Status = XNvm_EfuseEngineWriteField(&XNvm_EfuseDeviceDesc,
				XNVM_EFUSE_FIELD_FIPS_MODE, EnvDisFlag,
				&FipsMode, XIL_UNSIGNED_ONE);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	if (FipsVersion != XIL_UNSIGNED_ZERO) {
		PrgmVer = FipsVersion;

		for (BitIdx = XIL_UNSIGNED_ZERO; BitIdx < XNVM_EFUSE_FIPS_VERSION_NUM_BITS; BitIdx++) {
			if ((PrgmVer & XNVM_EFUSE_BIT0_MASK) != XIL_UNSIGNED_ZERO) {
				Status = XNvm_EfusePrgmFipsVersionBit(EnvDisFlag,
						BitIdx);
				if (Status != XST_SUCCESS) {
					goto END;
				}
			}
			PrgmVer = PrgmVer >> XIL_UNSIGNED_ONE;
		}
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	Programs UDS eFuses.
 *
 * @param	EnvDisFlag - When TRUE, skip temperature and voltage checks.
 * @param	EfuseUds - UDS words to program.
 *
 * @return
 *		- XST_SUCCESS on successful write.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM on bad arguments.
 *		- XNVM_EFUSE_ERR_WRITE_UDS on program failure.
 *		- XNVM_EFUSE_ERR_BEFORE_PROGRAMMING when validation fails.
 *
 ******************************************************************************/
int XNvm_EfuseWriteUds(u32 EnvDisFlag, XNvm_Uds *EfuseUds)
{
	int Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;

	if (EfuseUds != NULL) {
		Status = XNvm_EfuseEngineWriteField(&XNvm_EfuseDeviceDesc,
				XNVM_EFUSE_FIELD_UDS, EnvDisFlag, EfuseUds->Uds,
				XNVM_UDS_SIZE_IN_WORDS);
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	Programs a DME user key eFuse field.
 *
 * @param	EnvDisFlag - When TRUE, skip temperature and voltage checks.
 * @param	KeyType - DME user key 0/1/2/3 selector.
 * @param	EfuseKey - Key bytes in caller endianness; converted before program.
 *
 * @return
 *		- XST_SUCCESS on successful write.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM on bad arguments.
 *		- XNVM_EFUSE_ERR_BEFORE_PROGRAMMING when validation fails.
 *		- Field-specific write error on program failure.
 *
 ******************************************************************************/
int XNvm_EfuseWriteDmeUserKey(u32 EnvDisFlag, XNvm_DmeKeyType KeyType, XNvm_DmeKey *EfuseKey)
{
	u32 KeyBuf[XNVM_DME_USER_KEY_SIZE_IN_WORDS] = {XIL_UNSIGNED_ZERO};
	int Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
	XNvm_EfuseFieldId FieldId;

	if (KeyType > XNVM_DME_USER_KEY_MAX_VALUE) {
		goto END;
	}
	if (EfuseKey == NULL) {
		goto END;
	}

	FieldId = (XNvm_EfuseFieldId)(XNVM_EFUSE_FIELD_DME_KEY0 + (u32)KeyType);
	Status = XNvm_EfuseOpsChangeEndianness((u8 *)KeyBuf, (u8 *)EfuseKey->Key,
			sizeof(KeyBuf));
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	Status = XNvm_EfuseEngineWriteField(&XNvm_EfuseDeviceDesc, FieldId,
			EnvDisFlag, KeyBuf, XNVM_DME_USER_KEY_SIZE_IN_WORDS);

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	Programs one DME revoke eFuse bit.
 *
 * @param	EnvDisFlag - When TRUE, skip temperature and voltage checks.
 * @param	RevokeNum - DME revoke slot selector.
 *
 * @return
 *		- XST_SUCCESS on successful write.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM on bad arguments.
 *		- XST_GLITCH_ERROR on temporal fault in revoke selector.
 *		- XNVM_EFUSE_ERR_BEFORE_PROGRAMMING when validation fails.
 *		- XNVM_EFUSE_ERR_WRITE_DME_REVOKE_0 through _3 on program failure.
 *
 ******************************************************************************/
int XNvm_EfuseWriteDmeRevoke(u32 EnvDisFlag, XNvm_DmeRevoke RevokeNum)
{
	volatile int Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
	const XNvm_EfuseDevice *Dev = &XNvm_EfuseDeviceDesc;
	const XNvm_EfuseFieldDesc *Field = NULL;
	const XNvm_EfuseDmeRevokeMap *Revoke = NULL;
	const XNvm_EfuseDmeRevokeEntry *Entry = NULL;
	volatile u32 RevokeIndexTmp;
	u32 RevokeIndex = XIL_UNSIGNED_ZERO;
	u32 SubBit = XIL_UNSIGNED_ZERO;

	if (RevokeNum > XNVM_DME_REVOKE_MAX_VALUE) {
		goto END;
	}

	Field = XNvm_EfuseLookupFieldDesc(Dev, XNVM_EFUSE_FIELD_DME_REVOKE);
	if (Field == NULL) {
		goto END;
	}

	Revoke = Dev->DmeRevoke;
	if ((Revoke == NULL) || (Revoke->Table == NULL)) {
		goto END;
	}

	Status = XNvm_EfuseOpsValidateWriteField(Dev, XNVM_EFUSE_FIELD_DME_REVOKE,
			NULL, XIL_UNSIGNED_ZERO);
	if (Status != XST_SUCCESS) {
		if ((Status & (int)Field->ErrBeforePgm) == XIL_UNSIGNED_ZERO) {
			Status = (Status | (int)Field->ErrBeforePgm);
		}
		goto END;
	}

	RevokeIndex = (u32)RevokeNum;
	RevokeIndexTmp = RevokeIndex;
	if (RevokeIndexTmp != RevokeIndex) {
		Status = XST_GLITCH_ERROR;
		goto END;
	}

	if (RevokeIndex > Revoke->MaxIndex) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfuseEngineOpen(EnvDisFlag);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Entry = &Revoke->Table[RevokeIndex];
	for (SubBit = XIL_UNSIGNED_ZERO; SubBit < XNVM_EFUSE_DME_REVOKE_BITS_PER_INDEX;
			SubBit++) {
		Status = XNvm_EfusePgmAndVerifyBit(Revoke->Page, Entry->Row,
				Entry->Col[SubBit], XNVM_EFUSE_PROGRAM_VERIFY);
		if (Status != XST_SUCCESS) {
			Status = (int)((u32)Status | (Field->ErrWrite +
				(RevokeIndex << XNVM_EFUSE_ERROR_BYTE_SHIFT)));
			break;
		}
	}

	Status = XNvm_EfuseEngineClose(Status);

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	Programs the PLM in-place update disable eFuse.
 *
 * @param	EnvDisFlag - When TRUE, skip temperature and voltage checks.
 *
 * @return
 *		- XST_SUCCESS on successful write.
 *		- XNVM_EFUSE_ERR_WRITE_PLM_UPDATE on program failure.
 *		- XNVM_EFUSE_ERR_BEFORE_PROGRAMMING when validation fails.
 *
 ******************************************************************************/
int XNvm_EfuseWriteDisableInplacePlmUpdate(u32 EnvDisFlag)
{
	return XNvm_EfuseEngineWriteField(&XNvm_EfuseDeviceDesc,
			XNVM_EFUSE_FIELD_PLM_UPDATE_DISABLE, EnvDisFlag, NULL,
			XIL_UNSIGNED_ZERO);
}

/******************************************************************************/
/**
 * @brief	Programs BOOT_MODE_DISABLE eFuses.
 *
 * @param	EnvDisFlag - When TRUE, skip temperature and voltage checks.
 * @param	BootModeMask - Boot-mode disable mask to program.
 *
 * @return
 *		- XST_SUCCESS on successful write.
 *		- XNVM_EFUSE_ERR_WRITE_BOOT_MODE_DISABLE on program failure.
 *		- XNVM_EFUSE_ERR_BEFORE_PROGRAMMING when validation fails.
 *
 ******************************************************************************/
int XNvm_EfuseWriteBootModeDisable(u32 EnvDisFlag, u32 BootModeMask)
{
	return XNvm_EfuseEngineWriteField(&XNvm_EfuseDeviceDesc,
			XNVM_EFUSE_FIELD_BOOT_MODE_DIS, EnvDisFlag, &BootModeMask,
			XIL_UNSIGNED_ONE);
}

/******************************************************************************/
/**
 * @brief	Programs DME mode eFuses when supported by the platform.
 *
 * @param	EnvDisFlag - When TRUE, skip temperature and voltage checks.
 * @param	DmeMode - DME mode value to program.
 *
 * @return
 *		- XST_SUCCESS on successful write.
 *		- XNVM_EFUSE_ERROR_DME_NOT_SUPPORTED when the platform has no DME mode field.
 *		- XNVM_EFUSE_ERR_WRITE_DME_MODE on program failure.
 *		- XNVM_EFUSE_ERR_BEFORE_PROGRAMMING when validation fails.
 *
 ******************************************************************************/
int XNvm_EfuseWriteDmeMode(u32 EnvDisFlag, u32 DmeMode)
{
	int Status = (int)XNVM_EFUSE_ERROR_DME_NOT_SUPPORTED;

	if (XNvm_EfuseDeviceDesc.SupportsDmeMode != XIL_UNSIGNED_ZERO) {
		Status = XNvm_EfuseEngineWriteField(&XNvm_EfuseDeviceDesc,
				XNVM_EFUSE_FIELD_DME_MODE, EnvDisFlag, &DmeMode, XIL_UNSIGNED_ONE);
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	Programs manufacturing CRC and CRC salt eFuses.
 *
 * @param	EnvDisFlag - When TRUE, skip temperature and voltage checks.
 * @param	Crc - CRC value to program.
 *
 * @return
 *		- XST_SUCCESS on successful write.
 *		- XNVM_EFUSE_ERR_WRITE_CRC or XNVM_EFUSE_ERR_WRITE_CRC_SALT on failure.
 *		- XNVM_EFUSE_ERR_BEFORE_PROGRAMMING when validation fails.
 *
 ******************************************************************************/
int XNvm_EfuseWriteCrc(u32 EnvDisFlag, u32 Crc)
{
	return XNvm_EfuseEngineWriteField(&XNvm_EfuseDeviceDesc,
			XNVM_EFUSE_FIELD_CRC, EnvDisFlag, &Crc, XIL_UNSIGNED_ONE);
}

/******************************************************************************/
/**
 * @brief	Programs ROM reserved eFuses.
 *
 * @param	EnvDisFlag - When TRUE, skip temperature and voltage checks.
 * @param	RomRsvdBits - ROM_RSVD cache word to program.
 *
 * @return
 *		- XST_SUCCESS on successful write.
 *		- XNVM_EFUSE_ERR_WRITE_ROM_RSVD on program failure.
 *		- XNVM_EFUSE_ERR_BEFORE_PROGRAMMING when validation fails.
 *
 ******************************************************************************/
int XNvm_EfuseWriteRomRsvdBits(u32 EnvDisFlag, u32 RomRsvdBits)
{
	return XNvm_EfuseEngineWriteField(&XNvm_EfuseDeviceDesc,
			XNVM_EFUSE_FIELD_ROM_RSVD, EnvDisFlag, &RomRsvdBits,
			XIL_UNSIGNED_ONE);
}

/******************************************************************************/
/**
 * @brief	Reloads the eFuse cache and programs protection (Tbits) fuses.
 *
 * @return
 *		- XST_SUCCESS on successful cache reload and protection programming.
 *		- XNVM_EFUSE_ERR_CACHE_LOAD on cache reload failure.
 *		- XNVM_EFUSE_ERR_BEFORE_PROGRAMMING when controller setup fails.
 *		- Protection or CRC verification error on failure.
 *
 * @note	Frequent calls are discouraged; each reload reads the fuse array and
 *		may reduce eFuse lifetime.
 *
 ******************************************************************************/
int XNvm_EfuseCacheLoadNPrgmProtectionBits(void)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_EfuseEngineOpen(TRUE);
	if (Status != XST_SUCCESS) {
		goto END_CLOSE;
	}

	Status = XST_FAILURE;
	Status = XNvm_EfuseOpsCacheOperations();
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		goto END_CLOSE;
	}

	Status = XST_FAILURE;
	Status = XNvm_EfuseEnginePrgmProtectionBits(&XNvm_EfuseDeviceDesc);
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		goto END_CLOSE;
	}

	Status = XST_FAILURE;
	Status = XNvm_EfuseOpsCacheOperations();

END_CLOSE:
	Status = XNvm_EfuseEngineClose(Status);

	return Status;
}



/******************************************************************************/
/**
 * @brief	Disables programming mode and locks the eFuse controller.
 *
 * @return
 *		- XST_SUCCESS when the controller is locked cleanly.
 *		- Controller reset, disable, or lock error on failure.
 *
 ******************************************************************************/
int XNvm_EfuseCloseController(void)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_EfuseResetReadMode();
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XNvm_EfuseDisableProgramming();
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XNvm_EfuseLockController();

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	Computes one-time-programmable bits from requested and cached data.
 *
 * @param	ReqData - Requested cache values from the caller.
 * @param	PrgmData - Output buffer for (ReqData & ~Cache) words; must hold at
 *		least XNVM_EFUSE_PRGM_BUF_WORDS entries.
 * @param	StartOffset - First cache byte offset (inclusive).
 * @param	EndOffset - Last cache byte offset (inclusive).
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM on bad pointers, range, or offsets
 *		  not aligned to XNVM_WORD_LEN.
 *		- XNVM_EFUSE_ERR_CACHE_PARITY when cache parity is set.
 *
 ******************************************************************************/
int XNvm_EfuseComputeProgrammableBits(const u32 *ReqData, u32 *PrgmData,
		u32 StartOffset, u32 EndOffset)
{
	volatile int Status = XST_FAILURE;
	u32 EfuseIsrStatus = XIL_UNSIGNED_ZERO;
	u32 ReadReg = XIL_UNSIGNED_ZERO;
	volatile u32 Offset = XIL_UNSIGNED_ZERO;
	u32 WordIdx = XIL_UNSIGNED_ZERO;
	u32 NumWords = XIL_UNSIGNED_ZERO;

	if ((ReqData == NULL) || (PrgmData == NULL)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (((StartOffset % XNVM_WORD_LEN) != XIL_UNSIGNED_ZERO) ||
			((EndOffset % XNVM_WORD_LEN) != XIL_UNSIGNED_ZERO)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (EndOffset < StartOffset) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	NumWords = ((EndOffset - StartOffset) / XNVM_WORD_LEN) + 1U;
	if (NumWords > XNVM_EFUSE_PRGM_BUF_WORDS) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	EfuseIsrStatus = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
			XNVM_EFUSE_ISR_REG_OFFSET);
	if ((EfuseIsrStatus & XNVM_EFUSE_ISR_CACHE_ERROR) ==
			XNVM_EFUSE_ISR_CACHE_ERROR) {
		Status = (int)XNVM_EFUSE_ERR_CACHE_PARITY;
		goto END;
	}

	Offset = StartOffset;
	while (Offset <= EndOffset) {
		ReadReg = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR, Offset);
		WordIdx = (Offset - StartOffset) / XNVM_WORD_LEN;
		PrgmData[WordIdx] = (~ReadReg) & ReqData[WordIdx];
		Offset = Offset + XNVM_WORD_LEN;
	}

	if (Offset == (EndOffset + XNVM_WORD_LEN)) {
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	Programs and optionally verifies a single eFuse bit.
 *
 * @param	Page - eFuse page select.
 * @param	Row - Fuse row within the page.
 * @param	Col - Fuse column within the row.
 * @param	SkipVerify - XNVM_EFUSE_PROGRAM_VERIFY or XNVM_EFUSE_SKIP_VERIFY.
 *
 * @return
 *		- XST_SUCCESS on successful program (and verify when enabled).
 *		- XST_GLITCH_ERROR on temporal fault detection.
 *		- Program or verify error on failure.
 *
 ******************************************************************************/
int XNvm_EfusePgmAndVerifyBit(u32 Page, u32 Row, u32 Col, u32 SkipVerify)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	volatile u32 SkipVerifyTmp = SkipVerify;

	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmBit, Page,
			Row, Col);
	if ((Status == XST_SUCCESS) || (StatusTmp == XST_SUCCESS)) {
		if ((SkipVerify == XNVM_EFUSE_PROGRAM_VERIFY) ||
				(SkipVerifyTmp == XNVM_EFUSE_PROGRAM_VERIFY)) {
			if (SkipVerify != SkipVerifyTmp) {
				Status = XST_GLITCH_ERROR;
				goto END;
			}
			Status = XST_FAILURE;
			Status = XNvm_EfuseVerifyBit(Page, Row, Col);
		}
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	Programs and optionally verifies a rectangular fuse region.
 *
 * @param	PrgmInfo - Row/column geometry and verify policy.
 * @param	RowData - Row data words scanned LSB-first from ColStart through
 *		  ColEnd for each row. Bit 0 of the first word corresponds to
 *		  fuse column PrgmInfo->ColStart (not column 0). When ColStart
 *		  is non-zero, supply a band-relative bitmap; do not shift bits
 *		  to absolute column positions.
 * @param	NumWords - Number of valid u32 words at RowData.
 *
 * @return
 *		- XST_SUCCESS when all requested bits are programmed.
 *		- XNVM_EFUSE_ERR_NTHG_TO_BE_PROGRAMMED when no bits are set.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM on NULL pointers, bad geometry,
 *		  zero rows, StartRow + NumOfRows overflow, or undersized RowData.
 *		- XNVM_EFUSE_ERR_GLITCH_DETECTED when the row loop did not complete.
 *		- XST_GLITCH_ERROR or program/verify error on failure.
 *
 ******************************************************************************/
int XNvm_EfusePgmAndVerifyData(const XNvm_EfusePrgmInfo *PrgmInfo,
		const u32 *RowData, u32 NumWords)
{
	volatile int Status = (int)XNVM_EFUSE_ERR_NTHG_TO_BE_PROGRAMMED;
	const u32 *DataPtr = RowData;
	volatile u32 Row = XIL_UNSIGNED_ZERO;
	volatile u32 EndRow = XIL_UNSIGNED_ZERO;
	u32 Idx = XIL_UNSIGNED_ZERO;
	u32 Col = XIL_UNSIGNED_ZERO;
	u32 Data = XIL_UNSIGNED_ZERO;
	u32 ColSpan = XIL_UNSIGNED_ZERO;
	u32 TotalBits = XIL_UNSIGNED_ZERO;
	u32 BitsDone = XIL_UNSIGNED_ZERO;

	if ((PrgmInfo == NULL) || (RowData == NULL) ||
			(PrgmInfo->NumOfRows == XIL_UNSIGNED_ZERO)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if ((PrgmInfo->StartRow + PrgmInfo->NumOfRows) < PrgmInfo->StartRow) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Row = PrgmInfo->StartRow;
	EndRow = PrgmInfo->StartRow + PrgmInfo->NumOfRows;

	if (EndRow > (XNVM_EFUSE_LAST_ROW_IN_PAGE + 1U)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if ((u32)PrgmInfo->EfuseType > (u32)XNVM_EFUSE_PAGE_MAX) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	if (PrgmInfo->ColStart > PrgmInfo->ColEnd) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	if (PrgmInfo->ColEnd >= XNVM_EFUSE_MAX_BITS_IN_ROW) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	ColSpan = PrgmInfo->ColEnd - PrgmInfo->ColStart + 1U;
	TotalBits = PrgmInfo->NumOfRows * ColSpan;

	if (TotalBits / PrgmInfo->NumOfRows != ColSpan) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (NumWords == XIL_UNSIGNED_ZERO) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (((TotalBits + (XNVM_EFUSE_MAX_BITS_IN_ROW - 1U)) /
			XNVM_EFUSE_MAX_BITS_IN_ROW) > NumWords) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Data = *DataPtr;
	while (Row < EndRow) {
		Col = PrgmInfo->ColStart;
		while (Col <= PrgmInfo->ColEnd) {
			if ((Data & XNVM_EFUSE_BIT0_MASK) != XIL_UNSIGNED_ZERO) {
				Status = XST_FAILURE;
				Status = XNvm_EfusePgmAndVerifyBit(
						(u32)PrgmInfo->EfuseType, Row,
						Col, PrgmInfo->SkipVerify);
				if (Status != XST_SUCCESS) {
					XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
					goto END;
				}
			}
			Col++;
			Idx++;
			BitsDone++;
			if (Idx == XNVM_EFUSE_MAX_BITS_IN_ROW) {
				Idx = XIL_UNSIGNED_ZERO;
				if (BitsDone < TotalBits) {
					DataPtr++;
					Data = *DataPtr;
				} else {
					Data = XIL_UNSIGNED_ZERO;
				}
			} else {
				Data = Data >> XIL_UNSIGNED_ONE;
			}
		}
		Row++;
	}

	if (Row != EndRow) {
		Status = (int)XNVM_EFUSE_ERR_GLITCH_DETECTED;
	}

END:
	return Status;
}




/******************************************************************************/
/**
 * @brief	Programs one scattered FIPS version fuse bit (legacy column path).
 *
 * @param	EnvDisFlag - When TRUE, skip temperature and voltage checks.
 * @param	BitIdx - Version bit index (0..2).
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM on bad index or missing platform map.
 *		- Field write error on program failure.
 *
 * @note	Full FIPS version validation is the caller's responsibility.
 *
 ******************************************************************************/
static int XNvm_EfusePrgmFipsVersionBit(u32 EnvDisFlag, u32 BitIdx)
{
	volatile int Status = XST_FAILURE;
	const XNvm_EfuseDevice *Dev = &XNvm_EfuseDeviceDesc;
	const XNvm_EfuseFieldDesc *Field;
	const XNvm_EfuseFipsVersionMap *Map;

	Field = XNvm_EfuseLookupFieldDesc(Dev, XNVM_EFUSE_FIELD_FIPS_VERSION);
	Map = Dev->FipsVersion;
	if ((Field == NULL) || (Map == NULL) ||
			(BitIdx >= XNVM_EFUSE_FIPS_VERSION_NUM_BITS) ||
			(Map->BitCols[BitIdx] == XNVM_EFUSE_INVALID_COL)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfuseEngineOpen(EnvDisFlag);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfusePgmAndVerifyBit((u32)Map->Page, Map->Row,
			Map->BitCols[BitIdx], XNVM_EFUSE_PROGRAM_VERIFY);
	if (Status != XST_SUCCESS) {
		Status = (Status | (int)Field->ErrWrite);
	}

	Status = XNvm_EfuseEngineClose(Status);

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	Programs a single eFuse bit through the controller.
 *
 * @param	Page - eFuse page select.
 * @param	Row - Fuse row within the page.
 * @param	Col - Fuse column within the row.
 *
 * @return
 *		- XST_SUCCESS on successful program.
 *		- XNVM_EFUSE_ERR_PGM_TIMEOUT on controller timeout.
 *		- XNVM_EFUSE_ERR_PGM on controller program error.
 *
 ******************************************************************************/
static int XNvm_EfusePgmBit(u32 Page, u32 Row, u32 Col)
{
	volatile int Status = XST_FAILURE;
	u32 PgmAddr = XIL_UNSIGNED_ZERO;
	u32 EventMask = XIL_UNSIGNED_ZERO;

	PgmAddr = (Page << XNVM_EFUSE_ADDR_PAGE_SHIFT) |
		(Row << XNVM_EFUSE_ADDR_ROW_SHIFT) |
		(Col << XNVM_EFUSE_ADDR_COLUMN_SHIFT);

	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
		XNVM_EFUSE_PGM_ADDR_REG_OFFSET, PgmAddr);

	Status = (int)Xil_WaitForEvents((UINTPTR)(XNVM_EFUSE_CTRL_BASEADDR +
		XNVM_EFUSE_ISR_REG_OFFSET),
		(XNVM_EFUSE_ISR_PGM_DONE | XNVM_EFUSE_ISR_PGM_ERROR),
		(XNVM_EFUSE_ISR_PGM_DONE | XNVM_EFUSE_ISR_PGM_ERROR),
		XNVM_EFUSE_PGM_TIMEOUT_VAL,
		&EventMask);

	if (XST_TIMEOUT == Status) {
		Status = (int)XNVM_EFUSE_ERR_PGM_TIMEOUT;
	}
	else if ((EventMask & XNVM_EFUSE_ISR_PGM_ERROR) ==
			XNVM_EFUSE_ISR_PGM_ERROR) {
		Status = (int)XNVM_EFUSE_ERR_PGM;
	}
	else {
		Status = XST_SUCCESS;
	}

	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR, XNVM_EFUSE_ISR_REG_OFFSET,
			(XNVM_EFUSE_ISR_PGM_DONE | XNVM_EFUSE_ISR_PGM_ERROR));

	return Status;
}

/******************************************************************************/
/**
 * @brief	Reads one eFuse row from the controller.
 *
 * @param	Page - eFuse page select.
 * @param	Row - Fuse row within the page.
 * @param	RegData - Output buffer for the 32-bit fuse row image.
 *
 * @return
 *		- XST_SUCCESS on successful read.
 *		- XNVM_EFUSE_ERR_RD_TIMEOUT on controller timeout.
 *
 ******************************************************************************/
static int XNvm_EfuseReadRow(u32 Page, u32 Row, u32 *RegData)
{
	volatile int Status = XST_FAILURE;
	u32 RdAddr = XIL_UNSIGNED_ZERO;
	u32 EventMask = XIL_UNSIGNED_ZERO;

	RdAddr = (Page << XNVM_EFUSE_ADDR_PAGE_SHIFT) |
		(Row << XNVM_EFUSE_ADDR_ROW_SHIFT);

	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
		XNVM_EFUSE_RD_ADDR_REG_OFFSET, RdAddr);

	Status = (int)Xil_WaitForEvents((UINTPTR)(XNVM_EFUSE_CTRL_BASEADDR +
		XNVM_EFUSE_ISR_REG_OFFSET),
		XNVM_EFUSE_ISR_RD_DONE,
		XNVM_EFUSE_ISR_RD_DONE,
		XNVM_EFUSE_RD_TIMEOUT_VAL,
		&EventMask);
	if (XST_TIMEOUT == Status) {
		Status = (int)XNVM_EFUSE_ERR_RD_TIMEOUT;
	} else if ((EventMask & XNVM_EFUSE_ISR_RD_DONE) ==
			XNVM_EFUSE_ISR_RD_DONE) {
		*RegData = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_RD_DATA_REG_OFFSET);
		Status = XST_SUCCESS;
	} else {
		/** - Do Nothing */
	}

	/*
	 * RD_DONE is sticky and write-1-to-clear. Leaving it set would satisfy
	 * the wait in the next row read before that read completes, sampling
	 * RD_DATA while it still holds this row. Clear after wait and RD_DATA
	 * latch so wait and clear stay in the same function.
	 */
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
			XNVM_EFUSE_ISR_REG_OFFSET, XNVM_EFUSE_ISR_RD_DONE);

	return Status;
}

/******************************************************************************/
/**
 * @brief	Verifies that a fuse bit reads back as blown.
 *
 * @param	Page - eFuse page select.
 * @param	Row - Fuse row within the page.
 * @param	Col - Fuse column within the row.
 *
 * @return
 *		- XST_SUCCESS when the bit reads as set.
 *		- XNVM_EFUSE_ERR_PGM_VERIFY when the bit is not set.
 *		- Row read error on failure.
 *
 ******************************************************************************/
static int XNvm_EfuseVerifyBit(u32 Page, u32 Row, u32 Col)
{
	int Status = XST_FAILURE;
	volatile u32 RegData = XIL_UNSIGNED_ZERO;

	Status = XNvm_EfuseReadRow(Page, Row, (u32 *)(void *)&RegData);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((RegData & (XNVM_EFUSE_BIT0_MASK << Col)) != XIL_UNSIGNED_ZERO) {
		Status = XST_SUCCESS;
	}
	else {
		Status = (int)XNVM_EFUSE_ERR_PGM_VERIFY;
	}

END:
	return Status;
}
/** @} */
