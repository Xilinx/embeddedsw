/******************************************************************************
* Copyright (c) 2023 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023-2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xtrngpsx_hw.h
* This file contains trng core hardware definitions of VersalNet.
* @addtogroup trngpsx_api TRNGPSX APIs
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
* 1.6   hae  01/06/26 Fixed doxygen warnings
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
#define TRNG_STATUS    (0x00000004U) /**< TRNG Status register */

/* access_type: ro */
#define TRNG_STATUS_QCNT_SHIFT   9U /**< Queue count field shift */
#define TRNG_STATUS_QCNT_MASK    0x00000e00U /**< Queue count field mask */

/* access_type: ro */
#define TRNG_STATUS_CERTF_MASK    0x00000008U /**< Catastrophic error test failure mask */

/* access_type: ro */
#define TRNG_STATUS_DTF_MASK    0x00000002U /**< Deterministic test failure mask */

/* access_type: ro */
#define TRNG_STATUS_DONE_MASK    0x00000001U /**< Operation done mask */

/**
 * Register: TRNG_CTRL
 */
#define TRNG_CTRL    (0x00000008U) /**< TRNG Control register */

/* access_type: rw */
#define TRNG_CTRL_PERSODISABLE_MASK    0x00000400U /**< Personalization string disable mask */
#define TRNG_CTRL_PERSODISABLE_DEFVAL  0x0U /**< Personalization string disable default value */

/* access_type: rw */
#define TRNG_CTRL_SINGLEGENMODE_MASK    0x00000200U /**< Single generation mode mask */

/* access_type: rw */
#define TRNG_CTRL_EUMODE_MASK    0x00000100U /**< Entropy unit mode mask */

/* access_type: rw */
#define TRNG_CTRL_PRNGMODE_MASK    0x00000080U /**< PRNG mode mask */

/* access_type: rw */
#define TRNG_CTRL_TSTMODE_MASK    0x00000040U /**< Test mode mask */

/* access_type: rw */
#define TRNG_CTRL_PRNGSTART_MASK    0x00000020U /**< PRNG start mask */

/* access_type: rw */
#define TRNG_CTRL_PRNGXS_MASK    0x00000008U /**< PRNG external seed mask */

/* access_type: rw */
#define TRNG_CTRL_TRSSEN_MASK    0x00000004U /**< True random seed source enable mask */

/* access_type: rw */
#define TRNG_CTRL_PRNGSRST_MASK    0x00000001U /**< PRNG soft reset mask */

/**
 * Register: TRNG_CTRL_2
 */
#define TRNG_CTRL_2    (0x0000000CU) /**< TRNG Control 2 register */

/* access_type: wo */
#define TRNG_CTRL_2_REPCOUNTTESTCUTOFF_SHIFT   8U /**< Repetition count test cutoff shift */
#define TRNG_CTRL_2_REPCOUNTTESTCUTOFF_MASK    0x0001ff00U /**< Repetition count test cutoff mask */

/* access_type: wo */
#define TRNG_CTRL_2_DIT_SHIFT   0U /**< Digitization interval time shift */
#define TRNG_CTRL_2_DIT_MASK    0x0000001fU /**< Digitization interval time mask */
#define TRNG_CTRL_2_DIT_DEFVAL  0xcU /**< Digitization interval time default value */

/**
 * Register: TRNG_CTRL_3
 */
#define TRNG_CTRL_3    (0x00000010U) /**< TRNG Control 3 register */

/* access_type: wo */
#define TRNG_CTRL_3_ADAPTPROPTESTCUTOFF_SHIFT   8U /**< Adaptive proportion test cutoff shift */
#define TRNG_CTRL_3_ADAPTPROPTESTCUTOFF_MASK    0x0003ff00U /**< Adaptive proportion test cutoff mask */

/* access_type: wo */
#define TRNG_CTRL_3_DLEN_SHIFT   0U /**< Derivation function length shift */
#define TRNG_CTRL_3_DLEN_MASK    0x000000ffU /**< Derivation function length mask */

/**
 * Register: TRNG_CTRL_4
 */
#define TRNG_CTRL_4    (0x00000014U) /**< TRNG Control 4 register */

/**
 * Register: TRNG_PER_STRNG_11
 */
#define TRNG_PER_STRNG_11    (0x000000ACU) /**< TRNG Personalization String register */

/**
 * Register: TRNG_CORE_OUTPUT
 */
#define TRNG_CORE_OUTPUT    (0x000000C0U) /**< TRNG Core Output register */
/**
 * Register: TRNG_RESET
 */
#define TRNG_RESET    (0x000000D0U) /**< TRNG Reset register */
#define TRNG_RESET_DEFVAL   0x1U /**< TRNG reset default value */

/* access_type: rw */
#define TRNG_RESET_VAL_MASK    0x00000001U /**< TRNG reset value mask */

/**
 * Register: TRNG_OSC_EN
 */
#define TRNG_OSC_EN    (0x000000D4U) /**< TRNG Oscillator Enable register */

/* access_type: rw */
#define TRNG_OSC_EN_VAL_MASK    0x00000001U /**< Oscillator enable value mask */
#define TRNG_OSC_EN_VAL_DEFVAL  0x0U /**< Oscillator enable default value */

#ifdef __cplusplus
}
#endif
/** @} */
#endif /* __XTRNGPSX_HW_H_ */
