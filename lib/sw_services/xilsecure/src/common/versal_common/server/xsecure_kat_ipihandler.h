/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_kat_ipihandler.h
* @addtogroup xsecure_apis XilSecure Versal Handler APIs
* @{
* @cond xsecure_internal
* This file contains the xilsecure KAT IPI handler declaration.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0  kpt   07/15/22 Initial release
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

#ifndef XSECURE_KAT_IPIHANDLER_H_
#define XSECURE_KAT_IPIHANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_cmd.h"
#include "xsecure_defs.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
int XSecure_KatIpiHandler(XPlmi_Cmd *Cmd);
void XSecure_PerformKatOperation(XSecure_KatOp KatOp, u32 KatMask);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_KAT_PLAT_IPIHANDLER_H_ */
