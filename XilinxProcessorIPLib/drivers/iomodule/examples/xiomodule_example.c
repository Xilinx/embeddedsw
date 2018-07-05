/******************************************************************************
*
* Copyright (C) 2011 - 2015 Xilinx, Inc.  All rights reserved.
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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/******************************************************************************/
/**
*
* @file xiomodule_example.c
*
* This file contains a self test example using the IO Module driver
* (XIoModule) and hardware device. Please reference other device driver
* examples to see more examples of how the interrupts, timers and UART can be
* used by a software application.
*
* @note
*
* None
*
* <pre>
*
* MODIFICATION HISTORY:
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------
* 1.00a sa   07/15/11 First release
* 2.4   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xstatus.h"
#include "xiomodule.h"
#include "xil_exception.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define IOMODULE_DEVICE_ID XPAR_IOMODULE_0_DEVICE_ID


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

XStatus IoModuleExample(u16 DeviceId);

XStatus SetUpInterruptSystem(XIOModule *XIoModuleInstancePtr);

void DeviceDriverHandler(void *CallbackRef);


/************************** Variable Definitions *****************************/

static XIOModule IOModule; /* Instance of the IO Module */

/*
 * Create a shared variable to be used by the main thread of processing and
 * the interrupt processing.
 */
volatile static u32 InterruptProcessed = 0;


/*****************************************************************************/
/**
*
* This is the main function for the IO Module example .
*
* @param    None.
*
* @return   XST_SUCCESS to indicate success, otherwise XST_FAILURE.
*
* @note     None.
*
****************************************************************************/
int main(void)
{
    XStatus Status;

    /*
     *  Run the example , specify the Device ID generated in xparameters.h
     */
    Status = IOModuleExample(IOMODULE_DEVICE_ID);
    if (Status != XST_SUCCESS)
    {
		xil_printf("Iomodule Example Failed\r\n");
        return XST_FAILURE;
    }

	xil_printf("Successfully ran Iomodule Example\r\n");
    return XST_SUCCESS;
}


/*****************************************************************************/
/**
*
* This function is an example of how to use the IO Module driver component
* (XIOModule) and the hardware device.  This function is designed to work
* without external hardware devices to cause interrupts.  It may not return if
* the IO Module is not properly connected to the processor in either software
* or hardware.
*
* @param    DeviceId is device ID of the IO Module Device, typically
*           XPAR_<IOMODULE_instance>_DEVICE_ID value from xparameters.h
*
* @return   XST_SUCCESS to indicate success, otherwise XST_FAILURE
*
* @note     None.
*
******************************************************************************/
XStatus IOModuleExample(u16 DeviceId)
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

    /*
     * Setup the Interrupt System.
     */
    Status = SetUpInterruptSystem(&IOModule);
    if (Status != XST_SUCCESS)
    {
        return XST_FAILURE;
    }

    /*
     * Generate the interrupts.
     */
    Status = XOModule_GenerateIntr(&IOModule);
    if (Status != XST_SUCCESS)
    {
        return XST_FAILURE;
    }

    /*
     * Wait for the interrupts to be processed, if no interrupt occurs this
     * loop will wait forever.
     */
    while (1)
    {
        /*
         * If the interrupts occurred which is indicated by the global
         * variable which is set in the device driver handler, then
         * stop waiting
         */
        if (InterruptProcessed)
        {
            break;
        }
    }

    return XST_SUCCESS;
}

/******************************************************************************/
/**
*
* This function connects the interrupt handler of the IO Module to the
* processor.  This function is separate to allow it to be customized for each
* application.  Each processor or RTOS may require unique processing to connect
* the interrupt handler.
*
* @param    None.
*
* @return   None.
*
* @note     None.
*
****************************************************************************/
XStatus SetUpInterruptSystem(XIOModule *XIOModuleInstancePtr)
{
    XStatus Status;

    /*
     * Connect a device driver handler that will be called when an interrupt
     * for the device occurs, the device driver handler performs the specific
     * interrupt processing for the device
     */
    Status = XIOModule_Connect(XIOModuleInstancePtr, IOMODULE_DEVICE_ID,
                               (XInterruptHandler) DeviceDriverHandler,
                               (void *)0);
    if (Status != XST_SUCCESS)
    {
        return XST_FAILURE;
    }

    /*
     * Start the IO Module such that interrupts are enabled for all devices
     * that cause interrupts.
     */
    Status = XIOModule_Start(XIOModuleInstancePtr);
    if (Status != XST_SUCCESS)
    {
        return XST_FAILURE;
    }

    /*
     * Enable interrupts for the device and then cause interrupts so the
     * handlers will be called.
     */
    XIOModule_Enable(XIOModuleInstancePtr, IOMODULE_DEVICE_ID);

    /*
     * Initialize the exception table.
     */
    Xil_ExceptionInit();

    /*
     * Register the IO module interrupt handler with the exception table.
     */
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
		 (Xil_ExceptionHandler)XIOModule_DeviceInterruptHandler,
		 (void*) 0);

    /*
     * Enable exceptions.
     */
    Xil_ExceptionEnable();

    return XST_SUCCESS;
}

/******************************************************************************/
/**
*
* This function is designed to look like an interrupt handler in a device
* driver. This is typically a 2nd level handler that is called from the
* IO Module interrupt handler.  This handler would typically perform device
* specific processing such as reading and writing the registers of the device
* to clear the interrupt condition and pass any data to an application using
* the device driver.  Many drivers already provide this handler and the user
* is not required to create it.
*
* @param    CallbackRef is passed back to the device driver's interrupt handler
*           by the XIOModule driver.  It was given to the XIOModule driver in
*           the XIOModule_Connect() function call.  It is typically a pointer
*           to the device driver instance variable if using the Xilinx Level 1
*           device drivers.  In this example, we do not care about the callback
*           reference, so we passed it a 0 when connecting the handler to the
*           XIOModule driver and we make no use of it here.
*
* @return   None.
*
* @note     None.
*
****************************************************************************/
void DeviceDriverHandler(void *CallbackRef)
{
    /*
     * Indicate the interrupt has been processed using a shared variable.
     */
    InterruptsProcessed++;
}
