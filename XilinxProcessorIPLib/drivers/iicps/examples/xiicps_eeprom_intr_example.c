/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xiicps_eeprom_intr_example.c
*
* This file consists of a interrutp mode design example which uses the Xilinx
* PS IIC device and XIicPs driver to exercise the EEPROM.
* This example runs on zynqmp evaluation board (zcu102).
*
* The XIicPs_MasterSend() API is used to transmit the data and the
* XIicPs_MasterRecv() API is used to receive the data.
*
* The example is tested with a 2Kb/8Kb serial IIC EEPROM (ST M24C02/M24C08).
* The WP pin of this EEPROM is hardwired to ground on the HW in which this
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
* 3.11   rna  10/16/19 Added support for 64 page size Eeproms on Versal based
*		      boards, scanning for eeprom until found on all I2C
*		      instances
*        rna  03/26/20 Eeprom page size detection support is added.
* 3.18   gm   08/26/23 Added SDT support.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "sleep.h"
#include "xiicps.h"
#include "xscugic.h"
#include "xil_exception.h"
#include "xil_printf.h"
#include "xplatform_info.h"
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
#define INTC_DEVICE_ID	XPAR_SCUGIC_SINGLE_DEVICE_ID
#endif

/*
 * The following constant defines the address of the IIC Slave device on the
 * IIC bus. Note that since the address is only 7 bits, this constant is the
 * address divided by 2.
 */

#define IIC_SCLK_RATE		100000
#define SLV_MON_LOOP_COUNT 0x000FFFFF	/**< Slave Monitor Loop Count*/
#define MUX_ADDR 0x74
#define MAX_CHANNELS 0x08

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

int IicPsEepromIntrExample(void);
static int EepromWriteData(XIicPs *IicInstance, u16 ByteCount);
static int EepromReadData(XIicPs *IicInstance, u8 *BufferPtr, u16 ByteCount);
static void Handler(void *CallBackRef, u32 Event);
#ifndef SDT
static int IicPsSlaveMonitor(u16 Address, u16 DeviceId, u32 Int_Id);
static int SetupInterruptSystem(XIicPs *IicPsPtr, u32 Int_Id);
static int IicPsConfig(u16 DeviceId, u32 Int_Id);
static int IicPsFindDevice(u16 addr, u16 DeviceId);
#else
static int IicPsSlaveMonitor(u16 Address, UINTPTR BaseAddress);
static int IicPsConfig(UINTPTR BaseAddress);
static int IicPsFindDevice(u16 addr, UINTPTR BaseAddress);
#endif
static int MuxInitChannel(u16 MuxIicAddr, u8 WriteBuffer);
static int FindEepromDevice(u16 Address);
static int IicPsFindEeprom(u16 *Eeprom_Addr, u32 *PageSize);
static int FindEepromPageSize(u16 EepromAddr, u32 *PageSize_ptr);
/************************** Variable Definitions *****************************/
#ifndef TESTAPP_GEN
XIicPs IicInstance;		/* The instance of the IIC device. */
XScuGic InterruptController;	/* The instance of the Interrupt Controller. */
#endif
u32 Platform;

/*
 * Write buffer for writing a page.
 */
u8 WriteBuffer[sizeof(AddressType) + MAX_SIZE];

u8 ReadBuffer[MAX_SIZE];	/* Read buffer for reading a page. */

volatile u8 TransmitComplete;	/**< Flag to check completion of Transmission */
volatile u8 ReceiveComplete;	/**< Flag to check completion of Reception */
volatile u32 TotalErrorCount;	/**< Total Error Count Flag */
volatile u32 SlaveResponse;		/**< Slave Response Flag */

/**Searching for the required EEPROM Address and user can also add
 * their own EEPROM Address in the below array list**/
u16 EepromAddr[] = {0x54,0x55,0};
u16 MuxAddr[] = {0x74,0};
u16 EepromSlvAddr;
u32 PageSize;
#ifdef SDT
extern XIicPs_Config XIicPs_ConfigTable[XPAR_XIICPS_NUM_INSTANCES];
#endif
/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* Main function to call the Iic EEPROM interrupt example.
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
	int Status;

	xil_printf("IIC EEPROM Interrupt Example Test \r\n");

	/*
	 * Run the Iic EEPROM interrupt mode example.
	 */
	Status = IicPsEepromIntrExample();
	if (Status != XST_SUCCESS) {
		xil_printf("IIC EEPROM Interrupt Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran IIC EEPROM Interrupt Example Test\r\n");
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
int IicPsEepromIntrExample(void)
{
	u32 Index;
	int Status;
	AddressType Address = EEPROM_START_ADDRESS;
	int WrBfrOffset;


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
static int EepromWriteData(XIicPs *IicInstance, u16 ByteCount)
{

	TransmitComplete = FALSE;

	/*
	 * Send the Data.
	 */
	XIicPs_MasterSend(IicInstance, WriteBuffer,
			   ByteCount, EepromSlvAddr);

	/*
	 * Wait for the entire buffer to be sent, letting the interrupt
	 * processing work in the background, this function may get
	 * locked up in this loop if the interrupts are not working
	 * correctly.
	 */
	while (TransmitComplete == FALSE) {
		if (0 != TotalErrorCount) {
			return XST_FAILURE;
		}
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
static int EepromReadData(XIicPs *IicInstance, u8 *BufferPtr, u16 ByteCount)
{
	int Status;
	AddressType Address = EEPROM_START_ADDRESS;
	int WrBfrOffset;

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

	ReceiveComplete = FALSE;

	/*
	 * Receive the Data.
	 */
	XIicPs_MasterRecv(IicInstance, BufferPtr,
			   ByteCount, EepromSlvAddr);

	while (ReceiveComplete == FALSE) {
		if (0 != TotalErrorCount) {
			return XST_FAILURE;
		}
	}

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	while (XIicPs_BusIsBusy(IicInstance));

	return XST_SUCCESS;
}

#ifndef SDT
/******************************************************************************/
/**
*
* This function setups the interrupt system such that interrupts can occur
* for the IIC.
*
* @param	IicPsPtr contains a pointer to the instance of the Iic
*		which is going to be connected to the interrupt controller.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
*******************************************************************************/
static int SetupInterruptSystem(XIicPs *IicPsPtr, u32 Int_Id)
{
	int Status;
	XScuGic_Config *IntcConfig; /* Instance of the interrupt controller */

	Xil_ExceptionInit();

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(&InterruptController, IntcConfig,
					IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
				(Xil_ExceptionHandler)XScuGic_InterruptHandler,
				&InterruptController);

	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device.
	 */
	Status = XScuGic_Connect(&InterruptController, Int_Id,
			(Xil_InterruptHandler)XIicPs_MasterInterruptHandler,
			(void *)IicPsPtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Enable the interrupt for the Iic device.
	 */
	XScuGic_Enable(&InterruptController, Int_Id);


	/*
	 * Enable interrupts in the Processor.
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* This function is the handler which performs processing to handle data events
* from the IIC.  It is called from an interrupt context such that the amount
* of processing performed should be minimized.
*
* This handler provides an example of how to handle data for the IIC and
* is application specific.
*
* @param	CallBackRef contains a callback reference from the driver, in
*		this case it is the instance pointer for the IIC driver.
* @param	Event contains the specific kind of event that has occurred.
* @param	EventData contains the number of bytes sent or received for sent
*		and receive events.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void Handler(void *CallBackRef, u32 Event)
{
	/*
	 * All of the data transfer has been finished.
	 */

	if (0 != (Event & XIICPS_EVENT_COMPLETE_SEND)) {
		TransmitComplete = TRUE;
	} else if (0 != (Event & XIICPS_EVENT_COMPLETE_RECV)){
		ReceiveComplete = TRUE;
	} else if (0 != (Event & XIICPS_EVENT_SLAVE_RDY)) {
		SlaveResponse = TRUE;
	} else if (0 != (Event & XIICPS_EVENT_ERROR)){
		TotalErrorCount++;
	}
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
static int MuxInitChannel(u16 MuxIicAddr, u8 WriteBuffer)
{
	u8 Buffer = 0;

	TotalErrorCount = 0;
	TransmitComplete = FALSE;
	TotalErrorCount = 0;

	XIicPs_MasterSend(&IicInstance, &WriteBuffer,1,MuxIicAddr);
	while (TransmitComplete == FALSE) {
		if (0 != TotalErrorCount) {
			return XST_FAILURE;
		}
	}
	/*
	 * Wait until bus is idle to start another transfer.
	 */

	while (XIicPs_BusIsBusy(&IicInstance));

	ReceiveComplete = FALSE;
	/*
	 * Receive the Data.
	 */
	XIicPs_MasterRecv(&IicInstance, &Buffer,1, MuxIicAddr);

	while (ReceiveComplete == FALSE) {
		if (0 != TotalErrorCount) {
			return XST_FAILURE;
		}
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
* @param	DeviceId instance and Interrupt ID mapped to the device.
*
* @return	XST_SUCCESS if pass, otherwise XST_FAILURE.
*
* @note		None.
*
****************************************************************************/
#ifndef SDT
static int IicPsConfig(u16 DeviceId, u32 Int_Id)
#else
static int IicPsConfig(UINTPTR BaseAddress)
#endif
{
	int Status;
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
	 * Setup the Interrupt System.
	 */
#ifndef SDT
	Status = SetupInterruptSystem(&IicInstance, Int_Id);
#else
	Status = XSetupInterruptSystem(&IicInstance, XIicPs_MasterInterruptHandler,
			ConfigPtr->IntrId,
			ConfigPtr->IntrParent,
			XINTERRUPT_DEFAULT_PRIORITY);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the handlers for the IIC that will be called from the
	 * interrupt context when data has been sent and received, specify a
	 * pointer to the IIC driver instance as the callback reference so
	 * the handlers are able to access the instance data.
	 */
	XIicPs_SetStatusHandler(&IicInstance, (void *) &IicInstance, Handler);

	/*
	 * Set the IIC serial clock rate.
	 */
	XIicPs_SetSClk(&IicInstance, IIC_SCLK_RATE);
	return XST_SUCCESS;
}

#ifndef SDT
static int IicPsFindDevice(u16 addr, u16 DeviceId)
#else
static int IicPsFindDevice(u16 addr, UINTPTR BaseAddress)
#endif
{
	int Status;

#ifndef SDT
	if (DeviceId == 0){
		Status = IicPsSlaveMonitor(addr,DeviceId,XPAR_XIICPS_0_INTR);
		if (Status == XST_SUCCESS) {
			return XST_SUCCESS;
		}
	}
	else if (DeviceId == 1) {
		Status = IicPsSlaveMonitor(addr,DeviceId,XPAR_XIICPS_1_INTR);
		if (Status == XST_SUCCESS) {
			return XST_SUCCESS;
		}
	}
#else
	Status = IicPsSlaveMonitor(addr, BaseAddress);
	if (Status == XST_SUCCESS) {
		return XST_SUCCESS;
	}
#endif
	return XST_FAILURE;
}
/*****************************************************************************/
/**
* This function is use to figure out the Eeprom slave device
*
* @param	Eeprom slave address
*
* @param	Pagesize pointer
*
* @return	XST_SUCCESS if successful and also update the epprom slave
* device address in addr variable else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
static int IicPsFindEeprom(u16 *Eeprom_Addr,u32 *PageSize)
{
	int Status;
	u16 DeviceId;
	int MuxIndex,Index;
	u8 MuxChannel;
#ifdef SDT
	u32 BaseAddress;
#endif

	for (DeviceId = 0; DeviceId < XPAR_XIICPS_NUM_INSTANCES; DeviceId++) {
		for(MuxIndex=0;MuxAddr[MuxIndex] != 0;MuxIndex++){
#ifndef SDT
			Status = IicPsFindDevice(MuxAddr[MuxIndex], DeviceId);
#else
			BaseAddress = XIicPs_ConfigTable[DeviceId].BaseAddress;
			Status = IicPsFindDevice(MuxAddr[MuxIndex], BaseAddress);
#endif
			if (Status == XST_SUCCESS) {
				for(Index=0;EepromAddr[Index] != 0;Index++) {
					for(MuxChannel = 0x01; MuxChannel <= MAX_CHANNELS; MuxChannel = MuxChannel << 1) {
						Status = MuxInitChannel(MuxAddr[MuxIndex], MuxChannel);
						if (Status != XST_SUCCESS) {
							xil_printf("Failed to enable the MUX channel\r\n");
							return XST_FAILURE;
						}
						Status = FindEepromDevice(EepromAddr[Index]);
						FindEepromDevice(MUX_ADDR);
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
		for(Index=0;EepromAddr[Index] != 0;Index++) {
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
static int FindEepromDevice(u16 Address)
{
	int Index;
	XIicPs *IicPtr = &IicInstance;
	SlaveResponse = FALSE;

	XIicPs_DisableAllInterrupts(IicPtr->Config.BaseAddress);
	XIicPs_EnableSlaveMonitor(&IicInstance, Address);

		TotalErrorCount = 0;

		Index = 0;

		/*
		 * Wait for the Slave Monitor Interrupt, the interrupt processing
		 * works in the background, this function may get locked up in this
		 * loop if the interrupts are not working correctly or the slave
		 * never responds.
		 */
		while ((!SlaveResponse) && (Index < SLV_MON_LOOP_COUNT)) {
			Index++;

			/*
			 * Ignore any errors. The hardware generates NACK interrupts
			 * if the slave is not present.
			 */
			if (0 != TotalErrorCount) {
				xil_printf("Test error unexpected NACK\n");
			}
		}

		if (Index >= SLV_MON_LOOP_COUNT) {
			XIicPs_DisableSlaveMonitor(&IicInstance);
			return XST_FAILURE;

		}

		XIicPs_DisableSlaveMonitor(&IicInstance);
		return XST_SUCCESS;
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

	for (i = 0; i < 3; i++)
	{
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
		if (count == PageSize_test)
		{
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
static int IicPsSlaveMonitor(u16 Address, u16 DeviceId, u32 Int_Id)
#else
static int IicPsSlaveMonitor(u16 Address, UINTPTR BaseAddress)
#endif
{
	u32 Index;
	int Status;
	XIicPs *IicPtr;

	SlaveResponse = FALSE;
	/*
	 * Initialize the IIC driver so that it is ready to use.
	 */
#ifndef SDT
	Status = IicPsConfig(DeviceId,Int_Id);
#else
	Status = IicPsConfig(BaseAddress);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	IicPtr = &IicInstance;
	XIicPs_DisableAllInterrupts(IicPtr->Config.BaseAddress);
	XIicPs_EnableSlaveMonitor(&IicInstance, Address);

	TotalErrorCount = 0;

	Index = 0;

	/*
	 * Wait for the Slave Monitor Interrupt, the interrupt processing
	 * works in the background, this function may get locked up in this
	 * loop if the interrupts are not working correctly or the slave
	 * never responds.
	 */
	while ((!SlaveResponse) && (Index < SLV_MON_LOOP_COUNT)) {
		Index++;

		/*
		 * Ignore any errors. The hardware generates NACK interrupts
		 * if the slave is not present.
		 */
		if (0 != TotalErrorCount) {
			xil_printf("Test error unexpected NACK\n");
			return XST_FAILURE;
		}
	}

	if (Index >= SLV_MON_LOOP_COUNT) {
		return XST_FAILURE;

	}

	XIicPs_DisableSlaveMonitor(&IicInstance);
	return XST_SUCCESS;
}

/******************************************************************************/
