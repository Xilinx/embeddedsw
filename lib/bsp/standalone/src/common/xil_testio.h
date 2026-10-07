/******************************************************************************
* Copyright (c) 2009 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xil_testio.h
*
* @addtogroup common_test_utils Test Utilities for Memory and Caches
* <h2>I/O test </h2>
* The xil_testio.h file contains utility functions to test endian related memory
* IO functions.
*
* A subset of the memory tests can be selected or all of the tests can be run
* in order. If there is an error detected by a subtest, the test stops and the
* failure code is returned. Further tests are not run even if all of the tests
* are selected.
*
* @{
* <pre>
* MODIFICATION HISTORY:
*
* Ver    Who    Date    Changes
* ----- ---- -------- -----------------------------------------------
* 1.00 hbm  08/05/09 First release
* 9.6   ml   10/01/26 Fix doxygen 1.15.0 and compliance checker warnings
* </pre>
*
******************************************************************************/


#ifndef XIL_TESTIO_H
#define XIL_TESTIO_H

/**
 *@cond nocomments
 */
#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"

/************************** Constant Definitions *****************************/


#define XIL_TESTIO_DEFAULT 	0
#define XIL_TESTIO_LE		1
#define XIL_TESTIO_BE		2

/**
 *@endcond
 */

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

extern s32 Xil_TestIO8(u8 *Addr, s32 Length, u8 Value);
extern s32 Xil_TestIO16(u16 *Addr, s32 Length, u16 Value, s32 Kind, s32 Swap);
extern s32 Xil_TestIO32(u32 *Addr, s32 Length, u32 Value, s32 Kind, s32 Swap);

#ifdef __cplusplus
}
#endif

/**
* @} End of "addtogroup common_test_utils".
*/
#endif /* end of protection macro */
