/******************************************************************************
* Copyright (c) 2024-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file server/core/sha/sha_pmxc/xsecure_sha_ipihandler.h
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

/**
 * @addtogroup xsecure_apis XilSecure Server APIs
 * @{
 */
#ifndef XSECURE_SHA_IPIHANDLER_H_
#define XSECURE_SHA_IPIHANDLER_H_

#ifdef __cplusplus
extern "c" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_cmd.h"

/************************** Constant Definitions *****************************/
int XSecure_ShaIpiHandler(XPlmi_Cmd *Cmd);

/** @} */
#ifdef __cplusplus
}
#endif

#endif /* XSECURE_SHA_IPIHANDLER_H_ */
