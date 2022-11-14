/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file net/server/xnvm_efuse_cdohandler.h
* @addtogroup xnvm_versal_net_apis XilNvm Versal Net APIs
* @{
* This file contains the Versal_Net XilNvm eFUSE Cdo handler declaration.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 3.0  kal  07/12/2021 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XNVM_EFUSE_CDOHANDLER_H_
#define XNVM_EFUSE_CDOHANDLER_H_

#ifdef __cplusplus
extern "c" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifdef PLM_NVM
#include "xplmi_cmd.h"

/************************** Constant Definitions *****************************/
int XNvm_EfuseCdoHandler(XPlmi_Cmd *Cmd);

#endif /* PLM_NVM */

#ifdef __cplusplus
}
#endif

#endif /* XNVM_EFUSE_CDOHANDLER_H_ */
