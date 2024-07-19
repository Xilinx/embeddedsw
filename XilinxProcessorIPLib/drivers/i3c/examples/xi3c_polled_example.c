
/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
 * @file xi3c_polled_example.c
 *
 * Design example to use the I3C device as master in polled mode.
 *
 * It makes the slave static address as their dynamic address.
 * It sends and receives data from slave.
 *
 * <pre> MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.00 gm   02/9/24 First release
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

#define I3C_DATALEN		90
#define I3C_SLAVE_ADDR		0x45

XI3c Xi3c_Instance;
XI3c *InstancePtr = &Xi3c_Instance;
/************************** Function Prototypes *******************************/

#ifndef SDT
int I3cMasterPolledExample(u16 DeviceId);
#else
int I3cMasterPolledExample(UINTPTR BaseAddress);
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

	xil_printf("I3C Master Polled Example Test \r\n");

	/*
	 * Run the I3c polled example in master mode, specify the Device
	 * ID that is specified in xparameters.h.
	 */
#ifndef SDT
	Status = I3cMasterPolledExample(XI3C_DEVICE_ID);
#else
	Status = I3cMasterPolledExample(XI3C_BASEADDRESS);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("I3C Master Polled Example Test Failed, Status = 0x%x\r\n", Status);
		return XST_FAILURE;
	}

	xil_printf("Successfully ran I3C Master Polled Example Test\r\n");
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
int I3cMasterPolledExample(u16 DeviceId)
#else
int I3cMasterPolledExample(UINTPTR BaseAddress)
#endif
{
	int Status;
	XI3c_Config *CfgPtr;
	u8 TxData[I3C_DATALEN];
	u8 RxData[I3C_DATALEN];
	XI3c_Cmd Cmd;
	u16 Index;
	u8 MaxLen[2];

	MaxLen[0] = (I3C_DATALEN & 0xFF00) >> 8;
	MaxLen[1] = (I3C_DATALEN & 0x00FF);

#ifndef SDT
	CfgPtr = XI3c_LookupConfig(DeviceId);
#else
	CfgPtr = XI3c_LookupConfig(BaseAddress);
#endif
	if (NULL == CfgPtr) {
		return XST_FAILURE;
	}
	XI3c_CfgInitialize(InstancePtr, CfgPtr, CfgPtr->BaseAddress);

	XI3C_BusInit(InstancePtr);

	/*
	 * Set Static address as dynamic address
	 */
	Cmd.NoRepeatedStart = 1;	/**< Disable repeated start */
	Cmd.Tid = 0;
	Cmd.Pec = 0;
	Cmd.Rw = 0;
	Cmd.CmdType = 1;
	Status = XI3c_SendTransferCmd(InstancePtr, &Cmd,
				      (u8)XI3C_CCC_BRDCAST_SETAASA);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Set Max Write length
	 */
	Cmd.NoRepeatedStart = 0;
	Cmd.Tid = 0;
	Cmd.Pec = 0;
	Cmd.Rw = 0;
	Cmd.CmdType = 1;
	Status = XI3c_SendTransferCmd(InstancePtr, &Cmd, (u8)XI3C_CCC_SETMWL);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	Cmd.SlaveAddr = (u8)I3C_SLAVE_ADDR;
	Cmd.NoRepeatedStart = 1;
	Cmd.Tid = 0;
	Cmd.Pec = 0;
	Cmd.CmdType = 1;               	/**< SDR mode */
	Status = XI3c_MasterSendPolled(InstancePtr, &Cmd, MaxLen, 2);

	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Set Max read length
	 */
	Cmd.NoRepeatedStart = 0;
	Cmd.Tid = 0;
	Cmd.Pec = 0;
	Cmd.Rw = 0;
	Cmd.CmdType = 1;
	Status = XI3c_SendTransferCmd(InstancePtr, &Cmd, (u8)XI3C_CCC_SETMRL);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	Cmd.SlaveAddr = (u8)I3C_SLAVE_ADDR;
	Cmd.NoRepeatedStart = 1;
	Cmd.Tid = 0;
	Cmd.Pec = 0;
	Cmd.CmdType = 1;
	Status = XI3c_MasterSendPolled(InstancePtr, &Cmd, MaxLen, 2);

	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Fill data to buffer
	 */

	for (Index = 0; Index < I3C_DATALEN; Index++) {
		TxData[Index] = Index;		/** < Test data */
		RxData[Index] = 0;
	}

	/*
	 * Send
	 */
	Cmd.SlaveAddr = (u8)I3C_SLAVE_ADDR;
	Cmd.NoRepeatedStart = 1;
	Cmd.Tid = 0;
	Cmd.Pec = 0;
	Cmd.CmdType = 1;
	Status = XI3c_MasterSendPolled(InstancePtr, &Cmd, TxData, I3C_DATALEN);

	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Recv
	 */
	Cmd.SlaveAddr = I3C_SLAVE_ADDR;
	Cmd.NoRepeatedStart = 1;
	Cmd.Tid = 0;
	Cmd.Pec = 0;
	Cmd.CmdType = 1;
	Status = XI3c_MasterRecvPolled(InstancePtr, &Cmd, RxData, I3C_DATALEN);

	if (Status != XST_SUCCESS) {
		return Status;
	}

	for (Index = 0; Index < I3C_DATALEN; Index++) {
		if(TxData[Index] != RxData[Index]) {
			xil_printf("Data miss match at index 0x%x\r\n", Index);
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}
