/*******************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/

/*****************************************************************************/
/**
*
* @file xclk_wiz_g.c
* @brief Provides the CLK_WIZ configuration table.
*
* This file contains a configuration table that specifies the parameters of
* CLK_WIZ devices.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.12  tb  09/16/26 Align Doxygen comments with AMD standards.
* </pre>
*
******************************************************************************/

/**
* @addtogroup clk_wiz_api CLK_WIZ APIs
* @{
*/

#include "xparameters.h"
#include "xclk_wiz.h"

/**
 * @brief Holds CLK_WIZ configuration-table entries.
 */

XClk_Wiz_Config XClk_Wiz_ConfigTable[] = {
	{
		XPAR_CLK_MONITOR_DEVICE_ID,
		XPAR_CLK_MONITOR_BASEADDR,
		XPAR_CLK_MONITOR_ENABLE_CLOCK_MONITOR,
		XPAR_CLK_MONITOR_ENABLE_USER_CLOCK0,
		XPAR_CLK_MONITOR_ENABLE_USER_CLOCK1,
		XPAR_CLK_MONITOR_ENABLE_USER_CLOCK2,
		XPAR_CLK_MONITOR_ENABLE_USER_CLOCK3,
		XPAR_CLK_MONITOR_REF_CLK_FREQ,
		XPAR_CLK_MONITOR_USER_CLK_FREQ0,
		XPAR_CLK_MONITOR_USER_CLK_FREQ1,
		XPAR_CLK_MONITOR_USER_CLK_FREQ2,
		XPAR_CLK_MONITOR_USER_CLK_FREQ3,
		XPAR_CLK_MONITOR_PRECISION,
		XPAR_CLK_MONITOR_ENABLE_PLL0,
		XPAR_CLK_MONITOR_ENABLE_PLL1
	},
	{
		XPAR_CLK_WIZ_DYN_RECONFIG_DEVICE_ID,
		XPAR_CLK_WIZ_DYN_RECONFIG_BASEADDR,
		XPAR_CLK_WIZ_DYN_RECONFIG_ENABLE_CLOCK_MONITOR,
		XPAR_CLK_WIZ_DYN_RECONFIG_ENABLE_USER_CLOCK0,
		XPAR_CLK_WIZ_DYN_RECONFIG_ENABLE_USER_CLOCK1,
		XPAR_CLK_WIZ_DYN_RECONFIG_ENABLE_USER_CLOCK2,
		XPAR_CLK_WIZ_DYN_RECONFIG_ENABLE_USER_CLOCK3,
		XPAR_CLK_WIZ_DYN_RECONFIG_REF_CLK_FREQ,
		XPAR_CLK_WIZ_DYN_RECONFIG_USER_CLK_FREQ0,
		XPAR_CLK_WIZ_DYN_RECONFIG_USER_CLK_FREQ1,
		XPAR_CLK_WIZ_DYN_RECONFIG_USER_CLK_FREQ2,
		XPAR_CLK_WIZ_DYN_RECONFIG_USER_CLK_FREQ3,
		XPAR_CLK_WIZ_DYN_RECONFIG_PRECISION,
		XPAR_CLK_WIZ_DYN_RECONFIG_ENABLE_PLL0,
		XPAR_CLK_WIZ_DYN_RECONFIG_ENABLE_PLL1
	}
};
/** @} */
