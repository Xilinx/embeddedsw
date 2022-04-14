/******************************************************************************
* Copyright (C) 2013 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsdps_g.c
* @addtogroup Overview
* @{
*
* This file contains a configuration table that specifies the configuration of
* SD devices in the system.
*
*
******************************************************************************/

#include "xparameters.h"
#include "xsdps.h"

/**
 * The configuration table for devices
 */

XSdPs_Config XSdPs_ConfigTable[XPAR_XSDPS_NUM_INSTANCES] =
{
	{
		XPAR_PSU_SD_1_DEVICE_ID,
		XPAR_PSU_SD_1_BASEADDR,
		XPAR_PSU_SD_1_SDIO_CLK_FREQ_HZ,
		XPAR_PSU_SD_1_HAS_CD,
		XPAR_PSU_SD_1_HAS_WP,
		XPAR_PSU_SD_1_BUS_WIDTH,
		XPAR_PSU_SD_1_MIO_BANK,
		XPAR_PSU_SD_1_HAS_EMIO,
		XPAR_PSU_SD_1_SLOT_TYPE,
		XPAR_PSU_SD_1_IS_CACHE_COHERENT,
		XPAR_PSU_SD_1_CLK_50_SDR_ITAP_DLY,
		XPAR_PSU_SD_1_CLK_50_SDR_OTAP_DLY,
		XPAR_PSU_SD_1_CLK_50_DDR_ITAP_DLY,
		XPAR_PSU_SD_1_CLK_50_DDR_OTAP_DLY,
		XPAR_PSU_SD_1_CLK_100_SDR_OTAP_DLY,
		XPAR_PSU_SD_1_CLK_200_SDR_OTAP_DLY
	}
};
/** @} */
