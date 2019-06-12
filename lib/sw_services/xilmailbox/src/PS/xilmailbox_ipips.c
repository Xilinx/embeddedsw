/******************************************************************************
 * Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
 *
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file xilmailbox_ipips.c
 * @addtogroup xilmailbox_v1_0
 * @{
 * @details
 *
 * This file contains the definitions for ZynqMP and versal IPI implementation.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   adk  14/02/19    Initial Release
 *</pre>
 *
 *@note
 *****************************************************************************/
/***************************** Include Files *********************************/
#include "xilmailbox.h"
#include "xscugic.h"
#include "sleep.h"

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
static u32 XIpiPs_Init(XMailbox *InstancePtr, u8 DeviceId);
static u32 XIpiPs_Send(XMailbox *InstancePtr, u8 Is_Blocking);
static u32 XIpiPs_SendData(XMailbox *InstancePtr, void *MsgBufferPtr,
			   u32 MsgLen, u8 BufferType, u8 Is_Blocking);
static u32 XIpiPs_PollforDone(XMailbox *InstancePtr);
static u32 XIpiPs_RecvData(XMailbox *InstancePtr, void *MsgBufferPtr,
			   u32 MsgLen, u8 BufferType);
static XStatus XIpiPs_RegisterIrq(XScuGic *IntcInstancePtr,
				  XMailbox *InstancePtr,
				  u32 IpiIntrId);
static void XIpiPs_ErrorIntrHandler(void *XMailboxPtr);
static void XIpiPs_IntrHandler(void *XMailboxPtr);

/****************************************************************************/
/**
 * Initialize the XMailbox Instance
 *
 * @param	InstancePtr is a pointer to the instance to be worked on
 * @param	DeviceId is the IPI Instance to be worked on
 *
 * @return	XST_SUCCESS if initialization was successful
 * 		XST_FAILURE in case of failure
 */
/****************************************************************************/
u32 XMailbox_Initialize(XMailbox *InstancePtr, u8 DeviceId)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	memset(InstancePtr, 0, sizeof(XMailbox));

	InstancePtr->XMbox_IPI_SendData = XIpiPs_SendData;
	InstancePtr->XMbox_IPI_Send = XIpiPs_Send;
	InstancePtr->XMbox_IPI_Recv = XIpiPs_RecvData;

	Status = XIpiPs_Init(InstancePtr, DeviceId);
	return Status;
}

/****************************************************************************/
/**
 * Initialize the ZynqMP Mailbox Instance
 *
 * @param	InstancePtr is a pointer to the instance to be worked on
 * @param	DeviceId is the IPI Instance to be worked on
 *
 * @return	XST_SUCCESS if initialization was successful
 * 		XST_FAILURE in case of failure
 */
/****************************************************************************/
static u32 XIpiPs_Init(XMailbox *InstancePtr, u8 DeviceId)
{
	u32 Status = XST_FAILURE;
	XIpiPsu_Config *CfgPtr;
	XMailbox_Agent *DataPtr = &InstancePtr->Agent;
	XIpiPsu *IpiInstancePtr = &DataPtr->IpiInst;

	CfgPtr = XIpiPsu_LookupConfig(DeviceId);
	if (NULL == CfgPtr) {
		return Status;
	}

	Status = XIpiPsu_CfgInitialize(IpiInstancePtr, CfgPtr, CfgPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/* Enable reception of IPI from all CPUs */
	XIpiPsu_InterruptEnable(IpiInstancePtr, XIPIPSU_ALL_MASK);

	/* Clear Any existing Interrupts */
	XIpiPsu_ClearInterruptStatus(IpiInstancePtr, XIPIPSU_ALL_MASK);

	/* Register IRQ */
	Status = XIpiPs_RegisterIrq(&DataPtr->GicInst, InstancePtr,
				    CfgPtr->IntId);

	return Status;
}

/*****************************************************************************/
/**
 * This function triggers an IPI to a destnation CPU
 *
 * @param InstancePtr Pointer to the XMailbox instance.
 * @param Is_Blocking if set trigger the notification in blocking mode
 *
 * @return	XST_SUCCESS in case of success
 * 		XST_FAILURE in case of failure
 */
/****************************************************************************/
static u32 XIpiPs_Send(XMailbox *InstancePtr, u8 Is_Blocking)
{
	XMailbox_Agent *DataPtr = &InstancePtr->Agent;
	XIpiPsu *IpiInstancePtr = &DataPtr->IpiInst;
	u32 Status = XST_SUCCESS;

	XIpiPsu_TriggerIpi(IpiInstancePtr, DataPtr->RemoteId);
	if (Is_Blocking != 0U) {
		Status =  XIpiPs_PollforDone(InstancePtr);
	}

	return Status;
}

/*****************************************************************************/
/**
 * This function sends an IPI message to a destnation CPU
 *
 * @param InstancePtr Pointer to the XMailbox instance
 * @param MsgBufferPtr is the pointer to Buffer which contains the message to
 *	  be sent
 * @param MsgLen is the length of the buffer/message
 * @param BufferType is the type of buffer
 * @param Is_Blocking if set trigger the notification in blocking mode
 *
 * @return	XST_SUCCESS in case of success
 * 		XST_FAILURE in case of failure
 */
/****************************************************************************/
static u32 XIpiPs_SendData(XMailbox *InstancePtr, void *MsgBufferPtr,
			   u32 MsgLen, u8 BufferType, u8 Is_Blocking)
{
	XMailbox_Agent *DataPtr = &InstancePtr->Agent;
	XIpiPsu *IpiInstancePtr = &DataPtr->IpiInst;
	u32 Status = XST_SUCCESS;

	XIpiPsu_WriteMessage(IpiInstancePtr, DataPtr->RemoteId,
			     MsgBufferPtr, MsgLen, BufferType);
	XIpiPsu_TriggerIpi(IpiInstancePtr, DataPtr->RemoteId);
	if (Is_Blocking != 0U) {
		Status =  XIpiPs_PollforDone(InstancePtr);
	}

	return Status;
}

/*****************************************************************************/
/**
 * Poll for an acknowledgement using Observation Register.
 *
 * @param InstancePtr Pointer to the XMailbox instance
 *
 * @return	XST_SUCCESS in case of success
 * 		XST_FAILURE in case of failure
 */
/****************************************************************************/
static u32 XIpiPs_PollforDone(XMailbox *InstancePtr)
{
	XMailbox_Agent *DataPtr = &InstancePtr->Agent;
	XIpiPsu *IpiInstancePtr = &DataPtr->IpiInst;
	u32 Timeout = XIPI_DONE_TIMEOUT_VAL;
	u32 Status = XST_SUCCESS;
	u32 Flag;

	do {
		Flag = (XIpiPsu_ReadReg(IpiInstancePtr->Config.BaseAddress,
				XIPIPSU_OBS_OFFSET)) & (DataPtr->RemoteId);
		if (Flag == 0U) {
			break;
		}
		usleep(100);
		Timeout--;
	} while (Timeout != 0U);

	if (Timeout == 0U) {
		Status = XST_FAILURE;
	}

	return Status;
}

/*****************************************************************************/
/**
 * This function reads an IPI message
 *
 * @param InstancePtr Pointer to the XMailbox instance
 * @param MsgBufferPtr is the pointer to Buffer to which the read message needs
 *	  to be stored
 * @param MsgLen is the length of the buffer/message
 * @param BufferType is the type of buffer
 *
 * @return
 *	- XST_SUCCESS if successful
 *	- XST_FAILURE if unsuccessful
 *
 ****************************************************************************/
static u32 XIpiPs_RecvData(XMailbox *InstancePtr, void *MsgBufferPtr,
			   u32 MsgLen, u8 BufferType)
{
	u32 Status;
	XMailbox_Agent *DataPtr = &InstancePtr->Agent;
	XIpiPsu *IpiInstancePtr = &DataPtr->IpiInst;

	Status = XIpiPsu_ReadMessage(IpiInstancePtr, DataPtr->SourceId,
				     MsgBufferPtr, MsgLen, BufferType);
	return Status;
}

static XStatus XIpiPs_RegisterIrq(XScuGic *IntcInstancePtr,
				  XMailbox *InstancePtr,
				  u32 IpiIntrId) {
	u32 Status;
	XScuGic_Config *IntcConfigPtr;

	/* Initialize the interrupt controller driver */
	IntcConfigPtr = XScuGic_LookupConfig(XPAR_SCUGIC_0_DEVICE_ID);
	if (NULL == IntcConfigPtr) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfigPtr,
				       IntcConfigPtr->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the interrupt controller interrupt handler to the
	 * hardware interrupt handling logic in the processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			(Xil_ExceptionHandler)XScuGic_InterruptHandler,
				     IntcInstancePtr);

	Status = XScuGic_Connect(IntcInstancePtr, IpiIntrId,
				 (Xil_InterruptHandler) XIpiPs_IntrHandler,
				 (void *)InstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;

	}

	Status = XScuGic_Connect(IntcInstancePtr, XMAILBOX_INTR_ID,
				 (Xil_InterruptHandler) XIpiPs_ErrorIntrHandler,
				 (void *)InstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/* Enable the interrupt for the device */
	XScuGic_Enable(IntcInstancePtr, IpiIntrId);
	XScuGic_Enable(IntcInstancePtr, XMAILBOX_INTR_ID);

	/* Enable interrupts */
	Xil_ExceptionEnable();

	return Status;
}

static void XIpiPs_IntrHandler(void *XMailboxPtr)
{
	XMailbox *InstancePtr = (XMailbox *)((void *)XMailboxPtr);
	XMailbox_Agent *DataPtr = &InstancePtr->Agent;
	XIpiPsu *IpiInstancePtr = &DataPtr->IpiInst;
	u32 IntrStatus;

	IntrStatus = XIpiPsu_GetInterruptStatus(IpiInstancePtr);
	XIpiPsu_ClearInterruptStatus(IpiInstancePtr, IntrStatus);
	if (InstancePtr->RecvHandler != NULL) {
		InstancePtr->RecvHandler(InstancePtr->RecvRefPtr);
	}
}

static void XIpiPs_ErrorIntrHandler(void *XMailboxPtr)
{
	XMailbox *InstancePtr = (XMailbox *)((void *)XMailboxPtr);
	u32 Status;

	Status = XIpiPsu_ReadReg(IPI_BASEADDRESS, XIPIPSU_ISR_OFFSET);
	XIpiPsu_WriteReg(IPI_BASEADDRESS, XIPIPSU_ISR_OFFSET,
			 Status);
	if (InstancePtr->ErrorHandler != NULL) {
		InstancePtr->ErrorHandler(InstancePtr->ErrorRefPtr, Status);
	}
}
