/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_trng.h
* This file contains trng core hardware definitions of VersalNet.
* @addtogroup Overview
* @{
*
* This header file contains structure definitions, function declarations and macros
* to define TRNG Hardware state
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 5.0   kpt  05/05/22 Initial release
*       dc   07/12/22 Corrected comments
*       kpt  07/24/22 Moved KAT related code to xsecure_kat_plat.c
*
* </pre>
*
* @endcond
******************************************************************************/
#ifndef XSECURE_TRNG_H
#define XSECURE_TRNG_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"

/************************** Constant Definitions *****************************/
#define XSECURE_TRNG_DEFAULT_SEED_LIFE		256U	/**< Default seed life */
#define XSECURE_TRNG_PERS_STRING_LEN_IN_WORDS	12U	/**< Personalization string length in words */
#define XSECURE_TRNG_PERS_STRING_LEN_IN_BYTES	48U	/**< Personalization string length in bytes */
#define XSECURE_TRNG_SEC_STRENGTH_IN_BYTES	32U	/**< security strength in Bytes */

#if !defined(XSECURE_TRNG_USER_CFG_SEED_LIFE)
#define XSECURE_TRNG_USER_CFG_SEED_LIFE 256U
#endif

#if !defined(XSECURE_TRNG_USER_CFG_DF_LENGTH)
#define XSECURE_TRNG_USER_CFG_DF_LENGTH 7U
#endif

#if !defined(XSECURE_TRNG_USER_CFG_ADAPT_TEST_CUTOFF)
#define XSECURE_TRNG_USER_CFG_ADAPT_TEST_CUTOFF 612U
#endif

#if !defined(XSECURE_TRNG_USER_CFG_REP_TEST_CUTOFF)
#define XSECURE_TRNG_USER_CFG_REP_TEST_CUTOFF 33U
#endif

/**************************** Type Definitions *******************************/
typedef enum {
	XSECURE_TRNG_DRNG_MODE = 1,
	XSECURE_TRNG_PTRNG_MODE,
	XSECURE_TRNG_HRNG_MODE
} XSecure_TrngMode;

typedef struct {
	XSecure_TrngMode Mode;
	u8 DFLength;
	u16 AdaptPropTestCutoff;
	u16 RepCountTestCutoff;
	u32 SeedLife;
} XSecure_TrngUserConfig;

typedef struct {
	u32 ElapsedSeedLife;
} XSecure_TrngStatus;

typedef enum {
	XSECURE_TRNG_UNHEALTHY = 0,
	XSECURE_TRNG_HEALTHY,
	XSECURE_TRNG_CATASTROPHIC,
	XSECURE_TRNG_ERROR,
	XSECURE_TRNG_STARTUP_TEST
} XSecure_TrngErrorState;

typedef enum {
	XSECURE_TRNG_UNINITIALIZED_STATE = 0,	/**< Default state */
	XSECURE_TRNG_INSTANTIATE_STATE,		/**< Instantiate state */
	XSECURE_TRNG_RESEED_STATE,		/**< Reseed state */
	XSECURE_TRNG_GENERATE_STATE		/**< Generate state */
} XSecure_TrngTrngState;

typedef struct {
	XSecure_TrngUserConfig UserCfg;
	XSecure_TrngStatus TrngStats;
	XSecure_TrngErrorState ErrorState;
	XSecure_TrngTrngState State;
} XSecure_TrngInstance;

/************************** Function Prototypes ******************************/
int XSecure_TrngInstantiate(XSecure_TrngInstance *InstancePtr, const u8 *Seed, u32 SeedLength, const u8 *PersStr,
			const XSecure_TrngUserConfig *UserCfg);
int XSecure_TrngReseed(XSecure_TrngInstance *InstancePtr, const u8 *Seed, u8 DLen);
int XSecure_TrngGenerate(XSecure_TrngInstance *InstancePtr, u8 *RandBuf, u32 RandBufSize);
int XSecure_TrngUninstantiate(XSecure_TrngInstance *InstancePtr);
XSecure_TrngInstance *XSecure_GetTrngInstance(void);
int XSecure_TrngInitNCfgHrngMode(void);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
