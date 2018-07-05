/******************************************************************************
* Copyright (C) 2003 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xhwicap_g.c
* @addtogroup hwicap_v11_4
* @{
*
* This file contains a configuration table that specifies the configuration of
* Hwicap devices in the system. Each device in the system should have an
* entry in the table.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a bjb  12/08/03 First release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xhwicap.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/

/**
 * The configuration table for opb_hwicap devices
 */
XHwIcap_Config XHwIcap_ConfigTable[XPAR_XHWICAP_NUM_INSTANCES] =
{
    {
        XPAR_HWICAP_0_DEVICE_ID,    /* Unique ID of device */
        XPAR_HWICAP_0_BASEADDR,     /* Device base address */
    },
};


/** @} */
