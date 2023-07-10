/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
* @file xpciepsu_hw.h
*
* This header file contains identifiers and basic driver functions for the
* XPciePsu device driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 1.0	bs	08/21/2018	First release
* </pre>
*
*******************************************************************************/
#ifndef XPCIEPSU_HW_H /* prevent circular inclusions */
#define XPCIEPSU_HW_H /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/******************************** Include Files *******************************/
#include "xil_types.h"


/**************************** Constant Definitions ****************************/


/** @name Registers
*
* Register offsets for this device. Some of the registers
* are configurable at hardware build time such that may or may not exist
* in the hardware.
* @{
*/

/* helper mecros */
#define BIT(x) 					(((u32)1U) << (x))

#define BITSPERLONGLONG				64U

#define GENMASK(h, l) \
	(((~0ULL) << (l)) & (~0ULL >> (BITSPERLONGLONG - 1U - (h))))

/* PCI Express hardcore configuration register offset */
#define XPCIEPSU_PCIE_CORE_OFFSET               0x0U

/* Bridge core config registers */
#define XPCIEPSU_BRCFG_RX0			0x00000000U
#define XPCIEPSU_BRCFG_RX_MSG_FILTER 		0x00000020U

/* Egress - Bridge translation registers */
#define XPCIEPSU_E_BREG_CAPABILITIES 		0x00000200U
#define XPCIEPSU_E_BREG_CONTROL 		0x00000208U
#define XPCIEPSU_E_BREG_BASE_LO 		0x00000210U
#define XPCIEPSU_E_BREG_BASE_HI 		0x00000214U

#define XPCIEPSU_E_ECAM_CAPABILITIES 		0x00000220U
#define XPCIEPSU_E_ECAM_CONTROL 		0x00000228U
#define XPCIEPSU_E_ECAM_BASE_LO 		0x00000230U
#define XPCIEPSU_E_ECAM_BASE_HI 		0x00000234U

#define XPCIEPSU_I_ISUB_CONTROL 		0x000003E8U
#define SET_ISUB_CONTROL 			BIT(0U)
/* Rxed msg fifo  - Interrupt status registers */
#define XPCIEPSU_MSGF_MISC_MASK 		0x00000404U
#define XPCIEPSU_MSGF_LEG_MASK 			0x00000424U


/* Msg filter mask bits */
#define CFG_ENABLE_PM_MSG_FWD 			BIT(1U)
#define CFG_ENABLE_INT_MSG_FWD 			BIT(2U)
#define CFG_ENABLE_ERR_MSG_FWD 			BIT(3U)
#define CFG_ENABLE_MSG_FILTER_MASK                                             \
	(CFG_ENABLE_PM_MSG_FWD | CFG_ENABLE_INT_MSG_FWD                        \
	 | CFG_ENABLE_ERR_MSG_FWD)

/* Misc interrupt status mask bits */
#define MSGF_MISC_SR_RXMSG_AVAIL 		BIT(0U)
#define MSGF_MISC_SR_RXMSG_OVER 		BIT(1U)
#define MSGF_MISC_SR_SLAVE_ERR 			BIT(4U)
#define MSGF_MISC_SR_MASTER_ERR 		BIT(5U)
#define MSGF_MISC_SR_I_ADDR_ERR 		BIT(6U)
#define MSGF_MISC_SR_E_ADDR_ERR 		BIT(7U)
#define MSGF_MISC_SR_FATAL_AER 			BIT(16U)
#define MSGF_MISC_SR_NON_FATAL_AER 		BIT(17U)
#define MSGF_MISC_SR_CORR_AER 			BIT(18U)
#define MSGF_MISC_SR_UR_DETECT 			BIT(20U)
#define MSGF_MISC_SR_NON_FATAL_DEV 		BIT(22U)
#define MSGF_MISC_SR_FATAL_DEV 			BIT(23U)
#define MSGF_MISC_SR_LINK_DOWN 			BIT(24U)
#define MSGF_MSIC_SR_LINK_AUTO_BWIDTH		BIT(25U)
#define MSGF_MSIC_SR_LINK_BWIDTH 		BIT(26U)

#define MSGF_MISC_SR_MASKALL                                                   \
	(MSGF_MISC_SR_RXMSG_AVAIL | MSGF_MISC_SR_RXMSG_OVER                    \
	 | MSGF_MISC_SR_SLAVE_ERR | MSGF_MISC_SR_MASTER_ERR                    \
	 | MSGF_MISC_SR_I_ADDR_ERR | MSGF_MISC_SR_E_ADDR_ERR                   \
	 | MSGF_MISC_SR_FATAL_AER | MSGF_MISC_SR_NON_FATAL_AER                 \
	 | MSGF_MISC_SR_CORR_AER | MSGF_MISC_SR_UR_DETECT                      \
	 | MSGF_MISC_SR_NON_FATAL_DEV | MSGF_MISC_SR_FATAL_DEV                 \
	 | MSGF_MISC_SR_LINK_DOWN | MSGF_MSIC_SR_LINK_AUTO_BWIDTH              \
	 | MSGF_MSIC_SR_LINK_BWIDTH)


/* Legacy interrupt status mask bits */
#define MSGF_LEG_SR_INTA 			BIT(0U)
#define MSGF_LEG_SR_INTB 			BIT(1U)
#define MSGF_LEG_SR_INTC 			BIT(2U)
#define MSGF_LEG_SR_INTD 			BIT(3U)
#define MSGF_LEG_SR_MASKALL                                                    \
	(MSGF_LEG_SR_INTA | MSGF_LEG_SR_INTB | MSGF_LEG_SR_INTC                \
	 | MSGF_LEG_SR_INTD)

/* Bridge config interrupt mask */
#define BREG_PRESENT 				BIT(0U)
#define BREG_ENABLE 				BIT(0U)
#define BREG_ENABLE_FORCE 			BIT(1U)

/* E_ECAM status mask bits */
#define E_ECAM_PRESENT 				BIT(0U)
#define E_ECAM_CR_ENABLE 			BIT(0U)
#define E_ECAM_SIZE_LOC 			GENMASK(20U, 16U)
#define E_ECAM_SIZE_MIN 			GENMASK(23U, 16U)
#define E_ECAM_SIZE_SHIFT 			16U
#define PSU_ECAM_VALUE_DEFAULT			12U

#define CFG_DMA_REG_BAR 			GENMASK(2U, 0U)

/* Reading the PS_LINKUP */
#define XPCIEPSU_PS_LINKUP_OFFSET 		0x00000238U
#define XPCIEPSU_PHY_LINKUP_BIT 		BIT(0U)
#define XPCIEPSU_XPHY_RDY_LINKUP_BIT 		BIT(1U)

/* Parameters for the waiting for link up routine */
#define XPCIEPSU_LINK_WAIT_MAX_RETRIES 		10U
#define XPCIEPSU_LINK_WAIT_USLEEP_MIN 		90000U

/** @name ECAM Address Register bitmaps and masks
*
* @{
*/
#define XPCIEPSU_ECAM_MASK	0x0FFFFFFFU     /**< Mask of all valid bits */
#define XPCIEPSU_ECAM_BUS_MASK	0x0FF00000U /**< Bus Number Mask */
#define XPCIEPSU_ECAM_DEV_MASK	0x000F8000U /**< Device Number Mask */
#define XPCIEPSU_ECAM_FUN_MASK	0x00007000U /**< Function Number Mask */
#define XPCIEPSU_ECAM_REG_MASK	0x00000FFCU /**< Register Number Mask */

#define XPCIEPSU_ECAM_BUS_SHIFT	20U /**< Bus Number Shift Value */
#define XPCIEPSU_ECAM_DEV_SHIFT	15U /**< Device Number Shift Value */
#define XPCIEPSU_ECAM_FUN_SHIFT	12U /**< Function Number Shift Value */
#define XPCIEPSU_ECAM_REG_SHIFT	2U  /**< Register Number Shift Value */
/*@}*/
/******************** Macros (Inline Functions) Definitions *******************/
/**************************** Variable Definitions ****************************/

/***************************** Function Prototypes ****************************/
#ifdef __cplusplus
}
#endif

#endif /* XPCIEPSU_HW_H */

/** @} */
