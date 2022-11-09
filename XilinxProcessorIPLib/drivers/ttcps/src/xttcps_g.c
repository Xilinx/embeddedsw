/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xttcps_g.c
* @addtogroup ttcps Overview
* @{
*
* This file contains a configuration table where each entry is the
* configuration information for one timer counter device in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -----------------------------------------------
* 1.00a drg/jz 01/21/10 First release
* 2.00  hk     22/01/14 Added check for picking instances other than
*                       default.
* 3.00  kvn    02/13/15 Modified code for MISRA-C:2012 compliance.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xttcps.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/

/**
 * This table contains configuration information for each TTC device
 * in the system.
 */
XTtcPs_Config XTtcPs_ConfigTable[XPAR_XTTCPS_NUM_INSTANCES] = {
	{
		 (u16)XPAR_XTTCPS_0_DEVICE_ID,	/* Device ID for instance */
		 (u32)XPAR_XTTCPS_0_BASEADDR,	/* Device base address */
		 (u32)XPAR_XTTCPS_0_TTC_CLK_FREQ_HZ	/* Device input clock frequency */
	},
#ifdef XPAR_XTTCPS_1_DEVICE_ID
	{
		 (u16)XPAR_XTTCPS_1_DEVICE_ID,	/* Device ID for instance */
		 (u32)XPAR_XTTCPS_1_BASEADDR,	/* Device base address */
		 (u32)XPAR_XTTCPS_1_CLOCK_HZ	/* Device input clock frequency */
	},
#endif

#ifdef XPAR_XTTCPS_2_DEVICE_ID
	{
		 (u16)XPAR_XTTCPS_2_DEVICE_ID,	/* Device ID for instance */
		 (u32)XPAR_XTTCPS_2_BASEADDR,	/* Device base address */
		 (u32)XPAR_XTTCPS_2_CLOCK_HZ	/* Device input clock frequency */
	},
#endif

#ifdef XPAR_XTTCPS_3_DEVICE_ID
	{
		 (u16)XPAR_XTTCPS_3_DEVICE_ID,	/* Device ID for instance */
		 (u32)XPAR_XTTCPS_3_BASEADDR,	/* Device base address */
		 (u32)XPAR_XTTCPS_3_CLOCK_HZ	/* Device input clock frequency */
	},
#endif

#ifdef XPAR_XTTCPS_4_DEVICE_ID
	{
		 (u16)XPAR_XTTCPS_4_DEVICE_ID,	/* Device ID for instance */
		 (u32)XPAR_XTTCPS_4_BASEADDR,	/* Device base address */
		 (u32)XPAR_XTTCPS_4_CLOCK_HZ	/* Device input clock frequency */
	},
#endif

#ifdef XPAR_XTTCPS_5_DEVICE_ID
	{
		 (u16)XPAR_XTTCPS_5_DEVICE_ID,	/* Device ID for instance */
		 (u32)XPAR_XTTCPS_5_BASEADDR,	/* Device base address */
		 (u32)XPAR_XTTCPS_5_CLOCK_HZ	/* Device input clock frequency */
	},
#endif
};
/** @} */
