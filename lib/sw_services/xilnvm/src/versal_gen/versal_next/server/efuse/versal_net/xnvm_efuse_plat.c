/******************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
*
* @file versal_next/server/efuse/versal_net/xnvm_efuse_plat.c
*
* Platform PUF eFuse programming (helper data and user-fuse modes).
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
* PUF compile-time split (XNVM_ACCESS_PUF_USER_DATA):
*   undefined (default) — XNvm_EfuseWritePuf() via band engine.
*   defined             — XNvm_EfuseWritePufAsUserFuses() with dynamic COMPUTE_PRG bands.
* XNvm_EfuseWritePufSecCtrl() is built in both configurations.
*
*******************************************************************************/

/**
 * @addtogroup xnvm_efuse_server_apis XilNvm eFUSE Server APIs
 * @{
 */

/***************************** Include Files *********************************/
#include "xnvm_efuse_plat.h"
#include "xnvm_efuse.h"
#include "xnvm_efuse_common_hw.h"
#include "xnvm_efuse_hw.h"
#include "xnvm_efuse_common.h"
#include "xnvm_efuse_engine.h"
#include "xnvm_efuse_error.h"
#include "xnvm_efuse_common_field.h"
#include "xnvm_ops.h"
#include "xnvm_validate.h"

/************************** Constant Definitions *****************************/

static int XNvm_EfusePlatCheckCacheNonZero(u32 StartOffset, u32 NumRows);

#if !(defined(VERSAL_2VE_2VM) && defined(VERSAL_2VP_P))
#define XNVM_EFUSE_PUF_SEC_CTRL_INVLD_MASK	(0xE0000000U)
			/**< Mask for PUF control bits in PUF_ECC_PUF_CTRL register */

#ifdef XNVM_ACCESS_PUF_USER_DATA
static int XNvm_EfusePlatWritePufUserFuse(u32 EnvDisFlag, u32 StartPufFuseRow,
		u32 NumOfRows, const u32 *Data);
#endif

/************************** Function Definitions *****************************/
/******************************************************************************/
/**
 * @brief	This function programs Puf control bits specified by user.
 *
 * @param	EnvDisFlag - Environmental monitoring flag set by the user,
 *				when set to true it will not check for voltage
 *				and temperature limits.
 * @param 	PufCtrlBits - PufCtrlBits input to be programmed
 *
 * @return
 *		- XST_SUCCESS  On success.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM  On invalid input parameter.
 *		- XNVM_EFUSE_ERR_BEFORE_PROGRAMMING  Error before programming.
 *		- XNVM_EFUSE_ERR_WRITE_PUF_SEC_CTRL_BITS  Error while writing
 *			PUF security control bits (REGIS_DIS, HD_INVLD, REGEN_DIS).
 *
 ******************************************************************************/
int XNvm_EfuseWritePufSecCtrl(u32 EnvDisFlag, u32 PufCtrlBits)
{
	volatile int Status = XST_FAILURE;

	if ((PufCtrlBits == XIL_UNSIGNED_ZERO) ||
			((PufCtrlBits & (~XNVM_EFUSE_PUF_SEC_CTRL_INVLD_MASK)) !=
			XIL_UNSIGNED_ZERO)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfuseEngineWriteField(&XNvm_EfuseDeviceDesc,
			XNVM_EFUSE_FIELD_PUF_SEC_CTRL, EnvDisFlag,
			&PufCtrlBits, XIL_UNSIGNED_ONE);
	if ((Status != XST_SUCCESS) &&
			((Status & (int)XNVM_EFUSE_ERR_BEFORE_PROGRAMMING) ==
			XIL_UNSIGNED_ZERO)) {
		Status = (Status | (int)XNVM_EFUSE_ERR_WRITE_PUF_SEC_CTRL_BITS);
	}

END:
	return Status;
}

#ifndef XNVM_ACCESS_PUF_USER_DATA
/******************************************************************************/
/**
 * @brief	This function programs PUF ctrl and PUF helper data, Chash and
 * 		Aux.
 *
 * @param	PufHelperData - Pointer to XNvm_EfusePufHdAddr structure
 *
 * @return
 *		- XST_SUCCESS  On successful write.
 *		- XNVM_EFUSE_ERR_WRITE_PUF_HELPER_DATA  Error while writing Puf helper data.
 *		- XNVM_EFUSE_ERR_WRITE_PUF_SYN_DATA  Error while writing Puf Syndata.
 *		- XNVM_EFUSE_ERR_WRITE_PUF_CHASH  Error while writing Puf Chash.
 *		- XNVM_EFUSE_ERR_WRITE_PUF_AUX  Error while writing Puf Aux.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM  On Invalid input parameter.
 *		- XNVM_EFUSE_ERR_BEFORE_PROGRAMMING  Error before programming.
 *
 * @note	To generate PufSyndromeData please use XPuf_Registration API.
 *
 ******************************************************************************/
int XNvm_EfuseWritePuf(const XNvm_EfusePufHdAddr *PufHelperData)
{
	volatile int Status = XST_FAILURE;
	u32 EnvDisFlag = XIL_UNSIGNED_ZERO;

	if (PufHelperData == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (PufHelperData->PrgmPufHelperData != TRUE) {
		Status = XST_SUCCESS;
		goto END;
	}

	EnvDisFlag = PufHelperData->EnvMonitorDis;

	Status = XNvm_EfuseEngineWriteField(&XNvm_EfuseDeviceDesc,
			XNVM_EFUSE_FIELD_PUF_SYN_DATA, EnvDisFlag,
			PufHelperData->EfuseSynData,
			XNVM_PUF_FORMATTED_SYN_DATA_LEN_IN_WORDS);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseEngineWriteField(&XNvm_EfuseDeviceDesc,
			XNVM_EFUSE_FIELD_PUF_CHASH, EnvDisFlag,
			&PufHelperData->Chash, XIL_UNSIGNED_ONE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseEngineWriteField(&XNvm_EfuseDeviceDesc,
			XNVM_EFUSE_FIELD_PUF_AUX, EnvDisFlag,
			&PufHelperData->Aux, XIL_UNSIGNED_ONE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (PufHelperData->RoSwap != XIL_UNSIGNED_ZERO) {
		Status = XNvm_EfuseEngineWriteField(&XNvm_EfuseDeviceDesc,
				XNVM_EFUSE_FIELD_PUF_RO_SWAP, EnvDisFlag,
				&PufHelperData->RoSwap, XIL_UNSIGNED_ONE);
	}

END:
	return Status;
}

#else /* XNVM_ACCESS_PUF_USER_DATA */

/******************************************************************************/
/**
 * @brief	Programs a PUF syndrome row range via dynamic COMPUTE_PRG bands.
 *
 * @param	EnvDisFlag - When TRUE, skip environmental monitoring.
 * @param	StartPufFuseRow - First PUF user-fuse row (0..127).
 * @param	NumOfRows - Number of consecutive rows to program.
 * @param	Data - Source words (NumOfRows entries).
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- Validation, open, program, cache-reload, or close error.
 *
 ******************************************************************************/
static int XNvm_EfusePlatWritePufUserFuse(u32 EnvDisFlag, u32 StartPufFuseRow,
		u32 NumOfRows, const u32 *Data)
{
	volatile int Status = XST_FAILURE;
	u32 StartRow = StartPufFuseRow;
	u32 ToBeProgrammed = NumOfRows;
	u32 DataOffset = XIL_UNSIGNED_ZERO;
	u32 RowOffset = XIL_UNSIGNED_ZERO;
	u32 RowsRemainingInPage = XIL_UNSIGNED_ZERO;
	u32 NumRows = XIL_UNSIGNED_ZERO;
	u32 CacheStart = XIL_UNSIGNED_ZERO;
	XNvm_EfuseBand Band = {XIL_UNSIGNED_ZERO};

	if ((Data == NULL) || (NumOfRows == XIL_UNSIGNED_ZERO)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfuseValidatePufUserFuseAccess();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseEngineOpen(EnvDisFlag);
	if (Status != XST_SUCCESS) {
		goto END_CLOSE;
	}

	Band.ColStart = XNVM_EFUSE_PUF_SYN_DATA_START_COL_NUM;
	Band.ColEnd = XNVM_EFUSE_PUF_SYN_DATA_END_COL_NUM;
	Band.Flags = XNVM_EFUSE_BAND_FLAG_COMPUTE_PRG;

	while (ToBeProgrammed > XIL_UNSIGNED_ZERO) {
		RowOffset = StartRow & XNVM_EFUSE_PUF_PAGE_ROW_OFFSET_MASK;
		RowsRemainingInPage = XNVM_EFUSE_PUF_ROWS_PER_PAGE - RowOffset;
		NumRows = (ToBeProgrammed < RowsRemainingInPage) ? ToBeProgrammed :
				RowsRemainingInPage;
		CacheStart = XNVM_EFUSE_CACHE_PUF_SYN_DATA_OFFSET +
				(StartRow * XNVM_WORD_LEN);

		Band.StartRow = XNVM_EFUSE_PAGE_0_PUF_SYN_DATA_ROW + RowOffset;
		Band.Page = (XNvm_EfuseType)(StartRow >>
				XNVM_EFUSE_PUF_PAGE_ROW_INDEX_BITS);
		if ((u32)Band.Page > (u32)XNVM_EFUSE_PAGE_MAX) {
			Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
			goto END_CLOSE;
		}

		Band.NumRows = NumRows;
		Band.CacheStartOffset = CacheStart;
		Band.CacheEndOffset = CacheStart + ((NumRows - 1U) * XNVM_WORD_LEN);
		Band.SrcWordOffset = DataOffset;

		Status = XNvm_EfuseEngineProgramBand(&Band, Data, NumOfRows);
		if (Status != XST_SUCCESS) {
			Status = (Status | (int)XNVM_EFUSE_ERR_WRITE_PUF_FUSES);
			goto END_CLOSE;
		}

		ToBeProgrammed -= NumRows;
		StartRow += NumRows;
		DataOffset += NumRows;
	}

	Status = XNvm_EfuseOpsCacheOperations();

END_CLOSE:
	Status = XNvm_EfuseEngineClose(Status);

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs PUF HD eFuses as general purpose eFuses.
 *
 * @param	PufFuse - Pointer to the XNvm_EfusePufFuse structure
 *
 * @return
 *		- XST_SUCCESS If programs successfully.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM  On Invalid Parameter.
 *		- XNVM_EFUSE_ERR_PUF_SYN_ALREADY_PRGMD  Puf Syn data already programmed.
 *		- XNVM_EFUSE_ERR_PUF_CHASH_ALREADY_PRGMD  Puf chash already programmed.
 *		- XNVM_EFUSE_ERR_PUF_AUX_ALREADY_PRGMD  Puf Aux is already programmed.
 *		- XNVM_EFUSE_ERR_WRITE_PUF_FUSES  Error in writing PUF user fuses
 *
 * @note By default PUF eFuses are used for PUF helper data.
 *       To program PUF eFuses as general purpose efuses user needs to enable
 *       compile time macro XNVM_ACCESS_PUF_USER_DATA.
 *
 ******************************************************************************/
int XNvm_EfuseWritePufAsUserFuses(const XNvm_EfusePufFuse *PufFuse)
{
	volatile int Status = XST_FAILURE;
	u32 EnvDisFlag = XIL_UNSIGNED_ZERO;

	if (PufFuse == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (PufFuse->StartPufFuseRow > (XNVM_EFUSE_PUF_SYN_USERDATA_NUM_OF_ROWS - 1U)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if ((PufFuse->NumOfPufFusesRows < XIL_UNSIGNED_ONE) ||
		(PufFuse->NumOfPufFusesRows > XNVM_EFUSE_PUF_SYN_USERDATA_NUM_OF_ROWS)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if ((PufFuse->StartPufFuseRow + PufFuse->NumOfPufFusesRows) >
		 XNVM_EFUSE_PUF_SYN_USERDATA_NUM_OF_ROWS) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (PufFuse->PufFuseData == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	EnvDisFlag = (PufFuse->EnvMonitorDis == TRUE) ? TRUE : FALSE;

	if (PufFuse->PrgmPufFuse == TRUE) {
		Status = XNvm_EfusePlatWritePufUserFuse(EnvDisFlag,
				PufFuse->StartPufFuseRow, PufFuse->NumOfPufFusesRows,
				PufFuse->PufFuseData);
	} else {
		Status = XNvm_EfuseEngineOpen(EnvDisFlag);
		if (Status != XST_SUCCESS) {
			goto END_CLOSE;
		}

		Status = XNvm_EfuseOpsCacheOperations();

END_CLOSE:
		Status = XNvm_EfuseEngineClose(Status);
	}

END:
	return Status;
}
#endif /* XNVM_ACCESS_PUF_USER_DATA */
#endif /* !(VERSAL_2VE_2VM && VERSAL_2VP_P) */

/******************************************************************************/
/**
 * @brief	Returns XST_SUCCESS when the cache region has at least
 *		one programmed (non-zero) row.
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
 * @note	Scans from StartOffset; stops at the first non-zero row.
 *
 ******************************************************************************/
static int XNvm_EfusePlatCheckCacheNonZero(u32 StartOffset, u32 NumRows)
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
 * @brief	Runs Versal Net protection checks after cache reload.
 *
 * Protection status bits live in TBITS0_SVD. Each active protection bit
 * requires its protected fuse region to be programmed.
 *
 * @return
 *		- XST_SUCCESS when all active protection rules pass.
 *		- XNVM_EFUSE_ERR_ANCHOR_BIT_PATTERN on bad anchor pattern.
 *		- XNVM_EFUSE_ERR_IN_PROTECTION_CHECK on protected-data mismatch.
 *		- XNVM_EFUSE_XLNX_MANFAC_BITS_CRC_ERROR on CRC failure.
 *
 ******************************************************************************/
int XNvm_EfusePlatProtectionChecks(void)
{
	volatile int Status = XST_FAILURE;
	volatile u32 RegVal = XIL_UNSIGNED_ZERO;
	volatile u32 RegValTmp = XIL_UNSIGNED_ZERO;
	volatile u32 ProtVal = XIL_UNSIGNED_ZERO;
	volatile u32 ProtValTmp = XIL_UNSIGNED_ZERO;
	volatile u32 RowVal = XIL_UNSIGNED_ZERO;
	volatile u32 RowValTmp = XIL_UNSIGNED_ZERO;

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

	ProtVal = RegVal & XNVM_EFUSE_CACHE_TBITS0_SVD_SEC_CTRL_PROT_MASK;
	ProtValTmp = RegValTmp & XNVM_EFUSE_CACHE_TBITS0_SVD_SEC_CTRL_PROT_MASK;

	if ((ProtVal != XIL_UNSIGNED_ZERO) || (ProtValTmp != XIL_UNSIGNED_ZERO)) {
		RowVal = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_SECURITY_CONTROL_OFFSET);
		RowValTmp = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_SECURITY_CONTROL_OFFSET);

		if ((RowVal != RowValTmp) || (RowVal == XIL_UNSIGNED_ZERO)) {
			Status = (int)XNVM_EFUSE_ERR_IN_PROTECTION_CHECK;
			goto END;
		}
	}

	ProtVal = RegVal & XNVM_EFUSE_CACHE_TBITS0_SVD_SEC_MISC0_PROT_MASK;
	ProtValTmp = RegValTmp & XNVM_EFUSE_CACHE_TBITS0_SVD_SEC_MISC0_PROT_MASK;

	if ((ProtVal != XIL_UNSIGNED_ZERO) || (ProtValTmp != XIL_UNSIGNED_ZERO)) {
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

	ProtVal = RegVal & XNVM_EFUSE_CACHE_TBITS0_SVD_PPK_HASH_PROT_MASK;
	ProtValTmp = RegValTmp & XNVM_EFUSE_CACHE_TBITS0_SVD_PPK_HASH_PROT_MASK;

	if ((ProtVal != XIL_UNSIGNED_ZERO) || (ProtValTmp != XIL_UNSIGNED_ZERO)) {
		Status = XNvm_EfusePlatCheckCacheNonZero(
				XNVM_EFUSE_CACHE_PPK0_HASH_0_OFFSET,
				XNVM_EFUSE_TOTAL_PPK_HASH_ROWS);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	ProtVal = RegVal & XNVM_EFUSE_CACHE_TBITS0_SVD_META_HEADER_EXPORT_PROT_MASK;
	ProtValTmp = RegValTmp &
			XNVM_EFUSE_CACHE_TBITS0_SVD_META_HEADER_EXPORT_PROT_MASK;

	if ((ProtVal != XIL_UNSIGNED_ZERO) || (ProtValTmp != XIL_UNSIGNED_ZERO)) {
		Status = XNvm_EfusePlatCheckCacheNonZero(
				XNVM_EFUSE_CACHE_METAHEADER_IV_RANGE_0_OFFSET,
				XNVM_EFUSE_IV_NUM_OF_CACHE_ROWS);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	ProtVal = RegVal & XNVM_EFUSE_CACHE_TBITS0_SVD_CRC_PROT_MASK;
	ProtValTmp = RegValTmp & XNVM_EFUSE_CACHE_TBITS0_SVD_CRC_PROT_MASK;

	if ((ProtVal != XIL_UNSIGNED_ZERO) || (ProtValTmp != XIL_UNSIGNED_ZERO)) {
		RowVal = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_ME_ID_CODE_OFFSET) &
				XNVM_EFUSE_CACHE_ME_ID_CODE_CRC_SALT_MASK;
		RowValTmp = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_ME_ID_CODE_OFFSET) &
				XNVM_EFUSE_CACHE_ME_ID_CODE_CRC_SALT_MASK;

		if ((RowVal != XIL_UNSIGNED_ZERO) ||
				(RowValTmp != XIL_UNSIGNED_ZERO)) {
			Status = XNvm_EfuseOpsValidateCrc();
			if (Status != XST_SUCCESS) {
				Status = (int)XNVM_EFUSE_XLNX_MANFAC_BITS_CRC_ERROR;
				goto END;
			}
		}
	}

	ProtVal = RegVal & XNVM_EFUSE_CACHE_TBITS0_SVD_PUF_CHASH_PROT_MASK;
	ProtValTmp = RegValTmp & XNVM_EFUSE_CACHE_TBITS0_SVD_PUF_CHASH_PROT_MASK;

	if ((ProtVal != XIL_UNSIGNED_ZERO) || (ProtValTmp != XIL_UNSIGNED_ZERO)) {
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
	ProtValTmp = RegValTmp & XNVM_EFUSE_CACHE_TBITS0_SVD_SEC_MISC1_PROT_MASK;

	if ((ProtVal != XIL_UNSIGNED_ZERO) || (ProtValTmp != XIL_UNSIGNED_ZERO)) {
		RowVal = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_SECURITY_MISC_1_OFFSET);
		RowValTmp = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_SECURITY_MISC_1_OFFSET);

		if ((RowVal != RowValTmp) ||
			((RowVal & XNVM_EFUSE_SECURITY_MISC_1_PROT_MASK) ==
				XIL_UNSIGNED_ZERO)) {
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
/** @} */