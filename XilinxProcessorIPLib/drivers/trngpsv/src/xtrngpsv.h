/**************************************************************************************************
* Copyright (C) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xtrngpsv.h
 * @addtogroup Overview
 * @{
 * @details
 *
 * The Xilinx True Random Number Generator (TRNG) module in Versal - PMC TRNG consists of an
 * entropy source, a deterministic random bit generator (DRBG) and health test logic, which tests
 * the randomness of the generated data. The entropy source for the unit is an array of Ring
 * Oscillators.
 *
 * The Versal PMC TRNG is envisaged to operate in three basic modes: DRNG, PTRNG and HRNG modes.
 * Each of these can be operated with or without Derivative Function (DF), resulting in a total
 * of 6 different modes of operation.
 *
 * NIST SP-800-90A practically requires the true random generators based on CTR_DRBG to include a
 * derivation function (DF). This is expected to be  implemented inside the Silicon (TRNG IP).
 * However, the version of the IP used in Versal PMC doesn't have this implementation. Hence,
 * a software implementation of the DF is done in this driver.
 *
 * DRNG mode: Deterministic Random Number Generator mode. In this mode, the DRBG portion of the
 * TRNG is used. User provides the (external) seed in this mode.
 * PTRNG mode: Physical True Random Number Generator mode (aka Entropy mode). In this mode digitized
 * Entropy source is output as random number.
 * HRNG mode: Hybrid Random Number Generator mode. This is combination of above two modes in which
 * the Entropy source is used to provide the seed, which is fed to the DRBG, which in turn
 * generates the random number.
 *
 * DRNG mode with DF: It may not be common usecase to use the DF with DRNG as the general
 * expectation would be that the seed would have sufficient entropy. However, the below guideline
 * from section 10.2.1 of NIST SP-800-90A implies that need for DF for DRNG mode too:
 * "..the DRBG mechanism is specified to allow an implementation tradeoff with respect to the use
 * of this derivation function. The use of the derivation function is optional if either an
 * approved RBG or an entropy source provides full entropy output when entropy input is requested
 * by the DRBG mechanism. Otherwise, the derivation function shall be used".
 * Sufficient large entropy data from user is fed to DF to generate the seed which will be loaded
 * into the external seed registers. From here, it is similar to regular DRNG mode.
 *
 * PTRNG mode with DF: This mode is similar to PTRNG mode, however, the entropy data from the core
 * output registers are accumulated and fed to the DF (instead of directly consuming it). The output
 * of the DF would be final random data. In this mode, the output of DF is not seed but the random
 * data.
 *
 * HRNG mode with DF: This mode is the combination of the above two modes. The entropy data is fed
 * to the DF to produce seed. This seed is loaded to the external seed registers which provide seed
 * to the DRBG.
 *
 * During operation, the driver will be one of the 4 stages as mentioned below.
 * Transition from UNINITIALIZED to HEALTHY happens through the Instantiation process. The state of
 * Uninitialized can be reached from any other state by Uninstantiate operation. ERROR state reached
 * through SW error conditions or through indication of CTF (caused by Certification Randomness
 * Test failure), whereas the CATASTROPHIC_ERROR state results from DTF (i.e. hardware failure).
 *
 * More description of the driver operation for each function can be found in the xtrngpsv.c file.
 *
 * This driver is intended to be RTOS and processor independent. It works with physical addresses
 * only. Any needs for dynamic memory management, threads or thread mutual exclusion, virtual
 * memory, or cache control must be satisfied by the layer above this driver.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------------------------------------
 * 1.00  ssc  09/05/21 First release
 * 1.1   ssc  03/24/22 New error code XTRNGPSV_ERROR_GLITCH and doxygen fixes
 * 1.4   mmd  07/10/23 Included header file for crypto algorithm information
 *       ng   06/30/23 Added support for system device-tree flow
 * </pre>
 *
 ******************************************************************************/

#ifndef XTRNGPSV_H
#define XTRNGPSV_H

/*************************************** Include Files *******************************************/

#include "xil_types.h"
#include "xstatus.h"
#include "xil_util.h"
#include "sleep.h"
#include "xtrngpsv_hw.h"
#include "xtrngpsv_alginfo.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************************ Constant Definitions ***************************************/

#define XTRNGPSV_SEC_STRENGTH_BYTES	32U 	/**< Security strength in bytes */
#define XTRNGPSV_SEC_STRENGTH_LEN	8U 	/**< Security strength in Words */
#define XTRNGPSV_PERS_STR_LEN		12U 	/**< Personalization string length in dwords */
#define XTRNGPSV_PERS_STR_LEN_BYTES	48U 	/**< Personalization string length in bytes */
#define XTRNGPSV_SEED_LEN		12U 	/**< Seed length in dwords */
#define XTRNGPSV_SEED_LEN_BYTES		48U	/**< Seed length in bytes */

#define XTRNGPSV_GEN_LEN_BYTES		32U	/**< No. of bytes per Generate request */

#define RAND_BUF_LEN			4U 	/**< No. of 32 bit words */
#define MAX_PRE_DF_LEN_BYTES		160U	/**< With max DFLenMul of 9 (9+1)*128= 1280 bits */
#define MAX_PRE_DF_LEN_WORDS		40U	/**< With max DFLenMul of 9 (9+1)*128= 1280 bits */

#define DF_SEED	0U 		/**< to indicate DF called for seed  */
#define DF_RAND	1U 		/**< to indicate DF called for random number */

#define DF_IP_IV_LEN	4U		/**< Input IV Length for DF */
#define BYTES_PER_BLOCK	16U		/**< No. of bytes per block, equivalent to 128 bits*/
#define DF_PAD_DATA_LEN	8U		/**< Length of Padding data used for DF calculation */

#define XTRNGPSV_SUCCESS	(s32)XST_SUCCESS	/**< SUCCESS definition of TRNGPSV driver */
#define XTRNGPSV_FAILURE	(s32)XST_FAILURE	/**< FAILURE definition of TRNGPSV driver */

#define XTRNGPSV_TRUE	(u32)TRUE		/**< Boolean TRUE definition of TRNGPSV driver */
#define XTRNGPSV_FALSE	(u32)FALSE		/**< Boolean FALSE definition of TRNGPSV driver */

#define XTRNGPSV_SWAP_ENDIAN Xil_EndianSwap32	/**< Macro to swap endianness of 32 bit data */

/* Error codes definition */
typedef enum {
	XTRNGPSV_ERROR_INVALID_PARAM = 0x10, /**< 0x10 */
	XTRNGPSV_ERROR_INVALID_STATE, /**< 0x11 */
	XTRNGPSV_ERROR_UNNECESSARY_PARAM, /**< 0x12 */
	XTRNGPSV_ERROR_GLITCH,  /**< 0x13 */

	/* Error codes from Instantiation operation */
	XTRNGPSV_ERROR_NOT_UNINSTANTIATED = 0x20, /**< 0x20 */
	XTRNGPSV_ERROR_INVALID_USRCFG_MODE, /**< 0x21 */
	XTRNGPSV_ERROR_INVALID_USRCFG_SEEDLIFE, /**< 0x22 */
	XTRNGPSV_ERROR_INVALID_USRCFG_PREDRES, /**< 0x23 */
	XTRNGPSV_ERROR_NO_SEED_INSTANTIATE, /**< 0x24 */
	XTRNGPSV_ERROR_INVALID_USRCFG_DFDIS, /**< 0x25 */
	XTRNGPSV_ERROR_INVALID_USRCFG_DFLENMUL, /**< 0x26 */
	XTRNGPSV_ERROR_USRCFG_CPY, /**< 0x27 */
	XTRNGPSV_ERROR_INVALID_USRCFG_PERSPRES, /**< 0x28 */
	XTRNGPSV_ERROR_INVALID_USRCFG_SEEDPRES, /**< 0x29 */
	XTRNGPSV_ERROR_UNNECESSARY_PARAM_INSTANTIATE, /**< 0x2A */

	/* Error codes from Reseed operation*/
	XTRNGPSV_ERROR_NO_SEED = 0x30, /**< 0x30 */
	XTRNGPSV_ERROR_SEED_INVALID_MODE, /**< 0x31 */
	XTRNGPSV_ERROR_SAME_SEED, /**< 0x32 */
	XTRNGPSV_ERROR_INVALID_RESEED_DFLENMUL, /**< 0x33 */
	XTRNGPSV_ERROR_CERTF, /**< 0x34 */
	XTRNGPSV_ERROR_CERTF_SW_A5_PATTERN, /**< 0x35 */
	XTRNGPSV_ERROR_RESEED_TIMEOUT, /**< 0x36 */
	XTRNGPSV_ERROR_CPY_RESEED,  /**< 0x37 */

	/* Error codes from Generate operation*/
	XTRNGPSV_ERROR_INSUFFICIENT_RANDBUF = 0x40, /**< 0x40 */
	XTRNGPSV_ERROR_PREDRES_MISMATCH, /**< 0x41 */
	XTRNGPSV_ERROR_RESEEDING_REQUIRED, /**< 0x42 */
	XTRNGPSV_ERROR_RESEED_REQD_PREDRES, /**< 0x43 */
	XTRNGPSV_ERROR_INVALID_GEN_PREDRES, /**< 0x44 */
	XTRNGPSV_ERROR_CATASTROPHIC_DTF, /**< 0x45 */
	XTRNGPSV_ERROR_CATASTROPHIC_DTF_SW, /**< 0x46 */
	XTRNGPSV_ERROR_GENERATE_TIMEOUT, /**< 0x47 */
	XTRNGPSV_ERROR_INVALID_RANDBUF_ADDR, /**< 0x48 */

	/* Derivative Function related Error codes */
	XTRNGPSV_ERROR_DF_CPY = 0x50, /**< 0x50 */
	XTRNGPSV_ERROR_DF_SETUP_KEY_FAILED, /**< 0x51 */
	XTRNGPSV_ERROR_DF_MEMSET,/**< 0x52 */
	XTRNGPSV_ERROR_DF_MEMMOVE,/**< 0x53 */

	/* Health Test and KAT related Error codes */
	XTRNGPSV_ERROR_HEALTHTEST_INVALID_MODE = 0x60, /**< 0x60 */
	XTRNGPSV_ERROR_KAT_MISMATCH, /**< 0x61 */
	XTRNGPSV_ERROR_USRCFG_CPY_KAT, /**< 0x62 */
} XTrngpsv_ErrorCodes;

/************************************** Type Definitions *****************************************/

/* This typedef contains enumeration of different states of the TRNGPSV driver */
typedef enum {
	XTRNGPSV_UNINITIALIZED = 0, /**< 0 - Driver is not in Initialized state */
	XTRNGPSV_HEALTHY, /**< 1 - Driver is not in Healthy state */
	XTRNGPSV_ERROR, /**< 3 - Driver encountered error condition */
	XTRNGPSV_CATASTROPHIC /**< 3 - Driver encountered catastrophic error */
} XTrngpsv_State;

/* This typedef contains modes of operation of the TRNGPSV driver */
typedef enum {
	XTRNGPSV_HRNG = 0, /**< 0 - Hybrid RNG  */
	XTRNGPSV_DRNG, /**< 1 - Deterministic RNG */
	XTRNGPSV_PTRNG /**< 2 - Physical True RNG */
} XTrngpsv_Mode;

/* This typedef contains configuration information for the device */
typedef struct {
#ifndef SDT
	u16 DeviceId;		/**< DeviceId is the unique ID of the
					*  device */
#else
	char *Name;
#endif
	UINTPTR BaseAddress;	/**< BaseAddress is the physical base address
					*  of the device's registers */
} XTrngpsv_Config;

/* This typedef contains config information for the device, which is used during Instantiation */
typedef struct {
	XTrngpsv_Mode Mode; /**< Mode of the TRNG - DRBG/PTRNG/HRNG */
	u64 SeedLife; /**< #of Generate requests per seed (128 - 2^48)*/
	u32 PredResistanceEn; /**< Enables Prediction Resistance capability */
	u32 PersStrPresent; /**< Used to indicate if Personalization string is
	 present. FALSE => no personalization string */
	u32 PersString[XTRNGPSV_PERS_STR_LEN]; /**< 384 bits of Pers. String */
	u32 InitSeedPresent; /**< Used to indicate if InitSeed is present.
	 FALSE => no InitSeed */
	u32 InitSeed[MAX_PRE_DF_LEN_WORDS]; /**< Initial Seed + nonce of 128 bits,
	 actual seed+nonce is of (DFLenMul+1)*128 bits */
	u32 DFDisable; /**< Setting this to TRUE, disables the
	 Derivative Function; default is enable */
	u32 DFLenMul; /**< Multiplier used to determine num of bits
	 on the input of the DF construct */
} XTrngpsv_UsrCfg;

/* This typedef contains statistics of the TRNGPSV driver */
typedef struct {
	u64 RandBytes; /**< Number of random bytes provided since instantiate */
	u64 RandBytesReseed; /**< Random bytes generated after last reseed */
	u64 ElapsedSeedLife; /**< Generate requests done after last reseed */
} XTrngpsv_Stats;

/* This typedef contains the format in which the Block Cipher (DF) algorithm
 * expects the information
 */
typedef struct {
	u32 IvCounter[DF_IP_IV_LEN]; /**< Counter/seq number */
	u32 InputLen; /**< length indicating actual entropy
	 data length + pers string length in bytes*/
	u32 PostDfLen; /**< Length of entropy data (e.g. seed, random data)
	 post DF in bytes	*/
	u8 EntropyData[MAX_PRE_DF_LEN_BYTES]; /**< Input Entropy data */
	u8 PersString[XTRNGPSV_PERS_STR_LEN_BYTES];/**< Personalization String */
	u8 PadData[DF_PAD_DATA_LEN]; /**< Padding to make structure, multiple of 16Bytes */
} XTrngpsv_DFInput;

/* This typedef contains main instance of the TRNGPSV driver */
typedef struct {
	XTrngpsv_Config Config; /**< Hardware Configuration */
	XTrngpsv_UsrCfg UsrCfg; /**< Configuration from the user */
	XTrngpsv_Stats TrngStats; /**< TRNGPSV Statistics */
	XTrngpsv_State State; /**< takes one of the possible
	 values 	indicated by XTrngpsv_State */
	u32 RandBitBuf[RAND_BUF_LEN]; /**< buffer of random bits to minimize
	 latency */
	u32 EntropySize; /**< actual size (in bytes) of seed */
	XTrngpsv_DFInput DFInput; /**< data structure which is input
	 to the DF operation */
	u8 DFOutput[XTRNGPSV_SEED_LEN_BYTES]; /**< Output of the DF operation
	 (contains updated seed or random number) */
} XTrngpsv;

/************************************ Variable Definitions ***************************************/
extern XTrngpsv_Config XTrngpsv_ConfigTable[];

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/* Required functions in xtrngpsv.c */
#ifndef SDT
XTrngpsv_Config *XTrngpsv_LookupConfig(u16 DeviceId);
#else
XTrngpsv_Config *XTrngpsv_LookupConfig(UINTPTR BaseAddress);
#endif

s32 XTrngpsv_CfgInitialize(XTrngpsv *InstancePtr, const XTrngpsv_Config *CfgPtr,
		UINTPTR EffectiveAddr);
s32 XTrngpsv_Instantiate(XTrngpsv *InstancePtr, const XTrngpsv_UsrCfg *ConfigurValues);
s32 XTrngpsv_Reseed(XTrngpsv *InstancePtr, const u8 *ExtSeedPtr, u32 DFLenMul);
s32 XTrngpsv_Generate(XTrngpsv *InstancePtr, u8 *RandBufPtr, u32 RandBufSize, u8 PredResistanceEn);
s32 XTrngpsv_Uninstantiate(XTrngpsv *InstancePtr);

/* Functions in xtrngpsv_df.c */
s32 XTrngpsv_DF(XTrngpsv *InstancePtr, u8 *DFOutput, u32 DF_Flag, const u8 *PersStrPtr);

/* Functions in xtrngpsv_tests.c */
s32 XTrngpsv_RunKAT(XTrngpsv *InstancePtr);
s32 XTrngpsv_RunHealthTest(XTrngpsv *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
