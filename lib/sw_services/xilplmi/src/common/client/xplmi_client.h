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
 *       pre  07/10/24 Added support for configure secure communication command
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
/**< shift constant to place slr id*/
#define XPLMI_SLR_INDEX_SHIFT (6U)

/**< SlrIndexs constants*/
#define XPLMI_SLR_INDEX_0 (0U) /**< SLR Index 0 */
#define XPLMI_SLR_INDEX_1 (1U) /**< SLR Index 1 */
#define XPLMI_SLR_INDEX_2 (2U) /**< SLR Index 2 */
#define XPLMI_SLR_INDEX_3 (3U) /**< SLR Index 3 */

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
int XPlmi_ConfigSecureComm(XPlmi_ClientInstance *InstancePtr,
                           XPlmi_SsitSecComm *SsitSecCommDataPtr);
int XPlmi_InputSlrIndex(XPlmi_ClientInstance *InstancePtr, u32 SlrIndex);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_CLIENT_H */