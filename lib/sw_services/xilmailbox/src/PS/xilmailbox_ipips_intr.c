/******************************************************************************
* Copyright (c) 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilmailbox_ipips_intr.c
 * @addtogroup xilmailbox Overview
 * @{
 * @details
 *
 * This file contains the definitions for XIpiPs_PollforDone and XIpiPs_RegisterIrq.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.8   ht   07/24/23    Restructure the code for more modularity
 *
 *  *</pre>
 *
 *@note
 *****************************************************************************/

/***************************** Include Files *********************************/
#include "xilmailbox_ipips_control.h"

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
 * Poll for an acknowledgement using Observation Register.
 *
 * @param InstancePtr Pointer to the XMailbox instance
 *
 * @return      XST_SUCCESS in case of success
 *              XST_FAILURE in case of failure
 */
/****************************************************************************/
u32 XIpiPs_PollforDone(XMailbox *InstancePtr)
{
	/* Initialize the agent to store IPI channel information */
	XMailbox_Agent *DataPtr = &InstancePtr->Agent;
	XIpiPsu *IpiInstancePtr = &DataPtr->IpiInst;
	u32 Timeout = XIPI_DONE_TIMEOUT_VAL;
	u32 Status = XST_SUCCESS;
	u32 Flag;

	/* Poll for an acknowledgment using Observation register */
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

#ifndef __MICROBLAZE__
/*****************************************************************************/
/**
 * This function registers an irq
 *
 * @param IntcInstancePtr Pointer to the scugic instance
 * @param InstancePtr Pointer to the XMailbox instance
 * @param IpiIntrId is the interrupt id of the IPI
 *
 * @return
 *      - XST_SUCCESS if successful
 *      - XST_FAILURE if unsuccessful
 *
 ****************************************************************************/
XStatus XIpiPs_RegisterIrq(XScuGic *IntcInstancePtr,
			   XMailbox *InstancePtr,
			   u32 IpiIntrId)
{
#ifndef SDT
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
					(s32)IpiIntrId,
					(Xil_InterruptHandler)XIpiPs_IntrHandler,
					(void *)InstancePtr);

		XScuGic_RegisterHandler(IntcConfigPtr->CpuBaseAddress,
					(s32)XMAILBOX_INTR_ID,
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

#else
	XMailbox_Agent *DataPtr = &InstancePtr->Agent;
	XIpiPsu *IpiInstancePtr = &DataPtr->IpiInst;

	XSetupInterruptSystem(InstancePtr, (Xil_InterruptHandler)
			      XIpiPs_IntrHandler, IpiIntrId, IpiInstancePtr->Config.IntrParent,
			      XINTERRUPT_DEFAULT_PRIORITY);

	XSetupInterruptSystem(InstancePtr, (Xil_InterruptHandler)
			      XIpiPs_IntrHandler, XMAILBOX_INTR_ID, IpiInstancePtr->Config.IntrParent,
			      XINTERRUPT_DEFAULT_PRIORITY);
	return XST_SUCCESS;
#endif
}
#endif
