/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
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
* 1.00  kc   05/23/2019 Initial release
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
#define XPLMI_EM_ACTION_INVALID	0U
#define XPLMI_EM_ACTION_POR		1U
#define XPLMI_EM_ACTION_SRST		2U
#define XPLMI_EM_ACTION_CUSTOM		3U
#define XPLMI_EM_ACTION_ERROUT		4U
#define XPLMI_EM_ACTION_SUBSYS_SHUTDN	5U
#define XPLMI_EM_ACTION_SUBSYS_RESTART	6U
#define XPLMI_EM_ACTION_PSM_CR		7U
#define XPLMI_EM_ACTION_PSM_NCR		8U
#define XPLMI_EM_ACTION_NONE		9U
#define XPLMI_EM_ACTION_MAX		10U

/* Subsystem shutdown/restart related macros */
#define XPLMI_SUBSYS_SHUTDN_TYPE_SHUTDN		0U
#define XPLMI_SUBSYS_SHUTDN_TYPE_RESTART	1U
#define XPLMI_RESTART_SUBTYPE_SUBSYS		0U

/* PLMI ERROR Management error codes */
#define XPLMI_INVALID_ERROR_ID		(1U)
#define XPLMI_INVALID_ERROR_TYPE	(2U)
#define XPLMI_INVALID_ERROR_HANDLER	(3U)
#define XPLMI_INVALID_ERROR_ACTION	(4U)
#define XPLMI_LPD_UNINITIALIZED		(5U)

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
