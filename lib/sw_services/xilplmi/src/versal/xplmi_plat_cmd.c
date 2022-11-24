/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file versal/xplmi_plat_cmd.c
* @addtogroup xplmi_apis XilPlmi Versal APIs
* @{
* @cond xplmi_internal
* This file contains versal specific cmds and modules logic.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ====  ==== ======== ======================================================-
* 1.00  bm   07/06/2022 Initial release
*       ma   07/08/2022 Add support for Tamper Trigger over IPI
* 1.01  ng   11/11/2022 Fixed doxygen file name error
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_modules.h"
#include "xplmi.h"
#include "xplmi_generic.h"
#include "xplmi_ssit.h"
#include "xplmi_event_logging.h"
#include "xplmi_cmd.h"
#include "xil_util.h"
#include "xplmi_cdo.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function checks if the IPI command is accessible or not
 *
 * @param	CmdId is the Command ID
 * @param	IpiReqType is the IPI command request type
 *
 * @return	XST_SUCCESS on success and XST_FAILURE on failure
 *
 *****************************************************************************/
int XPlmi_CheckIpiAccess(u32 CmdId, u32 IpiReqType)
{
	int Status = XST_FAILURE;
	u32 ModuleCmdId = CmdId & XPLMI_PLM_GENERIC_CMD_ID_MASK;

	/* Secure check for PLMI IPI commands */
	switch (ModuleCmdId) {
		/*
		 * Check IPI request type for Event Logging IPI command
		 * and allow access only if the request is secure
		 */
		case XPLMI_PLM_GENERIC_EVENT_LOGGING_VAL:
			if (XPLMI_CMD_SECURE == IpiReqType) {
				Status = XST_SUCCESS;
			}
			break;

		/* Allow access for all other IPI commands */
		default:
			Status = XST_SUCCESS;
			break;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function checks whether the Cmd passed is supported
 * 			via IPI mechanism or not.
 *
 * @param	ModuleId is the module ID
 * @param	ApiId is the API ID
 *
 * @return	XST_SUCCESS on success and XST_FAILURE on failure
 *
 *****************************************************************************/
int XPlmi_ValidateCmd(u32 ModuleId, u32 ApiId)
{
	int Status = XST_FAILURE;

	/* Validate IPI Command */
	switch (ModuleId) {
		case XPLMI_MODULE_GENERIC_ID:
			/*
			 * Only Device ID, Event Logging and Get Board
			 * commands are allowed through IPI.
			 * All other commands are allowed only from CDO file.
			 */
			if ((ApiId == XPLMI_PLM_GENERIC_DEVICE_ID_VAL) ||
					(ApiId == XPLMI_PLM_GENERIC_EVENT_LOGGING_VAL) ||
					(ApiId == XPLMI_PLM_MODULES_FEATURES_VAL) ||
					(ApiId == XPLMI_PLM_MODULES_GET_BOARD_VAL) ||
					(ApiId == XPLMI_PLM_GENERIC_TAMP_TRIGGER_VAL)) {
				Status = XST_SUCCESS;
			}
			break;

		case XPLMI_MODULE_ERROR_ID:
			/*
			 * Only features command is allowed in EM module through IPI.
			 * Other EM commands are allowed only from CDO file.
			 */
			if (ApiId == XPLMI_PLM_MODULES_FEATURES_VAL) {
				Status = XST_SUCCESS;
			}
			break;

		case XPLMI_MODULE_LOADER_ID:
			/*
			 * Except Set Image Info command, all other commands are allowed
			 * in Loader module through IPI.
			 */
			if (ApiId != XPLMI_PLM_LOADER_SET_IMG_INFO_VAL) {
				Status = XST_SUCCESS;
			}
			break;

		default:
			/* Other module's commands are allowed through IPI */
			Status = XST_SUCCESS;
			break;
	}

	return Status;
}
