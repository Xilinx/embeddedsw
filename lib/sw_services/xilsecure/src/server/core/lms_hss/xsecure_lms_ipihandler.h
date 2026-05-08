/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file server/core/lms_hss/xsecure_lms_ipihandler.h
*
* This file contains the Xilsecure LMS IPI handler declaration.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date      Changes
* ----- ---- -------- ------------------------------------------------------------------------------
* 5.7   vss  04/29/26 Initial release
*
* </pre>
*
*
***************************************************************************************************/

/**
 * @addtogroup xsecure_lms_server_apis XilSecure LMS Server APIs
 * @{
 */
#ifndef XSECURE_LMS_IPIHANDLER_H_
#define XSECURE_LMS_IPIHANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

/****************************************** Include Files *****************************************/
#include "xplmi_cmd.h"
#include "xil_types.h"

/************************** Function Prototypes ******************************/
int XSecure_LmsIpiHandler(XPlmi_Cmd *Cmd);

/** @} */
#ifdef __cplusplus
}
#endif

#endif /* XSECURE_LMS_IPIHANDLER_H_ */
