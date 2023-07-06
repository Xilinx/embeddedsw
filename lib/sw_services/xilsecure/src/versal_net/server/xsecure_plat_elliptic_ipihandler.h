/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_plat_elliptic_ipihandler.h
* @addtogroup xsecure_apis XilSecure versal net Platform Handler APIs
* @{
* @cond xsecure_internal
* This file contains the xilsecure platform IPI handler declaration.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 5.2   har  06/20/23 Initial release
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

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
