
/*******************************************************************
*
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xufspsxc_g.c
* @addtogroup ufspsxc Overview
* @{
*
* The xufspsxc_g.c contains a configuration table that specifies the configuration of
* UFS devices in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------.
* 1.0   sk  01/16/24 First release
* 1.1   sk  03/12/25 Updated UFS config defines
*
* </pre>
*
******************************************************************************/

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
