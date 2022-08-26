/**************************************************************************************************
* Copyright (C) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xtrngpsv.c
 * @addtogroup Overview
 * @{
 *
 * Contains the required functions of the XTrngpsv driver. See xtrng.h for a description of the
 * driver.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.00  ssc  09/05/21 First release
 * 1.1   ssc  03/24/22 Updates based on Security best practices and other assorted changes
 * 1.2   kpt  08/03/22 Added volatile keyword to avoid compiler optimization of loop redundancy checks
 *       ssc  08/25/22 Updates based on Security best practices, error handling fix in
 * 					XTrngpsv_Generate, moved Xil_SecureRMW32 to BSP.
 *
 * </pre>
 *
 **************************************************************************************************/

/*************************************** Include Files *******************************************/

#include "xtrngpsv.h"

/************************************ Constant Definitions ***************************************/

#define XTRNGPSV_BURST_SIZE		16U	/**< QCNT of 4 * 4 bytes (reg width)= 16 bytes */
#define XTRNGPSV_BURST_SIZE_BITS	128U	/**< Burst size in bits */
#define XTRNGPSV_NUM_INIT_REGS		12U	/**< No. of SEED and PERS STRING registers each */
#define XTRNGPSV_REG_SIZE		32U	/**< Size of TRNG registers */
#define XTRNGPSV_BYTES_PER_REG		4U	/**< Number of bytes register (i.e. 32/8) */
#define XTRNGPSV_MAX_QCNT		4U	/**< Max value of QCNT field in STATUS register */

#define XTRNGPSV_RESEED_TIMEOUT		15000U	/**< Reseed timeout in micro-seconds */
#define XTRNGPSV_GENERATE_TIMEOUT	8000U	/**< Generate timeout in micro-seconds */

#define PRNGMODE_RESEED			0U	/**< PRNG in Reseed mode */
#define PRNGMODE_GEN			TRNG_CTRL_PRNGMODE_MASK	/**< PRNG in Generate mode */

#define XTRNGPSV_MIN_SEEDLIFE		1U	/**< Minimum seed life */
#define XTRNGPSV_MAX_SEEDLIFE		0x1000000000000U /**< Maximum seed life 2^^48 */
#define XTRNGPSV_MIN_DFLENMUL		2U 	/**< Minimum DF Length Multiplier.This assumes
						additional multiplier of 1 for nonce */
#define XTRNGPSV_MAX_DFLENMUL		9U	/**< Maximum DF Length Multiplier */

#define ALL_A_PATTERN_32	0xAAAAAAAAU	/**< Pattern of 10101010... */
#define ALL_5_PATTERN_32	0x55555555U	/**< Pattern of 01010101... */

#define RESET_DELAY	10U	/**< Delay used in Reset operations. */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/*************************************************************************************************/

/************************************ Function Prototypes ****************************************/

static void XTrngpsv_Reset(const XTrngpsv *InstancePtr);
static void XTrngpsv_SoftReset(const XTrngpsv *InstancePtr);
static void XTrngpsv_HoldReset(const XTrngpsv *InstancePtr);
static s32 XTrngpsv_CollectRandData(XTrngpsv *InstancePtr, u8 *RandGenBuf, u32 NumBytes);
static s32 XTrngpsv_ReseedInternal(XTrngpsv *InstancePtr, const u8 *ExtSeedPtr, u8 *PersStrPtr,
		u32 DFLenMul);
static s32 XTrngpsv_WriteRegs(const XTrngpsv *InstancePtr, u32 StartRegOffset, u32 NumRegs,
		const u8 *InitBuf);
static s32 XTrngpsv_CheckSeedPattern(u8 *EntropyData, u32 EntropyLength);
static inline u32 XTrngpsv_ReadReg(UINTPTR BaseAddress, u32 RegOffset);
static inline void XTrngpsv_WriteReg(UINTPTR BaseAddress, u32 RegOffset, u32 RegValue);
static inline void XTrngpsv_RMW32(UINTPTR BaseAddress, u32 RegOffset, u32 RegMask, u32 RegValue);
static inline s32 XTrngpsv_WaitForEvent(u32 BaseAddr, u32 RegOffset, u32 EventMask, u32 Event,
		u32 Timeout);

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief
 * This function initializes the TRNGPSV instance/driver. This function must be called prior to
 * using the driver. Initialization includes only setting up the device id and base address.
 * All the remaining parameters of the Instance will be initialized in Instantiate function.
 *
 * @param	InstancePtr is a pointer to the XTrngpsv instance to be worked on.
 * @param	CfgPtr points to the configuration structure associated with the TRNGPSV driver.
 * @param	EffectiveAddr is the base address of the device. If address translation is being
 * 		used, then this parameter must reflect the virtual base address. Otherwise, the
 * 		physical address should be used.
 *
 * @return
 *		- XTRNGPSV_SUCCESS if initialization was successful.
 *		- XTRNGPSV_ERROR_INVALID_PARAM if invalid parameter passed to this function.
 *
 **************************************************************************************************/
s32 XTrngpsv_CfgInitialize(XTrngpsv *InstancePtr, const XTrngpsv_Config *CfgPtr,
		UINTPTR EffectiveAddr)
{
	volatile s32 Status = XTRNGPSV_FAILURE;

	/* Validate arguments. */
	if (InstancePtr == NULL) {
		Status = (s32)XTRNGPSV_ERROR_INVALID_PARAM;
		goto END;
	}

	if ((CfgPtr == NULL) || (EffectiveAddr == (UINTPTR)0U)) {
		Status = (s32)XTRNGPSV_ERROR_INVALID_PARAM;
		goto SET_ERR;
	}

	/* Populate Config parameters */
	InstancePtr->Config.DeviceId = CfgPtr->DeviceId;
	InstancePtr->Config.BaseAddress = EffectiveAddr;

	InstancePtr->State = XTRNGPSV_UNINITIALIZED;
	Status = (s32)XTRNGPSV_SUCCESS;

SET_ERR:
	if (Status != XTRNGPSV_SUCCESS) {
		InstancePtr->State = XTRNGPSV_ERROR;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief
 * This function further initializes i.e. instantiates the TRNGPSV instance/driver with user
 * configuration, resets the TRNG core and reseeds with initial seed and personalization string
 * provided during Instantiation. Switching to a new state of attributes or changing any attributes
 * (configured during Instantiation) requires UnInstantiation of the current instance and
 * instantiation of a new instance. After successful Instantiation, State of the driver is changed
 * to Healthy.
 *
 * @param	InstancePtr is a pointer to the XTrngpsv instance to be worked on.
 * @param	ConfigurValues points to the user configuration structure associated with the
 * 		TRNGPSV driver.
 *
 * @return
 *		- XTRNGPSV_SUCCESS if Instantiation was successful.
 *		- XTRNGPSV_ERROR_INVALID_PARAM if invalid parameter(s) passed to this function.
 *		- XTRNGPSV_ERROR_NOT_UNINSTANTIATED if the driver is not uninstantiated earlier.
 *		- XTRNGPSV_ERROR_INVALID_USRCFG_MODE if invalid Mode parameter.
 *		- XTRNGPSV_ERROR_INVALID_USRCFG_SEEDLIFE if SeedLife parameter is invalid
 *		- XTRNGPSV_ERROR_INVALID_USRCFG_PREDRES if invalid Prediction Resistance parameter.
 *		- XTRNGPSV_ERROR_INVALID_USRCFG_PERSPRES if PersStrPresent parameter is invalid.
 *		- XTRNGPSV_ERROR_INVALID_USRCFG_SEEDPRES if InitSeedPresent parameter is invalid.
 *		- XTRNGPSV_ERROR_NO_SEED_INSTANTIATE if no seed passed for DRNG mode.
 *		- XTRNGPSV_ERROR_UNNECESSARY_PARAM_INSTANTIATE if seed passed for HRNG mode or if
 *		unnecessary seed related parameter passed for PTRNG mode.
 *		- XTRNGPSV_ERROR_INVALID_USRCFG_DFDIS if DFDisable parameter is invalid.
 *		- XTRNGPSV_ERROR_INVALID_USRCFG_DFLENMUL if DFLenMul parameter is invalid.
 *		- XTRNGPSV_ERROR_USRCFG_CPY if error during copy of ConfigurValues structure.
 *		- Other error codes from the called functions as defined in XTrngpsv_ErrorCodes.
 *
 **************************************************************************************************/

s32 XTrngpsv_Instantiate(XTrngpsv *InstancePtr, const XTrngpsv_UsrCfg *ConfigurValues)
{
	volatile s32 Status = XTRNGPSV_FAILURE;
	u8 *SeedPtr;
	u8 *PersPtr;

	/* Validate arguments. */
	if (InstancePtr == NULL) {
		Status = (s32)XTRNGPSV_ERROR_INVALID_PARAM;
		goto END;
	}

	if (ConfigurValues == NULL) {
		Status = (s32)XTRNGPSV_ERROR_INVALID_PARAM;
		goto SET_ERR;
	}

	/*
	 * If device not in uninitialized state, return error so that user can uninstantiate first
	 * before instantiating again.
	 */
	if (InstancePtr->State != XTRNGPSV_UNINITIALIZED) {
		Status = (s32)XTRNGPSV_ERROR_NOT_UNINSTANTIATED;
		goto SET_ERR;
	}

	/* Validate User configuration parameters */

	if ((ConfigurValues->Mode != XTRNGPSV_HRNG) && (ConfigurValues->Mode != XTRNGPSV_DRNG)
			&& (ConfigurValues->Mode != XTRNGPSV_PTRNG)) {
		Status = (s32)XTRNGPSV_ERROR_INVALID_USRCFG_MODE;
		goto SET_ERR;
	}

	if ((ConfigurValues->Mode != XTRNGPSV_PTRNG)
			&& ((ConfigurValues->SeedLife < XTRNGPSV_MIN_SEEDLIFE)
					|| (ConfigurValues->SeedLife > XTRNGPSV_MAX_SEEDLIFE))) {
		Status = (s32)XTRNGPSV_ERROR_INVALID_USRCFG_SEEDLIFE;
		goto SET_ERR;
	}

	if ((ConfigurValues->PredResistanceEn != XTRNGPSV_FALSE)
			&& (ConfigurValues->PredResistanceEn != XTRNGPSV_TRUE)) {
		Status = (s32)XTRNGPSV_ERROR_INVALID_USRCFG_PREDRES;
		goto SET_ERR;
	}

	if ((ConfigurValues->PersStrPresent != XTRNGPSV_FALSE)
			&& (ConfigurValues->PersStrPresent != XTRNGPSV_TRUE)) {
		Status = (s32)XTRNGPSV_ERROR_INVALID_USRCFG_PERSPRES;
		goto SET_ERR;
	}

	if ((ConfigurValues->InitSeedPresent != XTRNGPSV_FALSE)
			&& (ConfigurValues->InitSeedPresent != XTRNGPSV_TRUE)) {
		Status = (s32)XTRNGPSV_ERROR_INVALID_USRCFG_SEEDPRES;
		goto SET_ERR;
	}

	if ((ConfigurValues->InitSeedPresent == XTRNGPSV_FALSE)
			&& (ConfigurValues->Mode == XTRNGPSV_DRNG)) {
		Status = (s32)XTRNGPSV_ERROR_NO_SEED_INSTANTIATE;
		goto SET_ERR;
	}

	if ((ConfigurValues->InitSeedPresent == XTRNGPSV_TRUE)
			&& (ConfigurValues->Mode == XTRNGPSV_HRNG)) {
		Status = (s32)XTRNGPSV_ERROR_UNNECESSARY_PARAM_INSTANTIATE;
		goto SET_ERR;
	}

	if ((ConfigurValues->DFDisable != XTRNGPSV_FALSE)
			&& (ConfigurValues->DFDisable != XTRNGPSV_TRUE)) {
		Status = (s32)XTRNGPSV_ERROR_INVALID_USRCFG_DFDIS;
		goto SET_ERR;
	}

	if ((ConfigurValues->DFDisable == XTRNGPSV_FALSE)
			&& ((ConfigurValues->DFLenMul < XTRNGPSV_MIN_DFLENMUL)
					|| (ConfigurValues->DFLenMul > XTRNGPSV_MAX_DFLENMUL))) {
		Status = (s32)XTRNGPSV_ERROR_INVALID_USRCFG_DFLENMUL;
		goto SET_ERR;
	}

	if ((ConfigurValues->DFDisable == XTRNGPSV_TRUE) && (ConfigurValues->DFLenMul != 0U)) {
		Status = (s32)XTRNGPSV_ERROR_INVALID_USRCFG_DFLENMUL;
		goto SET_ERR;
	}

	if ((ConfigurValues->Mode == XTRNGPSV_PTRNG)
			&& ((ConfigurValues->InitSeedPresent == XTRNGPSV_TRUE)
					|| (ConfigurValues->PersStrPresent == XTRNGPSV_TRUE)
					|| (ConfigurValues->PredResistanceEn == XTRNGPSV_TRUE)
					|| (ConfigurValues->SeedLife != 0U))) {
		Status = (s32)XTRNGPSV_ERROR_UNNECESSARY_PARAM_INSTANTIATE;
		goto SET_ERR;
	}

	/* Copy user configuration attributes to the Instance pointer */
	Status = Xil_SecureMemCpy(&InstancePtr->UsrCfg, (u32)sizeof(XTrngpsv_UsrCfg),
			ConfigurValues, (u32)sizeof(XTrngpsv_UsrCfg));
	if (Status != XTRNGPSV_SUCCESS) {
		Status = (s32)XTRNGPSV_ERROR_USRCFG_CPY;
		goto SET_ERR;
	}

	/* Reset the device for a clean state */
	XTrngpsv_Reset(InstancePtr);

	SeedPtr = (InstancePtr->UsrCfg.InitSeedPresent == XTRNGPSV_TRUE) ?
			(u8*)InstancePtr->UsrCfg.InitSeed : NULL;
	PersPtr = (InstancePtr->UsrCfg.PersStrPresent == XTRNGPSV_TRUE) ?
			(u8*)InstancePtr->UsrCfg.PersString : NULL;

	Status = XTRNGPSV_FAILURE;
	/* Reseed device with initial seed and personalization string */
	if ((InstancePtr->UsrCfg.Mode == XTRNGPSV_HRNG)
			|| (InstancePtr->UsrCfg.Mode == XTRNGPSV_DRNG)) {
		Status = XTrngpsv_ReseedInternal(InstancePtr, SeedPtr, PersPtr,
				InstancePtr->UsrCfg.DFLenMul);
		if (Status != XTRNGPSV_SUCCESS) {
			goto SET_ERR;
		}
	}

	/* Mark the state of the device as Healthy */
	InstancePtr->State = XTRNGPSV_HEALTHY;
	Status = XTRNGPSV_SUCCESS;

SET_ERR:
	if (Status != XTRNGPSV_SUCCESS) {
		InstancePtr->State = XTRNGPSV_ERROR;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief
 * This function reseeds the TRNG in DRNG, HRNG modes. This function will be called by random
 * number consuming application. This is just a wrapper function calling XTrng_ReseedInternal()
 * which passes NULL as personalization string.
 *
 * @param	InstancePtr is a pointer to the XTrngpsv instance to be worked on.
 * @param	ExtSeedPtr points to the byte array containing external seed. This shall be NULL
 * 		for HRNG case.
 * @param	DFLenMul is DF Length multiplier used to determine number of bits on the input of
 * 		the DF construct which is (DFLenMul + 1)*128 bits. This indicates the size of the
 * 		seed pointed by ExtSeedPtr. This shall be 0 for Non-DF case.
 *
 * @return
 *		- XTRNGPSV_SUCCESS if Reseed was successful.
 *		- XTRNGPSV_ERROR_INVALID_PARAM if invalid parameter(s) passed to this function.
 *		- XTRNGPSV_ERROR_INVALID_STATE if driver is not Healthy state before invoking this.
 *		- XTRNGPSV_ERROR_SEED_INVALID_MODE if this is called in PTRNG mode.
 *		- XTRNGPSV_ERROR_NO_SEED if no seed passed for DRNG mode.
 *		- XTRNGPSV_ERROR_UNNECESSARY_PARAM if seed passed for non-DRNG mode.
 *		- XTRNGPSV_ERROR_INVALID_RESEED_DFLENMUL if invalid DFLenMul parameter passed.
 *		- XTRNGPSV_ERROR_SAME_SEED if same seed passed for both Instantiation and Reseed.
 *		- Other error codes from the called functions as defined in XTrngpsv_ErrorCodes.
 *
 **************************************************************************************************/
s32 XTrngpsv_Reseed(XTrngpsv *InstancePtr, const u8 *ExtSeedPtr, u32 DFLenMul)
{
	volatile s32 Status = XTRNGPSV_FAILURE;
	volatile s32 Result = XTRNGPSV_SUCCESS;
	volatile s32 ResultTmp = XTRNGPSV_SUCCESS;

	/* Validate parameters */
	if (InstancePtr == NULL) {
		Status = (s32)XTRNGPSV_ERROR_INVALID_PARAM;
		goto END;
	}

	if (InstancePtr->State != XTRNGPSV_HEALTHY) {
		Status = (s32)XTRNGPSV_ERROR_INVALID_STATE;
		goto SET_ERR;
	}

	if ((InstancePtr->UsrCfg.Mode != XTRNGPSV_DRNG)
			&& (InstancePtr->UsrCfg.Mode != XTRNGPSV_HRNG)) {
		Status = (s32)XTRNGPSV_ERROR_SEED_INVALID_MODE;
		goto SET_ERR;
	}

	if ((InstancePtr->UsrCfg.Mode == XTRNGPSV_DRNG) && (ExtSeedPtr == NULL)) {
		Status = (s32)XTRNGPSV_ERROR_NO_SEED;
		goto SET_ERR;
	}

	if ((InstancePtr->UsrCfg.Mode != XTRNGPSV_DRNG) && (ExtSeedPtr != NULL)) {
		Status = (s32)XTRNGPSV_ERROR_UNNECESSARY_PARAM;
		goto SET_ERR;
	}

	if ((InstancePtr->UsrCfg.DFDisable == XTRNGPSV_FALSE)
			&& ((DFLenMul < XTRNGPSV_MIN_DFLENMUL) || (DFLenMul > XTRNGPSV_MAX_DFLENMUL))) {
		Status = (s32)XTRNGPSV_ERROR_INVALID_RESEED_DFLENMUL;
		goto SET_ERR;
	}

	if ((InstancePtr->UsrCfg.DFDisable == XTRNGPSV_TRUE) && (DFLenMul != 0U)) {
		Status = (s32)XTRNGPSV_ERROR_INVALID_RESEED_DFLENMUL;
		goto SET_ERR;
	}

	if (ExtSeedPtr != NULL) {
		/* if initial seed during instantiation and reseed is same, it is an error*/
		XSECURE_TEMPORAL_IMPL(Result, ResultTmp, Xil_SMemCmp,
				ExtSeedPtr, XTRNGPSV_SEED_LEN_BYTES, InstancePtr->UsrCfg.InitSeed,
				InstancePtr->EntropySize, InstancePtr->EntropySize);
		if ((Result == XTRNGPSV_SUCCESS) || (ResultTmp == XTRNGPSV_SUCCESS)) {
			Status = (s32)XTRNGPSV_ERROR_SAME_SEED;
			goto SET_ERR;
		}
	}

	/* Call the actual reseed function */
	Status = XTrngpsv_ReseedInternal(InstancePtr, ExtSeedPtr, NULL, DFLenMul);

SET_ERR:
	if (Status != XTRNGPSV_SUCCESS) {
		InstancePtr->State = XTRNGPSV_ERROR;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief
 * This the function which actually generates and provides random bits to the caller. Number of bits
 * generated per call is 256 bits. If user needs more bits of random data, this API can be called
 * multiple times accordingly.
 *
 * @param	InstancePtr is a pointer to the XTrngpsv instance to be worked on.
 * @param	RandBufPtr points to the address of the buffer in to which the random data
 * 		generated has to be copied.
 * @param	RandBufSize is size of the buffer to which RandBufPtr points to
 * @param	PredResistanceEn is the flag that controls Generate level Prediction Resistance.
 * 		When enabled, it mandates fresh seed for every Generate operation.
 *
 * @return
 *		- XTRNGPSV_SUCCESS if Random number generation was successful.
 *		- XTRNGPSV_ERROR_INVALID_PARAM if invalid parameter(s) passed to this function.
 *		- XTRNGPSV_ERROR_INVALID_STATE if driver is not Healthy state before invoking this.
 *		- XTRNGPSV_ERROR_INSUFFICIENT_RANDBUF if length of Buffer passed is insufficient.
 *		- XTRNGPSV_ERROR_INVALID_GEN_PREDRES if Prediction Resistance set for PTRNG mode.
 *		- XTRNGPSV_ERROR_PREDRES_MISMATCH if Pred Resistance not set during Instantiate
 *		but set now.
 *		- XTRNGPSV_ERROR_RESEEDING_REQUIRED if SeedLife elapsed.
 *		- XTRNGPSV_ERROR_RESEED_REQD_PREDRES if seed is consumed and has Pred Resistance.
 *		- XTRNGPSV_ERROR_GLITCH if error caused due to glitch conditions.
 *		- Other error codes from the called functions as defined in XTrngpsv_ErrorCodes.
 *
 *************************************************************************************************/
s32 XTrngpsv_Generate(XTrngpsv *InstancePtr, u8 *RandBufPtr, u32 RandBufSize, u8 PredResistanceEn)
{

	volatile s32 Status = XTRNGPSV_FAILURE;
	u8 *RandGenBuf;
	u32 NumBytes = XTRNGPSV_SEC_STRENGTH_BYTES;

	/* validate parameters */
	if (InstancePtr == NULL) {
		Status = (s32)XTRNGPSV_ERROR_INVALID_PARAM;
		goto END;
	}

	if (RandBufPtr == NULL) {
		Status = (s32)XTRNGPSV_ERROR_INVALID_PARAM;
		goto SET_ERR;
	}

	if (RandBufSize < XTRNGPSV_SEC_STRENGTH_BYTES) {
		Status = (s32)XTRNGPSV_ERROR_INSUFFICIENT_RANDBUF;
		goto SET_ERR;
	}

	if (InstancePtr->State != XTRNGPSV_HEALTHY) {
		Status = (s32)XTRNGPSV_ERROR_INVALID_STATE;
		goto SET_ERR;
	}

	if ((InstancePtr->UsrCfg.Mode == XTRNGPSV_PTRNG) && (PredResistanceEn == XTRNGPSV_TRUE)) {
		Status = (s32)XTRNGPSV_ERROR_INVALID_GEN_PREDRES;
		goto SET_ERR;
	}

	/**
	 *  If during Instantiation Prediction Resistance is enabled and
	 *  not during Generate, its an error
	 */
	if ((InstancePtr->UsrCfg.PredResistanceEn == XTRNGPSV_FALSE)
			&& (PredResistanceEn == XTRNGPSV_TRUE)) {
		Status = (s32)XTRNGPSV_ERROR_PREDRES_MISMATCH;
		goto SET_ERR;
	}

	RandGenBuf = RandBufPtr;

	if (InstancePtr->UsrCfg.Mode == XTRNGPSV_HRNG) {
		/* Reseed if SeedLife elapsed */
		if (InstancePtr->TrngStats.ElapsedSeedLife >= InstancePtr->UsrCfg.SeedLife) {
			Status = XTrngpsv_ReseedInternal(InstancePtr, NULL, NULL, 0U);
			if (Status != XTRNGPSV_SUCCESS) {
				goto SET_ERR;
			}
		}

		/* If prediction resistance enabled but seed is not new,
		 * reseed now
		 */
		if ((InstancePtr->UsrCfg.PredResistanceEn == XTRNGPSV_TRUE)
				&& (PredResistanceEn == XTRNGPSV_TRUE)
				&& (InstancePtr->TrngStats.ElapsedSeedLife > 0U)) {

			Status = XTRNGPSV_FAILURE;
			Status = XTrngpsv_ReseedInternal(InstancePtr, NULL, NULL, 0U);
			if (Status != XTRNGPSV_SUCCESS) {
				goto SET_ERR;
			}
		}

		Status = XTRNGPSV_FAILURE;
		Status = Xil_SecureOut32(InstancePtr->Config.BaseAddress + TRNG_CTRL, PRNGMODE_GEN);
		if (Status != XTRNGPSV_SUCCESS) {
			Status = (s32)XTRNGPSV_ERROR_GLITCH;
			goto SET_ERR;
		}

	}
	else if (InstancePtr->UsrCfg.Mode == XTRNGPSV_DRNG) {

		/* Error if SeedLife elapsed */
		if (InstancePtr->TrngStats.ElapsedSeedLife > InstancePtr->UsrCfg.SeedLife) {
			Status = (s32)XTRNGPSV_ERROR_RESEEDING_REQUIRED;
			goto SET_ERR;
		}

		/* If prediction resistance enabled but seed is not new,
		 * its an error
		 */
		if ((InstancePtr->UsrCfg.PredResistanceEn == XTRNGPSV_TRUE)
				&& (PredResistanceEn == XTRNGPSV_TRUE)
				&& (InstancePtr->TrngStats.ElapsedSeedLife > 0U)) {
			Status = (s32)XTRNGPSV_ERROR_RESEED_REQD_PREDRES;
			goto SET_ERR;
		}

		Status = Xil_SecureOut32(InstancePtr->Config.BaseAddress + TRNG_CTRL, PRNGMODE_GEN);
		if (Status != XTRNGPSV_SUCCESS) {
			Status = (s32)XTRNGPSV_ERROR_GLITCH;
			goto SET_ERR;
		}
	}
	else { /* UsrCfg.Mode == XTRNGPSV_PTRNG */

		if (InstancePtr->UsrCfg.DFDisable == XTRNGPSV_FALSE) {

			NumBytes = (InstancePtr->UsrCfg.DFLenMul + 1U) * BYTES_PER_BLOCK;
			InstancePtr->EntropySize = NumBytes;

			/* fill the DFInput datastructure with 0s so that
			 * it can be populated
			 */
			Status = Xil_SMemSet((u8*)&InstancePtr->DFInput, (u32)sizeof(InstancePtr->DFInput), 0U,
					(u32)sizeof(InstancePtr->DFInput));
			if (Status != XTRNGPSV_SUCCESS) {
				goto SET_ERR;
			}

			RandGenBuf = (u8*)InstancePtr->DFInput.EntropyData;
		}
		/*
		 * Enable all the 8 ring oscillators used for entropy source
		 * Provide soft reset, Enable loading entropy data as random number
		 */
		Status = XTRNGPSV_FAILURE;
		Status = Xil_SecureOut32(InstancePtr->Config.BaseAddress + TRNG_OSC_EN,
					TRNG_OSC_EN_VAL_MASK);
		if (Status != XTRNGPSV_SUCCESS) {
			Status = (s32)XTRNGPSV_ERROR_GLITCH;
			goto SET_ERR;
		}

		XTrngpsv_SoftReset(InstancePtr);

		Status = XTRNGPSV_FAILURE;
		Status = Xil_SecureOut32(InstancePtr->Config.BaseAddress + TRNG_CTRL,
				TRNG_CTRL_EUMODE_MASK | TRNG_CTRL_TRSSEN_MASK);
		if (Status != XTRNGPSV_SUCCESS) {
			Status = (s32)XTRNGPSV_ERROR_GLITCH;
			goto SET_ERR;
		}
	}

	/* Collect random data based on above configuration*/
	XSECURE_TEMPORAL_CHECK(SET_ERR, Status, XTrngpsv_CollectRandData,
			InstancePtr, RandGenBuf, NumBytes);

	InstancePtr->TrngStats.RandBytesReseed += NumBytes;
	InstancePtr->TrngStats.RandBytes += NumBytes;
	InstancePtr->TrngStats.ElapsedSeedLife++;

	/* In PTRNG mode with DF, collected random data to be fed as
	 * input of DF
	 */
	if (InstancePtr->UsrCfg.DFDisable == XTRNGPSV_FALSE) {
		if (InstancePtr->UsrCfg.Mode == XTRNGPSV_PTRNG) {
			Status = XTrngpsv_DF(InstancePtr, RandBufPtr, DF_RAND, NULL);
			if (Status != XTRNGPSV_SUCCESS) {
				goto SET_ERR;
			}
		}
	}

SET_ERR:
	if ((Status != XTRNGPSV_SUCCESS) && (InstancePtr->State != XTRNGPSV_CATASTROPHIC)) {
		InstancePtr->State = XTRNGPSV_ERROR;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief
 * This function is used to put the TRNG in reset state, and clear the instance
 * data including configuration, status.
 *
 * @param	InstancePtr is a pointer to the XTrngpsv instance to be worked on.
 *
 * @return
 *		- XTRNGPSV_SUCCESS if Uninstantiation was successful.
 *		- XTRNGPSV_ERROR_INVALID_PARAM if invalid parameter(s) passed to this function.
 *		- XTRNGPSV_ERROR_INVALID_STATE if driver is not Healthy state before invoking this.
 *		- XTRNGPSV_ERROR_GLITCH if error caused due to glitch conditions.
 *
 **************************************************************************************************/
s32 XTrngpsv_Uninstantiate(XTrngpsv *InstancePtr)
{
	volatile s32 Status = XTRNGPSV_FAILURE;

	/* Validate arguments. */
	if (InstancePtr == NULL) {
		Status = (s32)XTRNGPSV_ERROR_INVALID_PARAM;
		goto END;
	}

	if (InstancePtr->State == XTRNGPSV_UNINITIALIZED) {
		Status = (s32)XTRNGPSV_ERROR_INVALID_STATE;
		goto SET_ERR;
	}

	/* clear contents of external seed and personalization string */
	Status = XTrngpsv_WriteRegs(InstancePtr, TRNG_EXT_SEED_0, XTRNGPSV_SEED_LEN, NULL);
	if (Status != XTRNGPSV_SUCCESS) {
		goto SET_ERR;
	}

	Status = XTRNGPSV_FAILURE;
	Status = XTrngpsv_WriteRegs(InstancePtr, TRNG_PER_STRNG_0, XTRNGPSV_PERS_STR_LEN, NULL);
	if (Status != XTRNGPSV_SUCCESS) {
		goto SET_ERR;
	}

	XTrngpsv_HoldReset(InstancePtr);

	/* Clear the instance datastructure */
	Status = XTRNGPSV_FAILURE;
	Status = Xil_SMemSet(((u8*)InstancePtr + sizeof(InstancePtr->Config)),
			(u32)(sizeof(XTrngpsv) - sizeof(InstancePtr->Config)),
			0U, (u32)(sizeof(XTrngpsv) - sizeof(InstancePtr->Config)));
	if (Status != XTRNGPSV_SUCCESS) {
		goto SET_ERR;
	}

	InstancePtr->State = XTRNGPSV_UNINITIALIZED;
	Status = XTRNGPSV_SUCCESS;

SET_ERR:
	if (Status != XTRNGPSV_SUCCESS) {
		InstancePtr->State = XTRNGPSV_ERROR;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief
 * This API is used to reset the TRNG. There are two resets involved (RESET register and
 * TRNG_CTRL.PRNGsrst), both of which are asserted and de-asserted again after a delay.
 *
 * @param	InstancePtr is a pointer to the XTrngpsv instance to be worked on.
 *
 * @return	None.
 *
 **************************************************************************************************/
static void XTrngpsv_Reset(const XTrngpsv *InstancePtr)
{
	XTrngpsv_WriteReg(InstancePtr->Config.BaseAddress, TRNG_RESET, TRNG_RESET_VAL_MASK);
	usleep(RESET_DELAY);
	XTrngpsv_WriteReg(InstancePtr->Config.BaseAddress, TRNG_RESET, 0U);

	XTrngpsv_SoftReset(InstancePtr);
}

/*************************************************************************************************/
/**
 * @brief
 * This API is used to issue soft reset the TRNG (TRNG_CTRL.PRNGsrst)
 *
 * @param	InstancePtr is a pointer to the XTrngpsv instance to be worked on.
 *
 * @return	None.
 *
 **************************************************************************************************/
static void XTrngpsv_SoftReset(const XTrngpsv *InstancePtr)
{
	XTrngpsv_RMW32(InstancePtr->Config.BaseAddress, TRNG_CTRL,
		TRNG_CTRL_PRNGSRST_MASK, TRNG_CTRL_PRNGSRST_MASK);
	usleep(RESET_DELAY);
	XTrngpsv_RMW32(InstancePtr->Config.BaseAddress, TRNG_CTRL, TRNG_CTRL_PRNGSRST_MASK, 0U);
}

/*************************************************************************************************/
/**
 * @brief
 * This API is used to keep the TRNG in reset state. There are two resets involved (RESET register
 * and TRNG_CTRL.PRNGsrst), both of which are asserted.
 *
 * @param	InstancePtr is a pointer to the XTrngpsv instance to be worked on.
 *
 * @return	None.
 *
 **************************************************************************************************/
static void XTrngpsv_HoldReset(const XTrngpsv *InstancePtr)
{

	XTrngpsv_RMW32(InstancePtr->Config.BaseAddress, TRNG_CTRL,
		TRNG_CTRL_PRNGSRST_MASK, TRNG_CTRL_PRNGSRST_MASK);
	XTrngpsv_WriteReg(InstancePtr->Config.BaseAddress, TRNG_RESET, TRNG_RESET_VAL_MASK);

	usleep(RESET_DELAY);
}

/*************************************************************************************************/
/**
 * @brief
 * This is the function in which actually generation of random data is started and collected
 * random data is returned to calling function.
 *
 * @param	InstancePtr is a pointer to the XTrngpsv instance to be worked on.
 * @param	RandGenBuf points to the address of the buffer in to which the random data
 * 		generated has to be copied.
 * @param	NumBytes is number of bytes to be collected.
 *
 * @return
 *		- XTRNGPSV_SUCCESS if Random number collection was successful.
 *		- XTRNGPSV_ERROR_GENERATE_TIMEOUT if timeout occurred waiting for QCNT to become 4.
 *		- XTRNGPSV_ERROR_CATASTROPHIC_DTF if DTF bit asserted in STATUS register.
 *		- XTRNGPSV_ERROR_CATASTROPHIC_DTF_SW if DTF error detected in software.
 *		- XTRNGPSV_ERROR_GLITCH if error caused due to glitch conditions.
 *
 *************************************************************************************************/
static s32 XTrngpsv_CollectRandData(XTrngpsv *InstancePtr, u8 *RandGenBuf, u32 NumBytes)
{
	volatile s32 Status = XTRNGPSV_FAILURE;
	u32 BufIndex = 0U;
	u32 WordCount;
	volatile u32 BurstCount;
	volatile u32 RegVal;
	volatile u32 RegValTmp;
	u32 PatternMatch;
	u32 NumBursts = NumBytes / XTRNGPSV_BURST_SIZE;

	Status = Xil_SecureRMW32(InstancePtr->Config.BaseAddress + TRNG_CTRL,
			TRNG_CTRL_PRNGSTART_MASK, TRNG_CTRL_PRNGSTART_MASK);
	if (Status != XTRNGPSV_SUCCESS) {
		goto END;
	}

	/* Loop as many times based on NumBytes requested. In each burst 128 bits are generated,
	 * which is reflected in QCNT value of 4 by hardware.
	 */
	for (BurstCount = 0U; BurstCount < NumBursts; BurstCount++) {

		Status = XTRNGPSV_FAILURE;
		Status = XTrngpsv_WaitForEvent((u32)InstancePtr->Config.BaseAddress,
				TRNG_STATUS, TRNG_STATUS_QCNT_MASK,
				(u32)XTRNGPSV_MAX_QCNT << TRNG_STATUS_QCNT_SHIFT,
				XTRNGPSV_GENERATE_TIMEOUT);
		if (Status != XTRNGPSV_SUCCESS) {
			Status = (s32)XTRNGPSV_ERROR_GENERATE_TIMEOUT;
			goto END;
		}

		if ((InstancePtr->UsrCfg.Mode == XTRNGPSV_DRNG)
				|| (InstancePtr->UsrCfg.Mode == XTRNGPSV_HRNG)) {
			/* DTF flag set during generate indicates catastrophic condition, which
			 * needs to be checked for every time
			 */
			RegVal = XTrngpsv_ReadReg(InstancePtr->Config.BaseAddress, TRNG_STATUS);

			if ((RegVal & TRNG_STATUS_DTF_MASK) ==
			TRNG_STATUS_DTF_MASK) {
				InstancePtr->State = XTRNGPSV_CATASTROPHIC;
				Status = (s32)XTRNGPSV_ERROR_CATASTROPHIC_DTF;
				goto END;
			}
		}

		/* Read the core output register 4 times to consume the random data generated
		 * for every burst.
		 */
		PatternMatch = XTRNGPSV_TRUE;

		for (WordCount = 0U; WordCount < XTRNGPSV_BURST_SIZE_BITS / XTRNGPSV_REG_SIZE;
				WordCount++) {

			XSECURE_TEMPORAL_IMPL(RegVal, RegValTmp, XTrngpsv_ReadReg,
					InstancePtr->Config.BaseAddress, TRNG_CORE_OUTPUT);

			/* Check if the value to be stored is same as the previous value stored
			 * in RandBitBuf, if this is TRUE for all the 4 words, it means the data
			 * generated by current burst and previous burst (128 bit) are same, which
			 * is an error (which is checked at end of the current burst)
			 */
			if ((InstancePtr->RandBitBuf[WordCount] != RegVal)
					&& (InstancePtr->RandBitBuf[WordCount] != RegValTmp)) {
				PatternMatch = XTRNGPSV_FALSE;
			}

			InstancePtr->RandBitBuf[WordCount] = RegVal;

			/* Skip loading to final buffer if the generated data need not be consumed
			 * (e.g. for entropy testing)
			 */
			if (RandGenBuf != NULL) {
				/* Random data to be stored with reversed endianness */
				*((u32*)RandGenBuf + BufIndex) = XTRNGPSV_SWAP_ENDIAN(RegVal);
			}

			BufIndex++;
		}

		if ((NumBursts > 1U) && (BurstCount > 0U) && (PatternMatch == XTRNGPSV_TRUE )) {
			InstancePtr->State = XTRNGPSV_CATASTROPHIC;
			Status = (s32)XTRNGPSV_ERROR_CATASTROPHIC_DTF_SW;
			goto END;
		}
	}

	if (BurstCount != NumBursts)
	{
		Status = (s32)XTRNGPSV_ERROR_GLITCH;
		goto END;
	}

	Status = XTRNGPSV_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief
 * This is the actual reseed function. This function will be called by XTrng_Reseed and during
 * Generate operation. Instantiation too calls this function.
 *
 * @param	InstancePtr is a pointer to the XTrngpsv instance to be worked on.
 * @param	ExtSeedPtr points to the byte array containing external seed. This shall be NULL
 * 		for HRNG case.
 * @param	PersStrPtr points to Personalization string (which is recommended) in addition to
 * 		seed. This is needed only during Instantiation.
 * @param	DFLenMul is DF Length multiplier used to determine number of bits on the input of
 * 		the DF construct which is (DFLenMul +1)*128 bits. This indicates the size of the
 * 		seed pointed by ExtSeedPtr. This shall be 0 for Non-DF case.
 *
 * @return
 *		- XTRNGPSV_SUCCESS if Reseed was successful.
 *		- XTRNGPSV_ERROR_CERTF_SW_A5_PATTERN if entropy source outputs all 5s or all As.
 *		- XTRNGPSV_ERROR_CPY_RESEED if error encountered during seed copy.
 *		- XTRNGPSV_ERROR_RESEED_TIMEOUT if timeout occurred waiting for done of reseeding.
 *		- XTRNGPSV_ERROR_CERTF if CTF flag is set in STATUS register.
 *		- XTRNGPSV_ERROR_GLITCH if error caused due to glitch conditions.
 *		- Other error codes from the called functions as defined in XTrngpsv_ErrorCodes.
 *
 **************************************************************************************************/
static s32 XTrngpsv_ReseedInternal(XTrngpsv *InstancePtr, const u8 *ExtSeedPtr, u8 *PersStrPtr,
		u32 DFLenMul)
{
	volatile s32 Status = XTRNGPSV_FAILURE;
	volatile s32 StatusTemp = XTRNGPSV_FAILURE;
	volatile u32 RegVal;
	volatile u32 RegValTmp;
	u8 EntropyOutput[XTRNGPSV_SEED_LEN_BYTES];
	const volatile u8 *SeedPtr = NULL;
	volatile XTrngpsv_Mode Mode = XTRNGPSV_HRNG;

	InstancePtr->TrngStats.RandBytesReseed = 0;
	InstancePtr->TrngStats.ElapsedSeedLife = 0;

	/* Determine actual size (in bytes) of seed based on DF mode or non-DF mode */
	if (InstancePtr->UsrCfg.DFDisable == XTRNGPSV_TRUE) {
		InstancePtr->EntropySize = XTRNGPSV_SEED_LEN_BYTES;
	}
	else {
		InstancePtr->EntropySize = (DFLenMul + 1U) * BYTES_PER_BLOCK;
	}

	Mode = InstancePtr->UsrCfg.Mode;

	if (InstancePtr->UsrCfg.DFDisable == XTRNGPSV_TRUE) {
		if ((Mode == XTRNGPSV_HRNG) || (Mode == XTRNGPSV_HRNG)) {

			/* Versal TRNG IP doesn't recognize alternate 1 and 0  pattern, hence the
			 * entropy output need to be monitored before using it as seed. This means,
			 * TRNG couldn't be configured for entropy source as seed source. Instead,
			 * entropy data is collected as random data, and after inspecting for
			 * pattern, is fed again to the external seed registers. This is essentially
			 * similar to HRNG + DF case except that there is no DF involved. This
			 * actually is configuration for PTRNG mode (not for reseed) to collect
			 * random output data from entropy source.
			 */

			Status = Xil_SecureOut32(InstancePtr->Config.BaseAddress + TRNG_OSC_EN,
					TRNG_OSC_EN_VAL_MASK);
			if (Status != XTRNGPSV_SUCCESS) {
				Status = (s32)XTRNGPSV_ERROR_GLITCH;
				goto SET_ERR;
			}

			/* Provide soft reset before configuring the entropy mode */
			XTrngpsv_SoftReset(InstancePtr);

			Status = XTRNGPSV_FAILURE;
			Status = Xil_SecureOut32(InstancePtr->Config.BaseAddress + TRNG_CTRL,
					TRNG_CTRL_EUMODE_MASK | TRNG_CTRL_TRSSEN_MASK);
			if (Status != XTRNGPSV_SUCCESS) {
				Status = (s32)XTRNGPSV_ERROR_GLITCH;
				goto SET_ERR;
			}

			/* TRNG_CTRL.PRNGstart will be asserted in XTrngpsv_CollectRandData() */

			XSECURE_TEMPORAL_CHECK(SET_ERR, Status, XTrngpsv_CollectRandData,
					InstancePtr, EntropyOutput, XTRNGPSV_SEED_LEN_BYTES);

			XSECURE_TEMPORAL_IMPL(Status, StatusTemp, XTrngpsv_CheckSeedPattern,
					EntropyOutput, XTRNGPSV_SEED_LEN_BYTES);

			if ((Status != XTRNGPSV_SUCCESS) || (StatusTemp != XTRNGPSV_SUCCESS)) {
				Status = (s32)XTRNGPSV_ERROR_CERTF_SW_A5_PATTERN;
				goto SET_ERR;
			}

			SeedPtr = (u8*)EntropyOutput;

		}
		else if (InstancePtr->UsrCfg.Mode == XTRNGPSV_DRNG) {
			SeedPtr = ExtSeedPtr;
		}
		else {
			/* for MISRA-C */
			SeedPtr = NULL;
		}

		/* Program TRNG_EXT_SEED and TRNG_PER_STRNG registers based on the parameters
		 * passed.
		 */

		Status = XTRNGPSV_FAILURE;
		Status = XTrngpsv_WriteRegs(InstancePtr, TRNG_EXT_SEED_0,
				XTRNGPSV_SEED_LEN, (const u8*)SeedPtr);
		if (Status != XTRNGPSV_SUCCESS) {
			goto SET_ERR;
		}

		if (PersStrPtr != NULL) {
			/* Loading personalization is used in both DRNG and HRNG cases */
			Status = XTRNGPSV_FAILURE;
			Status = XTrngpsv_WriteRegs(InstancePtr, TRNG_PER_STRNG_0,
					XTRNGPSV_PERS_STR_LEN, PersStrPtr);
			if (Status != XTRNGPSV_SUCCESS) {
				goto SET_ERR;
			}
		}
	}
	else { /* DF Mode */

		/* Fill the DFInput datastructure with 0s and copy the external seed (for DRNG)
		 * or collect the entropy output data to the input to the DF operation.
		 */
		Status = Xil_SMemSet((u8*)&InstancePtr->DFInput, (u32)sizeof(InstancePtr->DFInput), 0U,
				(u32)sizeof(InstancePtr->DFInput));
		if (Status != XTRNGPSV_SUCCESS) {
			goto SET_ERR;
		}

		if (InstancePtr->UsrCfg.Mode == XTRNGPSV_HRNG) {

			/* This actually is configuration for PTRNG mode (not for reseed) to
			 * collect random output data from entropy source.
			 */

			Status = XTRNGPSV_FAILURE;
			Status = Xil_SecureOut32(InstancePtr->Config.BaseAddress + TRNG_OSC_EN,
					TRNG_OSC_EN_VAL_MASK);
			if (Status != XTRNGPSV_SUCCESS) {
				Status = (s32)XTRNGPSV_ERROR_GLITCH;
				goto SET_ERR;
			}

			/* Provide soft reset before configuring the entropy mode */
			XTrngpsv_SoftReset(InstancePtr);

			XTrngpsv_WriteReg(InstancePtr->Config.BaseAddress, TRNG_CTRL,
					TRNG_CTRL_EUMODE_MASK | TRNG_CTRL_TRSSEN_MASK);

			/* TRNG_CTRL.PRNGstart will be asserted in XTrngpsv_CollectRandData() */

			XSECURE_TEMPORAL_CHECK(SET_ERR, Status, XTrngpsv_CollectRandData,
					InstancePtr, InstancePtr->DFInput.EntropyData, InstancePtr->EntropySize);

			XSECURE_TEMPORAL_IMPL(Status, StatusTemp, XTrngpsv_CheckSeedPattern,
					InstancePtr->DFInput.EntropyData, InstancePtr->EntropySize);

			if ((Status != XTRNGPSV_SUCCESS) || (StatusTemp != XTRNGPSV_SUCCESS)) {
				Status = (s32)XTRNGPSV_ERROR_CERTF_SW_A5_PATTERN;
				goto SET_ERR;
			}
		}
		else if (InstancePtr->UsrCfg.Mode == XTRNGPSV_DRNG) {
			Status = XTRNGPSV_FAILURE;
			Status = Xil_SecureMemCpy(InstancePtr->DFInput.EntropyData,
					InstancePtr->EntropySize, ExtSeedPtr,
					InstancePtr->EntropySize);
			if (Status != XTRNGPSV_SUCCESS) {
				Status = (s32)XTRNGPSV_ERROR_CPY_RESEED;
				goto SET_ERR;
			}
		}
		else {
			/* For MISRA-C */
		}

		/* Call the DF operation with input as external seed OR random data generated from
		 * entropy output
		 */
		XSECURE_TEMPORAL_CHECK(SET_ERR, Status, XTrngpsv_DF, InstancePtr,
				(u8 *)InstancePtr->DFOutput, DF_SEED, PersStrPtr);

		/* Output of DF (new seed) is input to the external seed registers, also configure
		 * for the external seed as seed source type.
		 */
		XSECURE_TEMPORAL_CHECK(SET_ERR, Status, XTrngpsv_WriteRegs,
				InstancePtr, TRNG_EXT_SEED_0, XTRNGPSV_SEED_LEN, InstancePtr->DFOutput);

		/* Note that there is no personalization string programmed here as the
		 * personalization string is considered already as input to the DF.
		 */
	}

	/* select Reseed operation, and configure for external seed */
	Status = XTRNGPSV_FAILURE;
	Status = Xil_SecureOut32(InstancePtr->Config.BaseAddress + TRNG_CTRL,
			PRNGMODE_RESEED | TRNG_CTRL_PRNGXS_MASK);
	if (Status != XTRNGPSV_SUCCESS) {
		Status = (s32)XTRNGPSV_ERROR_GLITCH;
		goto SET_ERR;
	}

	/* Start the reseed operation with above configuration and wait for STATUS.Done bit to be
	 * set. Monitor STATUS.CERTF bit, if set indicates SP800-90B entropy health test has failed.
	 */
	XTrngpsv_RMW32(InstancePtr->Config.BaseAddress, TRNG_CTRL, TRNG_CTRL_PRNGSTART_MASK,
			TRNG_CTRL_PRNGSTART_MASK);

	Status = XTrngpsv_WaitForEvent((u32)InstancePtr->Config.BaseAddress, TRNG_STATUS,
			TRNG_STATUS_DONE_MASK, TRNG_STATUS_DONE_MASK, XTRNGPSV_RESEED_TIMEOUT);
	if (Status != XTRNGPSV_SUCCESS) {
		Status = (s32)XTRNGPSV_ERROR_RESEED_TIMEOUT;
		goto SET_ERR;
	}

	XSECURE_TEMPORAL_IMPL(RegVal, RegValTmp, XTrngpsv_ReadReg,
			InstancePtr->Config.BaseAddress, TRNG_STATUS);
	if (((RegVal & TRNG_STATUS_CERTF_MASK) == TRNG_STATUS_CERTF_MASK)
			|| ((RegValTmp & TRNG_STATUS_CERTF_MASK) == TRNG_STATUS_CERTF_MASK)) {
		Status = (s32)XTRNGPSV_ERROR_CERTF;
		goto SET_ERR;
	}

	/* De-assert PRNGstart bit after done with reseeding. This is required in the cases where
	 * there are two successive reseed operations are done (one in Instantiate and then in
	 * Reseed).
	 */
	XTrngpsv_RMW32(InstancePtr->Config.BaseAddress, TRNG_CTRL, TRNG_CTRL_PRNGSTART_MASK, 0U);

	Status = XTRNGPSV_SUCCESS;

SET_ERR:
	if (Status != XTRNGPSV_SUCCESS) {
		InstancePtr->State = XTRNGPSV_ERROR;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief
 * Write to the register
 *
 * Write a byte array InitBuf as a sequence of words into an array of register locations starting
 * with StartRegOffset. The first byte from InitBuf is stored in last byte of target register,
 * the second byte to last but one byte of target register.. and so on. If InitBuf is NULL, the
 * target registers are filled with all 0s.
 *
 * @param	InstancePtr is a pointer to the XTrngpsv instance to be
 *			worked on
 * @param	StartRegOffset contains the offset from the base address of the
 *				device from where register writes to be done
 * @param	NumRegs is number of registers to be programmed
 * @param	InitBuf is the source of buffer from where data will be written to registers.
 *
 * @return
 *		- XTRNGPSV_SUCCESS if Write to Registers is successful
 *		- XTRNGPSV_ERROR_GLITCH if write and read values are different
 *		- XTRNGPSV_FAILURE if any other failure
 *
 **************************************************************************************************/
static s32 XTrngpsv_WriteRegs(const XTrngpsv *InstancePtr, u32 StartRegOffset, u32 NumRegs,
		const u8 *InitBuf)
{
	volatile u32 Index;
	u32 Count;
	u32 RegVal;
	u32 Offset;
	volatile s32 Status = XTRNGPSV_FAILURE;

	for (Index = 0U; Index < NumRegs; ++Index) {
		if (InitBuf != NULL) {
			RegVal = 0U;
			for (Count = 0U; Count < XTRNGPSV_BYTES_PER_REG; ++Count) {
				RegVal = (RegVal << 8U)
						| InitBuf[Index * XTRNGPSV_BYTES_PER_REG + Count];
			}
			Offset = StartRegOffset
				+ (XTRNGPSV_NUM_INIT_REGS - 1U - Index) * XTRNGPSV_BYTES_PER_REG;
			XTrngpsv_WriteReg(InstancePtr->Config.BaseAddress, Offset, RegVal);
		}
		else {
			XTrngpsv_WriteReg(InstancePtr->Config.BaseAddress,
					StartRegOffset + Index * XTRNGPSV_BYTES_PER_REG, 0U);
		}
	}

	if (Index != NumRegs) {
		Status = (s32)XTRNGPSV_ERROR_GLITCH;
		goto END;
	}

	Status = XTRNGPSV_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief
 * Checks the value of seed against certain patterns, which are invalid.
 *
 * @param	EntropyData is a pointer to the data from where data has to be checked against.
 * @param	EntropyLength is number of bytes for which patterns has to be checked.
 *
 * @return	None.
 *
 * @note	0xAAAAAAAA and 0x55555555 are treated as invalid.
 *
 **************************************************************************************************/
static s32 XTrngpsv_CheckSeedPattern(u8 *EntropyData, u32 EntropyLength)
{
	s32 Status = XTRNGPSV_FAILURE;
	u32 Index;
	u32 EntropyLengthInWords = EntropyLength / 4U;

	for (Index = 0U; Index < EntropyLengthInWords; Index++) {
		if ((*((u32*)EntropyData + Index) == ALL_A_PATTERN_32)
				|| (*((u32*)EntropyData + Index) == ALL_5_PATTERN_32)) {
			goto END;
		}
	}

	Status = XTRNGPSV_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief
 * Read from the register.
 *
 * @param	BaseAddress cntains the base address of the device.
 * @param	RegOffset contains the offset from the base address of the device.
 *
 * @return	The value read from the register.
 *
 **************************************************************************************************/
static inline u32 XTrngpsv_ReadReg(UINTPTR BaseAddress, u32 RegOffset)
{
	return Xil_In32((UINTPTR)(BaseAddress + RegOffset));
}

/**************************************************************************************************/
/**
 * @brief
 * Write to the register.
 *
 * @param	BaseAddress contains the base address of the device.
 * @param	RegOffset contains the offset from the base address of the device.
 * @param	RegValue is the value to be written to the register.
 *
 * @return	None.
 *
 **************************************************************************************************/
static inline void XTrngpsv_WriteReg(UINTPTR BaseAddress, u32 RegOffset, u32 RegValue)
{
	Xil_Out32((UINTPTR)(BaseAddress + RegOffset), RegValue);
}

/**************************************************************************************************/
/**
 * @brief
 * Read-Modify-Write register.
 *
 * @param	BaseAddress contains the base address of the device.
 * @param	RegOffset contains the offset from the base address of the device.
 * @param	RegMask indicates the bits to be modified.
 * @param	RegValue is the value to be written to the register.
 *
 * @return	None.
 *
 **************************************************************************************************/
static inline void XTrngpsv_RMW32(UINTPTR BaseAddress, u32 RegOffset, u32 RegMask, u32 RegValue)
{
	Xil_UtilRMW32(((u32)BaseAddress + RegOffset), RegMask, RegValue);
}

/**************************************************************************************************/
/**
 * @brief
 * Waits for an event to occur for a certain timeout duration.
 *
 * @param	BaseAddr is address of register to be checked for event(s) occurrence.
 * @param	RegOffset is offset of register from base address.
 * @param	EventMask is mask indicating event(s) to be checked.
 * @param 	Event is specific event(s) value to be checked.
 * @param	Timeout is max number of microseconds to wait for an event(s).
 *
 * @return
 *          XTRNGPSV_SUCCESS - On occurrence of the event(s).
 *          XTRNGPSV_FAILURE - Event did not occur before counter reaches 0.
 *
 **************************************************************************************************/
static inline s32 XTrngpsv_WaitForEvent(u32 BaseAddr, u32 RegOffset, u32 EventMask, u32 Event,
		u32 Timeout)
{
	return (s32)Xil_WaitForEvent(BaseAddr + RegOffset, EventMask, Event, Timeout);
}
/** @} */
