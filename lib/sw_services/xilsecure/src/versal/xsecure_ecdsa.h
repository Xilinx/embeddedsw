/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_ecdsa.h
* @addtogroup xsecure_ecdsa_apis XilSecure ECDSA APIs
* @{
* @cond xsecure_internal
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 4.0 	vns  03/27/19 First Release
* 4.2   har  11/07/19 Typo correction to enable compilation in C++
*
* </pre>
*
* @endcond
******************************************************************************/

#ifndef XSECURE_ECDSA_H_
#define XSECURE_ECDSA_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xsecure_utils.h"
/************************** Constant Definitions ****************************/
#define XSECURE_ECC_DATA_SIZE_WORDS (12U)
/***************************** Type Definitions ******************************/

/***************************** Function Prototypes ***************************/


/*****************************************************************************/
/**
 * @brief
 * This function validates public key for p384 curve.
 *
 * @param	Qx 	Pointer to the x co-ordinate of ECC point.
 * @param	Qy	Pointer to the y co-ordinate of ECC point.
 *
 * @return	XST_SUCCESS if verification was successful.
 *
 *
 ******************************************************************************/
int P384_validatekey(unsigned char *Qx, unsigned char *Qy);

/*****************************************************************************/
/**
 * @brief
 * This function verifies ECDSA signature(r,s) of the hash 'z'
 * (truncated to the group order size) using the public key 'Q'
 *
 * @param	Z 	Pointer to the expected hash of the signed data.
 * @param	Qx 	Pointer to the x co-ordinate of ECC point.
 * @param	Qy	Pointer to the y co-ordinate of ECC point.
 * @param	r 	Pointer to the r component of signature pair (r,s)
 * @param	s	Pointer to the s component of signature pair (r,s)
 *
 * @return	XST_SUCCESS if verification was successful.
 *
 *
 ******************************************************************************/

int P384_ecdsaverify(unsigned char *z, unsigned char *Qx,
		unsigned char *Qy, unsigned char *r, unsigned char *s);

u32 XSecure_EcdsaKat(void);

#ifdef __cplusplus
}
#endif

#endif
/* @} */
