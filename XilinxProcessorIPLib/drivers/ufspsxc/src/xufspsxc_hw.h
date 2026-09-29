/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XUFSPSXC_HW_H_ /* prevent circular inclusions */
#define XUFSPSXC_HW_H_

/*****************************************************************************/
/**
*
* @file xufspsxc_hw.h
* @brief Defines UFSPSXC register offsets and access macros.
*
* This file contains low level access functions using the base address
* directly without an instance.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------.
* 1.0   sk  01/16/24 Initial Version.
*
* 1.3   tb  09/16/26 Align Doxygen comments with AMD standards.
* </pre>
*
******************************************************************************/

/**
* @addtogroup ufspsxc_api UFSPSXC APIs
* @{
*/

/** @cond INTERNAL */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_assert.h"
#include "xil_io.h"
#include "xil_types.h"

/************************** Constant Definitions *****************************/

/** Host Controller Capabilities register offset. */
#define XUFSPSXC_HOST_CTRL_CAP		0x00U
/** UFS version register offset. */
#define XUFSPSXC_UFS_VER		0x08U
/** Host Controller Device ID register offset. */
#define XUFSPSXC_HCDDID			0x10U
/** Host Controller Product ID register offset. */
#define XUFSPSXC_HCPMID			0x14U
/** Auto-Hibernate Idle Timer register offset. */
#define XUFSPSXC_AHIT			0x18U
/** Interrupt Status register offset. */
#define XUFSPSXC_IS			0x20U
/** UTP Transfer Request Completion Status interrupt mask. */
#define XUFSPSXC_IS_UTRCS_MASK		0x1U
/** UIC Error interrupt mask. */
#define XUFSPSXC_IS_UE_MASK		0x4U
/** UIC Link Startup Status interrupt mask. */
#define XUFSPSXC_IS_ULSS_MASK		0x100U
/** UIC Power Mode Status interrupt mask. */
#define XUFSPSXC_IS_PWR_STS_MASK	0x10U
/** UIC Link Lost Status interrupt mask. */
#define XUFSPSXC_IS_ULLS_MASK		0x80U
/** UIC Command Completion Status interrupt mask. */
#define XUFSPSXC_IS_UCCS_MASK		0x400U

/** Host Controller Status register offset. */
#define XUFSPSXC_HCS			0x30U
/** Device Present status mask. */
#define XUFSPSXC_HCS_DP_MASK		0x1U
/** UTP Transfer Request List Ready mask. */
#define XUFSPSXC_HCS_UTRLRDY_MASK	0x2U
/** UIC Command Ready mask. */
#define XUFSPSXC_HCS_UCRDY_MASK		0x8U
/** UIC Power Mode Change Request Status mask. */
#define XUFSPSXC_HCS_UPMCRS_MASK	0x700U
/** UIC Command Ready bit shift. */
#define XUFSPSXC_HCS_UCRDY_SHIFT	0x3U
/** Command Completion Status mask. */
#define XUFSPSXC_HCS_CCS_MASK		0x800U

/** Host Controller Enable register offset. */
#define XUFSPSXC_HCE			0x34U
/** Host Controller Enable bit mask. */
#define XUFSPSXC_HCE_MASK		0x1U

/** UTP Transfer Request List Base Address register offset. */
#define XUFSPSXC_UTRLBA			0x50U
/** UTP Transfer Request List Base Address Upper register offset. */
#define XUFSPSXC_UTRLBAU		0x54U
/** UTP Transfer Request List Door Bell register offset. */
#define XUFSPSXC_UTRLDBR		0x58U
/** UTP Transfer Request List Clear register offset. */
#define XUFSPSXC_UTRLCLR		0x5CU
/** UTP Transfer Request List Run Stop register offset. */
#define XUFSPSXC_UTRLRSR		0x60U
/** UTP Transfer Request List run value. */
#define XUFSPSXC_UTRL_RUN		0x1U

/** UTP Transfer Request List Completion Notification register offset. */
#define XUFSPSXC_UTRLCNR		0x64U
/** UIC Command register offset. */
#define XUFSPSXC_UICCMD			0x90U
/** UIC Command Argument 1 register offset. */
#define XUFSPSXC_UCMDARG1		0x94U
/** UIC Command Argument 2 register offset. */
#define XUFSPSXC_UCMDARG2		0x98U
/** UIC command result-code mask in Argument 2. */
#define XUFSPSXC_UCMDARG2_ResCode_MASK	0xFFU

/** UIC Command Argument 3 register offset. */
#define XUFSPSXC_UCMDARG3		0x9CU
/** Host Controller Clock Divider register offset. */
#define XUFSPSXC_HCLKDIV		0xFCU

/** Default UFSPSXC controller base address. */
#define XUFSPSXC_BASEADDR		0xF10B0000U
/** IOU SLCR register base address. */
#define XUFSPSXC_IOU_SLCR_REG		0xF1060000U
/** CRP register base address. */
#define XUFSPSXC_CRP_REG		0xF1260000U
/** eFUSE cache register base address. */
#define XUFSPSXC_EFUSE_CACHE		0xF1250000U

/** UFS calibration register 1 offset. */
#define XUFSPSXC_UFS_CAL_1		0xBE8U

/** UFS reset control register offset. */
#define XUFSPSXC_CRP_UFS_RST		0x340U

/** UFS clock select register offset. */
#define XUFSPSXC_CLK_SEL		0x1044U
/** Configuration clock select field mask. */
#define XUFSPSXC_CFG_CLK_SEL_MASK	0x3FU
/** Reference clock select field mask. */
#define XUFSPSXC_REF_CLK_SEL_MASK	0xC0U
/** Reference clock select field shift. */
#define XUFSPSXC_REF_CLK_SEL_SHIFT	0x6U

/** SRAM control and status register offset. */
#define XUFSPSXC_SRAM_CSR			0x104CU
/** SRAM bypass mode mask. */
#define XUFSPSXC_SRAM_CSR_BYPASS_MASK		0x4U
/** SRAM external-init done mask. */
#define XUFSPSXC_SRAM_CSR_EXTID_DONE_MASK	0x2U
/** SRAM init-done mask. */
#define XUFSPSXC_SRAM_CSR_INIT_DONE_MASK	0x1U

/** PHY reset register offset. */
#define XUFSPSXC_PHY_RST		0x1050U

/** TX/RX configuration-ready register offset. */
#define XUFSPSXC_TX_RX_CFGRDY		0x1054U
/** TX/RX configuration-ready field mask. */
#define XUFSPSXC_TX_RX_CFGRDY_MASK	0xFU

/***************** Macros (Inline Functions) Definitions *********************/

#define XUfsPsxc_In32		Xil_In32	/**< Input operation. */
#define XUfsPsxc_Out32		Xil_Out32	/**< Output operation. */

/*****************************************************************************/
/**
* @brief Reads a 32-bit UFSPSXC register.
*
* This macro reads the given register.
*
* @param[in] BaseAddress Xilinx base address of the UFSPSXC core.
* @param[in] RegOffset Register offset of the register.
*
* @return	The 32-bit value of the register.
*
* @note
* C-style signature
*		u32 XUfsPsxc_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XUfsPsxc_ReadReg(BaseAddress, RegOffset) \
		XUfsPsxc_In32((BaseAddress) + (u32)(RegOffset))

/*****************************************************************************/
/**
* @brief Writes a 32-bit UFSPSXC register.
*
* This macro writes the value into the given register.
*
* @param[in] BaseAddress Xilinx base address of the UFSPSXC core.
* @param[in] RegOffset Register offset of the register.
* @param[in] Data 32-bit value to write to the register.
*
* @note
* C-style signature
*		void XUfsPsxc_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
*
******************************************************************************/
#define XUfsPsxc_WriteReg(BaseAddress, RegOffset, Data) \
		XUfsPsxc_Out32(((BaseAddress) + (u32)(RegOffset)), (u32)(Data))

#ifdef __cplusplus
}
#endif

/** @endcond */
#endif /* XUFSPSXC_HW_H_ */
/** @} */
