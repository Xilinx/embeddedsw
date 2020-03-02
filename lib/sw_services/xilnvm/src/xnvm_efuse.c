/******************************************************************************
*
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/

/******************************************************************************/
/**
*
* @file xnvm_efuse.c
*
* This file contains NVM library eFuse functions
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.0   kal  08/16/2019 Initial release
* 2.0   kal  02/27/2020 Added eFuse Wrapper APIs
*
* </pre>
*
* @note
*
*******************************************************************************/

/***************************** Include Files **********************************/
#include "sleep.h"
#include "xil_util.h"
#include "xnvm_efuse.h"
#include "xnvm_efuse_hw.h"
#include "xnvm_utils.h"

/*************************** Constant Definitions *****************************/

/***************************** Type Definitions *******************************/
/* Operation mode - Read, Program(Write) */
typedef enum {
	XNVM_EFUSE_MODE_RD,
	XNVM_EFUSE_MODE_PGM
} XNvm_EfuseOpMode;

/* eFUSE read type - Normal read, Margin read */
typedef enum {
	XNVM_EFUSE_NORMAL_RD,
	XNVM_EFUSE_MARGIN_RD
} XNvm_EfuseRdMode;

/****************** Macros (Inline Functions) Definitions *********************/

#define XNVM_ONE_MICRO_SECOND			(1U)
#define XNVM_ONE_MILLI_SECOND			(1000U)

/*************************** Function Prototypes ******************************/
static inline u32 XNvm_EfuseLockController(void);
static inline u32 XNvm_EfuseUnlockController(void);
static inline void XNvm_EfuseDisablePowerDown(void);
static inline void XNvm_EfuseSetReadMode(XNvm_EfuseRdMode RdMode);
static inline void XNvm_EfuseSetRefClk(void);
static inline void XNvm_EfuseEnableProgramming(void);
static inline void XNvm_EfuseDisableProgramming(void);
static inline void XNvm_EfuseInitTimers(void);
static u32 XNvm_EfuseSetupController(XNvm_EfuseOpMode Op,
					XNvm_EfuseRdMode RdMode);
static u32 XNvm_EfuseReadRow(u8 Page, u32 Row, u32* RowData);
static u32 XNvm_EfuseReadCache(u32 Row, u32* RowData);
static u32 XNvm_EfuseReadCacheRange(u32 StartRow, u8 RowCount, u32* RowData);
static u32 XNvm_EfusePgmBit(u32 Page, u32 Row, u32 Col);
static u32 XNvm_EfuseVerifyBit(u32 Page, u32 Row, u32 Col);
static u32 XNvm_EfusePgmAndVerifyBit(u32 Page, u32 Row, u32 Col);
static u32 XNvm_EfusePgmTBits(void);
static u32 XNvm_EfuseCacheLoad(void);
static u32 XNvm_EfuseCheckForTBits(void);
static u32 XNvm_EfusePgmRows(u32 StartRow, u8 RowCount,
			XNvm_EfuseType EfuseType, const u32* RowData);
static u32 XNvm_EfuseCheckZerosBfrPrgrmg(Xnvm_EfuseWriteData *WriteNvm);
static u32 XNvm_EfuseCheckZeros(u32 RowStart, u32 RowEnd,
				XNvm_EfuseType EfuseType);
static u32 XNvm_EfuseWriteChecks(Xnvm_EfuseWriteData *WriteNvm);
static u32 XNvm_EfuseReadSecCtrlBitsRegs(
				Xnvm_SecCtrlBits *ReadbackSecCtrlBits,
				u8 ReadOption);
static u32 XNvm_EfuseWritePufAux(Xnvm_PufHelperData *PrgmPufHelperData);
static u32 XNvm_EfuseWritePufChash(Xnvm_PufHelperData *PrgmPufHelperData);
static u32 XNvm_EfuseWritePufSynData(Xnvm_PufHelperData *PrgmPufHelperData);
static u32 XNvm_EfuseReadPufSecCtrlBitsRegs(
		Xnvm_PufSecCtrlBits *ReadBackPufSecCtrlBits, u8 ReadOption);
static u32 Xnvm_PrgmRevocationId(u32 RevokeIdEfuseNum, u32 EfuseData);
static u32 XNvm_EfuseCheckForZerosPuf(void);
static u32 XNvm_EfuseReadMiscCtrlBitsRegs(
		Xnvm_MiscCtrlBits *ReadMiscCtrlBits, u8 ReadOption);
static u32 XNvm_EfuseWriteSecCtrl(Xnvm_EfuseWriteData *WriteNvm);
static u32 Xnvm_PrgmIvRow(u32 EfuseData, u32 IvRow);

/*************************** Variable Definitions *****************************/

/*************************** Function Definitions *****************************/

/***************************************************************************/
/*
 * This function is used to program the AES key, User key 0, User key 1 and
 * PPK0/PPK1/PPK2 hash based on user inputs.
 *
 * @param	WriteNvm	Pointer to the Xnvm_EfuseWriteData.
 *
 * @return
 * 		- XST_SUCCESS if programs successfully.
 * 		- Errorcode on failure
 *
 * @note	After eFUSE programming is complete, the cache is automatically
 *		reloaded so all programmed eFUSE bits can be directly read from
 *		cache.
 *
 *****************************************************************************/
u32 XNvm_EfuseWrite(Xnvm_EfuseWriteData *WriteNvm)
{
	u32 Status = (u32)XST_FAILURE;
	u32 Crc;

	if (WriteNvm == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	/* Conditions to check programming is possible or not */
	Status = XNvm_EfuseWriteChecks(WriteNvm);
	if (Status != (u32)XST_SUCCESS) {
		Status = (Status | (u32)XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	/* Check for Zeros for Programming eFuse */
	Status = XNvm_EfuseCheckZerosBfrPrgrmg(WriteNvm);
	if (Status != (u32)XST_SUCCESS) {
		Status = (Status | (u32)XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	if (WriteNvm->CheckWriteFlags.AesKey == TRUE) {
		Status = XNvm_EfusePgmRows(XNVM_EFUSE_AES_KEY_START_ROW,
				XNVM_EFUSE_AES_KEY_NUM_OF_ROWS,
				XNVM_EFUSE_PAGE_0, &(WriteNvm->AesKey[0]));
		if (Status != XST_SUCCESS) {
			Status = (Status | (u32)XNVM_EFUSE_ERR_WRITE_AES_KEY);
			goto END;
		}
		Status = XNvm_EfuseCacheLoad();
		if (Status != XST_SUCCESS) {
			Status = (Status | (u32)XNVM_EFUSE_ERR_WRITE_AES_KEY);
			goto END;
		}
		Crc = XNvm_AesCrcCalc(&WriteNvm->AesKey[0]);

		Status = XNvm_EfuseCheckAesKeyCrc(Crc);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XNVM_EFUSE_ERR_CRC_VERIFICATION |
				(u32)XNVM_EFUSE_ERR_WRITE_AES_KEY;
			goto END;
		}
	}
	if (WriteNvm->CheckWriteFlags.UserKey0 == TRUE) {
		Status = XNvm_EfusePgmRows(XNVM_EFUSE_USER_KEY_0_START_ROW,
				XNVM_EFUSE_USER_KEY_NUM_OF_ROWS,
				XNVM_EFUSE_PAGE_0, &(WriteNvm->UserKey0[0]));
		 if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_USER0_KEY);
			goto END;
		}
		Status = XNvm_EfuseCacheLoad();
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_USER0_KEY);
		goto END;
		}
		Crc = XNvm_AesCrcCalc(&WriteNvm->UserKey0[0]);

		Status = XNvm_EfuseCheckUserKey0Crc(Crc);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XNVM_EFUSE_ERR_CRC_VERIFICATION |
				(u32)XNVM_EFUSE_ERR_WRITE_USER0_KEY;
			goto END;
		}
	}
	if (WriteNvm->CheckWriteFlags.UserKey1 == TRUE) {
		Status = XNvm_EfusePgmRows(XNVM_EFUSE_USER_KEY_1_START_ROW,
					XNVM_EFUSE_USER_KEY_NUM_OF_ROWS,
					XNVM_EFUSE_PAGE_0,
					&(WriteNvm->UserKey1[0]));
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_USER1_KEY);
		goto END;
		}
		Status = XNvm_EfuseCacheLoad();
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_USER1_KEY);
			goto END;
		}
		Crc = XNvm_AesCrcCalc(&WriteNvm->UserKey1[0]);

		Status = XNvm_EfuseCheckUserKey1Crc(Crc);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XNVM_EFUSE_ERR_CRC_VERIFICATION |
				(u32)XNVM_EFUSE_ERR_WRITE_USER1_KEY;
			goto END;
		}
	}
	if (WriteNvm->CheckWriteFlags.Ppk0Hash == TRUE) {
		Status = XNvm_EfusePgmRows(XNVM_EFUSE_PPK_0_HASH_START_ROW,
				XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS,
				XNVM_EFUSE_PAGE_0,
				&(WriteNvm->Ppk0Hash[0]));
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_PPK0_HASH);
			goto END;
		}
	}
	if (WriteNvm->CheckWriteFlags.Ppk1Hash == TRUE) {
		Status = XNvm_EfusePgmRows(XNVM_EFUSE_PPK_1_HASH_START_ROW,
				XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS,
				XNVM_EFUSE_PAGE_0,
				&(WriteNvm->Ppk1Hash[0]));
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_PPK1_HASH);
			goto END;
		}
	}
	if (WriteNvm->CheckWriteFlags.Ppk2Hash == TRUE) {
		Status = XNvm_EfusePgmRows(XNVM_EFUSE_PPK_2_HASH_START_ROW,
				XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS,
				XNVM_EFUSE_PAGE_0,
				&(WriteNvm->Ppk2Hash[0]));
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_PPK2_HASH);
			goto END;
		}
	}
	if (WriteNvm->CheckWriteFlags.DecOnly == TRUE) {
		Status = XNvm_EfusePgmRows(XNVM_EFUSE_DEC_EFUSE_ONLY_ROW,
				XNVM_EFUSE_DEC_EFUSE_ONLY_NUM_OF_ROWS,
				XNVM_EFUSE_PAGE_0,
				&(WriteNvm->DecEfuseOnly));
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_DEC_EFUSE_ONLY);
			goto END;
		}
	}

	/* Programming Secure and control bits */
	Status = XNvm_EfuseWriteSecCtrl(WriteNvm);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseCacheLoad();
END:
	XNvm_EfuseDisableProgramming();

	return Status;
}

/*****************************************************************************/
/*
 * This function programs secure control bits specified by user.
 *
 * @param	WriteNvm is an instance of efuseps of Versal.
 *
 * @return
 *		XST_SUCCESS - On success
 *		ErrorCode - on Failure
 *
 ******************************************************************************/
static u32 XNvm_EfuseWriteSecCtrl(Xnvm_EfuseWriteData *WriteNvm)
{
	u32 Status = (u32)XST_FAILURE;
	XNvm_EfuseType EfuseType = XNVM_EFUSE_PAGE_0;
	u32 Row = XNVM_EFUSE_SECURITY_CONTROL_ROW;
	u32 RowDataVal;
	u8 DataInBits[XNVM_EFUSE_MAX_BITS_IN_ROW] = {0};

	if (WriteNvm == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if ((WriteNvm->PrgmSecCtrlFlags.AesDis != 0x00U) ||
		(WriteNvm->PrgmSecCtrlFlags.JtagErrOutDis != 0x00U) ||
		(WriteNvm->PrgmSecCtrlFlags.JtagDis != 0x00U) ||
		(WriteNvm->PrgmSecCtrlFlags.HwTstBitsDis != 0x00U) ||
		(WriteNvm->PrgmSecCtrlFlags.IpDisWrLk != 0x00U) ||
		(WriteNvm->PrgmSecCtrlFlags.Ppk0WrLk != 0x00U) ||
		(WriteNvm->PrgmSecCtrlFlags.Ppk1WrLk != 0x00U) ||
		(WriteNvm->PrgmSecCtrlFlags.Ppk2WrLk != 0x00U) ||
		(WriteNvm->PrgmSecCtrlFlags.AesCrcLk != 0x00U) ||
		(WriteNvm->PrgmSecCtrlFlags.AesWrLk != 0x00U) ||
		(WriteNvm->PrgmSecCtrlFlags.UserKey0CrcLk != 0x00U) ||
		(WriteNvm->PrgmSecCtrlFlags.UserKey0WrLk != 0x00U) ||
		(WriteNvm->PrgmSecCtrlFlags.UserKey1CrcLk != 0x00U) ||
		(WriteNvm->PrgmSecCtrlFlags.UserKey1WrLk != 0x00U) ||
		(WriteNvm->PrgmSecCtrlFlags.SecDbgDis != 0x00U) ||
		(WriteNvm->PrgmSecCtrlFlags.SecLockDbgDis != 0x00U) ||
		(WriteNvm->PrgmSecCtrlFlags.PmcScEn != 0x00U) ||
		(WriteNvm->PrgmSecCtrlFlags.SvdWrLk != 0x00U) ||
		(WriteNvm->PrgmSecCtrlFlags.DnaWrLk != 0x00U) ||
		(WriteNvm->PrgmSecCtrlFlags.BootEnvWrLk != 0x00U) ||
		(WriteNvm->PrgmSecCtrlFlags.CacherWrLk != 0x00U) ||
		(WriteNvm->PrgmSecCtrlFlags.RegInitDis != 0x00U)) {

		Status = XNvm_EfuseReadCache(Row, &RowDataVal);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}

		XNvm_ConvertBitsToBytes((u8 *) &RowDataVal,
			DataInBits, XNVM_EFUSE_MAX_BITS_IN_ROW);
	}
	/*Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
					XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		goto END;
	}*/
	if ((WriteNvm->PrgmSecCtrlFlags.AesDis != 0x00U) &&
		(DataInBits[XNVM_EFUSE_SEC_AES_DIS] == 0x00U)) {
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
			XNVM_EFUSE_SEC_AES_DIS);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_AES_DIS);
			goto END;
		}
	}
	if ((WriteNvm->PrgmSecCtrlFlags.JtagErrOutDis != 0x00U) &&
		(DataInBits[XNVM_EFUSE_SEC_JTAG_ERROUT_DIS] == 0x00U)) {
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
			XNVM_EFUSE_SEC_JTAG_ERROUT_DIS);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_JTAG_ERROUT_DIS);
			goto END;
		}
	}
	if ((WriteNvm->PrgmSecCtrlFlags.JtagDis != 0x00U) &&
		(DataInBits[XNVM_EFUSE_SEC_JTAG_DIS] == 0x00U)) {
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
			XNVM_EFUSE_SEC_JTAG_DIS);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_JTAG_DIS);
			goto END;
		}
	}
	if ((WriteNvm->PrgmSecCtrlFlags.HwTstBitsDis != 0x00U) &&
		(DataInBits[XNVM_EFUSE_SEC_HWTSTBITS_DIS] == 0x00U)) {
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
			XNVM_EFUSE_SEC_HWTSTBITS_DIS);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_HWTSTBITS_DIS);
			goto END;
		}
	}
	if ((WriteNvm->PrgmSecCtrlFlags.IpDisWrLk != 0x00U) &&
		(DataInBits[XNVM_EFUSE_SEC_IP_DIS_WRLK] == 0x00U)) {
		 Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
			XNVM_EFUSE_SEC_IP_DIS_WRLK);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_IP_DIS_WR_LK);
			goto END;
		}
	}
	if ((WriteNvm->PrgmSecCtrlFlags.Ppk0WrLk != 0x00U) &&
		(DataInBits[XNVM_EFUSE_SEC_PPK0_WRLK] == 0x00U)) {
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
			XNVM_EFUSE_SEC_PPK0_WRLK);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_PPK0_WR_LK);
			goto END;
		}
	}
	if ((WriteNvm->PrgmSecCtrlFlags.Ppk1WrLk != 0x00U) &&
		(DataInBits[XNVM_EFUSE_SEC_PPK1_WRLK] == 0x00U)) {
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
			XNVM_EFUSE_SEC_PPK1_WRLK);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_PPK1_WR_LK);
			goto END;
		}
	}
	if ((WriteNvm->PrgmSecCtrlFlags.Ppk2WrLk != 0x00U) &&
		(DataInBits[XNVM_EFUSE_SEC_PPK2_WRLK] == 0x00U)) {
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
					XNVM_EFUSE_SEC_PPK2_WRLK);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_PPK2_WR_LK);
			goto END;
		}
	}
	if (WriteNvm->PrgmSecCtrlFlags.AesCrcLk != 0x00U) {
		if (DataInBits[XNVM_EFUSE_SEC_AES_CRC_LK_BIT_1] == 0x00U) {
			Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
				XNVM_EFUSE_SEC_AES_CRC_LK_BIT_1);
			if (Status != (u32)XST_SUCCESS) {
				Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_AES_CRC_LK_BIT_1);
				goto END;
			}
		}
		if (DataInBits[XNVM_EFUSE_SEC_AES_CRC_LK_BIT_2] == 0x00U) {
			Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
				XNVM_EFUSE_SEC_AES_CRC_LK_BIT_2);
			if (Status != (u32)XST_SUCCESS) {
				Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_AES_CRC_LK_BIT_2);
				goto END;
			}
		}
	}
	if ((WriteNvm->PrgmSecCtrlFlags.AesWrLk != 0x00U) &&
		(DataInBits[XNVM_EFUSE_SEC_AES_WRLK] == 0x00U)) {
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
			XNVM_EFUSE_SEC_AES_WRLK);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_AES_WR_LK);
			goto END;
		}
	}
	if ((WriteNvm->PrgmSecCtrlFlags.UserKey0CrcLk != 0x00U) &&
		(DataInBits[XNVM_EFUSE_SEC_USER_KEY0_CRC_LK] == 0x00U)) {
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
			XNVM_EFUSE_SEC_USER_KEY0_CRC_LK);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_USER_KEY0_CRC_LK);
			goto END;
		}
	}
	if ((WriteNvm->PrgmSecCtrlFlags.UserKey0WrLk != 0x00U) &&
		(DataInBits[XNVM_EFUSE_SEC_USER_KEY0_WRLK] == 0x00U)) {
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
			XNVM_EFUSE_SEC_USER_KEY0_WRLK);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_USER_KEY0_WR_LK);
		goto END;
		}
	}
	if ((WriteNvm->PrgmSecCtrlFlags.UserKey1CrcLk != 0x00U) &&
		(DataInBits[XNVM_EFUSE_SEC_USER_KEY1_CRC_LK] == 0x00U)) {
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
			XNVM_EFUSE_SEC_USER_KEY1_CRC_LK);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_USER_KEY1_CRC_LK);
			goto END;
		}
	}
	if ((WriteNvm->PrgmSecCtrlFlags.UserKey1WrLk != 0x00U) &&
		(DataInBits[XNVM_EFUSE_SEC_USER_KEY1_WRLK] == 0x00U)) {
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
			XNVM_EFUSE_SEC_USER_KEY1_WRLK);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_USER_KEY1_WR_LK);
			goto END;
		}
	}
	if (WriteNvm->PrgmSecCtrlFlags.SecDbgDis != 0x00U) {
		if (DataInBits[XNVM_EFUSE_SEC_SECDBG_DIS_BIT_1] == 0x00U) {
			Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
				XNVM_EFUSE_SEC_SECDBG_DIS_BIT_1);
			if (Status != (u32)XST_SUCCESS) {
				Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_SECDBG_DIS_BIT_1);
				goto END;
			}
		}
		if (DataInBits[XNVM_EFUSE_SEC_SECDBG_DIS_BIT_2] == 0x00U) {
			Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
				XNVM_EFUSE_SEC_SECDBG_DIS_BIT_2);
			if (Status != (u32)XST_SUCCESS) {
				Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_SECDBG_DIS_BIT_2);
				goto END;
			}
		}
	}
	if (WriteNvm->PrgmSecCtrlFlags.SecLockDbgDis != 0x00U) {
		if (DataInBits[XNVM_EFUSE_SEC_SECLOCKDBG_DIS_BIT_1] == 0x00U) {
			Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
				XNVM_EFUSE_SEC_SECLOCKDBG_DIS_BIT_1);
			if (Status != (u32)XST_SUCCESS) {
				Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_SECLOCKDBG_DIS_BIT_1);
				goto END;
			}
		}
		if (DataInBits[XNVM_EFUSE_SEC_SECLOCKDBG_DIS_BIT_2] == 0x00U) {
			Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
				XNVM_EFUSE_SEC_SECLOCKDBG_DIS_BIT_2);
			if (Status != (u32)XST_SUCCESS) {
				Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_SECLOCKDBG_DIS_BIT_2);
				goto END;
			}
		}
	}
	if (WriteNvm->PrgmSecCtrlFlags.PmcScEn != 0x00U) {
		if (DataInBits[XNVM_EFUSE_SEC_PMC_SC_EN_BIT_1] == 0x00U) {
			Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
				XNVM_EFUSE_SEC_PMC_SC_EN_BIT_1);
			if (Status != (u32)XST_SUCCESS) {
				Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_PMC_SC_EN_BIT_1);
				goto END;
			}
		}
		if (DataInBits[XNVM_EFUSE_SEC_PMC_SC_EN_BIT_2] == 0x00U) {
			Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
				XNVM_EFUSE_SEC_PMC_SC_EN_BIT_2);
			if (Status != (u32)XST_SUCCESS) {
				Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_PMC_SC_EN_BIT_2);
				goto END;
			}
		}
		if (DataInBits[XNVM_EFUSE_SEC_PMC_SC_EN_BIT_3] == 0x00U) {
			Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
				XNVM_EFUSE_SEC_PMC_SC_EN_BIT_3);
			if (Status != (u32)XST_SUCCESS) {
				Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_PMC_SC_EN_BIT_3);
				goto END;
			}
		}
	}
	if ((WriteNvm->PrgmSecCtrlFlags.SvdWrLk != 0x00U) &&
		(DataInBits[XNVM_EFUSE_SEC_SVD_WRLK] == 0x00U)) {
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
				XNVM_EFUSE_SEC_SVD_WRLK);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_SVD_WR_LK);
			goto END;
		}
	}
	if ((WriteNvm->PrgmSecCtrlFlags.DnaWrLk != 0x00U) &&
		(DataInBits[XNVM_EFUSE_SEC_DNA_WRLK] == 0x00U)) {
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
			XNVM_EFUSE_SEC_DNA_WRLK);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
			(u32)XNVM_EFUSE_ERR_WRTIE_DNA_WR_LK);
			goto END;
		}
	}
	if ((WriteNvm->PrgmSecCtrlFlags.BootEnvWrLk != 0x00U) &&
		(DataInBits[XNVM_EFUSE_SEC_BOOTENV_WRLK] == 0x00U)) {
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
			XNVM_EFUSE_SEC_BOOTENV_WRLK);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
			(u32)XNVM_EFUSE_ERR_WRTIE_BOOTENV_WR_LK);
			goto END;
		}
	}
	if ((WriteNvm->PrgmSecCtrlFlags.CacherWrLk != 0x00U) &&
		(DataInBits[XNVM_EFUSE_SEC_CACHE_WRLK] == 0x00U)) {
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
			XNVM_EFUSE_SEC_CACHE_WRLK);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
			(u32)XNVM_EFUSE_ERR_WRTIE_CACHE_WR_LK);
			goto END;
		}
	}
	if (WriteNvm->PrgmSecCtrlFlags.RegInitDis != 0x00U) {
		if (DataInBits[XNVM_EFUSE_SEC_REG_INIT_DIS_BIT_1] == 0x00U) {
			Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
				XNVM_EFUSE_SEC_REG_INIT_DIS_BIT_1);
			if (Status != (u32)XST_SUCCESS) {
				Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_REG_INIT_DIS_BIT_1);
				goto END;
			}
		}
		if (DataInBits[XNVM_EFUSE_SEC_REG_INIT_DIS_BIT_2] == 0x00U) {
			Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
				XNVM_EFUSE_SEC_REG_INIT_DIS_BIT_2);
			if (Status != (u32)XST_SUCCESS) {
				Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_REG_INIT_DIS_BIT_2);
				goto END;
			}
		}
	}

	Status = (u32)XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/* This function performs the CRC check of AES key
 *
 * @param	CrcValue	A 32 bit CRC value of an expected AES key.
 *
 * @return
 *		- XST_SUCCESS on successful CRC check.
 *		- ErrorCode on failure
 *
 * @note	For Calculating the CRC of the AES key use the
 *		XNvm_AesCrcCalc() function
 *
 ******************************************************************************/
u32 XNvm_EfuseCheckAesKeyCrc(u32 Crc)
{
	u32 Status = (u32)XST_FAILURE;
	u32 ReadReg = 0U;
	u32 TimeOut = 0U;

	XNvm_Efuse_WriteReg(XNVM_EFUSE_CTRL_BASEADDR,
		XNVM_EFUSE_AES_CRC_REG_OFFSET, Crc);

	while (TimeOut < XNVM_POLL_TIMEOUT) {
		ReadReg = XNvm_Efuse_ReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_STATUS_REG_OFFSET);
		if ((ReadReg & XNVM_EFUSE_CTRL_STATUS_AES_CRC_DONE_MASK) !=
								0x00U) {
			break;
		}
		TimeOut = TimeOut + 1U;
	}

	if ((ReadReg & XNVM_EFUSE_CTRL_STATUS_AES_CRC_DONE_MASK) ==
								0x00U) {
		Status = (u32)XST_FAILURE;
	}
	else if ((ReadReg &
			XNVM_EFUSE_CTRL_STATUS_AES_CRC_PASS_MASK) ==
			0x00U) {

		Status = (u32)XST_FAILURE;
	}
	else {
		Status = (u32)XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/* This function performs the CRC check of User key 0
 *
 * @param	CrcValue	A 32 bit CRC value of an expected User key 0.
 *
 * @return
 *		- XST_SUCCESS on successful CRC check.
 *		- ErrorCode on failure
 *
 * @note	For Calculating the CRC of the User key 0 use the
 *		XNvm_AesCrcCalc() function or
 * 		XNvm_RowAesCrcCalc() function
 *
 ******************************************************************************/
u32 XNvm_EfuseCheckUserKey0Crc(u32 Crc)
{
	u32 Status = (u32)XST_FAILURE;
	u32 ReadReg = 0U;
	u32 TimeOut = 0U;

	/* writing CRC value to check AES key's CRC */
	XNvm_Efuse_WriteReg(XNVM_EFUSE_CTRL_BASEADDR,
		XNVM_EFUSE_AES_USR_KEY0_CRC_REG_OFFSET, Crc);

	while (TimeOut < XNVM_POLL_TIMEOUT) {
		/* Poll for CRC Done bit */
		ReadReg = XNvm_Efuse_ReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_STATUS_REG_OFFSET);
		if ((ReadReg &
			XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_0_CRC_DONE_MASK) !=
								0x00U) {
			break;
		}
		TimeOut = TimeOut + 1U;
	}

	if ((ReadReg &
		XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_0_CRC_DONE_MASK) ==
								0x00U) {
		Status = (u32)XST_FAILURE;
	}
	else if ((ReadReg &
		XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_0_CRC_PASS_MASK) ==
								0x00U) {
		Status = (u32)XST_FAILURE;
	}
	else {
		Status = (u32)XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/* This function performs the CRC check of User key 1
 *
 * @param	CrcValue	A 32 bit CRC value of an expected User key 1.
 *
 * @return
 *		- XST_SUCCESS on successful CRC check.
 *		- ErrorCode on failure
 *
 * @note	For Calculating the CRC of the User key 1 use the
 *		XNvm_AesCrcCalc() function or
 * 		XNvm_RowAesCrcCalc() function
 *
 ******************************************************************************/
u32 XNvm_EfuseCheckUserKey1Crc(u32 Crc)
{
	u32 Status = (u32)XST_FAILURE;
	u32 ReadReg = 0U;
	u32 TimeOut = 0U;

	/* writing CRC value to check AES key's CRC */
	XNvm_Efuse_WriteReg(XNVM_EFUSE_CTRL_BASEADDR,
		XNVM_EFUSE_AES_USR_KEY1_CRC_REG_OFFSET, Crc);

	while (TimeOut < XNVM_POLL_TIMEOUT) {
		/* Poll for CRC Done bit */
		ReadReg = XNvm_Efuse_ReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_STATUS_REG_OFFSET);
		if ((ReadReg &
			XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_1_CRC_DONE_MASK) !=
								0x00U) {
			break;
		}
		TimeOut = TimeOut + 1U;
	}

	if ((ReadReg & XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_1_CRC_DONE_MASK) ==
								0x00U) {
		Status = (u32)XST_FAILURE;
	}
	else if ((ReadReg &
		XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_1_CRC_PASS_MASK) ==
								0x00U) {
		Status = (u32)XST_FAILURE;
	}
	else {
		Status = (u32)XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/*
 * This function is used to read the PS eFUSE secure control bits from cache or
 * eFUSE based on user input provided.
 *
 * @param	ReadBackSecCtrlBits	Pointer to the Xnvm_SecCtrlBits
 * 					which holds the read secure control bits.
 * @return
 * 		- XST_SUCCESS if reads successfully
 * 		- XST_FAILURE if reading is failed
 *
 * @note	Cache reload is required for obtaining updated values for
 *		ReadOption 0.
 *
 * ******************************************************************************/
u32 XNvm_EfuseReadSecCtrlBits(Xnvm_SecCtrlBits *ReadbackSecCtrlBits)
{
	u32 Status = (u32)XST_FAILURE;

	if (ReadbackSecCtrlBits == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
	}
	else {
		Status = XNvm_EfuseReadSecCtrlBitsRegs(ReadbackSecCtrlBits, 0);
	}

	return Status;
}

/*****************************************************************************/
/*
 * This function programs the eFUSEs with the PUF helper data.
 *
 * @param	PrgmPufHelperData	Pointer to the Xnvm_PufHelperData.
 *
 * @return
 *		- XST_SUCCESS if programs successfully.
 *		- Errorcode on failure
 *
 * @note	To generate PufSyndromeData please use
 *		XPuf_Registration API
 *
 *******************************************************************************/
u32 XNvm_EfuseWritePufHelperData(Xnvm_PufHelperData *PrgmPufHelperData)
{
	u32 Status = (u32)XST_FAILURE;

	if (PrgmPufHelperData == NULL)
	{
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfuseReadPufSecCtrlBits(
			&(PrgmPufHelperData->ReadPufSecCtrlBits));
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_RD_PUF_SEC_CTRL);
		goto END;
	}

	if ((PrgmPufHelperData->ReadPufSecCtrlBits.PufDis == TRUE) ||
		PrgmPufHelperData->ReadPufSecCtrlBits.PufSynLk == TRUE)
	{
		Status = ((u32)XNVM_EFUSE_ERR_FUSE_PROTECTED |
				(u32)XNVM_EFUSE_ERR_WRITE_PUF_HELPER_DATA);
		goto END;
	}

	Status = XNvm_EfuseCheckForZerosPuf();
	if (Status != (u32)XST_SUCCESS) {
		Status = (Status |
				(u32)XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
	}

	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
					XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (PrgmPufHelperData->PrgmSynData == TRUE) {
		Status = XNvm_EfuseWritePufSynData(PrgmPufHelperData);
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_PUF_SYN_DATA);
			goto END;
		}
	}
	if (PrgmPufHelperData->PrgmChash == TRUE) {
		Status = XNvm_EfuseWritePufChash(PrgmPufHelperData);
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_PUF_CHASH);
			goto END;
		}
	}
	if (PrgmPufHelperData->PrgmAux == TRUE) {
		Status = XNvm_EfuseWritePufAux(PrgmPufHelperData);
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_PUF_AUX);
		}
	}
END :
	XNvm_EfuseDisableProgramming();

	return Status;

}

/*****************************************************************************/
/*
 * This function is used to read the Puf eFUSE secure control bits from cache or
 * eFUSE based on user input provided.
 *
 * @param	ReadBackSecCtrlBits	Pointer to the Xnvm_PufSecCtrlBits
 *		which holds the read secure control bits.
 *
 * @return
 * 		- XST_SUCCESS if reads successfully
 *		- XST_FAILURE if reading is failed
 *
 *******************************************************************************/
u32 XNvm_EfuseReadPufSecCtrlBits(Xnvm_PufSecCtrlBits *ReadBackPufSecCtrlBits)
{
	u32 Status = (u32)XST_FAILURE;

	if (ReadBackPufSecCtrlBits == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
	}
	else {
		Status = XNvm_EfuseReadPufSecCtrlBitsRegs(
					ReadBackPufSecCtrlBits, 0);
	}

	return Status;
}

/*****************************************************************************/
/*
 * This function is used to read the Misc eFUSE control bits from cache or
 * eFUSE based on user input provided.
 *
 * @param	ReadBackSecCtrlBits	Pointer to the Xnvm_MiscCtrlBits
 *		which holds the read secure control bits.
 *
 * @return
 * 		- XST_SUCCESS if reads successfully
 *		- XST_FAILURE if reading is failed
 *
 *******************************************************************************/
u32 XNvm_EfuseReadMiscCtrlBits(Xnvm_MiscCtrlBits *ReadMiscCtrlBits)
{
	u32 Status = (u32)XST_FAILURE;

	if (ReadMiscCtrlBits == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
	}
	else {
		Status = XNvm_EfuseReadMiscCtrlBitsRegs(ReadMiscCtrlBits, 0);
	}

	return Status;
}

/*****************************************************************************/
/*
 * This function programs the eFUSEs with the IV.
 *
 * @param	EfuseIv		Pointer to the Xnvm_Iv.
 *
 * @param	XNvm_IvType	Type od the IV to be programmed.
 *
 * @return
 *		- XST_SUCCESS if programs successfully.
 *		- Errorcode on failure
 *
 *******************************************************************************/
u32 XNvm_EfuseWriteIv(Xnvm_Iv *EfuseIv, XNvm_IvType IvType)
{
	u32 Status = (u32)XST_FAILURE;
	u32 LockStatus;
	u32 Row;

	if (EfuseIv == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	if ((IvType != XNVM_EFUSE_BLACK_OBFUS_IV_TYPE) &&
		(IvType != XNVM_EFUSE_PLML_IV_TYPE) &&
		(IvType != XNVM_EFUSE_DATA_PARTITION_IV_TYPE) &&
		(IvType != XNVM_EFUSE_META_HEADER_IV_TYPE)) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
						XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (EfuseIv->PgrmMetaHeaderIv == TRUE) {
		for (Row = 0; Row < 3; Row++) {
			Status = Xnvm_PrgmIvRow(EfuseIv->Iv[Row],
				(XNVM_EFUSE_META_HEADER_IV_START_ROW + Row));
			if (Status != XST_SUCCESS) {
				Status = (Status |
					(u32)XNVM_EFUSE_ERR_WRITE_META_HEADER_IV);
				goto END;
			}
		}
	}
	if (EfuseIv->PgrmBlkObfusIv == TRUE) {
		for (Row = 0; Row < 3; Row++) {
			Status = Xnvm_PrgmIvRow(EfuseIv->Iv[Row],
				(XNVM_EFUSE_BLACK_OBFUS_IV_START_ROW + Row));
			if (Status != XST_SUCCESS) {
				Status = (Status |
					(u32)XNVM_EFUSE_ERR_WRITE_BLK_OBFUS_IV);
				goto END;
			}
		}
	}
	if (EfuseIv->PgmPlmlIv == TRUE) {
		for (Row = 0; Row < 3; Row++) {
			Status = Xnvm_PrgmIvRow(EfuseIv->Iv[Row],
				(XNVM_EFUSE_PLML_IV_START_ROW + Row));
			if (Status != XST_SUCCESS) {
				Status = (Status |
					(u32)XNVM_EFUSE_ERR_WRITE_PLML_IV);
				goto END;
			}
		}
	}
	if (EfuseIv->PgmDataPartitionIv == TRUE) {
		for (Row = 0; Row < 3; Row++) {
			Status = Xnvm_PrgmIvRow(EfuseIv->Iv[Row],
				(XNVM_EFUSE_DATA_PARTITION_IV_START_ROW + Row));
			if (Status != XST_SUCCESS) {
				Status = (Status |
					(u32)XNVM_EFUSE_ERR_WRITE_DATA_PARTITION_IV);
				goto END;
			}
		}

	}
	Status = XNvm_EfuseCacheLoad();

END :
	XNvm_EfuseDisableProgramming();
	LockStatus = XNvm_EfuseLockController();
	if (XST_SUCCESS == Status) {
		Status = LockStatus;
	}
	return Status;
}

/*****************************************************************************/
/*
 * This function programs the eFUSE row with the IV.
 *
 * @param	EfuseIv		Pointer to the Xnvm_Iv.
 *
 * @param	IvRow		IV Row to be programmed.
 *
 * @return
 *		- XST_SUCCESS if programs successfully.
 *		- Errorcode on failure
 *
 ********************************************************************************/
static u32 Xnvm_PrgmIvRow(u32 EfuseData, u32 IvRow)
{
	u32 Status = XST_FAILURE;
	u8 IvRowInBits[XNVM_EFUSE_MAX_BITS_IN_ROW] = {0};
	u8 IvRowInBitsRd[XNVM_EFUSE_MAX_BITS_IN_ROW] = {0};
	u32 ReadIv;
	u32 IvRowInBytes;
	u32 Column;

	XNvm_ConvertBitsToBytes((u8 *)&EfuseData,
			IvRowInBits, XNVM_EFUSE_MAX_BITS_IN_ROW);
	Status = XNvm_EfuseReadCache(IvRow, &ReadIv);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	XNvm_ConvertBitsToBytes((u8 *)&ReadIv, IvRowInBitsRd,
			XNVM_EFUSE_MAX_BITS_IN_ROW);
	for (Column = 0U; Column < XNVM_EFUSE_MAX_BITS_IN_ROW;
			Column++) {
		if ((IvRowInBits[Column] == 0U) &&
				(IvRowInBitsRd[Column] == 1U)) {
			Status = (u32)XNVM_EFUSE_ERR_BEFORE_PROGRAMMING |
				(u32)XNVM_EFUSE_ERR_BIT_CANT_REVERT;
			goto END;
		}
		if ((IvRowInBits[Column] == 1U) &&
				(IvRowInBitsRd[Column] == 1U)) {
			IvRowInBits[Column] = 0U;
		}
	}
	XNvm_ConvertBytesToBits(IvRowInBits, (u8 *)&IvRowInBytes,
			XNVM_EFUSE_MAX_BITS_IN_ROW);

	Status = XNvm_EfusePgmRows(IvRow, 0x1U,
			XNVM_EFUSE_PAGE_0, &IvRowInBytes);
END:
	return Status;
}

/*****************************************************************************/
/*
 * This function is used to read IV eFUSE bits from cache
 *
 * @param	EfuseIv		Pointer to the Xnvm_Iv
 *
 * @param	IvType		Type of the Iv to be programmed
 *
 * @return
 * 		- XST_SUCCESS if reads successfully
 *  		- XST_FAILURE if reading is failed
 *
 *******************************************************************************/
u32 XNvm_EfuseReadIv(Xnvm_Iv *EfuseIv, XNvm_IvType IvType)
{
	u32 Status = (u32)XST_FAILURE;

	if (EfuseIv == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	if ((IvType != XNVM_EFUSE_BLACK_OBFUS_IV_TYPE) &&
		(IvType != XNVM_EFUSE_PLML_IV_TYPE) &&
		(IvType != XNVM_EFUSE_DATA_PARTITION_IV_TYPE) &&
		(IvType != XNVM_EFUSE_META_HEADER_IV_TYPE)) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (IvType == XNVM_EFUSE_META_HEADER_IV_TYPE) {
		Status = XNvm_EfuseReadCacheRange(
				XNVM_EFUSE_META_HEADER_IV_START_ROW,
				XNVM_EFUSE_IV_NUM_OF_ROWS,
				&(EfuseIv->Iv[0]));
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_RD_META_HEADER_IV);
			goto END;
		}
	}
	else if (IvType == XNVM_EFUSE_BLACK_OBFUS_IV_TYPE) {
		Status = XNvm_EfuseReadCacheRange(
					XNVM_EFUSE_BLACK_OBFUS_IV_START_ROW,
					XNVM_EFUSE_IV_NUM_OF_ROWS,
					&(EfuseIv->Iv[0]));
		if (Status != XST_SUCCESS) {
			Status = (Status |
					(u32)XNVM_EFUSE_ERR_RD_BLACK_OBFUS_IV);
			goto END;
		}
	}
	else if (IvType == XNVM_EFUSE_PLML_IV_TYPE) {
		Status = XNvm_EfuseReadCacheRange(XNVM_EFUSE_PLML_IV_START_ROW,
						XNVM_EFUSE_IV_NUM_OF_ROWS,
						&(EfuseIv->Iv)[0]);
		if (Status != XST_SUCCESS) {
			Status = (Status | (u32)XNVM_EFUSE_ERR_RD_PLML_IV);
			goto END;
		}
	}
	else if (IvType == XNVM_EFUSE_DATA_PARTITION_IV_TYPE) {
		Status = XNvm_EfuseReadCacheRange(
					XNVM_EFUSE_DATA_PARTITION_IV_START_ROW,
					XNVM_EFUSE_IV_NUM_OF_ROWS,
					&(EfuseIv->Iv[0]));
		if (Status != XST_SUCCESS) {
		Status = (Status | (u32)XNVM_EFUSE_ERR_RD_DATA_PARTITION_IV);
			goto END;
		}
	}
	Status = (u32)XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/*
 * This function reads the PUF helper data from eFUSE cache.
 *
 * @param	RdPufHelperData		Pointer to Xnvm_PufHelperData which hold
 * 					the PUF helper data read from eFUSEs.
 *
 * @return
 *		- XST_SUCCESS if reads successfully.
 *		- Errorcode on failure.
 *
 *******************************************************************************/
u32 XNvm_EfuseReadPufHelperData(Xnvm_PufHelperData *RdPufHelperData)
{
	u32 Status = (u32)XST_FAILURE;

	if (RdPufHelperData == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfuseReadCacheRange(XNVM_EFUSE_PUF_SYN_START_ROW,
					XNVM_EFUSE_PUF_SYN_DATA_NUM_OF_ROWS,
					&(RdPufHelperData->EfuseSynData[0]));
	if (Status != XST_SUCCESS) {
		Status = (Status | (u32)XNVM_EFUSE_ERR_RD_PUF_SYN_DATA);
		goto END;
	}
	Status = XNvm_EfuseReadCache(XNVM_EFUSE_PUF_CHASH_ROW,
					&(RdPufHelperData->Chash));
	if (Status != XST_SUCCESS) {
		Status = (Status | (u32)XNVM_EFUSE_ERR_RD_PUF_CHASH);
		goto END;
	}
	Status = XNvm_EfuseReadCache(XNVM_EFUSE_PUF_AUX_ROW,
				&(RdPufHelperData->Aux));
	if (Status != XST_SUCCESS) {
		Status = (Status | (u32)XNVM_EFUSE_ERR_RD_PUF_AUX);
	}
END:
	return Status;
}

/*****************************************************************************/
/*
 * This function is used to read Dna eFUSE bits from cache
 *
 * @param	EfuseDna		Pointer to the Xnvm_Dna
 *
 * @return
 * 		- XST_SUCCESS if reads successfully
 *		- XST_FAILURE if reading is failed
 *
 *******************************************************************************/
u32 XNvm_EfuseReadDna(Xnvm_Dna *EfuseDna)
{
	u32 Status = (u32)XST_FAILURE;

	if (EfuseDna == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfuseReadCacheRange(XNVM_EFUSE_DNA_START_ROW,
					XNVM_EFUSE_DNA_NUM_OF_ROWS,
					&(EfuseDna->Dna[0]));
	if (Status != XST_SUCCESS) {
		Status = (Status | (u32)XNVM_EFUSE_ERR_RD_DNA);
		goto END;
	}
END:
	return Status;
}

/*****************************************************************************/
/*
 * This function is used to read DecEfuseOnly eFUSE bits from cache
 *
 * @param	DecOnly		Pointer to the DecOnly efuse data
 *
 * @return
 * 		- XST_SUCCESS if reads successfully
 *		- XST_FAILURE if reading is failed
 *
 *******************************************************************************/
u32 XNvm_EfuseReadDecOnly(u32* DecOnly)
{
	u32 Status = (u32)XST_FAILURE;

	if (DecOnly == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfuseReadCache(XNVM_EFUSE_DEC_EFUSE_ONLY_ROW,
					DecOnly);
	if (Status != XST_SUCCESS) {
		Status = (Status | (u32)XNVM_EFUSE_ERR_RD_DEC_ONLY);
	}
END:
	return Status;
}

/*****************************************************************************/
/*
 * This function reads the Ppk Hash from eFUSE cache.
 *
 * @param	EfusePpk	Pointer to Xnvm_PpkHash which hold
 * 				the PpkHash from eFUSE Cache.
 *
 * @param 	PpkType		Type of the Ppk to be programmed
 *
 * @return
 *		- XST_SUCCESS if reads successfully.
 *		- Errorcode on failure.
 *******************************************************************************/

u32 XNvm_EfuseReadPpkHash(Xnvm_PpkHash *EfusePpk, XNvm_PpkType PpkType)
{
	u32 Status = (u32)XST_FAILURE;
	u32 PpkRow;

	if (EfusePpk == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	if ((PpkType != XNVM_EFUSE_PPK0) &&
		(PpkType != XNVM_EFUSE_PPK1) &&
		(PpkType != XNVM_EFUSE_PPK2)) {

		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	if (PpkType == XNVM_EFUSE_PPK0) {
		PpkRow = XNVM_EFUSE_PPK_0_HASH_START_ROW;

	} else if (PpkType == XNVM_EFUSE_PPK1) {
		PpkRow = XNVM_EFUSE_PPK_1_HASH_START_ROW;
	} else {
		PpkRow = XNVM_EFUSE_PPK_2_HASH_START_ROW;
	}

	Status = XNvm_EfuseReadCacheRange(PpkRow,
					XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS,
					&(EfusePpk->Hash[0]));
	if (Status != XST_SUCCESS) {
		Status = (Status | (u32)XNVM_EFUSE_ERR_RD_PPK_HASH);
		goto END;
	}
END:
	return Status;
}

/*****************************************************************************/
/*
 * This function revokes the Ppk.
 *
 * @param	PpkRevoke	Pointer to Xnvm_RevokePpkFlags that contains
 * 				which Ppk to revoke.
 *
 * @return
 *		- XST_SUCCESS if reads successfully.
 *		- Errorcode on failure.
 *******************************************************************************/

u32 Xnvm_EfuseRevokePpk(Xnvm_RevokePpkFlags *PpkRevoke)
{
	u32 Status = (u32)XST_FAILURE;
	u32 LockStatus;
	u8 DataInBits[XNVM_EFUSE_MAX_BITS_IN_ROW] = {0};
	u32 RowData;

	if (PpkRevoke == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if ((PpkRevoke->Ppk0 != 0x00U) ||
		(PpkRevoke->Ppk1 != 0x00U) ||
		(PpkRevoke->Ppk2 != 0x00U)) {

		Status = XNvm_EfuseReadCache(XNVM_EFUSE_MISC_CTRL_ROW,
						&RowData);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_RD_MISC_CTRL_BITS);
			goto END;
		}

		XNvm_ConvertBitsToBytes((u8 *) &RowData,
			DataInBits, XNVM_EFUSE_MAX_BITS_IN_ROW);

		Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
						XNVM_EFUSE_MARGIN_RD);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	if (PpkRevoke->Ppk0 != 0x00U) {
		if (DataInBits[XNVM_EFUSE_MISC_PPK0_INVALID_BIT_1] == 0x00U) {
			Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
					XNVM_EFUSE_MISC_CTRL_ROW,
					XNVM_EFUSE_MISC_PPK0_INVALID_BIT_1);
			if (Status != XST_SUCCESS) {
				Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_PPK0_INVALID_BIT_1);
				goto END;
			}
		}
		if (DataInBits[XNVM_EFUSE_MISC_PPK0_INVALID_BIT_2] == 0x00U) {
			Status = XNvm_EfusePgmAndVerifyBit(
					XNVM_EFUSE_PAGE_0,
					XNVM_EFUSE_MISC_CTRL_ROW,
					XNVM_EFUSE_MISC_PPK0_INVALID_BIT_2);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_PPK0_INVALID_BIT_2);
				goto END;
			}
		}
	}
	if (PpkRevoke->Ppk1 != 0x00U) {
		if (DataInBits[XNVM_EFUSE_MISC_PPK1_INVALID_BIT_1] == 0x00U) {
			Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
					XNVM_EFUSE_MISC_CTRL_ROW,
					XNVM_EFUSE_MISC_PPK1_INVALID_BIT_1);
			if (Status != XST_SUCCESS) {
				Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_PPK1_INVALID_BIT_1);
				goto END;
			}
		}
		if (DataInBits[XNVM_EFUSE_MISC_PPK1_INVALID_BIT_2] == 0x00U) {
			Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
					XNVM_EFUSE_MISC_CTRL_ROW,
					XNVM_EFUSE_MISC_PPK1_INVALID_BIT_2);
			if (Status != XST_SUCCESS) {
				Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_PPK1_INVALID_BIT_2);
				goto END;
			}
		}
	}
	if (PpkRevoke->Ppk2 != 0x00U) {
		if (DataInBits[XNVM_EFUSE_MISC_PPK2_INVALID_BIT_1] == 0x00U) {
			Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
					XNVM_EFUSE_MISC_CTRL_ROW,
					XNVM_EFUSE_MISC_PPK2_INVALID_BIT_1);
			if (Status != XST_SUCCESS) {
				Status =
				(Status |
				(u32)XNVM_EFUSE_ERR_WRITE_PPK2_INVALID_BIT_1);
				goto END;
			}
		}
		if (DataInBits[XNVM_EFUSE_MISC_PPK2_INVALID_BIT_2] == 0x00U) {
			Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
					XNVM_EFUSE_MISC_CTRL_ROW,
					XNVM_EFUSE_MISC_PPK2_INVALID_BIT_2);
			if (Status != XST_SUCCESS) {
				Status =
				(Status |
				(u32)XNVM_EFUSE_ERR_WRITE_PPK2_INVALID_BIT_2);
				goto END;
			}
		}
	}
	Status = XNvm_EfuseCacheLoad();
END:
	XNvm_EfuseDisableProgramming();
	LockStatus = XNvm_EfuseLockController();
	if (XST_SUCCESS == Status) {
		Status = LockStatus;
	}
	return Status;
}

/*****************************************************************************/
/*
 * This function revokes the Spk.
 *
 * @param	WriteRevokeId	Pointer to Xnvm_RevokeSpk that contains
 * 				which Spk to revoke.
 *
 * @return
 *		- XST_SUCCESS if reads successfully.
 *		- Errorcode on failure.
 *******************************************************************************/
u32 Xnvm_EfuseRevokeSpk(Xnvm_RevokeIdEfuse *WriteRevokeId)
{
	u32 Status = (u32)XST_FAILURE;
	u32 LockStatus;

	if (WriteRevokeId == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if ((WriteRevokeId->CheckSpkEfuseToRevoke.RevokeId0 != 0x00U) ||
		(WriteRevokeId->CheckSpkEfuseToRevoke.RevokeId1 != 0x00U) ||
		(WriteRevokeId->CheckSpkEfuseToRevoke.RevokeId2 != 0x00U) ||
		(WriteRevokeId->CheckSpkEfuseToRevoke.RevokeId3 != 0x00U) ||
		(WriteRevokeId->CheckSpkEfuseToRevoke.RevokeId4 != 0x00U) ||
		(WriteRevokeId->CheckSpkEfuseToRevoke.RevokeId5 != 0x00U) ||
		(WriteRevokeId->CheckSpkEfuseToRevoke.RevokeId6 != 0x00U) ||
		(WriteRevokeId->CheckSpkEfuseToRevoke.RevokeId7 != 0x00U)) {

		if (XNvm_Efuse_ReadReg(XNVM_EFUSE_CTRL_BASEADDR,
			XNVM_EFUSE_PGM_LOCK_REG_OFFSET) != 0x00U) {
			Status = ((u32)XNVM_EFUSE_ERR_FUSE_PROTECTED |
				(u32)XNVM_EFUSE_ERR_WRITE_REVOCATION_IDS);
			goto END;
		}

		Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
						XNVM_EFUSE_MARGIN_RD);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	if (WriteRevokeId->CheckSpkEfuseToRevoke.RevokeId0 != 0x00U) {

		Status = Xnvm_PrgmRevocationId(XNVM_EFUSE_REVOCATION_ID_0,
						WriteRevokeId->RevokeId0);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_0);
			goto END;
		}
	}
	if (WriteRevokeId->CheckSpkEfuseToRevoke.RevokeId1 != 0x00U) {

		Status = Xnvm_PrgmRevocationId(XNVM_EFUSE_REVOCATION_ID_1,
						WriteRevokeId->RevokeId1);
		if (Status != XST_SUCCESS) {
			Status = (Status |
			(u32)XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_1);
			goto END;
		}
	}
	if (WriteRevokeId->CheckSpkEfuseToRevoke.RevokeId2 != 0x00U) {

		Status = Xnvm_PrgmRevocationId(XNVM_EFUSE_REVOCATION_ID_2,
						WriteRevokeId->RevokeId2);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_2);
			goto END;
		}

	}
	if (WriteRevokeId->CheckSpkEfuseToRevoke.RevokeId3 != 0x00U) {

		Status = Xnvm_PrgmRevocationId(XNVM_EFUSE_REVOCATION_ID_3,
						WriteRevokeId->RevokeId3);
		if (Status != XST_SUCCESS) {
			Status = (Status |
			(u32)XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_3);
			goto END;
		}
	}
	if (WriteRevokeId->CheckSpkEfuseToRevoke.RevokeId4 != 0x00U) {

		Status = Xnvm_PrgmRevocationId(XNVM_EFUSE_REVOCATION_ID_4,
						WriteRevokeId->RevokeId4);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_4);
			goto END;
		}
	}
	if (WriteRevokeId->CheckSpkEfuseToRevoke.RevokeId5 != 0x00U) {

		Status = Xnvm_PrgmRevocationId(XNVM_EFUSE_REVOCATION_ID_5,
						WriteRevokeId->RevokeId5);
		if (Status != XST_SUCCESS) {
		Status = (Status |
			(u32)XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_5);
			goto END;
		}
	}
	if (WriteRevokeId->CheckSpkEfuseToRevoke.RevokeId6 != 0x00U) {

		Status = Xnvm_PrgmRevocationId(XNVM_EFUSE_REVOCATION_ID_6,
						WriteRevokeId->RevokeId6);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_6);
			goto END;
		}
	}
	if (WriteRevokeId->CheckSpkEfuseToRevoke.RevokeId7 != 0x00U) {

		Status = Xnvm_PrgmRevocationId(XNVM_EFUSE_REVOCATION_ID_7,
						WriteRevokeId->RevokeId7);
		if (Status != XST_SUCCESS) {
			Status = (Status |
			(u32)XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_7);
			goto END;
		}
	}
	Status = XNvm_EfuseCacheLoad();
END:
	XNvm_EfuseDisableProgramming();
	LockStatus = XNvm_EfuseLockController();
	if (XST_SUCCESS == Status) {
		Status = LockStatus;
	}
	return Status;
}

/*****************************************************************************/
/*
 * This function reads the Revocation Fuse from eFUSE cache.
 *
 * @param	RevokeFusePtr	Pointer to Xnvm_PpkHash which hold
 * 				the PpkHash from eFUSE Cache.
 *
 * @param 	RevokeFuse_Num	Type of the Ppk to be programmed
 *
 * @return
 *		- XST_SUCCESS if reads successfully.
 *		- Errorcode on failure.
 *******************************************************************************/
u32 Xnvm_EfuseReadRevocationFuse(u32 *RevokeFusePtr, u8 RevokeFuse_Num)
{
	u32 Status = (u32)XST_FAILURE;
	u32 Row;

	if (RevokeFusePtr == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	if (RevokeFuse_Num > XNVM_EFUSE_REVOCATION_ID_7) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
	}
	else {
		Row = XNVM_EFUSE_REVOCATION_ID_0_ROW + RevokeFuse_Num;
		Status = XNvm_EfuseReadCache(Row, RevokeFusePtr);
	}
END :
	return Status;
}


/*****************************************************************************/
/*
 * This function is used verify eFUSE keys for Zeros before programming.
 *
 * @param	WriteNvm is a pointer to eFUSE ps instance.
 *
 * @return
 *		- XST_SUCCESS if keys are not programmed
 *		- ErrorCode if keys are already programmed.
 *
 *******************************************************************************/
static u32 XNvm_EfuseCheckZerosBfrPrgrmg(Xnvm_EfuseWriteData *WriteNvm)
{
	u32 Status = (u32)XST_FAILURE;

	if (WriteNvm == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_RD,
					XNVM_EFUSE_NORMAL_RD);
	if(Status != XST_SUCCESS) {
		goto END;
	}

	if (WriteNvm->CheckWriteFlags.AesKey == TRUE) {
		Status = XNvm_EfuseCheckAesKeyCrc(XNVM_EFUSE_CRC_AES_ZEROS);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XNVM_EFUSE_ERR_AES_ALREADY_PRGMD;
			goto END;
		}
	}
	if (WriteNvm->CheckWriteFlags.UserKey0 == TRUE) {
		Status = XNvm_EfuseCheckUserKey0Crc(XNVM_EFUSE_CRC_AES_ZEROS);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XNVM_EFUSE_ERR_USER_KEY0_ALREADY_PRGMD;
			goto END;
		}
	}
	if (WriteNvm->CheckWriteFlags.UserKey1 == TRUE) {
		Status = XNvm_EfuseCheckUserKey1Crc(XNVM_EFUSE_CRC_AES_ZEROS);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XNVM_EFUSE_ERR_USER_KEY1_ALREADY_PRGMD;
			goto END;
		}
	}
	if (WriteNvm->CheckWriteFlags.Ppk0Hash == TRUE) {
		Status = XNvm_EfuseCheckZeros(XNVM_EFUSE_PPK_0_HASH_START_ROW,
					(XNVM_EFUSE_PPK_0_HASH_START_ROW +
					XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS),
					XNVM_EFUSE_PAGE_0);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XNVM_EFUSE_ERR_PPK0_HASH_ALREADY_PRGMD;
			goto END;
		}
	}
	if (WriteNvm->CheckWriteFlags.Ppk1Hash == TRUE) {
		Status = XNvm_EfuseCheckZeros(XNVM_EFUSE_PPK_1_HASH_START_ROW,
					(XNVM_EFUSE_PPK_1_HASH_START_ROW +
					XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS),
					XNVM_EFUSE_PAGE_0);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XNVM_EFUSE_ERR_PPK1_HASH_ALREADY_PRGMD;
			goto END;
		}
	}
	if (WriteNvm->CheckWriteFlags.Ppk2Hash == TRUE) {
		Status = XNvm_EfuseCheckZeros(XNVM_EFUSE_PPK_2_HASH_START_ROW,
					(XNVM_EFUSE_PPK_2_HASH_START_ROW +
					XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS),
					XNVM_EFUSE_PAGE_0);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XNVM_EFUSE_ERR_PPK2_HASH_ALREADY_PRGMD;
			goto END;
		}
	}
END:
	return Status;
}

/*****************************************************************************/
/*
 * This function is used verify eFUSE keys for Zeros
 *
 * @param	RowStart is row number from which verification has to be
 *		started.
 * @param	RowEnd is row number till which verification has to be
 *		ended.
 * @param	EfuseType is the type of the eFUSE in which these rows reside.
 *
 * @return	XST_SUCCESS if keys are not programmed.
 * 		Errorcode on failure.
 *
 *******************************************************************************/
static u32 XNvm_EfuseCheckZeros(u32 RowStart, u32 RowEnd,
				XNvm_EfuseType EfuseType)
{
	u32 Status = (u32)XST_FAILURE;
	u8 Row;
	u32 RowDataVal = 0U;

	for (Row = RowStart; Row < RowEnd; Row++) {
		Status = XNvm_EfuseReadRow(EfuseType, Row, &RowDataVal);
		if ((Status != (u32)XST_SUCCESS) ||
			(RowDataVal != 0x00U)) {
			break;
		}
	}
	return Status;
}

/*****************************************************************************/
/*
 * This function performs pre checks for programming all the specified bits.
 *
 * @param	WriteNvm is the pointer to the XilSKey_ZynqMpEPs.
 *
 * @return
 *		XST_SUCCESS - if all the conditions for programming is satisfied
 *		Errorcode - if any of the conditions are not met
 *
 *******************************************************************************/
static u32 XNvm_EfuseWriteChecks(Xnvm_EfuseWriteData *WriteNvm)
{
	u32 Status = (u32)XST_FAILURE;
	u32 SecurityMisc0;

	if (WriteNvm == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfuseReadSecCtrlBits(
				&(WriteNvm->ReadBackSecCtrlBits));
	if(Status != (u32)XST_SUCCESS) {
		Status = XNVM_EFUSE_ERR_RD_SEC_CTRL_BITS;
		goto END;
	}

	if (WriteNvm->CheckWriteFlags.AesKey == TRUE) {
		if ((WriteNvm->ReadBackSecCtrlBits.AesDis == TRUE) ||
			(WriteNvm->ReadBackSecCtrlBits.AesWrLk == TRUE)) {
			Status = ((u32)XNVM_EFUSE_ERR_FUSE_PROTECTED |
					(u32)XNVM_EFUSE_ERR_WRITE_AES_KEY);
			goto END;
		}
	}
	if (WriteNvm->CheckWriteFlags.UserKey0 == TRUE) {
		if (WriteNvm->ReadBackSecCtrlBits.UserKey0WrLk == TRUE) {
			Status = ((u32)XNVM_EFUSE_ERR_FUSE_PROTECTED |
					(u32)XNVM_EFUSE_ERR_WRITE_USER0_KEY);
			goto END;
		}
	}
	if (WriteNvm->CheckWriteFlags.UserKey1 == TRUE) {
		if (WriteNvm->ReadBackSecCtrlBits.UserKey1WrLk == TRUE) {
			Status = ((u32)XNVM_EFUSE_ERR_FUSE_PROTECTED |
				(u32)XNVM_EFUSE_ERR_WRITE_USER1_KEY);
			goto END;
		}
	}
	if (WriteNvm->CheckWriteFlags.Ppk0Hash == TRUE) {
		if (WriteNvm->ReadBackSecCtrlBits.Ppk0WrLk == TRUE) {
			Status = ((u32)XNVM_EFUSE_ERR_FUSE_PROTECTED |
				(u32)XNVM_EFUSE_ERR_WRITE_PPK0_HASH);
			goto END;
		}
	}
	if (WriteNvm->CheckWriteFlags.Ppk1Hash == TRUE) {
		if (WriteNvm->ReadBackSecCtrlBits.Ppk1WrLk == TRUE) {
			Status = ((u32)XNVM_EFUSE_ERR_FUSE_PROTECTED |
				(u32)XNVM_EFUSE_ERR_WRITE_PPK1_HASH);
			goto END;
		}
	}
	if (WriteNvm->CheckWriteFlags.Ppk2Hash == TRUE) {
		if (WriteNvm->ReadBackSecCtrlBits.Ppk2WrLk == TRUE) {
			Status = ((u32)XNVM_EFUSE_ERR_FUSE_PROTECTED |
				(u32)XNVM_EFUSE_ERR_WRITE_PPK2_HASH);
			goto END;
		}
	}
	if (WriteNvm->CheckWriteFlags.DecOnly == TRUE) {
		SecurityMisc0 = XNvm_Efuse_ReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_SECURITY_MISC_0_OFFSET);
		if ((SecurityMisc0 &
		XNVM_EFUSE_CACHE_SECURITY_MISC_0_DEC_EFUSE_ONLY_MASK) !=
		0x00U) {
			Status =
			(u32)XNVM_EFUSE_ERR_DEC_EFUSE_ONLY_ALREADY_PRGMD;
		}
	}
END :
	return Status;
}

/*****************************************************************************/
/*
 * This function is used to read the eFUSE secure control bits from cache
 * or from eFUSE array based on user selection.
 *
 * @param	ReadBackSecCtrlBits is the pointer to the Xnvm_SecCtrlBits
 *		which holds the read secure control bits.
 * @param	ReadOption Indicates whether or not to read from the actual
 * 		eFUSE array or from the eFUSE cache.
 *		- 0(XNVM_EFUSE_RD_FROM_CACHE) Reads from eFUSE cache
 *		- 1(XNVM_EFUSE_READ_FROM_EFUSE) Reads from eFUSE array
 *
 * @return
 * 		- XST_SUCCESS if reads successfully
 *		- XST_FAILURE if reading is failed
 *
 * ******************************************************************************/
static u32 XNvm_EfuseReadSecCtrlBitsRegs(Xnvm_SecCtrlBits *ReadbackSecCtrlBits,
					u8 ReadOption)
{
	u32 RegData = 0U;
	u32 Status = (u32)XST_FAILURE;

	if (ReadOption == XNVM_EFUSE_RD_FROM_CACHE) {
		Status = XNvm_EfuseReadCache(XNVM_EFUSE_SECURITY_CONTROL_ROW,
						&RegData);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
	}
	else {
		Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_RD,
						XNVM_EFUSE_NORMAL_RD);
		if(Status != XST_SUCCESS) {
			goto END;
		}
		Status = XNvm_EfuseReadRow(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_SECURITY_CONTROL_ROW,
				&RegData);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
	}
	ReadbackSecCtrlBits->AesDis =
		(u8)(RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_AES_DIS_MASK);
	ReadbackSecCtrlBits->JtagErrOutDis =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_ERROR_OUT_DIS_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_JTAG_ERROR_OUT_DIS_SHIFT);
	ReadbackSecCtrlBits->JtagDis =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_JTAG_DIS_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_JTAG_DIS_SHIFT);
	ReadbackSecCtrlBits->HwTstBitsDis =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_HWTSTBITS_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_HWTSTBITS_DIS_SHIFT);
	ReadbackSecCtrlBits->IpDisWrLk =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_IPDIS_WR_LK_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_IPDIS_WR_LK_SHIFT);
	ReadbackSecCtrlBits->Ppk0WrLk =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK0_WR_LK_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK0_WR_LK_SHIFT);
	ReadbackSecCtrlBits->Ppk1WrLk =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK1_WR_LK_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK1_WR_LK_SHIFT);
	ReadbackSecCtrlBits->Ppk2WrLk =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK2_WR_LK_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK2_WR_LK_SHIFT);
	ReadbackSecCtrlBits->AesCrcLk =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_AES_CRC_LK_1_0_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_AES_CRC_LK_1_0_SHIFT);
	ReadbackSecCtrlBits->AesWrLk =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_AES_WR_LK_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_AES_WR_LK_SHIFT);
	ReadbackSecCtrlBits->UserKey0CrcLk =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_0_CRC_LK_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_0_CRC_LK_0_SHIFT);
	ReadbackSecCtrlBits->UserKey0WrLk =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_0_WR_LK_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_0_WR_LK_SHIFT);
	ReadbackSecCtrlBits->UserKey1CrcLk =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_1_CRC_LK_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_1_CRC_LK_0_SHIFT);
	ReadbackSecCtrlBits->UserKey1WrLk =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_1_WR_LK_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_1_WR_LK_SHIFT);
	ReadbackSecCtrlBits->SecDbgDis =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_SEC_DEBUG_DIS_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_SEC_DEBUG_DIS_1_0_SHIFT);
	ReadbackSecCtrlBits->PmcScEn =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PMC_SC_EN_2_0_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PMC_SC_EN_2_0_SHIFT);
	ReadbackSecCtrlBits->SvdWrLk =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_SVD_WR_LK_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_SVD_WR_LK_SHIFT);
	ReadbackSecCtrlBits->DnaWrLk =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_DNA_WR_LK_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_DNA_WR_LK_SHIFT);
	ReadbackSecCtrlBits->BootEnvWrLk =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_BOOT_ENV_WR_LK_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_BOOT_ENV_WR_LK_SHIFT);
	ReadbackSecCtrlBits->CacherWrLk =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_CAHER_WR_LK_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_CAHER_WR_LK_SHIFT);
	ReadbackSecCtrlBits->RegInitDis =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_REG_INIT_DIS_1_0_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_REG_INIT_DIS_1_0_SHIFT);
END:
	return Status;
}

/*****************************************************************************/
/*
 * This function programs the eFUSEs with the PUF syndrome data.
 *
 * @param	PrgmPufHelperData	Pointer to the Xnvm_PufHelperData.
 *
 * @return
 *		- XST_SUCCESS if programs successfully.
 *		- Errorcode on failure
 *
 * @note	To generate PufSyndromeData please use
 *		XPuf_Registration API
 *
 *******************************************************************************/
static u32 XNvm_EfuseWritePufSynData(Xnvm_PufHelperData *PrgmPufHelperData)
{
	u32 Status = (u32)XST_FAILURE;

	if (PrgmPufHelperData == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfusePgmRows(XNVM_EFUSE_PUF_SYN_START_ROW,
				XNVM_EFUSE_PUF_SYN_DATA_NUM_OF_ROWS,
				XNVM_EFUSE_PAGE_2,
				&(PrgmPufHelperData->EfuseSynData[0]));
END:
	return Status;
}

/*****************************************************************************/
/*
 * This function programs the eFUSEs with the PUF Chash.
 *
 * @param	PrgmPufHelperData	Pointer to the Xnvm_PufHelperData.
 *
 * @return
 *		- XST_SUCCESS if programs successfully.
 *		- Errorcode on failure
 *
 *******************************************************************************/
static u32 XNvm_EfuseWritePufChash(Xnvm_PufHelperData *PrgmPufHelperData)
{
	u32 Status = (u32)XST_FAILURE;
	u32 Chash;

	if (PrgmPufHelperData == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Chash = PrgmPufHelperData->Chash;

	Status = XNvm_EfusePgmRows(XNVM_EFUSE_PUF_CHASH_ROW,
				XNVM_EFUSE_PUF_CHASH_NUM_OF_ROWS,
				XNVM_EFUSE_PAGE_0, &Chash);
END:
	return Status;
}

/*****************************************************************************/
/*
 * This function programs the eFUSEs with the PUF Aux.
 *
 * @param	PrgmPufHelperData	Pointer to the Xnvm_PufHelperData.
 *
 * @return
 *		- XST_SUCCESS if programs successfully.
 *		- Errorcode on failure
 *
 *******************************************************************************/
static u32 XNvm_EfuseWritePufAux(Xnvm_PufHelperData *PrgmPufHelperData)
{
	u32 Status = (u32)XST_FAILURE;
	u8 Column;
	u32 AuxValue;

	if (PrgmPufHelperData == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	AuxValue = PrgmPufHelperData->Aux;
	for (Column = 0U; Column < XNVM_EFUSE_PUF_AUX_LEN_IN_BITS;
							Column++) {
		if ((AuxValue & 0x01) != 0x00U) {
			Status = XNvm_EfusePgmAndVerifyBit(
					XNVM_EFUSE_PAGE_0,
					XNVM_EFUSE_PUF_AUX_ROW,
					Column);
			if (Status != (u32)XST_SUCCESS) {
				goto END;
			}
		}
		AuxValue = AuxValue >> 1;
	}
END :
	return Status;

}
/***************************************************************************/
/*
 * This function checks whether PUF is already programmed or not.
 *
 * @return
 *		- XST_SUCCESS if all rows are zero
 *		- Errorcode if already programmed.
 *
 *******************************************************************************/
static u32 XNvm_EfuseCheckForZerosPuf(void)
{
	u32 Status = (u32)XST_FAILURE;
	u32 RowDataVal;

	Status = XNvm_EfuseCheckZeros(XNVM_EFUSE_PUF_SYN_START_ROW,
					(XNVM_EFUSE_PUF_SYN_START_ROW +
					XNVM_EFUSE_PUF_SYN_DATA_NUM_OF_ROWS),
					XNVM_EFUSE_PAGE_2);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XNVM_EFUSE_ERR_PUF_SYN_ALREADY_PRGMD;
		goto END;
	}

	Status = XNvm_EfuseCheckZeros(XNVM_EFUSE_PUF_CHASH_ROW,
					XNVM_EFUSE_PUF_CHASH_ROW,
					XNVM_EFUSE_PAGE_0);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XNVM_EFUSE_ERR_PUF_CHASH_ALREADY_PRGMD;
		goto END;
	}

	Status = XNvm_EfuseReadCache(XNVM_EFUSE_PUF_AUX_ROW, &RowDataVal);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}
	if ((RowDataVal &
		XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_PUF_ECC_23_0_MASK) != 0x00U) {
		Status = (u32)XNVM_EFUSE_ERR_PUF_AUX_ALREADY_PRGMD;
	}

END :
	return Status;

}

/*****************************************************************************/
/*
 * This function is used to read the eFUSE secure control bits from cache
 * or from eFUSE array based on user selection.
 *
 * @param	ReadBackPufSecCtrlBits is the pointer to Xnvm_PufSecCtrlBits
 * 		which holds the read secure control bits.
 * @param	ReadOption Indicates whether or not to read from the actual
 * 		eFUSE array or from the eFUSE cache.
 *		- 0(XNVM_EFUSE_RD_FROM_CACHE) Reads from eFUSE cache
 *		- 1(XNVM_EFUSE_RD_FROM_EFUSE) Reads from eFUSE array
 *
 *@return
 * 		- XST_SUCCESS if reads successfully
 * 		- XST_FAILURE if reading is failed
 *
 *******************************************************************************/
static u32 XNvm_EfuseReadPufSecCtrlBitsRegs(
	Xnvm_PufSecCtrlBits *ReadBackPufSecCtrlBits, u8 ReadOption)
{
	u32 PufEccCtrlReg = 0U;
	u32 PufSecurityCtrlReg = 0U;
	u32 Status = (u32)XST_FAILURE;

	if (ReadOption == XNVM_EFUSE_RD_FROM_CACHE) {
		Status = XNvm_EfuseReadCache(
			XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_OFFSET,
			&PufEccCtrlReg);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
		Status = XNvm_EfuseReadCache(
			XNVM_EFUSE_CACHE_SECURITY_CONTROL_OFFSET,
			&PufSecurityCtrlReg);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
	}
	else {
		Status = XNvm_EfuseReadRow(XNVM_EFUSE_PAGE_0,
			XNVM_EFUSE_SECURITY_CONTROL_ROW,
			&PufSecurityCtrlReg);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
		Status = XNvm_EfuseReadRow(XNVM_EFUSE_PAGE_0,
			XNVM_EFUSE_PUF_AUX_ROW,
			&PufEccCtrlReg);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
	}
	ReadBackPufSecCtrlBits->PufRegenDis =
		(u8)((PufEccCtrlReg &
		XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_PUF_REGEN_DIS_MASK) >>
		XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_PUF_REGEN_DIS_SHIFT);
	ReadBackPufSecCtrlBits->PufHdInvalid =
		(u8)((PufEccCtrlReg &
		XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_PUF_HD_INVLD_MASK) >>
		XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_PUF_HD_INVLD_SHIFT);
	ReadBackPufSecCtrlBits->PufTest2Dis =
		(u8)((PufSecurityCtrlReg &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_TEST2_DIS_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_TEST2_DIS_SHIFT);
	ReadBackPufSecCtrlBits->PufDis =
		(u8)((PufSecurityCtrlReg &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_DIS_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_DIS_SHIFT);
	ReadBackPufSecCtrlBits->PufSynLk =
		(u8)((PufSecurityCtrlReg &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_SYN_LK_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_SYN_LK_SHIFT);
END:
	return Status;
}

/*****************************************************************************/
/*
 * This function is used to read Misc control bits from cache
 * or from eFUSE array based on user selection.
 *
 * @param	ReadMiscCtrlBits is the pointer to Xnvm_MiscCtrlBits
 * 		which holds the read secure control bits.
 * @param	ReadOption Indicates whether or not to read from the actual
 * 		eFUSE array or from the eFUSE cache.
 *		- 0(XNVM_EFUSE_RD_FROM_CACHE) Reads from eFUSE cache
 *		- 1(XNVM_EFUSE_RD_FROM_EFUSE) Reads from eFUSE array
 *
 *@return
 * 		- XST_SUCCESS if reads successfully
 * 		- XST_FAILURE if reading is failed
 *
 *******************************************************************************/
static u32 XNvm_EfuseReadMiscCtrlBitsRegs(
	Xnvm_MiscCtrlBits *ReadMiscCtrlBits, u8 ReadOption)
{
	u32 ReadReg = 0U;
	u32 Status = (u32)XST_FAILURE;

	if (ReadMiscCtrlBits == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	if (ReadOption == XNVM_EFUSE_RD_FROM_CACHE) {
		Status = XNvm_EfuseReadCache(
			XNVM_EFUSE_MISC_CTRL_ROW,
			&ReadReg);
	}
	else {
		Status = XNvm_EfuseReadRow(XNVM_EFUSE_PAGE_0,
			XNVM_EFUSE_MISC_CTRL_ROW,
			&ReadReg);
	}

	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}
	ReadMiscCtrlBits->Ppk0Invalid =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_MISC_CTRL_PPK0_INVLD_1_0_MASK) >>
		XNVM_EFUSE_CACHE_MISC_CTRL_PPK0_INVLD_1_0_SHIFT);
	ReadMiscCtrlBits->Ppk1Invalid =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_MISC_CTRL_PPK1_INVLD_1_0_MASK) >>
		XNVM_EFUSE_CACHE_MISC_CTRL_PPK1_INVLD_1_0_SHIFT);
	ReadMiscCtrlBits->Ppk2Invalid =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_MISC_CTRL_PPK2_INVLD_1_0_MASK) >>
		XNVM_EFUSE_CACHE_MISC_CTRL_PPK2_INVLD_1_0_SHIFT);

	Status = (u32)XST_SUCCESS;
END :
	return Status;
}


/*****************************************************************************/
/*
 * This function Programs Revocation Id to the eFuse.
 *
 * @param	RevokeIdEfuseNum	Pointer to Xnvm_RevokeSpk that contains
 * 					which Spk to revoke.
 * @param 	EfuseData		EfuseData to be programmed to Revocation
 * 					eFuse.
 * @return
 *		- XST_SUCCESS if reads successfully.
 *		- Errorcode on failure.
 *******************************************************************************/
static u32 Xnvm_PrgmRevocationId(u32 RevokeIdEfuseNum, u32 EfuseData)
{
	u32 Status = (u32)XST_FAILURE;
	u8 RevokeIdInBits[XNVM_EFUSE_MAX_BITS_IN_ROW] = {0};
	u8 RevokeIdInBitsRd[XNVM_EFUSE_MAX_BITS_IN_ROW] = {0};
	u32 RevokeId;
	u32 Column;
	u32 Row;
	u32 RevokeIdInBytes;

	if (RevokeIdEfuseNum >= 8) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Row = RevokeIdEfuseNum + XNVM_EFUSE_REVOCATION_ID_0_ROW;

	XNvm_ConvertBitsToBytes((u8 *)&EfuseData,
			RevokeIdInBits, XNVM_EFUSE_MAX_BITS_IN_ROW);
	Status = XNvm_EfuseReadCache(Row, &RevokeId);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	XNvm_ConvertBitsToBytes((u8 *)&RevokeId, RevokeIdInBitsRd,
			XNVM_EFUSE_MAX_BITS_IN_ROW);
	for (Column = 0U; Column < XNVM_EFUSE_MAX_BITS_IN_ROW;
			Column++) {
		if ((RevokeIdInBits[Column] == 0U) &&
				(RevokeIdInBitsRd[Column] == 1U)) {
			Status = (u32)XNVM_EFUSE_ERR_BEFORE_PROGRAMMING |
				(u32)XNVM_EFUSE_ERR_BIT_CANT_REVERT;
			goto END;
		}
		if ((RevokeIdInBits[Column] == 1U) &&
			(RevokeIdInBitsRd[Column] == 1U)) {
			RevokeIdInBits[Column] = 0U;
		}
	}
	XNvm_ConvertBytesToBits(RevokeIdInBits, (u8 *)&RevokeIdInBytes,
					XNVM_EFUSE_MAX_BITS_IN_ROW);

	Status = XNvm_EfusePgmRows(Row, 0x1U,
			XNVM_EFUSE_PAGE_0, &RevokeIdInBytes);
END :
	return Status;
}

/******************************************************************************/
/**
 * @brief
 * This function sets and then verifies the specified bits in the eFUSE.
 *
 * @param	StartRow	Starting Row number (0-based addressing)
 *		RowCount	Number of Rows to be read
 *		RowData		Pointer to memory location where bitmap
 *				to be written is stored.
 *				Only bit set are used for programming eFUSE.
 *
 * @return
 *		XST_SUCCESS - 	Specified bit set in eFUSE
 *		XNVM_EFUSE_ERR_PGM_TIMEOUT - 	eFUSE programming timed out
 *		XNVM_EFUSE_ERR_PGM - 	eFUSE programming failed
 *		XNVM_EFUSE_ERR_PGM_VERIFY - 	Verification failed,
 *						specified bit is not set.
 *		XST_FAILURE - 	Unexpected error
 *
 ******************************************************************************/
u32 XNvm_EfusePgmRows(u32 StartRow, u8 RowCount,
			XNvm_EfuseType EfuseType, const u32* RowData)
{
	u32 Status = XST_FAILURE;
	u32 LockStatus = XST_FAILURE;
	u32 Data;
	u32 Row = StartRow;
	u32 Idx;

	if ((EfuseType != XNVM_EFUSE_PAGE_0) &&
		(EfuseType != XNVM_EFUSE_PAGE_1) &&
		(EfuseType != XNVM_EFUSE_PAGE_2)){

		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	if ((RowData == NULL) || (RowCount == 0)) {

		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
					XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseCheckForTBits();

	if (Status != XST_SUCCESS) {
		goto END;
	}

	do {
		Data = *RowData;
		Idx = 0;
		while(Data) {
			if(Data & 0x01) {
				Status = XNvm_EfusePgmAndVerifyBit(
					EfuseType, Row, Idx);
				if (Status != XST_SUCCESS) {
					goto END;
				}
			}
			Idx++;
			Data = Data >> 1;
		}

		RowCount--;
		Row++;
		RowData++;
	}
	while (RowCount > 0U);
END:
	LockStatus = XNvm_EfuseLockController();
	if (XST_SUCCESS == Status) {
		Status = LockStatus;
	}
	return Status;
}

/******************************************************************************/
/**
 * @brief
 * This function Locks the eFUSE Controller to prevent accidental writes to
 * eFUSE controller registers.
 *
 * @param	None.
 *
 * @return
 *		XST_SUCCESS - 	eFUSE controller locked
 *		XNVM_EFUSE_ERR_LOCK - Failed to lock eFUSE controller
 *					register access
 *		XST_FAILURE - 	Unexpected error
 *
 * @note	None.
 *
 ******************************************************************************/
static inline u32 XNvm_EfuseLockController(void)
{
	u32 Status = XST_FAILURE;
	u32 LockStatus;

	XNvm_Efuse_WriteReg(XNVM_EFUSE_CTRL_BASEADDR,
			XNVM_EFUSE_WR_LOCK_REG_OFFSET,
			~XNVM_EFUSE_WR_UNLOCK_PASSCODE);
	LockStatus = XNvm_Efuse_ReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_WR_LOCK_REG_OFFSET);
	if(0x01 == LockStatus)	{
		Status = XST_SUCCESS;
	}
	else {
		Status = XNVM_EFUSE_ERR_LOCK;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief
 * This function Unlocks the eFUSE Controller for writing to its registers.
 *
 * @param	None.
 *
 * @return
 *		XST_SUCCESS - 	eFUSE controller locked
 *		XNVM_EFUSE_ERR_UNLOCK - Failed to unlock eFUSE controller
 *						register access
 *		XST_FAILURE - 	Unexpected error
 *
 * @note	None.
 *
 ******************************************************************************/
static inline u32 XNvm_EfuseUnlockController(void)
{
	u32 Status = XST_FAILURE;
	u32 LockStatus;

	XNvm_Efuse_WriteReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_WR_LOCK_REG_OFFSET,
				XNVM_EFUSE_WR_UNLOCK_PASSCODE);
	LockStatus = XNvm_Efuse_ReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_WR_LOCK_REG_OFFSET);
	if(0x00 == LockStatus)	{
		Status = XST_SUCCESS;
	}
	else {
		Status = XNVM_EFUSE_ERR_UNLOCK;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief
 * This function disables power down of eFUSE macros.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
static inline void XNvm_EfuseDisablePowerDown(void)
{
	u32 PowerDownStatus;

	PowerDownStatus = XNvm_Efuse_ReadReg(XNVM_EFUSE_CTRL_BASEADDR,
						XNVM_EFUSE_PD_REG_OFFSET);
	if(XNVM_EFUSE_PD_ENABLE == PowerDownStatus) {
		/* When changing the Power Down state, wait a separation period
		 *  of 1us, before and after accessing the eFuse-Macro.
		 */
		usleep(XNVM_ONE_MICRO_SECOND);
		XNvm_Efuse_WriteReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_PD_REG_OFFSET,
					~XNVM_EFUSE_PD_ENABLE);
		usleep(XNVM_ONE_MICRO_SECOND);
	}
}

/******************************************************************************/
/**
 * @brief
 * This function sets read mode of eFUSE controller.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
static inline void XNvm_EfuseSetReadMode(XNvm_EfuseRdMode RdMode)
{
	if(XNVM_EFUSE_NORMAL_RD == RdMode) {
		XNvm_Efuse_WriteReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_CFG_REG_OFFSET,
					XNVM_EFUSE_CFG_NORMAL_RD);
	}
	else {
		XNvm_Efuse_WriteReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_CFG_REG_OFFSET,
					(XNVM_EFUSE_CFG_ENABLE_PGM |
					XNVM_EFUSE_CFG_MARGIN_RD));
	}
}

/******************************************************************************/
/**
 * @brief
 * This function sets reference clock of eFUSE controller.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
static inline void XNvm_EfuseSetRefClk(void)
{
	XNvm_Efuse_WriteReg(XNVM_CRP_BASE_ADDR,
				XNVM_CRP_EFUSE_REF_CLK_REG_OFFSET,
				XNVM_CRP_EFUSE_REF_CLK_SELSRC);
}

/******************************************************************************/
/**
 * @brief
 * This function enabled programming mode of eFUSE controller.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
static inline void XNvm_EfuseEnableProgramming(void)
{
	u32 Cfg = XNvm_Efuse_ReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_CFG_REG_OFFSET);

	Cfg = Cfg | XNVM_EFUSE_CFG_ENABLE_PGM;
	XNvm_Efuse_WriteReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_CFG_REG_OFFSET, Cfg);
}

/******************************************************************************/
/**
 * @brief
 * This function disables programming mode of eFUSE controller.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
static inline void XNvm_EfuseDisableProgramming(void)
{
	u32 Cfg = XNvm_Efuse_ReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_CFG_REG_OFFSET);

	Cfg = Cfg & ~XNVM_EFUSE_CFG_ENABLE_PGM;
	XNvm_Efuse_WriteReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_CFG_REG_OFFSET, Cfg);
}

/******************************************************************************/
/**
 * @brief
 * This function initializes eFUSE controller timers.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
static inline void XNvm_EfuseInitTimers(void)
{
	u32 Tpgm;
	u32 Trd;
	u32 Trdm;
	u32 Tsu_h_ps;
	u32 Tsu_h_ps_cs;
	u32 Tsu_h_cs;

	/* CLK_FREQ = 1/CLK_PERIOD */
	/* TPGM = ceiling(5us/REF_CLK_PERIOD) */
	Tpgm = Xil_Ceil(5.0e-6 * XNVM_PS_REF_CLK_FREQ);
	XNvm_Efuse_WriteReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_TPGM_REG_OFFSET, Tpgm);

	/* TRD = ceiling(217ns/REF_CLK_PERIOD) */
	Trd = Xil_Ceil(217.0e-9 * XNVM_PS_REF_CLK_FREQ);
	XNvm_Efuse_WriteReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_TRD_REG_OFFSET, Trd);

	/* TRDM = ceiling(500ns/REF_CLK_PERIOD)*/
	Trdm = Xil_Ceil(500.0e-9 * XNVM_PS_REF_CLK_FREQ);
	XNvm_Efuse_WriteReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_TRDM_REG_OFFSET, Trdm);

	/* TSU_H_PS = ceiling(208ns/REF_CLK_PERIOD) */
	Tsu_h_ps = Xil_Ceil(208.0e-9 * XNVM_PS_REF_CLK_FREQ);
	XNvm_Efuse_WriteReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_TSU_H_PS_REG_OFFSET,
				Tsu_h_ps);

	/* TSU_H_PS_CS = ceiling(143ns/REF_CLK_PERIOD) */
	Tsu_h_ps_cs = Xil_Ceil(143.0e-9 * XNVM_PS_REF_CLK_FREQ);
	XNvm_Efuse_WriteReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_TSU_H_PS_CS_REG_OFFSET,
				Tsu_h_ps_cs);

	/* TSU_H_CS = ceiling(184ns/REF_CLK_PERIOD) */
	Tsu_h_cs = Xil_Ceil(184.0e-9 * XNVM_PS_REF_CLK_FREQ);
	XNvm_Efuse_WriteReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_TSU_H_CS_REG_OFFSET,
				Tsu_h_cs);
}

/******************************************************************************/
/**
 * @brief
 * This function setups eFUSE controller for given operation and read mode.
 *
 * @param	Op - 	Opeartion to be performed read/program(write).
 *		RdMode - Read mode for eFUSE read operation
 *
 * @return
 *		XST_SUCCESS - eFUSE controller setup for given op
 *		XNVM_EFUSE_ERR_UNLOCK - Failed to unlock eFUSE controller
 *					register access
 *		XST_FAILURE - Unexpected error
 *
 * @note	None.
 *
 ******************************************************************************/
static u32 XNvm_EfuseSetupController(XNvm_EfuseOpMode Op,
			XNvm_EfuseRdMode RdMode)
{
	u32 Status = XST_FAILURE;

	Status = XNvm_EfuseUnlockController();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XNvm_EfuseDisablePowerDown();
	XNvm_EfuseSetRefClk();

	if (XNVM_EFUSE_MODE_PGM == Op) {
		XNvm_EfuseEnableProgramming();
	}

	XNvm_EfuseSetReadMode(RdMode);
	XNvm_EfuseInitTimers();

	/* Enable programming of Xilinx reserved EFUSE */
	XNvm_Efuse_WriteReg(XNVM_EFUSE_CTRL_BASEADDR,
			XNVM_EFUSE_TEST_CTRL_REG_OFFSET, 0x00);


	Status = XST_SUCCESS;

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief
 * This function reads 32-bit data from eFUSE specified Row and Page.
 *
 * @param	Page - Page number
 *		Row - Row number (0-based addressing)
 *		RowData	- Pointer to memory location where 32-bit read data
 *			is to be stored
 *
 * @return
 *		XST_SUCCESS - 32-bit data is read from specified location
 *		XNVM_EFUSE_ERR_RD_TIMEOUT - Timeout occured while reading the
 *						eFUSE
 *		XNVM_EFUSE_ERR_RD - eFUSE Read failed
 *		XST_FAILURE - Unexpected error
 *
 * @note	None.
 *
 ******************************************************************************/
static u32 XNvm_EfuseReadRow(u8 Page, u32 Row, u32* RowData)
{
	u32 Status = XST_FAILURE;
	u32 EventMask = 0U;
	u32 EfuseReadAddr;

	EfuseReadAddr = (Page << XNVM_EFUSE_ADDR_PAGE_SHIFT) |
			(Row << XNVM_EFUSE_ADDR_ROW_SHIFT);

	XNvm_Efuse_WriteReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_RD_ADDR_REG_OFFSET,
				EfuseReadAddr);
	Status = Xil_WaitForEvents((XNVM_EFUSE_CTRL_BASEADDR +
				XNVM_EFUSE_ISR_REG_OFFSET),
				(XNVM_EFUSE_ISR_RD_DONE |
				XNVM_EFUSE_ISR_RD_ERROR),
				(XNVM_EFUSE_ISR_RD_DONE |
				XNVM_EFUSE_ISR_RD_ERROR),
				XNVM_EFUSE_RD_TIMEOUT_VAL,
				&EventMask);
	if(XST_TIMEOUT == Status) {
		Status = XNVM_EFUSE_ERR_RD_TIMEOUT;
	}
	else if ((EventMask & XNVM_EFUSE_ISR_RD_ERROR) != 0x0U) {
		Status = XNVM_EFUSE_ERR_RD;
	}
	else {
		*RowData = XNvm_Efuse_ReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_RD_DATA_REG_OFFSET);
		Status = XST_SUCCESS;
	}
	return Status;
}

/******************************************************************************/
/**
 * @brief
 * This function reads 32-bit data from cache specified by Row
 *
 * @param	Row - Starting Row number (0-based addressing)
 *		RowData	- Pointer to memory location where read 32-bit row data
 *						is to be stored
 *
 * @return
 *		XST_SUCCESS - Specified data read
 *		XNVM_EFUSE_ERR_CACHE_PARITY - Parity error exist in cache
 *		XST_FAILURE - Unexpected error
 *
 * @note	None.
 *
 ******************************************************************************/
static u32 XNvm_EfuseReadCache(u32 Row, u32* RowData)
{
	u32 Status = XST_FAILURE;
	u32 CacheData;
	u32 IsrStatus;

	CacheData = Xil_In32(XNVM_EFUSE_CACHE_BASEADDR + Row * sizeof(u32));
	IsrStatus = XNvm_Efuse_ReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_ISR_REG_OFFSET);
	if (IsrStatus & XNVM_EFUSE_ISR_CACHE_ERROR) {
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
 * @brief
 * This function reads 32-bit rows from eFUSE cache.
 *
 * @param	StartRow - Starting Row number (0-based addressing)
 *		RowCount - Number of Rows to be read
 *		RowData  - Pointer to memory location where read 32-bit row data(s)
 *				is to be stored
 *
 * @return
 *		XST_SUCCESS	- Specified data read
 *		XNVM_EFUSE_ERR_CACHE_PARITY - Parity error exist in cache
 *		XST_FAILURE - Unexpected error
 *
 * @note	None.
 *
 ******************************************************************************/
static u32 XNvm_EfuseReadCacheRange(u32 StartRow, u8 RowCount, u32* RowData)
{
	u32 Status = XST_FAILURE;
	u32 Row = StartRow;

	do {
		Status = XNvm_EfuseReadCache(Row, RowData);
		RowCount--;
		Row++;
		RowData++;
	}
	while ((RowCount > 0U) && (XST_SUCCESS == Status));

	return Status;
}

/******************************************************************************/
/**
 * @brief
 * This function sets the specified bit in the eFUSE.
 *
 * @param	Page - Page number
 *		Row  - Row number (0-based addressing)
 *		Col  - Col number (0-based addressing)
 *
 * @return
 *		XST_SUCCESS	- Specified bit set in eFUSE
 *		XNVM_EFUSE_ERR_PGM_TIMEOUT - eFUSE programming timed out
 *		XNVM_EFUSE_ERR_PGM - eFUSE programming failed
 *		XST_FAILURE - Unexpected error
 *
 * @note	None.
 *
 ******************************************************************************/
static u32 XNvm_EfusePgmBit(u32 Page, u32 Row, u32 Col)
{
	u32 Status = XST_FAILURE;
	u32 PgmAddr;
	u32 EventMask;

	PgmAddr = (Page << XNVM_EFUSE_ADDR_PAGE_SHIFT) |
				(Row << XNVM_EFUSE_ADDR_ROW_SHIFT) |
				(Col << XNVM_EFUSE_ADDR_COLUMN_SHIFT);

	XNvm_Efuse_WriteReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_PGM_ADDR_REG_OFFSET, PgmAddr);
	Status = Xil_WaitForEvents((XNVM_EFUSE_CTRL_BASEADDR +
				XNVM_EFUSE_ISR_REG_OFFSET),
				(XNVM_EFUSE_ISR_PGM_DONE |
				XNVM_EFUSE_ISR_PGM_ERROR),
				(XNVM_EFUSE_ISR_PGM_DONE |
				XNVM_EFUSE_ISR_PGM_ERROR),
				XNVM_EFUSE_PGM_TIMEOUT_VAL,
				&EventMask);
	if (XST_TIMEOUT == Status) {
		Status = XNVM_EFUSE_ERR_PGM_TIMEOUT;
	} else if (EventMask & XNVM_EFUSE_ISR_PGM_ERROR) {
		Status = XNVM_EFUSE_ERR_PGM;
	} else {
		Status = XST_SUCCESS;
	}

	return Status;

}

/******************************************************************************/
/**
 * @brief
 * This function verify the specified bit set in the eFUSE.
 *
 * @param	Page- Page number
 *		Row - Row number (0-based addressing)
 *		Col - Col number (0-based addressing)
 *
 * @return
 *		XST_SUCCESS - Specified bit set in eFUSE
 *		XNVM_EFUSE_ERR_PGM_VERIFY - Verification failed,
 *						specified bit is not set.
 *		XST_FAILURE - Unexpected error
 *
 * @note	None.
 *
 ******************************************************************************/
static u32 XNvm_EfuseVerifyBit(u32 Page, u32 Row, u32 Col)
{
	u32 RdAddr;
	u32 RegData;
	u32 EventMask = 0x00;
	u32 Status = XST_FAILURE;

	RdAddr = (Page << XNVM_EFUSE_ADDR_PAGE_SHIFT) |
				(Row << XNVM_EFUSE_ADDR_ROW_SHIFT);
	XNvm_Efuse_WriteReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_RD_ADDR_REG_OFFSET, RdAddr);
	Status = Xil_WaitForEvents((XNVM_EFUSE_CTRL_BASEADDR +
				XNVM_EFUSE_ISR_REG_OFFSET),
				XNVM_EFUSE_ISR_RD_DONE,
				XNVM_EFUSE_ISR_RD_DONE,
				XNVM_EFUSE_PGM_TIMEOUT_VAL,
				&EventMask);
	if (XST_TIMEOUT == Status) {
		Status = XNVM_EFUSE_ERR_PGM_TIMEOUT;
	} else if (EventMask & XNVM_EFUSE_ISR_RD_DONE) {
		RegData = XNvm_Efuse_ReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_RD_DATA_REG_OFFSET);
		if (RegData & (0x01U << Col)) {
			Status = XST_SUCCESS;
		}
	} else {
		Status = XNVM_EFUSE_ERR_PGM_VERIFY;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief
 * This function sets and then verifies the specified bit in the eFUSE.
 *
 * @param	Page - Page number
 *		Row  - Row number (0-based addressing)
 *		Col  - Col number (0-based addressing)
 *
 * @return
 *		XST_SUCCESS - Specified bit set in eFUSE
 *		XNVM_EFUSE_ERR_PGM_TIMEOUT - eFUSE programming timed out
 *		XNVM_EFUSE_ERR_PGM - eFUSE programming failed
 *		XNVM_EFUSE_ERR_PGM_VERIFY - Verification failed,
 *						specified bit is not set.
 *		XST_FAILURE - Unexpected error
 *
 * @note	None.
 *
 ******************************************************************************/
static u32 XNvm_EfusePgmAndVerifyBit(u32 Page, u32 Row, u32 Col)
{
	u32 Status = XST_FAILURE;
	Status = XNvm_EfusePgmBit(Page, Row, Col);
	if(XST_SUCCESS == Status) {
		Status = XNvm_EfuseVerifyBit(Page, Row, Col);
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief
 * This function program Tbits
 *
 * @param	None.
 *
 * @return
 *		XST_SUCCESS - On Success
 *		XST_FAILURE - Failure in programming
 *
 * @note	None.
 *
 ******************************************************************************/
static u32 XNvm_EfusePgmTBits(void)
{
	u32 Status = XST_FAILURE;
	u32 TbitsPrgrmReg;
	u32 RowDataVal = 0U;
	u32 Column;

	/* Enable TBITS programming bit */
	TbitsPrgrmReg = XNvm_Efuse_ReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_TEST_CTRL_REG_OFFSET);

	XNvm_Efuse_WriteReg(XNVM_EFUSE_CTRL_BASEADDR,
			XNVM_EFUSE_TEST_CTRL_REG_OFFSET,
			(TbitsPrgrmReg & (~XNVM_EFUSE_TBITS_PRGRMG_EN_MASK)));

	Status = XNvm_EfuseReadRow(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_ROW,
				&RowDataVal);
	if (Status != (u32)XST_SUCCESS) {
		 goto END;
	}
	if (((RowDataVal >> XNVM_EFUSE_TBITS_SHIFT) &
			XNVM_EFUSE_TBITS_MASK) != 0x00U) {
		Status = (u32)XNVM_EFUSE_ERR_PGM_TBIT_PATTERN;
		goto END;
	}

	Status = XNvm_EfuseReadRow(XNVM_EFUSE_PAGE_1,
				XNVM_EFUSE_TBITS_ROW,
				&RowDataVal);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}
	if (((RowDataVal >> XNVM_EFUSE_TBITS_SHIFT) &
				XNVM_EFUSE_TBITS_MASK) != 0x00U) {
		Status = (u32)XNVM_EFUSE_ERR_PGM_TBIT_PATTERN;
		goto END;
	}

	Status = XNvm_EfuseReadRow(XNVM_EFUSE_PAGE_2,
				XNVM_EFUSE_TBITS_ROW,
				&RowDataVal);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}
	if (((RowDataVal >> XNVM_EFUSE_TBITS_SHIFT) &
				XNVM_EFUSE_TBITS_MASK) != 0x00U) {
		Status = (u32)XNVM_EFUSE_ERR_PGM_TBIT_PATTERN;
		goto END;
	}

	/* Programming Tbits with pattern 1010 */
	for (Column = XNVM_EFUSE_TBITS_0_COLUMN;
		Column <= XNVM_EFUSE_TBITS_3_COLUMN; Column++) {
		if ((Column == XNVM_EFUSE_TBITS_0_COLUMN) ||
			(Column == XNVM_EFUSE_TBITS_2_COLUMN)) {
			continue;
		}
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_ROW, Column);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_1,
				XNVM_EFUSE_TBITS_ROW, Column);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_2,
				XNVM_EFUSE_TBITS_ROW, Column);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
	}

	XNvm_Efuse_WriteReg(XNVM_EFUSE_CTRL_BASEADDR,
			XNVM_EFUSE_TEST_CTRL_REG_OFFSET,
			TbitsPrgrmReg);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function reloads the cache of eFUSE so that can be directly read from
 * cache.
 *
 * @param	None.
 *
 * @return	XST_SUCCESS on successful cache reload
 *		ErrorCode on failure
 *
 * @note	Not recommended to call this API frequently,
 *		if this API is called all the cache memory is reloaded
 *		by reading eFUSE array, reading eFUSE bit multiple times may
 *		diminish the life time.
 *
 *******************************************************************************/
static u32 XNvm_EfuseCacheLoad(void)
{
	u32 Status = (u32)XST_FAILURE;
	u32 RegStatus;
	volatile u32 CacheStatus;

	RegStatus = XNvm_Efuse_ReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_WR_LOCK_REG_OFFSET);
	/* Check the unlock status */
	if (RegStatus != 0U) {
		XNvm_Efuse_WriteReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_WR_LOCK_REG_OFFSET,
					XNVM_EFUSE_WR_UNLOCK_PASSCODE);
	}

	XNvm_Efuse_WriteReg(XNVM_EFUSE_CTRL_BASEADDR,
			XNVM_EFUSE_CACHE_LOAD_REG_OFFSET,
			XNVM_EFUSE_CACHE_LOAD_MASK);

	CacheStatus = Xil_WaitForEvent((XNVM_EFUSE_CTRL_BASEADDR +
				XNVM_EFUSE_STATUS_REG_OFFSET),
				XNVM_EFUSE_STATUS_CACHE_DONE,
				XNVM_EFUSE_STATUS_CACHE_DONE,
				XNVM_EFUSE_CACHE_LOAD_TIMEOUT_VAL);
	if (CacheStatus != XST_SUCCESS) {
		Status = (u32)XNVM_EFUSE_ERR_CACHE_LOAD;
		goto END;
	}

	CacheStatus = XNvm_Efuse_ReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_ISR_REG_OFFSET);
	if ((CacheStatus & XNVM_EFUSE_ISR_CACHE_ERROR) ==
			XNVM_EFUSE_ISR_CACHE_ERROR) {
		Status = (u32)XNVM_EFUSE_ERR_CACHE_LOAD;
		goto END;
	}
	Status = (u32)XST_SUCCESS;
END:
	return Status;


}

/******************************************************************************/
/**
 * @brief
 * This function checks wheather Tbits are programmed or not
 *
 * @param	None.
 *
 * @return
 *		XST_SUCCESS - On Success
 *		XST_FAILURE - On Failure
 *
 * @note	None.
 *
 ******************************************************************************/
static u32 XNvm_EfuseCheckForTBits(void)
{
	u32 Status = (u32)XST_FAILURE;
	u32 ReadReg;
	u32 TbitMask = (XNVM_EFUSE_STATUS_TBIT_0 |
			XNVM_EFUSE_STATUS_TBIT_1 |
			XNVM_EFUSE_STATUS_TBIT_2 );

	ReadReg = XNvm_Efuse_ReadReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_STATUS_REG_OFFSET);
	if ((ReadReg & TbitMask) != TbitMask)
	{
		Status = XNvm_EfusePgmTBits();
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
		Status = XNvm_EfuseCacheLoad();
		if (Status != (u32)XST_SUCCESS) {
				goto END;
		}
	} else {
		Status = (u32)XST_SUCCESS;
	}
END :
	return Status;
}
