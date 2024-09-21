/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file versal/server/xnvm_cmd.h
*
* This file Contains the client function prototypes, defines and macros for
* the NVM programming.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.0   kal  07/05/2021 Initial release
* 2.4   bsv  09/09/2021 Added PLM_NVM macro
* 3.3   ng   11/22/2023 Fixed doxygen grouping
*
* </pre>
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
void XNvm_CmdsInit(int (*GenericHandler)(void));

#endif /* PLM_NVM */

#ifdef __cplusplus
}
#endif

#endif /* XNVM_CMD_H_ */
