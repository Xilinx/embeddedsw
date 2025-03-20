/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2024 - 2025, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_plat_aes_ipihandler.h
* This file contains the Xilsecure Versal Net IPI Handler APIs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 5.3   har  02/05/24 Initial release
* 5.4   yog  04/29/24 Fixed doxygen grouping.
*       pre  03/02/25 Added under secure exclusion
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_aes_server_apis XilSecure AES Server APIs
* @{
*/
#ifndef XSECURE_PLAT_AES_IPIHANDLER_H_
#define XSECURE_PLAT_AES_IPIHANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_cmd.h"
#ifndef PLM_SECURE_EXCLUDE
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
int XSecure_PlatAesIpiHandler(XPlmi_Cmd *Cmd);
#endif
#ifdef __cplusplus
}
#endif

#endif /* XSECURE_PLAT_AES_IPIHANDLER_H_ */
/** @} */
