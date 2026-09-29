
/*******************************************************************
*
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xufspsxc_g.c
* @brief Provides the UFSPSXC configuration table.
*
* The table specifies the configuration of UFS devices in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------.
* 1.0   sk  01/16/24 First release
* 1.1   sk  03/12/25 Updated UFS config defines
* 1.3   vn  07/30/26 Fix Doxygen warnings
*
* 1.3   tb  09/16/26 Align Doxygen comments with AMD standards.
* </pre>
*
******************************************************************************/

/**
* @addtogroup ufspsxc_api UFSPSXC APIs
* @{
*/

#include "xparameters.h"
#include "xufspsxc.h"

/*
* The configuration table for devices
*/

XUfsPsxc_Config XUfsPsxc_ConfigTable[XPAR_XUFSPSXC_NUM_INSTANCES] =
{
	{
		XPAR_XUFSPSXC_0_COMPATIBLE,
		XPAR_XUFSPSXC_0_BASEADDR,
		XPAR_XUFSPSXC_0_CLK_FREQ_HZ,
		XPAR_XUFSPSXC_0_CFG_CLK_FREQ_HZ,
		XPAR_XUFSPSXC_0_REF_PAD_CLK_FREQ_HZ,
		XPAR_XUFSPSXC_0_IS_CACHE_COHERENT,
		XPAR_XUFSPSXC_0_INTERRUPTS
	}
};
/** @} */
