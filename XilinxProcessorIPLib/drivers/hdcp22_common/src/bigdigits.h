/* $Id: bigdigits.h $ */

/** @file
    Interface to core BigDigits "mp" functions using fixed-length arrays
*/

/***** BEGIN LICENSE BLOCK *****
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2001-15 David Ireland, D.I. Management Services Pty Limited
 * <http://www.di-mgt.com.au/bigdigits.html>. All rights reserved.
 *
 ***** END LICENSE BLOCK *****/
/*
 * Last updated:
 * $Date: 2015-10-22 10:23:00 $
 * $Revision: 2.5.0 $
 * $Author: dai $
 */

#ifndef BIGDIGITS_H_
#define BIGDIGITS_H_ 1

#include <stdlib.h>
#include <xil_printf.h>
#include "bigdtypes.h"

/**** USER CONFIGURABLE SECTION ****/

/* Disable dynamic memory allocation */
#define NO_ALLOCS

/* Define type and size of DIGIT */

/* [v2.1] Changed to use C99 exact-width types. */
/* [v2.2] Put macros for exact-width types in separate file "bigdtypes.h" */

/** The basic BigDigit element, an unsigned 32-bit integer */
typedef uint32_t DIGIT_T;
/** @cond */
typedef uint16_t HALF_DIGIT_T;

/* Sizes to match */
#define MAX_DIGIT 0xFFFFFFFFUL
#define MAX_HALF_DIGIT 0xFFFFUL	/* NB 'L' */
#define BITS_PER_DIGIT 32
#define HIBITMASK 0x80000000UL

/*	[v2.2] added option to avoid allocating temp storage
	and use fixed automatic arrays instead.
	Define NO_ALLOCS to invoke this.
	Only applicable to mp functions. Do not use with bd.
*/
/* Specify the maximum number of digits allowed in a temp mp array
   -- ignored unless NO_ALLOCS is defined */
#ifdef NO_ALLOCS
#define MAX_FIXED_BIT_LENGTH 4096
#define MAX_FIXED_DIGITS (MAX_FIXED_BIT_LENGTH / BITS_PER_DIGIT)
#endif

/**** END OF USER CONFIGURABLE SECTION ****/

/**** OPTIONAL PREPROCESSOR DEFINITIONS ****/
/*
   Choose one of {USE_SPASM | USE_64WITH32}
   USE_SPASM: to use the faster x86 ASM routines (if __asm option is available with your compiler).
   USE_64WITH32: to use the 64-bit integers if available (e.g. long long).
   Default: use default internal routines spDivide and spMultiply.
   The USE_SPASM option takes precedence over USE_64WITH32.
*/

/* Useful macros */
#define ISODD(x) ((x) & 0x1)
#define ISEVEN(x) (!ISODD(x))
#define mpISODD(x, n) (x[0] & 0x1)
#define mpISEVEN(x, n) (!(x[0] & 0x1))


#ifdef __cplusplus
extern "C" {
#endif

volatile const char *copyright_notice(void);
	/* Forces linker to include copyright notice in executable */
/** @endcond */

/*
 * Multiple precision calculations
 * Using known, equal ndigits
 * except where noted
 */

/*************************/
/* ARITHMETIC OPERATIONS */
/*************************/

/** Computes w = u + v, returns carry */
u32 mpAdd(u32 w[], const u32 u[], const u32 v[], size_t ndigits);

/** Computes w = u - v, returns borrow */
u32 mpSubtract(u32 w[], const u32 u[], const u32 v[], size_t ndigits);

/** Computes product w = u * v
@param[out] w To receive the product, an array of size 2 x `ndigits`
@param[in] u An array of size `ndigits`
@param[in] v An array of size `ndigits`
@param[in] ndigits size of arrays `u` and `v`
@warning The product must be of size 2 x `ndigits`
*/
int mpMultiply(u32 w[], const u32 u[], const u32 v[], size_t ndigits);

/** Computes integer division of u by v such that u=qv+r
@param[out] q to receive quotient = u div v, an array of size `udigits`
@param[out] r to receive divisor = u mod v, an array of size `udigits`
@param[in]  u dividend of size `udigits`
@param[in] udigits size of arrays `q` `r` and `u`
@param[in]  v divisor of size `vdigits`
@param[in] vdigits size of array `v`
@warning Trashes q and r first
*/
int mpDivide(u32 q[], u32 r[], const u32 u[],
	size_t udigits, u32 v[], size_t vdigits);

/** Computes remainder r = u mod v
@param[out] r to receive divisor = u mod v, an array of size `vdigits`
@param[in]  u dividend of size `udigits`
@param[in] udigits size of arrays `r` and `u`
@param[in]  v divisor of size `vdigits`
@param[in] vdigits size of array `v`
@remark Note that `r` is `vdigits` long here, but is `udigits` long in mpDivide().
*/
int mpModulo(u32 r[], const u32 u[], size_t udigits, u32 v[], size_t vdigits);

/** Computes square w = x^2
@param[out] w array of size 2 x `ndigits` to receive square
@param[in] x array of size `ndigits`
@param[in] ndigits size of array `x`
@warning The product `w` must be of size 2 x `ndigits`
*/
int mpSquare(u32 w[], const u32 x[], size_t ndigits);

/** Computes integer square root s = floor(sqrt(x)) */
int mpSqrt(u32 s[], const u32 x[], size_t ndigits);

/** Computes integer cube root s = floor(cuberoot(x)) */
int mpCubeRoot(u32 s[], const u32 x[], size_t ndigits);

/*************************/
/* COMPARISON OPERATIONS */
/*************************/
/* [v2.5] Changed to constant-time algorithms */

/** Returns true if a == b, else false, using constant-time algorithm
 *  @remark Constant-time with respect to `ndigits`
 */
int mpEqual(const u32 a[], const u32 b[], size_t ndigits);

/** Returns sign of `(a-b)` as `{-1,0,+1}` using constant-time algorithm
 *  @remark Constant-time with respect to `ndigits`
 */
int mpCompare(const u32 a[], const u32 b[], size_t ndigits);

/** Returns true if a is zero, else false, using constant-time algorithm
 *  @remark Constant-time with respect to `ndigits`
 */
int mpIsZero(const u32 a[], size_t ndigits);

/* OLDER, QUICKER VERSIONS */
/* Renamed in [v2.5] */

/** Returns true if a == b, else false (quick)
 *  @remark Not constant-time.
 */
int mpEqual_q(const u32 a[], const u32 b[], size_t ndigits);

/** Returns sign of `(a-b)` as `{-1,0,+1}` (quick)
 *  @remark Not constant-time.
 */
int mpCompare_q(const u32 a[], const u32 b[], size_t ndigits);

/** Returns true if a is zero, else false (quick)
 *  @remark Not constant-time.
 */
int mpIsZero_q(const u32 a[], size_t ndigits);


/****************************/
/* NUMBER THEORY OPERATIONS */
/****************************/
/* [v2.2] removed `const` restriction on m[] for mpModMult and mpModExp
 * (to allow faster in-place manipulation instead of using a temp variable).
 * [v2.5] added mpModExp_ct(), a constant-time variant of mpModExp().
 */

/** Computes y = x^e mod m */
int mpModExp(u32 y[], const u32 x[], const u32 e[], u32 m[], size_t ndigits);

/**	Computes y = x^e mod m in constant time
 *  @remark Resistant to simple power analysis attack on private exponent.
 *  Slower than mpModExp().
 */
int mpModExp_ct(u32 yout[], const u32 x[], const u32 e[], u32 m[], size_t ndigits);

/** Computes a = (x * y) mod m */
int mpModMult(u32 a[], const u32 x[], const u32 y[], u32 m[], size_t ndigits);

/** Computes the inverse of `u` modulo `m`, inv = u^{-1} mod m */
int mpModInv(u32 inv[], const u32 u[], const u32 m[], size_t ndigits);

/** Computes g = gcd(x, y), the greatest common divisor of x and y */
int mpGcd(u32 g[], const u32 x[], const u32 y[], size_t ndigits);

/** Returns the Jacobi symbol (a/n) in {-1, 0, +1}
@remark If n is prime then the Jacobi symbol becomes the Legendre symbol (a/p) defined to be
- (a/p) = +1 if a is a quadratic residue modulo p
- (a/p) = -1 if a is a quadratic non-residue modulo p
- (a/p) = 0 if a is divisible by p
*/
int mpJacobi(const u32 a[], const u32 n[], size_t ndigits);


/**********************/
/* BITWISE OPERATIONS */
/**********************/

/** Returns number of significant bits in a */
size_t mpBitLength(const u32 a[], size_t ndigits);

/** Computes a = b << x */
u32 mpShiftLeft(u32 a[], const u32 b[], size_t x, size_t ndigits);

/** Computes a = b >> x */
u32 mpShiftRight(u32 a[], const u32 b[], size_t x, size_t ndigits);

/** Computes bitwise a = b XOR c */
void mpXorBits(u32 a[], const u32 b[], const u32 c[], size_t ndigits);

/** Computes bitwise a = b OR c */
void mpOrBits(u32 a[], const u32 b[], const u32 c[], size_t ndigits);

/** Computes bitwise a = b AND c */
void mpAndBits(u32 a[], const u32 b[], const u32 c[], size_t ndigits);

/** Computes bitwise a = NOT b */
void mpNotBits(u32 a[], const u32 b[], size_t ndigits);

/** Computes a = a mod 2^L, ie clears all bits greater than L */
void mpModPowerOf2(u32 a[], size_t ndigits, size_t L);

/** Sets bit n of a (0..nbits-1) with value 1 or 0 */
int mpSetBit(u32 a[], size_t ndigits, size_t n, int value);

/** Returns value 1 or 0 of bit n (0..nbits-1) */
int mpGetBit(u32 a[], size_t ndigits, size_t n);


/*************************/
/* ASSIGNMENT OPERATIONS */
/*************************/

/** Sets a = 0 */
u32 mpSetZero(u32 a[], size_t ndigits);

/** Sets a = d where d is a single digit */
void mpSetDigit(u32 a[], u32 d, size_t ndigits);

/** Sets a = b */
void mpSetEqual(u32 a[], const u32 b[], size_t ndigits);


/**********************/
/* OTHER MP UTILITIES */
/**********************/

/** Returns number of significant non-zero digits in a */
size_t mpSizeof(const u32 a[], size_t ndigits);

/** Returns true (1) if `w` is probably prime
@param[in] w Number to test
@param[in] ndigits size of array `w`
@param[in] t The count of Rabin-Miller primality tests to carry out (recommended at least 80)
@returns true (1) if w is probably prime otherwise false (0)
@remark Uses FIPS-186-2/Rabin-Miller with trial division by small primes,
which is faster in most cases than mpRabinMiller().
@see mpRabinMiller().
*/
int mpIsPrime(u32 w[], size_t ndigits, size_t t);

/** Returns true (1) if `w` is probably prime using just the Rabin-Miller test
@see mpIsPrime() is preferred.
*/
int mpRabinMiller(u32 w[], size_t ndigits, size_t t);

/**********************************************/
/* FUNCTIONS THAT OPERATE WITH A SINGLE DIGIT */
/**********************************************/

/** Computes w = u + d, returns carry */
u32 mpShortAdd(u32 w[], const u32 u[], u32 d, size_t ndigits);

/** Computes w = u - d, returns borrow */
u32 mpShortSub(u32 w[], const u32 u[], u32 d, size_t ndigits);

/** Computes product p = x * d */
u32 mpShortMult(u32 p[], const u32 x[], u32 d, size_t ndigits);

/** Computes quotient q = u div d, returns remainder */
u32 mpShortDiv(u32 q[], const u32 u[], u32 d, size_t ndigits);

/** Computes remainder r = a mod d */
u32 mpShortMod(const u32 a[], u32 d, size_t ndigits);

/** Returns sign of (a - d) where d is a single digit */
int mpShortCmp(const u32 a[], u32 d, size_t ndigits);

/**************************************/
/* CORE SINGLE PRECISION CALCULATIONS */
/* (double where necessary)           */
/**************************************/

/* NOTE spMultiply and spDivide are used by almost all mp functions.
   Using the Intel MASM alternatives gives significant speed improvements
   -- to use, define USE_SPASM as a preprocessor directive.
   [v2.2] Removed references to spasm* versions.
*/

/** Computes p = x * y, where x and y are single digits */
int spMultiply(u32 p[2], u32 x, u32 y);

/** Computes quotient q = u div v, remainder r = u mod v, where q, r and v are single digits */
u32 spDivide(u32 *q, u32 *r, const u32 u[2], u32 v);

/****************************/
/* RANDOM NUMBER FUNCTIONS  */
/* CAUTION: NOT thread-safe */
/****************************/

/** Returns a simple pseudo-random digit between lower and upper.
@remark Not crypto secure.
@see spBetterRand()
*/
u32 spSimpleRand(u32 lower, u32 upper);

/** Generate a quick-and-dirty random mp number a of bit length at most `nbits` using plain-old-rand
@remark Not crypto secure.
@see mpRandomBits()
*/
size_t mpQuickRandBits(u32 a[], size_t ndigits, size_t nbits);

/* [Version 2.1: spBetterRand moved to spRandom.h] */

/*******************/
/* PRINT UTILITIES */
/*******************/

/* [v2.3] Added these more convenient print functions */

/** Print in hex format with optional prefix and suffix strings */
void mpPrintHex(const char *prefix, const u32 *p, size_t ndigits, const char *suffix);
/** Print in decimal format with optional prefix and suffix strings */
// void mpPrintDecimal(const char *prefix, const u32 *p, size_t ndigits, const char *suffix);

/* See also mpPrintDecimalSigned() - new in [v2.5] */

/* Older print functions, all printing in hex */
/** Print all digits in hex incl leading zero digits */
void mpPrint(const u32 *p, size_t ndigits);
/** Print all digits in hex with newlines */
void mpPrintNL(const u32 *p, size_t ndigits);
/** Print in hex but trim leading zero digits
@deprecated Use mpPrintHex()
*/
void mpPrintTrim(const u32 *p, size_t ndigits);
/** Print in hex, trim leading zeroes, add newlines
@deprecated Use mpPrintHex()
*/
void mpPrintTrimNL(const u32 *p, size_t ndigits);

/************************/
/* CONVERSION UTILITIES */
/************************/

/** Converts nbytes octets into big digit a of max size ndigits
@returns actual number of digits set */
size_t mpConvFromOctets(u32 a[], size_t ndigits, const unsigned char *c, size_t nbytes);
/** Converts big digit a into string of octets, in big-endian order, padding to nbytes or truncating if necessary.
@returns number of non-zero octets required. */
size_t mpConvToOctets(const u32 a[], size_t ndigits, unsigned char *c, size_t nbytes);

/****************************/
/* SIGNED INTEGER FUNCTIONS */
/****************************/

/*
NOTES ON SIGNED-INTEGER OPERATIONS
----------------------------------
You can choose to treat BigDigits integers as "signed" with their values stored in two's-complement representation.
A negative number will be a BigDigit integer with its left-most bit set to one, i.e.

    mpGetBit(a, ndigits, (ndigits * BITS_PER_DIGIT - 1)) == 1

This works automatically for simple arithmetic operations like add, subtract and multiply (but not division).
For example,

    mpSetDigit(u, 2, NDIGITS);
	mpSetDigit(v, 5, NDIGITS);
	mpSubtract(w, u, v, NDIGITS);
	mpPrintDecimalSigned("signed w=", w, NDIGITS, "\n");
	mpPrintHex("unsigned w=", w, NDIGITS, "\n");

will result in the output

    signed w=-3
    unsigned w=0xfffffffffffffffffffffffffffffffd

It does *not* work for division or any number-theoretic function like mpModExp(),
all of which treat their parameters as "unsigned" integers and will not give the "signed" result you expect.

To set a small negative number do:

    mpSetDigit(v, 5, NDIGITS);
    mpChs(v, v, NDIGITS);

--------------
*/


/** Returns true (1) if x < 0, else false (0)
 *  @remark Expects a negative number to be stored in two's-complement representation.
 */
int mpIsNegative(const u32 x[], size_t ndigits);

/** Sets x = -y
 *  @remark Expects a negative number to be stored in two's-complement representation.
 */
int mpChs(u32 x[], const u32 y[], size_t ndigits);

/** Sets x = |y|, the absolute value of y
 *  @remark Expects a negative number to be stored in two's-complement representation.
 */
int mpAbs(u32 x[], const u32 y[], size_t ndigits);

/****************/
/* VERSION INFO */
/****************/
/** Returns version number = major*1000+minor*100+release*10+PP_OPTIONS */
int mpVersion(void);
	/* Version number = major*1000+minor*100+release*10+uses_asm(0|1)+uses_64(0|2)+uses_noalloc(0|5)
		 E.g. Version 2.3.0 will return 230x where x denotes the preprocessor options
		 x | USE_SPASM | USE_64WITH32 | NO_ALLOCS
		 ----------------------------------------
		 0      No            No           No
		 1      Yes           No           No
		 2      No            Yes          No
		 3      Yes           Yes*         No
		 5      No            No           Yes
		 6      Yes           No           Yes
		 7      No            Yes          Yes
		 8      Yes           Yes*         Yes
		 ----------------------------------------
		 * USE_SPASM will take precedence over USE_64WITH32.
	 */

/** @cond */
/*************************************************************/
/* MEMORY ALLOCATION FUNCTIONS - USED INTERNALLY AND BY BIGD */
/*************************************************************/
/* [v2.2] added option to avoid memory allocation if NO_ALLOCS is defined */
#ifndef NO_ALLOCS
u32 *mpAlloc(size_t ndigits);
void mpFree(u32 **p);
#endif
void mpFail(char *msg);

/* Clean up by zeroising and freeing allocated memory */
#ifdef NO_ALLOCS
#define mpDESTROY(b, n) do{if(b != NULL)mpSetZero(b,n);}while(0)
#else
#define mpDESTROY(b, n) do{if(b != NULL)mpSetZero(b,n);mpFree(&b);}while(0)
#endif
/** @endcond */

#ifdef __cplusplus
}
#endif

#endif	/* BIGDIGITS_H_ */
