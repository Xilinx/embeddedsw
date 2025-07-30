
/*******************************************************************
* Copyright (c) 2010-2023 Xilinx Inc. All rights reserved.
* Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/
/**
 * *
* @file xv_frmbufwr_g.c
* @addtogroup v_frmbuf_wr Overview
*
**/

#ifndef SDT
#include "xparameters.h"
#endif
#include "xv_frmbufwr.h"

/*
 * XV_frmbufwr_ConfigTable contains the configuration parameters for each
 * instance of the XV_frmbufwr device present in the system. The values
 * are populated from hardware parameters defined in xparameters.h.
 */
XV_frmbufwr_Config XV_frmbufwr_ConfigTable[XPAR_XV_FRMBUFWR_NUM_INSTANCES] =
{
	{
		XPAR_V_FRMBUF_WR_0_DEVICE_ID,
		XPAR_V_FRMBUF_WR_0_S_AXI_CTRL_BASEADDR,
		XPAR_V_FRMBUF_WR_0_SAMPLES_PER_CLOCK,
		XPAR_V_FRMBUF_WR_0_MAX_COLS,
		XPAR_V_FRMBUF_WR_0_MAX_ROWS,
		XPAR_V_FRMBUF_WR_0_MAX_DATA_WIDTH,
		XPAR_V_FRMBUF_WR_0_AXIMM_DATA_WIDTH,
		XPAR_V_FRMBUF_WR_0_AXIMM_ADDR_WIDTH,
		XPAR_V_FRMBUF_WR_0_HAS_RGBX8,
		XPAR_V_FRMBUF_WR_0_HAS_YUVX8,
		XPAR_V_FRMBUF_WR_0_HAS_YUYV8,
		XPAR_V_FRMBUF_WR_0_HAS_RGBX10,
		XPAR_V_FRMBUF_WR_0_HAS_YUVX10,
		XPAR_V_FRMBUF_WR_0_HAS_Y_UV8,
		XPAR_V_FRMBUF_WR_0_HAS_Y_UV8_420,
		XPAR_V_FRMBUF_WR_0_HAS_RGB8,
		XPAR_V_FRMBUF_WR_0_HAS_YUV8,
		XPAR_V_FRMBUF_WR_0_HAS_Y_UV10,
		XPAR_V_FRMBUF_WR_0_HAS_Y_UV10_420,
		XPAR_V_FRMBUF_WR_0_HAS_Y8,
		XPAR_V_FRMBUF_WR_0_HAS_Y10,
		XPAR_V_FRMBUF_WR_0_HAS_BGRX8,
		XPAR_V_FRMBUF_WR_0_HAS_UYVY8,
		XPAR_V_FRMBUF_WR_0_HAS_BGR8,
		XPAR_V_FRMBUF_WR_0_HAS_RGBX12,
		XPAR_V_FRMBUF_WR_0_HAS_RGB16,
		XPAR_V_FRMBUF_WR_0_HAS_YUVX12,
		XPAR_V_FRMBUF_WR_0_HAS_Y_UV12,
		XPAR_V_FRMBUF_WR_0_HAS_Y_UV12_420,
		XPAR_V_FRMBUF_WR_0_HAS_Y12,
		XPAR_V_FRMBUF_WR_0_HAS_YUV16,
		XPAR_V_FRMBUF_WR_0_HAS_Y_UV16,
		XPAR_V_FRMBUF_WR_0_HAS_Y_UV16_420,
		XPAR_V_FRMBUF_WR_0_HAS_Y16,
		XPAR_V_FRMBUF_WR_0_HAS_Y_U_V8,
		XPAR_V_FRMBUF_WR_0_HAS_Y_U_V10,
		XPAR_V_FRMBUF_WR_0_HAS_Y_U_V8_420,
		XPAR_V_FRMBUF_WR_0_HAS_INTERLACED
	}
};
