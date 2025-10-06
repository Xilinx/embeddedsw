/******************************************************************************
* Copyright (c) 2023 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023-2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xtrngpsx_hw.h
* This file contains trng core hardware definitions of VersalNet.
* @addtogroup Overview
* @{
*
* This header file contains identifiers and register-level core macros that can be
* used to access the True Random Number Generator core.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kpt  01/04/23 Initial release
* 1.5   ank  09/26/25 Fixed MISRA-C Violations
*
* </pre>
*
* @endcond
******************************************************************************/
#ifndef __XTRNPSX_HW_H_
#define __XTRNPSX_HW_H_

#ifdef __cplusplus
extern "C" {
#endif

/************************** Constant Definitions ****************************/

/**
 * Register: TRNG_STATUS
 */
#define TRNG_STATUS    (0x00000004U)

/* access_type: ro */
#define TRNG_STATUS_QCNT_SHIFT   9U
#define TRNG_STATUS_QCNT_MASK    0x00000e00U

/* access_type: ro */
#define TRNG_STATUS_CERTF_MASK    0x00000008U

/* access_type: ro */
#define TRNG_STATUS_DTF_MASK    0x00000002U

/* access_type: ro */
#define TRNG_STATUS_DONE_MASK    0x00000001U

/**
 * Register: TRNG_CTRL
 */
#define TRNG_CTRL    (0x00000008U)

/* access_type: rw */
#define TRNG_CTRL_PERSODISABLE_MASK    0x00000400U
#define TRNG_CTRL_PERSODISABLE_DEFVAL  0x0U

/* access_type: rw */
#define TRNG_CTRL_SINGLEGENMODE_MASK    0x00000200U

/* access_type: rw */
#define TRNG_CTRL_EUMODE_MASK    0x00000100U

/* access_type: rw */
#define TRNG_CTRL_PRNGMODE_MASK    0x00000080U

/* access_type: rw */
#define TRNG_CTRL_TSTMODE_MASK    0x00000040U

/* access_type: rw */
#define TRNG_CTRL_PRNGSTART_MASK    0x00000020U

/* access_type: rw */
#define TRNG_CTRL_PRNGXS_MASK    0x00000008U

/* access_type: rw */
#define TRNG_CTRL_TRSSEN_MASK    0x00000004U

/* access_type: rw */
#define TRNG_CTRL_PRNGSRST_MASK    0x00000001U

/**
 * Register: TRNG_CTRL_2
 */
#define TRNG_CTRL_2    (0x0000000CU)

/* access_type: wo */
#define TRNG_CTRL_2_REPCOUNTTESTCUTOFF_SHIFT   8U
#define TRNG_CTRL_2_REPCOUNTTESTCUTOFF_MASK    0x0001ff00U

/* access_type: wo */
#define TRNG_CTRL_2_DIT_SHIFT   0U
#define TRNG_CTRL_2_DIT_MASK    0x0000001fU
#define TRNG_CTRL_2_DIT_DEFVAL  0xcU

/**
 * Register: TRNG_CTRL_3
 */
#define TRNG_CTRL_3    (0x00000010U)

/* access_type: wo */
#define TRNG_CTRL_3_ADAPTPROPTESTCUTOFF_SHIFT   8U
#define TRNG_CTRL_3_ADAPTPROPTESTCUTOFF_MASK    0x0003ff00U

/* access_type: wo */
#define TRNG_CTRL_3_DLEN_SHIFT   0U
#define TRNG_CTRL_3_DLEN_MASK    0x000000ffU

/**
 * Register: TRNG_CTRL_4
 */
#define TRNG_CTRL_4    (0x00000014U)

/**
 * Register: TRNG_PER_STRNG_11
 */
#define TRNG_PER_STRNG_11    (0x000000ACU)

/**
 * Register: TRNG_CORE_OUTPUT
 */
#define TRNG_CORE_OUTPUT    (0x000000C0U)
/**
 * Register: TRNG_RESET
 */
#define TRNG_RESET    (0x000000D0U)
#define TRNG_RESET_DEFVAL   0x1U

/* access_type: rw */
#define TRNG_RESET_VAL_MASK    0x00000001U

/**
 * Register: TRNG_OSC_EN
 */
#define TRNG_OSC_EN    (0x000000D4U)

/* access_type: rw */
#define TRNG_OSC_EN_VAL_MASK    0x00000001U
#define TRNG_OSC_EN_VAL_DEFVAL  0x0U

#ifdef __cplusplus
}
#endif

#endif /* __XTRNGPSX_HW_H_ */
