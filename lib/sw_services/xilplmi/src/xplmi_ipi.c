/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
 *
 * @file xplmi_ipi.c
 *
 * This is the file which contains ipi manager code.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00  ma   10/09/2018 Initial release
 * 1.01  kc   04/09/2019 Added code to register/enable/disable interrupts
 *       kc   05/21/2019 Updated IPI error code to response buffer
 *       ma   08/01/2019 Added LPD init code
 *       rv   02/04/2020 Set the 1st element of response array always to status
 *       bsv  02/13/2020 XilPlmi generic commands should not be supported
 *                       via IPI
 *       ma   02/21/2020 Added code to allow event logging command via IPI
 *       ma   02/28/2020 Added code to disallow EM commands over IPI
 *       bsv  03/09/2020 Added code to support CDO features command
 *       ma   03/19/2020 Added features command for EM module
 *       bsv  04/04/2020 Code clean up
 * 1.02  bsv  06/02/2020 Added code to support GET BOARD command and disallow
 *                       SET BOARD command via IPI
 *       bm   10/14/2020 Code clean up
 *       td   10/19/2020 MISRA C Fixes
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/
/***************************** Include Files *********************************/
#include "xplmi_ipi.h"
#include "xplmi_proc.h"
#include "xplmi_generic.h"
#include "xplmi_hw.h"

#ifdef XPAR_XIPIPSU_0_DEVICE_ID
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static int XPlmi_ValidateIpiCmd(u32 CmdId);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/* Instance of IPI Driver */
static XIpiPsu IpiInst;
static u32 IpiMaskList[XPLMI_IPI_MASK_COUNT] = {0U};

/*****************************************************************************/
/**
 * @brief	This function initializes the IPI.
 *
 * @param	None
 *
 * @return	Status	IPI initialization status
 *
 *****************************************************************************/
int XPlmi_IpiInit(void)
{
	int Status = XST_FAILURE;
	XIpiPsu_Config *IpiCfgPtr;
	u32 Index;

	/* Load Config for Processor IPI Channel */
	IpiCfgPtr = XIpiPsu_LookupConfig(XPAR_XIPIPSU_0_DEVICE_ID);
	if (IpiCfgPtr == NULL) {
		Status = XST_FAILURE;
		goto END;
	}

	/* Init Mask Lists */
	for (Index = 0U; Index < XPLMI_IPI_MASK_COUNT; Index++) {
		IpiMaskList[Index] = IpiCfgPtr->TargetList[Index].Mask;
	}

	/* Initialize the Instance pointer */
	Status = XIpiPsu_CfgInitialize(&IpiInst, IpiCfgPtr,
			IpiCfgPtr->BaseAddress);
	if (XST_SUCCESS != Status) {
		goto END;
	}

	/* Enable IPI from all Masters */
	for (Index = 0U; Index < XPLMI_IPI_MASK_COUNT; Index++) {
		XIpiPsu_InterruptEnable(&IpiInst,
			IpiCfgPtr->TargetList[Index].Mask);
	}

	/*
	 * Enable the IPI IRQ
	 */
	XPlmi_PlmIntrEnable(XPLMI_IPI_IRQ);

END:
	XPlmi_Printf(DEBUG_DETAILED,
			"%s: IPI init status: 0x%x\n\r", __func__, Status);
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This is the handler for IPI interrupts.
 *
 * @param	Data is unused and required for compliance with other
 *          interrupt handlers.
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlmi_IpiDispatchHandler(void *Data)
{
	int Status = XST_FAILURE;
	u32 SrcCpuMask;
	u32 Payload[XPLMI_IPI_MAX_MSG_LEN] = {0U};
	u32 MaskIndex;
	XPlmi_Cmd Cmd = {0U};
	int StatusTmp = XST_FAILURE;

	/* For MISRA C */
	(void )Data;

	SrcCpuMask = Xil_In32(IPI_PMC_ISR);

	for (MaskIndex = 0; MaskIndex < XPLMI_IPI_MASK_COUNT; MaskIndex++) {
		if ((SrcCpuMask & IpiMaskList[MaskIndex]) != 0U) {
			Status = XPlmi_IpiRead(IpiMaskList[MaskIndex], &Payload[0U],
				XPLMI_IPI_MAX_MSG_LEN, XIPIPSU_BUF_TYPE_MSG);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			Cmd.CmdId = Payload[0U];
			Cmd.IpiMask = IpiMaskList[MaskIndex];
			Status = XPlmi_ValidateIpiCmd(Cmd.CmdId);
			if (Status != XST_SUCCESS) {
				Status = XPlmi_UpdateStatus(XPLMI_ERR_IPI_CMD, 0);
				Cmd.Response[0U] = (u32)Status;
				/* Send response to caller */
				XPlmi_IpiWrite(Cmd.IpiMask, Cmd.Response,
					XPLMI_CMD_RESP_SIZE, XIPIPSU_BUF_TYPE_RESP);
				continue;
			}

			Cmd.Len = (Cmd.CmdId >> 16U) & 255U;
			if (Cmd.Len > XPLMI_MAX_IPI_CMD_LEN) {
				Cmd.Len = Payload[1U];
				Cmd.Payload = (u32 *)&Payload[2U];
			} else {
				Cmd.Payload = (u32 *)&Payload[1U];
			}
			Status = XPlmi_CmdExecute(&Cmd);
			Cmd.Response[0U] = (u32)Status;

			/* Send response to caller */
			XPlmi_IpiWrite(Cmd.IpiMask, Cmd.Response, XPLMI_CMD_RESP_SIZE,
				XIPIPSU_BUF_TYPE_RESP);
		}
	}

	XPlmi_Printf(DEBUG_DETAILED, "%s: IPI processed.\n\r", __func__);
	if (XST_SUCCESS != Status) {
		XPlmi_Printf(DEBUG_GENERAL, "%s: Error: Unhandled IPI received\n\r", __func__);
	}

	if ((LpdInitialized & LPD_INITIALIZED) == LPD_INITIALIZED) {
		/* Do not ack the PSM IPI interrupt as it is acked in LibPM */
		if (0U == (SrcCpuMask & IPI_PMC_ISR_PSM_BIT_MASK)) {
			XPlmi_Out32(IPI_PMC_ISR, SrcCpuMask);
		}
	}

END:
	/* Clear and enable the GIC IPI interrupt */
	StatusTmp = XPlmi_PlmIntrClear(XPLMI_IPI_IRQ);
	if ((StatusTmp != XST_SUCCESS) && (Status == XST_SUCCESS)) {
		Status = StatusTmp;
	}
	XPlmi_PlmIntrEnable(XPLMI_IPI_IRQ);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function writes an IPI message or a response to
 * destination CPU.
 *
 * @param	DestCpuMask Destination CPU IPI mask
 * 		MsgPtr		Pointer to message to be written
 * 		MsgLen		IPI message length
 * 		Type		IPI buffer type
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlmi_IpiWrite(u32 DestCpuMask, u32 *MsgPtr, u32 MsgLen, u8 Type)
{
	int Status = XST_FAILURE;

	if ((LpdInitialized & LPD_INITIALIZED) == LPD_INITIALIZED) {
		if ((NULL != MsgPtr) &&
			((MsgLen != 0U) && (MsgLen <= XPLMI_IPI_MAX_MSG_LEN)) &&
			((XIPIPSU_BUF_TYPE_MSG == Type) || (XIPIPSU_BUF_TYPE_RESP == Type))) {
			Status = XIpiPsu_WriteMessage(&IpiInst, DestCpuMask,
				MsgPtr, MsgLen, Type);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}
		XPlmi_Printf(DEBUG_DETAILED, "%s: IPI write status: 0x%x\r\n",
				__func__, Status);
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function reads an IPI message or response from source CPU.
 *
 * @param	SrcCpuMask	Source CPU IPI mask
 * 		MsgPtr 		IPI read message buffer
 * 		MsgLen		IPI message length
 * 		Type		IPI buffer type
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlmi_IpiRead(u32 SrcCpuMask, u32 *MsgPtr, u32 MsgLen, u8 Type)
{
	int Status = XST_FAILURE;

	if ((NULL != MsgPtr) && ((MsgLen != 0U) && (MsgLen <= XPLMI_IPI_MAX_MSG_LEN)) &&
		((XIPIPSU_BUF_TYPE_MSG == Type) || (XIPIPSU_BUF_TYPE_RESP == Type))) {
		/* Read Entire Message to Buffer */
		Status = XIpiPsu_ReadMessage(&IpiInst, SrcCpuMask, MsgPtr, MsgLen,
				Type);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	XPlmi_Printf(DEBUG_DETAILED, "%s: IPI read status: 0x%x\r\n", __func__, Status);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function triggers the IPI interrupt to destination CPU.
 *
 * @param	DestCpuMask Destination CPU IPI mask
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlmi_IpiTrigger(u32 DestCpuMask)
{
	int Status = XST_FAILURE;

	Status = XIpiPsu_TriggerIpi(&IpiInst, DestCpuMask);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function polls for IPI acknowledgment from destination CPU.
 *
 * @param	DestCpuMask Destination CPU IPI mask
 * 		TimeOutCount Timeout value
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlmi_IpiPollForAck(u32 DestCpuMask, u32 TimeOutCount)
{
	int Status = XST_FAILURE;

	Status = XIpiPsu_PollForAck(&IpiInst, DestCpuMask, TimeOutCount);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function checks whether the CmdID passed is supported
 * 			via IPI mechanism or not.
 *
 * @param	CmddId is the command ID
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_ValidateIpiCmd(u32 CmdId)
{
	int Status = XST_FAILURE;
	u32 CmdHndlr = CmdId & XPLMI_CMD_HNDLR_MASK;
	u32 PlmCmdId = CmdId & XPLMI_PLM_GENERIC_CMD_ID_MASK;

	if (CmdHndlr == XPLMI_CMD_HNDLR_PLM_VAL) {
		/*
		 * Only Device ID, Event Logging and Get Board
		 * commands are allowed through IPI.
		 * All other commands are allowed only from CDO file.
		 */
		if ((PlmCmdId == XPLMI_PLM_GENERIC_DEVICE_ID_VAL) ||
			(PlmCmdId == XPLMI_PLM_GENERIC_EVENT_LOGGING_VAL) ||
			(PlmCmdId == XPLMI_PLM_MODULES_FEATURES_VAL) ||
			(PlmCmdId == XPLMI_PLM_MODULES_GET_BOARD_VAL)) {
			Status = XST_SUCCESS;
		}
	} else if ((CmdHndlr == XPLMI_CMD_HNDLR_EM_VAL) &&
				((CmdId & XPLMI_CMD_API_ID_MASK) ==
					XPLMI_PLM_MODULES_FEATURES_VAL)) {
		/*
		 * Only features command is allowed in EM module through IPI.
		 * Other EM commands are allowed only from CDO file.
		 */
		Status = XST_SUCCESS;
	} else if (CmdHndlr != XPLMI_CMD_HNDLR_EM_VAL) {
		/*
		 * Other module's commands are allowed through IPI.
		 */
		Status = XST_SUCCESS;
	} else {
		/* Added for MISRA C */
	}

	return Status;
}
#endif /* XPAR_XIPIPSU_0_DEVICE_ID */
