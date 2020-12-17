/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xipipsu.c
* @addtogroup ipipsu_v2_8
* @{
*
* This file contains the implementation of the interface functions for XIpiPsu
* driver. Refer to the header file xipipsu.h for more detailed information.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who	Date	Changes
* ----- ------ -------- ----------------------------------------------
* 1.00	mjr	03/15/15	First Release
* 2.0	mjr	01/22/16	Fixed response buffer address
*                               calculation. CR# 932582.
* 2.1	kvn	05/05/16	Modified code for MISRA-C:2012 Compliance
* 2.2	kvn	02/17/17	Add support for updating ConfigTable at run time
* 2.4	sd	07/11/18	Fix a doxygen reported warning
* 2.6	sd	04/02/20	Restructured the code for more readability and modularity
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/
#include "xipipsu.h"
#include "xipipsu_hw.h"

/************************** Variable Definitions *****************************/
extern XIpiPsu_Config XIpiPsu_ConfigTable[XPAR_XIPIPSU_NUM_INSTANCES];

/****************************************************************************/
/**
 * Initialize the Instance pointer based on a given Config Pointer
 *
 * @param	InstancePtr is a pointer to the instance to be worked on
 * @param	CfgPtr is the device configuration structure containing required
 *		  	hardware build data
 * @param	EffectiveAddress is the base address of the device. If address
 *        	translation is not utilized, this parameter can be passed in using
 *        	CfgPtr->Config.BaseAddress to specify the physical base address.
 * @return	XST_SUCCESS if initialization was successful
 * 			XST_FAILURE in case of failure
 *
 */

XStatus XIpiPsu_CfgInitialize(XIpiPsu *InstancePtr, XIpiPsu_Config * CfgPtr,
		UINTPTR EffectiveAddress)
{
	u32 Index;
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	/* Set device base address and ID */
	InstancePtr->Config.DeviceId = CfgPtr->DeviceId;
	InstancePtr->Config.BaseAddress = EffectiveAddress;
	InstancePtr->Config.BitMask = CfgPtr->BitMask;
	InstancePtr->Config.IntId = CfgPtr->IntId;

	InstancePtr->Config.TargetCount = CfgPtr->TargetCount;

	for (Index = 0U; Index < CfgPtr->TargetCount; Index++) {
		InstancePtr->Config.TargetList[Index].Mask =
				CfgPtr->TargetList[Index].Mask;
		InstancePtr->Config.TargetList[Index].BufferIndex =
				CfgPtr->TargetList[Index].BufferIndex;
	}

	/* Mark the component as Ready */
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;
	return (XST_SUCCESS);
}

/**
 * @brief	Reset the given IPI register set.
 *        	This function can be called to disable the IPIs from all
 *        	the sources and clear any pending IPIs in status register
 *
 * @param 	InstancePtr is the pointer to current IPI instance
 *
 */

void XIpiPsu_Reset(XIpiPsu *InstancePtr)
{

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/**************Disable***************/

	XIpiPsu_WriteReg(InstancePtr->Config.BaseAddress, XIPIPSU_IDR_OFFSET,
			XIPIPSU_ALL_MASK);

	/**************Clear***************/
	XIpiPsu_WriteReg(InstancePtr->Config.BaseAddress, XIPIPSU_ISR_OFFSET,
			XIPIPSU_ALL_MASK);

}

/**
 * @brief	Trigger an IPI to a Destination CPU
 *
 * @param	InstancePtr is the pointer to current IPI instance
 * @param	DestCpuMask is the Mask of the CPU to which IPI is to be triggered
 *
 *
 * @return	XST_SUCCESS if successful
 * 			XST_FAILURE if an error occurred
 */

XStatus XIpiPsu_TriggerIpi(XIpiPsu *InstancePtr, u32 DestCpuMask)
{

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Trigger an IPI to the Target */
	XIpiPsu_WriteReg(InstancePtr->Config.BaseAddress, XIPIPSU_TRIG_OFFSET,
			DestCpuMask);
	return XST_SUCCESS;

}

/**
 * @brief Poll for an acknowledgement using Observation Register
 *
 * @param	InstancePtr is the pointer to current IPI instance
 * @param	DestCpuMask is the Mask of the destination CPU from which ACK is expected
 * @param	TimeOutCount is the Count after which the routines returns failure
 *
 * @return	XST_SUCCESS if successful
 * 			XST_FAILURE if a timeout occurred
 */

XStatus XIpiPsu_PollForAck(XIpiPsu *InstancePtr, u32 DestCpuMask,
		u32 TimeOutCount)
{
	u32 Flag, PollCount;
	XStatus Status;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	PollCount = 0U;
	/* Poll the OBS register until the corresponding DestCpu bit is cleared */
	do {
		Flag = (XIpiPsu_ReadReg(InstancePtr->Config.BaseAddress,
				XIPIPSU_OBS_OFFSET)) & (DestCpuMask);
		PollCount++;
		/* Check if the IPI was Acknowledged by the Target or we Timed Out*/
	} while ((0x00000000U != Flag) && (PollCount < TimeOutCount));

	if (PollCount >= TimeOutCount) {
		Status = XST_FAILURE;
	} else {
		Status = XST_SUCCESS;
	}

	return Status;
}

/**
 * @brief	Read an Incoming Message from a Source
 *
 * @param 	InstancePtr is the pointer to current IPI instance
 * @param 	SrcCpuMask is the Device Mask for the CPU which has sent the message
 * @param 	MsgPtr is the pointer to Buffer to which the read message needs to be stored
 * @param 	MsgLength is the length of the buffer/message
 * @param 	BufferType is the type of buffer (XIPIPSU_BUF_TYPE_MSG or XIPIPSU_BUF_TYPE_RESP)
 *
 * @return	XST_SUCCESS if successful
 * 			XST_FAILURE if an error occurred
 */

XStatus XIpiPsu_ReadMessage(XIpiPsu *InstancePtr, u32 SrcCpuMask, u32 *MsgPtr,
		u32 MsgLength, u8 BufferType)
{
	u32 *BufferPtr;
	u32 Index;
	XStatus Status;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(MsgPtr != NULL);
	Xil_AssertNonvoid(MsgLength <= XIPIPSU_MAX_MSG_LEN);

	BufferPtr = XIpiPsu_GetBufferAddress(InstancePtr, SrcCpuMask,
			InstancePtr->Config.BitMask, BufferType);
	if (BufferPtr != NULL) {
		/* Copy the IPI Buffer contents into Users's Buffer*/
		for (Index = 0U; Index < MsgLength; Index++) {
			MsgPtr[Index] = BufferPtr[Index];
		}
		Status = XST_SUCCESS;
	} else {
		Status = XST_FAILURE;
	}

	return Status;
}


/**
 * @brief	Send a Message to Destination
 *
 * @param	InstancePtr is the pointer to current IPI instance
 * @param	DestCpuMask is the Device Mask for the destination CPU
 * @param	MsgPtr is the pointer to Buffer which contains the message to be sent
 * @param	MsgLength is the length of the buffer/message
 * @param	BufferType is the type of buffer (XIPIPSU_BUF_TYPE_MSG or XIPIPSU_BUF_TYPE_RESP)
 *
 * @return	XST_SUCCESS if successful
 * 			XST_FAILURE if an error occurred
 */

XStatus XIpiPsu_WriteMessage(XIpiPsu *InstancePtr, u32 DestCpuMask, u32 *MsgPtr,
		u32 MsgLength, u8 BufferType)
{
	u32 *BufferPtr;
	u32 Index;
	XStatus Status;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(MsgPtr != NULL);
	Xil_AssertNonvoid(MsgLength <= XIPIPSU_MAX_MSG_LEN);

	BufferPtr = XIpiPsu_GetBufferAddress(InstancePtr,
			InstancePtr->Config.BitMask, DestCpuMask, BufferType);
	if (BufferPtr != NULL) {
		/* Copy the Message to IPI Buffer */
		for (Index = 0U; Index < MsgLength; Index++) {
			BufferPtr[Index] = MsgPtr[Index];
		}
		Status = XST_SUCCESS;
	} else {
		Status = XST_FAILURE;
	}

	return Status;
}

/*****************************************************************************/
/**
*
* Set up the device configuration based on the unique device ID. A table
* contains the configuration info for each device in the system.
*
* @param	DeviceId contains the ID of the device to set up the
*			configuration for.
 * @param	ConfigTblPtr is the device configuration structure containing required
 *		  	hardware build data
*
* @return	A pointer to the device configuration for the specified
*			device ID. See xipipsu.h for the definition of
*			XIpiPsu_Config.
*
* @note		This is for safety use case where in this function has to
* 			be called before CfgInitialize. So that driver will be
* 			initialized with the provided configuration. For non-safe
* 			use cases, this is not needed.
*
******************************************************************************/
void XIpiPsu_SetConfigTable(u32 DeviceId, XIpiPsu_Config *ConfigTblPtr)
{
	u32 Index;

	Xil_AssertVoid(ConfigTblPtr != NULL);

	for (Index = 0U; Index < XPAR_XIPIPSU_NUM_INSTANCES; Index++) {
		if (XIpiPsu_ConfigTable[Index].DeviceId == DeviceId) {
			XIpiPsu_ConfigTable[Index].BaseAddress = ConfigTblPtr->BaseAddress;
			XIpiPsu_ConfigTable[Index].BitMask = ConfigTblPtr->BitMask;
			XIpiPsu_ConfigTable[Index].BufferIndex = ConfigTblPtr->BufferIndex;
			XIpiPsu_ConfigTable[Index].IntId = ConfigTblPtr->IntId;
		}
	}
}
/** @} */
