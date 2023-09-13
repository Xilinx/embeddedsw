/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplmi_err_common.c
*
* This file contains error management code which is common for both versal and
* versal net PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ====  ==== ======== ======================================================-
* 1.00  kc   02/12/2019 Initial release
* 1.01  kc   08/01/2019 Added error management framework
*       ma   08/01/2019 Added LPD init code
*       sn   08/03/2019 Added code to wait until over-temperature condition
*						gets resolved before restart
*       bsv  08/29/2019 Added Multiboot and Fallback support
*       scs  08/29/2019 Added support for Extended IDCODE checks
* 1.02  ma   05/02/2020 Remove SRST error action for PSM errors as it is
*                       de-featured
*       ma   02/28/2020 Error actions related changes
*       bsv  04/04/2020 Code clean up
* 1.03  bsv  07/07/2020 Made functions used in single transaltion unit as
*						static
*       kc   08/11/2020 Added disabling and clearing of error which has actions
*                       selected as subsystem shutdown or restart or custom.
*                       They have to be re-enabled again using SetAction
*                       command.
*       bsv  09/21/2020 Set clock source to IRO before SRST for ES1 silicon
*       bm   10/14/2020 Code clean up
*       td   10/19/2020 MISRA C Fixes
* 1.04  td   01/07/2021 Fix warning in PLM memory log regarding NULL handler for
*                       PMC_PSM_NCR error
*       bsv  01/29/2021 Added APIs for checking and clearing NPI errors
* 1.05  pj   03/24/2021 Added API to update Subystem Id of the error node
*       pj   03/24/2021 Added API to trigger error handling from software.
*                       Added SW Error event and Ids for Healthy Boot errors
*       bm   03/24/2021 Added logic to store error status in RTCA
*       bl   04/01/2021 Update function signature for PmSystemShutdown
*       ma   04/05/2021 Added support for error configuration using Error Mask
*                       instead of Error ID. Also, added support to configure
*                       multiple errors at once.
*       ma   05/03/2021 Minor updates related to PSM and FW errors, trigger
*                       FW_CR for PSM and FW errors which have error action
*                       set as ERROR_OUT
*       bsv  05/15/2021 Remove warning for AXI_WRSTRB NPI error
*       ma   05/17/2021 Update only data field when writing error code to FW_ERR
*                       register
*       bm   05/18/2021 Ignore printing and storing of ssit errors for ES1 silicon
*       td   05/20/2021 Fixed blind write on locking NPI address space in
*                       XPlmi_ClearNpiErrors
* 1.06  ma   06/28/2021 Added handler for CPM_NCR error
*       ma   07/08/2021 Fix logic in reading link down error mask value
*       td   07/08/2021 Fix doxygen warnings
*       bsv  07/16/2021 Fix doxygen warnings
*       bsv  07/18/2021 Debug enhancements
*       kc   07/22/2021 XPlmi_PorHandler scope updated to global
*       ma   07/27/2021 Updated print statements in CPM handlers
*       bm   08/02/2021 Change debug log level of PMC error status prints
*       ma   08/06/2021 Save PMC_FW_ERR register value to RTCA and clear it
*       bsv  08/15/2021 Removed unwanted goto statements
*       rv   08/19/2021 Updated XPLMI_EM_ACTION_SUBSYS_RESTART error action
*			handling
*       ma   08/19/2021 Renamed error related macros
*       ma   08/30/2021 Modified XPlmi_ErrMgr function to handle errors in
*                       SSIT Slave SLRs
*       bsv  10/11/2021 Added boundary check before incrementing NumErrOuts
* 1.07  ma   12/17/2021 Clear SSIT_ERR register during EM init
*       bsv  12/24/2021 Move common defines from xilplmi and xilpm to common
*                       folder
*       is   01/10/2022 Updated XPlmi_SysMonOTDetect API to pass wait time arg
*       is   01/10/2022 Updated Copyright Year to 2022
*       ma   01/17/2022 Add exceptions to SW Errors list
*       ma   01/24/2022 Check error mask registers after error action is
*                       enabled or disabled
*       ma   01/24/2022 Check if error action is enabled before executing the
*                       handler
*       ma   02/01/2022 Fix SW-BP-INIT-TO-FAILURE warnings
*       ma   03/10/2022 Fix bug in disabling the error actions for PSM errors
*       is   03/22/2022 Add custom handler for XPPU/XMPU error events
* 1.08  ma   05/10/2022 Added PLM to PLM communication feature
*       ma   06/01/2022 Added PLM Print Log as new error action
*       bsv  06/10/2022 Add CommandInfo to a separate section in elf
*       rama 06/28/2022 Added new entries in ErrorTable to support XilSem
*                       events
*       bm   07/06/2022 Refactor versal and versal_net code
*       ma   07/08/2022 Added support for secure lockdown
*       ma   07/08/2022 Added support for executing secure lockdown when
*                       Halt Boot eFuses are blown
*       ma   07/19/2022 Disable interrupts before secure lockdown
*       kpt  07/21/2022 Replaced secure lockdown code with function
*       bm   07/22/2022 Update EAM logic for In-Place PLM Update
*       bm   07/22/2022 Retain critical data structures after In-Place PLM Update
*       bm   07/20/2022 Shutdown modules gracefully during update
*       bm   07/24/2022 Set PlmLiveStatus during boot time
*       ma   07/28/2022 Update FW_ERR register and return from XPlmi_ErrMgr if
*                       secure lockdown is in progress
*       ma   08/08/2022 Handle EAM errors at task level
*       ma   08/08/2022 Fix SW-BP-MAGIC-NUM warning
*       ma   09/02/2022 Print EAM errors only if they are enabled
*       ma   09/07/2022 Print ERR number as per the register database
* 1.09  bsv  09/30/2022 Make XPlmi_SoftResetHandler non-static so that
*                       it can be used in Image Selector
*       sk   10/27/2022 Updated logic to handle invalid node id
*       ng   11/11/2022 Updated doxygen comments
*       bm   01/03/2023 Remove Triggering of SSIT ERR2 from Slave SLR to
*                       Master SLR
*       bm   01/03/2023 Create Secure Lockdown as a Critical Priority Task
*       bm   01/03/2023 Notify Other SLRs about Secure Lockdown
*       ng   02/07/2023 Check to skip the multiboot reg update and SRST
*       bm   02/09/2023 Added support to return warnings
*       ng   03/12/2023 Fixed Coverity warnings
*		dd   03/28/2023 Updated doxygen comments
*       ng   03/30/2023 Updated algorithm and return values in doxygen comments
* 1.10  bm   06/13/2023 Add API to just log PLM error
*       sk   07/26/2023 Added redundant call for XPlmi_DetectSlaveSlrTamper
*       sk   07/26/2023 Added redundant check in XPlmi_DetectAndHandleTamper
*       sk   08/17/2023 Updated logic to handle SubsystemId for EM actions
*       sk   08/18/2023 Added redundant call for XPlmi_TriggerSLDOnHaltBoot
*       rama 08/30/2023 Changed EAM prints to DEBUG_ALWAYS for debug level_0 option
*       mss  09/04/2023 Added Null Check for EmInit
*       dd   09/12/2023 MISRA-C violation Rule 10.3 fixed
*       dd   09/12/2023 MISRA-C violation Rule 17.7 fixed
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
#ifndef VERSAL_NET
#include "xplmi_ssit.h"
#endif
#include "xplmi_tamper.h"
#include "xplmi_wdt.h"

/**@cond xplmi_internal
 * @{
 */

/************************** Constant Definitions *****************************/
#define XPLMI_UPDATE_TYPE_INCREMENT	(1U)
#define XPLMI_UPDATE_TYPE_DECREMENT	(2U)
#define XPLMI_MAX_ERR_OUTS		(0xFFFFFFFFU)

/* Command IDs supported in CDOs */
#define XPLMI_EM_SET_ACTION_CMD_ID	(1U)

/* BOOT modes */
#define XPLMI_PDI_SRC_JTAG		(0x0U)
#define XPLMI_PDI_SRC_USB		(0x7U)
#define XPLMI_PDI_SRC_SMAP		(0xAU)

/**
 * @}
 * @endcond
 */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static s32 (* PmSystemShutdown)(u32 SubsystemId, const u32 Type, const u32 SubType,
				const u32 CmdType);
static s32 (* PmSubsysRestart)(const u32 SubsystemId);
static void XPlmi_HandlePsmError(u32 ErrorNodeId, u32 RegMask);
static void XPlmi_ErrPSMIntrHandler(u32 ErrorNodeId, u32 RegMask);
static void XPlmi_ErrIntrSubTypeHandler(u32 ErrorNodeId, u32 RegMask);
static void XPlmi_EmClearError(XPlmi_EventType ErrorNodeType, u32 ErrorId);
static void XPlmi_DumpRegisters(void);
static void XPlmi_ErrOutNClearFwCR(u32 ErrorId);
static u32 XPlmi_UpdateNumErrOutsCount(u8 UpdateType);

/************************** Variable Definitions *****************************/
static u32 EmSubsystemId = 0U;

/*****************************************************************************/
/**
 * @brief	This function is called for logging PLM error into FW_ERR register
 *
 * @param	ErrStatus is the error code written to the FW_ERR register
 *
 * @return
 * 			- None
 *
 *****************************************************************************/
void XPlmi_LogPlmErr(int ErrStatus) {
	/** - Print the PLM Warning */
	if ((ErrStatus & XPLMI_WARNING_STATUS_MASK) != XPLMI_WARNING_STATUS_MASK) {
		/* Log PLM Error in FW_ERR register */
		XPlmi_UtilRMW(PMC_GLOBAL_PMC_FW_ERR, PMC_GLOBAL_PMC_FW_ERR_DATA_MASK,
				(u32)ErrStatus);
		/** - Print the PLM Error */
		XPlmi_Printf(DEBUG_PRINT_ALWAYS, "PLM Error Status: 0x%08lx\n\r", ErrStatus);
	}
	else {
		ErrStatus &= ~XPLMI_WARNING_STATUS_MASK;
		XPlmi_Printf(DEBUG_PRINT_ALWAYS, "PLM Warning Status: 0x%08lx\n\r", ErrStatus);
	}
}

/*****************************************************************************/
/**
 * @brief	This function is called in PLM error cases.
 *
 * @param	ErrStatus is the error code written to the FW_ERR register
 *
 * @return
 * 			- None
 *
 *****************************************************************************/
void XPlmi_ErrMgr(int ErrStatus)
{
#ifndef PLM_DEBUG_MODE
	u32 RegVal;
#endif
	u8 SlrType = XPlmi_GetSlrType();
	u32 BootMode;

	/* Log Plm Error status */
	XPlmi_LogPlmErr(ErrStatus);

	/**
	 * - Check if SLR Type is Master or Monolithic
	 * and take error action accordingly.
	 */
	if ((SlrType == XPLMI_SSIT_MASTER_SLR) ||
		(SlrType == XPLMI_SSIT_MONOLITIC)) {
		/**
		 * - Fallback if boot PDI is not done
		 * else just return, so that we receive next requests
		 */
		if (XPlmi_IsLoadBootPdiDone() == FALSE) {

			BootMode = XPlmi_In32(CRP_BOOT_MODE_USER) & CRP_BOOT_MODE_USER_BOOT_MODE_MASK;

			XPlmi_DumpRegisters();
			/**
			 * - If boot mode is jtag, donot reset. This is to keep
			 * the system state intact for further debug.
			 */
#ifndef PLM_DEBUG_MODE
			if(BootMode == XPLMI_PDI_SRC_JTAG)
#endif
			{
				while (TRUE) {
					XPlmi_SetPlmLiveStatus();
				}
			}

#ifndef PLM_DEBUG_MODE
		   /**
		    * - If Halt Boot eFuses are blown, then trigger secure lockdown.
		    * XPlmi_TriggerSLDOnHaltBoot function will run secure lockdown on domains other than PMC
			* and triggers TAMPER_RESP_0 to ROM for running secure lockdown on PMC domain.
			* The function will not return if eFuses are blown.
			* If Halt Boot eFuses are not blown, update multiboot register and trigger FW NCR.
			*/
			XSECURE_REDUNDANT_IMPL(XPlmi_TriggerSLDOnHaltBoot, XPLMI_TRIGGER_TAMPER_IMMEDIATE);

			/**
			 * - Update Multiboot register and perform SRST.
			 * Skip for slave boot modes.
			 */
			if((BootMode == XPLMI_PDI_SRC_USB) || (BootMode == XPLMI_PDI_SRC_SMAP)) {
				/**
				 * - If boot mode is USB or SMAP, PLM should not process
				 *   remaining tasks in slave boot modes.
				*/
				while(TRUE) {
					;
				}
			}
			RegVal = XPlmi_In32(PMC_GLOBAL_PMC_MULTI_BOOT);
			XPlmi_Out32(PMC_GLOBAL_PMC_MULTI_BOOT, RegVal + 1U);

			XPlmi_TriggerFwNcrError();
#endif
		}
	}

	return;
}

/****************************************************************************/
/**
* @brief    This function updates the SubystemId for the given error index.
*
* @param    ErrorNodeId is the node Id for the error event
* @param    ErrorMasks is the Register mask of the Errors
* @param    SubsystemId is the Subsystem ID for the error node.
*
* @return
* 			- None
*
****************************************************************************/
void XPlmi_UpdateErrorSubsystemId(u32 ErrorNodeId,
		u32 ErrorMasks, u32 SubsystemId)
{
	u32 ErrorId = XPlmi_EventNodeType(ErrorNodeId) * (u32)XPLMI_MAX_ERR_BITS;
	u32 ErrMasks = ErrorMasks;
	XPlmi_Error_t *ErrorTable = XPlmi_GetErrorTable();

	for (; ErrMasks != 0U; ErrMasks >>= 1U) {
		if (((ErrMasks & 0x1U) != 0U) &&
			(ErrorId < XPLMI_ERROR_SW_ERR_MAX)) {
			ErrorTable[ErrorId].SubsystemId = SubsystemId;
		}
		ErrorId++;
	}
}

/*****************************************************************************/
/**
 * @brief	This function triggers Power on Reset
 *
 * @return
 * 			- None
 *
 *****************************************************************************/
void XPlmi_PORHandler(void) {
	XPlmi_SysmonClkSetIro();
	XPlmi_UtilRMW(CRP_RST_PS, CRP_RST_PS_PMC_POR_MASK,
		CRP_RST_PS_PMC_POR_MASK);
	while(TRUE) {
		;
	}
}

/*****************************************************************************/
/**
 * @brief	This function returns Error Id for the given error node type and
 * 			error mask.
 *
 * @param	ErrorNodeId is the error node Id.
 * @param	RegMask  is register mask of the error.
 *
 * @return	ErrorId value.
 *
 *****************************************************************************/
u32 XPlmi_GetErrorId(u32 ErrorNodeId, u32 RegMask)
{
	u32 ErrorId = 0U;
	u32 Mask = RegMask;
	u32 ErrorNodeType = XPlmi_EventNodeType(ErrorNodeId);

	while (Mask != (u32)0U) {
		if ((Mask & 0x1U) == 0x1U) {
			break;
		}
		ErrorId++;
		Mask >>= 1U;
	}
	ErrorId += (ErrorNodeType * XPLMI_MAX_ERR_BITS);

	return ErrorId;
}

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
static void XPlmi_HandlePsmError(u32 ErrorNodeId, u32 RegMask)
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
}

/****************************************************************************/
/**
* @brief    This function handles the Software error triggered from within PLM.
*
* @param    ErrorNodeId is the node ID for the error event
* @param    RegMask is the register mask of the error received
*
* @return
* 			- None
*
****************************************************************************/
void XPlmi_HandleSwError(u32 ErrorNodeId, u32 RegMask)
{
	u32 ErrorId = XPlmi_GetErrorId(ErrorNodeId, RegMask);
	XPlmi_Error_t *ErrorTable = XPlmi_GetErrorTable();

	if ((ErrorNodeId == XIL_NODETYPE_EVENT_ERROR_SW_ERR) &&
			(ErrorId < XPLMI_ERROR_SW_ERR_MAX) &&
			(ErrorId >= XPLMI_ERROR_HB_MON_0)) {
		switch (ErrorTable[ErrorId].Action) {
		case XPLMI_EM_ACTION_POR:
			XPlmi_PORHandler();
			break;
		case XPLMI_EM_ACTION_SRST:
			XPlmi_SoftResetHandler();
			break;
		case XPLMI_EM_ACTION_ERROUT:
			/*
			 * Trigger error out using PMC FW_CR error
			 */
			(void)XPlmi_UpdateNumErrOutsCount(XPLMI_UPDATE_TYPE_INCREMENT);
			XPlmi_Out32(PMC_GLOBAL_PMC_ERR1_TRIG,
					PMC_GLOBAL_PMC_ERR1_TRIG_FW_CR_MASK);
			XPlmi_Printf(DEBUG_GENERAL, "FW_CR error out is triggered due to "
					"Error ID: 0x%x\r\n", ErrorId);
			break;
		case XPLMI_EM_ACTION_CUSTOM:
		case XPLMI_EM_ACTION_SUBSYS_SHUTDN:
		case XPLMI_EM_ACTION_SUBSYS_RESTART:
		case XPLMI_EM_ACTION_PRINT_TO_LOG:
			(void)XPlmi_EmDisable(ErrorNodeId, RegMask);
			if (ErrorTable[ErrorId].Handler != NULL) {
				ErrorTable[ErrorId].Handler(ErrorNodeId, RegMask);
			}
			break;
		case XPLMI_EM_ACTION_NONE:
			XPlmi_Printf(DEBUG_GENERAL, "No action is enabled for "
					"Error ID: 0x%x\r\n", ErrorId);
			break;
		default:
			XPlmi_Printf(DEBUG_GENERAL, "Invalid Error Action "
					"for software errors. Error ID: 0x%x\r\n", ErrorId);
			break;
		}
	} else {
		XPlmi_Printf(DEBUG_GENERAL, "Invalid SW Error Node: 0x%x and ErrorId: 0x%x\r\n",
									ErrorNodeId, ErrorId);
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
static void XPlmi_ErrPSMIntrHandler(u32 ErrorNodeId, u32 RegMask)
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

/****************************************************************************/
/**
* @brief    This function is the interrupt handler for Error subtype subsystem
* 			shutdown and subsystem restart.
*
* @param    ErrorNodeId is the node ID for the error event
* @param    RegMask is the register mask of the error received
*
* @return
* 			- None
*
****************************************************************************/
static void XPlmi_ErrIntrSubTypeHandler(u32 ErrorNodeId, u32 RegMask)
{
	int Status = XST_FAILURE;
	u32 ActionId;
	u32 ErrorId = XPlmi_GetErrorId(ErrorNodeId, RegMask);
	XPlmi_Error_t *ErrorTable = XPlmi_GetErrorTable();

	ActionId = ErrorTable[ErrorId].Action;

	switch (ActionId) {
	case XPLMI_EM_ACTION_SUBSYS_SHUTDN:
		XPlmi_Printf(DEBUG_GENERAL, "System shutdown 0x%x\r\n", ErrorTable[ErrorId].SubsystemId);
		Status = (*PmSystemShutdown)(ErrorTable[ErrorId].SubsystemId,
				XPLMI_SUBSYS_SHUTDN_TYPE_SHUTDN, 0U,
				XPLMI_CMD_SECURE);
		break;
	case XPLMI_EM_ACTION_SUBSYS_RESTART:
		XPlmi_Printf(DEBUG_GENERAL, "Subsystem Restart 0x%x\r\n", ErrorTable[ErrorId].SubsystemId);
		Status = (*PmSubsysRestart)(ErrorTable[ErrorId].SubsystemId);
		break;
	default:
		Status = XPLMI_INVALID_ERROR_ACTION;
		break;
	}

	if (XST_SUCCESS != Status) {
		XPlmi_Printf(DEBUG_GENERAL, "Error action 0x%x failed for "
				"Error: 0x%x\r\n", ActionId, ErrorId);
	}
}

/*****************************************************************************/
/**
 * @brief	This function detects and handles tamper condition in EAM IRQ
 * 			interrupt handler
 *
 * @return
 * 			- None
 *
 *****************************************************************************/
static void XPlmi_DetectAndHandleTamper(void)
{
	volatile u32 PmcErr2Status;
	volatile u32 PmcErr2StatusTmp;
	XPlmi_Error_t *ErrorTable = XPlmi_GetErrorTable();

	/** Handle Tamper condition triggered by ROM */
	if (ErrorTable[XPLMI_ERROR_PMCAPB].Handler != NULL) {
		PmcErr2Status = XPlmi_In32(PMC_GLOBAL_PMC_ERR2_STATUS);
		PmcErr2StatusTmp = XPlmi_In32(PMC_GLOBAL_PMC_ERR2_STATUS);
		PmcErr2Status &= XIL_EVENT_ERROR_MASK_PMCAPB;
		PmcErr2StatusTmp &= XIL_EVENT_ERROR_MASK_PMCAPB;
		/** Check if PMC APB error is set */
		if (PmcErr2Status || PmcErr2StatusTmp) {
			/** Disable PMC APB Error */
			(void)XPlmi_EmDisable(XIL_NODETYPE_EVENT_ERROR_PMC_ERR2,
					XIL_EVENT_ERROR_MASK_PMCAPB);
			/** Handle PMC APB Error */
			ErrorTable[XPLMI_ERROR_PMCAPB].Handler(
				XIL_NODETYPE_EVENT_ERROR_PMC_ERR2,
				XIL_EVENT_ERROR_MASK_PMCAPB);
			/** Clear PMC APB Error */
			XPlmi_Out32(PMC_GLOBAL_PMC_ERR2_STATUS, PmcErr2Status);
		}
	}

#ifdef PLM_ENABLE_PLM_TO_PLM_COMM
	/** For SSIT devices, Handle Tamper condition triggered by slave SLRs */
	XSECURE_REDUNDANT_IMPL(XPlmi_DetectSlaveSlrTamper);
#endif
}

/****************************************************************************/
/**
* @brief    This function is default interrupt handler for EAM error which
*           will add the task to the task queue.
*
* @param    CallbackRef is presently the interrupt number that is received
*
* @return
* 			- None
*
****************************************************************************/
void XPlmi_ErrIntrHandler(void *CallbackRef)
{
	XPlmi_TaskNode *Task = NULL;

	(void)CallbackRef;

	/**
	 * - Detect Tamper in interrupt context and trigger the task which
	 * processes the tamper response
	 */
	XPlmi_DetectAndHandleTamper();

	/** - Check if the task is already created */
	Task = XPlmi_GetTaskInstance(XPlmi_ErrorTaskHandler, NULL,
				XPLMI_INVALID_INTR_ID);
	if (Task == NULL) {
		XPlmi_Printf(DEBUG_GENERAL, "EAM task trigger error\n\r");
		goto END;
	}

	/**
	 * - Add the EAM task to the queue and disable interrupts
	 * at IOMODULE level
	 */
	XPlmi_TaskTriggerNow(Task);
	(void)XPlmi_PlmIntrDisable(XPLMI_IOMODULE_ERR_IRQ);

END:
	return;
}

/****************************************************************************/
/**
* @brief    This function is the interrupt handler for the EAM errors.
*
* @param    Data is presently passed as NULL
*
* @return
* 			- XST_SUCCESS always.
*
****************************************************************************/
int XPlmi_ErrorTaskHandler(void *Data)
{
	u32 ErrStatus[XPLMI_PMC_MAX_ERR_CNT];
	u32 ErrIrqMask[XPLMI_PMC_MAX_ERR_CNT];
	u32 ErrIndex;
	u32 Index;
	u32 RegMask;
	XPlmi_Error_t *ErrorTable = XPlmi_GetErrorTable();

	(void)Data;

	for (Index = 0U; Index < XPLMI_PMC_MAX_ERR_CNT; Index++) {
		ErrStatus[Index] = XPlmi_In32(PMC_GLOBAL_PMC_ERR1_STATUS +
					(Index * PMC_GLOBAL_REG_PMC_ERR_OFFSET));
		ErrIrqMask[Index] = XPlmi_In32(GET_PMC_IRQ_MASK(GET_PMC_ERR_ACTION_OFFSET(Index)));
		if ((ErrStatus[Index] & ~ErrIrqMask[Index]) != 0x0U) {
			XPlmi_Printf(DEBUG_PRINT_ALWAYS, "PMC EAM ERR%d: 0x%0x\r\n", (Index + 1U),
					ErrStatus[Index]);
		}
	}

	/**
	 * - Interrupt is selected as response for Custom, subsystem shutdown
	 * and subsystem restart actions. For these actions, error will be
	 * disabled. Agent should clear the source and enable the error again
	 * using SetAction. In SetAction, error will be cleared and enabled.
	 * For subsystem cases, during subsystem restart, error will be enabled
	 * again.
	 */

	for (ErrIndex = 0U; ErrIndex < XPLMI_PMC_MAX_ERR_CNT; ErrIndex++) {
		if (ErrStatus[ErrIndex] == 0U) {
			continue;
		}
		for (Index = GET_PMC_ERR_START(ErrIndex); Index <
				GET_PMC_ERR_END(ErrIndex); Index++) {
			if (Index >= XPLMI_ERROR_PMCERR_MAX) {
				break;
			}
			RegMask = XPlmi_ErrRegMask(Index);
			if (((ErrStatus[ErrIndex] & RegMask) != (u32)FALSE) &&
				((ErrIrqMask[ErrIndex] & RegMask) == 0x0U) &&
					((ErrorTable[Index].Handler != NULL) ||
					(Index == XPLMI_ERROR_PMC_PSM_NCR)) &&
					(ErrorTable[Index].Action != XPLMI_EM_ACTION_NONE)) {
				/* PSM errors are handled in PsmErrHandler */
				if (Index != XPLMI_ERROR_PMC_PSM_NCR) {
					(void)XPlmi_EmDisable(XIL_NODETYPE_EVENT_ERROR_PMC_ERR1 +
						(ErrIndex * XPLMI_EVENT_ERROR_OFFSET), RegMask);
					ErrorTable[Index].Handler(XIL_NODETYPE_EVENT_ERROR_PMC_ERR1 +
						(ErrIndex * XPLMI_EVENT_ERROR_OFFSET), RegMask);
				}
				else {
					XPlmi_ErrPSMIntrHandler(XIL_NODETYPE_EVENT_ERROR_PMC_ERR1 +
						(ErrIndex * XPLMI_EVENT_ERROR_OFFSET), RegMask);
				}
				XPlmi_EmClearError((XPlmi_EventType)(XPLMI_NODETYPE_EVENT_PMC_ERR1 + ErrIndex),
						Index);
			}
		}
	}

	/**
	 * - Clear and enable EAM errors at IOMODULE level
	 */
	(void)XPlmi_PlmIntrClear(XPLMI_IOMODULE_ERR_IRQ);
	XPlmi_PlmIntrEnable(XPLMI_IOMODULE_ERR_IRQ);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* @brief    This function is the interrupt handler for Error action "Print
*           to Log"
*
* @param    ErrorNodeId is the node ID for the error event
* @param    RegMask is the register mask of the error received
*
* @return
* 			- None
*
****************************************************************************/
void XPlmi_ErrPrintToLog(u32 ErrorNodeId, u32 RegMask)
{
	u32 ErrorId = XPlmi_GetErrorId(ErrorNodeId, RegMask);

	/** - Print NodeId, Mask and Error ID information of the error received. */
	XPlmi_Printf(DEBUG_PRINT_ALWAYS, "Received EAM error. ErrorNodeId: 0x%x,"
			" Register Mask: 0x%x. The corresponding Error ID: 0x%x\r\n",
			ErrorNodeId, RegMask, ErrorId);
}

/*****************************************************************************/
/**
 * @brief	This function clears any previous errors before enabling them.
 *
 * @param	ErrorNodeType is the node type for the error event
 * @param	ErrorId is the index of the error to be cleared
 *
 * @return
 * 			- None
 *
 *****************************************************************************/
static void XPlmi_EmClearError(XPlmi_EventType ErrorNodeType, u32 ErrorId)
{
	u32 RegMask = XPlmi_ErrRegMask(ErrorId);
	u32 Index;
	XPlmi_Error_t *ErrorTable = XPlmi_GetErrorTable();

	switch (XPlmi_GetEventIndex(ErrorNodeType)) {
	case XPLMI_NODETYPE_EVENT_PMC_INDEX:
		Index = (u32)ErrorNodeType - (u32)XPLMI_NODETYPE_EVENT_PMC_ERR1;
		/* Clear previous errors */
		XPlmi_Out32(PMC_GLOBAL_PMC_ERR1_STATUS +
			(Index * PMC_GLOBAL_REG_PMC_ERR_OFFSET), RegMask);
		break;
	case XPLMI_NODETYPE_EVENT_PSM_INDEX:
		Index = (u32)ErrorNodeType - (u32)XPLMI_NODETYPE_EVENT_PSM_ERR1;
		/* Clear previous errors */
		XPlmi_Out32(PSM_GLOBAL_REG_PSM_ERR1_STATUS +
			(Index * PSM_GLOBAL_REG_PSM_ERR_OFFSET), RegMask);
		if (ErrorTable[XPLMI_ERROR_PMC_PSM_CR].Action !=
				XPLMI_EM_ACTION_ERROUT) {
			XPlmi_ErrOutNClearFwCR(ErrorId);
		}
		break;
	case XPLMI_NODETYPE_EVENT_SW_INDEX:
		XPlmi_ErrOutNClearFwCR(ErrorId);
		break;
	default:
		/* Invalid Error Type */
		XPlmi_Printf(DEBUG_GENERAL,
			"Invalid ErrType 0x%x for Error Mask: 0x%0x\n\r",
			ErrorNodeType, RegMask);
		break;
	}
}

/*****************************************************************************/
/**
 * @brief	This function disables the error action for the given error mask.
 *
 * @param	ErrMaskRegAddr is the error action mask register address
 * @param	RegMask is the register mask of the error to be disabled
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XPLMI_ERROR_ACTION_NOT_DISABLED if error action is not disabled.
 *
 *****************************************************************************/
u32 EmDisableErrAction(u32 ErrMaskRegAddr, u32 RegMask)
{
	int Status = XPLMI_ERROR_ACTION_NOT_DISABLED;

	/**
	 * - Disable error action.
	 */
	XPlmi_Out32((ErrMaskRegAddr + PMC_PSM_DIS_REG_OFFSET), RegMask);
	/**
	 * - Check if the error action is disabled.
	 */
	if ((XPlmi_In32(ErrMaskRegAddr) & RegMask) == RegMask) {
		Status = XST_SUCCESS;
	}

	return (u32)Status;
}

/*****************************************************************************/
/**
 * @brief	This function enables the error action for the given error mask.
 *
 * @param	ErrMaskRegAddr is the error action mask register address
 * @param	RegMask is the register mask of the error to be disabled
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XPLMI_ERROR_ACTION_NOT_ENABLED if error action is not enabled.
 *
 *****************************************************************************/
static int EmEnableErrAction(u32 ErrMaskRegAddr, u32 RegMask)
{
	int Status = XPLMI_ERROR_ACTION_NOT_ENABLED;

	/* Enable the error action */
	XPlmi_Out32((ErrMaskRegAddr + PMC_PSM_EN_REG_OFFSET), RegMask);
	/* Check if the error action is enabled */
	if ((XPlmi_In32(ErrMaskRegAddr) & RegMask) == 0x0U) {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function disables the PMC error actions for the given mask.
 *
 * @param	RegOffset is the offset for the PMC ERR, POR ,IRQ mask,SRST mask
 * @param	RegMask is the register mask of the error to be disabled
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XPLMI_ERROR_ACTION_NOT_DISABLED if error action is not disabled.
 *
 *****************************************************************************/
int XPlmi_EmDisablePmcErrors(u32 RegOffset, u32 RegMask)
{
	u32 Status = (u32)XPLMI_ERROR_ACTION_NOT_DISABLED;

	/** - Disable all PMC error actions. */
	Status = EmDisableErrAction(GET_PMC_ERR_OUT_MASK(RegOffset), RegMask);
	Status |= EmDisableErrAction(GET_PMC_POR_MASK(RegOffset), RegMask);
	Status |= EmDisableErrAction(GET_PMC_IRQ_MASK(RegOffset), RegMask);
	Status |= EmDisableErrAction(GET_PMC_SRST_MASK(RegOffset), RegMask);

	return (int)Status;
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
 * @brief	This function disables the responses for the given error.
 *
 * @param	ErrorNodeId is the node Id for the error event
 * @param	RegMask is the register mask of the error to be disabled
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XPLMI_ERROR_ACTION_NOT_DISABLED if error action is not disabled.
 * 			- XPLMI_INVALID_ERROR_TYPE on invalid error type.
 *
 *****************************************************************************/
int XPlmi_EmDisable(u32 ErrorNodeId, u32 RegMask)
{
	int Status = XPLMI_ERROR_ACTION_NOT_DISABLED;
	XPlmi_EventType ErrorNodeType = (XPlmi_EventType)XPlmi_EventNodeType(ErrorNodeId);
	u32 Index;

	switch (XPlmi_GetEventIndex(ErrorNodeType)) {
	case XPLMI_NODETYPE_EVENT_PMC_INDEX:
		Index = (u32)ErrorNodeType - (u32)XPLMI_NODETYPE_EVENT_PMC_ERR1;
		/**
		 * - For XPLMI_NODETYPE_ErrorNodeTypeEVENT_PMC_INDEX - Disable POR, SRST,
		 *   Interrupt and PS Error Out.
		 */
		Status = XPlmi_EmDisablePmcErrors(
				GET_PMC_ERR_ACTION_OFFSET(Index), RegMask);
		break;
	case XPLMI_NODETYPE_EVENT_PSM_INDEX:
		Index = (u32)ErrorNodeType - (u32)XPLMI_NODETYPE_EVENT_PSM_ERR1;
		/**
		 * - For XPLMI_NODETYPE_EVENT_PSM_INDEX - Disable CR / NCR to PMC,
		 *   Interrupt.
		 */
		Status = XPlmi_EmDisablePsmErrors(
				GET_PSM_ERR_ACTION_OFFSET(Index), RegMask);
		break;
	case XPLMI_NODETYPE_EVENT_SW_INDEX:
		/**
		 * - For XPLMI_NODETYPE_EVENT_SW_INDEX - Do nothing.
		 */
		Status = XST_SUCCESS;
		break;
	default:
		/** - For invalid error return XPLMI_INVALID_ERROR_TYPE error code. */
		Status = XPLMI_INVALID_ERROR_TYPE;
		XPlmi_Printf(DEBUG_GENERAL,
			"Invalid ErrType 0x%x for Error Mask: 0x%0x\n\r",
			ErrorNodeId, RegMask);
		break;
	}

	return Status;
}


/*****************************************************************************/
/**
 * @brief	This function enables HW error actions for the given Error.
 *
 * @param	ErrorNodeType is the node type for the error event
 * @param	RegMask is the register mask of the error to be enabled
 * @param	Action is the HW Error action of the error to be enabled
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XPLMI_ERROR_ACTION_NOT_ENABLED if error action is not enabled.
 * 			- XPLMI_INVALID_ERROR_ACTION on invalid error action.
 * 			- XPLMI_INVALID_ERROR_TYPE on invalid error type.
 *
 *****************************************************************************/
static int XPlmi_EmEnableAction(XPlmi_EventType ErrorNodeType, u32 RegMask, u8 Action)
{
	int Status = XPLMI_ERROR_ACTION_NOT_ENABLED;
	u32 Index;
	u32 PmcActionMask;
	u8 IrqAction = (u8)FALSE;
	XPlmi_Error_t *ErrorTable = XPlmi_GetErrorTable();

	switch (Action) {
	case XPLMI_EM_ACTION_POR:
		PmcActionMask = PMC_GLOBAL_PMC_POR1_MASK;
		break;
	case XPLMI_EM_ACTION_SRST:
		PmcActionMask = PMC_GLOBAL_PMC_SRST1_MASK;
		break;
	case XPLMI_EM_ACTION_ERROUT:
		PmcActionMask = PMC_GLOBAL_PMC_ERR_OUT1_MASK;
		break;
	case XPLMI_EM_ACTION_CUSTOM:
	case XPLMI_EM_ACTION_SUBSYS_SHUTDN:
	case XPLMI_EM_ACTION_SUBSYS_RESTART:
	case XPLMI_EM_ACTION_PRINT_TO_LOG:
		PmcActionMask = PMC_GLOBAL_PMC_IRQ1_MASK;
		IrqAction = (u8)TRUE;
		break;
	default:
		Status = XPLMI_INVALID_ERROR_ACTION;
		break;
	}

	if (Status == XPLMI_INVALID_ERROR_ACTION) {
		goto END;
	}

	switch (XPlmi_GetEventIndex(ErrorNodeType)) {
	case XPLMI_NODETYPE_EVENT_PMC_INDEX:
		/* Enable error action for given PMC error mask */
		Index = (u32)ErrorNodeType - (u32)XPLMI_NODETYPE_EVENT_PMC_ERR1;
		Status = EmEnableErrAction(GET_PMC_ERR_ACTION_ADDR(PmcActionMask,
					Index), RegMask);
		break;
	case XPLMI_NODETYPE_EVENT_PSM_INDEX:
		Index = (u32)ErrorNodeType - (u32)XPLMI_NODETYPE_EVENT_PSM_ERR1;
		/*
		 * If PMC PSM CR error action is POR or SRST or ERROUT, set
		 * the error action for the given error as PSM CR to handle
		 * the action by HW. Otherwise, set it as PSM NCR to handle
		 * the action by PLM.
		 */
		if ((ErrorTable[XPLMI_ERROR_PMC_PSM_CR].Action == Action)
				&& (IrqAction == (u8)FALSE)) {
			/* Enable PSM CR error action for given error mask */
			Status = EmEnableErrAction(PSM_GLOBAL_REG_PSM_CR_ERR1_MASK +
				GET_PSM_ERR_ACTION_OFFSET(Index), RegMask);
		} else {
			/* Enable PSM NCR error action for given error mask */
			Status = EmEnableErrAction(PSM_GLOBAL_REG_PSM_NCR_ERR1_MASK+
				GET_PSM_ERR_ACTION_OFFSET(Index), RegMask);
		}
		break;
	case XPLMI_NODETYPE_EVENT_SW_INDEX:
		/* Do nothing */
		Status = XST_SUCCESS;
		break;
	default:
		Status = XPLMI_INVALID_ERROR_TYPE;
		XPlmi_Printf(DEBUG_GENERAL,
			"Invalid ErrType 0x%x for Error Mask: 0x%0x\n\r",
			ErrorNodeType, RegMask);
		break;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function configures the Action specified for a given Error ID.
 *
 * @param	NodeType is the error node type
 * @param	ErrorId is the index of the error to which given action to be set
 * @param	ActionId is the action that need to be set for ErrorId. Action
 * 		  	can be SRST/POR/ERR OUT/INT
 * @param	ErrorHandler If INT is defined as response, handler should be
 * 		  	defined.
 * @param       SubsystemId
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlmi_EmConfig(XPlmi_EventType NodeType, u32 ErrorId, u8 ActionId,
		XPlmi_ErrorHandler_t ErrorHandler, const u32 SubsystemId)
{
	int Status = XST_FAILURE;
	u32 RegMask = XPlmi_ErrRegMask(ErrorId);
	XPlmi_Error_t *ErrorTable = XPlmi_GetErrorTable();

	/**
	 * - Set error action for given error Id.
	 */
	ErrorTable[ErrorId].Action = ActionId;
	if (ActionId == XPLMI_EM_ACTION_CUSTOM) {
		ErrorTable[ErrorId].Handler = ErrorHandler;
	}
	else if (ActionId == XPLMI_EM_ACTION_PRINT_TO_LOG) {
		ErrorTable[ErrorId].Handler = XPlmi_ErrPrintToLog;
	}
	else if ((ActionId == XPLMI_EM_ACTION_SUBSYS_SHUTDN) ||
		(ActionId == XPLMI_EM_ACTION_SUBSYS_RESTART)) {
		ErrorTable[ErrorId].SubsystemId = SubsystemId;
		ErrorTable[ErrorId].Handler = XPlmi_ErrIntrSubTypeHandler;
	}

	/**
	 * - Enable the error action.
	 */
	if (ActionId != XPLMI_EM_ACTION_NONE) {
		Status = XPlmi_EmEnableAction(NodeType, RegMask, ActionId);
	}
	else {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sets the Action specified for a given Error Masks.
 *
 * @param	ErrorNodeId is the node ID for the error event
 * @param	ErrorMasks is the error masks to which specified action to be set
 * @param	ActionId is the action that need to be set for ErrorMasks. Action
 * 		  	can be SRST/POR/ERR OUT/INT
 * @param	ErrorHandler If INT is defined as response, handler should be
 * 		  	defined.
 * @param       SubsystemId
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XPLMI_INVALID_NODE_ID on invalid node ID.
 * 			- XPLMI_INVALID_ERROR_ID on invalid error ID.
 * 			- XPLMI_INVALID_ERROR_HANDLER on invalid error handler.
 *
 *****************************************************************************/
int XPlmi_EmSetAction(u32 ErrorNodeId, u32 ErrorMasks, u8 ActionId,
		XPlmi_ErrorHandler_t ErrorHandler, const u32 SubsystemId)
{
	int Status = XST_FAILURE;
	XPlmi_EventType NodeType = (XPlmi_EventType)XPlmi_EventNodeType(ErrorNodeId);
	u32 ErrorId = (u32)NodeType * (u32)XPLMI_MAX_ERR_BITS;
	u32 RegMask;
	u32 ErrMasks = ErrorMasks;
	XPlmi_Error_t *ErrorTable = XPlmi_GetErrorTable();
	XPLMI_EXPORT_CMD(XPLMI_EM_SET_ACTION_CMD_ID, XPLMI_MODULE_ERROR_ID,
		XPLMI_CMD_ARG_CNT_THREE, XPLMI_CMD_ARG_CNT_THREE);

	if ((ErrorNodeId  & (~XPLMI_NODE_TYPE_MASK)) != XIL_NODETYPE_EVENT_ERROR_ID_ENCODING) {
		Status = XPLMI_INVALID_NODE_ID;
		goto END1;
	}

	for ( ; ErrMasks != 0U; ErrMasks >>= 1U) {
		if ((ErrMasks & 0x1U) == 0U) {
			goto END;
		}
		RegMask = XPlmi_ErrRegMask(ErrorId);

		/**
		 * - Check for Valid Error ID.
		 */
		if ((ErrorId >= XPLMI_ERROR_SW_ERR_MAX) ||
				(ErrorTable[ErrorId].Action == XPLMI_EM_ACTION_INVALID)) {
			/* Invalid Error Id */
			Status = XPLMI_INVALID_ERROR_ID;
			XPlmi_Printf(DEBUG_GENERAL,
					"Invalid Error: 0x%0x\n\r", ErrorId);
			goto END;
		}

		/**
		 * - Check for Valid handler.
		 */
		if((XPLMI_EM_ACTION_CUSTOM == ActionId) && (NULL == ErrorHandler) &&
				(XPLMI_ERROR_PMC_PSM_NCR != ErrorId)) {
			/* Null handler */
			Status = XPLMI_INVALID_ERROR_HANDLER;
			XPlmi_Printf(DEBUG_GENERAL, "Invalid Error Handler \n\r");
			goto END;
		}

		if((ActionId > XPLMI_EM_ACTION_INVALID) &&
				(ActionId < XPLMI_EM_ACTION_MAX)) {
			/**
			 * - Disable the error actions for Error ID for configuring
			 * the requested error action.
			 */
			Status = XPlmi_EmDisable(ErrorNodeId, RegMask);
			if (XST_SUCCESS != Status) {
				/* Error action disabling failure */
				goto END;
			}
			/**
			 * - Clear any previous errors.
			 */
			XPlmi_EmClearError(NodeType, ErrorId);
		}

		/**
		 * - Configure the Error Action to given Error Id.
		 */
		Status = XPlmi_EmConfig(NodeType, ErrorId, ActionId, ErrorHandler, SubsystemId);

END:
		++ErrorId;
	}
END1:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function initializes the error module. Disables all the error
 * 			actions and registers default action.
 *
 * @param	SystemShutdown is the pointer to the PM system shutdown
 *		callback handler for action subtype system shutdown
 * @param	SubsystemRestart is pointer to the PM subsystem restart
 *		with CPU idle support handler for action subtype restart
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlmi_EmInit(XPlmi_ShutdownHandler_t SystemShutdown,
		  XPlmi_RestartHandler_t SubsystemRestart)
{
	int Status = XST_FAILURE;
	u32 Index;
	u32 PmcErrStatus[XPLMI_PMC_MAX_ERR_CNT];
	u32 FwErr;
	u32 RegMask;
	u32 ErrIndex;
	XPlmi_Error_t *ErrorTable = XPlmi_GetErrorTable();
	XPlmi_TaskNode *Task = NULL;

	/** Checking both Arguments for NULL, If any one of it found to be NULL Major Error Code will be returned */
	if (SystemShutdown == NULL || SubsystemRestart == NULL) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_EMINIT_INVALID_PARAM, Status);
		goto END;
	}

	/* Check if the task is already created */
	Task = XPlmi_GetTaskInstance(XPlmi_ErrorTaskHandler, NULL,
				XPLMI_INVALID_INTR_ID);
	if (Task == NULL) {
		/* Create task if it is not already created */
		Task = XPlmi_TaskCreate(XPLM_TASK_PRIORITY_0, XPlmi_ErrorTaskHandler, NULL);
		Task->IntrId = XPLMI_INVALID_INTR_ID;
	}

	/** - Register Error module commands */
	XPlmi_ErrModuleInit();

	PmSystemShutdown = SystemShutdown;
	PmSubsysRestart = SubsystemRestart;

	/* In-Place Update is applicable only for versal_net */
	if (XPlmi_IsPlmUpdateDone() == (u8)TRUE) {
		/* Reconfigure error actions after the update */
		XPlmi_ReconfigErrActions();
		/* Re-register Tamper Handler */
		Status = XPlmi_RegisterTamperIntrHandler();
		goto END;
	}

	/** - Detect if we are in over-temperature condition */
	XPlmi_SysMonOTDetect(XPLMI_SYSMON_NO_WAIT_TIME);

	/** - Clear SSIT_ERR register to stop error propagation to other SLRs */
	XPlmi_Out32(PMC_GLOBAL_SSIT_ERR, 0x0U);

	/** - Save FW_ERR register value to RTCA and clear it */
	FwErr = XPlmi_In32(PMC_GLOBAL_PMC_FW_ERR);
	XPlmi_Out32(XPLMI_RTCFG_PMC_FW_ERR_VAL_ADDR, FwErr);
	XPlmi_Out32(PMC_GLOBAL_PMC_FW_ERR, 0x0U);

	for (ErrIndex = 0U; ErrIndex < XPLMI_PMC_MAX_ERR_CNT; ErrIndex++) {
		/** - Disable all the Error Actions */
		(void)XPlmi_EmDisablePmcErrors(GET_PMC_ERR_ACTION_OFFSET(ErrIndex),
				MASK32_ALL_HIGH);
		PmcErrStatus[ErrIndex] = XPlmi_In32(PMC_GLOBAL_PMC_ERR1_STATUS +
					(ErrIndex * PMC_GLOBAL_REG_PMC_ERR_OFFSET));
		/** - Ignore SSIT Errors for Versal ES1 silicon */
		XPlmi_ClearSsitErrors(PmcErrStatus, ErrIndex);
		XPlmi_Out32(GET_RTCFG_PMC_ERR_ADDR(ErrIndex), PmcErrStatus[ErrIndex]);
		if (PmcErrStatus[ErrIndex] != 0U) {
			XPlmi_Printf(DEBUG_INFO, "PMC_GLOBAL_PMC_ERR%d_STATUS: "
				"0x%08x\n\r", ErrIndex + 1U, PmcErrStatus[ErrIndex]);
		}
		/** - Clear the error status registers */
		XPlmi_Out32(PMC_GLOBAL_PMC_ERR1_STATUS +
			(ErrIndex * PMC_GLOBAL_REG_PMC_ERR_OFFSET), MASK32_ALL_HIGH);

		for (Index = GET_PMC_ERR_START(ErrIndex); Index <
				GET_PMC_ERR_END(ErrIndex); Index++) {
			if (Index >= XPLMI_ERROR_PMCERR_MAX) {
				break;
			}
			if (ErrorTable[Index].Action == XPLMI_EM_ACTION_INVALID) {
				continue;
			}
			RegMask = XPlmi_ErrRegMask(Index);
			Status = XPlmi_EmSetAction(XIL_NODETYPE_EVENT_ERROR_PMC_ERR1 +
					(ErrIndex * XPLMI_EVENT_ERROR_OFFSET),
					RegMask, ErrorTable[Index].Action,
					ErrorTable[Index].Handler, ErrorTable[Index].SubsystemId);
			if (Status != XST_SUCCESS) {
				XPlmi_Printf(DEBUG_GENERAL,
					"Warning: XPlmi_EmInit: Failed to "
					"set action for PMC ERR%d: %u\r\n",
					ErrIndex + 1U, Index);
				goto END;
			}
		}
	}

	/** - Register Tamper Interrupt Handler */
	Status = XPlmi_RegisterTamperIntrHandler();

END:
	return Status;
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

/*****************************************************************************/
/**
 * @brief	This function dumps the registers which can help debugging.
 *
 * @return
 * 			- None
 *
 *****************************************************************************/
static void XPlmi_DumpRegisters(void)
{
	XPlmi_Printf(DEBUG_PRINT_ALWAYS, "============Register Dump============\n\r");

	XPlmi_Printf(DEBUG_PRINT_ALWAYS, "PMC_TAP_IDCODE: 0x%08x\n\r",
		XPlmi_In32(PMC_TAP_IDCODE));
	XPlmi_Printf(DEBUG_PRINT_ALWAYS, "EFUSE_CACHE_IP_DISABLE_0(EXTENDED IDCODE): "
			"0x%08x\n\r",
		XPlmi_In32(EFUSE_CACHE_IP_DISABLE_0));
	XPlmi_Printf(DEBUG_PRINT_ALWAYS, "PMC_TAP_VERSION: 0x%08x\n\r",
		XPlmi_In32(PMC_TAP_VERSION));
	XPlmi_Printf(DEBUG_PRINT_ALWAYS, "CRP_BOOT_MODE_USER: 0x%08x\n\r",
		XPlmi_In32(CRP_BOOT_MODE_USER));
	XPlmi_Printf(DEBUG_PRINT_ALWAYS, "CRP_BOOT_MODE_POR: 0x%08x\n\r",
		XPlmi_In32(CRP_BOOT_MODE_POR));
	XPlmi_Printf(DEBUG_PRINT_ALWAYS, "CRP_RESET_REASON: 0x%08x\n\r",
		XPlmi_In32(CRP_RESET_REASON));
	XPlmi_Printf(DEBUG_PRINT_ALWAYS, "PMC_GLOBAL_PMC_MULTI_BOOT: 0x%08x\n\r",
		XPlmi_In32(PMC_GLOBAL_PMC_MULTI_BOOT));
	XPlmi_Printf(DEBUG_PRINT_ALWAYS, "PMC_GLOBAL_PWR_STATUS: 0x%08x\n\r",
		XPlmi_In32(PMC_GLOBAL_PWR_STATUS));
	XPlmi_Printf(DEBUG_PRINT_ALWAYS, "PMC_GLOBAL_PMC_GSW_ERR: 0x%08x\n\r",
		XPlmi_In32(PMC_GLOBAL_PMC_GSW_ERR));
	XPlmi_Printf(DEBUG_PRINT_ALWAYS, "PMC_GLOBAL_PLM_ERR: 0x%08x\n\r",
		XPlmi_In32(PMC_GLOBAL_PLM_ERR));
	XPlmi_DumpErrNGicStatus();
	XPlmi_Printf(DEBUG_PRINT_ALWAYS, "============Register Dump============\n\r");
}

/*****************************************************************************/
/**
 * @brief	This function sets clock source to IRO for ES1 silicon and resets
 * the device.
 *
 * @return
 * 			- None
 *
 *****************************************************************************/
void XPlmi_SoftResetHandler(void)
{
	XPlmi_SysmonClkSetIro();
	/* Make sure every thing completes */
	DATA_SYNC;
	INST_SYNC;
	XPlmi_Out32(CRP_RST_PS, CRP_RST_PS_PMC_SRST_MASK);
	while (TRUE) {
		;
	}
}

/*****************************************************************************/
/**
 * @brief	This function sets clock source to IRO for ES1 silicon and triggers
 * FW NCR error.
 *
 * @return
 * 			- None
 *
 *****************************************************************************/
void XPlmi_TriggerFwNcrError(void)
{
	XPlmi_Error_t *ErrorTable = XPlmi_GetErrorTable();
	if ((ErrorTable[XPLMI_ERROR_FW_NCR].Action == XPLMI_EM_ACTION_SRST) ||
		(ErrorTable[XPLMI_ERROR_FW_NCR].Action == XPLMI_EM_ACTION_POR)) {
		XPlmi_SysmonClkSetIro();
	}

	/* Trigger FW NCR error by setting NCR_Flag in FW_ERR register */
	XPlmi_UtilRMW(PMC_GLOBAL_PMC_FW_ERR, PMC_GLOBAL_PMC_FW_ERR_NCR_FLAG_MASK,
			PMC_GLOBAL_PMC_FW_ERR_NCR_FLAG_MASK);
}

/*****************************************************************************/
/**
 * @brief	This function sets EmSubsystemId
 *
 * @param	Id pointer to set the EmSubsystemId
 *
 * @return
 * 			- None
 *
 *****************************************************************************/
void XPlmi_SetEmSubsystemId(const u32 *Id)
{
	EmSubsystemId = *Id;
}

/*****************************************************************************/
/**
 * @brief	This function returns EmSubsystemId
 *
 * @return
 * 			- EmSubsystemId
 *
 *****************************************************************************/
u32 XPlmi_GetEmSubsystemId(void)
{
	return EmSubsystemId;
}
/*****************************************************************************/
/**
 * @brief	This function updates NumErrOuts and returns number of error outs
 * count to the caller.
 *
 * @param	UpdateType is increment/decrement
 *
 * @return	Number of ErrOuts count
 *
 *****************************************************************************/
static u32 XPlmi_UpdateNumErrOutsCount(u8 UpdateType)
{
	u32 *NumErrOuts = XPlmi_GetNumErrOuts();

	if (UpdateType == XPLMI_UPDATE_TYPE_INCREMENT) {
		if (*NumErrOuts < XPLMI_MAX_ERR_OUTS) {
			++(*NumErrOuts);
		}
	} else {
		if (*NumErrOuts > 0U) {
			--(*NumErrOuts);
		}
	}

	return *NumErrOuts;
}

/*****************************************************************************/
/**
 * @brief	This function clears NPI errors.
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
******************************************************************************/
int XPlmi_CheckNpiErrors(void)
{
	int Status = XST_FAILURE;
	u32 ErrVal;
	u32 IsrVal = XPlmi_In32(NPI_NIR_REG_ISR);
	u32 ErrTypeVal = XPlmi_In32(NPI_NIR_ERR_TYPE);
	u32 ErrLogP0Info0Val = XPlmi_In32(NPI_NIR_ERR_LOG_P0_INFO_0);
	u32 ErrLogP0Info1Val = XPlmi_In32(NPI_NIR_ERR_LOG_P0_INFO_1);

	ErrVal =  IsrVal & NPI_NIR_REG_ISR_ERR_MASK;
	ErrVal |= ErrTypeVal & NPI_NIR_ERR_TYPE_ERR_MASK;
	if (ErrVal != 0U) {
		XPlmi_Printf(DEBUG_PRINT_ALWAYS, "NPI_NIR_ISR: 0x%08x\n\r"
			"NPI_NIR_ERR_TYPE: 0x%08x\n\r"
			"NPI_NIR_ERR_LOG_P0_INFO_0: 0x%08x\n\r"
			"NPI_NIR_ERR_LOG_P0_INFO_1: 0x%08x\n\r",
			IsrVal, ErrTypeVal, ErrLogP0Info0Val, ErrLogP0Info1Val);
	}
	else {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function clears NPI errors.
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
******************************************************************************/
int XPlmi_ClearNpiErrors(void)
{
	int Status = XST_FAILURE;

	/** - Unlock NPI address space */
	XPlmi_Out32(NPI_NIR_REG_PCSR_LOCK, NPI_NIR_REG_PCSR_UNLOCK_VAL);
	/** - Clear ISR */
	XPlmi_UtilRMW(NPI_NIR_REG_ISR, NPI_NIR_REG_ISR_ERR_MASK,
		NPI_NIR_REG_ISR_ERR_MASK);
	/** - Clear error type registers */
	XPlmi_UtilRMW(NPI_NIR_ERR_TYPE, NPI_NIR_ERR_TYPE_ERR_MASK,
		~(NPI_NIR_ERR_TYPE_ERR_MASK));
	XPlmi_Out32(NPI_NIR_ERR_LOG_P0_INFO_0, 0U);
	XPlmi_Out32(NPI_NIR_ERR_LOG_P0_INFO_1, 0U);
	/** - Lock NPI address space */
	Status = Xil_SecureOut32(NPI_NIR_REG_PCSR_LOCK, 1U);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_NPI_LOCK, Status);
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function updates error out count and clears PMC FW_CR error
 *
 * @return
 * 			- None
 *
 *****************************************************************************/
static void XPlmi_ErrOutNClearFwCR(u32 ErrorId)
{
	u32 NumErrOuts = 0U;
	XPlmi_Error_t *ErrorTable = XPlmi_GetErrorTable();

	/* If action is error out, clear PMC FW_CR error */
	if (ErrorTable[ErrorId].Action == XPLMI_EM_ACTION_ERROUT) {
		NumErrOuts = XPlmi_UpdateNumErrOutsCount(XPLMI_UPDATE_TYPE_DECREMENT);
		if (NumErrOuts == 0U) {
			XPlmi_Out32(PMC_GLOBAL_PMC_ERR1_STATUS,
				PMC_GLOBAL_PMC_ERR1_TRIG_FW_CR_MASK);
		}
	}
}
