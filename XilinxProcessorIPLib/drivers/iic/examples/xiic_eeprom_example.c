/******************************************************************************
* Copyright (C) 2006 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xiic_eeprom_example.c
*
* This file consists of a Interrupt mode design example which uses the Xilinx
* IIC device and XIic driver to exercise the EEPROM. The XIic driver uses the
* complete FIFO functionality to transmit/receive data.
*
* This example writes/reads from the lower 256 bytes of the IIC EEPROMS. Please
* refer to the datasheets of the IIC EEPROM's for details about the internal
* addressing and page size of these devices.
*
* The XIic_MasterSend() API is used to transmit the data and
* XIic_MasterRecv() API is used to receive the data.
*
* This example is tested on ML300/ML310/ML410/ML403/ML501/ML507/ML510/ML605/
* SP601, SP605, KC705 , ZC702 and ZC706  Xilinx boards.
*
* The ML310/ML510/ML410 boards have a on-board 64 Kb serial IIC EEPROM
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
* The ML501/ML505/ML507/ML605/SP601/SP605/KC705/ZC702/ZC706 boards have an
* on-board 8 Kb serial IIC EEPROM(STM M24C08). The WP pin of the IIC EEPROM is
* hardwired to ground on these boards.
*
* The AddressType for ML300/ML310/ML410/ML510 boards should be u16 as the
* address pointer in the on board EEPROM is 2 bytes.
*
* The AddressType for ML403/ML501/ML505/ML507/ML605/SP601/SP605/KC705/ZC702/
* ZC706 boards should be u8 as the address pointer for the on board EEPROM
* is 1 byte.
*
* The 7 bit IIC Slave address of the IIC EEPROM on the ML300/ML310/ML403/ML410/
* ML501/ML505/ML507/ML510 boards is 0x50.
* The 7 bit IIC Slave address of the IIC EEPROM on the ML605/SP601/SP605/KC705
* /ZC702/ZC706 boards is 0x54.
* Refer to the User Guide's of the respective boards for further information
* about the IIC slave address of IIC EEPROM's.
*
* The define EEPROM_ADDRESS in this file needs to be changed depending on
* the board on which this example is to be run. This is the IIC address of the
* EEPROM.
*
* The define IIC_MUX_ADDRESS in this file needs to be changed depending on
* the board on which this example is to be run. This is the IIC address of the
* the MUX.
*
* The define IIC_MUX_ENABLE should be defined so that the IIC Mux initialization
* is done for the boards on the which the EEPROM is connected to an IIC Mux.
* The boards with a MUX are a KC705/ZC702/ZC706.
*
* The define IIC_EEPROM_CHANNEL needs to be changed depending on the Channel
* number of EEPROM for IIC Mux. On KC705 it is 0x08 and ZC702 is 0x04. Please
* refer the User Guide's of the respective boards for further information
* about the Channel number to use EEPROM.
*
* This code assumes that no Operating System is being used.
*
*
* @note
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00a mta  02/24/06 Created.
* 1.00a mta  04/05/07 Added support for microblaze.
* 2.00a ktn  11/17/09 Updated to use the HAL APIs and replaced call to
*		      XIic_Initialize API with XIic_LookupConfig and
*		      XIic_CfgInitialize.
* 2.01a ktn  03/17/10 Updated the information about the EEPROM's used on
*		      ML605/SP601/SP605 boards. Updated the example so that it
*		      can be used to access the entire IIC EEPROM for devices
*		      like M24C04/M24C08 that use LSB bits of the IIC device
*		      select code (IIC slave address) to specify the higher
*		      address bits of the EEPROM internal address.
* 2.02a bss  01/30/13 Updated for using the GIC in case of Zynq
* 2.06a bss  02/14/13 Added MuxInit API to support Zynq and KC705 boards and
*		      modified to use ScuGic in case of Zynq CR# 683509
* 3.4   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xiic.h"
#include "xil_exception.h"
#include "xil_printf.h"

#ifdef XPAR_INTC_0_DEVICE_ID
 #include "xintc.h"
#else
 #include "xscugic.h"
#endif

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define IIC_DEVICE_ID	XPAR_IIC_0_DEVICE_ID



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


/*
 * The following constant defines the address of the IIC Slave device on the
 * IIC bus. Note that since the address is only 7 bits, this constant is the
 * address divided by 2.
 * The 7 bit IIC Slave address of the IIC EEPROM on the ML300/ML310/ML403/ML410/
 * ML501/ML505/ML507/ML510 boards is 0x50. The 7 bit IIC Slave address of the
 * IIC EEPROM on the ML605/SP601/SP605/KC705/ZC702/ZC706 boards is 0x54.
 * Please refer the User Guide's of the respective boards for further
 * information about the IIC slave address of IIC EEPROM's.
 */
#define EEPROM_ADDRESS 		0x54	/* 0xA0 as an 8 bit number. */


/*
 * The IIC_MUX_ADDRESS defines the address of the IIC MUX device on the
 * IIC bus. Note that since the address is only 7 bits, this constant is the
 * address divided by 2.
 * The IIC Slaves on the KC705/ZC702/ZC706 boards are connected to an
 * IIC MUX.
 * IIC_EEPROM_CHANNEL is the Channel number of EEPROM for IIC Mux. On KC705 it
 * is 0x08 and ZC702 is 0x04.Please refer the User Guide's of the respective
 * boards for further information about the Channel number to use EEPROM.
 */
#define IIC_MUX_ADDRESS 		0x74
#define IIC_EEPROM_CHANNEL		0x08

/*
 * This define should be uncommented if there is IIC MUX on the board to which
 * this EEPROM is connected. The boards that have IIC MUX are KC705/ZC702/ZC706.
 */
/* #define IIC_MUX_ENABLE */

/*
 * The page size determines how much data should be written at a time.
 * The ML310/ML300 board supports a page size of 32 and 16.
 * The write function should be called with this as a maximum byte count.
 */
#define PAGE_SIZE   16

/*
 * The Starting address in the IIC EEPROM on which this test is performed.
 */
#define EEPROM_TEST_START_ADDRESS   128

/**************************** Type Definitions *******************************/

/*
 * The AddressType for ML300/ML310/ML410/ML510 boards should be u16 as the address
 * pointer in the on board EEPROM is 2 bytes.
 * The AddressType for ML403/ML501/ML505/ML507/ML605/SP601/SP605/KC705/ZC702
 * /ZC706 boards should be u8 as the address pointer in the on board EEPROM is
 * 1 bytes.
 */
typedef u8 AddressType;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int IicEepromExample();

int EepromWriteData(u16 ByteCount);

int EepromReadData(u8 *BufferPtr, u16 ByteCount);

static int SetupInterruptSystem(XIic *IicInstPtr);

static void SendHandler(XIic *InstancePtr);

static void ReceiveHandler(XIic *InstancePtr);

static void StatusHandler(XIic *InstancePtr, int Event);

#ifdef IIC_MUX_ENABLE
static int MuxInit(void);
#endif
/************************** Variable Definitions *****************************/

XIic IicInstance;	/* The instance of the IIC device. */
INTC Intc; 	/* The instance of the Interrupt Controller Driver */

/*
 * Write buffer for writing a page.
 */
u8 WriteBuffer[sizeof(AddressType) + PAGE_SIZE];

u8 ReadBuffer[PAGE_SIZE];	/* Read buffer for reading a page. */

volatile u8 TransmitComplete;	/* Flag to check completion of Transmission */
volatile u8 ReceiveComplete;	/* Flag to check completion of Reception */

u8 EepromIicAddr;		/* Variable for storing Eeprom IIC address */

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* Main function to call the High level EEPROM example.
*
* @param	None.
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
	 * Run the EEPROM example.
	 */
	Status = IicEepromExample();
	if (Status != XST_SUCCESS) {
		xil_printf("IIC eeprom Example Failed\r\n");
		return XST_FAILURE;
	}
	xil_printf("Successfully ran IIC eeprom Example\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function writes, reads, and verifies the data to the IIC EEPROM. It
* does the write as a single page write, performs a buffered read.
*
* @param	None.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int IicEepromExample()
{
	u32 Index;
	int Status;
	XIic_Config *ConfigPtr;	/* Pointer to configuration data */
	AddressType Address = EEPROM_TEST_START_ADDRESS;
	EepromIicAddr = EEPROM_ADDRESS;

	/*
	 * Initialize the IIC driver so that it is ready to use.
	 */
	ConfigPtr = XIic_LookupConfig(IIC_DEVICE_ID);
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
	Status = SetupInterruptSystem(&IicInstance);
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


#ifdef IIC_MUX_ENABLE
	/*
	 * Initialize the IIC MUX on the boards on which the EEPROM
	 * are connected through the MUX.
	 */
	Status =  MuxInit();
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#endif

	/*
	 * Initialize the data to write and the read buffer.
	 */
	if (sizeof(Address) == 1) {
		WriteBuffer[0] = (u8) (EEPROM_TEST_START_ADDRESS);
		EepromIicAddr |= (EEPROM_TEST_START_ADDRESS >> 8) & 0x7;
	} else {
		WriteBuffer[0] = (u8) (EEPROM_TEST_START_ADDRESS >> 8);
		WriteBuffer[1] = (u8) (EEPROM_TEST_START_ADDRESS);
		ReadBuffer[Index] = 0;
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
	Status = EepromWriteData(sizeof(Address) + PAGE_SIZE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Read from the EEPROM.
	 */
	Status = EepromReadData(ReadBuffer, PAGE_SIZE);
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
		ReadBuffer[Index] = 0;
	}

	for (Index = 0; Index < PAGE_SIZE; Index++) {
		WriteBuffer[sizeof(Address) + Index] = Index;
		ReadBuffer[Index] = 0;
	}

	/*
	 * Write to the EEPROM.
	 */
	Status = EepromWriteData(sizeof(Address) + PAGE_SIZE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Read from the EEPROM.
	 */
	Status = EepromReadData(ReadBuffer, PAGE_SIZE);
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
* This function writes a buffer of data to the IIC serial EEPROM.
*
* @param	ByteCount contains the number of bytes in the buffer to be
*		written.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		The Byte count should not exceed the page size of the EEPROM as
*		noted by the constant PAGE_SIZE.
*
******************************************************************************/
int EepromWriteData(u16 ByteCount)
{
	int Status;

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
	Status = XIic_MasterSend(&IicInstance, WriteBuffer, ByteCount);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait till the transmission is completed.
	 */
	while ((TransmitComplete) || (XIic_IsIicBusy(&IicInstance) == TRUE)) {
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
				}
				else {
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
* This function reads data from the IIC serial EEPROM into a specified buffer.
*
* @param	BufferPtr contains the address of the data buffer to be filled.
* @param	ByteCount contains the number of bytes in the buffer to be read.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int EepromReadData(u8 *BufferPtr, u16 ByteCount)
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
	if (sizeof(Address) == 1) {
		WriteBuffer[0] = (u8) (EEPROM_TEST_START_ADDRESS);
	}
	else {
		WriteBuffer[0] = (u8) (EEPROM_TEST_START_ADDRESS >> 8);
		WriteBuffer[1] = (u8) (EEPROM_TEST_START_ADDRESS);
	}

	Status = EepromWriteData(sizeof(Address));
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
	Status = XIic_MasterRecv(&IicInstance, BufferPtr, ByteCount);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait till all the data is received.
	 */
	while ((ReceiveComplete) || (XIic_IsIicBusy(&IicInstance) == TRUE)) {

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
* This function setups the interrupt system so interrupts can occur for the
* IIC device. The function is application-specific since the actual system may
* or may not have an interrupt controller. The IIC device could be directly
* connected to a processor without an interrupt controller. The user should
* modify this function to fit the application.
*
* @param	IicInstPtr contains a pointer to the instance of the IIC device
*		which is going to be connected to the interrupt controller.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
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
	 * Initialize the exception table and register the interrupt
	 * controller handler with the exception table
	 */
	Xil_ExceptionInit();

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			 (Xil_ExceptionHandler)INTC_HANDLER, &Intc);

	/* Enable non-critical exceptions */
	Xil_ExceptionEnable();



	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This Send handler is called asynchronously from an interrupt
* context and indicates that data in the specified buffer has been sent.
*
* @param	InstancePtr is not used, but contains a pointer to the IIC
*		device driver instance which the handler is being called for.
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
* This Receive handler is called asynchronously from an interrupt
* context and indicates that data in the specified buffer has been Received.
*
* @param	InstancePtr is not used, but contains a pointer to the IIC
*		device driver instance which the handler is being called for.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void ReceiveHandler(XIic *InstancePtr)
{
	ReceiveComplete = 0;
}

/*****************************************************************************/
/**
* This Status handler is called asynchronously from an interrupt
* context and indicates the events that have occurred.
*
* @param	InstancePtr is a pointer to the IIC driver instance for which
*		the handler is being called for.
* @param	Event indicates the condition that has occurred.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void StatusHandler(XIic *InstancePtr, int Event)
{

}

#ifdef IIC_MUX_ENABLE
/*****************************************************************************/
/**
* This function initializes the IIC MUX to select EEPROM.
*
* @param	None.
*
* @return	XST_SUCCESS if pass, otherwise XST_FAILURE.
*
* @note		None.
*
****************************************************************************/
int MuxInit(void)
{

	int Status;
	/*
	 * Set the Slave address to the IIC MUC - PCA9543A.
	 */
	Status = XIic_SetAddress(&IicInstance, XII_ADDR_TO_SEND_TYPE,
				 IIC_MUX_ADDRESS);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Enabling all the channels
	 */
	WriteBuffer[0] = IIC_EEPROM_CHANNEL;

	Status = EepromWriteData(1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
#endif
