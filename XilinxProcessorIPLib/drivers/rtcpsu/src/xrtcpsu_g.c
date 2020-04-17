/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xrtcpsu_g.c
* @addtogroup rtcpsu_v1_9
* @{
*
* This file contains a configuration table that specifies the configuration
* of CAN devices in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date	Changes
* ----- -----  -------- -------------------------------------------------------
* 1.00  kvn  04/21/15 First release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xrtcpsu.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/

/**
 * This table contains configuration information for RTC device
 * in the system.
 */
XRtcPsu_Config XRtcPsu_ConfigTable[XPAR_XRTCPSU_NUM_INSTANCES] = {
	{
		(u16)XPAR_XRTCPSU_0_DEVICE_ID,	/* Unique ID of device */
		(u32)XPAR_XRTCPSU_0_BASEADDR	/* Base address of device */
	}
};
/** @} */
