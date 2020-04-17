/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPFW_ERROR_MANAGER_H_
#define XPFW_ERROR_MANAGER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xpfw_default.h"

/* Error IDs to identify each Error */
#define EM_ERR_ID_INVALID 0U

#define EM_ERR_ID_CSU_ROM 1U
#define EM_ERR_ID_PMU_PB 2U
#define EM_ERR_ID_PMU_SERVICE 3U
#define EM_ERR_ID_PMU_FW 4U
#define EM_ERR_ID_PMU_UC 5U
#define EM_ERR_ID_CSU 6U
#define EM_ERR_ID_PLL_LOCK 7U
#define EM_ERR_ID_PL 8U
#define EM_ERR_ID_TO 9U
#define EM_ERR_ID_AUX3 10U
#define EM_ERR_ID_AUX2 11U
#define EM_ERR_ID_AUX1 12U
#define EM_ERR_ID_AUX0 13U
#define EM_ERR_ID_CSU_SWDT 14U
#define EM_ERR_ID_CLK_MON 15U
#define EM_ERR_ID_XMPU 16U
#define EM_ERR_ID_PWR_SUPPLY 17U
#define EM_ERR_ID_FPD_SWDT 18U
#define EM_ERR_ID_LPD_SWDT 19U
#define EM_ERR_ID_RPU_CCF 20U
#define EM_ERR_ID_RPU_LS 21U
#define EM_ERR_ID_FPD_TEMP 22U
#define EM_ERR_ID_LPD_TEMP 23U
#define EM_ERR_ID_RPU1 24U
#define EM_ERR_ID_RPU0 25U
#define EM_ERR_ID_OCM_ECC 26U
#define EM_ERR_ID_DDR_ECC 27U

#define EM_ERR_ID_MAX 28U



/* Error Type identifies the HW register */
#define EM_ERR_TYPE_1    1U
#define EM_ERR_TYPE_2    2U


/* Action to be taken when an error occurs */
#define EM_ACTION_NONE   0U
#define EM_ACTION_POR    1U
#define EM_ACTION_SRST   2U
#define EM_ACTION_CUSTOM 3U
#define EM_ACTION_PSERR  4U
#define EM_ACTION_MAX    5U

/* EM action change permissions */
#define EM_ACTION_CHANGE_PERM_NONE	0U


/* Pointer to Error Handler Function */
typedef void (*XPfw_ErrorHandler_t) (u8 ErrorId);

/**
 * Init the Error Management Framework
 * All error signals in HW are enabled and Actions are disabled
 */
void XPfw_EmInit(void);

/**
 * Set action to be taken when a specific error occurs
 *
 * @param ErrorId is the ID for error as defined in this file
 * @param ActionId is one of the actions defined in this file
 * @param ErrorHandler is the handler to be called in case of custom action
 *
 * @return XST_SUCCESS if the action was successfully registered
 *         XST_FAILURE if the registration fails
 */
s32 XPfw_EmSetAction(u8 ErrorId, u8 ActionId, XPfw_ErrorHandler_t ErrorHandler);

/**
 * Disable action for the given error
 *
 * @param ErrorId is the ID for error as defined in this file
 *
 * @return XST_SUCCESS if the action was disabled
 *         XST_FAILURE if the action disable fails
 */
s32 XPfw_EmDisable(u8 ErrorId);

/**
 * Process the errors of specified type and call the handlers of triggered errors
 *
 * @param ErrorType is either TYPE_1 or TYPE_2 as defined in this file
 *
 * @return XST_SUCCESS if the error was handled successfully
 *         XST_FAILURE in case of failure
 */
s32 XPfw_EmProcessError(u8 ErrorType);

/**
 * Enable Report out of error via PSERR pin
 *
 * @param ErrorId is the ID for error to be reported out
 *
 */
s32 XPfw_EmEnablePSError(u8 ErrorId);

/**
 * Disable and re-enable PMU interrupts in PMU Global register
 */
void XPfw_PulseErrorInt(void);

/* Data Structure to hold Error Info */
struct XPfw_Error_t {
	const u32 RegMask;
	XPfw_ErrorHandler_t Handler;
	const u8 Type;
	u8 Action;
	u16 ChngPerm;
};

extern struct XPfw_Error_t ErrorTable[EM_ERR_ID_MAX];

#ifdef __cplusplus
}
#endif

#endif /* XPFW_ERROR_MANAGER_H_ */
