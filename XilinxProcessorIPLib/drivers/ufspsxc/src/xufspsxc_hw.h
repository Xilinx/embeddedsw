/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xufspsxc_hw.h
* @addtogroup ufspsxc Overview
* @{
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
* </pre>
*
******************************************************************************/
/** @cond INTERNAL */
#ifndef XUFSPSXC_HW_H_		/**< prevent circular inclusions */
#define XUFSPSXC_HW_H_		/**< by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_assert.h"
#include "xil_io.h"
#include "xil_types.h"

/************************** Constant Definitions *****************************/

#define XUFSPSXC_HOST_CTRL_CAP		0x00U
#define XUFSPSXC_UFS_VER			0x08U
#define XUFSPSXC_HCDDID				0x10U
#define XUFSPSXC_HCPMID				0x14U
#define XUFSPSXC_AHIT				0x18U
#define XUFSPSXC_IS					0x20U
#define XUFSPSXC_IS_UTRCS_MASK		0x1U
#define XUFSPSXC_IS_UE_MASK			0x4U
#define XUFSPSXC_IS_ULSS_MASK		0x100U
#define XUFSPSXC_IS_PWR_STS_MASK	0x10U
#define XUFSPSXC_IS_ULLS_MASK		0x80U
#define XUFSPSXC_IS_UCCS_MASK		0x400U

#define XUFSPSXC_HCS				0x30U
#define XUFSPSXC_HCS_DP_MASK		0x1U
#define XUFSPSXC_HCS_UTRLRDY_MASK	0x2U
#define XUFSPSXC_HCS_UCRDY_MASK		0x8U
#define XUFSPSXC_HCS_UPMCRS_MASK	0x700U
#define XUFSPSXC_HCS_UCRDY_SHIFT	0x3U
#define XUFSPSXC_HCS_CCS_MASK		0x800U

#define XUFSPSXC_HCE				0x34U
#define XUFSPSXC_HCE_MASK			0x1U

#define XUFSPSXC_UTRLBA				0x50U
#define XUFSPSXC_UTRLBAU			0x54U
#define XUFSPSXC_UTRLDBR			0x58U
#define XUFSPSXC_UTRLCLR			0x5CU
#define XUFSPSXC_UTRLRSR			0x60U
#define XUFSPSXC_UTRL_RUN			0x1U

#define XUFSPSXC_UTRLCNR			0x64U
#define XUFSPSXC_UICCMD				0x90U
#define XUFSPSXC_UCMDARG1			0x94U
#define XUFSPSXC_UCMDARG2			0x98U
#define XUFSPSXC_UCMDARG2_ResCode_MASK		0xFFU

#define XUFSPSXC_UCMDARG3			0x9CU
#define XUFSPSXC_HCLKDIV			0xFCU

#define XUFSPSXC_BASEADDR			0xF10B0000U
#define XUFSPSXC_IOU_SLCR_REG		0xF1060000U
#define XUFSPSXC_CRP_REG			0xF1260000U
#define XUFSPSXC_EFUSE_CACHE			0xF1250000U

#define XUFSPSXC_UFS_CAL_1			0xBE8U

#define XUFSPSXC_CRP_UFS_RST				0x340U

#define XUFSPSXC_CLK_SEL					0x1044U
#define XUFSPSXC_CFG_CLK_SEL_MASK			0x3FU
#define XUFSPSXC_REF_CLK_SEL_MASK			0xC0U
#define XUFSPSXC_REF_CLK_SEL_SHIFT			0x6U

#define XUFSPSXC_SRAM_CSR					0x104CU
#define XUFSPSXC_SRAM_CSR_BYPASS_MASK		0x4U
#define XUFSPSXC_SRAM_CSR_EXTID_DONE_MASK	0x2U
#define XUFSPSXC_SRAM_CSR_INIT_DONE_MASK	0x1U

#define XUFSPSXC_PHY_RST					0x1050U

#define XUFSPSXC_TX_RX_CFGRDY				0x1054U
#define XUFSPSXC_TX_RX_CFGRDY_MASK			0xFU

/***************** Macros (Inline Functions) Definitions *********************/

#define XUfsPsxc_In32		Xil_In32	/**< Input operation */
#define XUfsPsxc_Out32		Xil_Out32	/**< Output operation */

/*****************************************************************************/
/**
*
* This macro reads the given register.
*
* @param	BaseAddress is the Xilinx base address of the UFSPSXC core.
* @param	RegOffset is the register offset of the register.
*
* @return	The 32-bit value of the register.
*
* @note		C-style signature:
*		u32 XUfsPsxc_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XUfsPsxc_ReadReg(BaseAddress, RegOffset) \
		XUfsPsxc_In32((BaseAddress) + (u32)(RegOffset))

/*****************************************************************************/
/**
*
* This macro writes the value into the given register.
*
* @param	BaseAddress is the Xilinx base address of the UFSPSXC core.
* @param	RegOffset is the register offset of the register.
* @param	Data is the 32-bit value to write to the register.
*
* @return	None.
*
* @note		C-style signature:
*		void XUfsPsxc_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
*
******************************************************************************/
#define XUfsPsxc_WriteReg(BaseAddress, RegOffset, Data) \
		XUfsPsxc_Out32(((BaseAddress) + (u32)(RegOffset)), (u32)(Data))

#ifdef __cplusplus
}
#endif

#endif /* XUFSPSXC_H_ */
/** @endcond */
/** @} */
