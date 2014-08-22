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
*       (c) Copyright 2008 Xilinx Inc.
*       All rights reserved.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file ct.h
*
* Code test (CT) utility driver.
*
* This package is intented to help designers test their Xilinx device drivers
* in unit and integration testing environments. The macros and functions
* provide the following services:
*
*   - Automated data comparison
*   - Xilinx driver assert testing
*   - Interrupt utilities
*   - Error & message logging
*   - Test sequencing utilities
*   - Timing utilities
*
* The main benefit of this package is to simplify test code and make it easier
* for the tester to get more coverage in their integration and unit testing
* without having to replicate common testing constructs.
*
* <b>Integrating CT Source code</b>
*
* This package consists of this header file and an implementation file. The
* implementation can be operating system specific. When including this package
* in your test, use this file and one of the CT implementation files such as
* ct_vxworks.c.
*
* There are GNU specific "C" extensions used in this package and as a result
* make it non-ANSI compliant. The tester must compile this package and their
* other test code with the GNU compiler suite.
*
* The CT package requires that there be available standard header files
* stdio.h, string.h, and stdarg.h.
*
*
* <b>Setup and Test Sequencing</b>
*
* Before calling any comparison or utility function in this package, the tester
* should call CT_Init() to initialize the library. This function only needs to
* be called once.
*
* During tests, your code may be segmented into subtests where you would want
* separate out the results. At the end of all subtests, you may want to
* summarize all the tests run. The following pseudo code shows how this is done
* with CT sequencing function calls:
*
* <pre>
*
*    CT_Init();
*
*    // Subtest #1
*    CT_TestReset("This is my Subtest #1");
*      ...
*      // Subtest code
*      ...
*    Failures = CT_GetTestFailures();
*    CT_Message("Subtest #1 had %d failures\n", Failures);
*
*    // Subtest #2
*    CT_TestReset("This is my Subtest #2");
*      ...
*      // Subtest code
*      ...
*    Failures = CT_GetTestFailures();
*    CT_Message("Subtest #2 had %d failures\n", Failures);
*
*    Failures = CT_GetTotalFailures();
*    CT_Message("Total test failures = %d\n", Failures);
*
* </pre>
*
* <b>General Usage</b>
*
* The heart of this package utilizes macros to compare variables or memory
* areas. For example,
*
* <pre>
*
*     CT_CMP_NUM(int, ActualValue, ExpectedValue);
*
* </pre>
*
* compares two values of type int. If they are not equal, then an error message
* is logged and an internal counter is incremented. If they are equal, then the
* test proceeds as if nothing had occurred. Other examples:
*
* <pre>
*
*     CT_CMP_NUM(int, *ActualPtr, 5);
*     CT_CMP_NUM(int*, ActualPtr, 0x20002100);
*
* </pre>
*
* With each failure, a descriptive message is printed along with the line
* number of the invoking code.
*
* If the tester needs to make a comparison manually, then they can use the
* CT_LOG_FAILURE() macro to note the error in case the comparison is negative.
* If the tester needs to emit an informational message, then they can use the
* CT_Mesage() function. Calling this function does not increment error
* counters.
*
*
* <b>Message Logging</b>
*
* This package uses the printf() library for message logging.
*
* Ideally, the tester's environment should include some sort of UART. By
* default CT utilizes a pre configured console UART.
*
* If your system does not contain a UART, then another method of providing
* results includes printf'ing messages to memory. To enable this method,
* enable the definition of IO_USE_BUFFER in this file. To further tailor this
* method of logging, change IO_BUF_SIZE and IO_BUF_CUSHION. All output will
* be written to the TextBuf array. Use your debugger or your own test
* utilities to examine this buffer for generated output.
*
*
* <b>Limitations</b>
*
*  - This package must be compiled under the GNU compiler suite.
*  - CT_CMP macros can compare only ordinal data types. Structure type
*    comparisons require the tester to implement their own function.
*  - Some sort of BSP is required to support implementation of string.h,
*    stdio.h, and stdarg.h at a minimum.
*  - Advanced BSP support of signal.h is required to use CT_SetAlarm().
*    If support is not available, then this function will return an error.
*  - Testing asserts requires that NDEBUG not be defined in your driver under
*    test code.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who  Date     Changes
* --- ---- -------- -----------------------------------------------
*  1  rmm  12/23/03 First release for VxWorks
* </pre>
*
******************************************************************************/

#ifndef CT_H
#define CT_H

/***************************** Include Files *********************************/

#include "xil_types.h"  /* Needed for assert testing */
#include "xil_assert.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/************************** Constant Definitions *****************************/
/*
 * This is what the CT_Log* lines will be prepended with. The numerical arg
 * should be the line number within the source file containing the call to
 * the CT library function or macro.
 */
#define CT_ERR_FMT "FAIL: %04d: "

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
 *
 * Log a failure with a supplied message. This provides a useful wrapper
 * around the CT_LogFailure function that prints messages of the same format
 * as other CT macros:
 *
 * <pre>
 *   1  int i;
 *   2
 *   3  i = 10;
 *   4  if (i != 5)
 *   5  {
 *   6     CT_LOG_FAILURE("GetDataFromDevice() returned %d instead of 5", i);
 *   7     CT_LOG_FAILURE("D'oh");
 *   8  }
 *
 * yields the output:
 *
 *     FAIL: 0006: GetDataFromDevice() returned 10 instead of 5
 *     FAIL: 0007: D'oh
 * </pre>
 *
 * @param	fmt is a printf style format string
 * @param 	args is a variable argument list that matches fields in the
 *        	fmt string.
 * @note
 *   		Usage: CT_LOG_FAILURE(char* fmt, ...)
 *
 *****************************************************************************/
#define CT_LOG_FAILURE(fmt, args...) \
    CT_LogFailure(CT_ERR_FMT fmt "\n", __LINE__ , ## args)

/*****************************************************************************/
/**
 *
 * Compare numbers. If not equal, then output message and increment fail
 * counter. The message contains the line number of the failure, the
 * name of the variable, and its actual and expected values in hex and
 * decimal. An example:
 *
 * <pre>
 *     1  UINT32 result = 5;
 *     2  UINT32 expected = 17;
 *     3
 *     4  CT_CMP_NUM(UINT32, result, expected)
 *
 * yields the output:
 *
 *     FAIL: 0004: result=5(5h), expected 17(11h)
 *
 * </pre>
 *
 * @param	type is data type to compare (must be an ordinal type such as
 *		int)
 * @param	actual is the actual data retrieved from test
 * @param	expected is the expected value
 *
 * @note	Usage: CT_CMP_NUM(<type>, actual, expected)
 *
 ****************************************************************************/
#define CT_CMP_NUM(type, actual, expected)                           \
    if ((type)(actual) != (type)(expected))                          \
    {                                                                \
        CT_LogFailure(CT_ERR_FMT "%s=%d(%Xh), expected %d(%Xh)\n",   \
                      __LINE__, #actual, (int)actual, (int)actual,   \
                     (int)expected, (int)expected);                  \
    }

/************************** Function Prototypes ******************************/

void CT_Init(void);
void CT_TestReset(char *fmt, ...);
void CT_NotifyNextPass(void);
void CT_LogFailure(char* fmt, ...);
unsigned CT_GetTestFailures(void);
unsigned CT_GetTotalFailures(void);
void CT_Message(char *fmt, ...);
void CT_MemSet(void *Address, char Value, unsigned Bytes);
int CT_IsAssertDisabled(void);

void CT_VerboseOn(void);
void CT_VerboseOff(void);

int CT_GetUserInput(char* Prompt, char* Response, int MaxChars);
extern char inbyte(void);     /* Implemented by standalone BSP */
extern void outbyte(char);   /* Implemented by standalone BSP */

#endif  /* CT_H */
