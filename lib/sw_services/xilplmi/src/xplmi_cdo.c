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
* @file xplmi_cdo.c
*
* This file contains code to handling the CDO Buffer.
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
#include "xplmi_cdo.h"

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
static u32 XPlmi_CmdSize(u32 *Buf, u32 Len)
{
	u32 Size = 1;
	if (Len >= Size)
	{
		u32 CmdId = Buf[0];
		u32 PayloadLen = (CmdId & XPLMI_CMD_LEN_MASK) >> 16;
		if (PayloadLen == 255)
		{
			Size = 2;
			if (Len >= Size)
			{
				PayloadLen = Buf[1];
			}
		}
		Size += PayloadLen;
	}
	return Size;
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
static void XPlmi_SetupCmd(XPlmi_Cmd * Cmd, u32 *Buf)
{
	Cmd->CmdId = Buf[0];
	Cmd->Len = (Cmd->CmdId >> 16) & 255;
	Cmd->Payload = Buf + 1;
	if (Cmd->Len == 255) {
		Cmd->Len = Buf[1];
		Cmd->Payload = Buf + 2;
	}
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
int XPlmi_ProcessCdo(u32 *Buf, u32 Len)
{
	int Status;
	XPlmi_Cmd Cmd={0};
	u32 Size;

	while (Len > 0)
	{
		/** break if CMD says END of commands */
		/**
		 * TODO There is no way to find the PLM CDO length right now,
		 * with out reading the boot header.
		 * Breaking the loop when CMD_END is detected. This will be
		 * also useful for debugging.
		 */
		if (Buf[0] == XPLMI_CMD_END)
		{
			Status = XST_SUCCESS;
			break;
		}

		Size = XPlmi_CmdSize(Buf, Len);
		XPlmi_SetupCmd(&Cmd, Buf);
		Buf = Buf + Size;
		Len -= Size;
		Status = XPlmi_CmdExecute(&Cmd);
		if (Status != XST_SUCCESS)
		{
			break;
		}
	}

	return Status;
}
