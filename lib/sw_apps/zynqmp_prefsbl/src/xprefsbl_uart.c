/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 * @file xprefsbl_uart.c
 *
 * This is the main file which will contain the UART initialization
 *
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  		Date     Changes
 * ----- ---- -------- ---------------------------------------------------------
 * 1.00  Ana	  24/06/20 	First release
 *
 * </pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xprefsbl_main.h"
#if defined(XPREFSBL_UART_ENABLE) && defined(STDOUT_BASEADDRESS)
#include "xuartps.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#if ((XPAR_XUARTPS_NUM_INSTANCES == 2U) && (STDOUT_BASEADDRESS == 0xFF010000U))
#define XPREFSBL_UART_INDEX 		(1U)
#else
#define XPREFSBL_UART_INDEX 		(0U)
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
int XPrefsbl_UartConfiguration(void)
{
	int Status = XST_FAILURE;
	XUartPs_Config *Config;

	/*
	 * Initialize the UART driver so that it's ready to use
	 * Look up the configuration in the config table and then initialize it.
	 */
	Config = XUartPs_LookupConfig(XPREFSBL_UART_INDEX);
	if (NULL == Config) {
		Status = XPREFSBL_UART_CONFIG_ERROR;
		goto END;
	}

	Status = XUartPs_CfgInitialize(&Uart_Ps, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		Status = XPREFSBL_UART_CONFIG_INIT_ERROR;
		goto END;
	}

	XUartPs_SetBaudRate(&Uart_Ps, 115200U);

END:
	return Status;
}
#endif