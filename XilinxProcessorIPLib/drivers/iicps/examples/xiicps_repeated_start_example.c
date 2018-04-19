/******************************************************************************
*
* Copyright (C) 2014-2019 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
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
* This example can be used directly to read up to 16 pages
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
* 3.7   ask  04/17/18 Updated the Eeprom scanning mechanism
*		      as per the other examples (CR#997545)
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
#define MUX_ADDR 0x74
#define MAX_CHANNELS 0x04
#define SLV_MON_LOOP_COUNT 0x00FFFFF

/*
 * The page size determines how much data should be written at a time.
 * The write function should be called with this as a maximum byte count.
 */
#define MAX_SIZE	32
#define PAGE_SIZE_16	16
#define PAGE_SIZE_32	32
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
static s32 MuxInitChannel(u16 MuxIicAddr, u8 WriteBuffer);
int EepromReadDataRepStart(u8 *BufferPtr, u16 ByteCount);
static s32 FindEepromDevice(u16 Address);
static s32 IicPsFindEeprom(u16 *Eeprom_Addr, u32 *PageSize);
static s32 IicPsConfig(u16 DeviceId);
static s32 IicPsFindDevice(u16 addr);
static s32 IicPsSlaveMonitor(u16 Address, u16 DeviceId);

/************************** Variable Definitions *****************************/

XIicPs IicInstance;		/* The instance of the IIC device. */
u32 Platform;
u32 PageSize;
u16 EepromSlvAddr;
u16 EepromAddr[] = {0x54,0x55,0};
u16 MuxAddr[] = {0x74,0};

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
	*	Find the Eeprom .
	*/

	AddressTemp = Address;
	Status = IicPsFindEeprom(&EepromSlvAddr,&PageSize);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/*
	 * Initialize the data to write and the read buffer.
	 */
	if (PageSize == PAGE_SIZE_16) {
		WriteBuffer[0] = (u8) (Address);
		WrBfrOffset = 1;
	} else {
		WriteBuffer[0] = (u8) (Address >> 8);
		WriteBuffer[1] = (u8) (Address);
		WrBfrOffset = 2;
	}

	for (Index = 0; Index < PageSize; Index++) {
		WriteBuffer[WrBfrOffset + Index] = 0xFF;
		ReadBuffer[Index] = 0;
	}

		xil_printf("EepromWriteData\r\n");
		Status = EepromWriteData(WrBfrOffset + PAGE_SIZE);
		if (Status != XST_SUCCESS) {
			xil_printf("Failed EepromWriteData\r\n");
			return XST_FAILURE;
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
	for (Index = 0; Index < PageSize; Index++) {
		if (ReadBuffer[Index] != WriteBuffer[Index + WrBfrOffset]) {
			xil_printf("Verify the data read against the buffer Failed\r\n");
			return XST_FAILURE;
		}
	}

	AddressTemp = Address;
	if (PageSize == PAGE_SIZE_16) {
		WriteBuffer[0] = (u8) (Address);
		WrBfrOffset = 1;
	} else {
		WriteBuffer[0] = (u8) (Address >> 8);
		WriteBuffer[1] = (u8) (Address);
		WrBfrOffset = 2;
	}

	for (Index = 0; Index < PageSize; Index++) {
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
	for (Index = 0; Index < PageSize; Index++) {
		if (ReadBuffer[Index] != WriteBuffer[Index + WrBfrOffset]) {
			xil_printf("Verify the data read against the buffer Failed\r\n");
			return XST_FAILURE;
		}
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
	if (PageSize == PAGE_SIZE_16) {
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
	 * Disable repeated start option.
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
* This function initializes the IIC MUX to select the required channel.
*
* @param	MuxAddress and Channel select value.
*
* @return	XST_SUCCESS if pass, otherwise XST_FAILURE.
*
* @note		None.
*
****************************************************************************/
static s32 MuxInitChannel(u16 MuxIicAddr, u8 WriteBuffer)
{
	u8 Buffer = 0;
	s32 Status = 0;


	/*
	 * Wait until bus is idle to start another transfer.
	 */
	while (XIicPs_BusIsBusy(&IicInstance));

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
/*****************************************************************************/
/**
* This function is use to figure out the Eeprom slave device
*
* @param	addr: u16 variable
*
* @return	XST_SUCCESS if successful and also update the epprom slave
* device address in addr variable else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
static s32 IicPsFindEeprom(u16 *Eeprom_Addr,u32 *PageSize)
{
	s32 Status;
	u32 MuxIndex,Index;
	u8 MuxChannel;

	for(MuxIndex=0;MuxAddr[MuxIndex] != 0;MuxIndex++){
		Status = IicPsFindDevice(MuxAddr[MuxIndex]);
		if (Status == XST_SUCCESS) {
			for(Index=0;EepromAddr[Index] != 0;Index++) {
				for(MuxChannel = MAX_CHANNELS; MuxChannel > 0x0; MuxChannel = MuxChannel >> 1) {
					Status = MuxInitChannel(MuxAddr[MuxIndex], MuxChannel);
					if (Status != XST_SUCCESS) {
						xil_printf("Failed to enable the MUX channel\r\n");
						return XST_FAILURE;
					}
					Status = FindEepromDevice(EepromAddr[Index]);
					if (Status == XST_SUCCESS) {
						*Eeprom_Addr = EepromAddr[Index];
						*PageSize = PAGE_SIZE_16;
						return XST_SUCCESS;
					}
				}
			}
		}
	}
	for(Index=0;EepromAddr[Index] != 0;Index++) {
		Status = IicPsFindDevice(EepromAddr[Index]);
		if (Status == XST_SUCCESS) {
			*Eeprom_Addr = EepromAddr[Index];
			*PageSize = PAGE_SIZE_32;
			return XST_SUCCESS;
		}
	}
	return XST_FAILURE;
}
/*****************************************************************************/
/**
*
* This function checks the availability of EEPROM using slave monitor mode.
*
* @param	EEPROM address.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note 	None.
*
*******************************************************************************/
static s32 FindEepromDevice(u16 Address)
{
	u32 Index,IntrStatusReg;
	XIicPs *IicPtr = &IicInstance;


	XIicPs_EnableSlaveMonitor(&IicInstance, Address);

	Index = 0;
	/*
	 * Wait for the Slave Monitor status
	 */
	while (Index < SLV_MON_LOOP_COUNT) {
		Index++;
		/*
		 * Read the Interrupt status register.
		 */
		IntrStatusReg = XIicPs_ReadReg(IicPtr->Config.BaseAddress,
						 (u32)XIICPS_ISR_OFFSET);
		if (0U != (IntrStatusReg & XIICPS_IXR_SLV_RDY_MASK)) {
			XIicPs_DisableSlaveMonitor(&IicInstance);
			XIicPs_WriteReg(IicPtr->Config.BaseAddress,
					(u32)XIICPS_ISR_OFFSET, IntrStatusReg);
			return XST_SUCCESS;
		}
	}
	XIicPs_DisableSlaveMonitor(&IicInstance);

	return XST_FAILURE;
}
/*****************************************************************************/
/**
*
* This function is use to figure out the slave device is alive or not.
*
* @param        slave address and Device ID .
*
* @return       XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note         None.
*
*******************************************************************************/
static s32 IicPsFindDevice(u16 addr)
{
	s32 Status;

	Status = IicPsSlaveMonitor(addr,0);
	if (Status == XST_SUCCESS) {
		return XST_SUCCESS;
	}
	Status = IicPsSlaveMonitor(addr,1);
	if (Status == XST_SUCCESS) {
		return XST_SUCCESS;
	}

	return XST_FAILURE;
}
/*****************************************************************************/
/**
*
* This function checks the availability of a slave using slave monitor mode.
*
* @param	DeviceId is the Device ID of the IicPs Device and is the
*		XPAR_<IICPS_instance>_DEVICE_ID value from xparameters.h
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note 	None.
*
*******************************************************************************/
static s32 IicPsSlaveMonitor(u16 Address, u16 DeviceId)
{
	u32 Index,IntrStatusReg;
	s32 Status;
	XIicPs *IicPtr;

	/*
	 * Initialize the IIC driver so that it is ready to use.
	 */
	Status = IicPsConfig(DeviceId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	IicPtr = &IicInstance;
	XIicPs_EnableSlaveMonitor(&IicInstance, Address);

	Index = 0;
	/*
	 * Wait for the Slave Monitor Status
	 */
	while (Index < SLV_MON_LOOP_COUNT) {
		Index++;
		/*
		 * Read the Interrupt status register.
		 */
		IntrStatusReg = XIicPs_ReadReg(IicPtr->Config.BaseAddress,
						 (u32)XIICPS_ISR_OFFSET);
		if (0U != (IntrStatusReg & XIICPS_IXR_SLV_RDY_MASK)) {
			XIicPs_DisableSlaveMonitor(&IicInstance);
			XIicPs_WriteReg(IicPtr->Config.BaseAddress,
					(u32)XIICPS_ISR_OFFSET, IntrStatusReg);
			return XST_SUCCESS;
		}
	}
	XIicPs_DisableSlaveMonitor(&IicInstance);
	return XST_FAILURE;
}
/*****************************************************************************/
/**
* This function perform the initial configuration for the IICPS Device.
*
* @param	DeviceId instance.
*
* @return	XST_SUCCESS if pass, otherwise XST_FAILURE.
*
* @note		None.
*
****************************************************************************/
static s32 IicPsConfig(u16 DeviceId)
{
	s32 Status;
	XIicPs_Config *ConfigPtr;	/* Pointer to configuration data */

	/*
	 * Initialize the IIC driver so that it is ready to use.
	 */
	ConfigPtr = XIicPs_LookupConfig(DeviceId);
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
	return XST_SUCCESS;
}
