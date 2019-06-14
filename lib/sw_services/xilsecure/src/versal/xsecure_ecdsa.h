/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
*
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

#ifdef __cplusplus
extern "C" }
#endif

#endif
/* @} */
