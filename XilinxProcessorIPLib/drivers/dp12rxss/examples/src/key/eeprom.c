/*
 * eeprom.c
 *
 *  Created on: May 5, 2015
 *      Author: yashova
 */
/******************************************************************************
*
* Copyright (C) 2006 - 2014 Xilinx, Inc.  All rights reserved.
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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/************************** Variable Definitions *****************************/

#include "eeprom.h"
#include "keymgmt.h"
#include "xparameters.h"
#include "keygen_config.h"
#include "xintc.h"

XIic IicInstance;	/* The instance of the IIC device. */


/*
 * Write buffer for writing a page.
 */
u8 WriteBuffer[sizeof(AddressType) + PAGE_SIZE];
u8 MasterWriteBuffer[1024];
u32 MasterWriteBufIndex;
u8 ReadBuffer[PAGE_SIZE];	/* Read buffer for reading a page. */
u8 MasterReadBuffer[1024];
u32 MasterReadBufIndex;

u16 EepromIicAddr;		/* Variable for storing Eeprom IIC address */


#define XINTC                       XIntc

extern XINTC Intc;
#define INTRCNTRL 	Intc



/***************** Macros (Inline Functions) Definitions *********************/
volatile u8 TransmitComplete;	/* Flag to check completion of Transmission */
volatile u8 ReceiveComplete;	/* Flag to check completion of Reception */

/************************** Function Prototypes ******************************/

#ifdef IIC_MUX_ENABLE
static int MuxInit(void);
#endif

#define DEBUG_EEPROM_READ_SETADDR 	1

static void SendHandler(XIic *InstancePtr);
static void ReceiveHandler(XIic *InstancePtr);
static void StatusHandler(XIic *InstancePtr, int Event);

/************************** Function Definitions *****************************/

XStatus Init_IIC()
{

	XStatus Status;
	XIic_Config *ConfigPtr_IIC;	/* Pointer to configuration data */

	/*
	 * Initialize the IIC driver so that it is ready to use.
	 */
#if 0
	ConfigPtr_IIC = XIic_LookupConfig(EEPROM_IIC_1_DEVICE_ID);
	if (ConfigPtr_IIC == NULL) {
		return XST_FAILURE;
	}

	Status = XIic_CfgInitialize(&IicInstance, ConfigPtr_IIC,
									ConfigPtr_IIC->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf("IIC Initialized\r\n");
#endif
	/* Set handlers for the I2C read, write and status events */
	Status = IicSetHandler();
	if(Status != XST_SUCCESS){
		xil_printf("Handler Setup for IIC failed \r\n");
	}


	return XST_SUCCESS;
}

/*
 * This function brings the mux out of reset, the PCA9548A Low Voltage
 * 		8-Channel I2C Switch has a active-low reset input
 */
int SetGPOMuxReset(int resetn){
	XIic_WriteReg(IicInstance.BaseAddress , XIIC_GPO_REG_OFFSET, resetn);
	return XST_SUCCESS;
}


/*
 * This funciton sets the handlers for Send, Recieve and Event for the IIC
 */
int IicSetHandler(){

	int Status;

	//gpo[0] is mapped to MUX_RESET_B (P23) pin on the KC705 board
	int mux_resetn = 0x1;

	/* Set the Handlers for transmit and reception. */
	XIic_SetSendHandler(&IicInstance, &IicInstance,
				(XIic_Handler) SendHandler);
	XIic_SetRecvHandler(&IicInstance, &IicInstance,
				(XIic_Handler) ReceiveHandler);
	XIic_SetStatusHandler(&IicInstance, &IicInstance,
				  (XIic_StatusHandler) StatusHandler);

	/* Bring the MUX out of Reset */
	Status = SetGPOMuxReset(mux_resetn);
	if(Status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}
	/* Initialize the IIC MUX on the boards on which the EEPROM are
	 * 		connected through the MUX. */
	Status =  MuxInit();
	if (Status != XST_SUCCESS)
	{
		return XST_FAILURE;
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
	int Lencntr = 0;
	u8 *TempBufPntr = BufferPtr;
	AddressType Address = EEPROM_TEST_START_ADDRESS;
	EepromIicAddr = 0x0;

	while (Lencntr < ByteCount) {
		/*
		 * Set the Defaults.
		 */
		ReceiveComplete = 1;

		if (sizeof(Address) == 1) {
			u16 TempAddr = (EEPROM_TEST_START_ADDRESS + Lencntr);
			if (TempAddr >= 256) {
				WriteBuffer[0] = (u8)(TempAddr & 0x00ff);
			} else {
				WriteBuffer[0] = TempAddr;
			}
			TempAddr = TempAddr >> 8;
			EepromIicAddr = (EEPROM_ADDR | (TempAddr & 0x7));
		} else {
			WriteBuffer[0] = (u8) ((EEPROM_TEST_START_ADDRESS + Lencntr) >> 8);
			WriteBuffer[1] = (u8) ((EEPROM_TEST_START_ADDRESS + Lencntr));
		}
		/*
		 * Set the Slave address.
		 */
//xil_printf ("EEPROM addr is %x\r\n",EepromIicAddr);
		Status = XIic_SetAddress(&IicInstance, XII_ADDR_TO_SEND_TYPE,
					 EepromIicAddr);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
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
		Status = XIic_MasterRecv(&IicInstance, TempBufPntr, PAGE_SIZE);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

//		xil_printf("Read from %x = %x\r\n",Lencntr,*TempBufPntr);
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
		TempBufPntr = TempBufPntr + PAGE_SIZE;
		Lencntr = Lencntr + PAGE_SIZE;
	}

	return XST_SUCCESS;
}

int EepromReadDataOffset(u8 *BufferPtr, u16 ByteCount, u32 offset){
	int Status;
	int Lencntr = 0;
	u8 *TempBufPntr = BufferPtr;
	AddressType Address = EEPROM_TEST_START_ADDRESS;
	EepromIicAddr = 0x54;
	while (Lencntr < ByteCount) {
		/*
		 * Set the Defaults.
		 */
		ReceiveComplete = 1;

		if (sizeof(Address) == 1) {
			u16 TempAddr = (EEPROM_TEST_START_ADDRESS + offset + Lencntr);
#if DEBUG_EEPROM_READ_SETADDR
			xil_printf("Read from eeprom address = 0x%x (%d)",
						TempAddr,TempAddr);
#endif
			if (TempAddr >= 256) {
				WriteBuffer[0] = (u8)(TempAddr & 0x00ff);
#if DEBUG_EEPROM_READ_SETADDR
			xil_printf(" ... more that 8 bits ");
#endif
			} else {
				WriteBuffer[0] = TempAddr;
			}
			TempAddr = TempAddr >> 8;
#if DEBUG_EEPROM_READ_SETADDR
			xil_printf(" ... MSB 3 bits = 0x%3x ",TempAddr);
#endif
			EepromIicAddr |= (u16)(TempAddr & 0x7);
#if DEBUG_EEPROM_READ_SETADDR
			xil_printf(" ... Slave Set = 0x%x \r\n",EepromIicAddr);
#endif
		} else {
			WriteBuffer[0] = (u8) ((EEPROM_TEST_START_ADDRESS + offset
										+ Lencntr) >> 8);
			WriteBuffer[1] = (u8) ((EEPROM_TEST_START_ADDRESS + offset
										+ Lencntr));
		}
		/*
		 * Set the Slave address.
		 */
		Status = XIic_SetAddress(&IicInstance, XII_ADDR_TO_SEND_TYPE,
					 EepromIicAddr);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
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
		Status = XIic_MasterRecv(&IicInstance, TempBufPntr, PAGE_SIZE);
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
		TempBufPntr = TempBufPntr + PAGE_SIZE;
		Lencntr = Lencntr + PAGE_SIZE;
	}

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
