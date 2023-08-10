/*******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/


/******************************************************************************/
/**
*
* @file versal/server/xnvm_efuse.c
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
* 2.0   kal  02/27/2020 Added eFuse wrapper APIs to program AES keys, PPK hash,
*                       Revocation ID, SecCtrl eFuses, Puf HD and APIs to read
*                       eFuse Cache values.
*       kal  03/03/2020 Added protection eFuse row programming.
*       kal  05/04/2020 Restructured the code to have single interface
*       		XNvm_EfuseWrite for eFuse programming.
*       		Removed usage of conversion APIs and achieved the same
*       		logic with bitwise operators.
*       		Reduced number of Cache reloads.
* 2.1   rpo  06/06/2020 Support added to write glitch configuration data.
*       rpo  06/08/2020 Support added to program eFUSE halt boot bits to stop
*                       at ROM stage.
*       kal  07/09/2020 Fixed bugs in PUF Syndrome data read, ROW_58 protection
*                       Row programming.
*                       Read provision is expanded to all Misc Ctrl eFuses.
*	am   08/19/2020 Resolved MISRA C violations.
*	kal  09/03/2020 Fixed Security CoE review comments
*	am   10/13/2020 Resolved MISRA C violations
* 2.2   am   11/23/2020 Resolved MISRA C and Coverity warnings
* 	kal  12/23/2020 Removed unused variables
*	kal  01/07/2021	Added support to SecurityMisc1, BootEnvCtrl,MiscCtrl
*			and remaining eFuses in SecCtrl eFuse rows programming
*			and reading
*	kal  01/25/2021	Initialized variables to more secure state
*	kal  01/25/2021 Fix cache logic error in XNvm_EfuseReadCacheRange API
* 2.3   kal  02/22/2021 Add redundancy to loop in XNvm_EfusePgmAndVerifyRows
*	kal  02/26/2021 Fix all SW-BP-ZEROIZE related review comments
*	kal  03/02/2021 Add Environmental monitoring support before eFuse
*			programming
*	kal  04/08/2021 Fix SW-BP-INPUT-VALID related review comments.
*	kal  04/14/2021 Add a validation check for RowCount in
*			XNvm_EfusePgmAndVerifyRows API
*	kal  04/15/2021 Fix XNvm_EfuseSetReadMode - Blind writes
*	kal  04/27/2021 Reset status to failure when status is used more than
*			once in security critical places
*	kal  04/30/2021 Added a warning for user to re-boot the system when
*			CACHE_ERROR
*       kpt  05/06/2021 Corrected check to program SafetyMissionEn bit
*	kal  05/07/2021 Reset the read mode after eFuse operations are done
*       kpt  05/12/2021 Added check to set environmental disable flag and
*                       sysmon instance for individual write API's
*	kpt  05/20/2021 Added support for programming PUF efuses as
*                       general purpose data
*       am   05/22/2021 Resolved MISRA C violations
* 2.4   kal  07/13/2021 Fixed doxygen warnings
*       am   07/07/2021 Fixed 2021.1 review comments
*       kal  07/25/2021 Moved XNvm_Printf definition to xnvm_def.h
*       kpt  08/03/2021 Added XNvm_EfuseResetReadMode in
*                       XNvm_EfuseWritePufAsUserFuses
*       kal  08/13/2021 Add most restrictive range check for device temparature
*                       before eFuse programming
*       har  09/16/2021 Removed magic numbers in XNvm_EfuseWriteSecCtrl function
*                       Check for Trim2 instead of Trim3 before programming
*                       Protection bit 37
* 2.5   har  11/17/2021 Fix Blind Write in XNvm_EfuseDisableProgramming and
*                       XNvm_EfuseResetReadMode
*       har  01/03/2022 Renamed NumOfPufFuses as NumOfPufFusesRows
*       har  11/22/2021 Added an error case and full range check case for switch
*                       in XNvm_EfuseTemparatureCheck and XNvm_EfusePmcVoltageCheck
*                       Added checks for return value for
*                       XNvm_EfuseDisableProgramming() and XNvm_EfuseResetReadMode()
*       am   02/28/2022 Resolved MISRA C violations
* 3.0   kal  07/12/2022 Moved common code to xnvm_efuse_common.c in common directory
*       kal  08/02/2022 Fix Row37 protection check in XNvm_EfuseProtectionChecks API
*       kpt  08/03/2022 Added volatile keyword to avoid compiler optimization of loop redundancy check
*       dc   08/29/2022 Changed u8 to u32 type
* 3.1   skg  10/25/2022 Added in body comments for APIs
*       skg  12/07/2022 Added Additional PPKs hash and invalid support
*       kal  03/07/2023 Added volatile keyword to avoid compiler optimization in
*                       XNvm_EfuseWriteSecMisc1Fuses
* 3.2   kum 04/11/2023  Moved common code to xnvm_efuse_common.c
*	kpt 07/26/2023  Add missing else check in XNvm_EfuseReadPpkHash
*
* </pre>
*
* @note
*
*******************************************************************************/

/***************************** Include Files **********************************/
#include "sleep.h"
#include "xil_util.h"
#include "xil_io.h"
#include "xnvm_efuse.h"
#include "xnvm_efuse_hw.h"
#include "xnvm_efuse_common_hw.h"
#include "xnvm_utils.h"
#include "xstatus.h"

/*************************** Constant Definitions *****************************/
/**
 * @name eFuse macro definitions
 */
/**< CRC for Aes zero key */
#define XNVM_EFUSE_CRC_AES_ZEROS		(0x6858A3D5U)
/**< Efuse total number of rows */
#define XNVM_EFUSE_TOTAL_NUM_OF_ROWS		(768U)
/**< PPK hash number of eFuse rows */
#define XNVM_EFUSE_TOTAL_PPK_HASH_ROWS  (XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS * 3U)
/**< eFuse word length */
#define XNVM_EFUSE_WORD_LEN			(4U)
/**< Default secure value for 32 bit */
#define XNVM_EFUSE_SEC_DEF_VAL_ALL_BIT_SET	(0xFFFFFFFFU)
/**< Sysmon VCCPMC Amux ctrl value */
#define XNVM_EFUSE_SYSMON_VCCPMC_AMUX_CTRL	(0x0bU)
/**< Sysmon VCCPMC Abus sw1 value */
#define XNVM_EFUSE_SYSMON_VCCPMC_ABUS_SW1	(0x00U)
/**< Sysmon VCCPMC Abus sw0 value*/
#define XNVM_EFUSE_SYSMON_VCCPMC_ABUS_SW0	(0x02U)
/**< Sysmon VCCPMC mode value */
#define XNVM_EFUSE_SYSMON_VCCPMC_MODE		(0x00U)
/**< Sysmon number of measurement registers */
#define XNVM_EFUSE_SYSMON_NUM_MEASURE_REGS	(0x20U)
/**< Sysmon number of supplies per flag */
#define XNVM_EFUSE_SYSMON_NUM_SUPPLIES_PER_FLAG	(32U)
/**< Sysmon Psv timeout value */
#define XNVM_EFUSE_SYSMONPSV_TIMEOUT		(100000U)
/**< Fraction multiplier value */
#define XNVM_EFUSE_FRACTION_MUL_VALUE		(1000000U)

#define XNVM_NUM_OF_CACHE_ADDR_PER_PAGE		(0x400U)
/**< PPK3 and PPK4 enable bits mask*/
#define XNVM_EFUSE_PPK_3_PPK_4_ENABLE       (0x00030000U)
/**< Total number of PPKS */
#define XNVM_EFUSE_NUM_OF_PPKS				(3U)
/** @} */

/***************************** Type Definitions *******************************/

/****************** Macros (Inline Functions) Definitions *********************/

/*************************** Function Prototypes ******************************/
static int XNvm_EfuseReadCache(u32 Row, u32* RowData);
static int XNvm_EfuseReadCacheRange(u32 StartRow, u8 RowCount, u32* RowData);
static int XNvm_EfusePgmBit(XNvm_EfuseType Page, u32 Row, u32 Col);
static int XNvm_EfuseVerifyBit(XNvm_EfuseType Page, u32 Row, u32 Col);
static int XNvm_EfusePgmAndVerifyBit(XNvm_EfuseType Page, u32 Row, u32 Col);
static int XNvm_EfuseCacheLoadAndProtectionChecks(void);
static int XNvm_EfusePgmAndVerifyRows(u32 StartRow, u32 RowCount,
			XNvm_EfuseType EfuseType, const u32* RowData);
static int XNvm_EfuseCheckZeros(u32 RowStart, u32 RowEnd);
static int XNvm_EfuseComputeProgrammableBits(const u32 *ReqData, u32 *PrgmData,
                                                u32 StartRow, u32 EndRow);
static int XNvm_EfuseValidateWriteReq(const XNvm_EfuseData *WriteChecks);
static int XNvm_EfuseValidateIVsWriteReq(const XNvm_EfuseIvs *EfuseIv);
static int XNvm_EfuseValidateAesWriteReq(const XNvm_EfuseAesKeys *Keys);
static int XNvm_EfuseValidatePpkWriteReq(const XNvm_EfusePpkHash *Hash);
static int XNvm_EfuseValidateUserFusesWriteReq(const XNvm_EfuseUserData *WriteUserFuses);
static int XNvm_EfuseValidateDecOnlyWriteReq(const XNvm_EfuseData *WriteReq);
static int XNvm_EfuseValidateIV(const u32 *Iv, u32 Row);
static int XNvm_ValidateBootEnvCtrlWriteReq(
				const XNvm_EfuseBootEnvCtrlBits *BootEnvCtrl);
#ifndef XNVM_ACCESS_PUF_USER_DATA
static int XNvm_EfuseWritePufAux(u32 Aux);
static int XNvm_EfuseWritePufChash(u32 Chash);
static int XNvm_EfuseWritePufSynData(const u32 *SynData);
static int XNvm_EfuseWritePufSecCtrl(const XNvm_EfusePufSecCtrlBits *PufSecCtrlBits);
static int XNvm_EfuseIsPufHelperDataEmpty(void);
#endif
static int XNvm_EfuseWriteSecCtrl(const XNvm_EfuseSecCtrlBits *SecCtrl);
static int XNvm_EfusePrgmGlitchCfgValues(const XNvm_EfuseGlitchCfgBits *WriteGlitchCfg);
static int XNvm_EfusePrgmGlitchWriteLock(const XNvm_EfuseGlitchCfgBits *WriteGlitchCfg);
static int XNvm_EfusePrgmGdRomMonEn(const XNvm_EfuseGlitchCfgBits *WriteGlitchCfg);
static int XNvm_EfusePrgmGdRomHaltBootEn(const XNvm_EfuseGlitchCfgBits *WriteGlitchCfg);
static int XNvm_EfusePrgmAesKeys(const XNvm_EfuseAesKeys *Keys);
static int XNvm_EfusePrgmPpkHash(const XNvm_EfusePpkHash *Hash);
static int XNvm_EfusePrgmDecOnly(const XNvm_EfuseDecOnly *DecOnly);
static int XNvm_EfusePrgmUserFuses(const XNvm_EfuseUserData *WriteUserFuses);
static int XNvm_EfusePrgmIVs(const XNvm_EfuseIvs *Ivs);
static int XNvm_EfusePrgmPpkRevokeFuses(const XNvm_EfuseMiscCtrlBits *PpkSelect);
static int XNvm_EfusePrgmHaltBootonError(const XNvm_EfuseMiscCtrlBits *MiscCtrlData);
static int XNvm_EfusePrgmHaltBootEnvError(const XNvm_EfuseMiscCtrlBits *MiscCtrlData);
static int XNvm_EfusePrgmRevocationIdFuses(const XNvm_EfuseRevokeIds *RevokeIds);
static int XNvm_EfusePrgmCryptoKatEn(const XNvm_EfuseMiscCtrlBits *MiscCtrlData);
static int XNvm_EfusePrgmLbistEn(const XNvm_EfuseMiscCtrlBits *MiscCtrlData);
static int XNvm_EfusePrgmSafetyMissionEn(const XNvm_EfuseMiscCtrlBits *MiscCtrlData);
static int XNvm_EfusePrgmProtectionEfuse(void);
static int XNvm_EfusePrgmOffChipRevokeFuses(const XNvm_EfuseOffChipIds *OffChipIds);
static int XNvm_EfuseWriteBootEnvCtrl(const XNvm_EfuseBootEnvCtrlBits *BootEnvCtrl);
static int XNvm_EfuseWriteSecMisc1Fuses(const XNvm_EfuseSecMisc1Bits *Misc1Bits);
static int XNvm_EfuseIsAdditionalPpkEn(void);
#ifdef XNVM_ACCESS_PUF_USER_DATA
static int XNvm_EfusePrgmPufFuses(const XNvm_EfusePufFuse *WritePufFuses);
#endif
#ifdef XNVM_EN_ADD_PPKS
static int XNvm_EfusePrgmAdditionalPpkHash(const XNvm_EfuseAdditionalPpkHash *Hash);
static int XNvm_EfusePrgmAdditionalPpksMiscBits(const XNvm_EfuseMiscCtrlBits *WriteReq);
static int XNvm_EfuseValidateAdditionalPpkWriteReq(const XNvm_EfuseAdditionalPpkHash *WriteReq);
#endif
/*************************** Variable Definitions *****************************/

/*************************** Function Definitions *****************************/

/******************************************************************************/
/**
 * @brief	This function is used as a wrapper to program below eFuses
 * 		AES key
 * 		User key 0
 * 		User key 1
 * 		PPK0/PPK1/PPK2/PPK3/PPK4 hash
 * 		IVs
 * 		Revocation Ids
 * 		User Fuses
 * 		Secure and Control bits.
 * 		Miscellaneous Control bits.
 * 		Security Miscellaneous1 bits.
 * 		BootEnvCtrl bits.
 *
 * @param	WriteNvm - Pointer to Efuse data to be written.
 *
 * @return	- XST_SUCCESS - On Specified data write.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM	- On Invalid Parameter.
 *		- XNVM_EFUSE_ERR_NTHG_TO_BE_PROGRAMMED	- If nothing is
 *							programmed.
 *		- XNVM_EFUSE_ERR_BEFORE_PROGRAMMING - If error before
 *							programming.
 *		- XNVM_EFUSE_ERR_LOCK	- Lock eFUSE Control Register.
 *
 * @warning	After eFUSE programming is complete, the cache is automatically
 *		reloaded so all programmed eFUSE bits can be directly read from
 *		cache. If there is any CACHE_ERROR, user can reboot the system
 *		to reload the cache.
 *
 ******************************************************************************/
int XNvm_EfuseWrite(const XNvm_EfuseData *WriteNvm)
{
	volatile int Status = XST_FAILURE;
	int LockStatus = XST_FAILURE;
	int DisableStatus = XST_FAILURE;
	int ResetStatus = XST_FAILURE;

    /**
	 *  check for input parameters. Return XNVM_EFUSE_ERR_INVALID_PARAM if input parameters are invalid.
	 */
	if (WriteNvm == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
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
		(WriteNvm->GlitchCfgBits == NULL) &&
		(WriteNvm->BootEnvCtrl == NULL) &&
		(WriteNvm->Misc1Bits == NULL) &&
#ifdef XNVM_EN_ADD_PPKS
		(WriteNvm->AdditionalPpkHash == NULL) &&
#endif
		(WriteNvm->OffChipIds == NULL)) {
		Status = (int)XNVM_EFUSE_ERR_NTHG_TO_BE_PROGRAMMED;
		goto END;
	}

	if ((WriteNvm->EnvMonitorDis != TRUE)) {
		if (WriteNvm->SysMonInstPtr == NULL) {
			Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
			goto END;
		}

		/**
		 *  Perform Environmental monitoring checks
		 */

		Status = XNvm_EfuseTempAndVoltChecks(WriteNvm->SysMonInstPtr);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
			goto END;
		}
	}

    /**
	 *  Unlock Efuse controller
	 */
	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
					XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		goto END;
	}

    /**
	 *  Validate all the write requests for AesKeys, PPK hash 0/1/2/3/4, Revocation Ids, Ivs, DecOnly, User eFuses, Glitch configuration, BootEnvCtrl, Misc1 Ctrl and offchip revocation eFuses
	 */
	Status = XST_FAILURE;
	Status = XNvm_EfuseValidateWriteReq(WriteNvm);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	if (WriteNvm->GlitchCfgBits != NULL) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePrgmGlitchCfgValues(WriteNvm->GlitchCfgBits);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		if (WriteNvm->GlitchCfgBits->GlitchDetWrLk == TRUE) {
			Status = XST_FAILURE;
			Status =  XNvm_EfusePrgmGlitchWriteLock(WriteNvm->GlitchCfgBits);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}

		if (WriteNvm->GlitchCfgBits->GdRomMonitorEn == TRUE) {
			Status = XST_FAILURE;
			Status = XNvm_EfusePrgmGdRomMonEn(WriteNvm->GlitchCfgBits);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}

		if (WriteNvm->GlitchCfgBits->GdHaltBootEn == TRUE) {
			Status = XST_FAILURE;
			Status = XNvm_EfusePrgmGdRomHaltBootEn(WriteNvm->GlitchCfgBits);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}
	}

	if (WriteNvm->AesKeys != NULL) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePrgmAesKeys(WriteNvm->AesKeys);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	if (WriteNvm->PpkHash != NULL) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePrgmPpkHash(WriteNvm->PpkHash);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	if (WriteNvm->Ivs != NULL) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePrgmIVs(WriteNvm->Ivs);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	if (WriteNvm->DecOnly != NULL) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePrgmDecOnly(WriteNvm->DecOnly);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	if (WriteNvm->RevokeIds != NULL) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePrgmRevocationIdFuses(WriteNvm->RevokeIds);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	if (WriteNvm->OffChipIds != NULL) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePrgmOffChipRevokeFuses(WriteNvm->OffChipIds);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

#ifdef XNVM_EN_ADD_PPKS
    if (WriteNvm->MiscCtrlBits != NULL) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePrgmAdditionalPpksMiscBits(WriteNvm->MiscCtrlBits);
		if (Status != XST_SUCCESS) {
			goto END;
	   }
	}

	if (WriteNvm->AdditionalPpkHash != NULL) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePrgmAdditionalPpkHash(WriteNvm->AdditionalPpkHash);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
#endif

	if (WriteNvm->MiscCtrlBits != NULL) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePrgmPpkRevokeFuses(WriteNvm->MiscCtrlBits);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		if(WriteNvm->MiscCtrlBits->HaltBootError == TRUE) {
			Status = XST_FAILURE;
			Status = XNvm_EfusePrgmHaltBootonError(WriteNvm->MiscCtrlBits);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}

		if(WriteNvm->MiscCtrlBits->HaltBootEnv == TRUE) {
			Status = XST_FAILURE;
			Status = XNvm_EfusePrgmHaltBootEnvError(WriteNvm->MiscCtrlBits);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}

		if (WriteNvm->MiscCtrlBits->CryptoKatEn == TRUE) {
			Status = XST_FAILURE;
			Status = XNvm_EfusePrgmCryptoKatEn(WriteNvm->MiscCtrlBits);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}

		if (WriteNvm->MiscCtrlBits->LbistEn == TRUE) {
			Status = XST_FAILURE;
			Status = XNvm_EfusePrgmLbistEn(WriteNvm->MiscCtrlBits);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}

		if (WriteNvm->MiscCtrlBits->SafetyMissionEn == TRUE) {
			Status = XST_FAILURE;
			Status = XNvm_EfusePrgmSafetyMissionEn(WriteNvm->MiscCtrlBits);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}
	}

	if (WriteNvm->UserFuses != NULL) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePrgmUserFuses(WriteNvm->UserFuses);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	if (WriteNvm->Misc1Bits != NULL) {
		Status = XST_FAILURE;
		Status = XNvm_EfuseWriteSecMisc1Fuses(WriteNvm->Misc1Bits);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	if (WriteNvm->BootEnvCtrl != NULL) {
		Status = XST_FAILURE;
		Status = XNvm_EfuseWriteBootEnvCtrl(WriteNvm->BootEnvCtrl);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	if (WriteNvm->SecCtrlBits != NULL) {
		Status = XST_FAILURE;
		Status = XNvm_EfuseWriteSecCtrl(WriteNvm->SecCtrlBits);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

   /**
    *  Reload the cache
	*/
	Status = XST_FAILURE;
	Status = XNvm_EfuseCacheLoadAndProtectionChecks();
	if (Status != XST_SUCCESS) {
		goto END;
	}

    /**
	 *   Program the Protection Row Efuses
	 */
	Status = XST_FAILURE;
	Status = XNvm_EfusePrgmProtectionEfuse();
END:
    /**
	 *  Reset Read mode
	 */
	ResetStatus = XNvm_EfuseResetReadMode();
	if (XST_SUCCESS == Status) {
		Status = ResetStatus;
	}

	/**
	 *  Disable Read mode
	 */
	DisableStatus = XNvm_EfuseDisableProgramming();
	if (XST_SUCCESS == Status) {
		Status = DisableStatus;
	}

	/**
	 *  Lock Efuse controller
	 */
	LockStatus = XNvm_EfuseLockController();
	if (XST_SUCCESS == Status) {
		Status = LockStatus;
	}
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function is used to read the eFUSE secure control bits from
 *		cache.
 *
 * @param	SecCtrlBits - Pointer to the Xnvm_SecCtrlBits which holds the
 *						  read secure control bits.
 *
 * @return	- XST_SUCCESS - On Successful read.
 * 		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *		- XNVM_EFUSE_ERR_CACHE_PARITY  - Error while Cache reload.
 *
 ******************************************************************************/
int XNvm_EfuseReadSecCtrlBits(XNvm_EfuseSecCtrlBits *SecCtrlBits)
{
	int Status = XST_FAILURE;
	u32 RegData = 0U;

    /**
	 *  Perform input parameter validation. Return XNVM_EFUSE_ERR_INVALID_PARAM if input parameters are invalid
	 */
	if (SecCtrlBits == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	/**
	 *  Read Directly from cache offset of security control to fill the SecCtrlBits structure. Return XST_SUCCESS if read is success
	 */
	Status = XNvm_EfuseReadCache(XNVM_EFUSE_SECURITY_CONTROL_ROW,
			&RegData);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	SecCtrlBits->AesDis =
		(u8)(RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_AES_DIS_MASK);
	SecCtrlBits->JtagErrOutDis =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_JTAG_ERROUT_DIS_MASK) >>
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
	SecCtrlBits->SecLockDbgDis =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_SEC_LOCK_DBG_DIS_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_SEC_LOCK_DBG_DIS_1_0_SHIFT);
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
 * @brief	This function is used to read the security miscellaneous1 bits
 *		from cache.
 *
 * @param	SecMisc1Bits - Pointer to the XNvm_EfuseSecMisc1Bits which holds
 * 		               the security miscellaneous1 control bits.
 *
 * @return	- XST_SUCCESS - On Successful read.
 * 		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *		- XNVM_EFUSE_ERR_CACHE_PARITY  - Error while Cache reload.
 *
 ******************************************************************************/
int XNvm_EfuseReadSecMisc1Bits(XNvm_EfuseSecMisc1Bits *SecMisc1Bits)
{
	int Status = XST_FAILURE;
	u32 RegData = 0U;

    /**
	 *  Perform input parameter validation. Return XNVM_EFUSE_ERR_INVALID_PARAM if input parameters are invalid
	 */
	if (SecMisc1Bits == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	/**
	 *  Read Directly from cache offset of SEC_MISC1 to fill SecMisc1Bits structure
	 */
	Status = XNvm_EfuseReadCache(XNVM_EFUSE_SECURITY_MISC_1_ROW,
			&RegData);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	SecMisc1Bits->LpdMbistEn =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SEC_MISC_1_LPD_MBIST_EN_2_0_MASK) >>
		XNVM_EFUSE_CACHE_SEC_MISC_1_LPD_MBIST_EN_2_0_SHIFT);
	SecMisc1Bits->PmcMbistEn =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SEC_MISC_1_PMC_MBIST_EN_2_0_MASK) >>
		XNVM_EFUSE_CACHE_SEC_MISC_1_PMC_MBIST_EN_2_0_SHIFT);
	SecMisc1Bits->LpdNocScEn =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SEC_MISC_1_LPD_NOC_SC_EN_2_0_MASK) >>
		XNVM_EFUSE_CACHE_SEC_MISC_1_LPD_NOC_SC_EN_2_0_SHIFT);
	SecMisc1Bits->SysmonVoltMonEn =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SEC_MISC_1_SYSMON_VOLT_MON_EN_1_0_MASK) >>
		XNVM_EFUSE_CACHE_SEC_MISC_1_SYSMON_VOLT_MON_EN_1_0_SHIFT);
	SecMisc1Bits->SysmonTempMonEn =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_SEC_MISC_1_SYSMON_TEMP_MON_EN_1_0_MASK) >>
		XNVM_EFUSE_CACHE_SEC_MISC_1_SYSMON_TEMP_MON_EN_1_0_SHIFT);
END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function reads the Boot Environmental Control bits from
 *		cache.
 *
 * @param	BootEnvCtrlBits - Pointer to the XNvm_EfuseBootEnvCtrlBits which
 * 				holds the Boot Environmental control bits.
 *
 * @return	- XST_SUCCESS - On Successful read.
 * 		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *		- XNVM_EFUSE_ERR_CACHE_PARITY  - Error while Cache reload.
 *
 ******************************************************************************/
int XNvm_EfuseReadBootEnvCtrlBits(XNvm_EfuseBootEnvCtrlBits *BootEnvCtrlBits)
{
	int Status = XST_FAILURE;
	u32 RegData = 0U;

    /**
	 *  Perform input parameter validation. Return XNVM_EFUSE_ERR_INVALID_PARAM if input parameters are invalid
	 */
	if (BootEnvCtrlBits == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	/**
	 *  Read Directly from cache offset of BOOT_ENV_CTRL offset to fill the BootEnvCtrlBits structure. Return XST_SUCCESS if read is success
	 */
	Status = XNvm_EfuseReadCache(XNVM_EFUSE_BOOT_ENV_CTRL_ROW,
			&RegData);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	BootEnvCtrlBits->SysmonTempEn =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_EN_MASK) >>
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_EN_SHIFT);
	BootEnvCtrlBits->SysmonVoltEn =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_EN_MASK) >>
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_EN_SHIFT);
	BootEnvCtrlBits->SysmonTempHot =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_HOT_MASK) >>
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_HOT_SHIFT);
	BootEnvCtrlBits->SysmonVoltPmc =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_PMC_MASK) >>
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_PMC_SHIFT);
	BootEnvCtrlBits->SysmonVoltPslp =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_PSLP_MASK) >>
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_PSLP_SHIFT);
	BootEnvCtrlBits->SysmonVoltSoc =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_SOC_MASK) >>
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_SOC_SHIFT);
	BootEnvCtrlBits->SysmonTempCold =
		(u8)((RegData &
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_COLD_MASK) >>
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_COLD_SHIFT);
END:
	return Status;
}

#ifndef XNVM_ACCESS_PUF_USER_DATA

/******************************************************************************/
/**
 * @brief	This function programs the eFUSEs with the PUF helper data.
 *
 * @param	PufHelperData - Pointer to the Puf helper data to be written
 * 				to eFuse.
 *
 * @return	- XST_SUCCESS - if programs successfully.
 *		- XNVM_EFUSE_ERR_RD_PUF_SEC_CTRL       - Error while reading
 *								PufSecCtrl.
 * 		- XNVM_EFUSE_ERR_WRITE_PUF_HELPER_DATA - Error while writing
 *			  					Puf helper data.
 *		- XNVM_EFUSE_ERR_WRITE_PUF_SYN_DATA    - Error while writing
 *			 					Puf Syndata.
 *		- XNVM_EFUSE_ERR_WRITE_PUF_CHASH       - Error while writing
 *								Puf Chash.
 * 		- XNVM_EFUSE_ERR_WRITE_PUF_AUX         - Error while writing
 *								Puf Aux.
 *
 * @note	To generate PufSyndromeData please use XPuf_Registration API.
 *
 ******************************************************************************/
int XNvm_EfuseWritePuf(const XNvm_EfusePufHd *PufHelperData)
{
	volatile int Status = XST_FAILURE;
	int LockStatus = XST_FAILURE;
	int DisableStatus = XST_FAILURE;
	int ResetStatus = XST_FAILURE;
	u32 PufSecurityCtrlReg = XNVM_EFUSE_SEC_DEF_VAL_ALL_BIT_SET;

    /**
	 *  Validate Input parameters. Return XNVM_EFUSE_ERR_INVALID_PARAM if input parameters are invalid
	 */
	if (PufHelperData == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

    /**
	 *  Read driectly from cache offset of Puf SecCtrl bits
	 */
	Status = XNvm_EfuseReadCache(XNVM_EFUSE_SECURITY_CONTROL_ROW,
				&PufSecurityCtrlReg);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((PufSecurityCtrlReg &
		(XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_DIS_MASK |
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_SYN_LK_MASK)) != 0x0U) {

		Status = (XNVM_EFUSE_ERR_FUSE_PROTECTED |
				XNVM_EFUSE_ERR_WRITE_PUF_HELPER_DATA);
		goto END;
	}

	if (PufHelperData->EnvMonitorDis != TRUE) {
		if (PufHelperData->SysMonInstPtr == NULL) {
			Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
			goto END;
		}
		Status = XST_FAILURE;

		/**
		 *  Check Environmental monitoring checks for temperature and voltage
		 */
		Status = XNvm_EfuseTempAndVoltChecks(
					PufHelperData->SysMonInstPtr);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
			goto END;
		}
	}

	Status = XST_FAILURE;

	/**
	 *  Unlock Efuse controller
	 */
	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
					XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		goto END;
	}

    /**
	 *  Set eFuse controller parameters for efuse write operation
	 */
	if (PufHelperData->PrgmPufHelperData == TRUE) {
		Status = XST_FAILURE;
		Status = XNvm_EfuseIsPufHelperDataEmpty();
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
			goto END;
		}

		Status = XST_FAILURE;
		Status = XNvm_EfuseWritePufSynData(PufHelperData->EfuseSynData);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_PUF_SYN_DATA);
			goto END;
		}

		Status = XST_FAILURE;
		Status = XNvm_EfuseWritePufChash(PufHelperData->Chash);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_PUF_CHASH);
			goto END;
		}

		Status = XST_FAILURE;
		Status = XNvm_EfuseWritePufAux(PufHelperData->Aux);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_PUF_AUX);
			goto END;
		}
	}

	/**
	 *   Programming Puf Security control bits
	 */
	if (PufHelperData->PrgmPufSecCtrlBits == TRUE) {
		Status = XST_FAILURE;
		Status = XNvm_EfuseWritePufSecCtrl(
			&(PufHelperData->PufSecCtrlBits));
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	Status = XST_FAILURE;

	/**
	 *    Reload the cache once the programming of all the efuses requested are programmed
	 */
	Status = XNvm_EfuseCacheLoadAndProtectionChecks();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;

	/**
	 *  Program Corresponding Protection Row eFuses
	 */
	Status = XNvm_EfusePrgmProtectionEfuse();
END :

	/**
	 *  Reset Read mode
	 */
	ResetStatus = XNvm_EfuseResetReadMode();
	if (XST_SUCCESS == Status) {
		Status = ResetStatus;
	}

	/**
	 *   Disable eFuse Programming mode
	 */
	DisableStatus = XNvm_EfuseDisableProgramming();
	if (XST_SUCCESS == Status) {
		Status = DisableStatus;
	}

	/**
	 *  Lock eFuse controller
	 */
	LockStatus = XNvm_EfuseLockController();
	if (XST_SUCCESS == Status) {
		Status = LockStatus;
	}
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs the eFUSEs with the PUF syndrome data.
 *
 * @param	SynData - Pointer to the Puf Syndrome data to program the eFUSE.
 *
 * @return	- XST_SUCCESS - if programs successfully.
 *  		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *
 * @note	To generate PufSyndromeData please use XPuf_Registration API.
 *
 ******************************************************************************/
static int XNvm_EfuseWritePufSynData(const u32 *SynData)
{
	int Status = XST_FAILURE;

	if (SynData == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfusePgmAndVerifyRows(XNVM_EFUSE_PUF_SYN_START_ROW,
				XNVM_EFUSE_PUF_SYN_DATA_NUM_OF_ROWS,
				XNVM_EFUSE_PAGE_2,
				SynData);
END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs the eFUSEs with the PUF Chash.
 *
 * @param	Chash - A 32-bit Chash to program the eFUSE.
 *
 * @return	- XST_SUCCESS - if programs successfully.
 *  		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *
 ******************************************************************************/
static int XNvm_EfuseWritePufChash(u32 Chash)
{
	int Status = XST_FAILURE;

	Status = XNvm_EfusePgmAndVerifyRows(XNVM_EFUSE_PUF_CHASH_ROW,
				XNVM_EFUSE_PUF_CHASH_NUM_OF_ROWS,
				XNVM_EFUSE_PAGE_0, &Chash);
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs the eFUSEs with the PUF Aux.
 *
 * @param	Aux - A 32-bit Aux to program the eFUSE.
 *
 * @return	- XST_SUCCESS - if programs successfully.
 *  		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *
 ******************************************************************************/
static int XNvm_EfuseWritePufAux(u32 Aux)
{
	int Status = XST_FAILURE;
	u32 AuxData;

	AuxData = (Aux & XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_ECC_23_0_MASK);

	Status = XNvm_EfusePgmAndVerifyRows(XNVM_EFUSE_PUF_AUX_ROW,
					XNVM_EFUSE_PUF_AUX_NUM_OF_ROWS,
					XNVM_EFUSE_PAGE_0, &AuxData);
	return Status;

}
/******************************************************************************/
/**
 * @brief	This function checks whether PUF is already programmed or not.
 *
 * @return	- XST_SUCCESS - if all rows are zero.
 *		- XNVM_EFUSE_ERR_PUF_SYN_ALREADY_PRGMD	 - Puf Syn data already
 *							   programmed.
 *		- XNVM_EFUSE_ERR_PUF_CHASH_ALREADY_PRGMD - Puf chash already
 *							   programmed.
 *		- XNVM_EFUSE_ERR_PUF_AUX_ALREADY_PRGMD	 - Puf Aux is already
 *							   programmed.
 *
 ******************************************************************************/
static int XNvm_EfuseIsPufHelperDataEmpty(void)
{
	int Status = XST_FAILURE;
	u32 PufSynRowNum;
	u32 RowDataVal;

	Status = XNvm_EfuseCheckZeros(XNVM_EFUSE_PUF_CHASH_ROW,
					(XNVM_EFUSE_PUF_CHASH_ROW +
					XNVM_EFUSE_PUF_CHASH_NUM_OF_ROWS));
	if (Status != XST_SUCCESS) {
		Status = (int)XNVM_EFUSE_ERR_PUF_CHASH_ALREADY_PRGMD;
		goto END;
	}

	Status = XNvm_EfuseReadCache(XNVM_EFUSE_PUF_AUX_ROW, &RowDataVal);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	if ((RowDataVal &
		XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_ECC_23_0_MASK) != 0x00U) {
		Status = (int)XNVM_EFUSE_ERR_PUF_AUX_ALREADY_PRGMD;
		goto END;
	}

	/* Puf Syndrome eFuses are in Page 2 of eFuse memory map,
	 * hence effective row should be calculated to read
	 * correct eFuses.
	 */
	PufSynRowNum = XNVM_EFUSE_PUF_SYN_START_ROW +
		(XNVM_NUM_OF_ROWS_PER_PAGE * (u32)XNVM_EFUSE_PAGE_2);

	Status = XNvm_EfuseCheckZeros(PufSynRowNum,
		(PufSynRowNum + XNVM_EFUSE_PUF_SYN_DATA_NUM_OF_ROWS));
	if (Status != XST_SUCCESS) {
		Status = (int)XNVM_EFUSE_ERR_PUF_SYN_ALREADY_PRGMD;
		goto END;
	}
END :
	return Status;

}

/******************************************************************************/
/**
 * @brief	This function programs Puf control bits specified by user.
 *
 * @param	PufSecCtrlBits - Pointer to the XNvm_EfusePufSecCtrlBits
 * 				structure, which holds the PufSecCtrlBits data
 * 				to be written to the eFuse.
 *
 * @return	- XST_SUCCESS - On success.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM       -  Invalid parameter.
 *		- XNVM_EFUSE_ERR_WRITE_PUF_REGEN_DIS - 	Error while writing
 *						   	PUF_REGEN_DIS.
 *		- XNVM_EFUSE_ERR_WRITE_PUF_HD_INVLD  - 	Error while writing
 *							PUF_HD_INVLD.
 *		- XNVM_EFUSE_ERR_WRITE_PUF_SYN_LK - 	Error while writing
 * 							PUF_SYN_LK.
 *		- XNVM_EFUSE_ERR_WRITE_PUF_TEST2_DIS - 	Error while writing
 *							PUF_TEST2_DIS.
 *		- XNVM_EFUSE_ERR_WRITE_PUF_DIS       - 	Error while writing
 *							PUF_DIS.
 *
 ******************************************************************************/
static int XNvm_EfuseWritePufSecCtrl(const XNvm_EfusePufSecCtrlBits *PufSecCtrlBits)
{
	volatile int Status = XST_FAILURE;
	XNvm_EfuseType EfuseType = XNVM_EFUSE_PAGE_0;
	u32 Row = XNVM_EFUSE_PUF_AUX_ROW;
	u32 RowDataVal = XNVM_EFUSE_SEC_DEF_VAL_ALL_BIT_SET;

	if (PufSecCtrlBits == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if ((PufSecCtrlBits->PufRegenDis != FALSE) ||
		(PufSecCtrlBits->PufHdInvalid != FALSE)) {

		Status = XNvm_EfuseReadCache(Row, &RowDataVal);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	if ((PufSecCtrlBits->PufRegenDis == TRUE) &&
		((RowDataVal &
		XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_REGEN_DIS_MASK) !=
		XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_REGEN_DIS_MASK)) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
					XNVM_EFUSE_PUF_ECC_PUF_CTRL_REGEN_DIS_COLUMN);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_PUF_REGEN_DIS);
			goto END;
		}
	}
	if ((PufSecCtrlBits->PufHdInvalid == TRUE) &&
		((RowDataVal &
		XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_HD_INVLD_MASK) !=
		XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_HD_INVLD_MASK)) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
			XNVM_EFUSE_PUF_ECC_PUF_CTRL_HD_INVLD_COLUMN);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_PUF_HD_INVLD);
			goto END;
		}
	}

	RowDataVal = 0U;
	Row = XNVM_EFUSE_SECURITY_CONTROL_ROW;

	if ((PufSecCtrlBits->PufTest2Dis != FALSE) ||
		(PufSecCtrlBits->PufDis != FALSE) ||
		(PufSecCtrlBits->PufSynLk != FALSE)) {
		Status = XST_FAILURE;
		Status = XNvm_EfuseReadCache(Row, &RowDataVal);
		if (Status != XST_SUCCESS) {
			goto END;
		}

	}

	if ((PufSecCtrlBits->PufSynLk != FALSE) &&
		((RowDataVal &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_SYN_LK_MASK) !=
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_SYN_LK_MASK)) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
					(u32)XNVM_EFUSE_SEC_PUF_SYN_LK);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_PUF_SYN_LK);
			goto END;
		}
	}

	if ((PufSecCtrlBits->PufTest2Dis != FALSE) &&
		((RowDataVal &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_TEST2_DIS_MASK) !=
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_TEST2_DIS_MASK)) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
					(u32)XNVM_EFUSE_SEC_PUF_TEST2_DIS);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_PUF_TEST2_DIS);
			goto END;
		}
	}

	if ((PufSecCtrlBits->PufDis != FALSE) &&
		((RowDataVal &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_DIS_MASK) !=
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_DIS_MASK)) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
					(u32)XNVM_EFUSE_SEC_PUF_DIS);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_PUF_DIS);
			goto END;
		}
	}
	Status = XST_SUCCESS;
END:
	return Status;
}

#endif

/******************************************************************************/
/**
 * @brief	This function is used to read the Puf eFUSE secure control bits
 *		from cache.
 *
 * @param	PufSecCtrlBits - Pointer to the XNvm_PufSecCtrlBits, which holds
 *				the read secure control bits.
 *
 * @return	- XST_SUCCESS - On Successful read.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *		- XNVM_EFUSE_ERR_CACHE_PARITY  - Error while Cache reload.
 *
 ******************************************************************************/
int XNvm_EfuseReadPufSecCtrlBits(XNvm_EfusePufSecCtrlBits *PufSecCtrlBits)
{
	int Status = XST_FAILURE;
	u32 PufEccCtrlReg = 0U;
	u32 PufSecurityCtrlReg = 0U;

    /**
	 *  Validate input parameters. Return XNVM_EFUSE_ERR_INVALID_PARAM if input parameters are invalid.
	 */
	if (PufSecCtrlBits == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	/**
	 * @{ Read directly from cache offset of the PUF_ECC_PUF_CTRL and also SEC_CRTL to fill the PufSecCtrlBits structure.
	 *    Return XST_SUCESS if read is success.
	 */
	Status = XNvm_EfuseReadCache(XNVM_EFUSE_PUF_AUX_ROW, &PufEccCtrlReg);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XNvm_EfuseReadCache(XNVM_EFUSE_SECURITY_CONTROL_ROW,
					&PufSecurityCtrlReg);
	if (Status != XST_SUCCESS) {
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
 * @brief	This function is used to read the miscellaneous eFUSE control
 *		bits from cache.
 *
 * @param	MiscCtrlBits - Pointer to the XNvm_MiscCtrlBits which holds
 *				the read secure control bits.
 *
 * @return	- XST_SUCCESS - On Successful read.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *		- XNVM_EFUSE_ERR_CACHE_PARITY  - Error while Cache reload.
 *
 ******************************************************************************/
int XNvm_EfuseReadMiscCtrlBits(XNvm_EfuseMiscCtrlBits *MiscCtrlBits)
{
	int Status = XST_FAILURE;
	u32 ReadReg = 0U;

    /**
	 *  Validate input parameters. Return XNVM_EFUSE_ERR_INVALID_PARAM if input parameters are invalid
	 */
	if (MiscCtrlBits == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	/**
	 *  Read directly from cache offset of the MiscCtrl to fill the MiscCtrlBits structure
	 */
	Status = XNvm_EfuseReadCache(XNVM_EFUSE_MISC_CTRL_ROW, &ReadReg);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	MiscCtrlBits->GlitchDetHaltBootEn =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_MISC_CTRL_GD_HALT_BOOT_EN_1_0_MASK) >>
		XNVM_EFUSE_CACHE_MISC_CTRL_GD_HALT_BOOT_EN_1_0_SHIFT);
	MiscCtrlBits->GlitchDetRomMonitorEn =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_MISC_CTRL_GD_ROM_MONITOR_EN_MASK) >>
		XNVM_EFUSE_CACHE_MISC_CTRL_GD_ROM_MONITOR_EN_SHIFT);
	MiscCtrlBits->HaltBootError =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_MISC_CTRL_HALT_BOOT_ERROR_1_0_MASK) >>
		XNVM_EFUSE_CACHE_MISC_CTRL_HALT_BOOT_ERROR_1_0_SHIFT);
	MiscCtrlBits->HaltBootEnv =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_MISC_CTRL_HALT_BOOT_ENV_1_0_MASK) >>
		XNVM_EFUSE_CACHE_MISC_CTRL_HALT_BOOT_ENV_1_0_SHIFT);
	MiscCtrlBits->CryptoKatEn =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_MISC_CTRL_CRYPTO_KAT_EN_MASK) >>
		XNVM_EFUSE_CACHE_MISC_CTRL_CRYPTO_KAT_EN_SHIFT);
	MiscCtrlBits->LbistEn =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_MISC_CTRL_LBIST_EN_MASK) >>
		XNVM_EFUSE_CACHE_MISC_CTRL_LBIST_EN_SHIFT);
	MiscCtrlBits->SafetyMissionEn =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_MISC_CTRL_SAFETY_MISSION_EN_MASK) >>
		XNVM_EFUSE_CACHE_MISC_CTRL_SAFETY_MISSION_EN_SHIFT);
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
#ifdef XNVM_EN_ADD_PPKS
	MiscCtrlBits->Ppk3Invalid =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_MISC_CTRL_PPK3_INVLD_1_0_MASK) >>
		XNVM_EFUSE_CACHE_MISC_CTRL_PPK3_INVLD_1_0_SHIFT);
	MiscCtrlBits->Ppk4Invalid =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_MISC_CTRL_PPK4_INVLD_1_0_MASK) >>
		XNVM_EFUSE_CACHE_MISC_CTRL_PPK4_INVLD_1_0_SHIFT);
	MiscCtrlBits->AdditionalPpkEn =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_MISC_CTRL_ADD_PPK_1_0_MASK) >>
		XNVM_EFUSE_CACHE_MISC_CTRL_ADD_PPK_EN_SHIFT);
#endif
END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs the eFUSEs with the IV.
 *
 * @param	EfuseIv - Pointer to the XNvm_EfuseIvs, which contains IVs data
 * 				to be written to the Efuse..
 * @param   SysMonInstPtr - Pointer to the XSysMonPsv structure which contains
 *              XSysmonPsv driver instance data
 *
 * @return	- XST_SUCCESS - On Successful Write.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM        	 - On Invalid Parameter.
 *		- XNVM_EFUSE_ERR_WRITE_META_HEADER_IV	 - Error while writing
 *								MetaHeader Iv.
 *		- XNVM_EFUSE_ERR_WRITE_BLK_OBFUS_IV   	 - Error while writing
 *								BlkObfus IV.
 *		- XNVM_EFUSE_ERR_WRITE_PLM IV         	 - Error while writing
 *								PLM IV.
 *		- XNVM_EFUSE_ERR_WRITE_DATA_PARTITION_IV - Error while writing
 *			 					Data Partition IV.
 *
 ******************************************************************************/
int XNvm_EfuseWriteIVs(XNvm_EfuseIvs *EfuseIv, XSysMonPsv *SysMonInstPtr)
{
	int Status = XST_FAILURE;
	XNvm_EfuseData WriteIvs = {0U};

    /**
	 *  validate Input parameters. Return XNVM_EFUSE_ERR_NTHG_TO_BE_PROGRAMMED if input parameters are invalid
	 */
	if (EfuseIv == NULL) {
		Status = (int)XNVM_EFUSE_ERR_NTHG_TO_BE_PROGRAMMED;
		goto END;
	}

	if (SysMonInstPtr == NULL) {
		WriteIvs.EnvMonitorDis = TRUE;
	}
	else {
		WriteIvs.SysMonInstPtr = SysMonInstPtr;
		WriteIvs.EnvMonitorDis = FALSE;
	}

    /**
	 *  Fill the EfuseData structure with the IV address, Environmental disable flag and remaining as NULL
	 */
	WriteIvs.Ivs = EfuseIv;

    /**
	 *  @{ Fill the XNvm_EfuseData global structure with the EfuseIv structure and call XNvm_EfuseWrite API.
     *     Return the Status
	 */
	Status = XNvm_EfuseWrite(&WriteIvs);
END :
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function is used to read IV eFUSE bits from cache.
 *
 * @param	EfuseIv - Pointer to the XNvm_Iv, which contains read IV data.
 * @param	IvType  - Iv to be read.
 *
 * @return	- XST_SUCCESS - On Successful read.
 *  		- XNVM_EFUSE_ERR_INVALID_PARAM		- On Invalid Parameter.
 *		- XNVM_EFUSE_ERR_RD_META_HEADER_IV	- Error while reading
 *								MetaHeader IV.
 *		- XNVM_EFUSE_ERR_RD_BLACK_OBFUS_IV 	- Error while reading
 *			  					BlkObfus IV.
 *		- XNVM_EFUSE_ERR_RD_PLM_IV 		- Error while reading PLM IV.
 *		- XNVM_EFUSE_ERR_RD_DATA_PARTITION_IV	- Error while reading
 *			 					Data Partition IV.
 *
 ******************************************************************************/
int XNvm_EfuseReadIv(XNvm_Iv *EfuseIv, XNvm_IvType IvType)
{
	int Status = XST_FAILURE;
	int ClearStatus = XST_FAILURE;

    /**
	 *  validate input parameters. Return XNVM_EFUSE_ERR_INVALID_PARAM if input parameters are invalid
	 */
	if (EfuseIv == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if ((IvType != XNVM_EFUSE_BLACK_IV) &&
		(IvType != XNVM_EFUSE_PLM_IV_RANGE) &&
		(IvType != XNVM_EFUSE_DATA_PARTITION_IV_RANGE) &&
		(IvType != XNVM_EFUSE_META_HEADER_IV_RANGE)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

    /**
	 *   @{ Based on user input IvType Reads the efuse cache. Fills EfuseIv structure.
	 *      If read cache fails zeroize the EfuseIv data and return XST_FAILURE.
	 */
	if (IvType == XNVM_EFUSE_META_HEADER_IV_RANGE) {
		Status = XNvm_EfuseReadCacheRange(
				XNVM_EFUSE_META_HEADER_IV_START_ROW,
				XNVM_EFUSE_IV_NUM_OF_ROWS,
				EfuseIv->Iv);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_RD_META_HEADER_IV_RANGE);
			goto END_RST;
		}
	}
	else if (IvType == XNVM_EFUSE_BLACK_IV) {
		Status = XNvm_EfuseReadCacheRange(
					XNVM_EFUSE_BLACK_OBFUS_IV_START_ROW,
					XNVM_EFUSE_IV_NUM_OF_ROWS,
					EfuseIv->Iv);
		if (Status != XST_SUCCESS) {
			Status = (Status |
					XNVM_EFUSE_ERR_RD_BLACK_IV);
			goto END_RST;
		}
	}
	else if (IvType == XNVM_EFUSE_PLM_IV_RANGE) {
		Status = XNvm_EfuseReadCacheRange(XNVM_EFUSE_PLM_IV_START_ROW,
						XNVM_EFUSE_IV_NUM_OF_ROWS,
						EfuseIv->Iv);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_RD_PLM_IV_RANGE);
			goto END_RST;
		}
	}
	else {
		Status = XNvm_EfuseReadCacheRange(
					XNVM_EFUSE_DATA_PARTITION_IV_START_ROW,
					XNVM_EFUSE_IV_NUM_OF_ROWS,
					EfuseIv->Iv);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_RD_DATA_PARTITION_IV_RANGE);
			goto END_RST;
		}
	}

	Status = XST_SUCCESS;

END_RST:
	if (Status != XST_SUCCESS) {

		ClearStatus = XNvm_ZeroizeAndVerify((u8 *)EfuseIv,
							sizeof(XNvm_Iv));
		if (ClearStatus != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_IN_ZEROIZATION);
		}
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function reads the PUF helper data from eFUSE cache.
 *
 * @param	PufHelperData - Pointer to XNvm_EfusePufHd which holds the PUF
 * 						helper data read from eFUSEs.
 *
 * @return	- XST_SUCCESS - On successful read.
 *		- XNVM_EFUSE_ERR_RD_PUF_SYN_DATA - Error while reading SYN_DATA.
 *		- XNVM_EFUSE_ERR_RD_PUF_CHASH    - Error while reading CHASH.
 *		- XNVM_EFUSE_ERR_RD_PUF_AUX      - Error while reading AUX.
 *
 ******************************************************************************/
int XNvm_EfuseReadPuf(XNvm_EfusePufHd *PufHelperData)
{
	int Status = XST_FAILURE;
	int ClearStatus = XST_FAILURE;
	u32 PufSynRowNum;

    /**
	 *  Validate input parameters. Return XNVM_EFUSE_ERR_INVALID_PARAM if input parameters are invalid
	 */
	if (PufHelperData == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	/* Puf Syndrome eFuses are in Page 2 of eFuse memory map,
	 * hence effective row should be calculated to read
	 * correct eFuses.
	 */
	PufSynRowNum = XNVM_EFUSE_PUF_SYN_START_ROW +
		(XNVM_NUM_OF_ROWS_PER_PAGE * (u32)XNVM_EFUSE_PAGE_2);

    /**
	 *  Read directly from cache offset of the PUF syndrome data, Chash and Aux data to fill the PufHelperData structure. Return XST_SUCCESS if read is successful
	 */
	Status = XNvm_EfuseReadCacheRange(PufSynRowNum,
					XNVM_EFUSE_PUF_SYN_DATA_NUM_OF_ROWS,
					PufHelperData->EfuseSynData);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_RD_PUF_SYN_DATA);
		goto END_RST;
	}

	Status = XNvm_EfuseReadCache(XNVM_EFUSE_PUF_CHASH_ROW,
					&(PufHelperData->Chash));
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_RD_PUF_CHASH);
		goto END_RST;
	}
	Status = XNvm_EfuseReadCache(XNVM_EFUSE_PUF_AUX_ROW,
				&(PufHelperData->Aux));
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_RD_PUF_AUX);
		goto END_RST;
	}

	PufHelperData->Aux = PufHelperData->Aux &
				XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_ECC_23_0_MASK;

END_RST:
	if (Status != XST_SUCCESS) {
		ClearStatus = XNvm_ZeroizeAndVerify((u8 *)PufHelperData,
						sizeof(XNvm_EfusePufHd));
		if (ClearStatus != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_IN_ZEROIZATION);
		}
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function is used to read Dna eFUSE bits from cache.
 *
 * @param	EfuseDna - Pointer to the XNvm_Dna, which holds the read DNA.
 *
 * @return	- XST_SUCCESS - On Successful read.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid parameter.
 *		- XNVM_EFUSE_ERR_RD_DNA        - Error in reading DNA efuses.
 *
 ******************************************************************************/
int XNvm_EfuseReadDna(XNvm_Dna *EfuseDna)
{
	int Status = XST_FAILURE;

    /**
	 *  validate input parameters. Return XNVM_EFUSE_ERR_INVALID_PARAM if input parameters are invalid
	 */
	if (EfuseDna == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

    /**
	 *  Read directly from cache offset of the Dna to fill the Dna array. Return XST_SUCCESS if read success
	 */
	Status = XNvm_EfuseReadCacheRange(XNVM_EFUSE_DNA_START_ROW,
					XNVM_EFUSE_DNA_NUM_OF_ROWS,
					EfuseDna->Dna);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_RD_DNA);
		goto END;
	}
END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function is used to read DecEfuseOnly eFUSE bits from cache.
 *
 * @param	DecOnly - Pointer to the DecOnly efuse data.
 *
 * @return	- XST_SUCCESS - On Successful read.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *		- XNVM_EFUSE_ERR_RD_DEC_ONLY - Error in reading in Dec_only efuses.
 *
 * @note	If DecOnly eFuse is programmed that means boot can happen in
 * 		only Symmetric HWRoT boot mode.
 ******************************************************************************/
int XNvm_EfuseReadDecOnly(u32* DecOnly)
{
	int Status = XST_FAILURE;

    /**
	 *  validate input parameters. Return XNVM_EFUSE_ERR_INVALID_PARAM if input parameters are invalid
	 */
	if (DecOnly == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

     /**
	  *  Read directly from cache offset of the DecOnly eFuse to fill the input param. Return XST_SUCCESS if read success
	  */
	Status = XNvm_EfuseReadCache(XNVM_EFUSE_DEC_EFUSE_ONLY_ROW,
					DecOnly);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_RD_DEC_ONLY);
	}
END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function reads the Ppk Hash from eFUSE cache.
 *
 * @param	EfusePpk - Pointer to XNvm_PpkHash which holds the PpkHash from
 * 					   eFUSE Cache.
 * @param 	PpkType - Type of the Ppk to be programmed.
 *
 * @return	- XST_SUCCESS - On Successful read.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 * 		- XNVM_EFUSE_ERR_RD_PPK_HASH   - Error while reading PPK Hash.
 *
 ******************************************************************************/
int XNvm_EfuseReadPpkHash(XNvm_PpkHash *EfusePpk, XNvm_PpkType PpkType)
{
	int Status = XST_FAILURE;
	u32 PpkRow = XNVM_EFUSE_PPK_0_HASH_START_ROW;

    /**
	 *  validate input parameters. Return XNVM_EFUSE_ERR_INVALID_PARAM if input parameters are invalid
	 */
	if (EfusePpk == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (PpkType == XNVM_EFUSE_PPK0) {
		PpkRow = XNVM_EFUSE_PPK_0_HASH_START_ROW;
	} else if (PpkType == XNVM_EFUSE_PPK1) {
		PpkRow = XNVM_EFUSE_PPK_1_HASH_START_ROW;
	} else if (PpkType == XNVM_EFUSE_PPK2) {
		PpkRow = XNVM_EFUSE_PPK_2_HASH_START_ROW;
	}
#ifdef XNVM_EN_ADD_PPKS
	else if (PpkType == XNVM_EFUSE_PPK3) {
		PpkRow = XNVM_EFUSE_PPK_3_HASH_START_ROW;
	}
	else if (PpkType == XNVM_EFUSE_PPK4) {
		PpkRow = XNVM_EFUSE_PPK_4_HASH_START_ROW;
	}
#endif
	else {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

     /**
	  *  Read directly from cache offset of the mentioned ppk type to fill the ppk hash array. Return XST_SUCCESS if read success
	  */
	Status = XNvm_EfuseReadCacheRange(PpkRow,
					XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS,
					EfusePpk->Hash);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_RD_PPK_HASH);
		goto END;
	}
END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function revokes the Ppk.
 *
 * @param	PpkRevoke - Xnvm_RevokePpkFlags that tells which Ppk to revoke.
 * @param   SysMonInstPtr - Pointer to the XSysMonPsv structure which contains
 *              XSysmonPsv driver instance data
 *
 * @return	- XST_SUCCESS - On Successful Revocation.
 *		- XNVM_EFUSE_ERR_LOCK - Error while Locking the controller.
 *
 ******************************************************************************/
int XNvm_EfuseRevokePpk(XNvm_PpkType PpkRevoke, XSysMonPsv *SysMonInstPtr)
{
	int Status = XST_FAILURE;
	XNvm_EfuseData EfuseData = {0U};
	XNvm_EfuseMiscCtrlBits MiscCtrlBits = {0U};

    /**
	 *  validate input parameters. Return XNVM_EFUSE_ERR_INVALID_PARAM if input parameters are invalid
	 */
	if ((PpkRevoke != XNVM_EFUSE_PPK0) &&
		(PpkRevoke != XNVM_EFUSE_PPK1) &&
		(PpkRevoke != XNVM_EFUSE_PPK2)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	/**
	 *	@{ Fill the XNvm_EfuseMiscCtrlBits structure with PPK INVLD inputs from user and fill the XNvm_EfuseData global structure with the MiscCtrlBits structure and call XNvm_EfuseWrite API.
     *	   Return the Status.
	 */
	if (PpkRevoke == XNVM_EFUSE_PPK0) {
		MiscCtrlBits.Ppk0Invalid = TRUE;
	}
	else if (PpkRevoke == XNVM_EFUSE_PPK1) {
		MiscCtrlBits.Ppk1Invalid = TRUE;
	}
	else {
		MiscCtrlBits.Ppk2Invalid = TRUE;
	}

	if (SysMonInstPtr == NULL) {
		EfuseData.EnvMonitorDis = TRUE;
	}
	else {
		EfuseData.SysMonInstPtr = SysMonInstPtr;
		EfuseData.EnvMonitorDis = FALSE;
	}

	EfuseData.MiscCtrlBits = &MiscCtrlBits;

	Status = XNvm_EfuseWrite(&EfuseData);
END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function writes Revocation eFuses.
 *
 * @param	RevokeId - RevokeId number to program Revocation Id eFuses.
 * @param   SysMonInstPtr - Pointer to the XSysMonPsv structure which contains
 *              XSysmonPsv driver instance data
 *
 * @return	- XST_SUCCESS - On successful write to eFuse.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *		- XNVM_EFUSE_ERR_LOCK - Error while Locking the controller.
 *
 * @note
 * Example: 	If the revoke id to program is 64, it will program the 0th bit
 * 		of the REVOCATION_ID_2 eFuse row.
 *
 ******************************************************************************/
int XNvm_EfuseWriteRevocationId(u32 RevokeId, XSysMonPsv *SysMonInstPtr)
{
	int Status = XST_FAILURE;
	u32 RevokeIdRow;
	u32 RevokeIdBit;
	XNvm_EfuseRevokeIds WriteRevokeId = {0U};
	XNvm_EfuseData EfuseData = {0U};

    /**
	 *  validate input parameters. Return XNVM_EFUSE_ERR_INVALID_PARAM if input parameters are invalid
	 */
	if (RevokeId > (XNVM_MAX_REVOKE_ID_FUSES - 1U)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

    /**
	 *  Calculate the REVOCATION_ID eFuse row  and the column based on the input provided
	 */
	RevokeIdRow = (RevokeId / XNVM_EFUSE_MAX_BITS_IN_ROW);
	RevokeIdBit = (RevokeId % XNVM_EFUSE_MAX_BITS_IN_ROW);

	if (RevokeIdRow > (u32)XNVM_EFUSE_REVOCATION_ID_7) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

    /**
	 *  @{ Fill the XNvm_EfuseRevokeIds structure with the calculated inputs and fill the XNvm_EfuseData global structure with the XNvm_EfuseRevokeIds and call XNvm_EfuseWrite API.
     *     Return the Status.
	 */
	if (SysMonInstPtr == NULL) {
		EfuseData.EnvMonitorDis = TRUE;
	}
	else {
		EfuseData.SysMonInstPtr = SysMonInstPtr;
		EfuseData.EnvMonitorDis = FALSE;
	}

	WriteRevokeId.RevokeId[RevokeIdRow] = ((u32)1U << RevokeIdBit);
	WriteRevokeId.PrgmRevokeId = TRUE;

	EfuseData.RevokeIds = &WriteRevokeId;

	Status = XNvm_EfuseWrite(&EfuseData);

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function reads the Revocation Fuse from eFUSE cache.
 *
 * @param	RevokeFusePtr - Pointer to the data which hold the Revocation IDs
 * 							from eFUSE Cache.
 * @param 	RevokeFuseNum - Revocation ID fuse number to read.
 *
 * @return	- XST_SUCCESS - On successful read.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *		- XNVM_EFUSE_ERR_CACHE_PARITY  - Error in Cache reload.
 *
 ******************************************************************************/
int XNvm_EfuseReadRevocationId(u32 *RevokeFusePtr,
				XNvm_RevocationId RevokeFuseNum)
{
	int Status = XST_FAILURE;
	u32 Row;

    /**
	 *  validate input parameters. Return XNVM_EFUSE_ERR_INVALID_PARAM if input parameters are invalid
	 */
	if ((RevokeFusePtr == NULL) ||
		(RevokeFuseNum > XNVM_EFUSE_REVOCATION_ID_7)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	else {

		/**
		 *  @{ Read directly from cache offset of the Revocation Id to fill the Revocation Id array.
         *     Return XST_SUCCESS if the read is successful
		 */
		Row = XNVM_EFUSE_REVOCATION_ID_0_ROW + (u32)RevokeFuseNum;
		Status = XNvm_EfuseReadCache(Row, RevokeFusePtr);
	}
END :
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function reads the Offchip revoke eFuse value from
 * 		eFUSE cache.
 *
 * @param	OffchipIdPtr - Pointer to the data which hold the Offchip
 * 				revoke ID values.
 * @param 	OffchipIdNum - Offchip fuse number to read.
 *
 * @return	- XST_SUCCESS - On successful read.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *		- XNVM_EFUSE_ERR_CACHE_PARITY  - Error in Cache reload.
 *
 ******************************************************************************/
int XNvm_EfuseReadOffchipRevokeId(u32 *OffchipIdPtr,
					XNvm_OffchipId OffchipIdNum)
{
	int Status = XST_FAILURE;
	u32 Row;

     /**
	  *  validate input parameters. Return XNVM_EFUSE_ERR_INVALID_PARAM if input parameters are invalid
	  */
	if ((OffchipIdPtr == NULL) ||
		(OffchipIdNum < XNVM_EFUSE_OFFCHIP_REVOKE_ID_0) ||
		(OffchipIdNum > XNVM_EFUSE_OFFCHIP_REVOKE_ID_7)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
	}
	else {

		/**
		 *  Calculate offchip_revoke id offset based on which revoke id value is requested
		 */
		Row = XNVM_EFUSE_OFFCHIP_REVOKE_0_ROW + (u32)OffchipIdNum;

		/**
		 *  @{ Read directly from cache offset of the offchip revoke id offset to fill the OffchipIdPtr.
         *	   Return XST_SUCCESS if the read is successful
		 */
		Status = XNvm_EfuseReadCache(Row, OffchipIdPtr);
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function reads User eFuses from Cache.
 *
 *@param	UserFusesData - UserFuseData to be read from eFuse Cache.
 *
 *@return	- XST_SUCCESS - if reads successfully.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *		- XNVM_EFUSE_ERR_RD_USER_FUSES - Error in reading User fuses.
 *
 ******************************************************************************/
int XNvm_EfuseReadUserFuses(const XNvm_EfuseUserData *UserFusesData)
{
	int Status = XST_FAILURE;
	u32 Row;

     /**
	  *  validate input parameters. Return XNVM_EFUSE_ERR_INVALID_PARAM if input parameters are invalid
	  */
	if (UserFusesData == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	if ((UserFusesData->StartUserFuseNum < XNVM_USER_FUSE_START_NUM) ||
		(UserFusesData->StartUserFuseNum > XNVM_USER_FUSE_END_NUM)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	if ((UserFusesData->UserFuseData == NULL) ||
		(UserFusesData->NumOfUserFuses > XNVM_NUM_OF_USER_FUSES)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	if (((UserFusesData->StartUserFuseNum - 1U)  +
		UserFusesData->NumOfUserFuses) > XNVM_USER_FUSE_END_NUM) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Row = XNVM_EFUSE_USER_FUSE_START_ROW +
		(UserFusesData->StartUserFuseNum - 1U);

    /**
	 *  @{ Read directly from cache offset of the User Data eFuse to fill the UserFuseData address from specified UserFuse number (StartUserFuseNum) and number of eFuses to be read by NumOfUserFuses .
     *     Return XST_SUCCESS if the read is successful
	 */
	Status = XNvm_EfuseReadCacheRange(Row,
					(u8)(UserFusesData->NumOfUserFuses),
					UserFusesData->UserFuseData);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_RD_USER_FUSES);
	}
END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function Programs User eFuses.
 *
 * @param	WriteUserFuses - Pointer to the XNvm_EfuseUserData structure.
 * @param   SysMonInstPtr - Pointer to the XSysMonPsv structure which contains
 *              XSysmonPsv driver instance data
 *
 * @return	- XST_SUCCESS - if programming is successful.
 *		- XNVM_EFUSE_ERR_NTHG_TO_BE_PROGRAMMED - if NULL request is sent.
 *
 ******************************************************************************/
int XNvm_EfuseWriteUserFuses(XNvm_EfuseUserData *WriteUserFuses,
	XSysMonPsv *SysMonInstPtr)
{
	int Status = XST_FAILURE;
	XNvm_EfuseData UserFusesData = {0};

     /**
	  *  validate input parameters. Return XNVM_EFUSE_ERR_INVALID_PARAM if input parameters are invalid
	  */
	if (WriteUserFuses == NULL) {
		Status = (int)XNVM_EFUSE_ERR_NTHG_TO_BE_PROGRAMMED;
		goto END;
	}

	if (SysMonInstPtr == NULL) {
		UserFusesData.EnvMonitorDis = TRUE;
	}
	else {
		UserFusesData.SysMonInstPtr = SysMonInstPtr;
		UserFusesData.EnvMonitorDis = FALSE;
	}

    /**
	 * @{ Fill the XNvm_EfuseData global structure with the user provided WriteUserFuses and call XNvm_EfuseWrite API.
     *	  Return the XST_SUCCESS
	 */
	UserFusesData.UserFuses = WriteUserFuses;

	Status = XNvm_EfuseWrite(&UserFusesData);
END :
	return Status;
}

/******************************************************************************/
/**
 *
 * @brief	This function programs secure control bits specified by user.
 *
 * @param   	SecCtrl - Pointer to the XNvm_EfuseSecCtrlBits structure, which
 * 			holds SecCtrlBits to be written to eFuse.
 *
 * @return	- XST_SUCCESS - Specified bit set in eFUSE.
 *		- XNVM_EFUSE_ERROR_INVALID_PARAM	- Invalid Parameter.
 *		- XNVM_EFUSE_ERR_WRITE_PPK0_WR_LK	- Error while writing
 *							PPK0_WR_LK.
 *		- XNVM_EFUSE_ERR_WRITE_PPK1_WR_LK 	- Error while writing
 *							PPK1_WR_LK.
 * 		- XNVM_EFUSE_ERR_WRITE_PPK2_WR_LK 	- Error while writing
 *							PPK2_WR_LK.
 *		- XNVM_EFUSE_ERR_WRITE_AES_CRC_LK_BIT_1   - Error while writing
 *							AES_CRC_LK_BIT_1.
 * 		- XNVM_EFUSE_ERR_WRITE_AES_CRC_LK_BIT_2	  - Error while writing
 *							AES_CRC_LK_BIT_2.
 *		- XNVM_EFUSE_ERR_WRITE_AES_WR_LK          - Error while writing
 *							AES_WR_LK.
 *		- XNVM_EFUSE_ERR_WRITE_USER_KEY0_WR_LK    - Error while writing
 *							USER_KEY0_WR_LK.
 *		- XNVM_EFUSE_ERR_WRITE_USER_KEY1_WR_LK    - Error while writing
 *							USER_KEY1_WR_LK.
 *		- XNVM_EFUSE_ERR_WRITE_USER_KEY0_CRC_LK   - Error while writing
 *							USER_KEY0_CRC_LK.
 *		- XNVM_EFUSE_ERR_WRITE_USER_KEY1_CRC_LK   - Error while writing
 *							USER_KEY1_CRC_LK.
 *		- XNVM_EFUSE_ERR_WRITE_SECDBG_DIS_BIT_1   - Error while writing
 *							SECDBG_DIS_BIT_1.
 *		- XNVM_EFUSE_ERR_WRITE_SECDBG_DIS_BIT_2	  - Error while writing
 *							SECDBG_DIS_BIT_2.
 *		- XNVM_EFUSE_ERR_WRITE_BOOTENV_WR_LK	  - Error while writing
 *							BOOTENV_WR_LK.
 *		- XNVM_EFUSE_ERR_WRITE_REG_INIT_DIS_BIT_1 - Error while writing
 * 							REG_INIT_DIS_BIT_1.
 *		- XNVM_EFUSE_ERR_WRITE_REG_INIT_DIS_BIT_2 - Error while writing
 *							REG_INIT_DIS_BIT_2.
 *
 ******************************************************************************/
static int XNvm_EfuseWriteSecCtrl(const XNvm_EfuseSecCtrlBits *SecCtrl)
{
	volatile int Status = XST_FAILURE;
	XNvm_EfuseType EfuseType = XNVM_EFUSE_PAGE_0;
	u32 Row = XNVM_EFUSE_SECURITY_CONTROL_ROW;
	u32 RowDataVal = 0U;

	if (SecCtrl == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if ((SecCtrl->AesDis != FALSE) ||
		(SecCtrl->JtagErrOutDis != FALSE) ||
		(SecCtrl->JtagDis != FALSE) ||
		(SecCtrl->Ppk0WrLk != FALSE) ||
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
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	else {
		Status = XST_SUCCESS;
		goto END;
	}

	if ((SecCtrl->AesDis != FALSE) &&
		((RowDataVal &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_AES_DIS_MASK) !=
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_AES_DIS_MASK)) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
					(u32)XNVM_EFUSE_SEC_AES_DIS);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_AES_DIS);
			goto END;
		}
	}

	if ((SecCtrl->JtagErrOutDis != FALSE) &&
		((RowDataVal &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_JTAG_ERROUT_DIS_MASK) !=
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_JTAG_ERROUT_DIS_MASK)) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
					(u32)XNVM_EFUSE_SEC_JTAG_ERROUT_DIS);
		if (Status != XST_SUCCESS) {
			Status = (Status |
					XNVM_EFUSE_ERR_WRITE_JTAG_ERROUT_DIS);
			goto END;
		}
	}

	if ((SecCtrl->JtagDis != FALSE) &&
		((RowDataVal &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_JTAG_DIS_MASK) !=
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_JTAG_DIS_MASK)) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
					(u32)XNVM_EFUSE_SEC_JTAG_DIS);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_JTAG_DIS);
			goto END;
		}
	}

	if ((SecCtrl->Ppk0WrLk != FALSE) &&
		((RowDataVal &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK0_WR_LK_MASK) !=
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK0_WR_LK_MASK)) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
					(u32)XNVM_EFUSE_SEC_PPK0_WRLK);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_PPK0_WR_LK);
			goto END;
		}
	}
	if ((SecCtrl->Ppk1WrLk != FALSE) &&
		((RowDataVal &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK1_WR_LK_MASK) !=
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK1_WR_LK_MASK)) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
					(u32)XNVM_EFUSE_SEC_PPK1_WRLK);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_PPK1_WR_LK);
			goto END;
		}
	}
	if ((SecCtrl->Ppk2WrLk != FALSE) &&
		((RowDataVal &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK2_WR_LK_MASK) !=
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK2_WR_LK_MASK)) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
					(u32)XNVM_EFUSE_SEC_PPK2_WRLK);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_PPK2_WR_LK);
			goto END;
		}
	}
	if ((SecCtrl->AesCrcLk != FALSE) &&
		((RowDataVal &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_AES_CRC_LK_1_0_MASK) ==
		0x00U)) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
					(u32)XNVM_EFUSE_SEC_AES_CRC_LK_BIT_0);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_AES_CRC_LK_BIT_0);
			goto END;
		}
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
					(u32)XNVM_EFUSE_SEC_AES_CRC_LK_BIT_1);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_AES_CRC_LK_BIT_1);
			goto END;
		}
	}
	if ((SecCtrl->AesWrLk != FALSE) &&
		((RowDataVal &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_AES_WR_LK_MASK) !=
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_AES_WR_LK_MASK)) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
					(u32)XNVM_EFUSE_SEC_AES_WRLK);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_AES_WR_LK);
			goto END;
		}
	}
	if ((SecCtrl->UserKey0CrcLk != FALSE) &&
		((RowDataVal &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_0_CRC_LK_MASK) !=
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_0_CRC_LK_MASK)) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
					(u32)XNVM_EFUSE_SEC_USER_KEY0_CRC_LK);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_USER_KEY0_CRC_LK);
			goto END;
		}
	}
	if ((SecCtrl->UserKey0WrLk != FALSE) &&
		((RowDataVal &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_0_WR_LK_MASK) !=
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_0_WR_LK_MASK)) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
					(u32)XNVM_EFUSE_SEC_USER_KEY0_WRLK);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_USER_KEY0_WR_LK);
			goto END;
		}

	}
	if ((SecCtrl->UserKey1CrcLk != FALSE) &&
		((RowDataVal &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_1_CRC_LK_MASK) !=
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_1_CRC_LK_MASK)) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
					(u32)XNVM_EFUSE_SEC_USER_KEY1_CRC_LK);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_USER_KEY1_CRC_LK);
			goto END;
		}
	}
	if ((SecCtrl->UserKey1WrLk != FALSE) &&
		((RowDataVal &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_1_WR_LK_MASK) !=
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_1_WR_LK_MASK)) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
					(u32)XNVM_EFUSE_SEC_USER_KEY1_WRLK);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_USER_KEY1_WR_LK);
			goto END;
		}

	}
	if ((SecCtrl->SecDbgDis != FALSE) &&
		((RowDataVal &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_SEC_DEBUG_DIS_MASK) ==
		0x00U)) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
					(u32)XNVM_EFUSE_SEC_SECDBG_DIS_BIT_0);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_SECDBG_DIS_BIT_0);
			goto END;
		}
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
					(u32)XNVM_EFUSE_SEC_SECDBG_DIS_BIT_1);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_SECDBG_DIS_BIT_1);
			goto END;
		}
	}
	if ((SecCtrl->SecLockDbgDis != FALSE) &&
		((RowDataVal &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_SEC_LOCK_DBG_DIS_MASK) ==
		0x00U)) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
					(u32)XNVM_EFUSE_SEC_SECLOCKDBG_DIS_BIT_0);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_SECLOCKDBG_DIS_BIT_0);
			goto END;
		}
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
					(u32)XNVM_EFUSE_SEC_SECLOCKDBG_DIS_BIT_1);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_SECLOCKDBG_DIS_BIT_1);
			goto END;
		}
	}
	if ((SecCtrl->BootEnvWrLk != FALSE) &&
		((RowDataVal &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_BOOT_ENV_WR_LK_MASK) !=
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_BOOT_ENV_WR_LK_MASK)) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
					(u32)XNVM_EFUSE_SEC_BOOTENV_WRLK);
		if (Status != XST_SUCCESS) {
			Status = (Status |
			XNVM_EFUSE_ERR_WRITE_BOOTENV_WR_LK);
			goto END;
		}
	}
	if ((SecCtrl->RegInitDis != FALSE) &&
		((RowDataVal &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_REG_INIT_DIS_1_0_MASK) ==
		0x00U)) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
					(u32)XNVM_EFUSE_SEC_REG_INIT_DIS_BIT_0);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_REG_INIT_DIS_BIT_0);
			goto END;
		}
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(EfuseType, Row,
					(u32)XNVM_EFUSE_SEC_REG_INIT_DIS_BIT_1);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_REG_INIT_DIS_BIT_1);
			goto END;
		}
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function program Glitch Configuration.
 *
 * @param	WriteGlitchCfg - Pointer to glitch configuration data to be
 * 				programmed to eFuse.
 *
 * @return	- XST_SUCCESS - On Success.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM    - On Invalid Parameter.
 *		- XNVM_EFUSE_ERR_WRITE_GLITCH_CFG - Error in programming of
 *						glitch configuration.
 *
 ******************************************************************************/
static int XNvm_EfusePrgmGlitchCfgValues(const XNvm_EfuseGlitchCfgBits *WriteGlitchCfg)
{
	volatile int Status = XST_FAILURE;
	u32 PrgmGlitchCfg = 0U;

	if (WriteGlitchCfg == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
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

		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyRows(
				XNM_EFUSE_GLITCH_ANLG_TRIM_3,
				XNVM_EFUSE_GLITCH_NUM_OF_ROWS,
				XNVM_EFUSE_PAGE_0,
				&PrgmGlitchCfg);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_GLITCH_CFG);
			goto END;
		}
	}
	else {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs Glitch Write lock bit.
 *
 * @param	WriteGlitchCfg - Pointer to glitch configuration data to be
 *				written to eFuse.
 *
 * @return	- XST_SUCCESS - On Success.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM     - On Invalid Parameter.
 *		- XNVM_EFUSE_ERR_WRITE_GLITCH_WRLK - Error in programming
 *							 glitch write lock.
 *
 ******************************************************************************/
static int XNvm_EfusePrgmGlitchWriteLock(const XNvm_EfuseGlitchCfgBits *WriteGlitchCfg)
{
	int Status = XST_FAILURE;

	if (WriteGlitchCfg == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if ((WriteGlitchCfg->PrgmGlitch == TRUE)  &&
			(WriteGlitchCfg->GlitchDetWrLk == TRUE)) {
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNM_EFUSE_GLITCH_ANLG_TRIM_3,
				XNVM_EFUSE_GLITCH_WRLK_COLUMN);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_GLITCH_WRLK);
			goto END;
		}
	}
	else {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function enables the halt boot by ROM upon glitch detection.
 *
 * @param	WriteGlitchCfg - Pointer to glitch configuration data to be
 * 				written to eFuse.
 *
 * @return	- XST_SUCCESS - On Success.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM     - On Invalid Parameter.
 *		- XNVM_EFUSE_ERR_WRITE_GD_ROM_BITS - Error in programming of
 *							  Glitch ROM monitor.
 *
 ******************************************************************************/
static int XNvm_EfusePrgmGdRomHaltBootEn(const XNvm_EfuseGlitchCfgBits *WriteGlitchCfg)
{
	volatile int Status = XST_FAILURE;

	if (WriteGlitchCfg == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if ((WriteGlitchCfg->PrgmGlitch == TRUE)  &&
			(WriteGlitchCfg->GdHaltBootEn == TRUE)) {
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
			XNVM_EFUSE_MISC_CTRL_ROW,
			XNVM_EFUSE_GLITCH_HALT_EN_0_COLUMN);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_GD_ROM_BITS);
			goto END;
		}

		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_MISC_CTRL_ROW,
				XNVM_EFUSE_GLITCH_HALT_EN_1_COLUMN);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_GD_ROM_BITS);
			goto END;
		}
	}
	else {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function enables the glitch detection monitoring by ROM.
 *
 * @param	WriteGlitchCfg - Pointer to glitch configuration data.
 *
 * @return	- XST_SUCCESS - On Success.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM     - On Invalid Parameter.
 *		- XNVM_EFUSE_ERR_WRITE_GD_ROM_BITS - Error in programming of
 *							 Glitch ROM monitor.
 *
 ******************************************************************************/
static int XNvm_EfusePrgmGdRomMonEn(const XNvm_EfuseGlitchCfgBits *WriteGlitchCfg)
{
	int Status = XST_FAILURE;

	if (WriteGlitchCfg == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if ((WriteGlitchCfg->PrgmGlitch == TRUE) &&
			(WriteGlitchCfg->GdRomMonitorEn == TRUE)) {
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
					XNVM_EFUSE_MISC_CTRL_ROW,
					XNVM_EFUSE_GLITCH_ROM_EN_COLUMN);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_GD_ROM_BITS);
			goto END;
		}
	}
	else {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs security miscellaneous1 eFuses specified
 *              by user.
 *
 * @param	Misc1Bits	Pointer to the XNvm_EfuseSecMisc1Bits structure,
 * 				which holds the XNvm_EfuseSecMisc1Bits data to
 * 				be written to the eFuse.
 *
 * @return	- XST_SUCCESS 	- On success.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM		- Invalid parameter.
 *		- XNVM_EFUSE_ERR_WRITE_SECURITY_MISC_1	- Error while writing
 *							SECURITY_MISC1 ROW.
 *
 ******************************************************************************/
static int XNvm_EfuseWriteSecMisc1Fuses(const XNvm_EfuseSecMisc1Bits *Misc1Bits)
{
	volatile int Status = XST_FAILURE;
	u32 Row = XNVM_EFUSE_SECURITY_MISC_1_ROW;
	u32 RowDataVal = 0U;
	u32 WriteMask = 0U;

	if (Misc1Bits == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if ((Misc1Bits->LpdMbistEn != FALSE) ||
		(Misc1Bits->PmcMbistEn != FALSE) ||
		(Misc1Bits->LpdNocScEn != FALSE) ||
		(Misc1Bits->SysmonVoltMonEn != FALSE) ||
		(Misc1Bits->SysmonTempMonEn != FALSE)) {

		Status = XNvm_EfuseReadCache(Row, &RowDataVal);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	else {
		Status = XST_SUCCESS;
		goto END;
	}

	if (Misc1Bits->LpdMbistEn == TRUE) {
		WriteMask = (WriteMask | ((~RowDataVal) &
				XNVM_EFUSE_CACHE_SEC_MISC_1_LPD_MBIST_EN_2_0_MASK));
	}

	if (Misc1Bits->PmcMbistEn == TRUE) {
		WriteMask = (WriteMask | ((~RowDataVal) &
				XNVM_EFUSE_CACHE_SEC_MISC_1_PMC_MBIST_EN_2_0_MASK));
	}

	if (Misc1Bits->LpdNocScEn == TRUE) {
		WriteMask = (WriteMask | ((~RowDataVal) &
				XNVM_EFUSE_CACHE_SEC_MISC_1_LPD_NOC_SC_EN_2_0_MASK));
	}

	if (Misc1Bits->SysmonVoltMonEn == TRUE) {
		WriteMask = (WriteMask | ((~RowDataVal) &
				XNVM_EFUSE_CACHE_SEC_MISC_1_SYSMON_VOLT_MON_EN_1_0_MASK));
	}

	if (Misc1Bits->SysmonTempMonEn == TRUE) {
		WriteMask = (WriteMask | ((~RowDataVal) &
				XNVM_EFUSE_CACHE_SEC_MISC_1_SYSMON_TEMP_MON_EN_1_0_MASK));
	}

	Status = XNvm_EfusePgmAndVerifyRows(
			XNVM_EFUSE_SECURITY_MISC_1_ROW,
			XNVM_EFUSE_SECURITY_MISC_1_NUM_OF_ROWS,
			XNVM_EFUSE_PAGE_0,
			&WriteMask);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_WRITE_SECURITY_MISC_1);
	}

END :
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs Boot Environmental Control eFuses with
 * 		values specified by user.
 *
 * @param	BootEnvCtrl 	Pointer to the XNvm_EfuseBootEnvCtrlBits
 * 				structure, which holds the
 * 				XNvm_EfuseBootEnvCtrlBits data to be written
 * 				to the eFuse.
 *
 * @return	- XST_SUCCESS - On success.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM       -  Invalid parameter.
 *		- XNVM_EFUSE_ERR_WRITE_BOOT_ENV_CTRL - 	Error while writing
 *							BOOT_ENV_CTRL ROW.
 *
 ******************************************************************************/
static int XNvm_EfuseWriteBootEnvCtrl(const XNvm_EfuseBootEnvCtrlBits *BootEnvCtrl)
{
	int Status = XST_FAILURE;
	u32 Row = XNVM_EFUSE_BOOT_ENV_CTRL_ROW;
	u32 RowDataVal = 0U;
	u32 WriteMask = 0U;

	if (BootEnvCtrl == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if ((BootEnvCtrl->SysmonTempEn != FALSE) ||
		(BootEnvCtrl->SysmonVoltEn != FALSE) ||
		(BootEnvCtrl->SysmonVoltSoc != FALSE) ||
		(BootEnvCtrl->PrgmSysmonTempHot != FALSE) ||
		(BootEnvCtrl->PrgmSysmonVoltPmc != FALSE) ||
		(BootEnvCtrl->PrgmSysmonVoltPslp != FALSE) ||
		(BootEnvCtrl->PrgmSysmonTempCold != FALSE)) {

		Status = XNvm_EfuseReadCache(Row, &RowDataVal);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	else {
		Status = XST_SUCCESS;
		goto END;
	}

	if (BootEnvCtrl->SysmonTempEn == TRUE) {
		WriteMask = (WriteMask | ((~RowDataVal) &
			XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_EN_MASK));
	}

	if (BootEnvCtrl->SysmonVoltEn == TRUE) {
		WriteMask = (WriteMask | ((~RowDataVal) &
			XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_EN_MASK));
	}

	if (BootEnvCtrl->SysmonVoltSoc == TRUE) {
		WriteMask = (WriteMask | ((~RowDataVal) &
			XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_SOC_MASK));
	}

	if (BootEnvCtrl->PrgmSysmonTempHot == TRUE) {
		if ((RowDataVal &
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_HOT_MASK) == 0U) {
			WriteMask = (WriteMask |
			(((u32)BootEnvCtrl->SysmonTempHot <<
			XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_HOT_SHIFT) &
			XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_HOT_MASK));
		}
	}

	if (BootEnvCtrl->PrgmSysmonVoltPmc == TRUE) {
		if ((RowDataVal &
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_PMC_MASK) == 0U) {
			WriteMask = (WriteMask |
			(((u32)BootEnvCtrl->SysmonVoltPmc <<
			XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_PMC_SHIFT) &
			XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_PMC_MASK));
		}
	}

	if (BootEnvCtrl->PrgmSysmonVoltPslp == TRUE) {
		if ((RowDataVal &
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_PSLP_MASK) == 0U) {
			WriteMask = (WriteMask |
			(((u32)BootEnvCtrl->SysmonVoltPslp <<
			XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_PSLP_SHIFT) &
			XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_PSLP_MASK));
		}
	}

	if (BootEnvCtrl->PrgmSysmonTempCold == TRUE) {
		if ((RowDataVal &
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_COLD_MASK) == 0U) {
			WriteMask =
			(WriteMask | (((u32)BootEnvCtrl->SysmonTempCold <<
			XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_COLD_SHIFT) &
			XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_COLD_MASK));
		}
	}

	Status = XNvm_EfusePgmAndVerifyRows(
			XNVM_EFUSE_BOOT_ENV_CTRL_ROW,
			XNVM_EFUSE_BOOT_ENV_CTRL_NUM_OF_ROWS,
			XNVM_EFUSE_PAGE_0,
			&WriteMask);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_WRITE_BOOT_ENV_CTRL);
		goto END;
	}

	Status = XST_SUCCESS;

END :
	return Status;
}
/******************************************************************************/
/**
 * @brief	This function is used to program below eFuses -
 * 		AES key
 * 		User key 0
 * 		User key 1.
 *
 * @param	Keys - Pointer to the XNvm_EfuseAesKeys structure, which holds
 * 			Aes key data to be programmed to eFuse.
 *
 * @return	- XST_SUCCESS - Specified data read.
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
static int XNvm_EfusePrgmAesKeys(const XNvm_EfuseAesKeys *Keys)
{
	int Status = XST_FAILURE;
	u32 Crc;

	if (Keys == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (Keys->PrgmAesKey == TRUE) {
		Status = XNvm_EfusePgmAndVerifyRows(
				XNVM_EFUSE_AES_KEY_START_ROW,
				XNVM_EFUSE_AES_KEY_NUM_OF_ROWS,
				XNVM_EFUSE_PAGE_0, Keys->AesKey);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_AES_KEY);
			goto END;
		}
		Status = XNvm_EfuseCacheLoadAndProtectionChecks();
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_AES_KEY);
			goto END;
		}
		Crc = XNvm_AesCrcCalc(Keys->AesKey);

		Status = XNvm_EfuseCheckAesKeyCrc(XNVM_EFUSE_AES_CRC_REG_OFFSET,
					XNVM_EFUSE_CTRL_STATUS_AES_CRC_DONE_MASK,
					XNVM_EFUSE_CTRL_STATUS_AES_CRC_PASS_MASK,
					Crc);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_AES_KEY);
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
				XNVM_EFUSE_ERR_WRITE_USER_KEY0);
			goto END;
		}
		Status = XNvm_EfuseCacheLoadAndProtectionChecks();
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_USER_KEY0);
			goto END;
		}
		Crc = XNvm_AesCrcCalc(Keys->UserKey0);

		Status = XNvm_EfuseCheckAesKeyCrc(
			XNVM_EFUSE_AES_USR_KEY0_CRC_REG_OFFSET,
			XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_0_CRC_DONE_MASK,
			XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_0_CRC_PASS_MASK,
			Crc);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_USER_KEY0);
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
				XNVM_EFUSE_ERR_WRITE_USER_KEY1);
		goto END;
		}
		Status = XNvm_EfuseCacheLoadAndProtectionChecks();
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_USER_KEY1);
			goto END;
		}
		Crc = XNvm_AesCrcCalc(Keys->UserKey1);

		Status = XNvm_EfuseCheckAesKeyCrc(
			XNVM_EFUSE_AES_USR_KEY1_CRC_REG_OFFSET,
			XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_1_CRC_DONE_MASK,
			XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_1_CRC_PASS_MASK,
			Crc);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_USER_KEY1);
			goto END;
		}
	}
	Status = XST_SUCCESS;
END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function is used to program below eFuses -
 * 			PPK0/PPK1/PPK2 hash.
 *
 * @param	Hash - Pointer to the XNvm_EfusePpkHash structure which holds
 * 			ppk hash data to be programmed to eFuse.
 *
 * @return	- XST_SUCCESS - Specified data read.
 *		- XNVM_EFUSE_ERR_WRITE_PPK0_HASH - Error while writing PPK0 Hash.
 *		- XNVM_EFUSE_ERR_WRITE_PPK1_HASH - Error while writing PPK1 Hash.
 * 		- XNVM_EFUSE_ERR_WRITE_PPK2_HASH - Error while writing PPK2 Hash.
 *
 ******************************************************************************/
static int XNvm_EfusePrgmPpkHash(const XNvm_EfusePpkHash *Hash)
{
	int Status = XST_FAILURE;

	if (Hash == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (Hash->PrgmPpk0Hash == TRUE) {
		Status = XNvm_EfusePgmAndVerifyRows(
				XNVM_EFUSE_PPK_0_HASH_START_ROW,
				XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS,
				XNVM_EFUSE_PAGE_0,
				Hash->Ppk0Hash);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_PPK0_HASH);
			goto END;
		}
	}
	if (Hash->PrgmPpk1Hash == TRUE) {
		Status = XNvm_EfusePgmAndVerifyRows(
				XNVM_EFUSE_PPK_1_HASH_START_ROW,
				XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS,
				XNVM_EFUSE_PAGE_0,
				Hash->Ppk1Hash);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_PPK1_HASH);
			goto END;
		}
	}
	if (Hash->PrgmPpk2Hash == TRUE) {
		Status = XNvm_EfusePgmAndVerifyRows(
				XNVM_EFUSE_PPK_2_HASH_START_ROW,
				XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS,
				XNVM_EFUSE_PAGE_0,
				Hash->Ppk2Hash);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_PPK2_HASH);
			goto END;
		}
	}
	Status = XST_SUCCESS;
END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function is used to program below DEC_ONLY fuses.
 *
 * @param	DecOnly - Pointer to XNvm_EfuseDecOnly structure which holds
 * 			DecOnly flag which says to program eFuse.
 *
 * @return	- XST_SUCCESS - Specified data read.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM        - On Invalid Parameter.
 *		- XNVM_EFUSE_ERR_WRITE_DEC_EFUSE_ONLY - Error in DEC_ONLY
 *							programming.
 *		- XNVM_EFUSE_ERR_CACHE_PARITY         - Error in Cache reload.
 *
 ******************************************************************************/
static int XNvm_EfusePrgmDecOnly(const XNvm_EfuseDecOnly *DecOnly)
{
	int Status = XST_FAILURE;
	u32 PrgmDecOnly = XNVM_EFUSE_CACHE_DEC_EFUSE_ONLY_MASK;

	if (DecOnly == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	if (DecOnly->PrgmDecOnly == TRUE) {
		Status = XNvm_EfusePgmAndVerifyRows(
				XNVM_EFUSE_DEC_EFUSE_ONLY_ROW,
				XNVM_EFUSE_DEC_EFUSE_ONLY_NUM_OF_ROWS,
				XNVM_EFUSE_PAGE_0,
				&PrgmDecOnly);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_DEC_EFUSE_ONLY);
			goto END;
		}
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function is used to program all IVs.
 *
 * @param	Ivs - Pointer to the XNvm_EfuseIvs structure which holds IV data
 * 			to be programmed to eFuse.
 *
 * @return	- XST_SUCCESS - Specified data read
 *		- XNVM_EFUSE_ERR_INVALID_PARAM           - On Invalid Parameter.
 * 		- XNVM_EFUSE_ERR_WRITE_META_HEADER_IV    - Error while writing
 *							   Meta Iv.
 *		- XNVM_EFUSE_ERR_WRITE_BLK_OBFUS_IV 	 - Error while writing
 *							   BlkObfus IV.
 *		- XNVM_EFUSE_ERR_WRITE_PLM_IV 		 - Error while writing
 *							   PLM IV.
 * 		- XNVM_EFUSE_ERR_WRITE_DATA_PARTITION_IV - Error while writing Data
 *							   Partition IV.
 *
 ******************************************************************************/
static int XNvm_EfusePrgmIVs(const XNvm_EfuseIvs *Ivs)
{
	volatile int Status = XST_FAILURE;
	u32 PrgmIv[XNVM_EFUSE_IV_NUM_OF_ROWS];

	if (Ivs == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
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
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyRows(
			XNVM_EFUSE_META_HEADER_IV_START_ROW,
			XNVM_EFUSE_IV_NUM_OF_ROWS,
			XNVM_EFUSE_PAGE_0,
			PrgmIv);

		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_META_HEADER_IV_RANGE);
			goto END;
		}
	}
	if (Ivs->PrgmBlkObfusIv == TRUE) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyRows(
			XNVM_EFUSE_BLACK_OBFUS_IV_START_ROW,
			XNVM_EFUSE_IV_NUM_OF_ROWS,
			XNVM_EFUSE_PAGE_0,
			Ivs->BlkObfusIv);

		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_BLK_IV);
			goto END;
		}
	}
	if (Ivs->PrgmPlmIv == TRUE) {
		Status = XST_FAILURE;
		Status = XNvm_EfuseComputeProgrammableBits(Ivs->PlmIv,
					PrgmIv,
					XNVM_EFUSE_PLM_IV_START_ROW,
					(XNVM_EFUSE_PLM_IV_START_ROW +
					XNVM_EFUSE_IV_NUM_OF_ROWS));
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyRows(
			XNVM_EFUSE_PLM_IV_START_ROW,
			XNVM_EFUSE_IV_NUM_OF_ROWS,
			XNVM_EFUSE_PAGE_0,
			PrgmIv);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_PLM_IV_RANGE);
			goto END;
		}
	}
	if (Ivs->PrgmDataPartitionIv == TRUE) {
		Status = XST_FAILURE;
		Status = XNvm_EfuseComputeProgrammableBits(
				Ivs->DataPartitionIv,
				PrgmIv,
				XNVM_EFUSE_DATA_PARTITION_IV_START_ROW,
				(XNVM_EFUSE_DATA_PARTITION_IV_START_ROW +
				XNVM_EFUSE_IV_NUM_OF_ROWS));
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyRows(
			XNVM_EFUSE_DATA_PARTITION_IV_START_ROW,
			XNVM_EFUSE_IV_NUM_OF_ROWS,
			XNVM_EFUSE_PAGE_0,
			PrgmIv);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_DATA_PARTITION_IV_RANGE);
			goto END;
		}
	}

	Status = XST_SUCCESS;
END :
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
 * @param	StartRow - A 32 bit StartRow value of an expected Programmable Bits.
 * @param	EndRow   - A 32 bit EndRow value of an expected Programmable Bits.
 *
 * @return	- XST_SUCCESS - if the eFuse data computation is successful.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *		- XNVM_EFUSE_ERR_CACHE_PARITY  - Error in Cache reload.
 *
 ******************************************************************************/
static int XNvm_EfuseComputeProgrammableBits(const u32 *ReqData, u32 *PrgmData,
						u32 StartRow, u32 EndRow)
{
	int Status = XST_FAILURE;
	u32 ReadReg = 0U;
	u32 Index;

	if ((ReqData == NULL) ||
		(PrgmData == NULL)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	if (((StartRow > (XNVM_EFUSE_TOTAL_NUM_OF_ROWS - 1U)) ||
		(EndRow > XNVM_EFUSE_TOTAL_NUM_OF_ROWS))) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	for (Index = StartRow; Index < EndRow; Index++) {
		Status = XNvm_EfuseReadCache(Index, &ReadReg);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		PrgmData[Index - StartRow] = (~ReadReg) &
						ReqData[Index - StartRow];
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs Revocation Id eFuses.
 *
 * @param	RevokeIds - Pointer to XNvm_RevokeIdEfuse that contains
 * 						Revocation Id to write.
 *
 * @return	- XST_SUCCESS - On successful write of Revocation Id.
 *		- XNVM_EFUSE_ERR_WRITE_REVOCATION_IDS - Error in writing
 *							revoke id eFuses.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *		- XNVM_EFUSE_ERR_CACHE_PARITY  - Error in Cache reload.

 ******************************************************************************/
static int XNvm_EfusePrgmRevocationIdFuses(const XNvm_EfuseRevokeIds *RevokeIds)
{
	volatile int Status = XST_FAILURE;
	u32 PrgmRevokeIds[XNVM_NUM_OF_REVOKE_ID_FUSES];

	if (RevokeIds == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (RevokeIds->PrgmRevokeId != FALSE) {
		Status = XNvm_EfuseComputeProgrammableBits(RevokeIds->RevokeId,
						PrgmRevokeIds,
						XNVM_EFUSE_REVOCATION_ID_0_ROW,
						(XNVM_EFUSE_REVOCATION_ID_0_ROW +
						XNVM_NUM_OF_REVOKE_ID_FUSES));
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyRows(
				XNVM_EFUSE_REVOCATION_ID_0_ROW,
				XNVM_NUM_OF_REVOKE_ID_FUSES,
				XNVM_EFUSE_PAGE_0,
				PrgmRevokeIds);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_REVOCATION_IDS);
			goto END;
		}
	}

	Status = XST_SUCCESS;
END:
	return Status;

}

/******************************************************************************/
/**
 * @brief	This function programs OffChip Revoke eFuses.
 *
 * @param	OffChipIds - Pointer to XNvm_EfuseOffChipIds that contains
 * 						OffChipId to write.
 *
 * @return	- XST_SUCCESS - On successful write of OffChipId.
 *		- XNVM_EFUSE_ERR_WRITE_OFFCHIP_REVOKE_IDS - Error in writing
 *							OffChipId eFuses.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *		- XNVM_EFUSE_ERR_CACHE_PARITY  - Error in Cache reload.
 ******************************************************************************/
static int XNvm_EfusePrgmOffChipRevokeFuses(const XNvm_EfuseOffChipIds *OffChipIds)
{
	int Status = XST_FAILURE;
	u32 PrgmOffChipIds[XNVM_NUM_OF_REVOKE_ID_FUSES];

	if (OffChipIds == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (OffChipIds->PrgmOffchipId != FALSE) {
		Status = XNvm_EfuseComputeProgrammableBits(OffChipIds->OffChipId,
					PrgmOffChipIds,
					XNVM_EFUSE_OFFCHIP_REVOKE_0_ROW,
					(XNVM_EFUSE_OFFCHIP_REVOKE_0_ROW +
					XNVM_NUM_OF_OFFCHIP_ID_FUSES));
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Status = XNvm_EfusePgmAndVerifyRows(
				XNVM_EFUSE_OFFCHIP_REVOKE_0_ROW,
				XNVM_NUM_OF_OFFCHIP_ID_FUSES,
				XNVM_EFUSE_PAGE_0,
				PrgmOffChipIds);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_OFFCHIP_REVOKE_IDS);
			goto END;
		}
	}

	Status = XST_SUCCESS;
END:
	return Status;

}
/******************************************************************************/
/**
 * @brief	This function revokes the Ppk.
 *
 * @param	PpkSelect - Pointer to XNvm_EfuseMiscCtrlBits structure which
 * 			holds PPK_INVALID bits data to be programmed to eFuse.
 *
 * @return	- XST_SUCCESS - On Successful write of Ppk revoke efuses.
 *		- XNVM_EFUSE_ERR_WRITE_PPK0_INVALID_BIT_0 - Error in writing
 *							PPK0 Invalid bit0.
 *		- XNVM_EFUSE_ERR_WRITE_PPK0_INVALID_BIT_1 - Error in writing
 *							PPK0 Invalid bit1.
 *		- XNVM_EFUSE_ERR_WRITE_PPK1_INVALID_BIT_0 - Error in writing
 *							PPK1 Invalid bit0.
 *		- XNVM_EFUSE_ERR_WRITE_PPK1_INVALID_BIT_1 - Error in writing
 *							PPK1 Invalid bit1.
 *		- XNVM_EFUSE_ERR_WRITE_PPK2_INVALID_BIT_0 - Error in writing
 *							PPK2 Invalid bit0.
 *		- XNVM_EFUSE_ERR_WRITE_PPK2_INVALID_BIT_1 - Error in writing
 *							PPK2 Invalid bit1.
 *
 ******************************************************************************/
static int XNvm_EfusePrgmPpkRevokeFuses(const XNvm_EfuseMiscCtrlBits *PpkSelect)
{
	volatile int Status = XST_FAILURE;
	u32 RowData = XNVM_EFUSE_SEC_DEF_VAL_ALL_BIT_SET;

	if (PpkSelect == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if ((PpkSelect->Ppk0Invalid != FALSE) ||
		(PpkSelect->Ppk1Invalid != FALSE) ||
		(PpkSelect->Ppk2Invalid != FALSE)) {

		Status = XNvm_EfuseReadCache(XNVM_EFUSE_MISC_CTRL_ROW,
						&RowData);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_RD_MISC_CTRL_BITS);
			goto END;
		}
	}
	else {
		Status = XST_SUCCESS;
		goto END;
	}

	if ((PpkSelect->Ppk0Invalid != FALSE) &&
		((RowData & XNVM_EFUSE_CACHE_MISC_CTRL_PPK0_INVLD_1_0_MASK) ==
		0x00U)) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_MISC_CTRL_ROW,
				(u32)XNVM_EFUSE_MISC_PPK0_INVALID_BIT_0);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_PPK0_INVALID_BIT_0);
			goto END;
		}
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(
				XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_MISC_CTRL_ROW,
				(u32)XNVM_EFUSE_MISC_PPK0_INVALID_BIT_1);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_PPK0_INVALID_BIT_1);
			goto END;
		}
	}
	if ((PpkSelect->Ppk1Invalid != FALSE) &&
		((RowData & XNVM_EFUSE_CACHE_MISC_CTRL_PPK1_INVLD_1_0_MASK) ==
		0x00U)) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_MISC_CTRL_ROW,
				(u32)XNVM_EFUSE_MISC_PPK1_INVALID_BIT_0);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_PPK1_INVALID_BIT_0);
			goto END;
		}
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_MISC_CTRL_ROW,
				(u32)XNVM_EFUSE_MISC_PPK1_INVALID_BIT_1);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_PPK1_INVALID_BIT_1);
			goto END;
		}
	}
	if ((PpkSelect->Ppk2Invalid != FALSE) &&
		((RowData & XNVM_EFUSE_CACHE_MISC_CTRL_PPK2_INVLD_1_0_MASK) ==
		0x00U)) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_MISC_CTRL_ROW,
				(u32)XNVM_EFUSE_MISC_PPK2_INVALID_BIT_0);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				 XNVM_EFUSE_ERR_WRITE_PPK2_INVALID_BIT_0);
			goto END;
		}
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_MISC_CTRL_ROW,
				(u32)XNVM_EFUSE_MISC_PPK2_INVALID_BIT_1);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				 XNVM_EFUSE_ERR_WRITE_PPK2_INVALID_BIT_1);
			goto END;
		}
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function enables halt boot on general errors other than
 * 		environmental and glitch identified by ROM.
 *
 * @param	MiscCtrlData - Pointer to XNvm_EfuseMiscCtrlBits structure which
 * 			holds MiscCtrlBits data to be programmed to eFuse
 *
 * @return	- XST_SUCCESS - On Success.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM        - On Invalid Parameter.
 *		- XNVM_EFUSE_ERR_WRITE_HALT_BOOT_BITS - Error in programming of
 *							ROM flow control halt boot
 * 							bits for generic and
 *							environment errors.
 *
 ******************************************************************************/
static int XNvm_EfusePrgmHaltBootonError(const XNvm_EfuseMiscCtrlBits *MiscCtrlData)
{
	volatile int Status = XST_FAILURE;

	if (MiscCtrlData == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (MiscCtrlData->HaltBootError == TRUE) {
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
			XNVM_EFUSE_MISC_CTRL_ROW,
			XNVM_EFUSE_HALT_BOOT_ERROR_0);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_HALT_BOOT_BITS);
			goto END;
		}

		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_MISC_CTRL_ROW,
				XNVM_EFUSE_HALT_BOOT_ERROR_1);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_HALT_BOOT_BITS);
			goto END;
		}
	}
	else {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function enables the halt boot on environmental errors
 * 		identified by ROM.
 *
 * @param	MiscCtrlData - Pointer to XNvm_EfuseMiscCtrlBits structure which
 *			holds MiscCtrlBits data to be programmed to eFuse
 *
 * @return	- XST_SUCCESS - On Success.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM        - On Invalid Parameter.
 *		- XNVM_EFUSE_ERR_WRITE_HALT_BOOT_BITS - Error in programming of
 *							ROM flow control halt
 *							boot bits for generic
 *							and environment errors.
 *
 ******************************************************************************/
static int XNvm_EfusePrgmHaltBootEnvError(const XNvm_EfuseMiscCtrlBits *MiscCtrlData)
{
	volatile int Status = XST_FAILURE;

	if (MiscCtrlData == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (MiscCtrlData->HaltBootEnv == TRUE) {
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
			XNVM_EFUSE_MISC_CTRL_ROW,
			XNVM_EFUSE_HALT_ENV_ERROR_0);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_HALT_BOOT_BITS);
			goto END;
		}

		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_MISC_CTRL_ROW,
				XNVM_EFUSE_HALT_ENV_ERROR_1);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_HALT_BOOT_BITS);
			goto END;
		}
	}
	else {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs CryptoKat eFuse in MiscCtrl eFuse row.
 *
 * @param	MiscCtrlData - Pointer to XNvm_EfuseMiscCtrlBits structure which
 *			holds MiscCtrlBits data to be programmed to eFuse
 *
 * @return	- XST_SUCCESS - On Success.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM		- On Invalid Parameter.
 *		- XNVM_EFUSE_ERR_WRITE_CRYPTO_KAT_EN	- Error in writing
 *							CryptoKat eFuse.
 ******************************************************************************/
static int XNvm_EfusePrgmCryptoKatEn(const XNvm_EfuseMiscCtrlBits *MiscCtrlData)
{
	int Status = XST_FAILURE;

	if (MiscCtrlData == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (MiscCtrlData->CryptoKatEn == TRUE) {
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
			XNVM_EFUSE_MISC_CTRL_ROW,
			(u32)XNVM_EFUSE_MISC_CRYPTO_KAT_EN);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_CRYPTO_KAT_EN);
		}
	}
	else {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs LbistEn eFuse in MiscCtrl eFuse row.
 *
 * @param	MiscCtrlData - Pointer to XNvm_EfuseMiscCtrlBits structure which
 *			holds MiscCtrlBits data to be programmed to eFuse
 *
 * @return	- XST_SUCCESS - On Success.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM		- On Invalid Parameter.
 *		- XNVM_EFUSE_ERR_WRITE_LBIST_EN		- Error in writing
 *							LbistEn eFuse.
 ******************************************************************************/
static int XNvm_EfusePrgmLbistEn(const XNvm_EfuseMiscCtrlBits *MiscCtrlData)
{
	int Status = XST_FAILURE;

	if (MiscCtrlData == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (MiscCtrlData->LbistEn == TRUE) {
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
			XNVM_EFUSE_MISC_CTRL_ROW,
			(u32)XNVM_EFUSE_MISC_LBIST_EN);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_LBIST_EN);
		}
	}
	else {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs SafetyMissionEn eFuse in MiscCtrl
 * 		eFuse row.
 *
 * @param	MiscCtrlData - Pointer to XNvm_EfuseMiscCtrlBits structure which
 *			holds MiscCtrlBits data to be programmed to eFuse
 *
 * @return	- XST_SUCCESS - On Success.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM		- On Invalid Parameter.
 *		- XNVM_EFUSE_ERR_WRITE_SAFETY_MISSION_EN- Error in writing
 *							SafetyMissionEn eFuse.
 ******************************************************************************/
static int XNvm_EfusePrgmSafetyMissionEn(
		const XNvm_EfuseMiscCtrlBits *MiscCtrlData)
{
	int Status = XST_FAILURE;

	if (MiscCtrlData == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (MiscCtrlData->SafetyMissionEn == TRUE) {
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
			XNVM_EFUSE_MISC_CTRL_ROW,
			(u32)XNVM_EFUSE_MISC_SAFETY_MISSION_EN);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_SAFETY_MISSION_EN);
		}
	}
	else {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function validates all IVs requested for programming.
 *
 * @param	EfuseIv - Pointer to XNvm_EfuseIvs structure which holds IV data
 * 			to be programmed to eFuse.
 *
 * @return	- XST_SUCCESS - if validation is successful.
 *		- XNVM_EFUSE_ERR_WRITE_META_HEADER_IV_RANGE - Error in
 *							Metaheader IV range
 *							write request.
 *		- XNVM_EFUSE_ERR_BLK_OBFUS_IV_ALREADY_PRGMD - Error in Blk Obfus Iv
 *							  write request.
 *		- XNVM_EFUSE_ERR_WRITE_PLM_IV_RANGE - Error in Plm Iv range
 *							write request.
 *		- XNVM_EFUSE_ERR_WRITE_DATA_PARTITION_IV_RANGE - Error in
 *							Data Partition Iv range
 *							write request.
 *
 ******************************************************************************/
static int XNvm_EfuseValidateIVsWriteReq(const XNvm_EfuseIvs *EfuseIv)
{
	int Status = XST_FAILURE;

	if (EfuseIv == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (EfuseIv->PrgmMetaHeaderIv == TRUE) {
		Status = XNvm_EfuseValidateIV(EfuseIv->MetaHeaderIv,
					XNVM_EFUSE_META_HEADER_IV_START_ROW);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_META_HEADER_IV_RANGE);
			goto END;
		}
	}
	if (EfuseIv->PrgmBlkObfusIv == TRUE) {
		Status = XNvm_EfuseCheckZeros(XNVM_EFUSE_BLACK_OBFUS_IV_START_ROW,
					(XNVM_EFUSE_BLACK_OBFUS_IV_START_ROW +
					XNVM_EFUSE_IV_NUM_OF_ROWS));
		if (Status != XST_SUCCESS) {
			Status = (int)XNVM_EFUSE_ERR_BLK_OBFUS_IV_ALREADY_PRGMD;
			goto END;
		}
	}
	if (EfuseIv->PrgmPlmIv == TRUE) {
		Status = XNvm_EfuseValidateIV(EfuseIv->PlmIv,
					XNVM_EFUSE_PLM_IV_START_ROW);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_PLM_IV_RANGE);
			goto END;
		}
	}
	if (EfuseIv->PrgmDataPartitionIv == TRUE) {
		Status = XNvm_EfuseValidateIV(EfuseIv->DataPartitionIv,
					XNVM_EFUSE_DATA_PARTITION_IV_START_ROW);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_DATA_PARTITION_IV_RANGE);
			goto END;
		}
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function Validates single IV requested for programming
 *		bit by bit.
 *
 * @param	Iv  - Pointer to Iv data to be programmed.
 * @param	Row - Start row of the Iv to be validated.
 *
 * @return	- XST_SUCCESS - if validation is successful.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *		- XNVM_EFUSE_ERR_BIT_CANT_REVERT - if requested to revert the
 *						   already programmed bit.
 *
 *******************************************************************************/
static int XNvm_EfuseValidateIV(const u32 *Iv, u32 Row)
{
	int Status = XST_FAILURE;
	u32 IvRowsRd[XNVM_EFUSE_IV_LEN_IN_WORDS];
	u32 IvRow;

	if (Iv == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	if ((Row != XNVM_EFUSE_META_HEADER_IV_START_ROW) &&
		(Row != XNVM_EFUSE_PLM_IV_START_ROW) &&
		(Row != XNVM_EFUSE_DATA_PARTITION_IV_START_ROW)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	Status = XNvm_EfuseReadCacheRange(Row,
		XNVM_EFUSE_IV_NUM_OF_ROWS, IvRowsRd);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	for (IvRow = 0U; IvRow < XNVM_EFUSE_IV_LEN_IN_WORDS;
			IvRow++) {

		if ((IvRowsRd[IvRow] & Iv[IvRow]) != IvRowsRd[IvRow]) {
			Status = (XNVM_EFUSE_ERR_BEFORE_PROGRAMMING |
					XNVM_EFUSE_ERR_BIT_CANT_REVERT);
			goto END;
		}
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function validates all Aes keys requested for programming.
 *
 * @param	Keys - Pointer to XNvm_EfuseAesKeys structure.
 *
 * @return	- XST_SUCCESS - if validation is successful.
 *		- XNVM_EFUSE_ERR_RD_SEC_CTRL_BITS	- Error in reading
 *							   Sec Ctrl efuses.
 *		- XNVM_EFUSE_ERR_AES_ALREADY_PRGMD       - Aes key already
 *								programmed.
 *		- XNVM_EFUSE_ERR_USER_KEY0_ALREADY_PRGMD - User key 0 is already
 *								programmed.
 *		- XNVM_EFUSE_ERR_USER_KEY1_ALREADY_PRGMD - User key 1 is already
 *							 	programmed.
 *		- XNVM_EFUSE_ERR_FUSE_PROTECTED  - Efuse is write protected.
 *		- XNVM_EFUSE_ERR_WRITE_AES_KEY   - Error in writing Aes key.
 *		- XNVM_EFUSE_ERR_WRITE_USER0_KEY - Error in writing User key 0.
 *		- XNVM_EFUSE_ERR_WRITE_USER1_KEY - Error in writing User key 1.
 *
 ******************************************************************************/
static int XNvm_EfuseValidateAesWriteReq(const XNvm_EfuseAesKeys *Keys)
{
	int Status = XST_FAILURE;
	XNvm_EfuseSecCtrlBits ReadBackSecCtrlBits;

	Status = XNvm_EfuseReadSecCtrlBits(
			&ReadBackSecCtrlBits);
	if(Status != XST_SUCCESS) {
		Status = (int)XNVM_EFUSE_ERR_RD_SEC_CTRL_BITS;
		goto END;
	}

	if (Keys->PrgmAesKey == TRUE) {
		Status = XNvm_EfuseCheckAesKeyCrc(XNVM_EFUSE_AES_CRC_REG_OFFSET,
					XNVM_EFUSE_CTRL_STATUS_AES_CRC_DONE_MASK,
					XNVM_EFUSE_CTRL_STATUS_AES_CRC_PASS_MASK,
					XNVM_EFUSE_CRC_AES_ZEROS);
		if (Status != XST_SUCCESS) {
			Status = (int)XNVM_EFUSE_ERR_AES_ALREADY_PRGMD;
			goto END;
		}
		if ((ReadBackSecCtrlBits.AesDis == TRUE) ||
			(ReadBackSecCtrlBits.AesWrLk == TRUE)) {
			Status = (XNVM_EFUSE_ERR_FUSE_PROTECTED |
				XNVM_EFUSE_ERR_WRITE_AES_KEY);
			goto END;
		}
	}
	if (Keys->PrgmUserKey0 == TRUE) {
		Status = XNvm_EfuseCheckAesKeyCrc(
			XNVM_EFUSE_AES_USR_KEY0_CRC_REG_OFFSET,
			XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_0_CRC_DONE_MASK,
			XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_0_CRC_PASS_MASK,
			XNVM_EFUSE_CRC_AES_ZEROS);
		if (Status != XST_SUCCESS) {
			Status = (int)XNVM_EFUSE_ERR_USER_KEY0_ALREADY_PRGMD;
			goto END;
		}
		if ((ReadBackSecCtrlBits.AesDis == TRUE) ||
			(ReadBackSecCtrlBits.UserKey0WrLk == TRUE)) {
			Status = (XNVM_EFUSE_ERR_FUSE_PROTECTED |
				XNVM_EFUSE_ERR_WRITE_USER_KEY0);
			goto END;
		}
	}
	if (Keys->PrgmUserKey1 == TRUE) {
		Status = XNvm_EfuseCheckAesKeyCrc(
			XNVM_EFUSE_AES_USR_KEY1_CRC_REG_OFFSET,
			XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_1_CRC_DONE_MASK,
			XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_1_CRC_PASS_MASK,
			XNVM_EFUSE_CRC_AES_ZEROS);
		if (Status != XST_SUCCESS) {
			Status = (int)XNVM_EFUSE_ERR_USER_KEY1_ALREADY_PRGMD;
			goto END;
		}
		if ((ReadBackSecCtrlBits.AesDis == TRUE) ||
			(ReadBackSecCtrlBits.UserKey1WrLk == TRUE)) {
			Status = (XNVM_EFUSE_ERR_FUSE_PROTECTED |
					XNVM_EFUSE_ERR_WRITE_USER_KEY1);
			goto END;
		}
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function Validates all PPK Hash requested for programming.
 *
 * @param	Hash - Pointer to XNvm_EfusePpkHash structure which holds
 * 			PpkHash data to be programmed to eFuse.
 *
 * @return	- XST_SUCCESS - if reads successfully.
 *		- XNVM_EFUSE_ERR_PPK0_HASH_ALREADY_PRGMD - Ppk0 hash already
 *							   programmed.
 *		- XNVM_EFUSE_ERR_PPK1_HASH_ALREADY_PRGMD - Ppk1 hash already
 *							   programmed.
 *		- XNVM_EFUSE_ERR_PPK2_HASH_ALREADY_PRGMD - Ppk2 hash already
 *							   programmed.
 *		- XNVM_EFUSE_ERR_FUSE_PROTECTED  - Efuse is write protected.
 *		- XNVM_EFUSE_ERR_WRITE_PPK0_HASH - Error in writing ppk0 hash.
 *		- XNVM_EFUSE_ERR_WRITE_PPK1_HASH - Error in writing ppk1 hash.
 *		- XNVM_EFUSE_ERR_WRITE_PPK2_HASH - Error in writing ppk2 hash.
 *
 ******************************************************************************/
static int XNvm_EfuseValidatePpkWriteReq(const XNvm_EfusePpkHash *Hash)
{
	int Status = XST_FAILURE;
	XNvm_EfuseSecCtrlBits ReadBackSecCtrlBits;

	if (Hash->PrgmPpk0Hash == TRUE) {
		Status = XNvm_EfuseCheckZeros(XNVM_EFUSE_PPK_0_HASH_START_ROW,
					(XNVM_EFUSE_PPK_0_HASH_START_ROW +
					XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS));
		if (Status != XST_SUCCESS) {
			Status = (int)XNVM_EFUSE_ERR_PPK0_HASH_ALREADY_PRGMD;
			goto END;
		}
	}
	if (Hash->PrgmPpk1Hash == TRUE) {
		Status = XNvm_EfuseCheckZeros(XNVM_EFUSE_PPK_1_HASH_START_ROW,
					(XNVM_EFUSE_PPK_1_HASH_START_ROW +
					XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS));
		if (Status != XST_SUCCESS) {
			Status = (int)XNVM_EFUSE_ERR_PPK1_HASH_ALREADY_PRGMD;
			goto END;
		}
	}
	if (Hash->PrgmPpk2Hash == TRUE) {
		Status = XNvm_EfuseCheckZeros(XNVM_EFUSE_PPK_2_HASH_START_ROW,
					(XNVM_EFUSE_PPK_2_HASH_START_ROW +
					XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS));
		if (Status != XST_SUCCESS) {
			Status = (int)XNVM_EFUSE_ERR_PPK2_HASH_ALREADY_PRGMD;
			goto END;
		}
	}

	Status = XNvm_EfuseReadSecCtrlBits(
			&ReadBackSecCtrlBits);
	if(Status != XST_SUCCESS) {
		Status = (int)XNVM_EFUSE_ERR_RD_SEC_CTRL_BITS;
		goto END;
	}

	if (Hash->PrgmPpk0Hash == TRUE) {
		if (ReadBackSecCtrlBits.Ppk0WrLk == TRUE) {
			Status = (XNVM_EFUSE_ERR_FUSE_PROTECTED |
					XNVM_EFUSE_ERR_WRITE_PPK0_HASH);
			goto END;
		}
	}
	if (Hash->PrgmPpk1Hash == TRUE) {
		if (ReadBackSecCtrlBits.Ppk1WrLk == TRUE) {
			Status = (XNVM_EFUSE_ERR_FUSE_PROTECTED |
					XNVM_EFUSE_ERR_WRITE_PPK1_HASH);
			goto END;
		}
	}
	if (Hash->PrgmPpk2Hash == TRUE) {
		if (ReadBackSecCtrlBits.Ppk2WrLk == TRUE) {
			Status = (XNVM_EFUSE_ERR_FUSE_PROTECTED |
					XNVM_EFUSE_ERR_WRITE_PPK2_HASH);
			goto END;
		}
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function validates DEC_ONLY eFuse programming request.
 *
 * @param	WriteReq - Pointer to XNvm_EfuseData structure, which holds
 * 				eFuse data to be programmed.
 *
 * @return	- XST_SUCCESS - if validation is successful.
 *  		- XNVM_EFUSE_ERR_INVALID_PARAM 	- On Invalid Parameter.
 *		- XNVM_EFUSE_ERR_DEC_ONLY_KEY_MUST_BE_PRGMD- Aes key should be
 *							 programmed for DEC_ONLY
 *							 eFuse programming.
 *		- XNVM_EFUSE_ERR_DEC_ONLY_IV_MUST_BE_PRGMD - Blk IV should be
 *  				 			 programmed for DEC_ONLY
 *				   			 eFuse programming.
 *		- XNVM_EFUSE_ERR_BIT_CANT_REVERT	- if requested to revert
 *							 already programmed bit.
 *
 ******************************************************************************/
static int XNvm_EfuseValidateDecOnlyWriteReq(const XNvm_EfuseData *WriteReq)
{
	int Status = XST_FAILURE;
	u32 SecurityMisc0 = 0U;

	if (WriteReq == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	SecurityMisc0 = XNvm_EfuseReadReg(
				XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_SECURITY_MISC_0_OFFSET);
	if (WriteReq->DecOnly->PrgmDecOnly == TRUE) {
		if ((SecurityMisc0 &
			XNVM_EFUSE_CACHE_DEC_EFUSE_ONLY_MASK) ==
								0x00U) {
			Status = XNvm_EfuseCheckAesKeyCrc(XNVM_EFUSE_AES_CRC_REG_OFFSET,
					XNVM_EFUSE_CTRL_STATUS_AES_CRC_DONE_MASK,
					XNVM_EFUSE_CTRL_STATUS_AES_CRC_PASS_MASK,
					XNVM_EFUSE_CRC_AES_ZEROS);
			if (Status == XST_SUCCESS) {
				if (WriteReq->AesKeys != NULL) {
					if (WriteReq->AesKeys->PrgmAesKey != TRUE) {
						Status =
						(int)XNVM_EFUSE_ERR_DEC_ONLY_KEY_MUST_BE_PRGMD;
						goto END;
					}
				}
				else {
					Status =
					(int)XNVM_EFUSE_ERR_DEC_ONLY_KEY_MUST_BE_PRGMD;
					goto END;
				}
			}
			Status = XNvm_EfuseCheckZeros(
				XNVM_EFUSE_BLACK_OBFUS_IV_START_ROW,
				XNVM_EFUSE_BLACK_OBFUS_IV_START_ROW +
				XNVM_EFUSE_IV_NUM_OF_ROWS);
			if (Status == XST_SUCCESS) {
				if (WriteReq->Ivs != NULL) {
					if (WriteReq->Ivs->PrgmBlkObfusIv != TRUE) {
						Status =
						(int)XNVM_EFUSE_ERR_DEC_ONLY_IV_MUST_BE_PRGMD;
						goto END;
					}
				}
				else {
					Status =
					(int)XNVM_EFUSE_ERR_DEC_ONLY_IV_MUST_BE_PRGMD;
					goto END;
				}
			}
		}
		else {
			Status = (int)XNVM_EFUSE_ERR_DEC_ONLY_ALREADY_PRGMD;
			goto END;
		}
	}
	Status = XST_SUCCESS;
END:
	return Status;
}
/******************************************************************************/
/**
 * @brief	This function is used verify eFUSEs for Zeros.
 *
 * @param	RowStart - Row number from which verification has to be started.
 * @param	RowEnd   - Row number till which verification has to be ended.
 *
 * @return	- XST_SUCCESS - if efuses are not programmed.
 * 		- XST_FAILURE - if efuses are already programmed.
 *
 ******************************************************************************/
static int XNvm_EfuseCheckZeros(u32 RowStart, u32 RowEnd)
{
	volatile int Status = XST_FAILURE;
	u32 Row;
	u32 RowDataVal = 0x0U;

	for (Row = RowStart; Row < RowEnd; Row++) {
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

/******************************************************************************/
/**
 * @brief	This function performs pre checks for programming all the
 *		specified bits.
 *
 * @param	WriteChecks - Pointer to the XNvm_EfuseData which holds eFuse
 * 				data to be programmed.
 *
 * @return	- XST_SUCCESS - if all the conditions for programming is satisfied.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM   - On Invalid Parameter.
 *		- XNVM_EFUSE_ERR_WRITE_AES_KEY   - Error in writing Aes key.
 *		- XNVM_EFUSE_ERR_FUSE_PROTECTED  - Efuse is write protected.
 *		- XNVM_EFUSE_ERR_WRITE_PPK0_HASH - Error in writing ppk0 hash.
 *		- XNVM_EFUSE_ERR_WRITE_PPK1_HASH - Error in writing ppk1 hash.
 *		- XNVM_EFUSE_ERR_WRITE_PPK2_HASH - Error in writing ppk2 hash.
 *		- XNVM_EFUSE_ERR_BIT_CANT_REVERT - Efuse bit can't be reverted.
 *		- XNVM_EFUSE_ERR_WRITE_REVOCATION_IDS - Error in writing
 *							Revocation id efuses.
 *		- XNVM_EFUSE_ERR_AES_SHOULD_BE_PRGMD - Aes key should be programmed
 *						   to program DEC_ONLY.
 *		- XNVM_EFUSE_ERR_BLKOBFUS_IV_SHOULD_BE_PRGMD - Blk Obfuscated IV
 *						   should be programmed
 *						   to program DEC_ONLY.
 *		- XNVM_EFUSE_ERR_WRITE_USER0_KEY - Error in writing User key 0.
 *		- XNVM_EFUSE_ERR_WRITE_USER1_KEY - Error in writing User key 1.
 *
 ******************************************************************************/
static int XNvm_EfuseValidateWriteReq(const XNvm_EfuseData *WriteChecks)
{
	int Status = XST_FAILURE;

	if (WriteChecks == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
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
	if (WriteChecks->BootEnvCtrl != NULL) {
		Status = XNvm_ValidateBootEnvCtrlWriteReq(
				WriteChecks->BootEnvCtrl);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
#ifdef XNVM_EN_ADD_PPKS
	if (WriteChecks->AdditionalPpkHash != NULL) {
		Status = XNvm_EfuseValidateAdditionalPpkWriteReq(
				WriteChecks->AdditionalPpkHash);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
#endif

	Status = XST_SUCCESS;
END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function Programs User eFuses.
 *
 * @param	WriteUserFuses - Pointer to the XNvm_EfuseUserData structure.
 *
 * @return	- XST_SUCCESS - if programming is successful.
 *  		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *
 ******************************************************************************/
static int XNvm_EfusePrgmUserFuses(const XNvm_EfuseUserData *WriteUserFuses)
{
	int Status = XST_FAILURE;
	u32 UserFusesDataToPrgm[XNVM_NUM_OF_USER_FUSES];
	u32 StartRow;
	u32 EndRow;

	if (WriteUserFuses == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
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
 * @brief	This function Validates all user fuses requested for programming.
 *
 * @param	WriteUserFuses - Pointer to XNvm_EfuseUserData structure.
 *
 * @return	- XST_SUCCESS - if programming is successful.
 *		- XNVM_EFUSE_ERR_BIT_CANT_REVERT - Efuse bit can't be reverted.
 *  		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *
 ******************************************************************************/
static int XNvm_EfuseValidateUserFusesWriteReq(
					const XNvm_EfuseUserData *WriteUserFuses)
{
	int Status = XST_FAILURE;
	u32 UserFuseValueRd = 0U;
	u32 StartRow = 0U;
	u32 EndRow = 0U;
	u32 Row = 0U;

	if (WriteUserFuses == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	if ((WriteUserFuses->StartUserFuseNum < XNVM_USER_FUSE_START_NUM) ||
		(WriteUserFuses->StartUserFuseNum > XNVM_USER_FUSE_END_NUM)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	if ((WriteUserFuses->UserFuseData == NULL) ||
		(WriteUserFuses->NumOfUserFuses > XNVM_NUM_OF_USER_FUSES)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	if (((WriteUserFuses->StartUserFuseNum - 1U)  +
		WriteUserFuses->NumOfUserFuses) > XNVM_USER_FUSE_END_NUM) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	StartRow = XNVM_EFUSE_USER_FUSE_START_ROW +
			WriteUserFuses->StartUserFuseNum - 1U;

	EndRow = StartRow + WriteUserFuses->NumOfUserFuses;

	if ((StartRow < XNVM_EFUSE_USER_FUSE_START_ROW) ||
		(EndRow >
		(XNVM_EFUSE_USER_FUSE_START_ROW + XNVM_NUM_OF_USER_FUSES))) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (EndRow > XNVM_EFUSE_PPK_3_HASH_START_ROW) {
		Status = XNvm_EfuseIsAdditionalPpkEn();
		if (Status == XST_SUCCESS) {
			Status = (int)XNVM_EFUSE_ERR_USER_FUSE_PGM_NOT_ALLOWED;
			goto END;
		}
	}

	for (Row = StartRow; Row < EndRow; Row++) {
		Status = XNvm_EfuseReadCache(Row, &UserFuseValueRd);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		if ((UserFuseValueRd &
			WriteUserFuses->UserFuseData[Row - StartRow]) !=
			UserFuseValueRd) {
			Status = (int)XNVM_EFUSE_ERR_BIT_CANT_REVERT;
			goto END;
		}
	}
	Status = XST_SUCCESS;
END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function Validates BootEncCtrl row for programming.
 *
 * @param	BootEnvCtrl - Pointer to BootEnvCtrl structure.
 *
 * @return	- XST_SUCCESS - if programming is successful.
 *		- XNVM_EFUSE_ERR_BOOT_ENV_CTRL_ALREADY_PRGMD - Requested bits
 *							are already programmed.
 *  		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *
 ******************************************************************************/
static int XNvm_ValidateBootEnvCtrlWriteReq(
			const XNvm_EfuseBootEnvCtrlBits *BootEnvCtrl)
{
	int Status = XST_FAILURE;
	u32 Row = XNVM_EFUSE_BOOT_ENV_CTRL_ROW;
	u32 RowDataVal = 0U;

	if (BootEnvCtrl == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if ((BootEnvCtrl->SysmonTempEn != FALSE) ||
		(BootEnvCtrl->SysmonVoltEn != FALSE) ||
		(BootEnvCtrl->SysmonVoltSoc != FALSE) ||
		(BootEnvCtrl->PrgmSysmonTempHot != FALSE) ||
		(BootEnvCtrl->PrgmSysmonVoltPmc != FALSE) ||
		(BootEnvCtrl->PrgmSysmonVoltPslp != FALSE) ||
		(BootEnvCtrl->PrgmSysmonTempCold != FALSE)) {

		Status = XNvm_EfuseReadCache(Row, &RowDataVal);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	else {
		Status = XST_SUCCESS;
		goto END;
	}

	if (BootEnvCtrl->SysmonTempEn == TRUE) {
		if ((RowDataVal &
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_EN_MASK) != 0U) {
			Status = (int)XNVM_EFUSE_ERR_BOOT_ENV_CTRL_ALREADY_PRGMD;
			goto END;
		}
	}

	if (BootEnvCtrl->SysmonVoltEn == TRUE) {
		if ((RowDataVal &
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_EN_MASK) != 0U) {
			Status = (int)XNVM_EFUSE_ERR_BOOT_ENV_CTRL_ALREADY_PRGMD;
			goto END;
		}
	}

	if (BootEnvCtrl->SysmonVoltSoc == TRUE) {
		if ((RowDataVal &
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_SOC_MASK) != 0U) {
			Status = (int)XNVM_EFUSE_ERR_BOOT_ENV_CTRL_ALREADY_PRGMD;
			goto END;
		}
	}

	if (BootEnvCtrl->PrgmSysmonTempHot == TRUE) {
		if ((RowDataVal &
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_HOT_MASK) != 0U) {
			Status = (int)XNVM_EFUSE_ERR_BOOT_ENV_CTRL_ALREADY_PRGMD;
			goto END;
		}
	}

	if (BootEnvCtrl->PrgmSysmonVoltPmc == TRUE) {
		if ((RowDataVal &
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_PMC_MASK) != 0U) {
			Status = (int)XNVM_EFUSE_ERR_BOOT_ENV_CTRL_ALREADY_PRGMD;
			goto END;
		}
	}

	if (BootEnvCtrl->PrgmSysmonVoltPslp == TRUE) {
		if ((RowDataVal &
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_PSLP_MASK) != 0U) {
			Status = (int)XNVM_EFUSE_ERR_BOOT_ENV_CTRL_ALREADY_PRGMD;
			goto END;
		}
	}

	if (BootEnvCtrl->PrgmSysmonTempCold == TRUE) {
		if ((RowDataVal &
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_COLD_MASK) != 0U) {
			Status = (int)XNVM_EFUSE_ERR_BOOT_ENV_CTRL_ALREADY_PRGMD;
			goto END;
		}
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function Programs Protection eFuses.
 *
 * @return	- XST_SUCCESS - if protection efuse programming is successful.
 *		- XNVM_EFUSE_ERR_WRITE_ROW_43_0_PROT - Error in programming
 *							Row 43 protection bit 0.
 *		- XNVM_EFUSE_ERR_WRITE_ROW_43_1_PROT - Error in programming
 *							Row 43 protection bit 1.
 *		- XNVM_EFUSE_ERR_WRITE_ROW_57_0_PROT - Error in programming
 *							Row 57 protection bit 0.
 *		- XNVM_EFUSE_ERR_WRITE_ROW_57_1_PROT - Error in programming
 *							Row 57 protection bit 1.
 *		- XNVM_EFUSE_ERR_WRITE_ROW64_87_0_PROT - Error in programming
 *							 Row 64_87 protection
 *							 bit 0.
 *		- XNVM_EFUSE_ERR_WRITE_ROW64_87_1_PROT - Error in programming
 *							 Row 64_87 protection
 *							 bit 1.
 *		- XNVM_EFUSE_ERR_WRITE_ROW96_99_0_PROT - Error in programming
 *							 Row 96_99 protection
 *							 bit 0.
 *		- XNVM_EFUSE_ERR_WRITE_ROW96_99_1_PROT - Error in programming
 *							 Row 96_99 protection
 *							 bit 1.
 *		- XNVM_EFUSE_ERR_WRITE_ROW_40_PROT - Error in programming Row 40
 *							 protection bit.
 *		- XNVM_EFUSE_ERR_WRITE_ROW_37_PROT - Error in programming Row 37
 *							 protection bit.
 *		- XNVM_EFUSE_ERR_WRITE_ROW_42_PROT - Error in programming row 42
 *							 protection bit.
 *		- XNVM_EFUSE_ERR_WRITE_ROW_58_PROT - Error in programming row 58
 *							     protection bit.
 *
 *******************************************************************************/
static int XNvm_EfusePrgmProtectionEfuse(void)
{
	volatile int Status = XST_FAILURE;
	u32 SecurityCtrlData;
	u32 SecurityMisc0Data;
	u32 SecurityMisc1Data;
	u32 MiscCtrlData;
	u32 BootEnvCtrlRow;
	u32 AnlgTrim2Row;
	u32 AnlgTrim6Row;
	u32 AnlgTrim7Row;
	u32 TrimAms12Row;
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
				XNVM_EFUSE_ERR_WRITE_ROW_43_0_PROT);
			goto END;
		}
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW,
				XNVM_EFUSE_ROW_43_1_PROT_COLUMN);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_ROW_43_1_PROT);
			goto END;
		}
	}
	SecurityMisc0Data = XNvm_EfuseReadReg(
			XNVM_EFUSE_CACHE_BASEADDR,
			XNVM_EFUSE_CACHE_SECURITY_MISC_0_OFFSET);
	if ((SecurityMisc0Data &
		XNVM_EFUSE_CACHE_DEC_EFUSE_ONLY_MASK) !=
			0x00U) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW,
				XNVM_EFUSE_ROW_57_0_PROT_COLUMN);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_ROW_57_0_PROT);
			goto END;
		}
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW,
				XNVM_EFUSE_ROW_57_1_PROT_COLUMN);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_ROW_57_1_PROT);
			goto END;
		}
	}

	Status = XNvm_EfuseCheckZeros(XNVM_EFUSE_PPK_0_HASH_START_ROW,
			(XNVM_EFUSE_PPK_0_HASH_START_ROW +
			 (XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS * 3U)));
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW,
				XNVM_EFUSE_ROW64_87_0_PROT_COLUMN);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_ROW64_87_0_PROT);
			goto END;
		}
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW,
				XNVM_EFUSE_ROW64_87_1_PROT_COLUMN);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_ROW64_87_1_PROT);
			goto END;
		}
	}
	Status = XNvm_EfuseCheckZeros(
			XNVM_EFUSE_META_HEADER_IV_START_ROW,
			(XNVM_EFUSE_META_HEADER_IV_START_ROW +
			 XNVM_EFUSE_IV_NUM_OF_ROWS));
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW,
				XNVM_EFUSE_ROW96_99_0_PROT_COLUMN);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_ROW96_99_0_PROT);
			goto END;
		}
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW,
				XNVM_EFUSE_ROW96_99_1_PROT_COLUMN);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_ROW96_99_1_PROT);
			goto END;
		}
	}

	BootEnvCtrlRow = XNvm_EfuseReadReg(
			XNVM_EFUSE_CACHE_BASEADDR,
			XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_OFFSET);
	AnlgTrim2Row = XNvm_EfuseReadReg(
			XNVM_EFUSE_CACHE_BASEADDR,
			XNVM_EFUSE_CACHE_ANLG_TRIM_2_OFFSET);
	AnlgTrim6Row = XNvm_EfuseReadReg(
			XNVM_EFUSE_CACHE_BASEADDR,
			XNVM_EFUSE_CACHE_ANLG_TRIM_6_OFFSET);
	AnlgTrim7Row = XNvm_EfuseReadReg(
			XNVM_EFUSE_CACHE_BASEADDR,
			XNVM_EFUSE_CACHE_ANLG_TRIM_7_OFFSET);
	TrimAms12Row = XNvm_EfuseReadReg(
			XNVM_EFUSE_CACHE_BASEADDR,
			XNVM_EFUSE_CACHE_TRIM_AMS_12_OFFSET);
	if ((BootEnvCtrlRow != 0x00U) &&
		(AnlgTrim2Row != 0x00U) &&
		(AnlgTrim6Row != 0x00U) &&
		(AnlgTrim7Row != 0x00U) &&
		(TrimAms12Row != 0x00U)) {

		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW,
				XNVM_EFUSE_ROW_37_PROT_COLUMN);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_ROW_37_PROT);
			goto END;
		}
	}
	MiscCtrlData = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
			XNVM_EFUSE_CACHE_MISC_CTRL_OFFSET);
	if ((MiscCtrlData &
		(XNVM_EFUSE_CACHE_MISC_CTRL_PPK0_INVLD_1_0_MASK |
		XNVM_EFUSE_CACHE_MISC_CTRL_PPK1_INVLD_1_0_MASK |
		XNVM_EFUSE_CACHE_MISC_CTRL_PPK2_INVLD_1_0_MASK |
#ifdef XNVM_EN_ADD_PPKS
		XNVM_EFUSE_CACHE_MISC_CTRL_PPK3_INVLD_1_0_MASK |
		XNVM_EFUSE_CACHE_MISC_CTRL_PPK4_INVLD_1_0_MASK |
#endif
		XNVM_EFUSE_CACHE_MISC_CTRL_CRYPTO_KAT_EN_MASK |
		XNVM_EFUSE_CACHE_MISC_CTRL_GD_ROM_MONITOR_EN_MASK)) != 0x00U)
	{
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW,
				XNVM_EFUSE_ROW_40_PROT_COLUMN);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_ROW_40_PROT);
			goto END;
		}
	}

	PufChashData = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
					XNVM_EFUSE_CACHE_PUF_CHASH_OFFSET);
	if (PufChashData != 0x00U) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW,
				XNVM_EFUSE_ROW_42_PROT_COLUMN);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_ROW_42_PROT);
			goto END;
		}
	}

	SecurityMisc1Data = XNvm_EfuseReadReg(
			XNVM_EFUSE_CACHE_BASEADDR,
			XNVM_EFUSE_CACHE_SECURITY_MISC_1_OFFSET);
	if ((SecurityMisc1Data &
		XNVM_EFUSE_SECURITY_MISC_1_PROT_MASK) != 0x00U) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW,
				XNVM_EFUSE_ROW_58_PROT_COLUMN);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_ROW_58_PROT);
			goto END;
		}
	}
	Status = XNvm_EfuseCacheLoadAndProtectionChecks();
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
static int XNvm_EfusePgmAndVerifyRows(u32 StartRow, u32 RowCount,
			XNvm_EfuseType EfuseType, const u32* RowData)
{
	volatile int Status = XST_FAILURE;
	u32 Data;
	u32 Row = StartRow;
	volatile u32 Count = 0U;
	const u32* DataPtr = RowData;
	u32 Idx;

	if ((EfuseType != XNVM_EFUSE_PAGE_0) &&
		(EfuseType != XNVM_EFUSE_PAGE_1) &&
		(EfuseType != XNVM_EFUSE_PAGE_2)){

		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	if ((DataPtr == NULL) || (RowCount == 0U)) {

		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	while (Count < RowCount) {

		Data = DataPtr[Count];
		Idx = 0U;
		while ((Data != 0U) && (Idx < XNVM_EFUSE_MAX_BITS_IN_ROW)) {
			if ((Data & 0x01U) == 0x01U) {
				Status = XNvm_EfusePgmAndVerifyBit(EfuseType,
								Row, Idx);
				if (Status != XST_SUCCESS) {
					goto END;
				}
			}
			Idx++;
			Data = Data >> 1U;
		}

		Count++;
		Row++;
	}

	if (Count != RowCount) {
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
 * @brief	This function reads 32-bit data from cache specified by Row.
 *
 * @param	Row 	- Starting Row number (0-based addressing).
 * @param	RowData	- Pointer to memory location where read 32-bit row data
 *					  is to be stored.
 *
 * @return	- XST_SUCCESS - Specified data read.
 *		- XNVM_EFUSE_ERR_CACHE_PARITY - Parity Error exist in cache.
 *
 ******************************************************************************/
static int XNvm_EfuseReadCache(u32 Row, u32* RowData)
{
	int Status = XST_FAILURE;
	u32 CacheData;
	u32 IsrStatus;

	if (Row > (XNVM_EFUSE_TOTAL_NUM_OF_ROWS - 1U)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	if (RowData == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	CacheData = Xil_In32(XNVM_EFUSE_CACHE_BASEADDR + (Row * sizeof(u32)));
	IsrStatus = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_ISR_REG_OFFSET);
	if ((IsrStatus & XNVM_EFUSE_ISR_CACHE_ERROR)
			== XNVM_EFUSE_ISR_CACHE_ERROR) {
		Status = (int)XNVM_EFUSE_ERR_CACHE_PARITY;
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
 * @param	StartRow - Starting Row number (0-based addressing).
 * @param	RowCount - Number of Rows to be read.
 * @param	RowData  - Pointer to memory location where read 32-bit row data(s)
 *					   is to be stored.
 *
 * @return	- XST_SUCCESS	- Specified data read.
 *		- XNVM_EFUSE_ERR_CACHE_PARITY - Parity Error exist in cache.
 *
 ******************************************************************************/
static int XNvm_EfuseReadCacheRange(u32 StartRow, u8 RowCount, u32* RowData)
{
	volatile int Status = XST_FAILURE;
	u32 Row = StartRow;
	u32 Count;
	u32* Data = RowData;

	for (Count = 0; Count < RowCount; Count++) {
		Status = XST_FAILURE;
		Status = XNvm_EfuseReadCache(Row, Data);
		if (Status != XST_SUCCESS) {
			break;
		}
		Row++;
		Data++;
	}

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
	u32 CacheOffset = 0U;
	u32 BitVal = 0U;

	if (((Page == XNVM_EFUSE_PAGE_0) && ((Row < XNVM_EFUSE_AES_KEY_START_ROW) ||
		(Row > XNVM_EFUSE_USER_KEY_1_END_ROW))) || (Page == XNVM_EFUSE_PAGE_1) ||
		(Page == XNVM_EFUSE_PAGE_2)) {
		CacheOffset = ((u32)Page * XNVM_NUM_OF_CACHE_ADDR_PER_PAGE) +
			(Row * XNVM_EFUSE_WORD_LEN);
		BitVal = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR, CacheOffset) &
			(1U << Col);
		if (BitVal != 0x0U) {
			Status = XST_SUCCESS;
			goto END;
		}
	}

	PgmAddr = ((u32)Page << XNVM_EFUSE_ADDR_PAGE_SHIFT) |
		(Row << XNVM_EFUSE_ADDR_ROW_SHIFT) |
		(Col << XNVM_EFUSE_ADDR_COLUMN_SHIFT);

	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
		XNVM_EFUSE_PGM_ADDR_REG_OFFSET, PgmAddr);

	Status = (int)Xil_WaitForEvents((UINTPTR)(XNVM_EFUSE_CTRL_BASEADDR +
		XNVM_EFUSE_ISR_REG_OFFSET),
		(XNVM_EFUSE_ISR_PGM_DONE |
		XNVM_EFUSE_ISR_PGM_ERROR),
		(XNVM_EFUSE_ISR_PGM_DONE |
		XNVM_EFUSE_ISR_PGM_ERROR),
		XNVM_EFUSE_PGM_TIMEOUT_VAL,
		&EventMask);

	if (XST_TIMEOUT == Status) {
		Status = (int)XNVM_EFUSE_ERR_PGM_TIMEOUT;
	} else if ((EventMask & XNVM_EFUSE_ISR_PGM_ERROR)
					== XNVM_EFUSE_ISR_PGM_ERROR) {
		Status = (int)XNVM_EFUSE_ERR_PGM;
	} else {
		Status = XST_SUCCESS;
	}

	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
			XNVM_EFUSE_ISR_REG_OFFSET,
			(XNVM_EFUSE_ISR_PGM_DONE |
			XNVM_EFUSE_ISR_PGM_ERROR));

END:
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
 *		- XNVM_EFUSE_ERR_PGM_TIMEOUT - If Programming timeout has occurred.
 *		- XST_FAILURE                - Unexpected error.
 *
 ******************************************************************************/
static int XNvm_EfuseVerifyBit(XNvm_EfuseType Page, u32 Row, u32 Col)
{
	int Status = XST_FAILURE;
	u32 RdAddr;
	volatile u32 RegData = 0x00U;
	u32 EventMask = 0x00U;

	/*
	 * If Row Belongs to AES key or User key 0 or
	 * User key 1 can't verify the bit
	 * as those can be checked only CRC
	 */
	if (((Row >= XNVM_EFUSE_AES_KEY_START_ROW) &&
		(Row <= XNVM_EFUSE_USER_KEY_1_END_ROW))) {
		Status = XST_SUCCESS;
		goto END;
	}

	RdAddr = ((u32)Page << XNVM_EFUSE_ADDR_PAGE_SHIFT) |
		(Row << XNVM_EFUSE_ADDR_ROW_SHIFT);

	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
		XNVM_EFUSE_RD_ADDR_REG_OFFSET, RdAddr);

	Status = (int)Xil_WaitForEvents((UINTPTR)(XNVM_EFUSE_CTRL_BASEADDR +
		XNVM_EFUSE_ISR_REG_OFFSET),
		XNVM_EFUSE_ISR_RD_DONE,
		XNVM_EFUSE_ISR_RD_DONE,
		XNVM_EFUSE_RD_TIMEOUT_VAL,
		&EventMask);

	if (XST_TIMEOUT == Status) {
		Status = (int)XNVM_EFUSE_ERR_RD_TIMEOUT;
	} else if ((EventMask & XNVM_EFUSE_ISR_RD_DONE)
					== XNVM_EFUSE_ISR_RD_DONE) {
		RegData = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_RD_DATA_REG_OFFSET);
		if ((RegData & (((u32)0x01U) << Col)) != 0U) {
			Status = XST_SUCCESS;
		}
		else {
			Status = XST_FAILURE;
		}
	} else {
		Status = (int)XNVM_EFUSE_ERR_PGM_VERIFY;
	}
END:
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
 *
 * @return	- XST_SUCCESS - Specified bit set in eFUSE.
 *		- XNVM_EFUSE_ERR_PGM_TIMEOUT - eFUSE programming timed out.
 *		- XNVM_EFUSE_ERR_PGM 	- eFUSE programming failed.
 *		- XNVM_EFUSE_ERR_PGM_VERIFY  - Verification failed, specified bit
 *						is not set.
 *		- XST_FAILURE 	- Unexpected error.
 *
 ******************************************************************************/
static int XNvm_EfusePgmAndVerifyBit(XNvm_EfuseType Page, u32 Row, u32 Col)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_EfusePgmBit(Page, Row, Col);
	if(XST_SUCCESS == Status) {
		Status = XST_FAILURE;
		Status = XNvm_EfuseVerifyBit(Page, Row, Col);
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function performs the Protection checks when the eFuse
 * 		cache is reloaded
 *
 * @return	- XST_SUCCESS - on successful protection checks.
 *		- XNVM_EFUSE_ERR_IN_PROTECTION_CHECK - Error in protection check
 *		- XNVM_EFUSE_ERR_ANCHOR_BIT_PATTERN - Error in Anchor bits
 *							pattern
 *
 ******************************************************************************/
static int XNvm_EfuseProtectionChecks(void)
{
	int Status = XST_FAILURE;
	volatile u32 RegVal = 0U;
	volatile u32 RegValTmp = 0U;
	volatile u32 ProtVal;
	volatile u32 ProtValTmp;
	volatile u32 RowVal;
	volatile u32 RowValTmp;
	u32 MiscCtrlProtMask;
	u32 Index;

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

	ProtVal = RegVal & (XNVM_EFUSE_CACHE_TBITS0_SVD_ROW_43_PROT_MASK);
	ProtValTmp = RegValTmp & (XNVM_EFUSE_CACHE_TBITS0_SVD_ROW_43_PROT_MASK);

	if((ProtVal != 0x0U) || (ProtValTmp != 0x0U)) {
		RowVal = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_SECURITY_CONTROL_OFFSET);
		RowValTmp = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_SECURITY_CONTROL_OFFSET);

		if ((RowVal != RowValTmp) || (RowVal == 0x0U)) {
			Status = (int)XNVM_EFUSE_ERR_IN_PROTECTION_CHECK;
			goto END;
		}
	}

	ProtVal = RegVal & (XNVM_EFUSE_CACHE_TBITS0_SVD_ROW_57_PROT_MASK);
	ProtValTmp = RegValTmp & (XNVM_EFUSE_CACHE_TBITS0_SVD_ROW_57_PROT_MASK);

	if((ProtVal != 0x0U) || (ProtValTmp != 0x0U)) {
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

	ProtVal = RegVal & (XNVM_EFUSE_CACHE_TBITS0_SVD_ROW_64_87_PROT_MASK);
	ProtValTmp = RegValTmp &
			(XNVM_EFUSE_CACHE_TBITS0_SVD_ROW_64_87_PROT_MASK);

	if((ProtVal != 0x0U) || (ProtValTmp != 0x0U)) {
		RowVal = 0x0U;
		RowValTmp = 0x0U;

		for (Index = 0U; Index < XNVM_EFUSE_TOTAL_PPK_HASH_ROWS;
			Index++) {
			RowVal = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_PPK0_HASH_0_OFFSET +
				(Index * XNVM_EFUSE_WORD_LEN));
			RowValTmp = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
					XNVM_EFUSE_CACHE_PPK0_HASH_0_OFFSET +
					(Index * XNVM_EFUSE_WORD_LEN));
			if ((RowVal != 0x0U) || (RowValTmp != 0x0U)) {
				break;
			}
		}
		if ((RowVal != RowValTmp) || (RowVal == 0x0U)) {
			Status = (int)XNVM_EFUSE_ERR_IN_PROTECTION_CHECK;
			goto END;
		}
	}

	ProtVal = RegVal  & (XNVM_EFUSE_CACHE_TBITS0_SVD_ROW_96_99_PROT_MASK);
	ProtValTmp = RegValTmp & (XNVM_EFUSE_CACHE_TBITS0_SVD_ROW_96_99_PROT_MASK);

	if((ProtVal != 0x0U) || (ProtValTmp != 0x0U)) {
		RowVal = 0x0U;
		RowValTmp = 0x0U;

		for (Index = 0U; Index < XNVM_EFUSE_IV_NUM_OF_ROWS; Index++) {
			RowVal = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_METAHEADER_IV_RANGE_0_OFFSET +
				(Index * XNVM_EFUSE_WORD_LEN));
			RowValTmp = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_METAHEADER_IV_RANGE_0_OFFSET +
				(Index * XNVM_EFUSE_WORD_LEN));

			if ((RowVal != 0x0U) || (RowValTmp != 0x0U)) {
				break;
			}
		}
		if ((RowVal != RowValTmp) || (RowVal == 0x0U)) {
			Status = (int)XNVM_EFUSE_ERR_IN_PROTECTION_CHECK;
			goto END;
		}
	}

	ProtVal = RegVal & XNVM_EFUSE_CACHE_TBITS0_SVD_ROW_37_PROT_MASK;
	ProtValTmp = RegValTmp & XNVM_EFUSE_CACHE_TBITS0_SVD_ROW_37_PROT_MASK;

	if((ProtVal != 0x0U) || (ProtValTmp != 0x0U)) {
		RowVal = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_OFFSET);
		RowValTmp = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_OFFSET);

		if ((RowVal != RowValTmp) || (RowVal == 0x0U)) {
			Status = (int)XNVM_EFUSE_ERR_IN_PROTECTION_CHECK;
			goto END;
		}

		RowVal = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_ANLG_TRIM_2_OFFSET);
		RowValTmp = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_ANLG_TRIM_2_OFFSET);

		if ((RowVal != RowValTmp) || (RowVal == 0x0U)) {
			Status = (int)XNVM_EFUSE_ERR_IN_PROTECTION_CHECK;
			goto END;
		}

		RowVal = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_ANLG_TRIM_6_OFFSET);
		RowValTmp = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_ANLG_TRIM_6_OFFSET);

		if ((RowVal != RowValTmp) || (RowVal == 0x0U)) {
			Status = (int)XNVM_EFUSE_ERR_IN_PROTECTION_CHECK;
			goto END;
		}

		RowVal = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_ANLG_TRIM_7_OFFSET);
		RowValTmp = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_ANLG_TRIM_7_OFFSET);

		if ((RowVal != RowValTmp) || (RowVal == 0x0U)) {
			Status = (int)XNVM_EFUSE_ERR_IN_PROTECTION_CHECK;
			goto END;
		}

		RowVal = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_TRIM_AMS_12_OFFSET);
		RowValTmp = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_TRIM_AMS_12_OFFSET);

		if ((RowVal != RowValTmp) || (RowVal == 0x0U)) {
			Status = (int)XNVM_EFUSE_ERR_IN_PROTECTION_CHECK;
			goto END;
		}
	}

	ProtVal = RegVal & XNVM_EFUSE_CACHE_TBITS0_SVD_ROW_40_PROT_MASK;
	ProtValTmp = RegValTmp & XNVM_EFUSE_CACHE_TBITS0_SVD_ROW_40_PROT_MASK;

	MiscCtrlProtMask = (XNVM_EFUSE_CACHE_MISC_CTRL_PPK0_INVLD_1_0_MASK |
			XNVM_EFUSE_CACHE_MISC_CTRL_PPK1_INVLD_1_0_MASK |
			XNVM_EFUSE_CACHE_MISC_CTRL_PPK2_INVLD_1_0_MASK |
			XNVM_EFUSE_CACHE_MISC_CTRL_CRYPTO_KAT_EN_MASK |
			XNVM_EFUSE_CACHE_MISC_CTRL_GD_ROM_MONITOR_EN_MASK);
#ifdef XNVM_EN_ADD_PPKS
	MiscCtrlProtMask |= (XNVM_EFUSE_CACHE_MISC_CTRL_PPK3_INVLD_1_0_MASK |
			XNVM_EFUSE_CACHE_MISC_CTRL_PPK4_INVLD_1_0_MASK);
#endif

	if((ProtVal != 0x0U) || (ProtValTmp != 0x0U)) {
		RowVal = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_MISC_CTRL_OFFSET);
		RowValTmp = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_MISC_CTRL_OFFSET);

		if ((RowVal != RowValTmp) ||
			((RowVal & MiscCtrlProtMask) == 0x0U)) {
			Status = (int)XNVM_EFUSE_ERR_IN_PROTECTION_CHECK;
			goto END;
		}
	}

	ProtVal = RegVal & XNVM_EFUSE_CACHE_TBITS0_SVD_ROW_42_PROT_MASK;
	ProtValTmp = RegValTmp & XNVM_EFUSE_CACHE_TBITS0_SVD_ROW_42_PROT_MASK;

	if((ProtVal != 0x0U) || (ProtValTmp != 0x0U)) {
		RowVal = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_PUF_CHASH_OFFSET);
		RowValTmp = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_PUF_CHASH_OFFSET);

		if ((RowVal != RowValTmp) || (RowVal == 0x0U)) {
			Status = (int)XNVM_EFUSE_ERR_IN_PROTECTION_CHECK;
			goto END;
		}
	}

	ProtVal = RegVal & XNVM_EFUSE_CACHE_TBITS0_SVD_ROW_58_PROT_MASK;
	ProtValTmp = RegValTmp  & XNVM_EFUSE_CACHE_TBITS0_SVD_ROW_58_PROT_MASK;

	if((ProtVal != 0x0U) || (ProtValTmp != 0x0U)) {
		RowVal = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_SECURITY_MISC_1_OFFSET);
		RowValTmp = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_SECURITY_MISC_1_OFFSET);

		if ((RowVal != RowValTmp) ||
			((RowVal & XNVM_EFUSE_SECURITY_MISC_1_PROT_MASK) ==
									0x0U)) {
			Status = (int)XNVM_EFUSE_ERR_IN_PROTECTION_CHECK;
			goto END;
		}
	}

	Status = XST_SUCCESS;
END:
	return Status;

}

/******************************************************************************/
/**
 * @brief	This function reloads the cache of eFUSE so that can be directly
 * 			read from cache.
 *
 * @return	- XST_SUCCESS - on successful cache reload.
 *		- XNVM_EFUSE_ERR_CACHE_LOAD - Error while loading the cache.
 *
 * @note	Not recommended to call this API frequently,if this API is called
 *		all the cache memory is reloaded by reading eFUSE array,
 *		reading eFUSE bit multiple times may diminish the life time.
 *
 ******************************************************************************/
static int XNvm_EfuseCacheLoadAndProtectionChecks(void)
{
	int Status = XST_FAILURE;

	Status = XNvm_EfuseCacheReload();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseProtectionChecks();

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function is used to read the miscellaneous eFUSE control
 *		Enable bits from cache.
 *
 * @return	- XST_SUCCESS - On Successful read.
 *          - XST_FAILURE - on Read failure
 *		    - XNVM_EFUSE_5_PPKS_FEATURE_NOT_SUPPORTED - on enable bits are not set.
 *
 ******************************************************************************/
static int XNvm_EfuseIsAdditionalPpkEn(void)
{
	int Status = XST_FAILURE;
	volatile u32 ReadReg;
	volatile u32 ReadRegTmp;

	/* Read PPK3 and PPK4 enable set bits */
	Status = XNvm_EfuseReadCache(XNVM_EFUSE_MISC_CTRL_ROW, (u32*)&ReadReg);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseReadCache(XNVM_EFUSE_MISC_CTRL_ROW, (u32*)&ReadRegTmp);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	ReadReg = ReadReg & XNVM_EFUSE_PPK_3_PPK_4_ENABLE;
	ReadRegTmp = ReadRegTmp & XNVM_EFUSE_PPK_3_PPK_4_ENABLE;

	if ((ReadReg == 0x0U) || (ReadRegTmp == 0x0U)) {
		Status = (int)XNVM_EFUSE_5_PPKS_FEATURE_NOT_SUPPORTED;
		goto END;
	}
END:
	return Status;
}

#ifdef XNVM_ACCESS_PUF_USER_DATA

/******************************************************************************/
/**
 * @brief	This function Programs PUF eFuses.
 *
 * @param	WritePufFuses - Pointer to the XNvm_EfusePufFuse structure.
 *
 * @return	- XST_SUCCESS - if programming is successful.
 *  		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 * 			- XNVM_ERR_WRITE_PUF_USER_DATA -  On Invalid user data
 *
 * @note By default PUF eFuses are used for PUF helper data
 *       To program PUF eFuses as general purpose efuses user needs to enable
 *       compile time macro
 *       XNVM_ACCESS_PUF_USER_DATA -  For BareMetal support
 *
 ******************************************************************************/
static int XNvm_EfusePrgmPufFuses(const XNvm_EfusePufFuse *WritePufFuses)
{
	int Status = XST_FAILURE;
	u32 PufFusesDataToPrgm[XNVM_EFUSE_PUF_SYN_DATA_NUM_OF_ROWS] = {0U};
	u32 PufCacheValue;
	u32 Row = 0U;
	u32 StartRow;
	u32 EndRow;

	if (WritePufFuses == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	StartRow = XNVM_EFUSE_PUF_SYN_CACHE_READ_ROW +
			WritePufFuses->StartPufFuseRow - 1;
	EndRow = StartRow + WritePufFuses->NumOfPufFusesRows;

	/* Validate PufFuses before programming */
	for (Row = StartRow; Row < EndRow; Row++) {
		Status = XNvm_EfuseReadCache(Row, &PufCacheValue);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		if ((PufCacheValue &
			WritePufFuses->PufFuseData[Row - StartRow]) !=
			PufCacheValue) {
			Status = (int)XNVM_EFUSE_ERR_BIT_CANT_REVERT;
			goto END;
		}
		if ((Row - XNVM_EFUSE_PUF_SYN_CACHE_READ_ROW) ==
			XNVM_EFUSE_PUF_SYN_DATA_NUM_OF_ROWS - 1U) {
			if ((WritePufFuses->PufFuseData[Row - StartRow] &
				XNVM_PUF_ROW_UPPER_NIBBLE_MASK) != 0x00U) {
				Status = (int)XNVM_ERR_WRITE_PUF_USER_DATA;
				goto END;
			}
		}
	}

	/* Program Puf Fuses */
	Status = XNvm_EfuseComputeProgrammableBits(WritePufFuses->PufFuseData,
						PufFusesDataToPrgm,
						StartRow,
						EndRow);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	StartRow = XNVM_EFUSE_PUF_SYN_START_ROW +
				WritePufFuses->StartPufFuseRow - 1;
	Status = XNvm_EfusePgmAndVerifyRows(StartRow,
		WritePufFuses->NumOfPufFusesRows,
		XNVM_EFUSE_PAGE_2, PufFusesDataToPrgm);
END:
	return Status;

}

/******************************************************************************/
/**
 * @brief	This function programs PUF HD eFuses as general purpose eFuses.
 *
 * @param	PufFuse - Pointer to the XNvm_EfusePufFuse structure
 *
 * @return	- XST_SUCCESS - if programs successfully.
 *          - XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *			- XNVM_EFUSE_ERR_PUF_SYN_ALREADY_PRGMD - Puf Syn data already
 *							   programmed.
 *			- XNVM_EFUSE_ERR_PUF_CHASH_ALREADY_PRGMD - Puf chash already
 *							   programmed.
 *			- XNVM_EFUSE_ERR_PUF_AUX_ALREADY_PRGMD	 - Puf Aux is already
 *							   programmed.
 *          - XNVM_EFUSE_ERR_DEC_ONLY_ALREADY_PRGMD  - Dec only bits are
 *                             already programmed.
 *          - XNVM_EFUSE_ERR_WRITE_PUF_FUSES - Error in writing PUF user fuses
 *
 * @note By default PUF eFuses are used for PUF helper data
 *       To program PUF eFuses as general purpose efuses user needs to enable
 *       compile time macro
 *       XNVM_ACCESS_PUF_USER_DATA -  For BareMetal support
 *
 ******************************************************************************/
int XNvm_EfuseWritePufAsUserFuses(XNvm_EfusePufFuse *PufFuse)
{
	int Status = XST_FAILURE;
	int LockStatus = XST_FAILURE;
	int DisableStatus = XST_FAILURE;
	int ResetStatus = XST_FAILURE;
	u32 PufSecurityCtrlReg = 0U;
	u32 RowDataVal = 0U;
	u32 IsDecOnly = 0U;

    /**
	 *  validate input parameters. Return XNVM_EFUSE_ERR_INVALID_PARAM if input parameters are invalid
	 */
	if (PufFuse == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if ((PufFuse->StartPufFuseRow < 1U) ||
		(PufFuse->StartPufFuseRow  > XNVM_EFUSE_PUF_SYN_DATA_NUM_OF_ROWS)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if ((PufFuse->NumOfPufFusesRows < 1U) ||
		(PufFuse->NumOfPufFusesRows > XNVM_EFUSE_PUF_SYN_DATA_NUM_OF_ROWS)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if ((PufFuse->StartPufFuseRow +
		(PufFuse->NumOfPufFusesRows - 1U)) > XNVM_EFUSE_PUF_SYN_DATA_NUM_OF_ROWS) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (PufFuse->PufFuseData == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfuseReadCache(XNVM_EFUSE_SECURITY_CONTROL_ROW,
					&PufSecurityCtrlReg);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Check for PUF syndrome lock status */
	if ((PufSecurityCtrlReg &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_SYN_LK_MASK) != 0x0U) {
		Status = XNVM_EFUSE_ERR_FUSE_PROTECTED |
					XNVM_EFUSE_ERR_PUF_SYN_ALREADY_PRGMD;
		goto END;
	}

	/* Check for non-zero PUF Chash and AUX */
	Status = XNvm_EfuseCheckZeros(XNVM_EFUSE_PUF_CHASH_ROW,
					(XNVM_EFUSE_PUF_CHASH_ROW +
					XNVM_EFUSE_PUF_CHASH_NUM_OF_ROWS));
	if (Status != XST_SUCCESS) {
		Status = (int)XNVM_EFUSE_ERR_PUF_CHASH_ALREADY_PRGMD;
		goto END;
	}

	Status = XNvm_EfuseReadCache(XNVM_EFUSE_PUF_AUX_ROW, &RowDataVal);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((RowDataVal &
		XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_ECC_23_0_MASK) != 0x00U) {
		Status = (int)XNVM_EFUSE_ERR_PUF_AUX_ALREADY_PRGMD;
		goto END;
	}

	/**
     *  Check for DEC only bits
	 */
	IsDecOnly = XNvm_EfuseReadReg(
				XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_SECURITY_MISC_0_OFFSET);
	if ((IsDecOnly &
			XNVM_EFUSE_CACHE_DEC_EFUSE_ONLY_MASK) !=
								0x00U) {
		Status = (int)XNVM_EFUSE_ERR_DEC_ONLY_ALREADY_PRGMD;
		goto END;
	}

    /**
	 *  Monitor voltage and temperature if EnvMonitorDis is TRUE
	 */
	if (PufFuse->EnvMonitorDis != TRUE) {
		if (PufFuse->SysMonInstPtr == NULL) {
			Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
			goto END;
		}
		Status = XNvm_EfuseTempAndVoltChecks(
					PufFuse->SysMonInstPtr);
		if (Status != XST_SUCCESS) {
			Status = Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING;
			goto END;
		}
	}

    /**
	 * 	@{ Unlock the eFUSE controller. On failure return appropriate failure code.
     *	   Set reference clock.
     *	   Set read mode
	 */
	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
					XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
     *  Program PUF Fuses if it set by user
	 */
	if (PufFuse->PrgmPufFuse == TRUE) {
		Status = XNvm_EfusePrgmPufFuses(PufFuse);
		if (Status != XST_SUCCESS) {
			Status = Status | XNVM_EFUSE_ERR_WRITE_PUF_FUSES;
			goto END;
		}
	}

    /**
	 *  Reload the cache once programming of all eFuses requested are programmed
	 */
	Status = XNvm_EfuseCacheLoadAndProtectionChecks();

END:
    /**
	 *  Reset Read mode
	 */
	ResetStatus = XNvm_EfuseResetReadMode();
	if (XST_SUCCESS == Status) {
		Status = ResetStatus;
	}
	/**
	 *  disable eFuse Programming mode
	 */
	DisableStatus = XNvm_EfuseDisableProgramming();
	if (XST_SUCCESS == Status) {
		Status = DisableStatus;
	}

	/**
	 *  Lock eFuse controller
	 */
	LockStatus = XNvm_EfuseLockController();
	if (XST_SUCCESS == Status) {
		Status = LockStatus;
	}
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function reads the PUF HD eFuses as general purpose eFuses.
 *
 * @param	PufFuse - Pointer to the XNvm_EfusePufFuse structure
 *
 * @return	- XST_SUCCESS - if programs successfully.
 *          - XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *			- XNVM_EFUSE_ERR_PUF_SYN_ALREADY_PRGMD	 - Puf Syn data already
 *							   programmed.
 *			- XNVM_EFUSE_ERR_PUF_CHASH_ALREADY_PRGMD - Puf chash already
 *							   programmed.
 *			- XNVM_EFUSE_ERR_PUF_AUX_ALREADY_PRGMD	 - Puf Aux is already
 *							   programmed.
 *          - XNVM_EFUSE_ERR_DEC_ONLY_ALREADY_PRGMD  - Dec only bits are
 *                             already programmed.
 *          - XNVM_EFUSE_ERR_READ_PUF_FUSES - Error in reading PUF user fuses

 *
 * @note By default PUF eFuses are used for PUF helper data
 *       To program PUF eFuses as general purpose efuses user needs to enable
 *       compile time macro
 *       XNVM_ACCESS_PUF_USER_DATA -  For BareMetal support
 *
 ******************************************************************************/
int XNvm_EfuseReadPufAsUserFuses(XNvm_EfusePufFuse *PufFuse)
{
	int Status = XST_FAILURE;
	u32 Row = 0U;
	u32 PufSecurityCtrlReg = 0U;
	u32 RowDataVal = 0U;
	u32 IsDecOnly = 0U;

    /**
	 *  validate input parameters. Return XNVM_EFUSE_ERR_INVALID_PARAM if input parameters are invalid
	 */
	if (PufFuse == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

    /**
	 *  Check the PufSynLk bit,Chash,Aux and Deconly eFuses if they are blown. If not programmed proceed else return error
	 */
	if ((PufFuse->StartPufFuseRow < 1U) ||
		(PufFuse->StartPufFuseRow  > XNVM_EFUSE_PUF_SYN_DATA_NUM_OF_ROWS)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if ((PufFuse->NumOfPufFusesRows < 1U) ||
		(PufFuse->NumOfPufFusesRows > XNVM_EFUSE_PUF_SYN_DATA_NUM_OF_ROWS)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if ((PufFuse->StartPufFuseRow +
		(PufFuse->NumOfPufFusesRows - 1U)) > XNVM_EFUSE_PUF_SYN_DATA_NUM_OF_ROWS) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (PufFuse->PufFuseData == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}


	/* Check for PUF syndrome lock status */
	Status = XNvm_EfuseReadCache(XNVM_EFUSE_SECURITY_CONTROL_ROW,
					&PufSecurityCtrlReg);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((PufSecurityCtrlReg &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_SYN_LK_MASK) != 0x0U) {
		Status = XNVM_EFUSE_ERR_FUSE_PROTECTED |
					XNVM_EFUSE_ERR_PUF_SYN_ALREADY_PRGMD;
		goto END;
	}

	/* Check for non-zero PUF Chash and AUX */
	Status = XNvm_EfuseCheckZeros(XNVM_EFUSE_PUF_CHASH_ROW,
					(XNVM_EFUSE_PUF_CHASH_ROW +
					XNVM_EFUSE_PUF_CHASH_NUM_OF_ROWS));
	if (Status != XST_SUCCESS) {
		Status = (int)XNVM_EFUSE_ERR_PUF_CHASH_ALREADY_PRGMD;
		goto END;
	}

	Status = XNvm_EfuseReadCache(XNVM_EFUSE_PUF_AUX_ROW, &RowDataVal);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((RowDataVal &
		XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_ECC_23_0_MASK) != 0x00U) {
		Status = (int)XNVM_EFUSE_ERR_PUF_AUX_ALREADY_PRGMD;
		goto END;
	}

	/* Check for DEC only bits */
	IsDecOnly = XNvm_EfuseReadReg(
				XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_SECURITY_MISC_0_OFFSET);
	if ((IsDecOnly &
			XNVM_EFUSE_CACHE_DEC_EFUSE_ONLY_MASK) !=
								0x00U) {
		Status = (int)XNVM_EFUSE_ERR_DEC_ONLY_ALREADY_PRGMD;
		goto END;
	}

	Row = XNVM_EFUSE_PUF_SYN_CACHE_READ_ROW +
		PufFuse->StartPufFuseRow - 1U;

    /**
	 *  @{ Read Puf data from efuse cache by calculating offset range from start row to end row based on user inputs.
     *     Return XST_SUCCESS when read is successful or error code on failure
	 */
	Status = XNvm_EfuseReadCacheRange(Row,
					(u8)PufFuse->NumOfPufFusesRows,
					PufFuse->PufFuseData);
	if (Status != XST_SUCCESS) {
		Status = Status | XNVM_EFUSE_ERR_RD_PUF_FUSES;
	}
END:
	return Status;
}

#endif
#ifdef XNVM_EN_ADD_PPKS
/******************************************************************************/
/**
 * @brief	This function is used to program below eFuses -
 * 			PPK3/PPK4 hash.
 *
 * @param	Hash - Pointer to the XNvm_EfuseAdditionalPpkHash structure which holds
 * 			ppk hash data to be programmed to eFuse.
 *
 * @return	- XST_SUCCESS - Specified data read.
 *		- XNVM_EFUSE_ERR_WRITE_PPK3_HASH - Error while writing PPK3 Hash.
 *		- XNVM_EFUSE_ERR_WRITE_PPK4_HASH - Error while writing PPK4 Hash.
 *
 ******************************************************************************/
static int XNvm_EfusePrgmAdditionalPpkHash(const XNvm_EfuseAdditionalPpkHash *Hash)
{
	int Status = XST_FAILURE;

	if (Hash == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	/**< Read PPK3 and PPK4 enable bits*/
	Status = XNvm_EfuseIsAdditionalPpkEn();
	if(Status != XST_SUCCESS){
		goto END;
	}

	if (Hash->PrgmPpk3Hash == TRUE) {
		Status = XNvm_EfusePgmAndVerifyRows(
				XNVM_EFUSE_PPK_3_HASH_START_ROW,
				XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS,
				XNVM_EFUSE_PAGE_0,
				Hash->Ppk3Hash);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_PPK3_HASH);
			goto END;
		}
	}
	if (Hash->PrgmPpk4Hash == TRUE) {
		Status = XNvm_EfusePgmAndVerifyRows(
				XNVM_EFUSE_PPK_4_HASH_START_ROW,
				XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS,
				XNVM_EFUSE_PAGE_0,
				Hash->Ppk4Hash);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_PPK4_HASH);
			goto END;
		}
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs additional Ppk miscellaneous control bits.
 *
 * @param	WriteReq - Pointer to XNvm_EfuseMiscCtrlBits structure which
 * 			holds Misc ctrl bits data to be programmed to eFuse.
 *
 * @return	- XST_SUCCESS - On Successful write of Ppk Misc bits.
 *      - XNVM_EFUSE_ERR_RD_MISC_CTRL_BITS - Error while reading Misc
 *                          bits
 *      - XNVM_EFUSE_ERR_WRITE_ADD_PPK_EN - Error in writing additonal
 *                          PPK enable Misc bits
 *		- XNVM_EFUSE_ERR_WRITE_PPK3_INVALID_BIT_0 - Error in writing
 *							PPK3 Invalid bit0.
 *		- XNVM_EFUSE_ERR_WRITE_PPK3_INVALID_BIT_1 - Error in writing
 *							PPK3 Invalid bit1.
 *		- XNVM_EFUSE_ERR_WRITE_PPK4_INVALID_BIT_0 - Error in writing
 *							PPK4 Invalid bit0.
 *		- XNVM_EFUSE_ERR_WRITE_PPK4_INVALID_BIT_1 - Error in writing
 *							PPK4 Invalid bit1.
 *
 ******************************************************************************/
static int XNvm_EfusePrgmAdditionalPpksMiscBits(const XNvm_EfuseMiscCtrlBits *WriteReq)
{
	volatile int Status = XST_FAILURE;
	u32 RowData = XNVM_EFUSE_SEC_DEF_VAL_ALL_BIT_SET;

	if (WriteReq == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if ((WriteReq->Ppk3Invalid != FALSE) ||
		(WriteReq->Ppk4Invalid != FALSE) ||
		(WriteReq->AdditionalPpkEn != FALSE)) {
		Status = XNvm_EfuseReadCache(XNVM_EFUSE_MISC_CTRL_ROW,
						&RowData);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_RD_MISC_CTRL_BITS);
			goto END;
		}
	}
	else {
		Status = XST_SUCCESS;
		goto END;
	}

	if ((WriteReq->AdditionalPpkEn != FALSE) &&
		((RowData & XNVM_EFUSE_CACHE_MISC_CTRL_ADD_PPK_1_0_MASK) ==
		0x00U)) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_MISC_CTRL_ROW,
				(u32)XNVM_EFUSE_MISC_ADD_PPK_EN_BIT_0);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_ADD_PPK_EN);
			goto END;
		}
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(
				XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_MISC_CTRL_ROW,
				(u32)XNVM_EFUSE_MISC_ADD_PPK_EN_BIT_1);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_ADD_PPK_EN);
			goto END;
		}
	}

	/**< Read PPK3 and PPK4 enable bits*/
	if ((WriteReq->Ppk3Invalid != FALSE) ||
		(WriteReq->Ppk4Invalid != FALSE)) {
		Status = XST_FAILURE;
		Status = XNvm_EfuseIsAdditionalPpkEn();
		if(Status != XST_SUCCESS){
			goto END;
		}
	}

	if ((WriteReq->Ppk3Invalid != FALSE) &&
		((RowData & XNVM_EFUSE_CACHE_MISC_CTRL_PPK3_INVLD_1_0_MASK) ==
		0x00U)) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_MISC_CTRL_ROW,
				(u32)XNVM_EFUSE_MISC_PPK3_INVALID_BIT_0);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_PPK3_INVALID_BIT_0);
			goto END;
		}
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(
				XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_MISC_CTRL_ROW,
				(u32)XNVM_EFUSE_MISC_PPK3_INVALID_BIT_1);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_PPK3_INVALID_BIT_1);
			goto END;
		}
	}

	if ((WriteReq->Ppk4Invalid != FALSE) &&
		((RowData & XNVM_EFUSE_CACHE_MISC_CTRL_PPK4_INVLD_1_0_MASK) ==
		0x00U)) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_MISC_CTRL_ROW,
				(u32)XNVM_EFUSE_MISC_PPK4_INVALID_BIT_0);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_PPK4_INVALID_BIT_0);
			goto END;
		}
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_MISC_CTRL_ROW,
				(u32)XNVM_EFUSE_MISC_PPK4_INVALID_BIT_1);
		if (Status != XST_SUCCESS) {
			Status = (Status |
				XNVM_EFUSE_ERR_WRITE_PPK4_INVALID_BIT_1);
			goto END;
		}
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function reads the Additional Ppk Hash from eFUSE cache.
 *
 * @param	EfusePpk - Pointer to XNvm_PpkHash which holds the PpkHash from
 * 					   eFUSE Cache.
 * @param 	PpkType - Type of the Ppk to be programmed.
 *
 * @return	- XST_SUCCESS - On Successful read.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 * 		- XNVM_EFUSE_ERR_RD_PPK_HASH   - Error while reading PPK Hash.
 *
 ******************************************************************************/
int XNvm_EfuseReadAdditionalPpkHash(XNvm_PpkHash *EfusePpk, XNvm_PpkType PpkType)
{
	int Status = XST_FAILURE;
	u32 PpkRow;

    /**
	 *  validate input parameters. Return XNVM_EFUSE_ERR_INVALID_PARAM if input parameters are invalid
	 */
	if (EfusePpk == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if ((PpkType != XNVM_EFUSE_PPK3) &&
		(PpkType != XNVM_EFUSE_PPK4)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	if (PpkType == XNVM_EFUSE_PPK3) {
		PpkRow = XNVM_EFUSE_PPK_3_HASH_START_ROW;
	} else {
		PpkRow = XNVM_EFUSE_PPK_4_HASH_START_ROW;
	}

     /**
	  *  Read directly from cache offset of the mentioned ppk type to fill the ppk hash array. Return XST_SUCCESS if read success
	  */
	Status = XNvm_EfuseReadCacheRange(PpkRow,
					XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS,
					EfusePpk->Hash);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_RD_PPK_HASH);
		goto END;
	}
END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function Validates additional PPK Hash requested for programming.
 *
 * @param	WriteReq - Pointer to XNvm_EfuseAdditionalPpkHash structure which holds
 * 			additonal ppk Hash data to be programmed to eFuse.
 *
 * @return	- XST_SUCCESS - if additional PPK validation is successful.
 *      - XNVM_EFUSE_ERR_INVALID_PARAM - Error when invalid param is passed.
 *      - XNVM_EFUSE_ERR_ADD_PPK_PGM_NOT_ALLOWED -  Error when trying to program PPK3/PPK4
 *                             when PPK0/PPK1/PPK2 are not programmed.
 *		- XNVM_EFUSE_ERR_PPK3_HASH_ALREADY_PRGMD - Ppk3 hash already
 *							   programmed.
 *		- XNVM_EFUSE_ERR_PPK4_HASH_ALREADY_PRGMD - Ppk4 hash already
 *							   programmed.
 *
 ******************************************************************************/
static int XNvm_EfuseValidateAdditionalPpkWriteReq(const XNvm_EfuseAdditionalPpkHash *WriteReq)
{
	int Status = XST_FAILURE;
	u32 Index = 0U;
	u32 PpkStartRow = 0U;

	if (WriteReq == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if ((WriteReq->PrgmPpk3Hash == TRUE) ||
		(WriteReq->PrgmPpk4Hash == TRUE)) {
		for (Index = 0U; Index < XNVM_EFUSE_NUM_OF_PPKS; Index++) {
			PpkStartRow = (XNVM_EFUSE_PPK_0_HASH_START_ROW +
					Index * XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS);
			Status = XNvm_EfuseCheckZeros(PpkStartRow, (PpkStartRow +
					(Index * XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS)));
			if (Status == XST_SUCCESS) {
				Status = (int)XNVM_EFUSE_ERR_ADD_PPK_PGM_NOT_ALLOWED;
				goto END;
			}
		}
	}

	if (WriteReq->PrgmPpk3Hash == TRUE) {
		Status = XNvm_EfuseCheckZeros(XNVM_EFUSE_PPK_3_HASH_START_ROW,
					(XNVM_EFUSE_PPK_3_HASH_START_ROW +
					XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS));
		if (Status != XST_SUCCESS) {
			Status = (int)XNVM_EFUSE_ERR_PPK3_HASH_ALREADY_PRGMD;
			goto END;
		}
	}

	if (WriteReq->PrgmPpk4Hash == TRUE) {
		Status = XNvm_EfuseCheckZeros(XNVM_EFUSE_PPK_4_HASH_START_ROW,
					(XNVM_EFUSE_PPK_4_HASH_START_ROW +
					XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS));
		if (Status != XST_SUCCESS) {
			Status = (int)XNVM_EFUSE_ERR_PPK4_HASH_ALREADY_PRGMD;
			goto END;
		}
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

#endif /* END OF XNVM_EN_ADD_PPKS */
