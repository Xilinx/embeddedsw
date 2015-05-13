/******************************************************************************
*
* Copyright (C) 2014 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
* @file xiicps_repeated_start_example.c
*
* This file consists of a repeated start example using xiicps driver
* in polled mode. The slave used is an EEPROM.
*
* The example is tested with a 2Kb/8Kb serial IIC EEPROM (ST M24C02/M24C08).
* The WP pin of this EEPROM is hardwired to ground in the HW in which this
* was tested.
* This example can be used directly to read upto 16 pages
* from start address in this EEPROM (Since single address byte).
*
* The AddressType should be u8 as the address pointer in the on-board
* EEPROM is 1 bytes.
*
* This code assumes that no Operating System is being used.
*
* @note
*
* The I2C controller does not indicate completion of a receive transfer if HOLD
* bit is set. Due to this errata, repeated start cannot be used if a receive
* transfer is followed by any other transfer on Zynq platform.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 2.1   hk   03/15/10 First release
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "sleep.h"
#include "xiicps.h"
#include "xil_printf.h"
#include "xplatform_info.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define IIC_DEVICE_ID	XPAR_XIICPS_0_DEVICE_ID

/*
 * The following constant defines the address of the IIC Slave device on the
 * IIC bus. Note that since the address is only 7 bits, this constant is the
 * address divided by 2.
 */
#define IIC_SLAVE_ADDR		0x54
#define IIC_SCLK_RATE		100000
#define IIC_MUX_ADDRESS 	0x74

/*
 * The page size determines how much data should be written at a time.
 * The write function should be called with this as a maximum byte count.
 */
#define PAGE_SIZE		16

/*
 * The Starting address in the IIC EEPROM on which this test is performed.
 */
#define EEPROM_START_ADDRESS	0

/**************************** Type Definitions *******************************/

/*
 * The AddressType should be u8 as the address pointer in the on-board
 * EEPROM is 1 bytes.
 */
typedef u16 AddressType;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int IicPsRepeatedStartExample(void);
int EepromWriteData(u16 ByteCount);
int MuxInit(void);
int EepromReadDataRepStart(u8 *BufferPtr, u16 ByteCount);

/************************** Variable Definitions *****************************/

XIicPs IicInstance;		/* The instance of the IIC device. */
u32 Platform;

/*
 * Write buffer for writing a page.
 */
u8 WriteBuffer[sizeof(AddressType) + PAGE_SIZE];

u8 ReadBuffer[PAGE_SIZE*20];	/* Read buffer for reading a page. */

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* Main function to call the Iic repeated start example.
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

	xil_printf("IIC Repeated Start Example Test \r\n");

	/*
	 * Run the Iic repeated start example.
	 * Refer to note in the header - repeated start cannot be used
	 * on zynq platform if read transfer is followed by any other transfer.
	 */
	Status = IicPsRepeatedStartExample();
	if (Status != XST_SUCCESS) {
		xil_printf("IIC Repeated Start Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran IIC Repeated Start Example Test\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function writes, reads, and verifies the data to the IIC EEPROM.
* Page write is used. Buffered read with repeated start option is done.
*
* @param	None.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int IicPsRepeatedStartExample(void)
{
	u32 Index;
	int Status;
	XIicPs_Config *ConfigPtr;	/* Pointer to configuration data */
	AddressType Address = EEPROM_START_ADDRESS;
	AddressType AddressTemp;
	int PageCnt;
	int NumPages = 16;
	int WrBfrOffset;

	/*
	 * Initialize the IIC driver so that it is ready to use.
	 */
	ConfigPtr = XIicPs_LookupConfig(IIC_DEVICE_ID);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}

	Status = XIicPs_CfgInitialize(&IicInstance, ConfigPtr,
					ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the IIC serial clock rate.
	 */
	XIicPs_SetSClk(&IicInstance, IIC_SCLK_RATE);

	/*
	 * Set the channel value in IIC Mux.
	 */
	Platform = XGetPlatform_Info();
	if(Platform == XPLAT_ZYNQ) {
		Status = MuxInit();
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	AddressTemp = Address;
	for(PageCnt = 0; PageCnt < NumPages; PageCnt++) {
		/*
		 * Initialize the data to write and the read buffer.
		 */
		if (Platform == XPLAT_ZYNQ) {
			WriteBuffer[0] = (u8) (AddressTemp);
			WrBfrOffset = 1;
		} else {
			WriteBuffer[0] = (u8) (AddressTemp >> 8);
			WriteBuffer[1] = (u8) (AddressTemp);
			WrBfrOffset = 2;
		}

		for (Index = 0; Index < PAGE_SIZE; Index++) {
			WriteBuffer[WrBfrOffset + Index] = 0xFF;
			ReadBuffer[Index] = 0;
		}

		/*
		 * Write to the EEPROM.
		 */
		Status = EepromWriteData(WrBfrOffset + PAGE_SIZE);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		AddressTemp += PAGE_SIZE;
	}

	/*
	 * Read from the EEPROM.
	 */
	Status = EepromReadDataRepStart(ReadBuffer, PAGE_SIZE*NumPages);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Verify the data read against the data written.
	 */
	for (Index = 0; Index < PAGE_SIZE*NumPages; Index++) {
		if (ReadBuffer[Index] !=
			WriteBuffer[Index%PAGE_SIZE + WrBfrOffset]) {
			return XST_FAILURE;
		}

		ReadBuffer[Index] = 0;
	}

	AddressTemp = Address;
	for(PageCnt = 0; PageCnt < NumPages; PageCnt++) {
		/*
		 * Initialize the data to write and the read buffer.
		 */
		if (Platform == XPLAT_ZYNQ) {
			WriteBuffer[0] = (u8) (AddressTemp);
			WrBfrOffset = 1;
		} else {
			WriteBuffer[0] = (u8) (AddressTemp >> 8);
			WriteBuffer[1] = (u8) (AddressTemp);
			WrBfrOffset = 2;
		}

		for (Index = 0; Index < PAGE_SIZE; Index++) {
			WriteBuffer[WrBfrOffset + Index] = Index + 10;
			ReadBuffer[Index] = 0;
		}

		/*
		 * Write to the EEPROM.
		 */
		Status = EepromWriteData(WrBfrOffset + PAGE_SIZE);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		AddressTemp += PAGE_SIZE;
	}

	/*
	 * Read from the EEPROM.
	 */
	Status = EepromReadDataRepStart(ReadBuffer, PAGE_SIZE*NumPages);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Verify the data read against the data written.
	 */
	for (Index = 0; Index < PAGE_SIZE*NumPages; Index++) {
		if (ReadBuffer[Index] !=
			WriteBuffer[Index%PAGE_SIZE + WrBfrOffset]) {
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
	 * Send the Data.
	 */
	Status = XIicPs_MasterSendPolled(&IicInstance, WriteBuffer,
					  ByteCount, IIC_SLAVE_ADDR);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	if (!(IicInstance.IsRepeatedStart)) {
		while (XIicPs_BusIsBusy(&IicInstance));
	}

	/*
	 * Wait for a bit of time to allow the programming to complete
	 */
	usleep(250000);

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
int EepromReadDataRepStart(u8 *BufferPtr, u16 ByteCount)
{
	int Status;
	AddressType Address = EEPROM_START_ADDRESS;
	int WrBfrOffset;

	/*
	 * Enable repeated start option.
	 * This call will give an indication to the driver.
	 * The hold bit is actually set before beginning the following transfer
	 */
	XIicPs_SetOptions(&IicInstance, XIICPS_REP_START_OPTION);

	/*
	 * Position the Pointer in EEPROM.
	 */
	if (Platform == XPLAT_ZYNQ) {
		WriteBuffer[0] = (u8) (Address);
		WrBfrOffset = 1;
	} else {
		WriteBuffer[0] = (u8) (Address >> 8);
		WriteBuffer[1] = (u8) (Address);
		WrBfrOffset = 2;
	}

	Status = EepromWriteData(WrBfrOffset);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Disbale repeated start option.
	 * This call will give an indication to the driver.
	 * The hold bit is actually reset when the following transfer ends.
	 */
	XIicPs_ClearOptions(&IicInstance, XIICPS_REP_START_OPTION);

	/*
	 * Receive the Data.
	 */
	Status = XIicPs_MasterRecvPolled(&IicInstance, BufferPtr,
					  ByteCount, IIC_SLAVE_ADDR);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	while (XIicPs_BusIsBusy(&IicInstance));

	return XST_SUCCESS;
}

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
	u8 WriteBuffer;
	u8 MuxIicAddr = IIC_MUX_ADDRESS;
	u8 Buffer = 0;
	int Status = 0;

	/*
	 * Channel select value for EEPROM.
	 */
	WriteBuffer = 0x04;

	/*
	 * Send the Data.
	 */
	Status = XIicPs_MasterSendPolled(&IicInstance, &WriteBuffer,1,
					MuxIicAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	while (XIicPs_BusIsBusy(&IicInstance));

	/*
	 * Receive the Data.
	 */
	Status = XIicPs_MasterRecvPolled(&IicInstance, &Buffer,1, MuxIicAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	while (XIicPs_BusIsBusy(&IicInstance));

	return XST_SUCCESS;
}
