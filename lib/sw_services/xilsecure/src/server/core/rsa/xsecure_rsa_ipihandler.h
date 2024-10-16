/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_rsa_ipihandler.h
*
* This file contains the Xilsecure RSA IPI Handler declaration.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0  kal   03/23/21 Initial release
* 5.4  yog   04/29/24 Fixed doxygen grouping.
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_rsa_server_apis XilSecure RSA Server APIs
* @{
*/
#ifndef XSECURE_RSA_IPIHANDLER_H_
#define XSECURE_RSA_IPIHANDLER_H_

#ifdef __cplusplus
extern "c" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifndef PLM_RSA_EXCLUDE
#include "xplmi_cmd.h"

/************************** Constant Definitions *****************************/
int XSecure_RsaIpiHandler(XPlmi_Cmd *Cmd);

#endif

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_RSA_IPIHANDLER_H_ */
/** @} */
