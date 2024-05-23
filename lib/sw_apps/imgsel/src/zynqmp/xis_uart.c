/******************************************************************************
* Copyright (c) 2020-2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 * @file xis_uart.c
 *
 * This is the main file which will contain the UART initialization
 *
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------------------
 * 1.00  Ana  06/24/20 First release
 * 2.00  sd   05/17/24 Add SDT support
 *
 * </pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xis_main.h"
#if defined(XIS_UART_ENABLE) && defined(STDOUT_BASEADDRESS)
#include "xuartps.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#if ((XPAR_XUARTPS_NUM_INSTANCES == 2U) && (STDOUT_BASEADDRESS == 0xFF010000U))
#ifndef SDT
#define XIS_UART_DEVICE 		(1U)
#else
#define XIS_UART_DEVICE XPAR_XUARTPS_1_BASEADDR
#endif
#else
#ifndef SDT
#define XIS_UART_DEVICE 		(0U)
#else
#define XIS_UART_DEVICE XPAR_XUARTPS_0_BASEADDR
#endif
#endif

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
static XUartPs Uart_Ps;

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
 * This function perform the initial configuration for the UART Device.
 *
 * @param	None.
 *
 * @return	XST_SUCCESS if pass, otherwise XST_FAILURE.
 *
 ****************************************************************************/
int XIs_UartConfiguration(void)
{
	int Status = XST_FAILURE;
	XUartPs_Config *Config;

	/*
	 * Initialize the UART driver so that it's ready to use
	 * Look up the configuration in the config table and then initialize it.
	 */
	Config = XUartPs_LookupConfig(XIS_UART_DEVICE);
	if (NULL == Config) {
		Status = XIS_UART_CONFIG_ERROR;
		goto END;
	}

	Status = XUartPs_CfgInitialize(&Uart_Ps, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		Status = XIS_UART_CONFIG_INIT_ERROR;
		goto END;
	}

	XUartPs_SetBaudRate(&Uart_Ps, 115200U);

END:
	return Status;
}
#endif
