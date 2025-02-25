
/******************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
 * @file xi3c_slave_polled_example.c
 *
 * Design example to use the I3C device as slave in polled mode.
 *
 * It performs the send and receive operations in slave mode.
 *
 * Note:
 * Master need to check for slave devices availability and then assign address.
 *
 * <pre> MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.2 gm   02/18/25  Add support for Slave mode
 *
 * </pre>
 *
 ****************************************************************************/

#include "xil_printf.h"
#include "xi3c.h"
#include "xi3c_hw.h"

#ifndef SDT
#define XI3C_DEVICE_ID		XPAR_XI3C_0_DEVICE_ID
#else
#define XI3C_BASEADDRESS	XPAR_XI3C_0_BASEADDR
#endif

/*
 * Length should be less than half of fifo depth for slave mode
 */
#define I3C_DATALEN		64

XI3c Xi3c_Instance;
XI3c *InstancePtr = &Xi3c_Instance;
/************************** Function Prototypes *******************************/

#ifndef SDT
int I3cSlavePolledExample(u16 DeviceId);
#else
int I3cSlavePolledExample(UINTPTR BaseAddress);
#endif
/******************************************************************************/
/******************************************************************************/
/**
*
* Main function to call the polled example.
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

	xil_printf("I3C Slave polled Example Test \r\n");

	/*
	 * Run the I3c polled example in slave mode, specify the Device
	 * ID that is specified in xparameters.h.
	 */
#ifndef SDT
	Status = I3cSlavePolledExample(XI3C_DEVICE_ID);
#else
	Status = I3cSlavePolledExample(XI3C_BASEADDRESS);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("I3C Slave Polled Example Test Failed, Status = 0x%x\r\n", Status);
		return XST_FAILURE;
	}

	xil_printf("Successfully ran I3C Slave Polled Example Test\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function does a minimal test on the I3c device and driver as a
* design example. The purpose of this function is to illustrate
* how to use the XI3c driver.
*
* This function sends data and expects to receive the same data through the I3C
*
* This function uses polled driver mode of the I3C.
*
* @param	DeviceId is the Device ID of the I3c Device and is the
*		XPAR_<I3C_instance>_DEVICE_ID value from xparameters.h
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note
*
*******************************************************************************/
#ifndef SDT
int I3cSlavePolledExample(u16 DeviceId)
#else
int I3cSlavePolledExample(UINTPTR BaseAddress)
#endif
{
	int Status;
	XI3c_Config *CfgPtr;
	u8 TxData[I3C_DATALEN];
	u8 RxData[I3C_DATALEN];
	u16 Index;

#ifndef SDT
	CfgPtr = XI3c_LookupConfig(DeviceId);
#else
	CfgPtr = XI3c_LookupConfig(BaseAddress);
#endif
	if (NULL == CfgPtr) {
		return XST_FAILURE;
	}
	XI3c_CfgInitialize(InstancePtr, CfgPtr, CfgPtr->BaseAddress);

	/*
	 * Fill data to buffer
	 */
	for (Index = 0; Index < I3C_DATALEN; Index++) {
		TxData[Index] = Index + 1;		/** < Test data */
		RxData[Index] = 0;
	}

	/*
	 * Wait for address assignment
	 */
	while (!XI3c_IsDyncAddrAssigned(InstancePtr));

	/*
	 * Master need to send SETMRL CCC to set max read length
	 */
	while (!XI3c_IsRespAvailable(InstancePtr));

	Status = XI3c_SlaveRecvPolled(InstancePtr, RxData);

	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Master need to send SETMWL CCC to set max write length
	 */
	while (!XI3c_IsRespAvailable(InstancePtr));

	Status = XI3c_SlaveRecvPolled(InstancePtr, RxData);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Send
	 */
	Status = XI3c_SlaveSendPolled(InstancePtr, TxData, I3C_DATALEN);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	while (!XI3c_IsRespAvailable(InstancePtr));

	/*
	 * Check error status
	 */
	if (XI3c_GetErrorStatus(InstancePtr))
		return XST_FAILURE;

	/*
	 * Recv
	 */
	while (!XI3c_IsRespAvailable(InstancePtr));

	Status = XI3c_SlaveRecvPolled(InstancePtr, RxData);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	xil_printf("Slave Recv data: \r\n");
	for (Index = 0; Index < I3C_DATALEN; Index++) {
		xil_printf("0x%x  ", RxData[Index]);
		if (Index != 0 && Index % 10 == 0)
			xil_printf("\n");
	}
	return XST_SUCCESS;
}
