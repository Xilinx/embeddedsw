/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xscuwdt_intr_example.c
*
* This file contains a design example using the Xilinx SCU Private Watchdog
* Timer driver (XScuWdt) and hardware in Timer mode using interrupts.
*
* @note		None.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------
* 1.00a sdm  01/15/10 First release
* 2.5   asa  07/18/23 Added support for system device tree based
*                     workflow decouplig flow.
*					  Interrupt wrapper support has also been added.
* 2.5   dp   09/08/23 Update example to stop wdt at end of the test
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xscuwdt.h"
#include "xil_exception.h"
#include "xil_printf.h"
#ifndef SDT
#include "xscugic.h"
#else
#include "xinterrupt_wrap.h"
#endif

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are only defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define WDT_DEVICE_ID		XPAR_SCUWDT_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define WDT_IRPT_INTR		XPAR_SCUWDT_INTR
#else
#define SCUWDT_BASEADDRESS  XPAR_XSCUWDT_0_BASEADDR
#endif

#define HANDLER_CALLED		0xFFFFFFFF
#define WDT_LOAD_VALUE		0xFF

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
#ifndef SDT
int ScuWdtIntrExample(XScuGic *IntcInstancePtr, XScuWdt *WdtInstancePtr,
		      u16 WdtDeviceId, u16 WdtIntrId);
#else
int ScuWdtIntrExample(XScuWdt *WdtInstancePtr, UINTPTR BaseAddress);
#endif

static void WdtIntrHandler(void *CallBackRef);

#ifndef SDT
static int WdtSetupIntrSystem(XScuGic *IntcInstancePtr,
			      XScuWdt *WdtInstancePtr, u16 WdtIntrId);

static void WdtDisableIntrSystem(XScuGic *IntcInstancePtr, u16 WdtIntrId);
#endif
/************************** Variable Definitions *****************************/

XScuWdt WdtInstance;		/* Cortex SCU Private WatchDog Timer Instance */
#ifndef SDT
XScuGic IntcInstance;		/* Interrupt Controller Instance */
#endif
volatile u32 HandlerCalled;	/* flag is set when timeout interrupt occurs */

/*****************************************************************************/
/**
*
* Main function to call the Scu Private Wdt interrupt example.
*
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	int Status;

	xil_printf("SCU WDT Interrupt Example Test \r\n");

	/*
	 * Call the interrupt example, specify the parameters generated in
	 * xparameters.h
	 */

#ifndef SDT
	Status = ScuWdtIntrExample(&IntcInstance, &WdtInstance,
				   WDT_DEVICE_ID, WDT_IRPT_INTR);
#else
	Status = ScuWdtIntrExample(&WdtInstance, SCUWDT_BASEADDRESS);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("SCU WDT Interrupt Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran SCU WDT Interrupt Example Test\r\n");
	return XST_SUCCESS;
}
#endif
/*****************************************************************************/
/**
*
* This function tests the functioning of the Scu Private WDT driver and hardware
* in Timer mode using interrupts.
*
* After one expiration of the timeout interval, an interrupt is generated and
* the Event flag bit is set in the watchdog interrupt status register.
*
* @param	IntcInstancePtr is a pointer to the instance of the XScuGic
*		driver.
* @param	WdtInstancePtr is a pointer to the instance of XScuWdt driver.
* @param	WdtDeviceId is the Device ID of the XScuWdt device.
* @param	WdtIntrId is the Interrupt Id of the XScuWdt device.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
#ifndef SDT
int ScuWdtIntrExample(XScuGic *IntcInstancePtr, XScuWdt *WdtInstancePtr,
		      u16 WdtDeviceId, u16 WdtIntrId)
#else
int ScuWdtIntrExample(XScuWdt *WdtInstancePtr, UINTPTR BaseAddress)
#endif
{
	int Status;
	u32 Timebase = 0;
	u32 ExpiredTimeDelta = 0;
	XScuWdt_Config *ConfigPtr;
#ifdef SDT
	u32 Reg;
#endif

	/*
	 * Initialize the ScuWdt driver.
	 */
#ifndef SDT
	ConfigPtr = XScuWdt_LookupConfig(WdtDeviceId);
#else
	ConfigPtr = XScuWdt_LookupConfig(BaseAddress);
#endif
	/*
	 * This is where the virtual address would be used, this example
	 * uses physical address.
	 */
	Status = XScuWdt_CfgInitialize(WdtInstancePtr, ConfigPtr,
				       ConfigPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built correctly.
	 */
	Status = XScuWdt_SelfTest(WdtInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Put the watchdog timer in timer mode.
	 */
	XScuWdt_SetTimerMode(WdtInstancePtr);

	/*
	 * Load the watchdog counter register.
	 */
	XScuWdt_LoadWdt(WdtInstancePtr, WDT_LOAD_VALUE);

	/*
	 * Start the ScuWdt device.
	 */
	XScuWdt_Start(WdtInstancePtr);

	/*
	 * Determine how long it takes for the watchdog timer to expire,
	 * for later processing.
	 */
	while (1) {
		if (!(XScuWdt_IsTimerExpired(WdtInstancePtr))) {
			ExpiredTimeDelta++;
		} else {
			break;
		}
	}

	/*
	 * Stop the timer to set up the device in interrupt mode.
	 */
	XScuWdt_Stop(WdtInstancePtr);

	/*
	 * Connect the device to interrupt subsystem so that interrupts
	 * can occur.
	 */

#ifndef SDT
	Status = WdtSetupIntrSystem(IntcInstancePtr, WdtInstancePtr, WdtIntrId);
#else
	Status = XSetupInterruptSystem(WdtInstancePtr, &WdtIntrHandler,
				       ConfigPtr->IntrId,
				       ConfigPtr->IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
	/*
	 * Enable the watchdog interrupts for timer mode.
	 */
	Reg = XScuWdt_GetControlReg(WdtInstancePtr);
	XScuWdt_SetControlReg(WdtInstancePtr,
			      Reg | XSCUWDT_CONTROL_IT_ENABLE_MASK);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Reload the watchdog counter register.
	 */
	XScuWdt_RestartWdt(WdtInstancePtr);

	/*
	 * Start the ScuWdt device.
	 */
	HandlerCalled = 0;
	XScuWdt_Start(WdtInstancePtr);

	/*
	 * Verify that the watchdog timer does timeout when not restarted
	 * all the time, wait more than the amount of time it took for it
	 * to expire in the previous test.
	 */
	while (1) {
		/*
		 * If the handler is called, the test passed
		 */
		if ((HandlerCalled != 0)) {
			XScuWdt_WriteReg(WdtInstancePtr->Config.BaseAddr,
					 XSCUWDT_ISR_OFFSET,
					 XSCUWDT_ISR_EVENT_FLAG_MASK);
			break;
		}
	}

	/*
	 * Reload the watchdog counter to allow the next test to run normally.
	 */
	XScuWdt_RestartWdt(WdtInstancePtr);

	HandlerCalled = 0;

	/*
	 * Verify that the watchdog timer does not timeout when restarted
	 * all the time, wait more than the amount of time it took for it
	 * to expire in the previous test.
	 */
	while (1) {
		/*
		 * Reload the ScuWdt each pass through the loop.
		 */
		XScuWdt_RestartWdt(WdtInstancePtr);

		/*
		 * If more time has gone past than it took for it to expire
		 * when not restarted in the previous test, then stop as the
		 * restarting worked.
		 */
		Timebase++;
		if (Timebase > ExpiredTimeDelta + ExpiredTimeDelta) {
			break;
		}

		/*
		 * If the watchdog timer expired and/or handler called, then the
		 * test failed.
		 */
		if ((XScuWdt_IsTimerExpired(WdtInstancePtr)) ||
		    (HandlerCalled != 0)) {
			/*
			 * Disable and disconnect the interrupt system.
			 */
#ifndef SDT
			WdtDisableIntrSystem(IntcInstancePtr, WdtIntrId);
#else
			XDisconnectInterruptCntrl(ConfigPtr->IntrId, ConfigPtr->IntrParent);
#endif
			return XST_FAILURE;
		}
	}

	/*
	 * Disable and disconnect the interrupt system.
	 */
#ifndef SDT
	WdtDisableIntrSystem(IntcInstancePtr, WdtIntrId);
#else
	XDisconnectInterruptCntrl(ConfigPtr->IntrId, ConfigPtr->IntrParent);
#endif

   XScuWdt_Stop(WdtInstancePtr);

	return XST_SUCCESS;
}

#ifndef SDT
/*****************************************************************************/
/**
*
* This function sets up the interrupt system such that interrupts can occur
* for the device.
*
* @param	IntcInstancePtr is a pointer to the instance of the XScuGic
*		driver.
* @param	WdtInstancePtr is a pointer to the instance of XScuWdt driver.
* @param	WdtIntrId is the Interrupt Id of the XScuWdt device.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
static int WdtSetupIntrSystem(XScuGic *IntcInstancePtr,
			      XScuWdt *WdtInstancePtr, u16 WdtIntrId)
{
	int Status;
	u32 Reg;

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

	Xil_ExceptionInit();

	/*
	 * Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				     (Xil_ExceptionHandler)
				     XScuGic_InterruptHandler,
				     IntcInstancePtr);
#endif
	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device.
	 */
	Status = XScuGic_Connect(IntcInstancePtr, WdtIntrId,
				 (Xil_ExceptionHandler)WdtIntrHandler,
				 (void *)WdtInstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Enable the watchdog interrupts for timer mode.
	 */
	Reg = XScuWdt_GetControlReg(WdtInstancePtr);
	XScuWdt_SetControlReg(WdtInstancePtr,
			      Reg | XSCUWDT_CONTROL_IT_ENABLE_MASK);

	/*
	 * Enable the interrupt for the device.
	 */
	XScuGic_Enable(IntcInstancePtr, WdtIntrId);

#ifndef TESTAPP_GEN
	/*
	 * Enable interrupts in the Processor.
	 */
	Xil_ExceptionEnable();
#endif
	return XST_SUCCESS;
}
#endif
/*****************************************************************************/
/**
*
* This function is the Interrupt handler for the WDT Interrupt of the
* Wdt device. It is called on the expiration of the timer counter in
* interrupt context.
*
* @param	CallBackRef is a pointer to the callback function.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void WdtIntrHandler(void *CallBackRef)
{
	/*
	 * WDT timed out and interrupt occurred, let main test loop know.
	 */
	HandlerCalled = HANDLER_CALLED;
}

#ifndef SDT
/*****************************************************************************/
/**
*
* This function disables the interrupts that occur for the device.
*
* @param	IntcInstancePtr is the pointer to the instance of XScuGic
*		driver.
* @param	WdtIntrId is the Interrupt Id for the device.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void WdtDisableIntrSystem(XScuGic *IntcInstancePtr, u16 WdtIntrId)
{
	/*
	 * Disconnect and disable the interrupt for the Wdt.
	 */
	XScuGic_Disconnect(IntcInstancePtr, WdtIntrId);
}
#endif
