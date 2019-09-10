/******************************************************************************
* Copyright (C) 2018-2019 Xilinx, Inc. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
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
 * @brief
 *
 * @param
 *
 * @return
 *
 *****************************************************************************/
int XPlmi_CmdExecute(XPlmi_Cmd * Cmd)
{
	u32 ModuleId = (Cmd->CmdId & XPLMI_CMD_MODULE_ID_MASK) >> 8;
	u32 ApiId = Cmd->CmdId & XPLMI_CMD_API_ID_MASK;
	const XPlmi_Module * Module = NULL;
	const XPlmi_ModuleCmd * ModuleCmd = NULL;
	int Status;

	XPlmi_Printf(DEBUG_DETAILED, "CMD Execute \n\r");
	/** Assign Module */
	if (ModuleId < XPLMI_MAX_MODULES)
	{
		Module = Modules[ModuleId];
	}

	if (Module == NULL)
	{
		Status = XPLMI_UPDATE_STATUS(XPLMI_ERR_MODULE_MAX, 0x0);
		goto END;
	}

	/** Check if it is within the commands registered */
	if (ApiId >= Module->CmdCnt)
	{
		Status = XPLMI_UPDATE_STATUS(XPLMI_ERR_CMD_APIID, 0x0);
		goto END;
	}

	/** Check if proper handler is registered */
	ModuleCmd = &Module->CmdAry[ApiId];
	if (ModuleCmd->Handler == NULL)
	{
		Status = XPLMI_UPDATE_STATUS(XPLMI_ERR_CMD_HANDLER_NULL, 0x0);
		goto END;
	}

	XPlmi_Printf(DEBUG_DETAILED, "CMD 0x%0x, Len 0x%0x, PayloadLen 0x%0x \n\r",
		     Cmd->CmdId, Cmd->Len, Cmd->PayloadLen);

	/** Run the command handler */
	Status = ModuleCmd->Handler(Cmd);
	if (Status != XST_SUCCESS)
	{
		Status = XPLMI_UPDATE_STATUS(XPLMI_ERR_CMD_HANDLER, Status);
		goto END;
	}

	/** Increment the processed length and it can be used during resume */
	Cmd->ProcessedLen += Cmd->PayloadLen;
	/** Assign the same handler for Resume */
	Cmd->ResumeHandler = ModuleCmd->Handler;
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 *
 * @param
 *
 * @return
 *
 *****************************************************************************/
int XPlmi_CmdResume(XPlmi_Cmd * Cmd)
{
	int Status;

	XPlmi_Printf(DEBUG_DETAILED, "CMD Resume \n\r");
	Xil_AssertNonvoid(Cmd->ResumeHandler != NULL);
	Status = Cmd->ResumeHandler(Cmd);
	if (Status != XST_SUCCESS)
	{
		Status = XPLMI_UPDATE_STATUS(XPLMI_ERR_RESUME_HANDLER, Status);
		goto END;
	}

	/** Increment the processed length and it can be used during resume */
	Cmd->ProcessedLen += Cmd->PayloadLen;

END:
	return Status;
}
