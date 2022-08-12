/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xaxicdma_g.c
* @addtogroup axicdma_v4_10
* @{
*
* Provide a template for user to define their own hardware settings.
*
* If using XPS, XPS will automatically generate this file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a jz   08/16/10 First release
* 4.5   rsp  07/04/18 Sync XAxiCdma_Config initializer fields
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xaxicdma.h"
/************************** Constant Definitions *****************************/

XAxiCdma_Config XAxiCdma_ConfigTable[] =
{
	{
		XPAR_AXICDMA_0_DEVICE_ID,
		XPAR_AXICDMA_0_BASEADDR,
		XPAR_AXICDMA_0_INCLUDE_DRE,
		XPAR_AXICDMA_0_USE_DATAMOVER_LITE,
		XPAR_AXICDMA_0_M_AXI_DATA_WIDTH,
		XPAR_AXICDMA_0_M_AXI_MAX_BURST_LEN,
		XPAR_AXICDMA_0_ADDR_WIDTH
	}
};
/** @} */
