/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xnvm_cmd.c
*
* This file contains the xilnvm IPI handler implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kal  07/05/2021 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_hw.h"
#include "xplmi.h"
#include "xplmi_cmd.h"
#include "xplmi_generic.h"
#include "xplmi_modules.h"
#include "xnvm_defs.h"
#include "xnvm_bbram_ipihandler.h"
#include "xnvm_cmd.h"

/************************** Function Prototypes ******************************/

/************************** Constant Definitions *****************************/
static XPlmi_ModuleCmd XNvm_Cmds[XNVM_API_MAX];

static XPlmi_Module XPlmi_Nvm =
{
	XPLMI_MODULE_XILNVM_ID,
	XNvm_Cmds,
	XNVM_API(XNVM_API_MAX),
	NULL,
};

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function checks for the supported features based on the
 * 		requested API ID
 *
 * @param	ApiId	ApiId to check the supported features
 *
 * @return 	XST_SUCCESS		if the requested API ID is supported
 * 		XST_INVALID_PARAM	On invalid command
 *
 *****************************************************************************/
static int XNvm_FeaturesCmd(u32 ApiId)
{
	int Status = XST_INVALID_PARAM;

	switch (ApiId) {
	case XNVM_API(XNVM_BBRAM_WRITE_AES_KEY):
	case XNVM_API(XNVM_BBRAM_ZEROIZE):
	case XNVM_API(XNVM_BBRAM_WRITE_USER_DATA):
	case XNVM_API(XNVM_BBRAM_READ_USER_DATA):
	case XNVM_API(XNVM_BBRAM_LOCK_WRITE_USER_DATA):
		Status = XST_SUCCESS;
		break;
	default:
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Cmd not supported\r\n");
		Status = XST_INVALID_PARAM;
		break;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function processes XilNvm IPI commands
 *
 * @param	Cmd 	Pointer to the XPlmi_Cmd structure
 *
 * @return 	XST_SUCCESS		On successful IPI processing
 * 		XST_INVALID_PARAM	On invalid command
 * 		Error Code 		On Failure
 *****************************************************************************/
static int XNvm_ProcessCmd(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 *Pload = Cmd->Payload;

	if (Pload == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	switch (Cmd->CmdId & 0xFFU) {
	case XNVM_API(XNVM_API_FEATURES):
		Status = XNvm_FeaturesCmd(Pload[0]);
		break;
	case XNVM_API(XNVM_BBRAM_WRITE_AES_KEY):
	case XNVM_API(XNVM_BBRAM_ZEROIZE):
	case XNVM_API(XNVM_BBRAM_WRITE_USER_DATA):
	case XNVM_API(XNVM_BBRAM_READ_USER_DATA):
	case XNVM_API(XNVM_BBRAM_LOCK_WRITE_USER_DATA):
		Status = XNvm_BbramIpiHandler(Cmd);
		break;
	default:
		XNvm_Printf(XNVM_DEBUG_GENERAL, "CMD: INVALID PARAM\r\n");
		Status = XST_INVALID_PARAM;
		break;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function registers the XilNvm commands to the PLMI.
 *
 *****************************************************************************/
void XNvm_CmdsInit(void)
{
	u32 Idx;

	/* Register command handlers with XilPlmi */
	for (Idx = 0U; Idx < XPlmi_Nvm.CmdCnt; Idx++) {
		XNvm_Cmds[Idx].Handler = XNvm_ProcessCmd;
	}
	XPlmi_ModuleRegister(&XPlmi_Nvm);
}
