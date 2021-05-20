/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xhdcp1x_platform.h
* @addtogroup hdcp1x_v4_6
* @{
*
* This file provides the interface for the hdcp platform integration module
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  fidus  07/16/15 Initial release.
* </pre>
*
******************************************************************************/

#ifndef XHDCP1X_PLATFORM_H
/**< Prevent circular inclusions by using protection macros */
#define XHDCP1X_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xhdcp1x.h"
#include "xil_types.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

int XHdcp1x_PlatformIsKsvRevoked(const XHdcp1x *InstancePtr, u64 Ksv);
int XHdcp1x_PlatformTimerStart(XHdcp1x *InstancePtr, u16 TimeoutInMs);
int XHdcp1x_PlatformTimerStop(XHdcp1x *InstancePtr);
int XHdcp1x_PlatformTimerBusy(XHdcp1x *InstancePtr, u16 DelayInMs);

#ifdef __cplusplus
}
#endif

#endif  /* XHDCP1X_PLATFORM_H */
/** @} */
