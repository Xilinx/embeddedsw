
/*******************************************************************
* Copyright (c) 2010-2022 Xilinx, Inc. All Rights Reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/

/*****************************************************************************/
/**
*
* @file xv_multi_scaler_g.c
* @addtogroup v_multi_scaler Overview
* @{
*
* This file contains the device configuration table for the MultiScaler IP.
* The configuration table is automatically generated based on hardware
* parameters defined in xparameters.h. This table is used during device
* initialization to configure the MultiScaler driver instance.
*
* @note This is an auto-generated file. Manual modifications may be
*       overwritten during hardware regeneration.
*
******************************************************************************/

/***************************** Include Files *********************************/
#ifndef SDT
#include "xparameters.h"
#endif
#include "xv_multi_scaler.h"


/************************** Variable Definitions *****************************/

/**
 * MultiScaler device configuration table
 *
 * This table contains the configuration information for each MultiScaler
 * device in the system. Each entry includes:
 * - Device ID: Unique identifier for the device instance
 * - Base Address: AXI control interface base address
 * - Samples Per Clock: Number of samples processed per clock cycle
 * - Max Data Width: Maximum pixel data width supported
 * - Max Cols: Maximum number of horizontal pixels
 * - Max Rows: Maximum number of vertical pixels
 * - Phase Shift: Scaling phase shift precision
 * - Scale Mode: Supported scaling modes (bilinear, polyphase, etc.)
 * - Taps: Number of filter taps for polyphase scaling
 * - Max Outs: Maximum number of output channels supported
 *
 * The table is indexed by device ID for lookup during initialization.
 */
XV_multi_scaler_Config XV_multi_scaler_ConfigTable[] = {
	{
		XPAR_V_MULTI_SCALER_0_DEVICE_ID,
		XPAR_V_MULTI_SCALER_0_S_AXI_CTRL_BASEADDR,
		XPAR_V_MULTI_SCALER_0_SAMPLES_PER_CLOCK,
		XPAR_V_MULTI_SCALER_0_MAX_DATA_WIDTH,
		XPAR_V_MULTI_SCALER_0_MAX_COLS,
		XPAR_V_MULTI_SCALER_0_MAX_ROWS,
		XPAR_V_MULTI_SCALER_0_PHASE_SHIFT,
		XPAR_V_MULTI_SCALER_0_SCALE_MODE,
		XPAR_V_MULTI_SCALER_0_TAPS,
		XPAR_V_MULTI_SCALER_0_MAX_OUTS
	}
};
/** @} */
