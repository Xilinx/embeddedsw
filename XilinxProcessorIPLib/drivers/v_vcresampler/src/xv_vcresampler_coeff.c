/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_vcresampler_coeff.c
* @addtogroup v_vcresampler Overview
* @{
* @details
*
* This file provides the default fixed coefficient sets for supported taps
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   rco   07/31/15   Initial Release
* 3.0   rco   02/09/17   Fix c++ compilation warnings
* </pre>
*
******************************************************************************/
#include "xv_vcresampler_l2.h"

// 4 tap filter
#ifdef __cplusplus
extern "C"
#endif
/**
 * @brief Coefficient table for 4-tap vertical chroma resampling conversions.
 *
 * This 3D array contains the filter coefficients used for vertical chroma resampling
 * between different chroma subsampling formats (e.g., 4:2:2 to 4:2:0 and vice versa).
 *
 * Dimensions:
 * - [XV_VCRSMPLR_NUM_CONVERSIONS]: Number of conversion types (e.g., 422->420, 420->422).
 * - [XV_VCRSMPLR_MAX_PHASES]: Number of filter phases per conversion.
 * - [XV_VCRSMPLR_TAPS_4]: Number of taps (coefficients) per phase (4-tap filter).
 *
 * Each entry contains the coefficients for a specific conversion and phase.
 *
 * Example usage:
 *   XV_vcrsmplrcoeff_taps4[conversion][phase][tap]
 */
const short XV_vcrsmplrcoeff_taps4[XV_VCRSMPLR_NUM_CONVERSIONS][XV_VCRSMPLR_MAX_PHASES][XV_VCRSMPLR_TAPS_4] =
{
  //422->420
  {{   0, 1024, 2048, 1024},
   {   0,    0,    0,    0}
  },
  //420->422
  {{ 506, 1542, 1542,  506},
   {   0, 4096,    0,    0}
  }
};

// 6 tap filter
#ifdef __cplusplus
extern "C"
#endif
/**
 * @brief Coefficient table for vertical chroma resampler with 6 taps.
 *
 * This 3D array contains precomputed filter coefficients used by the vertical chroma resampler
 * for different conversion modes and phases. The coefficients are organized as follows:
 * - First dimension: Conversion type (e.g., 422->420, 420->422)
 * - Second dimension: Phase of the resampling process
 * - Third dimension: Tap coefficients for the filter (6 taps per phase)
 *
 * The coefficients are used to perform chroma resampling between different chroma subsampling formats.
 *
 * @see XV_VCRSMPLR_NUM_CONVERSIONS
 * @see XV_VCRSMPLR_MAX_PHASES
 * @see XV_VCRSMPLR_TAPS_6
 */

const short XV_vcrsmplrcoeff_taps6[XV_VCRSMPLR_NUM_CONVERSIONS][XV_VCRSMPLR_MAX_PHASES][XV_VCRSMPLR_TAPS_6] =
{
  //422->420
  {{   0,    0, 1298, 1500, 1298,    0},
   {   0,    0,    0,    0,    0,    0}
  },
  //420->422
  {{-327,  792, 1583, 1583,  792, -327},
   {   0,    0, 4096,    0,    0,    0}
  }
};

// 8 tap filter
#ifdef __cplusplus
extern "C"
#endif


/**
 * @brief Coefficient table for vertical chroma resampling with 8 taps.
 *
 * This 3D array contains precomputed filter coefficients used for vertical chroma resampling
 * between different chroma subsampling formats (e.g., 4:2:2 to 4:2:0 and vice versa).
 *
 * Dimensions:
 * - [XV_VCRSMPLR_NUM_CONVERSIONS]: Number of conversion types (e.g., 422->420, 420->422).
 * - [XV_VCRSMPLR_MAX_PHASES]: Number of filter phases.
 * - [XV_VCRSMPLR_TAPS_8]: Number of taps per filter (8 taps).
 *
 * Each entry contains the coefficients for a specific conversion and phase.
 *
 * Example conversions:
 * - 422->420: Downsampling from 4:2:2 to 4:2:0 chroma format.
 * - 420->422: Upsampling from 4:2:0 to 4:2:2 chroma format.
 */
const short XV_vcrsmplrcoeff_taps8[XV_VCRSMPLR_NUM_CONVERSIONS][XV_VCRSMPLR_MAX_PHASES][XV_VCRSMPLR_TAPS_8] =
{
  //422->420
  {{   0, -988,    0, 1703, 2666, 1703,    0, -988},
   {   0,    0,    0,    0,    0,    0,    0,    0}
  },
  //420->422
  {{-423, -903,  977, 2397, 2397,  977, -903, -423},
   {   0,    0,    0, 4096,    0,    0,    0,    0}
  }
};

// 10 tap filter
#ifdef __cplusplus
extern "C"
#endif


/**
 * @brief Coefficient table for vertical chroma resampler with 10 taps.
 *
 * This 3D array contains precomputed filter coefficients for chroma resampling
 * conversions between 4:2:2 and 4:2:0 formats. The coefficients are organized as:
 * - First dimension: Conversion type (e.g., 422->420, 420->422)
 * - Second dimension: Filter phase
 * - Third dimension: Tap index (10 taps per filter)
 *
 * The coefficients are used by the vertical chroma resampler hardware or software
 * to perform high-quality chroma sub-sampling or up-sampling.
 *
 * @see XV_VCRSMPLR_NUM_CONVERSIONS
 * @see XV_VCRSMPLR_MAX_PHASES
 * @see XV_VCRSMPLR_TAPS_10
 */
const short XV_vcrsmplrcoeff_taps10[XV_VCRSMPLR_NUM_CONVERSIONS][XV_VCRSMPLR_MAX_PHASES][XV_VCRSMPLR_TAPS_10] =
{
  //422->420
  {{   0,    0, -988,    0, 1703, 2666, 1703,    0, -988,    0},
   {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0}
  },
  //420->422
  {{ 305, -638, -586,  705, 2262, 2262,  705, -586, -638,  305},
   {   0,    0,    0,    0, 4096,    0,    0,    0,    0,    0}
  }
};
/* @} */