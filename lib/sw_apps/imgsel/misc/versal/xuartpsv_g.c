/*******************************************************************
* Copyright (C) 2022 Xilinx, Inc. All Rights Reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*
* Description: Driver configuration
*
*******************************************************************/

#include "xparameters.h"
#include "xuartpsv.h"

/*
* The configuration table for devices
*/

XUartPsv_Config XUartPsv_ConfigTable[XPAR_XUARTPSV_NUM_INSTANCES] =
{
	{
		XPAR_VERSAL_CIPS_0_PSPMC_0_PSV_SBSAUART_0_DEVICE_ID,
		XPAR_VERSAL_CIPS_0_PSPMC_0_PSV_SBSAUART_0_BASEADDR,
		XPAR_VERSAL_CIPS_0_PSPMC_0_PSV_SBSAUART_0_UART_CLK_FREQ_HZ,
		XPAR_VERSAL_CIPS_0_PSPMC_0_PSV_SBSAUART_0_HAS_MODEM,
		XPAR_VERSAL_CIPS_0_PSPMC_0_PSV_SBSAUART_0_BAUDRATE
	}
};
