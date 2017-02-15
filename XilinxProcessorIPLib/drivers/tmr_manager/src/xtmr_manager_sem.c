/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
/****************************************************************************/
/**
*
* @file xtmr_manager_intr.c
* @addtogroup tmr_manager_v1_0
* @{
*
* This file contains interrupt-related functions for the TMR Manager component
* (XTMR_Manager).
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.0   sa   04/05/17 First release
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xtmr_manager.h"
#include "xtmr_manager_i.h"
#include "xil_io.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/


/************************** Variable Definitions ****************************/

typedef void (*Handler)(XTMR_Manager *InstancePtr);


/****************************************************************************/
/**
*
* This function sets the handler that will be called when an event (interrupt)
* occurs in the driver. The purpose of the handler is to allow application
* specific processing to be performed.
*
* @param	InstancePtr is a pointer to the XTMR_Manager instance .
* @param	FuncPtr is the pointer to the callback function.
* @param	CallBackRef is the upper layer callback reference passed back
*		when the callback function is invoked.
*
* @return 	None.
*
* @note		There is no assert on the CallBackRef since the driver doesn't
*		know what it is (nor should it)
*
*****************************************************************************/
void XTMR_Manager_SetHandler(XTMR_Manager *InstancePtr,
			    XTMR_Manager_Handler FuncPtr, void *CallBackRef)
{
	/*
	 * Assert validates the input arguments
	 * CallBackRef not checked, no way to know what is valid
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(FuncPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	InstancePtr->Handler = FuncPtr;
	InstancePtr->CallBackRef = CallBackRef;
}


/*****************************************************************************/
/**
*
* This function disables the core interrupt. After calling this function,
* data may still be received by the core but no interrupt will be generated
* since the hardware device has no way to disable the receiver.
*
* @param	InstancePtr is a pointer to the XTMR_Manager instance.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XTMR_Manager_DisableInterrupt(XTMR_Manager *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Write to the control register to disable the interrupts.
	 */
	XTMR_Manager_WriteReg(InstancePtr->RegBaseAddress,
				XTM_SEMIMR_OFFSET, 0);
}

/*****************************************************************************/
/**
*
* This function enables the core interrupt such that an interrupt will occur
* when an unmasked SEM status input changes value.
*
* @param	InstancePtr is a pointer to the XTMR_Manager instance.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XTMR_Manager_EnableInterrupt(XTMR_Manager *InstancePtr, u32 mask)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Write to the control register to enable the interrupts.
	 */
	XTMR_Manager_WriteReg(InstancePtr->RegBaseAddress,
				XTM_SEMIMR_OFFSET, mask);
}

/** @} */
