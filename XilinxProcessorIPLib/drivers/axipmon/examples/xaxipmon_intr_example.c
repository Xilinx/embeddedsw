/******************************************************************************
* Copyright (C) 2012 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xaxipmon_intr_example.c
*
* This file contains a design example showing how to use the driver APIs of the
* AXI Performance Monitor driver in interrupt mode.
*
*
* @note
*
* Metric Counters are enabled. The Application/design for which Metrics need
* to be computed should be run and then the Metrics collected.
* Sampled Metric Counter is read after Sample Metric Counter Interrupt
* occurs.
*
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.00a bss    02/29/12 First release
* 2.00a bss    06/23/12 Updated to support v2_00a version of IP.
* 3.00a bss    09/03/12 Deleted XAxiPmon_SetAgent API to support
*						v2_01a version of IP.
* 3.01a bss	   10/25/12 Deleted XAxiPmon_EnableCountersData API to support
*						new version of IP.
* 5.00a bss	10/25/12 Modified call to XAxiPmon_SetSampleInterval as per
*			 new driver API.
* 6.4 mus    01/07/16 Added support for ZynqMP interrupt controller
* 6.5 ms     01/23/17 Modified xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings are
*                     available in all examples. This is a fix for CR-965028.
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xaxipmon.h"
#include "xparameters.h"
#include "xstatus.h"
#include "xil_exception.h"
#include "stdio.h"

#ifdef XPAR_INTC_0_DEVICE_ID
#include "xintc.h"
#else
#include "xscugic.h"
#endif
/************************** Constant Definitions ****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC						XIntc
#define INTC_HANDLER				XIntc_InterruptHandler
#define AXIPMON_DEVICE_ID 			XPAR_AXIPMON_0_DEVICE_ID
#define INTC_DEVICE_ID				XPAR_INTC_0_DEVICE_ID
#define INTC_AXIPMON_INTERRUPT_ID		XPAR_INTC_0_AXIPMON_0_VEC_ID
#else
#define INTC						XScuGic
#define INTC_HANDLER				XScuGic_InterruptHandler
#define AXIPMON_DEVICE_ID			XPAR_AXIPMON_0_DEVICE_ID
#define INTC_DEVICE_ID				XPAR_SCUGIC_0_DEVICE_ID
#define INTC_AXIPMON_INTERRUPT_ID	XPAR_XAPMPS_0_INTR
#endif

/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/

int AxiPmonInterruptExample(u16 AxiPmonDeviceId, u32 *Metrics);

static void AxiPmonInterruptHandler(void *CallBackRef);

static int AxiPmonSetupIntrSystem(INTC* IntcInstancePtr,
					XAxiPmon* InstancePtr, u16 IntrId);

/************************** Variable Definitions ****************************/

static XAxiPmon AxiPmonInst;	/* AXI Performance Monitor driver instance */
INTC Intc;	/* The Instance of the Interrupt Controller Driver */

/*
 * Shared variables used to test the callbacks.
 */
volatile static int SampleCounterIntr = FALSE;  /*
						 * Sample Interval Counter
						 * Overflow interrupt
						 */

/****************************************************************************/
/**
*
* Main function that invokes the example in this file.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note		None.
*
*****************************************************************************/
int main(void)
{

	int Status;
	u32 Metrics;

	/*
	 * Run the AxiPmon Interrupt example, specify the Device ID that is
	 * generated in xparameters.h .
	 */
	Status = AxiPmonInterruptExample(AXIPMON_DEVICE_ID, &Metrics);

	if (Status != XST_SUCCESS) {
		xil_printf("AXI Performance Monitor Interrupt example \
							failed\r\n");
		return XST_FAILURE;
	}
	xil_printf("Successfully ran AXI Performance Monitor Interrupt \
						Example\r\n");
	return XST_SUCCESS;
}


/*****************************************************************************/
/**
*
* This function runs a test on the AXI Performance Monitor device using the
* driver APIs.
* This function does the following tasks:
*	- Initiate the AXI Performance Monitor device driver instance
*	- Run self-test on the device
*	- Setup Interrupt System
*	- Sets Agent Number
*	- Sets Metric Set 0 as Metrics for Metric Counter 0
*	- Sets Upper and Lower Ranges for Incrementer 0
*	- Sets and loads Sample Interval
*	- Enables Metric Counters
*	- Calls Application for which Metrics need to be computed
*	- Enables Sample Interval Counter Interrupt
*	- Wait for interrupt and disables Interrupts after Interrupt occurs
*	- Reads Sampled Metric Counter 0
*	- Disables Metric Counters
*
* @param	AxiPmonDeviceId is the XPAR_<AXIPMON_instance>_DEVICE_ID value
*		from xparameters.h.
* @param	Metrics is an user referece variable in which computed metrics
*			will be filled
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note   	None
*
******************************************************************************/
int AxiPmonInterruptExample(u16 AxiPmonDeviceId, u32 *Metrics)
{
	int Status;
	XAxiPmon_Config *ConfigPtr;
	u8 SlotId = 0x1;
	u16 Range2 = 0x10;	/* Range 2 - 16 */
	u16 Range1 = 0x08;	/* Range 1 - 8 */
	XAxiPmon *AxiPmonInstPtr = &AxiPmonInst;
	u32 SampleInterval  = 0x3FFFF;

	/*
	 * Initialize the AxiPmon driver.
	 */
	ConfigPtr = XAxiPmon_LookupConfig(AxiPmonDeviceId);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}
	XAxiPmon_CfgInitialize(AxiPmonInstPtr, ConfigPtr,
				ConfigPtr->BaseAddress);

	/*
	 * Self Test the Axi Performance Monitor device
	 */
	Status = XAxiPmon_SelfTest(AxiPmonInstPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	Status = AxiPmonSetupIntrSystem(&Intc, AxiPmonInstPtr,
					INTC_AXIPMON_INTERRUPT_ID);


	/*
	 * Select Agent and required set of Metrics for a Counter.
	 * We can select another agent,Metrics for another counter by
	 * calling below function again.
	 */

	XAxiPmon_SetMetrics(AxiPmonInstPtr, SlotId, XAPM_METRIC_SET_0,
				XAPM_METRIC_COUNTER_0);

	/*
	 * Set Incrementer Ranges
	 */
	XAxiPmon_SetIncrementerRange(AxiPmonInstPtr, XAPM_INCREMENTER_0,
							Range2, Range1);


	XAxiPmon_SetSampleInterval(AxiPmonInstPtr, SampleInterval);

	XAxiPmon_LoadSampleIntervalCounter(AxiPmonInstPtr);

	/*
	 * Enable Metric Counters.
	 */
	XAxiPmon_EnableMetricsCounter(AxiPmonInstPtr);


	/*
	 * Enable Sample Interval Counter Overflow Interrupt
	 */
	XAxiPmon_IntrEnable(AxiPmonInstPtr, XAPM_IXR_SIC_OVERFLOW_MASK);

	/*
	 * Enable Global Interrupt
	 */
	XAxiPmon_IntrGlobalEnable(AxiPmonInstPtr);


	/*
	 * Application for which Metrics has to be computed should be
	 * called here
	 */

	/*
	 * Enable Sample Interval Counter and wait for interrupt to occur
	 */
	XAxiPmon_EnableSampleIntervalCounter(AxiPmonInstPtr);


	/** Wait until Sample Interval Overflow occurs */
	while(!(SampleCounterIntr));


	/** Disable Sample Interval Counter */
	XAxiPmon_DisableSampleIntervalCounter(AxiPmonInstPtr);

	/** Disable Sample Interval Counter Overflow Interrupt */
	XAxiPmon_IntrDisable(AxiPmonInstPtr, XAPM_IXR_SIC_OVERFLOW_MASK);

	/** Disable Global Interrupt */
	XAxiPmon_IntrGlobalDisable(AxiPmonInstPtr);

	/* Get Sampled Metric Counter 0 in Metrics */
	*Metrics = XAxiPmon_GetSampledMetricCounter(AxiPmonInstPtr,
					XAPM_METRIC_COUNTER_0);
	/*
	 * Disable Metric Counters.
	 */
	XAxiPmon_DisableMetricsCounter(AxiPmonInstPtr);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is the Interrupt Service Routine for the AXI Performance
* Monitor device. It will be called by the processor whenever an interrupt is
* asserted by the device.
*
* There are 13 different interrupts supported
*	- Sample Interval Counter Overflow Interrupt
*	- Global Clock Counter Overflow Interrupt
*	- Event Log FIFO full Interrupt
*	- Metric Counter 0 overflow Interrupt
*	- Metric Counter 1 overflow Interrupt
*	- Metric Counter 2 overflow Interrupt
*	- Metric Counter 3 overflow Interrupt
*	- Metric Counter 4 overflow Interrupt
*	- Metric Counter 5 overflow Interrupt
*	- Metric Counter 6 overflow Interrupt
*	- Metric Counter 7 overflow Interrupt
*	- Metric Counter 8 overflow Interrupt
*	- Metric Counter 9 overflow Interrupt
* This function only handles Sample Interval Counter Overflow Interrupt.
* User of this code may need to modify the code to meet needs of the
* application.
*
* @param	CallBackRef is the callback reference passed from the Interrupt
*		controller driver, which in our case is a pointer to the
*		driver instance.
*
* @return	None.
*
* @note		This function is called within interrupt context.
*
******************************************************************************/
static void AxiPmonInterruptHandler(void *CallBackRef)
{
	u32 IntrStatus;

	XAxiPmon *AxiPmonPtr = (XAxiPmon *)CallBackRef;

	/*
	 * Get the interrupt status from the device and check the value.
	 */
	IntrStatus = XAxiPmon_IntrGetStatus(AxiPmonPtr);

	if (IntrStatus & XAPM_IXR_SIC_OVERFLOW_MASK) {
		/*
		 * Set Sample Interval Counter Overflow interrupt flag so
		 * the code in application context can be aware of this interrupt.
		 */
		SampleCounterIntr = TRUE;

	}

	/*
	 * Clear Interrupt Status Register.
	 */
	XAxiPmon_IntrClear(AxiPmonPtr, IntrStatus);

 }




/*****************************************************************************/
/**
*
* This function performs the AXI Performance Monitor set up for Interrupts
*
* @param	IntcInstancePtr is a reference to the Interrupt Controller
*			driver Instance
* @param	InstancePtr is a reference to the  XAxiPmon driver Instance
* @param	IntrId is XPAR_<INTC_instance>_<AXIPMON_instance>_INTERRUPT_INTR
*			value from xparameters.h
*
* @return
*		- XST_SUCCESS if the interrupt setup is successful.
*		- XST_FAILURE if interrupt setup is not successful.
*
* @note		None.
*
******************************************************************************/
static int AxiPmonSetupIntrSystem(INTC* IntcInstancePtr, XAxiPmon* InstancePtr,
								u16 IntrId)
{
	int Status;
#ifdef XPAR_INTC_0_DEVICE_ID
	/*
	 * Initialize the interrupt controller driver so that it's ready to
	 * use.
	 */
	Status = XIntc_Initialize(IntcInstancePtr, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the handler that will be called when an interrupt
	 * for the device occurs, the handler defined above performs the
	 * specific interrupt processing for the device.
	 */
	Status = XIntc_Connect(IntcInstancePtr, IntrId,
		(XInterruptHandler) AxiPmonInterruptHandler, InstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the interrupt controller so interrupts are enabled for all
	 * devices that cause interrupts. Specify real mode so that the Axi
	 * Performance Monitor device can cause interrupts through the interrupt
	 * controller.
	 */
	Status = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the interrupt for the AXI Performance Monitor.
	 */
	XIntc_Enable(IntcInstancePtr, IntrId);
#else
	XScuGic_Config *IntcConfig;
	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}
	Status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
					IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the handler that will be called when an interrupt
	 * for the device occurs, the handler defined above performs the
	 * specific interrupt processing for the device.
	 */
	Status = XScuGic_Connect(IntcInstancePtr, IntrId,
		(XInterruptHandler) AxiPmonInterruptHandler, InstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	XScuGic_Enable(IntcInstancePtr, IntrId);
#endif
	/*
	 * Initialize the exception table.
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler) INTC_HANDLER,
					IntcInstancePtr);
	/*
	 * Enable exceptions.
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}
