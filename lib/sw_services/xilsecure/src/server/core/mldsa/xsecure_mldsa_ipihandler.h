/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file server/core/mldsa/xsecure_mldsa_ipihandler.h
*
* This file contains the Xilsecure MLDSA IPI handler declaration.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 5.7   tvp  03/20/26 Initial release
*
* </pre>
*
***************************************************************************************************/

/**
 * @addtogroup xsecure_mldsa_server_apis XilSecure MLDSA Server APIs
 * @{
 */
#ifndef XSECURE_MLDSA_IPIHANDLER_H_
#define XSECURE_MLDSA_IPIHANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

/****************************************** Include Files *****************************************/
#include "xplmi_cmd.h"

/*************************************** Constant Definitions *************************************/

/****************************************** Type Definitions **************************************/

/************************************** Function Prototypes ***************************************/
int XSecure_MldsaIpiHandler(XPlmi_Cmd *Cmd);

/** @} */
#ifdef __cplusplus
}
#endif

#endif /* XSECURE_MLDSA_IPIHANDLER_H_ */
