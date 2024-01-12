/**************************************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xplmi_client.h
 *
 * This file Contains the client function prototypes, defines and macros.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.00  dd   01/09/24 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/

#ifndef XPLMI_CLIENT_H
#define XPLMI_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/

#include "xil_types.h"
#include "xplmi_mailbox.h"
#include "xplmi_defs.h"


/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

typedef struct {
        u32 IdCode; /**< Id code */
        u32 ExtIdCode; /**< Extended id code */
} XLoader_DeviceIdCode; /**< xilplmi device id code */

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

int XPlmi_GetDeviceID(XPlmi_ClientInstance *InstancePtr, XLoader_DeviceIdCode *DeviceIdCode);
int XPlmi_GetBoard(XPlmi_ClientInstance *InstancePtr, u64 Addr, u32 Size, u32 *ResponseLength);
int XPlmi_TamperTrigger (XPlmi_ClientInstance *InstancePtr, u32 TamperResponse);
int XPlmi_EventLogging(XPlmi_ClientInstance *InstancePtr, u32 sub_cmd, u64 Addr, u32 Len);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_CLIENT_H */