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
* @file xtmr_manager.c
* @addtogroup tmr_manager_v1_0
* @{
*
* Contains required functions for the XTMR_Manager driver. See the xtmr_manager.h
* header file for more details on this driver.
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


/************************** Variable Definitions ****************************/


/************************** Function Prototypes *****************************/

extern void _xtmr_manager_initialize(XTMR_Manager *InstancePtr);

static void StubHandler(void *CallBackRef);


/****************************************************************************/
/**
*
* Initialize a XTMR_Manager instance. The receive and transmit FIFOs of the
* core are not flushed, so the user may want to flush them. The hardware
* device does not have any way to disable the receiver such that any valid
* data may be present in the receive FIFO.  This function disables the core
* interrupt. The baudrate and format of the data are fixed in the hardware
* at hardware build time.
*
* @param	InstancePtr is a pointer to the XTMR_Manager instance.
* @param	Config is a reference to a structure containing information
*		about a specific TMR Manager device. This function initializes
*		an InstancePtr object for a specific device specified by the
*		contents of Config. This function can initialize multiple
*		instance objects with the use of multiple calls giving different
*		Config information on each call.
* @param 	EffectiveAddr is the device base address in the virtual memory
*		address space. The caller is responsible for keeping the address
*		mapping from EffectiveAddr to the device physical base address
*		unchanged once this function is invoked. Unexpected errors may
*		occur if the address mapping changes after this function is
*		called. If address translation is not used, use
*		Config->BaseAddress for this parameters, passing the physical
*		address instead.
*
* @return
* 		- XST_SUCCESS if everything starts up as expected.
*
* @note		The Config pointer argument is not used by this function,
*		but is provided to keep the function signature consistent
*		with other drivers.
*
*****************************************************************************/
int XTMR_Manager_CfgInitialize(XTMR_Manager *InstancePtr, XTMR_Manager_Config *Config,
				UINTPTR EffectiveAddr)
{
	(void) Config;
	u32 cr;

	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/*
	 * Perform low-level initialization
	 */
        _xtmr_manager_initialize(InstancePtr);

	/*
	 * Set some default values, including setting the callback
	 * handlers to stubs.
	 */
	if (Config->ComparatorsMask) {
	  InstancePtr->Cmr0 = (u32)(Config->MaskRstValue & 0xffffffff);
	  InstancePtr->Cmr1 = (u32)(Config->MaskRstValue >> 32);
	}
	if (Config->BrkDelayWidth > 0)
		InstancePtr->Bdir = Config->BrkDelayRstValue;
	if (Config->SemInterface > 0)
		InstancePtr->SemImr = 0;
	if (Config->TestComparator == 2)
		InstancePtr->Cfir = 0;

	InstancePtr->PreResetHandler = NULL;
	InstancePtr->RecoveryHandler = NULL;
	InstancePtr->PostResetHandler = NULL;

	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	InstancePtr->RegBaseAddress = EffectiveAddr;

	InstancePtr->Handler = StubHandler;

	/*
	 * Write to the SEM interrupt mask register to disable the interrupts
	 */
	XTMR_Manager_WriteReg(InstancePtr->RegBaseAddress,
				XTM_SEMIMR_OFFSET, 0);

	/*
	 * Write to the Control Register to set RIR and MACGI1 to allow
         * recovery reset by default.
	 */
        cr = (1 << XTM_CR_RIR) | Config->Magic1;
	XTMR_Manager_WriteReg(InstancePtr->RegBaseAddress,
				XTM_CR_OFFSET, cr);
	InstancePtr->Cr = cr;

	/*
	 * Clear the statistics for this driver
	 */
	XTMR_Manager_ClearStats(InstancePtr);

	return XST_SUCCESS;
}


/****************************************************************************
*
* This function provides a stub handler such that if the application does not
* define a handler but enables interrupts, this function will be called.
*
* @param	CallBackRef has no purpose but is necessary to match the
*		interface for a handler.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
static void StubHandler(void *CallBackRef)
{
	(void) CallBackRef;

	/*
	 * Assert occurs always since this is a stub and should never be called
	 */
	Xil_AssertVoidAlways();
}


/** @} */
