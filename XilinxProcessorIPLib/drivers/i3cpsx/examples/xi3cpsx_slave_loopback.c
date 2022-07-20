/******************************************************************************
* Copyright (C) 2022 AMD, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xi3cpsx_slave_loopback.c
*
* This file consists of a slave mode design example which uses the Xilinx
* I3C device in slave mode in a loopback setup.
*
* The master recives the data and also sends the data to the slave.
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
* 1.00a sd  06/10/22 First release
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/

#include "xil_printf.h"
#include "xi3cpsx.h"
#include "xi3cpsx_hw.h"
#include "xi3cpsx_pr.h"

#define I3C_DEVICE_ID		XPAR_XI3CPSX_0_DEVICE_ID
#define I3C_SLAVE_ID		XPAR_XI3CPSX_1_DEVICE_ID
#define LEN 10

XI3cPsx Xi3cPs_InstanceMaster;
XI3cPsx *InstancePtr = &Xi3cPs_InstanceMaster;
XI3cPsx Xi3cPs_InstanceSlave;
XI3cPsx *InstancePtrSlave = &Xi3cPs_InstanceSlave;

int I3cPsxSlaveLoopbackExample(u16 DeviceId);
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

	xil_printf("I3C Slave Loopback Example Test \r\n");

	/*
	 * Run the I3c loopback example in slave mode, specify the Device
	 * ID that is specified in xparameters.h.
	 */
	Status = I3cPsxSlaveLoopbackExample(I3C_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("I3C Slave Loopback Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran I3C Slave Loopback Test\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function does a slave loopback test on the I3c device and driver as a
* design example. The purpose of this function is to illustrate
* how to use the XI3cPsx driver.
*
* This function sends data and expects to receive the same data through the I3C
* slave loopback. It expects the controllers to be in loopback.
*
* This function uses slave driver mode of the I3C.
*
* @param	DeviceId is the Device ID of the I3cPsx Device and is the
*		XPAR_<I3C_instance>_DEVICE_ID value from xparameters.h
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note
*
*******************************************************************************/
int I3cPsxSlaveLoopbackExample(u16 DeviceId)
{

	s32 Ret;
	XI3cPsx_Config *CfgPtr;
	u8 RxData[LEN];
	XI3cPsx_Cmd DAA_Cmd;
	u8 TxData[4] = { 0xAA, 0xBB, 0xCC, 0xDD};
	u8 TxDataTest[LEN] = { 55, 23, 24, 25, 26, 27, 28, 29, 30, 31   };
	struct CmdInfo CmdInfo;
	u16 RxLen;
	int Index;

	xil_printf("I3C Slave test application\n\r");

	CfgPtr = XI3cPsx_LookupConfig(I3C_SLAVE_ID);
	XI3cPsx_CfgInitialize(InstancePtrSlave, CfgPtr, CfgPtr->BaseAddress);


	XI3cPsx_SetupSlave(InstancePtrSlave, 0x5d);

	CfgPtr = XI3cPsx_LookupConfig(DeviceId);
	XI3cPsx_CfgInitialize(InstancePtr, CfgPtr, CfgPtr->BaseAddress);

/*
 * It is necessary to provide SCL clocks to the DWC_mipi_i3c Slave controller to make it come
 * out of the reset. This can be achieved by scheduling any transfer (dummy transfer) to the
 * controller ending in a STOP before initiating valid transfers. The dummy transfer is ignored by
 * the DWC_mipi_i3c Slave.
 *
 * So doing a dummy transter
*/
	XI3cPsx_ResetFifos(InstancePtr);
	CmdInfo.RxLen = 1;
	CmdInfo.RxBuff = RxData;
	CmdInfo.SlaveAddr = 0;
	CmdInfo.Cmd = I3C_CCC_GETDCR;
	Ret =	XI3cPsx_SendTransferCmd(InstancePtr, &CmdInfo);
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress,
				XI3CPSX_DEVICE_CTRL, XI3CPSX_DEVICE_CTRL_ENABLE_MASK |XI3CPSX_DEVICE_CTRL_RESUME_MASK);


	DAA_Cmd.TransCmd = COMMAND_PORT_ARG_DATA_LEN(3) | COMMAND_PORT_TRANSFER_ARG;
	DAA_Cmd.TransArg = (COMMAND_PORT_SPEED(0) |
			COMMAND_PORT_DEV_INDEX(0) |
			COMMAND_PORT_TID(3) |
			COMMAND_PORT_TOC |
			COMMAND_PORT_ROC);
	DAA_Cmd.RxBuf = NULL;
	Ret = XI3cPsx_MasterSendPolled(InstancePtr,(u8 *) &TxData, 3, DAA_Cmd);

	Ret = XI3cPsx_SlaveRecvPolled(InstancePtrSlave, (u8 *)&RxData);
	xil_printf("Data from slave is  %x %x %x\n", RxData[0], RxData[1], RxData[2]);

	XI3cPsx_ResetFifos(InstancePtr);
	DAA_Cmd.TransCmd = 0x000a101a ;
	DAA_Cmd.TransArg = 0x48000000;
	Ret = XI3cPsx_SlaveSendPolled(InstancePtrSlave, &TxDataTest,
		 LEN, DAA_Cmd);
	RxLen = LEN;
	DAA_Cmd.RxBuf = RxData;
	DAA_Cmd.TransCmd = COMMAND_PORT_ARG_DATA_LEN(0xa) | COMMAND_PORT_TRANSFER_ARG;
	DAA_Cmd.TransArg = (COMMAND_PORT_SPEED(0) |
			COMMAND_PORT_DEV_INDEX(0) |
			COMMAND_PORT_TOC |
			COMMAND_PORT_READ_TRANSFER |
			COMMAND_PORT_ROC);
	Ret = XI3cPsx_MasterRecvPolled(InstancePtr, RxData, RxLen, &DAA_Cmd);

	for( Index = 0 ; Index < LEN; Index++) {
		xil_printf("Data at slave is  %d\n", RxData[Index]);
	}

	xil_printf(" Successfully ran the I3C test\n");
	return 0;
}
