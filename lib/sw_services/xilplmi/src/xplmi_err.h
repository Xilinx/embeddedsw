/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
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
#define XPLMI_ERR_REG_MASK		(0x1FU)

/**************************** Type Definitions *******************************/
/* Pointer to Error Handler Function */
typedef void (*XPlmi_ErrorHandler_t) (u32 ErrorId, u32 ErrorMask);
/* Pointer to Shutdown Handler Function */
typedef s32 (*XPlmi_ShutdownHandler_t)(u32 SubsystemId, const u32 Type,
		const u32 SubType);

/* Data Structure to hold Error Info */
struct XPlmi_Error_t {
	XPlmi_ErrorHandler_t Handler;
	u8 Action;
	u32 SubsystemId;
};

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
 * @brief	This function returns register mask value for the error id mask
 * given.
 *
 * @param	ErrorMask  is the input.
 *
 * @return	Register mask value
 *
 *****************************************************************************/
static inline u32 XPlmi_ErrRegMask(u32 ErrorMask)
{
	return ((u32)0x1U << (ErrorMask & (u32)XPLMI_ERR_REG_MASK));
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
static inline XPlmi_EventType XPlmi_EventNodeType(u32 Id)
{
	Id = (Id & XPLMI_NODE_TYPE_MASK) >> XPLMI_NODE_TYPE_SHIFT;

	return (XPlmi_EventType)Id;
}

/************************** Function Prototypes ******************************/
void XPlmi_EmInit(XPlmi_ShutdownHandler_t SystemShutdown);
int XPlmi_PsEmInit(void);
int XPlmi_EmSetAction(u32 ErrorNodeId, u32 ErrorMask, u8 ActionId,
		XPlmi_ErrorHandler_t ErrorHandler);
int XPlmi_EmDisable(u32 ErrorNodeId, u32 ErrorMask);
void XPlmi_ErrIntrHandler(void *CallbackRef);
void XPlmi_SetEmSubsystemId(const u32 *Id);

/* Functions defined in xplmi_err_cmd.c */
void XPlmi_ErrModuleInit(void);

/************************** Variable Definitions *****************************/

/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* XPLMI_ERR_H */
