/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xpacketizer56_sinit.c
*
* This file contains static initialization method for Xilinx VoIP ST2022-6
* Packetizer core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00   mmo   02/12/16 Initial release.

* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xpacketizer56.h"
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
* This function returns a reference to an XPacketizer56_Config structure based
* on the core id, <i>DeviceId</i>. The return value will refer to an entry in
* the device configuration table defined in the xpacketizer56_g.c file.
*
* @param    DeviceId is the unique core ID of the ST2022-6 Packetizer core for
*       the lookup operation.
*
* @return   XPacketizer56_LookupConfig returns a reference to a config record
*       in the configuration table (in xpacketizer56_g.c) corresponding
*       to <i>DeviceId</i>, or NULL if no match is found.
*
* @note     None.
*
******************************************************************************/
XPacketizer56_Config *XPacketizer56_LookupConfig(u16 DeviceId)
{
    extern XPacketizer56_Config
        XPacketizer56_ConfigTable[XPAR_XPACKETIZER56_NUM_INSTANCES];
    XPacketizer56_Config *CfgPtr = NULL;
    u32 Index;

    /* Checking for device id for which instance it is matching */
    for (Index = (u32)0x0; Index < (u32)(XPAR_XPACKETIZER56_NUM_INSTANCES);
                                Index++) {

        /* Assigning address of config table if both device ids
         * are matched
         */
        if (XPacketizer56_ConfigTable[Index].DeviceId == DeviceId) {
            CfgPtr = &XPacketizer56_ConfigTable[Index];
            break;
        }
    }

    return (XPacketizer56_Config *)CfgPtr;
}
