/******************************************************************************
* Copyright (C) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xipipsu.c
* @brief Implements the interface functions of the XIpiPsu driver.
*
* Refer to the header file xipipsu.h for more detailed information.
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
* 2.9   ma  02/12/21    Added IPI CRC functionality
* 	    sdd	02/17/21	Doxygen fixes
*       ma  03/04/21    Initialize BufferIndex during IPI config init
*	sdd 03/10/21	Fixed misrac warnings.
*		     	Fixed doxygen warnings.
*	ag	03/31/21	Fixed IPI poll for ack condition check.
*	sd  06/02/21	Update the crc code remove the check for max length.
* 2.10	sd	07/14/21	Fix a unused label warning
* 2.12	sd	03/29/22	Make the message pointer in XIpiPsu_WriteMessage constant
* 2.14	ht	06/13/23	Restructured the code for more modularity
* 2.17	ht	11/08/24	Update description of Msglength for XIpiPsu_ReadMessage
* 				and XIpiPsu_WriteMessage.
* 2.18  an	08/07/25	Fix pointer to integer cast warning
* 2.20  vn	09/10/26	Align Doxygen comments with AMD standards
* </pre>
*
*****************************************************************************/

/**
* @addtogroup ipipsu_api IPIPSU APIs
* @{
*/

/***************************** Include Files ********************************/
#include "xipipsu.h"
#include "xipipsu_hw.h"
#include "xipipsu_buf.h"

/************************** Constant Definitions *****************************/

/****************************************************************************/
/**
 * @brief Initializes an IPI instance from a given configuration record.
 *
 * @param[out]	InstancePtr Pointer to the IPI instance.
 * @param[in]	CfgPtr Device configuration structure containing the required
 *		  	hardware build data.
 * @param[in]	EffectiveAddress Base address of the device. If address
 *        	translation is not used, pass CfgPtr->BaseAddress to specify the
 *        	physical base address.
 * @return	XST_SUCCESS after successful initialization.
 *
 */
XStatus XIpiPsu_CfgInitialize(XIpiPsu *InstancePtr, XIpiPsu_Config *CfgPtr,
			      UINTPTR EffectiveAddress)
{
	u32 Index;

	/* Validate the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	/* Set device base address and ID */
#ifndef SDT
	InstancePtr->Config.DeviceId = CfgPtr->DeviceId;
#endif
	InstancePtr->Config.BaseAddress = EffectiveAddress;
	InstancePtr->Config.BitMask = CfgPtr->BitMask;
	InstancePtr->Config.IntId = CfgPtr->IntId;
#ifdef SDT
	InstancePtr->Config.IntrParent = CfgPtr->IntrParent;
#endif
	InstancePtr->Config.BufferIndex = CfgPtr->BufferIndex;

	InstancePtr->Config.TargetCount = CfgPtr->TargetCount;

	/* Initialize the TargetList */
	for (Index = 0U; Index < CfgPtr->TargetCount; Index++) {
		InstancePtr->Config.TargetList[Index].Mask =
			CfgPtr->TargetList[Index].Mask;
		InstancePtr->Config.TargetList[Index].BufferIndex =
			CfgPtr->TargetList[Index].BufferIndex;
	}

	/* Mark the component as Ready */
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;
	return (XStatus) XST_SUCCESS;
}

/**
 * @brief	Resets the given IPI register set.
 *
 * Disables the IPIs from all sources and clears any pending IPIs in the status
 * register.
 *
 * @param[in]	InstancePtr Pointer to the IPI instance.
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
 * @brief	Triggers an IPI to a destination CPU.
 *
 * @param[in]	InstancePtr Pointer to the IPI instance.
 * @param[in]	DestCpuMask Mask of the CPU to which the IPI is triggered.
 *
 * @return	XST_SUCCESS after the trigger register is written.
 */
XStatus XIpiPsu_TriggerIpi(XIpiPsu *InstancePtr, u32 DestCpuMask)
{
	/* Validate the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Trigger an IPI to the Target */
	XIpiPsu_WriteReg(InstancePtr->Config.BaseAddress, XIPIPSU_TRIG_OFFSET,
			 DestCpuMask);
	return (XStatus) XST_SUCCESS;

}

/**
 * @brief Polls for an acknowledgment using the observation register.
 *
 * @param[in]	InstancePtr Pointer to the IPI instance.
 * @param[in]	DestCpuMask Mask of the destination CPU from which the
 *			acknowledgment is expected.
 * @param[in]	TimeOutCount Count after which the routine returns failure.
 *
 * @return
 *		- XST_SUCCESS if successful.
 *		- XST_FAILURE if a timeout occurs.
 */
XStatus XIpiPsu_PollForAck(const XIpiPsu *InstancePtr, u32 DestCpuMask,
			   u32 TimeOutCount)
{
	u32 Flag, PollCount;
	XStatus Status;

	/* Validate the input arguments */
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

	if (0x00000000U != Flag) {
		Status = (XStatus)XST_FAILURE;
	} else {
		Status = (XStatus)XST_SUCCESS;
	}

	return Status;
}


/**
 * @brief	Reads an incoming message from a source.
 *
 * @param[in]	InstancePtr Pointer to the IPI instance.
 * @param[in]	SrcCpuMask Device mask for the CPU that sent the message.
 * @param[out]	MsgPtr Buffer that receives the read message. Must not be NULL
 *			and must hold at least MsgLength words.
 * @param[in]	MsgLength Number of words to read (each word is 4 bytes). Must
 *			not exceed XIPIPSU_MAX_MSG_LEN.
 * @param[in]	BufferType Type of buffer (XIPIPSU_BUF_TYPE_MSG or
 *			XIPIPSU_BUF_TYPE_RESP).
 *
 * @return
 *		- XST_SUCCESS if successful.
 *		- XIPIPSU_CRC_ERROR if the CRC check fails (when CRC is
 *		  enabled).
 *		- XST_FAILURE if the buffer address is not valid.
 */
XStatus XIpiPsu_ReadMessage(XIpiPsu *InstancePtr, u32 SrcCpuMask, u32 *MsgPtr,
			    u32 MsgLength, u8 BufferType)
{
	XStatus Status = (XStatus) XST_FAILURE;
	u32 *BufferPtr;
	u32 Index;
	u32 Crc;

	(void)Crc;

	/* Validate the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(MsgPtr != NULL);
	Xil_AssertNonvoid(MsgLength <= XIPIPSU_MAX_MSG_LEN);

	/*Get the Buffer Address for a given pair of CPUs */
	BufferPtr = XIpiPsu_GetBufferAddress(InstancePtr, SrcCpuMask,
					     InstancePtr->Config.BitMask, BufferType);
	if (BufferPtr != NULL) {
#ifdef ENABLE_IPI_CRC
		Crc = XIpiPsu_CalculateCRC((u32)(uintptr_t)BufferPtr, XIPIPSU_W0_TO_W6_SIZE);

		/* Word 8 in IPI is reserved for storing CRC */
		if (BufferPtr[XIPIPSU_CRC_INDEX] != Crc) {
			Status = (XStatus)XIPIPSU_CRC_ERROR;
			goto END;
		}
#endif
		/* Copy the IPI Buffer contents into Users's Buffer*/
		for (Index = 0U; Index < MsgLength; Index++) {
			MsgPtr[Index] = BufferPtr[Index];
		}
		Status = (XStatus)XST_SUCCESS;
	}

#ifdef ENABLE_IPI_CRC
END:
#endif
	/* Return statement */
	return Status;
}


/**
 * @brief	Sends a message to a destination.
 *
 * @param[in]	InstancePtr Pointer to the IPI instance.
 * @param[in]	DestCpuMask Device mask for the destination CPU.
 * @param[in]	MsgPtr Buffer that contains the message to send. Must not
 *			be NULL and must hold at least MsgLength words.
 * @param[in]	MsgLength Number of words to send (each word is 4 bytes). Must
 *			not exceed XIPIPSU_MAX_MSG_LEN.
 * @param[in]	BufferType Type of buffer (XIPIPSU_BUF_TYPE_MSG or
 *			XIPIPSU_BUF_TYPE_RESP).
 *
 * @return
 *		- XST_SUCCESS if successful.
 *		- XST_FAILURE if the buffer address is not valid.
 */
XStatus XIpiPsu_WriteMessage(XIpiPsu *InstancePtr, u32 DestCpuMask, const u32 *MsgPtr,
			     u32 MsgLength, u8 BufferType)
{
	XStatus Status = (XStatus)XST_FAILURE;
	u32 *BufferPtr;
	u32 Index;

	/* Validate the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(MsgPtr != NULL);
	Xil_AssertNonvoid(MsgLength <= XIPIPSU_MAX_MSG_LEN);

	/*Get the Buffer Address for a given pair of CPUs */
	BufferPtr = XIpiPsu_GetBufferAddress(InstancePtr,
					     InstancePtr->Config.BitMask, DestCpuMask, BufferType);

	if (BufferPtr != NULL) {
		/* Copy the Message to IPI Buffer */
		for (Index = 0U; Index < MsgLength; Index++) {
			BufferPtr[Index] = MsgPtr[Index];
		}
#ifdef ENABLE_IPI_CRC
		/* Word 8 in IPI is reserved for storing CRC */
		BufferPtr[XIPIPSU_CRC_INDEX] =
			XIpiPsu_CalculateCRC((u32)(uintptr_t)BufferPtr, XIPIPSU_W0_TO_W6_SIZE);
#endif
		Status = (XStatus)XST_SUCCESS;
	}

	/* Return statement */
	return Status;
}

#ifndef SDT
/*****************************************************************************/
/**
*
* @brief	Sets up the device configuration based on the unique device ID.
*
* A table contains the configuration information for each device in the system.
*
* @param[in]	DeviceId ID of the device to set up the configuration for.
* @param[in]	ConfigTblPtr Device configuration structure containing the
*			required hardware build data.
*
* @note		Call this function before XIpiPsu_CfgInitialize in the safety
*		use case. The driver then initializes from the provided
*		configuration. Non-safety use cases do not require it.<br><br>
*		This function is available only in the XSCT/Classic flow.
*
******************************************************************************/
void XIpiPsu_SetConfigTable(u32 DeviceId, XIpiPsu_Config *ConfigTblPtr)
{
	u32 Index;

	/* Validate the input argument */
	Xil_AssertVoid(ConfigTblPtr != NULL);

	/* Loop through all the IPI devices present in the system */
#if XPAR_XIPIPSU_NUM_INSTANCES != 0
	for (Index = 0U; Index < XPAR_XIPIPSU_NUM_INSTANCES; Index++) {
		/* Set up the device configuration based on the unique device ID */
		if (XIpiPsu_ConfigTable[Index].DeviceId == DeviceId) {
			XIpiPsu_ConfigTable[Index].BaseAddress = ConfigTblPtr->BaseAddress;
			XIpiPsu_ConfigTable[Index].BitMask = ConfigTblPtr->BitMask;
			XIpiPsu_ConfigTable[Index].BufferIndex = ConfigTblPtr->BufferIndex;
			XIpiPsu_ConfigTable[Index].IntId = ConfigTblPtr->IntId;
		}
	}
#else
	(void)DeviceId;
	(void)Index;
#endif

}
#endif
/** @} */
