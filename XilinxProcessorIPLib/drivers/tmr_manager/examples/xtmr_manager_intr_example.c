/******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
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
* 1.7   adk  04/04/25 Ported example to the SDT flow.
* 1.10  ml   25/09/26 Fixed doxygen 1.15.0 and compliance checker warnings.
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xtmr_manager.h"
#ifdef SDT
#include "xinterrupt_wrap.h"
#else
#include "xintc.h"
#endif
#include "xil_exception.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define TMR_MANAGER_DEVICE_ID   XPAR_TMR_MANAGER_0_DEVICE_ID	/**< TMR Manager device ID. */
#define INTC_DEVICE_ID          XPAR_INTC_0_DEVICE_ID
#define TMR_MANAGER_INT_IRQ_ID  XPAR_INTC_0_TMR_MANAGER_0_VEC_ID
#else
#define TMR_MANAGER_DEVICE_ID	XPAR_TMR_MANAGER_0_BASEADDR	/**< TMR Manager base address. */
#endif


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/
#ifndef SDT
int TMR_ManagerIntrExample(u16 DeviceId);
#else
int TMR_ManagerIntrExample(UINTPTR BaseAddress);
#endif

/**
* @brief Set up the interrupt system for the TMR Manager example.
*
* @param[in] TMR_ManagerPtr is a pointer to the TMR Manager instance.
*
* @return XST_SUCCESS if successful, otherwise XST_FAILURE.
*/
int SetupInterruptSystem(XTMR_Manager *TMR_ManagerPtr);

void Handler(void *CallBackRef);

/************************** Variable Definitions *****************************/

 XTMR_Manager TMR_Manager;      /**< The instance of the TMR_Manager Device */

 XIntc InterruptController;     /**< The instance of the Interrupt Controller */

/*
 * The following variables are shared between non-interrupt processing and
 * interrupt processing such that they must be global.
 */

/*
 * The following counter is used to determine when SEM input events have
 * occurred.
 */
static volatile int TotalEventCount = 0;


/******************************************************************************/
/**
* Main function to call the TMR_Manager interrupt example.
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful
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
* @param[in] BaseAddress is the base address of the TMR_Manager device.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note
*
* This function contains an infinite loop such that if interrupts are not
* working it may never return.
*
****************************************************************************/
#ifndef SDT
int TMR_ManagerIntrExample(u16 DeviceId)
#else
int TMR_ManagerIntrExample(UINTPTR BaseAddress)
#endif
{
	int Status;

	/*
	 * Initialize the TMR_Manager driver so that it's ready to use.
	 */
#ifndef SDT
	Status = XTMR_Manager_Initialize(&TMR_Manager, DeviceId);
#else
	Status = XTMR_Manager_Initialize(&TMR_Manager, BaseAddress);
#endif
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
#ifndef SDT
	Status = SetupInterruptSystem(&TMR_Manager);
#else
	Status = XSetupInterruptSystem(&TMR_Manager, (XInterruptHandler)Handler,
				       TMR_Manager.Config.IntrId,
				       TMR_Manager.Config.IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
#endif
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
* @brief Handles TMR Manager interrupt events from an interrupt context.
*
* This handler provides an example of how to handle data for the
* TMR_Manager, but is application specific.
*
* @param[in] CallBackRef contains a callback reference from the driver;
*		in this case it is the instance pointer for the TMR_Manager.
*
*****************************************************************************/
void Handler(void *CallBackRef)
{
	(void)CallBackRef;
	TotalEventCount++;
}

/****************************************************************************/
/*
* This function sets up the interrupt system such that interrupts can occur
* for the TMR_Manager device. This function is application specific since the
* actual system may or may not have an interrupt controller. The TMR_Manager
* could be directly connected to a processor without an interrupt controller.
* The user should modify this function to fit the application.
*
* @param[in] TMR_ManagerPtr contains a pointer to the instance of the
*           TMR_Manager component which is going to be connected to the
*           interrupt controller.
*
* @return   XST_SUCCESS if successful, otherwise XST_FAILURE.
*
****************************************************************************/
#ifndef SDT
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
#endif
