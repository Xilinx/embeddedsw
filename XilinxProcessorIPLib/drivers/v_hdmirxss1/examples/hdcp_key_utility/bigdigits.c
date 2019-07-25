/* $Id: bigdigits.c $ */

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

/* Core code for BigDigits library "mp" functions */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "bigdigits.h"

/* For debugging - these are NOOPs */
#define DPRINTF0(s)
#define DPRINTF1(s, a1)

/***************************************/
/* VERSION NUMBERS - USED IN MPVERSION */
/***************************************/
static const int kMajor = 2, kMinor = 5, kRelease = 0;

/* Flags for preprocessor definitions used (=last digit of mpVersion) */
#ifdef USE_SPASM
static const int kUseSpasm = 1;
#else
static const int kUseSpasm = 0;
#endif

#ifdef USE_64WITH32
static const int kUse64with32 = 2;
#else
static const int kUse64with32 = 0;
#endif

#ifdef NO_ALLOCS
static const int kUseNoAllocs = 5;
#else
static const int kUseNoAllocs = 0;
#endif

/* Useful definitions */
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif
/* Internal macros */
#define BITS_PER_HALF_DIGIT (BITS_PER_DIGIT / 2)
#define BYTES_PER_DIGIT (BITS_PER_DIGIT / 8)
#define LOHALF(x) ((u32)((x) & MAX_HALF_DIGIT))
#define HIHALF(x) ((u32)((x) >> BITS_PER_HALF_DIGIT & MAX_HALF_DIGIT))
#define TOHIGH(x) ((u32)((x) << BITS_PER_HALF_DIGIT))
#define mpNEXTBITMASK(mask, n) do{if(mask==1){mask=HIBITMASK;n--;}else{mask>>=1;}}while(0)

/****************************/
/* ERROR HANDLING FUNCTIONS */
/****************************/
/* Change these to suit your tastes and operating system. */
#if defined(_WIN32) || defined(WIN32)
/* Win32 GUI alternative */
#ifndef STRICT
#define STRICT
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
void mpFail(char *msg)
{
	MessageBox(NULL, msg, "BigDigits Error", MB_ICONERROR);
	exit(EXIT_FAILURE);
}
#else	/* Ordinary console program */
void mpFail(char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}
#endif /* _WIN32 */

/*******************************/
/* MEMORY ALLOCATION FUNCTIONS */
/*******************************/
#ifndef NO_ALLOCS
u32 *mpAlloc(size_t ndigits)
{
	u32 *ptr;

	/* [v2.3] added check for zero digits. Thanks to "Radistao" */
	if (ndigits < 1) ndigits = 1;

	ptr = (u32 *)calloc(ndigits, sizeof(u32));
	if (!ptr)
		mpFail("mpAlloc: Unable to allocate memory.");

	return ptr;
}

void mpFree(u32 **p)
{
	if (*p)
	{
		free(*p);
		*p = NULL;
	}
}
#endif /* NO_ALLOCS */

/* Added in [v2.4] for ALLOC_BYTES and FREE_BYTES */
uint8_t zeroise_bytes(volatile void *v, size_t n)
{	/* Zeroise byte array b and make sure optimiser does not ignore this */
	volatile uint8_t optdummy;
	volatile uint8_t *b = (uint8_t*)v;
	while(n--)
		b[n] = 0;
	optdummy = *b;
	return optdummy;
}
/* [v2.4] Added explicit byte allocation functions with and without NO_ALLOCS */
#ifndef NO_ALLOCS
#define ALLOC_BYTES(b,n) do{(b)=calloc(n,1);if(!(b))mpFail("ALLOC_BYTES: Unable to allocate memory.");}while(0)
#define FREE_BYTES(b,n) do{zeroise_bytes((b),(n));free((b));}while(0)
#else
#define MAX_ALLOC_SIZE (MAX_FIXED_DIGITS*BYTES_PER_DIGIT)
#define ALLOC_BYTES(b,n) do{assert((n)<=MAX_ALLOC_SIZE);zeroise_bytes((b),(n));}while(0)
#define FREE_BYTES(b,n) zeroise_bytes((b),(n))
#endif


/* Force linker to include copyright notice in executable object image */
volatile const char *copyright_notice(void)
{
	return
"Contains multiple-precision arithmetic code originally written by David Ireland,"
" copyright (c) 2001-15 by D.I. Management Services Pty Limited <www.di-mgt.com.au>.";
}


/* To use, include this statement somewhere in the final code:

	copyright_notice();

It has no real effect at run time.
Thanks to Phil Zimmerman for this idea.
*/

/****************/
/* VERSION INFO */
/****************/
int mpVersion(void)
{
	return (kMajor * 1000 + kMinor * 100 + kRelease * 10 + kUseSpasm + kUse64with32 + kUseNoAllocs);
}

/**************************************/
/* CORE SINGLE PRECISION CALCULATIONS */
/* (double where necessary)           */
/**************************************/

/* [v2.2] Moved these functions into main file
	and added third option using 64-bit arithmetic if available.
OPTIONS:
1. define USE_64WITH32 to use 64-bit types on a 32-bit machine; or
2. define USE_SPASM to use Intel ASM (32-bit Intel compilers with __asm support); or
3. use default "long" calculations (any platform)
*/

#ifdef USE_64WITH32
/* 1. We are on a 32-bit machine with a 64-bit type available. */
#pragma message("USE_64WITH32 is set")

/* Make sure we have a uint64_t available */
#if defined (_WIN32) || defined(WIN32)
typedef unsigned __int64 uint64_t;
#elif !defined(HAVE_C99INCLUDES) && !defined(HAVE_SYS_TYPES)
typedef unsigned long long int uint64_t;
#endif

int spMultiply(uint32_t p[2], uint32_t x, uint32_t y)
{
	/* Use a 64-bit temp for product */
	uint64_t t = (uint64_t)x * (uint64_t)y;
	/* then split into two parts */
	p[1] = (uint32_t)(t >> 32);
	p[0] = (uint32_t)(t & 0xFFFFFFFF);

	return 0;
}

uint32_t spDivide(uint32_t *pq, uint32_t *pr, const uint32_t u[2], uint32_t v)
{
	uint64_t uu, q;
	uu = (uint64_t)u[1] << 32 | (uint64_t)u[0];
	q = uu / (uint64_t)v;
	//r = uu % (uint64_t)v;
	*pr = (uint32_t)(uu - q * v);
	*pq = (uint32_t)(q & 0xFFFFFFFF);
	return (uint32_t)(q >> 32);
}

#elif defined(USE_SPASM)
/* Use Intel MASM to compute sp products and divisions */
#pragma message("Using MASM")

int spMultiply(uint32_t p[2], uint32_t x, uint32_t y)
/* ASM version explicitly for 32-bit integers */
{
/* Computes p = (p1p0) = x * y. No restrictions on input. */
	__asm
	{
		mov eax, x
		xor edx, edx
		mul y
		; Product in edx:eax
		mov ebx, p
		mov dword ptr [ebx], eax
		mov dword ptr [ebx+4], edx
	}
	return 0;
}

uint32_t spDivide(uint32_t *pq, uint32_t *pr, const uint32_t u[2], uint32_t v)
/* ASM version explicitly for 32-bit integers */
{
/* Computes quotient q = u / v, remainder r = u mod v.
   Returns overflow (1) if q > word size (b) otherwise returns 0.
   CAUTION: Requires v >= [b/2] i.e. v to have its high bit set.
   (q1q0) = (u1u0)/v0
   (r0)   = (u1u0) mod v0
   Sets *pr = r0, *pq = q0 and returns "overflow" q1 (either 0 or 1).
*/
	uint32_t overflow = 0;
	__asm
	{
		; Dividend u in EDX:EAX, divisor in v
		mov ebx, u
		mov eax, dword ptr [ebx]
		mov edx, dword ptr [ebx+4]
		; Catch overflow (edx >= divisor)
		cmp edx, v
		jb no_overflow
		; If so, set edx = edx - divisor and flag it
		sub edx, v
		mov overflow, 1
no_overflow:
		div v
		; Quotient in EAX, Remainder in EDX
		mov ebx, pq
		mov dword ptr [ebx], eax
		mov ebx, pr
		mov dword ptr [ebx], edx
	}
	return overflow;
}

#else
/* Default routines the "long" way */

int spMultiply(u32 p[2], u32 x, u32 y)
{	/*	Computes p = x * y */
	/*	Ref: Arbitrary Precision Computation
	http://numbers.computation.free.fr/Constants/constants.html

         high    p1                p0     low
        +--------+--------+--------+--------+
        |      x1*y1      |      x0*y0      |
        +--------+--------+--------+--------+
               +-+--------+--------+
               |1| (x0*y1 + x1*y1) |
               +-+--------+--------+
                ^carry from adding (x0*y1+x1*y1) together
                        +-+
                        |1|< carry from adding LOHALF t
                        +-+  to high half of p0
	*/
	u32 x0, y0, x1, y1;
	u32 t, u, carry;

	/*	Split each x,y into two halves
		x = x0 + B*x1
		y = y0 + B*y1
		where B = 2^16, half the digit size
		Product is
		xy = x0y0 + B(x0y1 + x1y0) + B^2(x1y1)
	*/

	x0 = LOHALF(x);
	x1 = HIHALF(x);
	y0 = LOHALF(y);
	y1 = HIHALF(y);

	/* Calc low part - no carry */
	p[0] = x0 * y0;

	/* Calc middle part */
	t = x0 * y1;
	u = x1 * y0;
	t += u;
	if (t < u)
		carry = 1;
	else
		carry = 0;

	/*	This carry will go to high half of p[1]
		+ high half of t into low half of p[1] */
	carry = TOHIGH(carry) + HIHALF(t);

	/* Add low half of t to high half of p[0] */
	t = TOHIGH(t);
	p[0] += t;
	if (p[0] < t)
		carry++;

	p[1] = x1 * y1;
	p[1] += carry;


	return 0;
}

/* spDivide */

#define B (MAX_HALF_DIGIT + 1)

static void spMultSub(u32 uu[2], u32 qhat, u32 v1, u32 v0)
{
	/*	Compute uu = uu - q(v1v0)
		where uu = u3u2u1u0, u3 = 0
		and u_n, v_n are all half-digits
		even though v1, v2 are passed as full digits.
	*/
	u32 p0, p1, t;

	p0 = qhat * v0;
	p1 = qhat * v1;
	t = p0 + TOHIGH(LOHALF(p1));
	uu[0] -= t;
	if (uu[0] > MAX_DIGIT - t)
		uu[1]--;	/* Borrow */
	uu[1] -= HIHALF(p1);
}

u32 spDivide(u32 *q, u32 *r, const u32 u[2], u32 v)
{	/*	Computes quotient q = u / v, remainder r = u mod v
		where u is a double digit
		and q, v, r are single precision digits.
		Returns high digit of quotient (max value is 1)
		CAUTION: Assumes normalised such that v1 >= b/2
		where b is size of HALF_DIGIT
		i.e. the most significant bit of v should be one

		In terms of half-digits in Knuth notation:
		(q2q1q0) = (u4u3u2u1u0) / (v1v0)
		(r1r0) = (u4u3u2u1u0) mod (v1v0)
		for m = 2, n = 2 where u4 = 0
		q2 is either 0 or 1.
		We set q = (q1q0) and return q2 as "overflow"
	*/
	u32 qhat, rhat, t, v0, v1, u0, u1, u2, u3;
	u32 uu[2], q2;

	/* Check for normalisation */
	if (!(v & HIBITMASK))
	{	/* Stop if assert is working, else return error */
		assert(v & HIBITMASK);
		*q = *r = 0;
		return MAX_DIGIT;
	}

	/* Split up into half-digits */
	v0 = LOHALF(v);
	v1 = HIHALF(v);
	u0 = LOHALF(u[0]);
	u1 = HIHALF(u[0]);
	u2 = LOHALF(u[1]);
	u3 = HIHALF(u[1]);

	/* Do three rounds of Knuth Algorithm D Vol 2 p272 */

	/*	ROUND 1. Set j = 2 and calculate q2 */
	/*	Estimate qhat = (u4u3)/v1  = 0 or 1
		then set (u4u3u2) -= qhat(v1v0)
		where u4 = 0.
	*/
/* [Replaced in Version 2] -->
	qhat = u3 / v1;
	if (qhat > 0)
	{
		rhat = u3 - qhat * v1;
		t = TOHIGH(rhat) | u2;
		if (qhat * v0 > t)
			qhat--;
	}
<-- */
	qhat = (u3 < v1 ? 0 : 1);
	if (qhat > 0)
	{	/* qhat is one, so no need to mult */
		rhat = u3 - v1;
		/* t = r.b + u2 */
		t = TOHIGH(rhat) | u2;
		if (v0 > t)
			qhat--;
	}

	uu[1] = 0;		/* (u4) */
	uu[0] = u[1];	/* (u3u2) */
	if (qhat > 0)
	{
		/* (u4u3u2) -= qhat(v1v0) where u4 = 0 */
		spMultSub(uu, qhat, v1, v0);
		if (HIHALF(uu[1]) != 0)
		{	/* Add back */
			qhat--;
			uu[0] += v;
			uu[1] = 0;
		}
	}
	q2 = qhat;

	/*	ROUND 2. Set j = 1 and calculate q1 */
	/*	Estimate qhat = (u3u2) / v1
		then set (u3u2u1) -= qhat(v1v0)
	*/
	t = uu[0];
	qhat = t / v1;
	rhat = t - qhat * v1;
	/* Test on v0 */
	t = TOHIGH(rhat) | u1;
	if ((qhat == B) || (qhat * v0 > t))
	{
		qhat--;
		rhat += v1;
		t = TOHIGH(rhat) | u1;
		if ((rhat < B) && (qhat * v0 > t))
			qhat--;
	}

	/*	Multiply and subtract
		(u3u2u1)' = (u3u2u1) - qhat(v1v0)
	*/
	uu[1] = HIHALF(uu[0]);	/* (0u3) */
	uu[0] = TOHIGH(LOHALF(uu[0])) | u1;	/* (u2u1) */
	spMultSub(uu, qhat, v1, v0);
	if (HIHALF(uu[1]) != 0)
	{	/* Add back */
		qhat--;
		uu[0] += v;
		uu[1] = 0;
	}

	/* q1 = qhat */
	*q = TOHIGH(qhat);

	/* ROUND 3. Set j = 0 and calculate q0 */
	/*	Estimate qhat = (u2u1) / v1
		then set (u2u1u0) -= qhat(v1v0)
	*/
	t = uu[0];
	qhat = t / v1;
	rhat = t - qhat * v1;
	/* Test on v0 */
	t = TOHIGH(rhat) | u0;
	if ((qhat == B) || (qhat * v0 > t))
	{
		qhat--;
		rhat += v1;
		t = TOHIGH(rhat) | u0;
		if ((rhat < B) && (qhat * v0 > t))
			qhat--;
	}

	/*	Multiply and subtract
		(u2u1u0)" = (u2u1u0)' - qhat(v1v0)
	*/
	uu[1] = HIHALF(uu[0]);	/* (0u2) */
	uu[0] = TOHIGH(LOHALF(uu[0])) | u0;	/* (u1u0) */
	spMultSub(uu, qhat, v1, v0);
	if (HIHALF(uu[1]) != 0)
	{	/* Add back */
		qhat--;
		uu[0] += v;
		uu[1] = 0;
	}

	/* q0 = qhat */
	*q |= LOHALF(qhat);

	/* Remainder is in (u1u0) i.e. uu[0] */
	*r = uu[0];
	return q2;
}

#endif /* Conditional single-digit mult & div routines */

/************************/
/* ARITHMETIC FUNCTIONS */
/************************/
u32 mpAdd(u32 w[], const u32 u[], const u32 v[], size_t ndigits)
{
	/*	Calculates w = u + v
		where w, u, v are multiprecision integers of ndigits each
		Returns carry if overflow. Carry = 0 or 1.

		Ref: Knuth Vol 2 Ch 4.3.1 p 266 Algorithm A.
	*/

	u32 k;
	size_t j;

	assert(w != v);

	/* Step A1. Initialise */
	k = 0;

	for (j = 0; j < ndigits; j++)
	{
		/*	Step A2. Add digits w_j = (u_j + v_j + k)
			Set k = 1 if carry (overflow) occurs
		*/
		w[j] = u[j] + k;
		if (w[j] < k)
			k = 1;
		else
			k = 0;

		w[j] += v[j];
		if (w[j] < v[j])
			k++;

	}	/* Step A3. Loop on j */

	return k;	/* w_n = k */
}

u32 mpSubtract(u32 w[], const u32 u[], const u32 v[], size_t ndigits)
{
	/*	Calculates w = u - v where u >= v
		w, u, v are multiprecision integers of ndigits each
		Returns 0 if OK, or 1 if v > u.

		Ref: Knuth Vol 2 Ch 4.3.1 p 267 Algorithm S.
	*/

	u32 k;
	size_t j;

	assert(w != v);

	/* Step S1. Initialise */
	k = 0;

	for (j = 0; j < ndigits; j++)
	{
		/*	Step S2. Subtract digits w_j = (u_j - v_j - k)
			Set k = 1 if borrow occurs.
		*/
		w[j] = u[j] - k;
		if (w[j] > MAX_DIGIT - k)
			k = 1;
		else
			k = 0;

		w[j] -= v[j];
		if (w[j] > MAX_DIGIT - v[j])
			k++;

	}	/* Step S3. Loop on j */

	return k;	/* Should be zero if u >= v */
}

int mpMultiply(u32 w[], const u32 u[], const u32 v[], size_t ndigits)
{
	/*	Computes product w = u * v
		where u, v are multiprecision integers of ndigits each
		and w is a multiprecision integer of 2*ndigits

		Ref: Knuth Vol 2 Ch 4.3.1 p 268 Algorithm M.
	*/

	u32 k, t[2];
	size_t i, j, m, n;

	assert(w != u && w != v);

	m = n = ndigits;

	/* Step M1. Initialise */
	for (i = 0; i < 2 * m; i++)
		w[i] = 0;

	for (j = 0; j < n; j++)
	{
		/* Step M2. Zero multiplier? */
		if (v[j] == 0)
		{
			w[j + m] = 0;
		}
		else
		{
			/* Step M3. Initialise i */
			k = 0;
			for (i = 0; i < m; i++)
			{
				/* Step M4. Multiply and add */
				/* t = u_i * v_j + w_(i+j) + k */
				spMultiply(t, u[i], v[j]);

				t[0] += k;
				if (t[0] < k)
					t[1]++;
				t[0] += w[i+j];
				if (t[0] < w[i+j])
					t[1]++;

				w[i+j] = t[0];
				k = t[1];
			}
			/* Step M5. Loop on i, set w_(j+m) = k */
			w[j+m] = k;
		}
	}	/* Step M6. Loop on j */

	return 0;
}

/* mpDivide */

static u32 mpMultSub(u32 wn, u32 w[], const u32 v[],
					   u32 q, size_t n)
{	/*	Compute w = w - qv
		where w = (WnW[n-1]...W[0])
		return modified Wn.
	*/
	u32 k, t[2];
	size_t i;

	if (q == 0)	/* No change */
		return wn;

	k = 0;

	for (i = 0; i < n; i++)
	{
		spMultiply(t, q, v[i]);
		w[i] -= k;
		if (w[i] > MAX_DIGIT - k)
			k = 1;
		else
			k = 0;
		w[i] -= t[0];
		if (w[i] > MAX_DIGIT - t[0])
			k++;
		k += t[1];
	}

	/* Cope with Wn not stored in array w[0..n-1] */
	wn -= k;

	return wn;
}

static int QhatTooBig(u32 qhat, u32 rhat,
					  u32 vn2, u32 ujn2)
{	/*	Returns true if Qhat is too big
		i.e. if (Qhat * Vn-2) > (b.Rhat + Uj+n-2)
	*/
	u32 t[2];

	spMultiply(t, qhat, vn2);
	if (t[1] < rhat)
		return 0;
	else if (t[1] > rhat)
		return 1;
	else if (t[0] > ujn2)
		return 1;

	return 0;
}

int mpDivide(u32 q[], u32 r[], const u32 u[],
	size_t udigits, u32 v[], size_t vdigits)
{	/*	Computes quotient q = u / v and remainder r = u mod v
		where q, r, u are multiple precision digits
		all of udigits and the divisor v is vdigits.

		Ref: Knuth Vol 2 Ch 4.3.1 p 272 Algorithm D.

		Do without extra storage space, i.e. use r[] for
		normalised u[], unnormalise v[] at end, and cope with
		extra digit Uj+n added to u after normalisation.

		WARNING: this trashes q and r first, so cannot do
		u = u / v or v = u mod v.
		It also changes v temporarily so cannot make it const.
	*/
	size_t shift;
	int n, m, j;
	u32 bitmask, overflow;
	u32 qhat, rhat, t[2];
	u32 *uu, *ww;
	int qhatOK, cmp;

	/* Clear q and r */
	mpSetZero(q, udigits);
	mpSetZero(r, udigits);

	/* Work out exact sizes of u and v */
	n = (int)mpSizeof(v, vdigits);
	m = (int)mpSizeof(u, udigits);
	m -= n;

	/* Catch special cases */
	if (n == 0)
		return -1;	/* Error: divide by zero */

	if (n == 1)
	{	/* Use short division instead */
		r[0] = mpShortDiv(q, u, v[0], udigits);
		return 0;
	}

	if (m < 0)
	{	/* v > u, so just set q = 0 and r = u */
		mpSetEqual(r, u, udigits);
		return 0;
	}

	if (m == 0)
	{	/* u and v are the same length */
		cmp = mpCompare(u, v, (size_t)n);
		if (cmp < 0)
		{	/* v > u, as above */
			mpSetEqual(r, u, udigits);
			return 0;
		}
		else if (cmp == 0)
		{	/* v == u, so set q = 1 and r = 0 */
			mpSetDigit(q, 1, udigits);
			return 0;
		}
	}

	/*	In Knuth notation, we have:
		Given
		u = (Um+n-1 ... U1U0)
		v = (Vn-1 ... V1V0)
		Compute
		q = u/v = (QmQm-1 ... Q0)
		r = u mod v = (Rn-1 ... R1R0)
	*/

	/*	Step D1. Normalise */
	/*	Requires high bit of Vn-1
		to be set, so find most signif. bit then shift left,
		i.e. d = 2^shift, u' = u * d, v' = v * d.
	*/
	bitmask = HIBITMASK;
	for (shift = 0; shift < BITS_PER_DIGIT; shift++)
	{
		if (v[n-1] & bitmask)
			break;
		bitmask >>= 1;
	}

	/* Normalise v in situ - NB only shift non-zero digits */
	overflow = mpShiftLeft(v, v, shift, n);

	/* Copy normalised dividend u*d into r */
	overflow = mpShiftLeft(r, u, shift, n + m);
	uu = r;	/* Use ptr to keep notation constant */

	t[0] = overflow;	/* Extra digit Um+n */

	/* Step D2. Initialise j. Set j = m */
	for (j = m; j >= 0; j--)
	{
		/* Step D3. Set Qhat = [(b.Uj+n + Uj+n-1)/Vn-1]
		   and Rhat = remainder */
		qhatOK = 0;
		t[1] = t[0];	/* This is Uj+n */
		t[0] = uu[j+n-1];
		overflow = spDivide(&qhat, &rhat, t, v[n-1]);

		/* Test Qhat */
		if (overflow)
		{	/* Qhat == b so set Qhat = b - 1 */
			qhat = MAX_DIGIT;
			rhat = uu[j+n-1];
			rhat += v[n-1];
			if (rhat < v[n-1])	/* Rhat >= b, so no re-test */
				qhatOK = 1;
		}
		/* [VERSION 2: Added extra test "qhat && "] */
		if (qhat && !qhatOK && QhatTooBig(qhat, rhat, v[n-2], uu[j+n-2]))
		{	/* If Qhat.Vn-2 > b.Rhat + Uj+n-2
			   decrease Qhat by one, increase Rhat by Vn-1
			*/
			qhat--;
			rhat += v[n-1];
			/* Repeat this test if Rhat < b */
			if (!(rhat < v[n-1]))
				if (QhatTooBig(qhat, rhat, v[n-2], uu[j+n-2]))
					qhat--;
		}


		/* Step D4. Multiply and subtract */
		ww = &uu[j];
		overflow = mpMultSub(t[1], ww, v, qhat, (size_t)n);

		/* Step D5. Test remainder. Set Qj = Qhat */
		q[j] = qhat;
		if (overflow)
		{	/* Step D6. Add back if D4 was negative */
			q[j]--;
			overflow = mpAdd(ww, ww, v, (size_t)n);
		}

		t[0] = uu[j+n-1];	/* Uj+n on next round */

	}	/* Step D7. Loop on j */

	/* Clear high digits in uu */
	for (j = n; j < m+n; j++)
		uu[j] = 0;

	/* Step D8. Unnormalise. */

	mpShiftRight(r, r, shift, n);
	mpShiftRight(v, v, shift, n);

	return 0;
}


int mpSquare(u32 w[], const u32 x[], size_t ndigits)
/* New in Version 2.0 */
{
	/*	Computes square w = x * x
		where x is a multiprecision integer of ndigits
		and w is a multiprecision integer of 2*ndigits

		Ref: Menezes p596 Algorithm 14.16 with errata.
	*/

	u32 k, p[2], u[2], cbit, carry;
	size_t i, j, t, i2, cpos;

	assert(w != x);

	t = ndigits;

	/* 1. For i from 0 to (2t-1) do: w_i = 0 */
	i2 = t << 1;
	for (i = 0; i < i2; i++)
		w[i] = 0;

	carry = 0;
	cpos = i2-1;
	/* 2. For i from 0 to (t-1) do: */
	for (i = 0; i < t; i++)
	{
		/* 2.1 (uv) = w_2i + x_i * x_i, w_2i = v, c = u
		   Careful, w_2i may be double-prec
		*/
		i2 = i << 1; /* 2*i */
		spMultiply(p, x[i], x[i]);
		p[0] += w[i2];
		if (p[0] < w[i2])
			p[1]++;
		k = 0;	/* p[1] < b, so no overflow here */
		if (i2 == cpos && carry)
		{
			p[1] += carry;
			if (p[1] < carry)
				k++;
			carry = 0;
		}
		w[i2] = p[0];
		u[0] = p[1];
		u[1] = k;

		/* 2.2 for j from (i+1) to (t-1) do:
		   (uv) = w_{i+j} + 2x_j * x_i + c,
		   w_{i+j} = v, c = u,
		   u is double-prec
		   w_{i+j} is dbl if [i+j] == cpos
		*/
		k = 0;
		for (j = i+1; j < t; j++)
		{
			/* p = x_j * x_i */
			spMultiply(p, x[j], x[i]);
			/* p = 2p <=> p <<= 1 */
			cbit = (p[0] & HIBITMASK) != 0;
			k =  (p[1] & HIBITMASK) != 0;
			p[0] <<= 1;
			p[1] <<= 1;
			p[1] |= cbit;
			/* p = p + c */
			p[0] += u[0];
			if (p[0] < u[0])
			{
				p[1]++;
				if (p[1] == 0)
					k++;
			}
			p[1] += u[1];
			if (p[1] < u[1])
				k++;
			/* p = p + w_{i+j} */
			p[0] += w[i+j];
			if (p[0] < w[i+j])
			{
				p[1]++;
				if (p[1] == 0)
					k++;
			}
			if ((i+j) == cpos && carry)
			{	/* catch overflow from last round */
				p[1] += carry;
				if (p[1] < carry)
					k++;
				carry = 0;
			}
			/* w_{i+j} = v, c = u */
			w[i+j] = p[0];
			u[0] = p[1];
			u[1] = k;
		}
		/* 2.3 w_{i+t} = u */
		w[i+t] = u[0];
		/* remember overflow in w_{i+t} */
		carry = u[1];
		cpos = i+t;
	}

	/* (NB original step 3 deleted in Menezes errata) */

	/* Return w */

	return 0;
}

/** Returns true if a == b, else false. Not constant-time. */
int mpEqual_q(const u32 a[], const u32 b[], size_t ndigits)
{

	/* if (ndigits == 0) return -1; // deleted [v2.5] */

	while (ndigits--)
	{
		if (a[ndigits] != b[ndigits])
			return 0;	/* False */
	}

	return (!0);	/* True */
}

/** Returns sign of (a - b) as 0, +1 or -1. Not constant-time. */
int mpCompare_q(const u32 a[], const u32 b[], size_t ndigits)
{
	/* if (ndigits == 0) return 0; // deleted [v2.5] */

	while (ndigits--)
	{
		if (a[ndigits] > b[ndigits])
			return 1;	/* GT */
		if (a[ndigits] < b[ndigits])
			return -1;	/* LT */
	}

	return 0;	/* EQ */
}

/** Returns true if a == 0, else false. Not constant-time. */
int mpIsZero_q(const u32 a[], size_t ndigits)
{
	size_t i;

	/* if (ndigits == 0) return -1; // deleted [v2.5] */

	for (i = 0; i < ndigits; i++)	/* Start at lsb */
	{
		if (a[i] != 0)
			return 0;	/* False */
	}

	return (!0);	/* True */
}

/* CONSTANT-TIME COMPARISONS */
/* New and renamed in [v2.5] */

/* Constant-time comparisons of unsigned u32's */
#define IS_NONZERO_DIGIT(x) (((x)|(~(x)+1)) >> (BITS_PER_DIGIT-1))
#define IS_ZER0_DIGIT(x) (1 ^ IS_NONZERO_DIGIT((x)))

/** Returns 1 if a == b, else 0 (constant-time) */
int mpEqual(const u32 a[], const u32 b[], size_t ndigits)
{
	u32 dif = 0;

	while (ndigits--) {
		dif |= a[ndigits] ^ b[ndigits];
	}

	return (IS_ZER0_DIGIT(dif));
}

/** Returns 1 if a == 0, else 0 (constant-time) */
int mpIsZero(const u32 a[], size_t ndigits)
{
	u32 dif = 0;
	const u32 ZERO = 0;

	while (ndigits--) {
		dif |= a[ndigits] ^ ZERO;
	}

	return (IS_ZER0_DIGIT(dif));
}

/** Returns sign of (a - b) as 0, +1 or -1 (constant-time) */
int mpCompare(const u32 a[], const u32 b[], size_t ndigits)
{
	/* All these vars are either 0 or 1 */
	unsigned int gt = 0;
	unsigned int lt = 0;
	unsigned int mask = 1;	/* Set to zero once first inequality found */
	unsigned int c;

	while (ndigits--) {
		gt |= (a[ndigits] > b[ndigits]) & mask;
		lt |= (a[ndigits] < b[ndigits]) & mask;
		c = (gt | lt);
		mask &= (c-1);	/* Unchanged if c==0 or mask==0, else mask=0 */
	}

	return (int)gt - (int)lt;	/* EQ=0 GT=+1 LT=-1 */
}

/** Returns number of significant digits in a */
size_t mpSizeof(const u32 a[], size_t ndigits)
{
	while(ndigits--)
	{
		if (a[ndigits] != 0)
			return (++ndigits);
	}
	return 0;
}

size_t mpBitLength(const u32 d[], size_t ndigits)
/* Returns no of significant bits in d */
{
	size_t n, i, bits;
	u32 mask;

	if (!d || ndigits == 0)
		return 0;

	n = mpSizeof(d, ndigits);
	if (0 == n) return 0;

	for (i = 0, mask = HIBITMASK; mask > 0; mask >>= 1, i++)
	{
		if (d[n-1] & mask)
			break;
	}

	bits = n * BITS_PER_DIGIT - i;

	return bits;
}

void mpSetEqual(u32 a[], const u32 b[], size_t ndigits)
{	/* Sets a = b */
	size_t i;

	for (i = 0; i < ndigits; i++)
	{
		a[i] = b[i];
	}
}

u32 mpSetZero(u32 a[], size_t ndigits)
{	/* Sets a = 0 */

	/* Prevent optimiser ignoring this */
	u32 optdummy;
	u32 *p = a;

	while (ndigits--)
		a[ndigits] = 0;

	optdummy = *p;
	return optdummy;
}

void mpSetDigit(u32 a[], u32 d, size_t ndigits)
{	/* Sets a = d where d is a single digit */
	size_t i;

	for (i = 1; i < ndigits; i++)
	{
		a[i] = 0;
	}
	a[0] = d;
}

/**********************/
/* BIT-WISE FUNCTIONS */
/**********************/
u32 mpShiftLeft(u32 a[], const u32 *b,
	size_t shift, size_t ndigits)
{	/* Computes a = b << shift */
	/* [v2.1] Modified to cope with shift > BITS_PERDIGIT */
	size_t i, y, nw, bits;
	u32 mask, carry, nextcarry;

	/* Do we shift whole digits? */
	if (shift >= BITS_PER_DIGIT)
	{
		nw = shift / BITS_PER_DIGIT;
		i = ndigits;
		while (i--)
		{
			if (i >= nw)
				a[i] = b[i-nw];
			else
				a[i] = 0;
		}
		/* Call again to shift bits inside digits */
		bits = shift % BITS_PER_DIGIT;
		carry = b[ndigits-nw] << bits;
		if (bits)
			carry |= mpShiftLeft(a, a, bits, ndigits);
		return carry;
	}
	else
	{
		bits = shift;
	}

	/* Construct mask = high bits set */
	mask = ~(~(u32)0 >> bits);

	y = BITS_PER_DIGIT - bits;
	carry = 0;
	for (i = 0; i < ndigits; i++)
	{
		nextcarry = (b[i] & mask) >> y;
		a[i] = b[i] << bits | carry;
		carry = nextcarry;
	}

	return carry;
}

u32 mpShiftRight(u32 a[], const u32 b[], size_t shift, size_t ndigits)
{	/* Computes a = b >> shift */
	/* [v2.1] Modified to cope with shift > BITS_PERDIGIT */
	size_t i, y, nw, bits;
	u32 mask, carry, nextcarry;

	/* Do we shift whole digits? */
	if (shift >= BITS_PER_DIGIT)
	{
		nw = shift / BITS_PER_DIGIT;
		for (i = 0; i < ndigits; i++)
		{
			if ((i+nw) < ndigits)
				a[i] = b[i+nw];
			else
				a[i] = 0;
		}
		/* Call again to shift bits inside digits */
		bits = shift % BITS_PER_DIGIT;
		carry = b[nw-1] >> bits;
		if (bits)
			carry |= mpShiftRight(a, a, bits, ndigits);
		return carry;
	}
	else
	{
		bits = shift;
	}

	/* Construct mask to set low bits */
	/* (thanks to Jesse Chisholm for suggesting this improved technique) */
	mask = ~(~(u32)0 << bits);

	y = BITS_PER_DIGIT - bits;
	carry = 0;
	i = ndigits;
	while (i--)
	{
		nextcarry = (b[i] & mask) << y;
		a[i] = b[i] >> bits | carry;
		carry = nextcarry;
	}

	return carry;
}

int mpSetBit(u32 a[], size_t ndigits, size_t ibit, int value)
	/* Set bit n (0..nbits-1) with value 1 or 0 */
{
	size_t idigit, bit_to_set;
	u32 mask;

	/* Which digit? (0-based) */
	idigit = ibit / BITS_PER_DIGIT;
	if (idigit >= ndigits)
		return -1;

	/* Set mask */
	bit_to_set = ibit % BITS_PER_DIGIT;
	mask = 0x01 << bit_to_set;

	if (value)
		a[idigit] |= mask;
	else
		a[idigit] &= (~mask);

	return 0;
}

int mpGetBit(u32 a[], size_t ndigits, size_t ibit)
	/* Returns value 1 or 0 of bit n (0..nbits-1); or -1 if out of range */
{
	size_t idigit, bit_to_get;
	u32 mask;

	/* Which digit? (0-based) */
	idigit = ibit / BITS_PER_DIGIT;
	if (idigit >= ndigits)
		return -1;

	/* Set mask */
	bit_to_get = ibit % BITS_PER_DIGIT;
	mask = 0x01 << bit_to_get;

	return ((a[idigit] & mask) ? 1 : 0);
}

void mpModPowerOf2(u32 a[], size_t ndigits, size_t L)
	/* Computes a = a mod 2^L */
	/* i.e. clears all bits >= L */
{
	size_t i, nw, bits;
	u32 mask;

	/* High digits to clear */
	nw = L / BITS_PER_DIGIT;
	for (i = nw+1; i < ndigits; i++)
		a[i] = 0;
	/* Low bits to keep */
	bits = L % BITS_PER_DIGIT;
	mask = ~(~((u32)0) << bits);
	if (ndigits > nw)
		a[nw] &= mask;
}

void mpXorBits(u32 a[], const u32 b[], const u32 c[], size_t ndigits)
	/* Computes bitwise a = b XOR c */
{
	size_t i;
	for (i = 0; i < ndigits; i++)
		a[i] = b[i] ^ c[i];
}

void mpOrBits(u32 a[], const u32 b[], const u32 c[], size_t ndigits)
	/* Computes bitwise a = b OR c */
{
	size_t i;
	for (i = 0; i < ndigits; i++)
		a[i] = b[i] | c[i];
}

void mpAndBits(u32 a[], const u32 b[], const u32 c[], size_t ndigits)
	/* Computes bitwise a = b AND c */
{
	size_t i;
	for (i = 0; i < ndigits; i++)
		a[i] = b[i] & c[i];
}

void mpNotBits(u32 a[], const u32 b[], size_t ndigits)
	/* Computes bitwise a = NOT b */
{
	size_t i;
	for (i = 0; i < ndigits; i++)
		a[i] = ~b[i];
}

/*****************************************/
/* FUNCTIONS WITH A SINGLE (SHORT) DIGIT */
/*****************************************/
u32 mpShortAdd(u32 w[], const u32 u[], u32 v,
			   size_t ndigits)
{
	/*	Calculates w = u + v
		where w, u are multiprecision integers of ndigits each
		and v is a single precision digit.
		Returns carry if overflow.

		Ref: Derived from Knuth Algorithm A.
	*/

	u32 k;
	size_t j;

	k = 0;

	/* Add v to first digit of u */
	w[0] = u[0] + v;
	if (w[0] < v)
		k = 1;
	else
		k = 0;

	/* Add carry to subsequent digits */
	for (j = 1; j < ndigits; j++)
	{
		w[j] = u[j] + k;
		if (w[j] < k)
			k = 1;
		else
			k = 0;
	}

	return k;
}

u32 mpShortSub(u32 w[], const u32 u[], u32 v,
			   size_t ndigits)
{
	/*	Calculates w = u - v
		where w, u are multiprecision integers of ndigits each
		and v is a single precision digit.
		Returns borrow: 0 if u >= v, or 1 if v > u.

		Ref: Derived from Knuth Algorithm S.
	*/

	u32 k;
	size_t j;

	k = 0;

	/* Subtract v from first digit of u */
	w[0] = u[0] - v;
	if (w[0] > MAX_DIGIT - v)
		k = 1;
	else
		k = 0;

	/* Subtract borrow from subsequent digits */
	for (j = 1; j < ndigits; j++)
	{
		w[j] = u[j] - k;
		if (w[j] > MAX_DIGIT - k)
			k = 1;
		else
			k = 0;
	}

	return k;
}

u32 mpShortMult(u32 w[], const u32 u[], u32 v,
					size_t ndigits)
{
	/*	Computes product w = u * v
		Returns overflow k
		where w, u are multiprecision integers of ndigits each
		and v, k are single precision digits

		Ref: Knuth Algorithm M.
	*/

	u32 k, t[2];
	size_t j;

	if (v == 0)
	{	/* [2005-08-29] Set w = 0 */
		for (j = 0; j < ndigits; j++)
			w[j] = 0;
		return 0;
	}

	k = 0;
	for (j = 0; j < ndigits; j++)
	{
		/* t = x_i * v */
		spMultiply(t, u[j], v);
		/* w_i = LOHALF(t) + carry */
		w[j] = t[0] + k;
		/* Overflow? */
		if (w[j] < k)
			t[1]++;
		/* Carry forward HIHALF(t) */
		k = t[1];
	}

	return k;
}

u32 mpShortDiv(u32 q[], const u32 u[], u32 v,
				   size_t ndigits)
{
	/*	Calculates quotient q = u div v
		Returns remainder r = u mod v
		where q, u are multiprecision integers of ndigits each
		and r, v are single precision digits.

		Makes no assumptions about normalisation.

		Ref: Knuth Vol 2 Ch 4.3.1 Exercise 16 p625
	*/
	size_t j;
	u32 t[2], r;
	size_t shift;
	u32 bitmask, overflow, *uu;

	if (ndigits == 0) return 0;
	if (v == 0)	return 0;	/* Divide by zero error */

	/*	Normalise first */
	/*	Requires high bit of V
		to be set, so find most signif. bit then shift left,
		i.e. d = 2^shift, u' = u * d, v' = v * d.
	*/
	bitmask = HIBITMASK;
	for (shift = 0; shift < BITS_PER_DIGIT; shift++)
	{
		if (v & bitmask)
			break;
		bitmask >>= 1;
	}

	v <<= shift;
	overflow = mpShiftLeft(q, u, shift, ndigits);
	uu = q;

	/* Step S1 - modified for extra digit. */
	r = overflow;	/* New digit Un */
	j = ndigits;
	while (j--)
	{
		/* Step S2. */
		t[1] = r;
		t[0] = uu[j];
		overflow = spDivide(&q[j], &r, t, v);
	}

	/* Unnormalise */
	r >>= shift;

	return r;
}

u32 mpShortMod(const u32 a[], u32 d, size_t ndigits)
{
	/*	Calculates r = a mod d
		where a is a multiprecision integer of ndigits
		and r, d are single precision digits.
		Use remainder from divide function.
	*/

	u32 r = 0;
/* Allocate temp storage */
#ifdef NO_ALLOCS
	u32 q[MAX_FIXED_DIGITS * 2];
	assert(ndigits <= MAX_FIXED_DIGITS);
#else
	u32 *q;
	q = mpAlloc(ndigits * 2);
#endif

	r = mpShortDiv(q, a, d, ndigits);

	mpDESTROY(q, ndigits);

	return r;
}

/** Returns sign of (a - d) where d is a single digit */
int mpShortCmp(const u32 a[], u32 d, size_t ndigits)
{
	size_t i;
	int gt = 0;
	int lt = 0;

	/* Zero-length a => a is zero */
	if (ndigits == 0) return (d ? -1 : 0);

	/* If |a| > 1 then a > d */
	for (i = 1; i < ndigits; i++) {
		if (a[i] != 0)
			return 1;	/* GT */
	}

	lt = (a[0] < d);
	gt = (a[0] > d);

	return gt - lt;	/* EQ=0 GT=+1 LT=-1 */
}

/*********************************/
/* FUNCTIONS FOR SIGNED INTEGERS */
/*********************************/
/* Added [v2.2] */

/** Returns TRUE (1) if x < 0, else FALSE (0) */
int mpIsNegative(const u32 x[], size_t ndigits)
{
	return ((x[ndigits-1] & HIBITMASK) != 0);
}

/** Sets x = -y */
int mpChs(u32 x[], const u32 y[], size_t ndigits)
{
	int isneg = mpIsNegative(y, ndigits);

	if (isneg)
	{	/* negative to positive: x = ~(y-1) */
		mpShortSub(x, y, 1, ndigits);
		mpNotBits(x, x, ndigits);
	}
	else
	{	/* positive to negative: x = ~(y)+1 */
		/* [v2.3] FIXED: thanks to Valery Blazhnov of www.lissi.ru */
		mpNotBits(x, y, ndigits);
		mpShortAdd(x, x, 1, ndigits);
	}

	return isneg;
}

/** Sets x = |y| */
int mpAbs(u32 x[], const u32 y[], size_t ndigits)
{
	int isneg = mpIsNegative(y, ndigits);

	if (isneg)
		mpChs(x, y, ndigits);
	else
		mpSetEqual(x, y, ndigits);

	return isneg;
}

/*******************/
/* PRINT FUNCTIONS */
/*******************/
/* [v2.1] changed to use C99 format types */
void mpPrint(const u32 *p, size_t len)
{
	while (len--)
	{
		xil_printf("%08" PRIxBIGD " ", p[len]);
	}
}

void mpPrintNL(const u32 *p, size_t len)
{
	size_t i = 0;

	while (len--)
	{
		if ((i % 8) == 0 && i)
			xil_printf("\n");
		xil_printf("%08" PRIxBIGD " ", p[len]);
		i++;
	}
	xil_printf("\n");
}

void mpPrintTrim(const u32 *p, size_t len)
{
	/* Trim leading digits which are zero */
	while (len--)
	{
		if (p[len] != 0)
			break;
	}
	len++;
	/* Catch empty len to show 0 */
	if (0 == len) len = 1;

	mpPrint(p, len);
}

void mpPrintTrimNL(const u32 *p, size_t len)
{
	/* Trim leading zeroes */
	while (len--)
	{
		if (p[len] != 0)
			break;
	}
	len++;
	/* Catch empty len to show 0 */
	if (0 == len) len = 1;

	mpPrintNL(p, len);
}

void mpPrintHex(const char *prefix, const u32 *p, size_t len, const char *suffix)
{
	if (prefix) xil_printf("%s", prefix);
	/* Trim leading digits which are zero */
	while (len--)
	{
		if (p[len] != 0)
			break;
	}
	len++;
	if (0 == len) len = 1;
	/* print first digit without leading zeros */
	xil_printf("%" PRIxBIGD, p[--len]);
	while (len--)
	{
		xil_printf("%08" PRIxBIGD, p[len]);
	}
	if (suffix) xil_printf("%s", suffix);
}

/************************/
/* CONVERSION FUNCTIONS */
/************************/
size_t mpConvFromOctets(u32 a[], size_t ndigits, const unsigned char *c, size_t nbytes)
/* Converts nbytes octets into big digit a of max size ndigits
   Returns actual number of digits set (may be larger than mpSizeof)
*/
{
	size_t i;
	int j, k;
	u32 t;

	mpSetZero(a, ndigits);

	/* Read in octets, least significant first */
	/* i counts into big_d, j along c, and k is # bits to shift */
	for (i = 0, j = (int)nbytes - 1; i < ndigits && j >= 0; i++)
	{
		t = 0;
		for (k = 0; j >= 0 && k < BITS_PER_DIGIT; j--, k += 8)
			t |= ((u32)c[j]) << k;
		a[i] = t;
	}

	return i;
}

size_t mpConvToOctets(const u32 a[], size_t ndigits, unsigned char *c, size_t nbytes)
/* Convert big digit a into string of octets, in big-endian order,
   padding on the left to nbytes or truncating if necessary.
   Return number of octets required excluding leading zero bytes.
*/
{
	int j, k, len;
	u32 t;
	size_t i, noctets, nbits;

	nbits = mpBitLength(a, ndigits);
	noctets = (nbits + 7) / 8;

	len = (int)nbytes;

	for (i = 0, j = len - 1; i < ndigits && j >= 0; i++)
	{
		t = a[i];
		for (k = 0; j >= 0 && k < BITS_PER_DIGIT; j--, k += 8)
			c[j] = (unsigned char)(t >> k);
	}

	for ( ; j >= 0; j--)
		c[j] = 0;

	return (size_t)noctets;
}

/***************************/
/* NUMBER THEORY FUNCTIONS */
/***************************/
int mpModulo(u32 r[], const u32 u[], size_t udigits,
			 u32 v[], size_t vdigits)
{
	/*	Computes r = u mod v
		where r, v are multiprecision integers of length vdigits
		and u is a multiprecision integer of length udigits.
		r may overlap v.

		Note that r here is only vdigits long,
		whereas in mpDivide it is udigits long.

		Use remainder from mpDivide function.
	*/

	size_t nn = max(udigits, vdigits);
/* Allocate temp storage */
#ifdef NO_ALLOCS
	u32 qq[MAX_FIXED_DIGITS];
	u32 rr[MAX_FIXED_DIGITS];
	assert(nn <= MAX_FIXED_DIGITS);
#else
	u32 *qq, *rr;
	qq = mpAlloc(udigits);
	rr = mpAlloc(nn);
#endif


	/* rr[nn] = u mod v */
	mpDivide(qq, rr, u, udigits, v, vdigits);

	/* Final r is only vdigits long */
	mpSetEqual(r, rr, vdigits);

	mpDESTROY(rr, udigits);
	mpDESTROY(qq, udigits);

	return 0;
}

int mpModMult(u32 a[], const u32 x[], const u32 y[],
			  u32 m[], size_t ndigits)
{	/*	Computes a = (x * y) mod m */

/* Double-length temp variable p */
#ifdef NO_ALLOCS
	u32 p[MAX_FIXED_DIGITS * 2];
	assert(ndigits <= MAX_FIXED_DIGITS);
#else
	u32 *p;
	p = mpAlloc(ndigits * 2);
#endif

	/* Calc p[2n] = x * y */
	mpMultiply(p, x, y, ndigits);

	/* Then modulo (NOTE: a is OK at only ndigits long) */
	mpModulo(a, p, ndigits * 2, m, ndigits);

	mpDESTROY(p, ndigits * 2);

	return 0;
}

int mpModInv(u32 inv[], const u32 u[], const u32 v[], size_t ndigits)
{	/*	Computes inv = u^(-1) mod v */
	/*	Ref: Knuth Algorithm X Vol 2 p 342
		ignoring u2, v2, t2
		and avoiding negative numbers.
		Returns non-zero if inverse undefined.
	*/
	int bIterations;
	int result;
/* Allocate temp storage */
#ifdef NO_ALLOCS
	u32 u1[MAX_FIXED_DIGITS];
	u32 u3[MAX_FIXED_DIGITS];
	u32 v1[MAX_FIXED_DIGITS];
	u32 v3[MAX_FIXED_DIGITS];
	u32 t1[MAX_FIXED_DIGITS];
	u32 t3[MAX_FIXED_DIGITS];
	u32 q[MAX_FIXED_DIGITS];
	u32 w[2*MAX_FIXED_DIGITS];
	assert(ndigits <= MAX_FIXED_DIGITS);
#else
	u32 *u1, *u3, *v1, *v3, *t1, *t3, *q, *w;
	u1 = mpAlloc(ndigits);
	u3 = mpAlloc(ndigits);
	v1 = mpAlloc(ndigits);
	v3 = mpAlloc(ndigits);
	t1 = mpAlloc(ndigits);
	t3 = mpAlloc(ndigits);
	q  = mpAlloc(ndigits);
	w  = mpAlloc(2 * ndigits);
#endif

	/* Step X1. Initialise */
	mpSetDigit(u1, 1, ndigits);		/* u1 = 1 */
	mpSetEqual(u3, u, ndigits);		/* u3 = u */
	mpSetZero(v1, ndigits);			/* v1 = 0 */
	mpSetEqual(v3, v, ndigits);		/* v3 = v */

	bIterations = 1;	/* Remember odd/even iterations */
	while (!mpIsZero(v3, ndigits))		/* Step X2. Loop while v3 != 0 */
	{					/* Step X3. Divide and "Subtract" */
		mpDivide(q, t3, u3, ndigits, v3, ndigits);
						/* q = u3 / v3, t3 = u3 % v3 */
		mpMultiply(w, q, v1, ndigits);	/* w = q * v1 */
		mpAdd(t1, u1, w, ndigits);		/* t1 = u1 + w */

		/* Swap u1 = v1; v1 = t1; u3 = v3; v3 = t3 */
		mpSetEqual(u1, v1, ndigits);
		mpSetEqual(v1, t1, ndigits);
		mpSetEqual(u3, v3, ndigits);
		mpSetEqual(v3, t3, ndigits);

		bIterations = -bIterations;
	}

	if (bIterations < 0)
		mpSubtract(inv, v, u1, ndigits);	/* inv = v - u1 */
	else
		mpSetEqual(inv, u1, ndigits);	/* inv = u1 */

	/* Make sure u3 = gcd(u,v) == 1 */
	if (mpShortCmp(u3, 1, ndigits) != 0)
	{
		result = 1;
		mpSetZero(inv, ndigits);
	}
	else
		result = 0;

	/* Clear up */
	mpDESTROY(u1, ndigits);
	mpDESTROY(v1, ndigits);
	mpDESTROY(t1, ndigits);
	mpDESTROY(u3, ndigits);
	mpDESTROY(v3, ndigits);
	mpDESTROY(t3, ndigits);
	mpDESTROY(q, ndigits);
	mpDESTROY(w, 2*ndigits);

	return result;
}

int mpGcd(u32 d[], const u32 aa[], const u32 bb[], size_t ndigits)
{
	/* Computes d = gcd(a, b) */
	/* Changed to Binary GCD in [v2.3]
	 * Ref: Menezes Algorithm 14.54 plus some of Cohen Algorithm 1.3.5.
	 */

	unsigned int k;

/*	Allocate temp storage */
#ifdef NO_ALLOCS
	u32 a[MAX_FIXED_DIGITS];
	u32 b[MAX_FIXED_DIGITS];
	u32 r[MAX_FIXED_DIGITS];
	u32 t[MAX_FIXED_DIGITS];
	assert(ndigits <= MAX_FIXED_DIGITS);
#else
	u32 *a, *b, *r, *t;
	a = mpAlloc(ndigits);
	b = mpAlloc(ndigits);
	r = mpAlloc(ndigits);
	t = mpAlloc(ndigits);
#endif

	/* Copy input into temp vars */
	mpSetEqual(a, aa, ndigits);
	mpSetEqual(b, bb, ndigits);

	/* 1. [Reduce size once] */
	if (mpCompare(a, b, ndigits) < 0)
	{	/* Exchange a and b */
		mpSetEqual(t, a, ndigits);
		mpSetEqual(a, b, ndigits);
		mpSetEqual(b, t, ndigits);
	}
	/* If b = 0 output a and stop */
	if (mpIsZero(b, ndigits))
	{
		mpSetEqual(d, a, ndigits);
		goto done;
	}
	/* Set r <-- a mod b, a <-- b, b <-- r */
	mpModulo(r, a, ndigits, b, ndigits);
	mpSetEqual(a, b, ndigits);
	mpSetEqual(b, r, ndigits);
	/* If b = 0 output a and stop */
	if (mpIsZero(b, ndigits))
	{
		mpSetEqual(d, a, ndigits);
		goto done;
	}

	/* 2. [Compute power of 2] */
	k = 0;	/* g = 2^k <-- 1 */
	while (mpISEVEN(a, ndigits) && mpISEVEN(b, ndigits))
	{	/* While a and b are even */
		mpShiftRight(a, a, 1, ndigits);	/* a <-- a/2 */
		mpShiftRight(b, b, 1, ndigits);	/* b <-- b/2 */
		k++;	/* g <-- 2g */
	}
	while (!mpIsZero(a, ndigits))
	{
		/* 3. [Remove initial powers of 2] */
		while (mpISEVEN(a, ndigits))
			mpShiftRight(a, a, 1, ndigits);	/* a <-- a/2 until a is odd */
		while (mpISEVEN(b, ndigits))
			mpShiftRight(b, b, 1, ndigits);	/* b <-- b/2 until b is odd */
		/* 4. [Subtract] t = |a - b|/2 */
		if (mpCompare(b, a, ndigits) > 0)
			mpSubtract(t, b, a, ndigits);
		else
			mpSubtract(t, a, b, ndigits);
		mpShiftRight(t, t, 1, ndigits);
		/* if a >= b then set a <-- t otherwise set b <-- t */
		if (mpCompare(a, b, ndigits) >= 0)
			mpSetEqual(a, t, ndigits);
		else
			mpSetEqual(b, t, ndigits);

		/* 5. [Loop] */
	}
	/* Output (2^k.b) and stop */
	mpShiftLeft(d, b, k, ndigits);
done:

	mpDESTROY(a, ndigits);
	mpDESTROY(b, ndigits);
	mpDESTROY(r, ndigits);
	mpDESTROY(t, ndigits);

	return 0;	/* gcd is in d */
}

int mpSqrt(u32 s[], const u32 n[], size_t ndigits)
	/* Computes integer square root s = floor(sqrt(n)) i.e.
	the largest integer whose square is less than or equal to n */
	/* [Added v2.1, updated v2.3] Ref: H. Cohen Alg 1.7.1 */
{
/*	Allocate temp storage */
#ifdef NO_ALLOCS
	u32 x[MAX_FIXED_DIGITS];
	u32 y[MAX_FIXED_DIGITS];
	u32 q[MAX_FIXED_DIGITS];
	u32 r[MAX_FIXED_DIGITS];
	assert(ndigits <= MAX_FIXED_DIGITS);
#else
	u32 *x, *y, *q, *r;
	x = mpAlloc(ndigits);
	y = mpAlloc(ndigits);
	q = mpAlloc(ndigits);
	r = mpAlloc(ndigits);
#endif

	/* if (n <= 1) return n */
	if (mpShortCmp(n, 1, ndigits) <= 0)
	{
		mpSetEqual(s, n, ndigits);
		goto done;
	}

	/* 1. [Initialize] Set x = n */
	mpSetEqual(x, n, ndigits);

	while (1)
	{
		/* 2. [Newtonian step] Set y = [x + [n/x]]]/2 */
		mpDivide(q, r, n, ndigits, x, ndigits);
		mpAdd(y, x, q, ndigits);
		mpShiftRight(y, y, 1, ndigits);

		/* 3a. [Finished?] If y < x set x = y and go to step 2 */
		if (mpCompare(y, x, ndigits) >= 0)
			break;

		mpSetEqual(x, y, ndigits);
	}

	/* 3b. Otherwise output x and stop. */
	mpSetEqual(s, x, ndigits);

done:
	mpDESTROY(x, ndigits);
	mpDESTROY(y, ndigits);
	mpDESTROY(q, ndigits);
	mpDESTROY(r, ndigits);

	return 0;
}

int mpCubeRoot(u32 s[], const u32 n[], size_t ndigits)
	/* Computes integer cube root s = floor(cuberoot(n)) i.e.
	the largest integer whose cube is less than or equal to n */
	/* [Added v2.3] */
{
/*	Allocate temp storage */
#ifdef NO_ALLOCS
	u32 x[MAX_FIXED_DIGITS];
	u32 y[MAX_FIXED_DIGITS];
	u32 q[MAX_FIXED_DIGITS];
	u32 r[MAX_FIXED_DIGITS];
	assert(ndigits <= MAX_FIXED_DIGITS);
#else
	u32 *x, *y, *q, *r;
	x = mpAlloc(ndigits);
	y = mpAlloc(ndigits);
	q = mpAlloc(ndigits);
	r = mpAlloc(ndigits);
#endif

	/* if (n <= 1) return n */
	if (mpShortCmp(n, 1, ndigits) <= 0)
	{
		mpSetEqual(s, n, ndigits);
		goto done;
	}

	/* 1. [Initialize] Set x = n */
	mpSetEqual(x, n, ndigits);

	while (1)
	{
		/* 2. [Newtonian step] Set y = [2x + [n/x^2]]/3 */
		mpDivide(y, r, n, ndigits, x, ndigits);
		mpDivide(q, r, y, ndigits, x, ndigits);
		mpAdd(y, q, x, ndigits);
		mpAdd(q, y, x, ndigits);
		mpShortDiv(y, q, 3, ndigits);

		/* 3a. [Finished?] If y < x set x = y and go to step 2 */
		if (mpCompare(y, x, ndigits) >= 0)
			break;

		mpSetEqual(x, y, ndigits);
	}

	/* 3b. Otherwise output x and stop. */
	mpSetEqual(s, x, ndigits);

done:
	mpDESTROY(x, ndigits);
	mpDESTROY(y, ndigits);
	mpDESTROY(q, ndigits);
	mpDESTROY(r, ndigits);

	return 0;
}

int mpJacobi(const u32 a[], const u32 n[], size_t ndigits)
	/* Returns Jacobi(a, n) = {0, +1, -1} */
	/* [Added v2.2] */
{
/* Ref: Menezes.
Algorithm 2.149 Jacobi symbol (and Legendre symbol) computation
JACOBI(a,n)
INPUT: an odd integer n >= 3, and an integer a, 0 <= a < n.
OUTPUT: the Jacobi symbol (a/n) (and hence the Legendre symbol when n is prime).
1. If a = 0 then return(0).
2. If a = 1 then return(1).
3. Write a = 2^e.a_1, where a_1 is odd.
4. If e is even then set s <-- 1. Otherwise set s <-- 1 if n \equiv 1 or 7 (mod 8),
or set s <-- -1 if n \equiv 3 or 5 (mod 8).
5. If n \equiv 3 (mod 4) and a1 \equiv 3 (mod 4) then set s <-- -s.
6. Set n1 <-- n mod a1.
7. If a1 = 1 then return(s); otherwise return(s * JACOBI(n1,a1)).
*/

	int s;
	u32 nmod8;
	unsigned e;

/*	Allocate temp storage */
#ifdef NO_ALLOCS
	u32 a1[MAX_FIXED_DIGITS];
	u32 n1[MAX_FIXED_DIGITS];
	assert(ndigits <= MAX_FIXED_DIGITS);
#else
	u32 *a1, *n1;
	a1 = mpAlloc(ndigits);
	n1 = mpAlloc(ndigits);
#endif


	/* 1. If a = 0 then return(0). */
	if (mpIsZero(a, ndigits))
	{
		s = 0;
		goto done;
	}
	/* 2. If a = 1 then return(1). */
	if (mpShortCmp(a, 1, ndigits) == 0)
	{
		s = 1;
		goto done;
	}
	/* 3. Write a = 2^e.a_1, where a_1 is odd. */
	mpSetEqual(a1, a, ndigits);
	for (e = 0; mpISEVEN(a1, ndigits); e++)
	{
		 mpShiftRight(a1, a1, 1, ndigits);
	}
	/* 4.
	If e is even then set s <-- 1. Otherwise set s <-- 1 if n \equiv 1 or 7 (mod 8),
	or set s <-- -1 if n \equiv 3 or 5 (mod 8). */
	if (ISEVEN(e))
		s = 1;
	else
	{
		nmod8 = mpShortMod(n, 8, ndigits);
		if (nmod8 == 1 || nmod8 == 7)
			s = 1;
		else
			s = -1;
	}
	/* 5. If n \equiv 3 (mod 4) and a1 \equiv 3 (mod 4) then set s <-- -s. */
	if (mpShortMod(n, 4, ndigits) == 3 && mpShortMod(a1, 4, ndigits) == 3)
		s = -s;

	/*
	6. Set n1 <-- n mod a1.
	7. If a1 = 1 then return(s); otherwise return(s * JACOBI(n1,a1)).
	*/
	if (mpShortCmp(a1, 1, ndigits) != 0)
	{
		mpModulo(n1, n, ndigits, a1, ndigits);
		s = s * mpJacobi(n1, a1, ndigits);
	}

done:
	mpDESTROY(a1, ndigits);
	mpDESTROY(n1, ndigits);

	return s;
}


/* mpIsPrime: Changes in Version 2:
   Added mpAlloc for dynamic allocation
   Increased no of small primes
   Broke out mpRabinMiller() as a separate function
*/

static u32 SMALL_PRIMES[] = {
	3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43,
	47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101,
	103, 107, 109, 113,
	127, 131, 137, 139, 149, 151, 157, 163, 167, 173,
	179, 181, 191, 193, 197, 199, 211, 223, 227, 229,
	233, 239, 241, 251, 257, 263, 269, 271, 277, 281,
	283, 293, 307, 311, 313, 317, 331, 337, 347, 349,
	353, 359, 367, 373, 379, 383, 389, 397, 401, 409,
	419, 421, 431, 433, 439, 443, 449, 457, 461, 463,
	467, 479, 487, 491, 499, 503, 509, 521, 523, 541,
	547, 557, 563, 569, 571, 577, 587, 593, 599, 601,
	607, 613, 617, 619, 631, 641, 643, 647, 653, 659,
	661, 673, 677, 683, 691, 701, 709, 719, 727, 733,
	739, 743, 751, 757, 761, 769, 773, 787, 797, 809,
	811, 821, 823, 827, 829, 839, 853, 857, 859, 863,
	877, 881, 883, 887, 907, 911, 919, 929, 937, 941,
	947, 953, 967, 971, 977, 983, 991, 997,
};
#define N_SMALL_PRIMES (sizeof(SMALL_PRIMES)/sizeof(SMALL_PRIMES[0]))

int mpIsPrime(u32 w[], size_t ndigits, size_t t)
{	/*	Returns true if w is a probable prime */
	/*	Version 2: split out mpRabinMiller. */

	size_t i;

	/* Check the obvious */
	if (mpShortCmp(w, 2, ndigits) <= 0)
	{
		if (mpShortCmp(w, 2, ndigits) < 0)
			return 0;	/* 0 and 1 are not primes */
		return 1;		/* but 2 is a prime */
	}
	/* Otherwise any even number > 2 is not a prime */
	if (mpISEVEN(w, ndigits))
		return 0;

	/* First check for small primes, unless we could be one ourself */
	if (mpShortCmp(w, SMALL_PRIMES[N_SMALL_PRIMES-1], ndigits) > 0)
	{
		for (i = 0; i < N_SMALL_PRIMES; i++)
		{
			if (mpShortMod(w, SMALL_PRIMES[i], ndigits) == 0)
				return 0; /* Failed, so not a prime */
		}
	}
	else
	{	/* w is a small number, so check directly */
		for (i = 0; i < N_SMALL_PRIMES; i++)
		{
			if (mpShortCmp(w, SMALL_PRIMES[i], ndigits) == 0)
				return 1;	/* w is a small prime */
		}
		return 0;	/* w is not a small prime */
	}

	return mpRabinMiller(w, ndigits, t);
}

/* Local, simple rng functions used in Rabin-Miller */
static void rand_seed(void);
static u32 rand_between(u32 lower, u32 upper);

int mpRabinMiller(u32 w[], size_t ndigits, size_t t)
{
/*	Returns true (1) if w is a probable prime using the
	Rabin-Miller Probabilistic Primality Test.
	Carries out t iterations specified by user.
	Ref: FIPS-186-2 Appendix 2 Section 2.1.
	Also Schneier 2nd ed p 260 & Knuth Vol 2, p 379
	and ANSI 9.42-2003 Annex B.1.1.

	DSS Standard and ANSI 9.42 recommend using t >= 50
	for probability of error less than or equal to 2^-100.
	Ferguson & Schneier recommend t = 64 for prob error < 2^-128
	In practice, most random composites are caught in the first
	round or two and so specifying a large t will only affect
	the final check.

	[v2.1] Updated range of bases from [2, N-1] to [2, N-2]
	See ANSI 9.42-2003 Annex F.1.1 `Range of bases in Miller-Rabin test'
	(NB this does not impact existing implementations because N-1
	is unlikely to be chosen as a base).
*/

	/* Temp big digits */
	u32 maxrand;
	int failed, isprime;
	size_t i;
/*	Allocate temp storage */
#ifdef NO_ALLOCS
	u32 m[MAX_FIXED_DIGITS];
	u32 a[MAX_FIXED_DIGITS];
	u32 b[MAX_FIXED_DIGITS];
	u32 z[MAX_FIXED_DIGITS];
	u32 w1[MAX_FIXED_DIGITS];
	u32 j[MAX_FIXED_DIGITS];
	assert(ndigits <= MAX_FIXED_DIGITS);
#else
	u32 *m, *a, *b, *z, *w1, *j;
	m = mpAlloc(ndigits);
	a = mpAlloc(ndigits);
	b = mpAlloc(ndigits);
	z = mpAlloc(ndigits);
	w1 = mpAlloc(ndigits);
	j = mpAlloc(ndigits);
#endif

	/* Catch w <= 1 */
	if (mpShortCmp(w, 1, ndigits) <= 0)
	{
		isprime = 0;
		goto done;
	}

	/* Seed the simple RNG for later on */
	rand_seed();

	/*	Rabin-Miller from FIPS-186-2 Appendix 2.
	    Step 1. Set i = 1 [but do # tests requested by user].
		Step 2. Find a and m where w = 1 + (2^a)m
		m is odd and 2^a is largest power of 2 dividing w - 1
	*/
	mpShortSub(w1, w, 1, ndigits);	/* Store w1 = w - 1 */
	mpSetEqual(m, w1, ndigits);		/* Set m = w - 1 */
	/* for (a = 0; iseven(m); a++) */
	for (mpSetZero(a, ndigits); mpISEVEN(m, ndigits);
		mpShortAdd(a, a, 1, ndigits))
	{	/* Divide by 2 until m is odd */
		mpShiftRight(m, m, 1, ndigits);
	}

	/* assert((1 << a) * m + 1 == w); */

	/* Catch a small w */
	if (mpSizeof(w, ndigits) == 1)
		maxrand = w[0] - 2;	/* [v2.1] changed 1 to 2 */
	else
		maxrand = MAX_DIGIT;

	isprime = 1;
	for (i = 0; i < t; i++)
	{
		failed = 1;	/* Assume fail unless passed in loop */
		/* Step 3. Generate random integer b, 1 < b < w */
		/* [v2.1] changed to 1 < b < w-1 (see ANSI X9.42-2003 Annex B.1.1) */
		mpSetZero(b, ndigits);
		do
		{
			b[0] = rand_between(2, maxrand);
		} while (mpCompare(b, w, ndigits) >= 0);

		/* assert(1 < b && b < w); */

		/* Step 4. Set j = 0 and z = b^m mod w */
		mpSetZero(j, ndigits);
		mpModExp(z, b, m, w, ndigits);
		do
		{
			/* Step 5. If j = 0 and z = 1, or if z = w - 1 */
			/* i.e. if ((j == 0 && z == 1) || (z == w - 1)) */
			if ((mpIsZero(j, ndigits)
				&& mpShortCmp(z, 1, ndigits) == 0)
				|| (mpCompare(z, w1, ndigits) == 0))
			{	/* Passes on this loop  - go to Step 9 */
				failed = 0;
				break;
			}

			/* Step 6. If j > 0 and z = 1 */
			if (!mpIsZero(j, ndigits)
				&& (mpShortCmp(z, 1, ndigits) == 0))
			{	/* Fails - go to Step 8 */
				failed = 1;
				break;
			}

			/* Step 7. j = j + 1. If j < a set z = z^2 mod w */
			mpShortAdd(j, j, 1, ndigits);
			if (mpCompare(j, a, ndigits) < 0)
				mpModMult(z, z, z, w, ndigits);
			/* Loop: if j < a go to Step 5 */
		} while (mpCompare(j, a, ndigits) < 0);

		if (failed)
		{	/* Step 8. Not a prime - stop */
			isprime = 0;
			break;
		}
	}	/* Step 9. Go to Step 3 until i >= n */
	/* Else, if i = n, w is probably prime => success */

	/* Clean up */
done:
	mpDESTROY(m, ndigits);
	mpDESTROY(a, ndigits);
	mpDESTROY(b, ndigits);
	mpDESTROY(z, ndigits);
	mpDESTROY(w1, ndigits);
	mpDESTROY(j, ndigits);

	return isprime;
}

/***************************/
/* RANDOM NUMBER FUNCTIONS */
/***************************/

/* New in [v2.4] */
/** Generate a quick-and-dirty random mp number a <= 2^{nbits}-1 using plain-old-rand */
size_t mpQuickRandBits(u32 a[], size_t ndigits, size_t nbits)
{
	size_t ndig, nodd, i;
	u32 r;

	mpSetZero(a, ndigits);

	/* Catch too long nbits */
	if (nbits / BITS_PER_DIGIT > ndigits)
		nbits = ndigits * BITS_PER_DIGIT;

	ndig = nbits / BITS_PER_DIGIT;	/* # of complete digits */
	nodd = nbits % BITS_PER_DIGIT;	/* # of odd bits, perhaps zero */

	/* Fill each complete digit with random bits */
	for (i = 0; i < ndig; i++)
	{
		a[i] = spSimpleRand(0, MAX_DIGIT);
	}
	if (nodd)
	{
		r = spSimpleRand(0, MAX_DIGIT);
		r >>= BITS_PER_DIGIT - nodd;
		a[ndig] = r;
		i++;
	}

	return i;
}

/* Internal functions used for "simple" random numbers */

static void rand_seed()
/* [v2.2] Moved seeding process inside this function.
   Added clock() to time() to improve precision.
   [v2.4] Extra fudge with time shifted left by 16
*/
{
	/* Seed with system time and clock */
	//unsigned int seed = (((unsigned int)time(NULL) & 0xFFFF) << 16) ^ (unsigned int)clock();
	//srand(seed);
}

static u32 rand_between(u32 lower, u32 upper)
/* Returns a single pseudo-random digit between lower and upper.
   Uses rand(). Assumes srand() already called. */
{
	u32 d, range;
	unsigned char *bp;
	unsigned int i, nbits;
	u32 mask;

	if (upper <= lower) return lower;
	range = upper - lower;

	do
	{
		/* Generate a random DIGIT byte-by-byte using rand() */
		bp = (unsigned char *)&d;
		for (i = 0; i < sizeof(u32); i++)
		{
			bp[i] = (unsigned char)(rand() & 0xFF);
		}

		/* Trim to next highest bit above required range */
		mask = HIBITMASK;
		for (nbits = BITS_PER_DIGIT; nbits > 0; nbits--, mask >>= 1)
		{
			if (range & mask)
				break;
		}
		if (nbits < BITS_PER_DIGIT)
		{
			mask <<= 1;
			mask--;
		}
		else
			mask = MAX_DIGIT;

		d &= mask;

	} while (d > range);

	return (lower + d);
}

u32 spSimpleRand(u32 lower, u32 upper)
{	/*	Returns a pseudo-random digit.
		Handles own seeding using time.
		NOT for cryptographically-secure random numbers.
		NOT thread-safe because of static variable.
		Changed in Version 2 to use internal funcs.
	*/
	static unsigned seeded = 0;

	if (!seeded)
	{
		rand_seed();
		seeded++;
	}
	return rand_between(lower, upper);
}

/**************************/
/* MODULAR EXPONENTIATION */
/**************************/
/*	[v2.2] Modified to use sliding-window exponentiation.
	mpModExp_1 is the earlier version [<2.2] now using macros for modular squaring & mult
*/

#ifdef NO_ALLOCS
static int mpModExp_1(u32 y[], const u32 x[], const u32 n[], u32 d[], size_t ndigits);
#else
static int mpModExp_windowed(u32 y[], const u32 x[], const u32 n[], u32 d[], size_t ndigits);
#endif

int mpModExp(u32 y[], const u32 x[], const u32 n[], u32 d[], size_t ndigits)
	/* Computes y = x^n mod d */
{
#ifdef NO_ALLOCS
	return mpModExp_1(y, x, n, d, ndigits);
#else
	return mpModExp_windowed(y, x, n, d, ndigits);
#endif
}

/* MACROS TO DO MODULAR SQUARING AND MULTIPLICATION USING PRE-ALLOCATED TEMPS */
/* Required lengths |y|=|t1|=|t2|=2*n, |m|=n; but final |y|=n */
/* Square: y = (y * y) mod m */
#define mpMODSQUARETEMP(y,m,n,t1,t2) do{mpSquare(t1,y,n);mpDivide(t2,y,t1,n*2,m,n);}while(0)
/* Mult:   y = (y * x) mod m */
#define mpMODMULTTEMP(y,x,m,n,t1,t2) do{mpMultiply(t1,x,y,n);mpDivide(t2,y,t1,n*2,m,n);}while(0)
/* Mult:   w = (y * x) mod m */
#define mpMODMULTXYTEMP(w,y,x,m,n,t1,t2) do{mpMultiply(t1,x,y,(n));mpDivide(t2,w,t1,(n)*2,m,(n));}while(0)

static int mpModExp_1(u32 yout[], const u32 x[], const u32 e[], u32 m[], size_t ndigits)
{	/*	Computes y = x^e mod m */
	/*	"Classic" binary left-to-right method */
	/*  [v2.2] removed const restriction on m[] to avoid using an extra alloc'd var
		(m is changed in-situ during the divide operation then restored) */
	u32 mask;
	size_t n;
	size_t nn = ndigits * 2;
	/* Create some double-length temps */
#ifdef NO_ALLOCS
	u32 t1[MAX_FIXED_DIGITS * 2];
	u32 t2[MAX_FIXED_DIGITS * 2];
	u32 y[MAX_FIXED_DIGITS * 2];
	assert(ndigits <= MAX_FIXED_DIGITS);
#else
	u32 *t1, *t2, *y;
	t1 = mpAlloc(nn);
	t2 = mpAlloc(nn);
	y  = mpAlloc(nn);
#endif

	assert(ndigits != 0);

	n = mpSizeof(e, ndigits);
	/* Catch e==0 => x^0=1 */
	if (0 == n)
	{
		mpSetDigit(yout, 1, ndigits);
		goto done;
	}
	/* Find second-most significant bit in e */
	for (mask = HIBITMASK; mask > 0; mask >>= 1)
	{
		if (e[n-1] & mask)
			break;
	}
	mpNEXTBITMASK(mask, n);

	/* Set y = x */
	mpSetEqual(y, x, ndigits);

	/* For bit j = k-2 downto 0 */
	while (n)
	{
		/* Square y = y * y mod n */
		mpMODSQUARETEMP(y, m, ndigits, t1, t2);
		if (e[n-1] & mask)
		{	/*	if e(j) == 1 then multiply
				y = y * x mod n */
			mpMODMULTTEMP(y, x, m, ndigits, t1, t2);
		}

		/* Move to next bit */
		mpNEXTBITMASK(mask, n);
	}

	/* Return y */
	mpSetEqual(yout, y, ndigits);

done:
	mpDESTROY(t1, nn);
	mpDESTROY(t2, nn);
	mpDESTROY(y, ndigits);

	return 0;
}

/**	Computes y = x^e mod m in constant time using Coron's algorithm */
int mpModExp_ct(u32 yout[], const u32 x[], const u32 e[], u32 m[], size_t ndigits)
{
	/* Algorithm: Corons exponentiation (left-to-right)
	 * Square-and-multiply resistant against simple power attacks (SPA)
	 * Ref: Jean-Sebastian Coron, "Resistance Against Differential Power Analysis for
	 * Elliptic Curve Cryptosystems", August 1999.
	 * -- This version adapted from Coron's elliptic curve point scalar multiplication
	 *    to RSA-style modular exponentiation.
	 * Input: base x, modulus m, and
	 * exponent e = (e_k, e_{k-1},...,e_0) with e_k = 1
	 * Output: c = x^e mod m
	 * 1. c[0] = x
	 * 2. For i = k-2 downto 0 do:
	 * 3.    c[0] = c[0]^2 mod m
	 * 4.    c[1] = c[0] * x mod m
	 * 5.    c[0] = c[d_i]
	 * 6. Return c[0]
	 */
	u32 mask;
	size_t n;
	size_t nn = ndigits * 2;
	unsigned int ej;

	/* Create some double-length temps */
#ifdef NO_ALLOCS
	u32 t1[MAX_FIXED_DIGITS * 2];
	u32 t2[MAX_FIXED_DIGITS * 2];
	u32 c[2][MAX_FIXED_DIGITS * 2];
	assert(ndigits <= MAX_FIXED_DIGITS);
#else
	u32 *t1, *t2;
	u32 *c[2];
	t1 = mpAlloc(nn);
	t2 = mpAlloc(nn);
	c[0] = mpAlloc(nn);
	c[1] = mpAlloc(nn);
#endif

	assert(ndigits != 0);

	n = mpSizeof(e, ndigits);
	/* Catch e==0 => x^0=1 */
	if (0 == n)
	{
		mpSetDigit(yout, 1, ndigits);
		goto done;
	}
	/* Find second-most significant bit in e */
	for (mask = HIBITMASK; mask > 0; mask >>= 1)
	{
		if (e[n-1] & mask)
			break;
	}
	mpNEXTBITMASK(mask, n);

	/* Set c[0] = x */
	mpSetEqual(c[0], x, ndigits);

	/* For bit j = k-2 downto 0 */
	while (n)
	{
		/* Square c[0] = c[0]^2 mod n */
		mpMODSQUARETEMP(c[0], m, ndigits, t1, t2);
		/* Multiply c[1] = c[0] * x mod n */
		mpMODMULTXYTEMP(c[1], c[0], x, m, ndigits, t1, t2);
		/* c[0] = c[e(j)] */
		ej = (e[n-1] & mask) != 0;
		assert(ej <= 1);
		mpSetEqual(c[0], c[ej], ndigits);

		/* Move to next bit */
		mpNEXTBITMASK(mask, n);
	}

	/* Return c[0] */
	mpSetEqual(yout, c[0], ndigits);

done:
	mpDESTROY(t1, nn);
	mpDESTROY(t2, nn);
	mpDESTROY(c[0], ndigits);
	mpDESTROY(c[1], ndigits);

	return 0;
}



/* Use sliding window alternative only if NO_ALLOCS not defined */
#ifndef NO_ALLOCS

/*
SLIDING-WINDOW EXPONENTIATION
Ref: Menezes, chap 14, p616.
k is called the window size.

14.85 Algorithm Sliding-window exponentiation
INPUT: g, e = (e_t.e_{t-1}...e1.e0)_2 with e_t = 1, and an integer k >= 1.
OUTPUT: g^e.
1. Precomputation.
	1.1 g_1 <-- g, g_2 <-- g^2.
	1.2 For i from 1 to (2^{k-1} - 1) do: g_{2i+1} <-- g_{2i-1} * g_2.
2. A <-- 1, i <-- t.
3. While i >= 0 do the following:
	3.1 If e_i = 0 then do: A <-- A^2, i <-- i - 1.
	3.2 Otherwise (e_i != 0), find the longest bitstring e_i.e_{i-1}...e_l such that i-l+1 <= k
and e_l = 1, and do the following:
	A <-- A^{2i-l+1} * g_{(e_i.e_{i-1}...e_l)_2}
	i <-- l - 1.
4. Return(A).
*/

/*
Optimal values of k for various exponent sizes.
	The references on this differ in their recommendations.
	These values reflect experiments we've done on our systems.
	You can adjust this to suit your own situation.
*/
static size_t WindowLenTable[] =
{
/* k=1   2   3   4    5     6     7     8 */
     5, 16, 64, 240, 768, 1024, 2048, 4096
};
#define WINLENTBLMAX (sizeof(WindowLenTable)/sizeof(WindowLenTable[0]))

/*	The process used here to read bits into the lookahead buffer could be improved slightly as
	some bits are read in more than once. But we think this function is tricky enough without
	adding more complexity for marginal benefit.
*/

static int mpModExp_windowed(u32 yout[], const u32 g[],
			const u32 e[], u32 m[], size_t ndigits)
/* Computes y = g^e mod m using sliding-window exponentiation */
{
	size_t nbits;	/* Number of significant bits in e */
	size_t winlen;	/* Window size */
	u32 mask;	/* Bit mask used for main loop */
	size_t nwd;		/* Digit counter for main loop */
	u32 lkamask;	/* Bit mask for lookahead */
	size_t lkanwd;		/* Digit counter for lookahead */
	u32 lkabuf;	/* One-digit lookahead buffer */
	size_t lkalen;	/* Actual size of window for current lookahead buffer */
	int in_window;	/* Flag for in-window state */
	u32 *gtable[(1 << (WINLENTBLMAX-1))];	/* Table of ptrs to g1, g3, g5,... */
	size_t ngt;		/* No of elements in gtable */
	size_t idxmult;	/* Index (in gtable) of next multiplier to use: 0=g1, 1=g3, 2=g5,... */
	u32 *g2;	/* g2 = g^2 */
	u32 *temp1, *temp2;		/* Temp big digits, needed for MULT and SQUARE macros */
	u32 *a;		/* A */
	int aisone;		/* Flag that A == 1 */
	size_t nn;		/* 2 * ndigits */
	size_t i;		/* Temp counter */

	/* Get actual size of e */
	nbits = mpBitLength(e, ndigits);
	DPRINTF1("nbits=%d\n", nbits);

	/* Catch easy ones */
	if (nbits == 0)
	{	/* g^0 = 1 */
		mpSetDigit(yout, 1, ndigits);
		return 1;
	}
	if (nbits == 1)
	{	/* g^1 = g mod m */
		mpModulo(yout, g, ndigits, m, ndigits);
		return 1;
	}

	/* Lookup optimised window length for this size of e */
	for (winlen = 0; winlen < WINLENTBLMAX && winlen < BITS_PER_DIGIT; winlen++)
	{
		if (WindowLenTable[winlen] > nbits)
			break;
	}
	DPRINTF1("winlen=%d\n", winlen);

	/* Default to simple L-R method for 1-bit window */
	if (winlen <= 1)
		return mpModExp_1(yout, g, e, m, ndigits);

	/* Allocate temp vars - NOTE: all are 2n long */
	nn = 2 * ndigits;
	temp1 = mpAlloc(nn);
	temp2 = mpAlloc(nn);
	g2 = mpAlloc(nn);
	a = mpAlloc(nn);

	/* 1. PRECOMPUTATION */
	/* 1.1 g1 <-- g, (we already have g in the input, so just point to it) */
	gtable[0] = (u32 *)g;
	/* g2 <-- g^2 */
	mpModMult(g2, gtable[0], gtable[0], m, ndigits);

	/* 1.2 For i from 1 to (2^{k-1} - 1) do: g_{2i+1} <-- g_{2i-1} * g_2. */
	/* i.e. we store (g1, g3, g5, g7,...) */
	ngt = ((size_t)1 << (winlen - 1));
	for (i = 1; i < ngt; i++)
	{
		/* NOTE: we need these elements to be 2n digits long for the mpMODMULTTEMP fn,
			but the final result is only n digits long */
		gtable[i] = mpAlloc(nn);
		//mpModMult(gtable[i], gtable[i-1], g2, m, ndigits);
		mpSetEqual(gtable[i], gtable[i-1], ndigits);
		mpMODMULTTEMP(gtable[i], g2, m, ndigits, temp1, temp2);
	}

	/* 2. A <-- 1 (use flag) */
	//mpSetDigit(a, 1, ndigits);
	aisone = 1;

	/* Find most significant bit in e */
	nwd = mpSizeof(e, ndigits);
	for (mask = HIBITMASK; mask > 0; mask >>= 1)
	{
		if (e[nwd-1] & mask)
			break;
	}

	/* i <-- t; 3. While i >= 0 do the following: */
	/* i.e. look at high bit and every subsequent bit L->R in turn */
	lkalen = 0;
	in_window = 0;
	idxmult = 0;
	while (nwd)
	{
		/* We always square each time around */
		/* A <-- A^2 */
		if (!aisone)	/* 1^2 = 1! */
		{
			mpMODSQUARETEMP(a, m, ndigits, temp1, temp2);
		}

		if (!in_window)
		{	/* Do we start another window? */
			if ((e[nwd-1] & mask))
			{	/* Yes, bit is '1', so setup this window */
				in_window = 1;
				/* Read in look-ahead buffer (a single digit) */
				lkamask = mask;
				lkanwd = nwd;
				/* Read in this and the next winlen-1 bits into lkabuf */
				lkabuf = 0x1;
				for (i = 0; i < winlen-1; i++)
				{
					mpNEXTBITMASK(lkamask, lkanwd);
					lkabuf <<= 1;
					/* if lkanwd==0 we have passed the end, so just append a zero bit */
					if (lkanwd && (e[lkanwd-1] & lkamask))
					{
						lkabuf |= 0x1;
					}
				}
				DPRINTF1("(%x)", lkabuf);
				/* Compute this window's length */
				/* i.e. keep shifting right until we have a '1' bit at the end */
				for (lkalen = winlen - 1; lkalen > 0; lkalen--, lkabuf >>= 1)
				{
					if (ISODD(lkabuf))
						break;
				}
				/* Set next multipler to use */
				/* idx = (value(buf) - 1) / 2 */
				idxmult = lkabuf >> 1;

				DPRINTF0("1");

			}
			else
			{	/* No, bit is '0', so just loop */
				DPRINTF0("0 ");
			}
		}
		else
		{	/* We are in a window... */
			DPRINTF1("%s", ((e[nwd-1] & mask) ? "1" : "0"));
			/* Has it finished yet? */
			if (lkalen > 0)
			{
				lkalen--;
			}
		}
		/* Are we at end of this window? */
		if (in_window && lkalen < 1)
		{	/* Yes, so compute A <-- A * g_l */
			if (aisone)
			{
				mpSetEqual(a, gtable[idxmult], ndigits);
				aisone = 0;
			}
			else
			{
				mpMODMULTTEMP(a, gtable[idxmult], m, ndigits, temp1, temp2);
			}
			DPRINTF1("[%x]", idxmult);
			DPRINTF0("/ ");
			in_window = 0;
			lkalen = 0;
		}
		mpNEXTBITMASK(mask, nwd);
	}
	/* Finally, cope with anything left in the final window */
	if (in_window)
	{
		if (aisone)
		{
			mpSetEqual(a, gtable[idxmult], ndigits);
			aisone = 0;
		}
		else
		{
			mpMODMULTTEMP(a, gtable[idxmult], m, ndigits, temp1, temp2);
		}
		DPRINTF1("[%x]", idxmult);
		DPRINTF0("//");
	}
	DPRINTF0("\n");

	/* 4. Return (A) */
	mpSetEqual(yout, a, ndigits);

	/* Clean up */
	mpDESTROY(a, nn);
	mpDESTROY(g2, nn);
	mpDESTROY(temp1, nn);
	mpDESTROY(temp2, nn);
	/* CAUTION: don't clean gtable[0] */
	for (i = 1; i < ngt; i++)
		mpDESTROY(gtable[i], nn);

	return 0;
}

#endif /* !NO_ALLOCS */
