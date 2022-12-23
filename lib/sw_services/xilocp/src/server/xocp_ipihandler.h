/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2023, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xocp_ipihandler.h
* @addtogroup xocp_apis XilOcp APIs
* @{
* @cond xocp_internal
* This file contains the xilocp IPI handler declaration.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.1   am   12/21/22 Initial release
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

#ifndef XOCP_IPIHANDLER_H_
#define XOCP_IPIHANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_config.h"
#ifdef PLM_OCP
#include "xplmi_cmd.h"

/************************** Constant Definitions *****************************/
int XOcp_IpiHandler(XPlmi_Cmd *Cmd);

#endif /* PLM_OCP */

#ifdef __cplusplus
}
#endif

#endif /* XOCP_IPIHANDLER_H_ */