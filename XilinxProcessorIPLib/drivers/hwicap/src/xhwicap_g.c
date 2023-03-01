/******************************************************************************
* Copyright (C) 2003 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xhwicap_g.c
* @addtogroup hwicap Overview
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
* 11.5  Nava 09/30/22 Added new IDCODE's as mentioned in the ug570 Doc.
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
