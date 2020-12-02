/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hcresampler_coeff.c
* @addtogroup v_hcresampler_v3_2
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
* 1.00  rco   07/31/15   Initial Release
* 3.0   rco   02/09/17   Fix c++ compilation warnings
* </pre>
*
******************************************************************************/
#include "xv_hcresampler_l2.h"

// 4 tap filter
#ifdef __cplusplus
extern "C"
#endif
const short XV_hcrsmplrcoeff_taps4[XV_HCRSMPLR_NUM_CONVERSIONS][XV_HCRSMPLR_MAX_PHASES][XV_HCRSMPLR_TAPS_4] =
{
  //444->422
  {{   0, 1024, 2048, 1024},
   {   0,    0,    0,    0}
  },
  //422->444
  {{   0,    0, 4096,    0},
   { 506, 1542, 1542,  506}
  }
};

// 6 tap filter
#ifdef __cplusplus
extern "C"
#endif
const short XV_hcrsmplrcoeff_taps6[XV_HCRSMPLR_NUM_CONVERSIONS][XV_HCRSMPLR_MAX_PHASES][XV_HCRSMPLR_TAPS_6] =
{
  //444->422
  {{   0,    0, 1298, 1500, 1298,    0},
   {   0,    0,    0,    0,    0,    0}
  },
  //422->444
  {{   0,    0,    0, 4096,    0,    0},
   {-327,  792, 1583, 1583,  792, -327}
  }
};

// 8 tap filter
#ifdef __cplusplus
extern "C"
#endif
const short XV_hcrsmplrcoeff_taps8[XV_HCRSMPLR_NUM_CONVERSIONS][XV_HCRSMPLR_MAX_PHASES][XV_HCRSMPLR_TAPS_8] =
{
  //444->422
  {{   0, -988,    0, 1703, 2666, 1703,    0, -988},
   {   0,    0,    0,    0,    0,    0,    0,    0}
  },
  //422->444
  {{   0,    0,    0,    0, 4096,    0,    0,    0},
   {-423, -903,  977, 2397, 2397,  977, -903, -423}
  }
};

// 10 tap filter
#ifdef __cplusplus
extern "C"
#endif
const short XV_hcrsmplrcoeff_taps10[XV_HCRSMPLR_NUM_CONVERSIONS][XV_HCRSMPLR_MAX_PHASES][XV_HCRSMPLR_TAPS_10] =
{
  //444->422
  {{   0,    0, -988,    0, 1703, 2666, 1703,    0, -988,    0},
   {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0}
  },
  //422->444
  {{   0,    0,    0,    0,    0, 4096,    0,    0,    0,    0},
   { 305, -638, -586,  705, 2262, 2262,  705, -586, -638,  305}
  }
};
