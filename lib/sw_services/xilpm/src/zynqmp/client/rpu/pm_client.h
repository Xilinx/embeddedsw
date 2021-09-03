/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*
 * CONTENT
 * File is specific for each PU instance and must exist in order to
 * port Power Management code for some new PU.
 * Contains PU specific macros and macros to be defined depending on
 * the execution environment.
 */

#ifndef PM_CLIENT_H
#define PM_CLIENT_H

#include <xil_exception.h>
#include <xil_io.h>
#include "pm_rpu.h"
#include "pm_defs.h"
#include "pm_common.h"

#ifdef __cplusplus
extern "C" {
#endif


/** @cond xilpm_internal */
const char* XPm_GetMasterName(void);
#ifdef DEBUG_MODE
#if defined (__GNUC__)

#define pm_print(MSG, ...)	xil_printf("%s: "MSG, \
										XPm_GetMasterName(), ##__VA_ARGS__)

#elif defined (__ICCARM__)

#define pm_print	xil_printf

#endif
#else
#define pm_print(...)	{}
#endif


/** @endcond */

#ifdef __cplusplus
}
#endif

#endif /* PM_CLIENT_H */
