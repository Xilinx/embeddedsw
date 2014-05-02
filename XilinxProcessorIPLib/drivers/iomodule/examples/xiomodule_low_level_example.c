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
* @file xiomodule_low_level_example.c
*
* This file contains a design example using the low level-0 driver, interface
* of the IO Module driver.
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
* ----- ---- -------- ---------------------------------------------------------
* 1.00a sa   07/15/11 First release
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xstatus.h"
#include "xiomodule_l.h"
#include "mb_interface.h"


/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define IOMODULE_BASEADDR  XPAR_IOMODULE_SINGLE_BASEADDR
#define IOMODULE_DEVICE_ID XPAR_IOMODULE_SINGLE_DEVICE_ID


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

XStatus IOModuleLowLevelExample(u32 IOModuleBaseAddress);

void SetupInterruptSystem();

void DeviceDriverHandler(void *CallbackRef);


/************************** Variable Definitions *****************************/

/*
 * Create a shared variable to be used by the main thread of processing and
 * the interrupt processing.
 */
volatile static Xunit32 InterruptsProcessed = 0;


/*****************************************************************************/
/**
*
* This is the main function for the IO Module Low Level example.
*
* @param    None.
*
* @return   XST_SUCCESS to indicate success, otherwise XST_FAILURE.
*
* @note     None.
*
******************************************************************************/
int main(void)
{
    XStatus Status;

    /*
     * Run the low level example of the IO Module, specify the Base Address
     * generated in xparameters.h.
     */
    Status = IOModuleLowLevelExample(IOMODULE_BASEADDR);
    if (Status != XST_SUCCESS)
    {
        return XST_FAILURE;
    }

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
* @param    IOModuleBaseAddress is Base Address of the the IO Module Device
*
* @return   XST_SUCCESS to indicate success, otherwise XST_FAILURE
*
* @note     None.
*
******************************************************************************/
XStatus IOModuleLowLevelExample(u32 IOModuleBaseAddress)
{
    /*
     * Connect a device driver handler that will be called when an interrupt
     * for the device occurs, the device driver handler performs the specific
     * interrupt processing for the device
     */
    XIOModule_RegisterHandler(IOModuleBaseAddress, IOMODULE_DEVICE_INTR_ID,
                              (XInterruptHandler)DeviceDriverHandler,
                              (void *)0);
    /*
     * Enable interrupts for all devices that cause interrupts.
     */
    XIOModule_EnableIntr(IOModuleBaseAddress, IOMODULE_DEVICE_INT_MASK);

    /*
     * This step is processor specific, connect the handler for the interrupt
     * controller to the interrupt source for the processor.
     */
    SetupInterruptSystem();

    /*
     * Cause an interrupt so the handler will be called. This is done by
     * writing a 1 to the interrupt status bit for the device interrupt.
     */
    XIOModule_Out32(IOModuleBaseAddress + XIN_ISR_OFFSET,
		    IOMODULE_DEVICE_INT_MASK);

    /*
     * Wait for the interrupt to be processed, if the interrupt does not
     * occur this loop will wait forever.
     */
    while (1)
    {
        /*
         * If the interrupt occurred which is indicated by the global
         * variable which is set in the device driver handler, then
         * stop waiting
         */
        if (InterruptProcessed > 0)
        {
            break;
        }
    }

    return XST_SUCCESS;
}

/*****************************************************************************/
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
******************************************************************************/
void SetupInterruptSystem()
{
    /*
     * Enable the Interrupts on the Microblaze.
     */
    microblaze_enable_interrupts();
}


/*****************************************************************************/
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
******************************************************************************/
void DeviceDriverHandler(void *CallbackRef)
{
    /*
     * Indicate the interrupt has been processed using a shared variable.
     */
    InterruptProcessed++;
}
