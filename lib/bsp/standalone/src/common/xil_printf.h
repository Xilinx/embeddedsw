/******************************************************************************
* Copyright (C) 1995 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/**
* @file xil_printf.h
* @brief Lightweight printf interface for embedded systems.
*/

 #ifndef XIL_PRINTF_H
 #define XIL_PRINTF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include "xil_types.h"
#include "bspconfig.h"
#include "xil_stdarg.h"
#ifndef SDT
#include "xparameters.h"
#endif
#if defined (__aarch64__) && HYP_GUEST && EL1_NONSECURE && XEN_USE_PV_CONSOLE
#include "xen_console.h"
#endif

/*----------------------------------------------------*/
/* Use the following parameter passing structure to   */
/* make xil_printf re-entrant.                        */
/*----------------------------------------------------*/

struct params_s;


/**< The purpose of this routine is to output data the */
/* same as the standard printf function without the  */
/* overhead most run-time libraries involve. Usually */
/* the printf brings in many kilobytes of code and   */
/* that is unacceptable in most embedded systems.    */

typedef char8* charptr;
typedef s32 (*func_ptr)(int c); /**< Function pointer type for character output callbacks. */

/************************** Function Prototypes ******************************/
/**
 * @brief Print a formatted string to the console (subset of printf).
*/
void xil_printf( const char8 *ctrl1, ...);
/**
 * @brief Print a formatted string using a va_list (subset of vprintf).
 * @param[in] ctrl1 Format string controlling the output.
 * @param[in] argp  Variable argument list matching the format string.
 */
void xil_vprintf(const char8 *ctrl1, va_list argp);
/**
 * @brief Print a raw string to the console.
 */
void print( const char8 *ptr);
/**
 * @brief Send a single byte to the console output.
 * @param[in] c Character to send.
 */
extern void outbyte (char c);
/**
 * @brief Receive a single byte from the console input.
 * @return Character received from the console.
 */
extern char inbyte(void);

#ifdef __cplusplus
}
#endif

#endif	/* end of protection macro */
