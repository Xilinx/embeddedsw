/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/******************************************************************************/
/**
*
* @file xnvm_efuse.c
*
* This file contains eFuse functions of xilnvm library
* and provides the access to program eFUSE
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
#define XNVM_ONE_MICRO_SECOND			(1U)
#define XNVM_EFUSE_CRC_AES_ZEROS		(0x6858A3D5U)

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

/*****************************************************************************/
/**
 * This function reads the given register.
 *
 * @param	BaseAddress is the Xilinx base address of the eFuse or Bbram
 *		controller.
 * @param	RegOffset is the register offset of the register.
 *
 * @return	The 32-bit value of the register.
 *
 *******************************************************************************/
static INLINE u32 XNvm_EfuseReadReg(u32 BaseAddress, u32 RegOffset)
{
	return Xil_In32(BaseAddress + RegOffset);
}

/*****************************************************************************/
/*
 * This function writes the value into the given register.
 *
 * @param	BaseAddress is the Xilinx base address of the eFuse or Bbram
 *		controller.
 * @param	RegOffset is the register offset of the register.
 * @param	Data is the 32-bit value to write to the register.
 *
 ******************************************************************************/
static INLINE void XNvm_EfuseWriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
{
	Xil_Out32((BaseAddress + RegOffset), Data);
}

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
static u32 XNvm_EfusePgmAndVerifyRows(u32 StartRow, u8 RowCount,
			XNvm_EfuseType EfuseType, const u32* RowData);
static u32 XNvm_EfuseCheckZerosBfrPrgrmg(XNvm_EfuseWriteData *WriteNvm);
static u32 XNvm_EfuseCheckZeros(u32 RowStart, u32 RowEnd);
static u32 XNvm_EfuseWriteChecks(XNvm_EfuseWriteData *WriteNvm);
static u32 XNvm_EfuseReadSecCtrlBitsRegs(XNvm_SecCtrlBits *ReadbackSecCtrlBits);
static u32 XNvm_EfuseWritePufAux(XNvm_PufHelperData *PrgmPufHelperData);
static u32 XNvm_EfuseWritePufChash(XNvm_PufHelperData *PrgmPufHelperData);
static u32 XNvm_EfuseWritePufSynData(XNvm_PufHelperData *PrgmPufHelperData);
static u32 XNvm_EfuseReadPufSecCtrlBitsRegs(
		XNvm_PufSecCtrlBits *ReadBackPufSecCtrlBits);
static u32 XNvm_PrgmRevocationId(u32 RevokeIdEfuseNum, u32 EfuseData);
static u32 XNvm_EfuseCheckForZerosPuf(void);
static u32 XNvm_EfuseReadMiscCtrlBitsRegs(XNvm_MiscCtrlBits *ReadMiscCtrlBits);
static u32 XNvm_EfuseWriteSecCtrl(XNvm_EfuseWriteData *WriteNvm);
static u32 XNvm_PrgmIvRow(u32 EfuseData, u32 IvRow);
static u32 XNvm_PrgmProtectionEfuse(void);
static u32 XNvm_EfuseWritePufSecCtrl(XNvm_PufSecCtrlBits *PrgmPufSecCtrlBits);
/*************************** Variable Definitions *****************************/

/*************************** Function Definitions *****************************/

/***************************************************************************/
/*
 * This function is used to program the AES key, User key 0, User key 1 and
 * PPK0/PPK1/PPK2 hash and DEC_ONLY eFuses based on user inputs.
 *
 * @param	WriteNvm	Pointer to the Xnvm_EfuseWriteData.
 *
 * @return
 *	- XST_SUCCESS - Specified data read
 *	- XNVM_EFUSE_ERR_BEFORE_PROGRAMMING - Error before programming eFuse
 *	- XNVM_EFUSE_ERR_WRITE_AES_KEY - Error while writing AES key
 *	- XNVM_EFUSE_ERR_WRITE_USER0_KEY - Error while writing User key 0
 * 	- XNVM_EFUSE_ERR_WRITE_USER1_KEY - Error while writing User key 1
 *	- XNVM_EFUSE_ERR_WRITE_PPK0_HASH - Error while writing PPK0 Hash
 *	- XNVM_EFUSE_ERR_WRITE_PPK1_HASH - Error while writing PPK1 Hash
 * 	- XNVM_EFUSE_ERR_WRITE_PPK2_HASH - Error while writing PPK2 Hash
 *	- XNVM_EFUSE_ERR_WRITE_DEC_EFUSE_ONLY - Error while writing
 *						DEC_ONLY eFuse.
 * 	- XST_FAILURE - Unexpected error
 *
 * @note
 *	After eFUSE programming is complete, the cache is automatically
 *	reloaded so all programmed eFUSE bits can be directly read from
 *	cache.
 *
 *****************************************************************************/
u32 XNvm_EfuseWrite(XNvm_EfuseWriteData *WriteNvm)
{
	u32 Status = (u32)XST_FAILURE;
	u32 LockStatus = (u32)XST_FAILURE;
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

	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
					XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (WriteNvm->CheckWriteFlags.AesKey == TRUE) {
		Status = XNvm_EfusePgmAndVerifyRows(
				XNVM_EFUSE_AES_KEY_START_ROW,
				XNVM_EFUSE_AES_KEY_NUM_OF_ROWS,
				XNVM_EFUSE_PAGE_0, WriteNvm->AesKey);
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
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_AES_KEY);
			goto END;
		}
	}
	if (WriteNvm->CheckWriteFlags.UserKey0 == TRUE) {
		Status = XNvm_EfusePgmAndVerifyRows(
				XNVM_EFUSE_USER_KEY_0_START_ROW,
				XNVM_EFUSE_USER_KEY_NUM_OF_ROWS,
				XNVM_EFUSE_PAGE_0, WriteNvm->UserKey0);
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

		Status = XNvm_EfuseCheckAesUserKey0Crc(Crc);
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_USER0_KEY);
			goto END;
		}
	}
	if (WriteNvm->CheckWriteFlags.UserKey1 == TRUE) {
		Status = XNvm_EfusePgmAndVerifyRows(
					XNVM_EFUSE_USER_KEY_1_START_ROW,
					XNVM_EFUSE_USER_KEY_NUM_OF_ROWS,
					XNVM_EFUSE_PAGE_0,
					WriteNvm->UserKey1);
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

		Status = XNvm_EfuseCheckAesUserKey1Crc(Crc);
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_USER1_KEY);
			goto END;
		}
	}
	if (WriteNvm->CheckWriteFlags.Ppk0Hash == TRUE) {
		Status = XNvm_EfusePgmAndVerifyRows(
				XNVM_EFUSE_PPK_0_HASH_START_ROW,
				XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS,
				XNVM_EFUSE_PAGE_0,
				WriteNvm->Ppk0Hash);
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_PPK0_HASH);
			goto END;
		}
	}
	if (WriteNvm->CheckWriteFlags.Ppk1Hash == TRUE) {
		Status = XNvm_EfusePgmAndVerifyRows(
				XNVM_EFUSE_PPK_1_HASH_START_ROW,
				XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS,
				XNVM_EFUSE_PAGE_0,
				WriteNvm->Ppk1Hash);
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_PPK1_HASH);
			goto END;
		}
	}
	if (WriteNvm->CheckWriteFlags.Ppk2Hash == TRUE) {
		Status = XNvm_EfusePgmAndVerifyRows(
				XNVM_EFUSE_PPK_2_HASH_START_ROW,
				XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS,
				XNVM_EFUSE_PAGE_0,
				WriteNvm->Ppk2Hash);
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_PPK2_HASH);
			goto END;
		}
	}
	if (WriteNvm->CheckWriteFlags.DecOnly == TRUE) {
		Status = XNvm_EfusePgmAndVerifyRows(
				XNVM_EFUSE_DEC_EFUSE_ONLY_ROW,
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
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_PrgmProtectionEfuse();
END:
	XNvm_EfuseDisableProgramming();
	LockStatus = XNvm_EfuseLockController();
	if (XST_SUCCESS == Status) {
		Status = LockStatus;
	}
	return Status;
}

/*****************************************************************************/
/**
 *
 * This function programs secure control bits specified by user.
 *
 * @param   WriteNvm  Instance of Versal Ps eFuse
 *
 * @return
 *	- XST_SUCCESS - Specified bit set in eFUSE
 *	- XNVM_EFUSE_ERROR_INVALID_PARAM - Invalid Parameter.
 *	- XNVM_EFUSE_ERR_WRTIE_PPK0_WR_LK - Error while writing PPK0_WR_LK
 *	- XNVM_EFUSE_ERR_WRTIE_PPK1_WR_LK - Error while writing PPK1_WR_LK
 * 	- XNVM_EFUSE_ERR_WRTIE_PPK2_WR_LK - Error while writing PPK2_WR_LK
 *	- XNVM_EFUSE_ERR_WRTIE_AES_CRC_LK_BIT_1 - Error while writing
 *							AES_CRC_LK_BIT_1
 * 	- XNVM_EFUSE_ERR_WRTIE_AES_CRC_LK_BIT_2	- Error while writing
 *							AES_CRC_LK_BIT_2
 *	- XNVM_EFUSE_ERR_WRTIE_AES_WR_LK - Error while writing AES_WR_LK
 *	- XNVM_EFUSE_ERR_WRTIE_USER_KEY0_WR_LK - Error while writing
 *						USER_KEY0_WR_LK
 *	- XNVM_EFUSE_ERR_WRTIE_USER_KEY1_WR_LK - Error while writing
 *						USER_KEY1_WR_LK
 *	- XNVM_EFUSE_ERR_WRTIE_USER_KEY0_CRC_LK - Error while writing
 *						USER_KEY0_CRC_LK
 *	- XNVM_EFUSE_ERR_WRTIE_USER_KEY1_CRC_LK - Error while writing
 *						USER_KEY1_CRC_LK
 *	- XNVM_EFUSE_ERR_WRTIE_SECDBG_DIS_BIT_1 - Error while writing
 *						SECDBG_DIS_BIT_1
 *	- XNVM_EFUSE_ERR_WRTIE_SECDBG_DIS_BIT_2 - Error while writing
 *						SECDBG_DIS_BIT_2
 *	- XNVM_EFUSE_ERR_WRTIE_BOOTENV_WR_LK - Error while writing
 *						BOOTENV_WR_LK
 *	- XNVM_EFUSE_ERR_WRTIE_REG_INIT_DIS_BIT_1 - Error while writing
 * 						REG_INIT_DIS_BIT_1
 *	- XNVM_EFUSE_ERR_WRTIE_REG_INIT_DIS_BIT_2 - Error while writing
 *						REG_INIT_DIS_BIT_2
 ******************************************************************************/
static u32 XNvm_EfuseWriteSecCtrl(XNvm_EfuseWriteData *WriteNvm)
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

	if ((WriteNvm->PrgmSecCtrlFlags.Ppk0WrLk != FALSE) ||
		(WriteNvm->PrgmSecCtrlFlags.Ppk1WrLk != FALSE) ||
		(WriteNvm->PrgmSecCtrlFlags.Ppk2WrLk != FALSE) ||
		(WriteNvm->PrgmSecCtrlFlags.AesCrcLk != FALSE) ||
		(WriteNvm->PrgmSecCtrlFlags.AesWrLk != FALSE) ||
		(WriteNvm->PrgmSecCtrlFlags.UserKey0CrcLk != FALSE) ||
		(WriteNvm->PrgmSecCtrlFlags.UserKey0WrLk != FALSE) ||
		(WriteNvm->PrgmSecCtrlFlags.UserKey1CrcLk != FALSE) ||
		(WriteNvm->PrgmSecCtrlFlags.UserKey1WrLk != FALSE) ||
		(WriteNvm->PrgmSecCtrlFlags.SecDbgDis != FALSE) ||
		(WriteNvm->PrgmSecCtrlFlags.SecLockDbgDis != FALSE) ||
		(WriteNvm->PrgmSecCtrlFlags.BootEnvWrLk != FALSE) ||
		(WriteNvm->PrgmSecCtrlFlags.RegInitDis != FALSE)) {

		Status = XNvm_EfuseReadCache(Row, &RowDataVal);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}

		XNvm_ConvertHexToByteArray((u8 *) &RowDataVal,
			DataInBits, XNVM_EFUSE_MAX_BITS_IN_ROW);
	}

	Status = XNvm_EfuseCheckForTBits();
	if (Status != XST_SUCCESS) {
		goto END;
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
/**
 * This function performs the CRC check of AES key
 *
 * @param	CrcValue	A 32 bit CRC value of an expected AES key.
 *
 * @return
 *		- XST_SUCCESS - On successful CRC check.
 *		- XNVM_EFUSE_ERR_CRC_VERIFICATION - On failure.
 *
 * @note	For Calculating the CRC of the AES key use the
 *		XNvm_AesCrcCalc() function
 *
 ******************************************************************************/
u32 XNvm_EfuseCheckAesKeyCrc(u32 Crc)
{
	u32 Status = (u32)XST_FAILURE;
	u32 ReadReg = 0U;

	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
		XNVM_EFUSE_AES_CRC_REG_OFFSET, Crc);

	Status = Xil_WaitForEvent((XNVM_EFUSE_CTRL_BASEADDR +
				XNVM_EFUSE_STATUS_REG_OFFSET),
				XNVM_EFUSE_CTRL_STATUS_AES_CRC_DONE_MASK,
				XNVM_EFUSE_CTRL_STATUS_AES_CRC_DONE_MASK,
				XNVM_POLL_TIMEOUT);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	ReadReg = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_STATUS_REG_OFFSET);

	if ((ReadReg & XNVM_EFUSE_CTRL_STATUS_AES_CRC_DONE_MASK) ==
								0x00U) {
		Status = (u32)XST_FAILURE;
	}
	else if ((ReadReg &
			XNVM_EFUSE_CTRL_STATUS_AES_CRC_PASS_MASK) ==
			0x00U) {

		Status = (u32)XNVM_EFUSE_ERR_CRC_VERIFICATION;
	}
	else {
		Status = (u32)XST_SUCCESS;
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * This function performs the CRC check of User key 0
 *
 * @param	CrcValue	A 32 bit CRC value of an expected User key 0.
 *
 * @return
 *		- XST_SUCCESS - On successful CRC check.
 *		- XNVM_EFUSE_ERR_CRC_VERIFICATION - On failure.
 *
 * @note	For Calculating the CRC of the User key 0 use the
 *			XNvm_AesCrcCalc() function
 *
 ******************************************************************************/
u32 XNvm_EfuseCheckAesUserKey0Crc(u32 Crc)
{
	u32 Status = (u32)XST_FAILURE;
	u32 ReadReg = 0U;

	/* writing CRC value to check AES key's CRC */
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
		XNVM_EFUSE_AES_USR_KEY0_CRC_REG_OFFSET, Crc);

	Status = Xil_WaitForEvent((XNVM_EFUSE_CTRL_BASEADDR +
			XNVM_EFUSE_STATUS_REG_OFFSET),
			XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_0_CRC_DONE_MASK,
			XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_0_CRC_DONE_MASK,
			XNVM_POLL_TIMEOUT);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	ReadReg = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_STATUS_REG_OFFSET);
	if ((ReadReg &
		XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_0_CRC_DONE_MASK) ==
								0x00U) {
		Status = (u32)XST_FAILURE;
	}
	else if ((ReadReg &
		XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_0_CRC_PASS_MASK) ==
								0x00U) {
		Status = (u32)XNVM_EFUSE_ERR_CRC_VERIFICATION;
	}
	else {
		Status = (u32)XST_SUCCESS;
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * This function performs the CRC check of User key 1
 *
 * @param	CrcValue	A 32 bit CRC value of an expected User key 1.
 *
 * @return
 *		- XST_SUCCESS - On successful CRC check.
 *		- XNVM_EFUSE_ERR_CRC_VERIFICATION - On failure
 *
 * @note	For Calculating the CRC of the User key 1 use the
 *		XNvm_AesCrcCalc() function
 *
 ******************************************************************************/
u32 XNvm_EfuseCheckAesUserKey1Crc(u32 Crc)
{
	u32 Status = (u32)XST_FAILURE;
	u32 ReadReg = 0U;

	/* writing CRC value to check AES key's CRC */
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
		XNVM_EFUSE_AES_USR_KEY1_CRC_REG_OFFSET, Crc);

	Status = Xil_WaitForEvent((XNVM_EFUSE_CTRL_BASEADDR +
			XNVM_EFUSE_STATUS_REG_OFFSET),
			XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_1_CRC_DONE_MASK,
			XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_1_CRC_DONE_MASK,
			XNVM_POLL_TIMEOUT);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	ReadReg = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_STATUS_REG_OFFSET);

	if ((ReadReg & XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_1_CRC_DONE_MASK) ==
								0x00U) {
		Status = (u32)XST_FAILURE;
	}
	else if ((ReadReg &
		XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_1_CRC_PASS_MASK) ==
								0x00U) {
		Status = (u32)XNVM_EFUSE_ERR_CRC_VERIFICATION;
	}
	else {
		Status = (u32)XST_SUCCESS;
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to read the PS eFUSE secure control bits from cache.
 *
 * @param	ReadBackSecCtrlBits	Pointer to the Xnvm_SecCtrlBits
 * 					which holds the read secure control bits.
 * @return
 * 		- XST_SUCCESS - On Successfull read.
 * 		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter
 *
 * ******************************************************************************/
u32 XNvm_EfuseReadSecCtrlBits(XNvm_SecCtrlBits *ReadbackSecCtrlBits)
{
	u32 Status = (u32)XST_FAILURE;

	if (ReadbackSecCtrlBits == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
	}
	else {
		Status = XNvm_EfuseReadSecCtrlBitsRegs(ReadbackSecCtrlBits);
	}

	return Status;
}

/*****************************************************************************/
/**
 * This function programs the eFUSEs with the PUF helper data.
 *
 * @param	PrgmPufHelperData	Pointer to the Xnvm_PufHelperData.
 *
 * @return
 *		- XST_SUCCESS - if programs successfully.
 *		- XNVM_EFUSE_ERR_RD_PUF_SEC_CTRL - Error while reading PufSecCtrl
 * 		- XNVM_EFUSE_ERR_WRITE_PUF_HELPER_DATA - Error while writing Puf
 *							helper data.
 *		- XNVM_EFUSE_ERR_WRITE_PUF_SYN_DATA - Error while writing Puf Syn
 *							data.
 *		- XNVM_EFUSE_ERR_WRITE_PUF_CHASH - Error while writing Puf Chash.
 * 		- XNVM_EFUSE_ERR_WRITE_PUF_AUX - Error while writing Puf Aux.
 *
 * @note	To generate PufSyndromeData please use
 *		XPuf_Registration API
 *
 *******************************************************************************/
u32 XNvm_EfuseWritePuf(XNvm_PufHelperData *PrgmPufHelperData)
{
	u32 Status = (u32)XST_FAILURE;
	u32 LockStatus = (u32)XST_FAILURE;

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

	/* Programming Puf SecCtrl bits */
	Status = XNvm_EfuseWritePufSecCtrl(
			&(PrgmPufHelperData->PrgmPufSecCtrlBits));
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseCacheLoad();
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_PrgmProtectionEfuse();
END :
	XNvm_EfuseDisableProgramming();
	LockStatus = XNvm_EfuseLockController();
	if (XST_SUCCESS == Status) {
		Status = LockStatus;
	}
	return Status;

}

/*****************************************************************************/
/**
 * This function programs Puf control bits specified by user.
 *
 * @param	PrgmPufSecCtrlBits	Instance of XNvm_PufSecCtrlBits.
 *
 * @return
 *		- XST_SUCCESS - On success
 *		- XNVM_EFUSE_ERR_INVALID_PARAM - Invalid parameter.
 *		- XNVM_EFUSE_ERR_WRITE_PUF_REGEN_DIS - Error while writing
 *							PUF_REGEN_DIS.
 *		- XNVM_EFUSE_ERR_WRITE_PUF_HD_INVLD - Error while writing
 *							PUF_HD_INVLD
 *		- XNVM_EFUSE_ERR_WRITE_PUF_SYN_LK - Error while writing
 * 							PUF_SYN_LK.
 *		- XNVM_EFUSE_ERR_WRITE_PUF_TEST2_DIS - Error while writing
 *							PUF_TEST2_DIS
 *		- XNVM_EFUSE_ERR_WRITE_PUF_DIS - Error while writing PUF_DIS
 *
 ******************************************************************************/
static u32 XNvm_EfuseWritePufSecCtrl(XNvm_PufSecCtrlBits *PrgmPufSecCtrlBits)
{
	u32 Status = (u32)XST_FAILURE;
	u32 LockStatus = (u32)XST_FAILURE;
	XNvm_EfuseType EfuseType = XNVM_EFUSE_PAGE_0;
	u32 Row = XNVM_EFUSE_PUF_AUX_ROW;
	u32 RowDataVal;
	u8 DataInBits[XNVM_EFUSE_MAX_BITS_IN_ROW] = {0};


	if (PrgmPufSecCtrlBits == NULL) {
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

	if ((PrgmPufSecCtrlBits->PufRegenDis != FALSE) ||
		(PrgmPufSecCtrlBits->PufHdInvalid != FALSE)) {

		Status = XNvm_EfuseReadCache(Row, &RowDataVal);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}

		XNvm_ConvertHexToByteArray((u8 *) &RowDataVal,
			DataInBits, XNVM_EFUSE_MAX_BITS_IN_ROW);
	}

	if ((PrgmPufSecCtrlBits->PufRegenDis == TRUE) &&
		(DataInBits[XNVM_EFUSE_PUF_ECC_PUF_CTRL_REGEN_DIS_COLUMN] ==
		0x00U)) {
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
			XNVM_EFUSE_PUF_ECC_PUF_CTRL_REGEN_DIS_COLUMN);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRITE_PUF_REGEN_DIS);
			goto END;
		}
	}
	if ((PrgmPufSecCtrlBits->PufHdInvalid == TRUE) &&
		(DataInBits[XNVM_EFUSE_PUF_ECC_PUF_CTRL_HD_INVLD_COLUMN] ==
		0x00U)) {
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
			XNVM_EFUSE_PUF_ECC_PUF_CTRL_HD_INVLD_COLUMN);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRITE_PUF_HD_INVLD);
			goto END;
		}
	}

	Row = XNVM_EFUSE_SECURITY_CONTROL_ROW;

	if ((PrgmPufSecCtrlBits->PufTest2Dis != FALSE) ||
		(PrgmPufSecCtrlBits->PufDis != FALSE) ||
		(PrgmPufSecCtrlBits->PufSynLk != FALSE)) {

		Status = XNvm_EfuseReadCache(Row, &RowDataVal);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}

		XNvm_ConvertHexToByteArray((u8 *) &RowDataVal,
			DataInBits, XNVM_EFUSE_MAX_BITS_IN_ROW);
	}

	if ((PrgmPufSecCtrlBits->PufSynLk != 0x00U) &&
		(DataInBits[XNVM_EFUSE_SEC_PUF_SYN_LK] == 0x00U)) {
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
					XNVM_EFUSE_SEC_PUF_SYN_LK);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRITE_PUF_SYN_LK);
			goto END;
		}
	}

	if ((PrgmPufSecCtrlBits->PufTest2Dis != 0x00U) &&
		(DataInBits[XNVM_EFUSE_SEC_PUF_TEST2_DIS] == 0x00U)) {
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
				XNVM_EFUSE_SEC_PUF_TEST2_DIS);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRITE_PUF_TEST2_DIS);
			goto END;
		}
	}

	if ((PrgmPufSecCtrlBits->PufDis != 0x00U) &&
		(DataInBits[XNVM_EFUSE_SEC_PUF_DIS] == 0x00U)) {
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
				XNVM_EFUSE_SEC_PUF_DIS);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRITE_PUF_DIS);
			goto END;
		}
	}
	Status = XST_SUCCESS;
END:
	XNvm_EfuseDisableProgramming();
	LockStatus = XNvm_EfuseLockController();
	if (XST_SUCCESS == Status) {
		Status = LockStatus;
	}
	return Status;
}
/*****************************************************************************/
/**
 * This function is used to read the Puf eFUSE secure control bits from cache.
 *
 * @param	ReadBackSecCtrlBits	Pointer to the Xnvm_PufSecCtrlBits
 *					which holds the read secure control bits.
 *
 * @return
 * 		- XST_SUCCESS - On Successfull read
 *		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter
 *
 *******************************************************************************/
u32 XNvm_EfuseReadPufSecCtrlBits(XNvm_PufSecCtrlBits *ReadBackPufSecCtrlBits)
{
	u32 Status = (u32)XST_FAILURE;

	if (ReadBackPufSecCtrlBits == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
	}
	else {
		Status = XNvm_EfuseReadPufSecCtrlBitsRegs(ReadBackPufSecCtrlBits);
	}

	return Status;
}

/*****************************************************************************/
/**
 * This function is used to read the Misc eFUSE control bits from cache.
 *
 * @param	ReadMiscCtrlBits	Pointer to the Xnvm_MiscCtrlBits
 *					which holds the read secure control bits.
 *
 * @return
 * 		- XST_SUCCESS - On Successfull read.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *
 *******************************************************************************/
u32 XNvm_EfuseReadMiscCtrlBits(XNvm_MiscCtrlBits *ReadMiscCtrlBits)
{
	u32 Status = (u32)XST_FAILURE;

	if (ReadMiscCtrlBits == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
	}
	else {
		Status = XNvm_EfuseReadMiscCtrlBitsRegs(ReadMiscCtrlBits);
	}

	return Status;
}

/*****************************************************************************/
/**
 * This function programs the eFUSEs with the IV.
 *
 * @param	EfuseIv		Pointer to the Xnvm_Iv.
 *
 * @return
 *	- XST_SUCCESS - On Successfull Write.
 *	- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 * 	- XNVM_EFUSE_ERR_WRITE_META_HEADER_IV - Error while writing Meta Iv
 *	- XNVM_EFUSE_ERR_WRITE_BLK_OBFUS_IV - Error while writing BlkObfus IV
 *	- XNVM_EFUSE_ERR_WRITE_PLML_IV - Error while writing PLML IV
 * 	- XNVM_EFUSE_ERR_WRITE_DATA_PARTITION_IV - Error while writing Data
 *						Partition IV.
 *******************************************************************************/
u32 XNvm_EfuseWriteIVs(XNvm_Iv *EfuseIv)
{
	u32 Status = (u32)XST_FAILURE;
	u32 LockStatus = (u32)XST_FAILURE;
	u32 Row;

	if (EfuseIv == NULL) {
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
			Status = XNvm_PrgmIvRow(EfuseIv->MetaHeaderIv[Row],
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
			Status = XNvm_PrgmIvRow(EfuseIv->BlkObfusIv[Row],
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
			Status = XNvm_PrgmIvRow(EfuseIv->PlmlIv[Row],
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
			Status = XNvm_PrgmIvRow(EfuseIv->DataPartitionIv[Row],
				(XNVM_EFUSE_DATA_PARTITION_IV_START_ROW + Row));
			if (Status != XST_SUCCESS) {
				Status = (Status |
					(u32)XNVM_EFUSE_ERR_WRITE_DATA_PARTITION_IV);
				goto END;
			}
		}

	}
	Status = XNvm_EfuseCacheLoad();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_PrgmProtectionEfuse();
END :
	XNvm_EfuseDisableProgramming();
	LockStatus = XNvm_EfuseLockController();
	if (XST_SUCCESS == Status) {
		Status = LockStatus;
	}
	return Status;
}

/*****************************************************************************/
/**
 * This function programs the eFUSE row with the IV.
 *
 * @param	EfuseIv		Pointer to the Xnvm_Iv.
 *
 * @param	IvRow		IV Row to be programmed.
 *
 * @return
 *		- XST_SUCCESS - On Successfull Write.
 *		- XNVM_EFUSE_ERR_BIT_CANT_REVERT - Bit cant be reverted Error
 *
 ********************************************************************************/
static u32 XNvm_PrgmIvRow(u32 EfuseData, u32 IvRow)
{
	u32 Status = XST_FAILURE;
	u8 IvRowInBits[XNVM_EFUSE_MAX_BITS_IN_ROW] = {0};
	u8 IvRowInBitsRd[XNVM_EFUSE_MAX_BITS_IN_ROW] = {0};
	u32 ReadIv;
	u32 IvRowInBytes;
	u32 Column;

	XNvm_ConvertHexToByteArray((u8 *)&EfuseData,
			IvRowInBits, XNVM_EFUSE_MAX_BITS_IN_ROW);
	Status = XNvm_EfuseReadCache(IvRow, &ReadIv);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	XNvm_ConvertHexToByteArray((u8 *)&ReadIv, IvRowInBitsRd,
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
	XNvm_ConvertByteArrayToHex(IvRowInBits, (u8 *)&IvRowInBytes,
			XNVM_EFUSE_MAX_BITS_IN_ROW);

	Status = XNvm_EfusePgmAndVerifyRows(IvRow, 0x1U,
			XNVM_EFUSE_PAGE_0, &IvRowInBytes);
END:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to read IV eFUSE bits from cache
 *
 * @param	EfuseIv		Pointer to the Xnvm_Iv
 *
 * @return
 * 	- XST_SUCCESS - On Successfull read.
 *  	- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *	- XNVM_EFUSE_ERR_RD_META_HEADER_IV - Error while reading Meta IV
 *	- XNVM_EFUSE_ERR_RD_BLACK_OBFUS_IV - Error while reading BlkObfus IV
 *	- XNVM_EFUSE_ERR_RD_PLML_IV - Error while reading PLML IV
 *	- XNVM_EFUSE_ERR_RD_DATA_PARTITION_IV - Error while reading Data
 *						Partition IV
 *
 *******************************************************************************/
u32 XNvm_EfuseReadIVs(XNvm_Iv *EfuseIv)
{
	u32 Status = (u32)XST_FAILURE;

	if (EfuseIv == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfuseReadCacheRange(XNVM_EFUSE_META_HEADER_IV_START_ROW,
					XNVM_EFUSE_IV_NUM_OF_ROWS,
					EfuseIv->MetaHeaderIv);
	if (Status != XST_SUCCESS) {
		Status = (Status | (u32)XNVM_EFUSE_ERR_RD_META_HEADER_IV);
		goto END;
	}
	Status = XNvm_EfuseReadCacheRange(XNVM_EFUSE_BLACK_OBFUS_IV_START_ROW,
					XNVM_EFUSE_IV_NUM_OF_ROWS,
					EfuseIv->BlkObfusIv);
	if (Status != XST_SUCCESS) {
		Status = (Status | (u32)XNVM_EFUSE_ERR_RD_BLACK_OBFUS_IV);
		goto END;
	}
	Status = XNvm_EfuseReadCacheRange(XNVM_EFUSE_PLML_IV_START_ROW,
					XNVM_EFUSE_IV_NUM_OF_ROWS,
					EfuseIv->PlmlIv);
	if (Status != XST_SUCCESS) {
		Status = (Status | (u32)XNVM_EFUSE_ERR_RD_PLML_IV);
		goto END;
	}
	Status = XNvm_EfuseReadCacheRange(
			XNVM_EFUSE_DATA_PARTITION_IV_START_ROW,
			XNVM_EFUSE_IV_NUM_OF_ROWS,
			EfuseIv->DataPartitionIv);
	if (Status != XST_SUCCESS) {
		Status = (Status | (u32)XNVM_EFUSE_ERR_RD_DATA_PARTITION_IV);
		goto END;
	}
	Status = (u32)XST_SUCCESS;
END:
	return Status;
}

/******************************************************************************/
/**
 * This function reads the PUF helper data from eFUSE cache.
 *
 * @param	RdPufHelperData		Pointer to Xnvm_PufHelperData which hold
 * 					the PUF helper data read from eFUSEs.
 *
 * @return
 *	- XST_SUCCESS -  On successfull read.
 *	- XNVM_EFUSE_ERR_RD_PUF_SYN_DATA - Error while reading SYN_DATA.
 *	- XNVM_EFUSE_ERR_RD_PUF_CHASH - Error while reading CHASH.
 *	- XNVM_EFUSE_ERR_RD_PUF_AUX - Error while reading AUX.
 *
 ******************************************************************************/
u32 XNvm_EfuseReadPufHelperData(XNvm_PufHelperData *RdPufHelperData)
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
/**
 * This function is used to read Dna eFUSE bits from cache
 *
 * @param	EfuseDna	Pointer to the Xnvm_Dna
 *
 * @return
 * 		- XST_SUCCESS - On Successfull read.

 *		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid parameter.
 *
 *******************************************************************************/
u32 XNvm_EfuseReadDna(XNvm_Dna *EfuseDna)
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
/**
 * This function is used to read DecEfuseOnly eFUSE bits from cache
 *
 * @param	DecOnly		Pointer to the DecOnly efuse data
 *
 * @return
 * 		- XST_SUCCESS - On Successfull read

 *		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
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
/**
 * This function reads the Ppk Hash from eFUSE cache.
 *
 * @param	EfusePpk	Pointer to Xnvm_PpkHash which hold
 * 				the PpkHash from eFUSE Cache.
 *
 * @param 	PpkType		Type of the Ppk to be programmed
 *
 * @return
 *		- XST_SUCCESS - On Successfull read.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 * 		- XNVM_EFUSE_ERR_RD_PPK_HASH - Error while reading PPK Hash.
 *******************************************************************************/

u32 XNvm_EfuseReadPpkHash(XNvm_PpkHash *EfusePpk, XNvm_PpkType PpkType)
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
/**
 * This function revokes the Ppk.
 *
 * @param	PpkRevoke	Pointer to Xnvm_RevokePpkFlags that contains
 * 				which Ppk to revoke.
 *
 * @return
 *		- XST_SUCCESS - On Successfull Revocation.
 *		- Errorcode - On failure.
 *******************************************************************************/
u32 XNvm_EfuseRevokePpk(XNvm_RevokePpkFlags *PpkRevoke)
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

		XNvm_ConvertHexToByteArray((u8 *) &RowData,
			DataInBits, XNVM_EFUSE_MAX_BITS_IN_ROW);
	}

	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
			XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		goto END;
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
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_PrgmProtectionEfuse();
END:
	XNvm_EfuseDisableProgramming();
	LockStatus = XNvm_EfuseLockController();
	if (XST_SUCCESS == Status) {
		Status = LockStatus;
	}
	return Status;
}

/*****************************************************************************/
/**
 * This function writes Revocation eFuses.
 *
 * @param	WriteRevokeId	Pointer to Xnvm_RevokeSpk that contains
 * 				which Revocation Id to write.
 *
 * @return
 *		- XST_SUCCESS - On successfull read.
 *		- Errorcode - On failure.
 *******************************************************************************/
u32 XNvm_EfuseWriteRevocationId(XNvm_RevokeIdEfuse *WriteRevokeId)
{
	u32 Status = (u32)XST_FAILURE;

	if (WriteRevokeId == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if ((WriteRevokeId->PrgmRevokeId0 != 0x00U) ||
		(WriteRevokeId->PrgmRevokeId1 != 0x00U) ||
		(WriteRevokeId->PrgmRevokeId2 != 0x00U) ||
		(WriteRevokeId->PrgmRevokeId3 != 0x00U) ||
		(WriteRevokeId->PrgmRevokeId4 != 0x00U) ||
		(WriteRevokeId->PrgmRevokeId5 != 0x00U) ||
		(WriteRevokeId->PrgmRevokeId6 != 0x00U) ||
		(WriteRevokeId->PrgmRevokeId7 != 0x00U)) {

		if (XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
			XNVM_EFUSE_PGM_LOCK_REG_OFFSET) != 0x00U) {
			Status = ((u32)XNVM_EFUSE_ERR_FUSE_PROTECTED |
				(u32)XNVM_EFUSE_ERR_WRITE_REVOCATION_IDS);
			goto END;
		}
	}

	if (WriteRevokeId->PrgmRevokeId0 != 0x00U) {

		Status = XNvm_PrgmRevocationId(XNVM_EFUSE_REVOCATION_ID_0,
						WriteRevokeId->RevokeId0);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_0);
			goto END;
		}
	}
	if (WriteRevokeId->PrgmRevokeId1 != 0x00U) {

		Status = XNvm_PrgmRevocationId(XNVM_EFUSE_REVOCATION_ID_1,
						WriteRevokeId->RevokeId1);
		if (Status != XST_SUCCESS) {
			Status = (Status |
			(u32)XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_1);
			goto END;
		}
	}
	if (WriteRevokeId->PrgmRevokeId2 != 0x00U) {

		Status = XNvm_PrgmRevocationId(XNVM_EFUSE_REVOCATION_ID_2,
						WriteRevokeId->RevokeId2);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_2);
			goto END;
		}

	}
	if (WriteRevokeId->PrgmRevokeId3 != 0x00U) {

		Status = XNvm_PrgmRevocationId(XNVM_EFUSE_REVOCATION_ID_3,
						WriteRevokeId->RevokeId3);
		if (Status != XST_SUCCESS) {
			Status = (Status |
			(u32)XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_3);
			goto END;
		}
	}
	if (WriteRevokeId->PrgmRevokeId4 != 0x00U) {

		Status = XNvm_PrgmRevocationId(XNVM_EFUSE_REVOCATION_ID_4,
						WriteRevokeId->RevokeId4);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_4);
			goto END;
		}
	}
	if (WriteRevokeId->PrgmRevokeId5 != 0x00U) {

		Status = XNvm_PrgmRevocationId(XNVM_EFUSE_REVOCATION_ID_5,
						WriteRevokeId->RevokeId5);
		if (Status != XST_SUCCESS) {
		Status = (Status |
			(u32)XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_5);
			goto END;
		}
	}
	if (WriteRevokeId->PrgmRevokeId6 != 0x00U) {

		Status = XNvm_PrgmRevocationId(XNVM_EFUSE_REVOCATION_ID_6,
						WriteRevokeId->RevokeId6);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_6);
			goto END;
		}
	}
	if (WriteRevokeId->PrgmRevokeId7 != 0x00U) {

		Status = XNvm_PrgmRevocationId(XNVM_EFUSE_REVOCATION_ID_7,
						WriteRevokeId->RevokeId7);
		if (Status != XST_SUCCESS) {
			Status = (Status |
			(u32)XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_7);
			goto END;
		}
	}
	Status = XNvm_EfuseCacheLoad();
END:
	return Status;
}

/*****************************************************************************/
/**
 * This function reads the Revocation Fuse from eFUSE cache.
 *
 * @param	RevokeFusePtr	Pointer to Xnvm_PpkHash which hold
 * 				the PpkHash from eFUSE Cache.
 *
 * @param 	RevokeFuse_Num	Type of the Ppk to be programmed
 *
 * @return
 *		- XST_SUCCESS  - On uccessfull read.
 *		- Errorcode - On failure.
 *******************************************************************************/
u32 XNvm_EfuseReadRevocationId(u32 *RevokeFusePtr, u8 RevokeFuseNum)
{
	u32 Status = (u32)XST_FAILURE;
	u32 Row;

	if (RevokeFusePtr == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	if (RevokeFuseNum > XNVM_EFUSE_REVOCATION_ID_7) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
	}
	else {
		Row = XNVM_EFUSE_REVOCATION_ID_0_ROW + RevokeFuseNum;
		Status = XNvm_EfuseReadCache(Row, RevokeFusePtr);
	}
END :
	return Status;
}

/*****************************************************************************/
/**
 * This function is used verify eFUSE keys for Zeros before programming.
 *
 * @param	WriteNvm	Pointer to eFUSE ps instance.
 *
 * @return
 *		- XST_SUCCESS - if keys are not programmed
 *		- ErrorCode - if keys are already programmed.
 *
 *******************************************************************************/
static u32 XNvm_EfuseCheckZerosBfrPrgrmg(XNvm_EfuseWriteData *WriteNvm)
{
	u32 Status = (u32)XST_FAILURE;
	u32 LockStatus = (u32)XST_FAILURE;

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
		Status = XNvm_EfuseCheckAesUserKey0Crc(XNVM_EFUSE_CRC_AES_ZEROS);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XNVM_EFUSE_ERR_USER_KEY0_ALREADY_PRGMD;
			goto END;
		}
	}
	if (WriteNvm->CheckWriteFlags.UserKey1 == TRUE) {
		Status = XNvm_EfuseCheckAesUserKey1Crc(XNVM_EFUSE_CRC_AES_ZEROS);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XNVM_EFUSE_ERR_USER_KEY1_ALREADY_PRGMD;
			goto END;
		}
	}
	if (WriteNvm->CheckWriteFlags.Ppk0Hash == TRUE) {
		Status = XNvm_EfuseCheckZeros(XNVM_EFUSE_PPK_0_HASH_START_ROW,
					(XNVM_EFUSE_PPK_0_HASH_START_ROW +
					XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS));
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XNVM_EFUSE_ERR_PPK0_HASH_ALREADY_PRGMD;
			goto END;
		}
	}
	if (WriteNvm->CheckWriteFlags.Ppk1Hash == TRUE) {
		Status = XNvm_EfuseCheckZeros(XNVM_EFUSE_PPK_1_HASH_START_ROW,
					(XNVM_EFUSE_PPK_1_HASH_START_ROW +
					XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS));
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XNVM_EFUSE_ERR_PPK1_HASH_ALREADY_PRGMD;
			goto END;
		}
	}
	if (WriteNvm->CheckWriteFlags.Ppk2Hash == TRUE) {
		Status = XNvm_EfuseCheckZeros(XNVM_EFUSE_PPK_2_HASH_START_ROW,
					(XNVM_EFUSE_PPK_2_HASH_START_ROW +
					XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS));
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XNVM_EFUSE_ERR_PPK2_HASH_ALREADY_PRGMD;
			goto END;
		}
	}

	Status = (u32)XST_SUCCESS;
END:
	LockStatus = XNvm_EfuseLockController();
	if (XST_SUCCESS == Status) {
		Status = LockStatus;
	}

	return Status;
}

/*****************************************************************************/
/**
 * This function is used verify eFUSE keys for Zeros
 *
 * @param	RowStart	Row number from which verification has to be
 *						started.
 * @param	RowEnd		Row number till which verification has to be
 *						ended.
 * @param	EfuseType	Type of the eFUSE in which these rows reside.
 *
 * @return
 *		- XST_SUCCESS - if keys are not programmed.
 * 		- Errorcode - On failure.
 *
 *******************************************************************************/
static u32 XNvm_EfuseCheckZeros(u32 RowStart, u32 RowEnd)
{
	u32 Status = (u32)XST_FAILURE;
	u32 Row;
	u32 RowDataVal = 0U;

	for (Row = RowStart; Row < RowEnd; Row++) {
		Status  = XNvm_EfuseReadCache(Row, &RowDataVal);
		if (Status != (u32)XST_SUCCESS) {
			break;
		}

		if (RowDataVal != 0x00U) {
			Status = XST_FAILURE;
			break;
		}
	}

	return Status;
}

/*****************************************************************************/
/**
 * This function performs pre checks for programming all the specified bits.
 *
 * @param	WriteNvm	Pointer to the XNvm_EfuseWriteData.
 *
 * @return
 *		XST_SUCCESS - if all the conditions for programming is satisfied
 *		Errorcode - if any of the conditions are not met
 *
 *******************************************************************************/
static u32 XNvm_EfuseWriteChecks(XNvm_EfuseWriteData *WriteNvm)
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
		SecurityMisc0 = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
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
/**
 * This function is used to read the eFUSE secure control bits from cache
 * or from eFUSE array based on user selection.
 *
 * @param	ReadBackSecCtrlBits	Pointer to the Xnvm_SecCtrlBits
 *					which holds the read secure control bits.
 *
 * @return
 * 		- XST_SUCCESS - if reads successfully
 *		- XST_FAILURE - if reading is failed
 *
 * ******************************************************************************/
static u32 XNvm_EfuseReadSecCtrlBitsRegs(XNvm_SecCtrlBits *ReadbackSecCtrlBits)
{
	u32 RegData = 0U;
	u32 Status = (u32)XST_FAILURE;

	Status = XNvm_EfuseReadCache(XNVM_EFUSE_SECURITY_CONTROL_ROW,
			&RegData);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}
	ReadbackSecCtrlBits->AesDis =
		(u8)(RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_AES_DIS_MASK);
	ReadbackSecCtrlBits->JtagErrOutDis =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_ERR_OUT_DIS_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_JTAG_ERROR_OUT_DIS_SHIFT);
	ReadbackSecCtrlBits->JtagDis =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_JTAG_DIS_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_JTAG_DIS_SHIFT);
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
	ReadbackSecCtrlBits->BootEnvWrLk =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_BOOT_ENV_WR_LK_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_BOOT_ENV_WR_LK_SHIFT);
	ReadbackSecCtrlBits->RegInitDis =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_REG_INIT_DIS_1_0_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_REG_INIT_DIS_1_0_SHIFT);
END:
	return Status;
}

/*****************************************************************************/
/**
 * This function programs the eFUSEs with the PUF syndrome data.
 *
 * @param	PrgmPufHelperData	Pointer to the Xnvm_PufHelperData.
 *
 * @return
 *		- XST_SUCCESS - if programs successfully.
 *		- Errorcode - on failure
 *
 * @note	To generate PufSyndromeData please use
 *		XPuf_Registration API
 *
 *******************************************************************************/
static u32 XNvm_EfuseWritePufSynData(XNvm_PufHelperData *PrgmPufHelperData)
{
	u32 Status = (u32)XST_FAILURE;

	if (PrgmPufHelperData == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfusePgmAndVerifyRows(XNVM_EFUSE_PUF_SYN_START_ROW,
				XNVM_EFUSE_PUF_SYN_DATA_NUM_OF_ROWS,
				XNVM_EFUSE_PAGE_2,
				PrgmPufHelperData->EfuseSynData);
END:
	return Status;
}

/*****************************************************************************/
/**
 * This function programs the eFUSEs with the PUF Chash.
 *
 * @param	PrgmPufHelperData	Pointer to the Xnvm_PufHelperData.
 *
 * @return
 *		- XST_SUCCESS - if programs successfully.
 *		- Errorcode - on failure
 *
 *******************************************************************************/
static u32 XNvm_EfuseWritePufChash(XNvm_PufHelperData *PrgmPufHelperData)
{
	u32 Status = (u32)XST_FAILURE;
	u32 Chash;

	if (PrgmPufHelperData == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Chash = PrgmPufHelperData->Chash;

	Status = XNvm_EfusePgmAndVerifyRows(XNVM_EFUSE_PUF_CHASH_ROW,
				XNVM_EFUSE_PUF_CHASH_NUM_OF_ROWS,
				XNVM_EFUSE_PAGE_0, &Chash);
END:
	return Status;
}

/*****************************************************************************/
/**
 * This function programs the eFUSEs with the PUF Aux.
 *
 * @param	PrgmPufHelperData	Pointer to the Xnvm_PufHelperData.
 *
 * @return
 *		- XST_SUCCESS - if programs successfully.
 *		- Errorcode - On failure
 *
 *******************************************************************************/
static u32 XNvm_EfuseWritePufAux(XNvm_PufHelperData *PrgmPufHelperData)
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
/**
 * This function checks whether PUF is already programmed or not.
 *
 * @return
 *		- XST_SUCCESS - if all rows are zero
 *		- Errorcode - if already programmed.
 *
 *******************************************************************************/
static u32 XNvm_EfuseCheckForZerosPuf(void)
{
	u32 Status = (u32)XST_FAILURE;
	u32 RowDataVal;

	Status = XNvm_EfuseCheckZeros(XNVM_EFUSE_PUF_SYN_START_ROW,
					(XNVM_EFUSE_PUF_SYN_START_ROW +
					XNVM_EFUSE_PUF_SYN_DATA_NUM_OF_ROWS));
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XNVM_EFUSE_ERR_PUF_SYN_ALREADY_PRGMD;
		goto END;
	}

	Status = XNvm_EfuseCheckZeros(XNVM_EFUSE_PUF_CHASH_ROW,
					XNVM_EFUSE_PUF_CHASH_ROW);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XNVM_EFUSE_ERR_PUF_CHASH_ALREADY_PRGMD;
		goto END;
	}

	Status = XNvm_EfuseReadCache(XNVM_EFUSE_PUF_AUX_ROW, &RowDataVal);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}
	if ((RowDataVal &
		XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_ECC_23_0_MASK) != 0x00U) {
		Status = (u32)XNVM_EFUSE_ERR_PUF_AUX_ALREADY_PRGMD;
	}

END :
	return Status;

}

/*****************************************************************************/
/**
 * This function is used to read the eFUSE secure control bits from cache
 * or from eFUSE array based on user selection.
 *
 * @param	ReadBackPufSecCtrlBits 	Pointer to Xnvm_PufSecCtrlBits
 * 					which holds the read secure control bits.
 *
 *@return
 * 		- XST_SUCCESS - if reads successfully
 * 		- XST_FAILURE - if reading is failed
 *
 *******************************************************************************/
static u32 XNvm_EfuseReadPufSecCtrlBitsRegs(
	XNvm_PufSecCtrlBits *ReadBackPufSecCtrlBits)
{
	u32 PufEccCtrlReg = 0U;
	u32 PufSecurityCtrlReg = 0U;
	u32 Status = (u32)XST_FAILURE;

	Status = XNvm_EfuseReadCache(
			XNVM_EFUSE_PUF_AUX_ROW,
			&PufEccCtrlReg);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}
	Status = XNvm_EfuseReadCache(
			XNVM_EFUSE_SECURITY_CONTROL_ROW,
			&PufSecurityCtrlReg);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}
	ReadBackPufSecCtrlBits->PufRegenDis =
		(u8)((PufEccCtrlReg &
		XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_REGEN_DIS_MASK) >>
		XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_PUF_REGEN_DIS_SHIFT);
	ReadBackPufSecCtrlBits->PufHdInvalid =
		(u8)((PufEccCtrlReg &
		XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_HD_INVLD_MASK) >>
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
/**
 * This function is used to read Misc control bits from cache.
 *
 * @param	ReadMiscCtrlBits 	Pointer to Xnvm_MiscCtrlBits
 *					which holds the read secure control bits.
 *
 *@return
 * 		- XST_SUCCESS - if reads successfully
 * 		- XST_FAILURE - if reading is failed
 *
 *******************************************************************************/
static u32 XNvm_EfuseReadMiscCtrlBitsRegs(XNvm_MiscCtrlBits *ReadMiscCtrlBits)
{
	u32 ReadReg = 0U;
	u32 Status = (u32)XST_FAILURE;

	if (ReadMiscCtrlBits == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfuseReadCache(
			XNVM_EFUSE_MISC_CTRL_ROW,
			&ReadReg);
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
END :
	return Status;
}


/******************************************************************************/
/**
 * This function Programs Revocation Id to the eFuse.
 *
 * @param	RevokeIdEfuseNum	Revocation eFuse number to be programmed
 *
 * @param 	EfuseData		EfuseData to be programmed to Revocation
 * 							eFuse.
 * @return
 *		- XST_SUCCESS - if reads successfully.
 *		- Errorcode - On failure.
 ******************************************************************************/
static u32 XNvm_PrgmRevocationId(u32 RevokeIdEfuseNum, u32 EfuseData)
{
	u32 Status = (u32)XST_FAILURE;
	u32 LockStatus = (u32)XST_FAILURE;
	u8 RevokeIdInBits[XNVM_EFUSE_MAX_BITS_IN_ROW] = {0};
	u8 RevokeIdInBitsRd[XNVM_EFUSE_MAX_BITS_IN_ROW] = {0};
	u32 RevokeId;
	u32 Column;
	u32 Row;
	u32 RevokeIdInBytes;

	if (RevokeIdEfuseNum >= XNVM_NUM_OF_REVOKE_ID_FUSES) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Row = RevokeIdEfuseNum + XNVM_EFUSE_REVOCATION_ID_0_ROW;

	XNvm_ConvertHexToByteArray((u8 *)&EfuseData,
			RevokeIdInBits, XNVM_EFUSE_MAX_BITS_IN_ROW);
	Status = XNvm_EfuseReadCache(Row, &RevokeId);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	XNvm_ConvertHexToByteArray((u8 *)&RevokeId, RevokeIdInBitsRd,
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
	XNvm_ConvertByteArrayToHex(RevokeIdInBits, (u8 *)&RevokeIdInBytes,
					XNVM_EFUSE_MAX_BITS_IN_ROW);

	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
					XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XNvm_EfusePgmAndVerifyRows(Row, 0x1U,
			XNVM_EFUSE_PAGE_0, &RevokeIdInBytes);
END :
	XNvm_EfuseDisableProgramming();
	LockStatus = XNvm_EfuseLockController();
	if (XST_SUCCESS == Status) {
		Status = LockStatus;
	}
	return Status;
}

/******************************************************************************/
/*
 * This function Programs User eFuses.
 *
 * @param	StartRow	Start row of the user fuse to be programmed
 *
 * @param	RowCount 	Number of eFuse rows to be programmed.
 *
 * @param 	RowData		RowData to be programmed to User Fuse.
 *
 * @return
 *		- XST_SUCCESS - if reads successfully.
 *		- Errorcode - On failure.
 *
 ******************************************************************************/

u32 XNvm_EfuseWriteUserFuses(u32 StartUserFuseNum, u8 NumOfUserFuses,
				const u32* UserFuseData)
{
	u32 Status = (u32)XST_FAILURE;
	u32 LockStatus = (u32)XST_FAILURE;
	u8 UserFuseInBits[XNVM_EFUSE_MAX_BITS_IN_ROW] = {0};
	u8 UserFuseInBitsRd[XNVM_EFUSE_MAX_BITS_IN_ROW] = {0};
	u32 UserFuseValue;
	u32 Column;
	u32 Row;
	u32 UserFuseInBytes;
	u32 StartRow;
	u32 EndRow;

	if ((StartUserFuseNum < XNVM_USER_FUSE_START_NUM) &&
		(StartUserFuseNum > XNVM_USER_FUSE_END_NUM)) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	if (UserFuseData == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	if (NumOfUserFuses > XNVM_NUM_OF_USER_FUSES) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
					XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	StartRow = XNVM_EFUSE_USER_FUSE_START_ROW + StartUserFuseNum - 1U;
	EndRow = StartRow + NumOfUserFuses;

	for (Row = StartRow; Row < EndRow; Row++) {
		XNvm_ConvertHexToByteArray((u8 *)UserFuseData,
				UserFuseInBits, XNVM_EFUSE_MAX_BITS_IN_ROW);
		Status = XNvm_EfuseReadCache(Row, &UserFuseValue);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		XNvm_ConvertHexToByteArray((u8 *)&UserFuseValue, UserFuseInBitsRd,
				XNVM_EFUSE_MAX_BITS_IN_ROW);
		for (Column = 0U; Column < XNVM_EFUSE_MAX_BITS_IN_ROW;
				Column++) {
			if ((UserFuseInBits[Column] == 0U) &&
					(UserFuseInBitsRd[Column] == 1U)) {
				Status = (u32)XNVM_EFUSE_ERR_BEFORE_PROGRAMMING |
					(u32)XNVM_EFUSE_ERR_BIT_CANT_REVERT;
				goto END;
			}
			if ((UserFuseInBits[Column] == 1U) &&
					(UserFuseInBitsRd[Column] == 1U)) {
				UserFuseInBits[Column] = 0U;
			}
		}
		XNvm_ConvertByteArrayToHex(UserFuseInBits, (u8 *)&UserFuseInBytes,
				XNVM_EFUSE_MAX_BITS_IN_ROW);


		Status = XNvm_EfusePgmAndVerifyRows(Row, 0x1U,
				XNVM_EFUSE_PAGE_0, &UserFuseInBytes);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		memset(UserFuseInBits, 0, sizeof(UserFuseInBits));
		memset(UserFuseInBitsRd, 0, sizeof(UserFuseInBitsRd));
		UserFuseInBytes = 0x00U;
		UserFuseData++;
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

/******************************************************************************/
/**
 * This function reads User eFuses from Cache.
 *
 *@param	UserFuseData 	UserFuseData to be read from eFuse Cache.
 *
 *@return
 *	- XST_SUCCESS - if reads successfully.
 *	- Errorcode - On failure.
 *
 ******************************************************************************/
u32 XNvm_EfuseReadUserFuses(u32 *UserFuseData)
{
	u32 Status = XST_FAILURE;

	if (UserFuseData == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfuseReadCacheRange(XNVM_EFUSE_USER_FUSE_START_ROW,
			XNVM_NUM_OF_USER_FUSES,
			UserFuseData);
	if (Status != XST_SUCCESS) {
		Status = (Status | (u32)XNVM_EFUSE_ERR_RD_USER_FUSES);
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * This function Programs Protection eFuses.
 *
 * @return
 *		- XST_SUCCESS - if reads successfully.
 *		- Errorcode - On failure.
 *******************************************************************************/
static u32 XNvm_PrgmProtectionEfuse(void)
{
	u32 Status = XST_FAILURE;
	u32 LockStatus = XST_FAILURE;
	u32 SecurityCtrlData;
	u32 SecurityMisc0Data;
	u32 SecurityMisc1Data;
	u32 MiscCtrlData;
	u32 BootEnvCtrlRow;
	u32 PufChashData;

	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
					XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	SecurityCtrlData  = XNvm_EfuseReadReg(
			XNVM_EFUSE_CACHE_BASEADDR,
			XNVM_EFUSE_CACHE_SECURITY_CONTROL_OFFSET);

	if (SecurityCtrlData != 0x00U) {
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW,
				XNVM_EFUSE_ROW_43_0_PROT_COLUMN);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_ROW_43_0_PROT);
			goto END;
		}
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW,
				XNVM_EFUSE_ROW_43_1_PROT_COLUMN);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_ROW_43_1_PROT);
			goto END;
		}
	}
	SecurityMisc0Data = XNvm_EfuseReadReg(
			XNVM_EFUSE_CACHE_BASEADDR,
			XNVM_EFUSE_CACHE_SECURITY_MISC_0_OFFSET);
	if ((SecurityMisc0Data &
		XNVM_EFUSE_CACHE_SECURITY_MISC_0_DEC_EFUSE_ONLY_MASK) !=
			0x00U) {
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW,
				XNVM_EFUSE_ROW_57_PROT_COLUMN);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_ROW_57_0_PROT);
			goto END;
		}
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW,
				XNVM_EFUSE_ROW_57_1_PROT_COLUMN);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_ROW_57_1_PROT);
			goto END;
		}
	}

	Status = XNvm_EfuseCheckZeros(XNVM_EFUSE_PPK_0_HASH_START_ROW,
			(XNVM_EFUSE_PPK_0_HASH_START_ROW +
			 (XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS * 3)));
	if (Status != XST_SUCCESS) {
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW,
				XNVM_EFUSE_ROW64_87_0_PROT_COLUMN);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_ROW64_87_0_PROT);
			goto END;
		}
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW,
				XNVM_EFUSE_ROW64_87_1_PROT_COLUMN);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_ROW64_87_1_PROT);
			goto END;
		}
	}
	Status = XNvm_EfuseCheckZeros(
			XNVM_EFUSE_META_HEADER_IV_START_ROW,
			(XNVM_EFUSE_META_HEADER_IV_START_ROW +
			 XNVM_EFUSE_IV_NUM_OF_ROWS));
	if (Status != XST_SUCCESS) {
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW,
				XNVM_EFUSE_ROW96_99_0_PROT_COLUMN);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_ROW96_99_0_PROT);
			goto END;
		}
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW,
				XNVM_EFUSE_ROW96_99_1_PROT_COLUMN);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_ROW96_99_1_PROT);
			goto END;
		}
	}

	BootEnvCtrlRow = XNvm_EfuseReadReg(
			XNVM_EFUSE_CACHE_BASEADDR,
			XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_OFFSET);
	if (BootEnvCtrlRow != 0x00U) {
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW,
				XNVM_EFUSE_ROW_37_PROT_COLUMN);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_ROW_37_PROT);
			goto END;
		}
	}
	MiscCtrlData = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
			XNVM_EFUSE_CACHE_MISC_CTRL_OFFSET);
	if ((MiscCtrlData &
		(XNVM_EFUSE_CACHE_MISC_CTRL_PPK0_INVLD_1_0_MASK |
		XNVM_EFUSE_CACHE_MISC_CTRL_PPK1_INVLD_1_0_MASK |
		XNVM_EFUSE_CACHE_MISC_CTRL_PPK2_INVLD_1_0_MASK |
		XNVM_EFUSE_CACHE_MISC_CTRL_CRYPTO_KAT_EN_MASK |
		XNVM_EFUSE_CACHE_MISC_CTRL_GD_ROM_MONITOR_EN_MASK)) != 0x00U)
	{
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW,
				XNVM_EFUSE_ROW_40_PROT_COLUMN);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_ROW_40_PROT);
			goto END;
		}
	}

	PufChashData = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
					XNVM_EFUSE_CACHE_PUF_CHASH_OFFSET);
	if (PufChashData != 0x00U) {

		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW,
				XNVM_EFUSE_ROW_42_PROT_COLUMN);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_ROW_42_PROT);
			goto END;
		}
	}

	SecurityMisc1Data = XNvm_EfuseReadReg(
			XNVM_EFUSE_CACHE_BASEADDR,
			XNVM_EFUSE_CACHE_SECURITY_MISC_0_OFFSET);
	if ((SecurityMisc1Data &
		XNVM_EFUSE_SECURITY_MISC_1_PROT_MASK) != 0x00U) {
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW,
				XNVM_EFUSE_ROW_58_PROT_COLUMN);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_ROW_58_PROT);
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
/******************************************************************************/
/**
 * This function sets and then verifies the specified bits in the eFUSE.
 *
 * @param	StartRow	Starting Row number (0-based addressing)
 *		RowCount	Number of Rows to be read
 *		RowData		Pointer to memory location where bitmap
 *				to be written is stored.
 *				Only bit set are used for programming eFUSE.
 *
 * @return
 *		- XST_SUCCESS - Specified bit set in eFUSE
 *		- XNVM_EFUSE_ERR_PGM_TIMEOUT - eFUSE programming timed out
 *		- XNVM_EFUSE_ERR_PGM - eFUSE programming failed
 *		- XNVM_EFUSE_ERR_PGM_VERIFY - Verification failed,
 *						specified bit is not set.
 *		- XST_FAILURE - Unexpected error
 *
 ******************************************************************************/
u32 XNvm_EfusePgmAndVerifyRows(u32 StartRow, u8 RowCount,
			XNvm_EfuseType EfuseType, const u32* RowData)
{
	u32 Status = XST_FAILURE;
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
	return Status;
}

/******************************************************************************/
/**
 * This function Locks the eFUSE Controller to prevent accidental writes to
 * eFUSE controller registers.
 *
 * @return
 *		- XST_SUCCESS - eFUSE controller locked
 *		- XNVM_EFUSE_ERR_LOCK - Failed to lock eFUSE controller
 *					register access
 *		- XST_FAILURE - Unexpected error
 *
 ******************************************************************************/
static inline u32 XNvm_EfuseLockController(void)
{
	u32 Status = XST_FAILURE;
	u32 LockStatus;

	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
			XNVM_EFUSE_WR_LOCK_REG_OFFSET,
			~XNVM_EFUSE_WR_UNLOCK_PASSCODE);
	LockStatus = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
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
 * This function Unlocks the eFUSE Controller for writing to its registers.
 *
 * @return
 *		XST_SUCCESS - 	eFUSE controller locked
 *		XNVM_EFUSE_ERR_UNLOCK - Failed to unlock eFUSE controller
 *					register access
 *		XST_FAILURE - 	Unexpected error
 *
 ******************************************************************************/
static inline u32 XNvm_EfuseUnlockController(void)
{
	u32 Status = XST_FAILURE;
	u32 LockStatus;

	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_WR_LOCK_REG_OFFSET,
				XNVM_EFUSE_WR_UNLOCK_PASSCODE);
	LockStatus = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
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
 * This function disables power down of eFUSE macros.
 *
 ******************************************************************************/
static inline void XNvm_EfuseDisablePowerDown(void)
{
	u32 PowerDownStatus;

	PowerDownStatus = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
						XNVM_EFUSE_PD_REG_OFFSET);
	if(XNVM_EFUSE_PD_ENABLE == PowerDownStatus) {
		/* When changing the Power Down state, wait a separation period
		 *  of 1us, before and after accessing the eFuse-Macro.
		 */
		usleep(XNVM_ONE_MICRO_SECOND);
		XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_PD_REG_OFFSET,
					~XNVM_EFUSE_PD_ENABLE);
		usleep(XNVM_ONE_MICRO_SECOND);
	}
}

/******************************************************************************/
/**
 * This function sets read mode of eFUSE controller.
 *
 ******************************************************************************/
static inline void XNvm_EfuseSetReadMode(XNvm_EfuseRdMode RdMode)
{
	if(XNVM_EFUSE_NORMAL_RD == RdMode) {
		XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_CFG_REG_OFFSET,
					XNVM_EFUSE_CFG_NORMAL_RD);
	}
	else {
		XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_CFG_REG_OFFSET,
					(XNVM_EFUSE_CFG_ENABLE_PGM |
					XNVM_EFUSE_CFG_MARGIN_RD));
	}
}

/******************************************************************************/
/**
 * This function sets reference clock of eFUSE controller.
 *
 ******************************************************************************/
static inline void XNvm_EfuseSetRefClk(void)
{
	XNvm_EfuseWriteReg(XNVM_CRP_BASE_ADDR,
				XNVM_CRP_EFUSE_REF_CLK_REG_OFFSET,
				XNVM_CRP_EFUSE_REF_CLK_SELSRC);
}

/******************************************************************************/
/**
 * This function enabled programming mode of eFUSE controller.
 *
 *
 ******************************************************************************/
static inline void XNvm_EfuseEnableProgramming(void)
{
	u32 Cfg = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_CFG_REG_OFFSET);

	Cfg = Cfg | XNVM_EFUSE_CFG_ENABLE_PGM;
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_CFG_REG_OFFSET, Cfg);
}

/******************************************************************************/
/**
 * This function disables programming mode of eFUSE controller.
 *
 ******************************************************************************/
static inline void XNvm_EfuseDisableProgramming(void)
{
	u32 Cfg = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_CFG_REG_OFFSET);

	Cfg = Cfg & ~XNVM_EFUSE_CFG_ENABLE_PGM;
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_CFG_REG_OFFSET, Cfg);
}

/******************************************************************************/
/**
 * This function initializes eFUSE controller timers.
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
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_TPGM_REG_OFFSET, Tpgm);

	/* TRD = ceiling(217ns/REF_CLK_PERIOD) */
	Trd = Xil_Ceil(217.0e-9 * XNVM_PS_REF_CLK_FREQ);
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_TRD_REG_OFFSET, Trd);

	/* TRDM = ceiling(500ns/REF_CLK_PERIOD)*/
	Trdm = Xil_Ceil(500.0e-9 * XNVM_PS_REF_CLK_FREQ);
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_TRDM_REG_OFFSET, Trdm);

	/* TSU_H_PS = ceiling(208ns/REF_CLK_PERIOD) */
	Tsu_h_ps = Xil_Ceil(208.0e-9 * XNVM_PS_REF_CLK_FREQ);
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_TSU_H_PS_REG_OFFSET,
				Tsu_h_ps);

	/* TSU_H_PS_CS = ceiling(143ns/REF_CLK_PERIOD) */
	Tsu_h_ps_cs = Xil_Ceil(143.0e-9 * XNVM_PS_REF_CLK_FREQ);
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_TSU_H_PS_CS_REG_OFFSET,
				Tsu_h_ps_cs);

	/* TSU_H_CS = ceiling(184ns/REF_CLK_PERIOD) */
	Tsu_h_cs = Xil_Ceil(184.0e-9 * XNVM_PS_REF_CLK_FREQ);
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_TSU_H_CS_REG_OFFSET,
				Tsu_h_cs);
}

/******************************************************************************/
/**
 * This function setups eFUSE controller for given operation and read mode.
 *
 * @param	Op -	Opeartion to be performed read/program(write).
 *		RdMode - Read mode for eFUSE read operation
 *
 * @return
 *		- XST_SUCCESS - eFUSE controller setup for given op
 *		- XNVM_EFUSE_ERR_UNLOCK - Failed to unlock eFUSE controller
 *					register access
 *		- XST_FAILURE - Unexpected error
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
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
			XNVM_EFUSE_TEST_CTRL_REG_OFFSET, 0x00);


	Status = XST_SUCCESS;

END:
	return Status;
}

/******************************************************************************/
/**
 * This function reads 32-bit data from eFUSE specified Row and Page.
 *
 * @param	Page - Page number
 *		Row - Row number (0-based addressing)
 *		RowData	- Pointer to memory location where 32-bit read data
 *				is to be stored
 *
 * @return
 *		- XST_SUCCESS - 32-bit data is read from specified location
 *		- XNVM_EFUSE_ERR_RD_TIMEOUT - Timeout occured while reading the
 *						eFUSE
 *		- XNVM_EFUSE_ERR_RD - eFUSE Read failed
 *		- XST_FAILURE - Unexpected error
 *
 ******************************************************************************/
static u32 XNvm_EfuseReadRow(u8 Page, u32 Row, u32* RowData)
{
	u32 Status = XST_FAILURE;
	u32 EventMask = 0U;
	u32 EfuseReadAddr;

	EfuseReadAddr = (Page << XNVM_EFUSE_ADDR_PAGE_SHIFT) |
			(Row << XNVM_EFUSE_ADDR_ROW_SHIFT);

	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
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
		*RowData = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_RD_DATA_REG_OFFSET);
		Status = XST_SUCCESS;
	}
	return Status;
}

/******************************************************************************/
/**
 * This function reads 32-bit data from cache specified by Row
 *
 * @param	Row - Starting Row number (0-based addressing)
 *		RowData	- Pointer to memory location where read 32-bit row data
 *			is to be stored
 *
 * @return
 *		- XST_SUCCESS - Specified data read
 *		- XNVM_EFUSE_ERR_CACHE_PARITY - Parity error exist in cache
 *		- XST_FAILURE - Unexpected error
 *
 ******************************************************************************/
static u32 XNvm_EfuseReadCache(u32 Row, u32* RowData)
{
	u32 Status = XST_FAILURE;
	u32 CacheData;
	u32 IsrStatus;

	CacheData = Xil_In32(XNVM_EFUSE_CACHE_BASEADDR + Row * sizeof(u32));
	IsrStatus = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
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
 * This function reads 32-bit rows from eFUSE cache.
 *
 * @param	StartRow - Starting Row number (0-based addressing)
 *		RowCount - Number of Rows to be read
 *		RowData  - Pointer to memory location where read 32-bit row data(s)
 *				is to be stored
 *
 * @return
 *		- XST_SUCCESS	- Specified data read
 *		- XNVM_EFUSE_ERR_CACHE_PARITY - Parity error exist in cache
 *		- XST_FAILURE - Unexpected error
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
 * This function sets the specified bit in the eFUSE.
 *
 * @param	Page - Page number
 *		Row  - Row number (0-based addressing)
 *		Col  - Col number (0-based addressing)
 *
 * @return
 *		- XST_SUCCESS	- Specified bit set in eFUSE
 *		- XNVM_EFUSE_ERR_PGM_TIMEOUT - eFUSE programming timed out
 *		- XNVM_EFUSE_ERR_PGM - eFUSE programming failed
 *		- XST_FAILURE - Unexpected error
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

	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
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
 * This function verify the specified bit set in the eFUSE.
 *
 * @param	Page- Page number
 *		Row - Row number (0-based addressing)
 *		Col - Col number (0-based addressing)
 *
 * @return
 *		- XST_SUCCESS - Specified bit set in eFUSE
 *		- XNVM_EFUSE_ERR_PGM_VERIFY - Verification failed,
 *						specified bit is not set.
 *		- XST_FAILURE - Unexpected error
 *
 ******************************************************************************/
static u32 XNvm_EfuseVerifyBit(u32 Page, u32 Row, u32 Col)
{
	u32 RdAddr;
	u32 RegData;
	u32 EventMask = 0x00;
	u32 Status = XST_FAILURE;

	/*
	 * If Row Belongs to AES key or User key 0 or
	 * User key 1 can't verify the bit
	 * as those can be checked only CRC
	 */
	if (((Row >= XNVM_EFUSE_AES_KEY_START_ROW) &&
		(Row <= XNVM_EFUSE_USER_KEY_1_END_ROW))) {
		Status = (u32)XST_SUCCESS;
		goto END;
	}
	RdAddr = (Page << XNVM_EFUSE_ADDR_PAGE_SHIFT) |
				(Row << XNVM_EFUSE_ADDR_ROW_SHIFT);
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
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
		RegData = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_RD_DATA_REG_OFFSET);
		if (RegData & (0x01U << Col)) {
			Status = XST_SUCCESS;
		}
	} else {
		Status = XNVM_EFUSE_ERR_PGM_VERIFY;
	}
END:
	return Status;
}

/******************************************************************************/
/**
 * This function sets and then verifies the specified bit in the eFUSE.
 *
 * @param	Page - Page number
 *		Row  - Row number (0-based addressing)
 *		Col  - Col number (0-based addressing)
 *
 * @return
 *		- XST_SUCCESS - Specified bit set in eFUSE
 *		- XNVM_EFUSE_ERR_PGM_TIMEOUT - eFUSE programming timed out
 *		- XNVM_EFUSE_ERR_PGM - eFUSE programming failed
 *		- XNVM_EFUSE_ERR_PGM_VERIFY - Verification failed,
 *						specified bit is not set.
 *		- XST_FAILURE - Unexpected error
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
 * This function program Tbits
 *
 * @return
 *		- XST_SUCCESS - On Success
 *		- XST_FAILURE - Failure in programming
 *
 ******************************************************************************/
static u32 XNvm_EfusePgmTBits(void)
{
	u32 Status = XST_FAILURE;
	u32 TbitsPrgrmReg;
	u32 RowDataVal = 0U;
	u32 Column;

	/* Enable TBITS programming bit */
	TbitsPrgrmReg = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_TEST_CTRL_REG_OFFSET);

	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
			XNVM_EFUSE_TEST_CTRL_REG_OFFSET,
			(TbitsPrgrmReg & (~XNVM_EFUSE_TBITS_PRGRMG_EN_MASK)));

	Status = XNvm_EfuseReadRow(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW,
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
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW,
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
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW,
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
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW, Column);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_1,
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW, Column);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_2,
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW, Column);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
	}

	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
			XNVM_EFUSE_TEST_CTRL_REG_OFFSET,
			TbitsPrgrmReg);

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function reloads the cache of eFUSE so that can be directly read from
 * cache.
 *
 * @return
 * 		- XST_SUCCESS - on successful cache reload
 *		- ErrorCode - on failure
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

	RegStatus = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_WR_LOCK_REG_OFFSET);
	/* Check the unlock status */
	if (RegStatus != 0U) {
		XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_WR_LOCK_REG_OFFSET,
					XNVM_EFUSE_WR_UNLOCK_PASSCODE);
	}

	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
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

	CacheStatus = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
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
 * This function checks wheather Tbits are programmed or not
 *
 * @return
 *		- XST_SUCCESS - On Success
 *		- XST_FAILURE - On Failure
 *
 ******************************************************************************/
static u32 XNvm_EfuseCheckForTBits(void)
{
	u32 Status = (u32)XST_FAILURE;
	u32 ReadReg;
	u32 TbitMask = (XNVM_EFUSE_STATUS_TBIT_0 |
			XNVM_EFUSE_STATUS_TBIT_1 |
			XNVM_EFUSE_STATUS_TBIT_2 );

	ReadReg = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
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
