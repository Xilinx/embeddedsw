/******************************************************************************
* Copyright (C) 2007 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xmbox_g.c
* @addtogroup mbox_v4_4
* @{
*
* This file contains a configuration table that specifies the configuration
* of Mbox devices in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 1.00a ecm  06/01/07 Cleanup, new coding standard, check into XCS
* </pre>
*
* @note
*
* None.
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xmbox.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Prototypes ******************************/

XMbox_Config XMbox_ConfigTable[] =
{
	{
		XPAR_XMBOX_0_DEVICE_ID,
		XPAR_XMBOX_0_BASEADDR,
		XPAR_XMBOX_0_USE_FSL,
		XPAR_XMBOX_0_SEND_ID,
		XPAR_XMBOX_0_RECV_ID
	}
};
/** @} */
