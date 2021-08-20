/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplmi_err.h
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
*
* </pre>
*
* @note
*
******************************************************************************/
#ifndef XPLMI_ERR_H
#define XPLMI_ERR_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_debug.h"
#include "xplmi_error_node.h"
#include "xplmi_hw.h"

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
#define XPLMI_EM_ACTION_NONE			(7U)
#define XPLMI_EM_ACTION_MAX			(8U)

/* Subsystem shutdown/restart related macros */
#define XPLMI_SUBSYS_SHUTDN_TYPE_SHUTDN		(0U)
#define XPLMI_SUBSYS_SHUTDN_TYPE_RESTART	(1U)
#define XPLMI_RESTART_SUBTYPE_SUBSYS		(0U)

/* PLMI ERROR Management error codes */
#define XPLMI_INVALID_ERROR_ID		(1)
#define XPLMI_INVALID_ERROR_TYPE	(2)
#define XPLMI_INVALID_ERROR_HANDLER	(3)
#define XPLMI_INVALID_ERROR_ACTION	(4)
#define XPLMI_LPD_UNINITIALIZED		(5)
#define XPLMI_CANNOT_CHANGE_ACTION	(6)
#define XPLMI_INVALID_NODE_ID		(7)

/* Error Register mask */
#define XPLMI_MAX_ERR_BITS			(32U)

/**************************** Type Definitions *******************************/
/* Pointer to Error Handler Function */
typedef void (*XPlmi_ErrorHandler_t) (u32 ErrorNodeId, u32 RegMask);
/* Pointer to Shutdown Handler Function */
typedef s32 (*XPlmi_ShutdownHandler_t)(u32 SubsystemId, const u32 Type,
		const u32 SubType, const u32 CmdType);
/* Pointer to Subsystem Restart Handler Function */
typedef s32 (*XPlmi_RestartHandler_t)(const u32 SubsystemId);

/* Data Structure to hold Error Info */
struct XPlmi_Error_t {
	XPlmi_ErrorHandler_t Handler; /**< Error Handler function pointer */
	u8 Action; /**< Action to take on error */
	u32 SubsystemId; /**< Subsystem ID for shutdown or restart */
};

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
void XPlmi_EmInit(XPlmi_ShutdownHandler_t SystemShutdown,
		  XPlmi_RestartHandler_t SubsystemRestart);
int XPlmi_PsEmInit(void);
int XPlmi_EmSetAction(u32 ErrorNodeId, u32 ErrorMasks, u8 ActionId,
		XPlmi_ErrorHandler_t ErrorHandler);
void XPlmi_UpdateErrorSubsystemId(u32 ErrorNodeId, u32 ErrorMasks,
		u32 SubsystemId);
int XPlmi_EmDisable(u32 ErrorNodeId, u32 RegMask);
void XPlmi_ErrIntrHandler(void *CallbackRef);
void XPlmi_HandleSwError(u32 ErrorNodeId, u32 RegMask);
void XPlmi_SetEmSubsystemId(const u32 *Id);
int XPlmi_CheckNpiErrors(void);
int XPlmi_ClearNpiErrors(void);
void XPlmi_TriggerFwNcrError(void);
void XPlmi_PORHandler(void);

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

#endif /* XPLMI_ERR_H */
