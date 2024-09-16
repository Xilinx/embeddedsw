/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

#include "xparameters.h"
#include "xuartpsv.h"

/*
* The configuration table for devices
*/
XUartPsv_Config XUartPsv_ConfigTable[XPAR_XUARTPSV_NUM_INSTANCES] = {
	{
		XPAR_PSV_SBSAUART_0_DEVICE_ID,
		XPAR_PSV_SBSAUART_0_BASEADDR,
		XPAR_PSV_SBSAUART_0_UART_CLK_FREQ_HZ,
		XPAR_PSV_SBSAUART_0_HAS_MODEM,
		XPAR_PSV_SBSAUART_0_BAUDRATE
	}
};
