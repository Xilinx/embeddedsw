/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_plat_elliptic_ipihandler.h
*
* This file contains the Xilsecure platform IPI handler declaration.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 5.2   har  06/20/23 Initial release
* 5.4   yog  04/29/24 Fixed doxygen grouping.
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_ecdsa_server_apis XilSecure ECDSA Server APIs
* @{
*/
#ifndef XSECURE_PLAT_ELLIPTIC_IPIHANDLER_H_
#define XSECURE_PLAT_ELLIPTIC_IPIHANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifndef PLM_ECDSA_EXCLUDE
#include "xplmi_cmd.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
int XSecure_PlatEllipticIpiHandler(XPlmi_Cmd *Cmd);

#endif /* PLM_ECDSA_EXCLUDE */

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_PLAT_ELLIPTIC_IPIHANDLER_H_ */
/** @} */
