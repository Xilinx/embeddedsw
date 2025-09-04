/******************************************************************************
* Copyright (c) 2023 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
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
 * 1.9   sd   11/23/23    Clear the interrupts after calling the user handler.
 * 1.11  ht   11/12/24    Update description of MsgLen
 * 1.11  ht   01/02/25    Fix GCC warnings.
 * 1.12  an   08/22/25    Refactor to skip interrupt registration for PMC, PSM
 * 1.12  ht   09/02/25    IPI operation relied on the RemoteId stored in XMailbox
 * 			  Instance, which could lead to race conditions and data
 * 			  corruption when handling multiple requests from
 * 			  different IPI channels concurrently.
 * 			  Use RemoteId based APIs to support concurrent IPI
 * 			  channel requests.
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
#if !defined (VERSAL_PLM) && !defined (VERSAL_psm) && !defined (PSU_PMU)

#ifndef SDT
#if !defined (__MICROBLAZE__) && !defined (__riscv)
	Status = XIpiPs_RegisterIrq(&DataPtr->GicInst, InstancePtr,
				    CfgPtr->IntId);
#endif
#else
	Status = XIpiPs_RegisterIrq(InstancePtr, CfgPtr->IntId);
#endif

#endif
	return (u32)Status;
}

/*****************************************************************************/
/**
 * This function triggers an IPI to a destination CPU
 *
 * @param InstancePtr Pointer to the XMailbox instance.
 * @param Is_Blocking if set trigger the notification in blocking mode
 *
 * @return      XST_SUCCESS in case of success
 *              XST_FAILURE in case of failure
 *
 * @attention This API will be deprecated in 2026.2 and will be made obsolete
 *            in 2027.1 release. The functionality of this API can be reproduced
 *            with XIpiPs_SendById.
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
 * This function triggers an IPI to a destination CPU
 *
 * @param InstancePtr Pointer to the XMailbox instance.
 * @param RemoteId Mask of the CPU to which IPI is to be triggered
 * @param Is_Blocking if set trigger the notification in blocking mode
 *
 * @return      XST_SUCCESS in case of success
 *              XST_FAILURE in case of failure
 */
/****************************************************************************/
u32 XIpiPs_SendById(XMailbox *InstancePtr, u32 RemoteId, u8 Is_Blocking)
{
	XMailbox_Agent *DataPtr = &InstancePtr->Agent;
	XIpiPsu *IpiInstancePtr = &DataPtr->IpiInst;
	u32 Status = XST_SUCCESS;

	/* Trigger IPI to destination CPU */
	Status = (u32)XIpiPsu_TriggerIpi(IpiInstancePtr, RemoteId);
	if (Status != (u32)XST_SUCCESS) {
		return XST_FAILURE;
	}

	if (Is_Blocking != 0U) {
		Status =  XIpiPs_PollforDoneById(InstancePtr, RemoteId);
	}

	return Status;
}

/*****************************************************************************/
/**
 * This function sends an IPI message to a destination CPU
 *
 * @param InstancePtr Pointer to the XMailbox instance
 * @param MsgBufferPtr is the pointer to Buffer which contains the message to
 *        be sent
 * @param MsgLen is the number of messages (each message is 4bytes)
 * @param BufferType is the type of buffer
 * @param Is_Blocking if set trigger the notification in blocking mode
 *
 * @return      XST_SUCCESS in case of success
 *              XST_FAILURE in case of failure
 *
 * @attention This API will be deprecated in 2026.2 and will be made obsolete
 *            in 2027.1 release. The functionality of this API can be reproduced
 *            with XIpiPs_SendDataById.
 */
/****************************************************************************/
u32 XIpiPs_SendData(XMailbox *InstancePtr, void *MsgBufferPtr, u32 MsgLen,
		    u8 BufferType, u8 Is_Blocking)
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
 * This function sends an IPI message to a destination CPU
 *
 * @param InstancePtr Pointer to the XMailbox instance
 * @param MsgBufferPtr is the pointer to Buffer which contains the message to
 *        be sent
 * @param MsgLen is the number of messages (each message is 4bytes)
 * @param BufferType is the type of buffer
 * @param RemoteId Mask of the CPU to which IPI is to be triggered
 * @param Is_Blocking if set trigger the notification in blocking mode
 *
 * @return      XST_SUCCESS in case of success
 *              XST_FAILURE in case of failure
 */
/****************************************************************************/
u32 XIpiPs_SendDataById(XMailbox *InstancePtr, void *MsgBufferPtr, u32 MsgLen,
			u8 BufferType, u32 RemoteId, u8 Is_Blocking)
{
	XMailbox_Agent *DataPtr = &InstancePtr->Agent;
	XIpiPsu *IpiInstancePtr = &DataPtr->IpiInst;
	u32 Status = XST_SUCCESS;

	/* Send a Message to Destination */
	Status = (u32)XIpiPsu_WriteMessage(IpiInstancePtr, RemoteId,
					   (u32 *)MsgBufferPtr, MsgLen, BufferType);
	if (Status != (u32)XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Trigger IPI to destination CPU */
	Status = (u32)XIpiPsu_TriggerIpi(IpiInstancePtr, RemoteId);
	if (Status != (u32)XST_SUCCESS) {
		return XST_FAILURE;
	}

	if (Is_Blocking != 0U) {
		Status =  XIpiPs_PollforDoneById(InstancePtr, RemoteId);
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
 * @param MsgLen is the number of messages (each message is 4bytes)
 * @param BufferType is the type of buffer
 *
 * @return
 *      - XST_SUCCESS if successful
 *      - XST_FAILURE if unsuccessful
 *
 * @attention This API will be deprecated in 2026.2 and will be made obsolete
 *            in 2027.1 release. The functionality of this API can be reproduced
 *            with XIpiPs_RecvDataById.
 *
 ****************************************************************************/
u32 XIpiPs_RecvData(XMailbox *InstancePtr, void *MsgBufferPtr, u32 MsgLen,
		    u8 BufferType)
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
 * This function reads an IPI message
 *
 * @param InstancePtr Pointer to the XMailbox instance
 * @param MsgBufferPtr is the pointer to Buffer to which the read message needs
 *        to be stored
 * @param MsgLen is the number of messages (each message is 4bytes)
 * @param SourceId Device Mask for the CPU which has sent the message
 * @param BufferType is the type of buffer
 *
 * @return
 *      - XST_SUCCESS if successful
 *      - XST_FAILURE if unsuccessful
 *
 ****************************************************************************/
u32 XIpiPs_RecvDataById(XMailbox *InstancePtr, void *MsgBufferPtr, u32 MsgLen,
			u32 SourceId, u8 BufferType)
{
	u32 Status = XST_FAILURE;
	XMailbox_Agent *DataPtr = &InstancePtr->Agent;
	XIpiPsu *IpiInstancePtr = &DataPtr->IpiInst;

	Status = (u32)XIpiPsu_ReadMessage(IpiInstancePtr, SourceId,
					  (u32 *)MsgBufferPtr, MsgLen, BufferType);
	return Status;
}

#if !defined (VERSAL_PLM) && !defined (VERSAL_psm) && !defined (PSU_PMU)
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
	if (InstancePtr->RecvHandler != NULL) {
		InstancePtr->RecvHandler(InstancePtr->RecvRefPtr);
	}
	XIpiPsu_ClearInterruptStatus(IpiInstancePtr, IntrStatus);
}

#if !defined (__MICROBLAZE__) && !defined (__riscv)
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
#endif
