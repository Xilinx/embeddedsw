/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc. All rights reserved.
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
* @file xwdttb_gwdt_selftest_example.c
*
* This file contains an example for  using the Generic Watchdog Timer
* hardware and driver
*
* @note
*
* None
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.0   sne   02/04/19 Initial release
* </pre>
*
*****************************************************************************/
/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xwdttb.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are only defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define WDTTB_DEVICE_ID         XPAR_WDTTB_0_DEVICE_ID
/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/
int GWdtTbSelfTestExample(u16 DeviceId);
/************************** Variable Definitions *****************************/

XWdtTb GWatchdog; /* The instance of the WatchDog Timer  */
/*****************************************************************************/
/*
 * Main function to call the example.This function is not included if the
 * example is generated from the TestAppGen test tool.
 *
 * @param        None.
 *
 * @return
 *               - XST_SUCCESS if successful.
 *               - XST_FAILURE if unsuccessful.
 *
 * @note         None.
 *
 ******************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
        int Status;

        /*
         * Run the GWDT Self Test example , specify the device ID that is generated in
         * xparameters.h
         */
        Status = GWdtTbSelfTestExample(WDTTB_DEVICE_ID);
        if (Status != XST_SUCCESS){
                xil_printf("GWDT self test example failed\n\r");
                return XST_FAILURE;
        }
        xil_printf("\nGWDT self test example ran successfully\n\r");

        return XST_SUCCESS;
}
#endif
/*****************************************************************************/
/**
 * This function does a minimal test on Generic watchdog timer device and
 * driver as a design example. The purpose of this function is to illustrate
 * how to use the XwdtTb component.
 *
 * @param        DeviceId is the XPAR_<WDTTB_instance>_DEVICE_ID value from
 *               xparameters.h.
 *
 * @return
 *               - XST_SUCCESS if successful.
 *               - XST_FAILURE if unsuccessful.
 *
 * @note         None.
 *
 ****************************************************************************/

int GWdtTbSelfTestExample(u16 DeviceId)
{
        int Status;
        XWdtTb_Config *Config;

        /*
         * Initialize the WDTTB driver so that it's ready to use look up
         * configuration in the config table, then initialize it.
         */
        Config = XWdtTb_LookupConfig(DeviceId);
        if (NULL == Config)
        {
                return XST_FAILURE;
        }
        /*
         * Initialize the watchdog timer and window WDT driver so that
         * it is ready to use.
         */
        Status = XWdtTb_CfgInitialize(&GWatchdog, Config,
                        Config->BaseAddr);
        if (Status != XST_SUCCESS)
        {
                return XST_FAILURE;
        }
        /*
         * Perform a self-test to ensure that the hardware was built
         * correctly
         */
        Status = XWdtTb_SelfTest(&GWatchdog);
        if (Status != XST_SUCCESS)
        {
                return XST_FAILURE;
        }
        /* Reset all the Register of GWDT */
        Status =XWdtTb_Stop(&GWatchdog);
        if (Status != XST_SUCCESS)
        {
                return XST_FAILURE;
        }

        return XST_SUCCESS;
}
