/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xtrng.h
 * @addtogroup Overview
 * @{
 *
 * This header file contains structure definitions, function declarations and macros for TRNG HW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   05/20/24 Initial release
 *       ma   07/26/24 Removed XTrng_DisableAutoProcMode API and updated TRNG to support PTRNG mode
 *
 * </pre>
 *
 *************************************************************************************************/

#ifndef XTRNG_H
#define XTRNG_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xtrng_hw.h"

/************************************ Constant Definitions ***************************************/
#define XTRNG_DEFAULT_SEED_LIFE				256U /**< Default seed life */
#define XTRNG_PERS_STRING_LEN_IN_WORDS		12U /**< Personalization string length in words */
#define XTRNG_PERS_STRING_LEN_IN_BYTES		48U /**< Personalization string length in bytes */
#define XTRNG_SEC_STRENGTH_IN_BYTES			32U /**< Security strength in Bytes */
#define XTRNG_SEC_STRENGTH_IN_WORDS			8U /**< Security strength in words */
#define XTRNG_USER_CFG_SEED_LIFE			256U /**< User config seed life */
#define XTRNG_USER_CFG_DF_LENGTH			7U /**< User config DF Length */
#define XTRNG_USER_CFG_ADAPT_TEST_CUTOFF	612U /**< Cut off value for adaptive count test */
#define XTRNG_USER_CFG_REP_TEST_CUTOFF		33U /**< Cut off value for repetitive count test */

/************************************** Type Definitions *****************************************/
typedef struct _XTrng XTrng;

typedef enum {
	XTRNG_UNHEALTHY = 0, /**< TRNG in unhealthy state */
	XTRNG_HEALTHY, /**< TRNG in healthy state */
	XTRNG_CATASTROPHIC, /**< TRNG in catastrophic state */
	XTRNG_ERROR, /**< TRNG in error state */
	XTRNG_STARTUP_TEST /**< TRNG in startup test state */
} XTrng_ErrorState;

typedef enum {
	XTRNG_DRNG_MODE = 1, /**< DRNG Mode */
	XTRNG_PTRNG_MODE, /**< PTRNG Mode */
	XTRNG_HRNG_MODE /**< HRNG Mode */
} XTrng_Mode;

typedef struct {
	XTrng_Mode Mode; /**< TRNG Mode */
	u32 SeedLife; /**< Seed life */
	u16 AdaptPropTestCutoff; /**< Cut off value for adaptive count test */
	u16 RepCountTestCutoff; /**< Cut off value for repetitive count test */
	u8 DFLength; /**< DF input length */
	u8 PredResistance; /**< prediction resistance */
	u8 IsBlocking; /**< Blocking or Non-Blocking reseed */
} XTrng_UserConfig;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
XTrng *XTrng_GetInstance(u16 DeviceId);
s32 XTrng_CfgInitialize(XTrng *InstancePtr);
s32 XTrng_Instantiate(XTrng *InstancePtr, u8 *Seed, u32 SeedLength, u8 *PersStr,
		      XTrng_UserConfig *UserCfg);
s32 XTrng_Reseed(XTrng *InstancePtr, u8 *Seed, u8 DLen);
s32 XTrng_Generate(XTrng *InstancePtr, u8 *RandBuf, u32 RandBufSize, u8 PredResistance);
s32 XTrng_Uninstantiate(XTrng *InstancePtr);
s32 XTrng_InitNCfgTrngMode(XTrng *InstancePtr, XTrng_Mode Mode);
s32 XTrng_DrbgKat(XTrng *InstancePtr);
s32 XTrng_PreOperationalSelfTests(XTrng *InstancePtr);
s32 XTrng_EnableAutoProcMode(XTrng *InstancePtr);
s32 XTrng_ReadTrngFifo(XTrng *InstancePtr, u32 *OutputBuf, u32 OutputBufSize);
s32 XTrng_IsRandomNumAvailable(XTrng *InstancePtr);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XTRNG_H */
/** @} */
