/******************************************************************************
* Copyright (C) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xrtcpsu_g.c
* @brief Provides the configuration table for RTC PS devices.
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
* 1.18  vn   09/30/26 Align Doxygen comments with AMD standards
* </pre>
*
******************************************************************************/

/** @addtogroup rtcpsu Overview
 * @{
 */

/***************************** Include Files *********************************/

#include "xrtcpsu.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/

/**
 * @brief Holds the configuration information for the RTC device
 * in the system.
 */
XRtcPsu_Config XRtcPsu_ConfigTable[XPAR_XRTCPSU_NUM_INSTANCES] = {
	{
		(u16)XPAR_XRTCPSU_0_DEVICE_ID,	/* Unique ID of device */
		(u32)XPAR_XRTCPSU_0_BASEADDR	/* Base address of device */
	}
};
/** @} */
