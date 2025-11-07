/***************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xsecure_trng.h
*
* This file contains function declarations to use trngpsv.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 5.5   tvp  05/13/25 Initial release
*       sd   11/07/25 Update TRNG function name and return logic to return true
*                     when TRNG is uninitialized
*
* </pre>
*
***************************************************************************************************/
/**
* @addtogroup xsecure_trng_server_apis Xilsecure TRNG Server APIs
* @{
*/
#ifndef XSECURE_TRNG_H
#define XSECURE_TRNG_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files ********************************************/
#include "xil_types.h"
#include "xtrngpsv.h"
#include "stdbool.h"
#include "xstatus.h"

/************************************ Constant Definitions ****************************************/
#define XSECURE_TRNG_SEC_STRENGTH_IN_BYTES 		XTRNGPSV_SEC_STRENGTH_BYTES
							/**< Security strength in bytes for
							 * XTRNGPSV*/

#define XSECURE_TRNG_PER_STRING_LEN_IN_BYTES 		XTRNGPSV_PERS_STR_LEN_BYTES
							/**< Personalization string length in bytes
							 * for XTRNGPSV */

#if !defined(XSECURE_TRNG_USER_CFG_SEED_LIFE)
#define XSECURE_TRNG_USER_CFG_SEED_LIFE			(256U)	/**< User config Seed life */
#endif

/************************************* Macro Definitions ******************************************/
#define XSecure_Uninstantiate				XTrngpsv_Uninstantiate
							/**< Redefine XTrngpsv_Uninstantiate as
							 * XSecure_Uninstantiate for compatibility*/

/************************************ Variable Definitions ****************************************/

/************************************** Type Definitions ******************************************/
typedef XTrngpsv XSecure_TrngInstance;			 /**< typedef XTrngpsv as
							  * XSecure_TrngInstance for compatibility */

/* This typedef contains mode information on which TRNG operates */
typedef enum {
	XSECURE_TRNG_HRNG_MODE = 0U,	/**< HRNG mode for TRNG */
	XSECURE_TRNG_DRNG_MODE,		/**< DRNG mode for TRNG */
	XSECURE_TRNG_PTRNG_MODE		/**< PTRNG mode for TRNG */
} XSecureTrng_Mode;

/************************************ Function Prototypes *****************************************/
XSecure_TrngInstance *XSecure_GetTrngInstance(void);
int XSecure_TrngInitNCfgMode(int XSecureTrngMode, u8 *Seed, u32 SeedLength, u8 *PersStr);
int XSecure_PreOperationalSelfTests(XSecure_TrngInstance *TrngInstance);

/************************************ Function Definitions ****************************************/

/**************************************************************************************************/
/**
 * @brief	This is a wrapper function to check trngpsv initialization status.
 *
 * @param	TrngInstance Pointer to XTrngpsv instance.
 *
 * @return
 *		 - XST_SUCCESS  If TRNGPSV is not initialized.
 *		 - XST_FAILURE  If TRNGPSV is initialized.
 *
 **************************************************************************************************/
static inline u8 XSecure_TrngIsUninitialized(XSecure_TrngInstance *TrngInstance)
{
	return (TrngInstance->State == XTRNGPSV_UNINITIALIZED);
}

/**************************************************************************************************/
/**
 * @brief	This is a wrapper function to check trngpsv health status.
 *
 * @param	TrngInstance Pointer to XTrngpsv instance.
 *
 * @return
 *		 - XST_SUCCESS  If status of TRNGPSV is healthy.
 *		 - XST_FAILURE  If status of TRNGPSV is not healthy.
 *
 **************************************************************************************************/
static inline u8 XSecure_TrngIsHealthy(XSecure_TrngInstance *TrngInstance)
{
	return (TrngInstance->State == XTRNGPSV_HEALTHY);
}

/**************************************************************************************************/
/**
 * @brief	This wrapper to generate random bits.
 *
 * @param	TrngInstance	Pointer to the XTrngpsv instance.
 * @param	RandBuf		Pointer to memory address where generated random data will be stored
 * 				and the memory address should be word aligned.
 * @param	RandBufSize	Size of the buffer to which RandBufPtr points to and it should be
 * 				always greater than or equal to XTRNGPSV_SEC_STRENGTH_BYTES.
 * @param	PredResistance	Flag that controls Generate level Prediction Resistance. When
 * 				enabled, it mandates fresh seed for every Generate operation.
 *
 * @return
 *		- XST_SUCCESS On success.
 *		- ErrorCode On failure.
 *
 **************************************************************************************************/
static inline int XSecure_TrngGenerate(XSecure_TrngInstance *TrngInstance, u8 *RandBuf,
		u32 RandBufSize, u8 PredResistance)
{
	volatile int Status = XST_FAILURE;
	u8 TmpRandBuf[XTRNGPSV_SEC_STRENGTH_BYTES];

	if (RandBufSize < XTRNGPSV_SEC_STRENGTH_BYTES) {
		Status = XTrngpsv_Generate(TrngInstance, (u8 *)&TmpRandBuf,
					   XTRNGPSV_SEC_STRENGTH_BYTES, PredResistance);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = Xil_SMemCpy(RandBuf, RandBufSize, TmpRandBuf, XTRNGPSV_SEC_STRENGTH_BYTES,
				     RandBufSize);
	} else {
		Status = XTrngpsv_Generate(TrngInstance, RandBuf, RandBufSize, PredResistance);
	}
END:
	return Status;
}

#ifdef __cplusplus
}
#endif
#endif /** XSECURE_TRNG_H */
/** @} */
