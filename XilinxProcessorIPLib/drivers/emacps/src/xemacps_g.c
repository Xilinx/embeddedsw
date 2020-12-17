/******************************************************************************
* Copyright (C) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xemacps_g.c
* @addtogroup emacps_v3_13
* @{
*
* This file contains a configuration table that specifies the configuration of
* ethernet devices in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a wsy  01/10/10 First release
* 2.00  hk   22/01/14 Added check for picking second instance
* 3.00  kvn  02/13/15 Modified code for MISRA-C:2012 compliance.
* </pre>
*
* @internal
*
* This configuration table contains entries that are modified at runtime by the
* driver. This table reflects only the hardware configuration of the device.
* This emac configuration table contains software information in addition to
* hardware configuration.
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xemacps.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/

/*
 * The configuration table for emacps device
 */

XEmacPs_Config XEmacPs_ConfigTable[XPAR_XEMACPS_NUM_INSTANCES] = {
	{
		(u16)XPAR_XEMACPS_0_DEVICE_ID,  /* Device ID */
		(UINTPTR)XPAR_XEMACPS_0_BASEADDR,    /* Device base address */
		(u16)XPAR_XEMACPS_0_IS_CACHE_COHERENT,  /* Cache Coherency */
	},
#ifdef XPAR_XEMACPS_1_DEVICE_ID
	{
		(u16)XPAR_XEMACPS_1_DEVICE_ID,  /* Device ID */
		(UINTPTR)XPAR_XEMACPS_1_BASEADDR,    /* Device base address */
		(u16)XPAR_XEMACPS_1_IS_CACHE_COHERENT,  /* Cache coherency */
	}
#endif
};
/** @} */
