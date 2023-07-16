/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_aes_ipihandler.h
* @addtogroup xsecure_apis XilSecure Versal Handler APIs
* @{
* @cond xsecure_internal
* This file contains the xilsecure AES IPI handler declaration.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0  kal   03/23/21 Initial release
* 5.2  vss	 07/15/23 Added prototype for XSecure_MakeAesFree()

*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

#ifndef XSECURE_AES_IPIHANDLER_H_
#define XSECURE_AES_IPIHANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_cmd.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
int XSecure_AesIpiHandler(XPlmi_Cmd *Cmd);
void XSecure_MakeAesFree(void);
#ifdef __cplusplus
}
#endif

#endif /* XSECURE_AES_IPIHANDLER_H_ */
