/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XGPIOPS_HW_H
#define XGPIOPS_HW_H

/*****************************************************************************/
/**
*
* @file xgpiops_hw.h
* @brief Defines identifiers and basic functions for GPIO PS device access.
*
* This header file contains the identifiers and basic driver functions (or
* macros) that you can use to access the device. The other driver functions
* are defined in xgpiops.h.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------
* 1.00a sv   01/15/10 First Release
* 1.02a hk   08/22/13 Added low level reset API function prototype and
*                     related constant definitions
* 3.00  kvn  02/13/15 Modified code for MISRA-C:2012 compliance.
* 3.1	kvn  04/13/15 Corrected reset values of banks.
* 3.5   sne  03/14/19 Added versal support.
* 3.16  vn   09/10/26 Align Doxygen comments with AMD standards
*
* </pre>
*
******************************************************************************/

/** @addtogroup gpiops Overview
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xil_io.h"

/************************** Constant Definitions *****************************/

/** @name Register offsets for the GPIO. Each register is 32 bits.
 *
 * In the following descriptions, RW is read/write, RO is read-only, and WO
 * is write-only. LSW is the least significant word and MSW is the most
 * significant word.
 * @{
 */
/** Mask and Data Register LSW, WO. */
#define XGPIOPS_DATA_LSW_OFFSET  0x00000000U
/** Mask and Data Register MSW, WO. */
#define XGPIOPS_DATA_MSW_OFFSET  0x00000004U
/** Data Register, RW. */
#define XGPIOPS_DATA_OFFSET	 0x00000040U
/** Data Register - Input, RO. */
#define XGPIOPS_DATA_RO_OFFSET	 0x00000060U
/** Direction Mode Register, RW. */
#define XGPIOPS_DIRM_OFFSET	 0x00000204U
/** Output Enable Register, RW. */
#define XGPIOPS_OUTEN_OFFSET	 0x00000208U
/** Interrupt Mask Register, RO. */
#define XGPIOPS_INTMASK_OFFSET	 0x0000020CU
/** Interrupt Enable Register, WO. */
#define XGPIOPS_INTEN_OFFSET	 0x00000210U
/** Interrupt Disable Register, WO. */
#define XGPIOPS_INTDIS_OFFSET	 0x00000214U
/** Interrupt Status Register, RO. */
#define XGPIOPS_INTSTS_OFFSET	 0x00000218U
/** Interrupt Type Register, RW. */
#define XGPIOPS_INTTYPE_OFFSET	 0x0000021CU
/** Interrupt Polarity Register, RW. */
#define XGPIOPS_INTPOL_OFFSET	 0x00000220U
/** Interrupt On Any Register, RW. */
#define XGPIOPS_INTANY_OFFSET	 0x00000224U
/** @} */

/** @name Register offsets for each Bank.
 *  @{
 */
/** Data/Mask Registers offset. */
#define XGPIOPS_DATA_MASK_OFFSET 0x00000008U
/** Data Registers offset. */
#define XGPIOPS_DATA_BANK_OFFSET 0x00000004U
/** Registers offset. */
#define XGPIOPS_REG_MASK_OFFSET  0x00000040U
/** @} */

/* For backwards compatibility */
/** Mask for backward compatibility support. */
#define XGPIOPS_BYPM_MASK_OFFSET	(u32)0x40

/** @name Interrupt type reset values for each bank
 *  @{
 */
#ifdef XPAR_PSU_GPIO_0_BASEADDR
/** Bank 0 interrupt type reset value specific to Zynq Ultrascale+ MP. */
#define XGPIOPS_INTTYPE_BANK0_RESET  0x03FFFFFFU
/** Bank 1 interrupt type reset value specific to Zynq Ultrascale+ MP. */
#define XGPIOPS_INTTYPE_BANK1_RESET  0x03FFFFFFU
/** Bank 2 interrupt type reset value specific to Zynq Ultrascale+ MP. */
#define XGPIOPS_INTTYPE_BANK2_RESET  0x03FFFFFFU
#else
/** Bank 0 interrupt type reset value specific to Zynq. */
#define XGPIOPS_INTTYPE_BANK0_RESET  0xFFFFFFFFU
/** Bank 1 interrupt type reset value specific to Zynq. */
#define XGPIOPS_INTTYPE_BANK1_RESET  0x003FFFFFU
/** Bank 2 interrupt type reset value specific to Zynq. */
#define XGPIOPS_INTTYPE_BANK2_RESET  0xFFFFFFFFU
#endif

/** Bank 3 interrupt type reset value common to both platforms. */
#define XGPIOPS_INTTYPE_BANK3_RESET  0xFFFFFFFFU
/** Bank 4 interrupt type reset value specific to Zynq Ultrascale+ MP. */
#define XGPIOPS_INTTYPE_BANK4_RESET  0xFFFFFFFFU
/** Bank 5 interrupt type reset value specific to Zynq Ultrascale+ MP. */
#define XGPIOPS_INTTYPE_BANK5_RESET  0xFFFFFFFFU
/** @} */

/** Flag for the base address of PS_GPIO in Versal. */
#define XGPIOPS_PS_GPIO_BASEADDR     0xFF0B0000U
#define XGPIOPS_ZERO    0U  /**< Flag for value 0. */
#define XGPIOPS_ONE     1U  /**< Flag for value 1. */
#define XGPIOPS_TWO     2U  /**< Flag for value 2. */
#define XGPIOPS_THREE   3U  /**< Flag for value 3. */
#define XGPIOPS_FOUR    4U  /**< Flag for value 4. */
#define XGPIOPS_SIX     6U  /**< Flag for value 6. */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/****************************************************************************/
/**
*
* @brief Reads the given register.
*
* @param[in]   BaseAddr Base address of the device.
* @param[in]   RegOffset Offset of the register to read.
*
* @return      The 32-bit value of the register.
*
* @note        C-style signature:
*              u32 XGpioPs_ReadReg(UINTPTR BaseAddr, u32 RegOffset)
*
*****************************************************************************/
#define XGpioPs_ReadReg(BaseAddr, RegOffset)		\
		Xil_In32((BaseAddr) + (u32)(RegOffset))

/****************************************************************************/
/**
*
* @brief Writes to the given register.
*
* @param[in]   BaseAddr Base address of the device.
* @param[in]   RegOffset Offset of the register to write.
* @param[in]   Data 32-bit value to write to the register.
*
* @note        C-style signature:
*              void XGpioPs_WriteReg(UINTPTR BaseAddr, u32 RegOffset,
*                                    u32 Data)
*
*****************************************************************************/
#define XGpioPs_WriteReg(BaseAddr, RegOffset, Data)	\
		Xil_Out32((BaseAddr) + (u32)(RegOffset), (u32)(Data))

/************************** Function Prototypes ******************************/

void XGpioPs_ResetHw(UINTPTR BaseAddress);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/** @} */
#endif /* XGPIOPS_HW_H */
