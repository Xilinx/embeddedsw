/******************************************************************************
* Copyright (C) 2006 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xiic_multi_master_example.c
*
* This file consists of a Interrupt mode design example which uses the Xilinx
* IIC device and XIic driver to exercise the EEPROM on the Xilinx boards in a
* Multi master mode. This example has been tested with an off-board external
* IIC Master connected on the IIC bus.
*
* This example writes/reads from the lower 256 bytes of the IIC EEPROMS. Please
* refer to the datasheets of the IIC EEPROM's for details about the internal
* addressing and page size of these devices.
*
* The XIic_MasterSend() API is used to transmit the data and XIic_MasterRecv()
* API is used to receive the data.
*
* The example is tested on ML300/ML310/ML403/ML501 Xilinx boards.
*
* The ML310/ML410/ML510 boards have a on-board 64 Kb serial IIC EEPROM
* (Microchip 24LC64A). The WP pin of the IIC EEPROM is hardwired to ground on
* this board.
*
* The ML300 board has an on-board 32 Kb serial IIC EEPROM(Microchip 24LC32A).
* The WP pin of the IIC EEPROM has to be connected to ground for this example.
* The WP is connected to pin Y3 of the FPGA.
*
* The ML403 board has an on-board 4 Kb serial IIC EEPROM(Microchip 24LC04A).
* The WP pin of the IIC EEPROM is hardwired to ground on this board.
*
* The ML501/ML505/ML507/ML605/SP601/SP605 boards have an on-board 8 Kb serial
* IIC EEPROM(STM M24C08). The WP pin of the IIC EEPROM is hardwired to
* ground on these boards.
*
* The AddressType for ML300/ML310/ML410/ML510 boards should be u16 as the
* address pointer in the on board EEPROM is 2 bytes.
*
* The AddressType for ML403/ML501/ML505/ML507/ML605/SP601/SP605 boards should
* be u8 as the address pointer for the on board EEPROM is 1 byte.
*
* The 7 bit IIC Slave address of the IIC EEPROM on the ML300/ML310/ML403/ML410/
* ML501/ML505/ML507/ML510 boards is 0x50.
* The 7 bit IIC Slave address of the IIC EEPROM on the ML605/SP601/SP605 boards
* is 0x54.
* Refer to the User Guide's of the respective boards for further information
* about the IIC slave address of IIC EEPROM's.
*
* The define EEPROM_ADDRESS in this file needs to be changed depending on
* the board on which this example is to be run.
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
* 2.00a sdm  09/22/09 Converted all register accesses to 32 bit access.
*		      Updated to use the HAL APIs, replaced call to
*		      XIic_Initialize API with XIic_LookupConfig and
*		      XIic_CfgInitialize.
* 2.01a ktn  03/17/10 Updated the information about the EEPROM's used on
*		      ML605/SP601/SP605 boards.
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
 * The following constant defines the address of the IIC Slave device on the
 * IIC bus. Note that since the address is only 7 bits, this constant is the
 * address divided by 2.
 * The 7 bit IIC Slave address of the IIC EEPROM on the ML300/ML310/ML403/ML501/
 * ML410/ML505/ML507/ML510 boards is 0x50. The 7 bit IIC Slave address of the
 * IIC EEPROM on the ML605/SP601/SP605 boards is 0x54.
 * Please refer the User Guide's of the respective boards for further
 * information about the IIC slave address of IIC EEPROM's.
 */
#define EEPROM_ADDRESS 0x50	/* 0xA0 as an 8 bit number. */

/*
 * The page size determines how much data should be written at a time.
 * The ML300 board supports a page size of 32 and 16.
 * The write function should be called with this as a maximum byte count.
 */
#define PAGE_SIZE   16

/*
 * The Starting address in the IIC EEPROM on which this test is performed.
 */
#define EEPROM_TEST_START_ADDRESS   128

/**************************** Type Definitions *******************************/

/*
 * The AddressType for ML300/ML310/ML410/ML510 boards should be u16 as the
 * address pointer in the on board EEPROM is 2 bytes.
 * The AddressType for ML403/ML501/ML505/ML507/ML605/SP601/SP605 boards should
 * be u8 as the address pointer in the on board EEPROM is 1 bytes.
 */
typedef u8 AddressType;


/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int IicMultiMasterExample();

#ifndef SDT
static int SetupInterruptSystem(XIic *IicInstPtr);
#endif

static void SendHandler(XIic *InstancePtr);
static void StatusHandler(XIic *InstancePtr, int Event);

/************************** Variable Definitions *****************************/

XIic IicInstance;		/* The instance of the IIC device. */
#ifndef SDT
INTC Intc; 	/* The instance of the Interrupt Controller Driver */
#endif

/*
 * Write buffer for writing a page.
 */
u8 WriteBuffer[sizeof(AddressType) + PAGE_SIZE];

volatile u8 TransmitComplete;
volatile u8 ReceiveComplete;
volatile u8 BusNotBusy;

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* Main function to call the Multi Master example.
*
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note 	None.
*
******************************************************************************/
int main(void)
{
	int Status;

	/*
	 * Run the Multi master example.
	 */
	Status = IicMultiMasterExample();
	if (Status != XST_SUCCESS) {
		xil_printf("IIC multi master Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran IIC multi master Example\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function writes the data to the IIC EEPROM.
*
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
****************************************************************************/
int IicMultiMasterExample(void)
{
	u8 Index;
	int Status;
	XIic_Config *ConfigPtr;	/* Pointer to configuration data */
	AddressType Address = EEPROM_TEST_START_ADDRESS;

	/*
	 * Initialize the data to write and the read buffer.
	 */
	if (sizeof(Address) == 1) {
		WriteBuffer[0] = (u8) (Address);
	} else {
		WriteBuffer[0] = (u8) (Address >> 8);
		WriteBuffer[1] = (u8) (Address);
	}
	for (Index = 0; Index < PAGE_SIZE; Index++) {
		WriteBuffer[sizeof(Address) + Index] = 0xFF;
	}

	/*
	 * Include the multi master functionality.
	 */
	XIic_MultiMasterInclude();

	/*
	 * Initialize the IIC driver so that it is ready to use.
	 */
#ifndef SDT
	ConfigPtr = XIic_LookupConfig(XPAR_IIC_0_DEVICE_ID);
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
	 * Set the Transmit and status handlers.
	 */
	XIic_SetSendHandler(&IicInstance, &IicInstance,
			    (XIic_Handler) SendHandler);
	XIic_SetStatusHandler(&IicInstance, &IicInstance,
			      (XIic_StatusHandler) StatusHandler);

	/*
	 * Set the address of the slave.
	 */
	Status = XIic_SetAddress(&IicInstance, XII_ADDR_TO_SEND_TYPE,
				 EEPROM_ADDRESS);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	IicInstance.Stats.TxErrors = 0;
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
	 * Write to the EEPROM.
	 */
	XIic_MasterSend(&IicInstance, WriteBuffer, PAGE_SIZE);

	while (1) {
		/*
		 * If arbitration is lost and some time later Bus if bus
		 * becomes free transmit the data.
		 */
		if (BusNotBusy) {
			/*
			 * Start the IIC device.
			 */
			Status = XIic_Start(&IicInstance);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}

			/*
			 * Send the data.
			 */
			XIic_MasterSend(&IicInstance, WriteBuffer, PAGE_SIZE);

			/*
			 * Clear the Flag.
			 */
			BusNotBusy = 0;
		}

		/*
		 * This condition is required to be checked in the case where we
		 * are writing two consecutive buffers of data to the EEPROM.
		 * The EEPROM takes about 2 milliseconds time to update the data
		 * internally after a STOP has been sent on the bus.
		 * A NACK will be generated in the case of a second write before
		 * the EEPROM updates the data internally resulting in a
		 * Transmission Error.
		 */
		if (IicInstance.Stats.TxErrors != 0) {
			/*
			 * If the Slave didn't acknowledge then we should keep
			 * making attempts to transmit the data.
			 */
			IicInstance.Stats.TxErrors = 0;
			/*
			 * Start the IIC device.
			 */
			Status = XIic_Start(&IicInstance);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}

			/*
			 * Send the data.
			 */
			XIic_MasterSend(&IicInstance, WriteBuffer, PAGE_SIZE);
		}

		if ((!TransmitComplete) &&
		    (XIic_IsIicBusy(&IicInstance) == FALSE)) {
			break;
		}
	}

	/*
	 * Stop the IIC device.
	 */
	Status = XIic_Stop(&IicInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
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
* @param	IicInstPtr contains a pointer to the instance of the IIC which
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

/*****************************************************************************/
/**
* The Send handler is called asynchronously from an interrupt context and
* indicates that data in the specified buffer has been sent.
*
* @param	InstancePtr is a pointer to the IIC driver instance for which
*		the handler is being called for.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void SendHandler(XIic *InstancePtr)
{
	TransmitComplete = 0;
}

/*****************************************************************************/
/**
* The Status handler is called asynchronously from an interrupt context and
* indicates the events that have occurred.
*
* @param	InstancePtr is a pointer to the IIC driver instance for which
* 		the handler is being called for.
* @param	Event indicates the condition that has occurred.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void StatusHandler(XIic *InstancePtr, int Event)
{
	if (Event == XII_ARB_LOST_EVENT) {
		XIic_WriteReg(InstancePtr->BaseAddress, XIIC_CR_REG_OFFSET,
			      XIIC_CR_ENABLE_DEVICE_MASK);
		XIic_WriteIisr(InstancePtr->BaseAddress, XIIC_INTR_BNB_MASK);
		XIic_WriteIier(InstancePtr->BaseAddress, XIIC_INTR_BNB_MASK);
		InstancePtr->BNBOnly = TRUE;
	} else if (Event == XII_BUS_NOT_BUSY_EVENT) {
		XIic_WriteReg(InstancePtr->BaseAddress, XIIC_CR_REG_OFFSET,
			      0x0);
		BusNotBusy = 1;
	}
}
