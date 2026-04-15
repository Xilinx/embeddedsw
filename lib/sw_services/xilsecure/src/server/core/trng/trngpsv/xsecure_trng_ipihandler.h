/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file server/core/trng/trngpsv/xsecure_trng_ipihandler.h
*
* This file contains the Xilsecure TRNG IPI handler declaration.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 5.7   tvp  11/14/25 Initial release
*
* </pre>
*
***************************************************************************************************/

/**
 * @addtogroup xsecure_trng_server_apis XilSecure TRNG Server APIs
 * @{
 */
#ifndef XSECURE_TRNG_IPIHANDLER_H_
#define XSECURE_TRNG_IPIHANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

/****************************************** Include Files *****************************************/
#include "xplmi_cmd.h"

/*************************************** Constant Definitions *************************************/

/****************************************** Type Definitions **************************************/

/************************************** Function Prototypes ***************************************/
int XSecure_TrngIpiHandler(XPlmi_Cmd *Cmd);

/** @} */
#ifdef __cplusplus
}
#endif

#endif /* XSECURE_TRNG_IPIHANDLER_H_ */
