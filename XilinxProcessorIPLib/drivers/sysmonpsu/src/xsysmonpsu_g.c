/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsysmonpsu_g.c
* @addtogroup sysmonpsu_v2_6
*
* This file contains a configuration table that specifies the configuration
* of SYSMON devices in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date	Changes
* ----- -----  -------- -------------------------------------------------------
* 1.0   kvn  04/21/15 First release.
* 2.5   mn     07/06/18 Added Input Clock Frequency Information
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xsysmonpsu.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/

/**
 * This table contains configuration information for SYSMON device
 * in the system.
 */
XSysMonPsu_Config XSysMonPsu_ConfigTable[XPAR_XSYSMONPSU_NUM_INSTANCES] = {
	{
		(u16)XPAR_XSYSMONPSU_0_DEVICE_ID,/* Unique ID of device */
		(u32)XPAR_XSYSMONPSU_0_BASEADDR, /* Base address of device */
		XPAR_XSYSMONPSU_0_REF_FREQMHZ    /* Input Clock Frequency */
	}
};
