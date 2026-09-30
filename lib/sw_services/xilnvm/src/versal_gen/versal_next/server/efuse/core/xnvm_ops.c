/******************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
*
* @file versal_next/server/efuse/core/xnvm_ops.c
*
* Band-engine eFuse field operations and write-request validation.
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

/**
 * @addtogroup xnvm_efuse_server_apis XilNvm eFUSE Server APIs
 * @{
 */

/***************************** Include Files *********************************/
#include "xnvm_ops.h"
#include "xnvm_validate.h"
#include "xil_util.h"
#include "xnvm_common_defs.h"
#include "xnvm_defs.h"
#include "xnvm_utils.h"
#include "xstatus.h"
#include "xnvm_efuse_error.h"
#include "xnvm_efuse_hw.h"
#include "xnvm_efuse_common_field.h"
#include "xnvm_efuse_common_hw.h"
#include "xnvm_efuse_common.h"
#ifdef XNVM_EFUSE_HAS_2ND_AES_KEYS
#include "xnvm_efuse_plat.h"
#endif

/************************** Constant Definitions *****************************/

/** - Number of cache words in manufacturing CRC input. */
#define XNVM_EFUSE_CRC_INPUT_COUNT		(37U)
/** - Byte length of manufacturing CRC input buffer. */
#define XNVM_EFUSE_CRC_DATA_BYTE_COUNT		(XNVM_EFUSE_CRC_INPUT_COUNT * XNVM_WORD_LEN)
/** - All security-default bits set (CRC calculation). */
#define XNVM_EFUSE_SEC_DEF_VAL_ALL_BIT_SET	(0xFFFFFFFFU)
/** - Index of masked nibble in CRC input array. */
#define XNVM_EFUSE_CRC_MASKED_IDX		(15U)
/** - Mask applied to masked CRC input nibble. */
#define XNVM_EFUSE_CRC_MASKED_MASK		(0x0000000FU)
/** - CRC-32 reflected polynomial. */
#define XNVM_EFUSE_CRC32_POLY			(0xEDB88320U)
/** - Number of bits in one byte. */
#define XNVM_EFUSE_BITS_IN_A_BYTE		(8U)
/** - Bit-0 mask for fuse column indexing. */
#define XNVM_EFUSE_BIT0_MASK			(0x01U)
/** - Reverse CRC-32 polynomial. */
#define XNVM_EFUSE_REVERSE_POLYNOMIAL				(0x82F63B78U)

/**************************** Type Definitions *******************************/

/** - Post-program CRC verify descriptor for a key field. */
typedef struct {
	XNvm_EfuseFieldId FieldId; /**< Field receiving post-program CRC check */
	u32 CrcRegOffset; /**< CRC status register offset */
	u32 CrcDoneMask; /**< CRC done status mask */
	u32 CrcPassMask; /**< CRC pass status mask */
	u32 ErrWrite; /**< Error code on CRC/write failure */
	u32 CrcWordCount; /**< Number of data words included in CRC */
} XNvm_EfusePostProgDesc;

/** - Field identifier to AES-class key type mapping entry. */
typedef struct {
	XNvm_EfuseFieldId FieldId; /**< Band-engine field identifier */
	XNvm_AesKeyType KeyType; /**< Matching AES-class key type */
} XNvm_EfuseAesKeyFieldMap;

/************************** Function Prototypes ******************************/
static u32 XNvm_EfuseOpsCalculateCrc32(const u8 *Data, u32 Len);
static int XNvm_EfuseOpsProtectionChecks(void);
static int XNvm_EfuseOpsCheckCacheNonZero(u32 StartOffset, u32 NumRows);
static int XNvm_EfuseOpsPostProgram(XNvm_EfuseFieldId FieldId, const void *Data);
static int XNvm_EfuseOpsValidateCrcField(void);
static int XNvm_EfuseOpsValidateDmeKeyField(const XNvm_EfuseDevice *Dev,
		XNvm_EfuseFieldId FieldId);
static int XNvm_EfuseOpsValidateCacheEmptyBands(const XNvm_EfuseFieldDesc *Field,
		u32 ErrAlreadyPrgmd);
static int XNvm_EfuseOpsValidateCacheEmptyField(const XNvm_EfuseFieldDesc *Field,
		XNvm_EfuseFieldId FieldId);
static int XNvm_EfuseOpsGetCacheEmptyErr(u32 FieldId);
static int XNvm_EfuseOpsValidateFipsField(const XNvm_EfuseFieldDesc *Field,
		const u32 *Data, u32 NumWords);
static int XNvm_EfuseOpsValidateIvField(XNvm_EfuseFieldId FieldId,
		const u32 *Data, u32 NumWords);
static int XNvm_EfuseOpsFieldToAesKeyType(XNvm_EfuseFieldId FieldId,
		XNvm_AesKeyType *KeyType);

/************************** Variable Definitions *****************************/

/* Array of cache register offsets used for manufacturing CRC calculation */
static const u32 XNvm_EfuseCrcInputOffsets[XNVM_EFUSE_CRC_INPUT_COUNT] = {
	0x00000004U, 0x00000008U, 0x0000000CU, 0x0000001CU,
	0x00000020U, 0x00000024U, 0x00000028U, 0x0000002CU,
	0x00000098U, 0x0000009CU, 0x000000D8U, 0x000000DCU,
	0x000000E0U, 0x000000F4U, 0x000000F8U, 0x000000FCU,
	0x0000018CU,
	0x00000190U, 0x00000194U, 0x00000198U, 0x0000019CU,
	0x000001A0U, 0x000001A4U, 0x000001A8U, 0x000001ACU,
	0x000001B0U, 0x000001B4U, 0x000001B8U, 0x000001BCU,
	0x000001C0U,
	0x000001C4U, 0x000001C8U, 0x000001CCU,
	0x000001F4U, 0x000001F8U, 0x000001FCU, 0x00000200U,
};

/**
 * Field identifier to AES-class key type map.
 *
 * The two enumerations are deliberately not assumed to be parallel, so field
 * identifiers can be appended without disturbing key type numbering.
 */
static const XNvm_EfuseAesKeyFieldMap XNvm_EfuseAesKeyFieldTbl[] = {
	{ XNVM_EFUSE_FIELD_AES_KEY, XNVM_EFUSE_AES_KEY },
	{ XNVM_EFUSE_FIELD_USER_KEY0, XNVM_EFUSE_USER_KEY_0 },
	{ XNVM_EFUSE_FIELD_USER_KEY1, XNVM_EFUSE_USER_KEY_1 },
#ifdef XNVM_EFUSE_FIELD_AES_KEY_2ND
	{ XNVM_EFUSE_FIELD_AES_KEY_2ND, XNVM_EFUSE_AES_KEY_2ND },
	{ XNVM_EFUSE_FIELD_USER_KEY0_2ND, XNVM_EFUSE_USER_KEY_0_2ND },
	{ XNVM_EFUSE_FIELD_USER_KEY1_2ND, XNVM_EFUSE_USER_KEY_1_2ND },
#endif
};

static const XNvm_EfusePostProgDesc XNvm_EfusePostProgTbl[] = {
	{
		XNVM_EFUSE_FIELD_AES_KEY,
		XNVM_EFUSE_AES_CRC_REG_OFFSET,
		XNVM_EFUSE_CTRL_STATUS_AES_CRC_DONE_MASK,
		XNVM_EFUSE_CTRL_STATUS_AES_CRC_PASS_MASK,
		XNVM_EFUSE_ERR_WRITE_AES_KEY,
		XNVM_AES_KEY_SIZE_IN_WORDS,
	},
	{
		XNVM_EFUSE_FIELD_USER_KEY0,
		XNVM_EFUSE_AES_USR_KEY0_CRC_REG_OFFSET,
		XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_0_CRC_DONE_MASK,
		XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_0_CRC_PASS_MASK,
		XNVM_EFUSE_ERR_WRITE_USER_KEY0,
		XNVM_AES_KEY_SIZE_IN_WORDS,
	},
	{
		XNVM_EFUSE_FIELD_USER_KEY1,
		XNVM_EFUSE_AES_USR_KEY1_CRC_REG_OFFSET,
		XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_1_CRC_DONE_MASK,
		XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_1_CRC_PASS_MASK,
		XNVM_EFUSE_ERR_WRITE_USER_KEY1,
		XNVM_AES_KEY_SIZE_IN_WORDS,
	},
#if defined(XNVM_EFUSE_FIELD_AES_KEY_2ND) && \
	defined(XNVM_EFUSE_AES_KEY_0_2ND_CRC_REG_OFFSET)
	{
		XNVM_EFUSE_FIELD_AES_KEY_2ND,
		XNVM_EFUSE_AES_KEY_0_2ND_CRC_REG_OFFSET,
		XNVM_EFUSE_CTRL_STATUS_AES_KEY_0_2ND_CRC_DONE_MASK,
		XNVM_EFUSE_CTRL_STATUS_AES_KEY_0_2ND_CRC_PASS_MASK,
		XNVM_EFUSE_ERR_WRITE_AES_KEY,
		XNVM_AES_KEY_SIZE_IN_WORDS,
	},
	{
		XNVM_EFUSE_FIELD_USER_KEY0_2ND,
		XNVM_EFUSE_USER_KEY_0_2ND_CRC_REG_OFFSET,
		XNVM_EFUSE_CTRL_STATUS_USER_KEY_0_2ND_CRC_DONE_MASK,
		XNVM_EFUSE_CTRL_STATUS_USER_KEY_0_2ND_CRC_PASS_MASK,
		XNVM_EFUSE_ERR_WRITE_USER_KEY0,
		XNVM_AES_KEY_SIZE_IN_WORDS,
	},
	{
		XNVM_EFUSE_FIELD_USER_KEY1_2ND,
		XNVM_EFUSE_USER_KEY_1_2ND_CRC_REG_OFFSET,
		XNVM_EFUSE_CTRL_STATUS_USER_KEY_1_2ND_CRC_DONE_MASK,
		XNVM_EFUSE_CTRL_STATUS_USER_KEY_1_2ND_CRC_PASS_MASK,
		XNVM_EFUSE_ERR_WRITE_USER_KEY1,
		XNVM_AES_KEY_SIZE_IN_WORDS,
	},
#endif
	{
		XNVM_EFUSE_FIELD_UDS,
		XNVM_EFUSE_CTRL_UDS_DICE_CRC_OFFSET,
		XNVM_EFUSE_CTRL_STATUS_UDS_DICE_CRC_DONE_MASK,
		XNVM_EFUSE_CTRL_STATUS_UDS_DICE_CRC_PASS_MASK,
		XNVM_EFUSE_ERR_WRITE_UDS,
		XNVM_UDS_SIZE_IN_WORDS,
	},
};

/************************** Function Definitions *****************************/

/******************************************************************************/
/**
 * @brief	Maps a band-engine field identifier to its AES-class key type.
 *
 * @param	FieldId - Field identifier to translate.
 * @param	KeyType - Receives the matching AES-class key type.
 *
 * @return
 *		- XST_SUCCESS when FieldId names an AES-class key.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM otherwise.
 *
 ******************************************************************************/
static int XNvm_EfuseOpsFieldToAesKeyType(XNvm_EfuseFieldId FieldId,
		XNvm_AesKeyType *KeyType)
{
	int Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
	u32 Index = XIL_UNSIGNED_ZERO;

	if (KeyType == NULL) {
		goto END;
	}

	for (Index = XIL_UNSIGNED_ZERO; Index < (sizeof(XNvm_EfuseAesKeyFieldTbl) /
			sizeof(XNvm_EfuseAesKeyFieldTbl[0])); Index++) {
		if (XNvm_EfuseAesKeyFieldTbl[Index].FieldId == FieldId) {
			*KeyType = XNvm_EfuseAesKeyFieldTbl[Index].KeyType;
			Status = XST_SUCCESS;
			break;
		}
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	Maps an AES-class key type to its band-engine field identifier.
 *
 * @param	KeyType - AES-class key type to translate.
 * @param	FieldId - Receives the matching field identifier.
 *
 * @return
 *		- XST_SUCCESS when KeyType names a supported AES-class key.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM otherwise.
 *
 ******************************************************************************/
int XNvm_EfuseOpsAesKeyTypeToField(XNvm_AesKeyType KeyType,
		XNvm_EfuseFieldId *FieldId)
{
	int Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
	u32 Index = XIL_UNSIGNED_ZERO;

	if (FieldId == NULL) {
		goto END;
	}

	for (Index = XIL_UNSIGNED_ZERO; Index < (sizeof(XNvm_EfuseAesKeyFieldTbl) /
			sizeof(XNvm_EfuseAesKeyFieldTbl[0])); Index++) {
		if (XNvm_EfuseAesKeyFieldTbl[Index].KeyType == KeyType) {
			*FieldId = XNvm_EfuseAesKeyFieldTbl[Index].FieldId;
			Status = XST_SUCCESS;
			break;
		}
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	Rejects programming when the field write-lock cache bit is set.
 *
 * @param	Field - Field descriptor with optional write-lock metadata.
 *
 * @return
 *		- XST_SUCCESS when the field is not write-locked.
 *		- Field ErrWrite OR'd with XNVM_EFUSE_ERR_FUSE_PROTECTED when locked.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM when Field is NULL.
 *
 ******************************************************************************/
int XNvm_EfuseOpsValidateFieldWrLk(const XNvm_EfuseFieldDesc *Field)
{
	u32 CacheVal = XIL_UNSIGNED_ZERO;
	int Status = XST_FAILURE;

	if (Field == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
	} else if ((Field->Flags & XNVM_EFUSE_FIELD_FLAG_WR_LK) != XIL_UNSIGNED_ZERO) {
		CacheVal = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				Field->WrLkCacheOffset);
		if ((CacheVal & Field->WrLkMask) != XIL_UNSIGNED_ZERO) {
			Status = (int)(Field->ErrWrite |
				XNVM_EFUSE_ERR_FUSE_PROTECTED);
		} else {
			Status = XST_SUCCESS;
		}
	} else {
		Status = XST_SUCCESS;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	Validates an eFuse field write request (ValidateFlags, WrLk, PrecondId).
 *
 * @param	Dev - Platform device descriptor.
 * @param	FieldId - Engine field identifier to validate.
 * @param	Data - Source buffer for no-bit-revert validation; NULL when not required.
 * @param	NumWords - Number of 32-bit words in Data.
 *
 * @return
 *		- XST_SUCCESS if the write request may proceed.
 *		- Field-specific or invalid-param error on failure.
 *
 ******************************************************************************/
int XNvm_EfuseOpsValidateWriteField(const XNvm_EfuseDevice *Dev,
		XNvm_EfuseFieldId FieldId, const u32 *Data, u32 NumWords)
{
	const XNvm_EfuseFieldDesc *Field = NULL;
	int Status = XST_FAILURE;

	if (Dev == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Field = XNvm_EfuseLookupFieldDesc(Dev, FieldId);
	if (Field == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfuseOpsValidateFieldWrLk(Field);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((Dev->Ops != NULL) && (Dev->Ops->ValidateField != NULL)) {
		Status = Dev->Ops->ValidateField(Field);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	Status = XST_SUCCESS;

	if (Field->ValidateFlags != XNVM_EFUSE_VALIDATE_NONE) {
		if ((Field->ValidateFlags & XNVM_EFUSE_VALIDATE_CACHE_EMPTY) != XIL_UNSIGNED_ZERO) {
			Status = XNvm_EfuseOpsValidateCacheEmptyField(Field, FieldId);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}

		if ((Field->ValidateFlags & XNVM_EFUSE_VALIDATE_AES_KEY) != XIL_UNSIGNED_ZERO) {
			XNvm_AesKeyType AesKeyType;

			Status = XNvm_EfuseOpsFieldToAesKeyType(FieldId, &AesKeyType);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			Status = XNvm_EfuseValidateAesKeyWriteReq(AesKeyType);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}

		if (((Field->ValidateFlags & XNVM_EFUSE_VALIDATE_FIPS_MODE) != XIL_UNSIGNED_ZERO) ||
				((Field->ValidateFlags & XNVM_EFUSE_VALIDATE_FIPS_VERSION) !=
				 XIL_UNSIGNED_ZERO)) {
			Status = XNvm_EfuseOpsValidateFipsField(Field, Data, NumWords);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}

		if ((Field->ValidateFlags & XNVM_EFUSE_VALIDATE_NO_BIT_REVERT) !=
				XIL_UNSIGNED_ZERO) {
			Status = XNvm_EfuseOpsValidateIvField(FieldId, Data, NumWords);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}

		if ((Field->ValidateFlags & XNVM_EFUSE_VALIDATE_CRC_EMPTY) != XIL_UNSIGNED_ZERO) {
			Status = XNvm_EfuseOpsValidateCrcField();
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}

		if ((Field->ValidateFlags & XNVM_EFUSE_VALIDATE_DME_KEY) != XIL_UNSIGNED_ZERO) {
			Status = XNvm_EfuseOpsValidateDmeKeyField(Dev, FieldId);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}

		if ((Field->ValidateFlags & XNVM_EFUSE_VALIDATE_PUF_HELPER_EMPTY) != XIL_UNSIGNED_ZERO) {
			Status = XNvm_EfuseIsPufHelperDataEmpty();
			if (Status != XST_SUCCESS) {
				Status = (Status | (int)XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
				goto END;
			}
		}

#ifndef XNVM_ACCESS_PUF_USER_DATA
		if ((Field->ValidateFlags & XNVM_EFUSE_VALIDATE_PUF_HD_ACCESS) != XIL_UNSIGNED_ZERO) {
			Status = XNvm_EfuseOpsValidatePufHdAccess();
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}
#endif
	}

	/* DEC_ONLY field: validate write prerequisites (AES/PUF, not already blown). */
	if (Field->PrecondId == XNVM_EFUSE_PRECOND_DEC_ONLY) {
		Status = XNvm_EfuseValidateDecOnlyRequest();
	}

END:
	return Status;
}

#ifndef XNVM_ACCESS_PUF_USER_DATA
/******************************************************************************/
/**
 * @brief	Validates that PUF helper-data programming is still allowed.
 *
 * @return
 *		- XST_SUCCESS when PUF helper data is not disabled or locked.
 *		- XNVM_EFUSE_ERR_FUSE_PROTECTED OR'd with write helper-data error.
 *
 ******************************************************************************/
int XNvm_EfuseOpsValidatePufHdAccess(void)
{
	volatile int Status = XST_FAILURE;
	u32 PufSecurityCtrlReg = 0xFFFFFFFFU;

	PufSecurityCtrlReg = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
			XNVM_EFUSE_CACHE_SECURITY_CONTROL_OFFSET);
	if ((PufSecurityCtrlReg &
			(XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_DIS_MASK |
			XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_SYN_LK_MASK)) != XIL_UNSIGNED_ZERO) {
		Status = (int)(XNVM_EFUSE_ERR_FUSE_PROTECTED |
				XNVM_EFUSE_ERR_WRITE_PUF_HELPER_DATA);
	} else {
		Status = XST_SUCCESS;
	}

	return Status;
}
#endif /* XNVM_ACCESS_PUF_USER_DATA */

/******************************************************************************/
/**
 * @brief	Reverses byte order of a buffer in place into Dest from Src.
 *
 * @param	Dest - Output buffer (Size bytes).
 * @param	Src - Input buffer (Size bytes).
 * @param	Size - Number of bytes to reverse.
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM if Dest or Src is NULL.
 *
 * @note	Used to convert caller key word order to fuse row byte order
 *		(e.g. DME user keys before band-engine programming).
 *
 ******************************************************************************/
int XNvm_EfuseOpsChangeEndianness(u8 *Dest, u8 *Src, u32 Size)
{
	u32 Index = XIL_UNSIGNED_ZERO;
	u32 ReverseIndex = XIL_UNSIGNED_ZERO;
	int Status = XST_FAILURE;

	if ((Dest == NULL) || (Src == NULL)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
	} else {
		for (Index = XIL_UNSIGNED_ZERO; Index < Size; Index++) {
			ReverseIndex = (Size - 1U) - Index;
			Dest[Index] = Src[ReverseIndex];
		}
		Status = XST_SUCCESS;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	Binds server PostProgram hook into the platform device ops table.
 *
 ******************************************************************************/
void XNvm_EfuseOpsBindHooks(void)
{
	if (XNvm_EfuseDeviceDesc.Ops != NULL) {
		XNvm_EfuseDeviceDesc.Ops->PostProgram = XNvm_EfuseOpsPostProgram;
	}
}

/******************************************************************************/
/**
 * @brief	Reloads the eFuse cache and runs platform protection checks.
 *
 * @return
 *		- XST_SUCCESS when cache reload and checks pass.
 *		- Cache reload or protection-check error on failure.
 *
 ******************************************************************************/
int XNvm_EfuseOpsCacheOperations(void)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_EfuseCacheReload();
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		goto END;
	}

	Status = XST_FAILURE;
	if ((XNvm_EfuseDeviceDesc.Ops != NULL) &&
			(XNvm_EfuseDeviceDesc.Ops->ProtectionChecks != NULL)) {
		Status = XNvm_EfuseDeviceDesc.Ops->ProtectionChecks();
	} else {
		Status = XNvm_EfuseOpsProtectionChecks();
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	Verifies the Xilinx manufacturing CRC against the eFuse cache.
 *
 * @return
 *		- XST_SUCCESS if computed CRC matches the stored fuse value.
 *		- XST_FAILURE on CRC mismatch.
 *
 ******************************************************************************/
int XNvm_EfuseOpsValidateCrc(void)
{
	volatile int Status = XST_FAILURE;
	volatile u32 CalCrcVal = ~XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_CRC_OFFSET);
	volatile u32 EfuseCrcVal = XIL_UNSIGNED_ZERO;
	volatile u32 EfuseCrcValTmp = XIL_UNSIGNED_ZERO;
	u32 InputData[XNVM_EFUSE_CRC_INPUT_COUNT] = {XIL_UNSIGNED_ZERO};
	u32 Index = XIL_UNSIGNED_ZERO;
	u32 CacheRegVal = XIL_UNSIGNED_ZERO;

	for (Index = XIL_UNSIGNED_ZERO; Index < XNVM_EFUSE_CRC_INPUT_COUNT; Index++) {
		CacheRegVal = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNvm_EfuseCrcInputOffsets[Index]);
		if (Index == XNVM_EFUSE_CRC_MASKED_IDX) {
			CacheRegVal &= XNVM_EFUSE_CRC_MASKED_MASK;
		}
		InputData[Index] = CacheRegVal;
	}

	CalCrcVal = XNvm_EfuseOpsCalculateCrc32((u8 *)InputData,
			XNVM_EFUSE_CRC_DATA_BYTE_COUNT);

	Status = (int)XST_FAILURE;

	EfuseCrcVal = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
			XNVM_EFUSE_CACHE_CRC_OFFSET);
	EfuseCrcValTmp = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
			XNVM_EFUSE_CACHE_CRC_OFFSET);
	if ((CalCrcVal != EfuseCrcVal) || (CalCrcVal != EfuseCrcValTmp)) {
		Status = (int)XST_FAILURE;
	}
	else {
		Status = XST_SUCCESS;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	Checks that band cache regions are empty before programming.
 *
 * Walks every band with a non-zero cache span (legacy XNvm_EfuseCheckZeros
 * behaviour for PPK lower hash cache and 2VE upper user-fuse cache).
 * Applies to COMPUTE_PRG and DIRECT_WRITE bands that declare cache offsets.
 *
 * @param	Field - Field whose band cache regions are checked.
 * @param	ErrAlreadyPrgmd - Error to return when a region is non-zero.
 *
 * @return
 *		- XST_SUCCESS when all relevant cache regions are zero.
 *		- ErrAlreadyPrgmd on non-zero cache data.
 *
 ******************************************************************************/
static int XNvm_EfuseOpsValidateCacheEmptyBands(const XNvm_EfuseFieldDesc *Field,
		u32 ErrAlreadyPrgmd)
{
	volatile int Status = XST_FAILURE;
	u32 BandIdx = XIL_UNSIGNED_ZERO;
	const XNvm_EfuseBand *Band = NULL;
	const XNvm_EfuseBand *Bands = NULL;
	u32 NumRows = XIL_UNSIGNED_ZERO;

	if (Field->NumBands == XIL_UNSIGNED_ZERO) {
		Status = XST_SUCCESS;
		goto END;
	}

	Bands = Field->Bands;
	if (Bands == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	for (BandIdx = XIL_UNSIGNED_ZERO; BandIdx < Field->NumBands; BandIdx++) {
		Band = &Bands[BandIdx];

		if ((Band->CacheEndOffset >= Band->CacheStartOffset) &&
				((Band->CacheStartOffset != XIL_UNSIGNED_ZERO) ||
				 (Band->CacheEndOffset != XIL_UNSIGNED_ZERO))) {
			NumRows = ((Band->CacheEndOffset - Band->CacheStartOffset) /
					XNVM_WORD_LEN) + 1U;
			Status = XNvm_EfuseCheckZeros(Band->CacheStartOffset, NumRows);
			if (Status != XST_SUCCESS) {
				Status = (int)ErrAlreadyPrgmd;
				goto END;
			}
		}
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	Validates cache-empty policy for PPK hash or band-cache fields.
 *
 * @param	Field - Field descriptor being programmed.
 * @param	FieldId - Field identifier.
 *
 * @return
 *		- XST_SUCCESS when cache-empty checks pass.
 *		- Field-specific validation error on failure.
 *
 ******************************************************************************/
static int XNvm_EfuseOpsValidateCacheEmptyField(const XNvm_EfuseFieldDesc *Field,
		XNvm_EfuseFieldId FieldId)
{
	int Status = XST_FAILURE;
	XNvm_PpkType PpkType;

	if ((FieldId >= XNVM_EFUSE_FIELD_PPK0_HASH) &&
			(FieldId <= XNVM_EFUSE_FIELD_PPK_HASH_MAX)) {
		PpkType = XNVM_EFUSE_PPK0 +
			(XNvm_PpkType)(FieldId - XNVM_EFUSE_FIELD_PPK0_HASH);
		Status = XNvm_EfuseValidatePpkHashWriteReq(PpkType);
	} else {
		Status = XNvm_EfuseOpsValidateCacheEmptyBands(Field,
				(u32)XNvm_EfuseOpsGetCacheEmptyErr((u32)FieldId));
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	Validates FIPS mode and/or version write data.
 *
 * @param	Field - Field descriptor with FIPS validate flags.
 * @param	Data - Source words for the field write.
 * @param	NumWords - Number of source words supplied.
 *
 * @return
 *		- XST_SUCCESS when FIPS validation passes or is not required.
 *		- FIPS validation error on failure.
 *
 ******************************************************************************/
static int XNvm_EfuseOpsValidateFipsField(const XNvm_EfuseFieldDesc *Field,
		const u32 *Data, u32 NumWords)
{
	u32 FipsMode = XIL_UNSIGNED_ZERO;
	u32 FipsVersion = XIL_UNSIGNED_ZERO;
	int Status = XST_FAILURE;

	if ((Data == NULL) || (NumWords < XIL_UNSIGNED_ONE)) {
		Status = XST_SUCCESS;
	} else {
		if ((Field->ValidateFlags & XNVM_EFUSE_VALIDATE_FIPS_MODE) != XIL_UNSIGNED_ZERO) {
			FipsMode = Data[0];
		}
		if ((Field->ValidateFlags & XNVM_EFUSE_VALIDATE_FIPS_VERSION) != XIL_UNSIGNED_ZERO) {
			FipsVersion = Data[0];
		}
		Status = XNvm_EfuseValidateFipsInfo(FipsMode, FipsVersion);
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	Validates IV no-bit-revert policy for an IV-range field.
 *
 * @param	FieldId - IV field identifier.
 * @param	Data - IV words to validate.
 * @param	NumWords - Number of source words supplied.
 *
 * @return
 *		- XST_SUCCESS when IV validation passes.
 *		- Invalid-param or IV validation error on failure.
 *
 ******************************************************************************/
static int XNvm_EfuseOpsValidateIvField(XNvm_EfuseFieldId FieldId,
		const u32 *Data, u32 NumWords)
{
	XNvm_IvType IvType;
	XNvm_Iv EfuseIv;
	int Status = XST_FAILURE;

	if ((Data == NULL) || (NumWords < XNVM_EFUSE_IV_NUM_OF_CACHE_ROWS)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	IvType = XNVM_EFUSE_META_HEADER_IV_RANGE +
		(XNvm_IvType)(FieldId - XNVM_EFUSE_FIELD_IV_META_HEADER);
	Status = Xil_SMemCpy(EfuseIv.Iv, sizeof(EfuseIv.Iv), Data,
			(XNVM_EFUSE_IV_NUM_OF_CACHE_ROWS * sizeof(u32)),
			(XNVM_EFUSE_IV_NUM_OF_CACHE_ROWS * sizeof(u32)));
	if (Status != XST_SUCCESS) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfuseValidateIvWriteReq(IvType, &EfuseIv);

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	Checks that the manufacturing CRC cache slot is empty.
 *
 * @return
 *		- XST_SUCCESS when the CRC fuse row reads zero.
 *		- CRC write before-program error when non-zero.
 *
 ******************************************************************************/
static int XNvm_EfuseOpsValidateCrcField(void)
{
	u32 RegData = XIL_UNSIGNED_ZERO;
	volatile int Status = XST_FAILURE;

	RegData = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
			XNVM_EFUSE_CACHE_CRC_OFFSET);
	if (RegData != XIL_UNSIGNED_ZERO) {
		Status = (int)(XNVM_EFUSE_ERR_WRITE_CRC |
			XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
	} else {
		Status = XST_SUCCESS;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	Validates DME key write policy and cache emptiness.
 *
 * @param	Dev - Device table descriptor with DME key policy.
 * @param	FieldId - DME key field ID (KEY0..KEY3).
 *
 * @return
 *		- XST_SUCCESS when the key may be programmed.
 *		- DME mode, already-programmed, or invalid-param error.
 *
 ******************************************************************************/
static int XNvm_EfuseOpsValidateDmeKeyField(const XNvm_EfuseDevice *Dev,
		XNvm_EfuseFieldId FieldId)
{
	const XNvm_EfuseDmeKeyPolicy *Policy;
	u32 StartOffset = XIL_UNSIGNED_ZERO;
	u32 RdOffset = XIL_UNSIGNED_ZERO;
	u32 RegData = XIL_UNSIGNED_ZERO;
	volatile int Status = XST_FAILURE;

	if ((FieldId < XNVM_EFUSE_FIELD_DME_KEY0) ||
			(FieldId > XNVM_EFUSE_FIELD_DME_KEY3)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if ((Dev == NULL) || (Dev->DmeKeys == NULL)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Policy = Dev->DmeKeys;
	if ((FieldId - XNVM_EFUSE_FIELD_DME_KEY0) >= Policy->NumKeys) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (Policy->BlockIfDmeModeEn != XIL_UNSIGNED_ZERO) {
#ifndef VERSAL_2VE_2VM
		Status = XNvm_IsDmeModeEn();
		if (Status != XST_SUCCESS) {
			goto END;
		}
#endif
	}

	StartOffset = Policy->CacheOffsets[FieldId - XNVM_EFUSE_FIELD_DME_KEY0];
	RdOffset = StartOffset;
	while (RdOffset < (StartOffset +
			(XNVM_DME_USER_KEY_SIZE_IN_WORDS * XNVM_WORD_LEN))) {
		RegData = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR, RdOffset);
		if (RegData != XIL_UNSIGNED_ZERO) {
			Status = (int)XNVM_ERR_DME_KEY_ALREADY_PROGRAMMED;
			goto END;
		}
		RdOffset += XNVM_WORD_LEN;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	Maps a field ID to an already-programmed error code.
 *
 * @param	FieldId - Field whose cache-empty failure is being reported.
 *
 * @return	Field-specific already-programmed error, or generic
 *		XNVM_EFUSE_ERR_BEFORE_PROGRAMMING.
 *
 ******************************************************************************/
static int XNvm_EfuseOpsGetCacheEmptyErr(u32 FieldId)
{
	int Status = (u32)XNVM_EFUSE_ERR_BEFORE_PROGRAMMING;

	if ((FieldId >= (u32)XNVM_EFUSE_FIELD_PPK0_HASH) &&
			(FieldId <= (u32)XNVM_EFUSE_FIELD_PPK_HASH_MAX)) {
		Status = (int)((u32)XNVM_EFUSE_ERR_PPK0_HASH_ALREADY_PRGMD +
			((FieldId - (u32)XNVM_EFUSE_FIELD_PPK0_HASH) <<
			XNVM_EFUSE_ERROR_NIBBLE_SHIFT));
	} else if (FieldId == (u32)XNVM_EFUSE_FIELD_IV_BLACK) {
		Status = (u32)XNVM_EFUSE_ERR_BLK_OBFUS_IV_ALREADY_PRGMD;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	Post-program hook: reload cache, run protection checks, verify CRC.
 *
 * @param	FieldId - Field that was just programmed.
 * @param	Data - Pointer to programmed field data passed to WriteField.
 *
 * @return
 *		- XST_SUCCESS on success or for non-AES-class fields with no CRC step.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM if an AES-class key has no CRC descriptor.
 *		- Field-specific write error OR'd with CRC or protection failure.
 *
 ******************************************************************************/
static int XNvm_EfuseOpsPostProgram(XNvm_EfuseFieldId FieldId, const void *Data)
{
	const u32 *PrgmData = (const u32 *)Data;
	const XNvm_EfusePostProgDesc *Desc = NULL;
	volatile int Status = XST_FAILURE;
	XNvm_AesKeyType AesKeyType;
	u32 Crc = XIL_UNSIGNED_ZERO;
	u32 PostProgTblIdx = XIL_UNSIGNED_ZERO;

	for (PostProgTblIdx = XIL_UNSIGNED_ZERO; PostProgTblIdx < (sizeof(XNvm_EfusePostProgTbl) /
			sizeof(XNvm_EfusePostProgTbl[0])); PostProgTblIdx++) {
		if (XNvm_EfusePostProgTbl[PostProgTblIdx].FieldId == FieldId) {
			Desc = &XNvm_EfusePostProgTbl[PostProgTblIdx];
			break;
		}
	}
	if (Desc == NULL) {
		/*
		 * PPK/IV/etc. have no CRC step. An AES-class key without a
		 * descriptor would skip cache reload and CRC; reject it.
		 */
		if (XNvm_EfuseOpsFieldToAesKeyType(FieldId, &AesKeyType) ==
				XST_SUCCESS) {
			Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		} else {
			Status = XST_SUCCESS;
		}
		goto END;
	}

	Status = XNvm_EfuseOpsCacheOperations();
	if (Status != XST_SUCCESS) {
		Status = (int)((u32)Status | Desc->ErrWrite);
		goto END;
	}

	if (FieldId == XNVM_EFUSE_FIELD_UDS) {
		Crc = XNvm_EfuseOpsCrcCalc(PrgmData, Desc->CrcWordCount);
	} else {
		Crc = XNvm_AesCrcCalc(PrgmData);
	}

	Status = XNvm_EfuseCheckAesKeyCrc(Desc->CrcRegOffset, Desc->CrcDoneMask,
			Desc->CrcPassMask, Crc);
	if (Status != XST_SUCCESS) {
		Status = (int)((u32)Status | Desc->ErrWrite);
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	Verifies read consistency over a cache region and ensures at least
 *		one row is programmed (non-zero).
 *
 * @param	StartOffset - First cache byte offset in the region.
 * @param	NumRows - Number of 32-bit rows to check.
 *
 * @return
 *		- XST_SUCCESS when reads are consistent and at least one row is
 *		  non-zero.
 *		- XNVM_EFUSE_ERR_IN_PROTECTION_CHECK on read mismatch or if every
 *		  row is zero.
 *
 * @note	Scans from StartOffset; stops at the first non-zero row (legacy
 *		protection-check behaviour for PPK hash and meta-header IV).
 *
 ******************************************************************************/
static int XNvm_EfuseOpsCheckCacheNonZero(u32 StartOffset, u32 NumRows)
{
	volatile int Status = XST_FAILURE;
	volatile u32 RowVal = XIL_UNSIGNED_ZERO;
	volatile u32 RowValTmp = XIL_UNSIGNED_ZERO;
	u32 Index = XIL_UNSIGNED_ZERO;
	u32 Offset = XIL_UNSIGNED_ZERO;

	for (Index = XIL_UNSIGNED_ZERO; Index < NumRows; Index++) {
		Offset = StartOffset + (Index * XNVM_WORD_LEN);

		RowVal = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR, Offset);
		RowValTmp = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR, Offset);
		if ((RowVal != RowValTmp) ||
				((RowVal != XIL_UNSIGNED_ZERO) ||
				 (RowValTmp != XIL_UNSIGNED_ZERO))) {
			break;
		}
	}

	if ((RowVal != RowValTmp) || (RowVal == XIL_UNSIGNED_ZERO)) {
		Status = (int)XNVM_EFUSE_ERR_IN_PROTECTION_CHECK;
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	Runs platform protection checks after cache reload.
 *
 * @return
 *		- XST_SUCCESS when all active protection rules pass.
 *		- XNVM_EFUSE_ERR_ANCHOR_BIT_PATTERN on bad anchor pattern.
 *		- XNVM_EFUSE_ERR_IN_PROTECTION_CHECK on protected-data mismatch.
 *		- XNVM_EFUSE_XLNX_MANFAC_BITS_CRC_ERROR on CRC failure.
 *
 ******************************************************************************/
static int XNvm_EfuseOpsProtectionChecks(void)
{
	volatile int Status;
	volatile u32 RegVal = XIL_UNSIGNED_ZERO;
	volatile u32 RegValTmp = XIL_UNSIGNED_ZERO;
	volatile u32 ProtVal = XIL_UNSIGNED_ZERO;
	volatile u32 ProtValTmp = XIL_UNSIGNED_ZERO;
	volatile u32 RowVal = XIL_UNSIGNED_ZERO;
	volatile u32 RowValTmp = XIL_UNSIGNED_ZERO;

	Status = XST_FAILURE;
	RegVal = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
			XNVM_EFUSE_CACHE_TBITS0_SVD_OFFSET);
	RegValTmp = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
			XNVM_EFUSE_CACHE_TBITS0_SVD_OFFSET);

	ProtVal = RegVal & (XNVM_EFUSE_CACHE_TBITS0_SVD_ANCHOR_3_MASK |
				XNVM_EFUSE_CACHE_TBITS0_SVD_ANCHOR_2_MASK |
				XNVM_EFUSE_CACHE_TBITS0_SVD_ANCHOR_1_MASK |
				XNVM_EFUSE_CACHE_TBITS0_SVD_ANCHOR_0_MASK);
	ProtValTmp = RegValTmp & (XNVM_EFUSE_CACHE_TBITS0_SVD_ANCHOR_3_MASK |
				XNVM_EFUSE_CACHE_TBITS0_SVD_ANCHOR_2_MASK |
				XNVM_EFUSE_CACHE_TBITS0_SVD_ANCHOR_1_MASK |
				XNVM_EFUSE_CACHE_TBITS0_SVD_ANCHOR_0_MASK);

	if ((ProtVal != (XNVM_EFUSE_CACHE_TBITS0_SVD_ANCHOR_3_MASK |
			XNVM_EFUSE_CACHE_TBITS0_SVD_ANCHOR_1_MASK)) ||
		(ProtValTmp != (XNVM_EFUSE_CACHE_TBITS0_SVD_ANCHOR_3_MASK |
				XNVM_EFUSE_CACHE_TBITS0_SVD_ANCHOR_1_MASK))) {

		Status = (int)XNVM_EFUSE_ERR_ANCHOR_BIT_PATTERN;
		goto END;
	}

	ProtVal = RegVal & (XNVM_EFUSE_CACHE_TBITS0_SVD_SEC_CTRL_PROT_MASK);
	ProtValTmp = RegValTmp & (XNVM_EFUSE_CACHE_TBITS0_SVD_SEC_CTRL_PROT_MASK);

	if((ProtVal != XIL_UNSIGNED_ZERO) || (ProtValTmp != XIL_UNSIGNED_ZERO)) {
		RowVal = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_SECURITY_CONTROL_OFFSET);
		RowValTmp = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_SECURITY_CONTROL_OFFSET);

		if ((RowVal != RowValTmp) || (RowVal == XIL_UNSIGNED_ZERO)) {
			Status = (int)XNVM_EFUSE_ERR_IN_PROTECTION_CHECK;
			goto END;
		}
	}

	ProtVal = RegVal & (XNVM_EFUSE_CACHE_TBITS0_SVD_SEC_MISC0_PROT_MASK);
	ProtValTmp = RegValTmp & (XNVM_EFUSE_CACHE_TBITS0_SVD_SEC_MISC0_PROT_MASK);

	if((ProtVal != XIL_UNSIGNED_ZERO) || (ProtValTmp != XIL_UNSIGNED_ZERO)) {
		RowVal = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
			XNVM_EFUSE_CACHE_SECURITY_MISC_0_OFFSET) &
			XNVM_EFUSE_CACHE_DEC_EFUSE_ONLY_MASK;
		RowValTmp = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
			XNVM_EFUSE_CACHE_SECURITY_MISC_0_OFFSET) &
			XNVM_EFUSE_CACHE_DEC_EFUSE_ONLY_MASK;

		if ((RowVal != RowValTmp) || (RowVal !=
			XNVM_EFUSE_CACHE_DEC_EFUSE_ONLY_MASK)) {
			Status = (int)XNVM_EFUSE_ERR_IN_PROTECTION_CHECK;
			goto END;
		}
	}

	ProtVal = RegVal  & (XNVM_EFUSE_CACHE_TBITS0_SVD_META_HEADER_EXPORT_PROT_MASK);
	ProtValTmp = RegValTmp & (XNVM_EFUSE_CACHE_TBITS0_SVD_META_HEADER_EXPORT_PROT_MASK);

	if((ProtVal != XIL_UNSIGNED_ZERO) || (ProtValTmp != XIL_UNSIGNED_ZERO)) {
		Status = XNvm_EfuseOpsCheckCacheNonZero(
				XNVM_EFUSE_CACHE_METAHEADER_IV_RANGE_0_OFFSET,
				XNVM_EFUSE_IV_NUM_OF_CACHE_ROWS);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	ProtVal = RegVal & XNVM_EFUSE_CACHE_TBITS0_SVD_CRC_PROT_MASK;
	ProtValTmp = RegValTmp & XNVM_EFUSE_CACHE_TBITS0_SVD_CRC_PROT_MASK;

	if((ProtVal != XIL_UNSIGNED_ZERO) || (ProtValTmp != XIL_UNSIGNED_ZERO)) {
		RowVal = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_ME_ID_CODE_OFFSET) &
				XNVM_EFUSE_CACHE_ME_ID_CODE_CRC_SALT_MASK;
		RowValTmp = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_ME_ID_CODE_OFFSET) &
				XNVM_EFUSE_CACHE_ME_ID_CODE_CRC_SALT_MASK;

		if ((RowVal != XIL_UNSIGNED_ZERO) || (RowValTmp != XIL_UNSIGNED_ZERO)) {
			Status = XNvm_EfuseOpsValidateCrc();
			if (Status != XST_SUCCESS) {
				Status = (int)XNVM_EFUSE_XLNX_MANFAC_BITS_CRC_ERROR;
				goto END;
			}
		}
	}

	ProtVal = RegVal & XNVM_EFUSE_CACHE_TBITS0_SVD_PUF_CHASH_PROT_MASK;
	ProtValTmp = RegValTmp & XNVM_EFUSE_CACHE_TBITS0_SVD_PUF_CHASH_PROT_MASK;

	if((ProtVal != XIL_UNSIGNED_ZERO) || (ProtValTmp != XIL_UNSIGNED_ZERO)) {
		RowVal = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_PUF_CHASH_OFFSET);
		RowValTmp = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_PUF_CHASH_OFFSET);

		if ((RowVal != RowValTmp) || (RowVal == XIL_UNSIGNED_ZERO)) {
			Status = (int)XNVM_EFUSE_ERR_IN_PROTECTION_CHECK;
			goto END;
		}
	}

	ProtVal = RegVal & XNVM_EFUSE_CACHE_TBITS0_SVD_SEC_MISC1_PROT_MASK;
	ProtValTmp = RegValTmp  & XNVM_EFUSE_CACHE_TBITS0_SVD_SEC_MISC1_PROT_MASK;

	if((ProtVal != XIL_UNSIGNED_ZERO) || (ProtValTmp != XIL_UNSIGNED_ZERO)) {
		RowVal = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_SECURITY_MISC_1_OFFSET);
		RowValTmp = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_SECURITY_MISC_1_OFFSET);

		if ((RowVal != RowValTmp) ||
			((RowVal & XNVM_EFUSE_SECURITY_MISC_1_PROT_MASK) == XIL_UNSIGNED_ZERO)) {
			Status = (int)XNVM_EFUSE_ERR_IN_PROTECTION_CHECK;
			goto END;
		}
	}

	if ((ProtVal == ProtValTmp) && (RowVal == RowValTmp)) {
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	Calculates the eFuse-controller CRC over a word buffer.
 *
 * @param	Data - Pointer to the word buffer to checksum.
 * @param	NumWords - Number of 32-bit words in Data.
 *
 * @return	32-bit CRC value.
 *
 ******************************************************************************/
u32 XNvm_EfuseOpsCrcCalc(const u32 *Data, u32 NumWords)
{
	u32 Crc = XIL_UNSIGNED_ZERO;
	u32 Value;
	u32 Idx;
	u32 BitNo;
	volatile u32 Temp1Crc;
	volatile u32 Temp2Crc;

	for (Idx = XIL_UNSIGNED_ZERO; Idx < NumWords; Idx++) {
		Value = Data[NumWords - Idx - XIL_UNSIGNED_ONE];
		for (BitNo = XIL_UNSIGNED_ZERO; BitNo < 32U; BitNo++) {
			Temp1Crc = Crc >> XIL_UNSIGNED_ONE;
			Temp2Crc = Temp1Crc ^ XNVM_EFUSE_REVERSE_POLYNOMIAL;
			if (((Value ^ Crc) & XNVM_EFUSE_BIT0_MASK) != XIL_UNSIGNED_ZERO) {
				Crc = Temp2Crc;
			} else {
				Crc = Temp1Crc;
			}
			Value = Value >> XIL_UNSIGNED_ONE;
		}

		Value = NumWords - (u32)Idx;
		for (BitNo = XIL_UNSIGNED_ZERO; BitNo < 5U; BitNo++) {
			Temp1Crc = Crc >> XIL_UNSIGNED_ONE;
			Temp2Crc = Temp1Crc ^ XNVM_EFUSE_REVERSE_POLYNOMIAL;
			if (((Value ^ Crc) & XNVM_EFUSE_BIT0_MASK) != XIL_UNSIGNED_ZERO) {
				Crc = Temp2Crc;
			} else {
				Crc = Temp1Crc;
			}
			Value = Value >> XIL_UNSIGNED_ONE;
		}
	}

	return Crc;
}

/******************************************************************************/
/**
 * @brief	Calculates a 32-bit CRC over input data for manufacturing check.
 *
 * @param	Data - Input byte buffer.
 * @param	Len - Number of bytes to include in the CRC.
 *
 * @return	Computed 32-bit CRC value.
 *
 ******************************************************************************/
static u32 XNvm_EfuseOpsCalculateCrc32(const u8 *Data, u32 Len)
{
	u32 Crc = XNVM_EFUSE_SEC_DEF_VAL_ALL_BIT_SET;
	u32 Index = XIL_UNSIGNED_ZERO;
	u32 BitIdx = XIL_UNSIGNED_ZERO;

	for (Index = XIL_UNSIGNED_ZERO; Index < Len; Index++) {
		Crc ^= (u32)Data[Index];
		for (BitIdx = XIL_UNSIGNED_ZERO; BitIdx < XNVM_EFUSE_BITS_IN_A_BYTE;
				BitIdx++) {
			if ((Crc & XNVM_EFUSE_BIT0_MASK) != XIL_UNSIGNED_ZERO) {
				Crc = (Crc >> XIL_UNSIGNED_ONE) ^
					XNVM_EFUSE_CRC32_POLY;
			} else {
				Crc >>= XIL_UNSIGNED_ONE;
			}
		}
	}

	return (Crc ^ XNVM_EFUSE_SEC_DEF_VAL_ALL_BIT_SET);
}
/** @} */
