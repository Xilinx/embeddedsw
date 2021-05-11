/******************************************************************************
* Copyright (c) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/



#include "xpfw_ipi_manager.h"

/**
 * This file provides a framework for modules to send and receive IPI messages
 * PMU IPI-0 is used for communication initiated by other master.
 * PMU IPI-1 is used for communication initiated by PMU.
 *
 * Currently this framework supports for checking/embedding IPI ID of a module.
 * IPI ID is the MSB 16-bits of the first word in pay load.
 * Depending on the application requirements, more features like CheckSum,
 * Message Sequencing etc can be added.
 */

/* Instance of IPI Driver */
static XIpiPsu Ipi0Inst, Ipi1Inst;
XIpiPsu *Ipi0InstPtr = &Ipi0Inst;
static XIpiPsu *Ipi1InstPtr = &Ipi1Inst;

s32 XPfw_IpiManagerInit(void)
 {
	s32 Status;
	XIpiPsu_Config *Ipi0CfgPtr, *Ipi1CfgPtr;
	u32 i;

	/* Load Config for PMU IPI-0 */
	Ipi0CfgPtr = XIpiPsu_LookupConfig(XPMU_IPI_0);
	if (Ipi0CfgPtr == NULL) {
		Status = XST_FAILURE;
		goto Done;
	}

	/* Load Config for PMU IPI-1 */
	Ipi1CfgPtr = XIpiPsu_LookupConfig(XPMU_IPI_1);
	if (Ipi1CfgPtr == NULL) {
		Status = XST_FAILURE;
		goto Done;
	}

	/* Initialize the Instance pointer of IPI-0 channel */
	Status = XIpiPsu_CfgInitialize(Ipi0InstPtr, Ipi0CfgPtr,
			Ipi0CfgPtr->BaseAddress);
	if (XST_SUCCESS != Status) {
		goto Done;
	}

	/* Initialize the Instance pointer of IPI-1 channel */
	Status = XIpiPsu_CfgInitialize(Ipi1InstPtr, Ipi1CfgPtr,
			Ipi1CfgPtr->BaseAddress);
	if (XST_SUCCESS != Status) {
		goto Done;
	}

	/* Enable IPI-0 and IPI-1 from all Masters */
	for (i = 0U; i < XPFW_IPI_MASK_COUNT; i++) {
		XIpiPsu_InterruptEnable(Ipi0InstPtr,
					Ipi0CfgPtr->TargetList[i].Mask);
		XIpiPsu_InterruptEnable(Ipi1InstPtr,
					Ipi1CfgPtr->TargetList[i].Mask);
	}

Done:
	return Status;
}


s32 XPfw_IpiWriteMessage(const XPfw_Module_t *ModPtr, u32 DestCpuMask, u32 *MsgPtr, u32 MsgLen)
{
	s32 Status;

	if ((ModPtr == NULL) || (MsgPtr == NULL)) {
		Status = XST_FAILURE;
		goto Done;
	}

	MsgPtr[0] = (MsgPtr[0] & 0x0000FFFFU) | ((u32)ModPtr->IpiId << 16U);

	Status = XIpiPsu_WriteMessage(Ipi1InstPtr, DestCpuMask, MsgPtr, MsgLen,
	XIPIPSU_BUF_TYPE_MSG);

Done:
	return Status;
}


s32 XPfw_IpiWriteResponse(const XPfw_Module_t *ModPtr, u32 DestCpuMask, u32 *MsgPtr, u32 MsgLen)
 {
	s32 Status;

	if ((ModPtr == NULL) || (MsgPtr == NULL)) {
		Status = XST_FAILURE;
		goto Done;
	}

	MsgPtr[0] = (MsgPtr[0] & 0x0000FFFFU) | ((u32)ModPtr->IpiId << 16U);

	Status = XIpiPsu_WriteMessage(Ipi0InstPtr, DestCpuMask, MsgPtr, MsgLen,
			XIPIPSU_BUF_TYPE_RESP);

Done:
	return Status;
}

s32 XPfw_IpiReadMessage(u32 SrcCpuMask, u32 *MsgPtr, u32 MsgLen)
 {
	s32 Status = XST_FAILURE;
	u32 RespBuf[XPFW_IPI_MAX_MSG_LEN] = {0};

	/* Check if MsgPtr is NULL and return error */
	if (MsgPtr == NULL) {
		Status = XST_FAILURE;
		goto Done;
	}

	/* Read Entire Message to Buffer */
	Status = XIpiPsu_ReadMessage(Ipi0InstPtr, SrcCpuMask, MsgPtr, MsgLen,
			XIPIPSU_BUF_TYPE_MSG);

	/* Check for IPI CRC error */
	if (XIPIPSU_CRC_ERROR == Status) {
		/* Write error occurrence to PERS register */
		XPfw_RMW32(PMU_GLOBAL_PERS_GLOB_GEN_STORAGE5, IPI_CRC_ERROR_OCCURRED,
					IPI_CRC_ERROR_OCCURRED);

		/* Trigger FW Error1 */
		XPfw_RMW32(PMU_LOCAL_PMU_SERV_ERR, PMU_LOCAL_PMU_SERV_ERR_FWERR1_MASK,
					PMU_LOCAL_PMU_SERV_ERR_FWERR1_MASK);
		XPfw_RMW32(PMU_LOCAL_PMU_SERV_ERR, PMU_LOCAL_PMU_SERV_ERR_FWERR1_MASK,
					0x0U);
		XPfw_Printf(DEBUG_ERROR, "ERROR: IPI buffer CRC mismatch\r\n");
		Status = XST_FAILURE;
	}

Done:
	/* Send response for failure status */
	if (XST_SUCCESS != Status) {
		RespBuf[0] = (u32)Status;
		Status = XIpiPsu_WriteMessage(Ipi0InstPtr, SrcCpuMask, RespBuf,
				XPFW_IPI_MAX_MSG_LEN, XIPIPSU_BUF_TYPE_RESP);
	}
	return Status;
}

s32 XPfw_IpiReadResponse(const XPfw_Module_t *ModPtr, u32 SrcCpuMask, u32 *MsgPtr, u32 MsgLen)
 {
	s32 Status = XST_FAILURE;
	u32 MsgHeader = 0U;

	/* Check if ModPtr and MsgPtr are NULL and return error */
	if ((ModPtr == NULL) || (MsgPtr == NULL)) {
		Status = XST_FAILURE;
		goto Done;
	}
	/* Read the first word */
	Status = XIpiPsu_ReadMessage(Ipi1InstPtr, SrcCpuMask, &MsgHeader, 1U,
			XIPIPSU_BUF_TYPE_RESP);

	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto Done;
	}
	/* Check if IPI Id matches the upper 16 bits of first word*/
	if ((MsgHeader >> 16) != ModPtr->IpiId) {
		Status = XST_FAILURE;
		goto Done;
	}
	/* Read Entire Message to Buffer */
	Status = XIpiPsu_ReadMessage(Ipi1InstPtr, SrcCpuMask, MsgPtr, MsgLen,
			XIPIPSU_BUF_TYPE_RESP);

	/* Check for IPI CRC error */
	if (XIPIPSU_CRC_ERROR == Status) {
		/* Write error occurrence to PERS register */
		XPfw_RMW32(PMU_GLOBAL_PERS_GLOB_GEN_STORAGE5, IPI_CRC_ERROR_OCCURRED,
					IPI_CRC_ERROR_OCCURRED);

		/* Trigger FW Error1 */
		XPfw_RMW32(PMU_LOCAL_PMU_SERV_ERR, PMU_LOCAL_PMU_SERV_ERR_FWERR1_MASK,
					PMU_LOCAL_PMU_SERV_ERR_FWERR1_MASK);
		XPfw_RMW32(PMU_LOCAL_PMU_SERV_ERR, PMU_LOCAL_PMU_SERV_ERR_FWERR1_MASK,
					0x0U);
		XPfw_Printf(DEBUG_ERROR, "ERROR: IPI buffer CRC mismatch\r\n");
		Status = XST_FAILURE;
	}

Done:
	return Status;
}

inline s32 XPfw_IpiTrigger(u32 DestCpuMask)
{
	return XIpiPsu_TriggerIpi(Ipi1InstPtr, DestCpuMask);
}

inline s32 XPfw_IpiPollForAck(u32 DestCpuMask, u32 TimeOutCount)
{
	return XIpiPsu_PollForAck(Ipi1InstPtr, DestCpuMask, TimeOutCount);
}
