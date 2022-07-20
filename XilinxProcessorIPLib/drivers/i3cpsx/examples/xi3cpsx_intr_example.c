
/******************************************************************************
* Copyright (C) 2022 AMD, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
 * @file xi3cpsx_intr_example.c
 *
 * Design example to use the I3C device as master in interrupt-driven mode.
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
#include "xscugic.h"
#include "xil_exception.h"

XI3cPsx Xi3cPsx_Instance;
XScuGic InterruptController;	/* Instance of the Interrupt Controller */
XI3cPsx *InstancePtr = &Xi3cPsx_Instance;

static int SetupInterruptSystem(XI3cPsx *InstancePtr);

#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define I3C_INT_VEC_ID		XPAR_PSVXL_I3C_0_INTR
#define I3C_DEVICE_ID		XPAR_XI3CPSX_0_DEVICE_ID
#define I3C_WHO_AM_I		0x0F
#define I3C_CTRL		0x11
#define I3C_INT_SRC		0x1A
#define I3C_DATALEN		10

/************************** Function Prototypes *******************************/

int I3cPsxMasterIntrExample(u16 DeviceId);
/******************************************************************************/
/**
*
* Main function to call the interrupt example.
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

	xil_printf("I3C Master Interrupt Example Test \r\n");

	/*
	 * Run the I3c interrupt example in master mode, specify the Device
	 * ID that is specified in xparameters.h.
	 */
	Status = I3cPsxMasterIntrExample(I3C_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("I3C Master Interrupt Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran I3C Master Interrupt Example Test\r\n");
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
* This function uses interrupt driver mode of the I3C.
*
* @param	DeviceId is the Device ID of the I3cPsx Device and is the
*		XPAR_<I3C_instance>_DEVICE_ID value from xparameters.h
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note
*
*******************************************************************************/
int I3cPsxMasterIntrExample(u16 DeviceId)
{
	int Status;
	XI3cPsx_Config *CfgPtr;
	u8 RxData[I3C_DATALEN];
	u8 TxData[3] = { 0x0F, 0x0F, 0x0F};
	struct CmdInfo CmdInfo;
	u16 RxLen;
	u16 TxLen;
	XI3cPsx_Cmd DAA_Cmd;

	CfgPtr = XI3cPsx_LookupConfig(DeviceId);
	if (NULL == CfgPtr) {
		  return XST_FAILURE;
	}

	Status = XI3cPsx_CfgInitialize(InstancePtr, CfgPtr, CfgPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XI3cPsx_ResetFifos(InstancePtr);
	CmdInfo.RxLen = 1;
	CmdInfo.RxBuff = RxData;
	CmdInfo.SlaveAddr = 0;
	CmdInfo.Cmd = I3C_CCC_GETDCR;
	Status =	XI3cPsx_SendTransferCmd(InstancePtr, &CmdInfo);

	Status = SetupInterruptSystem(InstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	DAA_Cmd.TransCmd = COMMAND_PORT_SDA_DATA_BYTE_1(I3C_WHO_AM_I) |
			COMMAND_PORT_SDA_BYTE_STRB_1 |
			COMMAND_PORT_SHORT_DATA_ARG;
	DAA_Cmd.TransArg = (COMMAND_PORT_SPEED(0) |
			COMMAND_PORT_DEV_INDEX(0) |
			COMMAND_PORT_SDAP);
	DAA_Cmd.RxBuf = NULL;
	XI3cPsx_ResetFifos(InstancePtr);
	Status = XI3cPsx_MasterSend(InstancePtr, TxData, 1, DAA_Cmd);

	RxLen = 1;
	DAA_Cmd.RxBuf = RxData;
	DAA_Cmd.TransCmd = COMMAND_PORT_ARG_DATA_LEN(RxLen) | COMMAND_PORT_TRANSFER_ARG;
	DAA_Cmd.TransArg =	(COMMAND_PORT_READ_TRANSFER |
			COMMAND_PORT_SPEED(0) |
			COMMAND_PORT_DEV_INDEX(0) |
			COMMAND_PORT_TID(1) |
			COMMAND_PORT_ROC |
			COMMAND_PORT_TOC);
	Status = XI3cPsx_MasterRecv(InstancePtr, RxData, RxLen, &DAA_Cmd);
	xil_printf("Data at 0x0F is  %d\n", InstancePtr->RecvBufferPtr[0]);

	DAA_Cmd.TransCmd = COMMAND_PORT_SDA_DATA_BYTE_1(I3C_CTRL) |
		COMMAND_PORT_SDA_BYTE_STRB_1 | COMMAND_PORT_SDA_BYTE_STRB_2 | COMMAND_PORT_SDA_DATA_BYTE_2(2) |
			COMMAND_PORT_SHORT_DATA_ARG;
	DAA_Cmd.TransArg = (COMMAND_PORT_SPEED(0) |
			COMMAND_PORT_DEV_INDEX(0) | COMMAND_PORT_SDAP);
	DAA_Cmd.RxBuf = NULL;
	XI3cPsx_ResetFifos(InstancePtr);
	Status = XI3cPsx_MasterSend(InstancePtr, NULL, 0, DAA_Cmd);


	DAA_Cmd.TransCmd = COMMAND_PORT_SDA_DATA_BYTE_1(I3C_CTRL) |
		COMMAND_PORT_SDA_BYTE_STRB_1 |
			COMMAND_PORT_SHORT_DATA_ARG;
	DAA_Cmd.TransArg = (COMMAND_PORT_SPEED(0) |
		COMMAND_PORT_DEV_INDEX(0) |
			COMMAND_PORT_SDAP);
	DAA_Cmd.RxBuf = NULL;
	XI3cPsx_ResetFifos(InstancePtr);
	Status = XI3cPsx_MasterSend(InstancePtr, NULL, 0, DAA_Cmd);

	RxLen = 1;
	DAA_Cmd.RxBuf = RxData;
	DAA_Cmd.TransCmd = COMMAND_PORT_ARG_DATA_LEN(RxLen) | COMMAND_PORT_TRANSFER_ARG;
	DAA_Cmd.TransArg =	(COMMAND_PORT_READ_TRANSFER |
		COMMAND_PORT_SPEED(0) |
		COMMAND_PORT_DEV_INDEX(0) |
			COMMAND_PORT_TID(1) |
			COMMAND_PORT_ROC |
			COMMAND_PORT_TOC);
	Status = XI3cPsx_MasterRecv(InstancePtr, RxData, RxLen, &DAA_Cmd);
	xil_printf("Data at 0x11 is  %d %d\n", InstancePtr->RecvBufferPtr[0], InstancePtr->RecvBufferPtr[1]);

	DAA_Cmd.TransCmd = COMMAND_PORT_SDA_DATA_BYTE_1(I3C_INT_SRC) |
		COMMAND_PORT_SDA_BYTE_STRB_1 |
			COMMAND_PORT_SHORT_DATA_ARG;
	DAA_Cmd.TransArg = (COMMAND_PORT_SPEED(0) |
		COMMAND_PORT_DEV_INDEX(0) |
			COMMAND_PORT_SDAP);
	XI3cPsx_ResetFifos(InstancePtr);
	Status = XI3cPsx_MasterSend(InstancePtr, NULL, 0, DAA_Cmd);
	RxLen = I3C_DATALEN;
	DAA_Cmd.RxBuf = RxData;
	DAA_Cmd.TransCmd = COMMAND_PORT_ARG_DATA_LEN(RxLen) | COMMAND_PORT_TRANSFER_ARG;
	DAA_Cmd.TransArg =	(COMMAND_PORT_READ_TRANSFER |
		COMMAND_PORT_SPEED(0) |
		COMMAND_PORT_DEV_INDEX(0) |
			COMMAND_PORT_TID(1) |
			COMMAND_PORT_ROC |
			COMMAND_PORT_TOC);
	Status = XI3cPsx_MasterRecv(InstancePtr, RxData, I3C_DATALEN, &DAA_Cmd);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	return XST_SUCCESS;
}
/******************************************************************************/
/**
*
* This function setups the interrupt system such that interrupts can occur
* for the I3C.  This function is application specific since the actual
* system may or may not have an interrupt controller.  The I3C could be
* directly connected to a processor without an interrupt controller.  The
* user should modify this function to fit the application.
*
* @param	IicPsPtr contains a pointer to the instance of the Iic
*		which is going to be connected to the interrupt controller.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
*******************************************************************************/
static int SetupInterruptSystem(XI3cPsx *InstancePtr)
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
	Status = XScuGic_Connect(&InterruptController, I3C_INT_VEC_ID,
			(Xil_InterruptHandler)XI3cPsx_MasterInterruptHandler,
			(void *)InstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Enable the interrupt for the Iic device.
	 */
	XScuGic_Enable(&InterruptController, I3C_INT_VEC_ID);


	/*
	 * Enable interrupts in the Processor.
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}
