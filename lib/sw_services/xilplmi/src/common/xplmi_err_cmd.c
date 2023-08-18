/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
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
* 1.05  td   07/08/2021 Fix doxygen warnings
*       bsv  08/02/2021 Code clean up to reduce size
*       ma   08/19/2021 Renamed error related macros
* 1.06  bm   07/06/2022 Refactor versal and versal_net code
*       bm   07/13/2022 Update EAM logic for In-Place PLM Update
* 1.07  skg  10/17/2022 Added Null to invalid cmd handler of xplmi_ErrModule
*       ng   03/30/2023 Updated algorithm and return values in doxygen comments
*       bm   06/23/2023 Added access permissions for IPI commands
*       sk   08/17/2023 Updated EMSetAction function to update subsystemid
*                       while processing IPI request,
*                       Enable EMSetAction process via IPI
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_err_common.h"
#include "xplmi_modules.h"
#include "xplmi.h"
#include "xplmi_debug.h"
#include "xplmi_err.h"

/************************** Constant Definitions *****************************/

/* Error Module command Ids */
#define XPLMI_CMD_ID_EM_FEATURES	(0U)
#define XPLMI_CMD_ID_EM_SET_ACTION	(1U)

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
 * 			module.
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return
 * 			- XST_SUCCESS always.
 *
 *****************************************************************************/
static int XPlmi_CmdEmFeatures(XPlmi_Cmd * Cmd)
{
	int Status = XST_FAILURE;

	XPLMI_EXPORT_CMD(XPLMI_CMD_ID_EM_FEATURES, XPLMI_MODULE_GENERIC_ID,
		XPLMI_MAJOR_MODULE_VERSION, XPLMI_MINOR_MODULE_VERSION);

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
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XPLMI_INVALID_NODE_ID on invalid node ID.
 * 			- XPLMI_INVALID_ERROR_ACTION on invalid error action.
 * 			- XPLMI_CANNOT_CHANGE_ACTION if failed to change error action.
 * 			- XPLMI_LPD_UNINITIALIZED if LPD failed to initialize.
 *
 *****************************************************************************/
static int XPlmi_CmdEmSetAction(XPlmi_Cmd * Cmd)
{
	int Status = XST_FAILURE;

	u32 *IsPsmCrChanged = XPlmi_GetPsmCrState();
	XPlmi_EventType NodeType =
			(XPlmi_EventType)XPlmi_EventNodeType(Cmd->Payload[0U]);
	u32 ErrorAction = Cmd->Payload[1U];
	u32 ErrorMasks = Cmd->Payload[2U];
	u32 PmcPsmCrErrMask = (u32)1U << (u32)XPLMI_ERROR_PMC_PSM_CR;
	u32 PmcPsmNCrErrMask = (u32)1U << (u32)XPLMI_ERROR_PMC_PSM_NCR;
	u32 FwErrMask = (u32)1U << (u32)XPLMI_ERROR_FW_CR;
	u32 PmcPsmCrErrVal = ErrorMasks & PmcPsmCrErrMask;
	u32 PmcPsmNCrErrVal = ErrorMasks & PmcPsmNCrErrMask;
	u32 FwErrVal = ErrorMasks & FwErrMask;
	u32 SubsystemId;

	XPLMI_EXPORT_CMD(XPLMI_CMD_ID_EM_SET_ACTION, XPLMI_MODULE_GENERIC_ID,
		XPLMI_CMD_ARG_CNT_THREE, XPLMI_CMD_ARG_CNT_THREE);

	/* Check if it is an IPI or CDO request*/
	if (Cmd->IpiMask != 0U) {
		SubsystemId = Cmd->SubsystemId;
	} else {
		SubsystemId = XPlmi_GetEmSubsystemId();
	}

	XPlmi_Printf(DEBUG_DETAILED,
		"%s: NodeId: 0x%0x,  ErrorAction: 0x%0x, ErrorMasks: 0x%0x, SubsystemId: 0x%0x\n\r",
		 __func__, Cmd->Payload[0U], ErrorAction, ErrorMasks, SubsystemId);

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
	if (NodeType > XPLMI_NODETYPE_EVENT_SW_ERR) {
		Status = XPLMI_INVALID_NODE_ID;
		goto END;
	}

	if (NodeType == XPLMI_NODETYPE_EVENT_PMC_ERR1) {
		/* Only allow HW error actions for PSM_CR error */
		if (PmcPsmCrErrVal != 0U) {
			if ((XPLMI_EM_ACTION_SUBSYS_SHUTDN == ErrorAction) ||
				(XPLMI_EM_ACTION_SUBSYS_RESTART == ErrorAction)) {
				XPlmi_Printf(DEBUG_GENERAL, "Error: "
					"XPlmi_CmdEmSetAction: Invalid/unsupported"
					" error action 0x%x received for "
					"PMC PSM_CR error", ErrorAction);
				Status = XPLMI_INVALID_ERROR_ACTION;
				goto END;
			}

			if (*IsPsmCrChanged == (u32)TRUE) {
				XPlmi_Printf(DEBUG_GENERAL, "Error: "
					"XPlmi_CmdEmSetAction: PMC PSM_CR error"
					" action cannot be changed more than "
					"once\r\n");
				Status = XPLMI_CANNOT_CHANGE_ACTION;
				goto END;
			}
		}
		/* PMC's PSM_NCR error action must not be changed */
		if ((FwErrVal | PmcPsmNCrErrVal) != 0U) {
			XPlmi_Printf(DEBUG_GENERAL, "Error: XPlmi_CmdEmSetAction: "
				"Error Action cannot be changed for PMC FW CR "
				"and PSM_NCR\r\n");
			Status = XPLMI_CANNOT_CHANGE_ACTION;
			goto END;
		}
	}

	/*
	 * Allow error action setting for PSM errors only if LPD is initialized
	 */
	if ((XPlmi_GetEventIndex(NodeType) ==  XPLMI_NODETYPE_EVENT_PSM_INDEX) &&
		(XPlmi_IsLpdInitialized() != (u8)TRUE)) {
		XPlmi_Printf(DEBUG_GENERAL, "LPD is not initialized to configure "
				"PSM errors and actions\n\r");
		Status = XPLMI_LPD_UNINITIALIZED;
		goto END;
	}

	Status = XPlmi_RestrictErrActions(NodeType, ErrorMasks, ErrorAction);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XPlmi_EmSetAction(Cmd->Payload[0U], ErrorMasks, (u8)ErrorAction, NULL, SubsystemId);
	if(Status != XST_SUCCESS) {
		goto END;
	}

	if ((NodeType == XPLMI_NODETYPE_EVENT_PMC_ERR1) &&
		(PmcPsmCrErrVal  != 0U)) {
		*IsPsmCrChanged = (u32)TRUE;
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
 * @brief	Contains the array of PLM error commands access permissions
 *
 *****************************************************************************/
static XPlmi_AccessPerm_t XPlmi_ErrAccessPermBuff[XPLMI_ARRAY_SIZE(XPlmi_ErrCmds)] =
{
	XPLMI_ALL_IPI_FULL_ACCESS(XPLMI_CMD_ID_EM_FEATURES),
	XPLMI_ALL_IPI_FULL_ACCESS(XPLMI_CMD_ID_EM_SET_ACTION),
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
	XPlmi_ErrAccessPermBuff,
#ifdef VERSAL_NET
	NULL
#endif
};

/*****************************************************************************/
/**
 * @brief	This function registers the PLM error commands to the PLMI.
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
