/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xocp_init.h
 * @cond xocp_internal
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
 *
 * @endcond
 ******************************************************************************/
#ifndef XOCP_INIT_H_
#define XOCP_INIT_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifdef PLM_OCP
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
void XOcp_Init(void);

#endif /* PLM_OCP */

#ifdef __cplusplus
}
#endif

#endif /* XOCP_INIT_H_ */