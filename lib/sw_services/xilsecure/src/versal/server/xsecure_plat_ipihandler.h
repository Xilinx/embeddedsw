/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_plat_ipihandler.h
* @addtogroup xsecure_apis XilSecure Versal plat ipi handler APIs
* @{
* @cond xsecure_internal
* This file contains the xilsecure plat ipi Handler declaration.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 5.3   kpt  03/12/24 Initial release
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

#ifndef XSECURE_PLAT_IPIHANDLER_H_
#define XSECURE_PLAT_IPIHANDLER_H_

#ifdef __cplusplus
extern "c" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_cmd.h"

/************************** Constant Definitions *****************************/
int XSecure_PlatIpiHandler(XPlmi_Cmd *Cmd);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_PLAT_IPIHANDLER_H_ */
