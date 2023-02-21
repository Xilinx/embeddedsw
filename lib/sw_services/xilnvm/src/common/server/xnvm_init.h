/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xnvm_init.h
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- ---------- -------------------------------------------------------
 * 1.0   kal  07/05/2021 Initial release
 * 2.4   bsv  09/09/2021 Added PLM_NVM macro
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/
#ifndef XNVM_INIT_H_
#define XNVM_INIT_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifdef PLM_NVM
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
void XNvm_Init(void);

#endif /* PLM_NVM */

#ifdef __cplusplus
}
#endif

#endif /* XNVM_INIT_H_ */
