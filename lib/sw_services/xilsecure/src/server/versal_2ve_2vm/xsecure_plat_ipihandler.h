/******************************************************************************
* Copyright (c) 2024-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_plat_ipihandler.h
*
* This file contains the Xilsecure platform IPI handler declaration.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 5.4  kpt   01/13/23 Initial release
*
* </pre>
*
*
******************************************************************************/
/**
* @addtogroup xsecure_helper_server_apis Platform specific helper APIs in Xilsecure server
* @{
*/
#ifndef XSECURE_PLAT_IPIHANDLER_H_
#define XSECURE_PLAT_IPIHANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_cmd.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
int XSecure_PlatIpiHandler(XPlmi_Cmd *Cmd);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_PLAT_IPIHANDLER_H_ */
