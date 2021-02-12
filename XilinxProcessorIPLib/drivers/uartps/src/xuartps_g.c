/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xuartps_g.c
* @addtogroup uartps_v3_11
* @{
*
* This file contains a configuration table where each entry is a configuration
* structure for an XUartPs device in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date	Changes
* ----- ------ -------- -----------------------------------------------
* 1.00  drg/jz 05/13/08 First Release
* 2.00  hk     22/01/14 Added check for selecting uart0 instance.
* 3.00  kvn    02/13/15 Modified code for MISRA-C:2012 compliance.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xuartps.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/

/**
 * Each XUartPs device in the system has an entry in this table.
 */
XUartPs_Config XUartPs_ConfigTable[XPAR_XUARTPS_NUM_INSTANCES] = {
	{
		(u16)XPAR_XUARTPS_0_DEVICE_ID,
		(u32)XPAR_XUARTPS_0_BASEADDR,
		(u32)XPAR_XUARTPS_0_UART_CLK_FREQ_HZ,
		(s32)0
	},
#ifdef XPAR_XUARTPS_1_DEVICE_ID
	{
		(u16)XPAR_XUARTPS_1_DEVICE_ID,
		(u32)XPAR_XUARTPS_1_BASEADDR,
		(u32)XPAR_XUARTPS_1_UART_CLK_FREQ_HZ,
		(s32)0
	}
#endif
};
/** @} */
