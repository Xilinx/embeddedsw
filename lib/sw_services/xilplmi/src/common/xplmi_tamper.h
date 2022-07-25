/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
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
				XPlmi_TriggerSLDOnHaltBoot(); \
			} \
		}

/************************** Function Prototypes ******************************/
int XPlmi_RegisterTamperIntrHandler(void);
void XPlmi_ProcessTamperResponse(u32 TamperResp);
void XPlmi_TriggerSLDOnHaltBoot(void);

#ifdef __cplusplus
}
#endif

#endif /** XPLMI_TAMPER_H */
