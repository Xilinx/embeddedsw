/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserve.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpm_noc_config.h
*
* NOC configuration header file
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who  Date		 Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0  rpoolla	 23/01/18 Initial release
* 2.0  tnt	 01/11/22 Porting to xilpm
* </pre>
*
* @note
*
******************************************************************************/
#ifndef SRC_XPMCR_NOC_H_
#define SRC_XPMCR_NOC_H_

#include "xil_util.h"

XStatus XPm_NoCConfig(void);
XStatus XPm_NoCHWConfig(void);
#ifdef XCVP1902
XStatus XPm_NocConfig_vp1902(void);
#endif

#endif /* SRC_XPMCR_NOC_H_ */
