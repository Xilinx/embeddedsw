/******************************************************************************
* Copyright (c) 2024-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_sha_ipihandler.h
* @addtogroup xsecure_apis XilSecure Versal_2Ve_2Vm APIs
* @{
* @cond xsecure_internal
* This file contains the xilsecure SHA IPI handler declaration.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 5.4  kal   07/24/24 Initial release
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

#ifndef XSECURE_SHA_IPIHANDLER_H_
#define XSECURE_SHA_IPIHANDLER_H_

#ifdef __cplusplus
extern "c" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_cmd.h"

/************************** Constant Definitions *****************************/
int XSecure_ShaIpiHandler(XPlmi_Cmd *Cmd);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_SHA_IPIHANDLER_H_ */
