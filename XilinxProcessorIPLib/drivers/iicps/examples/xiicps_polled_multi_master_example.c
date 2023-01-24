/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xiicps_polled_multi_master_example.c
*
* Design example is to demonstrate multi-master support in polled mode.
* In case arbitration lost there will be retry..
*
* The XIicPs_MasterSendPolled() API is used to transmit the data and the
* XIicPs_MasterRecvPolled() API is used to receive the data.
* This example tested on ZCU102 board and slave address needs to be
* changed based on board design
*
* This example can run on zynqmp / versal IIC device as master in polled mode
* and Aardvark test hardware used as slave.
*
* It uses buffer size 132. Please set the send buffer of the Aardvark device to
* be continuous 64 bytes from 0x00 to 0x3F.
 *
 * <pre> MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.00a sg  03/09/19 First release
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
#define IIC_DEVICE_ID		XPAR_XIICPS_0_DEVICE_ID

/*
 * The slave address to send to and receive from.
 */
#define IIC_SLAVE_ADDR		0x55
#define IIC_SCLK_RATE		100000

/*
 * The following constant controls the length of the buffers to be sent
 * and received with the IIC.
 */
#define TEST_BUFFER_SIZE	132

/**************************** Type Definitions ********************************/


/************************** Function Prototypes *******************************/

s32 IicPsMultiMasterPolledExample(u16 DeviceId);
/************************** Variable Definitions ******************************/

XIicPs Iic;		/**< Instance of the IIC Device */

/*
 * The following buffers are used in this example to send and receive data
 * with the IIC.
 */
u8 SendBuffer[TEST_BUFFER_SIZE];    /**< Buffer for Transmitting Data */
u8 RecvBuffer[TEST_BUFFER_SIZE];    /**< Buffer for Receiving Data */


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
s32 main(void)
{
	s32 Status;

	xil_printf("IIC MultiMaster Polled Example Test \r\n");

	/*
	 * Run the Iic polled example in master mode, specify the Device
	 * ID that is specified in xparameters.h.
	 */
	Status = IicPsMultiMasterPolledExample(IIC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("IIC MultiMaster Polled Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran IIC MultiMaster Polled Example Test\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* The purpose of this function is to illustrate multi-master arbitration
* lost support in XIicPs driver.
*
* This function sends data and expects to receive data from slave device
*
* This function uses polled transfer functions to send and receive from device.
* In case arbitration lost status will re-initiate the transfer
*
* @param	DeviceId is the Device ID of the IicPs Device and is the
*		XPAR_<IICPS_instance>_DEVICE_ID value from xparameters.h
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
*******************************************************************************/
s32 IicPsMultiMasterPolledExample(u16 DeviceId)
{
	s32 Status;
	XIicPs_Config *Config;
	s32 Index;

	/*
	 * Initialize the IIC driver so that it's ready to use
	 * Look up the configuration in the config table,
	 * then initialize it.
	 */
	Config = XIicPs_LookupConfig(DeviceId);
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
		SendBuffer[Index] = (Index % TEST_BUFFER_SIZE);
		RecvBuffer[Index] = 0;
	}

	do {
		/*
		 * Wait until bus is idle to start another transfer.
		 */
		while (XIicPs_BusIsBusy(&Iic)) {
			/* NOP */
		}

		/*
		 * Send the buffer using the IIC and ignore the number of bytes sent
		 * as the return value since we are using it in interrupt mode.
		 */
		Status = XIicPs_MasterSendPolled(&Iic, SendBuffer,
		TEST_BUFFER_SIZE, IIC_SLAVE_ADDR);
	} while (Status == XST_IIC_ARB_LOST);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	do {
		/*
		 * Wait until bus is idle to start another transfer.
		 */
		while (XIicPs_BusIsBusy(&Iic)) {
			/* NOP */
		}

		Status = XIicPs_MasterRecvPolled(&Iic, RecvBuffer,
		TEST_BUFFER_SIZE, IIC_SLAVE_ADDR);
	} while (Status == XST_IIC_ARB_LOST);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Verify received data is correct.
	 */
	 for(Index = 0; Index < TEST_BUFFER_SIZE; Index ++) {

		/* Aardvark as slave can only set 64 bytes for output */
		if (RecvBuffer[Index] != Index % 64) {
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}
