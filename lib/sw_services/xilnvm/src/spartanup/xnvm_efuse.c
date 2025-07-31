/******************************************************************************
* Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xnvm_efuse.c
* This file contains the eFUSE API's of spartan ultrascale plus to program/read the
* eFUSE array.
*
* @note	None.
*
* </pre>
* MODIFICATION HISTORY:
*
* Ver   Who   Date     Changes
* ----- ----  -------- ------------------------------------------------------
* 1.0   kpt   07/30/24 First release
* 1.1   mb    04/11/25 Passed args to XNvm_EfuseCheckAesKeyCrc in correct order
* 3.5   hj    04/01/25 Remove flag checks from XNvm_EfusePrgmSecCtrlBits
*       hj    04/01/25 Update comment of XNvm_EfuseReadSecCtrlBits
*       hj    04/08/25 Rename XNVM_GET_BIT_VAL to XNVM_GET_8_BIT_VAL
*       hj    04/10/25 Rename PPK hash size macros
*       hj    04/10/25 Remove security control bits not exposed to user
*       hj    04/10/25 Fix PPK hash size end index in XNvm_EfuseValidatePpkWriteReq
* 3.6   hj    04/10/25 Remove zero IV validation check in dec_only case
*       hj    05/27/25 Support XILINX_CTRL efuse PUFHD_INVLD and DIS_SJTAG bit programming
*       mb    07/18/25 Add AES key CRC calculation steps
*       aa    07/24/25 Typecast to essential datatypes to avoid implicit conversions
*                      added const qualifier for function prototypes and added explicit
*                       parenthesis
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files *********************************/

#include "xnvm_efuse.h"
#include "xnvm_utils.h"

/************************** Constant Definitions *****************************/

/**< Timeout in term of number of times status register polled to check eFuse
 * Crc check id done.
 */
#define XNVM_POLL_TIMEOUT				(0x400U) /**< Poll timeout during CRC verification  */

#define XNVM_GET_8_BIT_VAL(Val, bits, shift)	(((Val) >> (shift)) & (unsigned char)(~(0xFFU << (bits))))
		/**< API to extract bit mask */

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/

/************************** Function Prototypes *****************************/
/**
 * @name API declarations
 * @{
 */
/**< Prototype declarations for xilnvm efuse API's */
/** @} */

static int XNvm_EfuseValidateAesWriteReq(const XNvm_EfuseAesKeys *AesKey);
static int XNvm_EfuseValidatePpkWriteReq(const XNvm_EfusePpkHash *PpkHash);
static int XNvm_EfuseValidateIvsWriteReq(const XNvm_EfuseAesIvs *Ivs);
static int XNvm_EfuseValidateDecOnlyWriteReq(const XNvm_EfuseData *EfuseData);
static int XNvm_EfuseValidateBeforeWriteReq(const XNvm_EfuseData *EfuseData);
static int XNvm_EfusePrgmAesKey(const XNvm_EfuseAesKeys *AesKey);
static int XNvm_EfusePrgmPpkHash(XNvm_EfusePpkHash *PpkHash);
static int XNvm_EfusePrgmIv(const XNvm_EfuseAesIvs *AesIv);
static int XNvm_EfusePrgmUserFuse(const XNvm_EfuseUserFuse *UserFuse);
static int XNvm_EfusePrgmSpkRevokeId(const XNvm_EfuseSpkRevokeId *SpkRevokeId);
static int XNvm_EfusePrgmAesRevokeId(const XNvm_EfuseAesRevokeId *AesRevokeId);
static int XNvm_EfusePrgmDecOnly(const XNvm_EfuseDecOnly *DecOnly);
static int XNvm_EfusePrgmSecCtrlBits(const XNvm_EfuseSecCtrl *SecCtrl);
static int XNvm_EfuseComputeProgrammableBits(const u32 *ReqData, u32 *PrgmData,
	u32 StartOffset, u32 EndOffset);
static int XNvm_EfuseCacheReload(void);
static int XNvm_EfusePgmAndVerifyData(const XNvm_EfusePrgmInfo *EfusePrgmInfo, const u32 *RowData);
static int XNvm_EfusePgmBit(u32 Row, u32 Col);
static int XNvm_EfuseReadRow(u32 Row, u32 *RegData);
static int XNvm_EfuseVerifyBit(u32 Row, u32 Col);
static int XNvm_EfusePgmAndVerifyBit(u32 Row, u32 Col, u32 SkipVerify);
static int XNvm_EfuseReadCache(u32 Offset, u32 *RowData);
static int XNvm_EfuseReadCacheRange(u32 StartOffset, u8 OffsetCount, u32 *RowData);
int XNvm_EfuseReadSecCtrlBits(XNvm_EfuseSecCtrlBits *SecCtrlBits);
static int XNvm_EfuseCheckZeros(u32 OffsetStart, u32 OffsetEnd);
static int XNvm_EfusePrgmPufHDInvld(const XNvm_EfuseXilinxCtrl *PufHDInvld);
static int XNvm_EfusePrgmXilinxCtrl(const XNvm_EfuseXilinxCtrl *XilinxCtrl);

/************************** Function Definitions *****************************/

/***************************************************************************/
/**
* This function is used to program the eFUSE of spartan ultrascale plus, based on user
* inputs
*
* @param	EfuseData	Pointer to the XNvm_EfuseData.
*
* @return
* 		- XST_SUCCESS if programs successfully.
* 		- Errorcode on failure
*
* @note		After eFUSE programming is complete, the cache is automatically
* 	reloaded so all programmed eFUSE bits can be directly read from cache.
*
****************************************************************************/
int XNvm_EfuseWrite(XNvm_EfuseData *EfuseData)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;

	if (EfuseData == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if ((EfuseData->AesKeys == NULL) &&
	    (EfuseData->AesRevokeId == NULL) &&
	    (EfuseData->DecOnly == NULL) &&
	    (EfuseData->Ivs == NULL) &&
	    (EfuseData->PpkHash == NULL) &&
	    (EfuseData->SecCtrlBits == NULL) &&
	    (EfuseData->SpkRevokeId == NULL) &&
	    (EfuseData->XilinxCtrl == NULL) &&
	    (EfuseData->UserFuse == NULL)) {
		Status = XNVM_EFUSE_ERR_NTHG_TO_BE_PROGRAMMED;
		goto END;
	}

	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM, XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		Status = Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING;
		goto END;
	}

	Status = XNvm_EfuseValidateBeforeWriteReq(EfuseData);
	if (Status != XST_SUCCESS) {
		Status = Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING;
		goto END_RST;
	}

	if (EfuseData->AesKeys != NULL) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePrgmAesKey(EfuseData->AesKeys);
		if (Status != XST_SUCCESS) {
			goto END_RST;
		}
	}

	if (EfuseData->PpkHash != NULL) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePrgmPpkHash(EfuseData->PpkHash);
		if (Status != XST_SUCCESS) {
			goto END_RST;
		}
	}

	if (EfuseData->Ivs != NULL) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePrgmIv(EfuseData->Ivs);
		if (Status != XST_SUCCESS) {
			goto END_RST;
		}
	}

	if (EfuseData->UserFuse != NULL) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePrgmUserFuse(EfuseData->UserFuse);
		if (Status != XST_SUCCESS) {
			goto END_RST;
		}
	}

	if (EfuseData->SpkRevokeId != NULL) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePrgmSpkRevokeId(EfuseData->SpkRevokeId);
		if (Status != XST_SUCCESS) {
			goto END_RST;
		}
	}

	if (EfuseData->AesRevokeId != NULL) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePrgmAesRevokeId(EfuseData->AesRevokeId);
		if (Status != XST_SUCCESS) {
			goto END_RST;
		}
	}

	if (EfuseData->DecOnly != NULL) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePrgmDecOnly(EfuseData->DecOnly);
		if (Status != XST_SUCCESS) {
			goto END_RST;
		}
	}

	if (EfuseData->XilinxCtrl != NULL) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePrgmXilinxCtrl(EfuseData->XilinxCtrl);
		if (Status != XST_SUCCESS) {
			goto END_RST;
		}
	}

	if (EfuseData->SecCtrlBits != NULL) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePrgmSecCtrlBits(EfuseData->SecCtrlBits);
		if (Status != XST_SUCCESS) {
			goto END_RST;
		}
	}

	Status = XNvm_EfuseCacheReload();

END_RST:
	SStatus = XNvm_EfuseResetReadMode();
	if (SStatus != XST_SUCCESS) {
		Status |= SStatus;
	}

	SStatus = XNvm_EfuseDisableProgramming();
	if (SStatus != XST_SUCCESS) {
		Status |= SStatus;
	}
END:
	return Status;

}

/******************************************************************************/
/**
 * @brief	This function validates AES efuse keys write request
 *
 * @param	AesKey - Pointer to XNvm_EfuseAesKeys.
 *
 * @return	- XST_SUCCESS	- On success.
 *          - Errorcode on failure
 *
 ******************************************************************************/
static int XNvm_EfuseValidateAesWriteReq(const XNvm_EfuseAesKeys *AesKey)
{
	volatile int Status = XST_FAILURE;
	XNvm_EfuseSecCtrlBits ReadBackSecCtrlBits;

	if (AesKey->PrgmAesKey == (u8)TRUE) {
		Status = XNvm_EfuseReadSecCtrlBits(
				 &ReadBackSecCtrlBits);
		if (Status != XST_SUCCESS) {
			Status = XNVM_EFUSE_ERR_RD_SEC_CTRL_BITS;
			goto END;
		}

		if ((ReadBackSecCtrlBits.AES_DIS == (u8)TRUE) ||
		    (ReadBackSecCtrlBits.AES_RD_WR_LK_0 == (u8)TRUE) ||
		    (ReadBackSecCtrlBits.AES_RD_WR_LK_1 == (u8)TRUE)) {
			Status = (XNVM_EFUSE_ERR_FUSE_PROTECTED |
				  XNVM_EFUSE_ERR_WRITE_AES_KEY);
			goto END;
		}

		Status = XNvm_EfuseCheckAesKeyCrc(XNVM_EFUSE_AES_CRC_OFFSET,
						  XNVM_EFUSE_STS_AES_CRC_DONE_MASK,
						  XNVM_EFUSE_STS_AES_CRC_PASS_MASK,
						  XNVM_EFUSE_CRC_AES_ZEROS);
		if (Status != XST_SUCCESS) {
			Status = XNVM_EFUSE_ERR_AES_ALREADY_PRGMD;
		}
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function validates PPK efuses write request
 *
 * @param	PpkHash - Pointer to XNvm_EfusePpkHash.
 *
 * @return	- XST_SUCCESS	- On success.
 *          - Errorcode on failure
 *
 ******************************************************************************/
static int XNvm_EfuseValidatePpkWriteReq(const XNvm_EfusePpkHash *PpkHash)
{
	int Status = XST_FAILURE;
	u32 RemPpkHashLen = 0U;

	if ((PpkHash->ActaulPpkHashSize != XNVM_EFUSE_PPK_HASH_256_SIZE_IN_BYTES) &&
	    (PpkHash->ActaulPpkHashSize != XNVM_EFUSE_PPK_HASH_384_SIZE_IN_BYTES)) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if ((PpkHash->PrgmPpk2Hash == (u8)TRUE)
	    && (PpkHash->ActaulPpkHashSize != XNVM_EFUSE_PPK_HASH_256_SIZE_IN_BYTES)) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	RemPpkHashLen = (PpkHash->ActaulPpkHashSize - XNVM_EFUSE_PPK_HASH_256_SIZE_IN_BYTES);
	if (PpkHash->PrgmPpk0Hash == (u8)TRUE) {
		Status = XNvm_EfuseCheckZeros(XNVM_EFUSE_PPK0_START_OFFSET,
					      (XNVM_EFUSE_PPK0_START_OFFSET + PpkHash->ActaulPpkHashSize));
		if (Status != XST_SUCCESS) {
			Status = XNVM_EFUSE_ERR_PPK0_HASH_ALREADY_PRGMD;
			goto END;
		}
	}

	if (PpkHash->PrgmPpk1Hash == (u8)TRUE) {
		Status = XNvm_EfuseCheckZeros((XNVM_EFUSE_PPK1_START_OFFSET + RemPpkHashLen),
					      (XNVM_EFUSE_PPK1_START_OFFSET + RemPpkHashLen +
					       PpkHash->ActaulPpkHashSize));
		if (Status != XST_SUCCESS) {
			Status = XNVM_EFUSE_ERR_PPK1_HASH_ALREADY_PRGMD;
			goto END;
		}
	}

	if (PpkHash->PrgmPpk2Hash == (u8)TRUE) {
		Status = XNvm_EfuseCheckZeros(XNVM_EFUSE_PPK2_START_OFFSET, XNVM_EFUSE_PPK2_START_OFFSET +
					      PpkHash->ActaulPpkHashSize);
		if (Status != XST_SUCCESS) {
			Status = XNVM_EFUSE_ERR_PPK2_HASH_ALREADY_PRGMD;
		}
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function validates IV efuses write request
 *
 * @param	Ivs - Pointer to XNvm_EfuseAesIvs.
 *
 * @return	- XST_SUCCESS	- On success.
 *          - Errorcode on failure
 *
 ******************************************************************************/
static int XNvm_EfuseValidateIvsWriteReq(const XNvm_EfuseAesIvs *Ivs)
{
	int Status = XST_FAILURE;
	u32 IvRow;
	u32 IvRowsRd[XNVM_EFUSE_AES_IV_SIZE_IN_WORDS];

	if (Ivs->PrgmIv == (u8)TRUE) {
		Status = XNvm_EfuseReadCacheRange(XNVM_EFUSE_IV_START_OFFSET, XNVM_EFUSE_AES_IV_SIZE_IN_WORDS,
						  IvRowsRd);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		for (IvRow = 0U; IvRow < XNVM_EFUSE_AES_IV_SIZE_IN_WORDS;
		     IvRow++) {
			if ((IvRowsRd[IvRow] & Ivs->AesIv[IvRow]) != IvRowsRd[IvRow]) {
				Status = (XNVM_EFUSE_ERR_BEFORE_PROGRAMMING |
					  XNVM_EFUSE_ERR_BIT_CANT_REVERT);
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
 * @brief	This function validates Deconly efuses write request
 *
 * @param	EfuseData - Pointer to XNvm_EfuseData.
 *
 * @return	- XST_SUCCESS	- On success.
 *          - Errorcode on failure
 *
 ******************************************************************************/
static int XNvm_EfuseValidateDecOnlyWriteReq(const XNvm_EfuseData *EfuseData)
{
	int Status = XST_FAILURE;
	u32 DecOnlyVal = 0U;
	XNvm_EfuseSecCtrlBits ReadSecCtrlBits;

	if (EfuseData->DecOnly->PrgmDeconly == (u8)TRUE) {
		Status = XNvm_EfuseReadCache(XNVM_EFUSE_DEC_ONLY_OFFSET, &DecOnlyVal);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		if (DecOnlyVal == 0U) {
			Status = XNvm_EfuseCheckAesKeyCrc(XNVM_EFUSE_AES_CRC_OFFSET,
							  XNVM_EFUSE_STS_AES_CRC_DONE_MASK,
							  XNVM_EFUSE_STS_AES_CRC_PASS_MASK,
							  XNVM_EFUSE_CRC_AES_ZEROS);
			if (Status == XST_SUCCESS) {
				if (EfuseData->AesKeys != NULL) {
					if (EfuseData->AesKeys->PrgmAesKey != (u8)TRUE) {
						Status = XNVM_EFUSE_ERR_DEC_ONLY_KEY_MUST_BE_PRGMD;
						goto END;
					}
				} else {
					Status = XNVM_EFUSE_ERR_DEC_ONLY_KEY_MUST_BE_PRGMD;
					goto END;
				}
			}

			Status =  XNvm_EfuseReadSecCtrlBits(&ReadSecCtrlBits);
			if (Status != XST_SUCCESS) {
				goto END;
			}

			if (ReadSecCtrlBits.HASH_PUF_OR_KEY != (u8)TRUE) {
				if (EfuseData->SecCtrlBits != NULL) {
					if (EfuseData->SecCtrlBits->PrgmHashPufOrKey != (u8)TRUE) {
						Status = XNVM_EFUSE_ERR_DEC_ONLY_HASH_OR_PUF_KEY_MUST_BE_PRGMD;
						goto END;
					}
				}
			}
		} else {
			Status = XNVM_EFUSE_ERR_DEC_ONLY_ALREADY_PRGMD;
		}
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function validates user configured write requests before programming
 *
 * @param	EfuseData - Pointer to XNvm_EfuseData.
 *
 * @return	- XST_SUCCESS	- On success.
 *          - Errorcode on failure
 *
 ******************************************************************************/
static int XNvm_EfuseValidateBeforeWriteReq(const XNvm_EfuseData *EfuseData)
{
	volatile int Status = XST_FAILURE;

	if (EfuseData->AesKeys != NULL) {
		Status =  XNvm_EfuseValidateAesWriteReq(EfuseData->AesKeys);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	if (EfuseData->PpkHash != NULL) {
		Status = XNvm_EfuseValidatePpkWriteReq(EfuseData->PpkHash);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	if (EfuseData->Ivs != NULL) {
		Status = XNvm_EfuseValidateIvsWriteReq(EfuseData->Ivs);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	if (EfuseData->DecOnly != NULL) {
		Status = XNvm_EfuseValidateDecOnlyWriteReq(EfuseData);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	Status = XST_SUCCESS;

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs AES key and validates the CRC of the provided key
 *          with programmed key
 *
 * @param	AesKey - Pointer to XNvm_EfuseAesKeys.
 *
 * @return	- XST_SUCCESS	- On successful programming.
 *          - Errorcode on failure
 *
 ******************************************************************************/
static int XNvm_EfusePrgmAesKey(const XNvm_EfuseAesKeys *AesKey)
{
	int Status = XST_FAILURE;
	XNvm_EfusePrgmInfo AesPrgmInfo = {0U};
	u32 Crc = 0U;

	AesPrgmInfo.StartRow = XNVM_EFUSE_AES_KEY_START_ROW;
	AesPrgmInfo.NumOfRows = XNVM_EFUSE_AES_KEY_NUM_OF_ROWS;
	AesPrgmInfo.ColStart = XNVM_EFUSE_AES_KEY_START_COL;
	AesPrgmInfo.ColEnd = XNVM_EFUSE_AES_KEY_END_COL;
	AesPrgmInfo.SkipVerify = (u32)TRUE;

	Status = XNvm_EfusePgmAndVerifyData(&AesPrgmInfo, AesKey->AesKey);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_WRITE_AES_KEY);
		goto END;
	}

	Status = XNvm_EfuseCacheReload();
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_WRITE_AES_KEY);
		goto END;
	}

	Crc = XNvm_AesCrcCalc(AesKey->AesKey);

	Status = XST_FAILURE;
	Status = XNvm_EfuseCheckAesKeyCrc(XNVM_EFUSE_AES_CRC_OFFSET, XNVM_EFUSE_STS_AES_CRC_DONE_MASK,
					  XNVM_EFUSE_STS_AES_CRC_PASS_MASK, Crc);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_WRITE_AES_KEY);
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs PPK0/1/2 hash based on user request
 *
 * @param	PpkHash - Pointer to XNvm_EfusePpkHash.
 *
 * @return	- XST_SUCCESS	- On successful programming.
 *          - Errorcode on failure
 *
 ******************************************************************************/
static int XNvm_EfusePrgmPpkHash(XNvm_EfusePpkHash *PpkHash)
{
	volatile int Status = XST_FAILURE;
	XNvm_EfusePrgmInfo PpkPrgmInfo = {0U};
	u32 RemainingPpkLen = 0U;
	u32 Ppk1NoofRows = 0U;

	RemainingPpkLen = (PpkHash->ActaulPpkHashSize - XNVM_EFUSE_PPK_HASH_256_SIZE_IN_BYTES);
	Ppk1NoofRows = (RemainingPpkLen != 0U) ? RemainingPpkLen : XNVM_EFUSE_PPK_HASH_256_SIZE_IN_BYTES;
	if (PpkHash->PrgmPpk0Hash ==  (u8)TRUE) {
		PpkPrgmInfo.StartRow = XNVM_EFUSE_PPK0_HASH_START_ROW;
		PpkPrgmInfo.NumOfRows = XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS;
		PpkPrgmInfo.ColStart = XNVM_EFUSE_PPK0_START_COL;
		PpkPrgmInfo.ColEnd = XNVM_EFUSE_PPK0_END_COL;
		PpkPrgmInfo.SkipVerify = (u32)FALSE;

		Status = XNvm_EfusePgmAndVerifyData(&PpkPrgmInfo, (const u32 *)PpkHash->Ppk0Hash);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_PPK0_HASH);
			goto END;
		}
		if (RemainingPpkLen != 0U) {
			PpkPrgmInfo.StartRow = XNVM_EFUSE_PPK1_HASH_START_ROW;
			PpkPrgmInfo.NumOfRows = RemainingPpkLen;
			PpkPrgmInfo.ColStart = XNVM_EFUSE_PPK1_START_COL;
			PpkPrgmInfo.ColEnd = XNVM_EFUSE_PPK1_END_COL;
			PpkPrgmInfo.SkipVerify = (u32)FALSE;

			Status = XST_FAILURE;
			Status = XNvm_EfusePgmAndVerifyData(&PpkPrgmInfo,
							    (const u32 *)((UINTPTR)PpkHash->Ppk0Hash + XNVM_EFUSE_PPK_HASH_256_SIZE_IN_BYTES));
			if (Status != XST_SUCCESS) {
				Status = (Status | XNVM_EFUSE_ERR_WRITE_PPK0_HASH);
				goto END;
			}
		}
	}

	if (PpkHash->PrgmPpk1Hash == (u8)TRUE) {
		PpkPrgmInfo.StartRow = (XNVM_EFUSE_PPK1_HASH_START_ROW + RemainingPpkLen);
		PpkPrgmInfo.NumOfRows = Ppk1NoofRows;
		PpkPrgmInfo.ColStart = XNVM_EFUSE_PPK1_START_COL;
		PpkPrgmInfo.ColEnd = XNVM_EFUSE_PPK1_END_COL;
		PpkPrgmInfo.SkipVerify = (u32)FALSE;

		Status = XNvm_EfusePgmAndVerifyData(&PpkPrgmInfo, (const u32 *)PpkHash->Ppk1Hash);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_PPK1_HASH);
			goto END;
		}

		if (RemainingPpkLen != 0U) {
			PpkPrgmInfo.StartRow = XNVM_EFUSE_PPK2_HASH_START_ROW;
			PpkPrgmInfo.NumOfRows = XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS;
			PpkPrgmInfo.ColStart = XNVM_EFUSE_PPK2_START_COL;
			PpkPrgmInfo.ColEnd = XNVM_EFUSE_PPK2_END_COL;
			PpkPrgmInfo.SkipVerify = (u32)FALSE;

			Status = XST_FAILURE;
			Status = XNvm_EfusePgmAndVerifyData(&PpkPrgmInfo,
							    (const u32 *)((UINTPTR)PpkHash->Ppk1Hash + Ppk1NoofRows));
			if (Status != XST_SUCCESS) {
				Status = (Status | XNVM_EFUSE_ERR_WRITE_PPK1_HASH);
				goto END;
			}
		}
	}

	if (PpkHash->PrgmPpk2Hash == (u8)TRUE) {
		PpkPrgmInfo.StartRow = XNVM_EFUSE_PPK2_HASH_START_ROW;
		PpkPrgmInfo.NumOfRows = XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS;
		PpkPrgmInfo.ColStart = XNVM_EFUSE_PPK2_START_COL;
		PpkPrgmInfo.ColEnd = XNVM_EFUSE_PPK2_END_COL;
		PpkPrgmInfo.SkipVerify = (u32)FALSE;

		Status = XNvm_EfusePgmAndVerifyData(&PpkPrgmInfo, (const u32 *)PpkHash->Ppk2Hash);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_PPK2_HASH);
			goto END;
		}
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs IV in to efuses
 *
 * @param	AesIv - Pointer to XNvm_EfuseAesIvs.
 *
 * @return	- XST_SUCCESS	- On successful programming.
 *          - Errorcode on failure
 *
 ******************************************************************************/
static int XNvm_EfusePrgmIv(const XNvm_EfuseAesIvs *AesIv)
{
	int Status = XST_FAILURE;
	XNvm_EfusePrgmInfo AesIvInfo = {0U};

	AesIvInfo.StartRow = XNVM_EFUSE_AES_IV_START_ROW;
	AesIvInfo.NumOfRows = XNVM_EFUSE_AES_IV_NUM_OF_ROWS;
	AesIvInfo.ColStart = XNVM_EFUSE_AES_IV_START_COL;
	AesIvInfo.ColEnd = XNVM_EFUSE_AES_IV_END_COL;
	AesIvInfo.SkipVerify = (u32)FALSE;

	Status = XNvm_EfusePgmAndVerifyData(&AesIvInfo, AesIv->AesIv);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_WRITE_IV);
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs user fuse
 *
 * @param	UserFuse - Pointer to XNvm_EfuseUserFuse.
 *
 * @return	- XST_SUCCESS	- On successful programming.
 *          - Errorcode on failure
 *
 ******************************************************************************/
static int XNvm_EfusePrgmUserFuse(const XNvm_EfuseUserFuse *UserFuse)
{
	int Status = XST_FAILURE;
	XNvm_EfusePrgmInfo UserFuseInfo = {0U};
	u32 UserFuseVal = 0U;

	Status = XNvm_EfuseComputeProgrammableBits((const u32 *)&UserFuse->UserFuseVal, &UserFuseVal,
		 XNVM_EFUSE_USER_FUSE_OFFSET, XNVM_EFUSE_USER_FUSE_OFFSET);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	UserFuseInfo.StartRow = XNVM_EFUSE_USER_FUSE_START_ROW;
	UserFuseInfo.NumOfRows = XNVM_EFUSE_USER_FUSE_NUM_OF_ROWS;
	UserFuseInfo.ColStart = XNVM_EFUSE_USER_FUSE_START_COL;
	UserFuseInfo.ColEnd = XNVM_EFUSE_USER_FUSE_END_COL;
	UserFuseInfo.SkipVerify = (u32)FALSE;

	Status = XNvm_EfusePgmAndVerifyData(&UserFuseInfo, (const u32 *)&UserFuse->UserFuseVal);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_WRITE_USER_FUSE);
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs SPK revoke id based on user choice
 *
 * @param	SpkRevokeId - Pointer to XNvm_EfuseSpkRevokeId.
 *
 * @return	- XST_SUCCESS	- On successful programming.
 *          - Errorcode on failure
 *
 ******************************************************************************/
static int XNvm_EfusePrgmSpkRevokeId(const XNvm_EfuseSpkRevokeId *SpkRevokeId)
{
	int Status = XST_FAILURE;
	XNvm_EfusePrgmInfo SpkRevokeIdInfo = {0U};
	u32 SpkRevokeIdFuse[XNVM_EFUSE_NUM_OF_REVOKE_ID_FUSES];

	Status = XNvm_EfuseComputeProgrammableBits(SpkRevokeId->RevokeId, SpkRevokeIdFuse,
		 XNVM_EFUSE_SPK_REVOKE_ID_OFFSET,
		 XNVM_EFUSE_SPK_REVOKE_ID_END_OFFSET);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	SpkRevokeIdInfo.StartRow = XNVM_EFUSE_SPK_REVOKE_ID_START_ROW;
	SpkRevokeIdInfo.NumOfRows = XNVM_EFUSE_SPK_REVOKE_ID_NUM_OF_ROWS;
	SpkRevokeIdInfo.ColStart = XNVM_EFUSE_SPK_REVOKE_ID_START_COL;
	SpkRevokeIdInfo.ColEnd = XNVM_EFUSE_SPK_REVOKE_ID_END_COL;
	SpkRevokeIdInfo.SkipVerify = (u32)FALSE;

	Status = XNvm_EfusePgmAndVerifyData(&SpkRevokeIdInfo, (const u32 *)&SpkRevokeIdFuse);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_WRITE_SPK_REVOKE_ID);
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs AES revoke id based on user choice
 *
 * @param	AesRevokeId - Pointer to XNvm_EfuseAesRevokeId.
 *
 * @return	- XST_SUCCESS	- On successful programming.
 *          - Errorcode on failure
 *
 ******************************************************************************/
static int XNvm_EfusePrgmAesRevokeId(const XNvm_EfuseAesRevokeId *AesRevokeId)
{
	int Status = XST_FAILURE;
	XNvm_EfusePrgmInfo AesRevokeIdInfo = {0U};
	u32 AesRevokeIdFuse;

	Status = XNvm_EfuseComputeProgrammableBits((const u32 *)&AesRevokeId->AesRevokeId, &AesRevokeIdFuse,
		 XNVM_EFUSE_AES_REVOKE_ID_OFFSET,
		 XNVM_EFUSE_AES_REVOKE_ID_OFFSET);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	AesRevokeIdInfo.StartRow = XNVM_EFUSE_AES_REVOKE_ID_START_ROW;
	AesRevokeIdInfo.NumOfRows = XNVM_EFUSE_AES_REVOKE_ID_NUM_OF_ROWS;
	AesRevokeIdInfo.ColStart = XNVM_EFUSE_AES_REVOKE_ID_START_COL;
	AesRevokeIdInfo.ColEnd = XNVM_EFUSE_AES_REVOKE_ID_END_COL;
	AesRevokeIdInfo.SkipVerify = (u32)FALSE;

	Status = XNvm_EfusePgmAndVerifyData(&AesRevokeIdInfo, (const u32 *)&AesRevokeIdFuse);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_WRITE_AES_REVOKE_ID);
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs DEC only fuses
 *
 * @param	DecOnly - Pointer to XNvm_EfuseDecOnly.
 *
 * @return	- XST_SUCCESS	- On successful programming.
 *          - Errorcode on failure
 *
 ******************************************************************************/
static int XNvm_EfusePrgmDecOnly(const XNvm_EfuseDecOnly *DecOnly)
{
	int Status = XST_FAILURE;
	XNvm_EfusePrgmInfo DecOnlyInfo = {0U};
	u32 PrgmDecOnly = XNVM_EFUSE_CACHE_DEC_EFUSE_ONLY_MASK;

	if (DecOnly->PrgmDeconly == (u8)TRUE) {
		DecOnlyInfo.StartRow = XNVM_EFUSE_DEC_ONLY_START_ROW;
		DecOnlyInfo.NumOfRows = XNVM_EFUSE_DEC_ONLY_NUM_OF_ROWS;
		DecOnlyInfo.ColStart = XNVM_EFUSE_DEC_ONLY_START_COL;
		DecOnlyInfo.ColEnd = XNVM_EFUSE_DEC_ONLY_END_COL;
		DecOnlyInfo.SkipVerify = (u32)FALSE;

		Status = XNvm_EfusePgmAndVerifyData(&DecOnlyInfo, (const u32 *)&PrgmDecOnly);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_DEC_ONLY);
		}
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function is used to program PUFHD_INVLD fuses.
 *
 * @param	PufHDInvld - Pointer to XNvm_EfuseXilinxCtrl structure which holds
 * 		PufHDInvld flag which says to program eFuse.
 *
 * @return
 *		- XST_SUCCESS  Specified data read.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM  Invalid input parameter.
 *		- XNVM_EFUSE_ERR_WRITE_PUF_HD_INVLD  Error in PUFHD_INVLD
 *						     programming.
 *
 ******************************************************************************/
static int XNvm_EfusePrgmPufHDInvld(const XNvm_EfuseXilinxCtrl *PufHDInvld)
{
	int Status = XST_FAILURE;
	XNvm_EfusePrgmInfo PufHDInvld_Info = {0U};
	u32 PrgmPufHDInvld = XNVM_EFUSE_PUFHD_INVLD_EFUSE_VAL;

	if (PufHDInvld == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	PufHDInvld_Info.StartRow = XNVM_EFUSE_PUF_HD_INVLD_START_ROW;
	PufHDInvld_Info.NumOfRows = XNVM_EFUSE_PUF_HD_INVLD_NUM_OF_ROWS;
	PufHDInvld_Info.ColStart = XNVM_EFUSE_PUF_HD_INVLD_START_COL;
	PufHDInvld_Info.ColEnd = XNVM_EFUSE_PUF_HD_INVLD_END_COL;
	PufHDInvld_Info.SkipVerify = (u32)FALSE;

	Status = XNvm_EfusePgmAndVerifyData(&PufHDInvld_Info, (const u32 *)&PrgmPufHDInvld);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_WRITE_PUFHD_INVLD);
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function is used to program XILINX_CTRL fuses.
 *
 * @param	XilinxCtrl - Pointer to XNvm_EfuseXilinxCtrl structure which holds
 * 		PufHDInvld and PrgmDisSJtag flag which says to program eFuse.
 *
 * @return
 *		- XST_SUCCESS  Specified data read.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM  Invalid input parameter.
 *		- XNVM_EFUSE_ERR_WRITE_PUF_HD_INVLD  Error in PUFHD_INVLD
 *						     programming.
 *		- XNVM_EFUSE_ERR_WRITE_DIS_SJTAG Error in DIS_SJTAG
 *						     programming.
 *
 ******************************************************************************/
static int XNvm_EfusePrgmXilinxCtrl(const XNvm_EfuseXilinxCtrl *XilinxCtrl)
{
	int Status = XST_FAILURE;
	int StatusTmp = XST_FAILURE;
	u32 Data;

	Status =  XNvm_EfuseReadCache(XNVM_EFUSE_XILINX_CTRL_OFFSET, &Data);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((XilinxCtrl->PrgmPufHDInvld == (u32)TRUE) &&
	    ((Data & XNVM_EFUSE_PUFHD_INVLD_EFUSE_MASK) == 0x0U)) {
		Status = XNvm_EfusePrgmPufHDInvld(XilinxCtrl);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	if ((XilinxCtrl->PrgmDisSJtag == (u32)TRUE) &&
	    ((Data & XNVM_EFUSE_DISSJTAG_EFUSE_MASK) != XNVM_EFUSE_DISSJTAG_EFUSE_MASK)) {
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_DIS_SJTAG_ROW,
				      XNVM_EFUSE_DIS_SJTAG_COL, FALSE);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = Status | XNVM_EFUSE_ERR_WRITE_DIS_SJTAG;
			goto END;
		}
	}

END:
	return Status;

}


/******************************************************************************/
/**
 * @brief	This function programs secure control bits
 *
 * @param	SecCtrl - Pointer to XNvm_EfuseSecCtrl.
 *
 * @return	- XST_SUCCESS	- On successful programming.
 *          - Errorcode on failure
 *
 ******************************************************************************/
static int XNvm_EfusePrgmSecCtrlBits(const XNvm_EfuseSecCtrl *SecCtrl)
{
	int Status = XST_FAILURE;
	int StatusTmp = XST_FAILURE;
	XNvm_EfuseSecCtrlBits ReadSecCtrlBits;

	Status =  XNvm_EfuseReadSecCtrlBits(&ReadSecCtrlBits);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNVM_EFUSE_ERR_INVALID_PARAM;

	if ((SecCtrl->PrgmAesDis ==  (u8)TRUE) && (ReadSecCtrlBits.AES_DIS != (u8)TRUE)) {
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_3,
				      XNVM_EFUSE_AES_DIS, FALSE);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = Status | XNVM_EFUSE_ERR_WRITE_AES_DIS;
			goto END;
		}
	}

	if ((SecCtrl->PrgmJtagDis ==  (u8)TRUE) && (ReadSecCtrlBits.JTAG_DIS != (u8)TRUE)) {
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_3,
				      XNVM_EFUSE_JTAG_DIS, FALSE);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = Status | XNVM_EFUSE_ERR_WRITE_JTAG_DIS;
			goto END;
		}
	}

	if ((SecCtrl->PrgmPpk2lck ==  (u8)TRUE) && (ReadSecCtrlBits.PPK2_WR_LK != (u8)TRUE)) {
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_3,
				      XNVM_EFUSE_PPK2_WR_LK, FALSE);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = Status | XNVM_EFUSE_ERR_WRITE_PPK2_WR_LCK;
			goto END;
		}
	}

	if ((SecCtrl->PrgmPpk1lck ==  (u8)TRUE) && (ReadSecCtrlBits.PPK1_WR_LK != (u8)TRUE)) {
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_3,
				      XNVM_EFUSE_PPK1_WR_LK, FALSE);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = Status | XNVM_EFUSE_ERR_WRITE_PPK1_WR_LCK;
			goto END;
		}
	}

	if ((SecCtrl->PrgmPpk0lck ==  (u8)TRUE) && (ReadSecCtrlBits.PPK0_WR_LK != (u8)TRUE)) {
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_3,
				      XNVM_EFUSE_PPK0_WR_LK, FALSE);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = Status | XNVM_EFUSE_ERR_WRITE_PPK0_WR_LCK;
			goto END;
		}
	}

	if ((SecCtrl->PrgmAesRdlk ==  (u8)TRUE) && (ReadSecCtrlBits.AES_RD_WR_LK_0 != (u8)TRUE)
	    && (ReadSecCtrlBits.AES_RD_WR_LK_1 != (u8)TRUE)) {
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_3,
				      XNVM_EFUSE_AES_RD_WR_LK_0, FALSE);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = Status | XNVM_EFUSE_ERR_WRITE_AES_RD_WR_LCK0;
			goto END;
		}
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_3,
				      XNVM_EFUSE_AES_RD_WR_LK_1, FALSE);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = Status | XNVM_EFUSE_ERR_WRITE_AES_RD_WR_LCK1;
			goto END;
		}
	}

	if ((SecCtrl->PrgmPpk2Invld == (u8)TRUE) && (ReadSecCtrlBits.PPK2_INVLD0 != (u8)TRUE)
	    && (ReadSecCtrlBits.PPK2_INVLD1 != (u8)TRUE)) {
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_2,
				      XNVM_EFUSE_PPK2_INVLD_0, FALSE);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = Status | XNVM_EFUSE_ERR_WRITE_PPK2_INVLD_0;
			goto END;
		}
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_2,
				      XNVM_EFUSE_PPK2_INVLD_1, FALSE);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = Status | XNVM_EFUSE_ERR_WRITE_PPK2_INVLD_1;
			goto END;
		}
	}

	if ((SecCtrl->PrgmPpk1Invld == (u8)TRUE) && (ReadSecCtrlBits.PPK1_INVLD0 != (u8)TRUE)
	    && (ReadSecCtrlBits.PPK1_INVLD1 != (u8)TRUE)) {
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_2,
				      XNVM_EFUSE_PPK1_INVLD_0, FALSE);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = Status | XNVM_EFUSE_ERR_WRITE_PPK1_INVLD_0;
			goto END;
		}
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_2,
				      XNVM_EFUSE_PPK1_INVLD_1, FALSE);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = Status | XNVM_EFUSE_ERR_WRITE_PPK1_INVLD_1;
			goto END;
		}
	}

	if ((SecCtrl->PrgmPpk0Invld == (u8)TRUE) && (ReadSecCtrlBits.PPK0_INVLD0 != (u8)TRUE)
	    && (ReadSecCtrlBits.PPK0_INVLD1 != (u8)TRUE)) {
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_2,
				      XNVM_EFUSE_PPK0_INVLD_0, FALSE);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = Status | XNVM_EFUSE_ERR_WRITE_PPK0_INVLD_0;
			goto END;
		}
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_2,
				      XNVM_EFUSE_PPK0_INVLD_1, FALSE);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = Status | XNVM_EFUSE_ERR_WRITE_PPK0_INVLD_1;
			goto END;
		}
	}

	if ((SecCtrl->PrgmPufTes2Dis == (u8)TRUE) && (ReadSecCtrlBits.PUF_TEST2_DIS != (u8)TRUE)) {
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_2,
				      XNVM_EFUSE_PUF_TEST2_DIS, FALSE);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = Status | XNVM_EFUSE_ERR_WRITE_PUF_TEST2_DIS;
			goto END;
		}
	}

	if ((SecCtrl->PrgmPufTes2Dis == (u8)TRUE) && (ReadSecCtrlBits.PUF_TEST2_DIS != (u8)TRUE)) {
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_2,
				      XNVM_EFUSE_PUF_TEST2_DIS, FALSE);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = Status | XNVM_EFUSE_ERR_WRITE_PUF_TEST2_DIS;
			goto END;
		}
	}

	if ((SecCtrl->PrgmRmaEn == (u8)TRUE) && (ReadSecCtrlBits.RMA_ENABLE_0 != (u8)TRUE)
	    && (ReadSecCtrlBits.RMA_ENABLE_1 != (u8)TRUE)) {
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_0,
				      XNVM_EFUSE_RMA_ENABLE_0, FALSE);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = Status | XNVM_EFUSE_ERR_WRITE_RMA_ENABLE_0;
			goto END;
		}

		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_1,
				      XNVM_EFUSE_RMA_ENABLE_1, FALSE);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = Status | XNVM_EFUSE_ERR_WRITE_RMA_ENABLE_1;
			goto END;
		}
	}

	if ((SecCtrl->PrgmRmaDis == (u8)TRUE) && (ReadSecCtrlBits.RMA_DISABLE_0 != (u8)TRUE)
	    && (ReadSecCtrlBits.RMA_DISABLE_1 != (u8)TRUE)) {
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_0,
				      XNVM_EFUSE_RMA_DISABLE_0, FALSE);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = Status | XNVM_EFUSE_ERR_WRITE_RMA_ENABLE_0;
			goto END;
		}

		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_1,
				      XNVM_EFUSE_RMA_DISABLE_1, FALSE);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = Status | XNVM_EFUSE_ERR_WRITE_RMA_ENABLE_1;
			goto END;
		}
	}

	if ((SecCtrl->PrgmLckdwn == (u8)TRUE) && (ReadSecCtrlBits.LCKDOWN != (u8)TRUE)) {
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_1,
				      XNVM_EFUSE_LCKDOWN, FALSE);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = Status | XNVM_EFUSE_ERR_WRITE_PUF_TEST2_DIS;
			goto END;
		}
	}

	if ((SecCtrl->PrgmDftDis == (u8)TRUE) && (ReadSecCtrlBits.DFT_DIS == (u8)FALSE)) {
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_1,
				      XNVM_EFUSE_DFT_DISABLE_0, FALSE);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = Status | XNVM_EFUSE_ERR_WRITE_DFT_DIS_0;
			goto END;
		}
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_1,
				      XNVM_EFUSE_DFT_DISABLE_1, FALSE);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = Status | XNVM_EFUSE_ERR_WRITE_DFT_DIS_1;
			goto END;
		}
	}

	if ((SecCtrl->PrgmCrcEn == (u8)TRUE) && (ReadSecCtrlBits.EFUSE_CRC_EN != (u8)TRUE)) {
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_1,
				      XNVM_EFUSE_CRC_EN, FALSE);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = Status | XNVM_EFUSE_ERR_WRITE_CRC_EN;
			goto END;
		}
	}

	if ((SecCtrl->PrgmHashPufOrKey == (u8)TRUE) && (ReadSecCtrlBits.HASH_PUF_OR_KEY != (u8)TRUE)) {
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_0,
				      XNVM_EFUSE_HASH_PUF_OR_KEY, FALSE);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = Status | XNVM_EFUSE_ERR_WRITE_HASH_PUF_OR_KEY;
			goto END;
		}
	}

	if ((SecCtrl->PrgmUserWrlk == (u8)TRUE) && (ReadSecCtrlBits.USER_WR_LK != (u8)TRUE)) {
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_4,
				      XNVM_EFUSE_USER_WR_LK, FALSE);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = Status | XNVM_EFUSE_ERR_WRITE_USER_WR_LK;
			goto END;
		}
	}

	if ((SecCtrl->PrgmJtagErrDis == (u8)TRUE) && (ReadSecCtrlBits.JTAG_ERR_OUT_DIS != (u8)TRUE)) {
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_4,
				      XNVM_EFUSE_JTAG_ERR_OUT_DIS, FALSE);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = Status | XNVM_EFUSE_ERR_WRITE_JTAG_ERR_OUT_DIS;
			goto END;
		}
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function is used to compute the eFuse bits to be programmed
 * 			to the eFuse.
 *
 * @param	ReqData  - Pointer to the user provided eFuse data to be written.
 * @param	PrgmData - Pointer to the computed eFuse bits to be programmed,
 * 			which means that this API fills only unprogrammed
 * 			and valid bits.
 * @param	StartOffset - A 32 bit Start Cache offset of an expected
 * 			Programmable Bits.
 * @param	EndOffset   - A 32 bit End Cache offset of an expected
 * 			Programmable Bits.
 *
 * @return	- XST_SUCCESS - if the eFuse data computation is successful.
 *			- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *			- XNVM_EFUSE_ERR_CACHE_PARITY  - Error in Cache reload.
 *
 ******************************************************************************/
static int XNvm_EfuseComputeProgrammableBits(const u32 *ReqData, u32 *PrgmData,
	u32 StartOffset, u32 EndOffset)
{
	volatile int Status = XST_FAILURE;
	u32 IsrStatus = XST_FAILURE;
	u32 ReadReg = 0U;
	volatile u32 Offset = 0U;
	u32 Idx = 0U;

	if ((ReqData == NULL) || (PrgmData == NULL)) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	IsrStatus = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
				      XNVM_EFUSE_ISR_OFFSET);
	if ((IsrStatus & XNVM_EFUSE_ISR_CACHE_ERROR)
	    == XNVM_EFUSE_ISR_CACHE_ERROR) {
		Status = XNVM_EFUSE_ERR_CACHE_PARITY;
		goto END;
	}

	Offset = StartOffset;
	while (Offset <= EndOffset) {
		Status = XNvm_EfuseReadCache(Offset, &ReadReg);
		if (Status != XST_SUCCESS) {
			break;
		}
		Idx = (Offset - StartOffset) / XNVM_WORD_LEN;
		PrgmData[Idx] = (~ReadReg) & ReqData[Idx];
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
 * @brief	This function reloads the cache of eFUSE so that can be directly
 * 			read from cache.
 *
 * @return	- XST_SUCCESS - on successful cache reload.
 *			- XNVM_EFUSE_ERR_CACHE_LOAD - Error while loading the cache.
 *
 * @note	Not recommended to call this API frequently,if this API is called
 *		all the cache memory is reloaded by reading eFUSE array,
 *		reading eFUSE bit multiple times may diminish the life time.
 *
 ******************************************************************************/
static int XNvm_EfuseCacheReload(void)
{
	volatile int Status = XST_FAILURE;
	u32 CacheStatus;

	/**
	 * @{ Write 1 to load bit of eFuse_CACHE_LOAD register.
	 *	  Wait for CACHE_DONE bit to set in EFUSE_STATUS register . If timed out return timeout error.
	 *	  Return XST_SUCCESS
	 */
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
			   XNVM_EFUSE_CACHE_LOAD_OFFSET,
			   XNVM_EFUSE_CACHE_LOAD_MASK);

	CacheStatus = Xil_WaitForEvent((UINTPTR)(XNVM_EFUSE_CTRL_BASEADDR +
				       XNVM_EFUSE_STS_OFFSET),
				       XNVM_EFUSE_STS_CACHE_DONE,
				       XNVM_EFUSE_STS_CACHE_DONE,
				       XNVM_EFUSE_CACHE_LOAD_TIMEOUT_VAL);
	if (CacheStatus != (u32)XST_SUCCESS) {
		Status = XNVM_EFUSE_ERR_CACHE_LOAD;
		goto END;
	}

	/**
	 *  @{ Read EFUSE_ISR_REG. If EFUSE_ISR_CHACE_ERROR set return cache load error.
	 *     Return XST_SUCCES.
	 */
	CacheStatus = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_ISR_OFFSET);
	if ((CacheStatus & XNVM_EFUSE_ISR_CACHE_ERROR) ==
	    XNVM_EFUSE_ISR_CACHE_ERROR) {
		Status = XNVM_EFUSE_ERR_CACHE_LOAD;
		goto END;
	}

	Status = XST_SUCCESS;
END:
	/**
	 *  Reset EFUSE_ISR_CACHE_ERROR bit to 1
	 */
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
			   XNVM_EFUSE_ISR_OFFSET,
			   XNVM_EFUSE_ISR_CACHE_ERROR);

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function sets and then verifies the specified bits
 *		in the eFUSE.
 *
 * @param	EfusePrgmInfo - Pointer to XNvm_EfusePrgmInfo structure
 * 				stores the info required to program the eFuses
 * @param	RowData   - Pointer to memory location where bitmap to be
 * 			written is stored. Only bit set are used for programming
 * 			eFUSE.
 *
 * @return	- XST_SUCCESS - Specified bit set in eFUSE.
 *  		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *
 ******************************************************************************/
static int XNvm_EfusePgmAndVerifyData(const XNvm_EfusePrgmInfo *EfusePrgmInfo, const u32 *RowData)
{
	volatile int Status = XNVM_EFUSE_ERR_NTHG_TO_BE_PROGRAMMED;
	const u32 *DataPtr = RowData;
	volatile u32 Row = EfusePrgmInfo->StartRow;
	volatile u32 EndRow = EfusePrgmInfo->StartRow + EfusePrgmInfo->NumOfRows;
	u32 Idx = 0U;
	u32 Col = 0U;
	u32 Data;

	if ((DataPtr == NULL) || (EfusePrgmInfo->NumOfRows == 0U)) {

		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Data = *DataPtr;
	while (Row < EndRow) {
		Col = EfusePrgmInfo->ColStart;
		while (Col <= EfusePrgmInfo->ColEnd) {
			if ((Data & 0x01U) != 0U) {
				Status = XST_FAILURE;
				Status = XNvm_EfusePgmAndVerifyBit(Row, Col, EfusePrgmInfo->SkipVerify);
				if (Status != XST_SUCCESS) {
					XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
					goto END;
				}
			}
			Col++;
			Idx++;
			if (Idx == XNVM_EFUSE_MAX_BITS_IN_ROW) {
				DataPtr++;
				Data = *DataPtr;
				Idx = 0;
			} else {
				Data = Data >> 1U;
			}
		}
		Row++;
	}

	if (Row != EndRow) {
		Status = XNVM_EFUSE_ERR_GLITCH_DETECTED;
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function sets the specified bit in the eFUSE.
 *
 * @param	Row  - It is an 32-bit Row number (0-based addressing).
 * @param	Col  - It is an 32-bit Col number (0-based addressing).
 *
 * @return	- XST_SUCCESS	- Specified bit set in eFUSE.
 *			- XNVM_EFUSE_ERR_PGM_TIMEOUT - eFUSE programming timed out.
 *			- XNVM_EFUSE_ERR_PGM - eFUSE programming failed.
 *
 ******************************************************************************/
static int XNvm_EfusePgmBit(u32 Row, u32 Col)
{
	volatile int Status = XST_FAILURE;
	u32 PgmAddr;
	u32 EventMask = 0U;

	PgmAddr = (Row << XNVM_EFUSE_ADDR_ROW_SHIFT) |
		  (Col << XNVM_EFUSE_ADDR_COLUMN_SHIFT);

	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
			   XNVM_EFUSE_PGM_ADDR_OFFSET, PgmAddr);

	Status = (int)Xil_WaitForEvents((UINTPTR)(XNVM_EFUSE_CTRL_BASEADDR + XNVM_EFUSE_ISR_OFFSET),
					(XNVM_EFUSE_ISR_PGM_DONE | XNVM_EFUSE_ISR_PGM_ERROR),
					(XNVM_EFUSE_ISR_PGM_DONE | XNVM_EFUSE_ISR_PGM_ERROR),
					XNVM_EFUSE_PGM_TIMEOUT_VAL,
					&EventMask);

	if (XST_TIMEOUT == Status) {
		Status = XNVM_EFUSE_ERR_PGM_TIMEOUT;
	} else if ((EventMask & XNVM_EFUSE_ISR_PGM_ERROR) == XNVM_EFUSE_ISR_PGM_ERROR) {
		Status = XNVM_EFUSE_ERR_PGM;
	} else {
		Status = XST_SUCCESS;
	}

	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR, XNVM_EFUSE_ISR_OFFSET,
			   (XNVM_EFUSE_ISR_PGM_DONE | XNVM_EFUSE_ISR_PGM_ERROR));

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function verify the specified bit set in the eFUSE.
 *
 * @param	Row - It is an 32-bit Row number (0-based addressing).
 * @param	RegData - Pointer to the register value.
 *
 * @return	- XST_SUCCESS - Specified bit set in eFUSE.
 *			- XNVM_EFUSE_ERR_PGM_VERIFY  - Verification failed, specified bit
 *						   is not set.
 *			- XNVM_EFUSE_ERR_PGM_TIMEOUT - If Programming timeout has occurred.
 *			- XST_FAILURE                - Unexpected error.
 *
 ******************************************************************************/
static int XNvm_EfuseReadRow(u32 Row, u32 *RegData)
{
	volatile int Status = XST_FAILURE;
	u32 RdAddr;
	u32 EventMask = 0x00U;

	RdAddr = Row << XNVM_EFUSE_ADDR_ROW_SHIFT;

	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
			   XNVM_EFUSE_RD_ADDR_OFFSET, RdAddr);

	Status = (int)Xil_WaitForEvents((UINTPTR)(XNVM_EFUSE_CTRL_BASEADDR +
					XNVM_EFUSE_ISR_OFFSET),
					XNVM_EFUSE_ISR_RD_DONE,
					XNVM_EFUSE_ISR_RD_DONE,
					XNVM_EFUSE_RD_TIMEOUT_VAL,
					&EventMask);
	if (XST_TIMEOUT == Status) {
		Status = XNVM_EFUSE_ERR_RD_TIMEOUT;
		goto END;
	}

	if ((EventMask & XNVM_EFUSE_ISR_RD_DONE) == XNVM_EFUSE_ISR_RD_DONE) {
		*RegData = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					     XNVM_EFUSE_RD_DATA_OFFSET);
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function verify the specified bit set in the eFUSE.
 *
 * @param	Row - It is an 32-bit Row number (0-based addressing).
 * @param	Col - It is an 32-bit Col number (0-based addressing).
 *
 * @return	- XST_SUCCESS - Specified bit set in eFUSE.
 *			- XNVM_EFUSE_ERR_PGM_VERIFY  - Verification failed, specified bit
 *						   is not set.
 *			- XNVM_EFUSE_ERR_PGM_TIMEOUT - If Programming timeout has occurred.
 *			- XST_FAILURE                - Unexpected error.
 *
 ******************************************************************************/
static int XNvm_EfuseVerifyBit(u32 Row, u32 Col)
{
	int Status = XST_FAILURE;
	volatile u32 RegData = 0x00U;

	Status = XNvm_EfuseReadRow(Row, (u32 *)&RegData);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((RegData & (((u32)0x01U) << Col)) != 0U) {
		Status = XST_SUCCESS;
	} else {
		Status = XNVM_EFUSE_ERR_PGM_VERIFY;
	}

	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
			   XNVM_EFUSE_ISR_OFFSET,
			   XNVM_EFUSE_ISR_RD_DONE);
END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function sets and then verifies the specified
 *			bit in the eFUSE.
 *
 * @param	Row  - It is an 32-bit Row number (0-based addressing).
 * @param	Col  - It is an 32-bit Col number (0-based addressing).
 * @param	SkipVerify - Skips verification of bit if set to non zero
 *
 * @return	- XST_SUCCESS - Specified bit set in eFUSE.
 *			- XNVM_EFUSE_ERR_PGM_TIMEOUT - eFUSE programming timed out.
 *			- XNVM_EFUSE_ERR_PGM 	- eFUSE programming failed.
 *			- XNVM_EFUSE_ERR_PGM_VERIFY  - Verification failed, specified bit
 *						is not set.
 *			- XST_FAILURE 	- Unexpected error.
 *
 ******************************************************************************/
static int XNvm_EfusePgmAndVerifyBit(u32 Row, u32 Col, u32 SkipVerify)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	volatile u32 SkipVerifyTmp = SkipVerify;

	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmBit, Row, Col);
	if ((Status == XST_SUCCESS) || (StatusTmp == XST_SUCCESS)) {
		if ((SkipVerify == XNVM_EFUSE_PROGRAM_VERIFY) ||
				(SkipVerifyTmp == XNVM_EFUSE_PROGRAM_VERIFY)) {
			/* Return XST_GLITCH_ERROR in case of glitch */
			if (SkipVerify != SkipVerifyTmp) {
				Status = XST_GLITCH_ERROR;
				goto END;
			}
			Status = XST_FAILURE;
			Status = XNvm_EfuseVerifyBit(Row, Col);
		}
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function performs the CRC check of AES key/User0 key/User1 key
 *
 * @param	CrcRegOffSet - Register offset of respective CRC register
 * @param	CrcDoneMask - Respective CRC done mask in status register
 * @param	CrcPassMask - Respective CRC pass mask in status register
 * @param	Crc - A 32 bit CRC value of an expected AES key.
 *
 * @return	- XST_SUCCESS - On successful CRC check.
 *			- XNVM_EFUSE_ERR_CRC_VERIFICATION - If AES boot key integrity
 *							check is failed.
 *			- XST_FAILURE - If AES boot key integrity check
 *							has not finished.
 *
 * @note	Expected CRC calculation steps:
 *		1. Convert the key provided at XNVM_EFUSE_AES_KEY into hexadecimal little-endian format.
 *		2. Calculate the CRC on this little-endian formatted AES key using XNvm_AesCrcCalc() function.
 *		3. Provide the calculated CRC as the value of expected CRC.
 *
 ******************************************************************************/
int XNvm_EfuseCheckAesKeyCrc(u32 CrcRegOffSet, u32 CrcDoneMask, u32 CrcPassMask, u32 Crc)
{
	int Status = XST_FAILURE;
	int LockStatus = XST_FAILURE;
	u32 ReadReg;
	u32 IsUnlocked = (u32)FALSE;

	/**
	 *  Read the WR_LOCK_REG. Unlock the controller if read as locked
	 */
	ReadReg = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
				    XNVM_EFUSE_WR_LOCK_OFFSET);
	if (XNVM_EFUSE_WRITE_LOCKED == ReadReg) {
		Status = XNvm_EfuseUnlockController();
		if (Status != XST_SUCCESS) {
			goto END;
		}
		IsUnlocked = (u32)TRUE;
	}

	/**
	 *  Write the crc to crcregoffset of eFuse_ctrl register
	 */
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR, CrcRegOffSet, Crc);

	/**
	 *  Wait for crcdone
	 */
	Status = (int)Xil_WaitForEvent((UINTPTR)(XNVM_EFUSE_CTRL_BASEADDR + XNVM_EFUSE_STS_OFFSET),
				       CrcDoneMask, CrcDoneMask, XNVM_POLL_TIMEOUT);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 *  Read efuse status register. If Crc is not done return XST_FAILURE
	 */
	ReadReg = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
				    XNVM_EFUSE_STS_OFFSET);

	if ((ReadReg & CrcDoneMask) != CrcDoneMask) {
		Status = XST_FAILURE;
	}
	/**
	 *  Return XNVM_EFUSE_ERR_CRC_VERIFICATION if Crc is not Pass. Return XST_SUCCESS upon crc pass and done
	 */
	else if ((ReadReg & CrcPassMask) != CrcPassMask) {
		Status = XNVM_EFUSE_ERR_CRC_VERIFICATION;
	} else {
		Status = XST_SUCCESS;
	}
END:
	/**
	 *  Lock efuse controller
	 */
	if (IsUnlocked == (u32)TRUE) {
		LockStatus = XNvm_EfuseLockController();
		if (XST_SUCCESS == Status) {
			Status = LockStatus;
		}
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function reads 32-bit data from cache specified by Row.
 *
 * @param	Offset 	- Offset from which data should be read (0-based addressing).
 * @param	RowData	- Pointer to memory location where read 32-bit row data
 *					  is to be stored.
 *
 * @return	- XST_SUCCESS - Specified data read.
 *			- XNVM_EFUSE_ERR_CACHE_PARITY - Parity Error exist in cache.
 *
 ******************************************************************************/
static int XNvm_EfuseReadCache(u32 Offset, u32 *RowData)
{
	int Status = XST_FAILURE;
	u32 CacheData;
	u32 IsrStatus;

	if (RowData == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	CacheData = Xil_In32(XNVM_EFUSE_CTRL_BASEADDR + Offset);
	IsrStatus = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
				      XNVM_EFUSE_ISR_OFFSET);
	if ((IsrStatus & XNVM_EFUSE_ISR_CACHE_ERR_MASK)
	    == XNVM_EFUSE_ISR_CACHE_ERR_MASK) {
		Status = XNVM_EFUSE_ERR_CACHE_PARITY;
		goto END;
	}
	*RowData = CacheData;
	Status = XST_SUCCESS;

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function reads 32-bit rows from eFUSE cache.
 *
 * @param	StartOffset - Starting Row number (0-based addressing).
 * @param	OffsetCount - Number of offset to be read.
 * @param	RowData  - Pointer to memory location where read 32-bit row data(s)
 *					   is to be stored.
 *
 * @return	- XST_SUCCESS	- Specified data read.
 *			- XNVM_EFUSE_ERR_CACHE_PARITY - Parity Error exist in cache.
 *
 ******************************************************************************/
static int XNvm_EfuseReadCacheRange(u32 StartOffset, u8 OffsetCount, u32 *RowData)
{
	volatile int Status = XST_FAILURE;
	u32 Row = StartOffset;
	u32 Count;
	u32 *Data = RowData;

	for (Count = 0; Count < OffsetCount; Count++) {
		Status = XST_FAILURE;
		Status = XNvm_EfuseReadCache(Row, Data);
		if (Status != XST_SUCCESS) {
			break;
		}
		Row += XNVM_EFUSE_WORD_LEN;
		Data++;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function reads secure control bits and CRC_EN register bits from eFUSE cache.
 *
 * @param	SecCtrlBits - Pointer to XNvm_EfuseSecCtrlBits where secure control
 *                        bits are read.
 *
 * @return	- XST_SUCCESS	- Specified data read.
 *			- XNVM_EFUSE_ERR_CACHE_PARITY - Parity Error exist in cache.
 *
 ******************************************************************************/
int XNvm_EfuseReadSecCtrlBits(XNvm_EfuseSecCtrlBits *SecCtrlBits)
{
	int Status = XST_FAILURE;
	u32 SecCtrlVal = 0U;

	Status = XNvm_EfuseReadCache(XNVM_EFUSE_CONTROL_OFFSET, &SecCtrlVal);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	SecCtrlBits->AES_DIS = (u8)XNVM_GET_8_BIT_VAL(SecCtrlVal, 1U, 1U);
	SecCtrlBits->RMA_DISABLE_0 = (u8)XNVM_GET_8_BIT_VAL(SecCtrlVal, 1U, 6U);
	SecCtrlBits->RMA_ENABLE_0 = (u8)XNVM_GET_8_BIT_VAL(SecCtrlVal, 1U, 7U);
	SecCtrlBits->JTAG_DIS = (u8)XNVM_GET_8_BIT_VAL(SecCtrlVal, 1U, 8U);
	SecCtrlBits->PUF_TEST2_DIS = (u8)XNVM_GET_8_BIT_VAL(SecCtrlVal, 1U, 9U);
	SecCtrlBits->HASH_PUF_OR_KEY = (u8)XNVM_GET_8_BIT_VAL(SecCtrlVal, 1U, 10U);
	SecCtrlBits->PPK2_WR_LK = (u8)XNVM_GET_8_BIT_VAL(SecCtrlVal, 1U, 11U);
	SecCtrlBits->PPK2_INVLD0 = (u8)XNVM_GET_8_BIT_VAL(SecCtrlVal, 1U, 12U);
	SecCtrlBits->PPK1_WR_LK = (u8)XNVM_GET_8_BIT_VAL(SecCtrlVal, 1U, 13U);
	SecCtrlBits->PPK1_INVLD0 = (u8)XNVM_GET_8_BIT_VAL(SecCtrlVal, 1U, 14U);
	SecCtrlBits->PPK0_WR_LK = (u8)XNVM_GET_8_BIT_VAL(SecCtrlVal, 1U, 15U);
	SecCtrlBits->PPK0_INVLD0 = (u8)XNVM_GET_8_BIT_VAL(SecCtrlVal, 1U, 16U);
	SecCtrlBits->AES_RD_WR_LK_0 = (u8)XNVM_GET_8_BIT_VAL(SecCtrlVal, 1U, 17U);
	SecCtrlBits->AES_RD_WR_LK_1 = (u8)XNVM_GET_8_BIT_VAL(SecCtrlVal, 1U, 18U);
	SecCtrlBits->JTAG_ERR_OUT_DIS = (u8)XNVM_GET_8_BIT_VAL(SecCtrlVal, 1U, 26U);
	SecCtrlBits->USER_WR_LK = (u8)XNVM_GET_8_BIT_VAL(SecCtrlVal, 1U, 27U);
	SecCtrlBits->PPK2_INVLD1 = (u8)XNVM_GET_8_BIT_VAL(SecCtrlVal, 1U, 29U);
	SecCtrlBits->PPK1_INVLD1 = (u8)XNVM_GET_8_BIT_VAL(SecCtrlVal, 1U, 30U);
	SecCtrlBits->PPK0_INVLD1 = (u8)XNVM_GET_8_BIT_VAL(SecCtrlVal, 1U, 31U);

	Status = XNvm_EfuseReadCache(XNVM_EFUSE_CRC_EN_OFFSET, &SecCtrlVal);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	SecCtrlBits->EFUSE_CRC_EN = (u8)XNVM_GET_8_BIT_VAL(SecCtrlVal, 1U, 0U);
	SecCtrlBits->DFT_DIS = (u8)XNVM_GET_8_BIT_VAL(SecCtrlVal, 2U, 1U);
	SecCtrlBits->LCKDOWN = (u8)XNVM_GET_8_BIT_VAL(SecCtrlVal, 1U, 3U);
	SecCtrlBits->RMA_DISABLE_1 = (u8)XNVM_GET_8_BIT_VAL(SecCtrlVal, 1U, 4U);
	SecCtrlBits->RMA_ENABLE_1 = (u8)XNVM_GET_8_BIT_VAL(SecCtrlVal, 1U, 5U);

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function reads PPK0/1/2 hash based on XNvm_EfusePpkType
 *          from eFUSE cache.
 *
 * @param	PpkType - is of type XNvm_EfusePpkType i.e. PPK0/1/2.
 * @param	PpkData - Pointer to the PPK data.
 * @param	PpkSize - Size of PPK it is either 32 or 48 bytes.
 *
 * @return	- XST_SUCCESS	- Specified data read.
 *          - Errorcode on failure
 *
 ******************************************************************************/
int XNvm_EfuseReadPpkHash(XNvm_EfusePpkType PpkType, u32 *PpkData, u32 PpkSize)
{
	int Status = XST_FAILURE;
	u32 PpkStartOffset = 0U;
	u32 OffsetCnt = 0U;

	if (PpkData ==  NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if ((PpkSize != XNVM_EFUSE_PPK_HASH_256_SIZE_IN_BYTES)
	    && (PpkSize != XNVM_EFUSE_PPK_HASH_384_SIZE_IN_BYTES)) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if ((PpkSize == XNVM_EFUSE_PPK_HASH_384_SIZE_IN_BYTES) && (PpkType == XNVM_EFUSE_PPK2)) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (PpkType ==  XNVM_EFUSE_PPK0) {
		PpkStartOffset = XNVM_EFUSE_PPK0_START_OFFSET;
		OffsetCnt = PpkSize / XNVM_EFUSE_WORD_LEN;
	} else if (PpkType == XNVM_EFUSE_PPK1) {
		PpkStartOffset = XNVM_EFUSE_PPK1_START_OFFSET;
		OffsetCnt = PpkSize / XNVM_EFUSE_WORD_LEN;
	} else if (PpkType == XNVM_EFUSE_PPK2) {
		PpkStartOffset = XNVM_EFUSE_PPK2_START_OFFSET;
		OffsetCnt = PpkSize / XNVM_EFUSE_WORD_LEN;
	} else {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfuseReadCacheRange(PpkStartOffset, (u8)OffsetCnt, PpkData);

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function reads SPK revoke id from eFUSE cache.
 *
 * @param	SpkRevokeData - is pointer to SPK revoke data.
 * @param	SpkRevokeRow  - SPK revoke row to be read it can be either 0/1/2.
 *
 * @return	- XST_SUCCESS	- Specified data read.
 *          - Errorcode on failure
 *
 ******************************************************************************/
int XNvm_EfuseReadSpkRevokeId(u32 *SpkRevokeData, u32 SpkRevokeRow)
{
	int Status = XST_FAILURE;

	if (SpkRevokeData == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (SpkRevokeRow >= XNVM_EFUSE_MAX_SPK_REVOKE_ID) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfuseReadCache(XNVM_EFUSE_SPK_REVOKE_ID_OFFSET + (SpkRevokeRow * XNVM_EFUSE_WORD_LEN),
				     SpkRevokeData);

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function reads AES revoke id from eFUSE cache.
 *
 * @param	AesRevokeData - is pointer to AES revoke data.
 *
 * @return	- XST_SUCCESS	- Specified data read.
 *          - Errorcode on failure
 *
 ******************************************************************************/
int XNvm_EfuseReadAesRevokeId(u32 *AesRevokeData)
{
	int Status = XST_FAILURE;

	if (AesRevokeData == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfuseReadCache(XNVM_EFUSE_AES_REVOKE_ID_OFFSET, AesRevokeData);

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function reads user efuses from eFUSE cache.
 *
 * @param	UserFuseData - is pointer to AES revoke data.
 *
 * @return	- XST_SUCCESS	- Specified data read.
 *          - Errorcode on failure
 *
 ******************************************************************************/
int XNvm_EfuseReadUserFuse(u32 *UserFuseData)
{
	int Status = XST_FAILURE;

	if (UserFuseData == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfuseReadCache(XNVM_EFUSE_USER_FUSE_OFFSET, UserFuseData);

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function reads IV based on XNvm_EfuseIvType from eFUSE cache.
 *
 * @param	IvType - is of type XNvm_EfuseIvType.
 * @param	IvData - Pointer to the iv data.
 *
 * @return	- XST_SUCCESS	- Specified data read.
 *          - Errorcode on failure
 *
 ******************************************************************************/
int XNvm_EfuseReadIv(XNvm_EfuseIvType IvType, u32 *IvData)
{
	int Status = XST_FAILURE;
	u32 IvStartOffset = 0U;

	if (IvData ==  NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (IvType ==  XNVM_EFUSE_IV_RANGE) {
		IvStartOffset = XNVM_EFUSE_IV_RANGE_START_OFFSET;
	} else if (IvType == XNVM_EFUSE_BLACK_IV) {
		IvStartOffset = XNVM_EFUSE_IV_START_OFFSET;
	} else {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfuseReadCacheRange(IvStartOffset, XNVM_EFUSE_AES_IV_SIZE_IN_WORDS, IvData);

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function reads DNA from eFUSE cache.
 *
 * @param	Dna - Pointer to the DNA data.
 *
 * @return	- XST_SUCCESS	- Specified data read.
 *          - Errorcode on failure
 *
 ******************************************************************************/
int XNvm_EfuseReadDna(u32 *Dna)
{
	int Status = XST_FAILURE;

	if (Dna ==  NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfuseReadCacheRange(XNVM_EFUSE_DNA_OFFSET, XNVM_EFUSE_DNA_SIZE_IN_WORDS, Dna);

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function reads DEC only fuses from eFUSE cache.
 *
 * @param	DecOnly - Pointer to the DEC only efuse data.
 *
 * @return	- XST_SUCCESS	- Specified data read.
 *          - Errorcode on failure
 *
 ******************************************************************************/
int XNvm_EfuseReadDecOnly(u32 *DecOnly)
{
	int Status = XST_FAILURE;

	if (DecOnly ==  NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfuseReadCache(XNVM_EFUSE_DEC_ONLY_OFFSET, DecOnly);

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function reads Xilinx Ctrl fuse from eFUSE cache.
 *
 * @param	XilinxCtrl - Pointer to the XilinxCtrl efuse data.
 *
 * @return	- XST_SUCCESS	- Specified data read.
 *          - Errorcode on failure
 *
 ******************************************************************************/
int XNvm_EfuseReadXilinxCtrl(XNvm_EfuseXilinxCtrl *XilinxCtrl)
{
	int Status = XST_FAILURE;
	u32 Data = 0U;

	if (XilinxCtrl ==  NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfuseReadCache(XNVM_EFUSE_XILINX_CTRL_OFFSET, &Data);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XilinxCtrl->PrgmPufHDInvld = XNVM_GET_8_BIT_VAL(Data, XNVM_EFUSE_PUFHD_INVLD_EFUSE_BITS,
							XNVM_EFUSE_PUFHD_INVLD_EFUSE_SHIFT);
	XilinxCtrl->PrgmDisSJtag = XNVM_GET_8_BIT_VAL(Data, XNVM_EFUSE_DISSJTAG_EFUSE_BITS,
							XNVM_EFUSE_DISSJTAG_EFUSE_SHIFT);
END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function is used verify eFUSEs for Zeros.
 *
 * @param	OffsetStart - Row number from which verification has to be started.
 * @param	OffsetEnd   - Row number till which verification has to be ended.
 *
 * @return	- XST_SUCCESS - if efuses are not programmed.
 * 			- XST_FAILURE - if efuses are already programmed.
 *
 ******************************************************************************/
static int XNvm_EfuseCheckZeros(u32 OffsetStart, u32 OffsetEnd)
{
	volatile int Status = XST_FAILURE;
	u32 Row;
	u32 RowDataVal = 0x0U;

	for (Row = OffsetStart; Row < OffsetEnd; Row = Row + XNVM_EFUSE_WORD_LEN) {
		Status = XST_FAILURE;
		Status  = XNvm_EfuseReadCache(Row, &RowDataVal);
		if (Status != XST_SUCCESS) {
			break;
		}

		if (RowDataVal != 0x00U) {
			Status = XST_FAILURE;
			break;
		}
	}

	return Status;
}
