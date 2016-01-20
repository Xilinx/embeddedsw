/* $Id: bigdigits.h $ */

/** @file
    Interface to core BigDigits "mp" functions using fixed-length arrays
*/

/******************** SHORT COPYRIGHT NOTICE**************************
This source code is part of the BigDigits multiple-precision
arithmetic library Version 2.4 originally written by David Ireland,
copyright (c) 2001-13 D.I. Management Services Pty Limited, all rights
reserved. It is provided "as is" with no warranties. You may use
this software under the terms of the full copyright notice
"bigdigitsCopyright.txt" that should have been included with this
library or can be obtained from <www.di-mgt.com.au/bigdigits.html>.
This notice must always be retained in any copy.
******************* END OF COPYRIGHT NOTICE***************************/
/*
	Last updated:
	$Date: 2013-04-27 17:19:00 $
	$Revision: 2.4.0 $
	$Author: dai $
*/

#ifndef BIGDIGITS_H_
#define BIGDIGITS_H_ 1

#include <stdlib.h>
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
#define MAX_FIXED_DIGITS (8192 / BITS_PER_DIGIT)
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

volatile char *copyright_notice(void);
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
DIGIT_T mpAdd(DIGIT_T w[], const DIGIT_T u[], const DIGIT_T v[], size_t ndigits);

/** Computes w = u - v, returns borrow */
DIGIT_T mpSubtract(DIGIT_T w[], const DIGIT_T u[], const DIGIT_T v[], size_t ndigits);

/** Computes product w = u * v
@param[out] w To receive the product, an array of size 2 x \c ndigits
@param[in] u An array of size \c ndigits
@param[in] v An array of size \c ndigits
@param[in] ndigits size of arrays \c u and \c v
@warning The product must be of size 2 x \c ndigits
*/
int mpMultiply(DIGIT_T w[], const DIGIT_T u[], const DIGIT_T v[], size_t ndigits);

/** Computes integer division of u by v such that u=qv+r
@param[out] q to receive quotient = u div v, an array of size \c udigits
@param[out] r to receive divisor = u mod v, an array of size \c udigits
@param[in]  u dividend of size \c udigits
@param[in] udigits size of arrays \c q \c r and \c u
@param[in]  v divisor of size \c vdigits
@param[in] vdigits size of array \c v
@warning Trashes q and r first
*/
int mpDivide(DIGIT_T q[], DIGIT_T r[], const DIGIT_T u[],
	size_t udigits, DIGIT_T v[], size_t vdigits);

/** Computes remainder r = u mod v
@param[out] r to receive divisor = u mod v, an array of size \c vdigits
@param[in]  u dividend of size \c udigits
@param[in] udigits size of arrays \c r and \c u
@param[in]  v divisor of size \c vdigits
@param[in] vdigits size of array \c v
@remark Note that \c r is \c vdigits long here, but is \c udigits long in mpDivide().
*/
int mpModulo(DIGIT_T r[], const DIGIT_T u[], size_t udigits, DIGIT_T v[], size_t vdigits);

/** Computes square w = x^2
@param[out] w array of size 2 x \c ndigits to receive square
@param[in] x array of size \c ndigits
@param[in] ndigits size of array \c x
@warning The product \c w must be of size 2 x \c ndigits
*/
int mpSquare(DIGIT_T w[], const DIGIT_T x[], size_t ndigits);

/** Computes integer square root s = floor(sqrt(x)) */
int mpSqrt(DIGIT_T s[], const DIGIT_T x[], size_t ndigits);

/** Computes integer cube root s = floor(cuberoot(x)) */
int mpCubeRoot(DIGIT_T s[], const DIGIT_T x[], size_t ndigits);

//*************************/
/* COMPARISON OPERATIONS */
/*************************/

/** Returns true (1) if a == b, else false (0) */
int mpEqual(const DIGIT_T a[], const DIGIT_T b[], size_t ndigits);

/** Returns sign of \c {0,1,-1) of \c (a-b) */
int mpCompare(const DIGIT_T a[], const DIGIT_T b[], size_t ndigits);

/** Returns true (1) if a is zero, else false (0) */
int mpIsZero(const DIGIT_T a[], size_t ndigits);

/****************************/
/* NUMBER THEORY OPERATIONS */
/****************************/

/* [v2.2] removed `const' restriction on m[] for mpModMult and mpModExp */

/** Computes a = (x * y) mod m */
int mpModMult(DIGIT_T a[], const DIGIT_T x[], const DIGIT_T y[], DIGIT_T m[], size_t ndigits);

/** Computes y = x^e mod m */
int mpModExp(DIGIT_T y[], const DIGIT_T x[], const DIGIT_T e[], DIGIT_T m[], size_t ndigits);

/** Computes the inverse of \c u modulo \c v, inv = u^{-1} mod v */
int mpModInv(DIGIT_T inv[], const DIGIT_T u[], const DIGIT_T v[], size_t ndigits);

/** Computes g = gcd(x, y), the greatest common divisor of x and y */
int mpGcd(DIGIT_T g[], const DIGIT_T x[], const DIGIT_T y[], size_t ndigits);

/** Returns the Jacobi symbol (a/n) = {-1, 0, +1}
@remark If n is prime then the Jacobi symbol becomes the Legendre symbol (a/p) defined to be
- (a/p) = +1 if a is a quadratic residue modulo p
- (a/p) = -1 if a is a quadratic non-residue modulo p
- (a/p) = 0 if a is divisible by p
*/
int mpJacobi(const DIGIT_T a[], const DIGIT_T n[], size_t ndigits);

/**********************/
/* BITWISE OPERATIONS */
/**********************/

/** Returns number of significant bits in a */
size_t mpBitLength(const DIGIT_T a[], size_t ndigits);

/** Computes a = b << x */
DIGIT_T mpShiftLeft(DIGIT_T a[], const DIGIT_T b[], size_t x, size_t ndigits);

/** Computes a = b >> x */
DIGIT_T mpShiftRight(DIGIT_T a[], const DIGIT_T b[], size_t x, size_t ndigits);

/** Computes bitwise a = b XOR c */
void mpXorBits(DIGIT_T a[], const DIGIT_T b[], const DIGIT_T c[], size_t ndigits);

/** Computes bitwise a = b OR c */
void mpOrBits(DIGIT_T a[], const DIGIT_T b[], const DIGIT_T c[], size_t ndigits);

/** Computes bitwise a = b AND c */
void mpAndBits(DIGIT_T a[], const DIGIT_T b[], const DIGIT_T c[], size_t ndigits);

/** Computes bitwise a = NOT b */
void mpNotBits(DIGIT_T a[], const DIGIT_T b[], size_t ndigits);

/** Computes a = a mod 2^L, ie clears all bits greater than L */
void mpModPowerOf2(DIGIT_T a[], size_t ndigits, size_t L);

/** Sets bit n of a (0..nbits-1) with value 1 or 0 */
int mpSetBit(DIGIT_T a[], size_t ndigits, size_t n, int value);

/** Returns value 1 or 0 of bit n (0..nbits-1) */
int mpGetBit(DIGIT_T a[], size_t ndigits, size_t n);

/*************************/
/* ASSIGNMENT OPERATIONS */
/*************************/

/** Sets a = 0 */
volatile DIGIT_T mpSetZero(volatile DIGIT_T a[], size_t ndigits);

/** Sets a = d where d is a single digit */
void mpSetDigit(DIGIT_T a[], DIGIT_T d, size_t ndigits);

/** Sets a = b */
void mpSetEqual(DIGIT_T a[], const DIGIT_T b[], size_t ndigits);

/****************************/
/* SIGNED INTEGER FUNCTIONS */
/****************************/

/** Returns true (1) if x < 0, else false (0)
@remark Negative numbers are stored in two's-complement representation. Use at your own risk.
*/
int mpIsNegative(const DIGIT_T x[], size_t ndigits);

/** Sets x = -y
@remark Negative numbers are stored in two's-complement representation. Use at your own risk.
*/
int mpChs(DIGIT_T x[], const DIGIT_T y[], size_t ndigits);

/** Sets x = |y|, the absolute value of y
@remark Negative numbers are stored in two's-complement representation. Use at your own risk.
*/
int mpAbs(DIGIT_T x[], const DIGIT_T y[], size_t ndigits);

/**********************/
/* OTHER MP UTILITIES */
/**********************/

/** Returns number of significant non-zero digits in a */
size_t mpSizeof(const DIGIT_T a[], size_t ndigits);

/** Returns true (1) if \c w is probably prime
@param[in] w Number to test
@param[in] ndigits size of array \c w
@param[in] t The count of Rabin-Miller primality tests to carry out (recommended at least 80)
@returns true (1) if w is probably prime otherwise false (0)
@remark Uses FIPS-186-2/Rabin-Miller with trial division by small primes,
which is faster in most cases than mpRabinMiller().
@see mpRabinMiller().
*/
int mpIsPrime(DIGIT_T w[], size_t ndigits, size_t t);

/** Returns true (1) if \c w is probably prime using just the Rabin-Miller test
@see mpIsPrime() is preferred.
*/
int mpRabinMiller(DIGIT_T w[], size_t ndigits, size_t t);

/**********************************************/
/* FUNCTIONS THAT OPERATE WITH A SINGLE DIGIT */
/**********************************************/

/** Computes w = u + d, returns carry */
DIGIT_T mpShortAdd(DIGIT_T w[], const DIGIT_T u[], DIGIT_T d, size_t ndigits);

/** Computes w = u - d, returns borrow */
DIGIT_T mpShortSub(DIGIT_T w[], const DIGIT_T u[], DIGIT_T d, size_t ndigits);

/** Computes product p = x * d */
DIGIT_T mpShortMult(DIGIT_T p[], const DIGIT_T x[], DIGIT_T d, size_t ndigits);

/** Computes quotient q = u div d, returns remainder */
DIGIT_T mpShortDiv(DIGIT_T q[], const DIGIT_T u[], DIGIT_T d, size_t ndigits);

/** Computes remainder r = a mod d */
DIGIT_T mpShortMod(const DIGIT_T a[], DIGIT_T d, size_t ndigits);

/** Returns sign of (a - d) where d is a single digit */
int mpShortCmp(const DIGIT_T a[], DIGIT_T d, size_t ndigits);

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
int spMultiply(DIGIT_T p[2], DIGIT_T x, DIGIT_T y);

/** Computes quotient q = u div v, remainder r = u mod v, where q, r and v are single digits */
DIGIT_T spDivide(DIGIT_T *q, DIGIT_T *r, const DIGIT_T u[2], DIGIT_T v);

/****************************/
/* RANDOM NUMBER FUNCTIONS  */
/* CAUTION: NOT thread-safe */
/****************************/

/** Returns a simple pseudo-random digit between lower and upper.
@remark Not crypto secure.
@see spBetterRand
*/
DIGIT_T spSimpleRand(DIGIT_T lower, DIGIT_T upper);

/** Generate a quick-and-dirty random mp number a of bit length at most \c nbits using plain-old-rand
@remark Not crypto secure.
@see mpRandomBits()
*/
size_t mpQuickRandBits(DIGIT_T a[], size_t ndigits, size_t nbits);

/* [Version 2.1: spBetterRand moved to spRandom.h] */

/*******************/
/* PRINT UTILITIES */
/*******************/

/* [v2.3] Added these more convenient print functions */
/** Print in hex format with optional prefix and suffix strings */
void mpPrintHex(const char *prefix, const DIGIT_T *p, size_t len, const char *suffix);
/** Print in decimal format with optional prefix and suffix strings */
void mpPrintDecimal(const char *prefix, const DIGIT_T *p, size_t len, const char *suffix);

/* Older print functions, all printing in hex */
/** Print all digits in hex incl leading zero digits */
void mpPrint(const DIGIT_T *p, size_t len);
/** Print all digits in hex with newlines */
void mpPrintNL(const DIGIT_T *p, size_t len);
/** Print in hex but trim leading zero digits
@deprecated Use mpPrintHex()
*/
void mpPrintTrim(const DIGIT_T *p, size_t len);
/** Print in hex, trim leading zeroes, add newlines
@deprecated Use mpPrintHex()
*/
void mpPrintTrimNL(const DIGIT_T *p, size_t len);

/************************/
/* CONVERSION UTILITIES */
/************************/

/** Converts nbytes octets into big digit a of max size ndigits
@returns actual number of digits set */
size_t mpConvFromOctets(DIGIT_T a[], size_t ndigits, const unsigned char *c, size_t nbytes);
/** Converts big digit a into string of octets, in big-endian order, padding to nbytes or truncating if necessary.
@returns number of non-zero octets required. */
size_t mpConvToOctets(const DIGIT_T a[], size_t ndigits, unsigned char *c, size_t nbytes);
/** Converts a string in decimal format to a big digit.
@returns actual number of (possibly zero) digits set. */
size_t mpConvFromDecimal(DIGIT_T a[], size_t ndigits, const char *s);
/** Converts big digit a into a string in decimal format, where s has size smax including the terminating zero.
@returns number of chars required excluding leading zeroes. */
size_t mpConvToDecimal(const DIGIT_T a[], size_t ndigits, char *s, size_t smax);
/** Converts a string in hexadecimal format to a big digit.
@return actual number of (possibly zero) digits set. */
size_t mpConvFromHex(DIGIT_T a[], size_t ndigits, const char *s);
/** Converts big digit a into a string in hexadecimal format,
   where s has size smax including the terminating zero.
@return number of chars required excluding leading zeroes. */
size_t mpConvToHex(const DIGIT_T a[], size_t ndigits, char *s, size_t smax);

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
DIGIT_T *mpAlloc(size_t ndigits);
void mpFree(DIGIT_T **p);
#endif
void mpFail(char *msg);

/* Clean up by zeroising and freeing allocated memory */
#ifdef NO_ALLOCS
#define mpDESTROY(b, n) do{if(b)mpSetZero(b,n);}while(0)
#else
#define mpDESTROY(b, n) do{if(b)mpSetZero(b,n);mpFree(&b);}while(0)
#endif
/** @endcond */

#ifdef __cplusplus
}
#endif

#endif	/* BIGDIGITS_H_ */
