/**************************************************************************************************
* Copyright (C) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xtrngpsv_hw.h
 * @addtogroup Overview
 * @{
 *
 * This header file contains identifiers and register-level core functions (or macros) that can be
 * used to access the True Random Number Generator core.
 *
 * For more information about the operation of this core see the hardware specification and
 * documentation in the higher level driver xtrngpsv.h file.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver  Who Date     Changes
 * ---- --- -------- -------------------------------------------------------------------------------
 * 1.00  ssc  09/05/21 First release
 * 1.1   ssc  03/24/22 Minor doxygen related fixes
 *       mb   09/14/23 Fix MISRA-C vioaltion 2.5
 *
 * </pre>
 *
 **************************************************************************************************/
#ifndef XTRNGPSV_HW_H_
#define XTRNGPSV_HW_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *****************************************************/

#include "xil_io.h"

/************************** Constant Definitions *************************************************/

/**
 * @name Register definitions
 * @{
 */
/* Register: TRNG_STATUS */
#define TRNG_STATUS    0x00000004U

#define TRNG_STATUS_QCNT_MASK    0x00000e00U
#define TRNG_STATUS_CERTF_MASK    0x00000008U
#define TRNG_STATUS_DTF_MASK    0x00000002U
#define TRNG_STATUS_DONE_MASK    0x00000001U

/* Register: TRNG_CTRL */
#define TRNG_CTRL    0x00000008U
#define TRNG_CTRL_EUMODE_MASK    0x00000100U
#define TRNG_CTRL_PRNGMODE_MASK    0x00000080U
#define TRNG_CTRL_PRNGSTART_MASK    0x00000020U
#define TRNG_CTRL_PRNGXS_MASK    0x00000008U
#define TRNG_CTRL_TRSSEN_MASK    0x00000004U
#define TRNG_CTRL_PRNGSRST_MASK    0x00000001U

/* Register: TRNG_EXT_SEED_0 */
#define TRNG_EXT_SEED_0    0x00000040U
/**
 * Below registers are not directly referenced in driver but are accessed accessed with offset
 * from TRNG_EXT_SEED_0
 *
 * Register: TRNG_EXT_SEED_1	0x00000044U
 * Register: TRNG_EXT_SEED_2	0x00000048U
 * Register: TRNG_EXT_SEED_3	0x0000004CU
 * Register: TRNG_EXT_SEED_4	0x00000050U
 * Register: TRNG_EXT_SEED_5	0x00000054U
 * Register: TRNG_EXT_SEED_6	0x00000058U
 * Register: TRNG_EXT_SEED_7	0x0000005CU
 * Register: TRNG_EXT_SEED_8	0x00000060U
 * Register: TRNG_EXT_SEED_9	0x00000064U
 * Register: TRNG_EXT_SEED_10	0x00000068U
 * Register: TRNG_EXT_SEED_11	0x0000006CU
 */

/* Register: TRNG_PER_STRNG_0 */
#define TRNG_PER_STRNG_0    0x00000080U
/**
 * Below registers are not directly referenced in driver but are accessed accessed with offset
 * from TRNG_PER_STRNG_0
 *
 * Register: TRNG_PER_STRNG_1	0x00000084U
 * Register: TRNG_PER_STRNG_2	0x00000088U
 * Register: TRNG_PER_STRNG_3	0x0000008CU
 * Register: TRNG_PER_STRNG_4	0x00000090U
 * Register: TRNG_PER_STRNG_5	0x00000094U
 * Register: TRNG_PER_STRNG_6	0x00000098U
 * Register: TRNG_PER_STRNG_7	0x0000009CU
 * Register: TRNG_PER_STRNG_8	0x000000A0U
 * Register: TRNG_PER_STRNG_9	0x000000A4U
 * Register: TRNG_PER_STRNG_10	0x000000A8U
 * Register: TRNG_PER_STRNG_11	0x000000ACU
 */

/* Register: TRNG_CORE_OUTPUT */
#define TRNG_CORE_OUTPUT    0x000000C0U

/* Register: TRNG_RESET */
#define TRNG_RESET    0x000000D0U
#define TRNG_RESET_VAL_MASK    0x00000001U

/* Register: TRNG_OSC_EN */
#define TRNG_OSC_EN    0x000000D4U
#define TRNG_OSC_EN_VAL_MASK    0x00000001U
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* XTRNGPSV_HW_H_ */
/** @} */
