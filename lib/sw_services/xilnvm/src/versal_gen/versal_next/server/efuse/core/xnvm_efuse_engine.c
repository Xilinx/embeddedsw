/******************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/******************************************************************************/
/**
*
* @file versal_next/server/efuse/core/xnvm_efuse_engine.c
*
* Band-engine implementation (field/band programming).
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
#include "xil_util.h"
#include "xnvm_efuse_engine.h"
#include "xnvm_efuse_hw.h"
#include "xnvm_efuse_common_hw.h"
#include "xnvm_efuse_plat_field.h"
#include "xnvm_efuse_error.h"
#include "xnvm_efuse_common.h"
#include "xnvm_ops.h"
#include "xnvm_validate.h"
#include "xnvm_utils.h"
#include "xnvm_defs.h"

/************************** Constant Definitions *****************************/
/** - Number of bits in one byte (efuse bit indexing). */
#define XNVM_EFUSE_BITS_IN_A_BYTE		(8U)

/************************** Function Prototypes *****************************/
static int XNvm_EfuseEngineProgramFieldBands(const XNvm_EfuseFieldDesc *Field,
		const u32 *Data, u32 NumWords);
static int XNvm_EfuseEngineCheckData(const u32 *Buf, u32 NumWords);
static int XNvm_EfuseEngineEnvCheck(u32 EnvDisFlag);
static int XNvm_EfuseEngineValidateWriteData(const XNvm_EfuseFieldDesc *Field,
		const u32 *Data, u32 NumWords);
static int XNvm_EfuseEngineProtTriggered(const XNvm_EfuseProtEntry *Entry);
static u32 XNvm_EfuseEngineBandCacheWord(const XNvm_EfuseBand *Band,
	u32 CacheWord);

/************************** Function Definitions *****************************/


/******************************************************************************/
/**
 * @brief	Opens the controller for programming (legacy order).
 *
 * @param	EnvDisFlag - When TRUE, skip environmental monitoring.
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- Environmental or setup error on failure.
 *
 * @note	Legacy flow: environmental checks, then unlock/setup controller.
 *
 ******************************************************************************/
int XNvm_EfuseEngineOpen(u32 EnvDisFlag)
{
	int Status = XST_FAILURE;

	XNvm_EfuseOpsBindHooks();

	Status = XNvm_EfuseEngineEnvCheck(EnvDisFlag);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
			XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	Closes the eFuse controller, preserving a prior error status.
 *
 * @param	Status - Status accumulated before close; close errors are
 *		OR'd in only when Status is XST_SUCCESS.
 *
 * @return	Combined status from programming and close steps.
 *
 ******************************************************************************/
int XNvm_EfuseEngineClose(int Status)
{
	int CloseStatus = XNvm_EfuseCloseController();

	if (Status == XST_SUCCESS) {
		Status |= CloseStatus;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	Programs all bands of a field through the band engine.
 *
 * @param	Dev - Device table descriptor.
 * @param	FieldId - Field to program.
 * @param	EnvDisFlag - When TRUE, skip temperature/voltage checks.
 * @param	Data - Source data words for the field bands.
 * @param	NumWords - Number of valid words in Data.
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- Field-specific before-program or write error on failure.
 *
 ******************************************************************************/
int XNvm_EfuseEngineWriteField(const XNvm_EfuseDevice *Dev,
		XNvm_EfuseFieldId FieldId, u32 EnvDisFlag, const u32 *Data,
		u32 NumWords)
{
	volatile int Status = XST_FAILURE;
	const XNvm_EfuseFieldDesc *Field;

	Field = XNvm_EfuseLookupFieldDesc(Dev, FieldId);
	if (Field == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfuseEngineValidateWriteData(Field, Data, NumWords);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseOpsValidateWriteField(Dev, FieldId, Data, NumWords);
	if (Status != XST_SUCCESS) {
		if ((Status & (int)Field->ErrBeforePgm) == XIL_UNSIGNED_ZERO) {
			Status = (Status | (int)Field->ErrBeforePgm);
		}
		goto END;
	}

	Status = XNvm_EfuseEngineOpen(EnvDisFlag);
	if (Status != XST_SUCCESS) {
		goto END_CLOSE;
	}

	Status = XNvm_EfuseEngineProgramFieldBands(Field, Data, NumWords);
	if (Status == (int)XNVM_EFUSE_ERR_NTHG_TO_BE_PROGRAMMED) {
		Status = XST_SUCCESS;
	}
	if (Status != XST_SUCCESS) {
		goto END_CLOSE;
	}

	if ((Dev->Ops != NULL) && (Dev->Ops->PostProgram != NULL)) {
		Status = Dev->Ops->PostProgram(FieldId, Data);
		if (Status != XST_SUCCESS) {
			Status = (Status | Field->ErrWrite);
		}
	}

END_CLOSE:
	Status = XNvm_EfuseEngineClose(Status);

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	Programs protection bits when their trigger conditions match.
 *
 * @param	Dev - Device table descriptor.
 *
 * @return
 *		- XST_SUCCESS on success, including an explicit empty table
 *		  (NO_PROTECTION flag with Protection == NULL and NumProtection == 0).
 *		- XNVM_EFUSE_ERR_INVALID_PARAM if Dev is NULL, the table pointer
 *		  and count disagree, or the table is missing without NO_PROTECTION.
 *		- Protection-specific error on program failure.
 *
 ******************************************************************************/
int XNvm_EfuseEnginePrgmProtectionBits(const XNvm_EfuseDevice *Dev)
{
	volatile int Status = XST_FAILURE;
	int TriggerStatus = XST_FAILURE;
	u32 Index = XIL_UNSIGNED_ZERO;
	u32 ProtMask = XIL_UNSIGNED_ZERO;
	const XNvm_EfuseProtEntry *Entry;

	if (Dev == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (((Dev->NumProtection == XIL_UNSIGNED_ZERO) && (Dev->Protection != NULL)) ||
			((Dev->NumProtection != XIL_UNSIGNED_ZERO) &&
			 (Dev->Protection == NULL))) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (Dev->NumProtection == XIL_UNSIGNED_ZERO) {
		if ((Dev->Flags & XNVM_EFUSE_DEVICE_FLAG_NO_PROTECTION) !=
				XIL_UNSIGNED_ZERO) {
			Status = XST_SUCCESS;
		}
		else {
			Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		}
		goto END;
	}

	for (Index = XIL_UNSIGNED_ZERO; Index < Dev->NumProtection; Index++) {
		Entry = &Dev->Protection[Index];
		TriggerStatus = XNvm_EfuseEngineProtTriggered(Entry);
		if (TriggerStatus == XST_FAILURE) {
			continue;
		}
		if (TriggerStatus != XST_SUCCESS) {
			Status = TriggerStatus;
			goto END;
		}

		ProtMask = XNvm_EfuseReadProtectionBits(Entry->ProtReadMask);
		if (ProtMask != XNVM_EFUSE_PROTECTION_BIT_CLEAR) {
			Status = XST_SUCCESS;
			continue;
		}

		Status = XNvm_EfusePgmAndVerifyBit((u32)Entry->Page,
				Entry->ProtRow, Entry->ProtCol0,
				XNVM_EFUSE_PROGRAM_VERIFY);
		if (Status != XST_SUCCESS) {
			Status = (Status | Entry->ErrProt0);
			goto END;
		}

		if (Entry->ProtCol1 != XNVM_EFUSE_INVALID_COL) {
			Status = XST_FAILURE;
			Status = XNvm_EfusePgmAndVerifyBit((u32)Entry->Page,
					Entry->ProtRow, Entry->ProtCol1,
					XNVM_EFUSE_PROGRAM_VERIFY);
			if (Status != XST_SUCCESS) {
				Status = (Status | Entry->ErrProt1);
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
 * @brief	Maps cache-format DIRECT_WRITE data to fuse row LSB-first word(s).
 *
 * @param	Band - Band geometry (ColStart..ColEnd).
 * @param	CacheWord - Request data in eFuse cache bit positions.
 *
 * @return	Fuse row data for PgmAndVerifyData LSB-first column walk.
 *
 ******************************************************************************/
static u32 XNvm_EfuseEngineBandCacheWord(const XNvm_EfuseBand *Band,
		u32 CacheWord)
{
	u32 FuseWord = XIL_UNSIGNED_ZERO;
	u32 Col = Band->ColStart;
	u32 BitIdx = XIL_UNSIGNED_ZERO;

	while (Col <= Band->ColEnd) {
		if ((CacheWord >> Col) & XIL_UNSIGNED_ONE) {
			FuseWord |= (XIL_UNSIGNED_ONE << BitIdx);
		}
		BitIdx++;
		Col++;
	}

	return FuseWord;
}


/******************************************************************************/
/**
 * @brief	Checks whether a program buffer contains any bits to blow.
 *
 * @param	Buf - Buffer produced by ComputeProgrammableBits().
 * @param	NumWords - Number of valid words in Buf.
 *
 * @return
 *		- XST_SUCCESS when at least one word is non-zero.
 *		- XNVM_EFUSE_ERR_NTHG_TO_BE_PROGRAMMED when all words are zero.
 *
 ******************************************************************************/
static int XNvm_EfuseEngineCheckData(const u32 *Buf, u32 NumWords)
{
	int Status = XNVM_EFUSE_ERR_NTHG_TO_BE_PROGRAMMED;
	u32 Idx = XIL_UNSIGNED_ZERO;

	for (Idx = XIL_UNSIGNED_ZERO; Idx < NumWords; Idx++) {
		if (Buf[Idx] != XIL_UNSIGNED_ZERO) {
			Status = XST_SUCCESS;
			break;
		}
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	Runs temperature and voltage checks before programming.
 *
 * @param	EnvDisFlag - When TRUE, skip environmental monitoring.
 *
 * @return
 *		- XST_SUCCESS when checks pass or are disabled.
 *		- Environmental error OR'd with
 *		  XNVM_EFUSE_ERR_BEFORE_PROGRAMMING on failure.
 *
 ******************************************************************************/
static int XNvm_EfuseEngineEnvCheck(u32 EnvDisFlag)
{
	int Status = XST_FAILURE;

	if (EnvDisFlag != TRUE) {
		Status = XNvm_EfuseTempAndVoltChecks(XPlmi_GetSysmonInst());
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		}
	} else {
		Status = XST_SUCCESS;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	Checks caller data size against field MinSrcWords from the table.
 *
 * @param	Field - Field descriptor with precomputed MinSrcWords.
 * @param	Data - Caller source buffer (may be NULL when MinSrcWords is 0).
 * @param	NumWords - Number of valid words in Data.
 *
 * @return
 *		- XST_SUCCESS when NumWords is sufficient.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM on NULL data or short buffer.
 *
 ******************************************************************************/
static int XNvm_EfuseEngineValidateWriteData(const XNvm_EfuseFieldDesc *Field,
		const u32 *Data, u32 NumWords)
{
	int Status = XST_FAILURE;

	if (Field == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (Field->NumBands != XIL_UNSIGNED_ZERO) {
		if (Field->Bands == NULL) {
			Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
			goto END;
		}
	}

	if ((Field->MinSrcWords != XIL_UNSIGNED_ZERO) &&
			((Data == NULL) || (NumWords < Field->MinSrcWords))) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	Programs one band of a field through the controller.
 *
 * @param	Band - Band descriptor to program.
 * @param	Data - Caller source data words (required for COMPUTE_PRG and
 *		  DIRECT_WRITE bands; may be NULL for FIXED_DATA bands).
 * @param	NumWords - Number of valid u32 words at Data.
 *
 * @return
 *		- XST_SUCCESS on success or when the band has nothing to program.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM on bad input or band flags.
 *		- Controller program error on failure.
 *
 ******************************************************************************/
int XNvm_EfuseEngineProgramBand(const XNvm_EfuseBand *Band,
		const u32 *Data, u32 NumWords)
{
	int Status = XST_FAILURE;
	XNvm_EfusePrgmInfo PrgmInfo = {XIL_UNSIGNED_ZERO};
	u32 PrgmBuf[XNVM_EFUSE_PRGM_BUF_WORDS] = {XIL_UNSIGNED_ZERO};
	const u32 *BandSrcData = NULL;
	u32 RowDataNumWords = XIL_UNSIGNED_ZERO;
	u32 BandFlags = XIL_UNSIGNED_ZERO;
	u32 BandMode = XIL_UNSIGNED_ZERO;
	u32 CacheWords = XIL_UNSIGNED_ZERO;

	if (Band == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	BandFlags = Band->Flags;
	BandMode = BandFlags & XNVM_EFUSE_BAND_MODE_MASK;

	if ((Data == NULL) && (BandMode != XNVM_EFUSE_BAND_FLAG_FIXED_DATA)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	switch (BandMode) {
	case XNVM_EFUSE_BAND_FLAG_FIXED_DATA:
		PrgmBuf[0] = XIL_UNSIGNED_ZERO;
		if (Band->FixedData != XIL_UNSIGNED_ZERO) {
			if ((Band->NumRows == XNVM_EFUSE_BAND_SINGLE_ROW_COUNT) &&
					(Band->ColStart == Band->ColEnd)) {
				PrgmBuf[0] = XIL_UNSIGNED_ONE;
			} else {
				PrgmBuf[0] = Band->FixedData;
			}
		}
		BandSrcData = PrgmBuf;
		RowDataNumWords = XIL_UNSIGNED_ONE;
		break;
	case XNVM_EFUSE_BAND_FLAG_COMPUTE_PRG:
		CacheWords = ((Band->CacheEndOffset - Band->CacheStartOffset) /
				XNVM_WORD_LEN) + 1U;
		if (CacheWords > XNVM_EFUSE_PRGM_BUF_WORDS) {
			Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
			goto END;
		}
		if ((Band->SrcWordOffset + CacheWords) > NumWords) {
			Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
			goto END;
		}
		Status = XNvm_EfuseCacheReload();
		if (Status != XST_SUCCESS) {
			Status = (Status | (int)XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
			goto END;
		}
		Status = XNvm_EfuseComputeProgrammableBits(
				&Data[Band->SrcWordOffset], PrgmBuf,
				Band->CacheStartOffset, Band->CacheEndOffset);
		if (Status != XST_SUCCESS) {
			Status = (Status | (int)XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
			goto END;
		}

		if ((Band->DataMask != XIL_UNSIGNED_ZERO) &&
				(Band->DataMask != XNVM_EFUSE_BAND_DATA_MASK_ALL)) {
			PrgmBuf[0] &= Band->DataMask;
		}

		Status = XNvm_EfuseEngineCheckData(PrgmBuf,
				CacheWords);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		BandSrcData = PrgmBuf;
		RowDataNumWords = CacheWords;
		break;
	case XNVM_EFUSE_BAND_FLAG_DIRECT_WRITE:
		if ((Band->DataMask != XIL_UNSIGNED_ZERO) &&
				(Band->DataMask != XNVM_EFUSE_BAND_DATA_MASK_ALL)) {
			PrgmBuf[0] = XNvm_EfuseEngineBandCacheWord(Band,
					Data[Band->SrcWordOffset] & Band->DataMask);
			BandSrcData = PrgmBuf;
			RowDataNumWords = XIL_UNSIGNED_ONE;
		} else {
			if (Band->SrcWordOffset >= NumWords) {
				Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
				goto END;
			}
			BandSrcData = &Data[Band->SrcWordOffset];
			RowDataNumWords = NumWords - Band->SrcWordOffset;
		}
		break;
	default:
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	PrgmInfo.StartRow = Band->StartRow;
	PrgmInfo.ColStart = Band->ColStart;
	PrgmInfo.ColEnd = Band->ColEnd;
	PrgmInfo.NumOfRows = Band->NumRows;
	PrgmInfo.EfuseType = (XNvm_EfuseType)Band->Page;

	if ((BandFlags & XNVM_EFUSE_BAND_FLAG_SKIP_VERIFY) != XIL_UNSIGNED_ZERO) {
		PrgmInfo.SkipVerify = XNVM_EFUSE_SKIP_VERIFY;
	}

	/* DIRECT_WRITE / FIXED_DATA: ERR_NTHG when no row bits set (legacy PgmAndVerifyData). */
	Status = XNvm_EfusePgmAndVerifyData(&PrgmInfo, BandSrcData,
			RowDataNumWords);

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	Programs all bands of a field through the band engine.
 *
 * @param	Field - Field whose bands are programmed in order.
 * @param	Data - Caller source data words (validated by caller).
 * @param	NumWords - Number of valid words in Data.
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- XNVM_EFUSE_ERR_NTHG_TO_BE_PROGRAMMED is skipped per band (no bits
 *		  to program in that band); other bands in the field still run.
 *		- Field/band error on failure.
 *
 ******************************************************************************/
static int XNvm_EfuseEngineProgramFieldBands(const XNvm_EfuseFieldDesc *Field,
		const u32 *Data, u32 NumWords)
{
	u32 BandIdx = XIL_UNSIGNED_ZERO;
	u32 WriteErr = XIL_UNSIGNED_ZERO;
	int Status = XST_FAILURE;
	const XNvm_EfuseBand *Bands = NULL;

	if (Field == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

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
		Status = XNvm_EfuseEngineProgramBand(&Bands[BandIdx], Data, NumWords);
		if (Status == (int)XNVM_EFUSE_ERR_NTHG_TO_BE_PROGRAMMED) {
			Status = XST_SUCCESS;
			continue;
		}
		if (Status != XST_SUCCESS) {
			if ((Status & (int)XNVM_EFUSE_ERR_BEFORE_PROGRAMMING) ==
					XIL_UNSIGNED_ZERO) {
				WriteErr = Field->ErrWrite;
				if (Bands[BandIdx].ErrWrite != XIL_UNSIGNED_ZERO) {
					WriteErr = Bands[BandIdx].ErrWrite;
				}
				Status = (Status | WriteErr);
			}
			goto END;
		}
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	Returns whether a protection trigger condition is satisfied.
 *
 * @param	Entry - Protection entry whose trigger is evaluated.
 *
 * @return
 *		- XST_SUCCESS when the trigger fired (protection should run).
 *		- XST_FAILURE when the trigger did not fire.
 *		- Other error (e.g. XNVM_EFUSE_ERR_CACHE_PARITY) on read failure.
 *		For RANGE_NONZERO, only XNvm_EfuseCheckZeros() returning XST_FAILURE
 *		(non-zero data) fires the trigger.
 *
 ******************************************************************************/
static int XNvm_EfuseEngineProtTriggered(const XNvm_EfuseProtEntry *Entry)
{
	volatile u32 RowData = XIL_UNSIGNED_ZERO;
	volatile u32 RowDataTmp = XIL_UNSIGNED_ZERO;
	int CheckZerosStatus = XST_FAILURE;
	int Status = XST_FAILURE;

	switch (Entry->TriggerType) {
	case XNVM_EFUSE_PROT_TRIGGER_REG_NONZERO:
		RowData = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR, Entry->CacheOffset);
		RowDataTmp = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR, Entry->CacheOffset);
		Status = ((RowData == RowDataTmp) && (RowData != XIL_UNSIGNED_ZERO)) ?
			XST_SUCCESS : XST_FAILURE;
		break;
	case XNVM_EFUSE_PROT_TRIGGER_REG_MASKED_NONZERO:
		RowData = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR, Entry->CacheOffset) &
			Entry->Mask;
		RowDataTmp = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR, Entry->CacheOffset) &
			Entry->Mask;
		Status = ((RowData == RowDataTmp) && (RowData != XIL_UNSIGNED_ZERO)) ?
			XST_SUCCESS : XST_FAILURE;
		break;
	case XNVM_EFUSE_PROT_TRIGGER_RANGE_NONZERO:
		CheckZerosStatus = XNvm_EfuseCheckZeros(Entry->CacheOffset,
				Entry->NumWords);
		if (CheckZerosStatus == XST_FAILURE) {
			Status = XST_SUCCESS;
		} else if (CheckZerosStatus != XST_SUCCESS) {
			Status = CheckZerosStatus;
		}
		break;
	default:
		Status = XST_FAILURE;
		break;
	}

	return Status;
}
/** @} */