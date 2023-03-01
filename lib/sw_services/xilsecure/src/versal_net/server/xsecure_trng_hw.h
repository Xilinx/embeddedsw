/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_trng_hw.h
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
* 5.0   kpt  05/05/22 Initial release
*
* </pre>
*
* @endcond
******************************************************************************/
#ifndef __XSECURE_TRNG_HW_H_
#define __XSECURE_TRNG_HW_H_

#ifdef __cplusplus
extern "C" {
#endif

/************************** Constant Definitions ****************************/
/**
 * TRNG Base Address
 */
#define XSECURE_TRNG_BASEADDR      0xF1230000U

/**
 * Register: TRNG_STATUS
 */
#define XSECURE_TRNG_STATUS    ( ( XSECURE_TRNG_BASEADDR ) + 0x00000004U)

/* access_type: ro */
#define XSECURE_TRNG_STATUS_QCNT_SHIFT   9U
#define XSECURE_TRNG_STATUS_QCNT_MASK    0x00000e00U

/* access_type: ro */
#define XSECURE_TRNG_STATUS_CERTF_MASK    0x00000008U

/* access_type: ro */
#define XSECURE_TRNG_STATUS_DTF_MASK    0x00000002U

/* access_type: ro */
#define XSECURE_TRNG_STATUS_DONE_MASK    0x00000001U

/**
 * Register: TRNG_CTRL
 */
#define XSECURE_TRNG_CTRL    ( ( XSECURE_TRNG_BASEADDR ) + 0x00000008U)

/* access_type: rw */
#define XSECURE_TRNG_CTRL_PERSODISABLE_MASK    0x00000400U
#define XSECURE_TRNG_CTRL_PERSODISABLE_DEFVAL  0x0U

/* access_type: rw */
#define XSECURE_TRNG_CTRL_EUMODE_MASK    0x00000100U

/* access_type: rw */
#define XSECURE_TRNG_CTRL_PRNGMODE_MASK    0x00000080U

/* access_type: rw */
#define XSECURE_TRNG_CTRL_TSTMODE_MASK    0x00000040U

/* access_type: rw */
#define XSECURE_TRNG_CTRL_PRNGSTART_MASK    0x00000020U

/* access_type: rw */
#define XSECURE_TRNG_CTRL_PRNGXS_MASK    0x00000008U

/* access_type: rw */
#define XSECURE_TRNG_CTRL_TRSSEN_MASK    0x00000004U

/* access_type: rw */
#define XSECURE_TRNG_CTRL_PRNGSRST_MASK    0x00000001U

/**
 * Register: TRNG_CTRL_2
 */
#define XSECURE_TRNG_CTRL_2    ( ( XSECURE_TRNG_BASEADDR ) + 0x0000000CU)

/* access_type: wo */
#define XSECURE_TRNG_CTRL_2_REPCOUNTTESTCUTOFF_SHIFT   8U
#define XSECURE_TRNG_CTRL_2_REPCOUNTTESTCUTOFF_MASK    0x0001ff00U
#define XSECURE_TRNG_CTRL_2_REPCOUNTTESTCUTOFF_DEFVAL  0x21U

/* access_type: wo */
#define XSECURE_TRNG_CTRL_2_DIT_SHIFT   0U
#define XSECURE_TRNG_CTRL_2_DIT_MASK    0x0000001fU
#define XSECURE_TRNG_CTRL_2_DIT_DEFVAL  0xcU

/**
 * Register: TRNG_CTRL_3
 */
#define XSECURE_TRNG_CTRL_3    ( ( XSECURE_TRNG_BASEADDR ) + 0x00000010U)

/* access_type: wo */
#define XSECURE_TRNG_CTRL_3_ADAPTPROPTESTCUTOFF_SHIFT   8U
#define XSECURE_TRNG_CTRL_3_ADAPTPROPTESTCUTOFF_MASK    0x0003ff00U
#define XSECURE_TRNG_CTRL_3_ADAPTPROPTESTCUTOFF_DEFVAL  0x264U

/* access_type: wo */
#define XSECURE_TRNG_CTRL_3_DLEN_SHIFT   0U
#define XSECURE_TRNG_CTRL_3_DLEN_MASK    0x000000ffU
#define XSECURE_TRNG_CTRL_3_DLEN_DEFVAL  0x9U

/**
 * Register: TRNG_CTRL_4
 */
#define XSECURE_TRNG_CTRL_4    ( ( XSECURE_TRNG_BASEADDR ) + 0x00000014U)

/**
 * Register: TRNG_PER_STRNG_11
 */
#define XSECURE_TRNG_PER_STRNG_11    ( ( XSECURE_TRNG_BASEADDR ) + 0x000000ACU)

/**
 * Register: TRNG_CORE_OUTPUT
 */
#define XSECURE_TRNG_CORE_OUTPUT    ( ( XSECURE_TRNG_BASEADDR ) + 0x000000C0U)
/**
 * Register: TRNG_RESET
 */
#define XSECURE_TRNG_RESET    ( ( XSECURE_TRNG_BASEADDR ) + 0x000000D0U)
#define XSECURE_TRNG_RESET_DEFVAL   0x1U

/* access_type: rw */
#define XSECURE_TRNG_RESET_VAL_MASK    0x00000001U

/**
 * Register: TRNG_OSC_EN
 */
#define XSECURE_TRNG_OSC_EN    ( ( XSECURE_TRNG_BASEADDR ) + 0x000000D4U)

/* access_type: rw */
#define XSECURE_TRNG_OSC_EN_VAL_MASK    0x00000001U
#define XSECURE_TRNG_OSC_EN_VAL_DEFVAL  0x0U

#ifdef __cplusplus
}
#endif

#endif /* __XSECURE_TRNG_HW_H_ */
