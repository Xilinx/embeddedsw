/******************************************************************************
*
* Copyright (C) 2010 - 2015 Xilinx, Inc.  All rights reserved.
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
/******************************************************************************/
/**
*
* @file xscugic_low_level_example.c
*
* This file contains a design example using the low level driver, interface
* of the Interrupt Controller driver.
*
* This example shows the use of the Interrupt Controller with the ARM
* processor.
*
* @note
*
* none
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00a drg  01/30/10 First release
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include <stdio.h>
#include "xparameters.h"
#include "xil_exception.h"
#include "xscugic_hw.h"
#include "xil_printf.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define CPU_BASEADDR		XPAR_SCUGIC_0_CPU_BASEADDR
#define DIST_BASEADDR		XPAR_SCUGIC_0_DIST_BASEADDR


#define GIC_DEVICE_INT_MASK        0x02010003 /* Bit [25:24] Target list filter
                                                 Bit [23:16] 16 = Target CPU iface 0
                                                 Bit [3:0] identifies the SFI */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

static int ScuGicLowLevelExample(u32 CpuBaseAddress, u32 DistBaseAddress);

void SetupInterruptSystem();

void LowInterruptHandler(void *CallbackRef);

static void GicDistInit(u32 BaseAddress);

static void GicCPUInit(u32 BaseAddress);


/************************** Variable Definitions *****************************/

/*
 * Create a shared variable to be used by the main thread of processing and
 * the interrupt processing
 */
volatile static u32 InterruptProcessed = FALSE;

/*****************************************************************************/
/**
*
* This is the main function for the Interrupt Controller Low Level example.
*
* @param	None.
*
* @return	XST_SUCCESS to indicate success, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	int Status;


	/*
	 * Run the low level example of Interrupt Controller, specify the Base
	 * Address generated in xparameters.h
	 */
	xil_printf("Low Level GIC Example Test\r\n");
	Status = ScuGicLowLevelExample(CPU_BASEADDR, DIST_BASEADDR);
	if (Status != XST_SUCCESS) {
		xil_printf("Low Level GIC Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Low Level GIC Example Test\r\n");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is an example of how to use the interrupt controller driver
* (XScuGic) and the hardware device.  This function is designed to
* work without any hardware devices to cause interrupts.  It may not return
* if the interrupt controller is not properly connected to the processor in
* either software or hardware.
*
* This function relies on the fact that the interrupt controller hardware
* has come out of the reset state such that it will allow interrupts to be
* simulated by the software.
*
* @param	CpuBaseAddress is Base Address of the Interrupt Controller
*		Device
*
* @return	XST_SUCCESS to indicate success, otherwise XST_FAILURE
*
* @note		None.
*
******************************************************************************/
static int ScuGicLowLevelExample(u32 CpuBaseAddress, u32 DistBaseAddress)
{


	GicDistInit(DistBaseAddress);

	GicCPUInit(CpuBaseAddress);

	/*
	 * This step is processor specific, connect the handler for the
	 * interrupt controller to the interrupt source for the processor
	 */
	SetupInterruptSystem();

	/*
	 * Enable the software interrupts only.
	 */
	XScuGic_WriteReg(DistBaseAddress, XSCUGIC_ENABLE_SET_OFFSET, 0x0000FFFF);

	/*
	 * Cause (simulate) an interrupt so the handler will be called.
	 * This is done by changing the interrupt source to be software driven,
	 * then set a bit which simulates an interrupt.
	 */
	XScuGic_WriteReg(DistBaseAddress, XSCUGIC_SFI_TRIG_OFFSET, GIC_DEVICE_INT_MASK);

	/*
	 * Wait for the interrupt to be processed, if the interrupt does not
	 * occur this loop will wait forever
	 */
	while (1)
	{
		/*
		 * If the interrupt occurred which is indicated by the global
		 * variable which is set in the device driver handler, then
		 * stop waiting
		 */
		if (InterruptProcessed != 0) {
			break;
		}
	}

	return XST_SUCCESS;
}
/*****************************************************************************/
/**
*
* This function connects the interrupt handler of the interrupt controller to
* the processor.  This function is separate to allow it to be customized for
* each application.  Each processor or RTOS may require unique processing to
* connect the interrupt handler.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void SetupInterruptSystem(void)
{
	/*
	 * Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the ARM processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
			(Xil_ExceptionHandler) LowInterruptHandler,
			(void *)CPU_BASEADDR);

	/*
	 * Enable interrupts in the ARM
	 */
	Xil_ExceptionEnable();
}

/*****************************************************************************/
/**
*
* This function is designed to look like an interrupt handler in a device
* driver. This is typically a 2nd level handler that is called from the
* interrupt controller interrupt handler.  This handler would typically
* perform device specific processing such as reading and writing the registers
* of the device to clear the interrupt condition and pass any data to an
* application using the device driver.
*
* @param    CallbackRef is passed back to the device driver's interrupt handler
*           by the XScuGic driver.  It was given to the XScuGic driver in the
*           XScuGic_Connect() function call.  It is typically a pointer to the
*           device driver instance variable if using the Xilinx Level 1 device
*           drivers.  In this example, we do not care about the callback
*           reference, so we passed it a 0 when connecting the handler to the
*           XScuGic driver and we make no use of it here.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void LowInterruptHandler(void *CallbackRef)
{
	u32 BaseAddress;
	u32 IntID;


	if (NULL == CallbackRef) {
		return;
	}

	BaseAddress = (u32)CallbackRef;

	/*
	 * Read the int_ack register to identify the interrupt and
	 * make sure it is valid.
	 */
	IntID = XScuGic_ReadReg(BaseAddress, XSCUGIC_INT_ACK_OFFSET) &
			    XSCUGIC_ACK_INTID_MASK;
	if(XSCUGIC_MAX_NUM_INTR_INPUTS < IntID){
		return;
	}

	/*
	 * If the interrupt is shared, do some locking here if there are
	 * multiple processors.
	 */

	/*
	 * Execute the ISR. For this example set the global to 1.
	 * The software trigger is cleared by the ACK.
	 */
	InterruptProcessed = 1;

	/*
	 * Write to the EOI register, we are all done here.
	 * Let this function return, the boot code will restore the stack.
	 */
	XScuGic_WriteReg(BaseAddress, XSCUGIC_EOI_OFFSET, IntID);
}


static void GicDistInit(u32 BaseAddress)
{
	u32 Int_Id;

	XScuGic_WriteReg(BaseAddress, XSCUGIC_DIST_EN_OFFSET, 0UL);

	/*
	 * Set the security domains in the int_security registers for non-secure interrupts
	 * All are secure, so leave at the default. Set to 1 for non-secure interrupts.
	 */


	/*
	 * For the Shared Peripheral Interrupts INT_ID[MAX..32], set:
	 */

	/*
	 * 1. The trigger mode in the int_config register
	 */
	for(Int_Id = 32; Int_Id<XSCUGIC_MAX_NUM_INTR_INPUTS;Int_Id+=16) {
		/*
		 * Each INT_ID uses two bits, or 16 INT_ID per register
		 * Set them all to be level sensitive, active HIGH.
		 */
		XScuGic_WriteReg(BaseAddress,
				XSCUGIC_INT_CFG_OFFSET + (Int_Id * 4)/16, 0UL);
	}


#define DEFAULT_PRIORITY    0xa0a0a0a0UL
#define DEFAULT_TARGET    0x01010101UL
	for(Int_Id = 0; Int_Id < XSCUGIC_MAX_NUM_INTR_INPUTS; Int_Id+=4){
		/*
		 * 2. The priority using int the priority_level register
		 * The priority_level and spi_target registers use one byte
		 * per INT_ID.
		 * Write a default value that can be changed elsewhere.
		 */
		XScuGic_WriteReg(BaseAddress,
				XSCUGIC_PRIORITY_OFFSET +((Int_Id *4)/4),
				DEFAULT_PRIORITY);
	}

	for(Int_Id = 32; Int_Id<XSCUGIC_MAX_NUM_INTR_INPUTS;Int_Id+=4){
		/*
		 * 3. The CPU interface in the spi_target register
		 */
		XScuGic_WriteReg(BaseAddress,
			XSCUGIC_SPI_TARGET_OFFSET +((Int_Id *4)/4),
			DEFAULT_TARGET);
	}

	for(Int_Id = 0; Int_Id<XSCUGIC_MAX_NUM_INTR_INPUTS;Int_Id+=32){
		/*
		 * 4. Enable the SPI using the enable_set register.
		 * Leave all disabled for now.
		 */
		XScuGic_WriteReg(BaseAddress,
			XSCUGIC_DISABLE_OFFSET +((Int_Id *4)/32), 0xFFFFFFFFUL);

	}

    XScuGic_WriteReg(BaseAddress, XSCUGIC_DIST_EN_OFFSET, 0x01UL);
}

static void GicCPUInit(u32 BaseAddress)
{
	/*
	 * Program the priority mask of the CPU using the Priority mask register
	 */
	XScuGic_WriteReg(BaseAddress, XSCUGIC_CPU_PRIOR_OFFSET, 0xF0);


	/*
	 * If the CPU operates in both security domains, set parameters in the control_s register.
	 * 1. Set FIQen=1 to use FIQ for secure interrupts,
	 * 2. Program the AckCtl bit
	 * 3. Program the SBPR bit to select the binary pointer behavior
	 * 4. Set EnableS = 1 to enable secure interrupts
	 * 5. Set EnbleNS = 1 to enable non secure interrupts
	 */

	/*
	 * If the CPU operates only in the secure domain, setup the control_s register.
	 * 1. Set FIQen=1,
	 * 2. Set EnableS=1, to enable the CPU interface to signal secure interrupts.
	 */
	XScuGic_WriteReg(BaseAddress, XSCUGIC_CONTROL_OFFSET, 0x01);

}
