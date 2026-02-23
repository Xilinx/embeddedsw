
/*******************************************************************
* Copyright (c) 2010-2022 Xilinx, Inc. All Rights Reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/

/**
 * @file xv_scenechange_g.c
 * @addtogroup v_scenechange Overview
 */

/***************************** Include Files *********************************/
#ifndef SDT
#include "xparameters.h"
#endif
#include "xv_scenechange.h"


/************************** Variable Definitions *****************************/
/**
 * Configuration table for SceneChange devices
 *
 * This table contains the configuration information for each SceneChange
 * device in the system. Each entry includes device ID, base address,
 * operational mode (memory/stream), and IP capabilities.
 */
XV_scenechange_Config XV_scenechange_ConfigTable[] =
{
	{
		XPAR_V_SCENECHANGE_0_DEVICE_ID,
		XPAR_V_SCENECHANGE_0_S_AXI_CTRL_BASEADDR,
		XPAR_XV_SCENECHANGE_0_MEMORY_BASED,
		XPAR_XV_SCENECHANGE_0_MAX_NR_STREAMS,
		XPAR_XV_SCENECHANGE_0_HISTOGRAM_BITS,
		XPAR_XV_SCENECHANGE_0_HAS_Y10,
		XPAR_XV_SCENECHANGE_0_HAS_Y8,
		XPAR_XV_SCENECHANGE_0_MAX_ROWS,
		XPAR_XV_SCENECHANGE_0_MAX_COLS
	}
};
