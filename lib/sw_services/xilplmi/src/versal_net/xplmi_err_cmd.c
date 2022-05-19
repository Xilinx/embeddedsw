/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplmi_err_cmd.c
* @addtogroup xplmi_apis XilPlmi Versal APIs
* @{
* @cond xplmi_internal
* This file contains error management commands code for the PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ====  ==== ======== ======================================================-
* 1.00  kc   02/12/2019 Initial release
* 1.01  kc   08/01/2019 Added error management framework
* 1.02  ma   02/28/2020 Error actions related changes
*       bsv  04/04/2020 Code clean up
* 1.03  bm   10/14/2020 Code clean up
*       ana  10/19/2020 Added doxygen comments
* 1.04  bm   02/18/2021 Added const to XPlmi_ErrCmds
*       ma   03/04/2021 Assign CheckIpiAccessHandler to NULL for EM module
*       pj   03/24/2021 Added support for software error nodes
*       ma   04/05/2021 Added support for error configuration using Error Mask
*                       instead of Error ID. Also, added support to configure
*                       multiple errors at once.
*       ma   05/03/2021 Minor updates related to PSM and FW errors
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_err.h"
#include "xplmi_modules.h"
#include "xplmi.h"
#include "xplmi_debug.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
/*****************************************************************************/
/**
 * @brief	Contains the module ID and PLM error commands array
 *
 *****************************************************************************/
/**
 * @{
 * @cond xplmi_internal
 */
static XPlmi_Module XPlmi_ErrModule;
/**
 * @}
 * @endcond
 */

/*****************************************************************************/
/**
 * @brief	This function is reserved to get the supported features for this
 * module.
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return	Returns XST_SUCCESS always for now
 *
 *****************************************************************************/
static int XPlmi_CmdEmFeatures(XPlmi_Cmd * Cmd)
{
	int Status = XST_FAILURE;

	XPlmi_Printf(DEBUG_DETAILED, "%s %p\n\r", __func__, Cmd);

	if (Cmd->Payload[0U] < XPlmi_ErrModule.CmdCnt) {
		Cmd->Response[1U] = (u32)XST_SUCCESS;
	} else {
		Cmd->Response[1U] = (u32)XST_FAILURE;
	}
	Status = XST_SUCCESS;
	Cmd->Response[0U] = (u32)Status;
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sets the error action as prescribed by the command.
 *			Command payload parameters are
 *				* Error Node ID
 *				* Error Action
 *					0 - Invalid
 *					1 - POR
 *					2 - SRST
 *					3 - Custom(Not supported)
 *					4 - ErrOut
 *					5 - Subsystem Shutdown
 *					6 - Subsystem Restart
 *					7 - None
 *			* Error ID Mask
 * @param	Cmd is pointer to the command structure
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_CmdEmSetAction(XPlmi_Cmd * Cmd)
{
	int Status = XST_FAILURE;
	static u8 IsPsmCrChanged = (u8)FALSE;
	u32 NodeId = Cmd->Payload[0U];
	u32 ErrorAction = Cmd->Payload[1U];
	u32 ErrorMasks = Cmd->Payload[2U];

	XPlmi_Printf(DEBUG_DETAILED,
		"%s: NodeId: 0x%0x,  ErrorAction: 0x%0x, ErrorMasks: 0x%0x\n\r",
		 __func__, NodeId, ErrorAction, ErrorMasks);

	/* Do not allow CUSTOM error action as it is not supported */
	if ((XPLMI_EM_ACTION_CUSTOM == ErrorAction) ||
		(ErrorAction >= XPLMI_EM_ACTION_MAX) ||
		(XPLMI_EM_ACTION_INVALID == ErrorAction)) {
		XPlmi_Printf(DEBUG_GENERAL,
			"Error: XPlmi_CmdEmSetAction: Invalid/unsupported error "
			"action 0x%x received for error mask 0x%x", ErrorAction, ErrorMasks);
		Status = XPLMI_INVALID_ERROR_ACTION;
		goto END;
	}

	/* Do not allow invalid node id */
	if ((NodeId != XPLMI_EVENT_ERROR_PMC_ERR1) &&
		(NodeId != XPLMI_EVENT_ERROR_PMC_ERR2) &&
		(NodeId != XPLMI_EVENT_ERROR_PMC_ERR3) &&
		(NodeId != XPLMI_EVENT_ERROR_PSM_ERR1) &&
		(NodeId != XPLMI_EVENT_ERROR_PSM_ERR2) &&
		(NodeId != XPLMI_EVENT_ERROR_PSM_ERR3) &&
		(NodeId != XPLMI_EVENT_ERROR_PSM_ERR4) &&
		(NodeId != XPLMI_EVENT_ERROR_SW_ERR)) {
		Status = XPLMI_INVALID_NODE_ID;
		goto END;
	}

	/* Only allow HW error actions for PSM_CR error */
	if ((NodeId == XPLMI_EVENT_ERROR_PMC_ERR1) &&
		((ErrorMasks &
			((u32)1U << (u32)XPLMI_NODEIDX_ERROR_PMC_PSM_CR)) != (u32)FALSE)) {
		if ((XPLMI_EM_ACTION_SUBSYS_SHUTDN == ErrorAction) ||
			(XPLMI_EM_ACTION_SUBSYS_RESTART == ErrorAction)) {
			XPlmi_Printf(DEBUG_GENERAL,
				"Error: XPlmi_CmdEmSetAction: Invalid/unsupported error "
				"action 0x%x received for PMC PSM_CR error", ErrorAction);
			Status = XPLMI_INVALID_ERROR_ACTION;
			goto END;
		}
		if (IsPsmCrChanged == (u8)TRUE) {
			XPlmi_Printf(DEBUG_GENERAL,
				"Error: XPlmi_CmdEmSetAction: PMC PSM_CR error action cannot be"
				"changed more than once\r\n");
			Status = XPLMI_CANNOT_CHANGE_ACTION;
			goto END;
		}
	}

	/* PMC's PSM_NCR error action must not be changed */
	if ((NodeId == XPLMI_EVENT_ERROR_PMC_ERR1) &&
		(((ErrorMasks &
			((u32)1U << (u32)XPLMI_NODEIDX_ERROR_FW_CR)) != (u32)FALSE) ||
		((ErrorMasks &
			((u32)1U << (u32)XPLMI_NODEIDX_ERROR_PMC_PSM_NCR)) != (u32)FALSE))) {
		XPlmi_Printf(DEBUG_GENERAL,
			"Error: XPlmi_CmdEmSetAction: Error Action "
			"cannot be changed for PMC FW CR and PSM_NCR\r\n");
		Status = XPLMI_CANNOT_CHANGE_ACTION;
		goto END;
	}

	/*
	 * Allow error action setting for PSM errors only if LPD is initialized
	 */
	if (((NodeId == XPLMI_EVENT_ERROR_PSM_ERR1) ||
		(NodeId == XPLMI_EVENT_ERROR_PSM_ERR2) ||
		(NodeId == XPLMI_EVENT_ERROR_PSM_ERR3) ||
		(NodeId == XPLMI_EVENT_ERROR_PSM_ERR4)) &&
		((LpdInitialized & LPD_INITIALIZED) != LPD_INITIALIZED)) {
		XPlmi_Printf(DEBUG_GENERAL, "LPD is not initialized to configure "
				"PSM errors and actions\n\r");
		Status = XPLMI_LPD_UNINITIALIZED;
		goto END;
	}

	if (((XPlmi_EventNodeType(NodeId) * (u32)XPLMI_MAX_ERR_BITS) ==
		XPLMI_NODEIDX_ERROR_PMX_WWDT) &&
		(ErrorAction > XPLMI_EM_ACTION_ERROUT) &&
		(ErrorAction != XPLMI_EM_ACTION_NONE)) {
		XPlmi_Printf(DEBUG_GENERAL, "Only HW Error Actions are supported for "
				"PMC WDT\n\r");
		Status = XPLMI_INVALID_ERROR_ACTION;
		goto END;
	}

	Status = XPlmi_EmSetAction(NodeId, ErrorMasks, (u8)ErrorAction, NULL);
	if(Status != XST_SUCCESS) {
		goto END;
	}

	if ((NodeId == XPLMI_EVENT_ERROR_PMC_ERR1) &&
		((ErrorMasks &
			((u32)1U << (u32)XPLMI_NODEIDX_ERROR_PMC_PSM_CR)) != (u32)FALSE)) {
		IsPsmCrChanged = (u8)TRUE;
	}

END:
	return Status;
}

/**
 * @{
 * @cond xplmi_internal
 */
/*****************************************************************************/
/**
 * @brief	Contains the array of PLM error commands
 *
 *****************************************************************************/
static const XPlmi_ModuleCmd XPlmi_ErrCmds[] =
{
	XPLMI_MODULE_COMMAND(XPlmi_CmdEmFeatures),
	XPLMI_MODULE_COMMAND(XPlmi_CmdEmSetAction),
};

/*****************************************************************************/
/**
 * @brief	Contains the module ID and PLM error commands array
 *
 *****************************************************************************/
static XPlmi_Module XPlmi_ErrModule =
{
	XPLMI_MODULE_ERROR_ID,
	XPlmi_ErrCmds,
	XPLMI_ARRAY_SIZE(XPlmi_ErrCmds),
	NULL,
	NULL,
};

/*****************************************************************************/
/**
 * @brief	This function registers the PLM error commands to the PLMI.
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_ErrModuleInit(void)
{
	XPlmi_ModuleRegister(&XPlmi_ErrModule);
}

/**
 * @}
 * @endcond
 */

 /** @} */
