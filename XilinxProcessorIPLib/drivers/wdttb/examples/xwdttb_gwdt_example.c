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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
* @file xwdttb_gwdt_example.c
* This file contains a example for  using the Generic Watchdog Timer
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
* 1.0   sne   03/04/19 Initial release for Generic Watchdog Timer.
*
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

int GWdtTbExample(u16 DeviceId);
/************************** Variable Definitions *****************************/

XWdtTb GWatchdog;       /* The instance of the WatchDog Time Base */
#define XWDTPSV_GWCVR0_COUNT1  0x00001000U  /*Generic Watchdog Compare Value Register 0 */
#define XWDTPSV_GWCVR1_COUNT1  0x00001000U  /*Generic Watchdog Compare Value Register 1 */
#define WDTPSV_GWOR_COUNT1     0x00010000U  /*Generic Watchdog offset value*/

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
         * Run the Generic Watchdog  example , specify the device ID that is generated in
         * xparameters.h
         */
        Status = GWdtTbExample(WDTTB_DEVICE_ID);
        if (Status != XST_SUCCESS)
        {
                xil_printf("Generic WdtTb example failed\n\r");
                return XST_FAILURE;
        }
        xil_printf("Generic WdtTb example ran successfully\n\r");

        return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
 *This function tests the functioning of the Generic watch dog Timer Feature
 * in Polled mode
 * In this function polls up to First window completion, once we reached to
 * generic_wdt_interrupt  point it will Refresh.
 * In this function once we reach to Second window time out point
 * It means generating generic_wdt_reset ,We are going to Refresh.
 *
 * @param        DeviceId is the XPAR_<WDTPSV_instance>_DEVICE_ID value from
 *                xparameters.h.
 * @return
 *               - XST_SUCCESS, in window, there is no bad event.
 *               - XST_FAILURE, otherwise.
 *
 * @note         None.
 *
 ****************************************************************************/

int GWdtTbExample(u16 DeviceId)
{
        int Status;
        int Count = 0,RefreshReg=0;
        XWdtTb_Config *Config;

        /*
         * Initialize the WDTPSV driver so that it's ready to use look up
         * configuration in the config table, then initialize it.
         */
        Config = XWdtTb_LookupConfig(DeviceId);
        if (NULL == Config) {
                return XST_FAILURE;
        }

        /*
         * Initialize the watchdog timer  driver so that
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
        xil_printf("\nSelf test completed \n\r");

        /* Update GWCVR0, GWCVR1 and GWOR Register */
        XWdtTb_SetGenericWdtWindow(&GWatchdog,XWDTPSV_GWCVR0_COUNT1, XWDTPSV_GWCVR1_COUNT1,WDTPSV_GWOR_COUNT1);

        /*
         * Start the watchdog timer, the General Watchdog  is automatically reset
         * when this occurs.
         */
        XWdtTb_Start(&GWatchdog);

        /*
         * Verify Whether the Generic WatchDog Refresh Status has been set in the next two
         * expire state.
         */
        while (1) {
                xil_printf(".");
                /* Refresh  General Watchdog timer if generic_wdt_interrupt occurred */
                if(RefreshReg<=2)
                {
                        if(XWdtTb_IsGenericWdtFWExpired(&GWatchdog))
                        {
                                XWdtTb_RestartWdt(&GWatchdog);
                        RefreshReg++;
                        xil_printf("\n Refresh kick%d\n\r",RefreshReg);
                        }
                }
                /*
                 * If the watchdog timer expired, then restart it.
                 */
                if (XWdtTb_IsWdtExpired(&GWatchdog))
                {

                        /*
                         * Restart the watchdog timer as a normal application
                         * would
                         */
                        XWdtTb_RestartWdt(&GWatchdog);
                        Count++;
                        xil_printf("\n\rRestart kick %d\n\r", Count);
                }

                /*
                 * Stop the watchdog timer
                 */
                if(Count ==3)
                {
                        XWdtTb_Stop(&GWatchdog);
                        break;
                }

        }

        return XST_SUCCESS;
}
