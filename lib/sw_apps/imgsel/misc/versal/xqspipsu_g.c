/*******************************************************************
* Copyright (C) 2022 Xilinx, Inc. All Rights Reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*
* Description: Driver configuration
*
*******************************************************************/

#include "xparameters.h"
#include "xqspipsu.h"

/*
* The configuration table for devices
*/

XQspiPsu_Config XQspiPsu_ConfigTable[XPAR_XQSPIPSU_NUM_INSTANCES] =
{
	{
		XPAR_VERSAL_CIPS_0_PSPMC_0_PSV_PMC_QSPI_0_DEVICE_ID,
		XPAR_VERSAL_CIPS_0_PSPMC_0_PSV_PMC_QSPI_0_BASEADDR,
		XPAR_VERSAL_CIPS_0_PSPMC_0_PSV_PMC_QSPI_0_QSPI_CLK_FREQ_HZ,
		XPAR_VERSAL_CIPS_0_PSPMC_0_PSV_PMC_QSPI_0_QSPI_MODE,
		XPAR_VERSAL_CIPS_0_PSPMC_0_PSV_PMC_QSPI_0_QSPI_BUS_WIDTH,
		XPAR_VERSAL_CIPS_0_PSPMC_0_PSV_PMC_QSPI_0_IS_CACHE_COHERENT
	}
};
