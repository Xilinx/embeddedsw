/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
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
 * 1.03  ma   02/12/2021 Return unique error codes in case of IPI read errors
 *       ma   03/04/2021 Code clean up
 *       ma   03/04/2021 Added access check for IPI commands
 *       ma   03/10/2021 Added code to disallow set image info Loader command
 *       bsv  03/24/2021 All IPIs to be acknowledged in XPlmi_IpiDispatchHandler
 *       bm   04/03/2021 Register IPI handler in IpiInit
 *       bsv  04/16/2021 Add provision to store Subsystem Id in XilPlmi
 *       bm   05/17/2021 Code cleanup
 *       bm   05/18/2021 Fix issue in IpiDispatchHandler
 *       har  05/18/2021 Updated Status to include library error code in case
 *                       of IPI access error
 * 1.04  bsv  06/09/2021 Add warning in case IPI-0 interrupt is disabled
 *       bsv  06/17/2021 Update warning in case some IPIs are disabled
 *       bsv  08/02/2021 Reduce PLM code size
 *       ma   08/05/2021 Add separate task for each IPI channel
 *       ma   08/12/2021 Fix issue in task creation for IPI channels
 *       bsv  08/15/2021 Removed unwanted goto statements
 *       rv   08/19/2021 Do not ack force power down command
 *       rv   08/22/2021 Use XPLMI_PLM_GENERIC_CMD_ID_MASK macro instead of hard
 *			 coded value
 *       rv   08/25/2021 Check for module ID also along with API ID for xilpm
 *			 force power down command
 *       bsv  10/11/2021 Added redundancy for CheckIpiAccess API
 * 1.05  ma   12/15/2021 Update function header for XPlmi_IpiDispatchHandler
 *       ma   01/17/2022 Enable SLVERR for IPI
 *       ma   02/04/2022 Print Command ID when IPI command execute fails
 *       bsv  03/05/2022 Fix exception while deleting two consecutive tasks of
 *                       same priority
 * 1.06  skd  04/21/2022 Misra-C violation Rule 8.7 fixed
 *	     rj   05/25/2022 Remove check for module ID and API ID for XilPM
 *			             force power down command
 *       skg  06/20/2022 Misra-C violation Rule 8.13 fixed
 *       bm   07/06/2022 Refactor versal and versal_net code
 *       bm   07/18/2022 Shutdown modules gracefully during update
 *       sk   08/08/2022 Set IPI task's to low priority
 * 1.07  skg  10/04/2022 Added support to handle valid and invalid commands
 *       ng   11/11/2022 Updated doxygen comments
 *       bm   02/04/2023 Added support to return warnings
 *       bm   03/09/2023 Add NULL check for module before using it
 *       bm   03/09/2023 Added redundant calls for XPlmi_ValidateCmd and
 *                       CheckIpiAccess
 *       ng   03/30/2023 Updated algorithm and return values in doxygen comments
 * 1.08  bm   06/23/2023 Added IPI access permissions validation
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/
/***************************** Include Files *********************************/
#include "xplmi.h"
#include "xplmi_ipi.h"
#include "xplmi_proc.h"
#include "xplmi_generic.h"
#include "xplmi_hw.h"
#include "xil_util.h"

#ifdef XPLMI_IPI_DEVICE_ID

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XPLMI_IPI_XSDB_MASTER_MASK	IPI_PMC_ISR_IPI5_BIT_MASK
#define XPLMI_PMC_IMAGE_ID		(0x1C000001U)
#define XPLMI_IPI_PMC_IMR_MASK		(0xFCU)
#define XPLMI_IPI_PMC_IMR_SHIFT		(0x2U)
#define XPLMI_IPI5_BUFFER_INDEX		(0x7U)
#define XPLMI_ACCESS_PERM_MASK		(0x3U)
#define XPLMI_ACCESS_PERM_SHIFT		(0x2U)

/************************** Function Prototypes ******************************/
static u32 XPlmi_GetIpiReqType(u32 CmdId, u32 SrcIndex);
static XPlmi_SubsystemHandler XPlmi_GetPmSubsystemHandler(
	XPlmi_SubsystemHandler SubsystemHandler);
static int XPlmi_IpiDispatchHandler(void *Data);
static int XPlmi_IpiCmdExecute(XPlmi_Cmd * CmdPtr, u32 * Payload);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/* Instance of IPI Driver */
static XIpiPsu IpiInst;
static XIpiPsu_Config *IpiCfgPtr;

/*****************************************************************************/
/**
 * @brief	This function stores address of XilPm API to retrieve
 * Subsystem Id based upon mask.
 *
 * @param	SubsystemHandler is handler to XilPm
 *		API called to retrieve Subsystem Id using Ipi mask
 *
 * @return	Address of XilPm API
 *
 *****************************************************************************/
static XPlmi_SubsystemHandler XPlmi_GetPmSubsystemHandler(
	XPlmi_SubsystemHandler SubsystemHandler)
{
	static XPlmi_SubsystemHandler Handler = NULL;

	if (SubsystemHandler != NULL) {
		Handler = SubsystemHandler;
	}

	return Handler;
}

/*****************************************************************************/
/**
 * @brief	This function initializes the IPI Driver Instance
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlmi_IpiDrvInit(void)
{
	int Status = XST_FAILURE;

	/** - Get the Config for Processor IPI Channel. */
	IpiCfgPtr = XIpiPsu_LookupConfig(XPLMI_IPI_DEVICE_ID);
	if (IpiCfgPtr == NULL) {
		goto END;
	}

	/** - Initialize the Instance pointer with the configuration. */
	Status = XIpiPsu_CfgInitialize(&IpiInst, IpiCfgPtr,
			IpiCfgPtr->BaseAddress);
	if (XST_SUCCESS != Status) {
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function initializes the IPI.
 *
 * @param	SubsystemHandler is handler to XilPm API called to retrieve
 *		Subsystem Id using Ipi mask
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XPLM_ERR_TASK_CREATE if failed to create task.
 *
 *****************************************************************************/
int XPlmi_IpiInit(XPlmi_SubsystemHandler SubsystemHandler)
{
	int Status = XST_FAILURE;
	u32 Index;
	u32 RegVal;
	u32 IpiIntrId;
	XPlmi_TaskNode *Task = NULL;

	/** - Load processor IPI config channel. */
	Status = XPlmi_IpiDrvInit();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** - Enable IPI from all Masters. */
	for (Index = 0U; Index < XPLMI_IPI_MASK_COUNT; Index++) {
		XIpiPsu_InterruptEnable(&IpiInst,
			IpiCfgPtr->TargetList[Index].Mask);

		IpiIntrId = XPlmi_GetIpiIntrId(IpiCfgPtr->TargetList[Index].BufferIndex);
		Task = XPlmi_GetTaskInstance(NULL, NULL, IpiIntrId);
		if (Task == NULL) {
			Task = XPlmi_TaskCreate(XPLM_TASK_PRIORITY_1,
					XPlmi_IpiDispatchHandler,
					(void *)IpiCfgPtr->TargetList[Index].BufferIndex);
			if (Task == NULL) {
				Status = XPlmi_UpdateStatus(XPLM_ERR_TASK_CREATE, 0);
				XPlmi_Printf(DEBUG_GENERAL, "IPI Interrupt task creation "
						"error\n\r");
				goto END;
			}
			Task->IntrId = IpiIntrId;
		}
	}

	/** - Enable SLVERR for IPI interface. */
	XPlmi_Out32(XIPIPSU_BASE_ADDR, XPLMI_SLAVE_ERROR_ENABLE_MASK);

	(void) XPlmi_GetPmSubsystemHandler(SubsystemHandler);

	/* In-Place Update is applicable only for versal_net */
	if (XPlmi_IsPlmUpdateDone() == (u8)TRUE) {
		XPlmi_IpiIntrHandler(NULL);
	}

	/** - Register and Enable the IPI IRQ. */
	Status = XPlmi_RegisterNEnableIpi();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* In-Place Update is applicable only for versal_net */
	if (XPlmi_IsPlmUpdateDone() != (u8)TRUE) {
		RegVal = XPlmi_In32(PS7_IPI_PMC_IMR);
		RegVal = (RegVal & XPLMI_IPI_PMC_IMR_MASK) >>
			XPLMI_IPI_PMC_IMR_SHIFT;
		if (RegVal == 0U) {
			goto END;
		}

		Index = 0U;
		XPlmi_Printf_WoTS(DEBUG_GENERAL, "INFO: IPIs disabled:");
		while(RegVal != 0U) {
			if ((RegVal & 1U) == 1U) {
				XPlmi_Printf_WoTS(DEBUG_GENERAL,
				" IPI-%u", Index);
			}
			RegVal = RegVal >> 1U;
			++Index;
		}
		XPlmi_Printf_WoTS(DEBUG_GENERAL, "\n\r");
	}

END:
	XPlmi_Printf(DEBUG_DETAILED,
			"%s: IPI init status: 0x%x\n\r", __func__, Status);
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This is the handler for IPI interrupts.
 *
 * @param	Data is the buffer index of the IPI channel interrupt that is
 * 			received
 *
 * @return
 * 			- XST_SUCCESS on success
 * 			- XPLMI_ERR_IPI_CMD if command cannot be executed through IPI.
 * 			- Other error codes returned through the called functions.
 *
 *****************************************************************************/
static int XPlmi_IpiDispatchHandler(void *Data)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	u32 Payload[XPLMI_IPI_MAX_MSG_LEN] = {0U};
	u8 MaskIndex;
	XPlmi_Cmd Cmd = {0U};
	u8 PendingPsmIpi = (u8)FALSE;

	for (MaskIndex = 0U; MaskIndex < XPLMI_IPI_MASK_COUNT; MaskIndex++) {
		if (IpiInst.Config.TargetList[MaskIndex].BufferIndex == (u32)Data) {
			break;
		}
	}

	if (MaskIndex != XPLMI_IPI_MASK_COUNT) {
		if (XPlmi_IsPlmUpdateInProgress() == (u8)TRUE) {
			Status = XST_SUCCESS;
			goto END;
		}
		Cmd.AckInPLM = (u8)TRUE;
		Cmd.IpiReqType = XPLMI_CMD_NON_SECURE;
		Cmd.IpiMask = IpiInst.Config.TargetList[MaskIndex].Mask;
		if (IPI_PMC_ISR_PSM_BIT_MASK == Cmd.IpiMask) {
			PendingPsmIpi = (u8)TRUE;
		}

		/**
		 * Read the IPI command and arguments
		*/
		Status = XPlmi_IpiRead(IpiInst.Config.TargetList[MaskIndex].Mask,
				&Payload[0U], XPLMI_IPI_MAX_MSG_LEN, XIPIPSU_BUF_TYPE_MSG);

		if (XST_SUCCESS != Status) {
			goto END;
		}

		/**
		 * Get IPI request type
		 */
		Cmd.CmdId = Payload[0U];
		Cmd.IpiReqType = XPlmi_GetIpiReqType(Cmd.CmdId,
				IpiInst.Config.TargetList[MaskIndex].BufferIndex);

		/**
		 * Validate the IPI command
		*/
		Status = XST_FAILURE;
		XSECURE_REDUNDANT_CALL(Status, StatusTmp, XPlmi_ValidateIpiCmd, &Cmd,
				IpiInst.Config.TargetList[MaskIndex].BufferIndex);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = XPlmi_UpdateStatus(XPLMI_ERR_IPI_CMD, Status | StatusTmp);
			goto END;
		}

		/**
		 * Get Subsystem Id
		 */
		if (Cmd.IpiMask == XPLMI_IPI_XSDB_MASTER_MASK) {
			Cmd.SubsystemId = XPLMI_PMC_IMAGE_ID;
		}
		else {
			Cmd.SubsystemId =
				(*XPlmi_GetPmSubsystemHandler(NULL))(Cmd.IpiMask);
		}

		Cmd.Len = (Cmd.CmdId >> 16U) & 255U;
		if (Cmd.Len > XPLMI_MAX_IPI_CMD_LEN) {
			Cmd.Len = Payload[1U];
			Cmd.Payload = (u32 *)&Payload[2U];
		} else {
			Cmd.Payload = (u32 *)&Payload[1U];
		}

		/* Ack PSM IPIs before running handlers */
		if (IPI_PMC_ISR_PSM_BIT_MASK == Cmd.IpiMask) {
			PendingPsmIpi = (u8)FALSE;
			XPlmi_Out32(IPI_PMC_ISR,
				IPI_PMC_ISR_PSM_BIT_MASK);
		}

		/**
		 * Execute the IPI command
		*/
		Status = XPlmi_IpiCmdExecute(&Cmd, Payload);

END:
		/**
		 *  Skip providing ack if it is handled in the command handler.
		 */
		if ((u8)TRUE == Cmd.AckInPLM) {
			Cmd.Response[0U] = (u32)Status & (~(u32)XPLMI_WARNING_STATUS_MASK);
			/**
			 * Send response to caller
			 */
			(void)XPlmi_IpiWrite(Cmd.IpiMask, Cmd.Response,
					XPLMI_CMD_RESP_SIZE,
					XIPIPSU_BUF_TYPE_RESP);
			/**
			 * Ack all IPIs
			 */
			if (XPlmi_IsLpdInitialized() == (u8)TRUE) {
				if ((IPI_PMC_ISR_PSM_BIT_MASK != Cmd.IpiMask) ||
				    (PendingPsmIpi == (u8)TRUE)) {
					XPlmi_Out32(IPI_PMC_ISR, Cmd.IpiMask);
				}
				XPlmi_Out32(IPI_PMC_IER, Cmd.IpiMask);
			}
		}
	}

	if (XST_SUCCESS != Status) {
		if ((Status & XPLMI_WARNING_STATUS_MASK) == XPLMI_WARNING_STATUS_MASK) {
			XPlmi_Printf(DEBUG_GENERAL, "%s: Warning: IPI command failed for "
					"Command ID: 0x%x\r\n", __func__, Cmd.CmdId);
		}
		else {
			XPlmi_Printf(DEBUG_GENERAL, "%s: Error: IPI command failed for "
					"Command ID: 0x%x\r\n", __func__, Cmd.CmdId);
		}
	} else {
		XPlmi_Printf(DEBUG_DETAILED, "%s: IPI processed.\n\r", __func__);
	}

	/**
	 * Clear and enable the IPI interrupt
	 */
	XPlmi_ClearIpiIntr();
	XPlmi_EnableIpiIntr();

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function writes an IPI message or a response to destination
 * 			CPU.
 *
 * @param	DestCpuMask Destination CPU IPI mask
 * @param	MsgPtr Pointer to message to be written
 * @param	MsgLen IPI message length
 * @param	Type IPI buffer type
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlmi_IpiWrite(u32 DestCpuMask, const u32 *MsgPtr, u32 MsgLen, u8 Type)
{
	int Status = XST_FAILURE;

	/**
	 * - If LPD is initialised, send the message. Otherwise return an error.
	*/
	if (XPlmi_IsLpdInitialized() == (u8)TRUE) {
		if ((NULL != MsgPtr) &&
			((MsgLen != 0U) && (MsgLen <= XPLMI_IPI_MAX_MSG_LEN)) &&
			((XIPIPSU_BUF_TYPE_MSG == Type) || (XIPIPSU_BUF_TYPE_RESP == Type))) {
			Status = XIpiPsu_WriteMessage(&IpiInst, DestCpuMask,
				MsgPtr, MsgLen, Type);
		}
	}

	XPlmi_Printf(DEBUG_DETAILED, "%s: IPI write status: 0x%x\r\n",
				__func__, Status);
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
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XPLMI_IPI_CRC_MISMATCH_ERR on IPI CRC mismatch error.
 * 			- XPLMI_IPI_READ_ERR if failed to process IPI request.
 *
 *****************************************************************************/
int XPlmi_IpiRead(u32 SrcCpuMask, u32 *MsgPtr, u32 MsgLen, u8 Type)
{
	int Status = XST_FAILURE;

	if ((NULL != MsgPtr) && ((MsgLen != 0U) && (MsgLen <= XPLMI_IPI_MAX_MSG_LEN)) &&
		((XIPIPSU_BUF_TYPE_MSG == Type) || (XIPIPSU_BUF_TYPE_RESP == Type))) {
		/** - Read Entire Message to Buffer. */
		Status = XIpiPsu_ReadMessage(&IpiInst, SrcCpuMask, MsgPtr, MsgLen,
				Type);
		if (Status != XST_SUCCESS) {
			if (XIPIPSU_CRC_ERROR == Status) {
				XPlmi_Printf(DEBUG_GENERAL,
						"%s: IPI CRC validation failed\r\n", __func__);
				Status = XPlmi_UpdateStatus(XPLMI_IPI_CRC_MISMATCH_ERR, 0);
			} else {
				XPlmi_Printf(DEBUG_GENERAL,
						"%s: IPI Buffer address or Message Length "
						"is invalid\r\n", __func__);
				Status = XPlmi_UpdateStatus(XPLMI_IPI_READ_ERR, 0);
			}
		}
	}

	XPlmi_Printf(DEBUG_DETAILED, "%s: IPI read status: 0x%x\r\n", __func__, Status);
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function triggers the IPI interrupt to destination CPU.
 *
 * @param	DestCpuMask Destination CPU IPI mask
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlmi_IpiTrigger(u32 DestCpuMask)
{
	int Status = XST_FAILURE;

	/** - Trigger IPI interrupt to destination CPU. */
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
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlmi_IpiPollForAck(u32 DestCpuMask, u32 TimeOutCount)
{
	int Status = XST_FAILURE;

	/** - Poll the destination CPU until you receive an acknowledgement. */
	Status = XIpiPsu_PollForAck(&IpiInst, DestCpuMask, TimeOutCount);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function checks whether the Cmd passed is supported
 * 			via IPI mechanism or not.
 *
 * @param	Cmd is the pointer to Cmd structure
 * @param	SrcIndex is the source index of IPI command
 *
 * @return
 * 		- XST_SUCCESS on success.
		- XPLMI_ERR_VALIDATE_IPI_INVALID_ID if the module id or ipi
		source index is not valid during ipi validation
		- XPLMI_ERR_VALIDATE_IPI_MODULE_NOT_REGISTERED if the module
		associated with the command's module id is not registered.
		- XPLMI_ERR_VALIDATE_IPI_INVALID_API_ID if the Api Id received
		is greater than the maximum supported commands in the module.
		- XPLMI_ERR_VALIDATE_IPI_NO_IPI_ACCESS if the Api Id received
		during IPI request doesn't have IPI access.
		- XPLMI_ERR_VALIDATE_IPI_NO_SECURE_ACCESS if the Api Id received
		during IPI request only supports non-secure request
		- XPLMI_ERR_VALIDATE_IPI_NO_NONSECURE_ACCESS if the Api Id
		received during IPI request only supports secure request
 *
 *****************************************************************************/
int XPlmi_ValidateIpiCmd(XPlmi_Cmd *Cmd, u32 SrcIndex)
{
	int Status = XST_FAILURE;
	u32 ModuleId = (Cmd->CmdId & XPLMI_CMD_MODULE_ID_MASK) >>
			XPLMI_CMD_MODULE_ID_SHIFT;
	u32 ApiId = Cmd->CmdId & XPLMI_PLM_GENERIC_CMD_ID_MASK;
	u32 AccessPerm = XPLMI_NO_IPI_ACCESS;
	const XPlmi_Module *Module = NULL;

	/**
	 * Validate module number and source IPI index
	 */
	if ((ModuleId >= XPLMI_MAX_MODULES) ||
		(SrcIndex > XPLMI_IPI5_BUFFER_INDEX) ||
		(SrcIndex == IPI_NO_BUF_CHANNEL_INDEX)) {
		Status = XPLMI_ERR_VALIDATE_IPI_INVALID_ID;
		goto END;
	}

	/** Check if the module is registered */
	Module = Modules[ModuleId];
	if (Module == NULL) {
		Status = XPLMI_ERR_VALIDATE_IPI_MODULE_NOT_REGISTERED;
		goto END;
	}

	/**
	 * Validate IPI Command access permission if the permission buffer is
	 * registered for the respective module
	 */
	if (Module->AccessPermBufferPtr != NULL) {
		/** Check if ApiId is greater than the max supported APIs by module */
		if (ApiId >= Module->CmdCnt) {
			/* If a Invalid Cmd Handler is registered skip throwing error */
			if (Module->InvalidCmdHandler == NULL) {
				Status = XPLMI_ERR_VALIDATE_IPI_INVALID_API_ID;
			}
			else {
				Status = XST_SUCCESS;
			}
			goto END;
		}
		AccessPerm = (u32)Module->AccessPermBufferPtr[ApiId] >> (XPLMI_ACCESS_PERM_SHIFT * SrcIndex);
		AccessPerm &= XPLMI_ACCESS_PERM_MASK;
		/** Check if the requested command is supported only through CDO */
		if (AccessPerm == XPLMI_NO_IPI_ACCESS) {
			Status = XPLMI_ERR_VALIDATE_IPI_NO_IPI_ACCESS;
			goto END;
		}
		/** Check the access permissions of the requested Api */
		if ((Cmd->IpiReqType == XPLMI_CMD_NON_SECURE) &&
				(AccessPerm == XPLMI_SECURE_IPI_ACCESS)) {
			Status = XPLMI_ERR_VALIDATE_IPI_NO_NONSECURE_ACCESS | XPLMI_WARNING_MINOR_MASK;
			goto END;
		}
		if ((Cmd->IpiReqType == XPLMI_CMD_SECURE) &&
				(AccessPerm == XPLMI_NON_SECURE_IPI_ACCESS)) {
			Status = XPLMI_ERR_VALIDATE_IPI_NO_SECURE_ACCESS | XPLMI_WARNING_MINOR_MASK;
			goto END;
		}
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function checks for the IPI command access permission and
 *          returns IPI request type to the caller
 *
 * @param	CmdId is command ID
 * @param	SrcIndex is the source IPI index
 *
 * @return	IpiReqType IPI command request type
 *
 *****************************************************************************/
static u32 XPlmi_GetIpiReqType(u32 CmdId, u32 SrcIndex)
{
	u32 CmdPerm = CmdId & IPI_CMD_HDR_SECURE_BIT_MASK;
	volatile u32 ChannelPerm = XPLMI_CMD_NON_SECURE;
	volatile u32 ChannelPermTmp = XPLMI_CMD_NON_SECURE;
	u32 IpiReqType = XPLMI_CMD_NON_SECURE;

	/**
	 * Treat command as non-secure if Command type is non-secure
	 */
	if (XPLMI_CMD_SECURE != CmdPerm) {
		goto END;
	}

	/**
	 * Read source agent IPI aperture TZ register
	 * and check source to PMC request type
	 */
	ChannelPerm = XPlmi_In32((IPI_APER_TZ_000_ADDR +
			(SrcIndex * XPLMI_WORD_LEN)));
	ChannelPermTmp = XPlmi_In32((IPI_APER_TZ_000_ADDR +
			(SrcIndex * XPLMI_WORD_LEN)));
	ChannelPerm &= IPI_APER_TZ_PMC_REQ_BUF_MASK;
	ChannelPermTmp &= IPI_APER_TZ_PMC_REQ_BUF_MASK;

	/**
	 * Request type is secure if both Channel type and Command type
	 * are secure
	 */
	if ((ChannelPerm == XPLMI_CMD_SECURE) &&
		(ChannelPermTmp == XPLMI_CMD_SECURE)) {
		IpiReqType = XPLMI_CMD_SECURE;
	}

END:
	return IpiReqType;
}

/*****************************************************************************/
/**
 * @brief	This function resumes the command after being partially executed.
 * 			Resume handler shall execute the command till the payload length.
 *
 * @param	CmdPtr is pointer to command structure
 * @param	Payload message buf from client
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XPLMI_ERR_MODULE_MAX if module is not registered.
 * 			- XPLMI_ERR_CMD_APIID on processing unregistered command ID.
 *
 *****************************************************************************/
static int XPlmi_IpiCmdExecute(XPlmi_Cmd * CmdPtr, u32 * Payload)
{
	volatile int Status = XST_FAILURE;
	u32 ModuleId = (CmdPtr->CmdId & XPLMI_CMD_MODULE_ID_MASK) >> XPLMI_CMD_MODULE_ID_SHIFT;
	u32 ApiId = CmdPtr->CmdId & XPLMI_CMD_API_ID_MASK;
	const XPlmi_Module *Module = NULL;


	/* Assign Module */
	if (ModuleId < XPLMI_MAX_MODULES) {
		Module = Modules[ModuleId];
	}
	else {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_MODULE_MAX, 0);
		goto END;
	}

	/** Check if the module is registered */
	if (Module == NULL) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_MODULE_NOT_REGISTERED, 0);
		goto END;
	}

	/* Check if it is within the commands registered */
	if (ApiId >= Module->CmdCnt) {
		if(Module->InvalidCmdHandler != NULL){
			Status = Module->InvalidCmdHandler(Payload, (u32 *)CmdPtr->Response);
		}
		else{
			Status = XPlmi_UpdateStatus(XPLMI_ERR_CMD_APIID, 0);
		}
	}
	else{
		 Status = XPlmi_CmdExecute(CmdPtr);
	}

END:
      return Status;
}
#endif /* XPLMI_IPI_DEVICE_ID */
