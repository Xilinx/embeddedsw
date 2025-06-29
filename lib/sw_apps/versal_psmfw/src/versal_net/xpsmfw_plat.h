/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
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

#define UART0_BASEADDR (0xF1920000U) /**< UART0 base address */
#define UART1_BASEADDR (0xF1930000U) /**< UART1 base address */
#define IOMODULE_BASEADDR (0xEBC80000U) /**< IOMODULE base address */

#define XPAR_XIPIPS_TARGET_PSV_PMC_0_CH0_MASK (0x00000002U) /**< PMC channel 0 mask */

#define PSM_GLOBAL_REG_GLOBAL_CNTRL PSMX_GLOBAL_REG_GLOBAL_CNTRL /**< PSM Global control register */
#define PSM_GLOBAL_REG_GLOBAL_CNTRL_FW_IS_PRESENT_MASK PSMX_GLOBAL_REG_GLOBAL_CNTRL_FW_IS_PRESENT_MASK /**< Check if FW is present */

#define PSM_GLOBAL_REG_ERR1_TRIG PSMX_GLOBAL_REG_PSM_ERR1_TRIG /**< Error trigger register */
#define PSM_GLOBAL_REG_ERR1_TRIG_PSM_B_NCR_MASK PSMX_GLOBAL_REG_PSM_ERR1_TRIG_SRC3_MASK /**< Error trigger register NCR mask */

/** PSM update related save and restore region */
extern u8 __psm_data_saved_start[];
extern u8 __psm_data_saved_end[];
#define XPSM_DATA_SAVED_START	((u32)(__psm_data_saved_start)) /**< Start of saved data region */
#define XPSM_DATA_SAVED_END	((u32)(__psm_data_saved_end)) /**< End of saved data region */

XStatus XPsmfw_PwrUpHandler(void);
XStatus XPsmfw_PwrDwnHandler(void);
XStatus XPsmfw_WakeupHandler(void);
XStatus XPsmfw_PwrCtlHandler(void);
XStatus XPsmFw_GicP2Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* XPSMFW_PLAT_H_ */
