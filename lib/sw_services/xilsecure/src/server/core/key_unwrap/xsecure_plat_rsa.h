/******************************************************************************
* Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_plat_rsa.h
*
* This file contains hardware interface related information for RSA device
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 5.2   kpt  06/25/23 Initial release
* 5.3   am   09/28/23 Added wrapper function prototypes for IPCore's RSA APIs
*       kpt  12/13/23 Added XSecure_RsaOperationParam
*       kpt  12/13/23 Added RSA CRT support for keyunwrap
* 5.3   ng   01/28/24 Added SDT support
*       ng   03/26/24 Fixed header include in SDT flow
*       ss   04/05/24 Fixed doxygen warnings
* 5.4   yog  04/29/24 Fixed doxygen grouping and doxygen warnings.
*       kpt  05/26/24 Added support RSA CRT and Expopt operation.
*       kpt  06/13/24 Add support for RSA key generation.
*       kpt  06/30/24 Added XSECURE_PRIME_FACTOR_MAX_P_SIZE and
*                     XSECURE_PRIME_FACTOR_MAX_Q_SIZE
*       kal  07/24/24 Code refactoring changes for versal_2ve_2vm
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_rsa_server_apis Xilsecure RSA Server APIs
* @{
*/
#ifndef XSECURE_PLAT_RSA_H_
#define XSECURE_PLAT_RSA_H_

/** For compilation in C++ */
#ifdef __cplusplus
#define externC extern "C"
#else
#define externC extern
#endif


/***************************** Include Files *********************************/

#ifdef SDT
#include "xsecure_config.h"
#endif

#ifndef PLM_RSA_EXCLUDE

#include "xil_types.h"
#include "xsecure_rsa_core.h"
#include "xsecure_rsa_q.h"
#include "xsecure_mgf.h"
#include "Rsa.h"

/************************** Constant Definitions ****************************/

#ifndef XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES
#define XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES (XSECURE_RSA_3072_SIZE_WORDS * 4U) /**< RSA default key size in bytes */
#endif
#define XSECURE_RSA_KEY_GEN_SIZE_IN_WORDS (XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES / 4U) /**< RSA key generation size in words */
#define XSECURE_RSA_MAX_KEY_GEN_SUPPORT	(2U) /**< Maximum keys that needs to be generated */
#define XSECURE_RSA_KEY_STATUS_WAIT     (1U)          /**< RSA key status wait */
#define XSECURE_RSA_2048_QUANT_SIZE  	(5U)          /**< RSA maximum quant size for 2048 bit key */
#define XSECURE_RSA_3072_QUANT_SIZE  	(2U)          /**< RSA maximum quant size for 3072 bit key */
#define XSECURE_RSA_4096_QUANT_SIZE  	(1U)          /**< RSA maximum quant size for 4096 bit key */

#define XSECURE_RSA_PUBLIC_EXPONENT		(0x10001U)    /**< RSA public exponent value */
#define XSECURE_RSA_PUB_EXP_SIZE		(4U)          /**< RSA public exponent size */
#define XSECURE_KEY_PAIR_GEN_POLL_INTERVAL (100U) /**< Key pair generation poll interval */
#define XSECURE_ECDSA_RSA_SOFT_RESET            (0xF1200040U) /**< ECDSA/RSA soft reset address */
#define XSECURE_RSA_SIZE_IN_BYTES		(512U)	      /**< 512 bytes for 4096 bit data */
#define XSECURE_PRIME_FACTOR_P_SIZE	(XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES >> 1U)  /**< size of first prime factor(P) */
#define XSECURE_PRIME_FACTOR_Q_SIZE	(XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES >> 1U)  /**< size of first prime factor(Q) */
#define XSECURE_PRIME_FACTOR_MAX_P_SIZE (XSECURE_RSA_SIZE_IN_BYTES >> 1U)      /**< 256 bytes size of first prime factor(P) */
#define XSECURE_PRIME_FACTOR_MAX_Q_SIZE (XSECURE_RSA_SIZE_IN_BYTES >> 1U)      /**< 256 bytes size of first prime factor(Q) */

/***************************** Type Definitions ******************************/
typedef RsaKeyPair XSecure_RsaKeyPtr;

typedef enum {
	XSECURE_RSA_KEY_DEFAULT_STATE = 0, /**< Default state */
	XSECURE_RSA_KEY_INIT_STATE,     /**< Key initialized state */
	XSECURE_RSA_KEY_GEN_STATE,      /**< Key generate state */
	XSECURE_RSA_KEY_READY_STATE     /**< Key ready state */
} XSecure_RsaKeyOpState;

typedef enum {
	XSECURE_RSA_KEY_FREE = 0, /**< RSA key is free */
	XSECURE_RSA_KEY_AVAIL /**< RSA key is available */
} XSecure_RsaKeyStatus;

/**
 * Input and output parameters for RSA
 * Optimal Asymmetric Encryption Padding scheme.
 */
typedef struct {
	u64 InputDataAddr;       /**< Input data address */
	u64 OutputDataAddr;      /**< Output data address */
	u64 OptionalLabelAddr;   /**< Optional label address */
	u32 InputDataSize;       /**< Input data size */
	u32 OutputDataSize;      /**< Output data size */
	u32 OptionalLabelSize;    /**< Optional label size */
	void *ShaInstancePtr;    /**< SHA instance for MGF */
	XSecure_ShaMode ShaType; /**< SHA type for MGF */
} XSecure_RsaOaepParam;

/** Input parameters pointers for RSA. */
typedef struct {
	u8 *Modulus;  /**< Modulus */
	u8 *P;        /**< Prime1 */
	u8 *Q;        /**< Prime2 */
	u8 *DP;       /**< Private exponent 1 */
	u8 *DQ;       /**< Private exponent 2 */
	u8 *QInv;     /**< Q inverse */
} XSecure_RsaKey;

typedef struct {
	u8 Mod[XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES];          /**< Modulus */
	u32 PubExp[XSECURE_RSA_KEY_GEN_SIZE_IN_WORDS]; /**< Public exponent */
} XSecure_RsaPubKey;

typedef struct {
	u8 Mod[XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES];    /**< Modulus */
	u8 Exp[XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES];    /**< Exponent */
	u8 P[XSECURE_PRIME_FACTOR_P_SIZE];            /**< Prime1 */
	u8 Q[XSECURE_PRIME_FACTOR_Q_SIZE];            /**< Prime2 */
	u8 DP[XSECURE_PRIME_FACTOR_P_SIZE];           /**< Private exponent 1 */
	u8 DQ[XSECURE_PRIME_FACTOR_Q_SIZE];           /**< Private exponent 2 */
	u8 QInv[XSECURE_PRIME_FACTOR_Q_SIZE];         /**< Q inverse */
}XSecure_RsaPrivKey;

typedef struct {
	XSecure_RsaPrivKey* PrivKey;     /**< Private Key */
	XSecure_RsaPubKey* PubKey; /**< Public Key */
} XSecure_RsaKeyPair;

typedef struct {
	XSecure_RsaKeyPair KeyPair;
	u32 QuantSize;
	u32 IsRsaKeyAvail;
} XSecure_RsaKeyGenParam;

typedef struct {
	XSecure_RsaKeyGenParam Key[XSECURE_RSA_MAX_KEY_GEN_SUPPORT];
	u32 KeyInUse;
} XSecure_RsaKeyMgmt;

/***************************** Function Prototypes ***************************/

int XSecure_RsaOaepEncrypt(XSecure_Rsa *InstancePtr, XSecure_RsaOaepParam *OaepParam);
int XSecure_RsaOaepDecrypt(XSecure_RsaPrivKey *PrivKey, XSecure_RsaOaepParam *OaepParam);
XSecure_RsaPrivKey* XSecure_GetRsaPrivateKey(u32 RsaIdx);
XSecure_RsaPubKey* XSecure_GetRsaPublicKey(u32 RsaIdx);
/**< API to retrieve RSA KEY in use index */
u32 XSecure_GetRsaKeyInUseIdx(void);
/**< API to destroy key pair that is in use */
int XSecure_RsaDestroyKeyInUse(void);
/**< API to add key pair generation to scheduler */
int XSecure_AddRsaKeyPairGenerationToScheduler(void);

/** @} */

#endif	/* PLM_RSA_EXCLUDE_H_ */

#endif /* XSECURE_PLAT_RSA_H_ */
