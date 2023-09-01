/******************************************************************************
* Copyright (c) 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilmailbox_ipips_control.c
 * @addtogroup xilmailbox Overview
 * @{
 * @details
 *
 * This file contains the options for ZynqMP and versal IPI implementation.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.8   ht   07/24/23    Restructure the code for more modularity.
 *
 *  *</pre>
 *
 *@note
 *****************************************************************************/

/***************************** Include Files *********************************/
#include "xilmailbox_ipips_control.h"

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

/****************************************************************************/
/**
 * Initialize the ZynqMP Mailbox Instance
 *
 * @param       InstancePtr is a pointer to the instance to be worked on
 * @param       DeviceId is the IPI Instance to be worked on
 *
 * @return      XST_SUCCESS if initialization was successful
 *              XST_FAILURE in case of failure
 */
/****************************************************************************/
#ifndef SDT
u32 XIpiPs_Init(XMailbox *InstancePtr, u8 DeviceId)
#else
u32 XIpiPs_Init(XMailbox *InstancePtr, UINTPTR BaseAddress)
#endif
{
	s32 Status = (s32)XST_FAILURE;
	XIpiPsu_Config *CfgPtr;
	XMailbox_Agent *DataPtr = &InstancePtr->Agent;
	XIpiPsu *IpiInstancePtr = &DataPtr->IpiInst;

#ifndef SDT
	CfgPtr = XIpiPsu_LookupConfig(DeviceId);
#else
	CfgPtr = XIpiPsu_LookupConfig(BaseAddress);
#endif

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
 * @return      XST_SUCCESS in case of success
 *              XST_FAILURE in case of failure
 */
/****************************************************************************/
u32 XIpiPs_Send(XMailbox *InstancePtr, u8 Is_Blocking)
{
	XMailbox_Agent *DataPtr = &InstancePtr->Agent;
	XIpiPsu *IpiInstancePtr = &DataPtr->IpiInst;
	u32 Status = XST_SUCCESS;

	/* Trigger IPI to destination CPU */
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
 *        be sent
 * @param MsgLen is the length of the buffer/message
 * @param BufferType is the type of buffer
 * @param Is_Blocking if set trigger the notification in blocking mode
 *
 * @return      XST_SUCCESS in case of success
 *              XST_FAILURE in case of failure
 */
/****************************************************************************/
u32 XIpiPs_SendData(XMailbox *InstancePtr, void *MsgBufferPtr,
		    u32 MsgLen, u8 BufferType, u8 Is_Blocking)
{
	XMailbox_Agent *DataPtr = &InstancePtr->Agent;
	XIpiPsu *IpiInstancePtr = &DataPtr->IpiInst;
	u32 Status = XST_SUCCESS;

	/* Send a Message to Destination */
	Status = (u32)XIpiPsu_WriteMessage(IpiInstancePtr, DataPtr->RemoteId,
					   (u32 *)MsgBufferPtr, MsgLen, BufferType);
	if (Status != (u32)XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Trigger IPI to destination CPU */
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
 * This function reads an IPI message
 *
 * @param InstancePtr Pointer to the XMailbox instance
 * @param MsgBufferPtr is the pointer to Buffer to which the read message needs
 *        to be stored
 * @param MsgLen is the length of the buffer/message
 * @param BufferType is the type of buffer
 *
 * @return
 *      - XST_SUCCESS if successful
 *      - XST_FAILURE if unsuccessful
 *
 ****************************************************************************/
u32 XIpiPs_RecvData(XMailbox *InstancePtr, void *MsgBufferPtr,
		    u32 MsgLen, u8 BufferType)
{
	u32 Status = XST_FAILURE;
	XMailbox_Agent *DataPtr = &InstancePtr->Agent;
	XIpiPsu *IpiInstancePtr = &DataPtr->IpiInst;

	Status = (u32)XIpiPsu_ReadMessage(IpiInstancePtr, DataPtr->SourceId,
					  (u32 *)MsgBufferPtr, MsgLen, BufferType);
	return Status;
}

#ifndef __MICROBLAZE__
/*****************************************************************************/
/**
 * This function implements the interrupt handler
 *
 * @param XMailboxPtr Pointer to the XMailbox instance
 *
 * @return      None
 *
 ****************************************************************************/
void XIpiPs_IntrHandler(void *XMailboxPtr)
{
	XMailbox *InstancePtr = (XMailbox *)((void *)XMailboxPtr);
	XMailbox_Agent *DataPtr = &InstancePtr->Agent;
	XIpiPsu *IpiInstancePtr = &DataPtr->IpiInst;
	u32 IntrStatus;

	/* Get the Status Register of the current IPI instance.*/
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
 * @return      None
 *
 ****************************************************************************/
void XIpiPs_ErrorIntrHandler(void *XMailboxPtr)
{
	const XMailbox *InstancePtr = (XMailbox *)((void *)XMailboxPtr);
	u32 Status = XST_FAILURE;

	/* Get the Status Register of the current IPI instance.*/
	Status = XIpiPsu_ReadReg(IPI_BASEADDRESS, XIPIPSU_ISR_OFFSET);
	XIpiPsu_WriteReg(IPI_BASEADDRESS, XIPIPSU_ISR_OFFSET,
			 Status);
	if (InstancePtr->ErrorHandler != NULL) {
		InstancePtr->ErrorHandler(InstancePtr->ErrorRefPtr, Status);
	}
}
#endif
