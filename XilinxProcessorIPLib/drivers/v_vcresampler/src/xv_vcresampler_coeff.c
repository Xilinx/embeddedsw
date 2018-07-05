/******************************************************************************
 *
 * Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xv_vcresampler_coeff.c
* @addtogroup v_vcresampler_v3_0
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
