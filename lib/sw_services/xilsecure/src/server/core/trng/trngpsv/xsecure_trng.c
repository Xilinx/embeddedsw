/***************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xsecure_trng.c
* This file contains function declaration to get random number.
*
* This header file contains function declaration to get random number.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 5.5   tvp  05/13/25 Initial release
*       sd   11/07/25 Add check to ensure TRNG is initialized before uninitializing
*
* </pre>
*
***************************************************************************************************/
/**
* @addtogroup xsecure_trng_server_apis Xilsecure TRNG Server APIs
* @{
*/

/*************************************** Include Files ********************************************/
#include "xsecure_plat.h"
#include "xsecure_trng.h"
#include "xsecure_utils.h"
#include "xtrngpsv.h"

/************************************ Constant Definitions ****************************************/
#define XSECURE_ECC_TRNG_DF_LENGTH	(2U) /**< Default length of xilsecure ecc true random number
					       generator*/
#define XTRNGPSV_USER_CFG_DF_LENGTH	(7U) /**< Default multiplier used to determine num of bits
					       on the input of the DF construct */


/************************************ Variable Definitions ****************************************/

/************************************ Function Prototypes *****************************************/

/************************************ Function Definitions ****************************************/

/**************************************************************************************************/
/**
 * @brief	This function provides the pointer to the common trng instance
 *
 * @return
 *		 - Pointer to the XSecure_TrngInstance instance
 *
 **************************************************************************************************/
XSecure_TrngInstance *XSecure_GetTrngInstance(void)
{
	static XSecure_TrngInstance TrngInstance = {0U};

	return &TrngInstance;
}

/*************************************************************************************************/
/**
 * @brief	This function initialize and configures the TRNG into DRNG/HRNG mode of operation.
 *
 * @return
 *		 - XST_SUCCESS  Upon success.
 *		 - XST_FAILURE  On failure.
 *
 **************************************************************************************************/
int XSecure_TrngInitNCfgMode(int XSecureTrngMode, u8 *Seed, u32 SeedLength, u8 *PersStr)
{
	XSecure_TrngInstance *TrngInstance = XSecure_GetTrngInstance();
	volatile int Status = XST_FAILURE;
	XTrngpsv_UsrCfg UsrCfg;

	Status = Xil_SMemSet(&UsrCfg, sizeof(XTrngpsv_UsrCfg), 0U, sizeof(XTrngpsv_UsrCfg));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (TrngInstance->State != XTRNGPSV_UNINITIALIZED) {
		Status = XSecure_Uninstantiate(TrngInstance);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	/* Initiate TRNG */
	UsrCfg.DFDisable = XTRNGPSV_FALSE;
	UsrCfg.PredResistanceEn = XTRNGPSV_FALSE;

	if (XSecureTrngMode == (int)XSECURE_TRNG_HRNG_MODE) {
		UsrCfg.SeedLife = XSECURE_TRNG_USER_CFG_SEED_LIFE;
		UsrCfg.Mode = XTRNGPSV_HRNG;
		UsrCfg.DFLenMul = XTRNGPSV_USER_CFG_DF_LENGTH;
	} else if (XSecureTrngMode == (int)XSECURE_TRNG_DRNG_MODE) {
		UsrCfg.SeedLife = XSECURE_TRNG_USER_CFG_SEED_LIFE;
		UsrCfg.Mode = XTRNGPSV_DRNG;
		UsrCfg.DFLenMul = XSECURE_ECC_TRNG_DF_LENGTH;
	} else {
		UsrCfg.SeedLife = 0U;
		UsrCfg.Mode = XTRNGPSV_PTRNG;
		UsrCfg.DFLenMul = XTRNGPSV_USER_CFG_DF_LENGTH;
	}
	if (SeedLength) {
		UsrCfg.InitSeedPresent = XTRNGPSV_TRUE;
		Status = Xil_SMemCpy(&UsrCfg.InitSeed, XTRNGPSV_SEED_LEN_BYTES, Seed, SeedLength,
				     SeedLength);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	} else {
		UsrCfg.InitSeedPresent = XTRNGPSV_FALSE;
	}
	if (PersStr != NULL) {
		UsrCfg.PersStrPresent = XTRNGPSV_TRUE;
		Status = Xil_SMemCpy(&UsrCfg.PersString, sizeof(UsrCfg.PersString), PersStr,
				     sizeof(UsrCfg.PersString), sizeof(UsrCfg.PersString));
		if (Status != XST_SUCCESS) {
			goto END;
		}
	} else {
		UsrCfg.PersStrPresent = XTRNGPSV_FALSE;
	}

	Status = XTrngpsv_Instantiate(TrngInstance, &UsrCfg);
	if (Status != XST_SUCCESS) {
		(void)XSecure_Uninstantiate(TrngInstance);
		goto END;

	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function runs preoperational self tests and updates TRNG error state
 *
 * @param	TrngInstance Pointer to XTrngpsv Instance.
 *
 * @return
 * 		- XST_SUCCESS On success
 * 		- Error Code On failure
 *
 **************************************************************************************************/
int XSecure_PreOperationalSelfTests(XSecure_TrngInstance *TrngInstance)
{
	volatile int Status = XST_FAILURE;
	XTrngpsv *InstancePtr = TrngInstance;

	/* Reset the TRNG state */
	if (!XSecure_TrngIsUninitialized(TrngInstance)){
		Status = XTrngpsv_Uninstantiate(InstancePtr);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	Status = XTrngpsv_RunKAT(InstancePtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XTrngpsv_RunHealthTest(InstancePtr);

END:
	return Status;
}
