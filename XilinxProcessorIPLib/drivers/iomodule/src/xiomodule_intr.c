/******************************************************************************
*
* Copyright (C) 2011 - 2015 Xilinx, Inc.  All rights reserved.
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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
*
* @file xiomodule_intr.c
* @addtogroup iomodule_v2_2
* @{
*
* This file contains the interrupt processing for the XIOModule component
* which is the driver for the Xilinx IO Module interrupt.  The interrupt
* processing is partitioned seperately such that users are not required to
* use the provided interrupt processing.  This file requires other files of
* the driver to be linked in also.
*
* Two different interrupt handlers are provided for this driver such that the
* user must select the appropriate handler for the application.  The first
* interrupt handler, XIOModule_VoidInterruptHandler, is provided for
* systems which use only a single interrupt controller or for systems that
* cannot otherwise provide an argument to the XIOModule interrupt handler
* (e.g., the RTOS interrupt vector handler may not provide such a facility).
* The constant XPAR_IOMODULE_SINGLE_DEVICE_ID must be defined for this
* handler to be included in the driver.  The second interrupt handler,
* XIOModule_InterruptHandler, uses an input argument which is an instance
* pointer to an interrupt controller driver such that multiple interrupt
* controllers can be supported.  This handler requires the calling function
* to pass it the appropriate argument, so another level of indirection may be
* required.
*
* Note that both of these handlers are now only provided for backward
* compatibility. The handler defined in xiomodule_l.c is the recommended
* handler.
*
* The interrupt processing may be used by connecting one of the interrupt
* handlers to the interrupt system.  These handlers do not save and restore
* the processor context but only handle the processing of the Interrupt
* Controller.  The two handlers are provided as working examples. The user is
* encouraged to supply their own interrupt handler when performance tuning is
* deemed necessary.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00a sa   07/15/11 First release
* 1.02a sa   07/25/12 Added UART interrupt related functions
* 1.03a sa   10/16/12 Moved UART interrupt related functions to separate file
* </pre>
*
* @internal
*
* This driver assumes that the context of the processor has been saved prior to
* the calling of the IO Module interrupt handler and then restored
* after the handler returns. This requires either the running RTOS to save the
* state of the machine or that a wrapper be used as the destination of the
* interrupt vector to save the state of the processor and restore the state
* after the interrupt handler returns.
*
******************************************************************************/

/***************************** Include Files *********************************/


#include "xparameters.h"
#include "xiomodule.h"
#include "xil_types.h"
#include "xil_assert.h"

/************************** Constant Definitions *****************************/

/*
 * Array of masks associated with the bit position, improves performance
 * in the ISR, this table is shared between all instances of the driver
 */
u32 XIOModule_TimerBitPosMask[XTC_DEVICE_TIMER_COUNT] = {
  1 << XIN_IOMODULE_PIT_1_INTERRUPT_INTR,
  1 << XIN_IOMODULE_PIT_2_INTERRUPT_INTR,
  1 << XIN_IOMODULE_PIT_3_INTERRUPT_INTR,
  1 << XIN_IOMODULE_PIT_4_INTERRUPT_INTR
};


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
*
* Interrupt handler for the driver used when there can be no argument passed
* to the handler.  This function is provided mostly for backward compatibility.
* The user should use XIOModule_DeviceInterruptHandler(), defined in
* xiomodule_l.c, if possible.
*
* The user must connect this function to the interrupt system such that it is
* called whenever the devices which are connected to it cause an interrupt.
*
* @return	None.
*
* @note
*
* The constant XPAR_IOMODULE_SINGLE_DEVICE_ID must be defined for this handler
* to be included in the driver compilation.
*
******************************************************************************/
#ifdef XPAR_IOMODULE_SINGLE_DEVICE_ID
void XIOModule_VoidInterruptHandler()
{
	/* Use the single instance to call the main interrupt handler */
	XIOModule_DeviceInterruptHandler(
		 (void *) XPAR_IOMODULE_SINGLE_DEVICE_ID);
}
#endif

/*****************************************************************************/
/**
*
* The interrupt handler for the driver. This function is provided mostly for
* backward compatibility.  The user should use
* XIOModule_DeviceInterruptHandler(), defined in xiomodule_l.c when
* possible and pass the device ID of the interrupt controller device as its
* argument.
*
* The user must connect this function to the interrupt system such that it is
* called whenever the devices which are connected to it cause an interrupt.
*
* @param	InstancePtr is a pointer to the XIOModule instance to be
*               worked on.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XIOModule_InterruptHandler(XIOModule * InstancePtr)
{
	/* Assert that the pointer to the instance is valid
	 */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Use the instance's device ID to call the main interrupt handler.
	 * (the casts are to avoid a compiler warning)
	 */
	XIOModule_DeviceInterruptHandler((void *)
			         ((u32) (InstancePtr->CfgPtr->DeviceId)));
}


/*****************************************************************************/
/**
*
* Sets the timer callback function, which the driver calls when the specified
* timer times out.
*
* @param	InstancePtr is a pointer to the XIOModule instance.
* @param	CallBackRef is the upper layer callback reference passed back
*		when the callback function is invoked.
* @param	FuncPtr is the pointer to the callback function.
*
* @return	None.
*
* @note
*
* The handler is called within interrupt context so the function that is
* called should either be short or pass the more extensive processing off
* to another task to allow the interrupt to return and normal processing
* to continue.
*
* This function is provided for compatibility, and only allows setting a
* single handler for all Programmable Interval Timers.
*
******************************************************************************/
void XIOModule_SetHandler(XIOModule * InstancePtr,
			  XIOModule_Timer_Handler FuncPtr,
			  void *CallBackRef)
{
	u8 Index;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(FuncPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	InstancePtr->Handler = FuncPtr;
	InstancePtr->CallBackRef = CallBackRef;

	for (Index = XIN_IOMODULE_PIT_1_INTERRUPT_INTR;
	     Index <= XIN_IOMODULE_PIT_4_INTERRUPT_INTR; Index++) {
	    InstancePtr->CfgPtr->HandlerTable[Index].Handler =
	      XIOModule_Timer_InterruptHandler;
	}
}

/*****************************************************************************/
/**
*
* Interrupt Service Routine (ISR) for the driver.  This function only performs
* processing for the Programmable Interval Timere and does not save and restore
* the interrupt context.
*
* @param	InstancePtr contains a pointer to the IO Module instance for
*		the interrupt.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XIOModule_Timer_InterruptHandler(void *InstancePtr)
{
    XIOModule *IOModulePtr = NULL;
    u8 Index;
    u32 IntPendingReg, ModeMask;

    /*
     * Verify that each of the inputs are valid.
     */
    Xil_AssertVoid(InstancePtr != NULL);

    /*
     * Convert the non-typed pointer to an IO Module instance pointer
     * such that there is access to the IO Module
     */
    IOModulePtr = (XIOModule *) InstancePtr;

    /*
     * Read the pending interrupts to be able to check if interrupt occurred
     */
    IntPendingReg = XIOModule_ReadReg(IOModulePtr->BaseAddress,
				      XIN_IPR_OFFSET);

    ModeMask = ~IOModulePtr->CurrentIMR;

    /*
     * Loop thru each timer in the device and call the callback function
     * for each timer which has caused an interrupt, but only if not fast
     */
    for (Index = 0; Index < XTC_DEVICE_TIMER_COUNT; Index++) {
	/*
	 * Check if timer is enabled
	 */
	if (IOModulePtr->CfgPtr->PitUsed[Index]) {

	    /*
	     * Check if timer expired and interrupt occured,
	     * but only if it is not a fast interrupt
	     */
	    if (IntPendingReg & ModeMask & XIOModule_TimerBitPosMask[Index]) {

		/*
		 * Increment statistics for the number of interrupts and call
		 * the callback to handle any application specific processing
		 */
		IOModulePtr->Timer_Stats[Index].Interrupts++;

		IOModulePtr->Handler(IOModulePtr->CallBackRef, Index);

		/*
		 * Acknowledge the interrupt in the interrupt controller
		 * acknowledge register to mark it as handled
		 */
		XIOModule_WriteReg(IOModulePtr->BaseAddress,
				   XIN_IAR_OFFSET,
				   XIOModule_TimerBitPosMask[Index]);
	    }
	}
    }
}
/** @} */
