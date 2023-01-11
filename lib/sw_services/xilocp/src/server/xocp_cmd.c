/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2023, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xocp_cmd.c
*
* This file contains the xilocp IPI handler implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.1   am   12/21/22 Initial release
*       am   01/10/23 Added client side condition for dme ApiId
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_config.h"
#ifdef PLM_OCP
#include "xplmi_hw.h"
#include "xplmi.h"
#include "xplmi_cmd.h"
#include "xplmi_generic.h"
#include "xplmi_modules.h"
#include "xocp_ipihandler.h"
#include "xocp_def.h"
#include "xocp_cmd.h"
#include "xocp_keymgmt.h"

/************************** Constant Definitions *****************************/
static XPlmi_ModuleCmd XOcp_Cmds[XOCP_API_MAX];

static XPlmi_Module XPlmi_Ocp =
{
	XPLMI_MODULE_XILOCP_ID,
	XOcp_Cmds,
	XOCP_API(XOCP_API_MAX),
	NULL,
	NULL,
	NULL
};

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static int XOcp_DevAkInput(XPlmi_Cmd *Cmd);

/************************** Function Definitions ******************************/

/*****************************************************************************/
/**
 * @brief   This function checks for the supported features based on the
 *          requested API ID
 *
 * @param   ApiId - ApiId to check the supported features
 *
 * @return
 *          -XST_SUCCESS - If the requested API ID is supported
 *          -XST_INVALID_PARAM - In case of unsupported API ID
 *
 *****************************************************************************/
static int XOcp_FeaturesCmd(u32 ApiId)
{
	int Status = XST_INVALID_PARAM;

	switch (ApiId) {
		case XOCP_API(XOCP_API_EXTENDPCR):
		case XOCP_API(XOCP_API_GETPCR):
		case XOCP_API(XOCP_API_GETPCRLOG):
		case XOCP_API(XOCP_API_GENDMERESP):
		case XOCP_API(XOCP_API_DEVAKINPUT):
			Status = XST_SUCCESS;
			break;
		default:
			XOcp_Printf(DEBUG_GENERAL, "Cmd not supported\r\n");
			Status = XST_INVALID_PARAM;
			break;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief   This function processes XilOcp IPI commands
 *
 * @param   Cmd - Pointer to the XPlmi_Cmd structure
 *
 * @return
 *          - XST_SUCCESS - On successful IPI processing
 *          - XST_INVALID_PARAM - On invalid command
 *          - Error Code - On Failure
 *
 *****************************************************************************/
static int XOcp_ProcessCmd(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 *Pload = Cmd->Payload;

	if (Pload == NULL) {
		goto END;
	}

	switch (Cmd->CmdId & XOCP_API_ID_MASK) {
		case XOCP_API(XOCP_API_FEATURES):
			Status = XOcp_FeaturesCmd(Pload[0]);
			break;
		case XOCP_API(XOCP_API_EXTENDPCR):
		case XOCP_API(XOCP_API_GETPCR):
		case XOCP_API(XOCP_API_GETPCRLOG):
		case XOCP_API(XOCP_API_GENDMERESP):
			Status = XOcp_IpiHandler(Cmd);
			break;
		case XOCP_API(XOCP_API_DEVAKINPUT):
			Status = XOcp_DevAkInput(Cmd);
			break;
		default:
			XOcp_Printf(DEBUG_GENERAL, "CMD: INVALID PARAM\r\n");
			Status = XST_INVALID_PARAM;
			break;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief   This function registers the XilOcp commands to the PLMI.
 *
 *****************************************************************************/
void XOcp_CmdsInit(void)
{
	u32 Idx;

	/** Register command handlers with XilPlmi */
	for (Idx = 0U; Idx < XPlmi_Ocp.CmdCnt; Idx++) {
		XOcp_Cmds[Idx].Handler = XOcp_ProcessCmd;
	}

	XPlmi_ModuleRegister(&XPlmi_Ocp);
}

/*****************************************************************************/
/**
 * @brief	This function processes XilOcp DEVAK input personalised string and
 *			corresponding subsystem IDs
 *
 * @param	Cmd - Pointer to the XPlmi_Cmd structure
 *
 * @return
 *			- XST_SUCCESS - On successful processing
 *          - Error Code - On Failure
 *
 *****************************************************************************/
static int XOcp_DevAkInput(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 *Pload = Cmd->Payload;

	Status = XOcp_DevAkInputStore(Pload[0], (u8 *)(UINTPTR)&Pload[1]);

	return Status;
}

#endif