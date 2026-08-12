/******************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xi3cpsx_slave_intr_loopback.c
*
* This file consists of a slave mode design example which uses the Xilinx
* I3C device in slave interrupt mode in a loopback setup.
*
* The master sends data to the slave. The slave receives
* the data using interrupt-driven mode. The received data is then verified
* against the transmitted data. Slave also sends the data to master.
*
* This code assumes that no Operating System is being used.
*
* @note
*
* It's a reference example and the data size should be less than or equal
* to FIFO size.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00a sd  06/10/26 First release
* 1.8   vn  07/30/26 Fix Doxygen warnings
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/

#include "xil_printf.h"
#include "xi3cpsx.h"
#include "xi3cpsx_hw.h"
#include "xi3cpsx_pr.h"
#include "xscugic.h"
#include "xil_exception.h"
#ifdef SDT
#include "xinterrupt_wrap.h"
#endif

/************************** Constant Definitions *****************************/

#ifndef SDT
#define I3C_DEVICE_ID		XPAR_XI3CPSX_1_DEVICE_ID
#else
#define I3C_DEVICE_ID		XPAR_XI3CPSX_1_BASEADDR
#endif

#ifndef SDT
#define I3C_SLAVE_ID		XPAR_XI3CPSX_0_DEVICE_ID
#else
#define I3C_SLAVE_ID		XPAR_XI3CPSX_0_BASEADDR
#endif

#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define I3C_SLAVE_INT_VEC_ID	XPAR_PSVXL_I3C_0_INTR
#define I3C_MASTER_INT_VEC_ID	XPAR_PSVXL_I3C_1_INTR

#define LEN			128
#define I3C_SLAVE_STATIC_ADDR	0x5d
#define TIMEOUT_COUNTER		10000000U

/************************** Function Prototypes *******************************/

#ifndef SDT
int I3cPsxSlaveIntrLoopbackExample(u16 DeviceId);
static int SetupInterruptSystem(XI3cPsx *InstancePtr);
static int SetupMasterInterruptSystem(XI3cPsx *InstancePtr);
#else
int I3cPsxSlaveIntrLoopbackExample(UINTPTR BaseAddress);
#endif

void XI3cPsx_SlaveStatusHandler(void *CallBackRef, u32 Event);

/************************** Variable Definitions ******************************/

u8 RxData[LEN];
u8 TxDataTest[LEN];

XI3cPsx Xi3cPs_InstanceMaster;
XI3cPsx *InstancePtr = &Xi3cPs_InstanceMaster;
XI3cPsx Xi3cPs_InstanceSlave;
XI3cPsx *InstancePtrSlave = &Xi3cPs_InstanceSlave;

#ifndef SDT
XScuGic InterruptController;
#endif

volatile u32 TransferComplete;
volatile u32 TotalErrorCount;

/******************************************************************************/
/**
*
* Main function to call the slave interrupt loopback example.
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

	xil_printf("I3C Slave Interrupt Loopback Example Test \r\n");

	Status = I3cPsxSlaveIntrLoopbackExample(I3C_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("I3C Slave Interrupt Loopback Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran I3C Slave Interrupt Loopback Test\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function does a slave interrupt loopback test on the I3c device and
* driver as a design example. The purpose of this function is to illustrate
* how to use the XI3cPsx driver in slave interrupt mode.
*
* This function sends data from the master and receives it on the slave
* through the I3C loopback. It expects the controllers to be in loopback.
*
* This function uses interrupt-driven slave mode and polled master mode
* of the I3C.
*
* @if SDT
* @param	BaseAddress contains the base address of the device
* @else
* @param	DeviceId is the Device ID of the I3cPsx Device and is the
*		XPAR_<I3C_instance>_DEVICE_ID value from xparameters.h
* @endif
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note	        In XSCT/classic flow, DeviceId is used to look up the device
*               configuration.
*
*******************************************************************************/
#ifndef SDT
int I3cPsxSlaveIntrLoopbackExample(u16 DeviceId)
#else
int I3cPsxSlaveIntrLoopbackExample(UINTPTR BaseAddress)
#endif
{
	s32 Ret;
	XI3cPsx_Config *CfgPtr;
	XI3cPsx_Cmd Cmd;
	u8 Index;
	int Status;
	u16 RxLen;
	u32 Timeout;

	/*
	 * Slave configuration
	 */
	CfgPtr = XI3cPsx_LookupConfig(I3C_SLAVE_ID);
	if (NULL == CfgPtr) {
		return XST_FAILURE;
	}

	Status = XI3cPsx_CfgInitialize(InstancePtrSlave, CfgPtr, CfgPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XI3cPsx_SetupSlave(InstancePtrSlave, I3C_SLAVE_STATIC_ADDR);

#ifndef SDT
	Status = SetupInterruptSystem(InstancePtrSlave);
#else
	Status = XSetupInterruptSystem(InstancePtrSlave, XI3cPsx_SlaveInterruptHandler,
				       CfgPtr->IntrId,
				       CfgPtr->IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XI3cPsx_SetStatusHandler(InstancePtrSlave, (void *)InstancePtrSlave, XI3cPsx_SlaveStatusHandler);

	/*
	 * Master configuration
	 */
#ifndef SDT
	CfgPtr = XI3cPsx_LookupConfig(DeviceId);
#else
	CfgPtr = XI3cPsx_LookupConfig(BaseAddress);
#endif
	if (NULL == CfgPtr) {
		return XST_FAILURE;
	}

	Status = XI3cPsx_CfgInitialize(InstancePtr, CfgPtr, CfgPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	for (Index = 0; Index < LEN; Index++) {
		TxDataTest[Index] = Index;
		RxData[Index] = 0;
	}

	/*
	 * Initiate slave receive in interrupt mode before master sends
	 */
	TransferComplete = FALSE;
	TotalErrorCount = 0;
	XI3cPsx_SlaveRecv(InstancePtrSlave, RxData, LEN);

	/*
	 * Transfer Argument
	 */
	Cmd.TransArg = COMMAND_PORT_ARG_DATA_LEN(LEN) | COMMAND_PORT_TRANSFER_ARG;

	/*
	 * Transfer Command
	 */
	Cmd.TransCmd = (COMMAND_PORT_SPEED(0) |
			COMMAND_PORT_DEV_INDEX(0) |
			COMMAND_PORT_TID(3) |
			COMMAND_PORT_TOC |
			COMMAND_PORT_ROC);
	Cmd.RxBuf = NULL;
	Ret = XI3cPsx_MasterSendPolled(InstancePtr, (u8 *)TxDataTest, LEN, Cmd);
	if (Ret != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait for slave receive to complete via interrupt
	 */
	Timeout = TIMEOUT_COUNTER;
	while (!TransferComplete) {
		if (0 != TotalErrorCount) {
			return XST_FAILURE;
		}
		if (0 == --Timeout) {
			xil_printf("Slave receive timed out\r\n");
			return XST_FAILURE;
		}
	}

	for (Index = 0; Index < LEN; Index++) {
		if (RxData[Index] != TxDataTest[Index]) {
			xil_printf("Data mismatch at index 0x%x \r\n", Index);
			xil_printf("Expected 0x%x got 0x%x \r\n", TxDataTest[Index], RxData[Index]);
			return XST_FAILURE;
		}
	}
#ifndef SDT
	Status = SetupMasterInterruptSystem(InstancePtr);
#else
	Status = XSetupInterruptSystem(InstancePtr, XI3cPsx_MasterInterruptHandler,
				       CfgPtr->IntrId,
				       CfgPtr->IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	XI3cPsx_SetStatusHandler(InstancePtr, (void *)InstancePtr, XI3cPsx_SlaveStatusHandler);

	TransferComplete = FALSE;
	TotalErrorCount = 0;

	XI3cPsx_SlaveSend(InstancePtrSlave, (u8 *)TxDataTest, LEN);

	RxLen = LEN;
	Cmd.TransArg = COMMAND_PORT_ARG_DATA_LEN(RxLen) | COMMAND_PORT_TRANSFER_ARG;
	Cmd.TransCmd =	(COMMAND_PORT_READ_TRANSFER |
			 COMMAND_PORT_SPEED(0) |
			 COMMAND_PORT_DEV_INDEX(0) |
			 COMMAND_PORT_TID(1) |
			 COMMAND_PORT_ROC |
			 COMMAND_PORT_TOC);
	Status = XI3cPsx_MasterRecv(InstancePtr, RxData, RxLen, &Cmd);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Timeout = TIMEOUT_COUNTER;

	while (!TransferComplete) {
		if (0 != TotalErrorCount) {
			return XST_FAILURE;
		}

		if (0 == --Timeout) {
			xil_printf("Master receive timed out\r\n");
			return XST_FAILURE;
		}
	}

	for (Index = 0; Index < LEN; Index++) {
		if (RxData[Index] != TxDataTest[Index]) {
			xil_printf("Data mismatch at index 0x%x \r\n", Index);
			xil_printf("Expected 0x%x got 0x%x \r\n", TxDataTest[Index], RxData[Index]);
			return XST_FAILURE;
		}
	}
	return XST_SUCCESS;
}

#ifndef SDT
/******************************************************************************/
/**
*
* This function setups the interrupt system such that interrupts can occur
* for the I3C. This function is application specific since the actual
* system may or may not have an interrupt controller.
*
* @param	InstancePtr contains a pointer to the instance of the I3C
*		device which is going to be connected to the interrupt
*		controller.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
*******************************************************************************/
static int SetupInterruptSystem(XI3cPsx *InstancePtr)
{
	int Status;
	XScuGic_Config *IntcConfig;

	Xil_ExceptionInit();

	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(&InterruptController, IntcConfig,
					IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
				     (Xil_ExceptionHandler)XScuGic_InterruptHandler,
				     &InterruptController);

	Status = XScuGic_Connect(&InterruptController, I3C_SLAVE_INT_VEC_ID,
				 (Xil_InterruptHandler)XI3cPsx_SlaveInterruptHandler,
				 (void *)InstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	XScuGic_Enable(&InterruptController, I3C_SLAVE_INT_VEC_ID);

	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

static int SetupMasterInterruptSystem(XI3cPsx *InstancePtr)
{
	int Status;

	Status = XScuGic_Connect(&InterruptController, I3C_MASTER_INT_VEC_ID,
				 (Xil_InterruptHandler)XI3cPsx_MasterInterruptHandler,
				 (void *)InstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	XScuGic_Enable(&InterruptController, I3C_MASTER_INT_VEC_ID);

	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* This function is the handler which updates transfer status upon events
* from the I3cPsx. It is called from an interrupt context such that the
* amount of processing performed should be minimized.
*
* @param	CallBackRef is a pointer to the callback reference.
* @param	Event is the status event from the I3cPsx.
*
* @note		None.
*
*******************************************************************************/
void XI3cPsx_SlaveStatusHandler(void *CallBackRef, u32 Event)
{
	(void)CallBackRef;

	if (Event != 0)
		TotalErrorCount++;

	TransferComplete = TRUE;
}
