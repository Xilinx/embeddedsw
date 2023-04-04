/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xplmi_tamper.h
 *
 * This file contains APIs for tamper processing routines
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- ---------- -------------------------------------------------------
 * 1.0   ma   07/08/2022 Initial release
 *       ma   07/19/2022 Change XPlmi_ProcessTamperResponse return type to void
 *       kpt  07/19/2022 Added temporal macro to trigger secure lockdown
 *						 if haltboot efuses are programmed
 *       ma   07/25/2022 Enhancements to secure lockdown code
 * 1.1   bm   01/03/2023 Create Secure Lockdown as a Critical Priority Task
 *       dd   03/28/2023 Updated doxygen comments
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/
#ifndef XPLMI_TAMPER_H
#define XPLMI_TAMPER_H

#ifdef __cplusplus
extern "C" {
#endif
/***************************** Include Files *********************************/
#include "xil_util.h"

/************************** Constant Definitions *****************************/
/* Flags for Triggering Tamper */
#define XPLMI_TRIGGER_TAMPER_TASK	(0U) /**< Trigger tamper as a Task */
#define XPLMI_TRIGGER_TAMPER_IMMEDIATE	(1U) /**< Trigger tamper immediately */

/* Secure Lockdown State */
#define XPLMI_SLD_NOT_TRIGGERED		(0U) /**< Secure lockdown not triggered */
#define XPLMI_SLD_TRIGGERED		(1U) /**< Secure lockdown triggered */
#define XPLMI_SLD_IN_PROGRESS		(2U) /**< Secure lockdown is in progress */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/******************************************************************************/
/**
 *
 * @brief 	Updates the return value of the called function into Status and StatusTmp
 * 			variables for redundancy. when called function returns a failure it triggers
 * 			secure lockdown if haltboot efuses are programmed.
 *
 * @param   MajorError contains the major error code
 * @param   Status is the variable which holds the return value of function
 *          executed
 * @param   StatusTmp is the variable which holds the value stored in Status
 * @param	function is the function to be executed
 * @param	Other params are arguments to the called function
 *
 * @return	None
 *
 ******************************************************************************/
#define XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(MajorError, Status, StatusTmp, function, ...) \
		{ \
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, function, __VA_ARGS__); \
			if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) { \
				Status |= StatusTmp; \
				Status = XPlmi_UpdateStatus((XPlmiStatus_t)MajorError, Status); \
				XPlmi_UtilRMW(PMC_GLOBAL_PMC_FW_ERR, PMC_GLOBAL_PMC_FW_ERR_DATA_MASK, \
					(u32)Status); \
				XPlmi_TriggerSLDOnHaltBoot(XPLMI_TRIGGER_TAMPER_TASK); \
			} \
		}

/************************** Function Prototypes ******************************/
int XPlmi_RegisterTamperIntrHandler(void);
void XPlmi_TriggerTamperResponse(u32 Response, u32 Flag);
void XPlmi_TriggerSLDOnHaltBoot(u32 Flag);
u32 XPlmi_SldState(void);

#ifdef __cplusplus
}
#endif

#endif /** XPLMI_TAMPER_H */
