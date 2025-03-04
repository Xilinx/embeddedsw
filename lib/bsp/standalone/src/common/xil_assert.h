/******************************************************************************
* Copyright (c) 2009 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xil_assert.h
*
* @addtogroup common_assert_apis Assert APIs and Macros
*
* The xil_assert.h file contains assert related functions and macros.
* Assert APIs/Macros specifies that a application program satisfies certain
* conditions at particular points in its execution. These function can be
* used by application programs to ensure that, application code is satisfying
* certain conditions.
*
* @{
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date   Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a hbm  07/14/09 First release
* 6.0   kvn  05/31/16 Make Xil_AsserWait a global variable
* 9.2   adk  08/05/24 Define __FILENAME__ macro in cmake which points to
* 		      just name of file instead of full file path, replace
* 		      __FILE__ with __FILENAME__ in assert APIs.
* 9.3   vmt  03/03/25 Fixed compilation warning of strrchr
*                     [-Wbuiltin-declaration-mismatch]
* </pre>
*
******************************************************************************/

/**
 *@cond nocomments
 */

#ifndef XIL_ASSERT_H	/* prevent circular inclusions */
#define XIL_ASSERT_H	/* by using protection macros */

#include "xil_types.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


/***************************** Include Files *********************************/


/************************** Constant Definitions *****************************/
#if ! defined(SDT)
#define __FILENAME__	__FILE__
#else
#define __FILENAME__	(strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__))
#endif

#define XIL_ASSERT_NONE     0U
#define XIL_ASSERT_OCCURRED 1U
#define XNULL NULL

extern u32 Xil_AssertStatus;
extern s32 Xil_AssertWait;
extern void Xil_Assert(const char8 *File, s32 Line);
/**
 *@endcond
 */
void XNullHandler(void *NullParameter);

/**
 * This data type defines a callback to be invoked when an
 * assert occurs. The callback is invoked only when asserts are enabled
 */
typedef void (*Xil_AssertCallback) (const char8 *File, s32 Line);

/***************** Macros (Inline Functions) Definitions *********************/

#ifndef NDEBUG

/*****************************************************************************/
/**
* @brief    This assert macro is to be used for void functions. This in
*           conjunction with the Xil_AssertWait boolean can be used to
*           accommodate tests so that asserts which fail allow execution to
*           continue.
*
* @param    Expression: expression to be evaluated. If it evaluates to
*           false, the assert occurs.
*
* @return   Returns void unless the Xil_AssertWait variable is true, in which
*           case no return is made and an infinite loop is entered.
*
******************************************************************************/
#define Xil_AssertVoid(Expression)                \
{                                                  \
    if (Expression) {                              \
        Xil_AssertStatus = XIL_ASSERT_NONE;       \
    } else {                                       \
        Xil_Assert(__FILENAME__, __LINE__);            \
        Xil_AssertStatus = XIL_ASSERT_OCCURRED;   \
        return;                                    \
    }                                              \
}

/*****************************************************************************/
/**
* @brief    This assert macro is to be used for functions that do return a
*           value. This in conjunction with the Xil_AssertWait boolean can be
*           used to accommodate tests so that asserts which fail allow execution
*           to continue.
*
* @param    Expression: expression to be evaluated. If it evaluates to false,
*           the assert occurs.
*
* @return   Returns 0 unless the Xil_AssertWait variable is true, in which
* 	        case no return is made and an infinite loop is entered.
*
******************************************************************************/
#define Xil_AssertNonvoid(Expression)             \
{                                                  \
    if (Expression) {                              \
        Xil_AssertStatus = XIL_ASSERT_NONE;       \
    } else {                                       \
        Xil_Assert(__FILENAME__, __LINE__);            \
        Xil_AssertStatus = XIL_ASSERT_OCCURRED;   \
        return 0;                                  \
    }                                              \
}

/*****************************************************************************/
/**
* @brief     Always assert. This assert macro is to be used for void functions.
*            Use for instances where an assert should always occur.
*
* @return    Returns void unless the Xil_AssertWait variable is true, in which
*	         case no return is made and an infinite loop is entered.
*
******************************************************************************/
#define Xil_AssertVoidAlways()                   \
{                                                  \
   Xil_Assert(__FILENAME__, __LINE__);                 \
   Xil_AssertStatus = XIL_ASSERT_OCCURRED;        \
   return;                                         \
}

/*****************************************************************************/
/**
* @brief   Always assert. This assert macro is to be used for functions that
*          do return a value. Use for instances where an assert should always
*          occur.
*
* @return Returns void unless the Xil_AssertWait variable is true, in which
*	      case no return is made and an infinite loop is entered.
*
******************************************************************************/
#define Xil_AssertNonvoidAlways()                \
{                                                  \
   Xil_Assert(__FILENAME__, __LINE__);                 \
   Xil_AssertStatus = XIL_ASSERT_OCCURRED;        \
   return 0;                                       \
}


#else

#define Xil_AssertVoid(Expression)
#define Xil_AssertVoidAlways()
#define Xil_AssertNonvoid(Expression)
#define Xil_AssertNonvoidAlways()

#endif

/************************** Function Prototypes ******************************/

void Xil_AssertSetCallback(Xil_AssertCallback Routine);

#ifdef __cplusplus
}
#endif

#endif	/* end of protection macro */
/**
* @} End of "addtogroup common_assert_apis".
*/
