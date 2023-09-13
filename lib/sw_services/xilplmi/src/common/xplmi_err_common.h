/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplmi_err_common.h
*
* This is the file which contains .
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
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
* 1.04  td   11/23/2020 MISRA C Rule 17.8 Fixes
*       bsv  01/29/2021 Added APIs for checking and clearing NPI errors
* 1.05  pj   03/24/2021 Added API for updating the SubsystemId of the error node
*                       Added API for handling and trigger sofware errors
*       bl   04/01/2021 Update XPlmi_ShutdownHandler_t typedef to remove
*                       warning
*       ma   04/05/2021 Added support for error configuration using Error Mask
*                       instead of Error ID. Also, added support to configure
*                       multiple errors at once.
*       ma   05/03/2021 Minor updates related to PSM and FW errors
*       td   05/20/2021 Fixed blind write on locking NPI address space in
*                       XPlmi_ClearNpiErrors
* 1.06  bsv  07/16/2021 Fix doxygen warnings
*       kc   07/22/2021 XPlmi_PorHandler scope updated to global
*       rv   08/04/2021 Added support to pass subsystem restart handler to
*			XPlmi_EmInit
*       rv   08/26/2021 Remove unused macro definitions
* 1.07  bsv  12/24/2021 Move common defines from xilplmi and xilpm to common
*                       folder
*       ma   01/24/2022 Add XPLMI_ERROR_ACTION_NOT_DISABLED and
*                       XPLMI_ERROR_ACTION_NOT_ENABLED minor error codes
*       skd  03/09/2022 Compilation warning fix
* 1.08  ma   05/10/2022 Added PLM to PLM communication feature
*       ma   06/01/2022 Added PLM Print Log as new error action
*       bm   07/06/2022 Refactor versal and versal_net code
*       ma   07/08/2022 Added support for secure lockdown
*       ma   07/19/2022 Disable interrupts before secure lockdown
*       bm   07/20/2022 Update EAM logic for In-Place PLM Update
*       ma   08/08/2022 Handle EAM errors at task level
* 1.09  bsv  09/30/2022 Make XPlmi_SoftResetHandler non-static so that
*                       it can be used in Image Selector
* 1.10  sk   07/18/2023 Added error codes for invalid address and
*                       LPD not initialized
*       sk   08/17/2023 Added declaration for XPlmi_GetEmSubsystemId,
*                       define for Invalid Subsystem
*       dd   09/12/2023 MISRA-C violation Rule 10.3 fixed
*
* </pre>
*
* @note
*
******************************************************************************/
#ifndef XPLMI_ERR_COMMON_H
#define XPLMI_ERR_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_debug.h"
#include "xil_error_node.h"
#include "xplmi_error_node.h"
#include "xplmi_hw.h"
#include "xil_hw.h"
#include "xplmi_cmd.h"

/**@cond xplmi_internal
 * @{
 */

/************************** Constant Definitions *****************************/
/* Action to be taken when an error occurs */
#define XPLMI_EM_ACTION_INVALID			(0U)
#define XPLMI_EM_ACTION_POR			(1U)
#define XPLMI_EM_ACTION_SRST			(2U)
#define XPLMI_EM_ACTION_CUSTOM			(3U)
#define XPLMI_EM_ACTION_ERROUT			(4U)
#define XPLMI_EM_ACTION_SUBSYS_SHUTDN		(5U)
#define XPLMI_EM_ACTION_SUBSYS_RESTART		(6U)
#define XPLMI_EM_ACTION_PRINT_TO_LOG		(7U)
#define XPLMI_EM_ACTION_NONE			(8U)
#define XPLMI_EM_ACTION_MAX			(9U)

/* Subsystem shutdown/restart related macros */
#define XPLMI_SUBSYS_SHUTDN_TYPE_SHUTDN		(0U)

/* PLMI ERROR Management error codes */
#define XPLMI_INVALID_ERROR_ID			(1)
#define XPLMI_INVALID_ERROR_TYPE		(2)
#define XPLMI_INVALID_ERROR_HANDLER		(3)
#define XPLMI_INVALID_ERROR_ACTION		(4)
#define XPLMI_LPD_UNINITIALIZED			(5)
#define XPLMI_CANNOT_CHANGE_ACTION		(6)
#define XPLMI_INVALID_NODE_ID			(7)
#define XPLMI_ERROR_ACTION_NOT_DISABLED		(8)
#define XPLMI_ERROR_ACTION_NOT_ENABLED		(9)
#define XPLMI_ERROR_INVALID_ADDRESS		(10U)
#define XPLMI_ERROR_LPD_NOT_INITIALIZED		(11U)

/* Error Register mask */
#define XPLMI_MAX_ERR_BITS			(32U)
#define XPLMI_REG_MAX_ERRORS			(0x20U)
#define XPLMI_EVENT_ERROR_OFFSET		(0x4000U)

#define XPLMI_PMC_PSM_ERR1_REG_OFFSET		(0x0U)
#define XPLMI_PMC_PSM_ERR2_REG_OFFSET		(0x10U)

#define GET_PMC_ERR_START(ErrIndex)		(XPLMI_ERROR_BOOT_CR + \
						(ErrIndex * XPLMI_REG_MAX_ERRORS))
#define GET_PMC_ERR_END(ErrIndex)		(XPLMI_ERROR_PMCERR1_MAX + \
						(ErrIndex * XPLMI_REG_MAX_ERRORS))
#define GET_PSM_ERR_START(ErrIndex)		(XPLMI_ERROR_PS_SW_CR + \
						(ErrIndex * XPLMI_REG_MAX_ERRORS))
#define GET_PSM_ERR_END(ErrIndex)		(XPLMI_ERROR_PSMERR1_MAX + \
						(ErrIndex * XPLMI_REG_MAX_ERRORS))
#define GET_PSM_ERR_ACTION_OFFSET(Index)	(Index * XPLMI_PMC_PSM_ERR2_REG_OFFSET)

/* Event error Indexes */
#define XPLMI_NODETYPE_EVENT_PMC_INDEX		(0x0U)
#define XPLMI_NODETYPE_EVENT_PSM_INDEX		(0x1U)
#define XPLMI_NODETYPE_EVENT_SW_INDEX		(0x2U)
#define XPLMI_NODETYPE_EVENT_INVALID_INDEX	(0x3U)

#define XPLMI_INVALID_SUBSYSTEM_ID		(0xFFFFFFFFU)

/**************************** Type Definitions *******************************/
/* Pointer to Error Handler Function */
typedef void (*XPlmi_ErrorHandler_t) (u32 ErrorNodeId, u32 RegMask);
/* Pointer to Shutdown Handler Function */
typedef s32 (*XPlmi_ShutdownHandler_t)(u32 SubsystemId, const u32 Type,
		const u32 SubType, const u32 CmdType);
/* Pointer to Subsystem Restart Handler Function */
typedef s32 (*XPlmi_RestartHandler_t)(const u32 SubsystemId);

/* Data Structure to hold Error Info */
typedef struct {
	XPlmi_ErrorHandler_t Handler; /**< Error Handler function pointer */
	u8 Action; /**< Action to take on error */
	u32 SubsystemId; /**< Subsystem ID for shutdown or restart */
} XPlmi_Error_t;

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
 * @brief	This function returns register mask value for the error id mask
 * given.
 *
 * @param	ErrorId  is the input.
 *
 * @return	Register mask value
 *
 *****************************************************************************/
static inline u32 XPlmi_ErrRegMask(u32 ErrorId)
{
	return ((u32)0x1U << (ErrorId % (u32)XPLMI_MAX_ERR_BITS));
}

/*****************************************************************************/
/**
 * @brief	This function returns Error event ID for the given error node ID.
 *
 * @param	Id is the event ID.
 *
 * @return	Error event ID
 *
 *****************************************************************************/
static inline u32 XPlmi_EventNodeType(u32 Id)
{
	u32 EventTypeId;

	EventTypeId = (Id & XPLMI_NODE_TYPE_MASK) >> XPLMI_NODE_TYPE_SHIFT;

	return EventTypeId;
}

/*****************************************************************************/
/**
 * @brief	This function checks if NPI is out of reset or not.
 *
 * @return	TRUE if NPI is out of reset, else FALSE
 *
******************************************************************************/
static inline u8 XPlmi_NpiOutOfReset(void)
{
	u8 RegVal = (u8)((XPlmi_In32(CRP_RST_NONPS) &
		CRP_RST_NONPS_NPI_RESET_MASK) >> CRP_RST_NONPS_NPI_RESET_SHIFT);

	if (RegVal == 0U) {
		RegVal = (u8)TRUE;
	}
	else {
		RegVal = (u8)FALSE;
	}

	return RegVal;
}

/************************** Function Prototypes ******************************/
int XPlmi_EmInit(XPlmi_ShutdownHandler_t SystemShutdown,
		  XPlmi_RestartHandler_t SubsystemRestart);
int XPlmi_PsEmInit(void);
int XPlmi_EmSetAction(u32 ErrorNodeId, u32 ErrorMasks, u8 ActionId,
		XPlmi_ErrorHandler_t ErrorHandler, const u32 SubsystemId);
void XPlmi_UpdateErrorSubsystemId(u32 ErrorNodeId, u32 ErrorMasks,
		u32 SubsystemId);
int XPlmi_EmDisable(u32 ErrorNodeId, u32 RegMask);
void XPlmi_ErrIntrHandler(void *CallbackRef);
void XPlmi_HandleSwError(u32 ErrorNodeId, u32 RegMask);
void XPlmi_SetEmSubsystemId(const u32 *Id);
u32 XPlmi_GetEmSubsystemId(void);
int XPlmi_CheckNpiErrors(void);
int XPlmi_ClearNpiErrors(void);
void XPlmi_TriggerFwNcrError(void);
void XPlmi_PORHandler(void);
void XPlmi_ErrPrintToLog(u32 ErrorNodeId, u32 RegMask);
u32 XPlmi_GetErrorId(u32 ErrorNodeId, u32 RegMask);
int XPlmi_EmDisablePmcErrors(u32 RegOffset, u32 RegMask);
int XPlmi_EmDisablePsmErrors(u32 RegOffset, u32 RegMask);
int XPlmi_EmConfig(XPlmi_EventType NodeType, u32 ErrorId, u8 ActionId,
		XPlmi_ErrorHandler_t ErrorHandler, const u32 SubsystemId);
u32 EmDisableErrAction(u32 ErrMaskRegAddr, u32 RegMask);
int XPlmi_ErrorTaskHandler(void *Data);
void XPlmi_SoftResetHandler(void);

/* Functions defined in xplmi_err_cmd.c */
void XPlmi_ErrModuleInit(void);

/************************** Variable Definitions *****************************/

/*****************************************************************************/

/**
 * @}
 * @endcond
 */

#ifdef __cplusplus
}
#endif

#endif /* XPLMI_ERR_COMMON_H */
