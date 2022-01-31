/******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
*
* @file xtmr_manager_intr_example.c
*
* This file contains a design example using the TMR_Manager driver
* (XTMR_Manager) and hardware device using the interrupt mode.
*
* @note
*
* None.
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.0   sa   04/05/17 First release
* 1.3	adk  31/01/22 Updated the example independent of SEM IP hardware
* 		      configuration.
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xtmr_manager.h"
#include "xintc.h"
#include "xil_exception.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define TMR_MANAGER_DEVICE_ID   XPAR_TMR_MANAGER_0_DEVICE_ID
#define INTC_DEVICE_ID          XPAR_INTC_0_DEVICE_ID
#define TMR_MANAGER_INT_IRQ_ID  XPAR_INTC_0_TMR_MANAGER_0_VEC_ID


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int TMR_ManagerIntrExample(u16 DeviceId);

int SetupInterruptSystem(XTMR_Manager *TMR_ManagerPtr);

void Handler(void *CallBackRef);

/************************** Variable Definitions *****************************/

 XTMR_Manager TMR_Manager;      /* The instance of the TMR_Manager Device */

 XIntc InterruptController;     /* The instance of the Interrupt Controller */

/*
 * The following variables are shared between non-interrupt processing and
 * interrupt processing such that they must be global.
 */

/*
 * The following counter is used to determine when SEM input events have
 * occurred.
 */
volatile static int TotalEventCount = 0;


/******************************************************************************/
/**
*
* Main function to call the TMR_Manager interrupt example.
*
* @param	None
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful
*
* @note		None
*
*******************************************************************************/
int main(void)
{
	int Status;

	/*
	 * Run the TMR_Manager Interrupt example, specify the Device ID that is
	 * generated in xparameters.h.
	 */
	xil_printf("\r\n--- Entering main() --- \r\n");
	Status = TMR_ManagerIntrExample(TMR_MANAGER_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	xil_printf("TMR Manager Interrupt test Passed\r\n");
        xil_printf("--- Exiting main() --- \r\n");

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function does a minimal test on the TMR_Manager device and driver as a
* design example. The purpose of this function is to illustrate
* how to use the XTMR_Manager component.
*
* This function sends data and expects to receive the same data through the
* TMR_Manager. The user must provide a physical loopback such that data which is
* transmitted will be received.
*
* This function uses interrupt driver mode of the TMR_Manager device. The calls
* to the TMR_Manager driver in the handlers should only use the non-blocking
* calls.
*
* @param	DeviceId is the Device ID of the TMR_Manager Device and is the
*		XPAR_<tmr_manager_instance>_DEVICE_ID value from xparameters.h.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note
*
* This function contains an infinite loop such that if interrupts are not
* working it may never return.
*
****************************************************************************/
int TMR_ManagerIntrExample(u16 DeviceId)
{
	int Status;

	/*
	 * Initialize the TMR_Manager driver so that it's ready to use.
	 */
	Status = XTMR_Manager_Initialize(&TMR_Manager, DeviceId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built correctly.
	 */
	Status = XTMR_Manager_SelfTest(&TMR_Manager);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the TMR_Manager to the interrupt subsystem such that interrupts can
	 * occur. This function is application specific.
	 */
	Status = SetupInterruptSystem(&TMR_Manager);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the interrupt of the TMR_Manager so that interrupts will occur.
	 */
	TotalEventCount = 0;
	XTMR_Manager_EnableInterrupt(&TMR_Manager, 0x7ff);

	/*
	 * Wait for a SEM input status event to occur, the function may get
	 * locked up in this loop if the interrupts are not working correctly.
	 */
	while ((TotalEventCount == 0)) {
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is the handler which performs processing to send data to the
* TMR_Manager. It is called from an interrupt context such that the amount of
* processing performed should be minimized. It is called when the transmit
* FIFO of the TMR_Manager is empty and more data can be sent through the
* TMR_Manager.
*
* This handler provides an example of how to handle data for the TMR_Manager,
* but is application specific.
*
* @param	CallBackRef contains a callback reference from the driver.
*		In this case it is the instance pointer for the TMR_Manager
*		driver.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
void Handler(void *CallBackRef)
{
	TotalEventCount++;
}

/****************************************************************************/
/**
*
* This function sets up the interrupt system such that interrupts can occur
* for the TMR_Manager device. This function is application specific since the
* actual system may or may not have an interrupt controller. The TMR_Manager
* could be directly connected to a processor without an interrupt controller.
* The user should modify this function to fit the application.
*
* @param    TMR_ManagerPtr contains a pointer to the instance of the
*           TMR_Manager component which is going to be connected to the
*           interrupt controller.
*
* @return   XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note     None.
*
****************************************************************************/
int SetupInterruptSystem(XTMR_Manager *TMR_ManagerPtr)
{

	int Status;


	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	Status = XIntc_Initialize(&InterruptController, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Connect a device driver handler that will be called when an interrupt
	 * for the device occurs, the device driver handler performs the
	 * specific interrupt processing for the device.
	 */
	Status = XIntc_Connect(&InterruptController, TMR_MANAGER_INT_IRQ_ID,
			   (Xil_ExceptionHandler)Handler,
			   (void *)TMR_ManagerPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts, specific real mode so that the
	 * TMR_Manager can cause interrupts through the interrupt controller.
	 */
	Status = XIntc_Start(&InterruptController, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the interrupt for the TMR_Manager device.
	 */
	XIntc_Enable(&InterruptController, TMR_MANAGER_INT_IRQ_ID);

	/*
	 * Initialize the exception table.
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			 (Xil_ExceptionHandler)XIntc_InterruptHandler,
			 &InterruptController);

	/*
	 * Enable exceptions.
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}
