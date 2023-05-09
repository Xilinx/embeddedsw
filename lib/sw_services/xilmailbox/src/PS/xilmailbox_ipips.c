/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilmailbox_ipips.c
 * @addtogroup Overview
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
 * 1.1   sd   16/08/19    Initialise status variable
 * 1.3   sd   03/03/21    Doxygen Fixes
 * 1.4   sd   23/06/21    Fix MISRA-C warnings
 * 1.5   dp   22/11/21    Update XIpiPs_RegisterIrq() to check whether GIC has
 *                        already been setup or not and if it was setup skip
 *                        initializing GIC again and just register handlers.
 * 1.6   sd   28/02/21    Add support for microblaze
 *       kpt  03/16/21    Fixed compilation warning on microblaze
 * 1.7   sd   01/04/22    Replace memset with Xil_SMemSet
 * 1.8   ana  05/02/23	  Updated XIpiPs_PollforDone logic to improve
 *						  AES client performance
 *</pre>
 *
 *@note
 *****************************************************************************/
/***************************** Include Files *********************************/
#include "xilmailbox.h"
#include "xil_util.h"
#ifndef __MICROBLAZE__
#include "xscugic.h"
#endif
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
#ifndef __MICROBLAZE__
static XStatus XIpiPs_RegisterIrq(XScuGic *IntcInstancePtr,
				  XMailbox *InstancePtr,
				  u32 IpiIntrId);
static void XIpiPs_ErrorIntrHandler(void *XMailboxPtr);
static void XIpiPs_IntrHandler(void *XMailboxPtr);
#endif

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
	u32 Status = XST_FAILURE;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	Status = (u32) Xil_SMemSet((void *)InstancePtr, sizeof(XMailbox), 0, sizeof(XMailbox));
	if (Status != XST_SUCCESS) {
		return Status;
	}

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
	s32 Status = (s32)XST_FAILURE;
	XIpiPsu_Config *CfgPtr;
	XMailbox_Agent *DataPtr = &InstancePtr->Agent;
	XIpiPsu *IpiInstancePtr = &DataPtr->IpiInst;

	CfgPtr = XIpiPsu_LookupConfig(DeviceId);
	if (NULL == CfgPtr) {
		return (u32)Status;
	}

	Status = XIpiPsu_CfgInitialize(IpiInstancePtr, CfgPtr, CfgPtr->BaseAddress);
	if (Status != (s32)XST_SUCCESS) {
		return (u32)Status;
	}

	/* Enable reception of IPI from all CPUs */
	XIpiPsu_InterruptEnable(IpiInstancePtr, XIPIPSU_ALL_MASK);

	/* Clear Any existing Interrupts */
	XIpiPsu_ClearInterruptStatus(IpiInstancePtr, XIPIPSU_ALL_MASK);

	/* Register IRQ */
#ifndef __MICROBLAZE__
	Status = XIpiPs_RegisterIrq(&DataPtr->GicInst, InstancePtr,
				    CfgPtr->IntId);
#endif

	return (u32)Status;
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

	Status = (u32)XIpiPsu_TriggerIpi(IpiInstancePtr, DataPtr->RemoteId);
	if (Status != (u32)XST_SUCCESS) {
		return XST_FAILURE;
	}

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

	Status = (u32)XIpiPsu_WriteMessage(IpiInstancePtr, DataPtr->RemoteId,
			     (u32 *)MsgBufferPtr, MsgLen, BufferType);
	if (Status != (u32)XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = (u32)XIpiPsu_TriggerIpi(IpiInstancePtr, DataPtr->RemoteId);
	if (Status != (u32)XST_SUCCESS) {
		return XST_FAILURE;
	}

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
		usleep(XIPI_IPI_DONE_BIT_SLEEP_IN_US);
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
	u32 Status = XST_FAILURE;
	XMailbox_Agent *DataPtr = &InstancePtr->Agent;
	XIpiPsu *IpiInstancePtr = &DataPtr->IpiInst;

	Status = (u32)XIpiPsu_ReadMessage(IpiInstancePtr, DataPtr->SourceId,
				     (u32 *)MsgBufferPtr, MsgLen, BufferType);
	return Status;
}

/*****************************************************************************/
/**
 * This function registers an irq
 *
 * @param IntcInstancePtr Pointer to the scugic instance
 * @param InstancePtr Pointer to the XMailbox instance
 * @param IpiIntrId is the interrupt id of the IPI
 *
 * @return
 *	- XST_SUCCESS if successful
 *	- XST_FAILURE if unsuccessful
 *
 ****************************************************************************/
#ifndef __MICROBLAZE__
static XStatus XIpiPs_RegisterIrq(XScuGic *IntcInstancePtr,
				  XMailbox *InstancePtr,
				  u32 IpiIntrId) {
	s32 Status = (s32)XST_FAILURE;
	XScuGic_Config *IntcConfigPtr;

	/* Initialize the interrupt controller driver */
	IntcConfigPtr = XScuGic_LookupConfig(XPAR_SCUGIC_0_DEVICE_ID);
	if (NULL == IntcConfigPtr) {
		return (s32)XST_FAILURE;
	}

	/* Check if the GIC is already setup by this time */
	if (XScuGic_IsInitialized(XPAR_SCUGIC_0_DEVICE_ID) == 1U) {
		/*
		 * GIC is already initialized, just register handlers using the
		 * interrupt Ids and return success.
		 */
		XScuGic_RegisterHandler(IntcConfigPtr->CpuBaseAddress,
					IpiIntrId,
					(Xil_InterruptHandler)XIpiPs_IntrHandler,
					(void *)InstancePtr);

		XScuGic_RegisterHandler(IntcConfigPtr->CpuBaseAddress,
					XMAILBOX_INTR_ID,
					(Xil_InterruptHandler)XIpiPs_ErrorIntrHandler,
					(void *)InstancePtr);
		/* Enable the interrupt for the device */
		XScuGic_EnableIntr(IntcConfigPtr->DistBaseAddress, IpiIntrId);
		XScuGic_EnableIntr(IntcConfigPtr->DistBaseAddress, XMAILBOX_INTR_ID);

		return (s32)XST_SUCCESS;
	}

	Status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfigPtr,
				       IntcConfigPtr->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return (s32)XST_FAILURE;
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
		return (s32)XST_FAILURE;

	}

	Status = XScuGic_Connect(IntcInstancePtr, XMAILBOX_INTR_ID,
				 (Xil_InterruptHandler) XIpiPs_ErrorIntrHandler,
				 (void *)InstancePtr);
	if (Status != XST_SUCCESS) {
		return (s32)XST_FAILURE;
	}


	/* Enable the interrupt for the device */
	XScuGic_Enable(IntcInstancePtr, IpiIntrId);
	XScuGic_Enable(IntcInstancePtr, XMAILBOX_INTR_ID);

	/* Enable interrupts */
	Xil_ExceptionEnable();

	return Status;
}

/*****************************************************************************/
/**
 * This function implements the interrupt handler
 *
 * @param XMailboxPtr Pointer to the XMailbox instance
 *
 * @return	None
 *
 ****************************************************************************/
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

/*****************************************************************************/
/**
 * This function implements the interrupt handler for errors
 *
 * @param XMailboxPtr Pointer to the XMailbox instance
 *
 * @return	None
 *
 ****************************************************************************/
static void XIpiPs_ErrorIntrHandler(void *XMailboxPtr)
{
	const XMailbox *InstancePtr = (XMailbox *)((void *)XMailboxPtr);
	u32 Status = XST_FAILURE;

	Status = XIpiPsu_ReadReg(IPI_BASEADDRESS, XIPIPSU_ISR_OFFSET);
	XIpiPsu_WriteReg(IPI_BASEADDRESS, XIPIPSU_ISR_OFFSET,
			 Status);
	if (InstancePtr->ErrorHandler != NULL) {
		InstancePtr->ErrorHandler(InstancePtr->ErrorRefPtr, Status);
	}
}
#endif
