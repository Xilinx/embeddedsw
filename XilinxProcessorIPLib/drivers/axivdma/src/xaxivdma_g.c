/******************************************************************************
* Copyright (C) 2012 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xaxivdma_g.c
* @addtogroup axivdma_v6_8
* @{
*
* Provide a template for user to define their own hardware settings.
*
* If using XPS, this file will be automatically generated.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a jz   08/16/10 First release
* 2.00a jz   12/10/10 Added support for direct register access mode, v3 core
* 2.01a jz   01/19/11 Added ability to re-assign BD addresses
* 	rkv  04/07/11 added new configuration parameter for enabling read of
*		      video parameters.
* 3.00a srt  08/26/11 Added new parameters for Flush on Frame Sync and Line
*		      Buffer Thresholds.
* 4.00a srt  11/21/11 Added new parameters for Genlock Source and Fsync
*		      Source Selection.
* 4.03a srt  01/18/13 Added TDATA_WIDTH parameters (CR: 691866)
* 4.04a srt	 03/03/13 Support for *_ENABLE_DEBUG_INFO_* debug configuration
*			parameters (CR: 703738)
* 5.1   sha  07/15/15 Added XPAR_XAXIVDMA_NUM_INSTANCES macro to control
*		      config table parameters.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xaxivdma.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

XAxiVdma_Config XAxiVdma_ConfigTable[] =
{
	{
#ifdef XPAR_XAXIVDMA_NUM_INSTANCES
		XPAR_AXIVDMA_0_DEVICE_ID,
		XPAR_AXIVDMA_0_BASEADDR,
		XPAR_AXIVDMA_0_NUM_FSTORES,
		XPAR_AXIVDMA_0_INCLUDE_MM2S,
		XPAR_AXIVDMA_0_INCLUDE_MM2S_DRE,
		XPAR_AXIVDMA_0_M_AXI_MM2S_DATA_WIDTH,
		XPAR_AXIVDMA_0_INCLUDE_S2MM,
		XPAR_AXIVDMA_0_INCLUDE_S2MM_DRE,
		XPAR_AXIVDMA_0_M_AXI_S2MM_DATA_WIDTH,
		XPAR_AXIVDMA_0_INCLUDE_SG,
		XPAR_AXIVDMA_0_ENABLE_VIDPRMTR_READS,
		XPAR_AXIVDMA_0_USE_FSYNC,
		XPAR_AXIVDMA_0_FLUSH_ON_SYNC,
		XPAR_AXIVDMA_0_MM2S_LINEBUFFER_DEPTH,
		XPAR_AXIVDMA_0_S2MM_LINEBUFFER_DEPTH,
		XPAR_AXIVDMA_0_MM2S_GENLOCK_MODE,
		XPAR_AXIVDMA_0_S2MM_GENLOCK_MODE,
		XPAR_AXIVDMA_0_INCLUDE_INTERNAL_GENLOCK,
		XPAR_AXIVDMA_0_S2MM_SOF_ENABLE,
		XPAR_AXIVDMA_0_M_AXIS_MM2S_TDATA_WIDTH,
		XPAR_AXIVDMA_0_S_AXIS_S2MM_TDATA_WIDTH,
		XPAR_AXIVDMA_0_ENABLE_DEBUG_INFO_1,
		XPAR_AXIVDMA_0_ENABLE_DEBUG_INFO_5,
		XPAR_AXIVDMA_0_ENABLE_DEBUG_INFO_6,
		XPAR_AXIVDMA_0_ENABLE_DEBUG_INFO_7,
		XPAR_AXIVDMA_0_ENABLE_DEBUG_INFO_9,
		XPAR_AXIVDMA_0_ENABLE_DEBUG_INFO_13,
		XPAR_AXIVDMA_0_ENABLE_DEBUG_INFO_14,
		XPAR_AXIVDMA_0_ENABLE_DEBUG_INFO_15,
		XPAR_AXIVDMA_0_ENABLE_DEBUG_ALL
#endif
	}
};
/** @} */
