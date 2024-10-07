/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xtrng_tests.c
 *
 * This file contains Known Answer Tests and Health Tests for TRNG component.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   05/20/24 Initial release
 *       ma   07/26/24 Added platform check to skip DRBG KAT on QEMU
 *       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xtrng_server_apis TRNG Server APIs
* @{
*/
/*************************************** Include Files *******************************************/
#include "xtrng.h"
#include "xasufw_status.h"
#include "xil_util.h"
#include "xasufw_hw.h"

/************************************ Constant Definitions ***************************************/
#define XTRNG_KAT_DEFAULT_DF_lENGTH 7U /**<Default Derivative function length of TRNG KAT*/
#define XTRNG_KAT_DEFAULT_SEED_LIFE 2U /**<Default seed life of TRNG KAT*/
#define XTRNG_KAT_SEED_LEN_IN_BYTES 128U /**<Seed length in bytes of TRNG KAT*/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XTrng_HealthTest(XTrng *InstancePtr);

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function runs DRBG self test i.e DRBG full cycle Instantiate+Reseed,
 * 		Reseed and Generate.
 *
 * @param	InstancePtr	Pointer to the TRNG instance.
 *
 * @return
 * 	- Upon successfully running DRBG KAT, it returns XASUFW_SUCCESS.
 * 	- XASUFW_TRNG_KAT_FAILED_ERROR, if generated random number is mismatched with expected.
 * 	- XASUFW_FAILURE, if there is any failure.
 *
 *************************************************************************************************/
s32 XTrng_DrbgKat(XTrng *InstancePtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XTrng_UserConfig UsrCfg;
	u8 RandBuf[XTRNG_SEC_STRENGTH_IN_BYTES];

	const u8 ExtSeed[XTRNG_KAT_SEED_LEN_IN_BYTES] = {
		0x3BU, 0xC3U, 0xEDU, 0x64U, 0xF4U, 0x80U, 0x1CU, 0xC7U,
		0x14U, 0xCCU, 0x35U, 0xEDU, 0x57U, 0x01U, 0x2AU, 0xE4U,
		0xBCU, 0xEFU, 0xDEU, 0xF6U, 0x7CU, 0x46U, 0xA6U, 0x34U,
		0xC6U, 0x79U, 0xE8U, 0x91U, 0x5DU, 0xB1U, 0xDBU, 0xA7U,
		0x49U, 0xA5U, 0xBBU, 0x4FU, 0xEDU, 0x30U, 0xB3U, 0x7BU,
		0xA9U, 0x8BU, 0xF5U, 0x56U, 0x4DU, 0x40U, 0x18U, 0x9FU,
		0x66U, 0x4EU, 0x39U, 0xC0U, 0x60U, 0xC8U, 0x8EU, 0xF4U,
		0x1CU, 0xB9U, 0x9DU, 0x7BU, 0x97U, 0x8BU, 0x69U, 0x62U,
		0x45U, 0x0CU, 0xD4U, 0x85U, 0xFCU, 0xDCU, 0x5AU, 0x2BU,
		0xFDU, 0xABU, 0x92U, 0x4AU, 0x12U, 0x52U, 0x7DU, 0x45U,
		0xD2U, 0x61U, 0x0AU, 0x06U, 0x74U, 0xA7U, 0x88U, 0x36U,
		0x4BU, 0xA2U, 0x65U, 0xEEU, 0x71U, 0x0BU, 0x5AU, 0x4EU,
		0x33U, 0xB2U, 0x7AU, 0x2EU, 0xC0U, 0xA6U, 0xF2U, 0x7DU,
		0xBDU, 0x7DU, 0xDFU, 0x07U, 0xBBU, 0xE2U, 0x86U, 0xFFU,
		0xF0U, 0x8EU, 0xA4U, 0xB1U, 0x46U, 0xDBU, 0xF7U, 0x8CU,
		0x3CU, 0x62U, 0x4DU, 0xF0U, 0x51U, 0x50U, 0xE7U, 0x85U
	};

	const u8 ReseedEntropy[XTRNG_KAT_SEED_LEN_IN_BYTES] = {
		0xDFU, 0x5EU, 0x4DU, 0x4FU, 0x38U, 0x9EU, 0x2AU, 0x3EU,
		0xF2U, 0xABU, 0x46U, 0xE3U, 0xA0U, 0x26U, 0x77U, 0x84U,
		0x0BU, 0x9DU, 0x29U, 0xB0U, 0x5DU, 0xCEU, 0xC8U, 0xC3U,
		0xF9U, 0x4DU, 0x32U, 0xF7U, 0xBAU, 0x6FU, 0xA3U, 0xB5U,
		0x35U, 0xCBU, 0xC7U, 0x5CU, 0x62U, 0x48U, 0x01U, 0x65U,
		0x3AU, 0xAAU, 0x34U, 0x2DU, 0x89U, 0x6EU, 0xEFU, 0x6FU,
		0x69U, 0x96U, 0xE7U, 0x84U, 0xDAU, 0xEFU, 0x4EU, 0xBEU,
		0x27U, 0x4EU, 0x9FU, 0x88U, 0xB1U, 0xA0U, 0x7FU, 0x83U,
		0xDBU, 0x4AU, 0xA9U, 0x42U, 0x01U, 0xF1U, 0x84U, 0x71U,
		0xA9U, 0xEFU, 0xB9U, 0xE8U, 0x7FU, 0x81U, 0xC7U, 0xC1U,
		0x6CU, 0x5EU, 0xACU, 0x00U, 0x47U, 0x34U, 0xA1U, 0x75U,
		0xC0U, 0xE8U, 0x7FU, 0x48U, 0x00U, 0x45U, 0xC9U, 0xE9U,
		0x41U, 0xE3U, 0x8DU, 0xD8U, 0x4AU, 0x63U, 0xC4U, 0x94U,
		0x77U, 0x59U, 0xD9U, 0x50U, 0x2AU, 0x1DU, 0x4CU, 0x47U,
		0x64U, 0xA6U, 0x66U, 0x60U, 0x16U, 0xE7U, 0x29U, 0xC0U,
		0xB1U, 0xCFU, 0x3BU, 0x3FU, 0x54U, 0x49U, 0x31U, 0xD4U
	};

	const u8 PersString[XTRNG_PERS_STRING_LEN_IN_BYTES] = {
		0xB2U, 0x80U, 0x7EU, 0x4CU, 0xD0U, 0xE4U, 0xE2U, 0xA9U,
		0x2FU, 0x1FU, 0x5DU, 0xC1U, 0xA2U, 0x1FU, 0x40U, 0xFCU,
		0x1FU, 0x24U, 0x5DU, 0x42U, 0x61U, 0x80U, 0xE6U, 0xE9U,
		0x71U, 0x05U, 0x17U, 0x5BU, 0xAFU, 0x70U, 0x30U, 0x18U,
		0xBCU, 0x23U, 0x18U, 0x15U, 0xCBU, 0xB8U, 0xA6U, 0x3EU,
		0x83U, 0xB8U, 0x4AU, 0xFEU, 0x38U, 0xFCU, 0x25U, 0x87U
	};

	const u8 ExpectedOutput[XTRNG_SEC_STRENGTH_IN_BYTES] = {
		0xEEU, 0xA7U, 0x5BU, 0xB6U, 0x2BU, 0x97U, 0xF0U, 0xC0U,
		0x0FU, 0xD6U, 0xABU, 0x13U, 0x00U, 0x87U, 0x7EU, 0xF4U,
		0x00U, 0x7FU, 0xD7U, 0x56U, 0xFEU, 0xE5U, 0xDFU, 0xA6U,
		0x55U, 0x5BU, 0xB2U, 0x86U, 0xDDU, 0x81U, 0x73U, 0xB2U
	};

	Status = Xil_SMemSet(&UsrCfg, sizeof(XTrng_UserConfig), 0U, sizeof(XTrng_UserConfig));
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	UsrCfg.DFLength = XTRNG_KAT_DEFAULT_DF_lENGTH;
	UsrCfg.Mode = XTRNG_DRBG_MODE;
	UsrCfg.SeedLife = XTRNG_KAT_DEFAULT_SEED_LIFE;
	UsrCfg.IsBlocking = XASU_TRUE;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XTrng_Instantiate(InstancePtr, ExtSeed, XTRNG_KAT_SEED_LEN_IN_BYTES, PersString,
				   &UsrCfg);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XTrng_Reseed(InstancePtr, ReseedEntropy, UsrCfg.DFLength);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XTrng_Generate(InstancePtr, RandBuf, XTRNG_SEC_STRENGTH_IN_BYTES, XASU_TRUE);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCmp(ExpectedOutput, XTRNG_SEC_STRENGTH_IN_BYTES, RandBuf,
			     XTRNG_SEC_STRENGTH_IN_BYTES, XTRNG_SEC_STRENGTH_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_TRNG_KAT_FAILED_ERROR;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XTrng_Uninstantiate(InstancePtr);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function runs preoperational self tests and updates TRNG error state
 *
 * @param	InstancePtr	Pointer to the TRNG instance.
 *
 * @return
 * 	- Upon success, it returns XASUFW_SUCCESS.
 * 	- XASUFW_FAILURE, if there is any failure.
 *
 *************************************************************************************************/
s32 XTrng_PreOperationalSelfTests(XTrng *InstancePtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/* Reset the TRNG state */
	Status = XTrng_Uninstantiate(InstancePtr);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if (XASUFW_PLATFORM != PMC_TAP_VERSION_PLATFORM_QEMU) {
		Status = XTrng_DrbgKat(InstancePtr);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XTrng_HealthTest(InstancePtr);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function runs health test on TRNG core.
 *
 * @param	InstancePtr	Pointer to the TRNG instance.
 *
 * @return
 * 	- Upon successful completion of health test on TRNG core, it returns XASUFW_SUCCESS.
 * 	- XASUFW_FAILURE, if there is any failure.
 *
 *************************************************************************************************/
static s32 XTrng_HealthTest(XTrng *InstancePtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XTrng_UserConfig UsrCfg;

	UsrCfg.Mode = XTRNG_HRNG_MODE;
	UsrCfg.AdaptPropTestCutoff = XTRNG_USER_CFG_ADAPT_TEST_CUTOFF;
	UsrCfg.RepCountTestCutoff = XTRNG_USER_CFG_REP_TEST_CUTOFF;
	UsrCfg.DFLength = XTRNG_USER_CFG_DF_LENGTH;
	UsrCfg.SeedLife = XTRNG_USER_CFG_SEED_LIFE;
	UsrCfg.IsBlocking = XASU_TRUE;

	Status = XTrng_Instantiate(InstancePtr, NULL, 0U, NULL, &UsrCfg);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XTrng_Uninstantiate(InstancePtr);

END:
	return Status;
}
/** @} */
