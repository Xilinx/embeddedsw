/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file versal/server/xnvm_efuse_ipihandler.h
*
* This file contains the xilnvm eFUSE IPI handler declaration.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kal  07/30/2021 Initial release
* 2.4   bsv  09/09/2021 Added PLM_NVM macro
* 3.3	vss  02/23/2024	Added IPI support for eFuse read and write
*       ng   11/22/2023 Fixed doxygen grouping
*
* </pre>
*
******************************************************************************/

#ifndef XNVM_EFUSE_IPIHANDLER_H_
#define XNVM_EFUSE_IPIHANDLER_H_

#ifdef __cplusplus
extern "c" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifdef PLM_NVM
#include "xplmi_cmd.h"

/************************** Constant Definitions *****************************/
int XNvm_EfuseIpiHandler(XPlmi_Cmd *Cmd);
int XNvm_EfuseRead(u32 Offset, u32 AddrLow, u32 AddrHigh, u32 Size);
#if (defined(XNVM_WRITE_KEY_MANAGEMENT_EFUSE)) || (defined(XNVM_WRITE_SECURITY_CRITICAL_EFUSE)) || \
	(defined (XNVM_WRITE_USER_EFUSE))
int XNvm_EfuseWriteAccess(const XPlmi_Cmd * Cmd, u32 AddrLow, u32 AddrHigh, u8 EnvMonitorDis);
#endif

#endif /* PLM_NVM */

#ifdef __cplusplus
}
#endif

#endif /* XNVM_EFUSE_IPIHANDLER_H_ */
