/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplmi_cmd.c
*
* This is the file which contains .
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   08/23/2018 Initial release
* 1.01  bsv  04/03/2020 Code clean up Xilpdi
* 1.02  kc   06/22/2020 Updated command handler error codes to include command
*                       IDs
* </pre>
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_cmd.h"
#include "xplmi_debug.h"
#include "xplmi_modules.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/*****************************************************************************/
/**
 * @brief	This function will call the command handler registered with the command.
 * Command handler shall execute the command till the payload length.
 *
 * @param	Cmd is pointer to command structure
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlmi_CmdExecute(XPlmi_Cmd * Cmd)
{
	int Status = XST_FAILURE;
	u32 CdoErr;
	u32 ModuleId = (Cmd->CmdId & XPLMI_CMD_MODULE_ID_MASK) >> 8U;
	u32 ApiId = Cmd->CmdId & XPLMI_CMD_API_ID_MASK;
	const XPlmi_Module * Module = NULL;
	const XPlmi_ModuleCmd * ModuleCmd = NULL;

	XPlmi_Printf(DEBUG_DETAILED, "CMD Execute \n\r");
	/* Assign Module */
	if (ModuleId < XPLMI_MAX_MODULES) {
		Module = Modules[ModuleId];
	}
	if (Module == NULL) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_MODULE_MAX, 0);
		goto END;
	}

	/* Check if it is within the commands registered */
	if (ApiId >= Module->CmdCnt) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_CMD_APIID, 0);
		goto END;
	}

	/* Check if proper handler is registered */
	ModuleCmd = &Module->CmdAry[ApiId];
	if (ModuleCmd->Handler == NULL) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_CMD_HANDLER_NULL, 0);
		goto END;
	}
	XPlmi_Printf(DEBUG_DETAILED, "CMD 0x%0x, Len 0x%0x, PayloadLen 0x%0x \n\r",
			Cmd->CmdId, Cmd->Len, Cmd->PayloadLen);

	/* Run the command handler */
	Status = ModuleCmd->Handler(Cmd);
	if (Status != XST_SUCCESS) {
		CdoErr = (u32)XPLMI_ERR_CDO_CMD + (Cmd->CmdId & XPLMI_ERR_CDO_CMD_MASK);
		Status = XPlmi_UpdateStatus((XPlmiStatus_t)CdoErr, Status);
		goto END;
	}

	/* Increment the processed length and it can be used during resume */
	Cmd->ProcessedLen += Cmd->PayloadLen;
	/* Assign the same handler for Resume */
	Cmd->ResumeHandler = ModuleCmd->Handler;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function resumes the command after being partially executed.
 * Resume handler shall execute the command till the payload length.
 *
 * @param	Cmd is pointer to command structure
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlmi_CmdResume(XPlmi_Cmd * Cmd)
{
	int Status = XST_FAILURE;

	XPlmi_Printf(DEBUG_DETAILED, "CMD Resume \n\r");
	Xil_AssertNonvoid(Cmd->ResumeHandler != NULL);
	Status = Cmd->ResumeHandler(Cmd);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_RESUME_HANDLER, Status);
		goto END;
	}

	/* Increment the processed length and it can be used during resume */
	Cmd->ProcessedLen += Cmd->PayloadLen;

END:
	return Status;
}
