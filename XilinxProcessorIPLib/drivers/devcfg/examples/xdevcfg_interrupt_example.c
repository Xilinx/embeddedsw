/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file  xdevcfg_interrupt_example.c
*
* This file contains a interrupt mode design example for the Device
* Configuration Interface. This example downloads a given bitstream to the FPGA
* fabric.
*
* BIT_STREAM_LOCATION specifies the memory location of the bitstream.
* BIT_STREAM_SIZE_WORDS specifies the size of the bitstream in words.
* User has to define these correctly for this example to work.
*
* @note		None
*
*
* MODIFICATION HISTORY:
*
*<pre>
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------
* 1.00a hvm  02/07/11 First release
* 1.00a nm   11/26/11 Holding FPGA in reset before download and
*                     releasing it after bitstream download. This code
*                     is not checking bitstream download errors.
*                     If the bitstream download fails, this test hangs.
* 2.00a nm   05/31/12 Updated the notes in the example for CR 660139 to add
*		      information that the 2 LSBs of the Source/Destination
*		      address when equal to 2�b01 indicate the last DMA command
*		      of an overall transfer.
* 		      Updated the example for CR 660835 so that input length for
*		      source/destination to the XDcfg_Transfer APIs is words
*		      (32 bit) and not bytes.
* 2.01a nm   11/21/12 Fixed CR# 688146. Modified the bitstream address.
* 2.02a nm   01/31/13 Fixed CR# 679335.
* 		      Removed disabling and enabling AXI interface.
*		      Clearing the interrupts before the transfer.
*		      Added support for partial reconfiguration.
* 3.00a kpc  02/10/14 Fixed the compilation error
* 3.1   kpc  04/22/14 Fixed CR#780203. Enable the pcap clock if it is not set.
*       ms   04/10/17 Modified filename tag to include the file in doxygen
*       ms   04/10/17 Modified filename tag to include the file in doxygen
*                     examples.
* 3.8  Nava  06/21/23 Added support for system device-tree flow.
*</pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xil_exception.h"
#ifndef SDT
#include "xscugic.h"
#else
#include "xinterrupt_wrap.h"
#endif
#include "xdevcfg.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are only defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define DCFG_DEVICE_ID		XPAR_XDCFG_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define DCFG_INTR_ID		XPAR_XDCFG_0_INTR
#else
#define DCFG_BASEADDR		XPAR_XDEVCFG_0_BASEADDR
#endif

/*
 * The BIT_STREAM_LOCATION is a dummy address and BIT_STREAM_SIZE_WORDS is a
 * dummy size. This has to replaced with the actual location of the bitstream.
 *
 * The 2 LSBs of the Source/Destination address when equal to 2�b01 indicates
 * the last DMA command of an overall transfer.
 * The 2 LSBs of the BIT_STREAM_LOCATION in this example is set to 2b01
 * indicating that this is the last DMA transfer (and the only one).
 */
#define BIT_STREAM_LOCATION	0x00400001	/* Bitstream location */
#define BIT_STREAM_SIZE_WORDS	0xF6EC0		/* Size in Words (32 bit)*/

/*
 * SLCR registers
 */
#define SLCR_LOCK	0xF8000004 /**< SLCR Write Protection Lock */
#define SLCR_UNLOCK	0xF8000008 /**< SLCR Write Protection Unlock */
#define SLCR_LVL_SHFTR_EN 0xF8000900 /**< SLCR Level Shifters Enable */
#ifndef SDT
#define SLCR_PCAP_CLK_CTRL XPAR_PS7_SLCR_0_S_AXI_BASEADDR + 0x168 /**< SLCR PCAP clock control register address */
#else
#define SLCR_PCAP_CLK_CTRL XPAR_SLCR_BASEADDR + 0x168
#endif

#define SLCR_PCAP_CLK_CTRL_EN_MASK 0x1
#define SLCR_LOCK_VAL	0x767B
#define SLCR_UNLOCK_VAL	0xDF0D

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
#ifndef SDT
int XDcfgInterruptExample(XScuGic *IntcInstPtr, XDcfg *DcfgInstance,
			  u16 DeviceId, u16 DcfgIntrId);
static int SetupInterruptSystem(XScuGic *IntcInstancePtr,
				XDcfg *DcfgInstPtr,
				u16 DcfgIntrId);
#else
int XDcfg_IntrExample(XDcfg *DcfgInstPtr, UINTPTR BaseAddress);
#endif

static void DcfgIntrHandler(void *CallBackRef, u32 IntrStatus);

/************************** Variable Definitions *****************************/

XDcfg DcfgInstance;   /* Device Configuration Interface Instance */
XScuGic IntcInstance; /* Instance of the Interrupt Controller driver */

volatile int DmaDone;
volatile int DmaPcapDone;
volatile int FpgaProgrammed;

/*****************************************************************************/
/**
* Main function to call the polled mode example.
*
* @param	None.
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	int Status;

	/*
	 * Call the example , specify the device ID  and vector ID that is
	 * generated in xparameters.h.
	 */
#ifndef SDT
	Status = XDcfgInterruptExample(&IntcInstance, &DcfgInstance,
				       DCFG_DEVICE_ID, DCFG_INTR_ID);
#else
	Status = XDcfg_IntrExample(&DcfgInstance, DCFG_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("Dcfg Interrupt Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Dcfg Interrupt Example Test\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function downloads the Non secure bit stream to the FPGA fabric
* using the Device Configuration Interface.
*
* @param	IntcInstPtr is a pointer to the instance of the Scu GIC driver.
* @param	DcfgInstPtr is a pointer to the instance of XDcfg driver.
* @param	DeviceId is the unique device id of the device.
* @param	DcfgIntrId is the interrupt Id.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None
*
****************************************************************************/
#ifndef SDT
int XDcfgInterruptExample(XScuGic *IntcInstPtr, XDcfg *DcfgInstPtr,
			  u16 DeviceId, u16 DcfgIntrId)
#else
int XDcfg_IntrExample(XDcfg *DcfgInstPtr, UINTPTR BaseAddress)
#endif
{
	int Status;
	u32 IntrStsReg = 0;
	u32 StatusReg;
	u32 PartialCfg = 0;

	XDcfg_Config *ConfigPtr;

	/*
	 * Initialize the Device Configuration Interface driver.
	 */
#ifndef SDT
	ConfigPtr = XDcfg_LookupConfig(DeviceId);
#else
	ConfigPtr = XDcfg_LookupConfig(BaseAddress);
#endif
	/*
	 * This is where the virtual address would be used, this example
	 * uses physical address.
	 */
	Status = XDcfg_CfgInitialize(DcfgInstPtr, ConfigPtr,
				     ConfigPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XDcfg_SelfTest(DcfgInstPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the interrupt system
	 */
#ifndef SDT
	Status = SetupInterruptSystem(IntcInstPtr, DcfgInstPtr, DcfgIntrId);
#else
	Status = XSetupInterruptSystem(DcfgInstPtr, XDcfg_InterruptHandler,
				       DcfgInstPtr->Config.IntrId,
				       DcfgInstPtr->Config.IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XDcfg_SetHandler(DcfgInstPtr, (void *)DcfgIntrHandler, DcfgInstPtr);

	DmaDone = FALSE;
	DmaPcapDone = FALSE;
	FpgaProgrammed = FALSE;

	/*
	 * Check first time configuration or partial reconfiguration
	 */
	IntrStsReg = XDcfg_IntrGetStatus(DcfgInstPtr);
	if (IntrStsReg & XDCFG_IXR_DMA_DONE_MASK) {
		PartialCfg = 1;
	}

	/*
	 * Enable the pcap clock.
	 */
	StatusReg = Xil_In32(SLCR_PCAP_CLK_CTRL);
	if (!(StatusReg & SLCR_PCAP_CLK_CTRL_EN_MASK)) {
		Xil_Out32(SLCR_UNLOCK, SLCR_UNLOCK_VAL);
		Xil_Out32(SLCR_PCAP_CLK_CTRL,
			  (StatusReg | SLCR_PCAP_CLK_CTRL_EN_MASK));
		Xil_Out32(SLCR_UNLOCK, SLCR_LOCK_VAL);
	}

	/*
	 * Disable the level-shifters from PS to PL.
	 */
	if (!PartialCfg) {
		Xil_Out32(SLCR_UNLOCK, SLCR_UNLOCK_VAL);
		Xil_Out32(SLCR_LVL_SHFTR_EN, 0xA);
		Xil_Out32(SLCR_LOCK, SLCR_LOCK_VAL);
	}

	/*
	 * Select PCAP interface for partial reconfiguration
	 */
	if (PartialCfg) {
		XDcfg_EnablePCAP(DcfgInstPtr);
		XDcfg_SetControlRegister(DcfgInstPtr, XDCFG_CTRL_PCAP_PR_MASK);
	}

	/*
	 * Clear the interrupt status bits
	 */
	XDcfg_IntrClear(DcfgInstPtr, (XDCFG_IXR_PCFG_DONE_MASK |
				      XDCFG_IXR_D_P_DONE_MASK |
				      XDCFG_IXR_DMA_DONE_MASK));

	/* Check if DMA command queue is full */
	StatusReg = XDcfg_ReadReg(DcfgInstPtr->Config.BaseAddr,
				  XDCFG_STATUS_OFFSET);
	if ((StatusReg & XDCFG_STATUS_DMA_CMD_Q_F_MASK) ==
	    XDCFG_STATUS_DMA_CMD_Q_F_MASK) {
		return XST_FAILURE;
	}

	/*
	 * Enable the DMA done, DMA_PCAP Done and PCFG Done interrupts.
	 */
	XDcfg_IntrEnable(DcfgInstPtr, (XDCFG_IXR_DMA_DONE_MASK |
				       XDCFG_IXR_D_P_DONE_MASK |
				       XDCFG_IXR_PCFG_DONE_MASK));

	/*
	 * Download bitstream in non secure mode
	 */
	XDcfg_Transfer(DcfgInstPtr, (u8 *)BIT_STREAM_LOCATION,
		       BIT_STREAM_SIZE_WORDS,
		       (u8 *)XDCFG_DMA_INVALID_ADDRESS,
		       0, XDCFG_NON_SECURE_PCAP_WRITE);

	while (!DmaDone);

	if (PartialCfg) {
		while (!DmaPcapDone);
	} else {
		while (!FpgaProgrammed);
		/*
		 * Enable the level-shifters from PS to PL.
		 */
		Xil_Out32(SLCR_UNLOCK, SLCR_UNLOCK_VAL);
		Xil_Out32(SLCR_LVL_SHFTR_EN, 0xF);
		Xil_Out32(SLCR_LOCK, SLCR_LOCK_VAL);
	}

	Status = XST_SUCCESS;

	XDcfg_IntrDisable(DcfgInstPtr, (XDCFG_IXR_DMA_DONE_MASK |
					XDCFG_IXR_D_P_DONE_MASK |
					XDCFG_IXR_PCFG_DONE_MASK));
#ifndef SDT
	XScuGic_Disable(IntcInstPtr, DcfgIntrId);

	XScuGic_Disconnect(IntcInstPtr, DcfgIntrId);
#else
	XDisconnectInterruptCntrl(DcfgInstPtr->Config.IntrId,
				  DcfgInstPtr->Config.IntrParent);
#endif

	return Status;
}

/*****************************************************************************/
/**
*
* Callback function (called from interrupt handler) to handle Device
* configuration interrupt.
*
* @param	CallBackRef is the callback reference passed from the interrupt
*		handler, which in our case is a pointer to the driver instance.
* @param	IntrStatus is a bit mask indicating the cause of the interrupt.
*		The mask values are defined in xdcfg_hw.h.
*
* @return	None.
*
* @note		This function is called by the driver within interrupt context.
*
******************************************************************************/
static void DcfgIntrHandler(void *CallBackRef, u32 IntrStatus)
{

	if (IntrStatus & XDCFG_IXR_DMA_DONE_MASK) {
		DmaDone = TRUE;
	}

	if (IntrStatus & XDCFG_IXR_D_P_DONE_MASK) {
		DmaPcapDone = TRUE;
	}

	if (IntrStatus & XDCFG_IXR_PCFG_DONE_MASK) {
		/*
		 * Disable PCFG DONE interrupt as this bit will remain set and will
		 * cause continuous interrupts.
		 */
		XDcfg_IntrDisable(&DcfgInstance, XDCFG_IXR_PCFG_DONE_MASK);
		FpgaProgrammed = TRUE;
	}
}


/*****************************************************************************/
/**
*
* This function sets up the interrupt system so interrupts can occur for the
* Device Configuration.
*
* @param	IntcInstancePtr is a pointer to the instance of GIC.
* @param	DevcfgInstancePtr contains a pointer to the instance of the DCFG
*		which is going to be connected to the interrupt
*		controller.
* @param	DcfgIntrId is the interrupt Id.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
****************************************************************************/
#ifndef SDT
static int SetupInterruptSystem(XScuGic *IntcInstancePtr,
				XDcfg *DcfgInstancePtr,
				u16 DcfgIntrId)
{
	int Status;

	XScuGic_Config *IntcConfig;

	Xil_ExceptionInit();

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	IntcConfig = XScuGic_LookupConfig(XPAR_SCUGIC_SINGLE_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
				       IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
				     (Xil_ExceptionHandler)XScuGic_InterruptHandler,
				     IntcInstancePtr);

	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device.
	 */
	Status = XScuGic_Connect(IntcInstancePtr, DcfgIntrId,
				 (Xil_InterruptHandler)XDcfg_InterruptHandler,
				 (void *)DcfgInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the interrupt for the DCFG.
	 */
	XScuGic_Enable(IntcInstancePtr, DcfgIntrId);

	/*
	 * Enable interrupts in the Processor.
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}
#endif
