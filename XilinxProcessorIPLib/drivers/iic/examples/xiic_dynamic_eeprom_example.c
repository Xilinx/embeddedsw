/******************************************************************************
* Copyright (C) 2006 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xiic_dynamic_eeprom_example.c
* @brief Shows IIC dynamic-mode EEPROM transfers.
*
* This file consists of a Interrupt mode design example which uses the Xilinx
* IIC device and XIic driver to exercise the EEPROM in Dynamic controller mode.
* The XIic driver uses the complete FIFO functionality to transmit/receive data.
*
* This example writes/reads from the lower 256 bytes of the IIC EEPROMS.
* refer to the datasheets of the IIC EEPROM's for details about the internal
* addressing and page size of these devices.
*
* The XIic_DynMasterSend() API is used to transmit the data and
* XIic_DynMasterRecv() API is used to receive the data.
*
* This example is tested on ML300/ML310/ML403/ML501/ML507/ML510/ML605/SP601 and
* SP605 Xilinx boards.
*
* The ML310/ML410/ML510/SCU35 boards have a on-board 64 Kb serial IIC EEPROM
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
* IIC EEPROM(STM M24C08). The WP pin of the IIC EEPROM is hardwired to ground
* on these boards.
*
* The AddressType for ML300/ML310/ML410/ML510/SCU200/SCU35 boards should be u16 as the
* address pointer in the on board EEPROM is 2 bytes.
*
* The AddressType for ML403/ML501/ML505/ML507/ML605/SP601/SP605 boards should
* be u8 as the address pointer for the on board EEPROM is 1 byte.
*
* The 7 bit IIC Slave address of the IIC EEPROM on the ML300/ML310/ML403/ML410/
* ML501/ML505/ML507/ML510/SCU200 boards is 0x50.
* The 7 bit IIC Slave address of the IIC EEPROM on the ML605/SP601/SP605 boards
* is 0x54.
* Refer to the User Guide's of the respective boards for further information
* about the IIC slave address of IIC EEPROM's.
*
* The define EEPROM_ADDRESS in this file needs to be changed depending on
* the board on which this example is to be run.
*
* This code assumes that no Operating System is being used.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00a mta  04/13/06 Created.
* 2.00a ktn  11/17/09 Updated to use the HAL APIs.
* 2.01a ktn  03/17/10 Updated the information about the EEPROM's used on
*		      ML605/SP601/SP605 boards. Updated the example so that it
*		      can be used to access the entire IIC EEPROM for devices
*		      like M24C04/M24C08 that use LSB bits of the IIC device
*		      select code (IIC slave address) to specify the higher
*		      address bits of the EEPROM internal address.
* 3.4   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
* 3.10  gm   07/09/23 Added SDT support
* 3.15  vlt  02/11/26 Updated comment lines for SCU200 and SCU35 EEPROM address,
*                     page size, and address type used for testing.
* 3.16  tb   09/22/26 Align Doxygen comments with AMD documentation standards
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xiic.h"
#include "xil_sutil.h"
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
/** IIC base address. */
#define	XIIC_BASEADDRESS	XPAR_XIIC_0_BASEADDR
#endif

#ifndef SDT
#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC_DEVICE_ID	XPAR_INTC_0_DEVICE_ID	/**< INTC device ID. */
/** IIC interrupt ID. */
#define IIC_INTR_ID	XPAR_INTC_0_IIC_0_VEC_ID
#define INTC			XIntc	/**< INTC instance type. */
#define INTC_HANDLER	XIntc_InterruptHandler	/**< INTC interrupt handler. */
#else
/** INTC device ID. */
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
/** IIC interrupt ID. */
#define IIC_INTR_ID		XPAR_FABRIC_IIC_0_VEC_ID
#define INTC			 	XScuGic	/**< INTC instance type. */
/** INTC interrupt handler. */
#define INTC_HANDLER		XScuGic_InterruptHandler
#endif
#endif

/*
 * The following constant defines the address of the IIC Slave device on
 * the IIC bus. Note that since the address is only 7 bits, this constant
 * is the address divided by 2. The 7 bit IIC Slave address of the IIC
 * EEPROM on the ML300/ML310/ML403/ML510/ ML501/ML505/ML507/ML510/SCU200
 * boards is 0x50. The 7 bit IIC Slave address of the IIC EEPROM on the
 * ML605/SP601/SP605 boards is 0x54. Refer to the user guide of the
 * respective board for further information about the IIC slave address
 * of IIC EEPROM's.
 */
#define EEPROM_ADDRESS		0x54	/**< 7-bit IIC EEPROM slave address. */

/*
 * The page size determines how much data should be written at a time.
 * The ML310/ML300/SCU200/SCU35 board supports a page size of 32 and 16.
 * The write function should be called with this as a maximum byte count.
 */
#define PAGE_SIZE		16	/**< EEPROM page size in bytes. */

/*
 * Defines the EEPROM start address for this test.
 * The Starting address in the IIC EEPROM on which this test is performed.
 */
#define EEPROM_TEST_START_ADDRESS   128	/**< EEPROM test start address. */

/*
 * Maximum delay count used for timeout handling
 */
/** Maximum delay count for timeout handling. */
#define MAX_DELAY_CNT               10000

/*
 * Internal IIC event mask
 */
#define EventMask                   0xFF	/**< Internal IIC event mask. */

/*
 * Internal event value used for IIC event checking
 */
/** Internal event value for IIC event checking. */
#define Event_Value                 0

/**************************** Type Definitions *******************************/

/**
* @brief Defines the EEPROM address type for the target board.
*
* The AddressType for ML300/ML310/ML410/ML510/SCU200/SCU35 boards should be u16
* as the address pointer in the on board EEPROM is 2 bytes. The AddressType for
* ML403/ML501/ML505/ML507/ML605/SP601/SP605 boards should be u8 as the address
* pointer in the on board EEPROM is 1 bytes.
*/
typedef u8 AddressType;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int IicDynEepromExample();

int DynEepromWriteData(u16 ByteCount);

int DynEepromReadData(u8 *BufferPtr, u16 ByteCount);

#ifndef SDT
static int SetupInterruptSystem(XIic *IicInstPtr);
#endif

static void SendHandler(void *CallBackRef, int ByteCount);

static void ReceiveHandler(void *CallBackRef, int ByteCount);

static void StatusHandler(XIic *InstancePtr, int Event);

/************************** Variable Definitions *****************************/

XIic IicInstance;	/**< The instance of the IIC device. */
#ifndef SDT
INTC Intc; 		/**< The instance of the Interrupt Controller Driver. */
#endif

/**
 * @brief Holds the data for writing a page.
 */
u8 WriteBuffer[sizeof(AddressType) + PAGE_SIZE];

u8 ReadBuffer[PAGE_SIZE];	/**< Read buffer for reading a page. */

volatile u8 TransmitComplete;
	/**< Flag to check completion of Transmission. */
volatile u8 ReceiveComplete;	/**< Flag to check completion of Reception. */

u8 EepromIicAddr;		/**< Variable for storing Eeprom IIC address. */

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* @brief Runs the Dynamic EEPROM example.
*
* Main function to call the Dynamic EEPROM example.
*
* @return
*        - XST_SUCCESS if successful.
*        - XST_FAILURE otherwise.
*
******************************************************************************/
int main(void)
{
	int Status;

	/*
	 * Run the Dynamic EEPROM example.
	 */
	Status = IicDynEepromExample();
	if (Status != XST_SUCCESS) {
		xil_printf("IIC dynamic eeprom Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran IIC dynamic eeprom Example\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Writes, reads, and verifies data on the IIC EEPROM in dynamic mode.
*
* This function writes, reads, and verifies the data to the IIC EEPROM in
* Dynamic controller mode. It does the write as a single page write, performs a
* buffered read.
*
* @return
*        - XST_SUCCESS if successful.
*        - XST_FAILURE otherwise.
*
******************************************************************************/
int IicDynEepromExample(void)
{
	u8 Index;
	int Status;
	XIic_Config *ConfigPtr;	/** Pointer to configuration data */
	AddressType Address = EEPROM_TEST_START_ADDRESS;
	EepromIicAddr = EEPROM_ADDRESS;

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
	 * Initialize the Dynamic IIC core.
	 */
	Status = XIic_DynamicInitialize(&IicInstance);
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
	 * Set the Handlers for transmit and reception.
	 */
	XIic_SetSendHandler(&IicInstance, &IicInstance,
			    (XIic_Handler) SendHandler);
	XIic_SetRecvHandler(&IicInstance, &IicInstance,
			    (XIic_Handler) ReceiveHandler);
	XIic_SetStatusHandler(&IicInstance, &IicInstance,
			      (XIic_StatusHandler) StatusHandler);


	/*
	 * Initialize the data to write and the read buffer.
	 */
	if (sizeof(Address) == 1) {
		WriteBuffer[0] = (u8) (EEPROM_TEST_START_ADDRESS);
		EepromIicAddr |= (EEPROM_TEST_START_ADDRESS >> 8) & 0x7;
	} else {
		WriteBuffer[0] = (u8) (EEPROM_TEST_START_ADDRESS >> 8);
		WriteBuffer[1] = (u8) (EEPROM_TEST_START_ADDRESS);
	}

	for (Index = 0; Index < PAGE_SIZE; Index++) {
		WriteBuffer[sizeof(Address) + Index] = 0xFF;
		ReadBuffer[Index] = 0;
	}

	/*
	 * Set the Slave address.
	 */
	Status = XIic_SetAddress(&IicInstance, XII_ADDR_TO_SEND_TYPE,
				 EepromIicAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Write to the EEPROM.
	 */
	Status = DynEepromWriteData(sizeof(Address) + PAGE_SIZE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Read from the EEPROM.
	 */
	Status = DynEepromReadData(ReadBuffer, PAGE_SIZE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Verify the data read against the data written.
	 */
	for (Index = 0; Index < PAGE_SIZE; Index++) {
		if (ReadBuffer[Index] != WriteBuffer[Index + sizeof(Address)]) {
			return XST_FAILURE;
		}

		ReadBuffer[Index] = 0;
	}

	/*
	 * Initialize the data to write and the read buffer.
	 */
	if (sizeof(Address) == 1) {
		WriteBuffer[0] = (u8) (EEPROM_TEST_START_ADDRESS);
	} else {
		WriteBuffer[0] = (u8) (EEPROM_TEST_START_ADDRESS >> 8);
		WriteBuffer[1] = (u8) (EEPROM_TEST_START_ADDRESS);
	}

	for (Index = 0; Index < PAGE_SIZE; Index++) {
		WriteBuffer[sizeof(Address) + Index] = Index;
	}

	/*
	 * Write to the EEPROM.
	 */
	Status = DynEepromWriteData(sizeof(Address) + PAGE_SIZE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Read from the EEPROM.
	 */
	Status = DynEepromReadData(ReadBuffer, PAGE_SIZE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Verify the data read against the data written.
	 */
	for (Index = 0; Index < PAGE_SIZE; Index++) {
		if (ReadBuffer[Index] != WriteBuffer[Index + sizeof(Address)]) {
			return XST_FAILURE;
		}

		ReadBuffer[Index] = 0;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Writes a buffer of data to the IIC serial EEPROM.
*
* This function writes a buffer of data to the IIC serial EEPROM.
*
* @param[in] ByteCount The number of bytes in the buffer to be written.
*
* @return
*        - XST_SUCCESS if successful.
*        - XST_FAILURE otherwise.
*
* @note The Byte count should not exceed the page size of the EEPROM as noted by
*       the constant PAGE_SIZE.
*
******************************************************************************/
int DynEepromWriteData(u16 ByteCount)
{
	int Status;
	int Timeout = MAX_DELAY_CNT;

	/*
	 * Set the defaults.
	 */
	TransmitComplete = 1;
	IicInstance.Stats.TxErrors = 0;

	/*
	 * Start the IIC device.
	 */
	Status = XIic_Start(&IicInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Send the Data.
	 */
	Status = XIic_DynMasterSend(&IicInstance, WriteBuffer, ByteCount);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait till the transmission is completed.
	 */
	while ((TransmitComplete) || (XIic_IsIicBusy(&IicInstance) == TRUE)) {
		if (Timeout -- == 0) {
			return XST_FAILURE;   /* timeout exit */
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
			 * Enable the IIC device.
			 */
			Status = XIic_Start(&IicInstance);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}


			if (!XIic_IsIicBusy(&IicInstance)) {
				/*
				 * Send the Data.
				 */
				Status = XIic_MasterSend(&IicInstance,
							 WriteBuffer,
							 ByteCount);
				if (Status == XST_SUCCESS) {
					IicInstance.Stats.TxErrors = 0;
				} else {

				}
			}
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

/*****************************************************************************/
/**
* @brief Reads data from the IIC serial EEPROM into a specified buffer.
*
* This function reads data from the IIC serial EEPROM into a specified buffer.
*
* @param[out] BufferPtr The address of the data buffer to be filled.
* @param[in] ByteCount The number of bytes in the buffer to be read.
*
* @return
*        - XST_SUCCESS if successful.
*        - XST_FAILURE otherwise.
*
******************************************************************************/
int DynEepromReadData(u8 *BufferPtr, u16 ByteCount)
{
	int Status;
	AddressType Address = EEPROM_TEST_START_ADDRESS;
	/*
	 * Set the Defaults.
	 */
	ReceiveComplete = 1;

	/*
	 * Position the Pointer in EEPROM.
	 */
	Status = DynEepromWriteData(sizeof(Address));
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the IIC device.
	 */
	Status = XIic_Start(&IicInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Receive the Data.
	 */
	Status = XIic_DynMasterRecv(&IicInstance, BufferPtr, ByteCount);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	* Wait for receive completion.
	*
	* The ReceiveComplete flag is set to 0 by the receive interrupt handler
	* once all requested bytes are successfully received. Xil_WaitForEvent()
	* polls this software flag until it matches the expected completion value
	* or the timeout expires, preventing an infinite wait in case the interrupt
	* is not serviced.
	*/
	Status = Xil_WaitForEvent((UINTPTR)&ReceiveComplete, EventMask, Event_Value, MAX_DELAY_CNT);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
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
/*****************************************************************************/
/**
* @brief Sets up the interrupt system for the IIC device.
*
* Sets up the interrupt system so interrupts can occur for the IIC device. The
* function is application-specific since the actual system may or may not have
* an interrupt controller. The IIC device could be directly connected to a
* processor without an interrupt controller. Modify this
* function to fit the application.
*
* This function sets up the interrupt system so interrupts can occur for the IIC
* device. The function is application-specific since the actual system may or
* may not have an interrupt controller. The IIC device could be directly
* connected to a processor without an interrupt controller. Modify
* this function to fit the application.
*
* @param[in] IicInstPtr Pointer to the instance of the IIC device which is going
*                       to be connected to the interrupt controller.
*
* @return
*        - XST_SUCCESS if successful.
*        - XST_FAILURE otherwise.
*
******************************************************************************/
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
* @brief Handles send completion from interrupt context.
*
* This Send handler is called asynchronously from an interrupt context and
* indicates that data in the specified buffer has been sent.
*
* @param[in] CallBackRef Pointer to the IIC device instance.
* @param[in] ByteCount The number of bytes sent.
*
******************************************************************************/
static void SendHandler(void *CallBackRef, int ByteCount)
{
    (void)ByteCount;
    (void)CallBackRef;

    TransmitComplete = 0;
}
/*****************************************************************************/
/**
* @brief Handles receive completion from interrupt context.
*
* This Receive handler is called asynchronously from an interrupt context and
* indicates that data in the specified buffer has been received.
*
* @param[in] CallBackRef Pointer to the IIC device instance.
* @param[in] ByteCount The number of bytes received.
*
******************************************************************************/
static void ReceiveHandler(void *CallBackRef, int ByteCount)
{
    (void)CallBackRef;
    (void)ByteCount;

    ReceiveComplete = 0;
}

/*****************************************************************************/
/**
* @brief Handles IIC status events from interrupt context.
*
* This Status handler is called asynchronously from an interrupt context and
* indicates the events that have occurred.
*
* @param[in] InstancePtr Pointer to the IIC driver instance for which the
*                        handler is being called for.
* @param[in] Event Indicates the condition that has occurred.
*
******************************************************************************/
static void StatusHandler(XIic *InstancePtr, int Event)
{

      (void)InstancePtr;
      (void)Event;
}
