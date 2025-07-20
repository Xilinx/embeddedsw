/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
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
* 3.10  mus  09/19/18 Update prototype of LowInterruptHandler to fix the GCC
*                     warning
* 4.0   mus  01/28/19  Updated to support Cortexa72 GIC (GIC500).
* 5.0   adk  04/18/22 Replace infinite while loop with
* 		      Xil_WaitForEventSet() API.
*       adk  20/07/22 Update the Xil_WaitForEventSet() API arguments as
*      		      per latest API.
* 5.1   mus  02/15/23 Added support for VERSAL_NET.
* 5.2   dp   06/20/23 Make interrupt as Group1 interrupt for Cortex-R52.
* 5.5   ml   12/20/24 Fixed GCC warnings
* 5.6   ml   07/21/25 Fixed GCC warnings
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include <stdio.h>
#include "xparameters.h"
#include "xil_exception.h"
#include "xscugic_hw.h"
#include "xil_printf.h"
#include "xstatus.h"
#include "xscugic.h"
#include "xil_util.h"
#if defined (VERSAL_NET)
#include "xplatform_info.h"
#endif

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define XSCUGIC_CPU_BASEADDR		XPAR_SCUGIC_0_CPU_BASEADDR
#define XSCUGIC_DIST_BASEADDR		XPAR_SCUGIC_0_DIST_BASEADDR
#else
#define XSCUGIC_DIST_BASEADDR		XPAR_XSCUGIC_0_BASEADDR
#endif

#if defined (GICv3)
#define GIC_DEVICE_INT_MASK        0x11000001 /* Bit [27:24] SGI Interrupt ID
                                                 Bit [15:0] Targeted CPUs */
#define XSUGIC_SGI_INT_ID	   0x5U       /* Modify it to intended SGI interrupt ID */
#else
#define GIC_DEVICE_INT_MASK        0x02010003 /* Bit [25:24] Target list filter
                                                 Bit [23:16] 16 = Target CPU iface 0
                                                 Bit [3:0] identifies the SFI */
#endif
#define XSCUGIC_SW_TIMEOUT_VAL	10000000U /* Wait for 10 sec */
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static int ScuGicLowLevelExample();
void SetupInterruptSystem();

void LowInterruptHandler(void *CallbackRef);

static void GicDistInit(u32 BaseAddress);

#if !defined (GICv3)
static void GicCPUInit(u32 BaseAddress);
#endif
/************************** Variable Definitions *****************************/

/*
 * Create a shared variable to be used by the main thread of processing and
 * the interrupt processing
 */
static volatile u32 InterruptProcessed = FALSE;

static XScuGic_Config *CfgPtr;
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
	Status = ScuGicLowLevelExample();
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
static int ScuGicLowLevelExample()
{
	int Status;
#if defined (GICv3)
	UINTPTR RedistBaseAddr;
	u64 Mask;
#endif
#if defined (VERSAL_NET)
	u32 CoreId, ClusterId;
#endif
	GicDistInit(XSCUGIC_DIST_BASEADDR);

#ifndef SDT
	CfgPtr = XScuGic_LookupConfigBaseAddr(XSCUGIC_DIST_BASEADDR);
#else
	CfgPtr = XScuGic_LookupConfig(XSCUGIC_DIST_BASEADDR);
#endif

#if !defined (GICv3)
	GicCPUInit(CfgPtr->CpuBaseAddress);
#endif

	/*
	 * This step is processor specific, connect the handler for the
	 * interrupt controller to the interrupt source for the processor
	 */
	SetupInterruptSystem();

	/*
	 * Enable the software interrupts only.
	 */
#if defined (GICv3)
	RedistBaseAddr = XScuGic_GetRedistBaseAddr();

	XScuGic_WriteReg(RedistBaseAddr + XSCUGIC_RDIST_SGI_PPI_OFFSET,
			 XSCUGIC_RDIST_ISENABLE_OFFSET, 0xFFFFFFFF);
#else
	XScuGic_WriteReg(XSCUGIC_DIST_BASEADDR, XSCUGIC_ENABLE_SET_OFFSET, 0x0000FFFF);
#endif

	/*
	 * Cause (simulate) an interrupt so the handler will be called.
	 * This is done by changing the interrupt source to be software driven,
	 * then set a bit which simulates an interrupt.
	 */
#if defined (GICv3)
#if defined (VERSAL_NET)

	CoreId = XGetCoreId();
	ClusterId = XGetClusterId();

#if defined (ARMR52)
	Mask = ( ClusterId << XSCUGIC_SGI1R_AFFINITY1_SHIFT);
	Mask |= (0x1U << CoreId);
#else
	Mask = ClusterId;
	Mask = (Mask << XSCUGIC_SGI1R_AFFINITY2_SHIFT);
	Mask |= (CoreId << XSCUGIC_SGI1R_AFFINITY1_SHIFT);
	Mask |= 0x1U;
#endif
	Mask |= (XSUGIC_SGI_INT_ID << XSCUGIC_SGIR_EL1_INITID_SHIFT);
#else
	Mask = GIC_DEVICE_INT_MASK;
#endif


#if EL3
	XScuGic_WriteICC_SGI0R_EL1(Mask);
#else
	XScuGic_WriteICC_SGI1R_EL1(Mask);
#endif
#else
	XScuGic_WriteReg(XSCUGIC_DIST_BASEADDR, XSCUGIC_SFI_TRIG_OFFSET, GIC_DEVICE_INT_MASK);
#endif
	/*
	 * Wait for the interrupt to be processed, if the interrupt does not
	 * occur return failure after timeout.
	 */
	Status = Xil_WaitForEventSet(XSCUGIC_SW_TIMEOUT_VAL, 1,
				     &InterruptProcessed);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
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
				     (void *)CfgPtr->CpuBaseAddress);
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
*           drivers.  In this example, we are passing it as scugic cpu
*           interface base address to access ack and EOI registers.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void LowInterruptHandler(void *CallbackRef)
{
#if ! defined (GICv3)
	u32 BaseAddress;
	BaseAddress = (u32)(UINTPTR)CallbackRef;
#else
	(void)CallbackRef;
#endif
	u32 IntID;

#if defined (GICv3)
	IntID = XScuGic_get_IntID();
#else
	/*
	 * Read the int_ack register to identify the interrupt and
	 * make sure it is valid.
	 */
	IntID = XScuGic_ReadReg(BaseAddress, XSCUGIC_INT_ACK_OFFSET) &
		XSCUGIC_ACK_INTID_MASK;
#endif
	if (XSCUGIC_MAX_NUM_INTR_INPUTS < IntID) {
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

#if defined (GICv3)
	XScuGic_ack_Int(IntID);

#else
	/*
	 * Write to the EOI register, we are all done here.
	 * Let this function return, the boot code will restore the stack.
	 */
	XScuGic_WriteReg(BaseAddress, XSCUGIC_EOI_OFFSET, IntID);
#endif
}


static void GicDistInit(u32 BaseAddress)
{
	u32 Int_Id;

#if defined (GICv3)
	u32 temp;
	u32 Waker_State;
	UINTPTR RedistBaseAddr;

	RedistBaseAddr = XScuGic_GetRedistBaseAddr();

#if defined (GIC600)
	XScuGic_WriteReg(RedistBaseAddr, XSCUGIC_RDIST_PWRR_OFFSET,
			 (XScuGic_ReadReg(RedistBaseAddr, XSCUGIC_RDIST_PWRR_OFFSET) &
			  (~XSCUGIC_RDIST_PWRR_RDPD_MASK)));
#endif
	/* CPU is active, reset GICR_WAKER.ProcessorSleep to enable interrupts */
	Waker_State = XScuGic_ReadReg(RedistBaseAddr, XSCUGIC_RDIST_WAKER_OFFSET);
	XScuGic_WriteReg(RedistBaseAddr, XSCUGIC_RDIST_WAKER_OFFSET,
			 Waker_State & (~ XSCUGIC_RDIST_WAKER_LOW_POWER_STATE_MASK));

	/* Enable system reg interface through ICC_SRE_EL1/EL3  */
#if EL3
	XScuGic_Enable_SystemReg_CPU_Interface_EL3();
#endif
	XScuGic_Enable_SystemReg_CPU_Interface_EL1();

	/* Disable distributor */
	temp = XScuGic_ReadReg(BaseAddress, XSCUGIC_DIST_EN_OFFSET);
	temp |= (XSCUGIC500_DCTLR_ARE_NS_ENABLE | XSCUGIC500_DCTLR_ARE_S_ENABLE);
	temp &= ~(XSCUGIC_EN_INT_MASK);
	XScuGic_WriteReg(BaseAddress, XSCUGIC_DIST_EN_OFFSET, temp);
#else
	XScuGic_WriteReg(BaseAddress, XSCUGIC_DIST_EN_OFFSET, 0UL);
#endif

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
	for (Int_Id = 32; Int_Id < XSCUGIC_MAX_NUM_INTR_INPUTS; Int_Id += 16) {
		/*
		 * Each INT_ID uses two bits, or 16 INT_ID per register
		 * Set them all to be level sensitive, active HIGH.
		 */
		XScuGic_WriteReg(BaseAddress,
				 XSCUGIC_INT_CFG_OFFSET + (Int_Id * 4) / 16, 0UL);
	}


#define DEFAULT_PRIORITY    0xa0a0a0a0UL
#if defined (GICv3)
#define DEFAULT_TARGET    0x0UL
#else
#define DEFAULT_TARGET    0x01010101UL
#endif

	for (Int_Id = 0; Int_Id < XSCUGIC_MAX_NUM_INTR_INPUTS; Int_Id += 4) {
		/*
		 * 2. The priority using int the priority_level register
		 * The priority_level and spi_target registers use one byte
		 * per INT_ID.
		 * Write a default value that can be changed elsewhere.
		 */
		XScuGic_WriteReg(BaseAddress,
				 XSCUGIC_PRIORITY_OFFSET + ((Int_Id * 4) / 4),
				 DEFAULT_PRIORITY);
	}
#if defined (GICv3)
	for (Int_Id = 32U; Int_Id < XSCUGIC_MAX_NUM_INTR_INPUTS; Int_Id = Int_Id + 1) {
		/*
		 * 3. The CPU interface in the spi_target register
		 * Only write to the SPI interrupts, so start at 32
		 */
		temp = Int_Id - 32;
		XScuGic_WriteReg(BaseAddress, XSCUGIC_IROUTER_OFFSET_CALC(temp),
				 DEFAULT_TARGET);
	}
#else
	for (Int_Id = 32; Int_Id < XSCUGIC_MAX_NUM_INTR_INPUTS; Int_Id += 4) {
		/*
		 * 3. The CPU interface in the spi_target register
		 */
		XScuGic_WriteReg(BaseAddress,
				 XSCUGIC_SPI_TARGET_OFFSET + ((Int_Id * 4) / 4),
				 DEFAULT_TARGET);
	}
#endif
	for (Int_Id = 0; Int_Id < XSCUGIC_MAX_NUM_INTR_INPUTS; Int_Id += 32) {
		/*
		 * 4. Enable the SPI using the enable_set register.
		 * Leave all disabled for now.
		 */
		XScuGic_WriteReg(BaseAddress,
				 XSCUGIC_DISABLE_OFFSET + ((Int_Id * 4) / 32), 0xFFFFFFFFUL);

	}
#if defined (GICv3)
	XScuGic_WriteReg(RedistBaseAddr + XSCUGIC_RDIST_SGI_PPI_OFFSET,
			 XSCUGIC_RDIST_IGROUPR_OFFSET, XSCUGIC_DEFAULT_SECURITY);
	temp = XScuGic_ReadReg(BaseAddress, XSCUGIC_DIST_EN_OFFSET);
	temp |= XSCUGIC_EN_INT_MASK;
	XScuGic_WriteReg(BaseAddress, XSCUGIC_DIST_EN_OFFSET, temp);

	XScuGic_Enable_Group1_Interrupts();
#if defined (ARMR52) || (defined (__aarch64__) && (EL1_NONSECURE == 0))
	XScuGic_Enable_Group0_Interrupts();
#endif

	XScuGic_set_priority_filter(0xff);
#else
	XScuGic_WriteReg(BaseAddress, XSCUGIC_DIST_EN_OFFSET, 0x01UL);
#endif
}

#if !defined (GICv3)
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
#endif
