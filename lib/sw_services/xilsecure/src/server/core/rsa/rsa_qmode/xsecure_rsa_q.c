/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_rsa_q.c
* This file contains Versal Net specific code for Xilsecure rsa quiet mode server.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 5.4   kal     08/26/24 Initial release
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_rsa_qmode_apis Xilsecure RSA QMODE Server APIs
* @{
*/
/***************************** Include Files *********************************/

#ifdef SDT
#include "xsecure_config.h"
#endif

#include "xparameters.h"
#include "xsecure_rsa_q.h"
#ifndef PLM_RSA_EXCLUDE

#include "xsecure_rsa.h"
#include "xsecure_plat.h"
#include "xsecure_error.h"
#include "xsecure_defs.h"
/************************** Constant Definitions *****************************/

/***************************** Type Definitions ******************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief       This function performs the RSA exponentiation using
 *		CRT (Chinese Remainder Theorem).
 *
 * @param	Hash	is the Hash of the exponentiation.
 * @param	P	is first factor, a positive integer.
 * @param	Q	is second factor, a positive integer.
 * @param	Dp	is first factor's CRT exponent, a positive integer.
 * @param	Dq	is second factor's CRT exponent, a positive integer.
 * @param	Qinv	is (first) CRT coefficient, a positive integer.
 * @param	Pub	is the public exponent to protect against the fault insertions.
 * @param	Mod	is the public modulus (p*q), if NULL, calculated internally.
 * @param	Len	is length of the full-length integer in bits.
 * @param	Res	is result of exponentiation r = (h^e) mod n.
 *
 * @return
 *		 - XST_SUCCESS  On success.
 *		 - XSECURE_RSA_EXPONENT_INVALID_PARAM  If any input parameter is invalid.
 *		 - XST_FAILURE  On failure.
 *
 ******************************************************************************/
int XSecure_RsaExpCRT(u8 *Hash, u8 *P, u8 *Q, u8 *Dp, u8 *Dq, u8 *Qinv, u8 *Pub,
	u8 *Mod, int Len, u8 *Res)
{
	volatile int Status = XST_FAILURE;

	/** Validate input parameters */
	if ((Hash == NULL) || (P == NULL) || (Q == NULL) || (Dp == NULL) ||
		(Dq == NULL) || (Qinv == NULL) || (Res == NULL)) {
		Status = (int)XSECURE_RSA_EXPONENT_INVALID_PARAM;
		goto END;
	}

	/** Release the RSA engine from reset */
	XSecure_Out32(XSECURE_ECDSA_RSA_SOFT_RESET, 0U);

	/** Perform the RSA exponentiation using CRT */
	Status = RSA_ExpCrtQ(Hash, P, Q, Dp, Dq, Qinv, Pub, Mod, Len, Res);

	/** Reset the RSA engine */
	XSecure_Out32(XSECURE_ECDSA_RSA_SOFT_RESET, 1U);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function performs the RSA exponentiation.
 *
 * @param	Hash	is Hash of the exponentiation.
 * @param	Exp	is exponent, a positive integer.
 * @param	Mod	is public modulus (p*q), NULL is invalid param.
 * @param	P	is first factor, a positive integer.
 * @param	Q	is second factor, a positive integer.
 * @param	Pub	is public exponent to protect against the fault insertions.
 * @param	Tot	is totient, a secret value equal to (p-1)*(q-1).
 * 			if NULL, calculated internally with p and q.
 * @param	Len	is length of the full-length integer in bits.
 * @param	Res	is result of exponentiation r = (h^e) mod n.
 *
 * @return
 *		 - XST_SUCCESS  On success.
 *		 - XSECURE_RSA_EXPONENT_INVALID_PARAM  If any input parameter is invalid.
 *		 - XST_FAILURE  On failure.
 *
 ******************************************************************************/
int XSecure_RsaExp(u8 *Hash, u8 *Exp, u8 *Mod, u8 *P, u8 *Q, u8 *Pub, u8 *Tot,
	int Len, u8 *Res)
{
	volatile int Status = XST_FAILURE;

	/** Validate input parameters */
	if ((Hash == NULL) || (Exp == NULL) || (Mod == NULL) || (Res == NULL)) {
		Status = (int)XSECURE_RSA_EXPONENT_INVALID_PARAM;
		goto END;
	}

	/** Release the RSA engine from reset */
	XSecure_Out32(XSECURE_ECDSA_RSA_SOFT_RESET, 0U);

	/** Perform the RSA exponentiation */
	Status = RSA_ExpQ(Hash, Exp, Mod, P, Q, Pub, Tot, Len, Res);

	/** Reset the RSA engine */
	XSecure_Out32(XSECURE_ECDSA_RSA_SOFT_RESET, 1U);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function perofrms the RSA exponentiation using pre-calculated modulus.
 *
 * @param	Hash	is Hash of the exponentiation.
 * @param	Exp	is exponent, a positive integer.
 * @param	Mod	is public modulus (p*q), NULL is invalid param.
 * @param	RN 	is pre-calculated modulus RmodN
 * @param	RRN	is pre-calculated modulus RRmodN
 * @param	P	is first factor, a positive integer.
 * @param	Q	is second factor, a positive integer.
 * @param	Pub	is public exponent to protect against the fault insertions.
 * @param	Tot	is totient, a secret value equal to (p-1)*(q-1).
 * 			if NULL, calculated internally with p and q.
 * @param	Len	is length of the full-length integer in bits.
 * @param	Res	is result of exponentiation r = (h^e) mod n.
 *
 * @return
 *		 - XST_SUCCESS  On success.
 *		 - XSECURE_RSA_EXPONENT_INVALID_PARAM  If any input parameter is invalid.
 *		 - XST_FAILURE  On failure.
 *
 ******************************************************************************/
int XSecure_RsaExpopt(u8 *Hash, u8 *Exp, u8 *Mod, u8 *RN, u8 *RRN, u8 *P, u8 *Q, u8 *Pub, u8 *Tot,
	int Len, u8 *Res)
{
	volatile int Status = XST_FAILURE;

	/** Validate input parameters */
	if ((Hash == NULL) || (Exp == NULL) || (Mod == NULL) || (Res == NULL) || (RN == NULL)
		|| (RRN == NULL)) {
		Status = (int)XSECURE_RSA_EXPONENT_INVALID_PARAM;
		goto END;
	}

	/** Release the RSA engine from reset */
	XSecure_Out32(XSECURE_ECDSA_RSA_SOFT_RESET, 0U);

	/** Perform the RSA exponentiation with pre-calculated modulus */
	Status = RSA_ExpoptQ(Hash, Exp, Mod, RN, RRN, P, Q, Pub, Tot, Len, Res);

	/** Reset the RSA engine */
	XSecure_Out32(XSECURE_ECDSA_RSA_SOFT_RESET, 1U);

END:
	return Status;
}

#endif
/** @} */
