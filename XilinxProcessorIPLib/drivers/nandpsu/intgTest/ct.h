/******************************************************************************
*
* (c) Copyright 2010-14 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file ct.h
*
* Code test (CT) utility driver.
*
* This package is intended to help designers test their Xilinx device drivers
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
* make it non-ANSI compliment. The tester must compile this package and their
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
* During tests, your code may be segmented into sub tests where you would want
* separate out the results. At the end of all sub tests, you may want to
summarize
* all the tests run. The following pseudo code shows how this is done with CT
* sequencing function calls:
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
*    CT_Message("Subtest #1 had %d failures\r\n", Failures);
*
*    // Subtest #2
*    CT_TestReset("This is my Subtest #2");
*      ...
*      // Subtest code
*      ...
*    Failures = CT_GetTestFailures();
*    CT_Message("Subtest #2 had %d failures\r\n", Failures);
*
*    Failures = CT_GetTotalFailures();
*    CT_Message("Total test failures = %d\r\n", Failures);
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
* With each failure, a descriptive message is printed along with the line number
* of the invoking code.
*
* If the tester needs to make a comparison manually, then they can use the
* CT_LOG_FAILURE() macro to note the error in case the comparison is negative.
* If the tester needs to emit an informational message, then they can use the
* CT_Mesage() function. Calling this function does not increment error counters.
*
*
* <b>Message Logging</b>
*
* This package uses the printf() library for message logging.
*
* Ideally, the tester's environment should include some sort of UART. By
* default CT utilizes a pre-configured console UART.
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
*
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
#include "intg.h"

/************************** Constant Definitions *****************************/


/**
 *  This is what the CT_Log* lines will be prefixed with. The numerical arg
 * should be the line number within the source file containing the call to
 * the CT library function or macro.
 */
#define CT_ERR_FMT "        FAIL: %s: %04d:"


/**************************** Type Definitions *******************************/

typedef unsigned long long CT_TIME_STAMP;	/**< Type defination for Time Stamp.*/

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
 * </pre>
 *
 * yields the output:
 *
 *     FAIL: 0006: GetDataFromDevice() returned 10 instead of 5
 *     FAIL: 0007: D'oh
 *
 * @param fmt is a printf style format string
 * @param args is a variable argument list that matches fields in the fmt
 *        string.
 *
 * @note
 *   Usage: CT_LOG_FAILURE(char* fmt, ...)
 *****************************************************************************/
#define CT_LOG_FAILURE(fmt, args...) \
    CT_LogFailure(CT_ERR_FMT fmt "\r\n", __FILE__, __LINE__ , ## args)

/* Note: In the preceding construct, the space between __FILE__, __LINE__ and
 * the comma are needed for the ## args to work when there is no "args"
 * expansion like in line 7 of the example shown above.
 */

/*****************************************************************************/
/**
 *
 * Test asserts for a function. This macro will verify whether the called
 * function generates an assert from the provided arguments. If it does not
 * then a failure message is logged.
 *
 * The design of this macro allows the tester to verify an assert occurs
 * with a single line of code with full type checking of the called
 * function. For example, if your function under test is coded as:
 *
 * <pre>
 *   void XDev_Start(Dev* Instance)
 *   {
 *      Xil_AssertVoid(Dev != NULL);
 *          ...
 *   }
 * </pre>
 *
 * Then use CT_ASSERT in the following way to verify that the assert is caught.
 *
 * <pre>
 *   CT_ASSERT(XDev_Start, (Dev*)NULL);
 * </pre>
 *
 * @param Function is the function being tested for asserts.
 * @param args is a variable number of arguments. It contains whatever arguments
 *        should cause an assert. These arguments are arranged by the
 *        preprocessor in the order provided by the tester.
 *
 * @note
 *   Usage: CT_ASSERT(SomeFunc, arg1, arg2, ...)
 *****************************************************************************/
#define CT_ASSERT(Function, args...) \
{ \
    if (CT_IsAssertDisabled())                                          \
        CT_LogFailure(CT_ERR_FMT "NDEBUG is defined, assert test failed\r\n", \
        __FILE__, __LINE__); 						\
    else                                                                \
    {                                                                   \
        XAssertStatus = XASSERT_NONE;                                   \
        XWaitInAssert = FALSE;                                         \
        (void)Function(args);                                           \
        XWaitInAssert = TRUE;                                          \
        if (XAssertStatus == XASSERT_NONE)                              \
          CT_LogFailure(CT_ERR_FMT "Assert failed\r\n", __FILE__, __LINE__);  \
    }                                                                   \
}

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
 * @param type is data type to compare (must be an ordinal type such as int)
 * @param actual is the actual data retrieved from test
 * @param expected is the expected value
 *
 * @note
 *   Usage: CT_CMP_NUM(<type>, actual, expected)
 ****************************************************************************/
#define CT_CMP_NUM(type, actual, expected)                           \
    if ((type)(actual) != (type)(expected))                          \
    {                                                                \
        printf(CT_ERR_FMT "%s=%d(%Xh), expected %d(%Xh)\r\n",   \
                   __FILE__, __LINE__, #actual, (int)actual, (int)actual,   \
                  (int)expected, (int)expected); \
        CT_LogFailure2(); \
    }

/*****************************************************************************/
/**
 *
 * Like CT_CMP_NUM except an extra argument is printed. Helpful if comparing
 * numbers in a loop where the loop index may prove useful if the test fails.
 *
 * <pre>
 *     1  UINT32 result[5];
 *     2  UINT32 expected[5];
 *     3  int i;
 *     4
 *     5  for (i=0; i<5; i++)
 *     6  {
 *     7     CT_CMP_NUM_ARG(UINT32, result[i], expected[i], i);
 *     8  }
 *     9
 *
 * yeilds the output if for example failure occurs at i=3:
 *
 *     FAIL: 0007: result=5(5h), expected 17(11h). i=3
 * </pre>
 *
 * @param type is data type to compare (must be an ordinal type such as int)
 * @param actual is the actual data retrieved from test
 * @param expected is the expected value
 * @param arg is an argument that can be treated as an integer.
 *
 * @note
 *   Usage: CT_CMP_NUM_ARG(<type>, actual, expected, arg1)
 ****************************************************************************/
#define CT_CMP_NUM_ARG(type, actual, expected, arg)                         \
    if ((type)(actual) != (type)(expected))                                 \
    {                                                                       \
        CT_LogFailure(CT_ERR_FMT "%s=%d(%Xh), expected %d(%Xh). %s=%d\r\n",  \
                 __FILE__, __LINE__, #actual, (int)actual, (int)actual,      \
                      (int)expected, (int)expected, #arg, arg);             \
    }

/*****************************************************************************/
/**
 *
 * Like CT_CMP_NUM_ARG except a second extra argument is printed. Helpful if
 * comparing numbers in a loop where the loop index may prove useful if the test
 * fails.
 *
 * <pre>
 *     1  UINT32 result[5];
 *     2  UINT32 expected[5];
 *     3  int i, j=(int)&result[0];
 *     4
 *     5  for (i=0; i<5; i++, j++)
 *     6  {
 *     7     CT_CMP_NUM_ARG2(UINT32, result[i], expected[i], i, j);
 *     8  }
 *     9
 *
 * yeilds the output if for example failure occurs at i=3:
 *
 *     FAIL: 0007: result=5(5h), expected 17(11h). i=3, j=543234
 * </pre>
 *
 * @param type is data type to compare (must be an ordinal type such as int)
 * @param actual is the actual data retrieved from test
 * @param expected is the expected value
 * @param arg1 is an argument that can be treated as an integer.
 * @param arg2 is an argument that can be treated as an integer.
 *
 * @note
 *   Usage: CT_CMP_NUM_ARG2(<type>, actual, expected, arg1, arg2)
 ****************************************************************************/
#define CT_CMP_NUM_ARG2(type, actual, expected, arg1, arg2)   \
    if ((type)(actual) != (type)(expected))                   \
    {                                                                    \
        CT_LogFailure(CT_ERR_FMT "%s=%d(%Xh), expected %d(%Xh). %s=%d. \
		       %s=%d\r\n",\
                    __FILE__, __LINE__, #actual, (int)actual, (int)actual,\
                    (int)expected, (int)expected, #arg1, arg1, #arg2, arg2);\
    }

/*****************************************************************************/
/**
 *
 * Like CT_CMP_NUM except a range of memory is compared.
 *
 * @param type is data type to compare (must be an ordinal type such as int)
 * @param actual is the actual data retrieved from test and must be an array
 *        of type
 * @param bytes is the number of bytes to test
 * @param expected is the expected value and must be of type.
 *
 * @note
 *   Usage: CT_CMP_NUM_RANGE(<type>, <type> actual[], bytes, expected)
 ****************************************************************************/
#define CT_CMP_NUM_RANGE(type, actual, bytes, expected)                      \
{                                                                            \
    int d112a;                                                               \
    int words = (bytes)/sizeof(type);                                        \
    type *a = (type*)(actual);                                               \
                                                                             \
    for (d112a=0; d112a<words; d112a++)                                      \
    {                                                                        \
        if (a[d112a] != expected)                                            \
        {                                                                    \
            CT_LogFailure(CT_ERR_FMT "%s[%d]=%d(%Xh), expected %d(%Xh)\r\n",  \
                           __FILE__, __LINE__, #actual, d112a, a[d112a],      \
                           a[d112a],                                          \
                          expected, expected);                               \
            break;                                                           \
        }                                                                    \
    }                                                                        \
}


/*****************************************************************************/
/**
 *
 * Macro that compare two ranges of memory. If the areas are not equal, then
 * a message is output and the CT_NumFailures counter is incremented. Comparison
 * stops at the first failure.
 *
 * The failure message contains the line number of the failure, the
 * name of the variable, and its actual and expected values in hex and
 * decimal. An example:
 *
 * <pre>
 *     1  UINT32 result[500];
 *     2  UINT32 expected[500];
 *     3
 *     4  CT_CMP_MEM(UINT32, result, expected, 500)
 *
 * yeilds the output if result[13]=100 and expected[13]=121:
 *
 *     FAIL: 0004: result[13]=100(64h), expected 121(79h)
 * </pre>
 *
 * @param type is data type to compare (must be an ordinal type such as int)
 * @param actual is the area of memory to test
 * @param expected is the expected contents
 * @param words is the number of data type "type" to test.
 *
 * @note
 *   Usage: CT_CMP_MEM(<type>, <type> *actual, <type> *expected, unsigned words)
 ****************************************************************************/
#define CT_CMP_MEM(type, actual, expected, words)                            \
{                                                                            \
    int d112a;                                                               \
    type *a = (type*)(actual);                                               \
    type *e = (type*)(expected);                                             \
                                                                             \
    for (d112a=0; d112a<words; d112a++)                                      \
    {                                                                        \
        if (a[d112a] != e[d112a])                                            \
        {                                                                    \
            CT_LogFailure(CT_ERR_FMT "%s[%d]=%d(%Xh), expected %d(%Xh)\r\n", \
                           __FILE__, __LINE__, #actual, d112a, a[d112a],     \
                           a[d112a],     \
                          e[d112a], e[d112a]);                               \
            break;                                                           \
        }                                                                    \
    }                                                                        \
}

/*****************************************************************************/
/**
 *
 * Like CT_CMP_MEM except an extra argument is printed. Helpful if comparing
 * in a loop where the loop index may prove useful if the test fails.
 *
 * <pre>
 *     1  UINT32 result[TEST_WORDS];
 *     2  UINT32 expected[TEST_WORDS];
 *     3  int i;
 *     4
 *     5  for (i=0; i<3; i++)
 *     6  {
 *     7     CT_CMP_MEM_ARG(UINT32, result, expected, TEST_WORDS, i);
 *     8     taskDelay(10);
 *     8  }
 *     9
 *
 * yeilds the output if for example failure occurs at i=1, and result[2]=1,
 * and expected[2]=3:
 *
 *     FAIL: 0007: result[2]=1(1h), expected 3(3h)
 * </pre>
 *
 * @param type is data type to compare (must be an ordinal type such as int)
 * @param actual is the area of memory to test
 * @param expected is the expected contents
 * @param words is the number of data type "type" to test.
 * @param index is an argument that can be treated as an integer.
 *
 * @note
 *    Usage: CT_CM_MEM_ARG(<type>, <type> actual[], <type> expected[], index)
 ****************************************************************************/
#define CT_CMP_MEM_ARG(type, actual, expected, words, index)          \
{                                                                     \
    int d112a;                                                        \
    type *a = (type*)(actual);                                        \
    type *e = (type*)(expected);                                      \
                                                                      \
    for (d112a=0; d112a<words; d112a++)                               \
    {                                                                 \
        if (a[d112a] != e[d112a])                                     \
        {                                                             \
          CT_LogFailure(CT_ERR_FMT "%s[%d]=%d(%Xh), expected %d(%Xh). \
					%s=%d\r\n",\
                          __FILE__, __LINE__, #actual, d112a, a[d112a],\
                          a[d112a], e[d112a],  				\
                          e[d112a], #index, index);                   \
            break;                                                    \
        }                                                             \
    }                                                                 \
}


/*****************************************************************************/
/**
 *
 * Duplicate the data type. Can be used on any declared type.
 *
 * @param type is data type to copy
 * @param dest is destination data pointer
 * @param src is source data pointer
 *
 * @note
 *   Usage: CT_COPY_STRUCT(<type>, <type> *dest, <type> *src)
 ****************************************************************************/
#define CT_COPY_STRUCT(type,dest,src)                                 \
{                                                                     \
    type *td = dest;                                                  \
    type *ts = src;                                                   \
    memcpy(td,ts,sizeof(type));                                       \
}



/************************** Function Prototypes ******************************/

void     CT_Init(void);
void     CT_TestReset(char *fmt, ...);
void     CT_NotifyNextPass(void);
void     CT_LogFailure(char* fmt, ...);
void     CT_LogFailure2();
unsigned CT_GetTestFailures(void);
unsigned CT_GetTotalFailures(void);
void     CT_Message(char *fmt, ...);
void     CT_MemSet(void *Address, char Value, unsigned Bytes);
int      CT_IsAssertDisabled(void);

#endif  /* CT_H */
