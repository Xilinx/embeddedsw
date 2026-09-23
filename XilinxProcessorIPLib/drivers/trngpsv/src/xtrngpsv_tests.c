/**************************************************************************************************
* Copyright (C) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xtrngpsv_tests.c
 * @addtogroup trngpsv_api TRNGPSV APIs
 * @{
 *
 * Contains Known Answer Tests and Health Tests for the TRNGPSV component.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.00  ssc  09/05/21 First release
 * 1.1   ssc  03/24/22 Updates based on Security best practices
 * 1.2   ssc  08/25/22 Enhanced KAT to include explicit Reseed and made a doxygen update
 * 1.9   rap  08/28/26 Fixed Doxygen warnings
 *       ks   09/15/26 Added PTRNG mode DF Generate test to the KAT
 * </pre>
 *
 **************************************************************************************************/

/*************************************** Include Files *******************************************/

#include "xtrngpsv.h"

/************************************ Constant Definitions ***************************************/
/**
 * @name Constant definitions for parameters used for Health and KAT tests
 * @{
 */
/**
 * Constant definitions for Health and KAT tests.
 */
#define HEALTH_TEST_SEEDLIFE	10U	/**< Seed life for Health test */
#define HEALTH_TEST_DFLENMUL	7U	/**< DF length multiplier for Health test */
#define KAT_SEEDLIFE            5U	/**< Seed life for KAT */
#define KAT_DFLENMUL            2U	/**< DF length multiplier for KAT */
/** Length of the guard pattern placed after the requested bytes in the PTRNG mode test. */
#define KAT_GUARD_LEN_BYTES	(XTRNGPSV_SEED_LEN_BYTES - XTRNGPSV_GEN_LEN_BYTES)
/** @} */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief
 * Health tests are entropy tests and hence should be run when the configured mode is of PTRNG or
 * HRNG mode. 1024 bits of entropy data to be generated and the CTF flag to be monitored. TRNG
 * configured in HRNG + DF serves this purpose ((7+1)*128 = 1024 bits). Since this flag is monitored
 * for every reseed, no need to monitor CTF explicitly in this test. This API is to be called at
 * startup, after instantiation (and reseeding). Even for PTRNG mode, for this test, mode to be
 * configured as HRNG (as there is no reseed operation involved in PTRNG mode).
 *
 * @param	InstancePtr is a pointer to the XTrngpsv instance to be worked on.
 *
 * @return
 *		- XTRNGPSV_SUCCESS if Health tests are successfully run.
 *		- XTRNGPSV_ERROR_INVALID_PARAM if invalid parameter passed to this function (OR)
 *		other error codes from the called functions as defined in XTrngpsv_ErrorCodes.
 *
 *************************************************************************************************/

s32 XTrngpsv_RunHealthTest(XTrngpsv *InstancePtr)
{
	volatile s32 Status = XTRNGPSV_FAILURE;

	XTrngpsv_UsrCfg UsrCfgTests;

	if (InstancePtr == NULL) {
		Status = (s32)XTRNGPSV_ERROR_INVALID_PARAM;
		goto END;
	}

	Status = Xil_SMemSet((u8*)&UsrCfgTests, (u32)sizeof(UsrCfgTests), 0U, (u32)sizeof(UsrCfgTests));
	if (Status != XTRNGPSV_SUCCESS) {
		goto SET_ERR;
	}

	/** - Populate user config parameters of TRNGPSV driver */

	UsrCfgTests.Mode = XTRNGPSV_HRNG;
	UsrCfgTests.SeedLife = HEALTH_TEST_SEEDLIFE;
	UsrCfgTests.PredResistanceEn = XTRNGPSV_FALSE;
	UsrCfgTests.DFDisable = XTRNGPSV_FALSE;
	UsrCfgTests.DFLenMul = HEALTH_TEST_DFLENMUL;
	UsrCfgTests.PersStrPresent = XTRNGPSV_FALSE;
	UsrCfgTests.InitSeedPresent = XTRNGPSV_FALSE;

	/** - Invoke Instantiate (which includes reseed) operation */
	Status = XTrngpsv_Instantiate(InstancePtr, &UsrCfgTests);
	if (Status != XTRNGPSV_SUCCESS) {
		goto SET_ERR;
	}

	/** - Status Reset */
	Status = XTRNGPSV_FAILURE;

	/**
	 * - CTF is not being monitored explicitly here since that is done as part of Reseeding
	 *   during Instantiation
	 */

	/** - Uninstantiate driver instance after Health Tests */
	Status = XTrngpsv_Uninstantiate(InstancePtr);

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
 * KAT can be run at startup and on demand. 256 bits are generated and ignored. This test can be
 * run when the TRNG is configured in DRNG and HRNG modes. For HRNG mode too, tests should be run
 * in DRNG mode. The Generate of the PTRNG mode is covered by a guard pattern check, as the
 * random data of that mode is not reproducible. If KAT fails, driver has to be put in Error
 * state.
 *
 * @param	InstancePtr is a pointer to the XTrngpsv instance to be worked on.
 *
 * @return
 *		- XTRNGPSV_SUCCESS if KAT is successful.
 *		- XTRNGPSV_ERROR_INVALID_PARAM if invalid parameter passed to this function.
 *		- XTRNGPSV_ERROR_USRCFG_CPY_KAT if copy of seed/personalization string failed.
 *		- XTRNGPSV_ERROR_KAT_MISMATCH if KAT result doesn't match with expected output.
 *		- XTRNGPSV_FAILURE if clearing of the local buffer of the PTRNG mode test failed (OR)
 *		other error codes from the called functions as defined in XTrngpsv_ErrorCodes.
 *
 *************************************************************************************************/
s32 XTrngpsv_RunKAT(XTrngpsv *InstancePtr)
{

	volatile s32 Status = XTRNGPSV_FAILURE;
	volatile s32 Result =  XTRNGPSV_FAILURE;
	volatile s32 ClrStatus = XTRNGPSV_FAILURE;
	XTrngpsv_UsrCfg UsrCfgTests;

	const u8 ExtSeed[XTRNGPSV_SEED_LEN_BYTES] = {
			0x3BU, 0xC3U, 0xEDU, 0x64U, 0xF4U, 0x80U, 0x1CU, 0xC7U,
			0x14U, 0xCCU, 0x35U, 0xEDU, 0x57U, 0x01U, 0x2AU, 0xE4U,
			0xBCU, 0xEFU, 0xDEU, 0xF6U, 0x7CU, 0x46U, 0xA6U, 0x34U,
			0xC6U, 0x79U, 0xE8U, 0x91U, 0x5DU, 0xB1U, 0xDBU, 0xA7U,
			0x49U, 0xA5U, 0xBBU, 0x4FU, 0xEDU, 0x30U, 0xB3U, 0x7BU,
			0xA9U, 0x8BU, 0xF5U, 0x56U, 0x4DU, 0x40U, 0x18U, 0x9FU,
	};

	const u8 ExtReSeed[XTRNGPSV_SEED_LEN_BYTES] = {
		    0xFDU, 0x85U, 0xA8U, 0x36U, 0xBBU, 0xA8U, 0x50U, 0x19U,
		    0x88U, 0x1EU, 0x8CU, 0x6BU, 0xADU, 0x23U, 0xC9U, 0x06U,
		    0x1AU, 0xDCU, 0x75U, 0x47U, 0x76U, 0x59U, 0xACU, 0xAEU,
		    0xA8U, 0xE4U, 0xA0U, 0x1DU, 0xFEU, 0x07U, 0xA1U, 0x83U,
		    0x2DU, 0xADU, 0x1CU, 0x13U, 0x6FU, 0x59U, 0xD7U, 0x0FU,
		    0x86U, 0x53U, 0xA5U, 0xDCU, 0x11U, 0x86U, 0x63U, 0xD6U,
	};

	const u8 PersString[XTRNGPSV_PERS_STR_LEN_BYTES] = {
			0xB2U, 0x80U, 0x7EU, 0x4CU, 0xD0U, 0xE4U, 0xE2U, 0xA9U,
			0x2FU, 0x1FU, 0x5DU, 0xC1U, 0xA2U, 0x1FU, 0x40U, 0xFCU,
			0x1FU, 0x24U, 0x5DU, 0x42U, 0x61U, 0x80U, 0xE6U, 0xE9U,
			0x71U, 0x05U, 0x17U, 0x5BU, 0xAFU, 0x70U, 0x30U, 0x18U,
			0xBCU, 0x23U, 0x18U, 0x15U, 0xCBU, 0xB8U, 0xA6U, 0x3EU,
			0x83U, 0xB8U, 0x4AU, 0xFEU, 0x38U, 0xFCU, 0x25U, 0x87U,
	};

	const u8 ExpectedOutput[XTRNGPSV_GEN_LEN_BYTES] = {
			0xEBU, 0xF6U, 0x97U, 0x6EU, 0x7EU, 0xF0U, 0x4AU, 0x34U,
			0xEEU, 0xF6U, 0xA8U, 0x1AU, 0x67U, 0x33U, 0xF2U, 0x3CU,
			0x27U, 0x24U, 0x7AU, 0x6DU, 0x0EU, 0x12U, 0xD7U, 0x42U,
			0x73U, 0xEDU, 0xDDU, 0x3FU, 0x07U, 0x23U, 0xD8U, 0x52U,
	};

	const u8 GuardPattern[KAT_GUARD_LEN_BYTES] = {
			0x2FU, 0x1DU, 0xC4U, 0x86U, 0x53U, 0xE0U, 0x7AU, 0x9BU,
			0x64U, 0xB1U, 0x08U, 0xDFU, 0x35U, 0x72U, 0xACU, 0x91U,
	};

	u32 Index;
	u8 CopyEvidence = 0U;
	u8 RandBufOut[XTRNGPSV_GEN_LEN_BYTES] = {0};
	u8 RandBufGuarded[XTRNGPSV_GEN_LEN_BYTES + KAT_GUARD_LEN_BYTES] = {0};

	if (InstancePtr == NULL) {
		Status = (s32)XTRNGPSV_ERROR_INVALID_PARAM;
		goto END;
	}

	Status = Xil_SMemSet((u8*)&UsrCfgTests, (u32)sizeof(UsrCfgTests), 0U, (u32)sizeof(UsrCfgTests));
	if (Status != XTRNGPSV_SUCCESS) {
		goto SET_ERR;
	}

	/** - Populate user config parameters of TRNGPSV driver */

	UsrCfgTests.Mode = XTRNGPSV_DRNG;
	UsrCfgTests.SeedLife = KAT_SEEDLIFE;
	UsrCfgTests.PredResistanceEn = XTRNGPSV_FALSE;
	UsrCfgTests.DFDisable = XTRNGPSV_FALSE;
	UsrCfgTests.DFLenMul = KAT_DFLENMUL;
	UsrCfgTests.PersStrPresent = XTRNGPSV_TRUE;
	UsrCfgTests.InitSeedPresent = XTRNGPSV_TRUE;

	Status = Xil_SMemCpy(&UsrCfgTests.InitSeed,
			(UsrCfgTests.DFLenMul + 1U) * BYTES_PER_BLOCK, ExtSeed,
			(u32)sizeof(ExtSeed), (u32)sizeof(ExtSeed));

	if (Status != XTRNGPSV_SUCCESS) {
		Status = (s32)XTRNGPSV_ERROR_USRCFG_CPY_KAT;
		goto SET_ERR;
	}

	if (UsrCfgTests.PersStrPresent == XTRNGPSV_TRUE) {
		Status = Xil_SMemCpy(UsrCfgTests.PersString,
				(u32)sizeof(UsrCfgTests.PersString), PersString,
				(u32)sizeof(PersString), (u32)sizeof(PersString));
		if (Status != XTRNGPSV_SUCCESS) {
			Status = (s32)XTRNGPSV_ERROR_USRCFG_CPY_KAT;
			goto SET_ERR;
		}
	}

	/** - Invoke Instantiate (which includes reseed), (explicit) Reseed and Generate operations */

	Status = XTrngpsv_Instantiate(InstancePtr, &UsrCfgTests);
	if (Status != XTRNGPSV_SUCCESS) {
		goto SET_ERR;
	}

	Status = XTrngpsv_Reseed(InstancePtr, ExtReSeed, KAT_DFLENMUL);
	if (Status != XTRNGPSV_SUCCESS) {
		goto SET_ERR;
	}

	Status = XTrngpsv_Generate(InstancePtr, RandBufOut, (u32)sizeof(RandBufOut),
			XTRNGPSV_FALSE);
	if (Status != XTRNGPSV_SUCCESS) {
		goto SET_ERR;
	}

	/**
	 * - If the generated Random data doesn't match with reference, error out.
	 */
	Result = Xil_SMemCmp(RandBufOut, XTRNGPSV_GEN_LEN_BYTES, ExpectedOutput,
				XTRNGPSV_GEN_LEN_BYTES, XTRNGPSV_GEN_LEN_BYTES);
	if (Result != XTRNGPSV_SUCCESS) {
		Status = (s32)XTRNGPSV_ERROR_KAT_MISMATCH;
		goto SET_ERR;
	}

	/** - Uninstantiate driver instance after the DRNG mode KAT. */
	Status = XTrngpsv_Uninstantiate(InstancePtr);
	if (Status != XTRNGPSV_SUCCESS) {
		goto SET_ERR;
	}

	/**
	 * - Generate in PTRNG mode with DF follows a separate path in which the DF output is
	 *   copied to the caller buffer. Random data of this mode is not reproducible, so
	 *   place a known pattern right after the requested bytes and check that the Generate
	 *   operation leaves it untouched.
	 */
	Status = Xil_SMemSet((u8*)&UsrCfgTests, (u32)sizeof(UsrCfgTests), 0U, (u32)sizeof(UsrCfgTests));
	if (Status != XTRNGPSV_SUCCESS) {
		goto SET_ERR;
	}

	UsrCfgTests.Mode = XTRNGPSV_PTRNG;
	UsrCfgTests.SeedLife = 0U;
	UsrCfgTests.PredResistanceEn = XTRNGPSV_FALSE;
	UsrCfgTests.DFDisable = XTRNGPSV_FALSE;
	UsrCfgTests.DFLenMul = KAT_DFLENMUL;
	UsrCfgTests.PersStrPresent = XTRNGPSV_FALSE;
	UsrCfgTests.InitSeedPresent = XTRNGPSV_FALSE;

	Status = Xil_SMemCpy(&RandBufGuarded[XTRNGPSV_GEN_LEN_BYTES], KAT_GUARD_LEN_BYTES,
			GuardPattern, (u32)sizeof(GuardPattern), (u32)sizeof(GuardPattern));
	if (Status != XTRNGPSV_SUCCESS) {
		goto SET_ERR;
	}

	Status = XTrngpsv_Instantiate(InstancePtr, &UsrCfgTests);
	if (Status != XTRNGPSV_SUCCESS) {
		goto SET_ERR;
	}

	Status = XTrngpsv_Generate(InstancePtr, RandBufGuarded, XTRNGPSV_GEN_LEN_BYTES,
			XTRNGPSV_FALSE);
	if (Status != XTRNGPSV_SUCCESS) {
		goto SET_ERR;
	}

	/**
	 * - The copy out of the DF output is the only write to the caller buffer in this
	 *   mode, so error out if the requested bytes were left untouched.
	 */
	for (Index = 0U; Index < XTRNGPSV_GEN_LEN_BYTES; Index++) {
		CopyEvidence |= RandBufGuarded[Index];
	}

	if (CopyEvidence == 0U) {
		Status = (s32)XTRNGPSV_ERROR_KAT_MISMATCH;
		goto SET_ERR;
	}

	/**
	 * - If the bytes beyond the requested random data got modified, error out.
	 */
	Result = XTRNGPSV_FAILURE;
	Result = Xil_SMemCmp(&RandBufGuarded[XTRNGPSV_GEN_LEN_BYTES], KAT_GUARD_LEN_BYTES,
			GuardPattern, (u32)sizeof(GuardPattern), (u32)sizeof(GuardPattern));
	if (Result != XTRNGPSV_SUCCESS) {
		Status = (s32)XTRNGPSV_ERROR_KAT_MISMATCH;
		goto SET_ERR;
	}

	/** - Uninstantiate driver instance after KAT */
	Status = XTrngpsv_Uninstantiate(InstancePtr);

SET_ERR:
	/** - Clear the random data left in the local buffer of the PTRNG mode test. */
	ClrStatus = Xil_SecureZeroize(RandBufGuarded, (u32)sizeof(RandBufGuarded));
	if (Status == XTRNGPSV_SUCCESS) {
		Status = ClrStatus;
	}

	if (Status != XTRNGPSV_SUCCESS) {
		InstancePtr->State = XTRNGPSV_ERROR;
	}

END:
	return Status;
}
/** @} */
