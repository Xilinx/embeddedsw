/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
        xil_printf("Successfully ran GWDT self test example\n\r");

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
