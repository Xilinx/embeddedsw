/***************************************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/


/**************************************************************************************************/
/**
*
* @file xplmi_asu_update.c
*
* This file contains ASU in-place update implementation for versal_2ve_2vm platforms.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ====  ==== ======== ==============================================================================
* 1.0   vm   03/16/26 Initial release
*
* </pre>
*
***************************************************************************************************/

/**
 * @addtogroup xilplmi_server_apis XilPlmi server APIs
 * @{
 */

/*************************************** Include Files ********************************************/
#include "xplmi_asu_update.h"
#include "xplmi_scheduler.h"
#include "xil_error_node.h"
#include "xplmi_err_common.h"

/************************************ Constant Definitions ****************************************/
#define XPLMI_ASU_UPDATE_TASK_ID		(0x121U) /**< Task Id for ASU update */
#define XPLMI_INVALID_UPDATE_ADDR		(0xFFFFFFFFU) /**< Invalid update address */
#define XPLMI_ASU_MAX_RETRY_COUNT		(600U) /**< Max retry count: 600 * 100ms = 60 sec */
#define XPLMI_US_TO_MS_DIV_FACTOR		(1000U) /**< Microseconds to milliseconds conversion factor */
#define XPLMI_UPDATE_FLAG_MASK			(0x1U) /**< In-Place Update flag Mask */
#define XPLMI_UPDATE_PAYLOAD_LEN		(0x2U) /**< In-Place Update payload len */
#define XPLMI_UPDATE_USING_IMAGE_STORE		(0U) /**< In-Place Update using PDI in Image Store,
							     1-DDR Location */

/************************************** Type Definitions ******************************************/

/******************************* Macros (Inline Functions) Definitions ****************************/

/************************************ Function Prototypes *****************************************/
static int XPlmi_AsuUpdateTask(void *Arg);

/************************************ Variable Definitions ****************************************/
static XPlmi_IsPdiAddrLookup_t XPlmi_IsPdiAddrLookup;	/**< Function pointer for PDI address
							     lookup */
static XPlmi_CheckAsuPresenceInPdi_t XPlmi_IsAsuPresenceInPdi;	/**< Function pointer for ASU
								     presence check */
static XPlmi_LoadAsuImage_t XLoader_LoadAsu; /**< Function pointer for ASU image loading */
static u32 UpdateAsuPdiAddr = XPLMI_INVALID_UPDATE_ADDR; /**< Update PDI address */

/**************************************************************************************************/
/**
 * @brief	This function gets UpdateAsuPdiAddr variable value.
 *
 * @return
 * 		- UpdateAsuPdiAddr value.
 *
 **************************************************************************************************/
u32 XPlmi_GetAsuUpdatePdiAddr(void)
{
	return UpdateAsuPdiAddr;
}

/**************************************************************************************************/
/**
 * @brief	This function checks if ASU firmware is present and running.
 *
 * @return
 * 		- TRUE if ASU firmware is present (FW_IS_PRESENT bit set).
 * 		- FALSE if ASU firmware is not running.
 *
 **************************************************************************************************/
u8 XPlmi_AsuFwIsPresent(void)
{
	u8 IsAsuFwPresent = FALSE;
	u32 RtcaVal;

	/**
	 * Check ASU firmware presence by verifying FW_IS_PRESENT bit in
	 * RTCA exec status register.
	 */
	RtcaVal = XPlmi_In32(XPLMI_ASU_RTCA_EXEC_STATUS);
	if ((RtcaVal & XPLMI_ASU_RTCA_FW_IS_PRESENT_MASK) == XPLMI_ASU_RTCA_FW_IS_PRESENT_MASK) {
		IsAsuFwPresent = TRUE;
	}

	return IsAsuFwPresent;
}

/**************************************************************************************************/
/**
 * @brief	This function initializes ASU in-place update subsystem.
 *
 * @param	IsPdiAddrLookupHandler is the PDI address look-up handler.
 * @param	CheckAsuPresenceHandler is the callback to check ASU presence in PDI.
 * @param	LoadAsuImageHandler is the callback to load ASU image from PDI.
 *
 * @return
 * 		- XST_SUCCESS if initialization successful.
 * 		- XPLM_ERR_TASK_CREATE if failed to create ASU update task.
 *
 **************************************************************************************************/
int XPlmi_AsuUpdateInit(XPlmi_IsPdiAddrLookup_t IsPdiAddrLookupHandler,
		XPlmi_CheckAsuPresenceInPdi_t CheckAsuPresenceHandler,
		XPlmi_LoadAsuImage_t LoadAsuImageHandler)
{
	volatile int Status = XST_FAILURE;
	XPlmi_TaskNode *Task = NULL;

	/** Store the handler function pointers */
	XPlmi_IsPdiAddrLookup = IsPdiAddrLookupHandler;
	XPlmi_IsAsuPresenceInPdi = CheckAsuPresenceHandler;
	XLoader_LoadAsu = LoadAsuImageHandler;

	/** Create ASU update task which will be triggered by IPI command */
	Task = XPlmi_TaskCreate(XPLM_TASK_PRIORITY_1, XPlmi_AsuUpdateTask, NULL);
	if (Task == NULL) {
		Status = XPlmi_UpdateStatus(XPLM_ERR_TASK_CREATE, 0);
		XPlmi_Printf(DEBUG_GENERAL, "ASU update task creation failed\n\r");
		goto END;
	}

	/** Assign task ID for identification */
	Task->IntrId = XPLMI_ASU_UPDATE_TASK_ID;

	Status = XST_SUCCESS;

END:
	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function handles In-Place ASU Update.
 *
 * @param	Cmd is the command structure with payload containing DDR address.
 *
 * @return
 * 		- XST_SUCCESS if ASU update successful.
 * 		- Error code on failure.
 *
 **************************************************************************************************/
int XPlmi_AsuUpdate(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 DdrRsvdAddr;
	u32 DdrRsvdSize;
	u32 PdiId;
	u32 Flag;
	u64 PdiAddr = XPLMI_INVALID_UPDATE_ADDR;
	XPlmi_TaskNode *Task = NULL;
	u32 AsuFwPresent;

	/** Check if the command structure is valid */
	if (Cmd == NULL) {
		Status = XPLMI_ERR_INVALID_PAYLOAD_LEN;
		goto END;
	}

	/** Validate payload length */
	if (Cmd->Len < XPLMI_UPDATE_PAYLOAD_LEN) {
		Status = XPLMI_ERR_INPLACE_UPDATE_INVALID_PAYLOAD_LEN;
		goto END;
	}

	/** Get DDR reserved address for ASU update */
	DdrRsvdAddr = XPlmi_In32(XPLMI_RTCFG_PLM_RSVD_DDR_ADDR);
	DdrRsvdSize = XPlmi_In32(XPLMI_RTCFG_PLM_RSVD_DDR_SIZE);

	/** Check if DDR reserved area is valid */
	if ((DdrRsvdAddr == XPLMI_INVALID_PLM_RSVD_DDR_ADDR) ||
		(DdrRsvdSize == XPLMI_INVALID_PLM_RSVD_DDR_SIZE) ||
		(((u64)DdrRsvdAddr + DdrRsvdSize) > (u64)XPLMI_2GB_END_ADDR)) {
		Status = XPLMI_ERR_INVALID_RSVD_DDR_REGION_UPDATE;
		goto END;
	}

	/** Check update flag */
	Flag = (Cmd->Payload[INDEX_ZERO] & XPLMI_UPDATE_FLAG_MASK);

	/** Check if update is from image store or DDR */
	if (Flag == XPLMI_UPDATE_USING_IMAGE_STORE) {
		if (XPlmi_In32(XPLMI_RTCFG_IMG_STORE_ADDRESS_HIGH) != 0U) {
			Status = XPLMI_ERR_INPLACE_UPDATE_FROM_IMAGE_STORE;
			XPlmi_Printf(DEBUG_GENERAL, "Image Store not in lower 2GB range\n\r");
			goto END;
		}

		PdiId = Cmd->Payload[INDEX_ONE];
		Status = XPlmi_IsPdiAddrLookup(PdiId, (u64*)&PdiAddr);
		if (Status != XST_SUCCESS) {
			Status = XPLMI_ERR_INPLACE_UPDATE_FROM_IMAGE_STORE;
			goto END;
		}

		UpdateAsuPdiAddr = (u32)PdiAddr;
	} else {
		UpdateAsuPdiAddr = Cmd->Payload[INDEX_ONE];
	}

	/** Check if ASU image is present in the specified PDI */
	Status = XPlmi_IsAsuPresenceInPdi(UpdateAsuPdiAddr);
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_GENERAL, "ASU image not found in PDI\n\r");
		goto END;
	}

	/** Check if ASU firmware is currently running */
	AsuFwPresent = XPlmi_AsuFwIsPresent();
	if (AsuFwPresent == FALSE) {
		XPlmi_Printf(DEBUG_GENERAL, "ASU firmware is not running\n\r");
		Status = XPLMI_ERR_ASU_FW_NOT_RUNNING;
		goto END;
	}

	/** Retrieve the pre-created ASU update task */
	Task = XPlmi_GetTaskInstance(NULL, NULL, XPLMI_ASU_UPDATE_TASK_ID);
	if (Task == NULL) {
		Status = XPLMI_ERR_INPLACE_UPDATE_TASK_NOT_FOUND;
		goto END;
	}

	/** Send event notification to trigger ASU shutdown */
	XPlmi_HandleSwError(XIL_NODETYPE_EVENT_ERROR_SW_ERR,
		XIL_EVENT_ERROR_MASK_ASU_UPDATE);

	/** Trigger the async task to monitor shutdown and load new firmware */
	XPlmi_TaskTriggerNow(Task);
	Status = XST_SUCCESS;

END:
	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function is the task handler for ASU in-place update. It polls for ASU
 *		shutdown completion, loads the new firmware image, resets ASU, and waits for
 *		the new firmware to initialize.
 *
 * @param	Arg is unused parameter.
 *
 * @return
 * 		- XST_SUCCESS if ASU update completed successfully.
 * 		- XPLMI_ERR_ASU_SHUTDOWN_TIMEOUT if ASU shutdown exceeds 60 seconds.
 * 		- XPLMI_ERR_ASU_FW_NOT_RUNNING if ASU firmware fails to restart after update.
 * 		- Error code from XLoader_LoadAsu() if firmware loading fails.
 *
 **************************************************************************************************/
static int XPlmi_AsuUpdateTask(void *Arg)
{
	int Status = XST_FAILURE;
	u32 CurrentState;
	static u32 RetryCount = 0U;
	(void)Arg;

	/** Check ASU shutdown state */
	CurrentState = XPlmi_In32(XPLMI_ASU_UPDATE_STATE_REG);

	/** Retry if shutdown is not complete */
	if (CurrentState != XPLMI_UPDATE_STATE_SHUTDOWN_DONE) {
		RetryCount++;

		if (RetryCount >= XPLMI_ASU_MAX_RETRY_COUNT) {
			XPlmi_Printf(DEBUG_GENERAL, "ASU shutdown timeout\n\r");
			Status = XPLMI_ERR_ASU_SHUTDOWN_TIMEOUT;
			goto END;
		}

		/** Reschedule task to retry later */
		Status = XPlmi_SchedulerAddTask(XPLMI_MODULE_GENERIC_ID,
			XPlmi_AsuUpdateTask, NULL,
			XPLMI_ASU_SHUTDOWN_POLL_INTERVAL_US / XPLMI_US_TO_MS_DIV_FACTOR,
			XPLM_TASK_PRIORITY_1, NULL, XPLMI_NON_PERIODIC_TASK);
		goto END;
	}

	/** Shutdown complete - reset retry counter for next update cycle */
	RetryCount = 0U;

	XPlmi_Printf(DEBUG_DETAILED, "ASU shutdown complete\n\r");

	/** Load new ASU firmware image */
	Status = XLoader_LoadAsu();
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_GENERAL, "Failed to load ASU image: 0x%x\n\r", Status);
		goto END;
	}

	/** Reset ASU to start new firmware */
	XPlmi_Out32(XPLMI_ASU_MB_SOFT_RST, XPLMI_ASU_MB_SOFT_RST_ASSERT_MASK);
	usleep(XPLMI_ASU_RESET_SETTLE_TIME_US);
	XPlmi_Out32(XPLMI_ASU_MB_SOFT_RST, XPLMI_ASU_MB_SOFT_RST_DEASSERT_MASK);

	/** Wait for ASU firmware to initialize */
	Status = XPlmi_UtilPollForMask(XPLMI_ASU_RTCA_EXEC_STATUS,
				       XPLMI_ASU_RTCA_FW_IS_PRESENT_MASK,
				       XPLMI_ASU_RST_TIMEOUT);
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_GENERAL, "ASU failed to restart after update\n\r");
		Status = XPLMI_ERR_ASU_FW_NOT_RUNNING;
		goto END;
	}

	XPlmi_Printf(DEBUG_DETAILED, "ASU update completed successfully\n\r");
	Status = XST_SUCCESS;

END:
	return Status;
}

/** @} End of xilplmi_server_apis group */
