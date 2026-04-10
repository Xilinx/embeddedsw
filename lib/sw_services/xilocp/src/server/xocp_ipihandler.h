/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2026, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xocp_ipihandler.h
* @cond xocp_internal
* This file contains the xilocp IPI handler declaration.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.1   am   12/21/22 Initial release
* 1.7   rpu  02/18/26 Fixed Doxygen warnings
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