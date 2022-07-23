/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xnvm_efuse.c
*
* This file contains the xilnvm server APIs implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kal  12/07/2022 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_util.h"
#include "xnvm_efuse.h"
#include "xnvm_efuse_common_hw.h"
#include "xnvm_efuse_hw.h"
#include "xnvm_utils.h"
#include "xnvm_validate.h"

/**************************** Type Definitions *******************************/
typedef struct {
	u32 StartRow;
	u32 ColStart;
	u32 ColEnd;
	u32 NumOfRows;
	u32 SkipVerify;
	XNvm_EfuseType EfuseType;
}XNvm_EfusePrgmInfo;

/************************** Function Prototypes ******************************/
static int XNvm_EfusePrgmIv(XNvm_IvType IvType, XNvm_Iv *EfuseIv);
static int XNvm_EfusePrgmPpkHash(XNvm_PpkType PpkType, XNvm_PpkHash *EfuseHash);
static int XNvm_EfusePrgmAesKey(XNvm_AesKeyType KeyType, XNvm_AesKey *EfuseKey);
static int XNvm_EfuseComputeProgrammableBits(const u32 *ReqData, u32 *PrgmData,
		u32 StartAddr, u32 EndAddr);
static int XNvm_EfusePgmAndVerifyData(XNvm_EfusePrgmInfo *EfuseData,
		const u32* RowData);
static int XNvm_EfusePgmBit(XNvm_EfuseType Page, u32 Row, u32 Col);
static int XNvm_EfuseVerifyBit(XNvm_EfuseType Page, u32 Row, u32 Col);
static int XNvm_EfusePgmAndVerifyBit(XNvm_EfuseType Page, u32 Row, u32 Col,
		u32 SkipVerify);
static int XNvm_EfuseCloseController(void);

/************************** Constant Definitions *****************************/
#define XNVM_CACHE_START_ADDRESS 	(0xF1250010U)
#define XNVM_CACHE_MAX_ADDRESS		(0xF1250BFCU)

/***************** Macros (Inline Functions) Definitions *********************/

/*************************** Function Prototypes ******************************/

/************************** Function Definitions *****************************/

/******************************************************************************/
/**
 * @brief	This function is used to take care of prerequisites to
 * 		program below eFuses
 * 		AES key/User key 0/User key 1.
 *
 * @param	KeyType - Type of key to be programmed
 * 			(AesKey/UserKey0/UserKey1)
 * @param	EfuseKey - Pointer to the XNvm_AesKey struture, which holds
 * 			Aes key to be programmed to eFuse.
 *
 * @return	- XST_SUCCESS - On Successful Write.
 *		- XNVM_EFUSE_ERR_BEFORE_PROGRAMMING - Error before programming
 *							eFuse.
 *		- XNVM_EFUSE_ERR_WRITE_AES_KEY	 - Error while writing
 *							AES key.
 *		- XNVM_EFUSE_ERR_WRITE_USER_KEY0 - Error while writing
 *							User key 0.
 * 		- XNVM_EFUSE_ERR_WRITE_USER_KEY1 - Error while writing
 *							User key 1.
 *
 ******************************************************************************/
int XNvm_EfuseWriteAesKey(XNvm_AesKeyType KeyType, XNvm_AesKey *EfuseKey)
{
	volatile int Status = XST_FAILURE;
	int CloseStatus = XST_FAILURE;

	if ((KeyType != XNVM_EFUSE_AES_KEY) &&
		(KeyType != XNVM_EFUSE_USER_KEY_0) &&
		(KeyType != XNVM_EFUSE_USER_KEY_1)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	if (EfuseKey == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
					XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
	Status = XNvm_EfuseValidateAesKeyWriteReq(KeyType);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	Status = XST_FAILURE;
	Status = XNvm_EfusePrgmAesKey(KeyType, EfuseKey);

END:
	CloseStatus = XNvm_EfuseCloseController();
	if (XST_SUCCESS == Status) {
		Status = CloseStatus;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function is used to to take care of prerequisitis to
 * 		program below eFuses
 * 		PPK0/PPK1/PPK2 hash.
 *
 * @param	PpkType - Type of ppk hash to be programmed(PPK0/PPK1/PPK2)
 * @param	EfuseHash - Pointer to the XNvm_PpkHash structure which holds
 * 			PPK hash to be programmed to eFuse.
 *
 * @return	- XST_SUCCESS - On Successful Write.
 *		- XNVM_EFUSE_ERR_WRITE_PPK0_HASH - Error while writing PPK0 Hash.
 *		- XNVM_EFUSE_ERR_WRITE_PPK1_HASH - Error while writing PPK1 Hash.
 * 		- XNVM_EFUSE_ERR_WRITE_PPK2_HASH - Error while writing PPK2 Hash.
 *
 ******************************************************************************/
int XNvm_EfuseWritePpkHash(XNvm_PpkType PpkType, XNvm_PpkHash *EfuseHash)
{
	volatile int Status = XST_FAILURE;
	int CloseStatus = XST_FAILURE;

	if ((PpkType != XNVM_EFUSE_PPK0) &&
		(PpkType != XNVM_EFUSE_PPK1) &&
		(PpkType != XNVM_EFUSE_PPK2)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	if (EfuseHash == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
					XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
	Status = XNvm_EfuseValidatePpkHashWriteReq(PpkType);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	Status = XST_FAILURE;
	Status = XNvm_EfusePrgmPpkHash(PpkType, EfuseHash);

END:
	CloseStatus = XNvm_EfuseCloseController();
	if (XST_SUCCESS == Status) {
		Status = CloseStatus;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function is used to to take care of prerequisitis to
 * 		program below IV eFuses
 *		XNVM_EFUSE_ERR_WRITE_META_HEADER_IV /
 *		XNVM_EFUSE_ERR_WRITE_BLK_OBFUS_IV /
 *		XNVM_EFUSE_ERR_WRITE_PLM_IV /
 *		XNVM_EFUSE_ERR_WRITE_DATA_PARTITION_IV
 *
 * @param	IvType - Type of IV eFuses to be programmmed
 * @param	EfuseIv - Pointer to the XNvm_EfuseIvs struture which holds IV
 * 			to be programmed to eFuse.
 *
 * @return	- XST_SUCCESS - On Successful Write.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM           - On Invalid Parameter.
 * 		- XNVM_EFUSE_ERR_WRITE_META_HEADER_IV    - Error while writing
 *							   Meta Iv.
 *		- XNVM_EFUSE_ERR_WRITE_BLK_OBFUS_IV 	 - Error while writing
 *							   BlkObfus IV.
 *		- XNVM_EFUSE_ERR_WRITE_PLM_IV 		 - Error while writing
 *							   PLM IV.
 * 		- XNVM_EFUSE_ERR_WRITE_DATA_PARTITION_IV - Error while writing
 * 							Data Partition IV.
 *
 ******************************************************************************/
int XNvm_EfuseWriteIv(XNvm_IvType IvType, XNvm_Iv *EfuseIv)
{
	volatile int Status = XST_FAILURE;
	int CloseStatus = XST_FAILURE;

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

	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
					XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
	Status = XNvm_EfuseValidateIvWriteReq(IvType, EfuseIv);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	Status = XST_FAILURE;
	Status = XNvm_EfusePrgmIv(IvType, EfuseIv);

END:
	CloseStatus = XNvm_EfuseCloseController();
	if (XST_SUCCESS == Status) {
		Status = CloseStatus;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function reads 32-bit rows from eFUSE cache.
 *
 * @param	StartOffset  - Start Cache Offset of the eFuse.
 * @param	RowCount - Number of 32 bit Rows to be read.
 * @param	RowData  - Pointer to memory location where read 32-bit row data(s)
 *					   is to be stored.
 *
 * @return	- XST_SUCCESS	- On successful Read.
 *		- XNVM_EFUSE_ERR_CACHE_PARITY - Parity Error exist in cache.
 *
 ******************************************************************************/
int XNvm_EfuseReadCacheRange(u32 StartOffset, u8 RegCount, u32* Data)
{
	volatile int Status = XST_FAILURE;
	u32 RegAddress = XNVM_EFUSE_CACHE_BASEADDR + StartOffset;
	u32 NumOfBytes = RegCount * XNVM_WORD_LEN;

	Status = Xil_SMemCpy((void *)Data, NumOfBytes,
			(const void *)RegAddress, NumOfBytes, NumOfBytes);

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function reloads the cache of eFUSE so that can be directly
 * 		read from cache and programs required protections eFuses.
 *
 * @return	- XST_SUCCESS - On Successful Cache Reload.
 *		- XNVM_EFUSE_ERR_CACHE_LOAD - Error while loading the cache.
 *
 * @note	Not recommended to call this API frequently,if this API is called
 *		all the cache memory is reloaded by reading eFUSE array,
 *		reading eFUSE bit multiple times may diminish the life time.
 *
 ******************************************************************************/
int XNvm_EfuseCacheLoadNPrgmProtectionBits(void)
{
	int Status = XST_FAILURE;

	Status = XNvm_EfuseCacheReload();

	//TODO protection eFuse programming to be done

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function performs all close operation of eFuse controller,
 * 		resets the read mode, disables programming mode	and locks the
 * 		controller back.
 *
 * @return	- XST_SUCCESS - On Successful Close.
 *		- XST_FAILURE - On Failure.
 *
 ******************************************************************************/
static int XNvm_EfuseCloseController(void)
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
 * @brief	This function is used to program below IV eFuses
 *		XNVM_EFUSE_ERR_WRITE_META_HEADER_IV /
 *		XNVM_EFUSE_ERR_WRITE_BLK_OBFUS_IV /
 *		XNVM_EFUSE_ERR_WRITE_PLM_IV /
 *		XNVM_EFUSE_ERR_WRITE_DATA_PARTITION_IV
 *
 * @param	IvType - Type of IV eFuses to be programmmed
 * @param	Ivs - Pointer to the XNvm_EfuseIvs struture which holds IV
 * 			to be programmed to eFuse.
 *
 * @return	- XST_SUCCESS - On Successful Programming.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM           - On Invalid Parameter.
 * 		- XNVM_EFUSE_ERR_WRITE_META_HEADER_IV    - Error while writing
 *							   Meta Iv.
 *		- XNVM_EFUSE_ERR_WRITE_BLK_OBFUS_IV 	 - Error while writing
 *							   BlkObfus IV.
 *		- XNVM_EFUSE_ERR_WRITE_PLM_IV 		 - Error while writing
 *							   PLM IV.
 * 		- XNVM_EFUSE_ERR_WRITE_DATA_PARTITION_IV - Error while writing
 * 							Data Partition IV.
 *
 ******************************************************************************/
static int XNvm_EfusePrgmIv(XNvm_IvType IvType, XNvm_Iv *EfuseIv)
{
	volatile int Status = XST_FAILURE;
	u32 PrgmIv[XNVM_EFUSE_IV_NUM_OF_CACHE_ROWS] = {0U};
	XNvm_EfusePrgmInfo EfuseData = {0U};
	u32 IvOffset = 0U;
	u32 Row = 0U;
	u32 StartColNum = 0U;
	u32 EndColNum = 0U;
	u32 NumOfRows = 0U;

	if (IvType == XNVM_EFUSE_META_HEADER_IV_RANGE) {
		IvOffset = XNVM_EFUSE_CACHE_METAHEADER_IV_RANGE_OFFSET;
		Row = XNVM_EFUSE_META_HEADER_IV_START_ROW;
		StartColNum = XNVM_EFUSE_METAHEADER_IV_RANGE_START_COL_NUM;
		EndColNum = XNVM_EFUSE_METAHEADER_IV_RANGE_END_COL_NUM;
		NumOfRows = XNVM_EFUSE_METAHEADER_IV_NUM_OF_ROWS;

	}
	else if (IvType == XNVM_EFUSE_BLACK_IV) {
		IvOffset = XNVM_EFUSE_CACHE_BLACK_IV_OFFSET;
		Row = XNVM_EFUSE_BLACK_IV_START_ROW;
		StartColNum = XNVM_EFUSE_BLACK_IV_START_COL_NUM;
		EndColNum = XNVM_EFUSE_BLACK_IV_END_COL_NUM;
		NumOfRows = XNVM_EFUSE_BLACK_IV_NUM_OF_ROWS;

	}
	else if (IvType == XNVM_EFUSE_PLM_IV_RANGE) {
		IvOffset = XNVM_EFUSE_CACHE_PLM_IV_RANGE_OFFSET;
		Row = XNVM_EFUSE_PLM_IV_START_ROW;
		StartColNum = XNVM_EFUSE_PLM_IV_RANGE_START_COL_NUM;
		EndColNum = XNVM_EFUSE_PLM_IV_RANGE_END_COL_NUM;
		NumOfRows = XNVM_EFUSE_PLM_IV_NUM_OF_ROWS;

	}
	else if (IvType == XNVM_EFUSE_DATA_PARTITION_IV_RANGE) {
		IvOffset = XNVM_EFUSE_CACHE_DATA_PARTITION_IV_OFFSET;
		Row = XNVM_EFUSE_DATA_PARTITION_IV_START_ROW;
		StartColNum = XNVM_EFUSE_DATA_PARTITION_IV_START_COL_NUM;
		EndColNum = XNVM_EFUSE_DATA_PARTITION_IV_END_COL_NUM;
		NumOfRows = XNVM_EFUSE_DATA_PARTITION_IV_NUM_OF_ROWS;

	}
	else {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfuseComputeProgrammableBits(EfuseIv->Iv, PrgmIv,
			IvOffset, (IvOffset +
			(XNVM_EFUSE_IV_NUM_OF_CACHE_ROWS * XNVM_WORD_LEN)));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	EfuseData.StartRow = Row;
	EfuseData.ColStart = StartColNum;
	EfuseData.ColEnd = EndColNum;
	EfuseData.NumOfRows = NumOfRows;
	EfuseData.EfuseType = XNVM_EFUSE_PAGE_0;

	Status = XST_FAILURE;
	Status = XNvm_EfusePgmAndVerifyData(&EfuseData, PrgmIv);

	if (Status != XST_SUCCESS) {
		Status = (Status |
				(XNVM_EFUSE_ERR_WRITE_META_HEADER_IV_RANGE +
				(IvType << XNVM_EFUSE_ERROR_BYTE_SHIFT)));
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function is used to program below eFuses
 * 		PPK0/PPK1/PPK2 hash.
 *
 * @param	PpkType - Type of ppk hash to be programmed(PPK0/PPK1/PPK2)
 * @param	EfuseHash - Pointer to the XNvm_PpkHash structure which holds
 * 			PPK hash to be programmed to eFuse.
 *
 * @return	- XST_SUCCESS - On Successful Programming.
 *		- XNVM_EFUSE_ERR_WRITE_PPK0_HASH - Error while writing PPK0 Hash.
 *		- XNVM_EFUSE_ERR_WRITE_PPK1_HASH - Error while writing PPK1 Hash.
 * 		- XNVM_EFUSE_ERR_WRITE_PPK2_HASH - Error while writing PPK2 Hash.
 *
 ******************************************************************************/
static int XNvm_EfusePrgmPpkHash(XNvm_PpkType PpkType, XNvm_PpkHash *EfuseHash)
{
	int Status = XST_FAILURE;
	XNvm_EfusePrgmInfo EfuseData = {0U};
	u32 Row = 0U;
	u32 StartColNum = 0U;
	u32 EndColNum = 0U;

	if (EfuseHash == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if(PpkType == XNVM_EFUSE_PPK0) {
		Row = XNVM_EFUSE_PPK0_HASH_START_ROW;
		StartColNum = XNVM_EFUSE_PPK0_HASH_START_COL_NUM;
		EndColNum = XNVM_EFUSE_PPK0_HASH_END_COL_NUM;
	}
	else if(PpkType == XNVM_EFUSE_PPK1) {
		Row = XNVM_EFUSE_PPK1_HASH_START_ROW;
		StartColNum = XNVM_EFUSE_PPK1_HASH_START_COL_NUM;
		EndColNum = XNVM_EFUSE_PPK1_HASH_END_COL_NUM;
	}
	else if (PpkType == XNVM_EFUSE_PPK2) {
		Row = XNVM_EFUSE_PPK2_HASH_START_ROW;
		StartColNum = XNVM_EFUSE_PPK2_HASH_START_COL_NUM;
		EndColNum = XNVM_EFUSE_PPK2_HASH_END_COL_NUM;
	}
	else {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	EfuseData.StartRow = Row;
	EfuseData.ColStart = StartColNum;
	EfuseData.ColEnd = EndColNum;
	EfuseData.NumOfRows = XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS;
	EfuseData.EfuseType = XNVM_EFUSE_PAGE_0;

	Status = XNvm_EfusePgmAndVerifyData(&EfuseData, EfuseHash->Hash);
	if (Status != XST_SUCCESS) {
		Status = (Status |
			(XNVM_EFUSE_ERR_WRITE_PPK0_HASH +
			(PpkType << XNVM_EFUSE_ERROR_BYTE_SHIFT)));
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function is used to program below eFuses
 * 		AES key/User key 0/User key 1.
 *
 * @param	KeyType - Type of key to be programmed
 * 			(AesKey/UserKey0/UserKey1)
 * @param	EfuseKey - Pointer to the XNvm_AesKey struture, which holds
 * 			Aes key to be programmed to eFuse.
 *
 * @return	- XST_SUCCESS - On Successful Programming.
 *		- XNVM_EFUSE_ERR_WRITE_AES_KEY	 - Error while writing
 *							AES key.
 *		- XNVM_EFUSE_ERR_WRITE_USER_KEY0 - Error while writing
 *							User key 0.
 * 		- XNVM_EFUSE_ERR_WRITE_USER_KEY1 - Error while writing
 *							User key 1.
 *
 ******************************************************************************/
static int XNvm_EfusePrgmAesKey(XNvm_AesKeyType KeyType, XNvm_AesKey *EfuseKey)
{
	int Status = XST_FAILURE;
	XNvm_EfusePrgmInfo EfuseData = {0U};
	int CrcRegOffset = 0U;
	int CrcDoneMask = 0U;
        int CrcPassMask	= 0U;
	u32 Crc;

	if (EfuseKey == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	EfuseData.EfuseType = XNVM_EFUSE_PAGE_0;
	EfuseData.SkipVerify = 1U;

	if (KeyType == XNVM_EFUSE_AES_KEY) {
		EfuseData.StartRow = XNVM_EFUSE_AES_KEY_0_TO_127_START_ROW;
		EfuseData.ColStart = XNVM_EFUSE_AES_KEY_0_TO_127_COL_START_NUM;
		EfuseData.ColEnd = XNVM_EFUSE_AES_KEY_0_TO_127_COL_END_NUM;
		EfuseData.NumOfRows = XNVM_EFUSE_AES_KEY_0_TO_127_NUM_OF_ROWS;

		Status = XNvm_EfusePgmAndVerifyData(&EfuseData, EfuseKey->Key);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_AES_KEY);
			goto END;
		}

		EfuseData.StartRow = XNVM_EFUSE_AES_KEY_128_TO_255_START_ROW;
		EfuseData.ColStart = XNVM_EFUSE_AES_KEY_128_TO_255_COL_START_NUM;
		EfuseData.ColEnd = XNVM_EFUSE_AES_KEY_128_TO_255_COL_END_NUM;
		EfuseData.NumOfRows = XNVM_EFUSE_AES_KEY_128_TO_255_NUM_OF_ROWS;

		Status = XNvm_EfusePgmAndVerifyData(&EfuseData, &EfuseKey->Key[4U]);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_AES_KEY);
			goto END;
		}

		CrcRegOffset = XNVM_EFUSE_AES_CRC_REG_OFFSET;
		CrcDoneMask = XNVM_EFUSE_CTRL_STATUS_AES_CRC_DONE_MASK;
		CrcPassMask = XNVM_EFUSE_CTRL_STATUS_AES_CRC_PASS_MASK;

	}
	else if (KeyType == XNVM_EFUSE_USER_KEY_0) {
		EfuseData.StartRow = XNVM_EFUSE_USER_KEY0_0_TO_63_START_ROW;
		EfuseData.ColStart = XNVM_EFUSE_USER_KEY0_0_TO_63_COL_START_NUM;
		EfuseData.ColEnd = XNVM_EFUSE_USER_KEY0_0_TO_63_COL_END_NUM;
		EfuseData.NumOfRows = XNVM_EFUSE_USER_KEY0_0_TO_63_NUM_OF_ROWS;

		Status = XNvm_EfusePgmAndVerifyData(&EfuseData, EfuseKey->Key);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_USER_KEY0);
			goto END;
		}

		EfuseData.StartRow = XNVM_EFUSE_USER_KEY0_64_TO_191_START_ROW;
		EfuseData.ColStart = XNVM_EFUSE_USER_KEY0_64_TO_191_COL_START_NUM;
		EfuseData.ColEnd = XNVM_EFUSE_USER_KEY0_64_TO_191_COL_END_NUM;
		EfuseData.NumOfRows = XNVM_EFUSE_USER_KEY0_64_TO_191_NUM_OF_ROWS;

		Status = XNvm_EfusePgmAndVerifyData(&EfuseData, &EfuseKey->Key[2U]);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_USER_KEY0);
			goto END;
		}

		EfuseData.StartRow = XNVM_EFUSE_USER_KEY0_192_TO_255_START_ROW;
		EfuseData.ColStart = XNVM_EFUSE_USER_KEY0_192_TO_255_COL_START_NUM;
		EfuseData.ColEnd = XNVM_EFUSE_USER_KEY0_192_TO_255_COL_END_NUM;
		EfuseData.NumOfRows = XNVM_EFUSE_USER_KEY0_192_TO_255_NUM_OF_ROWS;

		Status = XNvm_EfusePgmAndVerifyData(&EfuseData, &EfuseKey->Key[6U]);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_USER_KEY0);
			goto END;
		}

		CrcRegOffset = XNVM_EFUSE_AES_USR_KEY0_CRC_REG_OFFSET;
		CrcDoneMask = XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_0_CRC_DONE_MASK;
		CrcPassMask = XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_0_CRC_PASS_MASK;

	}
	else if (KeyType == XNVM_EFUSE_USER_KEY_1) {
		EfuseData.StartRow = XNVM_EFUSE_USER_KEY1_0_TO_63_START_ROW;
		EfuseData.ColStart = XNVM_EFUSE_USER_KEY1_0_TO_63_START_COL_NUM;
		EfuseData.ColEnd = XNVM_EFUSE_USER_KEY1_0_TO_63_END_COL_NUM;
		EfuseData.NumOfRows = XNVM_EFUSE_USER_KEY1_0_TO_63_NUM_OF_ROWS;

		Status = XNvm_EfusePgmAndVerifyData(&EfuseData, EfuseKey->Key);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_USER_KEY1);
			goto END;
		}

		EfuseData.StartRow = XNVM_EFUSE_USER_KEY1_64_TO_127_START_ROW;
		EfuseData.ColStart = XNVM_EFUSE_USER_KEY1_64_TO_127_START_COL_NUM;
		EfuseData.ColEnd = XNVM_EFUSE_USER_KEY1_64_TO_127_END_COL_NUM;
		EfuseData.NumOfRows = XNVM_EFUSE_USER_KEY1_64_TO_127_NUM_OF_ROWS;

		Status = XNvm_EfusePgmAndVerifyData(&EfuseData, &EfuseKey->Key[2U]);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_USER_KEY1);
			goto END;
		}

		EfuseData.StartRow = XNVM_EFUSE_USER_KEY1_128_TO_255_START_ROW;
		EfuseData.ColStart = XNVM_EFUSE_USER_KEY1_128_TO_255_START_COL_NUM;
		EfuseData.ColEnd = XNVM_EFUSE_USER_KEY1_128_TO_255_END_COL_NUM;
		EfuseData.NumOfRows = XNVM_EFUSE_USER_KEY1_128_TO_255_NUM_OF_ROWS;

		Status = XNvm_EfusePgmAndVerifyData(&EfuseData, &EfuseKey->Key[4U]);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_USER_KEY1);
			goto END;
		}

		CrcRegOffset = XNVM_EFUSE_AES_USR_KEY1_CRC_REG_OFFSET;
		CrcDoneMask = XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_1_CRC_DONE_MASK;
		CrcPassMask = XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_1_CRC_PASS_MASK;
	}
	else {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfuseCacheLoadNPrgmProtectionBits();
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_WRITE_AES_KEY);
		goto END;
	}
	Crc = XNvm_AesCrcCalc(EfuseKey->Key);

	Status = XNvm_EfuseCheckAesKeyCrc(CrcRegOffset, CrcDoneMask,
                        CrcPassMask, Crc);
	if (Status != XST_SUCCESS) {
		Status = (Status |
			(XNVM_EFUSE_ERR_WRITE_AES_KEY + (KeyType << XNVM_EFUSE_ERROR_BYTE_SHIFT)));
		goto END;
	}

	Status = XST_SUCCESS;

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
 *		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *		- XNVM_EFUSE_ERR_CACHE_PARITY  - Error in Cache reload.
 *
 ******************************************************************************/
static int XNvm_EfuseComputeProgrammableBits(const u32 *ReqData, u32 *PrgmData,
						u32 StartOffset, u32 EndOffset)
{
	int Status = XST_FAILURE;
	int IsrStatus = XST_FAILURE;
	u32 ReadReg = 0U;
	u32 Offset = 0U;
	u32 Idx = 0U;

	if ((ReqData == NULL) || (PrgmData == NULL)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	IsrStatus = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_ISR_REG_OFFSET);
	if ((IsrStatus & XNVM_EFUSE_ISR_CACHE_ERROR)
			== XNVM_EFUSE_ISR_CACHE_ERROR) {
		Status = (int)XNVM_EFUSE_ERR_CACHE_PARITY;
		goto END;
	}

	Offset = StartOffset;
	while (Offset <= EndOffset) {
		ReadReg = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR, Offset);

		Idx = (Offset - StartOffset) / XNVM_WORD_LEN;
		PrgmData[Idx] = (~ReadReg) & ReqData[Idx];
		Offset = Offset + XNVM_WORD_LEN;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function sets and then verifies the specified bits
 *		in the eFUSE.
 *
 * @param	StartRow  - Starting Row number (0-based addressing).
 * @param	RowCount  - Number of Rows to be written.
 * @param	EfuseType - It is an enum object of type XNvm_EfuseType.
 * @param	RowData   - Pointer to memory location where bitmap to be
 * 			written is stored. Only bit set are used for programming
 * 			eFUSE.
 *
 * @return	- XST_SUCCESS - Specified bit set in eFUSE.
 *  		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *
 ******************************************************************************/
static int XNvm_EfusePgmAndVerifyData(XNvm_EfusePrgmInfo *EfuseData, const u32* RowData)
{
	volatile int Status = XST_FAILURE;
	u32 BytesToPgmInRow = XNVM_EFUSE_MAX_BITS_IN_ROW /(EfuseData->ColEnd - EfuseData->ColStart + 1U);
	const u32* DataPtr = RowData;
	u32 Row = EfuseData->StartRow;
	volatile u32 Count = 0U;
	u32 Idx = 0U;
	u32 Col = 0U;
	u32 Data;

	if ((EfuseData->EfuseType != XNVM_EFUSE_PAGE_0) &&
		(EfuseData->EfuseType != XNVM_EFUSE_PAGE_1) &&
		(EfuseData->EfuseType != XNVM_EFUSE_PAGE_2)){

		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	if ((DataPtr == NULL) || (EfuseData->NumOfRows == 0U)) {

		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	while (Count < EfuseData->NumOfRows) {
		Data = *DataPtr;
		Idx = 0U;
		while (Idx < BytesToPgmInRow) {
			Col = EfuseData->ColStart;
			while (Col <= EfuseData->ColEnd) {
				if ((Data & 0x01U) == 0x01U) {
					Status = XNvm_EfusePgmAndVerifyBit(
							EfuseData->EfuseType, Row,
							Col, EfuseData->SkipVerify);
					if (Status != XST_SUCCESS) {
						goto END;
					}
				}
				Col++;
				Data = Data >> 1U;
			}
			Idx++;
			Row++;
			Count++;
		}
		DataPtr++;
	}

	if (Count != EfuseData->NumOfRows) {
		Status = (int)XNVM_EFUSE_ERR_GLITCH_DETECTED;
	}
	else {
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function sets the specified bit in the eFUSE.
 *
 * @param	Page - It is an enum variable of type XNvm_EfuseType.
 * @param	Row  - It is an 32-bit Row number (0-based addressing).
 * @param	Col  - It is an 32-bit Col number (0-based addressing).
 *
 * @return	- XST_SUCCESS	- Specified bit set in eFUSE.
 *		- XNVM_EFUSE_ERR_PGM_TIMEOUT - eFUSE programming timed out.
 *		- XNVM_EFUSE_ERR_PGM - eFUSE programming failed.
 *
 ******************************************************************************/
static int XNvm_EfusePgmBit(XNvm_EfuseType Page, u32 Row, u32 Col)
{
	int Status = XST_FAILURE;
	u32 PgmAddr;
	u32 EventMask = 0U;

	PgmAddr = ((u32)Page << XNVM_EFUSE_ADDR_PAGE_SHIFT) |
		(Row << XNVM_EFUSE_ADDR_ROW_SHIFT) |
		(Col << XNVM_EFUSE_ADDR_COLUMN_SHIFT);

	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
		XNVM_EFUSE_PGM_ADDR_REG_OFFSET, PgmAddr);

	Status = (int)Xil_WaitForEvents((XNVM_EFUSE_CTRL_BASEADDR + XNVM_EFUSE_ISR_REG_OFFSET),
			(XNVM_EFUSE_ISR_PGM_DONE | XNVM_EFUSE_ISR_PGM_ERROR),
			(XNVM_EFUSE_ISR_PGM_DONE | XNVM_EFUSE_ISR_PGM_ERROR),
			XNVM_EFUSE_PGM_TIMEOUT_VAL,
			&EventMask);

	if (XST_TIMEOUT == Status) {
		Status = (int)XNVM_EFUSE_ERR_PGM_TIMEOUT;
	}
	else if ((EventMask & XNVM_EFUSE_ISR_PGM_ERROR) == XNVM_EFUSE_ISR_PGM_ERROR) {
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
 * @brief	This function verify the specified bit set in the eFUSE.
 *
 * @param	Page - It is an enum variable of type XNvm_EfuseType.
 * @param	Row - It is an 32-bit Row number (0-based addressing).
 * @param	Col - It is an 32-bit Col number (0-based addressing).
 *
 * @return	- XST_SUCCESS - Specified bit set in eFUSE.
 *		- XNVM_EFUSE_ERR_PGM_VERIFY  - Verification failed, specified bit
 *						   is not set.
 *		- XNVM_EFUSE_ERR_PGM_TIMEOUT - If Programming timeout has occured.
 *		- XST_FAILURE                - Unexpected error.
 *
 ******************************************************************************/
static int XNvm_EfuseVerifyBit(XNvm_EfuseType Page, u32 Row, u32 Col)
{
	int Status = XST_FAILURE;
	u32 RdAddr;
	volatile u32 RegData = 0x00U;
	u32 EventMask = 0x00U;

	RdAddr = ((u32)Page << XNVM_EFUSE_ADDR_PAGE_SHIFT) |
		(Row << XNVM_EFUSE_ADDR_ROW_SHIFT);

	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
		XNVM_EFUSE_RD_ADDR_REG_OFFSET, RdAddr);

	Status = (int)Xil_WaitForEvents((XNVM_EFUSE_CTRL_BASEADDR +
		XNVM_EFUSE_ISR_REG_OFFSET),
		XNVM_EFUSE_ISR_RD_DONE,
		XNVM_EFUSE_ISR_RD_DONE,
		XNVM_EFUSE_RD_TIMEOUT_VAL,
		&EventMask);

	if (XST_TIMEOUT == Status) {
		Status = (int)XNVM_EFUSE_ERR_RD_TIMEOUT;
	}
	else if ((EventMask & XNVM_EFUSE_ISR_RD_DONE)
					== XNVM_EFUSE_ISR_RD_DONE) {
		RegData = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_RD_DATA_REG_OFFSET);
		if ((RegData & (((u32)0x01U) << Col)) != 0U) {
			Status = XST_SUCCESS;
		}
		else {
			Status = XST_FAILURE;
		}
	}
	else {
		Status = (int)XNVM_EFUSE_ERR_PGM_VERIFY;
	}

	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
			XNVM_EFUSE_ISR_REG_OFFSET,
			XNVM_EFUSE_ISR_RD_DONE);
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function sets and then verifies the specified
 *			bit in the eFUSE.
 *
 * @param	Page - It is an enum variable of type XNvm_EfuseType.
 * @param	Row  - It is an 32-bit Row number (0-based addressing).
 * @param	Col  - It is an 32-bit Col number (0-based addressing).
 * @param	SkipVerify - Skips verification of bit if set to non zero
 *
 * @return	- XST_SUCCESS - Specified bit set in eFUSE.
 *		- XNVM_EFUSE_ERR_PGM_TIMEOUT - eFUSE programming timed out.
 *		- XNVM_EFUSE_ERR_PGM 	- eFUSE programming failed.
 *		- XNVM_EFUSE_ERR_PGM_VERIFY  - Verification failed, specified bit
 *						is not set.
 *		- XST_FAILURE 	- Unexpected error.
 *
 ******************************************************************************/
static int XNvm_EfusePgmAndVerifyBit(XNvm_EfuseType Page, u32 Row, u32 Col, u32 SkipVerify)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_EfusePgmBit(Page, Row, Col);
	if(XST_SUCCESS == Status) {
		if (SkipVerify == 0U) {
			Status = XST_FAILURE;
			Status = XNvm_EfuseVerifyBit(Page, Row, Col);
		}
	}

	return Status;
}
