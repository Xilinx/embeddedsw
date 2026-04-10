/**************************************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xrsa.c
 *
 * This file contains implementation of the interface functions for RSA hardware engine.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   07/11/24 Initial release
 *       ss   08/20/24 Added 64-bit address support
 *       yog  08/25/24 Integrated FIH library
 *       ss   09/26/24 Fixed doxygen comments
 * 1.1   am   05/18/25 Fixed implicit conversion of operands
 *       kd   07/23/25 Fixed gcc warnings
 *       yog  01/28/26 Added RSA key pair generation API.
 *       ss   03/18/26 Added scheduler-based RSA key pair generation.
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xrsa_server_apis RSA Server APIs
* @{
*/
/*************************************** Include Files *******************************************/
#include "xrsa.h"
#include "xasu_rsainfo.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "Rsa.h"
#include "xil_util.h"
#include "xfih.h"
#include "xasu_generic.h"
#include "xtask.h"
#include "xkeymanager.h"
#include "xasu_def.h"
#include "xasufw_perf.h"

/************************************ Constant Definitions ***************************************/
/* Definitions for peripheral RSA */
#define XASU_RSA_BASEADDR		(0xEBF50000U)	/**< RSA base address */
#define XRSA_RESET_REG_OFFSET		(0x40U)		/**< RSA reset register offset */

/* Errors from third-party library */
#define XRSA_KEY_GEN_STEP_WAIT_STATE		(1)		/**< RSA key generation step needs
								more iterations (WAIT state) */
#define XRSA_KEY_PAIR_COMP_ERROR	(1)		/**< RSA third-party key pair compare error */
#define XRSA_RAND_GEN_ERROR		(2)		/**< RSA third-party random number
								generation error */
#define XRSA_MAX_PRIME_SIZE_IN_BYTES	(256U)		/**< RSA max prime size in bytes */
#define XRSA_PUBEXP_SIZE_IN_BYTES	(4U)		/**< RSA public exponent size in bytes */

#define XRSA_HALF_LEN(x)		((x) >> 1U)	/**< Calculate half value */
#define XRSA_BYTE_TO_BIT(x)		(((u32)(x) << 3U)) /**< Byte to bit conversion */

#define XRSA_PUB_EXP_INVALID_ZERO_VALUE		(0U)	/**< Indicates invalid public exponent
														value of zero */
#define XRSA_PUB_EXP_INVALID_ONE_VALUE		(1U)	/**< Indicates invalid public exponent
														value of one */
#define XRSA_PUB_EXP_INVALID_THREE_VALUE	(3U)	/**< Indicates invalid public exponent
														value of three */

#ifdef XASU_KEYMANAGER_ENABLE
#define XRSA_KEY_GEN_POLL_INTERVAL	(100U)	/**< Key pair generation scheduler task poll interval in ms */
#define XRSA_KEY_GEN_TASK_PRIORITY	(15U)	/**< Key generation scheduler task priority (lowest) */

#define XRSA_PUB_EXP_VALUE	(65537U) /**< RSA public exponent value. */

#define XRSA_KEY_GEN_VAULT_CAPACITY (2U) /**< Number of RSA key pairs that can be stored in ASU vault */

#define XRSA_PWCT_TEST_MSG_VALUE	(2U)	/**< PWCT test message value (must be > 0 and < modulus) */

#define XRSA_BORROW_PROPAGATION_FILL_VALUE	(0xFFU)	/**< Fill value during borrow propagation in subtraction */

/************************************** Type Definitions *****************************************/
typedef RsaKeyPair XRsa_KeyPtr;

/** State machine for scheduler-based RSA key pair generation */
typedef enum {
	XRSA_KEY_GEN_DEFAULT_STATE = 0U,	/**< Check vault key count */
	XRSA_KEY_GEN_INIT_STATE,		/**< Initialize key generation */
	XRSA_KEY_GEN_STEP_STATE,		/**< Incremental generation step */
	XRSA_KEY_GEN_READY_STATE,		/**< Finalize and store in vault */
} XRsa_KeyGenState;
#endif /* XASU_KEYMANAGER_ENABLE */

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XRsa_UpdateStatus(s32 Status);
static s32 XRsa_ValidatePubExp(u8 *BuffAddr);
static s32 XRsa_ValidateModulusNdInputdata(u8 *BuffAddr, u8 *InputData, u32 Len, u8 *ScratchBuf);
#ifdef XASU_KEYMANAGER_ENABLE
static s32 XRsa_GenerateKeyPairTask(void *Arg);
#endif /* XASU_KEYMANAGER_ENABLE */

/************************************ Variable Definitions ***************************************/
#ifdef XASU_KEYMANAGER_ENABLE
/** Buffer for key generation (separate from shared data block) */
static u8 RsaKeyGenBuf[XRSA_MAX_KEY_OBJ_SIZE_IN_BYTES];

/** Public exponent buffer for background key generation (must be full-key-size). */
static u32 RsaKeyGenPubExpBuf[XRSA_MAX_KEY_SIZE_IN_BYTES / sizeof(u32)];
#endif /* XASU_KEYMANAGER_ENABLE */

/*************************************************************************************************/
/**
 * @brief	This function performs RSA decryption using CRT algorithm for the provided
 * 		message by using private key.
 *
 * @param	DmaPtr		Pointer to the AsuDma instance.
 * @param	RsaParamsPtr	Pointer to the RSA parameters structure containing input/output
 * 				data addresses, length and output data length.
 * @param	KeyParamAddr	Address to all the parameters required for private decrypt
 * 				operation using CRT algorithm.
 * @param	OutDataLenPtr	Pointer to the output data length to be updated with actual
 * 				output length.
 *
 * @return
 *		- XASUFW_SUCCESS, if RSA decryption using CRT algorithm is successful.
 *		- XASUFW_FAILURE, if RSA decryption using CRT algorithm fails.
 *		- XASUFW_RSA_INVALID_PARAM, if input parameter validation fails.
 *		- XASUFW_ZEROIZE_MEMSET_FAIL, if zeroize memset fails.
 *		- XASUFW_DMA_COPY_FAIL, if DMA copy fails.
 *		- XASUFW_RSA_CHANGE_ENDIANNESS_ERROR, if endianness change error occurs.
 *		- XASUFW_MEM_COPY_FAIL, if memory copy fails.
 * 		- Also, this function can return termination error codes from 0x9CU to 0x9FU
 * 		please refer to xasufw_status.h.
 *
 *************************************************************************************************/
s32 XRsa_CrtOp(XAsufw_Dma *DmaPtr, const XAsu_RsaParams *RsaParamsPtr, u64 KeyParamAddr,
	       u32 *OutDataLenPtr)
{
	/**
	 * Capture the start time of the RSA CRT operation, if performance measurement is
	 * enabled.
	 */
	XASUFW_MEASURE_PERF_START(XASU_MODULE_RSA_ID);

	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(SStatus, XASUFW_FAILURE);
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	u8 *InData = XRsa_GetDataBlockAddr();
	XAsu_RsaCrtKeyComp *KeyPtr = (XAsu_RsaCrtKeyComp *)(InData + XRSA_MAX_KEY_SIZE_IN_BYTES);
	u8 *PubExpoArr = (u8 *)KeyPtr + sizeof(XAsu_RsaCrtKeyComp);
	u8 *OutData = PubExpoArr + XRSA_MAX_KEY_SIZE_IN_BYTES;

	/** Validate the input arguments. */
	if ((RsaParamsPtr == NULL) || (KeyParamAddr == 0U) ||
		(DmaPtr == NULL) || (OutDataLenPtr == NULL)) {
			Status = XASUFW_RSA_INVALID_PARAM;
			goto END;
	}

	if ((RsaParamsPtr->InputDataAddr == 0U) || (RsaParamsPtr->OutputDataAddr == 0U)) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	if (RsaParamsPtr->Len != RsaParamsPtr->KeySize) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	if (RsaParamsPtr->OutputDataLen < RsaParamsPtr->KeySize) {
		Status = XASUFW_RSA_INVALID_OUTPUT_BUF_LEN;
		goto END;
	}

	Status = XAsu_RsaValidateKeySize(RsaParamsPtr->KeySize);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	/** Release the RSA engine from reset. */
	XAsufw_CryptoCoreReleaseReset(XASU_RSA_BASEADDR, XRSA_RESET_REG_OFFSET);

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemSet(PubExpoArr, XRSA_MAX_KEY_SIZE_IN_BYTES, 0U,
			     XRSA_MAX_KEY_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ZEROIZE_MEMSET_FAIL;
		goto END;
	}

	/** Copy the input data to ASU memory using DMA. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, RsaParamsPtr->InputDataAddr, (u64)(UINTPTR)InData,
			       RsaParamsPtr->Len, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_DMA_COPY_FAIL;
		goto END;
	}

	/** Copy key parameters to ASU memory using DMA. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, KeyParamAddr, (u64)(UINTPTR)KeyPtr,
			       sizeof(XAsu_RsaCrtKeyComp), 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_DMA_COPY_FAIL;
		goto END;
	}

	/** Copy public exponent to pointer. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCpy(PubExpoArr, XRSA_MAX_KEY_SIZE_IN_BYTES, &(KeyPtr->PubKeyComp.PubExp),
			     XRSA_PUBEXP_SIZE_IN_BYTES, XRSA_PUBEXP_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_ValidatePubExp(PubExpoArr);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_PUB_EXP_INVALID_VALUE;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	SStatus = XRsa_ValidateModulusNdInputdata((u8 *)KeyPtr->PubKeyComp.Modulus, InData,
			RsaParamsPtr->KeySize, OutData);
	if (SStatus != XASUFW_SUCCESS) {
		Status = SStatus;
		goto END;
	}

	/**
	 * Endianness change from BE to LE for the following components
	 * - Input Data.
	 * - Public exponent.
	 * - Modulus.
	 * - first prime number.
	 * - second prime number.
	 * - derived value of first prime number.
	 * - derived value of second prime number.
	 * - Inverse of derived value of second prime number.
	 */

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness(InData, RsaParamsPtr->KeySize);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness(PubExpoArr, XRSA_PUBEXP_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness((u8 *)KeyPtr->PubKeyComp.Modulus, RsaParamsPtr->KeySize);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness((u8 *)KeyPtr->Prime1, XRSA_HALF_LEN(RsaParamsPtr->KeySize));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness((u8 *)KeyPtr->Prime2, XRSA_HALF_LEN(RsaParamsPtr->KeySize));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness((u8 *)KeyPtr->DP, XRSA_HALF_LEN(RsaParamsPtr->KeySize));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness((u8 *)KeyPtr->DQ, XRSA_HALF_LEN(RsaParamsPtr->KeySize));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness((u8 *)KeyPtr->QInv, XRSA_HALF_LEN(RsaParamsPtr->KeySize));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	/** Perform private decryption operation using CRT algorithm. */
	XFIH_CALL(RSA_ExpCrtQ, XFihVar, Status, InData, (u8 *)KeyPtr->Prime1, (u8 *)KeyPtr->Prime2,
		  (u8 *)KeyPtr->DP, (u8 *)KeyPtr->DQ, (u8 *)KeyPtr->QInv, PubExpoArr,
		  (u8 *)KeyPtr->PubKeyComp.Modulus, (s32)XRSA_BYTE_TO_BIT(RsaParamsPtr->KeySize), OutData);
	if (Status != XASUFW_SUCCESS) {
		Status = XRsa_UpdateStatus(Status);
		goto END;
	}

	/** Endianness change from LE to BE for output data. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness(OutData, RsaParamsPtr->KeySize);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	/** Copy output data to user memory using DMA. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)OutData, RsaParamsPtr->OutputDataAddr,
			       RsaParamsPtr->Len, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_DMA_COPY_FAIL;
		goto END;
	}

	/** Update actual output data length. */
	*OutDataLenPtr = RsaParamsPtr->KeySize;

	/**
	 * Measure and print the performance time for the RSA CRT operation, if performance
	 * measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_STOP(XASU_MODULE_RSA_ID);

END:
	/** Zeroize local copy of all the parameters. */
	ASSIGN_VOLATILE(SStatus, XASUFW_FAILURE);
	XFIH_CALL(Xil_SecureZeroize, XFihVar, SStatus, InData,
					XRSA_MAX_KEY_SIZE_IN_BYTES * XRSA_TOTAL_PARAMS);

	Status = XAsufw_UpdateBufStatus(Status, SStatus);

	/** Set the RSA engine under reset. */
	XAsufw_CryptoCoreSetReset(XASU_RSA_BASEADDR, XRSA_RESET_REG_OFFSET);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs RSA decryption for the provided message by using private key.
 *
 * @param	DmaPtr		Pointer to the AsuDma instance.
 * @param	RsaParamsPtr	Pointer to the RSA parameters structure containing input/output
 * 				data addresses, length, exponent address and output data length.
 * @param	KeyParamAddr	Address to the parameters required for RSA operation.
 * @param	OutDataLenPtr	Pointer to the output data length to be updated with actual
 * 				output length.
 *
 * @return
 *		- XASUFW_SUCCESS, if RSA decryption is successful.
 *		- XASUFW_FAILURE, if RSA decryption fails.
 *		- XASUFW_RSA_INVALID_PARAM, if input parameter validation fails.
 *		- XASUFW_ZEROIZE_MEMSET_FAIL, if zeroize memset fails.
 *		- XASUFW_DMA_COPY_FAIL, if DMA copy fails.
 *		- XASUFW_RSA_CHANGE_ENDIANNESS_ERROR, if endianness change error occurs.
 *		- XASUFW_MEM_COPY_FAIL, if memory copy fails.
 *		- XASUFW_RSA_INVALID_PRIME_TOT_FLAG, if invalid prime/totient flag.
 *		- Also, this function can return termination error codes from 0x9CU to 0x9EU and 0xA0U,
 * 		please refer to xasufw_status.h.
 *
 *************************************************************************************************/
s32 XRsa_PvtExp(XAsufw_Dma *DmaPtr, const XAsu_RsaParams *RsaParamsPtr, u64 KeyParamAddr,
		u32 *OutDataLenPtr)
{
	/**
	 * Capture the start time of the RSA private exponent operation, if performance measurement
	 * is enabled.
	 */
	XASUFW_MEASURE_PERF_START(XASU_MODULE_RSA_ID);

	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(SStatus, XASUFW_FAILURE);
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	u8 *InData = XRsa_GetDataBlockAddr();
	XAsu_RsaPvtKeyComp *KeyPtr = (XAsu_RsaPvtKeyComp *)(InData + XRSA_MAX_KEY_SIZE_IN_BYTES);
	u8 *RRN = (u8 *)KeyPtr + sizeof(XAsu_RsaPvtKeyComp);
	u8 *RN = RRN + XRSA_MAX_KEY_SIZE_IN_BYTES;
	u8 *PubExpoArr = RN + XRSA_MAX_KEY_SIZE_IN_BYTES;
	u8 *OutData = PubExpoArr + XRSA_MAX_KEY_SIZE_IN_BYTES;

	/** Validate the input arguments. */
	if ((RsaParamsPtr == NULL) || (KeyParamAddr == 0U) ||
		(DmaPtr == NULL) || (OutDataLenPtr == NULL)) {
			Status = XASUFW_RSA_INVALID_PARAM;
			goto END;
	}

	if ((RsaParamsPtr->InputDataAddr == 0U) || (RsaParamsPtr->OutputDataAddr == 0U)) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	if (RsaParamsPtr->Len != RsaParamsPtr->KeySize) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	if (RsaParamsPtr->OutputDataLen < RsaParamsPtr->KeySize) {
		Status = XASUFW_RSA_INVALID_OUTPUT_BUF_LEN;
		goto END;
	}

	Status = XAsu_RsaValidateKeySize(RsaParamsPtr->KeySize);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	/** Release the RSA engine from reset. */
	XAsufw_CryptoCoreReleaseReset(XASU_RSA_BASEADDR, XRSA_RESET_REG_OFFSET);

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemSet(PubExpoArr, XRSA_MAX_KEY_SIZE_IN_BYTES, 0U,
			     XRSA_MAX_KEY_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ZEROIZE_MEMSET_FAIL;
		goto END;
	}

	/** Copy the input data to ASU memory using DMA. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, RsaParamsPtr->InputDataAddr, (u64)(UINTPTR)InData,
			       RsaParamsPtr->Len, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_DMA_COPY_FAIL;
		goto END;
	}

	/** If key parameters are already in RSA reserved memory, no need to copy. */
	if (KeyParamAddr != (u64)(UINTPTR)KeyPtr) {
		/** Else, copy key parameters to RSA reserved memory using DMA. */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_DmaXfr(DmaPtr, KeyParamAddr, (u64)(UINTPTR)KeyPtr,
				       sizeof(XAsu_RsaPvtKeyComp), 0U);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_DMA_COPY_FAIL;
			goto END;
		}
	}

	/** Copy public exponent to pointer. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCpy(PubExpoArr, XRSA_MAX_KEY_SIZE_IN_BYTES, &(KeyPtr->PubKeyComp.PubExp),
			     XRSA_PUBEXP_SIZE_IN_BYTES, XRSA_PUBEXP_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_ValidatePubExp(PubExpoArr);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_PUB_EXP_INVALID_VALUE;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	SStatus = XRsa_ValidateModulusNdInputdata((u8 *)KeyPtr->PubKeyComp.Modulus, InData,
			RsaParamsPtr->KeySize, OutData);
	if (SStatus != XASUFW_SUCCESS) {
		Status = SStatus;
		goto END;
	}


	/**
	 * Endianness change from BE to LE for the following components
	 * - Input Data.
	 * - Public exponent.
	 * - Modulus.
	 * - Private exponent.
	 * - Prime number or totient.
	 * - Pre calculated exponent values if available (R mod N,R square mod N).
	 */

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness(InData, RsaParamsPtr->KeySize);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness(PubExpoArr, XRSA_PUBEXP_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness((u8 *)KeyPtr->PubKeyComp.Modulus, RsaParamsPtr->KeySize);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness((u8 *)KeyPtr->PvtExp, RsaParamsPtr->KeySize);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if (KeyPtr->PrimeCompOrTotientPrsnt == XRSA_TOTIENT_IS_PRSNT) {
		Status = XAsufw_ChangeEndianness((u8 *)KeyPtr->PrimeCompOrTotient, RsaParamsPtr->KeySize);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
			goto END;
		}
	} else if (KeyPtr->PrimeCompOrTotientPrsnt == XRSA_PRIME_NUM_IS_PRSNT) {
		Status = XAsufw_ChangeEndianness((u8 *)KeyPtr->PrimeCompOrTotient,
						 XRSA_HALF_LEN(RsaParamsPtr->KeySize));
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
			goto END;
		}
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_ChangeEndianness((u8 *)KeyPtr->PrimeCompOrTotient +
						 XRSA_MAX_PRIME_SIZE_IN_BYTES,
						 XRSA_HALF_LEN(RsaParamsPtr->KeySize));
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
			goto END;
		}
	} else if (KeyPtr->PrimeCompOrTotientPrsnt == XRSA_NO_PRIME_NO_TOT_PRSNT) {
		/* This is a valid scenario and does not return any error. */
	} else {
		Status = XASUFW_RSA_INVALID_PRIME_TOT_FLAG;
		goto END;
	}

	/**
	 * Perform private exponentiation operation by calculating exponentiation values or with
	 * pre calculated exponentiation values and with totient or prime numbers or without
	 * totient and prime numbers based on available parameters.
	 */
	if (RsaParamsPtr->ExpoCompAddr == 0U) {
		if (KeyPtr->PrimeCompOrTotientPrsnt == XRSA_TOTIENT_IS_PRSNT) {
			XFIH_CALL(RSA_ExpQ, XFihVar, Status, InData, (u8 *)KeyPtr->PvtExp,
				  (u8 *)KeyPtr->PubKeyComp.Modulus, NULL, NULL,
				  PubExpoArr, (u8 *)KeyPtr->PrimeCompOrTotient,
				  (s32)XRSA_BYTE_TO_BIT(RsaParamsPtr->KeySize), OutData);
		} else if (KeyPtr->PrimeCompOrTotientPrsnt == XRSA_PRIME_NUM_IS_PRSNT) {
			XFIH_CALL(RSA_ExpQ, XFihVar, Status, InData, (u8 *)KeyPtr->PvtExp,
				  (u8 *)KeyPtr->PubKeyComp.Modulus,
				  (u8 *)KeyPtr->PrimeCompOrTotient,
				  (u8 *)KeyPtr->PrimeCompOrTotient +
				  XRSA_MAX_PRIME_SIZE_IN_BYTES, PubExpoArr, NULL,
				  (s32)XRSA_BYTE_TO_BIT(RsaParamsPtr->KeySize), OutData);
		} else {
			XFIH_CALL(RSA_ExpQ, XFihVar, Status, InData, (u8 *)KeyPtr->PvtExp,
				  (u8 *)KeyPtr->PubKeyComp.Modulus, NULL, NULL, PubExpoArr,
				  NULL, (s32)XRSA_BYTE_TO_BIT(RsaParamsPtr->KeySize), OutData);
		}
	} else {
		/* DMA transfer of pre-calculated modulus values from client address to ASU
		memory if available */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_DmaXfr(DmaPtr, RsaParamsPtr->ExpoCompAddr, (u64)(UINTPTR)RRN,
				       sizeof(XAsu_RsaRModN), 0U);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_DMA_COPY_FAIL;
			goto END;
		}
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_ChangeEndianness(RRN, RsaParamsPtr->KeySize);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
			goto END;
		}
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_ChangeEndianness(RN, RsaParamsPtr->KeySize);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
			goto END;
		}
		if (KeyPtr->PrimeCompOrTotientPrsnt == XRSA_TOTIENT_IS_PRSNT) {
			XFIH_CALL(RSA_ExpoptQ, XFihVar, Status, InData, (u8 *)KeyPtr->PvtExp,
				  (u8 *)KeyPtr->PubKeyComp.Modulus, RN, RRN, NULL, NULL,
				  PubExpoArr, (u8 *)KeyPtr->PrimeCompOrTotient,
				  (s32)XRSA_BYTE_TO_BIT(RsaParamsPtr->KeySize), OutData);
		} else if (KeyPtr->PrimeCompOrTotientPrsnt == XRSA_PRIME_NUM_IS_PRSNT) {
			XFIH_CALL(RSA_ExpoptQ, XFihVar, Status, InData, (u8 *)KeyPtr->PvtExp,
				  (u8 *)KeyPtr->PubKeyComp.Modulus, RN, RRN,
				  (u8 *)KeyPtr->PrimeCompOrTotient,
				  (u8 *)KeyPtr->PrimeCompOrTotient +
				  XRSA_MAX_PRIME_SIZE_IN_BYTES, PubExpoArr, NULL,
				  (s32)XRSA_BYTE_TO_BIT(RsaParamsPtr->KeySize), OutData);
		} else {
			XFIH_CALL(RSA_ExpoptQ, XFihVar, Status, InData, (u8 *)KeyPtr->PvtExp,
				  (u8 *)KeyPtr->PubKeyComp.Modulus, RN, RRN,
				  NULL, NULL, PubExpoArr, NULL,
				  (s32)XRSA_BYTE_TO_BIT(RsaParamsPtr->KeySize), OutData);
		}

	}
	if (Status != XASUFW_SUCCESS) {
		Status = XRsa_UpdateStatus(Status);
		goto END;
	}

	/** Endianness change from LE to BE for output data. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness(OutData, RsaParamsPtr->KeySize);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	/** Copy output data to the user memory using DMA. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)OutData, RsaParamsPtr->OutputDataAddr,
			       RsaParamsPtr->Len, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_DMA_COPY_FAIL;
		goto END;
	}

	/** Update actual output data length. */
	*OutDataLenPtr = RsaParamsPtr->KeySize;

	ReturnStatus = XASUFW_RSA_DECRYPTION_SUCCESS;

	/**
	 * Measure and print the performance time for the RSA private exponent operation, if
	 * performance measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_STOP(XASU_MODULE_RSA_ID);

END:
	/** Zeroize local copy of all the parameters. */
	ASSIGN_VOLATILE(SStatus, XASUFW_FAILURE);
	XFIH_CALL(Xil_SecureZeroize, XFihVar, SStatus, InData,
					XRSA_MAX_KEY_SIZE_IN_BYTES * XRSA_TOTAL_PARAMS);

	Status = XAsufw_UpdateBufStatus(Status, SStatus);

	/** Set the RSA engine under reset. */
	XAsufw_CryptoCoreSetReset(XASU_RSA_BASEADDR, XRSA_RESET_REG_OFFSET);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs RSA encryption for the provided message by using public key.
 *
 * @param	DmaPtr		Pointer to the AsuDma instance.
 * @param	RsaParamsPtr	Pointer to the RSA parameters structure containing input/output
 * 				data addresses, length, exponent address and output data length.
 * @param	KeyParamAddr	Address to the parameters required for RSA operation.
 * @param	OutDataLenPtr	Pointer to the output data length to be updated with actual
 * 				output length.
 *
 * @return
 *		- XASUFW_SUCCESS, if RSA encryption is successful.
 *		- XASUFW_FAILURE, if RSA encryption fails.
 *		- XASUFW_RSA_INVALID_PARAM, if input parameter validation fails.
 *		- XASUFW_ZEROIZE_MEMSET_FAIL, if zeroize memset fails.
 *		- XASUFW_DMA_COPY_FAIL, if DMA copy fails.
 *		- XASUFW_RSA_CHANGE_ENDIANNESS_ERROR, if endianness change error occurs.
 *		- XASUFW_MEM_COPY_FAIL, if memory copy fails.
 *		- Also, this function can return termination error codes from 0x9CU to 0x9EU and 0xA1U
 * 		please refer to xasufw_status.h.
 *
 *************************************************************************************************/
s32 XRsa_PubExp(XAsufw_Dma *DmaPtr, const XAsu_RsaParams *RsaParamsPtr, u64 KeyParamAddr,
		u32 *OutDataLenPtr)
{
	/**
	 * Capture the start time of the RSA public exponent operation, if performance measurement
	 * is enabled.
	 */
	XASUFW_MEASURE_PERF_START(XASU_MODULE_RSA_ID);

	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(SStatus, XASUFW_FAILURE);
	u8 *InData = XRsa_GetDataBlockAddr();
	XAsu_RsaPubKeyComp *KeyPtr = (XAsu_RsaPubKeyComp *)(InData + XRSA_MAX_KEY_SIZE_IN_BYTES);
	u8 *RRN = (u8 *)KeyPtr + sizeof(XAsu_RsaPubKeyComp);
	u8 *PubExpoArr = RRN + XRSA_MAX_KEY_SIZE_IN_BYTES;
	u8 *OutData = PubExpoArr + XRSA_MAX_KEY_SIZE_IN_BYTES;

	/** Validate the input arguments. */
	if ((RsaParamsPtr == NULL) || (KeyParamAddr == 0U) ||
		(DmaPtr == NULL) || (OutDataLenPtr == NULL)) {
			Status = XASUFW_RSA_INVALID_PARAM;
			goto END;
	}

	if ((RsaParamsPtr->InputDataAddr == 0U) || (RsaParamsPtr->OutputDataAddr == 0U)) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	if (RsaParamsPtr->Len > RsaParamsPtr->KeySize) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	if (RsaParamsPtr->OutputDataLen < RsaParamsPtr->KeySize) {
		Status = XASUFW_RSA_INVALID_OUTPUT_BUF_LEN;
		goto END;
	}

	Status = XAsu_RsaValidateKeySize(RsaParamsPtr->KeySize);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	/** Release the RSA engine from reset. */
	XAsufw_CryptoCoreReleaseReset(XASU_RSA_BASEADDR, XRSA_RESET_REG_OFFSET);

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemSet(InData, XRSA_MAX_KEY_SIZE_IN_BYTES, 0U,
			     XRSA_MAX_KEY_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ZEROIZE_MEMSET_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemSet(PubExpoArr, XRSA_MAX_KEY_SIZE_IN_BYTES, 0U,
			     XRSA_MAX_KEY_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ZEROIZE_MEMSET_FAIL;
		goto END;
	}

	/** Copy the input data to ASU memory using DMA. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, RsaParamsPtr->InputDataAddr,
			(u64)(UINTPTR)(InData + RsaParamsPtr->KeySize - RsaParamsPtr->Len),
				RsaParamsPtr->Len, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_DMA_COPY_FAIL;
		goto END;
	}

	/** Key parameters are already in RSA reserved memory, no need to copy. */
	if (KeyParamAddr != (u64)(UINTPTR)KeyPtr) {
		/** Copy key parameters to ASU memory using DMA if from different location. */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_DmaXfr(DmaPtr, KeyParamAddr, (u64)(UINTPTR)KeyPtr,
				       sizeof(XAsu_RsaPubKeyComp), 0U);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_DMA_COPY_FAIL;
			goto END;
		}
	}

	/** Copy public exponent to pointer. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCpy(PubExpoArr, XRSA_MAX_KEY_SIZE_IN_BYTES, &(KeyPtr->PubExp),
			     XRSA_PUBEXP_SIZE_IN_BYTES, XRSA_PUBEXP_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_ValidatePubExp(PubExpoArr);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_PUB_EXP_INVALID_VALUE;
		goto END;
	}

	/**
	 * Endianness change from BE to LE for the following components
	 * - Input Data.
	 * - Public exponent.
	 * - Modulus.
	 * - Pre calculated exponent value if available (R square mod N).
	 */

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness(InData, RsaParamsPtr->KeySize);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness(PubExpoArr, XRSA_PUBEXP_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness((u8 *)KeyPtr->Modulus, RsaParamsPtr->KeySize);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	/**
	 * Perform public exponentiation operation by calculating exponentiation values or with
	 * pre calculated exponentiation values based on available parameters.
	 */
	if (RsaParamsPtr->ExpoCompAddr == 0U) {
		rsaexp(InData, PubExpoArr, (u8 *)KeyPtr->Modulus,
		       (s32)XRSA_BYTE_TO_BIT(RsaParamsPtr->KeySize), OutData);
	} else {
		/* DMA transfer of pre-calculated modulus values from client address to ASU
			memory */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_DmaXfr(DmaPtr, RsaParamsPtr->ExpoCompAddr, (u64)(UINTPTR)RRN,
				       sizeof(XAsu_RsaRRModN), 0U);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_DMA_COPY_FAIL;
			goto END;
		}
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_ChangeEndianness(RRN, RsaParamsPtr->KeySize);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
			goto END;
		}
		rsaexpopt(InData, PubExpoArr, (u8 *)KeyPtr->Modulus, RRN,
			  (s32)XRSA_BYTE_TO_BIT(RsaParamsPtr->KeySize), OutData);
	}

	/** Endianness change from LE to BE for output data. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness(OutData, RsaParamsPtr->KeySize);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	/** Copy output data to user memory using DMA. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)OutData, RsaParamsPtr->OutputDataAddr,
			       RsaParamsPtr->KeySize, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_DMA_COPY_FAIL;
		goto END;
	}

	/** Update actual output data length. */
	*OutDataLenPtr = RsaParamsPtr->KeySize;

	/**
	 * Measure and print the performance time for the RSA public exponent operation, if
	 * performance measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_STOP(XASU_MODULE_RSA_ID);

END:
	/** Zeroize local copy of all the parameters. */
	SStatus = XAsufw_SMemSet(InData, XRSA_MAX_KEY_SIZE_IN_BYTES * XRSA_TOTAL_PARAMS);

	Status = XAsufw_UpdateBufStatus(Status, SStatus);

	/** Set the RSA engine under reset. */
	XAsufw_CryptoCoreSetReset(XASU_RSA_BASEADDR, XRSA_RESET_REG_OFFSET);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is used to get RSA data block address.
 *
 * @return
 *		- Address of the data block.
 *
 *************************************************************************************************/
u8 *XRsa_GetDataBlockAddr(void)
{
	/* RSA data block memory allocated in ASU RAM. */
	static u8  Rsa_Data[XRSA_MAX_PARAM_SIZE_IN_BYTES] __attribute__ ((section (".rsa_data_block")));

	return Rsa_Data;
}

/*************************************************************************************************/
/**
 * @brief	This function maps the status returned from third-party library to the respective error
 * 		from xasufw_status.h.
 *
 * @param	Status	Status returned from third-party library.
 *
 * @return
 *		- XASUFW_RSA_RAND_GEN_ERROR, if random number generation fails.
 *		- XASUFW_RSA_KEY_PAIR_COMP_ERROR, if key pair comparison fails.
 *		- XASUFW_RSA_ERROR, if other errors occur.
 *
 *************************************************************************************************/
static s32 XRsa_UpdateStatus(s32 Status)
{
	CREATE_VOLATILE(SStatus, XASUFW_FAILURE);

	/* Updates status which relates to ASUFW_RSA module. */
	if (Status == XRSA_RAND_GEN_ERROR) {
		SStatus = XASUFW_RSA_RAND_GEN_ERROR;
	} else if (Status == XRSA_KEY_PAIR_COMP_ERROR) {
		SStatus = XASUFW_RSA_KEY_PAIR_COMP_ERROR;
	} else if (Status != XASUFW_SUCCESS) {
		SStatus = XASUFW_RSA_ERROR;
	}

	return SStatus;
}

/*************************************************************************************************/
/**
 * @brief	This function validates public exponent value.
 *
 * @param	BuffAddr	Buffer address of public exponent.
 *
 * @return
 *		- XASUFW_SUCCESS, if valid public exponent value is given.
 *		- XASUFW_FAILURE, if invalid public exponent value is given.
 *
 *************************************************************************************************/
static s32 XRsa_ValidatePubExp(u8 *BuffAddr)
{
	s32 Status = XASUFW_FAILURE;
	volatile u32 PubExpVal = 0U;

	PubExpVal = ((u32)BuffAddr[0U] << XASUFW_THREE_BYTE_SHIFT_VALUE) |
		((u32)BuffAddr[XASUFW_BUFFER_INDEX_ONE] << XASUFW_TWO_BYTE_SHIFT_VALUE) |
		((u32)BuffAddr[XASUFW_BUFFER_INDEX_TWO] << XASUFW_ONE_BYTE_SHIFT_VALUE) |
		((u32)BuffAddr[XASUFW_BUFFER_INDEX_THREE]);

	if ((PubExpVal != XRSA_PUB_EXP_INVALID_ZERO_VALUE) &&
	    (PubExpVal != XRSA_PUB_EXP_INVALID_ZERO_VALUE) &&
	    (PubExpVal !=  XRSA_PUB_EXP_INVALID_ONE_VALUE) &&
	    (PubExpVal != XRSA_PUB_EXP_INVALID_ONE_VALUE) &&
	    (PubExpVal != XRSA_PUB_EXP_INVALID_THREE_VALUE) &&
	    (PubExpVal != XRSA_PUB_EXP_INVALID_THREE_VALUE)) {
		Status = XASUFW_SUCCESS;
	}
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function checks whether the modulus and input data is valid or not
 *
 * @param	BuffAddr	Pointer to the buffer whose value needs to be checked.
 * @param	InputData	Pointer to the buffer whose value needs to be checked with.
 * @param       Len		Length of the buffers.
 * @param	ScratchBuf	Pointer to scratch buffer for temporary computation.
 *
 * @return
 *		- XASUFW_SUCCESS, if modulus data is greater than input data.
 *		- XASUFW_RSA_MOD_DATA_INVALID, if modulus data less than input data.
 *		- XASUFW_RSA_MOD_DATA_INPUT_DATA_EQUAL, if modulus data equal to input data.
 *
 *************************************************************************************************/
static s32 XRsa_ValidateModulusNdInputdata(u8 *BuffAddr, u8 *InputData, u32 Len, u8 *ScratchBuf)
{
	s32 Status = XASUFW_FAILURE;
	s32 SStatus = XASUFW_FAILURE;
	volatile u32 Index = 0U;
	volatile u32 Borrow = XASUFW_VALUE_ONE;
	/* Use caller-provided scratch buffer from RSA data block region. */
	u8 *ModulusMinus1 = ScratchBuf;
	u32 Idx = 0U;

	/** Check if modulus is non-zero. */
	Status = XAsu_IsBufferNonZero(BuffAddr, Len);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_MOD_DATA_IS_ZERO;
		goto END;
	}

	/** Check if modulus is odd (LSB should be 1) */
	if ((BuffAddr[Len - XASUFW_VALUE_ONE] & XASUFW_VALUE_ONE) == XASUFW_VALUE_ONE) {
		Status = XASUFW_SUCCESS;
	} else {
		Status = XASUFW_RSA_MOD_DATA_INVALID;
		goto END;
	}

	/** Validate and error out if the InputData is 0 or 1. */
	Status = XAsu_IsBufferNonZero(InputData, (Len - XASUFW_VALUE_ONE));
	if (Status != XASUFW_SUCCESS) {
		if ((InputData[Len - XASUFW_VALUE_ONE] == 0U) ||
		    (InputData[Len - XASUFW_VALUE_ONE] == XASUFW_VALUE_ONE)) {
			Status = XASUFW_RSA_MOD_DATA_INPUT_DATA_EQUAL;
			goto END;
		}
	}

	/** Check if InputData < (BuffAddr - 1) by comparing InputData with (BuffAddr - 1) */
	/** - Copy BuffAddr to ModulusMinus1 and subtract 1 from the last byte */
	for (Index = 0U; Index < Len; Index++) {
		ModulusMinus1[Index] = BuffAddr[Index];
	}

	/** - Subtract 1 from the last byte (least significant byte) with borrow propagation */
	for (Index = Len; Index > 0U; Index--) {
		Idx = Index - XASUFW_VALUE_ONE;
		if (ModulusMinus1[Idx] >= Borrow) {
			ModulusMinus1[Idx] = ModulusMinus1[Idx] - (u8)Borrow;
			Borrow = 0U;
			break;
		} else {
			ModulusMinus1[Idx] = XRSA_BORROW_PROPAGATION_FILL_VALUE;
			Borrow = XASUFW_VALUE_ONE;
		}
	}

	/** Now compare InputData with ModulusMinus1 (BuffAddr - 1) */
	Status = XASUFW_RSA_MOD_DATA_INPUT_DATA_EQUAL;
	for (Index = 0U; Index < Len; Index++) {
		if (ModulusMinus1[Index] > InputData[Index]) {
			Status = XASUFW_SUCCESS;
			Index = Len;
		} else if (ModulusMinus1[Index] < InputData[Index]) {
			Status = XASUFW_RSA_MOD_DATA_INVALID;
			Index = Len;
		} else {
			/* Continue to next byte when both values are equal. */
		}
	}

END:
	/** Zeroize the ModulusMinus1 buffer. */
	SStatus = Xil_SecureZeroize((u8 *)ModulusMinus1, XRSA_MAX_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, SStatus);

	return Status;
}

#ifdef XASU_KEYMANAGER_ENABLE
/*************************************************************************************************/
/**
 * @brief	Periodic task handler for scheduler-based RSA key pair generation. Implements a
 *		non-blocking state machine that generates RSA keys incrementally across multiple
 *		scheduler invocations and stores them in the ASU key vault.
 *
 * @param	Arg	Unused argument (required by task handler signature).
 *
 * @return
 *		- XASUFW_SUCCESS, on each successful step or when vault is full.
 *		- Error code, on failure during key generation or vault storage.
 *
 *************************************************************************************************/
static s32 XRsa_GenerateKeyPairTask(void *Arg)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(SStatus, XASUFW_FAILURE);
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	static XRsa_KeyGenState KeyGenState = XRSA_KEY_GEN_DEFAULT_STATE;
	static XRsa_KeyPtr KeyPairState;
	static u32 QuantSize = 0U;
	static u32 KeyLen = 0U;
	u8 *ModulusPtr = RsaKeyGenBuf;
	u8 *PvtExpPtr = ModulusPtr + XRSA_MAX_KEY_SIZE_IN_BYTES;
	u8 *PrimePPtr = PvtExpPtr + XRSA_MAX_KEY_SIZE_IN_BYTES;
	u8 *PrimeQPtr = PrimePPtr + XRSA_MAX_HALF_KEY_SIZE_IN_BYTES;
	u8 *DPPtr = PrimeQPtr + XRSA_MAX_HALF_KEY_SIZE_IN_BYTES;
	u8 *DQPtr = DPPtr + XRSA_MAX_HALF_KEY_SIZE_IN_BYTES;
	u8 *QInvPtr = DQPtr + XRSA_MAX_HALF_KEY_SIZE_IN_BYTES;
	u8 *PwctMsg = NULL;
	u8 *PwctEnc = NULL;
	u8 *PwctDec = NULL;

	(void)Arg;

	if (KeyGenState == XRSA_KEY_GEN_DEFAULT_STATE) {
		/** Check if ASU vault already has enough RSA keys. */
		if ((XKeyManager_GetAsuRsaActiveKeyCount(XASU_RSA_PVT_SUBVAULT_ID) >=
			XRSA_KEY_GEN_VAULT_CAPACITY) && (XKeyManager_GetAsuRsaActiveKeyCount(XASU_RSA_PUB_SUBVAULT_ID) >=
			XRSA_KEY_GEN_VAULT_CAPACITY)) {
			Status = XASUFW_SUCCESS;
			goto END;
		}

		KeyLen = XRSA_KEY_GEN_DEFAULT_SIZE;
		if (KeyLen == XRSA_2048_KEY_SIZE) {
			QuantSize = XRSA_2048_QUANT_SIZE;
		} else if (KeyLen == XRSA_3072_KEY_SIZE) {
			QuantSize = XRSA_3072_QUANT_SIZE;
		} else {
			QuantSize = XRSA_4096_QUANT_SIZE;
		}

		KeyGenState = XRSA_KEY_GEN_INIT_STATE;
	}

	if (KeyGenState == XRSA_KEY_GEN_INIT_STATE) {
		/**
		 * Capture the start time of the RSA key pair generation operation, if
		 * performance measurement is enabled.
		 */
		XASUFW_MEASURE_PERF_START(XASU_MODULE_RSA_ID);

		/**
		 * Initialize key pair state with dedicated buffer.
		 * Uses TRNG internally for random prime candidate generation.
		 */

		/** Zero-fill the public exponent buffer and set E=65537 at word[0] (LE). */
		Status = Xil_SMemSet(RsaKeyGenPubExpBuf, sizeof(RsaKeyGenPubExpBuf), 0U,
				     sizeof(RsaKeyGenPubExpBuf));
		if (Status != XASUFW_SUCCESS) {
			KeyGenState = XRSA_KEY_GEN_DEFAULT_STATE;
			goto END;
		}

		RsaKeyGenPubExpBuf[0U] = XRSA_PUB_EXP_VALUE;

		KeyPairState.M = ModulusPtr;
		KeyPairState.D = PvtExpPtr;
		KeyPairState.P = PrimePPtr;
		KeyPairState.Q = PrimeQPtr;
		KeyPairState.DP = DPPtr;
		KeyPairState.DQ = DQPtr;
		KeyPairState.iQ = QInvPtr;
		KeyPairState.E = (u8 *)RsaKeyGenPubExpBuf;
		KeyPairState.stage = 0U;
		KeyPairState.s = 0U;
		KeyPairState.bits2 = 0U;
		KeyPairState.iter = 0U;

		/** Release reset of RSA engine, run init, then set back under reset. */
		XAsufw_CryptoCoreReleaseReset(XASU_RSA_BASEADDR, XRSA_RESET_REG_OFFSET);

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = (s32)rsaprvkeyinit_Q(KeyLen * XASUFW_BYTE_LEN_IN_BITS, NULL, &KeyPairState);

		XAsufw_CryptoCoreSetReset(XASU_RSA_BASEADDR, XRSA_RESET_REG_OFFSET);

		if (Status != XASUFW_SUCCESS) {
			Status = XRsa_UpdateStatus(Status);
			KeyGenState = XRSA_KEY_GEN_DEFAULT_STATE;
			goto END;
		}

		KeyGenState = XRSA_KEY_GEN_STEP_STATE;
	}

	if (KeyGenState == XRSA_KEY_GEN_STEP_STATE) {
		/**
		 * Perform one incremental step of Rabin-Miller primality test.
		 * Returns XRSA_KEY_GEN_STEP_WAIT_STATE (1) when more work is needed.
		 */
		XAsufw_CryptoCoreReleaseReset(XASU_RSA_BASEADDR, XRSA_RESET_REG_OFFSET);

		ASSIGN_VOLATILE(Status, (s32)XASU_STATUS_FAIL);
		Status = (s32)rsaprvkeystep_Q(QuantSize, &KeyPairState);

		XAsufw_CryptoCoreSetReset(XASU_RSA_BASEADDR, XRSA_RESET_REG_OFFSET);

		if (Status == XRSA_KEY_GEN_STEP_WAIT_STATE) {
			/** Not done yet - return success so scheduler keeps calling. */
			Status = XASUFW_SUCCESS;
			goto END;
		}

		if (Status != XASUFW_SUCCESS) {
			/** Error during generation, restart from init on next call. */
			Status = XRsa_UpdateStatus(Status);
			KeyGenState = XRSA_KEY_GEN_INIT_STATE;
			goto END;
		}

		/** Key generation complete, proceed to finalization. */
		KeyGenState = XRSA_KEY_GEN_READY_STATE;
	}

	if (KeyGenState == XRSA_KEY_GEN_READY_STATE) {
		/**
		 * Pairwise Consistency Test (PWCT): Verify the generated key pair
		 * by encrypting a test message with (E, M) and decrypting with (D, M),
		 * then comparing the result with the original message.
		 * Keys are still in LE format from the library at this point.
		 */
		PwctMsg = XRsa_GetDataBlockAddr();
		PwctEnc = PwctMsg + XRSA_MAX_KEY_SIZE_IN_BYTES;
		PwctDec = PwctEnc + XRSA_MAX_KEY_SIZE_IN_BYTES;

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = Xil_SMemSet(PwctMsg, XRSA_MAX_KEY_SIZE_IN_BYTES, 0U,
				     XRSA_MAX_KEY_SIZE_IN_BYTES);
		if (Status != XASUFW_SUCCESS) {
			KeyGenState = XRSA_KEY_GEN_DEFAULT_STATE;
			Status = XASUFW_ZEROIZE_MEMSET_FAIL;
			goto END;
		}

		PwctMsg[0U] = XRSA_PWCT_TEST_MSG_VALUE;
		XAsufw_CryptoCoreReleaseReset(XASU_RSA_BASEADDR, XRSA_RESET_REG_OFFSET);

		rsaexp(PwctMsg, (u8 *)RsaKeyGenPubExpBuf, ModulusPtr,
		       (s32)XRSA_BYTE_TO_BIT(KeyLen), PwctEnc);

		XAsufw_CryptoCoreSetReset(XASU_RSA_BASEADDR, XRSA_RESET_REG_OFFSET);

		/** Decrypt: PwctDec = PwctEnc ^ D mod M (with E for internal verification) */
		XAsufw_CryptoCoreReleaseReset(XASU_RSA_BASEADDR, XRSA_RESET_REG_OFFSET);

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = RSA_ExpQ(PwctEnc, PvtExpPtr, ModulusPtr, NULL, NULL,
				  (u8 *)RsaKeyGenPubExpBuf, NULL,
				  (s32)XRSA_BYTE_TO_BIT(KeyLen), PwctDec);

		XAsufw_CryptoCoreSetReset(XASU_RSA_BASEADDR, XRSA_RESET_REG_OFFSET);

		if (Status != XASUFW_SUCCESS) {
			KeyGenState = XRSA_KEY_GEN_DEFAULT_STATE;
			Status = XASUFW_RSA_PWCT_DECRYPT_FAIL;
			goto END;
		}

		/** Compare decrypted output with original message. */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = Xil_SMemCmp(PwctDec, XRSA_MAX_KEY_SIZE_IN_BYTES,
				     PwctMsg, XRSA_MAX_KEY_SIZE_IN_BYTES, KeyLen);

		if (Status != XASUFW_SUCCESS) {
			KeyGenState = XRSA_KEY_GEN_DEFAULT_STATE;
			Status = XASUFW_RSA_PWCT_COMPARISON_FAIL;
			goto END;
		}
		/** Convert endianness from LE to BE for all key components. */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_ChangeEndianness(ModulusPtr, KeyLen);
		if (Status != XASUFW_SUCCESS) {
			KeyGenState = XRSA_KEY_GEN_DEFAULT_STATE;
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_ChangeEndianness(PvtExpPtr, KeyLen);
		if (Status != XASUFW_SUCCESS) {
			KeyGenState = XRSA_KEY_GEN_DEFAULT_STATE;
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_ChangeEndianness(PrimePPtr, XRSA_HALF_LEN(KeyLen));
		if (Status != XASUFW_SUCCESS) {
			KeyGenState = XRSA_KEY_GEN_DEFAULT_STATE;
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_ChangeEndianness(PrimeQPtr, XRSA_HALF_LEN(KeyLen));
		if (Status != XASUFW_SUCCESS) {
			KeyGenState = XRSA_KEY_GEN_DEFAULT_STATE;
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_ChangeEndianness(DPPtr, XRSA_HALF_LEN(KeyLen));
		if (Status != XASUFW_SUCCESS) {
			KeyGenState = XRSA_KEY_GEN_DEFAULT_STATE;
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_ChangeEndianness(DQPtr, XRSA_HALF_LEN(KeyLen));
		if (Status != XASUFW_SUCCESS) {
			KeyGenState = XRSA_KEY_GEN_DEFAULT_STATE;
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_ChangeEndianness(QInvPtr, XRSA_HALF_LEN(KeyLen));
		if (Status != XASUFW_SUCCESS) {
			KeyGenState = XRSA_KEY_GEN_DEFAULT_STATE;
			goto END;
		}

		/** Store the generated key pair in the ASU vault. */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XKeyManager_StoreRsaKeyPairInAsuVault(RsaKeyGenBuf, KeyLen, 0U);

		/** Reset to default state to check for more keys needed. */
		KeyGenState = XRSA_KEY_GEN_DEFAULT_STATE;

		/**
		 * Measure and print the performance time for the RSA key pair generation
		 * operation, if performance measurement is enabled.
		 */
		XASUFW_MEASURE_PERF_STOP(XASU_MODULE_RSA_ID);
	}

END:
	/**
	 * Securely zeroize key generation buffer unless in STEP_STATE,
	 * where intermediate key data must persist for the next scheduler call.
	 */
	if (KeyGenState != XRSA_KEY_GEN_STEP_STATE) {
		if (PwctMsg != NULL) {
			ASSIGN_VOLATILE(SStatus, XASUFW_FAILURE);
			SStatus = Xil_SecureZeroize(PwctMsg,
						XRSA_MAX_PARAM_SIZE_IN_BYTES);
			Status = XAsufw_UpdateBufStatus(Status, SStatus);
		}
		XFIH_CALL(Xil_SecureZeroize, XFihVar, SStatus, RsaKeyGenBuf,
						XRSA_MAX_KEY_OBJ_SIZE_IN_BYTES);
		Status = XAsufw_UpdateBufStatus(Status, SStatus);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Add the RSA key pair generation task to the ASUFW task scheduler. The task runs
 *		at lowest priority and checks the ASU vault periodically, generating keys
 *		incrementally when fewer than XRSA_KEY_GEN_VAULT_CAPACITY keys are available.
 *
 * @return
 *		- XASUFW_SUCCESS, on success.
 *		- XASUFW_RSA_KEY_GEN_ADD_TASK_ERROR, if task creation fails.
 *
 *************************************************************************************************/
s32 XRsa_AddKeyPairGenToScheduler(void)
{
	s32 Status = XASUFW_FAILURE;

	/** Task node for periodic RSA key pair generation */
	XTask_TaskNode *RsaKeyGenTask = NULL;

	/** Create a periodic task for RSA key pair generation. */
	RsaKeyGenTask = XTask_Create(XRSA_KEY_GEN_TASK_PRIORITY,
				     XRsa_GenerateKeyPairTask, NULL,
				     XRSA_KEY_GEN_POLL_INTERVAL);
	if (RsaKeyGenTask == NULL) {
		Status = XASUFW_RSA_KEY_GEN_ADD_TASK_ERROR;
		goto END;
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}
#endif /* XASU_KEYMANAGER_ENABLE */
/** @} */
