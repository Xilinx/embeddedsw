/******************************************************************************
* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
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
* <pre>
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
*       mb    07/22/25 Add XNvm_EfuseInitClockAndValidateFreq to set eFuse clock frequency and clk src
*       mb    08/25/25 Add support for boot mode disable eFUSE bits programming
*       mb    09/09/25 Set the EFUSE clock source register with user provided clock source
*       mb    10/03/25 Set lower bits 32 bits of black IV to zeros
*       mb    10/03/25 Update PPK macros for SPARTANUPLUSAES1
*       mb    10/14/25 Update logic of XNvm_EfusePrgmSpkRevokeId function
*       mb    11/11/2025 Add support for JTAG Boot mode disable efuse programming
*3.7    bha   01/23/2026 Add note in function documentation explaining validation
*                        purpose for efuse PUF hash programming
* 3.7   mb    02/09/2026 Rename secure control bit names for SPARTANUPLUSAES1
*       bha   03/02/2026 Used XSECURE_TEMPORAL_IMPL pattern for PUFHD_INVLD programming
*                        in XNvm_EfusePrgmXilinxCtrl
*	mb    03/18/2026 Add support for temperature and voltage checks before efuse programming
* 3.7   hae   02/27/2026 Support XILINX_CTRL OSPI_RESET_RECOVERY_DELAY_CTRL
*                        and ROM_RSVD_OSPI_DEV_RESET_CHOICE
*                        and ROM_OSPI_CMD_SEQ_CTRL eFuse bit programming
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files *********************************/

#include "xnvm_efuse.h"
#include "xnvm_utils.h"

/************************** Constant Definitions *****************************/
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
static int XNvm_EfusePrgmIv(XNvm_EfuseAesIvs *AesIv);
static int XNvm_EfusePrgmUserFuse(const XNvm_EfuseUserFuse *UserFuse);
static int XNvm_EfusePrgmSpkRevokeId(const XNvm_EfuseSpkRevokeId *SpkRevokeId);
static int XNvm_EfusePrgmAesRevokeId(const XNvm_EfuseAesRevokeId *AesRevokeId);
static int XNvm_EfusePrgmDecOnly(const XNvm_EfuseDecOnly *DecOnly);
static int XNvm_EfusePrgmSecCtrlBits(const XNvm_EfuseSecCtrlBits *SecCtrl);
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
static int XNvm_EfuseCheckZeros(u32 OffsetStart, u32 OffsetEnd);
static int XNvm_EfusePrgmPufHDInvld(const XNvm_EfuseXilinxCtrl *PufHDInvld);
static int XNvm_EfusePrgmRomOspiCmdSeqCtrl(const XNvm_EfuseXilinxCtrl *XilinxCtrl);
static int XNvm_EfusePrgmXilinxCtrl(const XNvm_EfuseXilinxCtrl *XilinxCtrl);
static int XNvm_EfuseInitClockAndValidateFreq(XNvm_EfuseData *EfuseData);
#ifdef SPARTANUPLUSAES1
static int XNvm_EfusePrgmBootModeDisBits(const XNvm_EfuseBootModeDis *BootModeDis);
#endif

/************************** Function Definitions *****************************/
/***************************************************************************/
/**
* @brief 	This function is used to program the eFUSE of spartan ultrascale plus, based on user
* 		inputs
*
* @param	EfuseData	Pointer to the XNvm_EfuseData.
*
* @return
* 		- XST_SUCCESS if programs successfully.
* 		- XNVM_EFUSE_ERR_INVALID_PARAM - On invalid parameter
* 		- XNVM_EFUSE_ERR_NTHG_TO_BE_PROGRAMMED - Nothing to be programmed
* 		- XNVM_EFUSE_ERR_BEFORE_PROGRAMMING - Error before programming
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
#ifndef SPARTANUPLUSAES1
	    (EfuseData->UserFuse == NULL)) {
#else
	    (EfuseData->UserFuse == NULL) && (EfuseData->BootModeDis == NULL)) {
#endif
		Status = XNVM_EFUSE_ERR_NTHG_TO_BE_PROGRAMMED;
		goto END;
	}

	/** - Initialize Efuse clock settings and validate frequency */
	Status = XNvm_EfuseInitClockAndValidateFreq(EfuseData);
	if (Status != XST_SUCCESS) {
		Status = Status;
		goto END;
	}

	/** - Perform temperature and voltage checks before programming */
#ifdef XNVM_ENABLE_ENV_MONITOR_CHECKS
	Status = XNvm_EfuseTempAndVoltChecks(EfuseData->SysMonInstPtr);
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		goto END;
	}
#endif

	/** - Sets up Efuse controller with validated frequency */
	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM, XNVM_EFUSE_MARGIN_RD,
					   EfuseData->EfuseClkFreq);
	if (Status != XST_SUCCESS) {
		Status = Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING;
		goto END_RST;
	}

	/** - Validate all the write requests for AesKeys, PPK hash 0/1/2, Ivs, DecOnly eFuses */
	Status = XNvm_EfuseValidateBeforeWriteReq(EfuseData);
	if (Status != XST_SUCCESS) {
		Status = Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING;
		goto END_RST;
	}

	/** - Program AES keys to eFUSE if AES key data is provided */
	if (EfuseData->AesKeys != NULL) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePrgmAesKey(EfuseData->AesKeys);
		if (Status != XST_SUCCESS) {
			goto END_RST;
		}
	}

	/** - Program PPK hash to eFUSE if PPK hash data is provided */
	if (EfuseData->PpkHash != NULL) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePrgmPpkHash(EfuseData->PpkHash);
		if (Status != XST_SUCCESS) {
			goto END_RST;
		}
	}

	/** - Program AES IV to eFUSE if IV data is provided */
	if (EfuseData->Ivs != NULL) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePrgmIv(EfuseData->Ivs);
		if (Status != XST_SUCCESS) {
			goto END_RST;
		}
	}

	/** - Program user fuse to eFUSE if user fuse data is provided */
	if (EfuseData->UserFuse != NULL) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePrgmUserFuse(EfuseData->UserFuse);
		if (Status != XST_SUCCESS) {
			goto END_RST;
		}
	}

	/** - Program SPK revoke ID to eFUSE if SPK revoke data is provided */
	if (EfuseData->SpkRevokeId != NULL) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePrgmSpkRevokeId(EfuseData->SpkRevokeId);
		if (Status != XST_SUCCESS) {
			goto END_RST;
		}
	}

	/** - Program AES revoke ID to eFUSE if AES revoke data is provided */
	if (EfuseData->AesRevokeId != NULL) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePrgmAesRevokeId(EfuseData->AesRevokeId);
		if (Status != XST_SUCCESS) {
			goto END_RST;
		}
	}

	/** - Program DEC only eFUSE bits if DEC only data is provided */
	if (EfuseData->DecOnly != NULL) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePrgmDecOnly(EfuseData->DecOnly);
		if (Status != XST_SUCCESS) {
			goto END_RST;
		}
	}

	/** - Program Xilinx control eFUSE bits if Xilinx control data is provided */
	if (EfuseData->XilinxCtrl != NULL) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePrgmXilinxCtrl(EfuseData->XilinxCtrl);
		if (Status != XST_SUCCESS) {
			goto END_RST;
		}
	}

	/** - Program security control bits to eFUSE if security control data is provided */
	if (EfuseData->SecCtrlBits != NULL) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePrgmSecCtrlBits(EfuseData->SecCtrlBits);
		if (Status != XST_SUCCESS) {
			goto END_RST;
		}
	}
#ifdef SPARTANUPLUSAES1
	/** - Program boot mode disable eFUSE bits if boot mode disable data is provided */
	if (EfuseData->BootModeDis != NULL) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePrgmBootModeDisBits(EfuseData->BootModeDis);
		if (Status != XST_SUCCESS) {
			goto END_RST;
		}
	}
#endif
	/** - Reload eFUSE cache after all programming operations are complete */
	Status = XNvm_EfuseCacheReload();

END_RST:
	/** - Reset read mode and disable programming controller regardless of programming status */
	SStatus = XNvm_EfuseResetReadMode();
	if (SStatus != XST_SUCCESS) {
		Status |= SStatus;
	}

	SStatus = XNvm_EfuseDisableProgramming();
	if (SStatus != XST_SUCCESS) {
		Status |= SStatus;
	}

	/** - Lock eFUSE controller */
	SStatus = XNvm_EfuseLockController();
	if (SStatus != XST_SUCCESS) {
		Status |= SStatus;
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function reads secure control bits and CRC_EN register bits from eFUSE cache.
 *
 * @param	SecCtrlBits - Pointer to XNvm_EfuseSecCtrlBits where secure control
 *                        bits are read.
 *
 * @note	Device-specific behavior:
 *
 * 		**SPARTANUPLUS devices:**
 * 		- Supports PPK0, PPK1, and PPK2 (three PPK hashes)
 * 		- PPK hash size: 256 bits (32 bytes)
 * 		- Reads Ppk2lck and Ppk2Invld bits for PPK2
 * 		- PPK2 eFUSE registers are shared between PPK2 hash and PUF hash storage
 * 		- The HashPufOrKey bit determines which hash type is stored:
 * 		  * When HashPufOrKey = 0: PPK2 registers contain PPK2 hash
 * 		  * When HashPufOrKey = 1: PPK2 registers contain PUF hash
 * 		- When programming PUF hash:
 * 		  * Ppk2Invld must be 0 (PPK2 register not invalidated)
 * 		  * Ppk2lck must be 0 (PPK2 register not write-locked)
 * 		  * PPK2 registers must not be already programmed
 * 		  * HashPufOrKey must be set to 1 to indicate PUF hash usage
 *
 * 		**SPARTANUPLUSAES1 devices:**
 * 		- Supports only PPK0 and PPK1 (two PPK hashes)
 * 		- PPK hash size: 384 bits (48 bytes)
 * 		- Ppk2lck and Ppk2Invld fields are not available
 * 		- Reads Ppk1lck and Ppk1Invld bits for PPK1
 * 		- PPK1 eFUSE registers are shared between PPK1 hash and PUF hash storage
 * 		- The HashPufOrKey bit determines which hash type is stored:
 * 		  * When HashPufOrKey = 0: PPK1 registers contain PPK1 hash
 * 		  * When HashPufOrKey = 1: PPK1 registers contain PUF hash
 * 		- When programming PUF hash:
 * 		  * Ppk1Invld must be 0 (PPK1 register not invalidated)
 * 		  * Ppk1lck must be 0 (PPK1 register not write-locked)
 * 		  * PPK1 registers must not be already programmed
 * 		  * HashPufOrKey must be set to 1 to indicate PUF hash usage
 *
 * @return
 * 		- XST_SUCCESS - Specified data read.
 * 		- XNVM_EFUSE_ERR_INVALID_PARAM - Invalid parameter.
 * 		- XNVM_EFUSE_ERR_CACHE_PARITY - Parity Error exist in cache.
 *
 ******************************************************************************/
int XNvm_EfuseReadSecCtrlBits(XNvm_EfuseSecCtrlBits *SecCtrlBits)
{
	int Status = XST_FAILURE;
	u32 SecCtrlVal = 0U;

	/** - Validate output pointer before reading security control eFUSE bits */
	if (SecCtrlBits ==  NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	/** - Read security control eFUSE cache register and extract individual bit fields */
	Status = XNvm_EfuseReadCache(XNVM_EFUSE_CONTROL_OFFSET, &SecCtrlVal);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	SecCtrlBits->ScanClearEn = (u32)XNVM_GET_8_BIT_VAL(SecCtrlVal, XNVM_EFUSE_SEC_CTRL_BITS, XNVM_EFUSE_SEC_CTRL_SCAN_CLEAR_EN_SHIFT);
	SecCtrlBits->AesDis = (u32)XNVM_GET_8_BIT_VAL(SecCtrlVal, XNVM_EFUSE_SEC_CTRL_BITS, XNVM_EFUSE_SEC_CTRL_AES_DIS_SHIFT);
	SecCtrlBits->RmaDis = (u32)XNVM_GET_8_BIT_VAL(SecCtrlVal, XNVM_EFUSE_SEC_CTRL_BITS, XNVM_EFUSE_SEC_CTRL_RMA_DISABLE_0_SHIFT);
	SecCtrlBits->RmaEn = (u32)XNVM_GET_8_BIT_VAL(SecCtrlVal, XNVM_EFUSE_SEC_CTRL_BITS, XNVM_EFUSE_SEC_CTRL_RMA_ENABLE_0_SHIFT);
	SecCtrlBits->JtagDis = (u32)XNVM_GET_8_BIT_VAL(SecCtrlVal, XNVM_EFUSE_SEC_CTRL_BITS, XNVM_EFUSE_SEC_CTRL_JTAG_DIS_SHIFT);
	SecCtrlBits->PufTes2Dis = (u32)XNVM_GET_8_BIT_VAL(SecCtrlVal, XNVM_EFUSE_SEC_CTRL_BITS, XNVM_EFUSE_SEC_CTRL_PUF_TEST2_DIS_SHIFT);
	SecCtrlBits->HashPufOrKey = (u32)XNVM_GET_8_BIT_VAL(SecCtrlVal, XNVM_EFUSE_SEC_CTRL_BITS, XNVM_EFUSE_SEC_CTRL_HASH_PUF_OR_KEY_SHIFT);
	SecCtrlBits->Ppk0lck = (u32)XNVM_GET_8_BIT_VAL(SecCtrlVal, XNVM_EFUSE_SEC_CTRL_BITS, XNVM_EFUSE_SEC_CTRL_PPK0_WR_LK_SHIFT);
	SecCtrlBits->Ppk0Invld = (u32)(XNVM_GET_8_BIT_VAL(SecCtrlVal, XNVM_EFUSE_SEC_CTRL_BITS, XNVM_EFUSE_SEC_CTRL_PPK0_INVLD0_SHIFT) ||
	                              XNVM_GET_8_BIT_VAL(SecCtrlVal, XNVM_EFUSE_SEC_CTRL_BITS, XNVM_EFUSE_SEC_CTRL_PPK0_INVLD1_SHIFT));
	SecCtrlBits->Ppk1lck = (u32)XNVM_GET_8_BIT_VAL(SecCtrlVal, XNVM_EFUSE_SEC_CTRL_BITS, XNVM_EFUSE_SEC_CTRL_PPK1_WR_LK_SHIFT);
	SecCtrlBits->Ppk1Invld = (u32)(XNVM_GET_8_BIT_VAL(SecCtrlVal, XNVM_EFUSE_SEC_CTRL_BITS, XNVM_EFUSE_SEC_CTRL_PPK1_INVLD0_SHIFT) ||
	                              XNVM_GET_8_BIT_VAL(SecCtrlVal, XNVM_EFUSE_SEC_CTRL_BITS, XNVM_EFUSE_SEC_CTRL_PPK1_INVLD1_SHIFT));
#ifndef SPARTANUPLUSAES1
	SecCtrlBits->Ppk2lck = (u32)XNVM_GET_8_BIT_VAL(SecCtrlVal, XNVM_EFUSE_SEC_CTRL_BITS, XNVM_EFUSE_SEC_CTRL_PPK2_WR_LK_SHIFT);
	SecCtrlBits->Ppk2Invld = (u32)(XNVM_GET_8_BIT_VAL(SecCtrlVal, XNVM_EFUSE_SEC_CTRL_BITS, XNVM_EFUSE_SEC_CTRL_PPK2_INVLD0_SHIFT) ||
	                              XNVM_GET_8_BIT_VAL(SecCtrlVal, XNVM_EFUSE_SEC_CTRL_BITS, XNVM_EFUSE_SEC_CTRL_PPK2_INVLD1_SHIFT));
#endif
	SecCtrlBits->AesRdlk = (u32)(XNVM_GET_8_BIT_VAL(SecCtrlVal, XNVM_EFUSE_SEC_CTRL_BITS, XNVM_EFUSE_SEC_CTRL_AES_RD_WR_LK_0_SHIFT) ||
	                             XNVM_GET_8_BIT_VAL(SecCtrlVal, XNVM_EFUSE_SEC_CTRL_BITS, XNVM_EFUSE_SEC_CTRL_AES_RD_WR_LK_1_SHIFT));
	SecCtrlBits->JtagErrDis = (u32)XNVM_GET_8_BIT_VAL(SecCtrlVal, XNVM_EFUSE_SEC_CTRL_BITS, XNVM_EFUSE_SEC_CTRL_JTAG_ERR_OUT_DIS_SHIFT);
	SecCtrlBits->UserWrlk = (u32)XNVM_GET_8_BIT_VAL(SecCtrlVal, XNVM_EFUSE_SEC_CTRL_BITS, XNVM_EFUSE_SEC_CTRL_USER_WR_LK_SHIFT);

	/** - Read CRC enable eFUSE cache register and extract CRC, DFT, lockdown and RMA bit fields */
	Status = XNvm_EfuseReadCache(XNVM_EFUSE_CRC_EN_OFFSET, &SecCtrlVal);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	SecCtrlBits->CrcEn = (u32)XNVM_GET_8_BIT_VAL(SecCtrlVal, XNVM_EFUSE_SEC_CTRL_BITS, XNVM_EFUSE_CRC_EN_SHIFT);
	SecCtrlBits->DftDis = (u32)XNVM_GET_8_BIT_VAL(SecCtrlVal, XNVM_EFUSE_SEC_CTRL_DFT_BITS, XNVM_EFUSE_DFT_DIS_SHIFT);
	SecCtrlBits->Lckdwn = (u32)XNVM_GET_8_BIT_VAL(SecCtrlVal, XNVM_EFUSE_SEC_CTRL_BITS, XNVM_EFUSE_LCKDOWN_SHIFT);
	SecCtrlBits->CrcRmaDis = (u32)XNVM_GET_8_BIT_VAL(SecCtrlVal, XNVM_EFUSE_SEC_CTRL_BITS, XNVM_EFUSE_RMA_DISABLE_1_SHIFT);
	SecCtrlBits->CrcRmaEn = (u32)XNVM_GET_8_BIT_VAL(SecCtrlVal, XNVM_EFUSE_SEC_CTRL_BITS, XNVM_EFUSE_RMA_ENABLE_1_SHIFT);

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
 * @return
 * 		- XST_SUCCESS - Specified data read.
 * 		- XNVM_EFUSE_ERR_INVALID_PARAM - Invalid input parameter.
 * 		- XST_FAILURE - Error in cache read operation.
 *
 ******************************************************************************/
int XNvm_EfuseReadPpkHash(XNvm_EfusePpkType PpkType, u32 *PpkData, u32 PpkSize)
{
	int Status = XST_FAILURE;
	u32 PpkStartOffset = 0U;
	u32 OffsetCnt = 0U;

	/** - Validate input parameters before reading PPK hash from eFUSE cache */
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

	/** - Determine PPK start offset based on PPK type and read the hash from cache range */
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
 * @return
 * 		- XST_SUCCESS - Specified data read.
 * 		- XNVM_EFUSE_ERR_INVALID_PARAM - On invalid parameter
 *
 ******************************************************************************/
int XNvm_EfuseReadSpkRevokeId(u32 *SpkRevokeData, u32 SpkRevokeRow)
{
	int Status = XST_FAILURE;

	/** - Validate input parameters before reading SPK revoke ID from eFUSE cache */
	if (SpkRevokeData == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (SpkRevokeRow >= XNVM_EFUSE_MAX_SPK_REVOKE_ID) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	/** - Read SPK revoke ID from computed cache offset based on revoke row index */
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
 * @return
 * 		- XST_SUCCESS - Specified data read.
 * 		- XNVM_EFUSE_ERR_INVALID_PARAM - On invalid parameter
 *
 ******************************************************************************/
int XNvm_EfuseReadAesRevokeId(u32 *AesRevokeData)
{
	int Status = XST_FAILURE;

	/** - Validate input pointer before reading AES revoke ID from eFUSE cache */
	if (AesRevokeData == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	/** - Read AES revoke ID value from eFUSE cache */
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
 * @return
 * 		- XST_SUCCESS - Specified data read.
 * 		- XNVM_EFUSE_ERR_INVALID_PARAM - On invalid parameter
 *
 ******************************************************************************/
int XNvm_EfuseReadUserFuse(u32 *UserFuseData)
{
	int Status = XST_FAILURE;

	/** - Validate input pointer before reading user fuse from eFUSE cache */
	if (UserFuseData == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	/** - Read user fuse value from eFUSE cache */
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
 * @return
 * 		- XST_SUCCESS - Specified data read.
 * 		- XNVM_EFUSE_ERR_INVALID_PARAM - On invalid parameter
 *
 ******************************************************************************/
int XNvm_EfuseReadIv(XNvm_EfuseIvType IvType, u32 *IvData)
{
	int Status = XST_FAILURE;
	u32 IvStartOffset = 0U;

	/** - Validate input parameters and select IV start offset based on IV type */
	if (IvData ==  NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	if (IvType ==  XNVM_EFUSE_AES_IV_RANGE) {
		IvStartOffset = XNVM_EFUSE_AES_IV_RANGE_START_OFFSET;
	} else if (IvType == XNVM_EFUSE_BLACK_IV) {
		IvStartOffset = XNVM_EFUSE_BLACK_IV_START_OFFSET;
	} else {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	/** - Read IV data from eFUSE cache starting at the determined offset */
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
 * @return
 * 		- XST_SUCCESS - Specified data read.
 * 		- XNVM_EFUSE_ERR_INVALID_PARAM - On invalid parameter
 *
 ******************************************************************************/
int XNvm_EfuseReadDna(u32 *Dna)
{
	int Status = XST_FAILURE;

	/** - Validate input pointer before reading DNA from eFUSE cache */
	if (Dna ==  NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	/** - Read DNA data from eFUSE cache over the DNA word range */
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
 * @return
 * 		- XST_SUCCESS - Specified data read.
 * 		- XNVM_EFUSE_ERR_INVALID_PARAM - On invalid parameter
 *
 ******************************************************************************/
int XNvm_EfuseReadDecOnly(u32 *DecOnly)
{
	int Status = XST_FAILURE;

	/** - Validate input pointer before reading DEC only eFUSE value from cache */
	if (DecOnly ==  NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	/** - Read DEC only value from eFUSE cache */
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
 * @return
 * 		- XST_SUCCESS - Specified data read.
 * 		- XNVM_EFUSE_ERR_INVALID_PARAM - On invalid parameter
 *
 ******************************************************************************/
int XNvm_EfuseReadXilinxCtrl(XNvm_EfuseXilinxCtrl *XilinxCtrl)
{
	int Status = XST_FAILURE;
	u32 Data = 0U;

	/** - Validate input pointer before reading Xilinx control eFUSE from cache */
	if (XilinxCtrl ==  NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	/** - Read Xilinx control register from eFUSE cache and extract individual eFUSE bit fields */
	Status = XNvm_EfuseReadCache(XNVM_EFUSE_XILINX_CTRL_OFFSET, &Data);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XilinxCtrl->PrgmPufHDInvld = XNVM_GET_8_BIT_VAL(Data, XNVM_EFUSE_PUFHD_INVLD_EFUSE_BITS,
							XNVM_EFUSE_PUFHD_INVLD_EFUSE_SHIFT);
	XilinxCtrl->PrgmDisSJtag = XNVM_GET_8_BIT_VAL(Data, XNVM_EFUSE_DIS_SJTAG_EFUSE_BITS,
							XNVM_EFUSE_DIS_SJTAG_EFUSE_SHIFT);
	XilinxCtrl->PrgmOspiResetRecoveryDelayCtrl = XNVM_GET_8_BIT_VAL(Data,
							XNVM_EFUSE_OSPI_RESET_RECOVERY_DELAY_CTRL_BITS,
							XNVM_EFUSE_OSPI_RESET_RECOVERY_DELAY_CTRL_SHIFT);
	XilinxCtrl->PrgmRomRsvdOspiDevResetChoice = XNVM_GET_8_BIT_VAL(Data,
							XNVM_EFUSE_ROM_RSVD_OSPI_DEV_RESET_CHOICE_BITS,
							XNVM_EFUSE_ROM_RSVD_OSPI_DEV_RESET_CHOICE_SHIFT);
	XilinxCtrl->PrgmRomOspiCmdSeqCtrl = XNVM_GET_8_BIT_VAL(Data,
							XNVM_EFUSE_ROM_OSPI_CMD_SEQ_CTRL_BITS,
							XNVM_EFUSE_ROM_OSPI_CMD_SEQ_CTRL_SHIFT);
END:
	return Status;
}

/******************************************************************************/
/**
 * @brief Initializes the eFUSE clock and validates its frequency.
 * This function sets up the Efuse clock configuration required for eFUSE programming
 * and verifies that the clock frequency is within the supported range for
 * reliable operation.
 *
 * @param  EfuseData	Pointer to the XNvm_EfuseData.
 *
 * @return
 * 		- XST_SUCCESS Clock initialization and frequency validation were successful.
 * 		- XNVM_EFUSE_ERR_INVALID_FREQUENCY If clock frequency is invalid.
 *
 *****************************************************************************/
static int XNvm_EfuseInitClockAndValidateFreq(XNvm_EfuseData *EfuseData)
{
	volatile int Status = XST_FAILURE;

	/**
	 * - If XNVM_SET_EFUSE_CLK_FREQUENCY_FROM_RTCA is set,
	 * read the frequency and clock source from RTCA space.
	 * and initialize to EfuseClkFreq and EfuseClkSrc.
	 */
#ifdef XNVM_SET_EFUSE_CLK_FREQUENCY_FROM_RTCA
	EfuseData->EfuseClkFreq = Xil_In32(XNVM_PLM_CONFIG_BASE_ADDRESS +
				XNVM_RTCA_EFUSE_CLK_FREQUENCY_OFFSET);
	EfuseData->EfuseClkSrc = Xil_In32(XNVM_EFUSE_CLK_CTRL_ADDR);
#else
	/**
	 * - If XNVM_SET_EFUSE_CLK_FREQUENCY_FROM_RTCA is not set,
	 * Set the frequency clock source value provided by user
	 * in XNVM_EFUSE_CLK_CTRL_ADDR register.
	 */
	Xil_UtilRMW32(XNVM_EFUSE_CLK_CTRL_ADDR, EfuseData->EfuseClkSrc,
			EfuseData->EfuseClkSrc);
#endif

	/**
	 * - Validate the Efuse clock frequency
	 */
	if ((EfuseData->EfuseClkFreq <= XNVM_EMCCLK_MIN_FREQUENCY) ||
		(EfuseData->EfuseClkFreq >= XNVM_EMCCLK_MAX_FREQUENCY)) {
		Status = XNVM_EFUSE_ERR_INVALID_CLK_FREQUENCY;
		goto END;
	}

	/**
	 * - If Clock source is EMC, set the EMC clock enable bit in XNVM_EFUSE_IO_CTRL_ADDR
	 */
	if (EfuseData->EfuseClkSrc == XNVM_EFUSE_CLK_SRC_EMCCLK_VALUE) {
		Xil_UtilRMW32(XNVM_EFUSE_IO_CTRL_ADDR, XNVM_EFUSE_EMC_CLK_EN_VAL,
				XNVM_EFUSE_EMC_CLK_EN_VAL);
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function validates AES efuse keys write request
 *
 * @param	AesKey - Pointer to XNvm_EfuseAesKeys.
 *
 * @return
 * 		- XST_SUCCESS - On success.
 * 		- XNVM_EFUSE_ERR_RD_SEC_CTRL_BITS - Error reading security control bits.
 * 		- XNVM_EFUSE_ERR_FUSE_PROTECTED | XNVM_EFUSE_ERR_WRITE_AES_KEY - AES fuses are protected.
 * 		- XNVM_EFUSE_ERR_AES_ALREADY_PRGMD - AES key is already programmed.
 *
 ******************************************************************************/
static int XNvm_EfuseValidateAesWriteReq(const XNvm_EfuseAesKeys *AesKey)
{
	volatile int Status = XST_FAILURE;
	XNvm_EfuseSecCtrlBits ReadBackSecCtrlBits = {0U};

	/** - Validate AES key write request if AES key programming is requested */
	if (AesKey->PrgmAesKey == (u32)TRUE) {
		/** - Read security control bits to check if AES key write is protected */
		Status = XNvm_EfuseReadSecCtrlBits(
				 &ReadBackSecCtrlBits);
		if (Status != XST_SUCCESS) {
			Status = XNVM_EFUSE_ERR_RD_SEC_CTRL_BITS;
			goto END;
		}

		/** - Check if AES key is disabled or write-locked; return error if protected */
		if ((ReadBackSecCtrlBits.AesDis == (u32)TRUE) ||
		    (ReadBackSecCtrlBits.AesRdlk == (u32)TRUE)) {
			Status = (XNVM_EFUSE_ERR_FUSE_PROTECTED |
				  XNVM_EFUSE_ERR_WRITE_AES_KEY);
			goto END;
		}

		/** - Check AES key CRC against zero; non-zero CRC means key is already programmed */
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
 * @return
 * 		- XST_SUCCESS - On success.
 * 		- XNVM_EFUSE_ERR_INVALID_PARAM - Invalid parameter.
 * 		- XNVM_EFUSE_ERR_PPK0_HASH_ALREADY_PRGMD - PPK0 hash is already programmed.
 * 		- XNVM_EFUSE_ERR_PPK1_HASH_ALREADY_PRGMD - PPK1 hash is already programmed.
 * 		- XNVM_EFUSE_ERR_PPK2_HASH_ALREADY_PRGMD - PPK2 hash is already programmed.
 *
 ******************************************************************************/
static int XNvm_EfuseValidatePpkWriteReq(const XNvm_EfusePpkHash *PpkHash)
{
	int Status = XST_FAILURE;
	XNvm_EfuseSecCtrlBits ReadSecCtrlBits = {0U};
#ifndef SPARTANUPLUSAES1
	u32 IsPpkPrgmRequested = (PpkHash->PrgmPpk0Hash == (u32)TRUE) ||
				 (PpkHash->PrgmPpk1Hash == (u32)TRUE) ||
				 (PpkHash->PrgmPpk2Hash == (u32)TRUE);
#else
	u32 IsPpkPrgmRequested = (PpkHash->PrgmPpk0Hash == (u32)TRUE) ||
				(PpkHash->PrgmPpk1Hash == (u32)TRUE);
#endif

	/**
	 * - If none of the PPK hash programming flags (PrgmPpk0Hash, PrgmPpk1Hash,
	 * PrgmPpk2Hash) are set, there's nothing to validate and the function
	 * can return Status as success.
	 */
	if (IsPpkPrgmRequested == (u32)FALSE) {
		Status = XST_SUCCESS;
		goto END;
	}

	/**
	 * - Check if the requested PPK hash size is valid
	 * For SPARTANUPLUS, PPK Hash should be 256 bits.
	 * For SPARTANUPLUSAES1, PPK Hash should be 384 bits.
	 */

	if (PpkHash->ActualPpkHashSize != XNVM_EFUSE_PPK_HASH_SIZE_IN_BYTES) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	/**
	 * - If PPK programming is requested, read respective WR_LK eFuse bits
	 * to make sure the PPK write is allowed.
	 */
	Status = XNvm_EfuseReadSecCtrlBits(&ReadSecCtrlBits);
	if (Status != XST_SUCCESS) {
		Status = (int)XNVM_EFUSE_ERR_RD_SEC_CTRL_BITS;
		goto END;
	}

	/**
	 * - Check if the requested PPK0 hash is already programmed
	 * and check PPK0 write lock bit is set.
	 */
	if (PpkHash->PrgmPpk0Hash == (u32)TRUE) {
		if (ReadSecCtrlBits.Ppk0lck == (u32)TRUE) {
			Status = (XNVM_EFUSE_ERR_FUSE_PROTECTED |
				  XNVM_EFUSE_ERR_WRITE_PPK0_HASH);
			goto END;
		}
		Status = XNvm_EfuseCheckZeros(XNVM_EFUSE_PPK0_START_OFFSET,
					      (XNVM_EFUSE_PPK0_START_OFFSET + PpkHash->ActualPpkHashSize));
		if (Status != XST_SUCCESS) {
			Status = XNVM_EFUSE_ERR_PPK0_HASH_ALREADY_PRGMD;
			goto END;
		}
	}

	/**
	 * - Check if the requested PPK1 hash is already programmed
	 * and check PPK1 write lock bit is set.
	 */
	if (PpkHash->PrgmPpk1Hash == (u32)TRUE) {
		if (ReadSecCtrlBits.Ppk1lck == (u32)TRUE) {
			Status = (XNVM_EFUSE_ERR_FUSE_PROTECTED |
				  XNVM_EFUSE_ERR_WRITE_PPK1_HASH);
			goto END;
		}
		Status = XNvm_EfuseCheckZeros(XNVM_EFUSE_PPK1_START_OFFSET,
					      (XNVM_EFUSE_PPK1_START_OFFSET +
					       PpkHash->ActualPpkHashSize));
		if (Status != XST_SUCCESS) {
			Status = XNVM_EFUSE_ERR_PPK1_HASH_ALREADY_PRGMD;
			goto END;
		}
	}

	/**
	 * - Check if the requested PPK2 hash is already programmed
	 * and check PPK2 write lock bit is set.
	 */
#ifndef SPARTANUPLUSAES1
	if (PpkHash->PrgmPpk2Hash == (u32)TRUE) {
		if (PpkHash->ActualPpkHashSize != XNVM_EFUSE_PPK_HASH_SIZE_IN_BYTES) {
			Status = XNVM_EFUSE_ERR_INVALID_PARAM;
			goto END;
		}
		if (ReadSecCtrlBits.Ppk2lck== (u32)TRUE) {
			Status = (XNVM_EFUSE_ERR_FUSE_PROTECTED |
				  XNVM_EFUSE_ERR_WRITE_PPK2_HASH);
			goto END;
		}
		Status = XNvm_EfuseCheckZeros(XNVM_EFUSE_PPK2_START_OFFSET, XNVM_EFUSE_PPK2_START_OFFSET +
					      PpkHash->ActualPpkHashSize);
		if (Status != XST_SUCCESS) {
			Status = XNVM_EFUSE_ERR_PPK2_HASH_ALREADY_PRGMD;
		}
	}
#endif

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function validates IV efuses write request
 *
 * @param	Ivs - Pointer to XNvm_EfuseAesIvs.
 *
 * @return
 * 		- XST_SUCCESS - On success.
 * 		- XNVM_EFUSE_ERR_BEFORE_PROGRAMMING | XNVM_EFUSE_ERR_BIT_CANT_REVERT - IV bits cannot be reverted.
 *
 ******************************************************************************/
static int XNvm_EfuseValidateIvsWriteReq(const XNvm_EfuseAesIvs *Ivs)
{
	int Status = XST_FAILURE;
	u32 IvRow;
	u32 IvRowsRd[XNVM_EFUSE_AES_IV_SIZE_IN_WORDS] = {0U};

	/** - Read IVs from eFUSE cache to check IV's are already programmed or not */
	if (Ivs->PrgmIv == (u32)TRUE) {
		if (Ivs->IvType == XNVM_EFUSE_AES_IV_RANGE) {
			Status = XNvm_EfuseReadCacheRange(XNVM_EFUSE_AES_IV_RANGE_START_OFFSET,
							  XNVM_EFUSE_AES_IV_SIZE_IN_WORDS,
							  IvRowsRd);
		}
		else {
			Status = XNvm_EfuseReadCacheRange(XNVM_EFUSE_BLACK_IV_START_OFFSET,
							  XNVM_EFUSE_AES_IV_SIZE_IN_WORDS,
							  IvRowsRd);
		}

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
 * @return
 * 		- XST_SUCCESS - On success.
 * 		- XNVM_EFUSE_ERR_DEC_ONLY_KEY_MUST_BE_PRGMD - AES key must be programmed before DEC_ONLY.
 * 		- XNVM_EFUSE_ERR_DEC_ONLY_HASH_OR_PUF_KEY_MUST_BE_PRGMD - Hash or PUF key must be programmed.
 * 		- XNVM_EFUSE_ERR_DEC_ONLY_ALREADY_PRGMD - DEC_ONLY is already programmed.
 *
 ******************************************************************************/
static int XNvm_EfuseValidateDecOnlyWriteReq(const XNvm_EfuseData *EfuseData)
{
	int Status = XST_FAILURE;
	u32 DecOnlyVal = 0U;
	XNvm_EfuseSecCtrlBits ReadSecCtrlBits;

	/** - Validate DEC only write request if DEC only programming is requested */
	if (EfuseData->DecOnly->PrgmDecOnly == (u32)TRUE) {
		/** - Read DEC only eFUSE cache value to check current programmed state */
		Status = XNvm_EfuseReadCache(XNVM_EFUSE_DEC_ONLY_OFFSET, &DecOnlyVal);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		/** - If DEC only is not yet programmed, validate AES key and PUF/key hash prerequisites */
		if (DecOnlyVal == 0U) {
			/** - Verify AES key is programmed by checking its CRC against zero value */
			Status = XNvm_EfuseCheckAesKeyCrc(XNVM_EFUSE_AES_CRC_OFFSET,
							  XNVM_EFUSE_STS_AES_CRC_DONE_MASK,
							  XNVM_EFUSE_STS_AES_CRC_PASS_MASK,
							  XNVM_EFUSE_CRC_AES_ZEROS);
			if (Status == XST_SUCCESS) {
				if (EfuseData->AesKeys != NULL) {
					if (EfuseData->AesKeys->PrgmAesKey != (u32)TRUE) {
						Status = XNVM_EFUSE_ERR_DEC_ONLY_KEY_MUST_BE_PRGMD;
						goto END;
					}
				} else {
					Status = XNVM_EFUSE_ERR_DEC_ONLY_KEY_MUST_BE_PRGMD;
					goto END;
				}
			}

			/** - Read security control bits to verify HashPufOrKey is set before DEC only */
			Status =  XNvm_EfuseReadSecCtrlBits(&ReadSecCtrlBits);
			if (Status != XST_SUCCESS) {
				goto END;
			}

			/** - Return error if neither existing nor requested HashPufOrKey bit is set */
			if (ReadSecCtrlBits.HashPufOrKey != (u32)TRUE) {
				if (EfuseData->SecCtrlBits != NULL) {
					if (EfuseData->SecCtrlBits->HashPufOrKey != (u32)TRUE) {
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
 * @return
 * 		- XST_SUCCESS - On success.
 * 		- XST_FAILURE - On validation failure.
 *
 ******************************************************************************/
static int XNvm_EfuseValidateBeforeWriteReq(const XNvm_EfuseData *EfuseData)
{
	volatile int Status = XST_FAILURE;

	/** - Validate AES key write request if AES keys are provided */
	if (EfuseData->AesKeys != NULL) {
		Status =  XNvm_EfuseValidateAesWriteReq(EfuseData->AesKeys);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	/** - Validate PPK hash write request if PPK hash data is provided */
	if (EfuseData->PpkHash != NULL) {
		Status = XNvm_EfuseValidatePpkWriteReq(EfuseData->PpkHash);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	/** - Validate IV write request if IV data is provided */
	if (EfuseData->Ivs != NULL) {
		Status = XNvm_EfuseValidateIvsWriteReq(EfuseData->Ivs);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	/** - Validate DEC only write request prerequisites if DEC only data is provided */
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
 *          	with programmed key
 *
 * @param	AesKey - Pointer to XNvm_EfuseAesKeys.
 *
 * @return
 * 		- XST_SUCCESS - On successful programming.
 * 		- XNVM_EFUSE_ERR_WRITE_AES_KEY - Error in AES key programming.
 *
 ******************************************************************************/
static int XNvm_EfusePrgmAesKey(const XNvm_EfuseAesKeys *AesKey)
{
	int Status = XST_FAILURE;
	XNvm_EfusePrgmInfo AesPrgmInfo = {0U};
	u32 Crc = 0U;

	/** - Initialize AES key eFUSE programming information with row, column and skip-verify settings */
	AesPrgmInfo.StartRow = XNVM_EFUSE_AES_KEY_START_ROW;
	AesPrgmInfo.NumOfRows = XNVM_EFUSE_AES_KEY_NUM_OF_ROWS;
	AesPrgmInfo.ColStart = XNVM_EFUSE_AES_KEY_START_COL;
	AesPrgmInfo.ColEnd = XNVM_EFUSE_AES_KEY_END_COL;
	AesPrgmInfo.SkipVerify = (u32)TRUE;

	/** - Program AES key data to eFUSE rows, skipping bit-level verification during write */
	Status = XNvm_EfusePgmAndVerifyData(&AesPrgmInfo, AesKey->AesKey);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_WRITE_AES_KEY);
		goto END;
	}

	/** - Reload eFUSE cache so the programmed AES key is accessible for CRC verification */
	Status = XNvm_EfuseCacheReload();
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_WRITE_AES_KEY);
		goto END;
	}

	/** - Calculate CRC of the provided AES key to verify against the programmed value */
	Crc = XNvm_AesCrcCalc(AesKey->AesKey);

	/** - Verify programmed AES key by checking its CRC matches the expected calculated CRC */
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
 * @return
 * 		- XST_SUCCESS - On successful programming.
 * 		- XNVM_EFUSE_ERR_WRITE_PPK0_HASH - Error in PPK0 hash programming.
 * 		- XNVM_EFUSE_ERR_WRITE_PPK1_HASH - Error in PPK1 hash programming.
 * 		- XNVM_EFUSE_ERR_WRITE_PPK2_HASH - Error in PPK2 hash programming.
 *
 ******************************************************************************/
static int XNvm_EfusePrgmPpkHash(XNvm_EfusePpkHash *PpkHash)
{
	volatile int Status = XST_FAILURE;
	XNvm_EfusePrgmInfo PpkPrgmInfo = {0U};
	u32 RemainingPpkLen = 0U;
	u32 Ppk1NoofRows = 0U;

	/** - Calculate remaining PPK length and PPK1 row count for 384-bit hash programming */
	RemainingPpkLen = (PpkHash->ActualPpkHashSize - XNVM_EFUSE_PPK_HASH_256_SIZE_IN_BYTES);
	Ppk1NoofRows = (RemainingPpkLen != 0U) ? RemainingPpkLen : XNVM_EFUSE_PPK_HASH_256_SIZE_IN_BYTES;
	/** - Program PPK0 hash to eFUSE rows if PPK0 programming is requested */
	if (PpkHash->PrgmPpk0Hash ==  (u32)TRUE) {
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

	/** - Program PPK1 hash to eFUSE rows if PPK1 programming is requested */
	if (PpkHash->PrgmPpk1Hash == (u32)TRUE) {
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
#ifndef SPARTANUPLUSAES1
	/** - Program PPK2 hash to eFUSE rows if PPK2 programming is requested */
	if (PpkHash->PrgmPpk2Hash == (u32)TRUE) {
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
#endif

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs IV in to efuses
 *
 * @param	AesIv - Pointer to XNvm_EfuseAesIvs.
 *
 * @return
 * 		- XST_SUCCESS - On successful programming.
 * 		- XNVM_EFUSE_ERR_WRITE_IV - Error in IV programming.
 *
 ******************************************************************************/
static int XNvm_EfusePrgmIv(XNvm_EfuseAesIvs *AesIv)
{
	int Status = XST_FAILURE;
	XNvm_EfusePrgmInfo AesIvInfo = {0U};

	AesIvInfo.StartRow = XNVM_EFUSE_AES_IV_START_ROW;
	AesIvInfo.NumOfRows = XNVM_EFUSE_AES_IV_NUM_OF_ROWS;
	AesIvInfo.ColStart = XNVM_EFUSE_AES_IV_START_COL;
	AesIvInfo.ColEnd = XNVM_EFUSE_AES_IV_END_COL;
	AesIvInfo.SkipVerify = (u32)FALSE;

	/** - If IV type is Black IV, set lower 32 bits to zero */
	if (AesIv->IvType == XNVM_EFUSE_BLACK_IV) {
		Status = Xil_SMemSet(AesIv->AesIv, sizeof(AesIv->AesIv), 0U, XNVM_EFUSE_WORD_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	Status = XNvm_EfusePgmAndVerifyData(&AesIvInfo, AesIv->AesIv);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_WRITE_IV);
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs user fuse
 *
 * @param	UserFuse - Pointer to XNvm_EfuseUserFuse.
 *
 * @return
 * 		- XST_SUCCESS - On successful programming.
 * 		- XNVM_EFUSE_ERR_WRITE_USER_FUSE - Error in user fuse programming.
 *
 ******************************************************************************/
static int XNvm_EfusePrgmUserFuse(const XNvm_EfuseUserFuse *UserFuse)
{
	int Status = XST_FAILURE;
	XNvm_EfusePrgmInfo UserFuseInfo = {0U};
	u32 UserFuseVal = 0U;

	/** - Compute programmable user fuse bits by masking out already programmed bits */
	Status = XNvm_EfuseComputeProgrammableBits((const u32 *)&UserFuse->UserFuseVal, &UserFuseVal,
		 XNVM_EFUSE_USER_FUSE_OFFSET, XNVM_EFUSE_USER_FUSE_OFFSET);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** - Initialize user fuse eFUSE programming information with row and column settings */
	UserFuseInfo.StartRow = XNVM_EFUSE_USER_FUSE_START_ROW;
	UserFuseInfo.NumOfRows = XNVM_EFUSE_USER_FUSE_NUM_OF_ROWS;
	UserFuseInfo.ColStart = XNVM_EFUSE_USER_FUSE_START_COL;
	UserFuseInfo.ColEnd = XNVM_EFUSE_USER_FUSE_END_COL;
	UserFuseInfo.SkipVerify = (u32)FALSE;

	/** - Program user fuse bits to eFUSE and verify the written data */
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
 * @return
 * 		- XST_SUCCESS - On successful programming.
 * 		- XNVM_EFUSE_ERR_WRITE_SPK_REVOKE_ID - Error in SPK revoke ID programming.
 *
 ******************************************************************************/
static int XNvm_EfusePrgmSpkRevokeId(const XNvm_EfuseSpkRevokeId *SpkRevokeId)
{
	int Status = XST_FAILURE;
	int StatusTmp = XST_FAILURE;
	u32 RevokeIdRow = 0U;
	u32 RevokeIdCol = 0U;

	/**
	 * - Validate input parameters.
	 * Return XNVM_EFUSE_ERR_INVALID_PARAM, if input parameters are invalid.
	 */
	if ((SpkRevokeId->RevokeIdNum) > (XNVM_MAX_REVOKE_ID_FUSES - 1U)) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	/**
	 * - Calculate the Row and Column numbers based on the revoke id number input provided.
	 */
	RevokeIdRow = XNVM_EFUSE_SPK_REVOKE_ID_START_ROW + ((SpkRevokeId->RevokeIdNum) /
							     XNVM_EFUSE_BITS_IN_A_BYTE);
	RevokeIdCol = XNVM_EFUSE_SPK_REVOKE_ID_START_COL + ((SpkRevokeId->RevokeIdNum) %
							     XNVM_EFUSE_BITS_IN_A_BYTE);

	/**
	 * - Program revocation Id bit.
	 * Return XNVM_EFUSE_ERR_WRITE_SPK_REVOKE_ID upon failure.
	 */
	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, RevokeIdRow,
			      RevokeIdCol, FALSE);
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		Status = Status | XNVM_EFUSE_ERR_WRITE_SPK_REVOKE_ID;
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
 * @return
 * 		- XST_SUCCESS - On successful programming.
 * 		- XNVM_EFUSE_ERR_WRITE_AES_REVOKE_ID - Error in AES revoke ID programming.
 *
 ******************************************************************************/
static int XNvm_EfusePrgmAesRevokeId(const XNvm_EfuseAesRevokeId *AesRevokeId)
{
	int Status = XST_FAILURE;
	XNvm_EfusePrgmInfo AesRevokeIdInfo = {0U};
	u32 AesRevokeIdFuse;

	/** - Compute programmable AES revoke ID bits by masking out already programmed bits */
	Status = XNvm_EfuseComputeProgrammableBits((const u32 *)&AesRevokeId->AesRevokeIdVal, &AesRevokeIdFuse,
		 XNVM_EFUSE_AES_REVOKE_ID_OFFSET,
		 XNVM_EFUSE_AES_REVOKE_ID_OFFSET);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** - Initialize AES revoke ID eFUSE programming information with row and column settings */
	AesRevokeIdInfo.StartRow = XNVM_EFUSE_AES_REVOKE_ID_START_ROW;
	AesRevokeIdInfo.NumOfRows = XNVM_EFUSE_AES_REVOKE_ID_NUM_OF_ROWS;
	AesRevokeIdInfo.ColStart = XNVM_EFUSE_AES_REVOKE_ID_START_COL;
	AesRevokeIdInfo.ColEnd = XNVM_EFUSE_AES_REVOKE_ID_END_COL;
	AesRevokeIdInfo.SkipVerify = (u32)FALSE;

	/** - Program AES revoke ID bits to eFUSE and verify the written data */
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
 * @return
 * 		- XST_SUCCESS - On successful programming.
 * 		- XNVM_EFUSE_ERR_WRITE_DEC_ONLY - Error in DEC only programming.
 *
 ******************************************************************************/
static int XNvm_EfusePrgmDecOnly(const XNvm_EfuseDecOnly *DecOnly)
{
	int Status = XST_FAILURE;
	XNvm_EfusePrgmInfo DecOnlyInfo = {0U};
	u32 PrgmDecOnly = XNVM_EFUSE_CACHE_DEC_EFUSE_ONLY_MASK;

	/** - Program DEC only eFUSE bits if DEC only programming is requested */
	if (DecOnly->PrgmDecOnly == (u32)TRUE) {
		/** - Initialize DEC only eFUSE programming information with row and column settings */
		DecOnlyInfo.StartRow = XNVM_EFUSE_DEC_ONLY_START_ROW;
		DecOnlyInfo.NumOfRows = XNVM_EFUSE_DEC_ONLY_NUM_OF_ROWS;
		DecOnlyInfo.ColStart = XNVM_EFUSE_DEC_ONLY_START_COL;
		DecOnlyInfo.ColEnd = XNVM_EFUSE_DEC_ONLY_END_COL;
		DecOnlyInfo.SkipVerify = (u32)FALSE;

		/** - Program DEC only bits to eFUSE and verify the written data */
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
 * 		- XST_SUCCESS  Specified data read.
 * 		- XNVM_EFUSE_ERR_INVALID_PARAM  Invalid input parameter.
 * 		- XNVM_EFUSE_ERR_WRITE_PUF_HD_INVLD  Error in PUFHD_INVLD
 * 						     programming.
 *
 ******************************************************************************/
static int XNvm_EfusePrgmPufHDInvld(const XNvm_EfuseXilinxCtrl *PufHDInvld)
{
	int Status = XST_FAILURE;
	XNvm_EfusePrgmInfo PufHDInvld_Info = {0U};
	u32 PrgmPufHDInvld = XNVM_EFUSE_PUFHD_INVLD_EFUSE_VAL;

	/** - Validate input pointer before programming PUFHD_INVLD eFUSE bits */
	if (PufHDInvld == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	/** - Initialize PUFHD_INVLD eFUSE programming information with row and column settings */
	PufHDInvld_Info.StartRow =  XNVM_EFUSE_XILINX_CTRL_START_ROW;
	PufHDInvld_Info.NumOfRows = XNVM_EFUSE_PUFHD_INVLD_NUM_OF_ROWS;
	PufHDInvld_Info.ColStart = XNVM_EFUSE_PUFHD_INVLD_START_COL;
	PufHDInvld_Info.ColEnd = XNVM_EFUSE_PUFHD_INVLD_END_COL;
	PufHDInvld_Info.SkipVerify = (u32)FALSE;

	/** - Program PUFHD_INVLD bits to eFUSE and verify the written data */
	Status = XNvm_EfusePgmAndVerifyData(&PufHDInvld_Info, (const u32 *)&PrgmPufHDInvld);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_WRITE_PUFHD_INVLD);
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function is used to program ROM_OSPI_CMD_SEQ_CTRL eFuse
 * 		bits
 *
 * @param	XilinxCtrl - Pointer to XNvm_EfuseXilinxCtrl structure which
 * 		holds PrgmRomOspiCmdSeqCtrl value to program into eFuse.
 *
 * @return
 * 		- XST_SUCCESS  On successful programming.
 * 		- XNVM_EFUSE_ERR_INVALID_PARAM  On NULL pointer input.
 * 		- XNVM_EFUSE_ERR_WRITE_ROM_OSPI_CMD_SEQ_CTRL  Error in
 * 						ROM_OSPI_CMD_SEQ_CTRL
 * 						efuse programming.
 *
 ******************************************************************************/
static int XNvm_EfusePrgmRomOspiCmdSeqCtrl(const XNvm_EfuseXilinxCtrl *XilinxCtrl)
{
	int Status = XST_FAILURE;
	XNvm_EfusePrgmInfo RomOspiCmdSeqCtrl_Info = {0U};
	u32 PrgmRomOspiCmdSeqCtrl;

	/** - Initialize ROM OSPI command sequence control eFUSE programming information */
	PrgmRomOspiCmdSeqCtrl = XilinxCtrl->PrgmRomOspiCmdSeqCtrl;

	RomOspiCmdSeqCtrl_Info.StartRow =  XNVM_EFUSE_XILINX_CTRL_START_ROW;
	RomOspiCmdSeqCtrl_Info.NumOfRows = XNVM_EFUSE_ROM_OSPI_CMD_SEQ_CTRL_NUM_OF_ROWS;
	RomOspiCmdSeqCtrl_Info.ColStart = XNVM_EFUSE_ROM_OSPI_CMD_SEQ_CTRL_START_COL;
	RomOspiCmdSeqCtrl_Info.ColEnd = XNVM_EFUSE_ROM_OSPI_CMD_SEQ_CTRL_END_COL;
	RomOspiCmdSeqCtrl_Info.SkipVerify = (u32)FALSE;

	/** - Program ROM OSPI command sequence control bits to eFUSE and verify */
	Status = XNvm_EfusePgmAndVerifyData(&RomOspiCmdSeqCtrl_Info, (const u32 *)&PrgmRomOspiCmdSeqCtrl);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_WRITE_ROM_OSPI_CMD_SEQ_CTRL);
	}
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function is used to program XILINX_CTRL fuses.
 *
 * @param	XilinxCtrl - Pointer to XNvm_EfuseXilinxCtrl structure which holds
 * 		PufHDInvld, PrgmDisSJtag, PrgmOspiResetRecoveryDelayCtrl,
 * 		PrgmRomRsvdOspiDevResetChoice and PrgmRomOspiCmdSeqCtrl flags
 * 		which say to program eFuse.
 *
 * @return
 * 		- XST_SUCCESS  Specified data read.
 * 		- XNVM_EFUSE_ERR_INVALID_PARAM  Invalid input parameter.
 * 		- XNVM_EFUSE_ERR_WRITE_PUF_HD_INVLD  Error in PUFHD_INVLD
 * 						     programming.
 * 		- XNVM_EFUSE_ERR_WRITE_DIS_SJTAG Error in DIS_SJTAG
 * 						     programming.
 * 		- XNVM_EFUSE_ERR_WRITE_OSPI_RESET_RECOVERY_DELAY_CTRL Error in
 * 						     OSPI Reset Recovery Delay Control programming.
 * 		- XNVM_EFUSE_ERR_WRITE_ROM_RSVD_OSPI_DEV_RESET_CHOICE Error in
 * 						     ROM RSVD OSPI Device Reset Choice programming.
 * 		- XNVM_EFUSE_ERR_WRITE_ROM_OSPI_CMD_SEQ_CTRL Error in
 * 						     ROM OSPI Cmd Seq Ctrl programming.
 *
 ******************************************************************************/
static int XNvm_EfusePrgmXilinxCtrl(const XNvm_EfuseXilinxCtrl *XilinxCtrl)
{
	int Status = XST_FAILURE;
	int StatusTmp = XST_FAILURE;
	u32 Data;

	/** - Read current Xilinx control eFUSE cache to identify already programmed bits */
	Status =  XNvm_EfuseReadCache(XNVM_EFUSE_XILINX_CTRL_OFFSET, &Data);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** - Program PUFHD_INVLD eFUSE bit if requested and not already set */
	if ((XilinxCtrl->PrgmPufHDInvld == (u32)TRUE) &&
	    ((Data & XNVM_EFUSE_PUFHD_INVLD_EFUSE_MASK) == 0x0U)) {
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePrgmPufHDInvld, XilinxCtrl);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
			goto END;
		}
	}
	/** - Program DIS_SJTAG eFUSE bit if requested and not already set */
	if ((XilinxCtrl->PrgmDisSJtag == (u32)TRUE) &&
	    ((Data & XNVM_EFUSE_DIS_SJTAG_EFUSE_MASK) != XNVM_EFUSE_DIS_SJTAG_EFUSE_MASK)) {
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit,  XNVM_EFUSE_XILINX_CTRL_START_ROW,
				      XNVM_EFUSE_DIS_SJTAG_COL, FALSE);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = Status | XNVM_EFUSE_ERR_WRITE_DIS_SJTAG;
			goto END;
		}
	}
	/** - Program OSPI reset recovery delay control eFUSE bit if requested and not already set */
	if ((XilinxCtrl->PrgmOspiResetRecoveryDelayCtrl == (u32)TRUE) &&
	    ((Data & XNVM_EFUSE_OSPI_RESET_RECOVERY_DELAY_CTRL_MASK) !=
	     XNVM_EFUSE_OSPI_RESET_RECOVERY_DELAY_CTRL_MASK)) {
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit,
				      XNVM_EFUSE_XILINX_CTRL_START_ROW,
				      XNVM_EFUSE_OSPI_RESET_RECOVERY_DELAY_CTRL_COL, FALSE);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = Status | XNVM_EFUSE_ERR_WRITE_OSPI_RESET_RECOVERY_DELAY_CTRL;
			goto END;
		}
	}
	/** - Program ROM RSVD OSPI device reset choice eFUSE bit if requested and not already set */
	if ((XilinxCtrl->PrgmRomRsvdOspiDevResetChoice == (u32)TRUE) &&
	    ((Data & XNVM_EFUSE_ROM_RSVD_OSPI_DEV_RESET_CHOICE_MASK) !=
	     XNVM_EFUSE_ROM_RSVD_OSPI_DEV_RESET_CHOICE_MASK)) {
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit,
				      XNVM_EFUSE_XILINX_CTRL_START_ROW,
				      XNVM_EFUSE_ROM_RSVD_OSPI_DEV_RESET_CHOICE_COL, FALSE);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = Status | XNVM_EFUSE_ERR_WRITE_ROM_RSVD_OSPI_DEV_RESET_CHOICE;
			goto END;
		}
	}
	/** - Program ROM OSPI command sequence control eFUSE bits if requested and not already set */
	if ((XilinxCtrl->PrgmRomOspiCmdSeqCtrl != 0x0U) &&
	    ((Data & XNVM_EFUSE_ROM_OSPI_CMD_SEQ_CTRL_MASK) == 0x0U)) {
		Status = XNvm_EfusePrgmRomOspiCmdSeqCtrl(XilinxCtrl);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

END:
	return Status;

}

#ifdef SPARTANUPLUSAES1
/******************************************************************************/
/**
 * @brief	This function programs Boot mode disable bits
 *
 * @param	BootModeDis	Pointer to XNvm_EfuseBootModeDis.
 *
 * @return
 * 		- XST_SUCCESS - If Boot mode disable bits are programmed successfully.
 * 		- XNVM_EFUSE_ERR_RD_CACHE_BOOT_MODE_DIS_BITS - Error when read Boot mode disable bits from cache fails.
 * 		- XNVM_EFUSE_ERR_WRITE_QSPI24_BOOT_MODE_DIS - Error in QSPI24 boot mode disable programming.
 * 		- XNVM_EFUSE_ERR_WRITE_QSPI32_BOOT_MODE_DIS - Error in QSPI32 boot mode disable programming.
 * 		- XNVM_EFUSE_ERR_WRITE_OSPI_BOOT_MODE_DIS - Error in OSPI boot mode disable programming.
 * 		- XNVM_EFUSE_ERR_WRITE_SMAP_BOOT_MODE_DIS - Error in SMAP boot mode disable programming.
 * 		- XNVM_EFUSE_ERR_WRITE_SERIAL_BOOT_MODE_DIS - Error in SERIAL boot mode disable programming.
 *
 ******************************************************************************/
static int XNvm_EfusePrgmBootModeDisBits(const XNvm_EfuseBootModeDis *BootModeDis)
{
	int Status = XST_FAILURE;
	int StatusTmp = XST_FAILURE;
	u32 BootModeDisVal = 0U;

	/**
	 * - Read Boot mode disable bits from eFUSE cache before programming.
	 * Return error in case of failure.
	 */
	Status = XNvm_EfuseReadCache(XNVM_EFUSE_BOOT_MODE_DIS_OFFSET, &BootModeDisVal);
	if (Status != XST_SUCCESS) {
		Status = XNVM_EFUSE_ERR_RD_CACHE_BOOT_MODE_DIS_BITS;
		goto END;
	}

	/**
	 * - Program QSPI24 Boot mode disable bit when it's not already set and
	 * QSPI24 Boot mode disable Efuse programming is enabled
	 */
	if (((BootModeDis->PrgmQspi24ModDis) == (u32)TRUE) &&
	    ((BootModeDisVal & XNVM_EFUSE_QSPI24_BOOT_MODE_DIS_MASK) != XNVM_EFUSE_QSPI24_BOOT_MODE_DIS_MASK))
	{
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_BOOT_MODE_DIS_ROW_60,
				      XNVM_EFUSE_QSPI24_BOOT_MODE_DIS_COL, FALSE);

		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = Status | XNVM_EFUSE_ERR_WRITE_QSPI24_BOOT_MODE_DIS;
			goto END;
		}
	}

	/**
	 * - Program QSPI32 Boot mode disable bit when it's not already set and
	 * QSPI32 Boot mode disable Efuse programming is enabled.
	 */
	if (((BootModeDis->PrgmQspi32ModDis) == (u32)TRUE) &&
	    ((BootModeDisVal & XNVM_EFUSE_QSPI32_BOOT_MODE_DIS_MASK) != XNVM_EFUSE_QSPI32_BOOT_MODE_DIS_MASK))
	{
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_BOOT_MODE_DIS_ROW_61,
				      XNVM_EFUSE_QSPI32_BOOT_MODE_DIS_COL, FALSE);

		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = Status | XNVM_EFUSE_ERR_WRITE_QSPI32_BOOT_MODE_DIS;
			goto END;
		}
	}

	/**
	 * - Program OSPI Boot mode disable bit when it's not already set and
	 * OSPI Boot mode disable Efuse programming is enabled
	 */
	if (((BootModeDis->PrgmOspiModDis) == (u32)TRUE) &&
	    ((BootModeDisVal & XNVM_EFUSE_OSPI_BOOT_MODE_DIS_MASK) != XNVM_EFUSE_OSPI_BOOT_MODE_DIS_MASK))
	{
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_BOOT_MODE_DIS_ROW_61,
				      XNVM_EFUSE_OSPI_BOOT_MODE_DIS_COL, FALSE);

		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = Status | XNVM_EFUSE_ERR_WRITE_OSPI_BOOT_MODE_DIS;
			goto END;
		}
	}

	/**
	 * - Program JTAG Boot mode disable bit when it's not already set and
	 * JTAG Boot mode disable Efuse programming is enabled
	 */
	if (((BootModeDis->PrgmJtagModDis) == (u32)TRUE) &&
	    ((BootModeDisVal & XNVM_EFUSE_JTAG_BOOT_MODE_DIS_MASK) != XNVM_EFUSE_JTAG_BOOT_MODE_DIS_MASK))
	{
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_BOOT_MODE_DIS_ROW_61,
				      XNVM_EFUSE_JTAG_BOOT_MODE_DIS_COL, FALSE);

		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = Status | XNVM_EFUSE_ERR_WRITE_JTAG_BOOT_MODE_DIS;
			goto END;
		}
	}

	/**
	 * - Program SMAP Boot mode disable bit when it's not already set and
	 * SMAP Boot mode disable Efuse programming is enabled
	 */
	if (((BootModeDis->PrgmSmapModDis) == (u32)TRUE) &&
	    ((BootModeDisVal & XNVM_EFUSE_SMAP_BOOT_MODE_DIS_MASK) != XNVM_EFUSE_SMAP_BOOT_MODE_DIS_MASK))
	{
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_BOOT_MODE_DIS_ROW_61,
				      XNVM_EFUSE_SMAP_BOOT_MODE_DIS_COL, FALSE);

		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = Status | XNVM_EFUSE_ERR_WRITE_SMAP_BOOT_MODE_DIS;
			goto END;
		}
	}

	/**
	 * - Program SERIAL Boot mode disable bit when it's not already set and
	 * SERIAL Boot mode disable Efuse programming is enabled
	 */
	if (((BootModeDis->PrgmSerialModDis) == (u32)TRUE) &&
	    ((BootModeDisVal & XNVM_EFUSE_SERIAL_BOOT_MODE_DIS_MASK) != XNVM_EFUSE_SERIAL_BOOT_MODE_DIS_MASK))
	{
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_BOOT_MODE_DIS_ROW_61,
				      XNVM_EFUSE_SERIAL_BOOT_MODE_DIS_COL, FALSE);

		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = Status | XNVM_EFUSE_ERR_WRITE_SERIAL_BOOT_MODE_DIS;
			goto END;
		}
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function reads Boot mode disable fuses from eFUSE cache.
 *
 * @param	BootModeDisBits	Pointer to the Boot mode disable efuse data.
 *
 * @return
 * 		- XST_SUCCESS - If Boot mode disable efuses are read successfully.
 * 		- XNVM_EFUSE_ERR_INVALID_PARAM - Invalid input parameter.
 * 		- XNVM_EFUSE_ERR_RD_CACHE_BOOT_MODE_DIS_BITS - Error when reading Boot mode disable bits from cache fails.
 *
 ******************************************************************************/
int XNvm_EfuseReadBootModeDisBits(XNvm_EfuseBootModeDis *BootModeDisBits)
{
	int Status = XST_FAILURE;
	u32 BootModeDisVal = 0U;

	if (BootModeDisBits ==  NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	/**
	 * - Read Boot mode disable bits from eFUSE cache before programming.
	 * Return error in case of failure.
	 */
	Status = XNvm_EfuseReadCache(XNVM_EFUSE_BOOT_MODE_DIS_OFFSET, &BootModeDisVal);
	if (Status != XST_SUCCESS) {
		Status = XNVM_EFUSE_ERR_RD_CACHE_BOOT_MODE_DIS_BITS;
		goto END;
	}

	/**
	 * - Initialize XNvm_EfuseBootModeDis structure.
	 */
	BootModeDisBits->PrgmQspi24ModDis = XNVM_GET_8_BIT_VAL(BootModeDisVal, XNVM_EFUSE_BOOT_MODE_DISABLE_EFUSE_BITS,
							XNVM_EFUSE_BOOT_MODE_DIS_QSPI24_EFUSE_SHIFT);
	BootModeDisBits->PrgmQspi32ModDis = XNVM_GET_8_BIT_VAL(BootModeDisVal, XNVM_EFUSE_BOOT_MODE_DISABLE_EFUSE_BITS,
							XNVM_EFUSE_BOOT_MODE_DIS_QSPI32_EFUSE_SHIFT);
	BootModeDisBits->PrgmOspiModDis = XNVM_GET_8_BIT_VAL(BootModeDisVal, XNVM_EFUSE_BOOT_MODE_DISABLE_EFUSE_BITS,
							XNVM_EFUSE_BOOT_MODE_DIS_OSPI_EFUSE_SHIFT);
	BootModeDisBits->PrgmJtagModDis = XNVM_GET_8_BIT_VAL(BootModeDisVal, XNVM_EFUSE_BOOT_MODE_DISABLE_EFUSE_BITS,
							XNVM_EFUSE_BOOT_MODE_DIS_JTAG_EFUSE_SHIFT);
	BootModeDisBits->PrgmSmapModDis = XNVM_GET_8_BIT_VAL(BootModeDisVal, XNVM_EFUSE_BOOT_MODE_DISABLE_EFUSE_BITS,
							XNVM_EFUSE_BOOT_MODE_DIS_SMAP_EFUSE_SHIFT);
	BootModeDisBits->PrgmSerialModDis = XNVM_GET_8_BIT_VAL(BootModeDisVal, XNVM_EFUSE_BOOT_MODE_DISABLE_EFUSE_BITS,
							XNVM_EFUSE_BOOT_MODE_DIS_SERIAL_EFUSE_SHIFT);

	Status = XST_SUCCESS;

END:
	return Status;
}
#endif

/******************************************************************************/
/**
 * @brief	This function programs secure control bits
 *
 * @param	SecCtrl - Pointer to XNvm_EfuseSecCtrl.
 *
 * @return
 * 		- XST_SUCCESS - On successful programming.
 * 		- XNVM_EFUSE_ERR_WRITE_AES_DIS - Error in AES disable programming.
 * 		- XNVM_EFUSE_ERR_WRITE_JTAG_DIS - Error in JTAG disable programming.
 * 		- XNVM_EFUSE_ERR_WRITE_PPK2_WR_LCK - Error in PPK2 write lock programming.
 * 		- XNVM_EFUSE_ERR_WRITE_PPK1_WR_LCK - Error in PPK1 write lock programming.
 * 		- XNVM_EFUSE_ERR_WRITE_PPK0_WR_LCK - Error in PPK0 write lock programming.
 * 		- XNVM_EFUSE_ERR_WRITE_AES_RD_WR_LCK0 - Error in AES read/write lock 0 programming.
 * 		- XNVM_EFUSE_ERR_WRITE_AES_RD_WR_LCK1 - Error in AES read/write lock 1 programming.
 * 		- XNVM_EFUSE_ERR_WRITE_PPK2_INVLD_0 - Error in PPK2 invalid 0 programming.
 * 		- XNVM_EFUSE_ERR_WRITE_PPK2_INVLD_1 - Error in PPK2 invalid 1 programming.
 * 		- XNVM_EFUSE_ERR_WRITE_PPK1_INVLD_0 - Error in PPK1 invalid 0 programming.
 * 		- XNVM_EFUSE_ERR_WRITE_PPK1_INVLD_1 - Error in PPK1 invalid 1 programming.
 * 		- XNVM_EFUSE_ERR_WRITE_PPK0_INVLD_0 - Error in PPK0 invalid 0 programming.
 * 		- XNVM_EFUSE_ERR_WRITE_PPK0_INVLD_1 - Error in PPK0 invalid 1 programming.
 * 		- XNVM_EFUSE_ERR_WRITE_PUF_TEST2_DIS - Error in PUF test2 disable programming.
 * 		- XNVM_EFUSE_ERR_WRITE_RMA_ENABLE_0 - Error in RMA enable 0 programming.
 * 		- XNVM_EFUSE_ERR_WRITE_RMA_ENABLE_1 - Error in RMA enable 1 programming.
 * 		- XNVM_EFUSE_ERR_WRITE_DFT_DIS_0 - Error in DFT disable 0 programming.
 * 		- XNVM_EFUSE_ERR_WRITE_DFT_DIS_1 - Error in DFT disable 1 programming.
 * 		- XNVM_EFUSE_ERR_WRITE_CRC_EN - Error in CRC enable programming.
 * 		- XNVM_EFUSE_ERR_WRITE_HASH_PUF_OR_KEY - Error in hash PUF or key programming.
 * 		- XNVM_EFUSE_ERR_WRITE_USER_WR_LK - Error in user write lock programming.
 * 		- XNVM_EFUSE_ERR_WRITE_JTAG_ERR_OUT_DIS - Error in JTAG error output disable programming.
 *
 ******************************************************************************/
static int XNvm_EfusePrgmSecCtrlBits(const XNvm_EfuseSecCtrlBits *SecCtrl)
{
	int Status = XST_FAILURE;
	int StatusTmp = XST_FAILURE;
	u32 SecCtrlVal = 0U;
	u32 SecCtrlCrcVal = 0U;

	/** - Read current security control eFUSE cache register to check already programmed bits */
	Status = XNvm_EfuseReadCache(XNVM_EFUSE_CONTROL_OFFSET, &SecCtrlVal);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (SecCtrl->ScanClearEn == (u32)TRUE) {
		if ((SecCtrlVal & XNVM_EFUSE_SEC_CTRL_SCAN_CLEAR_EN_MASK) != XNVM_EFUSE_SEC_CTRL_SCAN_CLEAR_EN_MASK) {
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_0,
				XNVM_EFUSE_SEC_CTRL_SCAN_CLEAR_EN_COL, FALSE);
			if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
				Status = Status | XNVM_EFUSE_ERR_WRITE_SCAN_CLEAR_EN;
				goto END;
			}
		}
	}
	if (SecCtrl->AesDis == (u32)TRUE) {
		if ((SecCtrlVal & XNVM_EFUSE_SEC_CTRL_AES_DIS_MASK) != XNVM_EFUSE_SEC_CTRL_AES_DIS_MASK) {
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_3,
				XNVM_EFUSE_SEC_CTRL_AES_DIS_COL, FALSE);
			if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
				Status = Status | XNVM_EFUSE_ERR_WRITE_AES_DIS;
				goto END;
			}
		}
	}

	if (SecCtrl->RmaDis == (u32)TRUE) {
		if ((SecCtrlVal & XNVM_EFUSE_SEC_CTRL_RMA_DISABLE_0_MASK) != XNVM_EFUSE_SEC_CTRL_RMA_DISABLE_0_MASK) {
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_0,
				XNVM_EFUSE_SEC_CTRL_RMA_DISABLE_0_COL, FALSE);
			if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
				Status = Status | XNVM_EFUSE_ERR_WRITE_RMA_DISABLE_0;
				goto END;
			}
		}
	}

	if (SecCtrl->RmaEn == (u32)TRUE) {
		if ((SecCtrlVal & XNVM_EFUSE_SEC_CTRL_RMA_ENABLE_0_MASK) != XNVM_EFUSE_SEC_CTRL_RMA_ENABLE_0_MASK) {
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_0,
				XNVM_EFUSE_SEC_CTRL_RMA_ENABLE_0_COL, FALSE);
			if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
				Status = Status | XNVM_EFUSE_ERR_WRITE_RMA_ENABLE_0;
				goto END;
			}
		}
	}

	if (SecCtrl->JtagDis == (u32)TRUE) {
		if ((SecCtrlVal & XNVM_EFUSE_SEC_CTRL_JTAG_DIS_MASK) != XNVM_EFUSE_SEC_CTRL_JTAG_DIS_MASK) {
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_3,
				XNVM_EFUSE_SEC_CTRL_JTAG_DIS_COL, FALSE);
			if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
				Status = Status | XNVM_EFUSE_ERR_WRITE_JTAG_DIS;
				goto END;
			}
		}
	}

	if (SecCtrl->PufTes2Dis == (u32)TRUE) {
		if ((SecCtrlVal & XNVM_EFUSE_SEC_CTRL_PUF_TEST2_DIS_MASK) != XNVM_EFUSE_SEC_CTRL_PUF_TEST2_DIS_MASK) {
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_2,
				XNVM_EFUSE_SEC_CTRL_PUF_TEST2_DIS_COL, FALSE);
			if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
				Status = Status | XNVM_EFUSE_ERR_WRITE_PUF_TEST2_DIS;
				goto END;
			}
		}
	}

	if (SecCtrl->HashPufOrKey == (u32)TRUE) {
		if ((SecCtrlVal & XNVM_EFUSE_SEC_CTRL_HASH_PUF_OR_KEY_MASK) != XNVM_EFUSE_SEC_CTRL_HASH_PUF_OR_KEY_MASK) {
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_0,
				XNVM_EFUSE_SEC_CTRL_HASH_PUF_OR_KEY_COL, FALSE);
			if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
				Status = Status | XNVM_EFUSE_ERR_WRITE_HASH_PUF_OR_KEY;
				goto END;
			}
		}
	}

	if (SecCtrl->Ppk0lck == (u32)TRUE) {
		if ((SecCtrlVal & XNVM_EFUSE_SEC_CTRL_PPK0_WR_LK_MASK) != XNVM_EFUSE_SEC_CTRL_PPK0_WR_LK_MASK) {
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_3,
				XNVM_EFUSE_SEC_CTRL_PPK0_WR_LK_COL, FALSE);
			if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
				Status = Status | XNVM_EFUSE_ERR_WRITE_PPK0_WR_LCK;
				goto END;
			}
		}
	}

	if (SecCtrl->Ppk1lck == (u32)TRUE) {
		if ((SecCtrlVal & XNVM_EFUSE_SEC_CTRL_PPK1_WR_LK_MASK) != XNVM_EFUSE_SEC_CTRL_PPK1_WR_LK_MASK) {
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_3,
				XNVM_EFUSE_SEC_CTRL_PPK1_WR_LK_COL, FALSE);
			if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
				Status = Status | XNVM_EFUSE_ERR_WRITE_PPK1_WR_LCK;
				goto END;
			}
		}
	}

	if (SecCtrl->Ppk0Invld == (u32)TRUE) {
		if ((SecCtrlVal & XNVM_EFUSE_SEC_CTRL_PPK0_INVLD0_MASK) != XNVM_EFUSE_SEC_CTRL_PPK0_INVLD0_MASK) {
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_2,
				XNVM_EFUSE_SEC_CTRL_PPK0_INVLD_0_COL, FALSE);
			if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
				Status = Status | XNVM_EFUSE_ERR_WRITE_PPK0_INVLD_0;
				goto END;
			}
		}

		if ((SecCtrlVal & XNVM_EFUSE_SEC_CTRL_PPK0_INVLD1_MASK) != XNVM_EFUSE_SEC_CTRL_PPK0_INVLD1_MASK) {
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_2,
				XNVM_EFUSE_SEC_CTRL_PPK0_INVLD_1_COL, FALSE);
			if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
				Status = Status | XNVM_EFUSE_ERR_WRITE_PPK0_INVLD_1;
				goto END;
			}
		}
	}

	if (SecCtrl->Ppk1Invld == (u32)TRUE) {
		if ((SecCtrlVal & XNVM_EFUSE_SEC_CTRL_PPK1_INVLD0_MASK) != XNVM_EFUSE_SEC_CTRL_PPK1_INVLD0_MASK) {
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_2,
				XNVM_EFUSE_SEC_CTRL_PPK1_INVLD_0_COL, FALSE);
			if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
				Status = Status | XNVM_EFUSE_ERR_WRITE_PPK1_INVLD_0;
				goto END;
			}
		}

		if ((SecCtrlVal & XNVM_EFUSE_SEC_CTRL_PPK1_INVLD1_MASK) != XNVM_EFUSE_SEC_CTRL_PPK1_INVLD1_MASK) {
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_2,
				XNVM_EFUSE_SEC_CTRL_PPK1_INVLD_1_COL, FALSE);
			if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
				Status = Status | XNVM_EFUSE_ERR_WRITE_PPK1_INVLD_1;
				goto END;
			}
		}
	}

#ifndef SPARTANUPLUSAES1
	if (SecCtrl->Ppk2lck == (u32)TRUE) {
		if ((SecCtrlVal & XNVM_EFUSE_SEC_CTRL_PPK2_WR_LK_MASK) != XNVM_EFUSE_SEC_CTRL_PPK2_WR_LK_MASK) {
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_3,
				XNVM_EFUSE_SEC_CTRL_PPK2_WR_LK_COL, FALSE);
			if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
				Status = Status | XNVM_EFUSE_ERR_WRITE_PPK2_WR_LCK;
				goto END;
			}
		}
	}

	if (SecCtrl->Ppk2Invld == (u32)TRUE) {
		if ((SecCtrlVal & XNVM_EFUSE_SEC_CTRL_PPK2_INVLD0_MASK) != XNVM_EFUSE_SEC_CTRL_PPK2_INVLD0_MASK) {
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_2,
				XNVM_EFUSE_SEC_CTRL_PPK2_INVLD_0_COL, FALSE);
			if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
				Status = Status | XNVM_EFUSE_ERR_WRITE_PPK2_INVLD_0;
				goto END;
			}
		}

		if ((SecCtrlVal & XNVM_EFUSE_SEC_CTRL_PPK2_INVLD1_MASK) != XNVM_EFUSE_SEC_CTRL_PPK2_INVLD1_MASK) {
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_2,
				XNVM_EFUSE_SEC_CTRL_PPK2_INVLD_1_COL, FALSE);
			if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
				Status = Status | XNVM_EFUSE_ERR_WRITE_PPK2_INVLD_1;
				goto END;
			}
		}
	}
#endif
	if (SecCtrl->AesRdlk == (u32)TRUE) {
		if ((SecCtrlVal & XNVM_EFUSE_SEC_CTRL_AES_RD_WR_LK_0_MASK) != XNVM_EFUSE_SEC_CTRL_AES_RD_WR_LK_0_MASK){
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_3,
				XNVM_EFUSE_SEC_CTRL_AES_RD_WR_LK_0_COL, FALSE);
			if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
				Status = Status | XNVM_EFUSE_ERR_WRITE_AES_RD_WR_LCK0;
				goto END;
			}
		}

	    if ((SecCtrlVal & XNVM_EFUSE_SEC_CTRL_AES_RD_WR_LK_1_MASK) != XNVM_EFUSE_SEC_CTRL_AES_RD_WR_LK_1_MASK) {
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_3,
				XNVM_EFUSE_SEC_CTRL_AES_RD_WR_LK_1_COL, FALSE);
			if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
				Status = Status | XNVM_EFUSE_ERR_WRITE_AES_RD_WR_LCK1;
				goto END;
			}
		}
	}

	if (SecCtrl->JtagErrDis == (u32)TRUE) {
		if ((SecCtrlVal & XNVM_EFUSE_SEC_CTRL_JTAG_ERR_OUT_DIS_MASK) != XNVM_EFUSE_SEC_CTRL_JTAG_ERR_OUT_DIS_MASK) {
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_4,
				XNVM_EFUSE_SEC_CTRL_JTAG_ERR_OUT_DIS_COL, FALSE);

			if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
				Status = Status | XNVM_EFUSE_ERR_WRITE_JTAG_ERR_OUT_DIS;
				goto END;
			}
		}
	}

	if (SecCtrl->UserWrlk == (u32)TRUE) {
		if ((SecCtrlVal & XNVM_EFUSE_SEC_CTRL_USER_WR_LK_MASK) != XNVM_EFUSE_SEC_CTRL_USER_WR_LK_MASK) {
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_4,
				XNVM_EFUSE_SEC_CTRL_USER_WR_LK_COL, FALSE);

			if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
				Status = Status | XNVM_EFUSE_ERR_WRITE_USER_WR_LK;
				goto END;
			}
		}
	}

	/** - Read CRC enable and DFT disable eFUSE cache register to check current state */
	Status = XNvm_EfuseReadCache(XNVM_EFUSE_CRC_EN_OFFSET, &SecCtrlCrcVal);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (SecCtrl->CrcEn == (u32)TRUE) {
		if((SecCtrlCrcVal & XNVM_EFUSE_SEC_CTRL_CRC_EN_MASK) != XNVM_EFUSE_SEC_CTRL_CRC_EN_MASK) {
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_1,
				XNVM_EFUSE_SEC_CTRL_CRC_EN_COL, FALSE);
			if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
				Status = Status | XNVM_EFUSE_ERR_WRITE_CRC_EN;
				goto END;
			}
		}
	}

	if (SecCtrl->DftDis == (u32)TRUE) {
		if ((SecCtrlCrcVal & XNVM_EFUSE_SEC_CTRL_DFT_DIS_0_MASK) != XNVM_EFUSE_SEC_CTRL_DFT_DIS_0_MASK) {
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_1,
				XNVM_EFUSE_SEC_CTRL_DFT_DISABLE_0_COL, FALSE);
			if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
				Status = Status | XNVM_EFUSE_ERR_WRITE_DFT_DIS_0;
				goto END;
			}
		}

		if ((SecCtrlCrcVal & XNVM_EFUSE_SEC_CTRL_DFT_DIS_1_MASK) != XNVM_EFUSE_SEC_CTRL_DFT_DIS_1_MASK) {
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_1,
				XNVM_EFUSE_SEC_CTRL_DFT_DISABLE_1_COL, FALSE);
			if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
				Status = Status | XNVM_EFUSE_ERR_WRITE_DFT_DIS_1;
				goto END;
			}
		}
	}

	if (SecCtrl->Lckdwn == (u32)TRUE) {
		if ((SecCtrlCrcVal & XNVM_EFUSE_SEC_CTRL_LCKDOWN_MASK) != XNVM_EFUSE_SEC_CTRL_LCKDOWN_MASK) {
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_1,
				XNVM_EFUSE_SEC_CTRL_LCKDOWN_COL, FALSE);
			if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
				Status = Status | XNVM_EFUSE_ERR_WRITE_LCK_DWN;
				goto END;
			}
		}
	}

	if (SecCtrl->CrcRmaDis == (u32)TRUE) {
		if ((SecCtrlVal & XNVM_EFUSE_SEC_CTRL_RMA_DISABLE_1_MASK) != XNVM_EFUSE_SEC_CTRL_RMA_DISABLE_1_MASK) {
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_1,
				XNVM_EFUSE_SEC_CTRL_RMA_DISABLE_1_COL, FALSE);

			if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
				Status = Status | XNVM_EFUSE_ERR_WRITE_RMA_DISABLE_1;
				goto END;
			}
		}
	}

	if (SecCtrl->CrcRmaEn == (u32)TRUE) {
	    if ((SecCtrlVal & XNVM_EFUSE_SEC_CTRL_RMA_ENABLE_1_MASK) != XNVM_EFUSE_SEC_CTRL_RMA_ENABLE_1_MASK) {
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmAndVerifyBit, XNVM_EFUSE_SEC_CTRL_ROW_1,
				XNVM_EFUSE_SEC_CTRL_RMA_ENABLE_1_COL, FALSE);

			if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
				Status = Status | XNVM_EFUSE_ERR_WRITE_RMA_ENABLE_1;
				goto END;
			}
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
 * @return
 * 		- XST_SUCCESS - if the eFuse data computation is successful.
 * 		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 * 		- XNVM_EFUSE_ERR_CACHE_PARITY  - Error in Cache reload.
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

	/** - Validate input pointers before computing programmable bits */
	if ((ReqData == NULL) || (PrgmData == NULL)) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	/** - Check ISR for cache parity error before reading eFUSE cache values */
	IsrStatus = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
				      XNVM_EFUSE_ISR_OFFSET);
	if ((IsrStatus & XNVM_EFUSE_ISR_CACHE_ERROR)
	    == XNVM_EFUSE_ISR_CACHE_ERROR) {
		Status = XNVM_EFUSE_ERR_CACHE_PARITY;
		goto END;
	}

	/** - Read cache at each offset and compute bits not yet programmed by masking existing values */
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
 * @return
 * 		- XST_SUCCESS - on successful cache reload.
 * 		- XNVM_EFUSE_ERR_CACHE_LOAD - Error while loading the cache.
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
	 *  - Write 1 to load bit of eFuse_CACHE_LOAD register.
	 *  Wait for CACHE_DONE bit to set in EFUSE_STATUS register . If timed out return timeout error.
	 *  Return XST_SUCCESS
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
	 *  - Read EFUSE_ISR_REG. If EFUSE_ISR_CHACE_ERROR set return cache load error.
	 *  Return XST_SUCCESS.
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
	 *  - Reset EFUSE_ISR_CACHE_ERROR bit to 1
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
 * @return
 * 		- XST_SUCCESS - Specified bit set in eFUSE.
 * 		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
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

	/** - Validate input parameters before starting eFUSE bit programming */
	if ((DataPtr == NULL) || (EfusePrgmInfo->NumOfRows == 0U)) {

		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	/** - Iterate through each row and column, programming only the set bits to eFUSE */
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

	/** - Verify all expected rows were programmed; return glitch error if row count mismatches */
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
 * @return
 * 		- XST_SUCCESS - Specified bit set in eFUSE.
 * 		- XNVM_EFUSE_ERR_PGM_TIMEOUT - eFUSE programming timed out.
 * 		- XNVM_EFUSE_ERR_PGM - eFUSE programming failed.
 *
 ******************************************************************************/
static int XNvm_EfusePgmBit(u32 Row, u32 Col)
{
	volatile int Status = XST_FAILURE;
	u32 PgmAddr;
	u32 EventMask = 0U;

	/** - Compute eFUSE programming address from the given row and column numbers */
	PgmAddr = (Row << XNVM_EFUSE_ADDR_ROW_SHIFT) |
		  (Col << XNVM_EFUSE_ADDR_COLUMN_SHIFT);

	/** - Write programming address to trigger the bit programming operation */
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
			   XNVM_EFUSE_PGM_ADDR_OFFSET, PgmAddr);

	/** - Wait for programming done or error event, then return appropriate status */
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
 * @return
 * 		- XST_SUCCESS - Specified bit set in eFUSE.
 * 		- XNVM_EFUSE_ERR_PGM_VERIFY - Verification failed, specified bit
 * 				is not set.
 * 		- XNVM_EFUSE_ERR_PGM_TIMEOUT - If Programming timeout has occurred.
 * 		- XST_FAILURE - Unexpected error.
 *
 ******************************************************************************/
static int XNvm_EfuseReadRow(u32 Row, u32 *RegData)
{
	volatile int Status = XST_FAILURE;
	u32 RdAddr;
	u32 EventMask = 0x00U;

	/** - Compute the read address from the row number and trigger the eFUSE read operation */
	RdAddr = Row << XNVM_EFUSE_ADDR_ROW_SHIFT;

	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
			   XNVM_EFUSE_RD_ADDR_OFFSET, RdAddr);

	/** - Wait for read done event and retrieve data from read data register */
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
 * @return
 * 		- XST_SUCCESS - Specified bit set in eFUSE.
 * 		- XNVM_EFUSE_ERR_PGM_VERIFY - Verification failed, specified bit
 * 				is not set.
 * 		- XNVM_EFUSE_ERR_PGM_TIMEOUT - If Programming timeout has occurred.
 * 		- XST_FAILURE - Unexpected error.
 *
 ******************************************************************************/
static int XNvm_EfuseVerifyBit(u32 Row, u32 Col)
{
	int Status = XST_FAILURE;
	volatile u32 RegData = 0x00U;

	/** - Read the eFUSE row to get current bit values for verification */
	Status = XNvm_EfuseReadRow(Row, (u32 *)&RegData);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** - Check if the specified column bit is set in the read row data */
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
 * @return
 * 		- XST_SUCCESS - Specified bit set in eFUSE.
 * 		- XNVM_EFUSE_ERR_PGM_TIMEOUT - eFUSE programming timed out.
 * 		- XNVM_EFUSE_ERR_PGM - eFUSE programming failed.
 * 		- XNVM_EFUSE_ERR_PGM_VERIFY - Verification failed, specified bit
 * 				is not set.
 * 		- XST_FAILURE - Unexpected error.
 *
 ******************************************************************************/
static int XNvm_EfusePgmAndVerifyBit(u32 Row, u32 Col, u32 SkipVerify)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	volatile u32 SkipVerifyTmp = SkipVerify;

	/** - Program the specified eFUSE bit using temporal implementation to detect glitches */
	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmBit, Row, Col);
	if ((Status == XST_SUCCESS) || (StatusTmp == XST_SUCCESS)) {
		/** - Verify the programmed bit if verification is not explicitly skipped */
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
 * @brief	This function reads 32-bit data from cache specified by Row.
 *
 * @param	Offset 	- Offset from which data should be read (0-based addressing).
 * @param	RowData	- Pointer to memory location where read 32-bit row data
 *					  is to be stored.
 *
 * @return
 * 		- XST_SUCCESS - Specified data read.
 * 		- XNVM_EFUSE_ERR_CACHE_PARITY - Parity Error exist in cache.
 *
 ******************************************************************************/
static int XNvm_EfuseReadCache(u32 Offset, u32 *RowData)
{
	int Status = XST_FAILURE;
	u32 CacheData;
	u32 IsrStatus;

	/** - Validate output pointer before reading eFUSE cache */
	if (RowData == NULL) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	/** - Read cache data at specified offset and check ISR for cache parity error */
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
 * @return
 * 		- XST_SUCCESS - Specified data read.
 *		- XNVM_EFUSE_ERR_CACHE_PARITY - Parity Error exist in cache.
 *
 ******************************************************************************/
static int XNvm_EfuseReadCacheRange(u32 StartOffset, u8 OffsetCount, u32 *RowData)
{
	volatile int Status = XST_FAILURE;
	u32 Row = StartOffset;
	u32 Count;
	u32 *Data = RowData;

	/** - Read consecutive cache offsets and store results into the output buffer */
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
 * @brief	This function is used verify eFUSEs for Zeros.
 *
 * @param	OffsetStart - Row number from which verification has to be started.
 * @param	OffsetEnd   - Row number till which verification has to be ended.
 *
 * @return
 * 		- XST_SUCCESS - if efuses are not programmed.
 * 		- XST_FAILURE - if efuses are already programmed.
 *
 ******************************************************************************/
static int XNvm_EfuseCheckZeros(u32 OffsetStart, u32 OffsetEnd)
{
	volatile int Status = XST_FAILURE;
	u32 Row;
	u32 RowDataVal = 0x0U;

	/** - Read each eFUSE cache row in the given range and verify each row is all zeros */
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
