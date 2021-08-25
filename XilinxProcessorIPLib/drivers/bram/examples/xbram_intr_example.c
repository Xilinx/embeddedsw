/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xbram_intr_example.c
*
* This file contains a design example using the BRAM driver
* (XBram) in an interrupt driven mode of operation. This example assumes
* that there is an interrupt controller in the hardware system and the
* BRAM device is connected to the interrupt controller.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a sa   05/11/10 Initial release.
* 3.01a sa   13/01/12 Changed XBram_SelfTest(InstancePtr) to
* 			 XBram_SelfTest(InstancePtr, XBRAM_IR_ALL_MASK)
* 			 as per new API (CR 639274)
* 4.1   ms   01/23/17 Modified xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings are
*                     available in all examples. This is a fix for CR-965028.
* 4.7   mus  08/25/21 BRAM and interrupt controller instance need not to be
*                     declared in case of peripheral test, added condition to skip
*                     them in case of peripheral test (CR#1108877)
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xbram.h"
#include "xil_exception.h"
#include "xintc.h"
#include "xil_cache.h"
#include <stdio.h>

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define BRAM_DEVICE_ID		XPAR_BRAM_1_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#define INTC_BRAM_INTERRUPT_ID	XPAR_INTC_0_BRAM_1_VEC_ID

/************************** Function Prototypes ******************************/
static void BramIntrExceptionHandler(void *InstancePtr);

static void BramDriverHandler(void *CallBackRef);

int BramIntrExample(XIntc* IntcInstancePtr, XBram* InstancePtr,
		     u16 DeviceId, u16 IntrId);

static void InitializeECC(XBram_Config *ConfigPtr, u32 EffectiveAddr);

static int BramSetupIntrSystem(XIntc* IntcInstancePtr, XBram* InstancePtr,
			 u16 DeviceId, u16 IntrId);

static void BramDisableIntr(XIntc* IntcInstancePtr, XBram* InstancePtr,
		      u16 IntrId);

/************************** Variable Definitions *****************************/

/*
 * The following are declared globally so they are zeroed and so they are
 * easily accessible from a debugger
 */
 #ifndef TESTAPP_GEN
XBram Bram;	/* The Instance of the BRAM Driver */

XIntc Intc;	/* The Instance of the Interrupt Controller Driver */
#endif

static volatile u32 IntrCount;		/* Total number of interrupts */
static volatile int ExceptionCount;	/* Total number of exceptions */

/****************************************************************************/
/**
* This function is the main function of the BRAM example.  It is
* for initializing the BRAM device and setting up interrupts.
*
* @param	None.
*
* @return
*		- XST_SUCCESS to indicate success.
*		- XST_FAILURE to indicate failure.
*
* @note		None.
*
*****************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	int Status;

	Status = BramIntrExample(&Intc, &Bram, BRAM_DEVICE_ID,
				 INTC_BRAM_INTERRUPT_ID);
	if (Status != XST_SUCCESS ) {
		xil_printf("Bram Interrupt Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Bram Interrupt Example\r\n");
	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* This is the entry function from the TestAppGen tool generated application
* which tests the interrupts when enabled in the BRAM
*
* @param	IntcInstancePtr is a reference to the Interrupt Controller
*		driver Instance
* @param	InstancePtr is a reference to the BRAM driver Instance
* @param	DeviceId is the XPAR_<BRAM_instance>_DEVICE_ID
*		value from xparameters.h
* @param	IntrId is XPAR_<INTC_instance>_<BRAM_instance>_VEC_ID
*		value from xparameters.h
*
* @return
*		- XST_SUCCESS if the example is successful.
*		- XST_FAILURE if the example failed.
*
* @note		None.
*
******************************************************************************/
int BramIntrExample(XIntc* IntcInstancePtr, XBram* InstancePtr,
		     u16 DeviceId, u16 IntrId)
{
	int Status;
	XBram_Config *ConfigPtr;

	/*
	 * Initialize the BRAM driver. If an error occurs then exit
	 */

	/*
	 * Lookup configuration data in the device configuration table.
	 * Use this configuration info down below when initializing this
	 * driver.
	 */
	ConfigPtr = XBram_LookupConfig(DeviceId);
	if (ConfigPtr == (XBram_Config *) NULL) {
		return XST_FAILURE;
	}

	Status = XBram_CfgInitialize(InstancePtr, ConfigPtr,
				     ConfigPtr->CtrlBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


        InitializeECC(ConfigPtr, ConfigPtr->CtrlBaseAddress);


	Status = BramSetupIntrSystem(IntcInstancePtr,
					InstancePtr, DeviceId,
					IntrId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	IntrCount = 0;
	ExceptionCount = 0;

	/*
	 * Execute the BRAM driver selftest, and then disable exceptions again.
	 * Running the driver test will inject errors if Fault Injection is
	 * enabled.
	 */
	Status = XBram_SelfTest(InstancePtr, XBRAM_IR_ALL_MASK);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	BramDisableIntr(IntcInstancePtr, InstancePtr, IntrId);

	if (InstancePtr->Config.FaultInjectionPresent) {
		if (ExceptionCount == 0 &&
			InstancePtr->Config.UncorrectableFailingDataRegs) {
			return XST_FAILURE;
		}
		if (IntrCount == 0 &&
			InstancePtr->Config.EccStatusInterruptPresent) {
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function performs the BRAM set up for Interrupts
*
* @param	IntcInstancePtr is a reference to the Interrupt Controller
*		driver Instance
* @param	InstancePtr is a reference to the BRAM driver Instance
* @param	DeviceId is the XPAR_<BRAM_instance>_DEVICE_ID
*		value from xparameters.h
* @param	IntrId is XPAR_<INTC_instance>_<BRAM_instance>_INTERRUPT_INTR
*		value from xparameters.h
*
* @return
*		- XST_SUCCESS if the interrupt setup is successful.
*		- XST_FAILURE if interrupt setup is not successful.
*
* @note		None.
*
******************************************************************************/
static int BramSetupIntrSystem(XIntc* IntcInstancePtr, XBram* InstancePtr,
			 u16 DeviceId, u16 IntrId)
{
	int Status;

 #ifndef TESTAPP_GEN
	/*
	 * Initialize the interrupt controller driver so that it's ready to
	 * use.specify the device ID that was generated in xparameters.h
	 */
	Status = XIntc_Initialize(IntcInstancePtr, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif

	/*
	 * Hook up interrupt Bram handler
	 */
	Status = XIntc_Connect(IntcInstancePtr, IntrId,
				(XInterruptHandler)BramDriverHandler,
				InstancePtr);

	/*
	 * Enable the BRAM interrupts so that errors can be detected
	 */
	XBram_InterruptEnable(InstancePtr, XBRAM_IR_ALL_MASK);

	/*
	 * Enable the interrupt vector at the interrupt controller
	 */
	XIntc_Enable(IntcInstancePtr, IntrId);

#ifndef TESTAPP_GEN
	/*
	 * Initialize the exception table and register the interrupt
	 * controller handler with the exception table
	 */
	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				 (Xil_ExceptionHandler)XIntc_InterruptHandler,
				 IntcInstancePtr);
#endif

	/*
	 * Set up exception handlers for bus error exceptions.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IPLB_EXCEPTION,
				(Xil_ExceptionHandler)BramIntrExceptionHandler,
					0);
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_DPLB_EXCEPTION,
				(Xil_ExceptionHandler)BramIntrExceptionHandler,
					0);

#ifndef TESTAPP_GEN
	/*
	 * Enable exceptions.
	 */
	Xil_ExceptionEnable();

	/*
	 * Start the interrupt controller such that interrupts are recognized
	 * and handled by the processor
	 */
	Status = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif

	return XST_SUCCESS;
}


/****************************************************************************/
/**
*
* This function ensures that ECC in the BRAM is initialized if no hardware
* initialization is available. The ECC bits are initialized by reading and
* writing data in the memory. This code is not optimized to only read data
* in initialized sections of the BRAM.
*
* @param	ConfigPtr is a reference to a structure containing information
*		about a specific BRAM device.
* @param 	EffectiveAddr is the device base address in the virtual memory
*		address space.
*
* @return
*		None
*
* @note		None.
*
*****************************************************************************/
static void InitializeECC(XBram_Config *ConfigPtr, u32 EffectiveAddr)
{
	u32 Addr;
	volatile u32 Data;

	if (ConfigPtr->EccPresent &&
	    ConfigPtr->EccOnOffRegister &&
	    ConfigPtr->EccOnOffResetValue == 0 &&
	    ConfigPtr->WriteAccess != 0) {
		for (Addr = ConfigPtr->MemBaseAddress;
		     Addr < ConfigPtr->MemHighAddress; Addr+=4) {
			Data = XBram_In32(Addr);
			XBram_Out32(Addr, Data);
		}
		XBram_WriteReg(EffectiveAddr, XBRAM_ECC_ON_OFF_OFFSET, 1);
	}
}


/*****************************************************************************/
/**
*
* This is the interrupt handler routine for the BRAM interrupt for this example.
*
* @param	CallbackRef is the Callback reference for the handler.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void BramDriverHandler(void *CallbackRef)
{
	XBram *BramPtr = (XBram *)CallbackRef;

	IntrCount++;

	/*
	 * Clear the Interrupt
	 */
	XBram_InterruptClear(BramPtr, XBRAM_IR_ALL_MASK);
}


/*****************************************************************************/
/**
*
* This function disables the interrupts for the BRAM and disables Exceptions.
*
* @param	IntcInstancePtr is a pointer to the Interrupt Controller
*		driver Instance
* @param	InstancePtr is a pointer to the BRAM driver Instance
* @param	IntrId is XPAR_<INTC_instance>_<BRAM_instance>_VEC_ID
*		value from xparameters.h
*
* @return	None
*
* @note		None
*
******************************************************************************/
static void BramDisableIntr(XIntc* IntcInstancePtr, XBram* InstancePtr, u16 IntrId)
{
	XBram_InterruptDisable(InstancePtr, XBRAM_IR_ALL_MASK);
	XIntc_Disable(IntcInstancePtr, IntrId);
#ifndef TESTAPP_GEN
	Xil_ExceptionDisable();
#endif
}


/*****************************************************************************/
/**
* Exception handler, invoked when an instruction bus exception or a data bus
* exception orrurs. The handler just counts exceptions, and returns.
*
* @param	InstancePtr points to the exception instance
*
* @return	None
*
* @note		None
*
******************************************************************************/
static void BramIntrExceptionHandler(void *InstancePtr)
{
	u32 Esr = mfesr();
	if ((Esr & 0x81F) == 0x804)
		ExceptionCount++;
}
