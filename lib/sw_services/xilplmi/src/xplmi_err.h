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
#include "xil_types.h"
#include "xil_assert.h"
#include "xplmi_status.h"
#include "xplmi_debug.h"
#include "xplmi_hw.h"
#include "xplmi_error_node.h"

/************************** Constant Definitions *****************************/
/* Action to be taken when an error occurs */
#define XPLMI_EM_ACTION_INVALID			(0U)
#define XPLMI_EM_ACTION_POR				(1U)
#define XPLMI_EM_ACTION_SRST			(2U)
#define XPLMI_EM_ACTION_CUSTOM			(3U)
#define XPLMI_EM_ACTION_ERROUT			(4U)
#define XPLMI_EM_ACTION_SUBSYS_SHUTDN	(5U)
#define XPLMI_EM_ACTION_SUBSYS_RESTART	(6U)
#define XPLMI_EM_ACTION_NONE			(7U)
#define XPLMI_EM_ACTION_MAX				(8U)

/* Subsystem shutdown/restart related macros */
#define XPLMI_SUBSYS_SHUTDN_TYPE_SHUTDN		(0U)
#define XPLMI_SUBSYS_SHUTDN_TYPE_RESTART	(1U)
#define XPLMI_RESTART_SUBTYPE_SUBSYS		(0U)

/* PLMI ERROR Management error codes */
#define XPLMI_INVALID_ERROR_ID		(1U)
#define XPLMI_INVALID_ERROR_TYPE	(2U)
#define XPLMI_INVALID_ERROR_HANDLER	(3U)
#define XPLMI_INVALID_ERROR_ACTION	(4U)
#define XPLMI_LPD_UNINITIALIZED		(5U)
#define XPLMI_CANNOT_CHANGE_ACTION	(6U)

/**************************** Type Definitions *******************************/
/* Pointer to Error Handler Function */
typedef void (*XPlmi_ErrorHandler_t) (u32 ErrorId, u32 ErrorMask);
extern s32 (* PmSystemShutdown)(u32 SubsystemId, const u32 Type,
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
inline u32 XPlmi_ErrRegMask(u32 ErrorMask)
{
	return (0x1U << (ErrorMask & 0x1FU));
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
inline u32 XPlmi_EventNodeType(u32 Id)
{
	return ((Id & XPLMI_NODE_TYPE_MASK) >> XPLMI_NODE_TYPE_SHIFT);
}

/************************** Function Prototypes ******************************/
void XPlmi_EmInit(s32 (* SystemShutdown)(u32 SubsystemId,
		const u32 Type, const u32 SubType));
int XPlmi_PsEmInit(void);
int XPlmi_EmSetAction(u32 ErrorNodeId, u32 ErrorMask, u8 ActionId,
		XPlmi_ErrorHandler_t ErrorHandler);
int XPlmi_EmDisable(u32 ErrorNodeId, u32 ErrorMask);
void XPlmi_ErrIntrHandler(void *CallbackRef);

/* Functions defined in xplmi_err_cmd.c */
void XPlmi_ErrModuleInit(void);

/************************** Variable Definitions *****************************/
extern u32 EmSubsystemId;
/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* XPLMI_ERR_H */
