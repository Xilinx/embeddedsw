/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xbram_intr_example.c
* @brief Demonstrates the XBram interrupt example.
*
* This file contains a design example using the BRAM driver
* (XBram) in an interrupt driven mode of operation. This example assumes
* that there is an interrupt controller in the hardware system and the
* BRAM device is connected to the interrupt controller.
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
* 4.9   sd   07/07/23 Added SDT support.
* 4.13  ml   07/14/25 Fix GCC warnings.
* 4.14  vlt  12/12/25 Update Doxygen comments to include SDT flow details.
* 4.15  vn   07/30/26 Fix Doxygen warnings
* 4.15  tb  09/15/26 Align Doxygen comments with AMD standards.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xbram.h"
#include "xil_exception.h"
#ifndef SDT
#include "xintc.h"
#endif
#include "xil_cache.h"
#include <stdio.h>
#ifdef SDT
#include "xinterrupt_wrap.h"
#endif

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define BRAM_DEVICE_ID		XPAR_BRAM_1_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#define INTC_BRAM_INTERRUPT_ID	XPAR_INTC_0_BRAM_1_VEC_ID
#endif

/************************** Function Prototypes ******************************/
static void BramDriverHandler(void *CallBackRef);

#ifndef SDT
static void BramIntrExceptionHandler(void *InstancePtr);
int BramIntrExample(XIntc* IntcInstancePtr, XBram* InstancePtr,
		     u16 DeviceId, u16 IntrId);
#else
int BramIntrExample(XBram* InstancePtr, UINTPTR BaseAddress);
#endif

static void InitializeECC(XBram_Config *ConfigPtr, u32 EffectiveAddr);

#ifndef SDT
static int BramSetupIntrSystem(XIntc* IntcInstancePtr, XBram* InstancePtr,
			 u16 DeviceId, u16 IntrId);

static void BramDisableIntr(XIntc* IntcInstancePtr, XBram* InstancePtr,
		      u16 IntrId);
#endif

/************************** Variable Definitions *****************************/

/*
 * The following are declared globally so they are zeroed and so they are
 * easily accessible from a debugger
 */
 #ifndef TESTAPP_GEN
XBram Bram;	/**< The instance of the BRAM driver. */

#ifndef SDT
XIntc Intc;	/**< The instance of the interrupt controller driver. */
#endif
#endif

static volatile u32 IntrCount; /**< Total number of interrupts. */
static volatile int ExceptionCount; /**< Total number of exceptions. */

/****************************************************************************/
/**
* @brief Runs the BRAM interrupt example.
*
* Initializes the BRAM device and sets up interrupts.
*
* @return
*		- XST_SUCCESS to indicate success.
*		- XST_FAILURE to indicate failure.
*
*****************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	int Status;

#ifndef SDT
	Status = BramIntrExample(&Intc, &Bram, BRAM_DEVICE_ID,
				 INTC_BRAM_INTERRUPT_ID);
#else
	Status = BramIntrExample(&Bram, XPAR_XBRAM_0_BASEADDR);
#endif
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
* @brief Tests BRAM interrupts from a TestAppGen application.
*
* @param[out]	InstancePtr Reference to the BRAM driver Instance.
* @if SDT
* @param[in]	BaseAddress Contains the base address of the device.
* @else
* @param[out]	IntcInstancePtr Reference to the Interrupt Controller
*   driver Instance.
* @param[in]	DeviceId BRAM device ID value from
*   xparameters.h.
* @param[in]	IntrId BRAM interrupt vector ID value from
*   xparameters.h.
* @endif
*
* @return
*		- XST_SUCCESS if the example is successful.
*		- XST_FAILURE if the example failed.
*
* @note	        In XSCT/classic flow, DeviceId is used to look up the device
*               configuration.
*
*****************************************************************************/
#ifndef SDT
int BramIntrExample(XIntc* IntcInstancePtr, XBram* InstancePtr,
		     u16 DeviceId, u16 IntrId)
#else
int BramIntrExample(XBram* InstancePtr, UINTPTR BaseAddress)
#endif
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
#ifndef SDT
	ConfigPtr = XBram_LookupConfig(DeviceId);
#else
	ConfigPtr = XBram_LookupConfig(BaseAddress);
#endif
	if (ConfigPtr == (XBram_Config *) NULL) {
		return XST_FAILURE;
	}

	Status = XBram_CfgInitialize(InstancePtr, ConfigPtr,
				     ConfigPtr->CtrlBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


        InitializeECC(ConfigPtr, ConfigPtr->CtrlBaseAddress);


#ifndef SDT
	Status = BramSetupIntrSystem(IntcInstancePtr,
					InstancePtr, DeviceId,
					IntrId);
#else
	Status = XSetupInterruptSystem(InstancePtr, &BramDriverHandler,
			               InstancePtr->Config.IntId,
				       InstancePtr->Config.IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	IntrCount = 0;
	ExceptionCount = 0;

	/*
	 * Execute the BRAM driver selftest, and then disable exceptions again.
	 * Running the driver test injects errors if Fault Injection is
	 * enabled.
	 */
	Status = XBram_SelfTest(InstancePtr, XBRAM_IR_ALL_MASK);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#ifndef SDT
	BramDisableIntr(IntcInstancePtr, InstancePtr, IntrId);
#else
	XDisconnectInterruptCntrl(InstancePtr->Config.IntId,
				       InstancePtr->Config.IntrParent);
#endif

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

#ifndef SDT
/*****************************************************************************/
/**
*
* @brief Sets up BRAM interrupts.
*
* @param[out]	IntcInstancePtr Reference to the Interrupt Controller
*   driver Instance.
* @param[in]	InstancePtr Reference to the BRAM driver Instance.
* @param[in]	DeviceId BRAM device ID value from
*   xparameters.h.
* @param[in]	IntrId BRAM interrupt ID value from
*   xparameters.h.
*
* @return
*		- XST_SUCCESS if the interrupt setup is successful.
*		- XST_FAILURE if interrupt setup is not successful.
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
#endif


/****************************************************************************/
/**
* @brief Initializes ECC in BRAM when hardware initialization is not available.
*
* Ensures that ECC in the BRAM is initialized if no hardware
* initialization is available. The ECC bits are initialized by reading and
* writing data in the memory. This code is not optimized to only read data
* in initialized sections of the BRAM.
*
* @param[in] ConfigPtr Reference to a structure containing information
*   about a specific BRAM device.
* @param[in] EffectiveAddr Device base address in the virtual memory
*   address space.
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
* @brief Handles the BRAM interrupt for this example.
*
* @param[in]	CallbackRef Callback reference for the handler.
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


#ifndef SDT
/*****************************************************************************/
/**
*
* @brief Disables BRAM interrupts and exceptions.
*
* @param[in]	IntcInstancePtr Pointer to the Interrupt Controller
*   driver Instance.
* @param[in]	InstancePtr Pointer to the BRAM driver Instance.
* @param[in]	IntrId BRAM interrupt vector ID value from
*   xparameters.h.
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
#endif


/*****************************************************************************/
/**
* @brief Counts instruction or data bus exceptions.
*
* @param[in]	InstancePtr Points to the exception instance.
*
******************************************************************************/
#ifndef SDT
static void BramIntrExceptionHandler(void *InstancePtr)
{
	(void)InstancePtr;
	u32 Esr = mfesr();
	if ((Esr & 0x81F) == 0x804)
		ExceptionCount++;
}
#endif
