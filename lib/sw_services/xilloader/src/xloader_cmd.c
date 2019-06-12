/******************************************************************************
* Copyright (C) 2019 Xilinx, Inc. All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xloader_cmd.c
*
* This file contains the xloader commands implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   03/12/2019 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xloader.h"
#include "xplmi_cmd.h"
#include "xplmi_modules.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/

static int XLoader_Reserved(XPlmi_Cmd * Cmd)
{
	XPlmi_Printf(DEBUG_DETAILED, "%s %p\n\r", __func__, Cmd);
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief This function provides load subsystem PDI command execution
 *  Command payload parameters are
 *	* PdiSrc - Boot Mode values, DDR, PCIe
 *	* PdiAddr - 64bit PDI address located in the Source
 *
 * @param Pointer to the command structure
 *
 * @return Returns the Load PDI command
 *****************************************************************************/
static int XLoader_LoadSubsystemPdi(XPlmi_Cmd * Cmd)
{
	int Status;
	u32 PdiSrc;
	u64 PdiAddr;
	XilPdi* PdiPtr = &SubsystemPdiIns;

	XPlmi_Printf(DEBUG_DETAILED, "%s \n\r", __func__);

	/** store the command fields in resume data */
	PdiSrc = Cmd->Payload[0];
	PdiAddr = (u64 )Cmd->Payload[1];
	PdiAddr = ((u64 )Cmd->Payload[2] |
		   (PdiAddr << 32));

	XPlmi_Printf(DEBUG_INFO, "Subsystem PDI Load: Started\n\r");

	PdiPtr->PdiType = XLOADER_PDI_TYPE_PARTIAL;
	Status = XLoader_LoadPdi(PdiPtr, PdiSrc, PdiAddr);
	if (Status != XST_SUCCESS)
	{
		/* Update the error code */
		XPlmi_ErrMgr(Status);
		goto END;
	}

	XPlmi_Printf(DEBUG_INFO, "Subsystem PDI Load: Done\n\r");
END:
	Cmd->Response[0] = Status;
	return Status;
}

/*****************************************************************************/
/**
 * @brief contains the array of PLM loader commands
 *
 *****************************************************************************/
static XPlmi_ModuleCmd XLoader_Cmds[] =
{
	XPLMI_MODULE_COMMAND(XLoader_Reserved),
	XPLMI_MODULE_COMMAND(XLoader_LoadSubsystemPdi),
};

/*****************************************************************************/
/**
 * @brief Contains the module ID and loader commands array
 *
 *****************************************************************************/
static XPlmi_Module XPlmi_Loader =
{
	XPLMI_MODULE_LOADER_ID,
	XLoader_Cmds,
	sizeof XLoader_Cmds / sizeof *XLoader_Cmds,
};

/*****************************************************************************/
/**
 * @brief This function registers the PLM Loader commands to the PLMI
 *
 * @param none
 *
 * @return none
 *
 *****************************************************************************/
void XLoader_CmdsInit(void)
{
	XPlmi_ModuleRegister(&XPlmi_Loader);
}
