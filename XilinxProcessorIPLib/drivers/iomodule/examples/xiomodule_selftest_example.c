/* $Id$ */
/******************************************************************************
*
* (c) Copyright 2011 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
******************************************************************************/
/******************************************************************************/
/**
*
* @file xiomodule_selftest_example.c
*
* This file contains a self test example using the IO Module driver
* (XIOModule) and hardware device. Please reference other device driver
* examples to see more examples of how the interrupts, timers and UART can be
* used by a software application.
*
* The TestApp Gen utility uses this file to perform the self test and setup
* of the IO Module.
*
* @note
*
* None
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- --------------------------------------------------------
* 1.00a sa   07/15/11 First release
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xstatus.h"
#include "xiomodule.h"
#include "mb_interface.h"


/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place. This definition is not
 * included if the example is generated from the TestAppGen test tool.
 */
#ifndef TESTAPP_GEN
#define IOMODULE_DEVICE_ID XPAR_IOMODULE_0_DEVICE_ID
#endif

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

XStatus IOModuleSelfTestExample(u16 DeviceId);

/************************** Variable Definitions *****************************/

static XIOModule IOModule; /* Instance of the IO Module */


/*****************************************************************************/
/**
*
* This is the main function for the IO Module example. This function is not
* included if the example is generated from the TestAppGen test tool.
*
* @param    None.
*
* @return   XST_SUCCESS to indicate success, otherwise XST_FAILURE.
*
* @note     None.
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
    XStatus Status;

    /*
     *  Run the example, specify the Device ID generated in xparameters.h
     */
    Status = IOModuleSelfTestExample(IOMODULE_DEVICE_ID);
    if (Status != XST_SUCCESS)
    {
        return XST_FAILURE;
    }

    return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* This function runs a self-test on the driver/device. This is a destructive
* test. This function is an example of how to use the IO Module driver
* component (XIOModule) and the hardware device.  This function is designed
* to work without external hardware devices to cause interrupts.  It may not
* return if the IO Module is not properly connected to the processor in
* either software or hardware.
*
* @param    DeviceId is device ID of the IO Module Device, typically
*           XPAR_<IOMODULE_instance>_DEVICE_ID value from xparameters.h
*
* @return   XST_SUCCESS to indicate success, otherwise XST_FAILURE
*
* @note     None.
*
******************************************************************************/
XStatus IOModuleSelfTestExample(u16 DeviceId)
{
    XStatus Status;

    /*
     * Initialize the IO Module driver so that it is ready to use.
     */
    Status = XIOModule_Initialize(&IOModule, DeviceId);
    if (Status != XST_SUCCESS)
    {
        return XST_FAILURE;
    }

    /*
     * Perform a self-test to ensure that the hardware was built correctly.
     */
    Status = XIOModule_SelfTest(&IOModule);
    if (Status != XST_SUCCESS)
    {
        return XST_FAILURE;
    }

    return XST_SUCCESS;
}
