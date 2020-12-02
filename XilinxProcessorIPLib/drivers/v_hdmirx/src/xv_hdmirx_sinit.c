/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hdmirx_sinit.c
*
* This file contains static initialization method for Xilinx HDMI RX core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.0   gm, mg 10/07/15 Initial release.

* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xv_hdmirx.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function returns a reference to an XV_HdmiRx_Config structure based
* on the core id, <i>DeviceId</i>. The return value will refer to an entry in
* the device configuration table defined in the xv_hdmirx_g.c file.
*
* @param    DeviceId is the unique core ID of the HDMI RX core for the
*       lookup operation.
*
* @return   XV_HdmiRx_LookupConfig returns a reference to a config record
*       in the configuration table (in xv_hdmirx_g.c) corresponding
*       to <i>DeviceId</i>, or NULL if no match is found.
*
* @note     None.
*
******************************************************************************/
XV_HdmiRx_Config *XV_HdmiRx_LookupConfig(u16 DeviceId)
{
    extern XV_HdmiRx_Config
            XV_HdmiRx_ConfigTable[XPAR_XV_HDMIRX_NUM_INSTANCES];
    XV_HdmiRx_Config *CfgPtr = NULL;
    u32 Index;

    /* Checking for device id for which instance it is matching */
    for (Index = (u32)0x0; Index < (u32)(XPAR_XV_HDMIRX_NUM_INSTANCES);
                                Index++) {

        /* Assigning address of config table if both device ids
         * are matched
         */
        if (XV_HdmiRx_ConfigTable[Index].DeviceId == DeviceId) {
            CfgPtr = &XV_HdmiRx_ConfigTable[Index];
            break;
        }
    }

    return (XV_HdmiRx_Config *)CfgPtr;
}
