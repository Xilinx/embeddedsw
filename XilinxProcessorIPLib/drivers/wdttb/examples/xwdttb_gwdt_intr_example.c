/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xwdttb_gwdt_intr_example.c
*
* This file contains a design example using the Generic Watchdog Timer Device
* (Gwdt) driver and hardware device using interrupt mode (for the GWDT
* interrupt).
*
* @note
*
* This example assumes that the reset output of the GWDT device is not
* connected to the reset of the processor. This example will not return
* if the interrupts are not working.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 5.0	sne  03/26/20 First release
* 5.1   sne  06/01/20 Configured Generic watchdog offset value.
* 5.7   sb   06/27/23 Correct the interrupt ID for Versal-net platform.
* 5.7   sb   07/12/23 Added support for system device-tree flow.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xwdttb.h"
#include "xil_exception.h"
#include "xscugic.h"
#include "xil_printf.h"
#ifdef SDT
#include "xinterrupt_wrap.h"
#endif
/************************** Constant Definitions *****************************/
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are only defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#ifndef TESTAPP_GEN
#define INTC		XScuGic
#define GWDT_DEVICE_ID		XPAR_WDTTB_0_DEVICE_ID
#ifdef VERSAL_NET
#define GWDT_INTR_VEC_ID       XPS_FPD_GWDT_2_INT_ID
#elif defined(versal)
#define GWDT_INTR_VEC_ID        XPS_LPD_GWDT_0_INT_ID
#endif
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#endif
#endif

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
#ifndef SDT
int GWdtIntrExample(XScuGic *IntcInstancePtr,
		    XWdtTb *GWdtInstancePtr, u16 GWdtDeviceId,
		    u16 GWdtIntrId);
#else
int WdtTbIntrExample(XWdtTb *GWdtInstancePtr, UINTPTR BaseAddress);
#endif
static void GWdtIntrHandler(void *CallBackRef);
#ifndef SDT
static int GWdtSetupIntrSystem(XScuGic *IntcInstancePtr,
			       XWdtTb *GWdtInstancePtr,
			       u16 GWdtIntrId);
static void GWdtDisableIntrSystem(XScuGic *IntcInstancePtr,
				  u16 GWdtIntrId);
#endif
/************************** Variable Definitions *****************************/
#ifndef TESTAPP_GEN
XWdtTb GWdtInstance;		/* Instance of Generic WatchDog Timer */
#ifndef SDT
INTC IntcInstance;		/* Instance of the Interrupt Controller */
#endif
#endif
static volatile int GWdtExpired;
#define WDTPSV_GWOR_COUNT     0x00110000U  /*Generic Watchdog offset value*/
/*****************************************************************************/
/**
* Main function to call the Generic Wdt interrupt example.
*
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
	 * Call the GWdt interrupt example, specify the parameters generated in
	 * xparameters.h
	 */
#ifndef SDT
	Status = GWdtIntrExample(&IntcInstance, &GWdtInstance, GWDT_DEVICE_ID,
				 GWDT_INTR_VEC_ID);
#else
	Status = WdtTbIntrExample(&GWdtInstance, XPAR_XWDTTB_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("Generic WDT interrupt example failed.\n\r");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran GWDT interrupt example\n\r");
	return XST_SUCCESS;
}
#endif
/*****************************************************************************/
/**
*
* This function tests the functioning of the Generic WatchDog Timer module
* in the Interrupt mode (for the gwdt_ws0 interrupt).
*
* After one expiration of the GWDT timeout interval, an interrupt is generated
* and the GWS[1] state bit is set to one in the Generic Watchdog Control and
* Status Register register.
* If the system can attempt to take corrective action that includes refreshing
* the watchdog within the second watch period. If the refresh is successful,
* the system returns to the previous normal operation. If it fails, then the
* second watch period expires and a second signal is generated.
* A GWDT reset sets the GWS[2] reset status bit in the status register so that
* the application code can determine if the last system reset was a WDT reset.
*
* This function assumes that the reset output of the GWDT device is not
* connected to the reset of the processor. The function allows the watchdog
* timer to timeout such that a reset will occur if it is connected.
*
* @param	IntcInstancePtr is a pointer to the instance of the XScuGic
*		driver.
* @param	GWdtInstancePtr is a pointer to the instance of WdtTb driver.
* @param	GWdtDeviceId is the Device ID of the GWDT Device and is
*		typically XPAR_<GWDT_instance>_DEVICE_ID value from
*		xparameters.h.
* @param	GWdtIntrId is the Interrupt Id of the WatchDog and is typically
*		XPAR_<FABRIC_instance>_<GWDT_instance>_INTERRUPT_VEC_ID
*		value from xparameters.h.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE, otherwise.
*
* @note		This example will not return if the interrupts are not working.
*
******************************************************************************/
#ifndef SDT
int GWdtIntrExample(XScuGic *IntcInstancePtr, XWdtTb *GWdtInstancePtr,
		    u16 GWdtDeviceId,
		    u16 GWdtIntrId)
#else
int WdtTbIntrExample(XWdtTb *GWdtInstancePtr, UINTPTR BaseAddress)
#endif
{
	int Status;
	XWdtTb_Config *Config;

	/*
	 * Initialize the WDTTB driver so that it's ready to use look up
	 * configuration in the config table, then initialize it.
	 */
#ifndef SDT
	Config = XWdtTb_LookupConfig(GWdtDeviceId);
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
	Status = XWdtTb_CfgInitialize(GWdtInstancePtr, Config,
				      Config->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built correctly
	 */
	Status = XWdtTb_SelfTest(GWdtInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Stop the timer to start the test for interrupt mode
	 */
	XWdtTb_Stop(GWdtInstancePtr);

	/*
	 * Connect the WdtTb to the interrupt subsystem so that interrupts
	 * can occur
	 */
#ifndef SDT
	Status = GWdtSetupIntrSystem(IntcInstancePtr, GWdtInstancePtr,
				     GWdtIntrId);
#else
	Status = XSetupInterruptSystem(GWdtInstancePtr, &GWdtIntrHandler,
				       Config->IntrId[2],
				       Config->IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Update GWOR Register */
	XWdtTb_SetGenericWdtWindow(GWdtInstancePtr, WDTPSV_GWOR_COUNT);

	/*
	 * Start the WdtTb device
	 */
	GWdtExpired = FALSE;
	XWdtTb_Start(GWdtInstancePtr);

	/*
	 * Wait for the first expiration of the GWDT
	 */
	while (GWdtExpired != TRUE);
	GWdtExpired = FALSE;

	/*
	 * Wait for the second expiration of the GWDT
	 */
	while (GWdtExpired != TRUE);
	GWdtExpired = FALSE;

	/*
	 * Disable and disconnect the interrupt system
	 */
#ifndef SDT
	GWdtDisableIntrSystem(IntcInstancePtr, GWdtIntrId);
#else
	XDisconnectInterruptCntrl(Config->IntrId[2], Config->IntrParent);
#endif

	/*
	 * Stop the timer
	 */
	XWdtTb_Stop(GWdtInstancePtr);

	return XST_SUCCESS;
}
#ifndef SDT
/*****************************************************************************/
/**
*
* This function setups the interrupt system such that WDT interrupt can occur
* for the WdtTb. This function is application specific since the actual
* system may or may not have an interrupt controller. The WdtTb device could be
* directly connected to a processor without an interrupt controller. The
* user should modify this function to fit the application.
*
* @param	IntcInstancePtr is a pointer to the instance of the Intc driver.
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
static int GWdtSetupIntrSystem(XScuGic *IntcInstancePtr,
			       XWdtTb *GWdtInstancePtr,
			       u16 GWdtIntrId)
{
	int Status;
	XScuGic_Config *IntcConfig;
	u8 Priority, Trigger;

	Xil_ExceptionInit();

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

	XScuGic_GetPriorityTriggerType(IntcInstancePtr, GWdtIntrId,
				       &Priority, &Trigger);
	Trigger = 3;
	XScuGic_SetPriorityTriggerType(IntcInstancePtr, GWdtIntrId,
				       Priority, Trigger);

	/*
	 * Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				     (Xil_ExceptionHandler)XScuGic_InterruptHandler,
				     IntcInstancePtr);

	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device.
	 */
	Status = XScuGic_Connect(IntcInstancePtr, GWdtIntrId,
				 (Xil_ExceptionHandler)GWdtIntrHandler,
				 (void *)GWdtInstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Enable the interrupt for the device.
	 */
	XScuGic_Enable(IntcInstancePtr, GWdtIntrId);

	/*
	 * Enable interrupts in the Processor.
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* This function is the Interrupt handler for the GWDT Interrupt of the
* GWdt device. It is called on the expiration of the WDT period and is called
* from an interrupt context.
*
* This function provides an example of how to handle GWDT interrupt of the
* GWdt device.
*
* @param	CallBackRef is a pointer to the callback function.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void GWdtIntrHandler(void *CallBackRef)
{
	XWdtTb *GWdtInstancePtr = (XWdtTb *)CallBackRef;

	/*
	 * Set the flag indicating that the WDT has expired
	 */
	GWdtExpired = TRUE;

	/*
	 * Restart the watchdog timer as a normal application would
	 */
	XWdtTb_RestartWdt(GWdtInstancePtr);
}

#ifndef SDT
/*****************************************************************************/
/**
*
* This function disables the interrupts that occur for the device.
*
* @param	IntcInstancePtr is the pointer to the instance of XScuGic
*		driver.
* @param	GWdtIntrId is the Interrupt Id for the device.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void GWdtDisableIntrSystem(XScuGic *IntcInstancePtr, u16 GWdtIntrId)
{

	/*
	 * Disconnect and disable the interrupt for the Wdt.
	 */
	XScuGic_Disconnect(IntcInstancePtr, GWdtIntrId);

}
#endif
