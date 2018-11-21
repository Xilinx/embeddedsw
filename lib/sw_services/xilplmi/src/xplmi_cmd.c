/******************************************************************************
* Copyright (C) 2018 Xilinx, Inc. All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
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
#include <xil_assert.h>
#include "xplmi_cmd.h"
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

	//Xil_AssertNonvoid(Cmd->ResumeHandler == NULL);

	/** Assign Module */
	if (ModuleId < XPLMI_MAX_MODULES)
	{
		Module = Modules[ModuleId];
	}
	if (Module == NULL)
	{
		Status = XST_FAILURE;
		goto END;
	}

	/** Check and call the registered command handler */
	if (ApiId >= Module->CmdCnt)
	{
		Status = XST_FAILURE;
		goto END;
	}

	ModuleCmd = &Module->CmdAry[ApiId];
	if (ModuleCmd->Handler == NULL)
	{
		Status = XST_FAILURE;
		goto END;
	}
	Status = ModuleCmd->Handler(Cmd);
	if (Status != XST_SUCCESS)
	{
		if (Cmd->ResumeHandler == NULL)
		{
			Cmd->ResumeHandler = ModuleCmd->Handler;
		}
	} else {
		Xil_AssertNonvoid(Cmd->ResumeHandler == NULL);
	}

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

	Xil_AssertNonvoid(Cmd->ResumeHandler != NULL);
	Status = Cmd->ResumeHandler(Cmd);
	if (Status != XST_SUCCESS) {
		Xil_AssertNonvoid(Cmd->ResumeHandler != NULL);
	} else {
		Cmd->ResumeHandler = NULL;
	}
	return Status;
}
