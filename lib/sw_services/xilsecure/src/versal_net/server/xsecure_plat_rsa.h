/******************************************************************************
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_plat_rsa.h
* @addtogroup xsecure_plat_rsa_apis XilSecure Platform RSA APIs
* @{
* @cond xsecure_internal
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
*	ss   04/05/24 Fixed doxygen warnings
*
* </pre>
*
* @endcond
******************************************************************************/

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
#include "xsecure_mgf.h"
#include "Rsa.h"

/************************** Constant Definitions ****************************/

#ifndef XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES
#define XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES (XSECURE_RSA_3072_SIZE_WORDS * 4U) /**< RSA default key size in bytes */
#endif
#define XSECURE_ECDSA_RSA_SOFT_RESET		(0xF1200040U) /**< ECDSA/RSA soft reset address */
#define XSECURE_RSA_SIZE_IN_BYTES		(512U)	      /**< 512 bytes for 4096 bit data */
#define XSECURE_PRIME_FACTOR_P_SIZE	(256U)  /**< 256 bytes size of first prime factor(P) */
#define XSECURE_PRIME_FACTOR_Q_SIZE	(256U)  /**< 256 bytes size of first prime factor(Q) */

/***************************** Type Definitions ******************************/

typedef struct {
	u64 InputDataAddr;       /**< Input data address */
	u64 OutputDataAddr;      /**< Output data address */
	u64 OptionalLabelAddr;   /**< Optional label address */
	u32 InputDataSize;       /**< Input data size */
	u32 OutputDataSize;      /**< Output data size */
	u32 OptionalLabelSize;    /**< Optional label size */
	void *ShaInstancePtr;    /**< SHA instance for MGF */
	XSecure_ShaType ShaType; /**< SHA type for MGF */
} XSecure_RsaOaepParam;

typedef struct {
	u8 *Modulus;  /**< Modulus */
	u8 *P;        /**< Prime1 */
	u8 *Q;        /**< Prime2 */
	u8 *DP;       /**< Private exponent 1 */
	u8 *DQ;       /**< Private exponent 2 */
	u8 *QInv;     /**< Q inverse */
} XSecure_RsaKey;

typedef struct {
	u8 *Modulus;     /**< Modulus */
	u8 *Exponent;    /**< Public exponent */
	u8 *ModExt;      /**< Modulus extension */
} XSecure_RsaPubKey;

typedef struct {
	u8 InData[XSECURE_RSA_SIZE_IN_BYTES];  /**< Input data */
	u8 Exp[XSECURE_RSA_SIZE_IN_BYTES];     /**< Exponent */
	u8 Mod[XSECURE_RSA_SIZE_IN_BYTES];     /**< Modulus */
	u8 Tot[XSECURE_RSA_SIZE_IN_BYTES];     /**< Totient */
	u8 P[XSECURE_PRIME_FACTOR_P_SIZE];     /**< Prime1 */
	u8 Q[XSECURE_PRIME_FACTOR_Q_SIZE];     /**< Prime2 */
	u32 PubExp;                            /**< Public Exponent */
} XSecure_RsaExpOperationParam;

/***************************** Function Prototypes ***************************/

int XSecure_RsaOaepEncrypt(XSecure_Rsa *InstancePtr, XSecure_RsaOaepParam *OaepParam);
int XSecure_RsaOaepDecrypt(XSecure_RsaKey *PrivKey, XSecure_RsaOaepParam *OaepParam);
XSecure_RsaKey *XSecure_GetRsaPrivateKey(void);
XSecure_RsaPubKey *XSecure_GetRsaPublicKey(void);

/**
 * @name Wrapper function prototypes for IPCores RSA quiet mode APIs
 * @{
 */
/**< Wrapper prototype declarations for IPCores APIs for RSA "quiet" operations */
int XSecure_RsaExpCRT(unsigned char *Hash, unsigned char *P, unsigned char *Q,
	unsigned char *Dp, unsigned char *Dq, unsigned char *Qinv, unsigned char *Pub,
	unsigned char *Mod, int Len, unsigned char *Res);

int XSecure_RsaExp(unsigned char *Hash, unsigned char *Exp, unsigned char *Mod,
	unsigned char *P, unsigned char *Q, unsigned char *Pub, unsigned char *Tot,
	int Len, unsigned char *Res);
/** @} */

#endif	/* PLM_RSA_EXCLUDE_H_ */

#endif /* XSECURE_PLAT_RSA_H_ */
/* @} */
