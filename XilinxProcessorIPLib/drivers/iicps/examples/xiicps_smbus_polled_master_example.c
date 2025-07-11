
/******************************************************************************
* Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 * @file xiicps_smbus_polled_master_example.c
 *
 * This example can run on zynqmp / versal platform evaluation board and
 * IIC controller configured as master in polled mode and loopback setup used
 * for slave.
 * It sends and receives the data using IIC master device for SMBus transfers.
 *
 * <pre> MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.00  gm  06/30/22 First release
 * 3.18  gm  07/14/23 Added SDT support.
 * 3.22  bkv 07/09/25 Fixed GCC Warnings.
 *
 * </pre>
 *
 ****************************************************************************/

/***************************** Include Files **********************************/
#include "xparameters.h"
#include "xiicps.h"
#include "xil_printf.h"

/************************** Constant Definitions ******************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define IIC_DEVICE_ID		XPAR_XIICPS_0_DEVICE_ID
#else
#define XIICPS_BASEADDRESS	XPAR_XIICPS_0_BASEADDR
#endif

/*
 * The slave address to send to and receive from.
 */
#define IIC_SLAVE_ADDR		0x45
#define IIC_SCLK_RATE		100000

/*
 * The following constant controls the length of the buffers to be sent
 * and received with the IIC.
 */
#define TEST_BUFFER_SIZE	250

/**************************** Type Definitions ********************************/


/************************** Function Prototypes *******************************/

#ifndef SDT
int IicPsMasterPolledExample(u16 DeviceId);
#else
int IicPsMasterPolledExample(UINTPTR BaseAddress);
#endif
int XIicPsSmbusPolledMasterWriteBlockData(XIicPs *InstancePtr, u8 Command, u8 ByteCount, u8 *SendBufferPtr);
int XIicPsSmbusPolledMasterReadBlockData(XIicPs *InstancePtr, u8 Command, u8 ByteCount, u8 *RecvBufferPtr);
/************************** Variable Definitions ******************************/

XIicPs Iic;		/**< Instance of the IIC Device */

/*
 * The following buffers are used in this example to send and receive data
 * with the IIC.
 */
u8 SendBuffer[TEST_BUFFER_SIZE];    /**< Buffer for Transmitting Data */
u8 RecvBuffer[TEST_BUFFER_SIZE];    /**< Buffer for Receiving Data */

u8 Cmd = 0x11;								/* Send Command */
u8 RecvByteCount = 0;

/******************************************************************************/
/**
*
* Main function to call the polled master example.
*
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful.
*
* @note		None.
*
*******************************************************************************/
int main(void)
{
	int Status;
	int Index;

	xil_printf("SMBus Master Polled Example Test \r\n");

	/*
	 * Run the Iic polled example in master mode, specify the Device
	 * ID that is specified in xparameters.h.
	 */
#ifndef SDT
	Status = IicPsMasterPolledExample(IIC_DEVICE_ID);
#else
	Status = IicPsMasterPolledExample(XIICPS_BASEADDRESS);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("SMBus Master Polled Example Test Failed\r\n");
		return XST_FAILURE;
	}

	/*
	 * Print receive operation data
	 */

	xil_printf("SMBus Master : Byte count = 0x%x \r\n", RecvByteCount);

	for (Index = 0; Index < TEST_BUFFER_SIZE; Index++) {
		xil_printf("SMBus Master : Data: RecvBuffer[%d] = 0x%x \r\n", Index, RecvBuffer[Index]);
	}

	xil_printf("Successfully ran SMBus Master Polled Example Test\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function does a minimal test on the Iic device and driver as a
* design example. The purpose of this function is to illustrate
* how to use the XIicPs driver.
*
* This function sends and receives data as a smbus master in poll mode of the IIC.
*
* @param	DeviceId is the Device ID of the IicPs Device and is the
*		XPAR_<IICPS_instance>_DEVICE_ID value from xparameters.h
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
*******************************************************************************/
#ifndef SDT
int IicPsMasterPolledExample(u16 DeviceId)
#else
int IicPsMasterPolledExample(UINTPTR BaseAddress)
#endif
{
	int Status;
	XIicPs_Config *Config;
	int Index;

	/*
	 * Initialize the IIC driver so that it's ready to use
	 * Look up the configuration in the config table,
	 * then initialize it.
	 */
#ifndef SDT
	Config = XIicPs_LookupConfig(DeviceId);
#else
	Config = XIicPs_LookupConfig(BaseAddress);
#endif
	if (NULL == Config) {
		return XST_FAILURE;
	}

	Status = XIicPs_CfgInitialize(&Iic, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built correctly.
	 */
	Status = XIicPs_SelfTest(&Iic);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the IIC serial clock rate.
	 */
	XIicPs_SetSClk(&Iic, IIC_SCLK_RATE);

	/*
	 * Initialize the send buffer bytes with a pattern to send and the
	 * the receive buffer bytes to zero to allow the receive data to be
	 * verified.
	 */
	for (Index = 0; Index < TEST_BUFFER_SIZE; Index++) {
		SendBuffer[Index] = Index;
	}

	Status = XIicPsSmbusPolledMasterWriteBlockData(&Iic, Cmd, TEST_BUFFER_SIZE, SendBuffer);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	while (XIicPs_BusIsBusy(&Iic)) {
		/* NOP */
	}

	/*
	 * SMBus Slave Receive operation
	 */
	Status = XIicPsSmbusPolledMasterReadBlockData(&Iic, Cmd, TEST_BUFFER_SIZE, RecvBuffer);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function writes a buffer of data to the smbus slave.
*
* @param	InstancePtr	contains the address of XIicPs instance.
* @param	Command contains send command.
* @param	ByteCount contains the number of bytes to write.
* @param	SendBufferPtr contains the address of send buffer.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note	None
*
******************************************************************************/
int XIicPsSmbusPolledMasterWriteBlockData(XIicPs *InstancePtr, u8 Command, u8 ByteCount, u8 *SendBufferPtr)
{
	int Status;
	u32 Index;
	u32 BufferIndex;
	static u8 SmbusSendBuffer[TEST_BUFFER_SIZE + 2];
	(void)InstancePtr;

	while (XIicPs_BusIsBusy(&Iic)) {
		/* NOP */
	}

	/*
	 * Fill and Send the Data.
	 */

	SmbusSendBuffer[0] = Command;
	SmbusSendBuffer[1] = ByteCount;

	for (Index = 2, BufferIndex = 0; Index < (TEST_BUFFER_SIZE + 2); Index++, BufferIndex++) {
		SmbusSendBuffer[Index] = SendBufferPtr[BufferIndex];
	}

	/*
	 * Send the buffer using the IIC and ignore the number of bytes sent
	 * as the return value since we are using it in interrupt mode.
	 */
	Status = XIicPs_MasterSendPolled(&Iic, SmbusSendBuffer, ByteCount + 2, IIC_SLAVE_ADDR);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	while (XIicPs_BusIsBusy(&Iic)) {
		/* NOP */
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function sends a command and reads a buffer of data from the smbus slave.
*
* @param	InstancePtr	contains the address of XIicPs instance.
* @param	Command contains send command.
* @param	ByteCount contains the number of bytes to read.
* @param	RecvBufferPtr contains the address of receive buffer.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note	None
*
******************************************************************************/
int XIicPsSmbusPolledMasterReadBlockData(XIicPs *InstancePtr, u8 Command, u8 ByteCount, u8 *RecvBufferPtr)
{
	int Status;
	u32 Index;
	u32 BufferIndex;
	static u8 SmbusRecvBuffer[TEST_BUFFER_SIZE + 1];
	(void)InstancePtr;

	/*
	 * Command Part
	 */

	/*
	 * Enable repeated start option.
	 * This call will give an indication to the driver.
	 * The hold bit is actually set before beginning the following transfer
	 */
	XIicPs_SetOptions(&Iic, XIICPS_REP_START_OPTION);

	SendBuffer[0] = Command;

	Status = XIicPs_MasterSendPolled(&Iic, SendBuffer, 1, IIC_SLAVE_ADDR);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XIicPs_ClearOptions(&Iic, XIICPS_REP_START_OPTION);

	/*
	 * Receive Part
	 */

	RecvByteCount = 0;

	for (Index = 0; Index < TEST_BUFFER_SIZE; Index++) {
		SmbusRecvBuffer[Index] = 0;
		RecvBufferPtr[Index] = 0;
	}
	SmbusRecvBuffer[Index] = 0;

	Status = XIicPs_MasterRecvPolled(&Iic, SmbusRecvBuffer,
					 ByteCount + 1, IIC_SLAVE_ADDR);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	while (XIicPs_BusIsBusy(&Iic));

	RecvByteCount = SmbusRecvBuffer[0];

	for (BufferIndex = 0, Index = 1; Index < (TEST_BUFFER_SIZE + 1); BufferIndex++, Index++) {
		RecvBufferPtr[BufferIndex] = SmbusRecvBuffer[Index];
	}

	return XST_SUCCESS;
}
