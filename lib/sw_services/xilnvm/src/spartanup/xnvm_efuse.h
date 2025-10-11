/******************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
*
* @file spartanup/xnvm_efuse.h
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
* 3.5   hj   04/02/2025 Remove unused PrgmAesWrlk variable
*       hj   04/10/2025 Rename PPK hash size macros
*       hj   04/10/2025 Remove security control bits not exposed to user
* 3.6   hj   04/10/2025 Remove zero IV validation check in dec_only case
*       hj   05/27/2025 Support XILINX_CTRL efuse PUFHD_INVLD and DIS_SJTAG bit programming
*       mb   07/02/2025 Update doxygen comments for structures definitions
*       aa   07/24/2025 Remove unused macros
*       mb   20/08/2025 Add EfuseCClkFreq and EfuseClkSrc to XNvm_EfuseData structure
*       mb   08/24/2025 Add support to program boot mode disable efuses
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
#include "xilnvm_bsp_config.h"

/*************************** Constant Definitions *****************************/

/**
* @}
*/
#define XNVM_EFUSE_WORD_LEN			(4U) /**< Word length */
#define XNVM_EFUSE_AES_KEY_SIZE_IN_WORDS	(8U) /**< AES key size in words */
#define XNVM_EFUSE_PPK_HASH_384_SIZE_IN_BYTES	(48U) /**< PPK hash size in bytes for all other
							parts except SU10P,SU25P,SU35P */
#define XNVM_EFUSE_AES_IV_SIZE_IN_BYTES		(12U) /**< AES IV size in bytes */
#define XNVM_EFUSE_AES_IV_SIZE_IN_WORDS		(3U) /**< AES IV size in words */
#define XNVM_EFUSE_PPK0_HASH_START_ROW		(0U) /**< PPK0 hash start row */
#define XNVM_EFUSE_PPK1_HASH_START_ROW		(1U) /**< PPK1 hash start row */
#define XNVM_EFUSE_PPK2_HASH_START_ROW		(4U) /**< PPK2 hash start row */
#define XNVM_EFUSE_PPK0_START_COL		(16U) /**< PPK0 hash start column */
#define XNVM_EFUSE_PPK0_END_COL			(23U) /**< PPK0 hash end column */
#define XNVM_EFUSE_PPK1_START_COL		(0U)  /**< PPK1 hash start column */
#define XNVM_EFUSE_PPK1_END_COL			(7U)  /**< PPK1 hash end column */
#define XNVM_EFUSE_PPK2_START_COL		(24U) /**< PPK2 hash start column */
#define XNVM_EFUSE_PPK2_END_COL			(31U) /**< PPK2 hash end column */
#define XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS		(32U) /**< PPK hash number of rows */
#define XNVM_EFUSE_SPK_REVOKE_ID_START_ROW	(48U) /**< SPK revoke ID start row */
#define XNVM_EFUSE_SPK_REVOKE_ID_START_COL	(24U) /**< SPK revoke ID start column */
#define XNVM_EFUSE_SPK_REVOKE_ID_END_COL	(31U) /**< SPK revoke ID end column */
#define XNVM_EFUSE_SPK_REVOKE_ID_NUM_OF_ROWS	(12U) /**< SPK revoke ID number of rows */
#define XNVM_EFUSE_AES_KEY_START_ROW		(32U) /**< AES key start row */
#define XNVM_EFUSE_AES_KEY_START_COL		(16U) /**< AES key start column */
#define XNVM_EFUSE_AES_KEY_END_COL		(23U) /**< AES key end column */
#define XNVM_EFUSE_AES_KEY_NUM_OF_ROWS		(32U) /**< AES key number of rows */
#define XNVM_EFUSE_AES_IV_START_ROW		(36U) /**< AES IV start row */
#define XNVM_EFUSE_AES_IV_START_COL		(24U) /**< AES IV start column */
#define XNVM_EFUSE_AES_IV_END_COL		(31U)  /**< AES IV end column */
#define XNVM_EFUSE_AES_REVOKE_ID_START_ROW	(62U) /**< AES revoke ID start row */
#define XNVM_EFUSE_AES_REVOKE_ID_START_COL	(8U) /**< AES revoke ID start column */
#define XNVM_EFUSE_AES_REVOKE_ID_END_COL	(15U) /**< AES revoke ID end column */
#define XNVM_EFUSE_AES_REVOKE_ID_NUM_OF_ROWS 	(2U) /**< AES revoke ID number of rows */
#define XNVM_EFUSE_USER_FUSE_START_ROW		(60U) /**< User efuse start row */
#define XNVM_EFUSE_USER_FUSE_START_COL		(24U) /**< User efuse start column */
#define XNVM_EFUSE_USER_FUSE_END_COL		(31U) /**< User efuse end column */
#define XNVM_EFUSE_USER_FUSE_NUM_OF_ROWS	(4U) /**< User efuse number of rows */
#define XNVM_EFUSE_DEC_ONLY_START_ROW		(48U) /**< Decrypt only start row */
#define XNVM_EFUSE_DEC_ONLY_START_COL		(8U) /**< Decrypt only start column */
#define XNVM_EFUSE_DEC_ONLY_END_COL		(15U) /**< Decrypt only end column */
#define XNVM_EFUSE_DEC_ONLY_NUM_OF_ROWS		(2U) /**< Decrypt only number of rows */

#define XNVM_EFUSE_PUF_HD_INVLD_START_ROW	(50U) /**< PUF_HD_INVLD start row */
#define XNVM_EFUSE_PUF_HD_INVLD_START_COL	(13U) /**< DPUF_HD_INVLD start column */
#define XNVM_EFUSE_PUF_HD_INVLD_END_COL		(14U) /**< PUF_HD_INVLD end column */
#define XNVM_EFUSE_PUF_HD_INVLD_NUM_OF_ROWS	(1U) /**< PUF_HD_INVLD number of rows */
#define XNVM_EFUSE_DIS_SJTAG_ROW		(50) /**< DIS_SJTAG start row */
#define XNVM_EFUSE_DIS_SJTAG_COL		(12) /**< DIS_SJTAG column */

#define XNVM_EFUSE_PPK_HASH_256_SIZE_IN_BYTES	(32U) /**< PPK hash size in bytes for parts-SU10P,SU25P,SU35P*/
#define XNVM_EFUSE_DEF_PPK_HASH_SIZE_IN_WORDS	(8U) /**< Default PPK hash size in words */
#define XNVM_EFUSE_NUM_OF_REVOKE_ID_FUSES	(3U) /**< Number of revoke id efuses */
#define XNVM_EFUSE_AES_IV_NUM_OF_ROWS		(12U) /**< AES IV number of rows */
#define XNVM_EFUSE_MAX_BITS_IN_ROW		(32U) /**< Maximum bits in a row */
#define XNVM_EFUSE_MAX_SPK_REVOKE_ID		(3U)  /**< Maximum SPK revoke id */
#define XNVM_EFUSE_DNA_SIZE_IN_WORDS		(3U) /**< DNA size in words */

#define XNVM_EFUSE_PROGRAM_VERIFY		(0U) /**< Verify eFuses after programming */

#define XNVM_EFUSE_WRITE_LOCKED			(1U) /**< Efuse write locked */
#define XNVM_EFUSE_WRITE_UNLOCKED		(0U) /**< Efuse write unlocked */
#define XNVM_EFUSE_CFG_ENABLE_PGM		(1U << XNVM_EFUSE_CFG_PGM_EN_SHIFT)
		/**< Enable efuse programming */
#define XNVM_EFUSE_CFG_MARGIN_RD		(XNVM_EFUSE_CFG_MARGIN_2_RD << XNVM_EFUSE_CFG_MARGIN_RD_SHIFT)
		/**< Enable efuse margin read */
#define XNVM_EFUSE_STATUS_TBIT_0		(1U << XNVM_EFUSE_STS_0_TBIT_SHIFT) /**< Tbit 0 value */

#define XNVM_EFUSE_ADDR_COLUMN_SHIFT		(0U) /**< Column shift */
#define XNVM_EFUSE_ADDR_ROW_SHIFT		(5U) /**< Row shift */
#define XNVM_EFUSE_ISR_PGM_DONE			(0x01U << 0U) /**< Program done value */
#define XNVM_EFUSE_ISR_PGM_ERROR		(0x01U << 1U) /**< Program error value */
#define XNVM_EFUSE_ISR_RD_DONE			(0x01U << 2U) /**< Read done value */
#define XNVM_EFUSE_ISR_CACHE_ERROR		(0x01U << 4U) /**< Cache error value */
#define XNVM_EFUSE_STS_CACHE_DONE		(0x01U << 5U) /**< Cache done value */

/**< Timeout in term of number of times status register polled to check eFUSE
 * programming operation complete
 */
#define XNVM_EFUSE_PGM_TIMEOUT_VAL		(100U) /**< Program timeout value */
#define XNVM_EFUSE_RD_TIMEOUT_VAL		(100U) /**< Read timeout value */

/**< Timeout in term of number of times status register polled to check eFuse
 * Cache load is done
 */
#define XNVM_EFUSE_CACHE_LOAD_TIMEOUT_VAL	(0x800U) /**< Cache load timeout value */

#define XNVM_EFUSE_CACHE_DEC_EFUSE_ONLY_MASK	(0x0000FFFFU) /**< Decrypt only efuse mask */

#define XNVM_EFUSE_SEC_DEF_VAL_ALL_BIT_SET	(0xFFFFFFFFU) /**< All bit set mask */

#define XNVM_EFUSE_SEC_DEF_VAL_BYTE_SET		(0xFFU) /**< Byte mask */

#define XNVM_EFUSE_CRC_AES_ZEROS		(0x6858A3D5U) /**< CRC for Aes zero key */
#define XNVM_EFUSE_PUFHD_INVLD_EFUSE_BITS	(0x02U) /*Puf Invalid Bits*/
#define XNVM_EFUSE_PUFHD_INVLD_EFUSE_SHIFT	(0x13U) /*Puf Invalid shift*/
#define XNVM_EFUSE_PUFHD_INVLD_EFUSE_MASK	(0x00006000U) /*Puf Invalid mask*/
#define XNVM_EFUSE_PUFHD_INVLD_EFUSE_VAL	(0x03U) /*Puf Invalid */

#define XNVM_EFUSE_DISSJTAG_EFUSE_BITS		(0x01U) /*Disable Secure JTAG shift*/
#define XNVM_EFUSE_DISSJTAG_EFUSE_SHIFT		(0x12U) /*Disable Secure JTAG shift*/
#define XNVM_EFUSE_DISSJTAG_EFUSE_MASK		(0x00001000U) /*Disable Secure JTAG shift*/

/**< Efuse clock programming related macros */
#ifndef XNVM_SET_EFUSE_CLK_FREQUENCY_FROM_RTCA
#define XNVM_SET_EFUSE_CLOCK_FREQUENCY_SRC_FROM_USER	/**< Set efuse clock frequency from user */
#endif

/***************************** Type Definitions *******************************/

typedef struct {
	u32 StartRow; /**< Start row number of eFuse */
	u32 ColStart; /**< Start column number of eFuse */
	u32 ColEnd; /**< End column number of eFuse */
	u32 NumOfRows; /**< Number of rows of eFuse  */
	u32 SkipVerify; /**< Flag to check if eFuse bit should be verified after programming */
} XNvm_EfusePrgmInfo;

/**
 * @{ eFuse control bits
 */
/**< This structer defines Security control bits*/

typedef struct {
	u8 HashPufOrKey; /**< Flag to read or program hash of PUF syndrome data or PPK */
	u8 RmaDis; /**< Flag to read or program RMA disable */
	u8 RmaEn; /**< Flag to read or program RMA enable */
	u8 CrcEn; /**< Flag to read or program CRC enable */
	u8 DftDis; /**< Flag to read or program DFT disable */
	u8 Lckdwn; /**< Flag to read or program lockdown enable */
	u8 PufTes2Dis; /**< Flag to read or program PUF test2 disable */
	u8 Ppk0Invld; /**< Flag to read or program PPK0 invalid */
	u8 Ppk1Invld; /**< Flag to read or program PPK1 invalid */
	u8 Ppk2Invld; /**< Flag to read or program PPK2 invalid */
	u8 AesRdlk; /**< Flag to read or program AES read lock */
	u8 Ppk0lck; /**< Flag to read or program PPK0 read/write lock */
	u8 Ppk1lck; /**< Flag to read or program PPK1 read/write lock */
	u8 Ppk2lck; /**< Flag to read or program PPK2 read/write lock */
	u8 JtagDis; /**< Flag to read or program JTAG disable */
	u8 AesDis; /**< Flag to read or program AES disable */
	u8 UserWrlk; /**< Flag to read or program user efuse write lock */
	u8 JtagErrDis; /**< Flag to read or program JTAG error out disable */
	u8 CrcRmaDis; /**< Flag to read or program RMA disable using CRC */
	u8 CrcRmaEn; /**< Flag to read or program RMA enable using CRC */
} XNvm_EfuseSecCtrlBits;

typedef struct {
	u32 AesKey[XNVM_EFUSE_AES_KEY_SIZE_IN_WORDS]; /**< AES key value to be programmed */
	u8 PrgmAesKey; /**< Flag to determine whether to program AES key or not */
} XNvm_EfuseAesKeys;

typedef struct {
	u32 AesIv[XNVM_EFUSE_AES_IV_SIZE_IN_WORDS]; /**< AES IV value to be programmed */
	u8 PrgmIv; /**< Flag to determine whether to program IV or not */
} XNvm_EfuseAesIvs;

typedef struct {
	u32 ActaulPpkHashSize; /**< PPK hash size to be programmed it can be either 256/384 bit */
	u8 Ppk0Hash[XNVM_EFUSE_PPK_HASH_384_SIZE_IN_BYTES]; /**< PPK0 hash value to be programmed */
	u8 Ppk1Hash[XNVM_EFUSE_PPK_HASH_384_SIZE_IN_BYTES]; /**< PPK1 hash value to be programmed */
	u8 Ppk2Hash[XNVM_EFUSE_PPK_HASH_384_SIZE_IN_BYTES]; /**< PPK2 hash value to be programmed */
	u8 PrgmPpk0Hash; /**< Flag to determine whether to program PPK0 hash or not */
	u8 PrgmPpk1Hash; /**< Flag to determine whether to program PPK1 hash or not */
	u8 PrgmPpk2Hash; /**< Flag to determine whether to program PPK2 hash or not */
} XNvm_EfusePpkHash;

typedef struct {
	u8 PrgmDeconly; /**< Flag to determine whether to program Decrypt only or not */
} XNvm_EfuseDecOnly;

typedef struct {
	u32 RevokeId[XNVM_EFUSE_NUM_OF_REVOKE_ID_FUSES]; /**< Revoke ID value */
	u8 PrgmSpkRevokeId; /**< Flag to determine whether to program SPK revoke ID or not */
} XNvm_EfuseSpkRevokeId;

typedef struct {
	u32 AesRevokeId; /**< AES revoke ID */
	u8 PrgmAesRevokeId; /**< Flag to determine whether to program AES revoke ID or not */
} XNvm_EfuseAesRevokeId;

typedef struct {
	u32 PrgmPufHDInvld;  /**< Program PUFHD_INVLD */
	u32 PrgmDisSJtag;    /**< Program Disable Secure JTAG */
} XNvm_EfuseXilinxCtrl;


typedef struct {
	u32 UserFuseVal; /**< User efuse value */
	u8 PrgmUserEfuse; /**< Program user efuse */
} XNvm_EfuseUserFuse;

#ifdef SPARTANUPLUSAES1
typedef struct {
	u32 PrgmQspi24ModDis; /* Flag to program QSPI24 bootmode disable efuse bit */
	u32 PrgmQspi32ModDis; /* Flag to program QSPI32 bootmode disable efuse bit */
	u32 PrgmOspiModDis;   /* Flag to program OSPI bootmode disable efuse bit */
	u32 PrgmSmapModDis;   /* Flag to program SMAP bootmode disable efuse bit */
	u32 PrgmSerialModDis; /* Flag to program SERIAL bootmode disable efuse bit */
} XNvm_EfuseBootModeDis;
#endif

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
	XNVM_EFUSE_PPK0, /**< PPK 0 */
	XNVM_EFUSE_PPK1, /**< PPK 1 */
	XNVM_EFUSE_PPK2 /**< PPK 2 */
} XNvm_EfusePpkType;

typedef enum {
	XNVM_EFUSE_IV_RANGE, /**< IV range */
	XNVM_EFUSE_BLACK_IV /**< Black IV */
} XNvm_EfuseIvType;

/**
 * @}
 * @endcond
 */

/**
 *  This structure defines sub structures of Spartan Ultrascale+ eFuses to be blown
 */
typedef struct {
	XNvm_EfuseAesKeys *AesKeys; /**< Pointer to Aes keys*/
	XNvm_EfusePpkHash *PpkHash; /**< Pointer to ppk hash*/
	XNvm_EfuseAesIvs *Ivs; /**< Pointer to the IVs structure*/
	XNvm_EfuseDecOnly *DecOnly; /**< Pointer to the DecOnly structure*/
	XNvm_EfuseSecCtrlBits *SecCtrlBits; /**< Pointer to security control bits*/
	XNvm_EfuseSpkRevokeId *SpkRevokeId; /**< Pointer to the SPK revoke ID structure*/
	XNvm_EfuseAesRevokeId *AesRevokeId; /**< Pointer to the AES revoke ID structure */
	XNvm_EfuseUserFuse *UserFuse; /**< Pointer to the user efuse structure */
	XNvm_EfuseXilinxCtrl *XilinxCtrl; /**< Pointer to the Xilinx Ctrl efuse structure */
#ifdef SPARTANUPLUSAES1
	XNvm_EfuseBootModeDis *BootModeDis; /**< Pointer to the boot mode disable structure */
#endif
	u32 EfuseClkFreq; /**< eFuse clock frequency */
	u32 EfuseClkSrc; /**< eFuse clock source */
} XNvm_EfuseData;

enum {
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
	XNVM_EFUSE_ERR_RSVD_1 = 0x80,  /**< 0x80 - RSVD error */
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
	XNVM_EFUSE_ERR_WRITE_PUFHD_INVLD = 0xAA00, /**< 0xAA00 - Error write PUF_HD_INVLD detected */
	XNVM_EFUSE_ERR_WRITE_DIS_SJTAG = 0xAB00, /**< 0xAB00 - Error write DIS_SJTAG detected */
	XNVM_EFUSE_ERR_RD_SEC_CTRL_BITS = 0xC000, /**< 0xC000 - Error read secure control bits */
	XNVM_EFUSE_ERR_INVALID_CLK_FREQUENCY = 0xD000, /**< 0xD000 - Error Invalid Clock Frequency */
	XNVM_EFUSE_ERR_BEFORE_PROGRAMMING = 0x80000, /**< 0x80000 - Error before programming */
	XNVM_EFUSE_ERR_RD_CACHE_BOOT_MODE_DIS_BITS = 0xB000, /**< 0xB000 - Error read Boot mode disable bits from cache */
	XNVM_EFUSE_ERR_WRITE_QSPI24_BOOT_MODE_DIS = 0xB100, /**< 0xB100 - Error write QSPI24 Boot mode disable */
	XNVM_EFUSE_ERR_WRITE_QSPI32_BOOT_MODE_DIS = 0xB200, /**< 0xB200 - Error write QSPI32 Boot mode disable */
	XNVM_EFUSE_ERR_WRITE_OSPI_BOOT_MODE_DIS = 0xB300, /**< 0xB300 - Error write OSPI Boot mode disable */
	XNVM_EFUSE_ERR_WRITE_SMAP_BOOT_MODE_DIS = 0xB400, /**< 0xB400 - Error write SMAP Boot mode disable */
	XNVM_EFUSE_ERR_WRITE_SERIAL_BOOT_MODE_DIS = 0xB500, /**< 0xB500 - Error write SERIAL Boot mode disable */
	XNVM_EFUSE_ERR_WRITE_RMA_DISABLE_0 = 0xB600, /**< 0xB600 - Error write rma disable0 */
	XNVM_EFUSE_ERR_WRITE_RMA_DISABLE_1 = 0xB700, /**< 0xB700 - Error write rma disable1 */
	XNVM_EFUSE_ERR_WRITE_LCK_DWN = 0xB800, /**< 0xB800 - Error write lock down */
};

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
int XNvm_EfuseReadXilinxCtrl(XNvm_EfuseXilinxCtrl *XilinxCtrl);
#ifdef SPARTANUPLUSAES1
int XNvm_EfuseReadBootModeDisBits(XNvm_EfuseBootModeDis *BootModeDisBits);
#endif

#ifdef __cplusplus
}
#endif

#endif	/* XNVM_EFUSE_H */

/* @} */
