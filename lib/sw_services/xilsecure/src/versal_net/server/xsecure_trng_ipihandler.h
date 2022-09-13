/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_trng_ipihandler.h
* @addtogroup xsecure_apis XilSecure Versal Handler APIs
* @{
* @cond xsecure_internal
* This file contains the xilsecure TRNG IPI handler declaration.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 5.0  kal   03/23/21 Initial release
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

#ifndef XSECURE_TRNG_IPIHANDLER_H_
#define XSECURE_TRNG_IPIHANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_cmd.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
int XSecure_TrngIpiHandler(XPlmi_Cmd *Cmd);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_TRNG_IPIHANDLER_H_ */
