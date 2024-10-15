/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_rsa_q.h
*
* This file contains hardware interface related information for RSA quite mode
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 5.4   kal  08/26/24 Initial release
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_rsa_qmode_apis Xilsecure RSA QMODE Server APIs
* @{
*/
#ifndef XSECURE_RSA_Q_H_
#define XSECURE_RSA_Q_H_

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
#include "Rsa.h"

/************************** Constant Definitions ****************************/
#define XSECURE_ECDSA_RSA_SOFT_RESET            (0xF1200040U) /**< ECDSA/RSA soft reset address */
#define XSECURE_RSA_SIZE_IN_BYTES		(512U)	      /**< 512 bytes for 4096 bit data */
#define XSECURE_PRIME_FACTOR_MAX_P_SIZE (XSECURE_RSA_SIZE_IN_BYTES >> 1U)      /**< 256 bytes size of first prime factor(P) */
#define XSECURE_PRIME_FACTOR_MAX_Q_SIZE (XSECURE_RSA_SIZE_IN_BYTES >> 1U)      /**< 256 bytes size of first prime factor(Q) */
#define XSECURE_RSA_PUB_EXP_SIZE		(4U)          /**< RSA public exponent size */

/***************************** Type Definitions ******************************/
typedef struct {
	u8 InData[XSECURE_RSA_SIZE_IN_BYTES];  /**< Input data */
	u8 Exp[XSECURE_RSA_SIZE_IN_BYTES];     /**< Exponent */
	u8 Mod[XSECURE_RSA_SIZE_IN_BYTES];     /**< Modulus */
	u8 P[XSECURE_PRIME_FACTOR_MAX_P_SIZE];     /**< Prime1 */
	u8 Q[XSECURE_PRIME_FACTOR_MAX_Q_SIZE];     /**< Prime2 */
	u8 DP[XSECURE_PRIME_FACTOR_MAX_P_SIZE];     /**< Prime1 */
	u8 DQ[XSECURE_PRIME_FACTOR_MAX_Q_SIZE];     /**< Prime2 */
	u8 QInv[XSECURE_PRIME_FACTOR_MAX_P_SIZE];   /**< Q inverse */
	u8 Tot[XSECURE_RSA_SIZE_IN_BYTES];     /**< Totient */
	u8 RN[XSECURE_RSA_SIZE_IN_BYTES];     /**< Precalculated modulus */
	u8 RRN[XSECURE_RSA_SIZE_IN_BYTES];     /**< Precalculated modulus */
	u32 PubExp;                            /**< Public Exponent */
} XSecure_RsaOperationParam;

/***************************** Function Prototypes ***************************/

int XSecure_RsaExpCRT(u8 *Hash, u8 *P, u8 *Q, u8 *Dp, u8 *Dq, u8 *Qinv, u8 *Pub,
	u8 *Mod, int Len, u8 *Res);
int XSecure_RsaExp(u8 *Hash, u8 *Exp, u8 *Mod, u8 *P, u8 *Q, u8 *Pub, u8 *Tot,
	int Len, u8 *Res);
int XSecure_RsaExpopt(u8 *Hash, u8 *Exp, u8 *Mod, u8 *RN, u8 *RRN, u8 *P, u8 *Q, u8 *Pub, u8 *Tot,
	int Len, u8 *Res);

/** @} */

#endif	/* PLM_RSA_EXCLUDE_H_ */

#endif /* XSECURE_RSA_Q_H_ */
