/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_elliptic_ipihandler.h
*
* This file contains the Xilsecure elliptic IPI handler declaration.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kal   03/04/2021 Initial release
* 4.6   har   07/14/2021 Fixed doxygen warnings
* 5.4   yog   04/29/2024 Fixed doxygen grouping.
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_ecdsa_server_apis XilSecure ECDSA Server APIs
* @{
*/
#ifndef XSECURE_ELLIPTIC_IPIHANDLER_H_
#define XSECURE_ELLIPTIC_IPIHANDLER_H_

#ifdef __cplusplus
extern "c" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifndef PLM_ECDSA_EXCLUDE
#include "xplmi_cmd.h"

/************************** Constant Definitions *****************************/
int XSecure_EllipticIpiHandler(XPlmi_Cmd *Cmd);

#endif

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_ELLIPTIC_IPIHANDLER_H_ */
/** @} */
