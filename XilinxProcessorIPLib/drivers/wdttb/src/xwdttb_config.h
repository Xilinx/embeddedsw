/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XWDTTB_CONFIG_H
#define XWDTTB_CONFIG_H

/*****************************************************************************/
/**
*
* @file xwdttb_config.h
* @brief Declares the window and timebase WDT helper functions.
*
* The xwdttb_config.h file contains the required functions of the XWdtTb driver.
* See xwdttb.h for a description of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 5.0   sne  11/19/19 First release
* 5.13  vn   09/30/26 Align Doxygen comments with AMD standards
*
* </pre>
*
******************************************************************************/
/** @addtogroup wdttb_api WDTTB APIs
 * @{
 */
/***************************** Include Files *********************************/

#include "xwdttb.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************** Function Prototypes ******************************/

/** @cond INTERNAL */
/*
 * Required functions in xwdttb_help.c
 */

s32 XWdtTb_DisableWinWdt(XWdtTb *InstancePtr);
void XWdtTb_EnableTimebaseWdt(XWdtTb *InstancePtr);
s32 XWdtTb_DisableTimebaseWdt(XWdtTb *InstancePtr);
/** @endcond */

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* end of protection macro */
