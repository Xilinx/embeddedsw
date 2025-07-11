/******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
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
* 4.4   sne  03/04/19 Added support for Versal.
* 4.5	sne  09/27/19 Updated example file to support AXI Timebase WDT
*		      and WWDT.
* 5.0	sne  03/11/20 Added XWdtTb_ConfigureWDTMode api to configure
*		      mode.
* 5.7	sb   07/12/23 Added support for system device-tree flow.
* 5.9	ht   07/22/24 Add support for peripheral tests in SDT flow.
* 5.10  ht   10/28/24 Fix compilation warnings in SDT flow peripheral tests.
* 5.11  bkv  07/08/25 Fixed GCC Warning.
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
#ifndef SDT
#define WDTTB_DEVICE_ID		XPAR_WDTTB_0_DEVICE_ID
#endif

/*
 * These constants are user modifiable to enable or disable Secondary Sequence
 * Timer (SST), Program Sequence Monitor (PSM), Fail Counter (FC) and Window
 * WDT protection. They are all enabled by default.
 */
#define WDTTB_EN_SST		0
#define WDTTB_EN_PSM		0
#define WDTTB_EN_FC             0
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
#define WDTTB_SW_COUNT		0x01110000	/**< Number of clock cycles for
						  *  second window */
#define WDTTB_BYTE_COUNT	154		/**< Selected byte count */
#define WDTTB_BYTE_SEGMENT	2		/**< Byte segment selected */
#define WDTTB_SST_COUNT     0x00011000      /**< Number of clock cycles for
                                                      Second sequence Timer */
/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

#ifndef SDT
int WinWdtTbExample(u16 DeviceId);
#else
int WinWdtTbExample(XWdtTb *WdtTbInstancePtr, UINTPTR BaseAddress);
#endif

/************************** Variable Definitions *****************************/

/* The instance of the Watchdog Time Base */
XWdtTb WindowWatchdogTimebase;

/*****************************************************************************/
/**
* Main function to call the example.This function is not included if the
* example is generated from the TestAppGen test tool.
*
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
#ifndef SDT
	Status = WinWdtTbExample(WDTTB_DEVICE_ID);
#else
	Status = WinWdtTbExample(&WindowWatchdogTimebase, XPAR_XWDTTB_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("Window WDT example failed\n\r");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Window WDT example.\n\r");

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
#ifndef SDT
int WinWdtTbExample(u16 DeviceId)
#else
int WinWdtTbExample(XWdtTb *WdtTbInstancePtr, UINTPTR BaseAddress)
#endif
{
	int Status;
	int Count = 0;
	XWdtTb_Config *Config;

#ifdef SDT
	(void)WdtTbInstancePtr;
#endif
	/*
	 * Initialize the WDTTB driver so that it's ready to use look up
	 * configuration in the config table, then initialize it.
	 */
#ifndef SDT
	Config = XWdtTb_LookupConfig(DeviceId);
#else
	Config = XWdtTb_LookupConfig(BaseAddress);
#endif
	if (NULL == Config) {
		return XST_FAILURE;
	}

	/*
	 * Initialize the watchdog timer and timebase driver so that
	 * it is ready to use.
	 */
	Status = XWdtTb_CfgInitialize(&WindowWatchdogTimebase, Config,
				      Config->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#ifndef SDT
	if (!WindowWatchdogTimebase.Config.IsPl) {
#else
	if (!(strcmp(WindowWatchdogTimebase.Config.Name, "xlnx,versal-wwdt-1.0"))) {
#endif
		/*Enable Window Watchdog Feature in WWDT */
		XWdtTb_ConfigureWDTMode(&WindowWatchdogTimebase, XWT_WWDT);
	}

	/*
	 * Perform a self-test to ensure that the hardware was built
	 * correctly
	 */
	Status = XWdtTb_SelfTest(&WindowWatchdogTimebase);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#if (WDTTB_EN_WDP)
	/* Enable extra protection */
	XWdtTb_EnableExtraProtection(&WindowWatchdogTimebase);
#else
	/* Disable extra protection */
	XWdtTb_DisableExtraProtection(&WindowWatchdogTimebase);
#endif /* End of WDP */

	/* Configure first and second window */
	XWdtTb_SetWindowCount(&WindowWatchdogTimebase, WDTTB_FW_COUNT,
			      WDTTB_SW_COUNT);

#if (WDTTB_EN_SST)
	/* Configure Second sequence timer */
#ifndef SDT
	if (!WindowWatchdogTimebase.Config.IsPl) {
#else
	if (!(strcmp(WindowWatchdogTimebase.Config.Name, "xlnx,versal-wwdt-1.0"))) {
#endif
		XWdtTb_SetSSTWindow(&WindowWatchdogTimebase, WDTTB_SST_COUNT);
	}
#endif
	/* Set interrupt position */
	XWdtTb_SetByteCount(&WindowWatchdogTimebase, WDTTB_BYTE_COUNT);
	XWdtTb_SetByteSegment(&WindowWatchdogTimebase, WDTTB_BYTE_SEGMENT);

#if (WDTTB_EN_SST)
	/* Enable Secondary Sequence Timer (SST) */
	XWdtTb_EnableSst(&WindowWatchdogTimebase);
#else
	/* Disable Secondary Sequence Timer (SST) */
	XWdtTb_DisableSst(&WindowWatchdogTimebase);
#endif /* End of SST */

#if (WDTTB_EN_PSM)
	/* Enable Program Sequence Monitor (PSM) */
	XWdtTb_EnablePsm(&WindowWatchdogTimebase);

	/* Write TSR0 with signature */
#ifndef SDT
	if (WindowWatchdogTimebase.Config.IsPl) {
#else
	if (!(strcmp(WindowWatchdogTimebase.Config.Name, "xlnx,versal-wwdt-1.0"))) {
#endif
		XWdtTb_WriteReg(WindowWatchdogTimebase.Config.BaseAddr, XWT_TSR0_OFFSET,
				WDTTB_TSR_VAL);
	} else {
		XWdtTb_WriteReg(WindowWatchdogTimebase.Config.BaseAddr, XWT_TSR0_WWDT_OFFSET,
				WDTTB_TSR_VAL);
	}
#else
	/* Disable Program Sequence Monitor (PSM) */
	XWdtTb_DisablePsm(&WindowWatchdogTimebase);
#endif /* End of PSM */

#if (WDTTB_EN_FC)
	/* Enable Fail Counter */
	XWdtTb_EnableFailCounter(&WindowWatchdogTimebase);
#else
	/* Disable Fail Counter */
	XWdtTb_DisableFailCounter(&WindowWatchdogTimebase);
#endif /* End of FC */

	/*
	 * Start the watchdog timer, the timebase is automatically reset
	 * when this occurs.
	 */
	XWdtTb_Start(&WindowWatchdogTimebase);

	while (1) {
		xil_printf(".");

		/* Check for interrupt programmed point */
		if (XWdtTb_GetIntrStatus(&WindowWatchdogTimebase)) {
			/* Set register space to writable */
			XWdtTb_SetRegSpaceAccessMode(&WindowWatchdogTimebase, 1);

			/* Clear interrupt point */
			XWdtTb_IntrClear(&WindowWatchdogTimebase);

			/* Set register space to read-only */
			XWdtTb_SetRegSpaceAccessMode(&WindowWatchdogTimebase, 0);
#if (WDTTB_EN_PSM)
			/* Set register space to writable */
			XWdtTb_SetRegSpaceAccessMode(&WindowWatchdogTimebase, 1);

			/* Write TSR1 with signature */
#ifndef SDT
			if (WindowWatchdogTimebase.Config.IsPl) {
#else
			if (!(strcmp(WindowWatchdogTimebase.Config.Name, "xlnx,versal-wwdt-1.0"))) {
#endif
				XWdtTb_WriteReg(WindowWatchdogTimebase.Config.BaseAddr,
						XWT_TSR1_OFFSET, WDTTB_TSR_VAL);
			} else {
				XWdtTb_WriteReg(WindowWatchdogTimebase.Config.BaseAddr,
						XWT_TSR1_WWDT_OFFSET, WDTTB_TSR_VAL);
			}
			/* Set register space to read-only */
			XWdtTb_SetRegSpaceAccessMode(&WindowWatchdogTimebase, 0);
#endif
			/* Set register space to writable */
			XWdtTb_SetRegSpaceAccessMode(&WindowWatchdogTimebase, 1);

			/*
			 * Restart the watchdog timer as a normal application
			 * would
			 */
			XWdtTb_RestartWdt(&WindowWatchdogTimebase);
			Count++;
			xil_printf("\n\rRestart kick %d\n\r", Count);
		}

		/* Check for last event */
		if (XWdtTb_GetLastEvent(&WindowWatchdogTimebase) !=
		    XWDTTB_NO_BAD_EVENT) {
			/* Set register space to writable */
			XWdtTb_SetRegSpaceAccessMode(&WindowWatchdogTimebase, 1);

			/* Stop the watchdog timer */
			XWdtTb_Stop(&WindowWatchdogTimebase);
			return XST_FAILURE;
		}

		/*
		 * Check whether the WatchDog timer restarts twice with window
		 * feature. If the timer restarts twice then the test is
		 * passed.
		 */
		if (Count == 2) {
#if (!WDTTB_EN_SST && !WDTTB_EN_FC && !WDTTB_EN_PSM &&!WDTTB_EN_WDP)
			XWdtTb_Stop(&WindowWatchdogTimebase);
#endif
			break;
		}
	}

#if (WDTTB_EN_SST)
	/* Wait for SST counter start */
	xil_printf("Waiting for SST to start .");
	while (!XWdtTb_IsResetPending(&WindowWatchdogTimebase)) {
		xil_printf(".");
	}

	/* Clear reset pending */
	XWdtTb_ClearResetPending(&WindowWatchdogTimebase);
#ifndef SDT
	if (WindowWatchdogTimebase.Config.IsPl) {
#else
	if (!(strcmp(WindowWatchdogTimebase.Config.Name, "xlnx,versal-wwdt-1.0"))) {
#endif
		xil_printf("\n\rSST counter value is 0x%x\n\r",
			   XWdtTb_ReadReg(WindowWatchdogTimebase.Config.BaseAddr,
					  XWT_STR_OFFSET));
	} else {
		xil_printf("\n\rSST counter value is 0x%x\n\r",
			   XWdtTb_ReadReg(WindowWatchdogTimebase.Config.BaseAddr,
					  XWT_STR_WWDT_OFFSET));
	}
#endif
	return XST_SUCCESS;
}
