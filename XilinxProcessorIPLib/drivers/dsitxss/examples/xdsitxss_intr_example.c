/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdsitxss_intr_example.c
*
* This file contains a design example using the XDsiTxSs driver with interrupts.
*
* This will provide interrupts for pixel under run and undefined data type
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date    Changes
* --- --- ------- -------------------------------------------------------
* 1.0 ram 11/2/16 Initial Release for MIPI DSI TX subsystem
* 1.1 ms  01/23/17 Modified xil_printf statement in main function to
*                  ensure that "Successfully ran" and "Failed" strings
*                  are available in all examples. This is a fix for
*                  CR-965028.
*     ms  04/05/17 Added tabspace for return statements in functions for
*                  proper documentation while generating doxygen.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdsitxss.h"
#include "xdsitxss_hw.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xstatus.h"
#include "xintc.h"

/* The unique device ID of the MIPI DSI Tx Subsystem instance to be used
 */
#define XDSITXSS_DEVICE_ID		XPAR_DSITXSS_0_DEVICE_ID
#define XINTC_DSITXSS_INTERRUPT_ID	XPAR_INTC_0_DSITXSS_0_VEC_ID
#define XINTC_DEVICE_ID			XPAR_INTC_0_DEVICE_ID

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

u32 DsiTxSs_IntrExample(u32 DeviceId);
u32 DsiTxSs_SetupIntrSystem(void);
void XDsiTxSs_IntrHandler(void *InstancePtr);
u32 XDsiTxSs_SetCallback(XDsiTxSs *InstancePtr, u32 HandlerType,
				void *CallbackFunc, void *CallbackRef);
void XDsiTxSs_SetGlobalInterrupt(void *InstancePtr);
void XDsiTxSs_InterruptEnable(void *InstancePtr, u32 Mask);

/* Interrupt helper functions */
void DsiTxSs_PixelUnderrunEventHandler(void *CallbackRef, u32 Mask);
void DsiTxSs_UnSupportDataEventHandler(void *CallbackRef, u32 Mask);
void DsiTxSs_CmdQFIFOFullEventHandler(void *CallbackRef, u32 Mask);

/************************** Variable Definitions *****************************/

XDsiTxSs DsiTxSs;

XIntc InterruptController; /* The instance of the Interrupt Controller */

volatile u8 data_err_flag = 1;
volatile u8 pixel_underrun_flag = 1;
volatile u8 cmdq_fifo_full_flag = 1;

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This is the main function for XDsiTxSs interrupt example. If the
* DsiTxSs_IntrExample function which sets up the system succeeds, this function
* will wait for the interrupts. Notify the events
*
* @param	None.
*
* @return
*		- XST_FAILURE if the interrupt example was unsuccessful.
*
* @note		Unless setup failed, main will never return since
*		DsiTxSs_IntrExample is blocking (it is waiting on interrupts
*		for Hot-Plug-Detect (HPD) events.
*
******************************************************************************/
#ifndef TESTAPP_GEN
s32 main()
{
	u32 Status;

	xil_printf("------------------------------------------\n\r");
	xil_printf("MIPI DSITXSS interrupt example\n\r");
	xil_printf("------------------------------------------\n\r\n\r");

	Status = DsiTxSs_IntrExample(XDSITXSS_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("MIPI DSITXSS interrupt example failed.");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran MIPI DSITXSS interrupt example\n\r");

	return XST_SUCCESS;
}
#endif

/******************************************************************************/
/**
*
* For Microblaze we use an assembly loop that is roughly the same regardless of
* optimization level, although caches and memory access time can make the delay
* vary.  Just keep in mind that after resetting or updating the PHY modes,
* the PHY typically needs time to recover.
*
* @param	Number of seconds to sleep
*
* @return	None
*
* @note		None
*
******************************************************************************/
void Delay(u32 Seconds)
{
#if defined (__MICROBLAZE__) || defined(__PPC__)
	static s32 WarningFlag = 0;

	/* If MB caches are disabled or do not exist, this delay loop could
	 * take minutes instead of seconds (e.g., 30x longer).  Print a warning
	 * message for the user (once).  If only MB had a built-in timer!
	 */
	if (((mfmsr() & 0x20) == 0) && (!WarningFlag)) {
		WarningFlag = 1;
	}

#define ITERS_PER_SEC   (XPAR_CPU_CORE_CLOCK_FREQ_HZ / 6)
    asm volatile ("\n"
			"1:               \n\t"
			"addik r7, r0, %0 \n\t"
			"2:               \n\t"
			"addik r7, r7, -1 \n\t"
			"bneid  r7, 2b    \n\t"
			"or  r0, r0, r0   \n\t"
			"bneid %1, 1b     \n\t"
			"addik %1, %1, -1 \n\t"
			:: "i"(ITERS_PER_SEC), "d" (Seconds));
#else
	sleep(Seconds);
#endif
}

/****************************************************************************/
/**
*
* This function setups the interrupt system such that interrupts can occur
* for the DSI device. This function is application specific since the
* actual system may or may not have an interrupt controller. The DSI
* could be directly connected to a processor without an interrupt controller.
* The user should modify this function to fit the application.
*
* @param	DsiTxSsPtr contains a pointer to the instance of the DSI
*		component which is going to be connected to the interrupt
*		controller.
*
* @return
*		- XST_SUCCESS if successfully setup the interrupt
*		- XST_FAILURE.failure to setup the interrupt
*
* @note		None.
*
****************************************************************************/
s32 SetupInterruptSystem(XDsiTxSs *DsiTxSsPtr)
{
	s32 Status;

	/* Setup call back handlers */
	XDsiTxSs_SetCallback(DsiTxSsPtr, XDSITXSS_HANDLER_UNSUPPORT_DATATYPE,
				DsiTxSs_UnSupportDataEventHandler, DsiTxSsPtr);
	XDsiTxSs_SetCallback(DsiTxSsPtr, XDSITXSS_HANDLER_PIXELDATA_UNDERRUN,
				DsiTxSs_PixelUnderrunEventHandler, DsiTxSsPtr);
	XDsiTxSs_SetCallback(DsiTxSsPtr, XDSITXSS_HANDLER_CMDQ_FIFOFULL,
				DsiTxSs_CmdQFIFOFullEventHandler, DsiTxSsPtr);

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	Status = XIntc_Initialize(&InterruptController, XINTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect a device driver handler that will be called when an interrupt
	 * for the device occurs, the device driver handler performs the
	 * specific interrupt processing for the device.
	 */
	Status = XIntc_Connect(&InterruptController, XINTC_DSITXSS_INTERRUPT_ID,
				(XInterruptHandler)XDsiTxSs_IntrHandler,
				(void *)DsiTxSsPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts, specific real mode so that
	 * the DSI can cause interrupts through the interrupt controller.
	 */
	Status = XIntc_Start(&InterruptController, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XIntc_Enable(&InterruptController, XINTC_DSITXSS_INTERRUPT_ID);

	Xil_ExceptionInit();

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			 (Xil_ExceptionHandler)XIntc_InterruptHandler,
			 &InterruptController);

	Xil_ExceptionEnable();

	XDsiTxSs_SetGlobalInterrupt(DsiTxSsPtr);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is the main entry point for the interrupt example using the
* DsiTxSs driver. This function will set up the system with interrupts handlers.
*
* @param	DeviceId is the unique device ID of the MIPI DSI
*		Subsystem core.
*
* @return
*		- XST_FAILURE if the system setup failed.
*		- XST_SUCCESS should never return since this function, if setup
*		was successful, is blocking.
*
* @note		If system setup was successful, this function is blocking in
*		order to illustrate interrupt handling taking place for HPD
*		events.
*
******************************************************************************/
u32 DsiTxSs_IntrExample(u32 DeviceId)
{
	XDsiTxSs_Config *CfgPtr;
	u32 Status = XST_SUCCESS;
	u8 Exit_Count = 0;

	CfgPtr = XDsiTxSs_LookupConfig(DeviceId);
	if (!CfgPtr) {
		return XST_FAILURE;
	}

	/*
	 * Initialize the DSI driver so that it is ready to use.
	 */
	Status = XDsiTxSs_CfgInitialize(&DsiTxSs, CfgPtr, CfgPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform self test to ensure the hardware built correctly
	 */
	Status = XDsiTxSs_SelfTest(&DsiTxSs);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the DSI to the interrupt subsystem such that interrupts can
	 * occur. This function is application specific.
	 */
	Status = SetupInterruptSystem(&DsiTxSs);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XDsiTxSs_InterruptEnable(&DsiTxSs, XDSITXSS_IER_ALLINTR_MASK);

	/*
	 * Please write code to generate interrupt condition here
	 */

	do {
	    Delay(1);
	    Exit_Count++;
	    if(Exit_Count > 3) {
		xil_printf("DSI TXInterrupt test failed \r\n");
		return XST_FAILURE;
	    }
	}
	while(data_err_flag && pixel_underrun_flag && cmdq_fifo_full_flag);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is called when a DPHY level error event is received by
* the MIPI DSI Rx Subsystem core.
*
* @param	CallbackRef is a pointer to the DsiTxSs instance.
*
* @param	Mask of interrupt which caused this event
*
* @return	None.
*
* @note		Use the DsiTxSs_UnSupportDataEventHandler driver function to set
* 		this function as the handler for Unsupport data error event.
*
******************************************************************************/
void DsiTxSs_UnSupportDataEventHandler(void *CallbackRef, u32 Mask)
{
	if (Mask & XDSITXSS_ISR_DATAIDERR_MASK) {
		xil_printf("Invalid data type Error \r\n");
		data_err_flag = 0;
	}
}

/*****************************************************************************/
/**
*
* This function is called when a Packet level error event is received by
* the MIPI DSI Tx Subsystem core.
*
* @param	CallbackRef is a pointer to the DsiTxSs instance.
*
* @param	Mask of interrupt which caused this event
*
* @return	None.
*
* @note		Use the DsiTxSs_PixelUnderrunEventHandler driver function to set
* 		this function as the handler for Packet level error event.
*
******************************************************************************/
void DsiTxSs_PixelUnderrunEventHandler(void *CallbackRef, u32 Mask)
{
	if (Mask & XDSITXSS_ISR_PIXELUNDERRUN_MASK) {
		xil_printf("Pixel Underrun Error \r\n");
		pixel_underrun_flag = 0;
	}
}

/*****************************************************************************/
/**
*
* This function is called when a Command Queue FIFO full error event is received
* by the MIPI DSI Tx Subsystem core.
*
* @param	CallbackRef is a pointer to the DsiTxSs instance.
*
* @param	Mask of interrupt which caused this event
*
* @return	None.
*
* @note		Use the DsiTxSs_CmdQFIFOFullEventHandler driver function to set
* 		this function as the handler for Command Queue FIFO Full error
* 		event.
*
******************************************************************************/
void DsiTxSs_CmdQFIFOFullEventHandler(void *CallbackRef, u32 Mask)
{
	if (Mask & XDSITXSS_ISR_CMDQ_FIFO_FULL_MASK) {
		xil_printf("Command queue FIFO Error \r\n");
		cmdq_fifo_full_flag = 0;
	}
}
