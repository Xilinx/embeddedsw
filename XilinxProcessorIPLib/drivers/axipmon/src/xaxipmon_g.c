/******************************************************************************
* Copyright (C) 2012 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xaxipmon_g.c
* @addtogroup axipmon_v6_8
* @{
*
* This file contains a configuration table that specifies the configuration
* of AxiMon devices in the system.
*
* See xaxipmon.h for more information about this driver.
*
* @note None.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.00a bss    02/22/12 First release
* 2.00a bss    06/23/12 Updated to support v2_00a version of IP.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xaxipmon.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Prototypes ******************************/

/**
 * This table contains configuration information for each System Monitor/ADC
 * device in the system.
 */
XAxiPmon_Config XAxiPmon_ConfigTable[XPAR_XAXIPMON_NUM_INSTANCES] =
{
	{
		XPAR_AXI_PERF_MON_0_DEVICE_ID,
		XPAR_AXI_PERF_MON_0_BASEADDR,
		XPAR_AXI_PERF_MON_0_GLOBAL_COUNT_WIDTH,
		XPAR_AXI_PERF_MON_0_METRICS_SAMPLE_COUNT_WIDTH
	}
};
/** @} */
