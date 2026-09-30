/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xmcdma_g.c
* @addtogroup mcdma Overview
* @{
* @brief Provides the MCDMA configuration table for the system.
*
* Provide a template for user to define their own hardware settings.
*
* If using XPS, XPS will automatically generate this file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 1.0   adk     18/07/17 Initial version.
* </pre>
*
******************************************************************************/

#include "xparameters.h"
#include "xmcdma.h"

/** Configuration table for each MCDMA instance in the system. */
XMcdma_Config XMcdma_ConfigTable[] = {
	{
		XPAR_AXI_MCDMA_0_DEVICE_ID,
		XPAR_AXI_MCDMA_0_BASEADDR,
		XPAR_AXI_MCDMA_0_ADDR_WIDTH,
		XPAR_AXI_MCDMA_0_ENABLE_SINGLE_INTR,
		XPAR_AXI_MCDMA_0_INCLUDE_MM2S,
		XPAR_AXI_MCDMA_0_INCLUDE_MM2S_DRE,
		XPAR_AXI_MCDMA_0_NUM_MM2S_CHANNELS,
		XPAR_AXI_MCDMA_0_INCLUDE_S2MM,
		XPAR_AXI_MCDMA_0_INCLUDE_S2MM_DRE,
		XPAR_AXI_MCDMA_0_NUM_S2MM_CHANNELS,
		XPAR_AXI_MCDMA_0_M_AXI_MM2S_DATA_WIDTH,
		XPAR_AXI_MCDMA_0_M_AXI_S2MM_DATA_WIDTH,
		XPAR_AXI_MCDMA_0_SG_LENGTH_WIDTH,
		XPAR_AXI_MCDMA_0_SG_INCLUDE_STSCNTRL_STRM
	}
};
/** @} */
