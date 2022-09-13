/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xnvm_cmd.h
 *
 * Header file for xnvm_cmd.c
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- ---------- -------------------------------------------------------
 * 3.0   kal  07/12/2022 Initial release
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/
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
void XNvm_CmdsInit(void);

#endif /* PLM_NVM */

#ifdef __cplusplus
}
#endif

#endif /* XNVM_CMD_H_ */
