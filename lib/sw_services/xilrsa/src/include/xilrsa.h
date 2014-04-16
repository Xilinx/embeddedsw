/******************************************************************************
*
* (c) Copyright 2014 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
*******************************************************************************/
/*****************************************************************************/
/**
*
* @file xilrsa.h
*
* This file contains the RSA algorithm functions
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who	Date		Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   hk   27/01/14 First release
*
* </pre>
*
* @note
*
******************************************************************************/
#ifndef ___XIL_RSA_H___
#define ___XIL_RSA_H___

#ifdef __cplusplus
extern "C" {
#endif
/***************************** Include Files *********************************/

/*
 * Digit size selection (32 or 16-bit). If supported by the CPU/compiler,
 * 32-bit digits are approximately 4 times faster
 */

//#define RSA_DIGIT_16
#define RSA_DIGIT_32

/*
 * RSA loop unrolling selection
 * RSA main loop can be unrolled 2, 4 or 8 ways
 */
#define RSA_UNROLL	1

/*
 * Select if ARM-optimized code is to be used. Only GCC for ARM is supported
 */
//#define RSA_ARM_OPTIMIZED

/*
 * Check the compatibility of the selection
 */
#if defined(RSA_DIGIT_16) && defined(RSA_DIGIT_32)
	#error Please select a digit size
#endif
#if !defined(RSA_DIGIT_16) && !defined(RSA_DIGIT_32)
	#error Please select just one digit size
#endif
#if (!defined(__GNUC__) || !defined(__arm__)) && defined(RSA_ARM_OPTIMIZED)
	#error Assembly level code is only supported for the GCC/ARM combination
#endif
#if (RSA_UNROLL != 1) && (RSA_UNROLL != 2) && (RSA_UNROLL != 4) && (RSA_UNROLL != 8)
	#error Only 1, 2, 4, and 8 unrolling are supported
#endif

#ifdef RSA_DIGIT_16
#define RSA_DIGIT	unsigned short
#define RSA_SDIGIT	short
#define RSA_DDIGIT	unsigned long
#endif
#ifdef RSA_DIGIT_32
#define RSA_DIGIT	unsigned long
#define RSA_SDIGIT	long
#define RSA_DDIGIT	unsigned long long
#endif

#define RSA_NUMBER	RSA_DIGIT *
#define RSA_NBITS	2048
#define RSA_NDIGITS	(RSA_NBITS/(sizeof(RSA_DIGIT)*8))
#define RSA_NBYTES	(RSA_NDIGITS*sizeof(RSA_DIGIT))

/*
 * Double-digit to single digit conversion
 */
#define RSA_MSB(x)  (x >> (sizeof(RSA_DIGIT)*8))
#define RSA_LSB(x)  (x & (RSA_DIGIT)~0)

#define SHA_BLKSIZE		512
#define SHA_BLKBYTES	(SHA_BLKSIZE/8)
#define SHA_BLKWORDS	(SHA_BLKBYTES/4)

#define SHA_VALSIZE		256
#define SHA_VALBYTES	(SHA_VALSIZE/8)
#define SHA_VALWORDS	(SHA_VALBYTES/4)

/*
 * SHA-256 context structure
 * Includes SHA-256 state, coalescing buffer to collect the processed strings, and
 * total byte length counter (used both to manage the buffer and for padding)
 */
typedef struct
{
	unsigned int state[8];
	unsigned char buffer[SHA_BLKBYTES];
	unsigned long long bytes;
} sha2_context;

/*
 * RSA-2048 user interfaces
 */
void rsa2048_exp(const unsigned char *base, const unsigned char * modular,
		const unsigned char *modular_ext, const unsigned char *exponent,
		unsigned char *result);
void rsa2048_pubexp(RSA_NUMBER a, RSA_NUMBER x,
		unsigned long e, RSA_NUMBER m, RSA_NUMBER rrm);

/*
 * SHA-256 user interfaces
 */
void sha_256(const unsigned char *in, const unsigned int size, unsigned char *out);
void sha2_starts(sha2_context *ctx);
void sha2_update(sha2_context *ctx, unsigned char* input, unsigned int ilen);
void sha2_finish(sha2_context *ctx, unsigned char* output);

/*
 * Preprocessing interface (pre-computation of R*R mod M)
 */
void modular_ext(const unsigned char *modular, unsigned char *res);

#ifdef __cplusplus
}
#endif

#endif /* ___XIL_RSA_H___ */


