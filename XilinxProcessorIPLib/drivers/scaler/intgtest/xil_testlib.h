/******************************************************************************
*
*       XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS"
*       AS A COURTESY TO YOU, SOLELY FOR USE IN DEVELOPING PROGRAMS AND
*       SOLUTIONS FOR XILINX DEVICES.  BY PROVIDING THIS DESIGN, CODE,
*       OR INFORMATION AS ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE,
*       APPLICATION OR STANDARD, XILINX IS MAKING NO REPRESENTATION
*       THAT THIS IMPLEMENTATION IS FREE FROM ANY CLAIMS OF INFRINGEMENT,
*       AND YOU ARE RESPONSIBLE FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE
*       FOR YOUR IMPLEMENTATION.  XILINX EXPRESSLY DISCLAIMS ANY
*       WARRANTY WHATSOEVER WITH RESPECT TO THE ADEQUACY OF THE
*       IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR
*       REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM CLAIMS OF
*       INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*       FOR A PARTICULAR PURPOSE.
*
*       (c) Copyright 2002-2014 Xilinx Inc.
*       All rights reserved.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xil_testlib.h
*
* This file contains Xilinx software exit messages. These messages are set to
* be matched with the scan strings for board test such that board test would
* know the result of a standalone test program.
*
******************************************************************************/

/***************************** Include Files *********************************/

#include <xil_printf.h>

#ifndef XIL_TESTLIB_H			/* prevent circular inclusions */
#define XIL_TESTLIB_H			/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************** Macros (Inline Functions) Definitions *********************/
/*
 * XIL_PASS puts out a message to signal board test that a program has pass
 * the testing program without errors
 *
 * @return	None.
 *
 * @note	None.
 */
#define XIL_PASS() xil_printf("\r\nREGRESSION TEST PASS.\r\n")

/* XIL_FAIL puts out a message to signal board test that a program has not
 * pass the testing program
 *
 * @param	numErrors, number of errors.
 *
 * @return	None.
 *
 * @note	None.
 */
#define XIL_FAIL(numErrors) xil_printf("\r\nREGRESSION TEST FAIL WITH %d \
		ERRORS.\r\n", numErrors)

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
