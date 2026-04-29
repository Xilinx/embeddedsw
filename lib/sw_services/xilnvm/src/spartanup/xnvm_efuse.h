/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
*
* @file spartanup/xnvm_efuse.h
* @addtogroup xnvm_spartan_ultrascaleplus_efuse_apis XilNvm Spartan Ultrascale Plus eFuse APIs
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
*       mb   10/04/2025 PPKs updates for SPARTANUPLUSAES1
*       mb   10/14/2025 Update logic for programming Revoke ID's
*       mb   11/11/2025 Add support for JTAG Boot mode disable efuse programming
* 3.7   mb   02/09/2026 Rename secure control bit names for SPARTANUPLUSAES1
*       mb   03/18/2026 Add support for temperature and voltage checks before efuse programming
* 3.7   hae  02/27/2026 Support XILINX_CTRL OSPI_RESET_RECOVERY_DELAY_CTRL
*                       and ROM_RSVD_OSPI_DEV_RESET_CHOICE
*                       and ROM_OSPI_CMD_SEQ_CTRL eFuse bit programming
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
#ifdef XNVM_ENABLE_ENV_MONITOR_CHECKS
#include "xsysmon.h"
#endif

/*************************** Constant Definitions *****************************/

#define XNVM_EFUSE_WORD_LEN			(4U) /**< Word length */
#define XNVM_EFUSE_BITS_IN_A_BYTE		(8U) /**< Number of bits in a byte */
#define XNVM_EFUSE_AES_KEY_SIZE_IN_WORDS	(8U) /**< AES key size in words */
#define XNVM_EFUSE_PPK_HASH_256_SIZE_IN_BYTES	(32U) /**< PPK hash size in bytes for parts-SU10P,SU25P,SU35P*/
#define XNVM_EFUSE_PPK_HASH_384_SIZE_IN_BYTES	(48U) /**< PPK hash size in bytes for all other
							* parts except SU10P,SU25P,SU35P */
#define XNVM_EFUSE_AES_IV_SIZE_IN_BYTES		(12U) /**< AES IV size in bytes */
#define XNVM_EFUSE_AES_IV_SIZE_IN_WORDS		(XNVM_EFUSE_AES_IV_SIZE_IN_BYTES \
						/ XNVM_EFUSE_WORD_LEN) /**< AES IV size in words */
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
#define XNVM_EFUSE_XILINX_CTRL_START_ROW	(50U) /**< Xilinx control bits start row */

/**
 * @name PPK Efuse programming related macros
 * @{
 */
#ifndef SPARTANUPLUSAES1
#define XNVM_EFUSE_PPK_HASH_LEN_IN_BITS		(256U) /**< PPK hash length in bits */
#define XNVM_EFUSE_DEF_PPK_HASH_SIZE_IN_WORDS	(8U) /**< Default PPK hash size in words */
#define XNVM_EFUSE_PPK_HASH_SIZE_IN_BYTES	XNVM_EFUSE_PPK_HASH_256_SIZE_IN_BYTES
						/**< PPK hash size in bytes */
#define XNVM_EFUSE_PPK_END			XNVM_EFUSE_PPK2 /**< Last PPK */
#else
#define XNVM_EFUSE_PPK_HASH_LEN_IN_BITS		(384U) /**< PPK hash length in bits */
#define XNVM_EFUSE_DEF_PPK_HASH_SIZE_IN_WORDS	(12U) /**< Default PPK hash size in words */
#define XNVM_EFUSE_PPK_HASH_SIZE_IN_BYTES	XNVM_EFUSE_PPK_HASH_384_SIZE_IN_BYTES
						/**< PPK hash size in bytes */
#define XNVM_EFUSE_PPK_END			XNVM_EFUSE_PPK1 /**< Last PPK */
#endif
/** @} */

#define XNVM_EFUSE_AES_IV_NUM_OF_ROWS		(12U) /**< AES IV number of rows */
#define XNVM_EFUSE_MAX_BITS_IN_ROW		(32U) /**< Maximum bits in a row */
#define XNVM_EFUSE_MAX_SPK_REVOKE_ID		(3U)  /**< Maximum SPK revoke id */
#define XNVM_EFUSE_DNA_SIZE_IN_WORDS		(3U) /**< DNA size in words */
#define XNVM_MAX_REVOKE_ID_FUSES		(XNVM_EFUSE_MAX_SPK_REVOKE_ID	\
						* XNVM_EFUSE_MAX_BITS_IN_ROW)	/**< Maximum revocation ID eFuses */

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

/**
 * @name Timeout in term of number of times status register polled to check eFuse programming operation complete
 * @{
 */
#define XNVM_EFUSE_PGM_TIMEOUT_VAL		(100U) /**< Program timeout value */
#define XNVM_EFUSE_RD_TIMEOUT_VAL		(100U) /**< Read timeout value */
/** @} */

/**
 * @name Timeout in term of number of times status register polled to check eFuse Cache load is done
 * @{
 */
#define XNVM_EFUSE_CACHE_LOAD_TIMEOUT_VAL	(0x800U) /**< Cache load timeout value */
/** @} */

#define XNVM_EFUSE_CACHE_DEC_EFUSE_ONLY_MASK	(0x0000FFFFU) /**< Decrypt only efuse mask */

#define XNVM_EFUSE_SEC_DEF_VAL_ALL_BIT_SET	(0xFFFFFFFFU) /**< All bit set mask */

#define XNVM_EFUSE_SEC_DEF_VAL_BYTE_SET		(0xFFU) /**< Byte mask */

#define XNVM_EFUSE_CRC_AES_ZEROS		(0x6858A3D5U) /**< CRC for Aes zero key */

#define XNVM_EFUSE_PUFHD_INVLD_EFUSE_VAL	(0x03U) /**< Puf Invalid */

#ifndef XNVM_SET_EFUSE_CLK_FREQUENCY_FROM_RTCA
#define XNVM_SET_EFUSE_CLOCK_FREQUENCY_SRC_FROM_USER	/**< Set efuse clock frequency from user */
#endif

/***************************** Type Definitions *******************************/

/**
 * eFuse programming information
 */
typedef struct {
	u32 StartRow; /**< Start row number of eFuse */
	u32 ColStart; /**< Start column number of eFuse */
	u32 ColEnd; /**< End column number of eFuse */
	u32 NumOfRows; /**< Number of rows of eFuse  */
	u32 SkipVerify; /**< Flag to check if eFuse bit should be verified after programming */
} XNvm_EfusePrgmInfo;

/**
 * eFuse IV type
 */
typedef enum {
	XNVM_EFUSE_AES_IV_RANGE, /**< IV range */
	XNVM_EFUSE_BLACK_IV /**< Black IV */
} XNvm_EfuseIvType;

/**
 * eFuse security control bits
 */
typedef struct {
	u32 HashPufOrKey; /**< 0: PPK1/2 registers refer to PPK1/2 HASH; 1: PPK1/2 registers refer to PUF_SYN HASH */
	u32 RmaDis; /**< RMA Disable, 0: Default; 1: Disables RMA when set */
	u32 RmaEn; /**< RMA Enable, 0: Default; 1: Enables RMA when set */
	u32 DftDis; /**< DFT Disable, 0: Default; 1: Disables DFT when set */
	u32 Lckdwn; /**< Lockdown, 0: Default; 1: Enables lockdown when set */
	u32 PufTes2Dis; /**< PUF TestMode2 Disable, 0: Default; 1: Disable the PUF TestMode2 permanently */
	u32 AesRdlk; /**< AES Read/Write Lock, 0: Default; 1: Locks writing to AES KEY and disables CRC checks on AES boot key */
	u32 JtagDis; /**< JTAG Disable, 0: Default; 1: Disable all JTAG instructions when set */
	u32 AesDis; /**< AES Disable, 0: Default; 1: Disable the AES engine */
	u32 ScanClearEn; /**< Scan clear enable, 0: Default; 1: Enables scan clear when set */
	u32 Ppk0lck; /**< PPK0 Lock, 0: Default (No protection from writes); 1: Locks writing to PPK0 eFuses */
	u32 Ppk0Invld; /**< PPK0 Invalid, 0: Default (No protection from writes); 1: Marks PPK0 as invalid */
	u32 Ppk1lck; /**< PPK1 Lock, 0: Default (No protection from writes); 1: Locks writing to PPK1 eFuses */
	u32 Ppk1Invld; /**< PPK1 Invalid, 0: Default (No protection from writes); 1: Marks PPK1 as invalid */
#ifndef SPARTANUPLUSAES1
	u32 Ppk2lck; /**< PPK2 Lock, 0: Default (No protection from writes); 1: Locks writing to PPK2 eFuses */
	u32 Ppk2Invld; /**< PPK2 Invalid, 0: Default (No protection from writes); 1: Marks PPK2 as invalid */
#endif
	u32 UserWrlk; /**< User Write Lock, 0: Default (User eFuses writable); 1: Locks writing to User eFuses */
	u32 JtagErrDis; /**< JTAG Error Out Disable, 0: Default; 1: Disables JTAG error out when set */
	u32 CrcEn; /**< CRC Enable, 0: Default; 1: Enables CRC when set */
	u32 CrcRmaDis; /**< RMA Disable using CRC, 0: Default; 1: Disables RMA when set */
	u32 CrcRmaEn; /**< RMA Enable using CRC, 0: Default; 1: Enables RMA when set */
} XNvm_EfuseSecCtrlBits;

/**
 * eFuse AES key
 */
typedef struct {
	u32 AesKey[XNVM_EFUSE_AES_KEY_SIZE_IN_WORDS]; /**< AES key value to be programmed */
	u32 PrgmAesKey; /**< Flag to determine whether to program AES key or not */
} XNvm_EfuseAesKeys;

/**
 * eFuse AES IV
 */
typedef struct {
	u32 AesIv[XNVM_EFUSE_AES_IV_SIZE_IN_WORDS]; /**< AES IV value to be programmed */
	u32 PrgmIv; /**< Flag to determine whether to program IV or not */
	XNvm_EfuseIvType IvType; /**< IV type */
} XNvm_EfuseAesIvs;

/**
 * eFuse PPK hashes
 */
typedef struct {
	u32 ActualPpkHashSize; /**< PPK hash size to be programmed it can be either 256/384 bit */
	u8 Ppk0Hash[XNVM_EFUSE_PPK_HASH_SIZE_IN_BYTES]; /**< PPK0 hash value to be programmed */
	u8 Ppk1Hash[XNVM_EFUSE_PPK_HASH_SIZE_IN_BYTES]; /**< PPK1 hash value to be programmed */
	u32 PrgmPpk0Hash; /**< Flag to determine whether to program PPK0 hash or not */
	u32 PrgmPpk1Hash; /**< Flag to determine whether to program PPK1 hash or not */
#ifndef SPARTANUPLUSAES1
	u8 Ppk2Hash[XNVM_EFUSE_PPK_HASH_SIZE_IN_BYTES]; /**< PPK2 hash value to be programmed */
	u32 PrgmPpk2Hash; /**< Flag to determine whether to program PPK2 hash or not */
#endif
} XNvm_EfusePpkHash;

/**
 * eFuse Decrypt only
 */
typedef struct {
	u32 PrgmDecOnly; /**< Flag to determine whether to program Decrypt only or not */
} XNvm_EfuseDecOnly;

/**
 * Efuse SPK revoke ID
 */
typedef struct {
	u32 RevokeIdNum; /**< Revoke ID Bit number */
	u32 PrgmSpkRevokeId; /**< Flag to determine whether to program SPK revoke ID or not */
} XNvm_EfuseSpkRevokeId;

/**
 * eFuse AES revoke ID info
 */
typedef struct {
	u32 AesRevokeIdVal; /**< eFuse AES revoke ID value */
	u32 PrgmAesRevokeId; /**< Flag to determine whether to program eFuse AES revoke ID or not */
} XNvm_EfuseAesRevokeId;

/**
 * eFuse Xilinx control
 */
typedef struct {
	u32 PrgmPufHDInvld;  /**< Program PUFHD_INVLD */
	u32 PrgmDisSJtag;    /**< Program Disable Secure JTAG */
	u32 PrgmOspiResetRecoveryDelayCtrl; /**< Program OSPI Reset Recovery Delay Control */
	u32 PrgmRomRsvdOspiDevResetChoice;  /**< Program ROM RSVD OSPI Device Reset Choice */
	u32 PrgmRomOspiCmdSeqCtrl;          /**< Program ROM OSPI Command Sequence Control */
} XNvm_EfuseXilinxCtrl;

/**
 * eFuse user fuse info
 */
typedef struct {
	u32 UserFuseVal; /**< User eFuse value */
	u32 PrgmUserEfuse; /**< Flag to determine whether to program user eFuse or not */
} XNvm_EfuseUserFuse;

#ifdef SPARTANUPLUSAES1
/**
 * eFuse boot mode disable
 */
typedef struct {
	u32 PrgmQspi24ModDis; /**< Flag to program QSPI24 bootmode disable eFuse bit */
	u32 PrgmQspi32ModDis; /**< Flag to program QSPI32 bootmode disable eFuse bit */
	u32 PrgmOspiModDis;   /**< Flag to program OSPI bootmode disable eFuse bit */
	u32 PrgmJtagModDis;   /**< Flag to program JTAG bootmode disable eFuse bit */
	u32 PrgmSmapModDis;   /**< Flag to program SMAP bootmode disable eFuse bit */
	u32 PrgmSerialModDis; /**< Flag to program SERIAL bootmode disable eFuse bit */
} XNvm_EfuseBootModeDis;
#endif

/**
 * eFuse PPK type
 */
typedef enum {
	XNVM_EFUSE_PPK0, /**< eFuse PPK 0 */
	XNVM_EFUSE_PPK1, /**< eFuse PPK 1 */
	XNVM_EFUSE_PPK2 /**< eFuse PPK 2 */
} XNvm_EfusePpkType;


/**
 * Structure defines sub structures of Spartan Ultrascale plus eFuses to be blown
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
#ifdef XNVM_ENABLE_ENV_MONITOR_CHECKS
	XSysMon *SysMonInstPtr; /**< Pointer to XSysMon instance for temperature and voltage checks */
#endif
} XNvm_EfuseData;

/**
 * eFuse error codes
 */
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
	XNVM_EFUSE_ERR_FUSE_PROTECTED = 0xF0, /**< 0xF0 - Error fuse protected */
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
	XNVM_EFUSE_ERR_WRITE_OSPI_RESET_RECOVERY_DELAY_CTRL = 0xAC00, /**< 0xAC00 - Error write OSPI Reset Recovery Delay Control */
	XNVM_EFUSE_ERR_WRITE_ROM_RSVD_OSPI_DEV_RESET_CHOICE = 0xAD00, /**< 0xAD00 - Error write ROM RSVD OSPI Device Reset Choice */
	XNVM_EFUSE_ERR_WRITE_ROM_OSPI_CMD_SEQ_CTRL = 0xAE00, /**< 0xAE00 - Error write ROM OSPI Cmd Seq Ctrl */
	XNVM_EFUSE_ERR_WRITE_SCAN_CLEAR_EN = 0xAF00, /**< - Error write SCAN_CLEAR_EN */
	XNVM_EFUSE_ERR_RD_SEC_CTRL_BITS = 0xC000, /**< 0xC000 - Error read secure control bits */
	XNVM_EFUSE_ERR_INVALID_CLK_FREQUENCY = 0xD000, /**< 0xD000 - Error Invalid Clock Frequency */
	XNVM_EFUSE_ERR_RD_CACHE_BOOT_MODE_DIS_BITS = 0xB000, /**< 0xB000 - Error read Boot mode disable bits from cache */
	XNVM_EFUSE_ERR_WRITE_QSPI24_BOOT_MODE_DIS = 0xB100, /**< 0xB100 - Error write QSPI24 Boot mode disable */
	XNVM_EFUSE_ERR_WRITE_QSPI32_BOOT_MODE_DIS = 0xB200, /**< 0xB200 - Error write QSPI32 Boot mode disable */
	XNVM_EFUSE_ERR_WRITE_OSPI_BOOT_MODE_DIS = 0xB300, /**< 0xB300 - Error write OSPI Boot mode disable */
	XNVM_EFUSE_ERR_WRITE_SMAP_BOOT_MODE_DIS = 0xB400, /**< 0xB400 - Error write SMAP Boot mode disable */
	XNVM_EFUSE_ERR_WRITE_SERIAL_BOOT_MODE_DIS = 0xB500, /**< 0xB500 - Error write SERIAL Boot mode disable */
	XNVM_EFUSE_ERR_WRITE_RMA_DISABLE_0 = 0xB600, /**< 0xB600 - Error write rma disable0 */
	XNVM_EFUSE_ERR_WRITE_RMA_DISABLE_1 = 0xB700, /**< 0xB700 - Error write rma disable1 */
	XNVM_EFUSE_ERR_WRITE_LCK_DWN = 0xB800, /**< 0xB800 - Error write lock down */
	XNVM_EFUSE_ERR_WRITE_JTAG_BOOT_MODE_DIS = 0xB900, /**< 0xB900 - Error write JTAG Boot mode disable */
	XNVM_EFUSE_ERR_BEFORE_PROGRAMMING = 0x80000, /**< 0x80000 - Error before programming */
	XNVM_EFUSE_ERR_READ_TEMPERATURE_OUT_OF_RANGE = 0xF100, /**< 0xF100 - Temperature is out of range */
	XNVM_EFUSE_ERR_READ_VOLTAGE_OUT_OF_RANGE = 0xF200, /**< 0xF200 - Voltage is out of range */
	XNVM_EFUSE_ERR_SYSMON_NOT_AVAILABLE = 0xF300 /**< 0xF300 - SysMon not available */
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
int XNvm_EfuseReadXilinxCtrl(XNvm_EfuseXilinxCtrl *XilinxCtrl);
#ifdef SPARTANUPLUSAES1
int XNvm_EfuseReadBootModeDisBits(XNvm_EfuseBootModeDis *BootModeDisBits);
#endif

#ifdef __cplusplus
}
#endif

#endif	/* XNVM_EFUSE_H */

/** @} */
