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

/************************** Function Prototypes ******************************/
int XPlmi_RegisterTamperIntrHandler(void);
int XPlmi_ProcessTamperResponse(u32 TamperResp);

#ifdef __cplusplus
}
#endif

#endif /** XPLMI_TAMPER_H */