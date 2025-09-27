/***************************************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xsecure_trng_versal_net.c
* This file implements function to use TRNGPSX
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
*
* </pre>
*
***************************************************************************************************/
/**
* @addtogroup xsecure_trng_server_apis Xilsecure TRNG Server APIs
* @{
*/

/*************************************** Include Files ********************************************/
#include "xtrngpsx.h"
#include "xsecure_trng.h"
#include "xsecure_utils.h"
#include "xsecure_plat.h"

/************************************ Constant Definitions ****************************************/
#define XSECURE_ECC_TRNG_DF_LENGTH			(2U) /**< Default length of xilsecure ecc
							       true random number generator*/

/************************************* Variable Definitions ***************************************/

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

/**************************************************************************************************/
/**
 * @brief	This function initialize and configures the TRNG into DRNG/HRNG mode of operation.
 *
 * @param	XSecureTrngMode	Trng Mode of operation
 * @param	Seed		Holds the address of Seed buffer
 * @param	SeedLength	Holds size Seed
 * @param	PersStr		Personalization string
 *
 * @return
 *		 - XST_SUCCESS  Upon success.
 *		 - XST_FAILURE  On failure.
 *
 **************************************************************************************************/
int XSecure_TrngInitNCfgMode(int XSecureTrngMode, u8 *Seed, u32 SeedLength, u8 *PersStr)
{
	volatile int Status = XST_FAILURE;
	XTrngpsx_UserConfig UsrCfg;
	XSecure_TrngInstance *TrngInstance = XSecure_GetTrngInstance();

	Status = Xil_SMemSet(&UsrCfg, sizeof(XTrngpsx_UserConfig), 0U, sizeof(XTrngpsx_UserConfig));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (TrngInstance->State != XTRNGPSX_UNINITIALIZED_STATE ) {
		Status = XSecure_Uninstantiate(TrngInstance);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		XSecure_UpdateTrngCryptoStatus(XSECURE_CLEAR_BIT);
	}

	/* Initiate TRNG */
	UsrCfg.SeedLife = XSECURE_TRNG_USER_CFG_SEED_LIFE ;
	UsrCfg.IsBlocking = FALSE;
	if (XSecureTrngMode == (int)XSECURE_TRNG_HRNG_MODE) {
		UsrCfg.Mode = XTRNGPSX_HRNG_MODE;
		UsrCfg.AdaptPropTestCutoff = XSECURE_TRNG_USER_CFG_ADAPT_TEST_CUTOFF;
		UsrCfg.RepCountTestCutoff = XSECURE_TRNG_USER_CFG_REP_TEST_CUTOFF;
		UsrCfg.DFLength = XSECURE_TRNG_USER_CFG_DF_LENGTH ;
	} else if (XSecureTrngMode == (int)XTRNGPSX_DRNG_MODE) {
		UsrCfg.Mode = XTRNGPSX_DRNG_MODE;
		UsrCfg.DFLength = XSECURE_ECC_TRNG_DF_LENGTH;
	} else {
		UsrCfg.Mode = XTRNGPSX_PTRNG_MODE;
		UsrCfg.DFLength = XTRNGPSX_USER_CFG_DF_LENGTH;
		UsrCfg.AdaptPropTestCutoff = XTRNGPSX_USER_CFG_ADAPT_TEST_CUTOFF;
		UsrCfg.RepCountTestCutoff = XTRNGPSX_USER_CFG_REP_TEST_CUTOFF;
	}

	Status = XTrngpsx_Instantiate(TrngInstance, Seed, SeedLength, PersStr, &UsrCfg);
	if (Status != XST_SUCCESS) {
		(void)XSecure_Uninstantiate(TrngInstance);
		XSecure_UpdateTrngCryptoStatus(XSECURE_CLEAR_BIT);
		goto END;
	}
	XSecure_UpdateTrngCryptoStatus(XSECURE_SET_BIT);

END:
	return Status;
}
