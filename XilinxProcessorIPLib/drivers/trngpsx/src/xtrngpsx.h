/******************************************************************************
* Copyright (c) 2023 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xtrngpsx.h
* This file contains trng definitions of VersalNet.
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
* 1.0   kpt  01/04/23 Initial release
*       kpt  05/18/23 Updated adapttestcutoff and reptestcutoff default values
* 1.1   mmd  07/09/23 Included header file for crypto algorithm information
*       ng   09/04/23 Added SDT support
* 1.2   kpt  01/09/24 Added error code XTRNGPSX_INVALID_BLOCKING_MODE
* 1.5   ank  09/26/25 Fixed MISRA-C Violations
*
* </pre>
*
* @endcond
******************************************************************************/
#ifndef XTRNGPSX_H
#define XTRNGPSX_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xtrngpsx_alginfo.h"

/************************** Constant Definitions *****************************/
#define XTRNGPSX_PERS_STRING_LEN_IN_WORDS	12U	/**< Personalization string length in words */
#define XTRNGPSX_PERS_STRING_LEN_IN_BYTES	48U	/**< Personalization string length in bytes */
#define XTRNGPSX_SEC_STRENGTH_IN_BYTES	32U	/**< security strength in Bytes */

#if !defined(XTRNGPSX_USER_CFG_SEED_LIFE)
#define XTRNGPSX_USER_CFG_SEED_LIFE 256U
#endif

#if !defined(XTRNGPSX_USER_CFG_DF_LENGTH)
#define XTRNGPSX_USER_CFG_DF_LENGTH 7U
#endif

#if !defined(XTRNGPSX_USER_CFG_ADAPT_TEST_CUTOFF)
#define XTRNGPSX_USER_CFG_ADAPT_TEST_CUTOFF 645U
#endif

#if !defined(XTRNGPSX_USER_CFG_REP_TEST_CUTOFF)
#define XTRNGPSX_USER_CFG_REP_TEST_CUTOFF 66U
#endif

/**************************** Type Definitions *******************************/

/* This typedef contains mode information on which TRNG operates */
typedef enum {
	XTRNGPSX_DRNG_MODE = 1,
	XTRNGPSX_PTRNG_MODE,
	XTRNGPSX_HRNG_MODE
} XTrngpsx_Mode;

/* This typedef contains configuration information for the device */
typedef struct {
#ifndef SDT
	u16 DeviceId;	/**< DeviceId is the unique ID of the device */
#else
	char *Name;
#endif
	UINTPTR BaseAddress;	/**< BaseAddress is the physical base address of the device's registers */
} XTrngpsx_Config;

/* This typedef contains user configuration related to TRNG */
typedef struct {
	XTrngpsx_Mode Mode; /**< TRNG mode */
	u32 SeedLife;            /**< seed life */
	u16 AdaptPropTestCutoff; /**< Adaptive test cutoff */
	u16 RepCountTestCutoff;  /**< Repititive test cutoff */
	u8 DFLength;	/**< df length */
	u8 PredResistance; /**< prediction resistance */
	u8 IsBlocking; /**< Blocking or Non-Blocking reseed */
} XTrngpsx_UserConfig;

/* This typedef contains status related to TRNG */
typedef struct {
	u32 ElapsedSeedLife; /**< elapsed seed life */
} XTrngpsx_Status;

/* This typedef contains error status related to TRNG */
typedef enum {
	XTRNGPSX_UNHEALTHY = 1, /**< unhealthy state */
	XTRNGPSX_HEALTHY,       /**< healthy state */
	XTRNGPSX_CATASTROPHIC,  /**< catastropic state */
	XTRNGPSX_ERROR,         /**< error state */
	XTRNGPSX_STARTUP_TEST   /**< startup test */
} XTrngpsx_ErrorState;

/* This typedef contains trng driver state */
typedef enum {
	XTRNGPSX_UNINITIALIZED_STATE = 1,	/**< Default state */
	XTRNGPSX_INSTANTIATE_STATE,		/**< Instantiate state */
	XTRNGPSX_RESEED_STATE,		/**< Reseed state */
	XTRNGPSX_GENERATE_STATE		/**< Generate state */
} XTrngpsx_State;

/* This typedef contains all the information related to TRNG driver */
typedef struct {
	XTrngpsx_Config Config;   /**< device configuration */
	XTrngpsx_UserConfig UserCfg;/**< user configuration */
	XTrngpsx_Status Stats;      /**< Trng status */
	XTrngpsx_ErrorState ErrorState;/**< Trng error state */
	XTrngpsx_State State;   /**< Trng driver state */
} XTrngpsx_Instance;

typedef enum {
	XTRNGPSX_INVALID_PARAM = 0x0,	/**< 0x0 - Invalid argument */
	XTRNGPSX_INVALID_MODE,			/**< 0x1 - Error when TRNG operation mode is
										invalid */
	XTRNGPSX_INVALID_DF_LENGTH,		/**< 0x2 - Error when DF length is invalid */
	XTRNGPSX_INVALID_SEED_LIFE,		/**< 0x3 - Error when seed life is invalid */
	XTRNGPSX_INVALID_SEED_VALUE,	/**< 0x4 - Error when seed is NULL */
	XTRNGPSX_INVALID_SEED_LENGTH,	/**< 0x5 - Error when input seed length doesn't
										match with df length */
	XTRNGPSX_INVALID_STATE,			/**< 0x6 - Error when TRNG state is
										invalid */
	XTRNGPSX_INVALID_ADAPTPROPTEST_CUTOFF_VALUE, /**< 0x7 - Invalid adaptive proptest
													cutoff value */
	XTRNGPSX_INVALID_REPCOUNTTEST_CUTOFF_VALUE,	 /**< 0x8 - Invalid repitive count test
													cutoff value */
	XTRNGPSX_USER_CFG_COPY_ERROR,		 /**< 0x9 - Error during memcpy of
													UserConfig structure */
	XTRNGPSX_UNHEALTHY_STATE,			 /**< 0xA - Error when device fails KAT or
													health tests */
	XTRNGPSX_INVALID_BUF_SIZE,			 /**< 0xB - Error invalid buffer size */
	XTRNGPSX_RESEED_REQUIRED_ERROR,		 /**< 0xC - Error seed life expired and
													reseed required in DRBG mode */
	XTRNG_PSX_INVALID_PREDRES_VALUE,      /**< 0xD - Invalid predication resistance valye */
	XTRNGPSX_MEMSET_UNINSTANTIATE_ERROR,	 /**< 0xE - Error during memset */
	XTRNGPSX_TIMEOUT_ERROR,			 		 /**< 0xF - Timeout while waiting for
												done bit during reseed or generate*/
	XTRNGPSX_CATASTROPHIC_CTF_ERROR,		 /**< 0x10 - CTF error during reseed */
	XTRNGPSX_CATASTROPHIC_DTF_ERROR,		 /**< 0x11 - DTF error during generate */
	XTRNGPSX_KAT_FAILED_ERROR,			     /**< 0x12 - Error when resultant TRNG o/p
												doesn't match with expected o/p */
	XTRNGPSX_WRITE_ERROR,			 		 /**< 0x13 - Error occurred while writing in
												to the register */
	XTRNGPSX_INVALID_BLOCKING_MODE,			 /**< 0x14 - Error occurred when invalid blocking mode
												is selected */
} XTrngpsx_ErrorCodes;

/************************************ Variable Definitions ***************************************/
extern XTrngpsx_Config XTrngpsx_ConfigTable[];

/************************** Function Prototypes ******************************/
#ifndef SDT
XTrngpsx_Config *XTrngpsx_LookupConfig(u16 DeviceId);
#else
XTrngpsx_Config *XTrngpsx_LookupConfig(UINTPTR BaseAddress);
#endif
int XTrngpsx_CfgInitialize(XTrngpsx_Instance *InstancePtr, const XTrngpsx_Config *CfgPtr,
		UINTPTR EffectiveAddr);
int XTrngpsx_Instantiate(XTrngpsx_Instance *InstancePtr, const u8 *Seed, u32 SeedLength, const u8 *PersStr,
			const XTrngpsx_UserConfig *UserCfg);
int XTrngpsx_Reseed(XTrngpsx_Instance *InstancePtr, const u8 *Seed, u8 DLen);
int XTrngpsx_Generate(XTrngpsx_Instance *InstancePtr, u8 *RandBuf, u32 RandBufSize, u8 PredResistance);
int XTrngpsx_Uninstantiate(XTrngpsx_Instance *InstancePtr);
int XTrngpsx_DRBGKat(XTrngpsx_Instance *InstancePtr);
int XTrngpsx_HealthTest(XTrngpsx_Instance *InstancePtr);
int XTrngpsx_PreOperationalSelfTests(XTrngpsx_Instance *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
