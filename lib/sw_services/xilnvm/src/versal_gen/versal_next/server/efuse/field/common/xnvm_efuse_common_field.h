/******************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/******************************************************************************/
/**
*
* @file versal_next/server/efuse/field/common/xnvm_efuse_common_field.h
*
* eFuse band-engine field types, flags, band macros, and shared extern symbols.
 * Band/protection/field table definitions: xnvm_efuse_plat_field.h (per platform).
 * Included from xnvm_efuse_engine.c, xnvm_ops.c, and plat sources via common_field.h.
*
* @cond xnvm_internal
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- --------   -------------------------------------------------------
* 3.8   vss  08/20/2026 Initial release
*
* </pre>
*
* @note
*
* @endcond
*
*******************************************************************************/
#ifndef XNVM_EFUSE_COMMON_FIELD_H_
#define XNVM_EFUSE_COMMON_FIELD_H_

#ifdef __cplusplus
extern "C" {
#endif

/****************************** Include Files *********************************/
#include "xil_types.h"
#include "xil_util.h"
#include "xnvm_common_defs.h"
#include "xnvm_efuse_common.h"

/************************** Constant Definitions *****************************/

/*
 * Band flags (XNvm_EfuseBand.Flags). Use one primary mode; SKIP_VERIFY is
 * optional for AES/UserKey/UDS only (not DME keys or PUF helper data).
 */
/** - Cache-read/modify/program path using XNvm_EfuseComputeProgrammableBits(). */
#define XNVM_EFUSE_BAND_FLAG_COMPUTE_PRG	(XIL_UNSIGNED_ONE << XIL_UNSIGNED_ZERO)
/** - Program caller row data directly without cache merge. */
#define XNVM_EFUSE_BAND_FLAG_DIRECT_WRITE	(XIL_UNSIGNED_ONE << XIL_UNSIGNED_ONE)
/** - Program a constant value from Band->FixedData instead of caller Data[]. */
#define XNVM_EFUSE_BAND_FLAG_FIXED_DATA		(XIL_UNSIGNED_ONE << 2U)
/** @brief Skip read-back verify after each bit program (AES/UserKey/UDS only). */
#define XNVM_EFUSE_BAND_FLAG_SKIP_VERIFY	(XIL_UNSIGNED_ONE << 4U)
/** - Mask of mutually exclusive band programming modes. */
#define XNVM_EFUSE_BAND_MODE_MASK		(XNVM_EFUSE_BAND_FLAG_COMPUTE_PRG | \
						XNVM_EFUSE_BAND_FLAG_DIRECT_WRITE | \
						XNVM_EFUSE_BAND_FLAG_FIXED_DATA)

/** - No per-field metadata flags set. */
#define XNVM_EFUSE_FIELD_FLAG_NONE		(XIL_UNSIGNED_ZERO)
/** - Field has a write-lock cache bit checked before programming. */
#define XNVM_EFUSE_FIELD_FLAG_WR_LK		(XIL_UNSIGNED_ONE << XIL_UNSIGNED_ZERO)

/** - No device-level flags set. */
#define XNVM_EFUSE_DEVICE_FLAG_NONE		(XIL_UNSIGNED_ZERO)
/**
 * Platform has no engine protection table on purpose. XNvm_EfuseEnginePrgmProtectionBits
 * may return success only when this flag is set together with Protection == NULL
 * and NumProtection == 0.
 */
#define XNVM_EFUSE_DEVICE_FLAG_NO_PROTECTION	(XIL_UNSIGNED_ONE << XIL_UNSIGNED_ZERO)

/** - Default source word offset within caller buffer for a band. */
#define XNVM_EFUSE_BAND_SRC_WORD_OFFSET_DEFAULT	(XIL_UNSIGNED_ZERO)
/** - Data mask selecting all bits in a band word. */
#define XNVM_EFUSE_BAND_DATA_MASK_ALL		(0xFFFFFFFFU)
/** - Default fixed-data value for FIXED_DATA bands. */
#define XNVM_EFUSE_BAND_FIXED_DATA_DEFAULT	(XIL_UNSIGNED_ZERO)
/** - Default band write error code placeholder. */
#define XNVM_EFUSE_BAND_ERR_WRITE_DEFAULT	(XIL_UNSIGNED_ZERO)
/** - Data mask selecting no bits (full-row DIRECT_WRITE). */
#define XNVM_EFUSE_BAND_DATA_MASK_NONE		(XIL_UNSIGNED_ZERO)
/** - Row count for single-row band descriptors. */
#define XNVM_EFUSE_BAND_SINGLE_ROW_COUNT	(XIL_UNSIGNED_ONE)

/*
 * Pre-program validation flags (XNvm_EfuseFieldDesc.ValidateFlags).
 * OR into field rows in xnvm_efuse_plat_field.h.
 * CACHE_EMPTY: cache band regions must read zero.
 */
/** - No pre-program validation flags set. */
#define XNVM_EFUSE_VALIDATE_NONE		(XIL_UNSIGNED_ZERO)
/** - Cache band regions must read zero before programming. */
#define XNVM_EFUSE_VALIDATE_CACHE_EMPTY		(XIL_UNSIGNED_ONE << XIL_UNSIGNED_ZERO)
/** - Request must not clear already-blown cache bits (COMPUTE_PRG fields). */
#define XNVM_EFUSE_VALIDATE_NO_BIT_REVERT	(XIL_UNSIGNED_ONE << XIL_UNSIGNED_ONE)
/** - Validate AES key programming policy. */
#define XNVM_EFUSE_VALIDATE_AES_KEY		(XIL_UNSIGNED_ONE << 2U)
/** - Validate DME key programming policy. */
#define XNVM_EFUSE_VALIDATE_DME_KEY		(XIL_UNSIGNED_ONE << 4U)
/** - CRC cache register must be empty before programming. */
#define XNVM_EFUSE_VALIDATE_CRC_EMPTY		(XIL_UNSIGNED_ONE << 5U)
/** - PUF helper-data region must be empty before programming. */
#define XNVM_EFUSE_VALIDATE_PUF_HELPER_EMPTY	(XIL_UNSIGNED_ONE << 6U)
/** - PUF helper-data access must be allowed. */
#define XNVM_EFUSE_VALIDATE_PUF_HD_ACCESS	(XIL_UNSIGNED_ONE << 7U)
/** - Validate FIPS mode field programming. */
#define XNVM_EFUSE_VALIDATE_FIPS_MODE		(XIL_UNSIGNED_ONE << 9U)
/** - Validate FIPS version field programming. */
#define XNVM_EFUSE_VALIDATE_FIPS_VERSION	(XIL_UNSIGNED_ONE << 10U)

/** - Invalid fuse column sentinel. */
#define XNVM_EFUSE_INVALID_COL			(0xFFFFFFFFU)

/** - Number of bits in the FIPS version field. */
#define XNVM_EFUSE_FIPS_VERSION_NUM_BITS	(3U)
/** - DME revoke programs two fuse bits per index (glitch-check pair). */
#define XNVM_EFUSE_DME_REVOKE_BITS_PER_INDEX		(2U)

#define XNVM_EFUSE_SEC_CTRL_NUM_BANDS			(XIL_UNSIGNED_ONE)
						/**< SEC_CTRL field band count */
#define XNVM_EFUSE_MISC_CTRL_NUM_BANDS			(XIL_UNSIGNED_ONE)
						/**< MISC_CTRL field band count */
#define XNVM_EFUSE_BOOT_ENV_CTRL_NUM_BANDS		(XIL_UNSIGNED_ONE)
						/**< BOOT_ENV_CTRL field band count */
#define XNVM_EFUSE_SEC_MISC1_NUM_BANDS			(XIL_UNSIGNED_ONE)
						/**< SEC_MISC1 field band count */
#define XNVM_EFUSE_ROM_RSVD_NUM_BANDS			(XIL_UNSIGNED_ONE)
						/**< ROM_RSVD field band count */
#define XNVM_EFUSE_BOOT_MODE_DIS_NUM_BANDS		(XIL_UNSIGNED_ONE)
						/**< BOOT_MODE_DIS field band count */
#define XNVM_EFUSE_GLITCH_CONFIG_NUM_BANDS		(XIL_UNSIGNED_ONE)
						/**< GLITCH_CONFIG field band count */
#define XNVM_EFUSE_GLITCH_WR_LK_NUM_BANDS		(XIL_UNSIGNED_ONE)
						/**< GLITCH_WR_LK field band count */
#define XNVM_EFUSE_DEC_ONLY_NUM_BANDS			(XIL_UNSIGNED_ONE)
						/**< DEC_ONLY field band count */
#define XNVM_EFUSE_REVOKE_ID_NUM_BANDS			(2U)
						/**< REVOKE_ID field band count */
#define XNVM_EFUSE_REVOKE_ID_NUM_ROWS			(16U)
						/**< REVOKE_ID fuse rows per band */
#define XNVM_EFUSE_REVOKE_ID_BAND0_CACHE_WORDS		(4U)
						/**< REVOKE_ID band-0 cache word count */
#define XNVM_EFUSE_REVOKE_ID_BAND1_SRC_OFFSET		(4U)
						/**< REVOKE_ID band-1 source word offset */
#define XNVM_EFUSE_CACHE_REVOCATION_ID_0_OFFSET		(0x000000B0U)
						/**< Cache offset for revocation ID 0 */
#define XNVM_EFUSE_CACHE_OFFCHIP_REVOKE_0_OFFSET	(0x00000160U)
						/**< Cache offset for off-chip revoke ID 0 */
#define XNVM_EFUSE_CACHE_ANLG_TRIM_3_GLITCH_DET_WR_LK_MASK	(0x80000000U)
						/**< Glitch-det WR_LK mask in ANLG_TRIM_3 cache */
#define XNVM_EFUSE_CACHE_ROM_RSVD_PLM_UPDATE_MASK	(0x00000400U)
						/**< PLM update disable bit in ROM_RSVD cache */
#define XNVM_EFUSE_REVOKE_ID_0_TO_127_END_COL_NUM	(23U)
						/**< Last column for revocation IDs 0-127 */
#define XNVM_EFUSE_REVOKE_ID_128_TO_255_END_COL_NUM	(31U)
						/**< Last column for revocation IDs 128-255 */
#define XNVM_EFUSE_OFFCHIP_REVOKE_ID_NUM_BANDS		(XIL_UNSIGNED_ONE)
						/**< OFFCHIP_REVOKE_ID field band count */
#define XNVM_EFUSE_PLM_UPDATE_DISABLE_NUM_BANDS		(XIL_UNSIGNED_ONE)
						/**< PLM_UPDATE_DISABLE field band count */
#define XNVM_EFUSE_FIPS_MODE_NUM_BANDS			(XIL_UNSIGNED_ONE)
						/**< FIPS_MODE field band count */
#define XNVM_EFUSE_DME_MODE_NUM_BANDS			(XIL_UNSIGNED_ONE)
						/**< DME_MODE field band count */
#define XNVM_EFUSE_AES_KEY_NUM_BANDS			(2U)
						/**< AES_KEY field band count */
#define XNVM_EFUSE_USER_KEY0_NUM_BANDS			(3U)
						/**< USER_KEY0 field band count */
#define XNVM_EFUSE_USER_KEY1_NUM_BANDS			(3U)
						/**< USER_KEY1 field band count */
#define XNVM_EFUSE_UDS_NUM_BANDS			(4U)
						/**< UDS field band count */
#define XNVM_EFUSE_DME_KEY_NUM_BANDS			(XIL_UNSIGNED_ONE)
						/**< DME_KEY field band count */
#define XNVM_EFUSE_IV_META_HEADER_NUM_BANDS		(XIL_UNSIGNED_ONE)
						/**< IV_META_HEADER field band count */
#define XNVM_EFUSE_IV_BLACK_NUM_BANDS			(XIL_UNSIGNED_ONE)
						/**< IV_BLACK field band count */
#define XNVM_EFUSE_IV_PLM_NUM_BANDS			(XIL_UNSIGNED_ONE)
						/**< IV_PLM field band count */
#define XNVM_EFUSE_IV_DATA_PARTITION_NUM_BANDS		(XIL_UNSIGNED_ONE)
						/**< IV_DATA_PARTITION field band count */
#ifndef XNVM_ACCESS_PUF_USER_DATA
#define XNVM_EFUSE_PUF_SYN_DATA_NUM_BANDS		(2U)
						/**< PUF_SYN_DATA field band count */
#define XNVM_EFUSE_PUF_CHASH_NUM_BANDS			(XIL_UNSIGNED_ONE)
						/**< PUF_CHASH field band count */
#define XNVM_EFUSE_PUF_AUX_NUM_BANDS			(XIL_UNSIGNED_ONE)
						/**< PUF_AUX field band count */
#define XNVM_EFUSE_PUF_RO_SWAP_NUM_BANDS		(XIL_UNSIGNED_ONE)
						/**< PUF_RO_SWAP field band count */
#endif
#define XNVM_EFUSE_PUF_SEC_CTRL_NUM_BANDS		(XIL_UNSIGNED_ONE)
						/**< PUF_SEC_CTRL field band count */
#define XNVM_EFUSE_CRC_NUM_BANDS			(2U)
						/**< CRC field band count */

/** - DME key cache offsets used by shared validation policy */
#define XNVM_EFUSE_DME_0_USER_EFUSE_CACHE_OFFSET		(0x00000240U)
						/**< DME key 0 user cache offset */
#define XNVM_EFUSE_DME_1_USER_EFUSE_CACHE_OFFSET		(0x00000270U)
						/**< DME key 1 user cache offset */
#define XNVM_EFUSE_DME_2_USER_EFUSE_CACHE_OFFSET		(0x000002A0U)
						/**< DME key 2 user cache offset */
#define XNVM_EFUSE_DME_3_USER_EFUSE_CACHE_OFFSET		(0x000002D0U)
						/**< DME key 3 user cache offset */

/**************************** Type Definitions *******************************/

/** -
 * This enum defines protection (Tbits) trigger conditions.
 */
typedef enum {
	XNVM_EFUSE_PROT_TRIGGER_REG_NONZERO, /**< Trigger when cache word non-zero */
	XNVM_EFUSE_PROT_TRIGGER_REG_MASKED_NONZERO, /**< Trigger when (reg & Mask) non-zero */
	XNVM_EFUSE_PROT_TRIGGER_RANGE_NONZERO, /**< Trigger when any word in range non-zero */
} XNvm_EfuseProtTriggerType;

/** -
 * This enum defines field programming preconditions.
 */
typedef enum {
	XNVM_EFUSE_PRECOND_DEC_ONLY, /** DEC_ONLY programmed precondition. */
	XNVM_EFUSE_PRECOND_NONE = 0xFFFFFFFFU, /**< No precondition */
} XNvm_EfusePrecondId;

/** -
 * This enum defines band-engine eFuse field identifiers.
 */
typedef enum {
	XNVM_EFUSE_FIELD_SEC_CTRL, /**< Security control word */
	XNVM_EFUSE_FIELD_MISC_CTRL, /**< Miscellaneous control word */
	XNVM_EFUSE_FIELD_SEC_MISC1, /**< Security miscellaneous 1 word */
	XNVM_EFUSE_FIELD_BOOT_ENV_CTRL, /**< Boot environment control */
	XNVM_EFUSE_FIELD_ROM_RSVD, /**< ROM reserved bits */
	XNVM_EFUSE_FIELD_BOOT_MODE_DIS, /**< Boot mode disable bits */
	XNVM_EFUSE_FIELD_GLITCH_CONFIG, /**< Glitch configuration */
	XNVM_EFUSE_FIELD_GLITCH_WR_LK, /**< Glitch detection write lock */
	XNVM_EFUSE_FIELD_DEC_ONLY, /**< DEC_ONLY fuse */
	XNVM_EFUSE_FIELD_REVOKE_ID, /**< Image revocation ID (split-column bands) */
	XNVM_EFUSE_FIELD_OFFCHIP_REVOKE_ID, /**< Off-chip revocation ID index */
	XNVM_EFUSE_FIELD_DME_REVOKE, /**< DME revoke index */
	XNVM_EFUSE_FIELD_PLM_UPDATE_DISABLE, /**< In-place PLM update disable */
	XNVM_EFUSE_FIELD_FIPS_MODE, /**< FIPS mode enable bit */
	XNVM_EFUSE_FIELD_FIPS_VERSION, /**< FIPS version value */
	XNVM_EFUSE_FIELD_PPK0_HASH, /**< PPK0 hash */
	XNVM_EFUSE_FIELD_PPK1_HASH, /**< PPK1 hash */
	XNVM_EFUSE_FIELD_PPK2_HASH, /**< PPK2 hash */
	XNVM_EFUSE_FIELD_IV_META_HEADER, /**< Meta-header IV */
	XNVM_EFUSE_FIELD_IV_BLACK, /**< Black obfuscation IV */
	XNVM_EFUSE_FIELD_IV_PLM, /**< PLM IV */
	XNVM_EFUSE_FIELD_IV_DATA_PARTITION, /**< Data-partition IV */
	XNVM_EFUSE_FIELD_AES_KEY, /**< AES red key */
	XNVM_EFUSE_FIELD_USER_KEY0, /**< User key 0 */
	XNVM_EFUSE_FIELD_USER_KEY1, /**< User key 1 */
	XNVM_EFUSE_FIELD_UDS, /**< UDS */
	XNVM_EFUSE_FIELD_DME_KEY0, /**< DME user key 0 */
	XNVM_EFUSE_FIELD_DME_KEY1, /**< DME user key 1 */
	XNVM_EFUSE_FIELD_DME_KEY2, /**< DME user key 2 */
	XNVM_EFUSE_FIELD_DME_KEY3, /**< DME user key 3 */
	XNVM_EFUSE_FIELD_DME_MODE, /**< DME mode enable */
	XNVM_EFUSE_FIELD_PUF_SYN_DATA, /**< PUF syndrome data */
	XNVM_EFUSE_FIELD_PUF_CHASH, /**< PUF CHASH */
	XNVM_EFUSE_FIELD_PUF_AUX, /**< PUF AUX */
	XNVM_EFUSE_FIELD_PUF_RO_SWAP, /**< PUF RO swap enable */
	XNVM_EFUSE_FIELD_PUF_SEC_CTRL, /**< PUF security control bits (row 95 cols 29-31) */
	XNVM_EFUSE_FIELD_CRC, /**< Manufacturing CRC */
	XNVM_EFUSE_FIELD_COUNT /**< Number of field IDs (not a field) */
} XNvm_EfuseFieldId;

/** -
 * This structure defines a rectangular eFuse programming band.
 */
typedef struct {
	u32 StartRow; /**< First fuse row on Page */
	u32 ColStart; /**< First column (inclusive) */
	u32 ColEnd; /**< Last column (inclusive) */
	u32 NumRows; /**< Number of consecutive rows */
	u32 Page; /**< eFuse page select */
	u32 CacheStartOffset; /**< Start byte offset in eFuse cache */
	u32 CacheEndOffset; /**< End byte offset in eFuse cache */
	u32 SrcWordOffset; /**< Index into caller Data[] for DIRECT_WRITE/COMPUTE_PRG */
	u32 DataMask; /**< AND mask applied to source word before program */
	u32 FixedData; /**< Constant value for FIXED_DATA bands */
	u32 Flags; /**< Band mode flags (XNVM_EFUSE_BAND_FLAG_*) */
	u32 ErrWrite; /**< Error code returned on program failure */
} XNvm_EfuseBand;

/** -
 * This structure defines a band-engine field descriptor.
 */
typedef struct {
	XNvm_EfuseFieldId Id; /**< Field identifier */
	const XNvm_EfuseBand *Bands; /**< Band table for this field */
	u32 NumBands; /**< Number of entries in Bands */
	u32 Flags; /**< Field flags (XNVM_EFUSE_FIELD_FLAG_*) */
	u32 ValidateFlags; /**< Pre-program validation policy */
	u32 WrLkCacheOffset; /**< Cache offset of write-lock word */
	u32 WrLkMask; /**< Write-lock bit mask in cache */
	u32 ErrBeforePgm; /**< Error when validation fails before program */
	u32 ErrWrite; /**< Error when program fails */
	XNvm_EfusePrecondId PrecondId; /**< Field-specific pre-write check (see XNvm_EfusePrecondId) */
	u32 MinSrcWords; /**< Minimum caller buffer length in 32-bit words */
} XNvm_EfuseFieldDesc;

/** -
 * This structure defines a protection (Tbits) trigger entry.
 */
typedef struct {
	XNvm_EfuseProtTriggerType TriggerType; /**< Condition that arms programming */
	u32 CacheOffset; /**< Cache address monitored for trigger */
	u32 NumWords; /**< Word count for RANGE_NONZERO trigger */
	u32 Mask; /**< Bit mask for MASKED_NONZERO trigger */
	u32 Page; /**< eFuse page for protection bit */
	u32 ProtRow; /**< Protection fuse row */
	u32 ProtCol0; /**< First protection fuse column */
	u32 ProtCol1; /**< Second protection fuse column (paired check) */
	u32 ProtReadMask; /**< Expected read-back mask after program */
	u32 ErrProt0; /**< Error code for ProtCol0 failure */
	u32 ErrProt1; /**< Error code for ProtCol1 failure */
} XNvm_EfuseProtEntry;

/** -
 * This structure defines one DME revoke table entry.
 */
typedef struct {
	u32 Row; /**< Fuse row for this revoke slot */
	u32 Col[2U]; /**< Glitch-check column pair */
} XNvm_EfuseDmeRevokeEntry;

/** -
 * This structure defines the platform DME revoke coordinate table.
 */
typedef struct {
	u32 Page; /**< eFuse page select */
	u32 MaxIndex; /**< Largest valid revoke index */
	const XNvm_EfuseDmeRevokeEntry *Table; /**< Per-index row and column pair */
} XNvm_EfuseDmeRevokeMap;

/** -
 * This structure defines FIPS version scattered-column mapping.
 */
typedef struct {
	u32 Row; /**< Fuse row holding version bits */
	u32 Page; /**< eFuse page select */
	u32 BitCols[XNVM_EFUSE_FIPS_VERSION_NUM_BITS]; /**< Column for each version bit */
	u32 MaxVersion; /**< Largest encodable version value */
} XNvm_EfuseFipsVersionMap;

/** -
 * This structure defines DME key validation policy.
 */
typedef struct {
	const u32 *CacheOffsets; /**< Per-key cache offsets to check empty */
	u32 NumKeys; /**< Number of DME keys in policy */
	u32 BlockIfDmeModeEn; /**< Non-zero when DME mode blocks programming */
} XNvm_EfuseDmeKeyPolicy;

typedef struct XNvm_EfuseDevice XNvm_EfuseDevice;

/** -
 * This typedef defines a post-program field hook callback.
 */
typedef int (*XNvm_EfuseFieldPostProgramFn)(XNvm_EfuseFieldId FieldId,
		const void *Data);

/** -
 * This typedef defines a post-cache-reload protection-check callback.
 */
typedef int (*XNvm_EfuseProtectionChecksFn)(void);

/** -
 * This typedef defines an optional extra pre-program field check.
 */
typedef int (*XNvm_EfuseValidateFieldFn)(const XNvm_EfuseFieldDesc *Field);

/** -
 * This structure defines platform device operation hooks.
 */
typedef struct {
	XNvm_EfuseFieldPostProgramFn PostProgram; /**< Optional post-program callback */
	XNvm_EfuseProtectionChecksFn ProtectionChecks; /**< Optional post-reload checks; NULL uses default */
	XNvm_EfuseValidateFieldFn ValidateField; /**< Optional extra pre-program checks; NULL skips */
} XNvm_EfuseDeviceOps;

/** -
 * This structure defines the band-engine platform device descriptor.
 */
struct XNvm_EfuseDevice {
	const XNvm_EfuseFieldDesc *CommonFields; /**< Common field descriptors */
	u32 NumCommonFields; /**< Number of common field descriptors */
	const XNvm_EfuseFieldDesc *Fields; /**< Platform-specific field descriptors */
	u32 NumFields; /**< Number of platform field descriptors */
	const XNvm_EfuseProtEntry *Protection; /**< Protection trigger table */
	u32 NumProtection; /**< Number of protection entries */
	u32 Flags; /**< Device flags (XNVM_EFUSE_DEVICE_FLAG_*) */
	XNvm_EfuseDeviceOps *Ops; /**< Platform operation hooks */
	const XNvm_EfuseDmeRevokeMap *DmeRevoke; /**< DME revoke coordinate table */
	const XNvm_EfuseFipsVersionMap *FipsVersion; /**< FIPS version map */
	const XNvm_EfuseDmeKeyPolicy *DmeKeys; /**< DME key validation policy */
	u32 SupportsDmeMode; /**< Non-zero when device supports DME mode field */
};

/** - Upper bound for PPK hash field identifiers. */
#define XNVM_EFUSE_FIELD_PPK_HASH_MAX \
	((XNvm_EfuseFieldId)((u32)XNVM_EFUSE_FIELD_PPK2_HASH))

/***************** Macros (Inline Functions) Definitions *********************/

/** - Single cache-word band on eFuse page 0. */
#define XNVM_EFUSE_BAND_WORD(CStart, CEnd, CacheOff, Rows, Row, BandFlags) \
	{ \
		.StartRow = (Row), \
		.ColStart = (CStart), \
		.ColEnd = (CEnd), \
		.NumRows = (Rows), \
		.Page = XNVM_EFUSE_PAGE_0, \
		.CacheStartOffset = (CacheOff), \
		.CacheEndOffset = (CacheOff), \
		.SrcWordOffset = XNVM_EFUSE_BAND_SRC_WORD_OFFSET_DEFAULT, \
		.DataMask = XNVM_EFUSE_BAND_DATA_MASK_ALL, \
		.FixedData = XNVM_EFUSE_BAND_FIXED_DATA_DEFAULT, \
		.Flags = (BandFlags), \
		.ErrWrite = XNVM_EFUSE_BAND_ERR_WRITE_DEFAULT, \
	}

/** - Multi-word or direct-write band on eFuse page 0. */
#define XNVM_EFUSE_BAND_SECTION(CStart, CEnd, CacheStart, CacheEnd, Rows, \
		Row, SrcOff, BandFlags) \
	{ \
		.StartRow = (Row), \
		.ColStart = (CStart), \
		.ColEnd = (CEnd), \
		.NumRows = (Rows), \
		.Page = XNVM_EFUSE_PAGE_0, \
		.CacheStartOffset = (CacheStart), \
		.CacheEndOffset = (CacheEnd), \
		.SrcWordOffset = (SrcOff), \
		.DataMask = XNVM_EFUSE_BAND_DATA_MASK_ALL, \
		.FixedData = XNVM_EFUSE_BAND_FIXED_DATA_DEFAULT, \
		.Flags = (BandFlags), \
		.ErrWrite = XNVM_EFUSE_BAND_ERR_WRITE_DEFAULT, \
	}

/** - Like XNVM_EFUSE_BAND_SECTION on an arbitrary eFuse page. */
#define XNVM_EFUSE_BAND_SECTION_PAGE(EfusePage, CStart, CEnd, CacheStart, CacheEnd, \
		Rows, Row, SrcOff, BandFlags) \
	{ \
		.StartRow = (Row), \
		.ColStart = (CStart), \
		.ColEnd = (CEnd), \
		.NumRows = (Rows), \
		.Page = (EfusePage), \
		.CacheStartOffset = (CacheStart), \
		.CacheEndOffset = (CacheEnd), \
		.SrcWordOffset = (SrcOff), \
		.DataMask = XNVM_EFUSE_BAND_DATA_MASK_ALL, \
		.FixedData = XNVM_EFUSE_BAND_FIXED_DATA_DEFAULT, \
		.Flags = (BandFlags), \
		.ErrWrite = XNVM_EFUSE_BAND_ERR_WRITE_DEFAULT, \
	}

/*************************** Variable Declarations ****************************/
extern const XNvm_EfuseDevice XNvm_EfuseDeviceDesc;

/************************** Function Prototypes ******************************/
/******************************************************************************/
/** -
 * @brief	Finds a field descriptor in the device table by ID.
 *
 * @param	Dev - Device table descriptor.
 * @param	FieldId - Field identifier to look up.
 *
 * @return	Pointer to the field descriptor, or NULL if not found.
 *
 ******************************************************************************/
static inline const XNvm_EfuseFieldDesc *XNvm_EfuseLookupFieldDesc(
		const XNvm_EfuseDevice *Dev, XNvm_EfuseFieldId FieldId)
{
	u32 Index = XIL_UNSIGNED_ZERO;
	const XNvm_EfuseFieldDesc *Field = NULL;

	if ((Dev != NULL) && (Dev->CommonFields != NULL)) {
		for (Index = XIL_UNSIGNED_ZERO; Index < Dev->NumCommonFields; Index++) {
			if (Dev->CommonFields[Index].Id == FieldId) {
				Field = &Dev->CommonFields[Index];
				break;
			}
		}
	}

	if ((Field == NULL) && (Dev != NULL) && (Dev->Fields != NULL)) {
		for (Index = XIL_UNSIGNED_ZERO; Index < Dev->NumFields; Index++) {
			if (Dev->Fields[Index].Id == FieldId) {
				Field = &Dev->Fields[Index];
				break;
			}
		}
	}

	return Field;
}

#ifdef __cplusplus
}
#endif

#endif /* XNVM_EFUSE_COMMON_FIELD_H_ */
