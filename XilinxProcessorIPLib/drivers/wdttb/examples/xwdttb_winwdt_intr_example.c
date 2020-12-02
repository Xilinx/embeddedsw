/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xwdttb_winwdt_intr_example.c
*
* This file contains a design example using the TimeBase Watchdog Timer Device
* (WdtTb) driver and hardware device using interrupt mode (for the WDT
* interrupt).
*
* @note
*
* This example assumes that the reset output of the WdtTb device is not
* connected to the reset of the processor. This example will not return
* if the interrupts are not working.
*
* MODIFICATION HISTORY:
*
* <pre>
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 4.0   sha  02/04/16 First release
* 5.0	sne  03/26/20 Updated example to support versal platform.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xwdttb.h"
#include "xil_exception.h"

#ifdef XPAR_INTC_0_DEVICE_ID
#include "xintc.h"
#include <stdio.h>
#else
#include "xscugic.h"
#include "xil_printf.h"
#endif

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are only defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef TESTAPP_GEN
#define WDTTB_DEVICE_ID         XPAR_WDTTB_0_DEVICE_ID
#endif

#ifdef XPAR_INTC_0_DEVICE_ID
 #define INTC_DEVICE_ID         XPAR_INTC_0_DEVICE_ID
 #define WDTTB_IRPT_INTR         XPAR_INTC_0_WDTTB_0_VEC_ID
#else
 #define INTC_DEVICE_ID         XPAR_SCUGIC_SINGLE_DEVICE_ID
 #ifdef versal
  #define WDTTB_IRPT_INTR       XPAR_XWWDT_0_INTR
 #else
  #define WDTTB_IRPT_INTR       XPAR_FABRIC_WDTTB_0_VEC_ID
 #endif
#endif /* XPAR_INTC_0_DEVICE_ID */

#define WIN_WDT_SW_COUNT	0xF00000	/**< Number of clock cycles for
						  *  second window */
#define WIN_WDT_SBC_COUNT	16		/**< Selected byte count */
#define WIN_WDT_BSS_COUNT	2		/**< Byte segment selected */

/**************************** Type Definitions *******************************/

#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC			XIntc
#define INTC_HANDLER		XIntc_InterruptHandler
#else
#define INTC			XScuGic
#define INTC_HANDLER		XScuGic_InterruptHandler
#endif /* XPAR_INTC_0_DEVICE_ID */

/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int WinWdtIntrExample(INTC *IntcInstancePtr,
			XWdtTb *WdtTbInstancePtr,
			u16 WdtTbDeviceId,
			u16 WdtTbIntrId);

static void WdtTbIntrHandler(void *CallBackRef);
static int WdtTbSetupIntrSystem(INTC *IntcInstancePtr,
				XWdtTb *WdtTbInstancePtr,
				u16 WdtTbIntrId);
static void WdtTbDisableIntrSystem(INTC *IntcInstancePtr,
				u16 WdtTbIntrId);

/************************** Variable Definitions *****************************/

#ifndef TESTAPP_GEN
XWdtTb WdtTbInstance;	/* Instance of Time Base WatchDog Timer */
INTC IntcInstance;	/* Instance of the Interrupt Controller */
#endif

static volatile int WdtExpired;

/*****************************************************************************/
/**
* Main function to call the WdtTb interrupt example.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if successful
*		- XST_FAILURE, otherwise.
*
* @note		None.
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	int Status;

	/*
	 * Call the WdtTb interrupt example, specify the parameters generated in
	 * xparameters.h
	 */
	Status = WinWdtIntrExample(&IntcInstance,
				&WdtTbInstance,
				WDTTB_DEVICE_ID,
				WDTTB_IRPT_INTR);
	if (Status != XST_SUCCESS) {
		xil_printf("Window WDT interrupt example failed.\n\r");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Window WDT interrupt example.\n\r");
	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* This function tests the functioning of the Window Watchdog Timer in the
* interrupt mode.
*
* This function waits for interrupt programmed point in second window. If the
* interrupt has occurred, interrupt handler sets a flag and restarts the timer.
* This function then clears the interrupt, flag and waits for second interrupt
* to occur and continue waiting for second interrupt as mentioned above.
*
* This function assumes that the reset output of the Window Watchdog Timer
* is not connected to the reset of the processor. The function allows the
* Window Watchdog Timer to timeout such that a reset will occur if it is
* connected.
*
* @param	IntcInstancePtr is a pointer to the instance of the INTC
*		driver.
* @param	WdtTbInstancePtr is a pointer to the instance of WdtTb driver.
* @param	WdtTbDeviceId is the Device ID of the WdtTb Device and is
*		typically XPAR_<WDTTB_instance>_DEVICE_ID value from
*		xparameters.h.
* @param	WdtTbIntrId is the Interrupt Id of the WatchDog and is
*		typically XPAR_<INTC_instance>_<WDTTB_instance>_WDT_
*		INTERRUPT_VEC_ID value from xparameters.h.
*
* @return
*		- XST_SUCCESS if interrupt example run successfully.
*		- XST_FAILURE, if reset has occurred.
*
* @note		None.
*
******************************************************************************/
int WinWdtIntrExample(INTC *IntcInstancePtr,
			XWdtTb *WdtTbInstancePtr,
			u16 WdtTbDeviceId,
			u16 WdtTbIntrId)
{
	int Status;
	XWdtTb_Config *Config;

	/*
	 * Initialize the WDTTB driver so that it's ready to use look up
	 * configuration in the config table, then initialize it.
	 */
	Config = XWdtTb_LookupConfig(WdtTbDeviceId);
	if (NULL == Config) {
		return XST_FAILURE;
	}

	/*
	 * Initialize the watchdog timer and timebase driver so that
	 * it is ready to use.
	 */
	Status = XWdtTb_CfgInitialize(WdtTbInstancePtr, Config,
				      Config->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	if(!WdtTbInstancePtr->Config.IsPl) {
		/*Enable Window Watchdog Feature in WWDT */
		XWdtTb_ConfigureWDTMode(WdtTbInstancePtr, XWT_WWDT);
	}

	/*
	 * Perform a self-test to ensure that the hardware was built correctly
	 */
	Status = XWdtTb_SelfTest(WdtTbInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the WdtTb to the interrupt subsystem so that interrupts
	 * can occur
	 */
	Status = WdtTbSetupIntrSystem(IntcInstancePtr,
					WdtTbInstancePtr,
					WdtTbIntrId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Set register space to writable */
	XWdtTb_SetRegSpaceAccessMode(WdtTbInstancePtr, 1);

	/* Configure first and second window */
	XWdtTb_SetWindowCount(WdtTbInstancePtr, 0, WIN_WDT_SW_COUNT);

	/* Set interrupt position */
	XWdtTb_SetByteCount(WdtTbInstancePtr, WIN_WDT_SBC_COUNT);
	XWdtTb_SetByteSegment(WdtTbInstancePtr, WIN_WDT_BSS_COUNT);

	/* Disable Secondary Sequence Timer (SST) */
	XWdtTb_DisableSst(WdtTbInstancePtr);

	/* Disable Program Sequence Monitor (PSM) */
	XWdtTb_DisablePsm(WdtTbInstancePtr);

	/* Disable Fail Counter */
	XWdtTb_DisableFailCounter(WdtTbInstancePtr);

	/*
	 * Start the watchdog timer as a normal application would
	 */
	XWdtTb_Start(WdtTbInstancePtr);
	WdtExpired = FALSE;

	/* Set register space to writable */
	XWdtTb_SetRegSpaceAccessMode(WdtTbInstancePtr, 1);

	/*
	 * Wait for the first occurrence of interrupt programmed point.
	 */
	while (WdtExpired != TRUE);


	/* Clear interrupt point */
	XWdtTb_IntrClear(WdtTbInstancePtr);
	WdtExpired = FALSE;

	/* Wait for the second occurrence of interrupt programmed point */
	while (WdtExpired != TRUE);

	/* Clear interrupt point */
	XWdtTb_IntrClear(WdtTbInstancePtr);
	WdtExpired = FALSE;

	/* Check for last event */
	if (XWdtTb_GetLastEvent(WdtTbInstancePtr) != XWDTTB_NO_BAD_EVENT) {
		/* Disable and disconnect the interrupt system */
		WdtTbDisableIntrSystem(IntcInstancePtr, WdtTbIntrId);

		/* Stop the timer */
		XWdtTb_Stop(WdtTbInstancePtr);
		return XST_FAILURE;
	}

	/* Disable and disconnect the interrupt system */
	WdtTbDisableIntrSystem(IntcInstancePtr, WdtTbIntrId);

	/* Stop the timer */
	XWdtTb_Stop(WdtTbInstancePtr);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function setups the interrupt system such that WDT interrupt can occur
* for the WdtTb. This function is application specific since the actual
* system may or may not have an interrupt controller. The WdtTb device could be
* directly connected to a processor without an interrupt controller. The
* user should modify this function to fit the application.
*
* @param	IntcInstancePtr is a pointer to the instance of the Intc
*		driver.
* @param	WdtTbInstancePtr is a pointer to the instance of WdtTb driver.
* @param	WdtTbIntrId is the Interrupt Id of the WDT interrupt and is
*		typically
*		XPAR_<INTC_instance>_<WDTTB_instance>_WDT_INTERRUPT_VEC_ID
*		value from xparameters.h.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE, otherwise.
*
* @note		None.
*
******************************************************************************/
static int WdtTbSetupIntrSystem(INTC *IntcInstancePtr,
				XWdtTb *WdtTbInstancePtr,
				u16 WdtTbIntrId)
{
	int Status;
#ifdef XPAR_INTC_0_DEVICE_ID
#ifndef TESTAPP_GEN
	/*
	 * Initialize the interrupt controller driver so that
	 * it's ready to use. Specify the device ID that is generated in
	 * xparameters.h
	 */
	Status = XIntc_Initialize(IntcInstancePtr, INTC_DEVICE_ID);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif /* TESTAPP_GEN */

	/*
	 * Connect a device driver handler that will be called when the WDT
	 * interrupt for the device occurs, the device driver handler performs
	 * the specific interrupt processing for the device
	 */
	Status = XIntc_Connect(IntcInstancePtr, WdtTbIntrId,
			(XInterruptHandler)WdtTbIntrHandler,
			(void *)WdtTbInstancePtr);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#ifndef TESTAPP_GEN
	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts
	 */
	Status = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif /* TESTAPP_GEN */

	/* Enable the WDT interrupt of the WdtTb Device */
	XIntc_Enable(IntcInstancePtr, WdtTbIntrId);
#else

#ifndef TESTAPP_GEN
	XScuGic_Config *IntcConfig;

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
					IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif /* TESTAPP_GEN */

	XScuGic_SetPriorityTriggerType(IntcInstancePtr, WdtTbIntrId,
					0xA0, 0x3);

	/*
	 * Connect the interrupt handler that will be called when an
	 * interrupt occurs for the device.
	 */
	Status = XScuGic_Connect(IntcInstancePtr, WdtTbIntrId,
				(Xil_ExceptionHandler)WdtTbIntrHandler,
				WdtTbInstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/* Enable the interrupt for the Timer device */
	XScuGic_Enable(IntcInstancePtr, WdtTbIntrId);
#endif /* XPAR_INTC_0_DEVICE_ID */

#ifndef TESTAPP_GEN
	/* Initialize the exception table */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			(Xil_ExceptionHandler)INTC_HANDLER,
			IntcInstancePtr);

	/* Enable non-critical exceptions */
	Xil_ExceptionEnable();

#endif /* TESTAPP_GEN */

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is the Interrupt handler for the WDT Interrupt of the
* WdtTb device. It is called when reached to the interrupt programmed point
* and is called from an interrupt context.
*
* @param	CallBackRef is a pointer to the callback reference.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void WdtTbIntrHandler(void *CallBackRef)
{
	XWdtTb *WdtTbInstancePtr = (XWdtTb *)CallBackRef;

	/* Set the flag indicating that the WDT has expired */
	WdtExpired = TRUE;

	/* Restart the watchdog timer as a normal application would */
	XWdtTb_RestartWdt(WdtTbInstancePtr);
}

/*****************************************************************************/
/**
*
* This function disables the interrupts that occur for the WdtTb.
*
* @param	IntcInstancePtr is the pointer to the instance of INTC driver.
* @param	WdtTbIntrId is the WDT Interrupt Id and is typically
*		XPAR_<INTC_instance>_<WDTTB_instance>_WDT_INTERRUPT_VEC_ID
*		value from xparameters.h.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void WdtTbDisableIntrSystem(INTC *IntcInstancePtr, u16 WdtTbIntrId)
{

	/* Disconnect and disable the interrupt for the WdtTb */
#ifdef XPAR_INTC_0_DEVICE_ID
	XIntc_Disconnect(IntcInstancePtr, WdtTbIntrId);
#else
	/* Disconnect the interrupt */
	XScuGic_Disable(IntcInstancePtr, WdtTbIntrId);
	XScuGic_Disconnect(IntcInstancePtr, WdtTbIntrId);

#endif
}
