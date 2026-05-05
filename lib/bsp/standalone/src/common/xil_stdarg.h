/******************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
* @file xil_stdarg.h
*
* @brief Compiler-portable support for variable argument handling.
*
* This header provides a lightweight abstraction for variadic argument
* handling in the standalone BSP. For GCC/Clang-compatible toolchains,
* variadic support is implemented using compiler builtins (__builtin_va_*).
* For other toolchains (such as IAR and ARMCC), it falls back to the
* standard <stdarg.h> implementation.
*
* This approach ensures portability across different compilers while
* maintaining efficient handling of variadic arguments according to
* the underlying ABI.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 9.5   vmt  04/30/26 Initial version
* </pre>
*
******************************************************************************/
#ifndef XIL_STDARG_H
#define XIL_STDARG_H

#if defined (__GNUC__)

/**
* @brief Type representing a variable argument list.
*
* Maps to the compiler's internal representation of variadic arguments.
*/
typedef __builtin_va_list va_list;

/**
* @brief Initialize a variable argument list.
*
* @param ap   Variable of type va_list to initialize.
* @param last The last named parameter before the variadic arguments.
*
*/
#ifndef va_start
#define va_start(ap, last) __builtin_va_start(ap, last)
#endif

/**
* @brief Retrieve the next argument from a variable argument list.
*
* @param ap   Variable argument list previously initialized by va_start().
* @param type Type of the argument to retrieve.
*
* @return The next argument in the list, cast to @p type.
*/
#ifndef va_arg
#define va_arg(ap, type)   __builtin_va_arg(ap, type)
#endif

/**
* @brief End traversal of the variadic argument list.
*
* @param ap The va_list object to clean up.
*/
#ifndef va_end
#define va_end(ap)         __builtin_va_end(ap)
#endif

/**
* @brief Copy the state of one va_list to another.
*
* @param dest The destination va_list.
* @param src  The source va_list.
*/
#ifndef va_copy
#define va_copy(dest, src) __builtin_va_copy(dest, src)
#endif

#else

/* Fallback to toolchain-provided stdarg implementation */
#include <stdarg.h>

#endif /* __GNUC__ */

#endif /* XIL_STDARG_H */
