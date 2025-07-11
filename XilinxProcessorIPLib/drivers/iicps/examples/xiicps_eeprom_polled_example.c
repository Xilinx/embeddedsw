/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xiicps_eeprom_polled_example.c
*
* This file consists of a polled mode design example which uses the Xilinx PS
* IIC device and XIicPs driver to exercise the EEPROM.
* This example runs on zynqmp evaluation board (zcu102).
*
* The XIicPs_MasterSendPolled() API is used to transmit the data and
* XIicPs_MasterRecvPolled() API is used to receive the data.
*
* The example is tested with a 2Kb/8Kb serial IIC EEPROM (ST M24C02/M24C08).
* The WP pin of this EEPROM is hardwired to ground in the HW in which this
* was tested.
*
* The AddressType should be u8 as the address pointer in the on-board
* EEPROM is 1 bytes.
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
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00a sdm  03/15/10 First release
* 1.01a sg   04/13/12 Added MuxInit function for initializing the IIC Mux
*		      on the ZC702 board and to configure it for accessing
*		      the IIC EEPROM.
*                     Updated to use usleep instead of delay loop
* 1.04a hk   09/03/13 Removed GPIO code to pull MUX out of reset - CR#722425.
* 2.3 	sk	 10/07/14 Removed multiple initializations for read buffer.
* 3.3   Nava 08/08/16 Adopt the Dynamic EEPROM finding support.
* 3.11   rna  10/16/19 Added support for 64 page size eeproms on Versal based
*		      boards, scanning for eeprom until found on all I2C
*		      instances
*        rna  03/26/20 Eeprom page size detection support is added.
* 3.18   gm   07/14/23 Added SDT support.
* 3.22   bkv  07/09/25 Fixed GCC Warning.
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
 * The following constant defines the address of the IIC Slave device on the
 * IIC bus. Note that since the address is only 7 bits, this constant is the
 * address divided by 2.
 */

#define IIC_SCLK_RATE		100000
#define SLV_MON_LOOP_COUNT 0x00FFFFFF	/**< Slave Monitor Loop Count*/
#define MUX_ADDR 0x74
#define MAX_CHANNELS 0x04

/*
 * The page size determines how much data should be written at a time.
 * The write function should be called with this as a maximum byte count.
 */
#define MAX_SIZE		64
#define PAGE_SIZE_16	16
#define PAGE_SIZE_32	32
#define PAGE_SIZE_64	64

/*
 * The Starting address in the IIC EEPROM on which this test is performed.
 */
#define EEPROM_START_ADDRESS	0

/**************************** Type Definitions *******************************/

/*
 * The AddressType should be u8 as the address pointer in the on-board
 * EEPROM is 1 byte.
 */
typedef u16 AddressType;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

s32 IicPsEepromPolledExample(void);
static s32 EepromWriteData(XIicPs *IicInstance, u16 ByteCount);
static s32 EepromReadData(XIicPs *IicInstance, u8 *BufferPtr, u16 ByteCount);
#ifndef SDT
static s32 IicPsSlaveMonitor(u16 Address, u16 DeviceId);
static s32 IicPsConfig(u16 DeviceId);
static s32 IicPsFindDevice(u16 addr, u16 DeviceId);
#else
static s32 IicPsSlaveMonitor(u16 Address, UINTPTR BaseAddress);
static s32 IicPsConfig(UINTPTR BaseAddress);
static s32 IicPsFindDevice(u16 addr, UINTPTR BaseAddress);
#endif
static s32 MuxInitChannel(u16 MuxIicAddr, u8 WriteBuffer);
static s32 FindEepromDevice(u16 Address);
static s32 IicPsFindEeprom(u16 *Eeprom_Addr, u32 *PageSize);
static int FindEepromPageSize(u16 EepromAddr, u32 *PageSize_ptr);
/************************** Variable Definitions *****************************/
#ifndef TESTAPP_GEN
XIicPs IicInstance;		/* The instance of the IIC device. */
#endif
u32 Platform;

/*
 * Write buffer for writing a page.
 */
u8 WriteBuffer[sizeof(AddressType) + MAX_SIZE];

u8 ReadBuffer[MAX_SIZE];	/* Read buffer for reading a page. */

/**Searching for the required EEPROM Address and user can also add
 * their own EEPROM Address in the below array list**/
u16 EepromAddr[] = {0x54, 0x55, 0};
u16 MuxAddr[] = {0x74, 0};
u16 EepromSlvAddr;
u32 PageSize;
#ifdef SDT
extern XIicPs_Config XIicPs_ConfigTable[XPAR_XIICPS_NUM_INSTANCES];
#endif

/************************** Function Definitions *****************************/


/*****************************************************************************/
/**
* Main function to call the Iic EEPROM polled example.
*
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	s32 Status;

	xil_printf("IIC EEPROM Polled Mode Example Test \r\n");

	/*
	 * Run the Iic EEPROM Polled Mode example.
	 */
	Status = IicPsEepromPolledExample();
	if (Status != XST_SUCCESS) {
		xil_printf("IIC EEPROM Polled Mode Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran IIC EEPROM Polled Mode Example Test\r\n");
	return XST_SUCCESS;
}
#endif
/*****************************************************************************/
/**
* This function writes, reads, and verifies the data to the IIC EEPROM. It
* does the write as a single page write, performs a buffered read.
*
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
s32 IicPsEepromPolledExample(void)
{
	u32 Index;
	s32 Status;
	AddressType Address = EEPROM_START_ADDRESS;
	u32 WrBfrOffset;


	Status = IicPsFindEeprom(&EepromSlvAddr, &PageSize);
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

	/*
	 * Write to the EEPROM.
	 */
	Status = EepromWriteData(&IicInstance, WrBfrOffset + PageSize);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Read from the EEPROM.
	 */
	Status = EepromReadData(&IicInstance, ReadBuffer, PageSize);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Verify the data read against the data written.
	 */
	for (Index = 0; Index < PageSize; Index++) {
		if (ReadBuffer[Index] != WriteBuffer[Index + WrBfrOffset]) {
			return XST_FAILURE;
		}
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
		WriteBuffer[WrBfrOffset + Index] = Index + 10;
		ReadBuffer[Index] = 0;
	}

	/*
	 * Write to the EEPROM.
	 */
	Status = EepromWriteData(&IicInstance, WrBfrOffset + PageSize);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Read from the EEPROM.
	 */
	Status = EepromReadData(&IicInstance, ReadBuffer, PageSize);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Verify the data read against the data written.
	 */
	for (Index = 0; Index < PageSize; Index++) {
		if (ReadBuffer[Index] != WriteBuffer[Index + WrBfrOffset]) {
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
static s32 EepromWriteData(XIicPs *IicInstance, u16 ByteCount)
{

	s32 Status;

	/*
	 * Send the Data.
	 */

	Status = XIicPs_MasterSendPolled(IicInstance, WriteBuffer,
					 ByteCount, EepromSlvAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	while (XIicPs_BusIsBusy(IicInstance));

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
static s32 EepromReadData(XIicPs *IicInstance, u8 *BufferPtr, u16 ByteCount)
{
	s32 Status;
	AddressType Address = EEPROM_START_ADDRESS;
	u32 WrBfrOffset;

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

	Status = EepromWriteData(IicInstance, WrBfrOffset);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Receive the Data.
	 */

	Status = XIicPs_MasterRecvPolled(IicInstance, BufferPtr,
					 ByteCount, EepromSlvAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/*
	 * Wait until bus is idle to start another transfer.
	 */
	while (XIicPs_BusIsBusy(IicInstance));

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
	Status = XIicPs_MasterSendPolled(&IicInstance, &WriteBuffer, 1,
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
	Status = XIicPs_MasterRecvPolled(&IicInstance, &Buffer, 1, MuxIicAddr);
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
* This function perform the initial configuration for the IICPS Device.
*
* @param	DeviceId instance.
*
* @return	XST_SUCCESS if pass, otherwise XST_FAILURE.
*
* @note		None.
*
****************************************************************************/
#ifndef SDT
static s32 IicPsConfig(u16 DeviceId)
#else
static s32 IicPsConfig(UINTPTR BaseAddress)
#endif
{
	s32 Status;
	XIicPs_Config *ConfigPtr;	/* Pointer to configuration data */

	/*
	 * Initialize the IIC driver so that it is ready to use.
	 */
#ifndef SDT
	ConfigPtr = XIicPs_LookupConfig(DeviceId);
#else
	ConfigPtr = XIicPs_LookupConfig(BaseAddress);
#endif
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

#ifndef SDT
static s32 IicPsFindDevice(u16 addr, u16 DeviceId)
#else
static s32 IicPsFindDevice(u16 addr, UINTPTR BaseAddress)
#endif
{
	s32 Status;

#ifndef SDT
	Status = IicPsSlaveMonitor(addr, DeviceId);
#else
	Status = IicPsSlaveMonitor(addr, BaseAddress);
#endif
	if (Status == XST_SUCCESS) {
		return XST_SUCCESS;
	}
	return XST_FAILURE;
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
static s32 IicPsFindEeprom(u16 *Eeprom_Addr, u32 *PageSize)
{
	s32 Status;
	u32 MuxIndex, Index;
	u8 MuxChannel;
	u16 DeviceId;
#ifdef SDT
	u32 BaseAddress;
#endif

	for (DeviceId = 0; DeviceId < XPAR_XIICPS_NUM_INSTANCES; DeviceId++) {
		for (MuxIndex = 0; MuxAddr[MuxIndex] != 0; MuxIndex++) {
#ifndef SDT
			Status = IicPsFindDevice(MuxAddr[MuxIndex], DeviceId);
#else
			BaseAddress = XIicPs_ConfigTable[DeviceId].BaseAddress;
			Status = IicPsFindDevice(MuxAddr[MuxIndex], BaseAddress);
#endif
			if (Status == XST_SUCCESS) {
				for (Index = 0; EepromAddr[Index] != 0; Index++) {
					for (MuxChannel = MAX_CHANNELS; MuxChannel > 0x0; MuxChannel = MuxChannel >> 1) {
						Status = MuxInitChannel(MuxAddr[MuxIndex], MuxChannel);
						if (Status != XST_SUCCESS) {
							xil_printf("Failed to enable the MUX channel\r\n");
							return XST_FAILURE;
						}
						Status = FindEepromDevice(EepromAddr[Index]);
						if (Status == XST_SUCCESS) {
							*Eeprom_Addr = EepromAddr[Index];
							Status = FindEepromPageSize(EepromAddr[Index], PageSize);
							if (Status != XST_SUCCESS) {
								xil_printf("Failed to find the page size of 0X%X EEPROM\r\n", EepromAddr[Index]);
								return XST_FAILURE;
							}
							xil_printf("Page size %d\r\n", *PageSize);
							return XST_SUCCESS;
						}
					}
				}
			}
		}
		for (Index = 0; EepromAddr[Index] != 0; Index++) {
#ifndef SDT
			Status = IicPsFindDevice(EepromAddr[Index], DeviceId);
#else
			Status = IicPsFindDevice(EepromAddr[Index], BaseAddress);
#endif
			if (Status == XST_SUCCESS) {
				*Eeprom_Addr = EepromAddr[Index];
				*PageSize = PAGE_SIZE_32;
				return XST_SUCCESS;
			}
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
	u32 Index, IntrStatusReg;
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
* This function is used to figure out page size Eeprom slave device
*
* @param	Eeprom Address
*
* @param	Pagesize pointer
*
* @return	XST_SUCCESS if successful and also update the epprom slave
* device pagesize else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
static int FindEepromPageSize(u16 EepromAddr, u32 *PageSize_ptr)
{
	u32 Index, i;
	int Status = XST_FAILURE;
	AddressType Address = EEPROM_START_ADDRESS;
	int WrBfrOffset = 0;
	u32 ps[3] = {64, 32, 16};
	u32 PageSize_test, count;
	(void)EepromAddr;

	for (i = 0; i < 3; i++) {
		count = 0;
		PageSize_test = ps[i];
		*PageSize_ptr = PageSize_test;
		/*
		 * Initialize the data to write and the read buffer.
		 */
		if (PageSize_test == PAGE_SIZE_16) {
			WriteBuffer[0] = (u8) (Address);
			WrBfrOffset = 1;
		} else {
			WriteBuffer[0] = (u8) (Address >> 8);
			WriteBuffer[1] = (u8) (Address);
			WrBfrOffset = 2;
		}

		for (Index = 0; Index < PageSize_test; Index++) {
			WriteBuffer[WrBfrOffset + Index] = Index + i;
			ReadBuffer[Index] = 0;
		}

		/*
		 * Write to the EEPROM.
		 */
		Status = EepromWriteData(&IicInstance, WrBfrOffset + PageSize_test);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		/*
		 * Read from the EEPROM.
		 */
		Status = EepromReadData(&IicInstance, ReadBuffer, PageSize_test);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		/*
		 * Verify the data read against the data written.
		 */
		for (Index = 0; Index < PageSize_test; Index++) {
			if (ReadBuffer[Index] == Index + i) {
				count++;
			}
		}
		if (count == PageSize_test) {
			return XST_SUCCESS;
		}
	}
	return Status;
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
#ifndef SDT
static s32 IicPsSlaveMonitor(u16 Address, u16 DeviceId)
#else
static s32 IicPsSlaveMonitor(u16 Address, UINTPTR BaseAddress)
#endif
{
	u32 Index, IntrStatusReg;
	s32 Status;
	XIicPs *IicPtr;

	/*
	 * Initialize the IIC driver so that it is ready to use.
	 */
#ifndef SDT
	Status = IicPsConfig(DeviceId);
#else
	Status = IicPsConfig(BaseAddress);
#endif
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
