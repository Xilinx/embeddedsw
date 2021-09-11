/**************************************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xtrngpsv_tests.c
 * @addtogroup trngpsv_v1_0
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
 *
 * </pre>
 *
 **************************************************************************************************/

/*************************************** Include Files *******************************************/

#include "xtrngpsv.h"

/************************************ Constant Definitions ***************************************/

#define HEALTH_TEST_SEEDLIFE	10U
#define HEALTH_TEST_DFLENMUL	7U
#define KAT_SEEDLIFE	5U
#define KAT_DFLENMUL	2U

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
	s32 Status = XTRNGPSV_FAILURE;

	XTrngpsv_UsrCfg UsrCfgTests;

	if (InstancePtr == NULL) {
		Status = (s32)XTRNGPSV_ERROR_INVALID_PARAM;
		goto END;
	}

	(void)memset((u8*)&UsrCfgTests, 0, sizeof(UsrCfgTests));

	/* Populate user config parameters of TRNGPSV driver */

	UsrCfgTests.Mode = XTRNGPSV_HRNG;
	UsrCfgTests.SeedLife = HEALTH_TEST_SEEDLIFE;
	UsrCfgTests.PredResistanceEn = XTRNGPSV_FALSE;
	UsrCfgTests.DFDisable = XTRNGPSV_FALSE;
	UsrCfgTests.DFLenMul = HEALTH_TEST_DFLENMUL;
	UsrCfgTests.PersStrPresent = XTRNGPSV_FALSE;
	UsrCfgTests.InitSeedPresent = XTRNGPSV_FALSE;

	/* Invoke Instantiate (which includes reseed) operation */
	Status = XTrngpsv_Instantiate(InstancePtr, &UsrCfgTests);
	if (Status != XTRNGPSV_SUCCESS) {
		goto SET_ERR;
	}

	/* CTF is not being monitored explicitly here since that is done as part of Reseeding
	 * during Instantiation
	 */

	/* Uninstantiate driver instance after Health Tests */
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
 * in DRNG mode. If KAT fails, driver has to be put in Error state.
 *
 * @param	InstancePtr is a pointer to the XTrngpsv instance to be worked on.
 *
 * @return
 *		- XTRNGPSV_SUCCESS if KAT is successful.
 *		- XTRNGPSV_ERROR_INVALID_PARAM if invalid parameter passed to this function.
 *		- XTRNGPSV_ERROR_USRCFG_CPY_KAT if copy of seed/personalization string failed.
 *		- XTRNGPSV_ERROR_KAT_MISMATCH if KAT result doesn't match with expected output (OR)
 *		other error codes from the called functions as defined in XTrngpsv_ErrorCodes.
 *
 *************************************************************************************************/
s32 XTrngpsv_RunKAT(XTrngpsv *InstancePtr)
{

	s32 Status = XTRNGPSV_FAILURE;
	s32 Result;
	XTrngpsv_UsrCfg UsrCfgTests;

	const u8 ExtSeed[XTRNGPSV_SEED_LEN_BYTES] = {
			0x3BU, 0xC3U, 0xEDU, 0x64U, 0xF4U, 0x80U, 0x1CU, 0xC7U,
			0x14U, 0xCCU, 0x35U, 0xEDU, 0x57U, 0x01U, 0x2AU, 0xE4U,
			0xBCU, 0xEFU, 0xDEU, 0xF6U, 0x7CU, 0x46U, 0xA6U, 0x34U,
			0xC6U, 0x79U, 0xE8U, 0x91U, 0x5DU, 0xB1U, 0xDBU, 0xA7U,
			0x49U, 0xA5U, 0xBBU, 0x4FU, 0xEDU, 0x30U, 0xB3U, 0x7BU,
			0xA9U, 0x8BU, 0xF5U, 0x56U, 0x4DU, 0x40U, 0x18U, 0x9FU,
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
			0x91U, 0x9AU, 0x6BU, 0x99U, 0xD5U, 0xBCU, 0x2CU, 0x11U,
			0x5FU, 0x3AU, 0xFCU, 0x0BU, 0x0EU, 0x7BU, 0xC7U, 0x69U,
			0x4DU, 0xE1U, 0xE5U, 0xFEU, 0x59U, 0x9EU, 0xAAU, 0x41U,
			0xD3U, 0x48U, 0xFDU, 0x3DU, 0xD2U, 0xC4U, 0x50U, 0x1EU,
	};

	u8 RandBufOut[XTRNGPSV_GEN_LEN_BYTES];

	if (InstancePtr == NULL) {
		Status = (s32)XTRNGPSV_ERROR_INVALID_PARAM;
		goto END;
	}

	(void)memset((u8*)&UsrCfgTests, 0, sizeof(UsrCfgTests));

	/* Populate user config parameters of TRNGPSV driver */

	UsrCfgTests.Mode = XTRNGPSV_DRNG;
	UsrCfgTests.SeedLife = KAT_SEEDLIFE;
	UsrCfgTests.PredResistanceEn = XTRNGPSV_FALSE;
	UsrCfgTests.DFDisable = XTRNGPSV_FALSE;
	UsrCfgTests.DFLenMul = KAT_DFLENMUL;
	UsrCfgTests.PersStrPresent = XTRNGPSV_TRUE;
	UsrCfgTests.InitSeedPresent = XTRNGPSV_TRUE;

	Status = Xil_SecureMemCpy(&UsrCfgTests.InitSeed,
			(UsrCfgTests.DFLenMul + 1U) * BYTES_PER_BLOCK, ExtSeed,
			(u32)sizeof(ExtSeed));

	if (Status != XTRNGPSV_SUCCESS) {
		Status = (s32)XTRNGPSV_ERROR_USRCFG_CPY_KAT;
		goto SET_ERR;
	}

	if (UsrCfgTests.PersStrPresent == XTRNGPSV_TRUE) {
		Status = Xil_SecureMemCpy(UsrCfgTests.PersString,
				(u32)sizeof(UsrCfgTests.PersString), PersString,
				(u32)sizeof(PersString));
		if (Status != XTRNGPSV_SUCCESS) {
			Status = (s32)XTRNGPSV_ERROR_USRCFG_CPY_KAT;
			goto SET_ERR;
		}
	}

	/* Invoke Instantiate (which includes reseed) and Generate operations */

	Status = XTrngpsv_Instantiate(InstancePtr, &UsrCfgTests);
	if (Status != XTRNGPSV_SUCCESS) {
		goto SET_ERR;
	}

	Status = XTrngpsv_Generate(InstancePtr, RandBufOut, (u32)sizeof(RandBufOut),
			XTRNGPSV_FALSE);
	if (Status != XTRNGPSV_SUCCESS) {
		goto SET_ERR;
	}

	/* If the generated Randam data doesn't match with reference,
	 * error out
	 */
	Result = Xil_MemCmp(RandBufOut, ExpectedOutput, XTRNGPSV_GEN_LEN_BYTES);
	if (Result != XTRNGPSV_SUCCESS) {
		Status = (s32)XTRNGPSV_ERROR_KAT_MISMATCH;
		goto SET_ERR;
	}

	/* Uninstantiate driver instance after KAT */
	Status = XTrngpsv_Uninstantiate(InstancePtr);

SET_ERR:
	if (Status != XTRNGPSV_SUCCESS) {
		InstancePtr->State = XTRNGPSV_ERROR;
	}

END:
	return Status;
}
/** @} */
