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
* @file xplmi_generic.c
*
* This is the file which contains general commands.
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
#include "xplmi_generic.h"
#include "xstatus.h"
#include "xplmi_util.h"
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
static int XPlmi_Reserved(XPlmi_Cmd * Cmd)
{
	XPlmi_Printf(DEBUG_INFO, "%s %p\n\r", __func__, Cmd);
	return XST_SUCCESS;
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
static int XPlmi_MaskPoll(XPlmi_Cmd * Cmd)
{
	u32 Addr = Cmd->Payload[0];
	u32 Mask = Cmd->Payload[1];
	u32 TimeOutInMs = Cmd->Payload[2];
	int Status = XPlmi_UtilPollForMask(Addr, Mask, TimeOutInMs);
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
static int XPlmi_MaskWrite(XPlmi_Cmd * Cmd)
{
	u32 Addr = Cmd->Payload[0];
	u32 Mask = Cmd->Payload[1];
	u32 Value = Cmd->Payload[2];

	XPlmi_UtilRMW(Addr,Mask, Value);
	return XST_SUCCESS;
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
static int XPlmi_Write(XPlmi_Cmd * Cmd)
{
	u32 Addr = Cmd->Payload[0];
	u32 Value = Cmd->Payload[1];

	XPlmi_Printf(DEBUG_INFO, "%s \n\r", __func__);

	Xil_Out32(Addr, Value);
	return XST_SUCCESS;
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
static int XPlmi_Delay(XPlmi_Cmd * Cmd)
{
	u32 TimeOut;

	XPlmi_Printf(DEBUG_INFO, "%s \n\r", __func__);

	/** TODO implement timer based delay */
	TimeOut = Cmd->Payload[0] * 1000;

	XPlmi_UtilWait(TimeOut);
	return XST_SUCCESS;
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
static int XPlmi_DmaWrite(XPlmi_Cmd * Cmd)
{
	XPlmi_Printf(DEBUG_INFO, "%s %p\n\r", __func__, Cmd);
	return XST_SUCCESS;
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
static int XPlmi_MaskPoll64(XPlmi_Cmd * Cmd)
{
	XPlmi_Printf(DEBUG_INFO, "%s %p\n\r", __func__, Cmd);
	return XST_SUCCESS;
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
static int XPlmi_MaskWrite64(XPlmi_Cmd * Cmd)
{
	XPlmi_Printf(DEBUG_INFO, "%s %p\n\r", __func__, Cmd);
	return XST_SUCCESS;
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
static int XPlmi_Write64(XPlmi_Cmd * Cmd)
{
	XPlmi_Printf(DEBUG_INFO, "%s %p\n\r", __func__, Cmd);
	return XST_SUCCESS;
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
static int XPlmi_DmaXfer(XPlmi_Cmd * Cmd)
{
	XPlmi_Printf(DEBUG_INFO, "%s %p\n\r", __func__, Cmd);
	return XST_SUCCESS;
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
static XPlmi_ModuleCmd XPlmi_GenericCmds[] =
{
	XPLMI_MODULE_COMMAND(XPlmi_Reserved),
	XPLMI_MODULE_COMMAND(XPlmi_MaskPoll),
	XPLMI_MODULE_COMMAND(XPlmi_MaskWrite),
	XPLMI_MODULE_COMMAND(XPlmi_Write),
	XPLMI_MODULE_COMMAND(XPlmi_Delay),
	XPLMI_MODULE_COMMAND(XPlmi_DmaWrite),
	XPLMI_MODULE_COMMAND(XPlmi_MaskPoll64),
	XPLMI_MODULE_COMMAND(XPlmi_MaskWrite64),
	XPLMI_MODULE_COMMAND(XPlmi_Write64),
	XPLMI_MODULE_COMMAND(XPlmi_DmaXfer)
};

/*****************************************************************************/
/**
 * @brief
 *
 * @param
 *
 * @return
 *
 *****************************************************************************/
static XPlmi_Module XPlmi_Generic =
{
	XPLMI_MODULE_GENERIC_ID,
	XPlmi_GenericCmds,
	sizeof XPlmi_GenericCmds / sizeof *XPlmi_GenericCmds,
};

/*****************************************************************************/
/**
 * @brief
 *
 * @param
 *
 * @return
 *
 *****************************************************************************/
void XPlmi_GenericInit(void)
{
	XPlmi_ModuleRegister(&XPlmi_Generic);
}
