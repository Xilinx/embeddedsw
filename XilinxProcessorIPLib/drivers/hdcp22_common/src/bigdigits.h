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

/**
 * Add two multi-precision integers.
 *
 * @param w Output array to store the sum
 * @param u First input multi-precision integer
 * @param v Second input multi-precision integer
 * @param ndigits Number of digits in the integers
 * @return Carry value from the addition
 */
u32 mpAdd(u32 w[], const u32 u[], const u32 v[], size_t ndigits);

/** Computes w = u - v, returns borrow */
/**
 * Subtract two multi-precision integers.
 *
 * @param w Output array to store the result (u - v)
 * @param u First input array (minuend)
 * @param v Second input array (subtrahend)
 * @param ndigits Number of digits in the arrays
 * @return Borrow value (0 if no borrow, non-zero otherwise)
 */

u32 mpSubtract(u32 w[], const u32 u[], const u32 v[], size_t ndigits);

/** Computes w = u * v */
/**
 * Multiply two multi-precision integers.
 *
 * @param w Output array to store the result (u - v)
 * @param u First input array (minuend)
 * @param v Second input array (subtrahend)
 * @param ndigits Number of digits in the arrays
 * @return Borrow value (0 if no borrow, non-zero otherwise
 */
int mpMultiply(u32 w[], const u32 u[], const u32 v[], size_t ndigits);

/**
 * Performs integer division: u = q*v + r
 *
 * @param q Output quotient array (size: udigits)
 * @param r Output remainder array (size: udigits)
 * @param u Input dividend array (size: udigits)
 * @param udigits Size of q, r, and u arrays
 * @param v Input divisor array (size: vdigits)
 * @param vdigits Size of v array
 * @return 0 if successful
 *
 * @note q and r are overwritten without preservation
 */
int mpDivide(u32 q[], u32 r[], const u32 u[],
	size_t udigits, u32 v[], size_t vdigits);

/**
 * Computes remainder r = u mod v
 *
 * @param r Output remainder array (size: vdigits)
 * @param u Input dividend array (size: udigits)
 * @param udigits Size of u array
 * @param v Input divisor array (size: vdigits)
 * @param vdigits Size of r and v arrays
 * @return 0 if successful
 */
int mpModulo(u32 r[], const u32 u[], size_t udigits, u32 v[], size_t vdigits);

/**
 * Computes the square of a big digit number.
 *
 * @param w Output array to store the result (size: 2 * ndigits)
 * @param x Input array to be squared (size: ndigits)
 * @param ndigits Number of digits in input array
 * @return Overflow value from the squaring operation
 */
int mpSquare(u32 w[], const u32 x[], size_t ndigits);

/** Computes integer square root s = floor(sqrt(x)) */
/**
 * Computes the square root of a multi-precision integer.
 *
 * @param s Output array to store the square root result
 * @param x Input array containing the value to compute square root of
 * @param ndigits Number of digits in the multi-precision integer
 * @return Status code indicating success or failure
 */

int mpSqrt(u32 s[], const u32 x[], size_t ndigits);

/** Computes integer cube root s = floor(cuberoot(x)) */
/**
 * Computes the cube root of a multi-precision integer.
 *
 * @param s Output array to store the cube root result
 * @param x Input array containing the value to compute cube root of
 * @param ndigits Number of digits in the multi-precision integer
 * @return Status code indicating success or failure
 */
int mpCubeRoot(u32 s[], const u32 x[], size_t ndigits);

/*************************/
/* COMPARISON OPERATIONS */
/*************************/
/* [v2.5] Changed to constant-time algorithms */

/** Returns true if a == b, else false, using constant-time algorithm
 *  @remark Constant-time with respect to `ndigits`
 */
/**
 * Compares two multi-precision integers for equality.
 *
 * @param a Pointer to first multi-precision integer array
 * @param b Pointer to second multi-precision integer array
 * @param ndigits Number of digits to compare
 * @return Non-zero if equal, zero if not equal
 */

int mpEqual(const u32 a[], const u32 b[], size_t ndigits);

/** Returns sign of `(a-b)` as `{-1,0,+1}` using constant-time algorithm
 *  @remark Constant-time with respect to `ndigits`
 */

/**
 * Compares two multi-precision integers.
 *
 * @param a First multi-precision integer array
 * @param b Second multi-precision integer array
 * @param ndigits Number of digits to compare
 * @return -1 if a < b, 0 if a == b, +1 if a > b
 */
int mpCompare(const u32 a[], const u32 b[], size_t ndigits);

/** Returns true if a is zero, else false, using constant-time algorithm
 *  @remark Constant-time with respect to `ndigits`
 */


/**
 * Checks if a multi-precision integer is zero.
 *
 * @param a Multi-precision integer array to check
 * @param ndigits Number of digits in the array
 * @return Non-zero if a is zero, zero otherwise
 */
int mpIsZero(const u32 a[], size_t ndigits);

/* OLDER, QUICKER VERSIONS */
/* Renamed in [v2.5] */

/** Returns true if a == b, else false (quick)
 *  @remark Not constant-time.
 */

/**
 * @brief Compares two multi-precision integers for equality.
 *
 * @param a Pointer to the first multi-precision integer array.
 * @param b Pointer to the second multi-precision integer array.
 * @param ndigits Number of digits to compare.
 *
 * @return Non-zero if equal, 0 if not equal.
 */

int mpEqual_q(const u32 a[], const u32 b[], size_t ndigits);

/** Returns sign of `(a-b)` as `{-1,0,+1}` (quick)
 *  @remark Not constant-time.
 */



/**
 * Compares two multi-precision integers (quick version).
 *
 * @param a First multi-precision integer array
 * @param b Second multi-precision integer array
 * @param ndigits Number of digits to compare
 * @return -1 if a < b, 0 if a == b, +1 if a > b
 */
int mpCompare_q(const u32 a[], const u32 b[], size_t ndigits);

/**
 * Checks if a multi-precision integer is zero (quick version).
 *
 * @param a Multi-precision integer array to check
 * @param ndigits Number of digits in the array
 * @return Non-zero if a is zero, zero otherwise
 */
int mpIsZero_q(const u32 a[], size_t ndigits);


/****************************/
/* NUMBER THEORY OPERATIONS */
/****************************/
/* [v2.2] removed `const` restriction on m[] for mpModMult and mpModExp
 * (to allow faster in-place manipulation instead of using a temp variable).
 * [v2.5] added mpModExp_ct(), a constant-time variant of mpModExp().
 */

/**
 * Computes modular exponentiation y = x^e mod m
 *
 * @param y Output array to store the result
 * @param x Base array
 * @param e Exponent array
 * @param m Modulus array
 * @param ndigits Number of digits in the arrays
 * @return Status code indicating success or failure
 */
int mpModExp(u32 y[], const u32 x[], const u32 e[], u32 m[], size_t ndigits);

/**
 * Computes modular exponentiation y = x^e mod m in constant time
 *
 * @param yout Output array to store the result
 * @param x Base array
 * @param e Exponent array
 * @param m Modulus array
 * @param ndigits Number of digits in the arrays
 * @return Status code indicating success or failure
 * @remark Resistant to simple power analysis attack on private exponent.
 * Slower than mpModExp().
 */
int mpModExp_ct(u32 yout[], const u32 x[], const u32 e[], u32 m[], size_t ndigits);

/**
 * Computes modular multiplication a = (x * y) mod m
 *
 * @param a Output array to store the result
 * @param x First input array
 * @param y Second input array
 * @param m Modulus array
 * @param ndigits Number of digits in the arrays
 * @return Status code indicating success or failure
 */
int mpModMult(u32 a[], const u32 x[], const u32 y[], u32 m[], size_t ndigits);

/**
 * Computes the modular multiplicative inverse inv = u^{-1} mod m
 *
 * @param inv Output array to store the inverse
 * @param u Input array to invert
 * @param m Modulus array
 * @param ndigits Number of digits in the arrays
 * @return Status code indicating success or failure
 */
int mpModInv(u32 inv[], const u32 u[], const u32 m[], size_t ndigits);

/**
 * Computes the greatest common divisor g = gcd(x, y)
 *
 * @param g Output array to store the GCD
 * @param x First input array
 * @param y Second input array
 * @param ndigits Number of digits in the arrays
 * @return Status code indicating success or failure
 */
int mpGcd(u32 g[], const u32 x[], const u32 y[], size_t ndigits);

/**
 * Computes the Jacobi symbol (a/n)
 *
 * @param a Input array (numerator)
 * @param n Input array (denominator, must be odd)
 * @param ndigits Number of digits in the arrays
 * @return Jacobi symbol value: -1, 0, or +1
 * @remark If n is prime then the Jacobi symbol becomes the Legendre symbol (a/p) defined to be
 * - (a/p) = +1 if a is a quadratic residue modulo p
 * - (a/p) = -1 if a is a quadratic non-residue modulo p
 * - (a/p) = 0 if a is divisible by p
 */
int mpJacobi(const u32 a[], const u32 n[], size_t ndigits);


/**********************/
/* BITWISE OPERATIONS */
/**********************/

/** Returns number of significant bits in a */
/**
 * Returns number of significant bits in a
 *
 * @param a Multi-precision integer array
 * @param ndigits Number of digits in the array
 * @return Number of significant bits
 */
size_t mpBitLength(const u32 a[], size_t ndigits);

/**
 * Computes a = b << x (shift left)
 *
 * @param a Output array for result
 * @param b Input array to shift
 * @param x Number of bits to shift left
 * @param ndigits Number of digits in arrays
 * @return Overflow bits
 */
u32 mpShiftLeft(u32 a[], const u32 b[], size_t x, size_t ndigits);

/**
 * Computes a = b >> x (shift right)
 *
 * @param a Output array for result
 * @param b Input array to shift
 * @param x Number of bits to shift right
 * @param ndigits Number of digits in arrays
 * @return Underflow bits
 */
u32 mpShiftRight(u32 a[], const u32 b[], size_t x, size_t ndigits);

/**
 * Computes bitwise a = b XOR c
 *
 * @param a Output array for result
 * @param b First input array
 * @param c Second input array
 * @param ndigits Number of digits in arrays
 */
void mpXorBits(u32 a[], const u32 b[], const u32 c[], size_t ndigits);

/**
 * Computes bitwise a = b OR c
 *
 * @param a Output array for result
 * @param b First input array
 * @param c Second input array
 * @param ndigits Number of digits in arrays
 */
void mpOrBits(u32 a[], const u32 b[], const u32 c[], size_t ndigits);

/**
 * Computes bitwise a = b AND c
 *
 * @param a Output array for result
 * @param b First input array
 * @param c Second input array
 * @param ndigits Number of digits in arrays
 */
void mpAndBits(u32 a[], const u32 b[], const u32 c[], size_t ndigits);

/**
 * Computes bitwise a = NOT b
 *
 * @param a Output array for result
 * @param b Input array
 * @param ndigits Number of digits in arrays
 */
void mpNotBits(u32 a[], const u32 b[], size_t ndigits);

/**
 * Computes a = a mod 2^L (clears all bits greater than L)
 *
 * @param a Array to modify
 * @param ndigits Number of digits in array
 * @param L Bit position threshold
 */
void mpModPowerOf2(u32 a[], size_t ndigits, size_t L);

/**
 * Sets bit n of a with specified value
 *
 * @param a Array to modify
 * @param ndigits Number of digits in array
 * @param n Bit position (0..nbits-1)
 * @param value Bit value (1 or 0)
 * @return Status code
 */
int mpSetBit(u32 a[], size_t ndigits, size_t n, int value);

/**
 * Returns value of bit n
 *
 * @param a Array to read from
 * @param ndigits Number of digits in array
 * @param n Bit position (0..nbits-1)
 * @return Bit value (1 or 0)
 */
int mpGetBit(u32 a[], size_t ndigits, size_t n);


/*************************/
/* ASSIGNMENT OPERATIONS */
/*************************/

/**
 * Sets a multi-precision integer to zero
 *
 * @param a Array to set to zero
 * @param ndigits Number of digits in array
 * @return Always returns 0
 */
u32 mpSetZero(u32 a[], size_t ndigits);

/**
 * Sets a multi-precision integer to a single digit value
 *
 * @param a Array to set
 * @param d Single digit value to assign
 * @param ndigits Number of digits in array
 */
void mpSetDigit(u32 a[], u32 d, size_t ndigits);

/**
 * Copies one multi-precision integer to another
 *
 * @param a Destination array
 * @param b Source array to copy from
 * @param ndigits Number of digits to copy
 */
void mpSetEqual(u32 a[], const u32 b[], size_t ndigits);


/**********************/
/* OTHER MP UTILITIES */
/**********************/

/**
 * Returns number of significant non-zero digits in a
 *
 * @param a Multi-precision integer array
 * @param ndigits Number of digits in the array
 * @return Number of significant non-zero digits
 */
size_t mpSizeof(const u32 a[], size_t ndigits);

/**
 * Returns true (1) if w is probably prime
 *
 * @param w Number to test
 * @param ndigits Size of array w
 * @param t The count of Rabin-Miller primality tests to carry out (recommended at least 80)
 * @return True (1) if w is probably prime otherwise false (0)
 * @remark Uses FIPS-186-2/Rabin-Miller with trial division by small primes,
 * which is faster in most cases than mpRabinMiller().
 * @see mpRabinMiller()
 */
int mpIsPrime(u32 w[], size_t ndigits, size_t t);

/**
 * Returns true (1) if w is probably prime using just the Rabin-Miller test
 *
 * @param w Number to test
 * @param ndigits Size of array w
 * @param t The count of Rabin-Miller primality tests to carry out
 * @return True (1) if w is probably prime otherwise false (0)
 * @see mpIsPrime() is preferred
 */
int mpRabinMiller(u32 w[], size_t ndigits, size_t t);

/**********************************************/
/* FUNCTIONS THAT OPERATE WITH A SINGLE DIGIT */
/**********************************************/


/**
 * Computes w = u + d
 *
 * @param w Output array for result
 * @param u Input multi-precision integer array
 * @param d Single digit to add
 * @param ndigits Number of digits in arrays
 * @return Carry value from the addition
 */
u32 mpShortAdd(u32 w[], const u32 u[], u32 d, size_t ndigits);

/**
 * Computes w = u - d
 *
 * @param w Output array for result
 * @param u Input multi-precision integer array
 * @param d Single digit to subtract
 * @param ndigits Number of digits in arrays
 * @return Borrow value from the subtraction
 */
u32 mpShortSub(u32 w[], const u32 u[], u32 d, size_t ndigits);

/**
 * Computes product p = x * d
 *
 * @param p Output array for result
 * @param x Input multi-precision integer array
 * @param d Single digit multiplier
 * @param ndigits Number of digits in arrays
 * @return Overflow from multiplication
 */
u32 mpShortMult(u32 p[], const u32 x[], u32 d, size_t ndigits);

/**
 * Computes quotient q = u / d
 *
 * @param q Output array for quotient
 * @param u Input multi-precision integer array (dividend)
 * @param d Single digit divisor
 * @param ndigits Number of digits in arrays
 * @return Remainder from the division
 */
u32 mpShortDiv(u32 q[], const u32 u[], u32 d, size_t ndigits);

/**
 * Computes remainder r = a mod d
 *
 * @param a Input multi-precision integer array
 * @param d Single digit modulus
 * @param ndigits Number of digits in array
 * @return Remainder value
 */
u32 mpShortMod(const u32 a[], u32 d, size_t ndigits);

/**
 * Compares multi-precision integer with single digit
 *
 * @param a Multi-precision integer array
 * @param d Single digit to compare
 * @param ndigits Number of digits in array
 * @return -1 if a < d, 0 if a == d, +1 if a > d
 */
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

/**
 * Computes product of two single digits: p = x * y
 *
 * @param p Output array to store the product (size: 2)
 * @param x First single digit multiplicand
 * @param y Second single digit multiplicand
 * @return Status code indicating success or failure
 */
int spMultiply(u32 p[2], u32 x, u32 y);

/**
 * Computes quotient and remainder: q = u / v, r = u % v
 *
 * @param q Output pointer to store the quotient (single digit)
 * @param r Output pointer to store the remainder (single digit)
 * @param u Input dividend array (size: 2)
 * @param v Single digit divisor
 * @return The quotient value
 */
u32 spDivide(u32 *q, u32 *r, const u32 u[2], u32 v);

/****************************/
/* RANDOM NUMBER FUNCTIONS  */
/* CAUTION: NOT thread-safe */
/****************************/

/** Returns a simple pseudo-random digit between lower and upper.
@remark Not crypto secure.
@see spBetterRand()
*/
/**
 * Returns a simple pseudo-random digit between lower and upper
 *
 * @param lower Lower bound (inclusive)
 * @param upper Upper bound (inclusive)
 * @return Random digit value between lower and upper
 * @remark Not crypto secure.
 * @see spBetterRand()
 */
u32 spSimpleRand(u32 lower, u32 upper);

/**
 * Generate a quick-and-dirty random mp number a of bit length at most `nbits` using plain-old-rand
 *
 * @param a Output array to store random number
 * @param ndigits Number of digits in array
 * @param nbits Maximum bit length of random number
 * @return Actual number of significant digits set
 * @remark Not crypto secure.
 * @see mpRandomBits()
 */
size_t mpQuickRandBits(u32 a[], size_t ndigits, size_t nbits);

/* [Version 2.1: spBetterRand moved to spRandom.h] */

/*******************/
/* PRINT UTILITIES */
/*******************/

/* [v2.3] Added these more convenient print functions */

/**
 * Print multi-precision integer in hexadecimal format
 *
 * @param prefix String to print before the number (can be NULL)
 * @param p Multi-precision integer array to print
 * @param ndigits Number of digits in the array
 * @param suffix String to print after the number (can be NULL)
 */
void mpPrintHex(const char *prefix, const u32 *p, size_t ndigits, const char *suffix);

// void mpPrintDecimal(const char *prefix, const u32 *p, size_t ndigits, const char *suffix);

/* See also mpPrintDecimalSigned() - new in [v2.5] */

/* Older print functions, all printing in hex */

/**
 * Print all digits in hexadecimal including leading zeros
 *
 * @param p Multi-precision integer array to print
 * @param ndigits Number of digits in the array
 */
void mpPrint(const u32 *p, size_t ndigits);

/**
 * Print all digits in hexadecimal with newlines
 *
 * @param p Multi-precision integer array to print
 * @param ndigits Number of digits in the array
 */
void mpPrintNL(const u32 *p, size_t ndigits);

/**
 * Print in hexadecimal but trim leading zero digits
 *
 * @param p Multi-precision integer array to print
 * @param ndigits Number of digits in the array
 * @deprecated Use mpPrintHex()
 */
void mpPrintTrim(const u32 *p, size_t ndigits);

/**
 * Print in hexadecimal, trim leading zeros, and add newlines
 *
 * @param p Multi-precision integer array to print
 * @param ndigits Number of digits in the array
 * @deprecated Use mpPrintHex()
 */
void mpPrintTrimNL(const u32 *p, size_t ndigits);

/************************/
/* CONVERSION UTILITIES */
/************************/

/**
 * Converts array of octets to multi-precision integer
 *
 * @param a Output array to store the converted number
 * @param ndigits Maximum size of output array
 * @param c Input array of octets (bytes) in big-endian order
 * @param nbytes Number of bytes in the input array
 * @return Actual number of digits set in output array
 */
size_t mpConvFromOctets(u32 a[], size_t ndigits, const unsigned char *c, size_t nbytes);

/**
 * Converts multi-precision integer to array of octets
 *
 * @param a Input multi-precision integer array
 * @param ndigits Number of digits in input array
 * @param c Output array to store octets (bytes) in big-endian order
 * @param nbytes Size of output array (will pad or truncate as needed)
 * @return Number of non-zero octets required
 */
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


/**
 * Returns true (1) if x < 0, else false (0)
 *
 * @param x Multi-precision integer array to check
 * @param ndigits Number of digits in the array
 * @return 1 if x is negative, 0 otherwise
 * @remark Expects a negative number to be stored in two's-complement representation.
 */
int mpIsNegative(const u32 x[], size_t ndigits);

/**
 * Sets x = -y (negation)
 *
 * @param x Output array to store the negated result
 * @param y Input array to negate
 * @param ndigits Number of digits in the arrays
 * @return Status code indicating success or failure
 * @remark Expects a negative number to be stored in two's-complement representation.
 */
int mpChs(u32 x[], const u32 y[], size_t ndigits);

/**
 * Sets x = |y|, the absolute value of y
 *
 * @param x Output array to store the absolute value
 * @param y Input array
 * @param ndigits Number of digits in the arrays
 * @return Status code indicating success or failure
 * @remark Expects a negative number to be stored in two's-complement representation.
 * @returns 0 if y >= 0, 1 if y < 0
 */
int mpAbs(u32 x[], const u32 y[], size_t ndigits);

/****************/
/* VERSION INFO */
/****************/
/** Returns version number = major*1000+minor*100+release*10+PP_OPTIONS */

/**
 * Returns version number = major*1000+minor*100+release*10+PP_OPTIONS
 *
 * @return Version number encoded as integer
 * @remark Version number = major*1000+minor*100+release*10+uses_asm(0|1)+uses_64(0|2)+uses_noalloc(0|5)
 *         E.g. Version 2.3.0 will return 230x where x denotes the preprocessor options
 *         x | USE_SPASM | USE_64WITH32 | NO_ALLOCS
 *         ----------------------------------------
 *         0      No            No           No
 *         1      Yes           No           No
 *         2      No            Yes          No
 *         3      Yes           Yes*         No
 *         5      No            No           Yes
 *         6      Yes           No           Yes
 *         7      No            Yes          Yes
 *         8      Yes           Yes*         Yes
 *         ----------------------------------------
 *         * USE_SPASM will take precedence over USE_64WITH32.
 */
int mpVersion(void);

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
