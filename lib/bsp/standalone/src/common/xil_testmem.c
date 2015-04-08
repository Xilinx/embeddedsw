/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information  of Xilinx, Inc.
* and is protected under U.S. and  international copyright and other
* intellectual property  laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any  rights to the
* materials distributed herewith. Except as  otherwise provided in a valid
* license issued to you by  Xilinx, and to the maximum extent permitted by
* applicable law:
* (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND  WITH ALL FAULTS, AND
* XILINX HEREBY DISCLAIMS ALL WARRANTIES  AND CONDITIONS, EXPRESS, IMPLIED,
* OR STATUTORY, INCLUDING  BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
* NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE
* and
* (2) Xilinx shall not be liable (whether in contract or tort,  including
* negligence, or under any other theory of liability) for any loss or damage of
* any kind or nature  related to, arising under or in connection with these
* materials, including for any direct, or any indirect,  special, incidental,
* or consequential loss or damage  (including loss of data, profits, goodwill,
* or any type of  loss or damage suffered as a result of any action brought
* by a third party) even if such damage or loss was  reasonably foreseeable
* or Xilinx had been advised of the  possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe  performance, such as life-support or
* safety devices or  systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any  other applications
* that could lead to death, personal  injury, or severe property or environmental
* damage  (individually and collectively, "Critical  Applications").
* Customer assumes the sole risk and liability of any use of Xilinx products in
* Critical  Applications, subject only to applicable laws and  regulations
* governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xil_testmem.c
*
* Contains the memory test utility functions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver    Who    Date    Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a hbm  08/25/09 First release
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/
#include "xil_testmem.h"
#include "xil_io.h"
#include "xil_assert.h"

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
* Perform a destructive 32-bit wide memory test.
*
* @param    Addr is a pointer to the region of memory to be tested.
* @param    Words is the length of the block.
* @param    Pattern is the constant used for the constant pattern test, if 0,
*           0xDEADBEEF is used.
* @param    Subtest is the test selected. See xil_testmem.h for possible
*	    values.
*
* @return
*
* - 0 is returned for a pass
* - -1 is returned for a failure
*
* @note
*
* Used for spaces where the address range of the region is smaller than
* the data width. If the memory range is greater than 2 ** Width,
* the patterns used in XIL_TESTMEM_WALKONES and XIL_TESTMEM_WALKZEROS will
* repeat on a boundry of a power of two making it more difficult to detect
* addressing errors. The XIL_TESTMEM_INCREMENT and XIL_TESTMEM_INVERSEADDR
* tests suffer the same problem. Ideally, if large blocks of memory are to be
* tested, break them up into smaller regions of memory to allow the test
* patterns used not to repeat over the region tested.
*
*****************************************************************************/
s32 Xil_TestMem32(u32 *Addr, u32 Words, u32 Pattern, u8 Subtest)
{
	u32 I;
	u32 j;
	u32 Val;
	u32 FirtVal;
	u32 WordMem32;
	s32 Status = 0;

	Xil_AssertNonvoid(Words != (u32)0);
	Xil_AssertNonvoid(Subtest <= (u8)XIL_TESTMEM_MAXTEST);
	Xil_AssertNonvoid(Addr != NULL);

	/*
	 * variable initialization
	 */
	Val = XIL_TESTMEM_INIT_VALUE;
	FirtVal = XIL_TESTMEM_INIT_VALUE;


	if((Subtest == XIL_TESTMEM_ALLMEMTESTS) || (Subtest == XIL_TESTMEM_INCREMENT)) {
		/*
		 * Fill the memory with incrementing
		 * values starting from 'FirtVal'
		 */
		for (I = 0U; I < Words; I++) {
			*(Addr+I) = Val;
			Val++;
		}

		/*
		 * Restore the reference 'Val' to the
		 * initial value
		 */
		Val = FirtVal;

		/*
		 * Check every word within the words
		 * of tested memory and compare it
		 * with the incrementing reference
		 * Val
		 */

		for (I = 0U; I < Words; I++) {
			WordMem32 = *(Addr+I);

			if (WordMem32 != Val) {
				Status = -1;
				goto End_Label;
			}

			Val++;
		}
	}

	if((Subtest == XIL_TESTMEM_ALLMEMTESTS) || (Subtest == XIL_TESTMEM_WALKONES)) {
		/*
		 * set up to cycle through all possible initial
		 * test Patterns for walking ones test
		 */

		for (j = 0U; j < (u32)32; j++) {
			/*
			 * Generate an initial value for walking ones test
			 * to test for bad data bits
			 */

			Val = (1U << j);

			/*
			 * START walking ones test
			 * Write a one to each data bit indifferent locations
			 */

			for (I = 0U; I < (u32)32; I++) {
				/* write memory location */
				*(Addr+I) = Val;
				Val = (u32) RotateLeft(Val, 32U);
			}

			/*
			 * Restore the reference 'val' to the
			 * initial value
			 */
			Val = 1U << j;

			/* Read the values from each location that was
			 * written */
			for (I = 0U; I < (u32)32; I++) {
				/* read memory location */

				WordMem32 = *(Addr+I);

				if (WordMem32 != Val) {
					Status = -1;
					goto End_Label;
				}

				Val = (u32)RotateLeft(Val, 32U);
			}
		}
	}

	if((Subtest == XIL_TESTMEM_ALLMEMTESTS) || (Subtest == XIL_TESTMEM_WALKZEROS)) {
		/*
		 * set up to cycle through all possible
		 * initial test Patterns for walking zeros test
		 */

		for (j = 0U; j < (u32)32; j++) {

			/*
			 * Generate an initial value for walking ones test
			 * to test for bad data bits
			 */

			Val = ~(1U << j);

			/*
			 * START walking zeros test
			 * Write a one to each data bit indifferent locations
			 */

			for (I = 0U; I < (u32)32; I++) {
				/* write memory location */
				*(Addr+I) = Val;
				Val = ~((u32)RotateLeft(~Val, 32U));
			}

			/*
			 * Restore the reference 'Val' to the
			 * initial value
			 */

			Val = ~(1U << j);

			/* Read the values from each location that was
			 * written */
			for (I = 0U; I < (u32)32; I++) {
				/* read memory location */
				WordMem32 = *(Addr+I);
				if (WordMem32 != Val) {
					Status = -1;
					goto End_Label;
				}
				Val = ~((u32)RotateLeft(~Val, 32U));
			}

		}
	}

	if((Subtest == XIL_TESTMEM_ALLMEMTESTS) || (Subtest == XIL_TESTMEM_INVERSEADDR)) {
		/* Fill the memory with inverse of address */
		for (I = 0U; I < Words; I++) {
			/* write memory location */
			Val = (u32) (~((INTPTR) (&Addr[I])));
			*(Addr+I) = Val;
		}

		/*
		 * Check every word within the words
		 * of tested memory
		 */

		for (I = 0U; I < Words; I++) {
			/* Read the location */
			WordMem32 = *(Addr+I);
			Val = (u32) (~((INTPTR) (&Addr[I])));

			if ((WordMem32 ^ Val) != 0x00000000U) {
				Status = -1;
				goto End_Label;
			}
		}
	}

	if((Subtest == XIL_TESTMEM_ALLMEMTESTS) || (Subtest == XIL_TESTMEM_FIXEDPATTERN)) {
		/*
		 * Generate an initial value for
		 * memory testing
		 */

		if (Pattern == (u32)0) {
			Val = 0xDEADBEEFU;
		}
		else {
			Val = Pattern;
		}

		/*
		 * Fill the memory with fixed Pattern
		 */

		for (I = 0U; I < Words; I++) {
			/* write memory location */
			*(Addr+I) = Val;
		}

		/*
		 * Check every word within the words
		 * of tested memory and compare it
		 * with the fixed Pattern
		 */

		for (I = 0U; I < Words; I++) {

			/* read memory location */

			WordMem32 = *(Addr+I);
			if (WordMem32 != Val) {
				Status = -1;
				goto End_Label;
			}
		}
	}

End_Label:
	return Status;
}

/*****************************************************************************/
/**
*
* Perform a destructive 16-bit wide memory test.
*
* @param    Addr is a pointer to the region of memory to be tested.
* @param    Words is the length of the block.
* @param    Pattern is the constant used for the constant Pattern test, if 0,
*           0xDEADBEEF is used.
* @param    Subtest is the test selected. See xil_testmem.h for possible
*	    values.
*
* @return
*
* - -1 is returned for a failure
* - 0 is returned for a pass
*
* @note
*
* Used for spaces where the address range of the region is smaller than
* the data width. If the memory range is greater than 2 ** Width,
* the patterns used in XIL_TESTMEM_WALKONES and XIL_TESTMEM_WALKZEROS will
* repeat on a boundry of a power of two making it more difficult to detect
* addressing errors. The XIL_TESTMEM_INCREMENT and XIL_TESTMEM_INVERSEADDR
* tests suffer the same problem. Ideally, if large blocks of memory are to be
* tested, break them up into smaller regions of memory to allow the test
* patterns used not to repeat over the region tested.
*
*****************************************************************************/
s32 Xil_TestMem16(u16 *Addr, u32 Words, u16 Pattern, u8 Subtest)
{
	u32 I;
	u32 j;
	u16 Val;
	u16 FirtVal;
	u16 WordMem16;
	s32 Status = 0;

	Xil_AssertNonvoid(Words != (u32)0);
	Xil_AssertNonvoid(Subtest <= XIL_TESTMEM_MAXTEST);
	Xil_AssertNonvoid(Addr != NULL);

	/*
	 * variable initialization
	 */
	Val = XIL_TESTMEM_INIT_VALUE;
	FirtVal = XIL_TESTMEM_INIT_VALUE;

	/*
	 * selectthe proper Subtest(s)
	 */

	if((Subtest == XIL_TESTMEM_ALLMEMTESTS) || (Subtest == XIL_TESTMEM_INCREMENT)) {
		/*
		 * Fill the memory with incrementing
		 * values starting from 'FirtVal'
		 */
		for (I = 0U; I < Words; I++) {
			/* write memory location */
			*(Addr+I) = Val;
			Val++;
		}
		/*
		 * Restore the reference 'Val' to the
		 * initial value
		 */
		Val = FirtVal;

		/*
		 * Check every word within the words
		 * of tested memory and compare it
		 * with the incrementing reference val
		 */

		for (I = 0U; I < Words; I++) {
			/* read memory location */
			WordMem16 = *(Addr+I);
			if (WordMem16 != Val) {
				Status = -1;
				goto End_Label;
			}
			Val++;
		}
	}

	if((Subtest == XIL_TESTMEM_ALLMEMTESTS) || (Subtest == XIL_TESTMEM_WALKONES)) {
		/*
		 * set up to cycle through all possible initial test
		 * Patterns for walking ones test
		 */

		for (j = 0U; j < (u32)16; j++) {
			/*
			 * Generate an initial value for walking ones test
			 * to test for bad data bits
			 */

			Val = (u16)((u32)1 << j);
			/*
			 * START walking ones test
			 * Write a one to each data bit indifferent locations
			 */

			for (I = 0U; I < (u32)16; I++) {
				/* write memory location */
				*(Addr+I) = Val;
				Val = (u16)RotateLeft(Val, 16U);
			}
			/*
			 * Restore the reference 'Val' to the
			 * initial value
			 */
			Val = (u16)((u32)1 << j);
			/* Read the values from each location that was written */
			for (I = 0U; I < (u32)16; I++) {
				/* read memory location */
				WordMem16 = *(Addr+I);
				if (WordMem16 != Val) {
					Status = -1;
					goto End_Label;
				}
				Val = (u16)RotateLeft(Val, 16U);
			}
		}
	}

	if((Subtest == XIL_TESTMEM_ALLMEMTESTS) || (Subtest == XIL_TESTMEM_WALKZEROS)) {
		/*
		 * set up to cycle through all possible initial
		 * test Patterns for walking zeros test
		 */

		for (j = 0U; j < (u32)16; j++) {
			/*
			 * Generate an initial value for walking ones
			 * test to test for bad
			 * data bits
			 */

			Val = ~(1U << j);
			/*
			 * START walking zeros test
			 * Write a one to each data bit indifferent locations
			 */

			for (I = 0U; I < (u32)16; I++) {
				/* write memory location */
				*(Addr+I) = Val;
				Val = ~((u16)RotateLeft(~Val, 16U));
			}
			/*
			 * Restore the reference 'Val' to the
			 * initial value
			 */
			Val = ~(1U << j);
			/* Read the values from each location that was written */
			for (I = 0U; I < (u32)16; I++) {
				/* read memory location */
				WordMem16 = *(Addr+I);
				if (WordMem16 != Val) {
					Status = -1;
					goto End_Label;
				}
				Val = ~((u16)RotateLeft(~Val, 16U));
			}

		}
	}

	if((Subtest == XIL_TESTMEM_ALLMEMTESTS) || (Subtest == XIL_TESTMEM_INVERSEADDR)) {
		/* Fill the memory with inverse of address */
		for (I = 0U; I < Words; I++) {
			/* write memory location */
			Val = (u16) (~((INTPTR)(&Addr[I])));
			*(Addr+I) = Val;
		}
		/*
		 * Check every word within the words
		 * of tested memory
		 */

		for (I = 0U; I < Words; I++) {
			/* read memory location */
			WordMem16 = *(Addr+I);
			Val = (u16) (~((INTPTR) (&Addr[I])));
			if ((WordMem16 ^ Val) != 0x0000U) {
				Status = -1;
				goto End_Label;
			}
		}
	}

	if((Subtest == XIL_TESTMEM_ALLMEMTESTS) || (Subtest == XIL_TESTMEM_FIXEDPATTERN)) {
		/*
		 * Generate an initial value for
		 * memory testing
		 */
		if (Pattern == (u16)0) {
			Val = 0xDEADU;
		}
		else {
			Val = Pattern;
		}

		/*
		 * Fill the memory with fixed pattern
		 */

		for (I = 0U; I < Words; I++) {
			/* write memory location */
			*(Addr+I) = Val;
		}

		/*
		 * Check every word within the words
		 * of tested memory and compare it
		 * with the fixed pattern
		 */

		for (I = 0U; I < Words; I++) {
			/* read memory location */
			WordMem16 = *(Addr+I);
			if (WordMem16 != Val) {
				Status = -1;
				goto End_Label;
			}
		}
	}

End_Label:
	return Status;
}


/*****************************************************************************/
/**
*
* Perform a destructive 8-bit wide memory test.
*
* @param    Addr is a pointer to the region of memory to be tested.
* @param    Words is the length of the block.
* @param    Pattern is the constant used for the constant pattern test, if 0,
*           0xDEADBEEF is used.
* @param    Subtest is the test selected. See xil_testmem.h for possible
*	    values.
*
* @return
*
* - -1 is returned for a failure
* - 0 is returned for a pass
*
* @note
*
* Used for spaces where the address range of the region is smaller than
* the data width. If the memory range is greater than 2 ** Width,
* the patterns used in XIL_TESTMEM_WALKONES and XIL_TESTMEM_WALKZEROS will
* repeat on a boundry of a power of two making it more difficult to detect
* addressing errors. The XIL_TESTMEM_INCREMENT and XIL_TESTMEM_INVERSEADDR
* tests suffer the same problem. Ideally, if large blocks of memory are to be
* tested, break them up into smaller regions of memory to allow the test
* patterns used not to repeat over the region tested.
*
*****************************************************************************/
s32 Xil_TestMem8(u8 *Addr, u32 Words, u8 Pattern, u8 Subtest)
{
	u32 I;
	u32 j;
	u8 Val;
	u8 FirtVal;
	u8 WordMem8;
	s32 Status = 0;

	Xil_AssertNonvoid(Words != (u32)0);
	Xil_AssertNonvoid(Subtest <= XIL_TESTMEM_MAXTEST);
	Xil_AssertNonvoid(Addr != NULL);

	/*
	 * variable initialization
	 */
	Val = XIL_TESTMEM_INIT_VALUE;
	FirtVal = XIL_TESTMEM_INIT_VALUE;

	/*
	 * select the proper Subtest(s)
	 */

	if((Subtest == XIL_TESTMEM_ALLMEMTESTS) || (Subtest == XIL_TESTMEM_INCREMENT)) {
		/*
		 * Fill the memory with incrementing
		 * values starting from 'FirtVal'
		 */
		for (I = 0U; I < Words; I++) {
			/* write memory location */
			*(Addr+I) = Val;
			Val++;
		}
		/*
		 * Restore the reference 'Val' to the
		 * initial value
		 */
		Val = FirtVal;
		/*
		 * Check every word within the words
		 * of tested memory and compare it
		 * with the incrementing reference
		 * Val
		 */

		for (I = 0U; I < Words; I++) {
			/* read memory location */
			WordMem8 = *(Addr+I);
			if (WordMem8 != Val) {
				Status = -1;
				goto End_Label;
			}
			Val++;
		}
	}

	if((Subtest == XIL_TESTMEM_ALLMEMTESTS) || (Subtest == XIL_TESTMEM_WALKONES)) {
		/*
		 * set up to cycle through all possible initial
		 * test Patterns for walking ones test
		 */

		for (j = 0U; j < (u32)8; j++) {
			/*
			 * Generate an initial value for walking ones test
			 * to test for bad data bits
			 */
			Val = (u8)((u32)1 << j);
			/*
			 * START walking ones test
			 * Write a one to each data bit indifferent locations
			 */
			for (I = 0U; I < (u32)8; I++) {
				/* write memory location */
				*(Addr+I) = Val;
				Val = (u8)RotateLeft(Val, 8U);
			}
			/*
			 * Restore the reference 'Val' to the
			 * initial value
			 */
			Val = (u8)((u32)1 << j);
			/* Read the values from each location that was written */
			for (I = 0U; I < (u32)8; I++) {
				/* read memory location */
				WordMem8 = *(Addr+I);
				if (WordMem8 != Val) {
					Status = -1;
					goto End_Label;
				}
				Val = (u8)RotateLeft(Val, 8U);
			}
		}
	}

	if((Subtest == XIL_TESTMEM_ALLMEMTESTS) || (Subtest == XIL_TESTMEM_WALKZEROS)) {
		/*
		 * set up to cycle through all possible initial test
		 * Patterns for walking zeros test
		 */

		for (j = 0U; j < (u32)8; j++) {
			/*
			 * Generate an initial value for walking ones test to test
			 * for bad data bits
			 */
			Val = ~(1U << j);
			/*
			 * START walking zeros test
			 * Write a one to each data bit indifferent locations
			 */
			for (I = 0U; I < (u32)8; I++) {
				/* write memory location */
				*(Addr+I) = Val;
				Val = ~((u8)RotateLeft(~Val, 8U));
			}
			/*
			 * Restore the reference 'Val' to the
			 * initial value
			 */
			Val = ~(1U << j);
			/* Read the values from each location that was written */
			for (I = 0U; I < (u32)8; I++) {
				/* read memory location */
				WordMem8 = *(Addr+I);
				if (WordMem8 != Val) {
					Status = -1;
					goto End_Label;
				}

				Val = ~((u8)RotateLeft(~Val, 8U));
			}
		}
	}

	if((Subtest == XIL_TESTMEM_ALLMEMTESTS) || (Subtest == XIL_TESTMEM_INVERSEADDR)) {
		/* Fill the memory with inverse of address */
		for (I = 0U; I < Words; I++) {
			/* write memory location */
			Val = (u8) (~((INTPTR) (&Addr[I])));
			*(Addr+I) = Val;
		}

		/*
		 * Check every word within the words
		 * of tested memory
		 */

		for (I = 0U; I < Words; I++) {
			/* read memory location */
			WordMem8 = *(Addr+I);
			Val = (u8) (~((INTPTR) (&Addr[I])));
			if ((WordMem8 ^ Val) != 0x00U) {
				Status = -1;
				goto End_Label;
			}
		}
	}

	if((Subtest == XIL_TESTMEM_ALLMEMTESTS) || (Subtest == XIL_TESTMEM_FIXEDPATTERN)) {
		/*
		 * Generate an initial value for
		 * memory testing
		 */

		if (Pattern == (u8)0) {
			Val = 0xA5U;
		}
		else {
			Val = Pattern;
		}
		/*
		 * Fill the memory with fixed Pattern
		 */
		for (I = 0U; I < Words; I++) {
			/* write memory location */
			*(Addr+I) = Val;
		}
		/*
		 * Check every word within the words
		 * of tested memory and compare it
		 * with the fixed Pattern
		 */

		for (I = 0U; I < Words; I++) {
			/* read memory location */
			WordMem8 = *(Addr+I);
			if (WordMem8 != Val) {
				Status = -1;
				goto End_Label;
			}
		}
	}

End_Label:
	return Status;
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
	u32 LocalInput = Input;

	/*
	 * set up the WidthMask and the MsbMask
	 */

	MsbMask = 1U << (Width - 1U);

	WidthMask = (MsbMask << (u32)1) - (u32)1;

	/*
	 * set the Width of the Input to the correct width
	 */

	LocalInput = LocalInput & WidthMask;

	Msb = LocalInput & MsbMask;

	ReturnVal = LocalInput << 1U;

	if (Msb != 0x00000000U) {
		ReturnVal = ReturnVal | (u32)0x00000001;
	}

	ReturnVal = ReturnVal & WidthMask;

	return ReturnVal;

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
	u32 LocalInput = Input;
	/*
	 * set up the WidthMask and the MsbMask
	 */

	MsbMask = 1U << (Width - 1U);

	WidthMask = (MsbMask << 1U) - 1U;

	/*
	 * set the width of the input to the correct width
	 */

	LocalInput = LocalInput & WidthMask;

	ReturnVal = LocalInput >> 1U;

	Lsb = LocalInput & 0x00000001U;

	if (Lsb != 0x00000000U) {
		ReturnVal = ReturnVal | MsbMask;
	}

	ReturnVal = ReturnVal & WidthMask;

	return ReturnVal;

}
#endif /* ROTATE_RIGHT */

