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
* @file xwdttb_winwdt_example.c
*
* This file contains a design example using the Watchdog Timer Timebase
* (XWdtTb) driver with window feature. The WDTTB core will be generated either
* in legacy or window feature.
*
* @note
*
* None
*
* MODIFICATION HISTORY:
*
* <pre>
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 4.0   sha  02/04/16 First release.
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
#define WDTTB_DEVICE_ID		XPAR_WDTTB_0_DEVICE_ID

/*
 * These constants are user modifiable to enable or disable Secondary Sequence
 * Timer (SST), Program Sequence Monitor (PSM), Fail Counter (FC) and Window
 * WDT protection. They are all enabled by default.
 */
#define WDTTB_EN_SST		0
#define WDTTB_EN_PSM		0
#define WDTTB_EN_FC		0
#define WDTTB_EN_WDP		0

/*
 * This constant is used as Task Status Register (TSR) value for additional
 * check when PSM is enabled.
 */
#define WDTTB_TSR_VAL		'W'

/*
 * These constants are user modifiable and provide values to configure Window
 * Watchdog Timer.
 * 1. First Window Count
 * 2. Second Window Count
 * 3. Selected Byte Count
 * 4. Byte Segment Selection
 */
#define WDTTB_FW_COUNT		31		/**< Number of clock cycles for
						  *  first window */
#define WDTTB_SW_COUNT		1000000000	/**< Number of clock cycles for
						  *  second window */
#define WDTTB_BYTE_COUNT	154		/**< Selected byte count */
#define WDTTB_BYTE_SEGMENT	2		/**< Byte segment selected */

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int WinWdtTbExample(u16 DeviceId);

/************************** Variable Definitions *****************************/

/* The instance of the Watchdog Time Base */
XWdtTb WatchdogTimebase;

/*****************************************************************************/
/**
* Main function to call the example.This function is not included if the
* example is generated from the TestAppGen test tool.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if unsuccessful.
*
* @note		None.
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	int Status;

	/*
	 * Call the example, specify the device ID that is generated
	 * in xparameters.h
	 */
	Status = WinWdtTbExample(WDTTB_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Window WDT example failed\n\r");
		return XST_FAILURE;
	}

	xil_printf("Window WDT example ran successfully\n\r");

	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
* This function tests the functioning of the TimeBase WatchDog Timer module
* with window feature in the polled mode.
*
* In window, this function polls interrupt programmed point in second window,
* checks the interrupt bit is set. If the bit is set clears the bit and restart
* the watchdog timer. If bit is not cleared before overflowing the second
* window, the watchdog timer resets.
*
* This function may require some time (seconds or even minutes) to execute
* because it waits for the watchdog timer to expire.
*
* @param	DeviceId is the XPAR_<WDTB_instance>_DEVICE_ID value from
*		xparameters.h.
*
* @return
*		- XST_SUCCESS, in window, there is no bad event.
*		- XST_FAILURE, otherwise.
*
* @note		None.
*
****************************************************************************/
int WinWdtTbExample(u16 DeviceId)
{
	int Status;
	int Count = 0;
	XWdtTb_Config *Config;

	/*
	 * Initialize the WDTTB driver so that it's ready to use look up
	 * configuration in the config table, then initialize it.
	 */
	Config = XWdtTb_LookupConfig(DeviceId);
	if (NULL == Config) {
		return XST_FAILURE;
	}

	/*
	 * Initialize the watchdog timer and timebase driver so that
	 * it is ready to use.
	 */
	Status = XWdtTb_CfgInitialize(&WatchdogTimebase, Config,
			Config->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built
	 * correctly
	 */
	Status = XWdtTb_SelfTest(&WatchdogTimebase);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#if (WDTTB_EN_WDP)
	/* Enable extra protection */
	XWdtTb_EnableExtraProtection(&WatchdogTimebase);
#else
	/* Disable extra protection */
	XWdtTb_DisableExtraProtection(&WatchdogTimebase);
#endif /* End of WDP */

	/* Configure first and second window */
	XWdtTb_SetWindowCount(&WatchdogTimebase, WDTTB_FW_COUNT,
		WDTTB_SW_COUNT);

	/* Set interrupt position */
	XWdtTb_SetByteCount(&WatchdogTimebase, WDTTB_BYTE_COUNT);
	XWdtTb_SetByteSegment(&WatchdogTimebase, WDTTB_BYTE_SEGMENT);

#if (WDTTB_EN_SST)
	/* Enable Secondary Sequence Timer (SST) */
	XWdtTb_EnableSst(&WatchdogTimebase);
#else
	/* Disable Secondary Sequence Timer (SST) */
	XWdtTb_DisableSst(&WatchdogTimebase);
#endif /* End of SST */

#if (WDTTB_EN_PSM)
	/* Enable Program Sequence Monitor (PSM) */
	XWdtTb_EnablePsm(&WatchdogTimebase);

	/* Write TSR0 with signature */
	XWdtTb_WriteReg(WatchdogTimebase.Config.BaseAddr, XWT_TSR0_OFFSET,
		WDTTB_TSR_VAL);
#else
	/* Disable Program Sequence Monitor (PSM) */
	XWdtTb_DisablePsm(&WatchdogTimebase);
#endif /* End of PSM */

#if (WDTTB_EN_FC)
	/* Enable Fail Counter */
	XWdtTb_EnableFailCounter(&WatchdogTimebase);
#else
	/* Disable Fail Counter */
	XWdtTb_DisableFailCounter(&WatchdogTimebase);
#endif /* End of FC */

	/*
	 * Start the watchdog timer, the timebase is automatically reset
	 * when this occurs.
	 */
	XWdtTb_Start(&WatchdogTimebase);

	while (1) {
		xil_printf(".");

		/* Check for interrupt programmed point */
		if (XWdtTb_GetIntrStatus(&WatchdogTimebase)) {
			/* Set register space to writable */
			XWdtTb_SetRegSpaceAccessMode(&WatchdogTimebase, 1);

			/* Clear interrupt point */
			XWdtTb_IntrClear(&WatchdogTimebase);

			/* Set register space to read-only */
			XWdtTb_SetRegSpaceAccessMode(&WatchdogTimebase, 0);
#if (WDTTB_EN_PSM)
			/* Set register space to writable */
			XWdtTb_SetRegSpaceAccessMode(&WatchdogTimebase, 1);

			/* Write TSR1 with signature */
			XWdtTb_WriteReg(WatchdogTimebase.Config.BaseAddr,
				XWT_TSR1_OFFSET, WDTTB_TSR_VAL);

			/* Set register space to read-only */
			XWdtTb_SetRegSpaceAccessMode(&WatchdogTimebase, 0);
#endif
			/* Set register space to writable */
			XWdtTb_SetRegSpaceAccessMode(&WatchdogTimebase, 1);

			/*
			 * Restart the watchdog timer as a normal application
			 * would
			 */
			XWdtTb_RestartWdt(&WatchdogTimebase);
			Count++;
			xil_printf("\n\rRestart kick %d\n\r", Count);
		}

		/* Check for last event */
		if (XWdtTb_GetLastEvent(&WatchdogTimebase) !=
				XWDTTB_NO_BAD_EVENT) {
			/* Set register space to writable */
			XWdtTb_SetRegSpaceAccessMode(&WatchdogTimebase, 1);

			/* Stop the watchdog timer */
			XWdtTb_Stop(&WatchdogTimebase);
			return XST_FAILURE;
		}

		/*
		 * Check whether the WatchDog timer restarts twice with window
		 * feature. If the timer restarts twice then the test is
		 * passed.
		 */
		if(Count == 2) {
			break;
		}
	}

#if (WDTTB_EN_SST)
	/* Wait for SST counter start */
	xil_printf("Waiting for SST to start .");
	while (!XWdtTb_IsResetPending(&WatchdogTimebase)) {
		xil_printf(".");
	}

	/* Clear reset pending */
	XWdtTb_ClearResetPending(&WatchdogTimebase);
	xil_printf("\n\rSST counter value is 0x%x\n\r",
		XWdtTb_ReadReg(WatchdogTimebase.Config.BaseAddr,
			XWT_STR_OFFSET));
#endif
	return XST_SUCCESS;
}
