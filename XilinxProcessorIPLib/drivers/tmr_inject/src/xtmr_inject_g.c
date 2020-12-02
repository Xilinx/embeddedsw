/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xtmr_inject_g.c
* @addtogroup tmr_inject_v1_3
* @{
*
* This file contains a configuration table that specifies the configuration of
* TMR Inject devices in the system. Each device in the system should have an
* entry in the table.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.0   sa   04/05/17 First release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xtmr_inject.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/

/**
 * The configuration table for TMR Inject devices
 */
XTMR_Inject_Config XTMR_Inject_ConfigTable[XPAR_XTMR_INJECT_NUM_INSTANCES] =
{
	{
		XPAR_TMRINJECT_0_DEVICE_ID,	/* Unique ID of device */
		XPAR_TMRINJECT_0_BASEADDR,	/* Device base address */
		XPAR_TMRINJECT_0_MAGIC,		/* Magic Byte parameter */
		XPAR_TMRINJECT_0_CPU_ID		/* CPU Identifier */
	},
};

/** @} */
