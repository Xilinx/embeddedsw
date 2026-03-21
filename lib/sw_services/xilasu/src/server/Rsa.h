/******************************************************************************
* Copyright (C) 2022 IP Cores, Inc.  All rights reserved.
* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**<
* RSA compact code for RSA5X, "quiet" version
* API definitions
*/
#ifndef RSA_H
#define RSA_H
#include "xil_types.h"
/**< For compilation in C++ */
#ifdef __cplusplus
#define externC extern "C"
#else
#define externC extern
#endif

/**
 * @name IPCores RSA quiet mode API declarations
 * @{
 */
/**< Prototype declarations for IPCores APIs for RSA "quiet" operations */
externC int RSA_ExpQ(unsigned char *base, unsigned char *exp, unsigned char *mod,
		     unsigned char *p, unsigned char *q, unsigned char *pub, unsigned char *tot, int len,
		     unsigned char *res);

externC int RSA_ExpCrtQ(unsigned char *base, unsigned char *p, unsigned char *q,
			unsigned char *dp, unsigned char *dq, unsigned char *qinv, unsigned char *pub,
			unsigned char *mod, int len, unsigned char *res);

externC int RSA_ExpoptQ(unsigned char *base, unsigned char *exp, unsigned char *mod,
			unsigned char *r, unsigned char *rr, unsigned char *p, unsigned char *q,
			unsigned char *pub, unsigned char *tot, int len, unsigned char *res);

externC void rsaexpopt(unsigned char *base, unsigned char *exp, unsigned char *mod,
		       unsigned char *rr, int len, unsigned char *res);

externC void rsaexp(unsigned char *base, unsigned char *exp, unsigned char *mod, int len,
		    unsigned char *res);
//
// RSA key pair definition
//
typedef struct {
	u8* E;	// Public exponent (e)
	u8* M;	// Modulus
	u8* D;	// Secret exponent (d)
	u8* P;	// Secret prime (P, half-size)
	u8* Q;	// Secret prime (Q, half-size)
	u8* DP;	// Secret value (dP, half-size)
	u8* DQ;	// Secret value (dQ, half-size)
	u8* iQ;	// Secret value (inverted Q, half-size)
    // Additional state machine variables
    unsigned stage;     // State machine stage
    unsigned s;         // MR 's' value
    unsigned bits2;     // Half-size in bits
    unsigned iter;      // Number of MR iterations remaining
} RsaKeyPair;

//
// Initialize the key generation
//
// Returns: 0 if parameters validated successfully, error otherwise
// Input:	bits  – number of bits in the RSA key (2048, 3072, etc.)
//			e     - pointer to a public exponent (65537 will be used if NULL)
// Output:	state – pointer to an RsaKeyGen structure
//
externC int rsaprvkeyinit_Q(u32 bits, u8* e, RsaKeyPair* state);

//
// Perform a step of the key generation
//
// Returns: 0 if completed successfully, 1 if unfinished, error otherwise
// Input:	quant – incremental effort (in half-size RSA exponentiations)
// I/O:    	state – pointer to an RsaKeyGen structure
//
externC int rsaprvkeystep_Q(u32 quant, RsaKeyPair* state);
/** @} */

#endif  /* RSA_H_ */
