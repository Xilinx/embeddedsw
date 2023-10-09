/******************************************************************************
* Copyright (C) 2022 IP Cores, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**<
* RSA compact code for RSA5X, "quiet" version
* API definitions
*/
#ifndef RSA_H
#define RSA_H

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
externC int RSA_ExpQ(unsigned char* base, unsigned char* exp, unsigned char* mod,
	unsigned char* p, unsigned char* q, unsigned char* pub, unsigned char* tot, int len,
	unsigned char* res);

externC int RSA_ExpCrtQ(unsigned char* base, unsigned char* p, unsigned char* q,
	unsigned char* dp, unsigned char* dq, unsigned char* qinv, unsigned char* pub,
	unsigned char* mod, int len, unsigned char* res);

/** @} */

#endif  /* RSA_H_ */
