/******************************************************************************
* Copyright (C) 2002 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xutil.h
* @addtogroup common_v1_2
* @{
*
* This file contains utility functions such as memory test functions.
*
* <b>Memory test description</b>
*
* A subset of the memory tests can be selected or all of the tests can be run
* in order. If there is an error detected by a subtest, the test stops and the
* failure code is returned. Further tests are not run even if all of the tests
* are selected.
*
* Subtest descriptions:
* <pre>
* XUT_ALLMEMTESTS:
*       Runs all of the following tests
*
* XUT_INCREMENT:
*       Incrementing Value Test.
*       This test starts at 'XUT_MEMTEST_INIT_VALUE' and uses the incrementing
*       value as the test value for memory.
*
* XUT_WALKONES:
*       Walking Ones Test.
*       This test uses a walking '1' as the test value for memory.
*       location 1 = 0x00000001
*       location 2 = 0x00000002
*       ...
*
* XUT_WALKZEROS:
*       Walking Zero's Test.
*       This test uses the inverse value of the walking ones test
*       as the test value for memory.
*       location 1 = 0xFFFFFFFE
*       location 2 = 0xFFFFFFFD
*       ...
*
* XUT_INVERSEADDR:
*       Inverse Address Test.
*       This test uses the inverse of the address of the location under test
*       as the test value for memory.
*
* XUT_FIXEDPATTERN:
*       Fixed Pattern Test.
*       This test uses the provided patters as the test value for memory.
*       If zero is provided as the pattern the test uses '0xDEADBEEF".
* </pre>
*
* <i>WARNING</i>
*
* The tests are <b>DESTRUCTIVE</b>. Run before any initialized memory spaces
* have been set up.
*
* The address, Addr, provided to the memory tests is not checked for
* validity except for the NULL case. It is possible to provide a code-space
* pointer for this test to start with and ultimately destroy executable code
* causing random failures.
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
* <pre>
* MODIFICATION HISTORY:
*
* Ver    Who    Date    Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a ecm  11/01/01 First release
* 1.00a xd   11/03/04 Improved support for doxygen.
* </pre>
*
******************************************************************************/

#ifndef XUTIL_H			/* prevent circular inclusions */
#define XUTIL_H			/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xbasic_types.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/

/* xutil_memtest defines */

#define XUT_MEMTEST_INIT_VALUE  1

/** @name Memory subtests
 * @{
 */
/**
 * See the detailed description of the subtests in the file description.
 */
#define XUT_ALLMEMTESTS     0
#define XUT_INCREMENT       1
#define XUT_WALKONES        2
#define XUT_WALKZEROS       3
#define XUT_INVERSEADDR     4
#define XUT_FIXEDPATTERN    5
#define XUT_MAXTEST         XUT_FIXEDPATTERN
/* @} */

/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

/* xutil_memtest prototypes */

int XUtil_MemoryTest32(u32 *Addr, u32 Words, u32 Pattern, u8 Subtest);
int XUtil_MemoryTest16(u16 *Addr, u32 Words, u16 Pattern, u8 Subtest);
int XUtil_MemoryTest8(u8 *Addr, u32 Words, u8 Pattern, u8 Subtest);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
