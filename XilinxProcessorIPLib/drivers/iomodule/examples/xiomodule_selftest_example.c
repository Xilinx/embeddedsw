/******************************************************************************
* Copyright (C) 2011 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
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
* 2.4   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
* 2.15  sa   01/05/23 Removed inclusion of mb_interface.h, as it is not
*                     required.
* 2.19  ml   04/18/25 Added support for system device-tree flow.
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xstatus.h"
#include "xiomodule.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place. This definition is not
 * included if the example is generated from the TestAppGen test tool.
 */
#ifndef TESTAPP_GEN
#ifndef SDT
#define	IOMODULE_DEVICE_ID	XPAR_IOMODULE_0_DEVICE_ID
#else
#define	IOMODULE_DEVICE_ID	XPAR_IOMODULE_0_BASEADDR
#endif
#endif

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

XStatus IOModuleSelfTestExample(u32 DeviceId);

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
	xil_printf("Iomodule selftest Example Failed\r\n");
        return XST_FAILURE;
    }

    xil_printf("Successfully ran Iomodule selftest Example\r\n");
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
XStatus IOModuleSelfTestExample(u32 DeviceId)
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
