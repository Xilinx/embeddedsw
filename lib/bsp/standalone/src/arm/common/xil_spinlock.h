/******************************************************************************
* Copyright (C) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xil_spinlock.h
*
* This header file contains function prototypes to be used while using Xilinx
* spinlocking mechanism.
* Please refer to file header contents of xil_spinlock.c to understand in
* detail the spinlocking mechanism.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who   Date     Changes
* ----- ----- -------- -----------------------------------------------
* 7.5 	asa   02/16/21 First release
* 7.6	sk	  08/05/21 Add Boolean check and braces for Xil_IsSpinLockEnabled
* 			           if condition to fix misrac violations.
* 7.7	sk	  01/10/22 Update XIL_SPINLOCK_ENABLED from signed to unsigned to
* 			           fix misra_c_2012_rule_10_4 violation.
* 9.0   ml    03/03/23 Add description to fix doxygen warnings.
* 9.3   asa   03/04/25 Add support for Cortex-A* and clang.
* 9.6   ml   10/01/26 Fix doxygen 1.15.0 and compliance checker warnings
* </pre>
*
******************************************************************************/

#ifndef XIL_SPINLOCK_H
#define XIL_SPINLOCK_H

/***************************** Include Files ********************************/
#include "xil_types.h"
#include "xstatus.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined(__GNUC__)
/************************** Function Prototypes *****************************/
u32 Xil_SpinLock(void);
u32 Xil_SpinUnlock(void);
u32 Xil_InitializeSpinLock(UINTPTR lockaddr, UINTPTR lockflagaddr,
		                                                    u32 lockflag);
void Xil_ReleaseSpinLock(void);
u32 Xil_IsSpinLockEnabled(void);

/************************** MACRO Definitions ****************************/
#define XIL_SPINLOCK_LOCKVAL	0x10203040  /**< Magic value indicating spinlock is held. */
#define XIL_SPINLOCK_RESETVAL	0x40302010  /**< Magic value indicating spinlock is released. */
#define XIL_SPINLOCK_ENABLE     0x17273747  /**< Value written to enable spinlock feature. */
#define XIL_SPINLOCK_ENABLED	0x17273747U /**< Unsigned check value for enabled spinlock state. */
/***************** Macros (Inline Functions) Definitions ********************/

#endif  /* (__GNUC__) */
/***************************************************************************/
/** @brief Acquire the spinlock to protect shared register access across CPUs. */
#if defined(__GNUC__)
#define XIL_SPINLOCK()              \
    if(Xil_IsSpinLockEnabled()!=(u32)0) {    \
        Xil_SpinLock();  }
#else
#define XIL_SPINLOCK()
#endif /* (__GNUC__) */

/** @brief Release the spinlock previously taken. */
#if defined(__GNUC__)
#define XIL_SPINUNLOCK()              \
    if(Xil_IsSpinLockEnabled()!=(u32)0) {      \
        Xil_SpinUnlock();  }
#else
#define XIL_SPINUNLOCK()
#endif /* (__GNUC__) */


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* XIL_SPINLOCK_H */
