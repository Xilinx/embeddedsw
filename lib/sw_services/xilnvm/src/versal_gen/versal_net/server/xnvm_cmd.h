/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file net/server/xnvm_cmd.h
 *
* This file Contains the client function prototypes, defines and macros for
* the NVM programming.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- ---------- -------------------------------------------------------
 * 3.0   kal  07/12/2022 Initial release
 * 3.3   kpt  02/21/2024 Added support to extend secure state
 *
 * </pre>
 *
 *
 ******************************************************************************/

/**
 * @addtogroup xnvm_server_apis XilNvm Server APIs
 * @{
 */

#ifndef XNVM_CMD_H_
#define XNVM_CMD_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifdef PLM_NVM
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
void XNvm_CmdsInit(int (*OcpHandler)(void));

#endif /* PLM_NVM */

#ifdef __cplusplus
}
#endif

#endif /* XNVM_CMD_H_ */
