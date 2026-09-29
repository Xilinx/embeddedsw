/******************************************************************************
* Copyright (C) 2017 - 2021 Xilinx, Inc.  All Rights Reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xuartpsv_g.c
*
* @brief Contains the configuration table for the XUartPsv driver.
*
******************************************************************************/

/** @addtogroup uartpsv Overview
 * @{
 */

#include "xparameters.h"
#include "xuartpsv.h"

/**
 * @brief Configuration table for XUartPsv devices.
 */
XUartPsv_Config XUartPsv_ConfigTable[] =
{
	{
		XPAR_SBSA_UART_DEVICE_ID,
		XPAR_SBSA_UART_BASEADDR,
		XPAR_SBSA_UART_CLK_FREQ_HZ,
		XPAR_SBSA_UART_HAS_MODEM
	}
};
/** @} */
