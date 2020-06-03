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
static u32 XNvm_EfuseCheckZeros(u32 RowStart, u32 RowEnd);
static u32 XNvm_EfuseComputeProgrammableBits(u32 *ReqData, u32 *PrgmData,
                                                u32 StartRow, u32 EndRow);
static u32 XNvm_EfuseValidateWriteReq(XNvm_EfuseData *WriteChecks);
static u32 XNvm_EfuseValidateRevokeIdsWriteReq(XNvm_EfuseRevokeIds *RevokeIds);
static u32 XNvm_EfuseValidateIVsWriteReq(XNvm_EfuseIvs *EfuseIv);
static u32 XNvm_EfuseValidateAesWriteReq(XNvm_EfuseAesKeys *Keys);
static u32 XNvm_EfuseValidatePpkWriteReq(XNvm_EfusePpkHash *Hash);
static u32 XNvm_EfuseValidateUserFusesWriteReq(XNvm_EfuseUserData *WriteUserFuses);
static u32 XNvm_EfuseValidateDecOnlyWriteReq(XNvm_EfuseData *WriteReq);
static u32 XNvm_EfuseValidateIV(u32 *Iv, u32 Row);
static u32 XNvm_EfuseWritePufAux(u32 Aux);
static u32 XNvm_EfuseWritePufChash(u32 Chash);
static u32 XNvm_EfuseWritePufSynData(u32 *SynData);
static u32 XNvm_EfuseWritePufSecCtrl(XNvm_EfusePufSecCtrlBits *PufSecCtrlBits);
static u32 XNvm_EfuseIsPufHelperDataEmpty(void);
static u32 XNvm_EfuseWriteSecCtrl(XNvm_EfuseSecCtrlBits *SecCtrl);
static u32 XNvm_EfusePrgmGlitchCfgValues(XNvm_EfuseGlitchCfgBits *WriteGlitchCfg);
static u32 XNvm_EfusePrgmGlitchWriteLock(XNvm_EfuseGlitchCfgBits *WriteGlitchCfg);
static u32 XNvm_EfusePrgmGdRomMonEn(XNvm_EfuseGlitchCfgBits *WriteGlitchCfg);
static u32 XNvm_EfusePrgmGdRomHaltBootEn(XNvm_EfuseGlitchCfgBits *WriteGlitchCfg);
static u32 XNvm_EfusePrgmAesKeys(XNvm_EfuseAesKeys *Keys);
static u32 XNvm_EfusePrgmPpkHash(XNvm_EfusePpkHash *Hash);
static u32 XNvm_EfusePrgmDecOnly(XNvm_EfuseDecOnly *DecOnly);
static u32 XNvm_EfusePrgmUserFuses(XNvm_EfuseUserData *WriteUserFuses);
static u32 XNvm_EfusePrgmIVs(XNvm_EfuseIvs *Ivs);
static u32 XNvm_EfusePrgmPpkRevokeFuses(XNvm_EfuseMiscCtrlBits *PpkSelect);
static u32 XNvm_EfusePrgmRevocationIdFuses(XNvm_EfuseRevokeIds *WriteRevokeId);
static u32 XNvm_EfusePrgmProtectionEfuse(void);

/*************************** Variable Definitions *****************************/

/*************************** Function Definitions *****************************/

/***************************************************************************/
/**
 * This function is used as a wrapper to program below eFuses
 * AES key
 * User key 0
 * User key 1
 * PPK0/PPK1/PPK2 hash
 * IVs
 * Revocation Ids
 * User Fuses
 * Secure and Control bits.
 *
 * @param	WriteNvm	Pointer to the XNvm_EfuseData.
 *
 * @return
 *	- XST_SUCCESS - On Specified data write
 * 	- Error code - On failure
 *
 * @note
 *	After eFUSE programming is complete, the cache is automatically
 *	reloaded so all programmed eFUSE bits can be directly read from
 *	cache.
 *
 *****************************************************************************/
u32 XNvm_EfuseWrite(XNvm_EfuseData *WriteNvm)
{
	u32 Status = (u32)XST_FAILURE;
	u32 LockStatus = (u32)XST_FAILURE;

	if (WriteNvm == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if ((WriteNvm->AesKeys == NULL) &&
		(WriteNvm->PpkHash == NULL) &&
		(WriteNvm->DecOnly == NULL) &&
		(WriteNvm->SecCtrlBits == NULL) &&
		(WriteNvm->RevokeIds == NULL) &&
		(WriteNvm->MiscCtrlBits == NULL) &&
		(WriteNvm->Ivs == NULL) &&
		(WriteNvm->UserFuses == NULL) &&
		(WriteNvm->GlitchCfgBits == NULL)) {
		Status = XNVM_EFUSE_ERR_NTHG_TO_BE_PROGRAMMED;
		goto END;
	}

	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
					XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseValidateWriteReq(WriteNvm);
	if (Status != (u32)XST_SUCCESS) {
		Status = (Status | (u32)XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	if (WriteNvm->GlitchCfgBits != NULL) {

		Status = XNvm_EfusePrgmGlitchCfgValues(WriteNvm->GlitchCfgBits);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		if (WriteNvm->GlitchCfgBits->GlitchDetWrLk == TRUE) {
			Status =  XNvm_EfusePrgmGlitchWriteLock(WriteNvm->GlitchCfgBits);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}

		if (WriteNvm->GlitchCfgBits->GdRomMonitorEn == TRUE) {
			Status = XNvm_EfusePrgmGdRomMonEn(WriteNvm->GlitchCfgBits);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}

		if (WriteNvm->GlitchCfgBits->GdHaltBootEn == TRUE) {
			Status = XNvm_EfusePrgmGdRomHaltBootEn(WriteNvm->GlitchCfgBits);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}
	}

	if (WriteNvm->AesKeys != NULL) {

		Status = XNvm_EfusePrgmAesKeys(WriteNvm->AesKeys);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	if (WriteNvm->PpkHash != NULL) {

		Status = XNvm_EfusePrgmPpkHash(WriteNvm->PpkHash);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	if (WriteNvm->Ivs != NULL) {

		Status = XNvm_EfusePrgmIVs(WriteNvm->Ivs);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
	}
	if (WriteNvm->DecOnly != NULL) {

		Status = XNvm_EfusePrgmDecOnly(WriteNvm->DecOnly);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	if (WriteNvm->RevokeIds != NULL) {

		Status = XNvm_EfusePrgmRevocationIdFuses(WriteNvm->RevokeIds);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
	}
	if (WriteNvm->MiscCtrlBits != NULL) {

		Status = XNvm_EfusePrgmPpkRevokeFuses(WriteNvm->MiscCtrlBits);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
	}
	if (WriteNvm->UserFuses != NULL) {

		Status = XNvm_EfusePrgmUserFuses(WriteNvm->UserFuses);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
	}
	if (WriteNvm->SecCtrlBits != NULL) {

		Status = XNvm_EfuseWriteSecCtrl(WriteNvm->SecCtrlBits);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
	}
	Status = XNvm_EfuseCacheLoad();
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfusePrgmProtectionEfuse();
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
	u32 LockStatus = 0U;
	u32 ReadReg = 0U;
	u8 IsUnlocked = FALSE;

	LockStatus = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_WR_LOCK_REG_OFFSET);
	if(0x01 == LockStatus)	{
		Status = XNvm_EfuseUnlockController();
		if (Status != XST_SUCCESS) {
			goto END;
		}
		IsUnlocked = TRUE;
	}
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
	if (IsUnlocked == TRUE) {
		XNvm_EfuseLockController();
		IsUnlocked = FALSE;
	}
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
	u32 LockStatus = 0U;
	u8 IsUnlocked = FALSE;

	LockStatus = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_WR_LOCK_REG_OFFSET);
	if(0x01 == LockStatus)	{
		Status = XNvm_EfuseUnlockController();
		if (Status != XST_SUCCESS) {
			goto END;
		}
		IsUnlocked = TRUE;
	}
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
	if (IsUnlocked == TRUE) {
		XNvm_EfuseLockController();
		IsUnlocked = FALSE;
	}
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
	u32 LockStatus = 0U;
	u8 IsUnlocked = FALSE;

	LockStatus = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_WR_LOCK_REG_OFFSET);
	if(0x01 == LockStatus)	{
		Status = XNvm_EfuseUnlockController();
		if (Status != XST_SUCCESS) {
			goto END;
		}
		IsUnlocked = TRUE;
	}
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
	if (IsUnlocked == TRUE) {
		XNvm_EfuseLockController();
		IsUnlocked = FALSE;
	}
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to read the eFUSE secure control bits from cache.
 *
 * @param	SecCtrlBits		Pointer to the Xnvm_SecCtrlBits
 * 					which holds the read secure control bits.
 * @return
 * 		- XST_SUCCESS - On Successful read.
 * 		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter
 *
 ******************************************************************************/
u32 XNvm_EfuseReadSecCtrlBits(XNvm_EfuseSecCtrlBits *SecCtrlBits)
{
	u32 Status = (u32)XST_FAILURE;
	u32 RegData = 0U;

	if (SecCtrlBits == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	Status = XNvm_EfuseReadCache(XNVM_EFUSE_SECURITY_CONTROL_ROW,
			&RegData);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}
	SecCtrlBits->AesDis =
		(u8)(RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_AES_DIS_MASK);
	SecCtrlBits->JtagErrOutDis =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_ERR_OUT_DIS_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_JTAG_ERROR_OUT_DIS_SHIFT);
	SecCtrlBits->JtagDis =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_JTAG_DIS_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_JTAG_DIS_SHIFT);
	SecCtrlBits->Ppk0WrLk =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK0_WR_LK_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK0_WR_LK_SHIFT);
	SecCtrlBits->Ppk1WrLk =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK1_WR_LK_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK1_WR_LK_SHIFT);
	SecCtrlBits->Ppk2WrLk =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK2_WR_LK_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK2_WR_LK_SHIFT);
	SecCtrlBits->AesCrcLk =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_AES_CRC_LK_1_0_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_AES_CRC_LK_1_0_SHIFT);
	SecCtrlBits->AesWrLk =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_AES_WR_LK_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_AES_WR_LK_SHIFT);
	SecCtrlBits->UserKey0CrcLk =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_0_CRC_LK_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_0_CRC_LK_0_SHIFT);
	SecCtrlBits->UserKey0WrLk =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_0_WR_LK_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_0_WR_LK_SHIFT);
	SecCtrlBits->UserKey1CrcLk =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_1_CRC_LK_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_1_CRC_LK_0_SHIFT);
	SecCtrlBits->UserKey1WrLk =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_1_WR_LK_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_1_WR_LK_SHIFT);
	SecCtrlBits->SecDbgDis =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_SEC_DEBUG_DIS_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_SEC_DEBUG_DIS_1_0_SHIFT);
	SecCtrlBits->BootEnvWrLk =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_BOOT_ENV_WR_LK_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_BOOT_ENV_WR_LK_SHIFT);
	SecCtrlBits->RegInitDis =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_REG_INIT_DIS_1_0_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_REG_INIT_DIS_1_0_SHIFT);
END:
	return Status;
}

/******************************************************************************/
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
u32 XNvm_EfuseWritePuf(XNvm_EfusePufHd *PufHelperData)
{
	u32 Status = (u32)XST_FAILURE;
	u32 LockStatus = (u32)XST_FAILURE;
	u32 PufSecurityCtrlReg;

	if (PufHelperData == NULL)
	{
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfuseReadCache(
				XNVM_EFUSE_SECURITY_CONTROL_ROW,
				&PufSecurityCtrlReg);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	if ((PufSecurityCtrlReg &
		(XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_DIS_MASK |
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_SYN_LK_MASK)) != 0x0U) {

		Status = ((u32)XNVM_EFUSE_ERR_FUSE_PROTECTED |
				(u32)XNVM_EFUSE_ERR_WRITE_PUF_HELPER_DATA);
		goto END;
	}

	Status = XNvm_EfuseIsPufHelperDataEmpty();
	if (Status != (u32)XST_SUCCESS) {
		Status = (Status |
				(u32)XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
	}

	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
					XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (PufHelperData->PrgmPufHelperData == TRUE) {

		Status = XNvm_EfuseWritePufSynData(PufHelperData->EfuseSynData);
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_PUF_SYN_DATA);
			goto END;
		}

		Status = XNvm_EfuseWritePufChash(PufHelperData->Chash);
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_PUF_CHASH);
			goto END;
		}

		Status = XNvm_EfuseWritePufAux(PufHelperData->Aux);
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_PUF_AUX);
		}
	}

	/* Programming Puf SecCtrl bits */
	Status = XNvm_EfuseWritePufSecCtrl(
			&(PufHelperData->PufSecCtrlBits));
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseCacheLoad();
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfusePrgmProtectionEfuse();
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
 * This function is used to read the Puf eFUSE secure control bits from cache.
 *
 * @param	PufSecCtrlBits		Pointer to the Xnvm_PufSecCtrlBits
 *					which holds the read secure control bits.
 *
 * @return
 * 		- XST_SUCCESS - On Successful read
 *		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter
 *
 *******************************************************************************/
u32 XNvm_EfuseReadPufSecCtrlBits(XNvm_EfusePufSecCtrlBits *PufSecCtrlBits)
{
	u32 Status = (u32)XST_FAILURE;
	u32 PufEccCtrlReg = 0U;
	u32 PufSecurityCtrlReg = 0U;

	if (PufSecCtrlBits == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	Status = XNvm_EfuseReadCache(XNVM_EFUSE_PUF_AUX_ROW, &PufEccCtrlReg);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}
	Status = XNvm_EfuseReadCache(XNVM_EFUSE_SECURITY_CONTROL_ROW,
					&PufSecurityCtrlReg);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}
	PufSecCtrlBits->PufRegenDis =
		(u8)((PufEccCtrlReg &
		XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_REGEN_DIS_MASK) >>
		XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_PUF_REGEN_DIS_SHIFT);
	PufSecCtrlBits->PufHdInvalid =
		(u8)((PufEccCtrlReg &
		XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_HD_INVLD_MASK) >>
		XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_PUF_HD_INVLD_SHIFT);
	PufSecCtrlBits->PufTest2Dis =
		(u8)((PufSecurityCtrlReg &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_TEST2_DIS_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_TEST2_DIS_SHIFT);
	PufSecCtrlBits->PufDis =
		(u8)((PufSecurityCtrlReg &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_DIS_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_DIS_SHIFT);
	PufSecCtrlBits->PufSynLk =
		(u8)((PufSecurityCtrlReg &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_SYN_LK_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_SYN_LK_SHIFT);
END:
	return Status;
}

/******************************************************************************/
/**
 * This function is used to read the Misc eFUSE control bits from cache.
 *
 * @param	ReadMiscCtrlBits	Pointer to the Xnvm_MiscCtrlBits
 *					which holds the read secure control bits.
 *
 * @return
 * 		- XST_SUCCESS - On Successful read.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *
 ******************************************************************************/
u32 XNvm_EfuseReadMiscCtrlBits(XNvm_EfuseMiscCtrlBits *MiscCtrlBits)
{
	u32 Status = (u32)XST_FAILURE;
	u32 ReadReg = 0U;

	if (MiscCtrlBits == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	Status = XNvm_EfuseReadCache(XNVM_EFUSE_MISC_CTRL_ROW, &ReadReg);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}
	MiscCtrlBits->Ppk0Invalid =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_MISC_CTRL_PPK0_INVLD_1_0_MASK) >>
		XNVM_EFUSE_CACHE_MISC_CTRL_PPK0_INVLD_1_0_SHIFT);
	MiscCtrlBits->Ppk1Invalid =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_MISC_CTRL_PPK1_INVLD_1_0_MASK) >>
		XNVM_EFUSE_CACHE_MISC_CTRL_PPK1_INVLD_1_0_SHIFT);
	MiscCtrlBits->Ppk2Invalid =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_MISC_CTRL_PPK2_INVLD_1_0_MASK) >>
		XNVM_EFUSE_CACHE_MISC_CTRL_PPK2_INVLD_1_0_SHIFT);
END:
	return Status;
}

/******************************************************************************/
/**
 * This function programs the eFUSEs with the IV.
 *
 * @param	EfuseIv		Pointer to the XNvm_Iv.
 *
 * @return
 *	- XST_SUCCESS - On Successful Write.
 *	- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 * 	- XNVM_EFUSE_ERR_WRITE_META_HEADER_IV - Error while writing Meta Iv
 *	- XNVM_EFUSE_ERR_WRITE_BLK_OBFUS_IV - Error while writing BlkObfus IV
 *	- XNVM_EFUSE_ERR_WRITE_PLM IV - Error while writing PLM IV
 * 	- XNVM_EFUSE_ERR_WRITE_DATA_PARTITION_IV - Error while writing Data
 *						Partition IV.
 ******************************************************************************/
u32 XNvm_EfuseWriteIVs(XNvm_EfuseIvs *EfuseIv)
{
	u32 Status = (u32)XST_FAILURE;
	XNvm_EfuseData WriteIvs = {NULL};

	if (EfuseIv == NULL) {
		Status = XNVM_EFUSE_ERR_NTHG_TO_BE_PROGRAMMED;
		goto END;
	}

	WriteIvs.Ivs = EfuseIv;

	Status = XNvm_EfuseWrite(&WriteIvs);
END :
	return Status;
}

/******************************************************************************/
/**
 * This function is used to read IV eFUSE bits from cache
 *
 * @param	EfuseIv		Pointer to the Xnvm_Iv
 *
 * @return
 * 	- XST_SUCCESS - On Successful read.
 *  	- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *	- XNVM_EFUSE_ERR_RD_META_HEADER_IV - Error while reading Meta IV
 *	- XNVM_EFUSE_ERR_RD_BLACK_OBFUS_IV - Error while reading BlkObfus IV
 *	- XNVM_EFUSE_ERR_RD_PLM_IV - Error while reading PLM IV
 *	- XNVM_EFUSE_ERR_RD_DATA_PARTITION_IV - Error while reading Data
 *						Partition IV
 *
 ******************************************************************************/
u32 XNvm_EfuseReadIv(XNvm_Iv *EfuseIv, XNvm_IvType IvType)
{
	u32 Status = (u32)XST_FAILURE;

	if (EfuseIv == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	if ((IvType != XNVM_EFUSE_BLACK_OBFUS_IV_TYPE) &&
		(IvType != XNVM_EFUSE_PLM_IV_TYPE) &&
		(IvType != XNVM_EFUSE_DATA_PARTITION_IV_TYPE) &&
		(IvType != XNVM_EFUSE_META_HEADER_IV_TYPE)) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (IvType == XNVM_EFUSE_META_HEADER_IV_TYPE) {
		Status = XNvm_EfuseReadCacheRange(
				XNVM_EFUSE_META_HEADER_IV_START_ROW,
				XNVM_EFUSE_IV_NUM_OF_ROWS,
				EfuseIv->Iv);
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
					EfuseIv->Iv);
		if (Status != XST_SUCCESS) {
			Status = (Status |
					(u32)XNVM_EFUSE_ERR_RD_BLACK_OBFUS_IV);
			goto END;
		}
	}
	else if (IvType == XNVM_EFUSE_PLM_IV_TYPE) {
		Status = XNvm_EfuseReadCacheRange(XNVM_EFUSE_PLM_IV_START_ROW,
						XNVM_EFUSE_IV_NUM_OF_ROWS,
						EfuseIv->Iv);
		if (Status != XST_SUCCESS) {
			Status = (Status | (u32)XNVM_EFUSE_ERR_RD_PLM_IV);
			goto END;
		}
	}
	else if (IvType == XNVM_EFUSE_DATA_PARTITION_IV_TYPE) {
		Status = XNvm_EfuseReadCacheRange(
					XNVM_EFUSE_DATA_PARTITION_IV_START_ROW,
					XNVM_EFUSE_IV_NUM_OF_ROWS,
					EfuseIv->Iv);
		if (Status != XST_SUCCESS) {
		Status = (Status | (u32)XNVM_EFUSE_ERR_RD_DATA_PARTITION_IV);
			goto END;
		}
	}
	Status = (u32)XST_SUCCESS;
END:
	return Status;
}

/******************************************************************************/
/**
 * This function reads the PUF helper data from eFUSE cache.
 *
 * @param	PufHelperData		Pointer to Xnvm_PufHelperData which hold
 * 					the PUF helper data read from eFUSEs.
 *
 * @return
 *	- XST_SUCCESS -  On successful read.
 *	- XNVM_EFUSE_ERR_RD_PUF_SYN_DATA - Error while reading SYN_DATA.
 *	- XNVM_EFUSE_ERR_RD_PUF_CHASH - Error while reading CHASH.
 *	- XNVM_EFUSE_ERR_RD_PUF_AUX - Error while reading AUX.
 *
 ******************************************************************************/
u32 XNvm_EfuseReadPuf(XNvm_EfusePufHd *PufHelperData)
{
	u32 Status = (u32)XST_FAILURE;

	if (PufHelperData == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfuseReadCacheRange(XNVM_EFUSE_PUF_SYN_START_ROW,
					XNVM_EFUSE_PUF_SYN_DATA_NUM_OF_ROWS,
					PufHelperData->EfuseSynData);
	if (Status != XST_SUCCESS) {
		Status = (Status | (u32)XNVM_EFUSE_ERR_RD_PUF_SYN_DATA);
		goto END;
	}
	Status = XNvm_EfuseReadCache(XNVM_EFUSE_PUF_CHASH_ROW,
					&(PufHelperData->Chash));
	if (Status != XST_SUCCESS) {
		Status = (Status | (u32)XNVM_EFUSE_ERR_RD_PUF_CHASH);
		goto END;
	}
	Status = XNvm_EfuseReadCache(XNVM_EFUSE_PUF_AUX_ROW,
				&(PufHelperData->Aux));
	if (Status != XST_SUCCESS) {
		Status = (Status | (u32)XNVM_EFUSE_ERR_RD_PUF_AUX);
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * This function is used to read Dna eFUSE bits from cache
 *
 * @param	EfuseDna	Pointer to the Xnvm_Dna
 *
 * @return
 * 		- XST_SUCCESS - On Successful read.

 *		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid parameter.
 *
 ******************************************************************************/
u32 XNvm_EfuseReadDna(XNvm_Dna *EfuseDna)
{
	u32 Status = (u32)XST_FAILURE;

	if (EfuseDna == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfuseReadCacheRange(XNVM_EFUSE_DNA_START_ROW,
					XNVM_EFUSE_DNA_NUM_OF_ROWS,
					EfuseDna->Dna);
	if (Status != XST_SUCCESS) {
		Status = (Status | (u32)XNVM_EFUSE_ERR_RD_DNA);
		goto END;
	}
END:
	return Status;
}

/******************************************************************************/
/**
 * This function is used to read DecEfuseOnly eFUSE bits from cache
 *
 * @param	DecOnly		Pointer to the DecOnly efuse data
 *
 * @return
 * 		- XST_SUCCESS - On Successful read
 *
 *		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *
 ******************************************************************************/
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

/******************************************************************************/
/**
 * This function reads the Ppk Hash from eFUSE cache.
 *
 * @param	EfusePpk	Pointer to Xnvm_PpkHash which hold
 * 				the PpkHash from eFUSE Cache.
 *
 * @param 	PpkType		Type of the Ppk to be programmed
 *
 * @return
 *		- XST_SUCCESS - On Successful read.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 * 		- XNVM_EFUSE_ERR_RD_PPK_HASH - Error while reading PPK Hash.
 ******************************************************************************/
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
					EfusePpk->Hash);
	if (Status != XST_SUCCESS) {
		Status = (Status | (u32)XNVM_EFUSE_ERR_RD_PPK_HASH);
		goto END;
	}
END:
	return Status;
}

/******************************************************************************/
/**
 * This function revokes the Ppk.
 *
 * @param	PpkRevoke	Xnvm_RevokePpkFlags that tells
 * 				which Ppk to revoke.
 *
 * @return
 *		- XST_SUCCESS - On Successful Revocation.
 *		- Errorcode - On failure.
 ******************************************************************************/
u32 XNvm_EfuseRevokePpk(XNvm_PpkType PpkRevoke)
{
	u32 Status = (u32)XST_FAILURE;
	XNvm_EfuseData EfuseData = {NULL};
	XNvm_EfuseMiscCtrlBits MiscCtrlBits = {0U};

	if (PpkRevoke == XNVM_EFUSE_PPK0) {
		MiscCtrlBits.Ppk0Invalid = TRUE;
	}
	else if (PpkRevoke == XNVM_EFUSE_PPK1) {
		MiscCtrlBits.Ppk1Invalid = TRUE;
	}
	else if (PpkRevoke == XNVM_EFUSE_PPK2) {
		MiscCtrlBits.Ppk2Invalid = TRUE;
	}

	EfuseData.MiscCtrlBits = &MiscCtrlBits;

	Status = XNvm_EfuseWrite(&EfuseData);

	return Status;
}

/******************************************************************************/
/**
 * This function writes Revocation eFuses.
 *
 * @param	RevokeId	RevokeId number to program Revocation Id eFuses.
 *
 * Example: If the revoke id to program is 64, it will program the 0th bit of
 * the REVOCATION_ID_2 eFuse row
 *
 * @return
 *		- XST_SUCCESS - On successful write to eFuse.
 *		- Errorcode - On failure.
 ******************************************************************************/
u32 XNvm_EfuseWriteRevocationId(u32 RevokeId)
{
	u32 Status = (u32)XST_FAILURE;
	u32 RevokeIdRow;
	u32 RevokeIdBit;
	XNvm_EfuseRevokeIds WriteRevokeId = {0U};
	XNvm_EfuseData EfuseData = {NULL};

	if (RevokeId > 255U) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
                goto END;
	}

	RevokeIdRow = (RevokeId / XNVM_EFUSE_MAX_BITS_IN_ROW);
	RevokeIdBit = (RevokeId % XNVM_EFUSE_MAX_BITS_IN_ROW);

	WriteRevokeId.RevokeId[RevokeIdRow] = 1U << RevokeIdBit;

	switch(RevokeIdRow) {
	case XNVM_EFUSE_REVOCATION_ID_0:
		WriteRevokeId.PrgmRevokeId0 = TRUE;
		break;
	case XNVM_EFUSE_REVOCATION_ID_1:
		WriteRevokeId.PrgmRevokeId1 = TRUE;
		break;
	case XNVM_EFUSE_REVOCATION_ID_2:
		WriteRevokeId.PrgmRevokeId2 = TRUE;
		break;
	case XNVM_EFUSE_REVOCATION_ID_3:
		WriteRevokeId.PrgmRevokeId3 = TRUE;
		break;
	case XNVM_EFUSE_REVOCATION_ID_4:
		WriteRevokeId.PrgmRevokeId4 = TRUE;
		break;
	case XNVM_EFUSE_REVOCATION_ID_5:
		WriteRevokeId.PrgmRevokeId5 = TRUE;
		break;
	case XNVM_EFUSE_REVOCATION_ID_6:
		WriteRevokeId.PrgmRevokeId6 = TRUE;
		break;
	case XNVM_EFUSE_REVOCATION_ID_7:
		WriteRevokeId.PrgmRevokeId7 = TRUE;
		break;
	}

	EfuseData.RevokeIds = &WriteRevokeId;

	Status = XNvm_EfuseWrite(&EfuseData);
END:
	return Status;
}

/******************************************************************************/
/**
 * This function reads the Revocation Fuse from eFUSE cache.
 *
 * @param	RevokeFusePtr	Pointer to the data which hold
 * 				the Revocation IDs from eFUSE Cache.
 *
 * @param 	RevokeFuseNum	Revocation ID fuse number to read
 *
 * @return
 *		- XST_SUCCESS  - On uccessfull read.
 *		- Errorcode - On failure.
 ******************************************************************************/
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

/******************************************************************************/
/**
 * This function reads User eFuses from Cache.
 *
 *@param	UserFuseData 	UserFuseData to be read from eFuse Cache.
 *
 *@return
 *	- XST_SUCCESS - if reads successfully.
 *	- XNVM_EFUSE_ERR_RD_USER_FUSES - Error in reading User fuses.
 ******************************************************************************/
u32 XNvm_EfuseReadUserFuses(XNvm_EfuseUserData *UserFusesData)
{
	u32 Status = XST_FAILURE;
	u32 Row;

	if (UserFusesData == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Row = XNVM_EFUSE_USER_FUSE_START_ROW + (
		UserFusesData->StartUserFuseNum - 1U);
	Status = XNvm_EfuseReadCacheRange(Row, UserFusesData->NumOfUserFuses,
						UserFusesData->UserFuseData);
	if (Status != XST_SUCCESS) {
		Status = (Status | (u32)XNVM_EFUSE_ERR_RD_USER_FUSES);
	}
END:
	return Status;
}

/******************************************************************************/
/*
 * This function Programs User eFuses.
 *
 * @param	WriteUserFuses	Pointer to the XNvm_UserEfuseData structure
 *
 * @return
 *		- XST_SUCCESS - if programming is successful.
 *		- XNVM_EFUSE_ERR_NTHG_TO_BE_PROGRAMMED - if NULL request is sent
 *
 ******************************************************************************/
u32 XNvm_EfuseWriteUserFuses(XNvm_EfuseUserData *WriteUserFuses)
{
	u32 Status = (u32)XST_FAILURE;
	XNvm_EfuseData UserFusesData = {NULL};

	if (WriteUserFuses == NULL) {
		Status = XNVM_EFUSE_ERR_NTHG_TO_BE_PROGRAMMED;
		goto END;
	}

	UserFusesData.UserFuses = WriteUserFuses;

	Status = XNvm_EfuseWrite(&UserFusesData);
END :
	return Status;
}

/*****************************************************************************/
/**
 *
 * This function programs secure control bits specified by user.
 *
 * @param   SecCtrl  Pointer to the XNvm_EfuseSecCtrlBits structure
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
static u32 XNvm_EfuseWriteSecCtrl(XNvm_EfuseSecCtrlBits *SecCtrl)
{
	u32 Status = (u32)XST_FAILURE;
	XNvm_EfuseType EfuseType = XNVM_EFUSE_PAGE_0;
	u32 Row = XNVM_EFUSE_SECURITY_CONTROL_ROW;
	u32 RowDataVal = 0U;

	if (SecCtrl == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if ((SecCtrl->Ppk0WrLk != FALSE) ||
		(SecCtrl->Ppk1WrLk != FALSE) ||
		(SecCtrl->Ppk2WrLk != FALSE) ||
		(SecCtrl->AesCrcLk != FALSE) ||
		(SecCtrl->AesWrLk != FALSE) ||
		(SecCtrl->UserKey0CrcLk != FALSE) ||
		(SecCtrl->UserKey0WrLk != FALSE) ||
		(SecCtrl->UserKey1CrcLk != FALSE) ||
		(SecCtrl->UserKey1WrLk != FALSE) ||
		(SecCtrl->SecDbgDis != FALSE) ||
		(SecCtrl->SecLockDbgDis != FALSE) ||
		(SecCtrl->BootEnvWrLk != FALSE) ||
		(SecCtrl->RegInitDis != FALSE)) {

		Status = XNvm_EfuseReadCache(Row, &RowDataVal);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
	}
	else {
		Status = (u32)XST_SUCCESS;
		goto END;
	}

	if ((SecCtrl->Ppk0WrLk != 0x00U) &&
		((RowDataVal &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK0_WR_LK_MASK) == 0U)) {
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
			XNVM_EFUSE_SEC_PPK0_WRLK);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_PPK0_WR_LK);
			goto END;
		}
	}
	if ((SecCtrl->Ppk1WrLk != 0x00U) &&
		((RowDataVal &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK1_WR_LK_MASK) == 0U)) {
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
			XNVM_EFUSE_SEC_PPK1_WRLK);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_PPK1_WR_LK);
			goto END;
		}
	}
	if ((SecCtrl->Ppk2WrLk != 0x00U) &&
		((RowDataVal &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK2_WR_LK_MASK) == 0U)) {
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
					XNVM_EFUSE_SEC_PPK2_WRLK);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_PPK2_WR_LK);
			goto END;
		}
	}
	if ((SecCtrl->AesCrcLk != 0x00U) &&
		((RowDataVal &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_AES_CRC_LK_1_0_MASK) ==
		0x00U)) {
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
				XNVM_EFUSE_SEC_AES_CRC_LK_BIT_0);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_AES_CRC_LK_BIT_0);
			goto END;
		}
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
				XNVM_EFUSE_SEC_AES_CRC_LK_BIT_1);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_AES_CRC_LK_BIT_1);
			goto END;
		}
	}
	if ((SecCtrl->AesWrLk != 0x00U) &&
		((RowDataVal &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_AES_WR_LK_MASK) ==
		0x00U)) {
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
					XNVM_EFUSE_SEC_AES_WRLK);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_AES_WR_LK);
			goto END;
		}
	}
	if ((SecCtrl->UserKey0CrcLk != 0x00U) &&
		((RowDataVal &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_0_CRC_LK_MASK) ==
		0x00U)) {
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
					XNVM_EFUSE_SEC_USER_KEY0_CRC_LK);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_USER_KEY0_CRC_LK);
			goto END;
		}
	}
	if ((SecCtrl->UserKey0WrLk != 0x00U) &&
		((RowDataVal &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_0_WR_LK_MASK) ==
		0x00U)) {
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
			XNVM_EFUSE_SEC_USER_KEY0_WRLK);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_USER_KEY0_WR_LK);
			goto END;
		}

	}
	if ((SecCtrl->UserKey1CrcLk != 0x00U) &&
		((RowDataVal &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_1_CRC_LK_MASK) ==
		0x00U)) {
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
			XNVM_EFUSE_SEC_USER_KEY1_CRC_LK);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_USER_KEY1_CRC_LK);
			goto END;
		}
	}
	if ((SecCtrl->UserKey1WrLk != 0x00U) &&
		((RowDataVal &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_1_WR_LK_MASK) ==
		0x00U)) {
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
			XNVM_EFUSE_SEC_USER_KEY1_WRLK);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_USER_KEY1_WR_LK);
			goto END;
		}

	}
	if ((SecCtrl->SecDbgDis != 0x00U) &&
		((RowDataVal &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_SEC_DEBUG_DIS_MASK) ==
		0x00U)) {
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
				XNVM_EFUSE_SEC_SECDBG_DIS_BIT_0);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_SECDBG_DIS_BIT_0);
			goto END;
		}
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
				XNVM_EFUSE_SEC_SECDBG_DIS_BIT_1);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_SECDBG_DIS_BIT_1);
			goto END;
		}
	}
	if ((SecCtrl->SecLockDbgDis != 0x00U) &&
		((RowDataVal &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_SEC_LOCK_DBG_DIS_MASK) ==
		0x00U)) {
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
				XNVM_EFUSE_SEC_SECLOCKDBG_DIS_BIT_0);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_SECLOCKDBG_DIS_BIT_0);
			goto END;
		}
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
				XNVM_EFUSE_SEC_SECLOCKDBG_DIS_BIT_1);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_SECLOCKDBG_DIS_BIT_1);
			goto END;
		}
	}
	if ((SecCtrl->BootEnvWrLk != 0x00U) &&
		((RowDataVal &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_BOOT_ENV_WR_LK_MASK) ==
		0x00U)) {
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
			XNVM_EFUSE_SEC_BOOTENV_WRLK);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
			(u32)XNVM_EFUSE_ERR_WRTIE_BOOTENV_WR_LK);
			goto END;
		}
	}
	if ((SecCtrl->RegInitDis != 0x00U) &&
		((RowDataVal &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_REG_INIT_DIS_1_0_MASK) ==
		0x00U)) {
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
				XNVM_EFUSE_SEC_REG_INIT_DIS_BIT_0);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_REG_INIT_DIS_BIT_0);
			goto END;
		}
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
				XNVM_EFUSE_SEC_REG_INIT_DIS_BIT_1);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRTIE_REG_INIT_DIS_BIT_1);
			goto END;
		}
	}

	Status = (u32)XST_SUCCESS;
END:
	return Status;
}

/******************************************************************************/
/**
 * This function programs Puf control bits specified by user.
 *
 * @param	PrgmPufSecCtrlBits	Instance of XNvm_EfusePufSecCtrlBits.
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
static u32 XNvm_EfuseWritePufSecCtrl(XNvm_EfusePufSecCtrlBits *PufSecCtrlBits)
{
	u32 Status = (u32)XST_FAILURE;
	XNvm_EfuseType EfuseType = XNVM_EFUSE_PAGE_0;
	u32 Row = XNVM_EFUSE_PUF_AUX_ROW;
	u32 RowDataVal = 0U;

	if (PufSecCtrlBits == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if ((PufSecCtrlBits->PufRegenDis != FALSE) ||
		(PufSecCtrlBits->PufHdInvalid != FALSE)) {

		Status = XNvm_EfuseReadCache(Row, &RowDataVal);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
	}

	if ((PufSecCtrlBits->PufRegenDis == TRUE) &&
		((RowDataVal &
		XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_REGEN_DIS_MASK) == 0x00U)) {
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
			XNVM_EFUSE_PUF_ECC_PUF_CTRL_REGEN_DIS_COLUMN);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRITE_PUF_REGEN_DIS);
			goto END;
		}
	}
	if ((PufSecCtrlBits->PufHdInvalid == TRUE) &&
		((RowDataVal &
		XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_HD_INVLD_MASK) == 0x00U)) {
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
			XNVM_EFUSE_PUF_ECC_PUF_CTRL_HD_INVLD_COLUMN);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRITE_PUF_HD_INVLD);
			goto END;
		}
	}

	RowDataVal = 0U;
	Row = XNVM_EFUSE_SECURITY_CONTROL_ROW;

	if ((PufSecCtrlBits->PufTest2Dis != FALSE) ||
		(PufSecCtrlBits->PufDis != FALSE) ||
		(PufSecCtrlBits->PufSynLk != FALSE)) {

		Status = XNvm_EfuseReadCache(Row, &RowDataVal);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}

	}

	if ((PufSecCtrlBits->PufSynLk != 0x00U) &&
		((RowDataVal &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_SYN_LK_MASK) ==
		0x00U)) {
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
					XNVM_EFUSE_SEC_PUF_SYN_LK);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRITE_PUF_SYN_LK);
			goto END;
		}
	}

	if ((PufSecCtrlBits->PufTest2Dis != 0x00U) &&
		((RowDataVal &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_TEST2_DIS_MASK) ==
		0x00U)) {
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
				XNVM_EFUSE_SEC_PUF_TEST2_DIS);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status |
				(u32)XNVM_EFUSE_ERR_WRITE_PUF_TEST2_DIS);
			goto END;
		}
	}

	if ((PufSecCtrlBits->PufDis != 0x00U) &&
		((RowDataVal &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_DIS_MASK) == 0x00U)) {
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
	return Status;
}

/******************************************************************************/
/**
 * This function program Glitch Configuration
 *
 * @param	WriteGlitchCfg	Pointer to glitch configuration data
 *
 * @return
 *		- XST_SUCCESS - On Success
 *		- XST_FAILURE - Failure in programming
 *
 ******************************************************************************/
static u32 XNvm_EfusePrgmGlitchCfgValues(XNvm_EfuseGlitchCfgBits *WriteGlitchCfg)
{
	u32 Status = (u32)XST_FAILURE;
	u32 PrgmGlitchCfg = 0U;

	if (WriteGlitchCfg == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (WriteGlitchCfg->PrgmGlitch == TRUE) {
		Status = XNvm_EfuseComputeProgrammableBits(
				&WriteGlitchCfg->GlitchDetTrim,
				&PrgmGlitchCfg,
				XNM_EFUSE_GLITCH_ANLG_TRIM_3,
				(XNM_EFUSE_GLITCH_ANLG_TRIM_3 +
				XNVM_EFUSE_GLITCH_NUM_OF_ROWS));
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Status = XNvm_EfusePgmAndVerifyRows(
				XNM_EFUSE_GLITCH_ANLG_TRIM_3,
				XNVM_EFUSE_GLITCH_NUM_OF_ROWS,
				XNVM_EFUSE_PAGE_0,
				&PrgmGlitchCfg);
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_GLITCH_CFG);
			goto END;
		}
	}
	else {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * This function programs Glitch Write lock bit
 *
 * @param	WriteGlitchCfg	Pointer to glitch configuration data
 *
 * @return
 *		- XST_SUCCESS - On Success
 *		- XST_FAILURE - Failure in programming
 *
 ******************************************************************************/
static u32 XNvm_EfusePrgmGlitchWriteLock(XNvm_EfuseGlitchCfgBits *WriteGlitchCfg)
{
	u32 Status = (u32)XST_FAILURE;

	if (WriteGlitchCfg == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if ((WriteGlitchCfg->PrgmGlitch == TRUE)  &&
			(WriteGlitchCfg->GlitchDetWrLk == TRUE)) {
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNM_EFUSE_GLITCH_ANLG_TRIM_3,
				XNVM_EFUSE_GLITCH_WRLK_COLUMN);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_GLITCH_WRLK);
			goto END;
		}
	}
	else {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * This function enables the halt boot by ROM upon glitch detection.
 *
 * @param	WriteGlitchCfg	Pointer to glitch configuration data
 *
 * @return
 *		- XST_SUCCESS - On Success
 *		- XST_FAILURE - Failure in programming
 ******************************************************************************/
static u32 XNvm_EfusePrgmGdRomHaltBootEn(XNvm_EfuseGlitchCfgBits *WriteGlitchCfg)
{
	u32 Status = (u32)XST_FAILURE;

	if (WriteGlitchCfg == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if ((WriteGlitchCfg->PrgmGlitch == TRUE)  &&
			(WriteGlitchCfg->GdHaltBootEn == TRUE)) {
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
			XNVM_EFUSE_MISC_CTRL_ROW,
			XNVM_EFUSE_GLITCH_HALT_EN_0_COLUMN);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_GD_ROM_BITS);
			goto END;
		}

		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_MISC_CTRL_ROW,
				XNVM_EFUSE_GLITCH_HALT_EN_1_COLUMN);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_GD_ROM_BITS);
			goto END;
		}
	}
	else {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * This function enables the glitch detection monitoring by ROM.
 *
 * @param	WriteGlitchCfg	Pointer to glitch configuration data
 *
 * @return
 *		- XST_SUCCESS - On Success
 *		- XST_FAILURE - Failure in programming
 ******************************************************************************/
static u32 XNvm_EfusePrgmGdRomMonEn(XNvm_EfuseGlitchCfgBits *WriteGlitchCfg)
{
	u32 Status = (u32)XST_FAILURE;

	if (WriteGlitchCfg == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if ((WriteGlitchCfg->PrgmGlitch == TRUE) &&
			(WriteGlitchCfg->GdRomMonitorEn == TRUE)) {
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
			XNVM_EFUSE_MISC_CTRL_ROW,
			XNVM_EFUSE_GLITCH_ROM_EN_COLUMN);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_GD_ROM_BITS);
			goto END;
		}
	}
	else {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
	}

END:
	return Status;
}

/******************************************************************************/
/*
 * This function is used to program below eFuses
 * AES key
 * User key 0
 * User key 1
 *
 * @param	Keys	Pointer to the XNvm_EfuseAesKeys.
 *
 * @return
 *	- XST_SUCCESS - Specified data read
 *	- XNVM_EFUSE_ERR_BEFORE_PROGRAMMING - Error before programming eFuse
 *	- XNVM_EFUSE_ERR_WRITE_AES_KEY - Error while writing AES key
 *	- XNVM_EFUSE_ERR_WRITE_USER_KEY0 - Error while writing User key 0
 * 	- XNVM_EFUSE_ERR_WRITE_USER_KEY1 - Error while writing User key 1
 * 	- XST_FAILURE - Unexpected error
 *
 ******************************************************************************/
static u32 XNvm_EfusePrgmAesKeys(XNvm_EfuseAesKeys *Keys)
{
	u32 Status = (u32)XST_FAILURE;
	u32 Crc;

	if (Keys == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (Keys->PrgmAesKey == TRUE) {
		Status = XNvm_EfusePgmAndVerifyRows(
				XNVM_EFUSE_AES_KEY_START_ROW,
				XNVM_EFUSE_AES_KEY_NUM_OF_ROWS,
				XNVM_EFUSE_PAGE_0, Keys->AesKey);
		if (Status != XST_SUCCESS) {
			Status = (Status | (u32)XNVM_EFUSE_ERR_WRITE_AES_KEY);
			goto END;
		}
		Status = XNvm_EfuseCacheLoad();
		if (Status != XST_SUCCESS) {
			Status = (Status | (u32)XNVM_EFUSE_ERR_WRITE_AES_KEY);
			goto END;
		}
		Crc = XNvm_AesCrcCalc(Keys->AesKey);

		Status = XNvm_EfuseCheckAesKeyCrc(Crc);
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_AES_KEY);
			goto END;
		}
	}
	if (Keys->PrgmUserKey0 == TRUE) {
		Status = XNvm_EfusePgmAndVerifyRows(
				XNVM_EFUSE_USER_KEY_0_START_ROW,
				XNVM_EFUSE_USER_KEY_NUM_OF_ROWS,
				XNVM_EFUSE_PAGE_0, Keys->UserKey0);
		 if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_USER_KEY0);
			goto END;
		}
		Status = XNvm_EfuseCacheLoad();
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_USER_KEY0);
			goto END;
		}
		Crc = XNvm_AesCrcCalc(Keys->UserKey0);

		Status = XNvm_EfuseCheckAesUserKey0Crc(Crc);
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_USER_KEY0);
			goto END;
		}
	}
	if (Keys->PrgmUserKey1 == TRUE) {
		Status = XNvm_EfusePgmAndVerifyRows(
					XNVM_EFUSE_USER_KEY_1_START_ROW,
					XNVM_EFUSE_USER_KEY_NUM_OF_ROWS,
					XNVM_EFUSE_PAGE_0,
					Keys->UserKey1);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_USER_KEY1);
		goto END;
		}
		Status = XNvm_EfuseCacheLoad();
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_USER_KEY1);
			goto END;
		}
		Crc = XNvm_AesCrcCalc(Keys->UserKey1);

		Status = XNvm_EfuseCheckAesUserKey1Crc(Crc);
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_USER_KEY1);
			goto END;
		}
	}
	Status = (u32)XST_SUCCESS;
END:
	return Status;
}

/******************************************************************************/
/*
 * This function is used to program below eFuses
 * PPK0/PPK1/PPK2 hash
 *
 * @param	Hash	Pointer to the XNvm_EfusePpkHash.
 *
 * @return
 *	- XST_SUCCESS - Specified data read
 *	- XNVM_EFUSE_ERR_WRITE_PPK0_HASH - Error while writing PPK0 Hash
 *	- XNVM_EFUSE_ERR_WRITE_PPK1_HASH - Error while writing PPK1 Hash
 * 	- XNVM_EFUSE_ERR_WRITE_PPK2_HASH - Error while writing PPK2 Hash
 * 	- XST_FAILURE - Unexpected error
 *
 ******************************************************************************/
static u32 XNvm_EfusePrgmPpkHash(XNvm_EfusePpkHash *Hash)
{
	u32 Status = (u32)XST_FAILURE;

	if (Hash == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (Hash->PrgmPpk0Hash == TRUE) {
		Status = XNvm_EfusePgmAndVerifyRows(
				XNVM_EFUSE_PPK_0_HASH_START_ROW,
				XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS,
				XNVM_EFUSE_PAGE_0,
				Hash->Ppk0Hash);
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_PPK0_HASH);
			goto END;
		}
	}
	if (Hash->PrgmPpk1Hash == TRUE) {
		Status = XNvm_EfusePgmAndVerifyRows(
				XNVM_EFUSE_PPK_1_HASH_START_ROW,
				XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS,
				XNVM_EFUSE_PAGE_0,
				Hash->Ppk1Hash);
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_PPK1_HASH);
			goto END;
		}
	}
	if (Hash->PrgmPpk2Hash == TRUE) {
		Status = XNvm_EfusePgmAndVerifyRows(
				XNVM_EFUSE_PPK_2_HASH_START_ROW,
				XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS,
				XNVM_EFUSE_PAGE_0,
				Hash->Ppk2Hash);
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_PPK2_HASH);
			goto END;
		}
	}
	Status = (u32)XST_SUCCESS;
END:
	return Status;
}

/******************************************************************************/
/*
 * This function is used to program below DEC_ONLY fuses.
 *
 * @param	DecOnly		Pointer to XNvm_EfuseDecOnly structure
 *
 * @return
 *	- XST_SUCCESS - Specified data read
 *	- XNVM_EFUSE_ERR_WRITE_DEC_EFUSE_ONLY - Error while writing
 *						DEC_ONLY eFuse.
 ******************************************************************************/
static u32 XNvm_EfusePrgmDecOnly(XNvm_EfuseDecOnly *DecOnly)
{
	u32 Status = (u32)XST_FAILURE;
	u32 PrgmDecOnly = 0U;

	if (DecOnly == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	if (DecOnly->PrgmDecOnly == TRUE) {
		Status = XNvm_EfuseComputeProgrammableBits(
					&DecOnly->DecEfuseOnly,
					&PrgmDecOnly,
					XNVM_EFUSE_DEC_EFUSE_ONLY_ROW,
					(XNVM_EFUSE_DEC_EFUSE_ONLY_ROW +
					XNVM_EFUSE_DEC_EFUSE_ONLY_NUM_OF_ROWS));
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XNvm_EfusePgmAndVerifyRows(
				XNVM_EFUSE_DEC_EFUSE_ONLY_ROW,
				XNVM_EFUSE_DEC_EFUSE_ONLY_NUM_OF_ROWS,
				XNVM_EFUSE_PAGE_0,
				&PrgmDecOnly);
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_DEC_EFUSE_ONLY);
			goto END;
		}
	}

	Status = (u32)XST_SUCCESS;
END:
	return Status;
}

/******************************************************************************/
/*
 * This function is used to program all IVs
 *
 * @param	EfuseIv		Pointer to the XNvm_EfuseIvs.
 *
 * @return
 *	- XST_SUCCESS - Specified data read
 *	- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 * 	- XNVM_EFUSE_ERR_WRITE_META_HEADER_IV - Error while writing Meta Iv
 *	- XNVM_EFUSE_ERR_WRITE_BLK_OBFUS_IV - Error while writing BlkObfus IV
 *	- XNVM_EFUSE_ERR_WRITE_PLM_IV - Error while writing PLM IV
 * 	- XNVM_EFUSE_ERR_WRITE_DATA_PARTITION_IV - Error while writing Data
 *						Partition IV.
 ******************************************************************************/
static u32 XNvm_EfusePrgmIVs(XNvm_EfuseIvs *Ivs)
{
	u32 Status = (u32)XST_FAILURE;
	u32 PrgmIv[XNVM_EFUSE_IV_NUM_OF_ROWS] = {0U};

	if (Ivs == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (Ivs->PrgmMetaHeaderIv == TRUE) {
		Status = XNvm_EfuseComputeProgrammableBits(Ivs->MetaHeaderIv,
					PrgmIv,
					XNVM_EFUSE_META_HEADER_IV_START_ROW,
					(XNVM_EFUSE_META_HEADER_IV_START_ROW +
					XNVM_EFUSE_IV_NUM_OF_ROWS));
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XNvm_EfusePgmAndVerifyRows(
			XNVM_EFUSE_META_HEADER_IV_START_ROW,
			XNVM_EFUSE_IV_NUM_OF_ROWS,
			XNVM_EFUSE_PAGE_0,
			PrgmIv);

		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_META_HEADER_IV);
			goto END;
		}
	}
	if (Ivs->PrgmBlkObfusIv == TRUE) {
		Status = XNvm_EfusePgmAndVerifyRows(
			XNVM_EFUSE_BLACK_OBFUS_IV_START_ROW,
			XNVM_EFUSE_IV_NUM_OF_ROWS,
			XNVM_EFUSE_PAGE_0,
			Ivs->BlkObfusIv);

		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_BLK_OBFUS_IV);
			goto END;
		}
	}
	if (Ivs->PrgmPlmIv == TRUE) {
		Status = XNvm_EfuseComputeProgrammableBits(Ivs->PlmIv,
					PrgmIv,
					XNVM_EFUSE_PLM_IV_START_ROW,
					(XNVM_EFUSE_PLM_IV_START_ROW +
					XNVM_EFUSE_IV_NUM_OF_ROWS));
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XNvm_EfusePgmAndVerifyRows(
			XNVM_EFUSE_PLM_IV_START_ROW,
			XNVM_EFUSE_IV_NUM_OF_ROWS,
			XNVM_EFUSE_PAGE_0,
			PrgmIv);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_PLM_IV);
			goto END;
		}
	}
	if (Ivs->PrgmDataPartitionIv == TRUE) {
		Status = XNvm_EfuseComputeProgrammableBits(
				Ivs->DataPartitionIv,
				PrgmIv,
				XNVM_EFUSE_DATA_PARTITION_IV_START_ROW,
				(XNVM_EFUSE_DATA_PARTITION_IV_START_ROW +
				XNVM_EFUSE_IV_NUM_OF_ROWS));
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XNvm_EfusePgmAndVerifyRows(
			XNVM_EFUSE_DATA_PARTITION_IV_START_ROW,
			XNVM_EFUSE_IV_NUM_OF_ROWS,
			XNVM_EFUSE_PAGE_0,
			PrgmIv);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_DATA_PARTITION_IV);
			goto END;
		}
	}

	Status = XST_SUCCESS;
END :
	return Status;
}

/******************************************************************************/
/*
 * This function is used to compute the eFuse bits to be programmed
 * to the eFuse.
 *
 * @param	ReqData		Pointer to the user provided eFuse data to
 * 				be written.
 *
 * @param	PrgmData	Pointer to the computed eFuse bits to be
 * 				programmed, which means that this API fills
 * 				only unprogrammed and valid bits.
 * @return
 *	- XST_SUCCESS - if the eFuse data computation is successful
 *	- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 ******************************************************************************/
static u32 XNvm_EfuseComputeProgrammableBits(u32 *ReqData, u32 *PrgmData,
						u32 StartRow, u32 EndRow)
{
	u32 Status = (u32)XST_FAILURE;
	u32 ReadReg = 0U;
	u32 NewData = 0U;
	u32 Index;
	u32 Column;
	u32 Mask;

	if ((ReqData == NULL) ||
		(PrgmData == NULL)) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	for (Index = StartRow; Index < EndRow; Index++) {
		Status = XNvm_EfuseReadCache(Index, &ReadReg);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		for (Column = 0U; Column < XNVM_EFUSE_MAX_BITS_IN_ROW;
				Column++) {

			Mask = 1U << Column;

			if (((ReqData[Index - StartRow] & Mask) == Mask) &&
				((ReadReg & Mask) == 0U)) {

				 NewData |= Mask;
			}
		}
		PrgmData[Index - StartRow] = NewData;
		NewData = 0U;
	}

	Status = (u32)XST_SUCCESS;
END:
	return Status;
}

/******************************************************************************/
/**
 * This function programs Revocation Id eFuses.
 *
 * @param	RevokeIds	Pointer to XNvm_RevokeIdEfuse that contains
 * 				Revocation Id to write.
 *
 * @return
 *	- XST_SUCCESS - On successful write of Revocation Id.
 *	- XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_0 - Error in writing revoke id 0
 *	- XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_1 - Error in writing revoke id 1
 *	- XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_2 - Error in writing revoke id 2
 *	- XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_3 - Error in writing revoke id 3
 *	- XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_4 - Error in writing revoke id 4
 *	- XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_5 - Error in writing revoke id 5
 *	- XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_6 - Error in writing revoke id 6
 *	- XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_7 - Error in writing revoke id 7
 ******************************************************************************/
static u32 XNvm_EfusePrgmRevocationIdFuses(XNvm_EfuseRevokeIds *RevokeIds)
{
	u32 Status = (u32)XST_FAILURE;
	u32 PrgmRevokeIds[XNVM_NUM_OF_REVOKE_ID_FUSES] = {0U};
	u32 Row;

	if (RevokeIds == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfuseComputeProgrammableBits(RevokeIds->RevokeId,
					PrgmRevokeIds,
					XNVM_EFUSE_REVOCATION_ID_0_ROW,
					(XNVM_EFUSE_REVOCATION_ID_0_ROW +
					XNVM_NUM_OF_REVOKE_ID_FUSES));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (RevokeIds->PrgmRevokeId0 != 0x00U) {

		Row = XNVM_EFUSE_REVOCATION_ID_0_ROW +
			XNVM_EFUSE_REVOCATION_ID_0;

		Status = XNvm_EfusePgmAndVerifyRows(Row, 0x1U,
	                XNVM_EFUSE_PAGE_0,
			&PrgmRevokeIds[XNVM_EFUSE_REVOCATION_ID_0]);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_0);
			goto END;
		}
	}
	if (RevokeIds->PrgmRevokeId1 != 0x00U) {
		Row = XNVM_EFUSE_REVOCATION_ID_0_ROW +
			XNVM_EFUSE_REVOCATION_ID_1;

		Status = XNvm_EfusePgmAndVerifyRows(Row, 0x1U,
	                XNVM_EFUSE_PAGE_0,
			&PrgmRevokeIds[XNVM_EFUSE_REVOCATION_ID_1]);
		if (Status != XST_SUCCESS) {
			Status = (Status |
			(u32)XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_1);
			goto END;
		}
	}
	if (RevokeIds->PrgmRevokeId2 != 0x00U) {
		Row = XNVM_EFUSE_REVOCATION_ID_0_ROW +
			XNVM_EFUSE_REVOCATION_ID_2;

		Status = XNvm_EfusePgmAndVerifyRows(Row, 0x1U,
	                XNVM_EFUSE_PAGE_0,
			&PrgmRevokeIds[XNVM_EFUSE_REVOCATION_ID_2]);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_2);
			goto END;
		}

	}
	if (RevokeIds->PrgmRevokeId3 != 0x00U) {
		Row = XNVM_EFUSE_REVOCATION_ID_0_ROW +
			XNVM_EFUSE_REVOCATION_ID_3;

		Status = XNvm_EfusePgmAndVerifyRows(Row, 0x1U,
	                XNVM_EFUSE_PAGE_0,
			&PrgmRevokeIds[XNVM_EFUSE_REVOCATION_ID_3]);
		if (Status != XST_SUCCESS) {
			Status = (Status |
			(u32)XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_3);
			goto END;
		}
	}
	if (RevokeIds->PrgmRevokeId4 != 0x00U) {
		Row = XNVM_EFUSE_REVOCATION_ID_0_ROW +
			XNVM_EFUSE_REVOCATION_ID_4;

		Status = XNvm_EfusePgmAndVerifyRows(Row, 0x1U,
	                XNVM_EFUSE_PAGE_0,
			&PrgmRevokeIds[XNVM_EFUSE_REVOCATION_ID_4]);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_4);
			goto END;
		}
	}
	if (RevokeIds->PrgmRevokeId5 != 0x00U) {
		Row = XNVM_EFUSE_REVOCATION_ID_0_ROW +
			XNVM_EFUSE_REVOCATION_ID_5;

		Status = XNvm_EfusePgmAndVerifyRows(Row, 0x1U,
	                XNVM_EFUSE_PAGE_0,
			&PrgmRevokeIds[XNVM_EFUSE_REVOCATION_ID_5]);
		if (Status != XST_SUCCESS) {
			Status = (Status |
			(u32)XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_5);
			goto END;
		}
	}
	if (RevokeIds->PrgmRevokeId6 != 0x00U) {
		Row = XNVM_EFUSE_REVOCATION_ID_0_ROW +
			XNVM_EFUSE_REVOCATION_ID_6;

		Status = XNvm_EfusePgmAndVerifyRows(Row, 0x1U,
	                XNVM_EFUSE_PAGE_0,
			&PrgmRevokeIds[XNVM_EFUSE_REVOCATION_ID_6]);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_6);
			goto END;
		}
	}
	if (RevokeIds->PrgmRevokeId7 != 0x00U) {
		Row = XNVM_EFUSE_REVOCATION_ID_0_ROW +
			XNVM_EFUSE_REVOCATION_ID_7;

		Status = XNvm_EfusePgmAndVerifyRows(Row, 0x1U,
	                XNVM_EFUSE_PAGE_0,
			&PrgmRevokeIds[XNVM_EFUSE_REVOCATION_ID_7]);
		if (Status != XST_SUCCESS) {
			Status = (Status |
			(u32)XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_7);
			goto END;
		}
	}

	Status = XST_SUCCESS;
END:
	return Status;

}

/******************************************************************************/
/**
 * This function revokes the Ppk.
 *
 * @param	PpkSelect	Pointer to XNvm_EfuseMiscCtrlBits struture.
 *
 * @return
 *	- XST_SUCCESS - On Successful write of Ppk revoke efuses.
 *	- XNVM_EFUSE_ERR_WRITE_PPK0_INVALID_BIT_0 - Error in writing PPK0 Invld
 *	- XNVM_EFUSE_ERR_WRITE_PPK0_INVALID_BIT_1 - Error in writing PPK0 Invld
 *	- XNVM_EFUSE_ERR_WRITE_PPK1_INVALID_BIT_0 - Error in writing PPK1 Invld
 *	- XNVM_EFUSE_ERR_WRITE_PPK1_INVALID_BIT_1 - Error in writing PPK1 Invld
 *	- XNVM_EFUSE_ERR_WRITE_PPK2_INVALID_BIT_0 - Error in writing PPK2 Invld
 *	- XNVM_EFUSE_ERR_WRITE_PPK2_INVALID_BIT_1 - Error in writing PPK2 Invld
 ******************************************************************************/
static u32 XNvm_EfusePrgmPpkRevokeFuses(XNvm_EfuseMiscCtrlBits *PpkSelect)
{
	u32 Status = (u32)XST_FAILURE;
	u32 RowData = 0U;

	if (PpkSelect == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if ((PpkSelect->Ppk0Invalid != 0x00U) ||
		(PpkSelect->Ppk1Invalid != 0x00U) ||
		(PpkSelect->Ppk2Invalid != 0x00U)) {

		Status = XNvm_EfuseReadCache(XNVM_EFUSE_MISC_CTRL_ROW,
						&RowData);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_RD_MISC_CTRL_BITS);
			goto END;
		}
	}
	else {
		Status = (u32)XST_SUCCESS;
		goto END;
	}

	if ((PpkSelect->Ppk0Invalid != 0x00U) &&
		((RowData & XNVM_EFUSE_CACHE_MISC_CTRL_PPK0_INVLD_1_0_MASK) ==
		0x00U)) {
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_MISC_CTRL_ROW,
				XNVM_EFUSE_MISC_PPK0_INVALID_BIT_0);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_PPK0_INVALID_BIT_0);
			goto END;
		}
		Status = XNvm_EfusePgmAndVerifyBit(
				XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_MISC_CTRL_ROW,
				XNVM_EFUSE_MISC_PPK0_INVALID_BIT_1);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_PPK0_INVALID_BIT_1);
			goto END;
		}
	}
	if ((PpkSelect->Ppk1Invalid != 0x00U) &&
		((RowData & XNVM_EFUSE_CACHE_MISC_CTRL_PPK1_INVLD_1_0_MASK) ==
		0x00U)) {
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_MISC_CTRL_ROW,
				XNVM_EFUSE_MISC_PPK1_INVALID_BIT_0);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_PPK1_INVALID_BIT_0);
			goto END;
		}
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_MISC_CTRL_ROW,
				XNVM_EFUSE_MISC_PPK1_INVALID_BIT_1);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_PPK1_INVALID_BIT_1);
			goto END;
		}
	}
	if ((PpkSelect->Ppk2Invalid != 0x00U) &&
		((RowData & XNVM_EFUSE_CACHE_MISC_CTRL_PPK2_INVLD_1_0_MASK) ==
		0x00U)) {
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_MISC_CTRL_ROW,
				XNVM_EFUSE_MISC_PPK2_INVALID_BIT_0);
		if (Status != XST_SUCCESS) {
			Status =
				(Status |
				 (u32)XNVM_EFUSE_ERR_WRITE_PPK2_INVALID_BIT_0);
			goto END;
		}
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

	Status = (u32)XST_SUCCESS;
END:
	return Status;
}

/******************************************************************************/
/**
 * This function Validates all IVs requested for programming.
 *
 * @param	EfuseIv		Pointer to XNvm_EfuseIvs structure.
 *
 * @return
 *		- XST_SUCCESS - if validation is successful.
 *		- XNVM_EFUSE_ERR_WRITE_META_HEADER_IV - Error in Metaheader IV
 *							write request.
 *		- XNVM_EFUSE_ERR_BLK_OBFUS_IV_ALREADY_PRGMD - Error in Blk Obfus
 *							Iv write request
 *		- XNVM_EFUSE_ERR_WRITE_PLM_IV - Error in Plm Iv write request
 *		- XNVM_EFUSE_ERR_WRITE_DATA_PARTITION_IV - Error in
 *							Data Partition Iv write
 ******************************************************************************/
static u32 XNvm_EfuseValidateIVsWriteReq(XNvm_EfuseIvs *EfuseIv)
{
	u32 Status = (u32)XST_FAILURE;

	if (EfuseIv == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (EfuseIv->PrgmMetaHeaderIv == TRUE) {
		Status = XNvm_EfuseValidateIV(EfuseIv->MetaHeaderIv,
					XNVM_EFUSE_META_HEADER_IV_START_ROW);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_META_HEADER_IV);
			goto END;
		}
	}
	if (EfuseIv->PrgmBlkObfusIv == TRUE) {
		Status = XNvm_EfuseCheckZeros(XNVM_EFUSE_BLACK_OBFUS_IV_START_ROW,
					(XNVM_EFUSE_BLACK_OBFUS_IV_START_ROW +
					XNVM_EFUSE_IV_NUM_OF_ROWS));
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XNVM_EFUSE_ERR_BLK_OBFUS_IV_ALREADY_PRGMD;
			goto END;
		}
	}
	if (EfuseIv->PrgmPlmIv == TRUE) {
		Status = XNvm_EfuseValidateIV(EfuseIv->PlmIv,
					XNVM_EFUSE_PLM_IV_START_ROW);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_PLM_IV);
			goto END;
		}
	}
	if (EfuseIv->PrgmDataPartitionIv == TRUE) {
		Status = XNvm_EfuseValidateIV(EfuseIv->DataPartitionIv,
					XNVM_EFUSE_DATA_PARTITION_IV_START_ROW);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				(u32)XNVM_EFUSE_ERR_WRITE_DATA_PARTITION_IV);
			goto END;
		}
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/******************************************************************************/
/**
 * This function Validates single IV requested for programming bit by bit.
 *
 * @param	Iv	Pointer to Iv data.
 *
 * @param	Row	Start row of the Iv to be validated.
 *
 * @return
 *		- XST_SUCCESS - if validation is successful.
 *		- XNVM_EFUSE_ERR_BIT_CANT_REVERT - if requested to revert the
 *						already programmed bit.
 *******************************************************************************/
static u32 XNvm_EfuseValidateIV(u32 *Iv, u32 Row)
{
	u32 Status = (u32)XST_FAILURE;
	u32 IvRowsRd[XNVM_EFUSE_IV_LEN_IN_WORDS];
	u32 Column;
	u32 Mask;
	u32 IvRow;

	if (Iv == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfuseReadCacheRange(Row,
					XNVM_EFUSE_IV_NUM_OF_ROWS,
					&(IvRowsRd[0]));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	for (IvRow = 0; IvRow < XNVM_EFUSE_IV_LEN_IN_WORDS;
			IvRow++) {

		for (Column = 0U; Column < XNVM_EFUSE_MAX_BITS_IN_ROW;
				Column++) {

			Mask = 1U << Column;

			if (((Iv[IvRow] & Mask) == 0U) &&
				((IvRowsRd[IvRow] & Mask) == Mask)) {

				Status = (u32)XNVM_EFUSE_ERR_BEFORE_PROGRAMMING |
					(u32)XNVM_EFUSE_ERR_BIT_CANT_REVERT;
				goto END;
			}
		}
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * This function validates all Aes keys requested for programming.
 *
 * @param	Keys	Pointer to XNvm_EfuseAesKeys structure.
 *
 * @return
 *	- XST_SUCCESS - if validation is successful.
 *	- XNVM_EFUSE_ERR_AES_ALREADY_PRGMD - Aes key already programmed
 *	- XNVM_EFUSE_ERR_USER_KEY0_ALREADY_PRGMD - User key 0 is already
 *							programmed
 *	- XNVM_EFUSE_ERR_USER_KEY1_ALREADY_PRGMD - User key 1 is already
 *							programmed
 *	- XNVM_EFUSE_ERR_FUSE_PROTECTED - Efuse is write protected
 *	- XNVM_EFUSE_ERR_WRITE_AES_KEY - Error in writing Aes key
 *	- XNVM_EFUSE_ERR_WRITE_USER0_KEY - Error in writing User key 0
 *	- XNVM_EFUSE_ERR_WRITE_USER1_KEY - Error in writing User key 1
 ******************************************************************************/
static u32 XNvm_EfuseValidateAesWriteReq(XNvm_EfuseAesKeys *Keys)
{
	u32 Status = (u32)XST_FAILURE;
	XNvm_EfuseSecCtrlBits ReadBackSecCtrlBits = {0U};

	Status = XNvm_EfuseReadSecCtrlBits(
			&ReadBackSecCtrlBits);
	if(Status != (u32)XST_SUCCESS) {
		Status = XNVM_EFUSE_ERR_RD_SEC_CTRL_BITS;
		goto END;
	}

	if (Keys->PrgmAesKey == TRUE) {
		Status = XNvm_EfuseCheckAesKeyCrc(XNVM_EFUSE_CRC_AES_ZEROS);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XNVM_EFUSE_ERR_AES_ALREADY_PRGMD;
			goto END;
		}
		if ((ReadBackSecCtrlBits.AesDis == TRUE) ||
			(ReadBackSecCtrlBits.AesWrLk == TRUE)) {
			Status = ((u32)XNVM_EFUSE_ERR_FUSE_PROTECTED |
				(u32)XNVM_EFUSE_ERR_WRITE_AES_KEY);
			goto END;
		}
	}
	if (Keys->PrgmUserKey0 == TRUE) {
		Status = XNvm_EfuseCheckAesUserKey0Crc(XNVM_EFUSE_CRC_AES_ZEROS);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XNVM_EFUSE_ERR_USER_KEY0_ALREADY_PRGMD;
			goto END;
		}
		if ((ReadBackSecCtrlBits.AesDis == TRUE) ||
			(ReadBackSecCtrlBits.UserKey0WrLk == TRUE)) {
			Status = ((u32)XNVM_EFUSE_ERR_FUSE_PROTECTED |
				(u32)XNVM_EFUSE_ERR_WRITE_USER_KEY0);
			goto END;
		}
	}
	if (Keys->PrgmUserKey1 == TRUE) {
		Status = XNvm_EfuseCheckAesUserKey1Crc(XNVM_EFUSE_CRC_AES_ZEROS);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XNVM_EFUSE_ERR_USER_KEY1_ALREADY_PRGMD;
			goto END;
		}
		if ((ReadBackSecCtrlBits.AesDis == TRUE) ||
			(ReadBackSecCtrlBits.UserKey1WrLk == TRUE)) {
			Status = ((u32)XNVM_EFUSE_ERR_FUSE_PROTECTED |
					(u32)XNVM_EFUSE_ERR_WRITE_USER_KEY1);
			goto END;
		}
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/******************************************************************************/
/**
 * This function Validates all PPK Hash requested for programming.
 *
 * @param	Hash		Pointer to XNvm_EfusePpkHash structure.
 *
 * @return
 *	- XST_SUCCESS - if reads successfully.
 *	- XNVM_EFUSE_ERR_PPK0_HASH_ALREADY_PRGMD - Ppk0 hash already programmed
 *	- XNVM_EFUSE_ERR_PPK1_HASH_ALREADY_PRGMD - Ppk1 hash already programmed
 *	- XNVM_EFUSE_ERR_PPK2_HASH_ALREADY_PRGMD - Ppk2 hash already programmed
 *	- XNVM_EFUSE_ERR_FUSE_PROTECTED - Efuse is write protected
 *	- XNVM_EFUSE_ERR_WRITE_PPK0_HASH - Error in writing ppk0 hash
 *	- XNVM_EFUSE_ERR_WRITE_PPK1_HASH - Error in writing ppk1 hash
 *	- XNVM_EFUSE_ERR_WRITE_PPK2_HASH - Error in writing ppk2 hash
 ******************************************************************************/
static u32 XNvm_EfuseValidatePpkWriteReq(XNvm_EfusePpkHash *Hash)
{
	u32 Status = (u32)XST_FAILURE;
	XNvm_EfuseSecCtrlBits ReadBackSecCtrlBits = {0U};

	Status = XNvm_EfuseReadSecCtrlBits(
			&ReadBackSecCtrlBits);
	if(Status != (u32)XST_SUCCESS) {
		Status = XNVM_EFUSE_ERR_RD_SEC_CTRL_BITS;
		goto END;
	}
	if (Hash->PrgmPpk0Hash == TRUE) {
		Status = XNvm_EfuseCheckZeros(XNVM_EFUSE_PPK_0_HASH_START_ROW,
					(XNVM_EFUSE_PPK_0_HASH_START_ROW +
					XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS));
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XNVM_EFUSE_ERR_PPK0_HASH_ALREADY_PRGMD;
			goto END;
		}
		if (ReadBackSecCtrlBits.Ppk0WrLk == TRUE) {
			Status = ((u32)XNVM_EFUSE_ERR_FUSE_PROTECTED |
					(u32)XNVM_EFUSE_ERR_WRITE_PPK0_HASH);
			goto END;
		}
	}
	if (Hash->PrgmPpk1Hash == TRUE) {
		Status = XNvm_EfuseCheckZeros(XNVM_EFUSE_PPK_1_HASH_START_ROW,
					(XNVM_EFUSE_PPK_1_HASH_START_ROW +
					XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS));
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XNVM_EFUSE_ERR_PPK1_HASH_ALREADY_PRGMD;
			goto END;
		}
	}
	if (Hash->PrgmPpk2Hash == TRUE) {
		Status = XNvm_EfuseCheckZeros(XNVM_EFUSE_PPK_2_HASH_START_ROW,
					(XNVM_EFUSE_PPK_2_HASH_START_ROW +
					XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS));
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XNVM_EFUSE_ERR_PPK2_HASH_ALREADY_PRGMD;
			goto END;
		}
	}

	Status = XNvm_EfuseReadSecCtrlBits(
			&ReadBackSecCtrlBits);
	if(Status != (u32)XST_SUCCESS) {
		Status = XNVM_EFUSE_ERR_RD_SEC_CTRL_BITS;
		goto END;
	}

	if (Hash->PrgmPpk0Hash == TRUE) {
		if (ReadBackSecCtrlBits.Ppk0WrLk == TRUE) {
			Status = ((u32)XNVM_EFUSE_ERR_FUSE_PROTECTED |
					(u32)XNVM_EFUSE_ERR_WRITE_PPK0_HASH);
			goto END;
		}
	}
	if (Hash->PrgmPpk1Hash == TRUE) {
		if (ReadBackSecCtrlBits.Ppk1WrLk == TRUE) {
			Status = ((u32)XNVM_EFUSE_ERR_FUSE_PROTECTED |
					(u32)XNVM_EFUSE_ERR_WRITE_PPK1_HASH);
			goto END;
		}
	}
	if (Hash->PrgmPpk2Hash == TRUE) {
		if (ReadBackSecCtrlBits.Ppk2WrLk == TRUE) {
			Status = ((u32)XNVM_EFUSE_ERR_FUSE_PROTECTED |
					(u32)XNVM_EFUSE_ERR_WRITE_PPK2_HASH);
			goto END;
		}
	}

	Status = (u32)XST_SUCCESS;
END:
	return Status;
}

/******************************************************************************/
/**
 * This function validates DEC_ONLY eFuse programming request.
 *
 * @param	WriteReq	Pointer to XNvm_EfuseData structure.
 *
 * @return
 *	- XST_SUCCESS - if validation is successful.
 *	- XNVM_EFUSE_ERR_AES_SHOULD_BE_PRGMD - Aes key should be programmed
 *						to program DEC_ONLY
 *
 *	- XNVM_EFUSE_ERR_BLKOBFUS_IV_SHOULD_BE_PRGMD - Blk Obfuscated IV
 *						should be programmed to
 *						program DEC_ONLY
 ******************************************************************************/
static u32 XNvm_EfuseValidateDecOnlyWriteReq(XNvm_EfuseData *WriteReq)
{
	u32 Status = (u32)XST_FAILURE;
	u32 SecurityMisc0 = 0U;
	u32 Mask = 0U;
	u32 Column;

	if (WriteReq == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
                goto END;
	}

	SecurityMisc0 = XNvm_EfuseReadReg(
				XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_SECURITY_MISC_0_OFFSET);
	if ((WriteReq->DecOnly->PrgmDecOnly == TRUE) &&
		((SecurityMisc0 &
		XNVM_EFUSE_CACHE_SECURITY_MISC_0_DEC_EFUSE_ONLY_MASK) ==
								0x00U)) {
		Status = XNvm_EfuseCheckAesKeyCrc(
				XNVM_EFUSE_CRC_AES_ZEROS);
		if (Status == (u32)XST_SUCCESS) {
			if (WriteReq->AesKeys != NULL) {
				if (WriteReq->AesKeys->PrgmAesKey != TRUE) {
					Status =
					(u32)XNVM_EFUSE_ERR_DEC_ONLY_KEY_MUST_BE_PRGMD;
					goto END;
				}
			}
			else {
				Status =
				(u32)XNVM_EFUSE_ERR_DEC_ONLY_KEY_MUST_BE_PRGMD;
				goto END;
			}
		}
		Status = XNvm_EfuseCheckZeros(
				XNVM_EFUSE_BLACK_OBFUS_IV_START_ROW,
				XNVM_EFUSE_BLACK_OBFUS_IV_START_ROW +
				XNVM_EFUSE_IV_NUM_OF_ROWS);
		if (Status == (u32)XST_SUCCESS) {
			if (WriteReq->Ivs != NULL) {
				if (WriteReq->Ivs->PrgmBlkObfusIv != TRUE) {
					Status =
					(u32)XNVM_EFUSE_ERR_DEC_ONLY_IV_MUST_BE_PRGMD;
					goto END;
				}
			}
			else {
				Status =
				(u32)XNVM_EFUSE_ERR_DEC_ONLY_IV_MUST_BE_PRGMD;
				goto END;
			}
		}
	}
	else if (WriteReq->DecOnly->PrgmDecOnly == TRUE) {
		for (Column = 0U; Column < XNVM_EFUSE_MAX_BITS_IN_ROW;
				Column++) {

			Mask = 1U << Column;

			if (((WriteReq->DecOnly->DecEfuseOnly &
				Mask) == 0U) && ((SecurityMisc0 & Mask) ==
							Mask)) {

				Status =
				(u32)XNVM_EFUSE_ERR_WRITE_DEC_EFUSE_ONLY |
				(u32)XNVM_EFUSE_ERR_BIT_CANT_REVERT;
				goto END;
			}
		}
	}

	Status = (u32)XST_SUCCESS;
END:
	return Status;
}
/******************************************************************************/
/**
 * This function is used verify eFUSEs for Zeros
 *
 * @param	RowStart	Row number from which verification has to be
 *						started.
 * @param	RowEnd		Row number till which verification has to be
 *						ended.
 * @return
 *		- XST_SUCCESS - if efuses are not programmed.
 * 		- XST_FAILURE - if efuses are already programmed.
 ******************************************************************************/
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

/******************************************************************************/
/**
 * This function performs pre checks for programming all the specified bits.
 *
 * @param	WriteData	Pointer to the XNvm_EfuseData.
 *
 * @return
 *		XST_SUCCESS - if all the conditions for programming is satisfied
 *		Errorcode - if any of the conditions are not met
 *
 ******************************************************************************/
static u32 XNvm_EfuseValidateWriteReq(XNvm_EfuseData *WriteChecks)
{
	u32 Status = (u32)XST_FAILURE;

	if (WriteChecks == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (WriteChecks->AesKeys != NULL) {
		Status = XNvm_EfuseValidateAesWriteReq(WriteChecks->AesKeys);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	if (WriteChecks->PpkHash != NULL) {
		Status = XNvm_EfuseValidatePpkWriteReq(WriteChecks->PpkHash);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	if (WriteChecks->RevokeIds != NULL) {
		Status = XNvm_EfuseValidateRevokeIdsWriteReq(
					WriteChecks->RevokeIds);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	if (WriteChecks->Ivs != NULL) {
		Status = XNvm_EfuseValidateIVsWriteReq(WriteChecks->Ivs);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	if (WriteChecks->DecOnly != NULL) {
		Status = XNvm_EfuseValidateDecOnlyWriteReq(WriteChecks);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	if (WriteChecks->UserFuses != NULL) {
		Status = XNvm_EfuseValidateUserFusesWriteReq(
					WriteChecks->UserFuses);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	Status = (u32)XST_SUCCESS;
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
static u32 XNvm_EfuseWritePufSynData(u32 *SynData)
{
	u32 Status = (u32)XST_FAILURE;

	if (SynData == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfusePgmAndVerifyRows(XNVM_EFUSE_PUF_SYN_START_ROW,
				XNVM_EFUSE_PUF_SYN_DATA_NUM_OF_ROWS,
				XNVM_EFUSE_PAGE_2,
				SynData);
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
static u32 XNvm_EfuseWritePufChash(u32 Chash)
{
	u32 Status = (u32)XST_FAILURE;

	Status = XNvm_EfusePgmAndVerifyRows(XNVM_EFUSE_PUF_CHASH_ROW,
				XNVM_EFUSE_PUF_CHASH_NUM_OF_ROWS,
				XNVM_EFUSE_PAGE_0, &Chash);
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
static u32 XNvm_EfuseWritePufAux(u32 Aux)
{
	u32 Status = (u32)XST_FAILURE;
	u32 AuxData;

	AuxData = (Aux & XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_ECC_23_0_MASK);

	Status = XNvm_EfusePgmAndVerifyRows(XNVM_EFUSE_PUF_AUX_ROW, 0x01U,
						XNVM_EFUSE_PAGE_0, &AuxData);
	return Status;

}
/***************************************************************************/
/**
 * This function checks whether PUF is already programmed or not.
 *
 * @return
 *	- XST_SUCCESS - if all rows are zero
 *	- XNVM_EFUSE_ERR_PUF_SYN_ALREADY_PRGMD - Puf Syn data already programmed
 *	- XNVM_EFUSE_ERR_PUF_CHASH_ALREADY_PRGMD - Puf chash already programmed
 *	- XNVM_EFUSE_ERR_PUF_AUX_ALREADY_PRGMD - Puf Aux is already programmed
 *******************************************************************************/
static u32 XNvm_EfuseIsPufHelperDataEmpty(void)
{
	u32 Status = (u32)XST_FAILURE;
	u32 RowDataVal;

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

	Status = XNvm_EfuseCheckZeros(XNVM_EFUSE_PUF_SYN_START_ROW,
					(XNVM_EFUSE_PUF_SYN_START_ROW +
					XNVM_EFUSE_PUF_SYN_DATA_NUM_OF_ROWS));
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XNVM_EFUSE_ERR_PUF_SYN_ALREADY_PRGMD;
		goto END;
	}
END :
	return Status;

}

/******************************************************************************/
/**
 * This function Validates all Revocation Ids requested for programming.
 *
 * @param	RevokeIds	Pointer to XNvm_EfuseRevokeIds structure.
 *
 * @return
 *		- XST_SUCCESS - if validation is successful.
 *		- XNVM_EFUSE_ERR_BIT_CANT_REVERT - Efuse bit cant be reverted
 *		- XNVM_EFUSE_ERR_WRITE_REVOCATION_IDS - Error in writing
 *						Revocation id efuses.
 ******************************************************************************/
static u32 XNvm_EfuseValidateRevokeIdsWriteReq(XNvm_EfuseRevokeIds *RevokeIds)
{
	u32 Status = (u32)XST_FAILURE;
	u32 RevokeIdRd;
	u32 RevokeIdNum;
	u32 Column;
	u32 Mask;
	u32 Row;

	if (RevokeIds == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if ((RevokeIds->PrgmRevokeId0 != FALSE) ||
		(RevokeIds->PrgmRevokeId1 != FALSE) ||
		(RevokeIds->PrgmRevokeId2 != FALSE) ||
		(RevokeIds->PrgmRevokeId3 != FALSE) ||
		(RevokeIds->PrgmRevokeId4 != FALSE) ||
		(RevokeIds->PrgmRevokeId5 != FALSE) ||
		(RevokeIds->PrgmRevokeId6 != FALSE) ||
		(RevokeIds->PrgmRevokeId7 != FALSE)) {

		if (XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_PGM_LOCK_REG_OFFSET) != 0x00U) {
			Status = ((u32)XNVM_EFUSE_ERR_FUSE_PROTECTED |
				(u32)XNVM_EFUSE_ERR_WRITE_REVOCATION_IDS);
			goto END;
		}

		for (RevokeIdNum = 0; RevokeIdNum < XNVM_NUM_OF_REVOKE_ID_FUSES;
				RevokeIdNum++) {
			Row = RevokeIdNum + XNVM_EFUSE_REVOCATION_ID_0_ROW;

			Status = XNvm_EfuseReadCache(Row, &RevokeIdRd);
			if (Status != XST_SUCCESS) {
				goto END;
			}

			for (Column = 0U; Column < XNVM_EFUSE_MAX_BITS_IN_ROW;
					Column++) {

				Mask = 1U << Column;

				if (((RevokeIds->RevokeId[RevokeIdNum] &
					Mask) == 0U) && ((RevokeIdRd & Mask) ==
								Mask)) {

					Status = (u32)XNVM_EFUSE_ERR_WRITE_REVOCATION_IDS |
						(u32)XNVM_EFUSE_ERR_BIT_CANT_REVERT;
					goto END;
				}
			}
		}
	}
	Status = XST_SUCCESS;
END:
	return Status;
}

/******************************************************************************/
/*
 * This function Programs User eFuses.
 *
 * @param	WriteUserFuses	Pointer to the XNvm_EfuseUserData structure
 *
 * @return
 *		- XST_SUCCESS - if programming is successful.
 *		- XST_FAILURE - On programming failure.
 *
 ******************************************************************************/
static u32 XNvm_EfusePrgmUserFuses(XNvm_EfuseUserData *WriteUserFuses)
{
	u32 Status = (u32)XST_FAILURE;
	u32 UserFusesDataToPrgm[XNVM_NUM_OF_USER_FUSES] = {0U};
	u32 StartRow;
	u32 EndRow;

	if (WriteUserFuses == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	StartRow = XNVM_EFUSE_USER_FUSE_START_ROW +
			WriteUserFuses->StartUserFuseNum - 1U;
	EndRow = StartRow + WriteUserFuses->NumOfUserFuses;

	Status = XNvm_EfuseComputeProgrammableBits(WriteUserFuses->UserFuseData,
						UserFusesDataToPrgm,
						StartRow,
						EndRow);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XNvm_EfusePgmAndVerifyRows(StartRow,
				WriteUserFuses->NumOfUserFuses,
				XNVM_EFUSE_PAGE_0, UserFusesDataToPrgm);
END:
	return Status;
}

/******************************************************************************/
/**
 * This function Validates all user fuses requested for programming.
 *
 * @param	WriteUserFuses		Pointer to XNvm_EfuseUserData structure.
 *
 * @return
 *		- XST_SUCCESS - if programming is successful.
 *		- XNVM_EFUSE_ERR_BIT_CANT_REVERT - Efuse bit cant be reverted.
 ******************************************************************************/
static u32 XNvm_EfuseValidateUserFusesWriteReq(
					XNvm_EfuseUserData *WriteUserFuses)
{
	u32 Status = XST_FAILURE;
	u32 UserFuseValueRd = 0U;
	u32 StartRow = 0U;
	u32 EndRow = 0U;
	u32 Row = 0U;
	u32 Column;
	u32 Mask;

	if (WriteUserFuses == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	if ((WriteUserFuses->StartUserFuseNum < XNVM_USER_FUSE_START_NUM) ||
		(WriteUserFuses->StartUserFuseNum > XNVM_USER_FUSE_END_NUM)) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	if ((WriteUserFuses->UserFuseData == NULL) ||
		(WriteUserFuses->NumOfUserFuses > XNVM_NUM_OF_USER_FUSES)) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	if (((WriteUserFuses->StartUserFuseNum - 1U)  +
		WriteUserFuses->NumOfUserFuses) > XNVM_USER_FUSE_END_NUM) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	StartRow = XNVM_EFUSE_USER_FUSE_START_ROW +
			WriteUserFuses->StartUserFuseNum - 1U;

	EndRow = StartRow + WriteUserFuses->NumOfUserFuses;

	for (Row = StartRow; Row < EndRow; Row++) {
		Status = XNvm_EfuseReadCache(Row, &UserFuseValueRd);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		for (Column = 0U; Column < XNVM_EFUSE_MAX_BITS_IN_ROW;
				Column++) {

			Mask = 1U << Column;

			if (((WriteUserFuses->UserFuseData[Row - StartRow] &
				Mask) == 0U) &&
				((UserFuseValueRd & Mask) == Mask)) {

				Status = (u32)XNVM_EFUSE_ERR_BEFORE_PROGRAMMING |
					(u32)XNVM_EFUSE_ERR_BIT_CANT_REVERT;
				goto END;
			}
		}
	}
	Status = XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 * This function Programs Protection eFuses.
 *
 * @return
 *	- XST_SUCCESS - if protection efuse programming is successful.
 *	- XNVM_EFUSE_ERR_WRITE_ROW_43_0_PROT - Error in programming Row 43
 *						protection bit 0
 *	- XNVM_EFUSE_ERR_WRITE_ROW_43_1_PROT - Error in programming Row 43
 *						protection bit 1
 *	- XNVM_EFUSE_ERR_WRITE_ROW_57_0_PROT - Error in programming Row 57
 *						protection bit 0
 *	- XNVM_EFUSE_ERR_WRITE_ROW_57_1_PROT - Error in programming Row 57
 *						protection bit 1
 *	- XNVM_EFUSE_ERR_WRITE_ROW64_87_0_PROT - Error in programming Row 64_87
 *						protection bit 0
 *	- XNVM_EFUSE_ERR_WRITE_ROW64_87_1_PROT - Error in programming Row 64_87
 *						protection bit 1
 *	- XNVM_EFUSE_ERR_WRITE_ROW96_99_0_PROT - Error in programming Row 96_99
 *						protection bit 0
 *	- XNVM_EFUSE_ERR_WRITE_ROW96_99_1_PROT - Error in programing Row 96_99
 *						protection bit 1
 *	- XNVM_EFUSE_ERR_WRITE_ROW_40_PROT - Error in programming Row 40
 *						protection bit
 *	- XNVM_EFUSE_ERR_WRITE_ROW_37_PROT - Error in programming Row 37
 *						protection bit
 *	- XNVM_EFUSE_ERR_WRITE_ROW_42_PROT - Error in programming row 42
 *						protection bit
 *	- XNVM_EFUSE_ERR_WRITE_ROW_58_PROT - Error in programming row 58
 *						protection bit
 *******************************************************************************/
static u32 XNvm_EfusePrgmProtectionEfuse(void)
{
	u32 Status = XST_FAILURE;
	u32 SecurityCtrlData;
	u32 SecurityMisc0Data;
	u32 SecurityMisc1Data;
	u32 MiscCtrlData;
	u32 BootEnvCtrlRow;
	u32 PufChashData;

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
static u32 XNvm_EfusePgmAndVerifyRows(u32 StartRow, u8 RowCount,
			XNvm_EfuseType EfuseType, const u32* RowData)
{
	u32 Status = (u32)XST_FAILURE;
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

	Status = (u32)XST_SUCCESS;
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

	Status = XNvm_EfuseCheckForTBits();
	if (Status != XST_SUCCESS) {
		goto END;
	}
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
