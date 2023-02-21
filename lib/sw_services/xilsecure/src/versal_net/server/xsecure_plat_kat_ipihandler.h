/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_plat_kat_ipihandler.h
* @addtogroup xsecure_apis XilSecure versal net Handler APIs
* @{
* @cond xsecure_internal
* This file contains the xilsecure KAT platform IPI handler declaration.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0  kpt   07/15/22 Initial release
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

#ifndef XSECURE_PLAT_KAT_IPIHANDLER_H_
#define XSECURE_PLAT_KAT_IPIHANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_cmd.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
int XSecure_KatPlatIpiHandler(XPlmi_Cmd *Cmd);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_PLAT_KAT_IPIHANDLER_H_ */
