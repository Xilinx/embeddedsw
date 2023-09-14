/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpuf_cmd.c
*
* This file contains the xilpuf IPI handler implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kpt  01/04/2022 Initial release
*       bm   07/06/2022 Refactor versal and versal_net code
* 2.1   skg  10/04/2022 Added NULL to hidden handler in Xplmi_module structure
*       skg  12/14/2022 Added invalid command handler in Xplmi_module structure
*       am   02/13/2023 Fixed MISRA C violations
*       am   02/17/2023 Fixed HIS_COMF violations
* 2.2   bm   06/23/2023 Added access permissions for IPI commands
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_config.h"
#ifdef PLM_PUF
#include "xplmi_hw.h"
#include "xplmi.h"
#include "xplmi_cmd.h"
#include "xplmi_generic.h"
#include "xplmi_modules.h"
#include "xpuf_ipihandler.h"
#include "xpuf_defs.h"
#include "xpuf_cmd.h"
#ifndef VERSAL_NET
#include "xplmi_ssit.h"
#endif

/************************** Function Prototypes ******************************/
static int XPuf_InvalidCmdHandler(u32 *Payload, u32 *RespBuf);

/************************** Constant Definitions *****************************/
static XPlmi_ModuleCmd XPuf_Cmds[XPUF_API_MAX];

/* Buffer holding access permissions of puf module commands */
static XPlmi_AccessPerm_t XPuf_AccessPermBuff[XPUF_API_MAX] =
{
	XPLMI_ALL_IPI_FULL_ACCESS(XPUF_API_FEATURES),
	XPLMI_ALL_IPI_FULL_ACCESS(XPUF_PUF_REGISTRATION),
	XPLMI_ALL_IPI_FULL_ACCESS(XPUF_PUF_REGENERATION),
	XPLMI_ALL_IPI_FULL_ACCESS(XPUF_PUF_CLEAR_PUF_ID),
};

static XPlmi_Module XPlmi_Puf =
{
	XPLMI_MODULE_XILPUF_ID,
	XPuf_Cmds,
	XPUF_API(XPUF_API_MAX),
	XPuf_InvalidCmdHandler,
	XPuf_AccessPermBuff,
#ifdef VERSAL_NET
	NULL
#endif
};

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
 * @brief	This function calls the handler for invalid commands
 *
 * @param	Payload  payload data
 * @param   RespBuf buffer to store response of slaves
 *
 * @return
 *			 - XST_SUCCESS  On successful communication
 *			 - error code  On failure
 *
 *****************************************************************************/
static int XPuf_InvalidCmdHandler(u32 *Payload, u32 *RespBuf)
{
	volatile int Status = XST_FAILURE;

#ifndef VERSAL_NET
	Status = XPlmi_SendIpiCmdToSlaveSlr(Payload, RespBuf);
#else
	(void)Payload;
	(void)RespBuf;
	Status = XST_SUCCESS;
#endif

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function checks for the supported features based on the
 * 		requested API ID
 *
 * @param	ApiId	ApiId to check the supported features
 *
 * @return
 *			 - XST_SUCCESS  if the requested API ID is supported
 *			 - XST_INVALID_PARAM  On invalid command
 *
 *****************************************************************************/
static int XPuf_FeaturesCmd(u32 ApiId)
{
	int Status = XST_INVALID_PARAM;

	/**
	 * Check the requested ApiId (registration, regeneration or puf Id),
	 * return XST_SUCCESS for valid ApiId else, reurn XST_INVALID_PARAM if the ApiId is invalid.
	 */
	switch (ApiId) {
		case XPUF_PUF_REGISTRATION:
		case XPUF_PUF_REGENERATION:
		case XPUF_PUF_CLEAR_PUF_ID:
			Status = XST_SUCCESS;
			break;

		default:
			XPuf_Printf(XPUF_DEBUG_GENERAL, "Cmd not supported\r\n");
			break;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function processes XilPuf IPI commands
 *
 * @param	Cmd 	Pointer to the XPlmi_Cmd structure
 *
 * @return
 *			 - XST_SUCCESS  On successful IPI processing
 *			 - XST_INVALID_PARAM  On invalid command
 *			 - Error Code  On Failure
 *****************************************************************************/
static int XPuf_ProcessCmd(XPlmi_Cmd *Cmd)
{
	int Status = XST_INVALID_PARAM;
	const u32 *Pload = Cmd->Payload;

	if (Pload == NULL) {
		goto END;
	}

	/**
	 * Check for requested puf Apid, process the valid ApiId
	 * else, return XST_INVALID_PARAM as error code.
	 */
	switch (Cmd->CmdId & XPUF_API_ID_MASK) {
		case XPUF_API(XPUF_API_FEATURES):
			Status = XPuf_FeaturesCmd(Pload[0U]);
			break;
		case XPUF_API(XPUF_PUF_REGISTRATION):
		case XPUF_API(XPUF_PUF_REGENERATION):
		case XPUF_API(XPUF_PUF_CLEAR_PUF_ID):
			Status = XPuf_IpiHandler(Cmd);
			break;

		default:
			XPuf_Printf(XPUF_DEBUG_GENERAL, "CMD: INVALID PARAM\r\n");
			break;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function registers the XilPuf commands to the PLMI.
 *
 *****************************************************************************/
void XPuf_CmdsInit(void)
{
	u32 Idx;

	/**
	 * Register command handlers of XilPUF with XilPlmi.
	 */
	for (Idx = 0U; Idx < XPlmi_Puf.CmdCnt; Idx++) {
		XPuf_Cmds[Idx].Handler = XPuf_ProcessCmd;
	}
	XPlmi_ModuleRegister(&XPlmi_Puf);
}
#endif
