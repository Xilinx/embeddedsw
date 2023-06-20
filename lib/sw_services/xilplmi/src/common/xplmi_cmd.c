/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplmi_cmd.c
*
* This is the file which contains command execution code.
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
*       bm   10/14/2020 Code clean up
*       td   10/19/2020 MISRA C Fixes
* 1.03  td   07/08/2021 Fix doxygen warnings
* 1.04  ng   11/11/2022 Updated doxygen comments
*       bm   03/09/2023 Add NULL check for module before using it
*       ng   03/30/2023 Updated algorithm and return values in doxygen comments
* 1.05  bm   06/13/2023 Add API to just log PLM error
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_cmd.h"
#include "xplmi_debug.h"
#include "xplmi_modules.h"
#include "xil_assert.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/*****************************************************************************/
/**
 * @brief	This function will call the command handler registered with the
 * 			command. Command handler shall execute the command till the
 * 			payload length.
 *
 * @param	CmdPtr is pointer to command structure
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XPLMI_ERR_MODULE_MAX if the module is not registered.
 * 			- XPLMI_ERR_CMD_APIID on invalid module and unregistered CMD ID.
 * 			- XPLMI_ERR_CMD_HANDLER_NULL if command handler is not registered.
 * 			- XPLMI_ERR_CDO_CMD on invalid CDO command handler.
 *
 *****************************************************************************/
int XPlmi_CmdExecute(XPlmi_Cmd *CmdPtr)
{
	int Status = XST_FAILURE;
	u32 CdoErr;
	u32 ModuleId = (CmdPtr->CmdId & XPLMI_CMD_MODULE_ID_MASK) >> 8U;
	u32 ApiId = CmdPtr->CmdId & XPLMI_CMD_API_ID_MASK;
	const XPlmi_Module *Module = NULL;
	const XPlmi_ModuleCmd *ModuleCmd = NULL;

	XPlmi_Printf(DEBUG_DETAILED, "CMD Execute \n\r");
	/** - Assign Module */
	if (ModuleId >= XPLMI_MAX_MODULES) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_MODULE_MAX, 0);
		goto END;
	}

	/** - Check if the module is registered */
	Module = Modules[ModuleId];
	if (Module == NULL) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_MODULE_NOT_REGISTERED, 0);
		goto END;
	}

	/** - Check if it is within the commands registered */
	if (ApiId >= Module->CmdCnt) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_CMD_APIID, 0);
		goto END;
	}

	/** - Check if proper handler is registered */
	ModuleCmd = &Module->CmdAry[ApiId];
	if (ModuleCmd->Handler == NULL) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_CMD_HANDLER_NULL, 0);
		goto END;
	}
	XPlmi_Printf(DEBUG_DETAILED, "CMD 0x%0x, Len 0x%0x, PayloadLen 0x%0x \n\r",
			CmdPtr->CmdId, CmdPtr->Len, CmdPtr->PayloadLen);

	/** - Run the command handler */
	Status = ModuleCmd->Handler(CmdPtr);
	if (Status != XST_SUCCESS) {
		CdoErr = (u32)XPLMI_ERR_CDO_CMD + (CmdPtr->CmdId & XPLMI_ERR_CDO_CMD_MASK);
		Status = XPlmi_UpdateStatus((XPlmiStatus_t)CdoErr, Status);
		if (CmdPtr->DeferredError != (u8)TRUE) {
			goto END;
		}
		else {
			/* If Deferred Error, log the error and continue */
			XPlmi_Printf(DEBUG_GENERAL, "Deferring CDO Error\n\r");
			XPlmi_LogPlmErr(Status);
			Status = XST_SUCCESS;
		}
	}

	/** - Increment the processed length and it can be used during resume */
	CmdPtr->ProcessedLen += CmdPtr->PayloadLen;
	/** - Assign the same handler for Resume */
	CmdPtr->ResumeHandler = ModuleCmd->Handler;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function resumes the command after being partially executed.
 * 			Resume handler shall execute the command till the payload length.
 *
 * @param	CmdPtr is pointer to command structure
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XPLMI_ERR_RESUME_HANDLER if CDO CMD resume handler fails.
 *
 *****************************************************************************/
int XPlmi_CmdResume(XPlmi_Cmd * CmdPtr)
{
	int Status = XST_FAILURE;

	XPlmi_Printf(DEBUG_DETAILED, "CMD Resume \n\r");
	Xil_AssertNonvoid(CmdPtr->ResumeHandler != NULL);
	Status = CmdPtr->ResumeHandler(CmdPtr);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_RESUME_HANDLER, Status);
		goto END;
	}

	/* Increment the processed length and it can be used during resume */
	CmdPtr->ProcessedLen += CmdPtr->PayloadLen;

END:
	return Status;
}
