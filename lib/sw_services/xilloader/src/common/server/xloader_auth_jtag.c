/******************************************************************************
* Copyright (c) 2025, Advanced Micro Devices, Inc. All Rights Reserved.
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

/************************** Constant Definitions *****************************/
#ifdef PLM_AUTH_JTAG_PPK_SPK
#define XLOADER_PMC_TAP_LOCK_ADDR		(0xF11B1000U)
				/**< Address of PMC_TAP_LOCK register */
#define XLOADER_PMC_TAP_LOCK_VAL		(0x1U)
				/**< Value to lock write to PMC_TAP registers */
#define XLOADER_PMC_TAP_UNLOCK_VAL		(0x0U)
				/**< Value to unlock write to PMC_TAP registers */
#define XLOADER_AUTH_JTAG_INT_STATUS_WAIT_TIMEOUT	(1000000U)
	/**< Timeout to wait for authenticated JTAG interrupt status bit to be set */
#endif

/**************************** Type Definitions *******************************/
typedef struct {
	u32 JtagTimeOut;	/**< Timeout value set by user */
	u8 JtagTimerEnabled;	/**< Enable JTAG timer */
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
				Status = XPlmi_UpdateStatus(XLOADER_ERR_GLITCH_DETECTED, 0U);
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
				Status = XPlmi_UpdateStatus( XLOADER_ERR_ADD_TASK_SCHEDULER, 0U);
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
					 * - When AUTH_JTAG_LOCK_DIS eFuse is programmed, allow only one
					 * failed attempt for AuthJTag message. For the second failure
					 * trigger secure lock down.
					 *
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
	}

#ifdef PLM_AUTH_JTAG_PPK_SPK
	/** - Lock the PMC_TAP registers */
	XPlmi_Out32(XLOADER_PMC_TAP_LOCK_ADDR, XLOADER_PMC_TAP_LOCK_VAL);
#endif
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
 *		- XLOADER_ERR_AUTH_JTAG_DMA_XFR if failed to get authenticated JTAG
 *		data with DMA transfer.
 *		- XLOADER_ERR_AUTH_JTAG_DISABLED if JTAG authentication disable
 *		bit is set in efuse.
 *		- XLOADER_ERR_GLITCH_DETECTED if glitch is detected.
 *		- XLOADER_ERR_AUTH_JTAG_EFUSE_AUTH_COMPULSORY if PPK is not
 *		programmed JTAG authentication fails.
 *		- XLOADER_ERR_AUTH_JTAG_GET_DMA if failed to get DMA instance for
 *		JTAG authentication.
 *		- XLOADER_ERR_AUTH_JTAG_PPK_VERIFY_FAIL if failed to verify PPK,
 *		during JTAG authentication.
 *		- XLOADER_ERR_AUTH_JTAG_SPK_REVOKED if revoke ID is programmed,
 *		during JTAG authentication.
 *		- XLOADER_ERR_AUTH_JTAG_HASH_CALCULATION_FAIL if failed to calculate
 *		hash.
 *		- XLOADER_ERR_AUTH_JTAG_SIGN_VERIFY_FAIL if failed to verify
 *		signature.
 *
 ******************************************************************************/
static int XLoader_AuthJtagPpkOnly(u32 *TimeOut)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	int ClearStatus = XST_FAILURE;
	volatile u32 AuthJtagDis = 0U;
	volatile u32 AuthJtagDisTmp = 0U;
	u32 RevokeId = 0U;
	XLoader_SecureParams SecureParams = {0U};
	XSecure_Sha3Hash Sha3Hash = {0U};
	XSecure_Sha *ShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);
	u32 ReadAuthReg;
	volatile u8 UseDna;
	volatile u8 UseDnaTmp;
	u32 SecureStateAHWRoT = XLoader_GetAHWRoT(NULL);

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
		Status = XPlmi_UpdateStatus(XLOADER_ERR_AUTH_JTAG_DMA_XFR, 0U);
		goto END;
	}


	/** - Check efuse bits for secure debug disable */
	AuthJtagDis = XPlmi_In32(XLOADER_EFUSE_CACHE_SECURITY_CONTROL_OFFSET) &
					XLOADER_AUTH_JTAG_DIS_MASK;
	AuthJtagDisTmp = XPlmi_In32(XLOADER_EFUSE_CACHE_SECURITY_CONTROL_OFFSET) &
					XLOADER_AUTH_JTAG_DIS_MASK;
	if ((AuthJtagDis == XLOADER_AUTH_JTAG_DIS_MASK) ||
		(AuthJtagDisTmp == XLOADER_AUTH_JTAG_DIS_MASK)) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_AUTH_JTAG_DISABLED, 0U);
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
			Status = XPlmi_UpdateStatus(XLOADER_ERR_GLITCH_DETECTED, 0U);
		}
		else {
			Status = XPlmi_UpdateStatus(
				XLOADER_ERR_AUTH_JTAG_EFUSE_AUTH_COMPULSORY, 0U);
		}
		goto END;
	}

	SecureParams.PmcDmaInstPtr = XPlmi_GetDmaInstance(PMCDMA_0_DEVICE);
	if (SecureParams.PmcDmaInstPtr == NULL) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_AUTH_JTAG_GET_DMA, 0U);
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

	Status = XSecure_ShaFinish(ShaInstPtr, (u64)(UINTPTR)&Sha3Hash, XLOADER_SHA3_LEN);
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
					XLOADER_ERR_AUTH_JTAG_INVALID_DNA, 0U);
				goto END;
			}
		}
		Status = XLoader_EnableJtag((u32)XLOADER_CONFIG_DAP_STATE_ALL_DBG);
		*TimeOut = SecureParams.AuthJtagMessagePtr->JtagEnableTimeout;
	}

END:
	ClearStatus = XPlmi_MemSetBytes(&Sha3Hash, XLOADER_SHA3_LEN, 0U,
		XLOADER_SHA3_LEN);
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
 *		- XLOADER_ERR_AUTH_JTAG_DMA_XFR if failed to get authenticated JTAG
 *		data with DMA transfer.
 *		- XLOADER_ERR_AUTH_JTAG_DISABLED if JTAG authentication disable
 *		bit is set in efuse.
 *		- XLOADER_ERR_GLITCH_DETECTED if glitch is detected.
 *		- XLOADER_ERR_AUTH_JTAG_EFUSE_AUTH_COMPULSORY if PPK is not
 *		programmed JTAG authentication fails.
 *		- XLOADER_ERR_AUTH_JTAG_GET_DMA if failed to get DMA instance for
 *		JTAG authentication.
 *		- XLOADER_ERR_AUTH_JTAG_PPK_VERIFY_FAIL if failed to verify PPK,
 *		during JTAG authentication.
 *		- XLOADER_ERR_AUTH_JTAG_SPK_REVOKED if revoke ID is programmed,
 *		during JTAG authentication.
 *		- XLOADER_ERR_AUTH_JTAG_HASH_CALCULATION_FAIL if failed to calculate
 *		hash.
 *		- XLOADER_ERR_AUTH_JTAG_SIGN_VERIFY_FAIL if failed to verify
 *		signature.
 *
 ******************************************************************************/
static int XLoader_AuthJtagPpkNSpk(u32 *TimeOut)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	int ClearStatus = XST_FAILURE;
	volatile u32 AuthJtagDis = 0U;
	volatile u32 AuthJtagDisTmp = 0U;
	u32 RevokeId = 0U;
	static XLoader_SecureParams SecureParams = {0U};
	XSecure_Sha *ShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);
	u32 ReadAuthReg;
	volatile u8 UseDna;
	volatile u8 UseDnaTmp;
	u32 SecureStateAHWRoT = XLoader_GetAHWRoT(NULL);
	u32 MsgLenInBytes = 0U;
	u32 IdWord;
	u8 SpkHash[XLOADER_SHA3_LEN];
	u8 Sha3Hash[XLOADER_SHA3_LEN];
	u32 AuthType;
	u32 CopyLen;
	u32 MsgLenInWords;
	u32 RemainingWords;
	XLoader_AuthJtagData KeyData;
	u32* CurrPtr;

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
		Status = XST_FAILURE;
		goto END;
	}

	MsgLenInBytes = XPlmi_In32(XLOADER_PMC_TAP_AUTH_JTAG_DATA_OFFSET);

	/** - Store initial values */
	SecureParams.AuthJtagMessagePtr->IdWord = IdWord;
	SecureParams.AuthJtagMessagePtr->AuthJtagMessageLen = MsgLenInBytes;

	/**
	 * - Total words to copy includes IdWord and Length which have been already read
	 *   Remaining words to copy is total words minus the 2 words already read
	 */
	MsgLenInWords = MsgLenInBytes / 4U;
	RemainingWords = MsgLenInWords - 2U;

	/**
	 * - Set current pointer to start after IdWord and Length
	 */
	CurrPtr = (u32*)SecureParams.AuthJtagMessagePtr;
	CurrPtr += 2U;

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
		Status = (int)Xil_WaitForEvent((UINTPTR)XLOADER_PMC_TAP_AUTH_JTAG_INT_STATUS_OFFSET,
			XLOADER_PMC_TAP_AUTH_JTAG_INT_STATUS_MASK, XLOADER_PMC_TAP_AUTH_JTAG_INT_STATUS_MASK,
			XLOADER_AUTH_JTAG_INT_STATUS_WAIT_TIMEOUT);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		/**
		 * - Determine the number of words to copy in this iteration
		 *   If remaining data is larger than PMC TAP Authenticated JTAG data buffer,
		 *   copy maximum chunk size minus 2 words (IdWord + Length overhead)
		 *   Otherwise, copy all remaining words
		 */
		if (RemainingWords >= XLOADER_AUTH_JTAG_DATA_LEN_IN_WORDS) {
			CopyLen = XLOADER_AUTH_JTAG_DATA_LEN_IN_WORDS - 2U;
		}
		else {
			CopyLen = RemainingWords;
		}

		/** - Perform DMA transfer from PMC TAP data registers to memory buffer */
		Status = XPlmi_DmaXfr(XLOADER_PMC_TAP_AUTH_JTAG_DATA_OFFSET, (u64)(UINTPTR)CurrPtr, CopyLen,
			XPLMI_PMCDMA_0);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_AUTH_JTAG_DMA_XFR, 0U);
			goto END;
		}

		CurrPtr += CopyLen;
		RemainingWords -= CopyLen;

		/** - Clear Auth Jtag Interrupt Status register in PMC TAP module */
		XPlmi_Out32(XLOADER_PMC_TAP_AUTH_JTAG_INT_STATUS_OFFSET,
			XLOADER_PMC_TAP_AUTH_JTAG_INT_STATUS_MASK);
	}

	/** - Check efuse bits for secure debug disable */
	AuthJtagDis = XPlmi_In32(XLOADER_EFUSE_CACHE_SECURITY_CONTROL_OFFSET) &
					XLOADER_AUTH_JTAG_DIS_MASK;
	AuthJtagDisTmp = XPlmi_In32(XLOADER_EFUSE_CACHE_SECURITY_CONTROL_OFFSET) &
					XLOADER_AUTH_JTAG_DIS_MASK;
	if ((AuthJtagDis == XLOADER_AUTH_JTAG_DIS_MASK) ||
		(AuthJtagDisTmp == XLOADER_AUTH_JTAG_DIS_MASK)) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_AUTH_JTAG_DISABLED, 0U);
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
			Status = XPlmi_UpdateStatus(XLOADER_ERR_GLITCH_DETECTED, 0U);
		}
		else {
			Status = XPlmi_UpdateStatus(
				XLOADER_ERR_AUTH_JTAG_EFUSE_AUTH_COMPULSORY, 0U);
		}
		goto END;
	}

	SecureParams.PmcDmaInstPtr = XPlmi_GetDmaInstance(PMCDMA_0_DEVICE);
	if (SecureParams.PmcDmaInstPtr == NULL) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_AUTH_JTAG_GET_DMA, 0U);
		goto END;
	}

	/** - Run KAT for SHA3 engine */
	XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XLOADER_ERR_KAT_FAILED, Status, StatusTmp, XSecure_Sha3Kat, ShaInstPtr);
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_KAT_FAILED, Status);
		goto END;
	}

	KeyData.PpkData = (u32*)SecureParams.AuthJtagMessagePtr->AuthJtagData;
	KeyData.SpkHeader = (XLoader_SpkHeader *)(KeyData.PpkData + (SecureParams.AuthJtagMessagePtr->TotalPpkSize / 4U));
	KeyData.SpkData = (u32*)((u32 *)(KeyData.SpkHeader) + (XLOADER_SPK_HEADER_SIZE / 4U));
	KeyData.SPKSignature = (u32*)(KeyData.SpkData + (KeyData.SpkHeader->TotalSPKSize / 4U));
	KeyData.EnableJtagSignature = (u32*)(KeyData.SPKSignature + (KeyData.SpkHeader->TotalSignatureSize / 4U));

	/** - Verify PPK in the authenticated JTAG data. */
	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_PpkVerify, &SecureParams, SecureParams.AuthJtagMessagePtr->ActualPpkSize);
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_AUTH_JTAG_PPK_VERIFY_FAIL,
			Status);
		goto END;
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

	/** - Run KAT before verifying AUTH JTAG message */
	Status = XST_FAILURE;
	Status = XLoader_AuthKat(&SecureParams);
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
	AuthType = XLoader_GetAuthPubAlgo(&SecureParams.AuthJtagMessagePtr->AuthHdr);
	if ((AuthType == XLOADER_PUB_STRENGTH_RSA_4096) ||
		(AuthType == XLOADER_PUB_STRENGTH_ECDSA_P384) ||
		(AuthType == XLOADER_PUB_STRENGTH_ECDSA_P521)) {
		XSECURE_TEMPORAL_CHECK(END, Status, XLoader_VerifySignature, &SecureParams,
			(u8 *)&SpkHash, (XLoader_RsaKey *)KeyData.PpkData, (u8*)KeyData.SPKSignature);
	}

	/** - Calculate hash for Authenticated JTAG signature */
	Status = XLoader_ShaDigestCalculation((u8 *)&SecureParams.AuthJtagMessagePtr->IdWord,
			(MsgLenInBytes - SecureParams.AuthJtagMessagePtr->ActualAuthJtagSignSize),
			&Sha3Hash[0]);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SPK_HASH_CALC_FAIL, Status);
		goto END;
	}

	/** - Verify signature of Auth Jtag data */
	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_VerifySignature,
		&SecureParams, (u8 *)&Sha3Hash, (XLoader_RsaKey *)KeyData.SpkData, (u8*)KeyData.EnableJtagSignature);
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
					XLOADER_ERR_AUTH_JTAG_INVALID_DNA, 0U);
				goto END;
			}
		}
		Status = XLoader_EnableJtag((u32)XLOADER_CONFIG_DAP_STATE_ALL_DBG);
		*TimeOut = SecureParams.AuthJtagMessagePtr->JtagEnableTimeout;
	}

END:
	/** - Clear Auth Jtag Interrupt Status register in PMC TAP module */
	XPlmi_Out32(XLOADER_PMC_TAP_AUTH_JTAG_INT_STATUS_OFFSET,
		XLOADER_PMC_TAP_AUTH_JTAG_INT_STATUS_MASK);


	ClearStatus = XPlmi_MemSetBytes(&Sha3Hash, XLOADER_SHA3_LEN, 0U,
			XLOADER_SHA3_LEN);
	if (ClearStatus != XST_SUCCESS) {
		Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_ERR);
	}
	return Status;
}

#endif /**< PLM_AUTH_JTAG_PPK_SPK */
#endif	/**< PLM_AUTH_JTAG */
