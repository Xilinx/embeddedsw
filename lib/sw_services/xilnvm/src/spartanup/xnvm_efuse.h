/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
*
* @file versal/server/xnvm_efuse.h
* @addtogroup xnvm_versal_efuse_apis XilNvm spartan ultrascale plus eFuse APIs
* @{
*
* @cond xnvm_internal
* This file contains function declarations of eFuse APIs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- ---------- --------------------------------------------------------
* 1.0   kpt  07/30/2024 Initial release
*
* </pre>
*
* @note
*
* @endcond
*******************************************************************************/
#ifndef XNVM_EFUSE_H
#define XNVM_EFUSE_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************** Include Files *********************************/
#include "xil_io.h"
#include "xil_types.h"
#include "xstatus.h"
#include "xil_util.h"
#include "xnvm_efuse_hw.h"

/*************************** Constant Definitions *****************************/

/**
* @}
*/
#define XNVM_EFUSE_WORD_LEN					(4U)
#define XNVM_EFUSE_TOTAL_NUM_OF_ROWS		(64U)
#define XNVM_EFUSE_AES_KEY_SIZE_IN_WORDS	(8U)
#define XNVM_EFUSE_PPK_HASH_SIZE_IN_BYTES	(48U)
#define XNVM_EFUSE_PPK_HASH_SIZE_IN_WORDS	(12U)
#define XNVM_EFUSE_AES_IV_SIZE_IN_BYTES		(12U)
#define XNVM_EFUSE_AES_IV_SIZE_IN_WORDS		(3U)
#define XNVM_EFUSE_TOTAL_NUM_OF_PPKS		(3U)
#define XNVM_EFUSE_SEC_CTRL_ROW_0			(0U)
#define XNVM_EFUSE_SEC_CTRL_ROW_1			(1U)
#define XNVM_EFUSE_SEC_CTRL_ROW_2			(2U)
#define XNVM_EFUSE_PPK0_HASH_START_ROW		(0U)
#define XNVM_EFUSE_PPK1_HASH_START_ROW		(1U)
#define XNVM_EFUSE_PPK2_HASH_START_ROW		(4U)
#define XNVM_EFUSE_PPK0_START_COL			(16U)
#define XNVM_EFUSE_PPK0_END_COL				(23U)
#define XNVM_EFUSE_PPK1_START_COL			(0U)
#define XNVM_EFUSE_PPK1_END_COL				(7U)
#define XNVM_EFUSE_PPK2_START_COL			(24U)
#define XNVM_EFUSE_PPK2_END_COL				(31U)
#define XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS		(32U)
#define XNVM_EFUSE_SPK_REVOKE_ID_START_ROW	(48U)
#define XNVM_EFUSE_SPK_REVOKE_ID_END_ROW	(59U)
#define XNVM_EFUSE_SPK_REVOKE_ID_START_COL	(24U)
#define XNVM_EFUSE_SPK_REVOKE_ID_END_COL	(31U)
#define XNVM_EFUSE_SPK_REVOKE_ID_NUM_OF_ROWS (12U)
#define XNVM_EFUSE_AES_KEY_START_ROW		(32U)
#define XNVM_EFUSE_AES_KEY_START_COL		(16U)
#define XNVM_EFUSE_AES_KEY_END_COL			(23U)
#define XNVM_EFUSE_AES_KEY_NUM_OF_ROWS		(32U)
#define XNVM_EFUSE_AES_IV_START_ROW			(36U)
#define XNVM_EFUSE_AES_IV_END_ROW			(47U)
#define XNVM_EFUSE_AES_IV_START_COL			(24U)
#define XNVM_EFUSE_AES_IV_END_COL			(31U)
#define XNVM_EFUSE_AES_REVOKE_ID_START_ROW	(62U)
#define XNVM_EFUSE_AES_REVOKE_ID_END_ROW	(63U)
#define XNVM_EFUSE_AES_REVOKE_ID_START_COL	(8U)
#define XNVM_EFUSE_AES_REVOKE_ID_END_COL	(15U)
#define XNVM_EFUSE_AES_REVOKE_ID_NUM_OF_ROWS (2U)
#define XNVM_EFUSE_USER_FUSE_START_ROW		(60U)
#define XNVM_EFUSE_USER_FUSE_END_ROW		(63U)
#define XNVM_EFUSE_USER_FUSE_START_COL		(24U)
#define XNVM_EFUSE_USER_FUSE_END_COL		(31U)
#define XNVM_EFUSE_USER_FUSE_NUM_OF_ROWS	(4U)
#define XNVM_EFUSE_USER_FUSE_SIZE_IN_BYTES	(4U)
#define XNVM_EFUSE_DEC_ONLY_START_ROW		(48U)
#define XNVM_EFUSE_DEC_ONLY_START_COL		(8U)
#define XNVM_EFUSE_DEC_ONLY_END_COL			(15U)
#define XNVM_EFUSE_DEC_ONLY_NUM_OF_ROWS		(2U)
#define XNVM_EFUSE_DEF_PPK_HASH_SIZE_IN_BYTES (32U)
#define XNVM_EFUSE_DEF_PPK_HASH_SIZE_IN_WORDS (8U)
#define XNVM_EFUSE_NUM_OF_REVOKE_ID_FUSES  (3U)
#define XNVM_EFUSE_AES_IV_NUM_OF_ROWS		(12U)
#define XNVM_EFUSE_MAX_BITS_IN_ROW			(32U)
#define XNVM_EFUSE_SEC_CTRL_ROW_0			(0U)
#define XNVM_EFUSE_SEC_CTRL_ROW_1			(1U)
#define XNVM_EFUSE_SEC_CTRL_ROW_2			(2U)
#define XNVM_EFUSE_SEC_CTRL_ROW_3			(3U)
#define XNVM_EFUSE_SEC_CTRL_ROW_4			(18U)
#define XNVM_EFUSE_MAX_SPK_REVOKE_ID		(3U)
#define XNVM_EFUSE_DNA_SIZE_IN_WORDS		(3U)

#define XNVM_EFUSE_PROGRAM_VERIFY		(0U) /**< Verify eFuses after programming */

#define XNVM_EFUSE_WRITE_LOCKED		(1U) /* Efuse write locked */
#define XNVM_EFUSE_WRITE_UNLOCKED	(0U) /** Efuse write unlocked */
#define XNVM_EFUSE_CFG_ENABLE_PGM	(1U << XNVM_EFUSE_CFG_PGM_EN_SHIFT) /** Enable efuse programming */
#define XNVM_EFUSE_CFG_MARGIN_RD	(XNVM_EFUSE_CFG_MARGIN_2_RD << XNVM_EFUSE_CFG_MARGIN_RD_SHIFT)
									/** Enable efuse margin read */
#define XNVM_EFUSE_STATUS_TBIT_0	(1 << XNVM_EFUSE_STS_0_TBIT_SHIFT)
#define XNVM_EFUSE_STATUS_TBIT_1	(1 << XNVM_EFUSE_STS_2_TBIT_SHIFT)
#define XNVM_EFUSE_STATUS_TBIT_2	(1 << XNVM_EFUSE_STS_3_TBIT_SHIFT)

#define XNVM_EFUSE_ADDR_COLUMN_SHIFT			(0U)
#define XNVM_EFUSE_ADDR_ROW_SHIFT			(5U)
#define XNVM_EFUSE_ADDR_PAGE_SHIFT			(13U)
#define XNVM_EFUSE_ISR_PGM_DONE				(0x01U << 0U)
#define XNVM_EFUSE_ISR_PGM_ERROR			(0x01U << 1U)
#define XNVM_EFUSE_ISR_RD_DONE				(0x01U << 2U)
#define XNVM_EFUSE_ISR_CACHE_ERROR			(0x01U << 4U)
#define XNVM_EFUSE_STS_CACHE_DONE			(0x01U << 5U)
/**< Timeout in term of number of times status register polled to check eFUSE
 * programming operation complete
 */
#define XNVM_EFUSE_PGM_TIMEOUT_VAL			(100U)
#define XNVM_EFUSE_RD_TIMEOUT_VAL			(100U)

/**< Timeout in term of number of times status register polled to check eFuse
 * Cache load is done
 */
#define XNVM_EFUSE_CACHE_LOAD_TIMEOUT_VAL		(0x800U)

#define XNVM_EFUSE_CACHE_DEC_EFUSE_ONLY_MASK	(0x0000FFFFU)

#define XNVM_EFUSE_SEC_DEF_VAL_ALL_BIT_SET	(0xFFFFFFFFU)

#define XNVM_EFUSE_SEC_DEF_VAL_BYTE_SET	(0xFFU)

#define XNVM_EFUSE_PS_REF_CLK_FREQ	33330000U

/**< CRC for Aes zero key */
#define XNVM_EFUSE_CRC_AES_ZEROS		(0x6858A3D5U)

/***************************** Type Definitions *******************************/

typedef struct {
	u32 StartRow; /**< Start row number of eFuse */
	u32 ColStart; /**< Start column number of eFuse */
	u32 ColEnd; /**< End column number of eFuse */
	u32 NumOfRows; /**< Number of rows of eFuse  */
	u32 SkipVerify; /**< Flag to check if eFuse bit should be verified after programming */
}XNvm_EfusePrgmInfo;

/**
 * @{ eFuse control bits
 */
 /**< This structer defines Security control bits*/

typedef enum {
	XNVM_EFUSE_SVD_0,
	XNVM_EFUSE_SVD_1,
	XNVM_EFUSE_SVD_2,
	XNVM_EFUSE_RMA_DISABLE_0,
	XNVM_EFUSE_RMA_ENABLE_0,
	XNVM_EFUSE_SCAN_CLR_EN,
	XNVM_EFUSE_HASH_PUF_OR_KEY,
	XNVM_EFUSE_AXI_DIS,
	XNVM_EFUSE_MDM_DISABLE_0 = 24,
	XNVM_EFUSE_MDM_DISABLE_1,
	XNVM_EFUSE_ICAP_DIS,
	XNVM_EFUSE_MCAP_DIS,
	XNVM_EFUSE_TBIT_0,
	XNVM_EFUSE_TBIT_1,
	XNVM_EFUSE_TBIT_2,
	XNVM_EFUSE_TBIT_3
}XNvm_EfuseSecCtrlBitRow0;

typedef enum {
	XNVM_EFUSE_CRC_EN = 24,
	XNVM_EFUSE_DFT_DISABLE_0,
	XNVM_EFUSE_DFT_DISABLE_1,
	XNVM_EFUSE_LCKDOWN,
	XNVM_EFUSE_RMA_DISABLE_1,
	XNVM_EFUSE_RMA_ENABLE_1
}XNvm_EfuseSecCtrlBitRow1;

typedef enum {
	XNVM_EFUSE_PUF_TEST2_DIS = 24,
	XNVM_EFUSE_PPK0_INVLD_0,
	XNVM_EFUSE_PPK0_INVLD_1,
	XNVM_EFUSE_PPK1_INVLD_0,
	XNVM_EFUSE_PPK1_INVLD_1,
	XNVM_EFUSE_PPK2_INVLD_0,
	XNVM_EFUSE_PPK2_INVLD_1,
	XNVM_EFUSE_EXPORT_CONTROL
}XNvm_EfuseSecCtrlBitRow2;

typedef enum {
	XNVM_EFUSE_AES_RD_WR_LK_0 = 24,
	XNVM_EFUSE_AES_RD_WR_LK_1,
	XNVM_EFUSE_PPK0_WR_LK,
	XNVM_EFUSE_PPK1_WR_LK,
	XNVM_EFUSE_PPK2_WR_LK,
	XNVM_EFUSE_JTAG_DIS,
	XNVM_EFUSE_AES_DIS,
	XNVM_EFUSE_AES_CM_DIS,
}XNvm_EfuseSecCtrlBitRow3;

typedef enum {
	XNVM_EFUSE_DNA_WR_LK = 10,
	XNVM_EFUSE_MEM_CLR_EN = 12,
	XNVM_EFUSE_JTAG_ERR_OUT_DIS = 14,
	XNVM_EFUSE_USER_WR_LK,
}XNvm_EfuseSecCtrlBitRow4;

typedef struct {
	u8 PrgmScanClr;
	u8 PrgmHashPufOrKey;
	u8 PrgmAxiDis;
	u8 PrgmMdmDis;
	u8 PrgmIcapDis;
	u8 PrgmMcapDis;
	u8 PrgmRmaDis;
	u8 PrgmRmaEn;
	u8 PrgmCrcEn;
	u8 PrgmDftDis;
	u8 PrgmLckdwn;
	u8 PrgmPufTes2Dis;
	u8 PrgmPpk0Invld;
	u8 PrgmPpk1Invld;
	u8 PrgmPpk2Invld;
	u8 PrgmExportCtrl;
	u8 PrgmAesRdlk;
	u8 PrgmAesWrlk;
	u8 PrgmPpk0lck;
	u8 PrgmPpk1lck;
	u8 PrgmPpk2lck;
	u8 PrgmJtagDis;
	u8 PrgmAesDis;
	u8 PrgmAesCmDis;
	u8 PrgmUserWrlk;
	u8 PrgmMemClrEn;
	u8 PrgmDnaWrlk;
	u8 PrgmJtagErrDis;
} XNvm_EfuseSecCtrl;

typedef struct {
	u8 RMA_ENABLE_1;
	u8 RMA_DISABLE_1;
	u8 LCKDOWN;
	u8 DFT_DIS;
	u8 EFUSE_CRC_EN;
	u8 PPK0_INVLD1;
	u8 PPK1_INVLD1;
	u8 PPK2_INVLD1;
	u8 MDM_DISABLE_1;
	u8 USER_WR_LK;
	u8 JTAG_ERR_OUT_DIS;
	u8 OSC_TRIMMED;
	u8 MEM_CLEAR_EN;
	u8 SVD_WR_LK;
	u8 DNA_WR_LK;
	u8 SHA_DISABLE;
	u8 AXI_DISABLE;
	u8 MDM_DISABLE_0;
	u8 AES_RD_WR_LK_1;
	u8 AES_RD_WR_LK_0;
	u8 PPK0_INVLD0;
	u8 PPK0_WR_LK;
	u8 PPK1_INVLD0;
	u8 PPK1_WR_LK;
	u8 PPK2_INVLD0;
	u8 PPK2_WR_LK;
	u8 HASH_PUF_OR_KEY;
	u8 PUF_TEST2_DIS;
	u8 JTAG_DIS;
	u8 RMA_ENABLE_0;
	u8 RMA_DISABLE_0;
	u8 EXPORT_CONTROL;
	u8 ICAP_DIS;
	u8 MCAP_DIS;
	u8 AES_CM_DIS;
	u8 AES_DIS;
	u8 SCAN_CLEAR_EN;
} XNvm_EfuseSecCtrlBits;

typedef struct {
	u32 AesKey[XNVM_EFUSE_AES_KEY_SIZE_IN_WORDS];
	u8 PrgmAesKey;
} XNvm_EfuseAesKeys;

typedef struct {
	u32 AesIv[XNVM_EFUSE_AES_IV_SIZE_IN_WORDS];
	u8 PrgmIv;
} XNvm_EfuseAesIvs;

typedef struct {
	u32 ActaulPpkHashSize;
	u8 Ppk0Hash[XNVM_EFUSE_PPK_HASH_SIZE_IN_BYTES];
	u8 Ppk1Hash[XNVM_EFUSE_PPK_HASH_SIZE_IN_BYTES];
	u8 Ppk2Hash[XNVM_EFUSE_PPK_HASH_SIZE_IN_BYTES];
	u8 PrgmPpk0Hash;
	u8 PrgmPpk1Hash;
	u8 PrgmPpk2Hash;
} XNvm_EfusePpkHash;

typedef struct {
	u8 PrgmDeconly;
} XNvm_EfuseDecOnly;

typedef struct {
	u32 RevokeId[XNVM_EFUSE_NUM_OF_REVOKE_ID_FUSES];
	u8 PrgmSpkRevokeId;
} XNvm_EfuseSpkRevokeId;

typedef struct {
	u32 AesRevokeId;
	u8 PrgmAesRevokeId;
} XNvm_EfuseAesRevokeId;

typedef struct {
	u32 UserFuseVal;
	u8 PrgmUserEfuse;
} XNvm_EfuseUserFuse;

/**
 * @name  Operation mode
 */
typedef enum {
	XNVM_EFUSE_MODE_RD, /**< eFuse read mode */
	XNVM_EFUSE_MODE_PGM /**< eFuse program mode */
} XNvm_EfuseOpMode;
/** @} */

/**
 * @name  Read mode
 */
typedef enum {
	XNVM_EFUSE_NORMAL_RD, /**< eFuse normal read */
	XNVM_EFUSE_MARGIN_RD /**< eFuse margin read */
} XNvm_EfuseRdMode;
/** @} */

typedef enum {
	XNVM_EFUSE_PPK0,
	XNVM_EFUSE_PPK1,
	XNVM_EFUSE_PPK2
} XNvm_EfusePpkType;

typedef enum {
	XNVM_EFUSE_IV_RANGE,
	XNVM_EFUSE_BLACK_IV
} XNvm_EfuseIvType;

/**
 * @}
 * @endcond
 */

/**
 *  This structure defines sub structures of Versal eFuses to be blown
 */
typedef struct {
	XNvm_EfuseAesKeys *AesKeys; /**< Pointer to Aes keys*/
	XNvm_EfusePpkHash *PpkHash; /**< Pointer to ppk hash*/
	XNvm_EfuseAesIvs *Ivs; /**< Pointer to the IVs structure*/
	XNvm_EfuseDecOnly *DecOnly; /**< Pointer to the DecOnly structure*/
	XNvm_EfuseSecCtrl *SecCtrlBits; /**< Pointer to security control bits*/
	XNvm_EfuseSpkRevokeId *SpkRevokeId; /**< Pointer to the SPK revoke Id structure*/
	XNvm_EfuseAesRevokeId *AesRevokeId; /**< Pointer to the AES revoke id structure */
	XNvm_EfuseUserFuse *UserFuse; /** Pointer to the user efuse structure */
}XNvm_EfuseData;

typedef enum {
	XNVM_EFUSE_ERR_UNLOCK = 2U, /**< 0x2 - Error efuse unlock */
	XNVM_EFUSE_ERR_LOCK, /**< 0x3 - Error efuse locked */
	XNVM_EFUSE_ERR_PGM_TBIT_PATTERN, /**< 0x4 - Error program tbit pattern */
	XNVM_EFUSE_ERR_INVALID_PARAM, /**< 0x5 - Error invalid param */
	XNVM_EFUSE_ERR_CRC_VERIFICATION, /**< 0x6 - Error CRC verification */
	XNVM_EFUSE_ERR_CACHE_PARITY, /**< 0x7 - Error cache parity */
	XNVM_EFUSE_ERR_PGM_TIMEOUT, /**< 0x8 - Error program timeout */
	XNVM_EFUSE_ERR_PGM, /**< 0x9 - Error program */
	XNVM_EFUSE_ERR_RD_TIMEOUT, /**< 0xA - Error read timeout */
	XNVM_EFUSE_ERR_PGM_VERIFY, /**< 0xB - Error program verify */
	XNVM_EFUSE_ERR_CACHE_LOAD, /**< 0xC - Error cache reload */

	XNVM_EFUSE_ERR_AES_ALREADY_PRGMD = 0x10, /**< 0x10 - Error AES key already programmed */
	XNVM_EFUSE_ERR_PPK0_HASH_ALREADY_PRGMD = 0x20, /**< 0x20 - Error PPK0 hash already programmed */
	XNVM_EFUSE_ERR_PPK1_HASH_ALREADY_PRGMD = 0x30, /**< 0x30 - Error PPK1 hash already programmed */
	XNVM_EFUSE_ERR_PPK2_HASH_ALREADY_PRGMD = 0x40, /**< 0x40 - Error PPK2 hash already programmed */
	XNVM_EFUSE_ERR_DEC_ONLY_ALREADY_PRGMD = 0x50, /**< 0x50 - Error DEC only already programmed */
	XNVM_EFUSE_ERR_NTHG_TO_BE_PROGRAMMED = 0x60, /**< 0x60 - Error nothing to be programmed */
	XNVM_EFUSE_ERR_DEC_ONLY_KEY_MUST_BE_PRGMD = 0x70,  /**< 0x70 - Error dec only key must be programmed */
	XNVM_EFUSE_ERR_DEC_ONLY_IV_MUST_BE_PRGMD = 0x80,  /**< 0x80 - Error dec only iv must be programmed */
	XNVM_EFUSE_ERR_DEC_ONLY_HASH_OR_PUF_KEY_MUST_BE_PRGMD = 0x90, /**< 0x90 - Error hash or puf key must be programmed */
	XNVM_EFUSE_ERR_BIT_CANT_REVERT = 0xA0,  /**< 0xA0 - Error bit can't revert */
	XNVM_EFUSE_ERR_FUSE_PROTECTED = 0xF0, /**< 0xA0 - Error fuse protected */
	XNVM_EFUSE_ERR_WRITE_AES_KEY = 0x8000, /**< 0x8000 - Error write AES key */
	XNVM_EFUSE_ERR_WRITE_PPK0_HASH = 0x8100, /**< 0x8100 - Error write PPK0 hash */
	XNVM_EFUSE_ERR_WRITE_PPK1_HASH = 0x8200, /**< 0x8200 - Error write PPK1 hash */
	XNVM_EFUSE_ERR_WRITE_PPK2_HASH = 0x8300, /**< 0x8300 - Error write PPK2 hash */
	XNVM_EFUSE_ERR_WRITE_IV = 0x8400, /**< 0x8400 - Error write IV */
	XNVM_EFUSE_ERR_WRITE_USER_FUSE = 0x8500, /**< 0x8500 - Error write user fuse */
	XNVM_EFUSE_ERR_WRITE_SPK_REVOKE_ID = 0x8600, /**< 0x8600 - Error write spk revoke id */
	XNVM_EFUSE_ERR_WRITE_AES_REVOKE_ID = 0x8700, /**< 0x8700 - Error write aes revoke id */
	XNVM_EFUSE_ERR_WRITE_DEC_ONLY = 0x8800, /**< 0x8800 - Error write dec only */
	XNVM_EFUSE_ERR_WRITE_AES_CM_DIS = 0x8900, /**< 0x8900 - Error write AES CM disable */
	XNVM_EFUSE_ERR_WRITE_AES_DIS = 0x8A00, /**< 0x8A00 - Error write AES disable */
	XNVM_EFUSE_ERR_WRITE_JTAG_DIS = 0x8B00, /**< 0x8B00 - Error write jtag disable */
	XNVM_EFUSE_ERR_WRITE_PPK0_WR_LCK = 0x8C00, /**< 0x8C00 - Error write PPK0 write lock */
	XNVM_EFUSE_ERR_WRITE_PPK1_WR_LCK = 0x8D00, /**< 0x8D00 - Error write PPK1 write lock */
	XNVM_EFUSE_ERR_WRITE_PPK2_WR_LCK = 0x8E00, /**< 0x8E00 - Error write PPK2 write lock */
	XNVM_EFUSE_ERR_WRITE_AES_RD_WR_LCK0 = 0x8F00, /**< 0x8F00 - Error write AES read write lock0 */
	XNVM_EFUSE_ERR_WRITE_AES_RD_WR_LCK1 = 0x9000, /**< 0x9000 - Error write AES read write lock1 */
	XNVM_EFUSE_ERR_WRITE_EXPORT_CTRL = 0x9100, /**< 0x9100 - Error write export control */
	XNVM_EFUSE_ERR_WRITE_PPK0_INVLD_0 = 0x9200, /**< 0x9200 - Error write PPK0 invalid0 */
	XNVM_EFUSE_ERR_WRITE_PPK0_INVLD_1 = 0x9300, /**< 0x9300 - Error write PPK0 invalid1 */
	XNVM_EFUSE_ERR_WRITE_PPK1_INVLD_0 = 0x9400, /**< 0x9400 - Error write PPK1 invalid0 */
	XNVM_EFUSE_ERR_WRITE_PPK1_INVLD_1 = 0x9500, /**< 0x9500 - Error write PPK1 invalid1 */
	XNVM_EFUSE_ERR_WRITE_PPK2_INVLD_0 = 0x9600, /**< 0x9600 - Error write PPK2 invalid0 */
	XNVM_EFUSE_ERR_WRITE_PPK2_INVLD_1 = 0x9700, /**< 0x9700 - Error write PPK2 invalid1 */
	XNVM_EFUSE_ERR_WRITE_PUF_TEST2_DIS = 0x9800, /**< 0x9800 - Error write PPK2 invalid1 */
	XNVM_EFUSE_ERR_WRITE_RMA_ENABLE_0 = 0x9900, /**< 0x9900 - Error write rma enable0 */
	XNVM_EFUSE_ERR_WRITE_RMA_ENABLE_1 = 0x9A00, /**< 0x9A00 - Error write rma enable1 */
	XNVM_EFUSE_ERR_WRITE_DFT_DIS_0 = 0x9B00, /**< 0x9B00 - Error write DFT disable10 */
	XNVM_EFUSE_ERR_WRITE_DFT_DIS_1 = 0x9C00, /**< 0x9C00 - Error write DFT disable1 */
	XNVM_EFUSE_ERR_WRITE_CRC_EN = 0x9D00, /**< 0x9D00 - Error write CRC enable */
	XNVM_EFUSE_ERR_WRITE_MCAP_DIS = 0x9E00, /**< 0x9E00 - Error write MACP disable */
	XNVM_EFUSE_ERR_WRITE_ICAP_DIS = 0x9F00, /**< 0x9F00 - Error write ICAP disable */
	XNVM_EFUSE_ERR_WRITE_MDM_DIS_0 = 0xA000, /**< 0xA000 - Error write MDM disable 0 */
	XNVM_EFUSE_ERR_WRITE_MDM_DIS_1 = 0xA100, /**< 0xA100 - Error write MDM disable 1 */
	XNVM_EFUSE_ERR_WRITE_AXI_DIS = 0xA200, /**< 0xA200 - Error write AXI disable */
	XNVM_EFUSE_ERR_WRITE_HASH_PUF_OR_KEY = 0xA300, /**< 0xA300 - Error write hash puf or key */
	XNVM_EFUSE_ERR_WRITE_SCAN_CLR = 0xA400, /**< 0xA400 - Error write scan clear */
	XNVM_EFUSE_ERR_WRITE_USER_WR_LK = 0xA500, /**< 0xA500 - Error write user write lock */
	XNVM_EFUSE_ERR_WRITE_MEM_CLR_EN = 0xA600, /**< 0xA600 - Error write mem clean enable */
	XNVM_EFUSE_ERR_WRITE_JTAG_ERR_OUT_DIS = 0xA700, /**< 0xA700 - Error write jtag error out disable */
	XNVM_EFUSE_ERR_WRITE_DNA_WR_LK = 0xA800, /**< 0xA800 - Error write DNA write lock */
	XNVM_EFUSE_ERR_GLITCH_DETECTED = 0xA900, /**< 0xA900 - Error glitch detected */

	XNVM_EFUSE_ERR_RD_SEC_CTRL_BITS = 0xC000, /**< 0xC000 - Error read secure control bits */
	XNVM_EFUSE_ERR_BEFORE_PROGRAMMING = 0x80000, /**< 0x80000 - Error before programming */
} XNvm_EfuseErr;

/*************************** Function Prototypes ******************************/
int XNvm_EfuseWrite(XNvm_EfuseData *EfuseData);
int XNvm_EfuseReadSecCtrlBits(XNvm_EfuseSecCtrlBits *SecCtrlBits);
int XNvm_EfuseReadPpkHash(XNvm_EfusePpkType PpkType, u32 *PpkData, u32 PpkSize);
int XNvm_EfuseReadSpkRevokeId(u32 *SpkRevokeData, u32 SpkRevokeRow);
int XNvm_EfuseReadAesRevokeId(u32 *AesRevokeData);
int XNvm_EfuseReadUserFuse(u32 *UserFuseData);
int XNvm_EfuseReadIv(XNvm_EfuseIvType IvType, u32 *IvData);
int XNvm_EfuseReadDna(u32 *Dna);
int XNvm_EfuseReadDecOnly(u32 *DecOnly);
int XNvm_EfuseCheckAesKeyCrc(u32 CrcRegOffSet, u32 CrcDoneMask, u32 CrcPassMask, u32 Crc);

#ifdef __cplusplus
}
#endif

#endif	/* XNVM_EFUSE_H */

/* @} */
