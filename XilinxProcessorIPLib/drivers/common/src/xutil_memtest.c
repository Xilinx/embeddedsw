/******************************************************************************
* Copyright (C) 2002 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xutil_memtest.c
* @addtogroup common_v1_2
* @{
*
* Contains the memory test utility functions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver    Who    Date    Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a ecm  11/01/01 First release
* 1.00a xd   11/03/04 Improved support for doxygen.
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/
#include "xbasic_types.h"
#include "xstatus.h"
#include "xutil.h"

/************************** Constant Definitions ****************************/
/************************** Function Prototypes *****************************/

static u32 RotateLeft(u32 Input, u8 Width);

/* define ROTATE_RIGHT to give access to this functionality */
/* #define ROTATE_RIGHT */
#ifdef ROTATE_RIGHT
static u32 RotateRight(u32 Input, u8 Width);
#endif /* ROTATE_RIGHT */


/*****************************************************************************/
/**
*
* Performs a destructive 32-bit wide memory test.
*
* @param    Addr is a pointer to the region of memory to be tested.
* @param    Words is the length of the block.
* @param    Pattern is the constant used for the constant pattern test, if 0,
*           0xDEADBEEF is used.
* @param    Subtest is the test selected. See xutil.h for possible values.
*
* @return
*
* - XST_MEMTEST_FAILED is returned for a failure
* - XST_SUCCESS is returned for a pass
*
* @note
*
* Used for spaces where the address range of the region is smaller than
* the data width. If the memory range is greater than 2 ** width,
* the patterns used in XUT_WALKONES and XUT_WALKZEROS will repeat on a
* boundary of a power of two making it more difficult to detect addressing
* errors. The XUT_INCREMENT and XUT_INVERSEADDR tests suffer the same
* problem. Ideally, if large blocks of memory are to be tested, break
* them up into smaller regions of memory to allow the test patterns used
* not to repeat over the region tested.
*
*****************************************************************************/
int XUtil_MemoryTest32(u32 *Addr, u32 Words, u32 Pattern, u8 Subtest)
{
	u32 i;
	u32 j;
	u32 Val = XUT_MEMTEST_INIT_VALUE;
	u32 FirstVal = XUT_MEMTEST_INIT_VALUE;
	u32 Word;

	XASSERT_NONVOID(Words != 0);
	XASSERT_NONVOID(Subtest <= XUT_MAXTEST);

	/*
	 * Select the proper Subtest
	 */


	switch (Subtest) {

	case XUT_ALLMEMTESTS:

		/* this case executes all of the Subtests */

		/* fall through case statement */

	case XUT_INCREMENT:
		{

			/*
			 * Fill the memory with incrementing
			 * values starting from 'FirstVal'
			 */
			for (i = 0L; i < Words; i++) {
				Addr[i] = Val;

				/* write memory location */

				Val++;
			}

			/*
			 * Restore the reference 'Val' to the
			 * initial value
			 */

			Val = FirstVal;

			/*
			 * Check every word within the Words
			 * of tested memory and compare it
			 * with the incrementing reference
			 * Val
			 */

			for (i = 0L; i < Words; i++) {
				Word = Addr[i];

				if (Word != Val) {
					return XST_MEMTEST_FAILED;
				}

				Val++;
			}


			if (Subtest != XUT_ALLMEMTESTS) {
				return XST_SUCCESS;
			}


		}		/* end of case 1 */

		/* fall through case statement */

	case XUT_WALKONES:
		{
			/*
			 * set up to cycle through all possible initial
			 * test Patterns for walking ones test
			 */

			for (j = 0L; j < 32; j++) {
				/*
				 * Generate an initial value for walking ones test to test for bad
				 * data bits
				 */

				Val = 1 << j;

				/*
				 * START walking ones test
				 * Write a one to each data bit indifferent locations
				 */

				for (i = 0L; i < 32; i++) {

					/* write memory location */

					Addr[i] = Val;
					Val = (u32) RotateLeft(Val, 32);

				}

				/*
				 * Restore the reference 'Val' to the
				 * initial value
				 */
				Val = 1 << j;

				/* Read the values from each location that was written */

				for (i = 0L; i < 32; i++) {
					/* read memory location */

					Word = Addr[i];

					if (Word != Val) {
						return XST_MEMTEST_FAILED;
					}

					Val = (u32) RotateLeft(Val, 32);

				}

			}

			if (Subtest != XUT_ALLMEMTESTS) {
				return XST_SUCCESS;
			}


		}		/* end of case 2 */

		/* fall through case statement */

	case XUT_WALKZEROS:
		{
			/*
			 * set up to cycle through all possible
			 * initial test Patterns for walking zeros test
			 */

			for (j = 0L; j < 32; j++) {

				/*
				 * Generate an initial value for walking ones test to test for
				 * bad data bits
				 */

				Val = ~(1 << j);

				/*
				 * START walking zeros test
				 * Write a one to each data bit indifferent locations
				 */

				for (i = 0L; i < 32; i++) {

					/* write memory location */

					Addr[i] = Val;
					Val = ~((u32) RotateLeft(~Val, 32));

				}

				/*
				 * Restore the reference 'Val' to the
				 * initial value
				 */

				Val = ~(1 << j);

				/* Read the values from each location that was written */

				for (i = 0L; i < 32; i++) {

					/* read memory location */

					Word = Addr[i];

					if (Word != Val) {
						return XST_MEMTEST_FAILED;
					}

					Val = ~((u32) RotateLeft(~Val, 32));

				}

			}

			if (Subtest != XUT_ALLMEMTESTS) {
				return XST_SUCCESS;
			}

		}		/* end of case 3 */

		/* fall through case statement */

	case XUT_INVERSEADDR:
		{

			/* Fill the memory with inverse of address */

			for (i = 0L; i < Words; i++) {

				/* write memory location */

				Val = (u32) (~((u32) (&Addr[i])));

				Addr[i] = Val;

			}

			/*
			 * Check every word within the Words
			 * of tested memory
			 */

			for (i = 0L; i < Words; i++) {

				/* Read the location */

				Word = Addr[i];

				Val = (u32) (~((u32) (&Addr[i])));

				if ((Word ^ Val) != 0x00000000) {
					return XST_MEMTEST_FAILED;
				}
			}

			if (Subtest != XUT_ALLMEMTESTS) {
				return XST_SUCCESS;
			}


		}		/* end of case 4 */


		/* fall through case statement */

	case XUT_FIXEDPATTERN:
		{

			/*
			 * Generate an initial value for
			 * memory testing
			 */

			if (Pattern == 0) {
				Val = 0xDEADBEEF;

			}
			else {
				Val = Pattern;

			}

			/*
			 * Fill the memory with fixed pattern
			 */

			for (i = 0L; i < Words; i++) {
				/* write memory location */

				Addr[i] = Val;

			}

			/*
			 * Check every word within the Words
			 * of tested memory and compare it
			 * with the fixed pattern
			 */

			for (i = 0L; i < Words; i++) {

				/* read memory location */

				Word = Addr[i];

				if (Word != Val) {
					return XST_MEMTEST_FAILED;
				}
			}

			if (Subtest != XUT_ALLMEMTESTS) {
				return XST_SUCCESS;
			}

		}		/* end of case 5 */

		/* this break is for the prior fall through case statements */

		break;

	default:
		{
			return XST_MEMTEST_FAILED;
		}

	}			/* end of switch */

	/* Successfully passed memory test ! */

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Performs a destructive 16-bit wide memory test.
*
* @param    Addr is a pointer to the region of memory to be tested.
* @param    Words is the length of the block.
* @param    Pattern is the constant used for the constant pattern test, if 0,
*           0xDEADBEEF is used.
* @param    Subtest is the test selected. See xutil.h for possible values.
*
* @return
*
* - XST_MEMTEST_FAILED is returned for a failure
* - XST_SUCCESS is returned for a pass
*
* @note
*
* Used for spaces where the address range of the region is smaller than
* the data width. If the memory range is greater than 2 ** width,
* the patterns used in XUT_WALKONES and XUT_WALKZEROS will repeat on a
* boundary of a power of two making it more difficult to detect addressing
* errors. The XUT_INCREMENT and XUT_INVERSEADDR tests suffer the same
* problem. Ideally, if large blocks of memory are to be tested, break
* them up into smaller regions of memory to allow the test patterns used
* not to repeat over the region tested.
*
*****************************************************************************/
int XUtil_MemoryTest16(u16 *Addr, u32 Words, u16 Pattern, u8 Subtest)
{
	u32 i;
	u32 j;
	u16 Val = XUT_MEMTEST_INIT_VALUE;
	u16 FirstVal = XUT_MEMTEST_INIT_VALUE;
	u16 Word;

	XASSERT_NONVOID(Words != 0);
	XASSERT_NONVOID(Subtest <= XUT_MAXTEST);

	/*
	 * selectthe proper Subtest(s)
	 */

	switch (Subtest) {

	case XUT_ALLMEMTESTS:

		/* this case executes all of the Subtests */

		/* fall through case statement */

	case XUT_INCREMENT:
		{

			/*
			 * Fill the memory with incrementing
			 * values starting from 'FirstVal'
			 */
			for (i = 0L; i < Words; i++) {
				/* write memory location */

				Addr[i] = Val;

				Val++;
			}

			/*
			 * Restore the reference 'Val' to the
			 * initial value
			 */

			Val = FirstVal;

			/*
			 * Check every word within the Words
			 * of tested memory and compare it
			 * with the incrementing reference
			 * Val
			 */

			for (i = 0L; i < Words; i++) {

				/* read memory location */

				Word = Addr[i];

				if (Word != Val) {
					return XST_MEMTEST_FAILED;
				}
				Val++;
			}

			if (Subtest != XUT_ALLMEMTESTS) {
				return XST_SUCCESS;
			}

		}		/* end of case 1 */

		/* fall through case statement */

	case XUT_WALKONES:
		{
			/*
			 * set up to cycle through all possible initial test
			 * Patterns for walking ones test
			 */

			for (j = 0L; j < 16; j++) {
				/*
				 * Generate an initial value for walking ones test to test for bad
				 * data bits
				 */

				Val = 1 << j;

				/*
				 * START walking ones test
				 * Write a one to each data bit indifferent locations
				 */

				for (i = 0L; i < 16; i++) {

					/* write memory location */

					Addr[i] = Val;

					Val = (u16) RotateLeft(Val, 16);

				}

				/*
				 * Restore the reference 'Val' to the
				 * initial value
				 */

				Val = 1 << j;

				/* Read the values from each location that was written */

				for (i = 0L; i < 16; i++) {

					/* read memory location */

					Word = Addr[i];

					if (Word != Val) {
						return XST_MEMTEST_FAILED;
					}

					Val = (u16) RotateLeft(Val, 16);

				}

			}

			if (Subtest != XUT_ALLMEMTESTS) {
				return XST_SUCCESS;
			}


		}		/* end of case 2 */

		/* fall through case statement */

	case XUT_WALKZEROS:
		{
			/*
			 * set up to cycle through all possible initial
			 * test Patterns for walking zeros test
			 */

			for (j = 0L; j < 16; j++) {

				/*
				 * Generate an initial value for walking ones
				 * test to test for bad
				 * data bits
				 */

				Val = ~(1 << j);

				/*
				 * START walking zeros test
				 * Write a one to each data bit indifferent locations
				 */

				for (i = 0L; i < 16; i++) {


					/* write memory location */

					Addr[i] = Val;
					Val = ~((u16) RotateLeft(~Val, 16));

				}

				/*
				 * Restore the reference 'Val' to the
				 * initial value
				 */

				Val = ~(1 << j);

				/* Read the values from each location that was written */

				for (i = 0L; i < 16; i++) {

					/* read memory location */

					Word = Addr[i];

					if (Word != Val) {
						return XST_MEMTEST_FAILED;
					}

					Val = ~((u16) RotateLeft(~Val, 16));

				}

			}

			if (Subtest != XUT_ALLMEMTESTS) {
				return XST_SUCCESS;
			}

		}		/* end of case 3 */

		/* fall through case statement */

	case XUT_INVERSEADDR:
		{

			/* Fill the memory with inverse of address */

			for (i = 0L; i < Words; i++) {
				/* write memory location */

				Val = (u16) (~((u32) (&Addr[i])));
				Addr[i] = Val;

			}

			/*
			 * Check every word within the Words
			 * of tested memory
			 */

			for (i = 0L; i < Words; i++) {

				/* read memory location */

				Word = Addr[i];

				Val = (u16) (~((u32) (&Addr[i])));

				if ((Word ^ Val) != 0x0000) {
					return XST_MEMTEST_FAILED;
				}
			}

			if (Subtest != XUT_ALLMEMTESTS) {
				return XST_SUCCESS;
			}


		}		/* end of case 4 */


		/* fall through case statement */

	case XUT_FIXEDPATTERN:
		{

			/*
			 * Generate an initial value for
			 * memory testing
			 */

			if (Pattern == 0) {
				Val = 0xDEAD;

			}
			else {
				Val = Pattern;

			}

			/*
			 * Fill the memory with fixed pattern
			 */

			for (i = 0L; i < Words; i++) {

				/* write memory location */

				Addr[i] = Val;

			}

			/*
			 * Check every word within the Words
			 * of tested memory and compare it
			 * with the fixed pattern
			 */

			for (i = 0L; i < Words; i++) {

				/* read memory location */

				Word = Addr[i];

				if (Word != Val) {
					return XST_MEMTEST_FAILED;
				}
			}

			if (Subtest != XUT_ALLMEMTESTS) {
				return XST_SUCCESS;
			}

		}		/* end of case 5 */

		/* this break is for the prior fall through case statements */

		break;

	default:
		{
			return XST_MEMTEST_FAILED;
		}

	}			/* end of switch */

	/* Successfully passed memory test ! */

	return XST_SUCCESS;
}


/*****************************************************************************/
/**
*
* Performs a destructive 8-bit wide memory test.
*
* @param    Addr is a pointer to the region of memory to be tested.
* @param    Words is the length of the block.
* @param    Pattern is the constant used for the constant pattern test, if 0,
*           0xDEADBEEF is used.
* @param    Subtest is the test selected. See xutil.h for possible values.
*
* @return
*
* - XST_MEMTEST_FAILED is returned for a failure
* - XST_SUCCESS is returned for a pass
*
* @note
*
* Used for spaces where the address range of the region is smaller than
* the data width. If the memory range is greater than 2 ** width,
* the patterns used in XUT_WALKONES and XUT_WALKZEROS will repeat on a
* boundary of a power of two making it more difficult to detect addressing
* errors. The XUT_INCREMENT and XUT_INVERSEADDR tests suffer the same
* problem. Ideally, if large blocks of memory are to be tested, break
* them up into smaller regions of memory to allow the test patterns used
* not to repeat over the region tested.
*
*****************************************************************************/
int XUtil_MemoryTest8(u8 *Addr, u32 Words, u8 Pattern, u8 Subtest)
{
	u32 i;
	u32 j;
	u8 Val = XUT_MEMTEST_INIT_VALUE;
	u8 FirstVal = XUT_MEMTEST_INIT_VALUE;
	u8 Word;

	XASSERT_NONVOID(Words != 0);
	XASSERT_NONVOID(Subtest <= XUT_MAXTEST);

	/*
	 * select the proper Subtest(s)
	 */

	switch (Subtest) {

	case XUT_ALLMEMTESTS:

		/* this case executes all of the Subtests */

		/* fall through case statement */

	case XUT_INCREMENT:
		{

			/*
			 * Fill the memory with incrementing
			 * values starting from 'FirstVal'
			 */
			for (i = 0L; i < Words; i++) {

				/* write memory location */

				Addr[i] = Val;
				Val++;
			}

			/*
			 * Restore the reference 'Val' to the
			 * initial value
			 */

			Val = FirstVal;

			/*
			 * Check every word within the Words
			 * of tested memory and compare it
			 * with the incrementing reference
			 * Val
			 */

			for (i = 0L; i < Words; i++) {

				/* read memory location */

				Word = Addr[i];

				if (Word != Val) {
					return XST_MEMTEST_FAILED;
				}
				Val++;
			}


			if (Subtest != XUT_ALLMEMTESTS) {
				return XST_SUCCESS;
			}


		}		/* end of case 1 */

		/* fall through case statement */

	case XUT_WALKONES:
		{
			/*
			 * set up to cycle through all possible initial
			 * test Patterns for walking ones test
			 */

			for (j = 0L; j < 8; j++) {
				/*
				 * Generate an initial value for walking ones test to test
				 * for bad data bits
				 */

				Val = 1 << j;

				/*
				 * START walking ones test
				 * Write a one to each data bit indifferent locations
				 */

				for (i = 0L; i < 8; i++) {

					/* write memory location */

					Addr[i] = Val;
					Val = (u8) RotateLeft(Val, 8);
				}

				/*
				 * Restore the reference 'Val' to the
				 * initial value
				 */
				Val = 1 << j;

				/* Read the values from each location that was written */

				for (i = 0L; i < 8; i++) {

					/* read memory location */

					Word = Addr[i];

					if (Word != Val) {
						return XST_MEMTEST_FAILED;
					}

					Val = (u8) RotateLeft(Val, 8);

				}

			}

			if (Subtest != XUT_ALLMEMTESTS) {
				return XST_SUCCESS;
			}


		}		/* end of case 2 */

		/* fall through case statement */

	case XUT_WALKZEROS:
		{
			/*
			 * set up to cycle through all possible initial test
			 * Patterns for walking zeros test
			 */

			for (j = 0L; j < 8; j++) {

				/*
				 * Generate an initial value for walking ones test to test
				 * for bad data bits
				 */

				Val = ~(1 << j);

				/*
				 * START walking zeros test
				 * Write a one to each data bit indifferent locations
				 */

				for (i = 0L; i < 8; i++) {


					/* write memory location */

					Addr[i] = Val;
					Val = ~((u8) RotateLeft(~Val, 8));

				}

				/*
				 * Restore the reference 'Val' to the
				 * initial value
				 */

				Val = ~(1 << j);

				/* Read the values from each location that was written */

				for (i = 0L; i < 8; i++) {

					/* read memory location */

					Word = Addr[i];

					if (Word != Val) {
						return XST_MEMTEST_FAILED;
					}

					Val = ~((u8) RotateLeft(~Val, 8));

				}

			}

			if (Subtest != XUT_ALLMEMTESTS) {
				return XST_SUCCESS;
			}

		}		/* end of case 3 */

		/* fall through case statement */

	case XUT_INVERSEADDR:
		{

			/* Fill the memory with inverse of address */

			for (i = 0L; i < Words; i++) {

				/* write memory location */

				Val = (u8) (~((u32) (&Addr[i])));
				Addr[i] = Val;

			}

			/*
			 * Check every word within the Words
			 * of tested memory
			 */

			for (i = 0L; i < Words; i++) {

				/* read memory location */

				Word = Addr[i];

				Val = (u8) (~((u32) (&Addr[i])));

				if ((Word ^ Val) != 0x00) {
					return XST_MEMTEST_FAILED;
				}
			}

			if (Subtest != XUT_ALLMEMTESTS) {
				return XST_SUCCESS;
			}


		}		/* end of case 4 */


		/* fall through case statement */

	case XUT_FIXEDPATTERN:
		{

			/*
			 * Generate an initial value for
			 * memory testing
			 */

			if (Pattern == 0) {
				Val = 0xA5;

			}
			else {
				Val = Pattern;

			}

			/*
			 * Fill the memory with fixed pattern
			 */

			for (i = 0L; i < Words; i++) {

				/* write memory location */

				Addr[i] = Val;

			}

			/*
			 * Check every word within the Words
			 * of tested memory and compare it
			 * with the fixed pattern
			 */

			for (i = 0L; i < Words; i++) {

				/* read memory location */

				Word = Addr[i];

				if (Word != Val) {
					return XST_MEMTEST_FAILED;
				}
			}

			if (Subtest != XUT_ALLMEMTESTS) {
				return XST_SUCCESS;
			}

		}		/* end of case 5 */

		/* this break is for the prior fall through case statements */

		break;

	default:
		{
			return XST_MEMTEST_FAILED;
		}

	}			/* end of switch */

	/* Successfully passed memory test ! */

	return XST_SUCCESS;
}


/*****************************************************************************/
/**
*
* Rotates the provided value to the left one bit position
*
* @param    Input is value to be rotated to the left
* @param    Width is the number of bits in the input data
*
* @return
*
* The resulting unsigned long value of the rotate left
*
* @note
*
* None.
*
*****************************************************************************/
static u32 RotateLeft(u32 Input, u8 Width)
{
	u32 Msb;
	u32 ReturnVal;
	u32 WidthMask;
	u32 MsbMask;

	/*
	 * set up the WidthMask and the MsbMask
	 */

	MsbMask = 1 << (Width - 1);

	WidthMask = (MsbMask << 1) - 1;

	/*
	 * set the width of the Input to the correct width
	 */

	Input = Input & WidthMask;

	Msb = Input & MsbMask;

	ReturnVal = Input << 1;

	if (Msb != 0x00000000) {
		ReturnVal = ReturnVal | 0x00000001;
	}

	ReturnVal = ReturnVal & WidthMask;

	return (ReturnVal);

}

#ifdef ROTATE_RIGHT
/*****************************************************************************/
/**
*
* Rotates the provided value to the right one bit position
*
* @param    Input is value to be rotated to the right
* @param    Width is the number of bits in the input data
*
* @return
*
* The resulting u32 value of the rotate right
*
* @note
*
* None.
*
*****************************************************************************/
static u32 RotateRight(u32 Input, u8 Width)
{
	u32 Lsb;
	u32 ReturnVal;
	u32 WidthMask;
	u32 MsbMask;

	/*
	 * set up the WidthMask and the MsbMask
	 */

	MsbMask = 1 << (Width - 1);

	WidthMask = (MsbMask << 1) - 1;

	/*
	 * set the width of the Input to the correct width
	 */

	Input = Input & WidthMask;

	ReturnVal = Input >> 1;

	Lsb = Input & 0x00000001;

	if (Lsb != 0x00000000) {
		ReturnVal = ReturnVal | MsbMask;
	}

	ReturnVal = ReturnVal & WidthMask;

	return (ReturnVal);

}
#endif /* ROTATE_RIGHT */
/** @} */
