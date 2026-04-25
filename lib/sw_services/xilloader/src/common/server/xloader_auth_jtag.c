/******************************************************************************
* Copyright (c) 2025 - 2026, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xloader_auth_jtag.c
*
* This file contains the code related to authenticating the JTAG access.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 2.3   har  09/26/2025 Initial release
* 2.4   abh  11/20/2025 Fixed MISRA-C violations
*       tvp  03/05/2026 Use XLoader_AuthKey to accommodate new algorithms
*                       support
*       sk   04/02/2026 Moved TAP unlock defines to Header file
*       vns  04/04/2026 Added JtagUnlockedByAuth flag to XLoader_AuthJtagStatus
*                       and XLoader_DisableJtagIfOpenedByAuthJtag to lock DAP
*                       during PLM update
*       sri  04/24/2026 Added LMS support for authenticated JTAG message
*
* </pre>
*
******************************************************************************/

/**
 * @addtogroup xloader_server_apis XilLoader Server APIs
 * @{
 */

/***************************** Include Files *********************************/
#include "xloader_auth_jtag.h"

#ifdef PLM_AUTH_JTAG
#include "xloader_secure.h"
#include "xloader_plat.h"
#include "xplmi.h"
#include "xplmi_scheduler.h"
#include "xloader_kat.h"
#if defined(VERSAL_2VE_2VM) || defined(VERSAL_2VP_P)
#include "xsecure_lms_core.h"
#endif

/************************** Constant Definitions *****************************/
#ifdef PLM_AUTH_JTAG_PPK_SPK
#define XLOADER_AUTH_JTAG_INT_STATUS_WAIT_TIMEOUT	(1000000U)
	/**< Timeout to wait for authenticated JTAG interrupt status bit to be set */
#define XLOADER_AUTH_JTAG_MAX_MSG_SIZE			(0x8000U)
		/**< The Authenticated JTAG message is stored in XPLMI_PMCRAM_CHUNK_MEMORY_1
		which is 32KB in size. Hence, the maximum acceptable length of
		Authenticated JTAG message can be 32KB (for buffer overflow checks) */
#define XLOADER_MIN_VALID_AUTH_JTAG_MSG_SIZE		(0x1E0)
		/**< For all the supported algorithms, the least possible size of the authenticated
		JTAG message is for ECDSA P-384 algorithm i.e 0x1E0. */
#define XLOADER_AUTH_JTAG_MSG_HEADER_LEN_IN_WORDS	(2U)
				/**< IdWord + MsgLength */
#endif

#define XLOADER_INVALID_REVOCATION_ID		(XLOADER_REVOCATION_IDMAX + 1U)
			/**< Invalid Revocation ID used for initialization */

/**************************** Type Definitions *******************************/
typedef struct {
	u32 JtagTimeOut;	/**< Timeout value set by user */
	u8 JtagTimerEnabled;	/**< Enable JTAG timer */
	u8 JtagUnlockedByAuth;	/**< TRUE only after successful Auth JTAG unlock */
	volatile u8 AuthFailCounter;
		/**< Counter for failed attempts to authenticate JTAG */
	volatile u8 AuthFailCounterTmp;	/**< For temporal redundancy */
} XLoader_AuthJtagStatus;
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
#ifndef PLM_AUTH_JTAG_PPK_SPK
static int XLoader_AuthJtagPpkOnly(u32 *TimeOut);
#else
static int XLoader_AuthJtagPpkNSpk(u32 *TimeOut);
#endif
/************************** Variable Definitions *****************************/
static XLoader_AuthJtagStatus AuthJtagStatus = {0U};

/*****************************************************************************/

/******************************************************************************/
/**
 * @brief   This function adds periodic checks of the status of Auth
 * 			JTAG interrupt status to the scheduler.
 *
 * @return
 * 			- XST_SUCCESS otherwise error code is returned
 * 			- XLOADER_ERR_GLITCH_DETECTED if glitch is detected.
 * 			- XLOADER_ERR_ADD_TASK_SCHEDULER if failed to add task to
 * 			scheduler.
 *
 ******************************************************************************/
int XLoader_AddAuthJtagToScheduler(void)
{
	volatile int Status = XST_FAILURE;
	volatile u32 AuthJtagDis = XLOADER_AUTH_JTAG_DIS_MASK;
	volatile u32 AuthJtagDisTmp = XLOADER_AUTH_JTAG_DIS_MASK;
	u32 ReadAuthReg;
	u32 SecureStateAHWRoT = XLoader_GetAHWRoT(NULL);

	/**
	 * - Read auth_jtag_disable efuse bits(20:19) of Security Control register
	 * in Efuse Cache module.
	 */
	AuthJtagDis = XPlmi_In32(XLOADER_EFUSE_CACHE_SECURITY_CONTROL_OFFSET) &
		XLOADER_AUTH_JTAG_DIS_MASK;
	AuthJtagDisTmp = XPlmi_In32(XLOADER_EFUSE_CACHE_SECURITY_CONTROL_OFFSET) &
		XLOADER_AUTH_JTAG_DIS_MASK;

	/**
	 * - Read PPK hash registers in Efuse Cache module and
	 * check if they are non-zero.
	 *
	 * - Check the preconditions for adding task to the scheduler
	 *   - Auth jtag disable efuse bits should not be set.
	 *   - PPK hash registers should be non zero.
	 *
	 * - If the preconditions are correct then add task to the scheduler for
	 * calling the API to check Auth JTAG interrupt status.
	 */
	if ((AuthJtagDis != XLOADER_AUTH_JTAG_DIS_MASK) &&
		(AuthJtagDisTmp != XLOADER_AUTH_JTAG_DIS_MASK)) {
		ReadAuthReg = XPlmi_In32(XPLMI_RTCFG_SECURESTATE_AHWROT_ADDR);
		Status = XLoader_CheckSecureState(ReadAuthReg, SecureStateAHWRoT,
			XPLMI_RTCFG_SECURESTATE_AHWROT);
		if (Status != XST_SUCCESS) {
			if (ReadAuthReg != SecureStateAHWRoT) {
				Status = XPlmi_UpdateStatus(XLOADER_ERR_GLITCH_DETECTED, XIL_SIGNED_ZERO);
			}
			else {
				Status = XST_SUCCESS;
			}
		}
		else {
			Status = XST_FAILURE;

			Status = XPlmi_SchedulerAddTask(XPLMI_MODULE_LOADER_ID,
				XLoader_CheckAuthJtagIntStatus, NULL,
				XLOADER_AUTH_JTAG_INT_STATUS_POLL_INTERVAL,
				XPLM_TASK_PRIORITY_1, NULL, XPLMI_PERIODIC_TASK);
			if (Status != XST_SUCCESS) {
				Status = XPlmi_UpdateStatus( XLOADER_ERR_ADD_TASK_SCHEDULER, XIL_SIGNED_ZERO);
			}
			else {
				XPlmi_Printf(DEBUG_INFO, "Auth Jtag task added successfully\r\n");
			}
		}
	}
	else {
		/**
		 * - The task should not be added to the scheduler if Auth JTAG
		 * disable efuse bit is set or PPK hash is not programmed in
		 * efuse. Thus forcing the Status to be XST_SUCCESS.
		 */
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief	This function checks the status of Auth JTAG interrupt status and
* 			it disables the Jtag as per the timeout set by user.
*
* @param	Arg Not used in the function currently
*
* @return
* 			- XST_SUCCESS on success.
* 			- XLOADER_DAP_TIMEOUT_DISABLED if timeout is disabled for DAP.
*
* @note    	If Auth JTAG interrupt status is set, then XLoader_AuthJtag
* 			API will be called.
*****************************************************************************/
int XLoader_CheckAuthJtagIntStatus(void *Arg)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	volatile u32 InterruptStatus = 0U;
	volatile u32 InterruptStatusTmp = 0U;
	volatile u32 LockDisStatus = 0U;
	volatile u32 LockDisStatusTmp = 0U;

	(void)Arg;

#ifdef PLM_AUTH_JTAG_PPK_SPK
	/** - Unlock the PMC_TAP registers */
	XPlmi_Out32(XLOADER_PMC_TAP_LOCK_ADDR, XLOADER_PMC_TAP_UNLOCK_VAL);
#endif

	/** - Read Auth Jtag Interrupt Status register in PMC TAP module. */
	InterruptStatus = XPlmi_In32(XLOADER_PMC_TAP_AUTH_JTAG_INT_STATUS_OFFSET) &
		XLOADER_PMC_TAP_AUTH_JTAG_INT_STATUS_MASK;
	InterruptStatusTmp = XPlmi_In32(XLOADER_PMC_TAP_AUTH_JTAG_INT_STATUS_OFFSET) &
		XLOADER_PMC_TAP_AUTH_JTAG_INT_STATUS_MASK;

	if ((InterruptStatus == XLOADER_PMC_TAP_AUTH_JTAG_INT_STATUS_MASK) &&
		(InterruptStatusTmp == XLOADER_PMC_TAP_AUTH_JTAG_INT_STATUS_MASK)) {

#ifndef PLM_AUTH_JTAG_PPK_SPK
		/** - Clear Auth Jtag Interrupt Status register in PMC TAP module */
		XPlmi_Out32(XLOADER_PMC_TAP_AUTH_JTAG_INT_STATUS_OFFSET,
			XLOADER_PMC_TAP_AUTH_JTAG_INT_STATUS_MASK);

		/** - Call XLoader_AuthJtagPpkOnly API to authenticate JTAG access */
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_AuthJtagPpkOnly,
			&AuthJtagStatus.JtagTimeOut);
#else
		/** - Call XLoader_AuthJtagPpkNSpk API to authenticate JTAG access */
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_AuthJtagPpkNSpk,
			&AuthJtagStatus.JtagTimeOut);

#endif
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			AuthJtagStatus.AuthFailCounter += 1U;
			AuthJtagStatus.AuthFailCounterTmp += 1U;
			/**
			 * - Check if Auth JTAG Lock disable efuse bits are set.
			 * If set then allow limited number of attempts to enable JTAG.
			 *
			 */
			LockDisStatus = XPlmi_In32(
					XLOADER_EFUSE_CACHE_SECURITY_CONTROL_OFFSET) &
				XLOADER_AUTH_JTAG_LOCK_DIS_MASK;
			LockDisStatusTmp = XPlmi_In32(
					XLOADER_EFUSE_CACHE_SECURITY_CONTROL_OFFSET) &
				XLOADER_AUTH_JTAG_LOCK_DIS_MASK;
			if ((LockDisStatus == XLOADER_AUTH_JTAG_LOCK_DIS_MASK) ||
					(LockDisStatusTmp == XLOADER_AUTH_JTAG_LOCK_DIS_MASK)) {
				if ((AuthJtagStatus.AuthFailCounter >= XLOADER_AUTH_JTAG_MAX_ATTEMPTS) ||
						(AuthJtagStatus.AuthFailCounterTmp >= XLOADER_AUTH_JTAG_MAX_ATTEMPTS)) {
					/**
					 * - When AUTH_JTAG_LOCK_DIS eFuse is programmed, upon failure
					 * of the authentication, the secure lockdown is triggered.
					 */
					XPlmi_TriggerTamperResponse(XPLMI_RTCFG_TAMPER_RESP_SLD_1_MASK,
							XPLMI_TRIGGER_TAMPER_TASK);
				}
			}
			goto END;
		}

		if (AuthJtagStatus.JtagTimeOut == 0U) {
			AuthJtagStatus.JtagTimerEnabled = FALSE;
		}
		else {
			AuthJtagStatus.JtagTimerEnabled = TRUE;
		}
		AuthJtagStatus.JtagUnlockedByAuth = (u8)TRUE;
	}
	else {
		if (AuthJtagStatus.JtagTimerEnabled == TRUE) {
			AuthJtagStatus.JtagTimeOut--;
			if (AuthJtagStatus.JtagTimeOut == 0U) {
				Status = (int)XLOADER_DAP_TIMEOUT_DISABLED;
				goto END;
			}
		}
		Status = XST_SUCCESS;
	}

END:
	/** - Reset DAP status */
	if (Status != XST_SUCCESS) {
		(void)XLoader_DisableJtag();
		AuthJtagStatus.JtagTimerEnabled = FALSE;
		AuthJtagStatus.JtagTimeOut = (u32)0U;
		AuthJtagStatus.JtagUnlockedByAuth = (u8)FALSE;
	}

#ifdef PLM_AUTH_JTAG_PPK_SPK
	/** - Lock the PMC_TAP registers */
	XPlmi_Out32(XLOADER_PMC_TAP_LOCK_ADDR, XLOADER_PMC_TAP_LOCK_VAL);
#endif
	return Status;
}

/******************************************************************************/
/**
* @brief	This function disables JTAG only if it was previously opened via
* 		an authenticated JTAG unlock. JTAG opened by other means (e.g.
* 		Configure JTAG State with non-secure debug) is left untouched.
*
* @return
* 		- XST_SUCCESS on success or if JTAG was not opened by Auth JTAG.
* 		- Error code from XLoader_DisableJtag() on failure.
*
******************************************************************************/
int XLoader_DisableJtagIfOpenedByAuthJtag(void)
{
	volatile int Status = XST_FAILURE;

	if (AuthJtagStatus.JtagUnlockedByAuth == (u8)TRUE) {
		Status = XLoader_DisableJtag();
		AuthJtagStatus.JtagUnlockedByAuth = (u8)FALSE;
		AuthJtagStatus.JtagTimerEnabled = (u8)FALSE;
		AuthJtagStatus.JtagTimeOut = 0U;
	}
	else {
		Status = XST_SUCCESS;
	}

	return Status;
}

#ifndef PLM_AUTH_JTAG_PPK_SPK
/*****************************************************************************/
/**
 * @brief	This function authenticates the data pushed in through PMC TAP
 *		before enabling the JTAG. It verifies the signature of the data using
 *		the PPK stored in efuse.
 *
 * @param	TimeOut Pointer to store the timeout value set by the user
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- XLOADER_ERR_AUTH_JTAG_INVALID_PARAM if TimeOut pointer is NULL.
 *		- XLOADER_ERR_AUTH_JTAG_DMA_XFR if failed to get authenticated JTAG
 *		data with DMA transfer.
 *		- XLOADER_ERR_AUTH_JTAG_DISABLED if JTAG authentication disable
 *		bit is set in efuse.
 *		- XLOADER_ERR_GLITCH_DETECTED if glitch is detected.
 *		- XLOADER_ERR_AUTH_JTAG_EFUSE_AUTH_COMPULSORY if PPK is not
 *		programmed JTAG authentication fails.
 *		- XLOADER_ERR_AUTH_JTAG_GET_DMA if failed to get DMA instance for
 *		JTAG authentication.
 *		- XLOADER_ERR_KAT_FAILED if KAT (Known Answer Test) fails.
 *		- XLOADER_ERR_AUTH_JTAG_PPK_VERIFY_FAIL if failed to verify PPK,
 *		during JTAG authentication.
 *		- XLOADER_ERR_AUTH_JTAG_SPK_REVOKED if revoke ID is programmed,
 *		during JTAG authentication.
 *		- XLOADER_ERR_AUTH_JTAG_HASH_CALCULATION_FAIL if failed to calculate
 *		hash.
 *		- XLOADER_ERR_AUTH_JTAG_SIGN_VERIFY_FAIL if failed to verify
 *		signature.
 *		- XLOADER_ERR_AUTH_JTAG_INVALID_DNA if DNA validation fails.
 *
 ******************************************************************************/
static int XLoader_AuthJtagPpkOnly(u32 *TimeOut)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	int ClearStatus = XST_FAILURE;
	volatile u32 AuthJtagDis = 0U;
	volatile u32 AuthJtagDisTmp = 0U;
	volatile u32 RevokeId = XLOADER_INVALID_REVOCATION_ID;
	XLoader_SecureParams SecureParams = {0U};
	XSecure_Sha3Hash Sha3Hash = {0U};
	XSecure_Sha *ShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);
	u32 ReadAuthReg;
	volatile u8 UseDna;
	volatile u8 UseDnaTmp;
	u32 SecureStateAHWRoT = XLoader_GetAHWRoT(NULL);

	/** - Validate TimeOut pointer */
	if (TimeOut == NULL) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_AUTH_JTAG_INVALID_PARAM, XIL_SIGNED_ZERO);
		goto END;
	}

	/** - Check efuse bits for secure debug disable */
	AuthJtagDis = XPlmi_In32(XLOADER_EFUSE_CACHE_SECURITY_CONTROL_OFFSET) &
					XLOADER_AUTH_JTAG_DIS_MASK;
	AuthJtagDisTmp = XPlmi_In32(XLOADER_EFUSE_CACHE_SECURITY_CONTROL_OFFSET) &
					XLOADER_AUTH_JTAG_DIS_MASK;
	if ((AuthJtagDis == XLOADER_AUTH_JTAG_DIS_MASK) ||
		(AuthJtagDisTmp == XLOADER_AUTH_JTAG_DIS_MASK)) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_AUTH_JTAG_DISABLED, XIL_SIGNED_ZERO);
		goto END;
	}

	/**
	 * -To reduce stack usage, instance of XLoader_AuthJtagMessage is moved to
	 * a structure called XLoader_StoreSecureData which resides at
	 * XPLMI_PMC_CHUNK_MEMORY_1.
	 */
	SecureParams.AuthJtagMessagePtr = (XLoader_AuthJtagMessage *)
		(UINTPTR)XPLMI_PMCRAM_CHUNK_MEMORY_1;

	Status = XPlmi_MemSetBytes(SecureParams.AuthJtagMessagePtr,
		sizeof(XLoader_AuthJtagMessage), 0U, sizeof(XLoader_AuthJtagMessage));
	if (Status != XST_SUCCESS) {
		XPLMI_STATUS_GLITCH_DETECT(Status);
		goto END;
	}

	/**
	 * - Read Authenticated JTAG data from the PMC TAP registers and
	 * store it in a local buffer.
	 */
	Status = XPlmi_DmaXfr(XLOADER_PMC_TAP_AUTH_JTAG_DATA_OFFSET,
			(u64)(u32)SecureParams.AuthJtagMessagePtr,
			XLOADER_AUTH_JTAG_DATA_LEN_IN_WORDS, XPLMI_PMCDMA_0);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_AUTH_JTAG_DMA_XFR, XIL_SIGNED_ZERO);
		goto END;
	}

	/**
	 * - Check Secure State of device
	 * If A-HWRoT is not enabled then return error
	 */
	ReadAuthReg = XPlmi_In32(XPLMI_RTCFG_SECURESTATE_AHWROT_ADDR);
	Status = XLoader_CheckSecureState(ReadAuthReg, SecureStateAHWRoT,
		XPLMI_RTCFG_SECURESTATE_AHWROT);
	if (Status != XST_SUCCESS) {
		XPLMI_STATUS_GLITCH_DETECT(Status);
		if (ReadAuthReg != SecureStateAHWRoT) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_GLITCH_DETECTED, XIL_SIGNED_ZERO);
		}
		else {
			Status = XPlmi_UpdateStatus(
				XLOADER_ERR_AUTH_JTAG_EFUSE_AUTH_COMPULSORY, XIL_SIGNED_ZERO);
		}
		goto END;
	}

	SecureParams.PmcDmaInstPtr = XPlmi_GetDmaInstance(PMCDMA_0_DEVICE);
	if (SecureParams.PmcDmaInstPtr == NULL) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_AUTH_JTAG_GET_DMA, XIL_SIGNED_ZERO);
		goto END;
	}

	XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XLOADER_ERR_KAT_FAILED, Status, StatusTmp, XSecure_Sha3Kat, ShaInstPtr);
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_KAT_FAILED, Status);
		goto END;
	}

	/** -  Verify PPK in the authenticated JTAG data. */
	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_PpkVerify, &SecureParams, XLOADER_PPK_SIZE);
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_AUTH_JTAG_PPK_VERIFY_FAIL,
			Status);
		goto END;
	}

	/**
	 * - Calculate hash of the Authentication Header in the authenticated
	 * JTAG data.
	 */
	Status = XSecure_ShaStart(ShaInstPtr, XSECURE_SHA3_384);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_AUTH_JTAG_HASH_CALCULATION_FAIL,
						 Status);
		goto END;
	}

	Status = XSecure_ShaLastUpdate(ShaInstPtr);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_AUTH_JTAG_HASH_CALCULATION_FAIL,
			 Status);
		goto END;
	}

	Status = XSecure_ShaUpdate(ShaInstPtr,
		 (UINTPTR)&(SecureParams.AuthJtagMessagePtr->AuthHdr),
		 XLOADER_AUTH_JTAG_DATA_AH_LENGTH);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(
			XLOADER_ERR_AUTH_JTAG_HASH_CALCULATION_FAIL, Status);
		goto END;
	}

	Status = XSecure_ShaFinish(ShaInstPtr, (u64)(UINTPTR)&Sha3Hash, XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(
			XLOADER_ERR_AUTH_JTAG_HASH_CALCULATION_FAIL, Status);
		goto END;
	}

	/** - Run KAT before verifying AUTH JTAG message */
	Status = XST_FAILURE;
	Status = XLoader_AuthKat(&SecureParams);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_KAT_FAILED, Status);
		goto END;
	}

	/** -  Verify signature of Auth Jtag data */
	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_VerifySignature,
		&SecureParams, Sha3Hash.Hash,
		&(SecureParams.AuthJtagMessagePtr->PpkData),
		(u8*)&(SecureParams.AuthJtagMessagePtr->EnableJtagSignature));
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		Status = XPlmi_UpdateStatus(
			XLOADER_ERR_AUTH_JTAG_SIGN_VERIFY_FAIL, Status);
		goto END;
	}

	UseDna = (u8)(SecureParams.AuthJtagMessagePtr->Attrb &
			XLOADER_AC_AH_DNA_MASK);
	UseDnaTmp = (u8)(SecureParams.AuthJtagMessagePtr->Attrb &
			XLOADER_AC_AH_DNA_MASK);
	if ((UseDna != (u8)FALSE) || (UseDnaTmp != (u8)FALSE)) {
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, Xil_SMemCmp_CT,
			SecureParams.AuthJtagMessagePtr->Dna,
			XLOADER_EFUSE_DNA_LEN_IN_BYTES,
			(void *)XLOADER_EFUSE_DNA_START_OFFSET,
			XLOADER_EFUSE_DNA_LEN_IN_BYTES,
			XLOADER_EFUSE_DNA_LEN_IN_BYTES);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = XPlmi_UpdateStatus(
				XLOADER_ERR_AUTH_JTAG_INVALID_DNA, XIL_SIGNED_ZERO);
			goto END;
		}
	}

	/** - Verify if SPK Id is revoked or not. */
	RevokeId = SecureParams.AuthJtagMessagePtr->RevocationIdMsgType &
			XLOADER_AC_AH_REVOKE_ID_MASK;
	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_VerifyRevokeId,
		RevokeId);
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_AUTH_JTAG_SPK_REVOKED,
			Status);
		goto END;
	}

	Status = XLoader_EnableJtag((u32)XLOADER_CONFIG_DAP_STATE_ALL_DBG);
	*TimeOut = SecureParams.AuthJtagMessagePtr->JtagEnableTimeout;

END:
	ClearStatus = XPlmi_MemSetBytes(&Sha3Hash, XSECURE_SHA_384_HASH_SIZE_IN_BYTES, 0U,
		XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
	if (ClearStatus != XST_SUCCESS) {
		Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_ERR);
	}
	return Status;
}

#else
/*****************************************************************************/
/**
 * @brief	This function authenticates the data pushed in through PMC TAP
 *		before enabling the JTAG. It verifies the signature of the data using
 *		the SPK which is verified using the PPK stored in efuse.
 *
 * @param	TimeOut Pointer to store the timeout value set by the user
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- XLOADER_ERR_AUTH_JTAG_INVALID_PARAM if TimeOut pointer is NULL.
 *		- XLOADER_ERR_AUTH_JTAG_DISABLED if JTAG authentication disable
 *		bit is set in efuse.
 *		- XLOADER_ERR_AUTH_JTAG_INVALID_IDWORD if IdWord in authenticated
 *		JTAG message is invalid.
 *		- XLOADER_ERR_AUTH_JTAG_INVALID_MSG_LEN if message length is
 *		invalid or out of bounds.
 *		- XLOADER_ERR_AUTH_JTAG_DMA_XFR if failed to get authenticated JTAG
 *		data with DMA transfer.
 *		- XLOADER_ERR_GLITCH_DETECTED if glitch is detected.
 *		- XLOADER_ERR_AUTH_JTAG_EFUSE_AUTH_COMPULSORY if PPK is not
 *		programmed JTAG authentication fails.
 *		- XLOADER_ERR_AUTH_JTAG_GET_DMA if failed to get DMA instance for
 *		JTAG authentication.
 *		- XLOADER_ERR_KAT_FAILED if KAT (Known Answer Test) fails.
 *		- XLOADER_ERR_AUTH_JTAG_PPK_VERIFY_FAIL if failed to verify PPK,
 *		during JTAG authentication.
 *		- XLOADER_ERR_SPK_HASH_CALC_FAIL if SPK hash calculation fails.
 *		- XLOADER_ERR_AUTH_JTAG_SPK_REVOKED if revoke ID is programmed,
 *		during JTAG authentication.
 *		- XLOADER_ERR_AUTH_JTAG_HASH_CALCULATION_FAIL if failed to calculate
 *		hash.
 *		- XLOADER_ERR_AUTH_JTAG_SIGN_VERIFY_FAIL if failed to verify
 *		signature.
 *		- XLOADER_ERR_AUTH_JTAG_INVALID_DNA if DNA validation fails.
 *
 ******************************************************************************/
static int XLoader_AuthJtagPpkNSpk(u32 *TimeOut)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	int ClearStatus = XST_FAILURE;
	volatile u32 AuthJtagDis = 0U;
	volatile u32 AuthJtagDisTmp = 0U;
	volatile u32 RevokeId = XLOADER_INVALID_REVOCATION_ID;
	static XLoader_SecureParams SecureParams = {0U};
	XSecure_Sha *ShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);
	u32 ReadAuthReg;
	volatile u8 UseDna;
	volatile u8 UseDnaTmp;
	u32 SecureStateAHWRoT = XLoader_GetAHWRoT(NULL);
	u32 MsgLenInBytes = 0U;
	u32 IdWord;
	u8 SpkHash[XSECURE_SHA_384_HASH_SIZE_IN_BYTES];
	u8 Sha3Hash[XSECURE_SHA_384_HASH_SIZE_IN_BYTES];
	u32 AuthType;
	u8 IsLmsAuth;
	u32 CopyLen;
	u32 MsgLenInWords;
	volatile u32 RemainingWords;
	XLoader_AuthJtagData KeyData;
	u32* CurrPtr;

	/** - Validate TimeOut pointer */
	if (TimeOut == NULL) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_AUTH_JTAG_INVALID_PARAM, XIL_SIGNED_ZERO);
		goto END;
	}

	/** - Check efuse bits for secure debug disable */
	AuthJtagDis = XPlmi_In32(XLOADER_EFUSE_CACHE_SECURITY_CONTROL_OFFSET) &
					XLOADER_AUTH_JTAG_DIS_MASK;
	AuthJtagDisTmp = XPlmi_In32(XLOADER_EFUSE_CACHE_SECURITY_CONTROL_OFFSET) &
					XLOADER_AUTH_JTAG_DIS_MASK;
	if ((AuthJtagDis == XLOADER_AUTH_JTAG_DIS_MASK) ||
		(AuthJtagDisTmp == XLOADER_AUTH_JTAG_DIS_MASK)) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_AUTH_JTAG_DISABLED, XIL_SIGNED_ZERO);
		goto END;
	}

	/**
	 * - To reduce stack usage, instance of XLoader_AuthJtagMessage is moved to
	 * a structure called XLoader_StoreSecureData which resides at
	 * XPLMI_PMC_CHUNK_MEMORY_1.
	 */
	SecureParams.AuthJtagMessagePtr = (XLoader_AuthJtagMessage *)(UINTPTR)XPLMI_PMCRAM_CHUNK_MEMORY_1;

	Status = XPlmi_MemSetBytes(SecureParams.AuthJtagMessagePtr,
		sizeof(XLoader_AuthJtagMessage), 0U, sizeof(XLoader_AuthJtagMessage));
	if (Status != XST_SUCCESS) {
		XPLMI_STATUS_GLITCH_DETECT(Status);
		goto END;
	}

	/** - Read IdWord and Message Length */
	IdWord = XPlmi_In32(XLOADER_PMC_TAP_AUTH_JTAG_DATA_OFFSET);
	if (IdWord != XLOADER_AUTH_JTAG_IDWORD) {
		XPlmi_Printf(DEBUG_GENERAL, "ERROR: Invalid ID word 0x%x\r\n", IdWord);
		Status = XPlmi_UpdateStatus(XLOADER_ERR_AUTH_JTAG_INVALID_IDWORD, XIL_SIGNED_ZERO);
		goto END;
	}

	MsgLenInBytes = XPlmi_In32(XLOADER_PMC_TAP_AUTH_JTAG_DATA_OFFSET);

	/**
	 * - Validate message length:
	 *   It should not be zero
	 *   It should not exceed the maximum buffer size allocated for authenticated JTAG message (32KB)
	 *   It should be a multiple of word length (4 bytes) since data is read in word chunks from PMC TAP
	 *   It should be at least the minimum size required for the supported algorithms
	 *   (0x1E0 bytes for ECDSA P-384, which is the smallest of all supported algorithms)
	*/
	if ((MsgLenInBytes == 0U) ||
		(MsgLenInBytes > XLOADER_AUTH_JTAG_MAX_MSG_SIZE) ||
		((MsgLenInBytes % XPLMI_WORD_LEN) != 0U) ||
		(MsgLenInBytes < XLOADER_MIN_VALID_AUTH_JTAG_MSG_SIZE)) {

		XPlmi_Printf(DEBUG_GENERAL, "ERROR: Invalid message length 0x%x\r\n", MsgLenInBytes);
		Status = XPlmi_UpdateStatus(XLOADER_ERR_AUTH_JTAG_INVALID_MSG_LEN, XIL_SIGNED_ZERO);
		goto END;
	}

	/** - Store initial values */
	SecureParams.AuthJtagMessagePtr->IdWord = IdWord;
	SecureParams.AuthJtagMessagePtr->AuthJtagMessageLen = MsgLenInBytes;

	/**
	 * - Total words to copy includes IdWord and Length which have been already read
	 *   Remaining words to copy is total words minus the 2 words already read
	 */
	MsgLenInWords = MsgLenInBytes / XPLMI_WORD_LEN;
	RemainingWords = MsgLenInWords - XLOADER_AUTH_JTAG_MSG_HEADER_LEN_IN_WORDS;

	/**
	 * - Set current pointer to start after IdWord and Length
	 */
	CurrPtr = (u32*)SecureParams.AuthJtagMessagePtr;
	CurrPtr += XLOADER_AUTH_JTAG_MSG_HEADER_LEN_IN_WORDS;

	/**
	 * - Read remaining authenticated JTAG data in chunks from PMC TAP
	 *   For messages which are > 512 words, they are transferred in multiple iterations
	 */
	while (RemainingWords > 0U) {
		/**
		 * - Wait for AUTH_JTAG interrupt status to indicate data is ready
		 *   This ensures hardware has prepared the next chunk of data
		 *   before attempting to read from PMC TAP registers
		 */
		Status = XST_FAILURE;
		Status = (int)Xil_WaitForEvent((UINTPTR)XLOADER_PMC_TAP_AUTH_JTAG_INT_STATUS_OFFSET,
			XLOADER_PMC_TAP_AUTH_JTAG_INT_STATUS_MASK, XLOADER_PMC_TAP_AUTH_JTAG_INT_STATUS_MASK,
			XLOADER_AUTH_JTAG_INT_STATUS_WAIT_TIMEOUT);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		/**
		 * - Determine the number of words to copy in this iteration.
		 *   The first hardware chunk already had IdWord and MsgLen consumed,
		 *   so reduce CopyLen accordingly on the first chunk.
		 */
		if (RemainingWords >= XLOADER_AUTH_JTAG_DATA_LEN_IN_WORDS) {
			CopyLen = XLOADER_AUTH_JTAG_DATA_LEN_IN_WORDS;
			if (CurrPtr == ((u32 *)SecureParams.AuthJtagMessagePtr +
					XLOADER_AUTH_JTAG_MSG_HEADER_LEN_IN_WORDS)) {
				CopyLen -= XLOADER_AUTH_JTAG_MSG_HEADER_LEN_IN_WORDS;
			}
		}
		else {
			CopyLen = RemainingWords;
		}

		/** - Perform DMA transfer from PMC TAP data registers to memory buffer */
		Status = XST_FAILURE;
		Status = XPlmi_DmaXfr(XLOADER_PMC_TAP_AUTH_JTAG_DATA_OFFSET, (u64)(UINTPTR)CurrPtr, CopyLen,
			XPLMI_PMCDMA_0);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_AUTH_JTAG_DMA_XFR, XIL_SIGNED_ZERO);
			goto END;
		}

		CurrPtr += CopyLen;
		RemainingWords -= CopyLen;

		/** - Clear Auth Jtag Interrupt Status register in PMC TAP module */
		XPlmi_Out32(XLOADER_PMC_TAP_AUTH_JTAG_INT_STATUS_OFFSET,
			XLOADER_PMC_TAP_AUTH_JTAG_INT_STATUS_MASK);
	}

	/**
	 * - RemainingWords must be zero after the read loop terminates;
	 *   a non-zero value indicates a glitch/tamper condition
	 */
	if (RemainingWords != 0U) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_GLITCH_DETECTED, XIL_SIGNED_ZERO);
		goto END;
	}

	/**
	 * - Check Secure State of device
	 * If A-HWRoT is not enabled then return error
	 */
	Status = XST_FAILURE;
	ReadAuthReg = XPlmi_In32(XPLMI_RTCFG_SECURESTATE_AHWROT_ADDR);
	Status = XLoader_CheckSecureState(ReadAuthReg, SecureStateAHWRoT,
		XPLMI_RTCFG_SECURESTATE_AHWROT);
	if (Status != XST_SUCCESS) {
		XPLMI_STATUS_GLITCH_DETECT(Status);
		if (ReadAuthReg != SecureStateAHWRoT) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_GLITCH_DETECTED, XIL_SIGNED_ZERO);
		}
		else {
			Status = XPlmi_UpdateStatus(
				XLOADER_ERR_AUTH_JTAG_EFUSE_AUTH_COMPULSORY, XIL_SIGNED_ZERO);
		}
		goto END;
	}

	SecureParams.PmcDmaInstPtr = XPlmi_GetDmaInstance(PMCDMA_0_DEVICE);
	if (SecureParams.PmcDmaInstPtr == NULL) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_AUTH_JTAG_GET_DMA, XIL_SIGNED_ZERO);
		goto END;
	}

	/** - Run KAT for SHA3 engine */
	XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XLOADER_ERR_KAT_FAILED, Status, StatusTmp, XSecure_Sha3Kat, ShaInstPtr);
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_KAT_FAILED, Status);
		goto END;
	}

	KeyData.PpkData = (u32*)SecureParams.AuthJtagMessagePtr->AuthJtagData;
	KeyData.SpkHeader = (XLoader_SpkHeader *)(KeyData.PpkData + (SecureParams.AuthJtagMessagePtr->TotalPpkSize / XPLMI_WORD_LEN));
	KeyData.SpkData = (u32*)((u32 *)(KeyData.SpkHeader) + (XLOADER_SPK_HEADER_SIZE / XPLMI_WORD_LEN));
	KeyData.SPKSignature = (u32*)(KeyData.SpkData + (KeyData.SpkHeader->TotalSPKSize / XPLMI_WORD_LEN));
	KeyData.EnableJtagSignature = (u32*)(KeyData.SPKSignature + (KeyData.SpkHeader->TotalSignatureSize / XPLMI_WORD_LEN));

	/** - Verify PPK in the authenticated JTAG data. */
	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_PpkVerify, &SecureParams, SecureParams.AuthJtagMessagePtr->ActualPpkSize);
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_AUTH_JTAG_PPK_VERIFY_FAIL,
			Status);
		goto END;
	}

	/** - Run respective Authentication type KAT before verifying AUTH JTAG message */
	AuthType = XLoader_GetAuthPubAlgo(&SecureParams.AuthJtagMessagePtr->AuthHdr);
	IsLmsAuth = (((AuthType == XLOADER_PUB_STRENGTH_LMS) ||
		(AuthType == XLOADER_PUB_STRENGTH_LMS_HSS)) ? (u8)TRUE : (u8)FALSE);
	if ((IsLmsAuth != (u8)TRUE) &&
		(AuthType != XLOADER_PUB_STRENGTH_RSA_4096) &&
		(AuthType != XLOADER_PUB_STRENGTH_ECDSA_P384) &&
		(AuthType != XLOADER_PUB_STRENGTH_ECDSA_P521)) {
		XPlmi_Printf(DEBUG_INFO, "Authentication type is invalid\n\r");
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_INVALID_AUTH, 0);
		goto END;
	}

	if (IsLmsAuth != (u8)TRUE) {
		Status = XST_FAILURE;
		Status = XLoader_AuthKat(&SecureParams);
	}
	else {
		/** - Get the LMS Hash algorithm present in public key */
		Status = XSecure_GetLmsHashAlgo(AuthType, (u8 *)KeyData.PpkData,
				&SecureParams.SignHashAlgo);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_GET_LMS_ALGO_FAILED, Status);
			goto END;
		}
		Status = XST_FAILURE;
		Status = XLoader_LmsKat(&SecureParams, AuthType);
	}
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_KAT_FAILED, Status);
		goto END;
	}

	/** - Calculate hash for SPK signature */
	Status = XLoader_ShaDigestCalculation((u8 *)KeyData.SpkHeader,
			XLOADER_SPK_HEADER_SIZE + KeyData.SpkHeader->TotalSPKSize,
			&SpkHash[0]);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SPK_HASH_CALC_FAIL, Status);
		goto END;
	}

	/** - Verify SPK signature */
	if (IsLmsAuth != (u8)TRUE) {
		XSECURE_TEMPORAL_CHECK(END, Status, XLoader_VerifySignature, &SecureParams,
			(u8 *)&SpkHash, (XLoader_AuthKey *)KeyData.PpkData, (u8*)KeyData.SPKSignature);
	}
	else {
		SecureParams.AcPtr->AuthHdr = SecureParams.AuthJtagMessagePtr->AuthHdr;

		XSECURE_TEMPORAL_CHECK(END, Status, XLoader_VerifyLmsSignature, &SecureParams,
			(u8 *)KeyData.SPKSignature,
			KeyData.SpkHeader->SignatureSize,
			(u8 *)KeyData.PpkData,
			SecureParams.AuthJtagMessagePtr->ActualPpkSize,
			(u8 *)KeyData.SpkHeader,
			XLOADER_SPK_HEADER_SIZE + KeyData.SpkHeader->SPKSize);
	}

	/** - Verify if SPK Id is revoked or not. */
	RevokeId = KeyData.SpkHeader->SPKId;
	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_VerifyRevokeId, RevokeId);
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_AUTH_JTAG_SPK_REVOKED, Status);
		goto END;
	}

	/** - Calculate hash for Authenticated JTAG signature */
	Status = XST_FAILURE;
	Status = XLoader_ShaDigestCalculation((u8 *)&SecureParams.AuthJtagMessagePtr->IdWord,
			(MsgLenInBytes - SecureParams.AuthJtagMessagePtr->ActualAuthJtagSignSize),
			&Sha3Hash[0]);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SPK_HASH_CALC_FAIL, Status);
		goto END;
	}

	/** - Verify signature of Auth Jtag data */
	if (IsLmsAuth != (u8)TRUE) {
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_VerifySignature,
			&SecureParams, (u8 *)&Sha3Hash, (XLoader_AuthKey *)KeyData.SpkData, (u8*)KeyData.EnableJtagSignature);
	}
	else {
		SecureParams.AcPtr->AuthHdr = SecureParams.AuthJtagMessagePtr->AuthHdr;

		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_VerifyLmsSignature, &SecureParams,
			(u8 *)KeyData.EnableJtagSignature,
			SecureParams.AuthJtagMessagePtr->ActualAuthJtagSignSize,
			(u8 *)KeyData.SpkData,
			KeyData.SpkHeader->SPKSize,
			(u8 *)&SecureParams.AuthJtagMessagePtr->IdWord,
			(MsgLenInBytes - SecureParams.AuthJtagMessagePtr->ActualAuthJtagSignSize));
	}

	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		Status = XPlmi_UpdateStatus(
			XLOADER_ERR_AUTH_JTAG_SIGN_VERIFY_FAIL, Status);
	}
	else {
		UseDna = (u8)(SecureParams.AuthJtagMessagePtr->Attrb &
				XLOADER_AC_AH_DNA_MASK);
		UseDnaTmp = (u8)(SecureParams.AuthJtagMessagePtr->Attrb &
				XLOADER_AC_AH_DNA_MASK);
		if ((UseDna != FALSE) || (UseDnaTmp != FALSE)) {
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, Xil_SMemCmp_CT,
				SecureParams.AuthJtagMessagePtr->Dna,
				XLOADER_EFUSE_DNA_LEN_IN_BYTES,
				(void *)XLOADER_EFUSE_DNA_START_OFFSET,
				XLOADER_EFUSE_DNA_LEN_IN_BYTES,
				XLOADER_EFUSE_DNA_LEN_IN_BYTES);
			if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
				Status = XPlmi_UpdateStatus(
					XLOADER_ERR_AUTH_JTAG_INVALID_DNA, XIL_SIGNED_ZERO);
				goto END;
			}
		}
		Status = XST_FAILURE;
		Status = XLoader_EnableJtag((u32)XLOADER_CONFIG_DAP_STATE_ALL_DBG);
		*TimeOut = SecureParams.AuthJtagMessagePtr->JtagEnableTimeout;
	}

END:
	/** - Clear Auth Jtag Interrupt Status register in PMC TAP module */
	XPlmi_Out32(XLOADER_PMC_TAP_AUTH_JTAG_INT_STATUS_OFFSET,
		XLOADER_PMC_TAP_AUTH_JTAG_INT_STATUS_MASK);


	ClearStatus = XPlmi_MemSetBytes(&Sha3Hash, XSECURE_SHA_384_HASH_SIZE_IN_BYTES, 0U,
			XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
	if (ClearStatus != XST_SUCCESS) {
		Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_ERR);
	}
	return Status;
}

#endif /**< PLM_AUTH_JTAG_PPK_SPK */
#endif	/**< PLM_AUTH_JTAG */

/** @} end of xloader_server_apis group */
