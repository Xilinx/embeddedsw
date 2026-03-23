/**************************************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xplmi_plat_client.h
 *
 * This file Contains the client function prototypes, defines and macros.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.00  vm   03/16/26 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/

#ifndef XPLMI_PLAT_CMD_CLIENT_H
#define XPLMI_PLAT_CMD_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xplmi_mailbox.h"
#include "xplmi_defs.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
int XPlmi_InPlaceAsuUpdate_ImageStore(XPlmi_ClientInstance *Ptr, const u32 Flag, u32 PdiId);
int XPlmi_InPlaceAsuUpdate_DDR(XPlmi_ClientInstance *Ptr, const u32 Flag, u32 DDRAddr);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_PLAT_CMD_CLIENT_H */
