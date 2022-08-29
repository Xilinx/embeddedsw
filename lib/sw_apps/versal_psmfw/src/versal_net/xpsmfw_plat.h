/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpsmfw_plat.h
*
* This file contains definitions related to versal net platform
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who		Date		Changes
* ---- ---- -------- ------------------------------
* 1.00  sr   06/23/2022 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPSMFW_PLAT_H
#define XPSMFW_PLAT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xstatus.h"
#include "psmx_local.h"
#include "psmx_global.h"

/**
 * IPI Base Address
 */
#define IPI_BASEADDR      (0XEB300000U)

#define UART0_BASEADDR (0xF1920000U)
#define UART1_BASEADDR (0xF1930000U)
#define IOMODULE_BASEADDR (0xEBC80000U)

#define XPAR_XIPIPS_TARGET_PSV_PMC_0_CH0_MASK (0x00000002U)

#define PSM_GLOBAL_REG_GLOBAL_CNTRL PSMX_GLOBAL_REG_GLOBAL_CNTRL
#define PSM_GLOBAL_REG_GLOBAL_CNTRL_FW_IS_PRESENT_MASK PSMX_GLOBAL_REG_GLOBAL_CNTRL_FW_IS_PRESENT_MASK

#define PSM_GLOBAL_REG_ERR1_TRIG PSMX_GLOBAL_REG_PSM_ERR1_TRIG
#define PSM_GLOBAL_REG_ERR1_TRIG_PSM_B_NCR_MASK PSMX_GLOBAL_REG_PSM_ERR1_TRIG_SRC3_MASK

XStatus XPsmfw_PwrUpHandler(void);
XStatus XPsmfw_PwrDwnHandler(void);
XStatus XPsmfw_WakeupHandler(void);
XStatus XPsmfw_PwrCtlHandler(void);
XStatus XPsmFw_GicP2Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* XPSMFW_PLAT_H_ */
