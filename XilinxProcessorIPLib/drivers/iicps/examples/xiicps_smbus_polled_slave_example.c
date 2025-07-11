/******************************************************************************
* Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 * @file xiicps_smbus_polled_slave_example.c
 *
 * This example can run on zynqmp / versal platform evaluation board and
 * IIC controller configured slave in polled mode and loopback setup used for
 * master.
 * It sends and receives the data using IIC device as slave for SMBus transfers.
 *
 * <pre> MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.00  gm  05/10/22 First release
 * 3.18  gm  07/14/23 Added SDT support.
 * 3.22  bkv 07/09/25 Fixed GCC Warning.
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

/* The slave address to send to and receive from.
 */
#define IIC_SLAVE_ADDR		0x45
#define IIC_SCLK_RATE		400000

/*
 * The following constant controls the length of the buffers to be sent
 * and received with the IIC
 */
#define BUFFER_SIZE	250

/**************************** Type Definitions ********************************/

/************************** Function Prototypes *******************************/

#ifndef SDT
int IicPsSmbusSlavePolledExample(u16 DeviceId);
#else
int IicPsSmbusSlavePolledExample(UINTPTR BaseAddress);
#endif
int XIicPsSmbusPolledWriteBlockData(XIicPs *InstancePtr, u8 *Command, u8 ByteCount, u8 *SendBufferPtr);
int XIicPsSmbusPolledReadBlockData(XIicPs *InstancePtr, u8 *Command, u8 *ByteCount, u8 *RecvBufferPtr);

/************************** Variable Definitions ******************************/

XIicPs Iic;				/* Instance of the IIC Device */

/*
 * The following buffers are used in this example to send and receive data
 * with the IIC. These buffers are defined as global so that they are not
 * defined on the stack.
 */
u8 SendBuffer[BUFFER_SIZE];	/* Buffer for Transmitting Data */
u8 RecvBuffer[BUFFER_SIZE];	/* Buffer for Receiving Data */

u8 RecvByteCount = 0;
u8 RecvCmd;			/* Received command */
u8 Cmd;				/* Command received during Send operation */

/******************************************************************************/
/**
*
* Main function to call the polled slave example.
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

	xil_printf("IIC SMBus Slave Polled Example Test \r\n");

	/*
	 * Run the Iic polled slave example , specify the Device ID that is
	 * generated in xparameters.h.
	 */
#ifndef SDT
	Status = IicPsSmbusSlavePolledExample(IIC_DEVICE_ID);
#else
	Status = IicPsSmbusSlavePolledExample(XIICPS_BASEADDRESS);
#endif

	if (Status != XST_SUCCESS) {
		xil_printf("IIC SMBus Slave Polled Example Test Failed\r\n");
		return XST_FAILURE;
	}

	/*
	 * Print receive operation data
	 */

	xil_printf("SMBus Slave : Receive operation : Command = 0x%x \r\n", RecvCmd);
	xil_printf("SMBus Slave : Byte count: RecvByteCount = 0x%x \r\n", RecvByteCount);
	for (Index = 0; Index < BUFFER_SIZE; Index++) {
		xil_printf("SMBus Slave : Data: RecvBuffer[%d] = 0x%x \r\n", Index, RecvBuffer[Index] );
	}

	/*
	 * Print send operation data
	 */

	xil_printf("SMBus Slave : Send operation : Command = 0x%x \r\n", Cmd);

	xil_printf("Successfully ran IIC Slave Polled Example Test\r\n");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function does polled mode transfer in slave mode. It first sends to
* master then receives.
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
int IicPsSmbusSlavePolledExample(u16 DeviceId)
#else
int IicPsSmbusSlavePolledExample(UINTPTR BaseAddress)
#endif
{
	int Status;
	XIicPs_Config *Config;
	int Index = 0;

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

	XIicPs_SetupSlave(&Iic, IIC_SLAVE_ADDR);

	/*
	 * Set the IIC serial clock rate.
	 */
	XIicPs_SetSClk(&Iic, IIC_SCLK_RATE);


	/*
	 * SMBus Slave Receive operation
	 */

	XIicPsSmbusPolledReadBlockData(&Iic, &RecvCmd, &RecvByteCount, RecvBuffer);

	/*
	 * SMBus Slave Send operation
	 */

	for (Index = 0; Index < BUFFER_SIZE; Index++) {
		SendBuffer[Index] = Index;
	}
	XIicPsSmbusPolledWriteBlockData(&Iic, &Cmd, BUFFER_SIZE, SendBuffer);

	return XST_SUCCESS;
}

int XIicPsSmbusPolledWriteBlockData(XIicPs *InstancePtr, u8 *Command, u8 ByteCount, u8 *SendBufferPtr)
{
	int Status;
	u8 Cmmd = 0;
	u32 Index;
	u32 BufferIndex;
	static u8 SmbusSendBuffer[BUFFER_SIZE + 1];

	InstancePtr->RecvBufferPtr = &Cmmd;

	/*
	 * Command Recv part
	 */

	while ((XIicPs_RxDataValidStatus(InstancePtr)) != 0x20U) {
		/* NOP */
	}

	XIicPs_RecvByte(InstancePtr);

	*Command = Cmmd;

	XIicPs_DisableAllInterrupts(InstancePtr->Config.BaseAddress);
	(void)XIicPs_ReadReg(InstancePtr->Config.BaseAddress, (u32)XIICPS_ISR_OFFSET);

	/*
	 * SMBus Slave Send part
	 */
	SmbusSendBuffer[0] = ByteCount;

	for (Index = 1, BufferIndex = 0; Index < (BUFFER_SIZE + 1); Index++, BufferIndex++) {
		SmbusSendBuffer[Index] = SendBufferPtr[BufferIndex];
	}

	Status = XIicPs_SlaveSendPolled(&Iic, SmbusSendBuffer, BUFFER_SIZE + 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait until the bus transfer finishes.
	 */
	while (XIicPs_BusIsBusy(&Iic)) {
		/* NOP */
	}

	return XST_SUCCESS;
}

int XIicPsSmbusPolledReadBlockData(XIicPs *InstancePtr, u8 *Command, u8 *ByteCount, u8 *RecvBufferPtr)
{
	int Status;
	u32 Index;
	u32 BufferIndex;
	static u8 SmbusRecvBuffer[BUFFER_SIZE + 2];
	(void)InstancePtr;

	/*
	 * Receive data from master.
	 * Receive errors will be signaled through event flag.
	 */

	for (Index = 0; Index < BUFFER_SIZE; Index++) {
		SmbusRecvBuffer[Index] = 0;
		RecvBufferPtr[Index] = 0;
	}

	Status = XIicPs_SlaveRecvPolled(&Iic, SmbusRecvBuffer, 0);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	*Command = SmbusRecvBuffer[0];
	*ByteCount = SmbusRecvBuffer[1];

	for (BufferIndex = 0, Index = 2; Index < (BUFFER_SIZE + 2); BufferIndex++, Index ++) {
		RecvBufferPtr[BufferIndex] = SmbusRecvBuffer[Index];
	}

	return XST_SUCCESS;
}
