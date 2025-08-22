
/******************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplmi_err_psm.c
*
* This file contains error management code which is common for both versal and
* versal net PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ====  ==== ======== ======================================================-
* 1.00  sk   02/20/2025 Initial release
* 2.3   tvp  08/12/2025 ssit is not required for Versal_2vp
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_err_common.h"
#include "xplmi.h"
#include "xplmi_sysmon.h"
#include "xplmi_err.h"
#include "xplmi_plat.h"
#if (!defined(VERSAL_NET) && !defined(VERSAL_2VP))
#include "xplmi_ssit.h"
#endif
#include "xplmi_tamper.h"
#include "xplmi_wdt.h"

/************************** Function Prototypes ******************************/
static void XPlmi_HandlePsmError(u32 ErrorNodeId, u32 RegMask);

/****************************************************************************/
/**
* @brief    This function handles the PSM error routed to PLM.
*
* @param    ErrorNodeId is the node ID for the error event
* @param    RegMask is the register mask of the error received
*
* @return
* 			- None
*
****************************************************************************/
void XPlmi_HandlePsmError(u32 ErrorNodeId, u32 RegMask)
{
	u32 ErrorId;
	XPlmi_EventType ErrorNodeType = (XPlmi_EventType)XPlmi_EventNodeType(ErrorNodeId);
	XPlmi_Error_t *ErrorTable = XPlmi_GetErrorTable();

	ErrorId = XPlmi_GetErrorId(ErrorNodeId, RegMask);

	(void)XPlmi_EmDisable(ErrorNodeId, RegMask);
	switch (ErrorTable[ErrorId].Action) {
	case XPLMI_EM_ACTION_POR:
		XPlmi_PORHandler();
		break;
	case XPLMI_EM_ACTION_SRST:
		XPlmi_SoftResetHandler();
		break;
	case XPLMI_EM_ACTION_ERROUT:
		/*
		 * Clear PSM error and trigger error out using PMC FW_CR error
		 */
		(void)XPlmi_UpdateNumErrOutsCount(XPLMI_UPDATE_TYPE_INCREMENT);
		XPlmi_EmClearError(ErrorNodeType, ErrorId);
		XPlmi_Out32(PMC_GLOBAL_PMC_ERR1_TRIG,
				PMC_GLOBAL_PMC_ERR1_TRIG_FW_CR_MASK);
		XPlmi_Printf(DEBUG_GENERAL, "FW_CR error out is triggered due to "
				"Error ID: 0x%x\r\n", ErrorId);
		break;
	case XPLMI_EM_ACTION_CUSTOM:
	case XPLMI_EM_ACTION_SUBSYS_SHUTDN:
	case XPLMI_EM_ACTION_SUBSYS_RESTART:
	case XPLMI_EM_ACTION_PRINT_TO_LOG:
	case XPLMI_EM_ACTION_SLD:
	case XPLMI_EM_ACTION_SLD_WITH_IO_TRI:
		if (ErrorTable[ErrorId].Handler != NULL) {
			ErrorTable[ErrorId].Handler(ErrorNodeId, RegMask);
		}
		XPlmi_EmClearError(ErrorNodeType, ErrorId);
		break;
	default:
		XPlmi_EmClearError(ErrorNodeType, ErrorId);
		XPlmi_Printf(DEBUG_GENERAL, "Invalid Error Action "
				"for PSM errors. Error ID: 0x%x\r\n", ErrorId);
		break;
	}

	/*
	 * Reset Action in ErrorTable to XPLMI_EM_ACTION_NONE if previous action is
	 * not invalid or custom.
	 */
	if ((ErrorTable[ErrorId].Action != XPLMI_EM_ACTION_INVALID) &&
		(ErrorTable[ErrorId].Action != XPLMI_EM_ACTION_CUSTOM)) {
		ErrorTable[ErrorId].Action = XPLMI_EM_ACTION_NONE;
	}
}


/****************************************************************************/
/**
* @brief    This function is the interrupt handler for PSM Errors.
*
* @param    ErrorNodeId is the node ID for the error event
* @param    RegMask is the register mask of the error received
*
* @return
* 			- None
*
****************************************************************************/
void XPlmi_ErrPSMIntrHandler(u32 ErrorNodeId, u32 RegMask)
{
	u32 ErrStatus[XPLMI_PSM_MAX_ERR_CNT];
	u32 ErrCrMask[XPLMI_PSM_MAX_ERR_CNT];
	u32 ErrNcrMask[XPLMI_PSM_MAX_ERR_CNT];
	u32 ErrMask[XPLMI_PSM_MAX_ERR_CNT];
	u32 Index;
	u32 ErrIndex;
	u32 ErrRegMask;
	XPlmi_Error_t *ErrorTable = XPlmi_GetErrorTable();

	(void)ErrorNodeId;
	(void)RegMask;

	XPlmi_Printf(DEBUG_PRINT_ALWAYS, "PSM EAM Interrupt: ");
	for (Index = 0U; Index < XPLMI_PSM_MAX_ERR_CNT; Index++) {
		ErrStatus[Index] = XPlmi_In32(PSM_GLOBAL_REG_PSM_ERR1_STATUS +
					(Index * PSM_GLOBAL_REG_PSM_ERR_OFFSET));
		ErrCrMask[Index] = XPlmi_In32(PSM_GLOBAL_REG_PSM_CR_ERR1_MASK +
					(Index * PMC_GLOBAL_PSM_ERR_ACTION_OFFSET));
		ErrNcrMask[Index] = XPlmi_In32(PSM_GLOBAL_REG_PSM_NCR_ERR1_MASK +
					(Index * PMC_GLOBAL_PSM_ERR_ACTION_OFFSET));
		ErrMask[Index] = ErrCrMask[Index] & ErrNcrMask[Index];
		XPlmi_Printf_WoTS(DEBUG_PRINT_ALWAYS, "ERR%d: 0x%0x ", (Index + 1U),
					ErrStatus[Index]);
	}
	XPlmi_Printf_WoTS(DEBUG_PRINT_ALWAYS, "\n\r");

	for (ErrIndex = 0U; ErrIndex < XPLMI_PSM_MAX_ERR_CNT; ErrIndex++) {
		if (ErrStatus[ErrIndex] == 0U) {
			continue;
		}
		for (Index = GET_PSM_ERR_START(ErrIndex); Index <
				GET_PSM_ERR_END(ErrIndex); Index++) {
			if (Index >= XPLMI_ERROR_PSMERR_MAX) {
				break;
			}
			ErrRegMask = XPlmi_ErrRegMask(Index);
			if (((ErrStatus[ErrIndex] & ErrRegMask) != (u32)FALSE) &&
				((ErrMask[ErrIndex] & ErrRegMask) == 0x0U) &&
				(ErrorTable[Index].Action != XPLMI_EM_ACTION_NONE)) {
				XPlmi_HandlePsmError(XIL_NODETYPE_EVENT_ERROR_PSM_ERR1 +
					(ErrIndex * XPLMI_EVENT_ERROR_OFFSET),
					ErrRegMask);
			}
		}
	}
}

/*****************************************************************************/
/**
 * @brief	This function disables the PSM error actions for the given mask.
 *
 * @param	RegOffset is the offset for the PSM ERR1 errors
 * @param	RegMask is the register mask of the error to be disabled
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XPLMI_ERROR_ACTION_NOT_DISABLED if error action is not disabled.
 *
 *****************************************************************************/
int XPlmi_EmDisablePsmErrors(u32 RegOffset, u32 RegMask)
{
	u32 Status = (u32)XPLMI_ERROR_ACTION_NOT_DISABLED;

	/** - Disable all PSM error actions. */
	Status = EmDisableErrAction((PSM_GLOBAL_REG_PSM_CR_ERR1_MASK + RegOffset),
			RegMask);
	Status |= EmDisableErrAction((PSM_GLOBAL_REG_PSM_NCR_ERR1_MASK + RegOffset),
			RegMask);

	return (int)Status;
}

/*****************************************************************************/
/**
 * @brief	This function initializes the PSM error actions. Disables all the
 * 			PSM error actions and registers default action.
 *
 * @return
 * 			- XST_SUCCESS always.
 *
*****************************************************************************/
int XPlmi_PsEmInit(void)
{
	int Status = XST_FAILURE;
	u32 Index;
	u32 ErrIndex;
	u32 PsmErrStatus[XPLMI_PSM_MAX_ERR_CNT];
	u32 RegMask;
	XPlmi_Error_t *ErrorTable = XPlmi_GetErrorTable();

	for (ErrIndex = 0U; ErrIndex < XPLMI_PSM_MAX_ERR_CNT; ErrIndex++) {
		/**
		 * - Disable all the Error Actions.
		 */
		(void)XPlmi_EmDisablePsmErrors(GET_PSM_ERR_ACTION_OFFSET(ErrIndex),
				MASK32_ALL_HIGH);
		PsmErrStatus[ErrIndex] = XPlmi_In32(PSM_GLOBAL_REG_PSM_ERR1_STATUS +
					(ErrIndex * PSM_GLOBAL_REG_PSM_ERR_OFFSET));
		XPlmi_Out32(GET_RTCFG_PSM_ERR_ADDR(ErrIndex), PsmErrStatus[ErrIndex]);
		if (PsmErrStatus[ErrIndex] != 0U) {
			XPlmi_Printf(DEBUG_GENERAL, "PSM_GLOBAL_REG_PSM_ERR%d_STATUS: "
				"0x%08x\n\r", ErrIndex + 1U, PsmErrStatus[ErrIndex]);
		}
		/**
		 * - Clear the error status registers.
		 */
		XPlmi_Out32(PSM_GLOBAL_REG_PSM_ERR1_STATUS +
			(ErrIndex * PSM_GLOBAL_REG_PSM_ERR_OFFSET), MASK32_ALL_HIGH);

		/**
		 * - Set the default actions as defined in the Error table.
		 */
		for (Index = GET_PSM_ERR_START(ErrIndex); Index <
				GET_PSM_ERR_END(ErrIndex); Index++) {
			if (Index >= XPLMI_ERROR_PSMERR_MAX) {
				break;
			}
			if (ErrorTable[Index].Action != XPLMI_EM_ACTION_INVALID) {
				RegMask = XPlmi_ErrRegMask(Index);
				if (XPlmi_EmSetAction(XIL_NODETYPE_EVENT_ERROR_PSM_ERR1 +
					(ErrIndex * XPLMI_EVENT_ERROR_OFFSET),
					RegMask, ErrorTable[Index].Action,
					ErrorTable[Index].Handler, ErrorTable[Index].SubsystemId) != XST_SUCCESS) {
					XPlmi_Printf(DEBUG_GENERAL,
						"Warning: XPlmi_PsEmInit: Failed to "
						"set action for PSM ERR%d: %u\r\n",
						Index + 1U, Index);
				}
			}
		}
	}

	Status = XST_SUCCESS;

	return Status;
}
