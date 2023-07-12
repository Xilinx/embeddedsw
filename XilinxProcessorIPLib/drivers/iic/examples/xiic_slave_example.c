/******************************************************************************
* Copyright (C) 2006 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xiic_slave_example.c
*
* This file consists of a Interrupt mode design example which uses the Xilinx
* IIC device and XIic driver to exercise the slave functionality of the IIC
* device.
*
* The XIic_SlaveSend() API is used to transmit the data and
* XIic_SlaveRecv() API is used to receive the data.
*
* The example is tested on ML300/ML310/ML403/ML501 Xilinx boards.
*
* The IIC devices that are present on the Xilinx boards donot support the Master
* functionality. This example has been tested with an off board external IIC
* Master device and the IIC device configured as a Slave.
*
* This code assumes that no Operating System is being used.
*
* @note
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a mta  03/01/06 Created.
* 2.00a ktn  11/17/09 Updated to use the HAL APIs and replaced call to
*		      XIic_Initialize API with XIic_LookupConfig and
*		      XIic_CfgInitialize. Some of the macros have been
*		      renamed in the IIC driver and some renamed macros are
*		      used in this example.
* 3.4   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
* 3.10  gm   07/09/23 Added SDT support.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xiic.h"
#ifndef SDT
#ifdef XPAR_INTC_0_DEVICE_ID
#include "xintc.h"
#else
#include "xscugic.h"
#endif
#endif
#include "xil_exception.h"
#include "xil_printf.h"
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
#define IIC_DEVICE_ID		XPAR_IIC_0_DEVICE_ID
#else
#define	XIIC_BASEADDRESS	XPAR_XIIC_0_BASEADDR
#endif

#ifndef SDT
#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC_DEVICE_ID	XPAR_INTC_0_DEVICE_ID
#define IIC_INTR_ID	XPAR_INTC_0_IIC_0_VEC_ID
#define INTC			XIntc
#define INTC_HANDLER	XIntc_InterruptHandler
#else
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define IIC_INTR_ID		XPAR_FABRIC_IIC_0_VEC_ID
#define INTC			 	XScuGic
#define INTC_HANDLER		XScuGic_InterruptHandler
#endif
#endif
/*
 * The following constant defines the address of the IIC device on the IIC bus.
 * Since the address is only 7 bits, this constant is the address divided by 2.
 */
#define SLAVE_ADDRESS		0x70	/* 0xE0 as an 8 bit number. */

#define RECEIVE_COUNT		25
#define SEND_COUNT		25


/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int IicSlaveExample();
int SlaveWriteData(u16 ByteCount);
int SlaveReadData(u8 *BufferPtr, u16 ByteCount);
#ifndef SDT
static int SetupInterruptSystem(XIic *IicInstPtr);
#endif
static void StatusHandler(XIic *InstancePtr, int Event);
static void SendHandler(XIic *InstancePtr);
static void ReceiveHandler(XIic *InstancePtr);

/************************** Variable Definitions *****************************/

XIic IicInstance;		/* The instance of the IIC device. */
#ifndef SDT
INTC Intc; 	/* The instance of the Interrupt Controller Driver */
#endif

u8 WriteBuffer[SEND_COUNT];	/* Write buffer for writing a page. */
u8 ReadBuffer[RECEIVE_COUNT];	/* Read buffer for reading a page. */

volatile u8 TransmitComplete;
volatile u8 ReceiveComplete;

volatile u8 SlaveRead;
volatile u8 SlaveWrite;

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* Main function to call the IIC Slave example.
*
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	int Status;

	/*
	 * Run the IIC Slave example.
	 */
	Status = IicSlaveExample();
	if (Status != XST_SUCCESS) {
		xil_printf("IIC slave Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran IIC slave Example\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function writes and reads the data as a slave. The IIC master on the bus
* initiates the transfers.
*
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int IicSlaveExample(void)
{
	int Status;
	u8 Index;
	XIic_Config *ConfigPtr;	/* Pointer to configuration data */


	/*
	 * Initialize the IIC driver so that it is ready to use.
	 */
#ifndef SDT
	ConfigPtr = XIic_LookupConfig(IIC_DEVICE_ID);
#else
	ConfigPtr = XIic_LookupConfig(XIIC_BASEADDRESS);
#endif
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}

	Status = XIic_CfgInitialize(&IicInstance, ConfigPtr,
				    ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the Interrupt System.
	 */
#ifndef SDT
	Status = SetupInterruptSystem(&IicInstance);
#else
	Status = XSetupInterruptSystem(&IicInstance, &XIic_InterruptHandler,
				       ConfigPtr->IntrId, ConfigPtr->IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Include the Slave functions.
	 */
	XIic_SlaveInclude();

	/*
	 * Set the Transmit, Receive and Status Handlers.
	 */
	XIic_SetStatusHandler(&IicInstance, &IicInstance,
			      (XIic_StatusHandler) StatusHandler);
	XIic_SetSendHandler(&IicInstance, &IicInstance,
			    (XIic_Handler) SendHandler);
	XIic_SetRecvHandler(&IicInstance, &IicInstance,
			    (XIic_Handler) ReceiveHandler);

	/*
	 * Set the Address as a RESPOND type.
	 */
	Status = XIic_SetAddress(&IicInstance, XII_ADDR_TO_RESPOND_TYPE,
				 SLAVE_ADDRESS);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * The IIC Master on this bus should initiate the transfer
	 * and write data to the slave at this instance.
	 */
	SlaveReadData(ReadBuffer, RECEIVE_COUNT);

	for (Index = 0; Index < SEND_COUNT; Index++) {
		WriteBuffer[Index] = Index;
	}

	/*
	 * The IIC Master on this bus should initiate the transfer
	 * and read data from the slave.
	 */
	SlaveWriteData(SEND_COUNT);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function reads a buffer of bytes  when the IIC Master on the bus writes
* data to the slave device.
*
* @param	BufferPtr contains the address of the data buffer to be filled.
* @param	ByteCount contains the number of bytes in the buffer to be read.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None
*
******************************************************************************/
int SlaveReadData(u8 *BufferPtr, u16 ByteCount)
{
	int Status;

	/*
	 * Set the defaults.
	 */
	ReceiveComplete = 1;

	/*
	 * Start the IIC device.
	 */
	Status = XIic_Start(&IicInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the Global Interrupt Enable.
	 */
	XIic_IntrGlobalEnable(IicInstance.BaseAddress);

	/*
	 * Wait for AAS interrupt and completion of data reception.
	 */
	while ((ReceiveComplete) || (XIic_IsIicBusy(&IicInstance) == TRUE)) {
		if (SlaveRead) {
			XIic_SlaveRecv(&IicInstance, ReadBuffer, RECEIVE_COUNT);
			SlaveRead = 0;
		}
	}

	/*
	 * Disable the Global Interrupt Enable.
	 */
	XIic_IntrGlobalDisable(IicInstance.BaseAddress);

	/*
	 * Stop the IIC device.
	 */
	Status = XIic_Stop(&IicInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function writes a buffer of bytes to the IIC bus when the IIC master
* initiates a read operation.
*
* @param	ByteCount contains the number of bytes in the buffer to be
*		written.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int SlaveWriteData(u16 ByteCount)
{
	int Status;

	/*
	 * Set the defaults.
	 */
	TransmitComplete = 1;

	/*
	 * Start the IIC device.
	 */
	Status = XIic_Start(&IicInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the Global Interrupt Enable.
	 */
	XIic_IntrGlobalEnable(IicInstance.BaseAddress);

	/*
	 * Wait for AAS interrupt and transmission to complete.
	 */
	while ((TransmitComplete) || (XIic_IsIicBusy(&IicInstance) == TRUE)) {
		if (SlaveWrite) {
			XIic_SlaveSend(&IicInstance, WriteBuffer, SEND_COUNT);
			SlaveWrite = 0;
		}
	}

	/*
	 * Disable the Global Interrupt Enable bit.
	 */
	XIic_IntrGlobalDisable(IicInstance.BaseAddress);

	/*
	 * Stop the IIC device.
	 */
	Status = XIic_Stop(&IicInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* This Status handler is called asynchronously from an interrupt context and
* indicates the events that have occurred.
*
* @param	InstancePtr is not used, but contains a pointer to the IIC
* 		device driver instance which the handler is being called for.
* @param	Event indicates whether it is a request for a write or read.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
static void StatusHandler(XIic *InstancePtr, int Event)
{
	/*
	 * Check whether the Event is to write or read the data from the slave.
	 */
	if (Event == XII_MASTER_WRITE_EVENT) {
		/*
		 * Its a Write request from Master.
		 */
		SlaveRead = 1;
	} else {
		/*
		 * Its a Read request from the master.
		 */
		SlaveWrite = 1;
	}
}

/****************************************************************************/
/**
* This Send handler is called asynchronously from an interrupt
* context and indicates that data in the specified buffer has been sent.
*
* @param	InstancePtr is a pointer to the IIC driver instance for which
*		the handler is being called for.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
static void SendHandler(XIic *InstancePtr)
{
	TransmitComplete = 0;
}

/****************************************************************************/
/**
* This Receive handler is called asynchronously from an interrupt
* context and indicates that data in the specified buffer has been Received.
*
* @param	InstancePtr is a pointer to the IIC driver instance for which
* 		the handler is being called for.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
static void ReceiveHandler(XIic *InstancePtr)
{
	ReceiveComplete = 0;
}

#ifndef SDT
/****************************************************************************/
/**
* This function setups the interrupt system so interrupts can occur for the
* IIC. The function is application-specific since the actual system may or
* may not have an interrupt controller. The IIC device could be directly
* connected to a processor without an interrupt controller. The user should
* modify this function to fit the application.
*
* @param	IicInstPtr contains a pointer to the instance of the IIC  which
*		is going to be connected to the interrupt controller.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
****************************************************************************/
static int SetupInterruptSystem(XIic *IicInstPtr)
{
	int Status;

#ifdef XPAR_INTC_0_DEVICE_ID

	/*
	 * Initialize the interrupt controller driver so that it's ready to use.
	 */
	Status = XIntc_Initialize(&Intc, INTC_DEVICE_ID);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device.
	 */
	Status = XIntc_Connect(&Intc, IIC_INTR_ID,
			       (XInterruptHandler) XIic_InterruptHandler,
			       IicInstPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the interrupt controller so interrupts are enabled for all
	 * devices that cause interrupts.
	 */
	Status = XIntc_Start(&Intc, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the interrupts for the IIC device.
	 */
	XIntc_Enable(&Intc, IIC_INTR_ID);

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

	Status = XScuGic_CfgInitialize(&Intc, IntcConfig,
				       IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XScuGic_SetPriorityTriggerType(&Intc, IIC_INTR_ID,
				       0xA0, 0x3);

	/*
	 * Connect the interrupt handler that will be called when an
	 * interrupt occurs for the device.
	 */
	Status = XScuGic_Connect(&Intc, IIC_INTR_ID,
				 (Xil_InterruptHandler)XIic_InterruptHandler,
				 IicInstPtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Enable the interrupt for the IIC device.
	 */
	XScuGic_Enable(&Intc, IIC_INTR_ID);

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
				     &Intc);

	/*
	 * Enable non-critical exceptions.
	 */
	Xil_ExceptionEnable();


	return XST_SUCCESS;
}
#endif
