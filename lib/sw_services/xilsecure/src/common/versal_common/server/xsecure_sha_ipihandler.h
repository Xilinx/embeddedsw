/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_sha_ipihandler.h
* @addtogroup xsecure_apis XilSecure Versal APIs
* @{
* @cond xsecure_internal
* This file contains the xilsecure SHA3 IPI handler declaration.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kal   03/04/2021 Initial release
* 5.2   vss	  07/15/23 Added prototype for XSecure_MakeSha3Free()
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

#ifndef XSECURE_SHA_IPIHANDLER_H_
#define XSECURE_SHA_IPIHANDLER_H_

#ifdef __cplusplus
extern "c" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_cmd.h"

/************************** Constant Definitions *****************************/
int XSecure_Sha3IpiHandler(XPlmi_Cmd *Cmd);
void XSecure_MakeSha3Free(void);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_SHA_IPIHANDLER_H_ */
