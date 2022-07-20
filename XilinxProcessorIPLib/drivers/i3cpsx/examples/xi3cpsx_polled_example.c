
/******************************************************************************
* Copyright (C) 2022 AMD, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
 * @file xi3cpsx_polled_example.c
 *
 * Design example to use the I3C device as master in polled mode.
 *
 * It sends and also receives data from and to slave.
 *
 * <pre> MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.00 sd   06/21/22 First release
 *
 * </pre>
 *
 ****************************************************************************/

#include "xil_printf.h"
#include "xi3cpsx.h"
#include "xi3cpsx_hw.h"
#include "xi3cpsx_pr.h"

#define I3C_DEVICE_ID		XPAR_XI3CPSX_0_DEVICE_ID
#define I3C_WHO_AM_I		0x0F
#define I3C_CTRL		0x11
#define I3C_INT_SRC		0x1A
#define I3C_DATALEN		10

XI3cPsx Xi3cPs_Instance;
XI3cPsx *InstancePtr = &Xi3cPs_Instance;
/************************** Function Prototypes *******************************/

int I3cPsxMasterPolledExample(u16 DeviceId);
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
	Status = I3cPsxMasterPolledExample(I3C_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("I3C Master Polled Example Test Failed\r\n");
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
* how to use the XI3cPsx driver.
*
* This function sends data and expects to receive the same data through the I3C
*
* This function uses polled driver mode of the I3C.
*
* @param	DeviceId is the Device ID of the I3cPsx Device and is the
*		XPAR_<I3C_instance>_DEVICE_ID value from xparameters.h
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note
*
*******************************************************************************/
int I3cPsxMasterPolledExample(u16 DeviceId)
{
	int Status;
	XI3cPsx_Config *CfgPtr;
	u8 RxData[I3C_DATALEN];
	struct CmdInfo CmdInfo;
	u16 RxLen;
	u16 TxLen;
	XI3cPsx_Cmd DAA_Cmd;

	CfgPtr = XI3cPsx_LookupConfig(DeviceId);
	if (NULL == CfgPtr) {
		  return XST_FAILURE;
	}
	XI3cPsx_CfgInitialize(InstancePtr, CfgPtr, CfgPtr->BaseAddress);

	XI3cPsx_ResetFifos(InstancePtr);
	CmdInfo.RxLen = 1;
	CmdInfo.RxBuff = RxData;
	CmdInfo.SlaveAddr = 0;
	CmdInfo.Cmd = I3C_CCC_GETDCR;
	Status = XI3cPsx_SendTransferCmd(InstancePtr, &CmdInfo);

	DAA_Cmd.TransCmd = COMMAND_PORT_SDA_DATA_BYTE_1(I3C_WHO_AM_I) |
		COMMAND_PORT_SDA_BYTE_STRB_1 |
			COMMAND_PORT_SHORT_DATA_ARG;
	DAA_Cmd.TransArg = (COMMAND_PORT_SPEED(0) |
		COMMAND_PORT_DEV_INDEX(0) |
			COMMAND_PORT_SDAP);
	DAA_Cmd.RxBuf = NULL;
	XI3cPsx_ResetFifos(InstancePtr);
	Status = XI3cPsx_MasterSendPolled(InstancePtr, NULL, 0, DAA_Cmd);

	RxLen = 1;
	DAA_Cmd.RxBuf = RxData;
	DAA_Cmd.TransCmd = COMMAND_PORT_ARG_DATA_LEN(RxLen) | COMMAND_PORT_TRANSFER_ARG;
	DAA_Cmd.TransArg =	(COMMAND_PORT_READ_TRANSFER |
		COMMAND_PORT_SPEED(0) |
		COMMAND_PORT_DEV_INDEX(0) |
			COMMAND_PORT_TID(1) |
			COMMAND_PORT_ROC |
			COMMAND_PORT_TOC);
	Status = XI3cPsx_MasterRecvPolled(InstancePtr, RxData, RxLen, &DAA_Cmd);

	xil_printf("Data at 0x0F is %d\n", RxData[0]);

	DAA_Cmd.TransCmd = COMMAND_PORT_SDA_DATA_BYTE_1(I3C_CTRL) |
		COMMAND_PORT_SDA_BYTE_STRB_1 | COMMAND_PORT_SDA_BYTE_STRB_2 | COMMAND_PORT_SDA_DATA_BYTE_2(2) |
			COMMAND_PORT_SHORT_DATA_ARG;
	DAA_Cmd.TransArg = (COMMAND_PORT_SPEED(0) |
		COMMAND_PORT_DEV_INDEX(0) |
			COMMAND_PORT_SDAP);
	DAA_Cmd.RxBuf = NULL;
	XI3cPsx_ResetFifos(InstancePtr);
	Status = XI3cPsx_MasterSendPolled(InstancePtr, NULL, 0, DAA_Cmd);

	DAA_Cmd.TransCmd = COMMAND_PORT_SDA_DATA_BYTE_1(I3C_CTRL) |
		COMMAND_PORT_SDA_BYTE_STRB_1 |
			COMMAND_PORT_SHORT_DATA_ARG;
	DAA_Cmd.TransArg = (COMMAND_PORT_SPEED(0) |
		COMMAND_PORT_DEV_INDEX(0) |
			COMMAND_PORT_SDAP);
	DAA_Cmd.RxBuf = NULL;
	XI3cPsx_ResetFifos(InstancePtr);
	Status = XI3cPsx_MasterSendPolled(InstancePtr, NULL, 0, DAA_Cmd);

	RxLen = 1;
	DAA_Cmd.RxBuf = RxData;
	DAA_Cmd.TransCmd = COMMAND_PORT_ARG_DATA_LEN(RxLen) | COMMAND_PORT_TRANSFER_ARG;
	DAA_Cmd.TransArg =	(COMMAND_PORT_READ_TRANSFER |
		COMMAND_PORT_SPEED(0) |
		COMMAND_PORT_DEV_INDEX(0) |
			COMMAND_PORT_TID(1) |
			COMMAND_PORT_ROC |
			COMMAND_PORT_TOC);
	Status = XI3cPsx_MasterRecvPolled(InstancePtr, RxData, RxLen, &DAA_Cmd);
	xil_printf("Data at 0x11 is  %d \n", RxData[0]);

	DAA_Cmd.TransCmd = COMMAND_PORT_SDA_DATA_BYTE_1(I3C_INT_SRC) |
		COMMAND_PORT_SDA_BYTE_STRB_1 |
			COMMAND_PORT_SHORT_DATA_ARG;
	DAA_Cmd.TransArg = (COMMAND_PORT_SPEED(0) |
		COMMAND_PORT_DEV_INDEX(0) |
			COMMAND_PORT_SDAP);
	XI3cPsx_ResetFifos(InstancePtr);
	Status = XI3cPsx_MasterSendPolled(InstancePtr, NULL, 0, DAA_Cmd);
	RxLen = I3C_DATALEN;
	DAA_Cmd.RxBuf = RxData;
	DAA_Cmd.TransCmd = COMMAND_PORT_ARG_DATA_LEN(RxLen) | COMMAND_PORT_TRANSFER_ARG;
	DAA_Cmd.TransArg =	(COMMAND_PORT_READ_TRANSFER |
		COMMAND_PORT_SPEED(0) |
		COMMAND_PORT_DEV_INDEX(0) |
			COMMAND_PORT_TID(1) |
			COMMAND_PORT_ROC |
			COMMAND_PORT_TOC);
	Status = XI3cPsx_MasterRecvPolled(InstancePtr, RxData, I3C_DATALEN, &DAA_Cmd);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}
