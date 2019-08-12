/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
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
#define BIT(x) 								(1 << (x))

/* each bus required 1 MB ecam space */
#define GET_MAX_BUS_NO(ecam_sz) 			(((ecam_sz) / (1024 * 1024)) - 1)

#define BITSPERLONG 						64
#define GENMASK(h, l) (((~0ULL) << (l)) & (~0ULL) >> (BITSPERLONG - 1 - (h)))

/* Bridge core config registers */
#define XPCIEPSU_BRCFG_RX0 							0x00000000
#define XPCIEPSU_BRCFG_INTERRUPT 					0x00000010
#define XPCIEPSU_BRCFG_RX_MSG_FILTER 				0x00000020

/* Egress - Bridge translation registers */
#define XPCIEPSU_E_BREG_CAPABILITIES 					0x00000200
#define XPCIEPSU_E_BREG_CONTROL 						0x00000208
#define XPCIEPSU_E_BREG_BASE_LO 						0x00000210
#define XPCIEPSU_E_BREG_BASE_HI 						0x00000214
#define XPCIEPSU_E_ECAM_CAPABILITIES 					0x00000220
#define XPCIEPSU_E_ECAM_CONTROL 						0x00000228
#define XPCIEPSU_E_ECAM_BASE_LO 						0x00000230
#define XPCIEPSU_E_ECAM_BASE_HI 						0x00000234

#define XPCIEPSU_I_ISUB_CONTROL 						0x000003E8
#define SET_ISUB_CONTROL 								BIT(0)
/* Rxed msg fifo  - Interrupt status registers */
#define XPCIEPSU_MSGF_MISC_MASK 						0x00000404
#define XPCIEPSU_MSGF_LEG_MASK 							0x00000424


/* Msg filter mask bits */
#define CFG_ENABLE_PM_MSG_FWD 			BIT(1)
#define CFG_ENABLE_INT_MSG_FWD 			BIT(2)
#define CFG_ENABLE_ERR_MSG_FWD 			BIT(3)
#define CFG_ENABLE_MSG_FILTER_MASK                                             \
	(CFG_ENABLE_PM_MSG_FWD | CFG_ENABLE_INT_MSG_FWD                        \
	 | CFG_ENABLE_ERR_MSG_FWD)

/* Misc interrupt status mask bits */
#define MSGF_MISC_SR_RXMSG_AVAIL 		BIT(0)
#define MSGF_MISC_SR_RXMSG_OVER 		BIT(1)
#define MSGF_MISC_SR_SLAVE_ERR 			BIT(4)
#define MSGF_MISC_SR_MASTER_ERR 		BIT(5)
#define MSGF_MISC_SR_I_ADDR_ERR 		BIT(6)
#define MSGF_MISC_SR_E_ADDR_ERR 		BIT(7)
#define MSGF_MISC_SR_FATAL_AER 			BIT(16)
#define MSGF_MISC_SR_NON_FATAL_AER 		BIT(17)
#define MSGF_MISC_SR_CORR_AER 			BIT(18)
#define MSGF_MISC_SR_UR_DETECT 			BIT(20)
#define MSGF_MISC_SR_NON_FATAL_DEV 		BIT(22)
#define MSGF_MISC_SR_FATAL_DEV 			BIT(23)
#define MSGF_MISC_SR_LINK_DOWN 			BIT(24)
#define MSGF_MSIC_SR_LINK_AUTO_BWIDTH 	BIT(25)
#define MSGF_MSIC_SR_LINK_BWIDTH 		BIT(26)

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
#define MSGF_LEG_SR_INTA 			BIT(0)
#define MSGF_LEG_SR_INTB 			BIT(1)
#define MSGF_LEG_SR_INTC 			BIT(2)
#define MSGF_LEG_SR_INTD 			BIT(3)
#define MSGF_LEG_SR_MASKALL                                                    \
	(MSGF_LEG_SR_INTA | MSGF_LEG_SR_INTB | MSGF_LEG_SR_INTC                \
	 | MSGF_LEG_SR_INTD)

/* Bridge config interrupt mask */
#define BREG_PRESENT 				BIT(0)
#define BREG_ENABLE 				BIT(0)
#define BREG_ENABLE_FORCE 			BIT(1)

/* E_ECAM status mask bits */
#define E_ECAM_PRESENT 				BIT(0)
#define E_ECAM_CR_ENABLE 			BIT(0)
#define E_ECAM_SIZE_LOC 			GENMASK(20, 16)
#define E_ECAM_SIZE_MIN 			GENMASK(23, 16)
#define E_ECAM_SIZE_SHIFT 			16
#define PSU_ECAM_VALUE_DEFAULT 		12

#define CFG_DMA_REG_BAR 			GENMASK(2, 0)

/* Reading the PS_LINKUP */
#define XPCIEPSU_PS_LINKUP_OFFSET 			0x00000238
#define XPCIEPSU_PHY_LINKUP_BIT 		BIT(0)
#define XPCIEPSU_XPHY_RDY_LINKUP_BIT 			BIT(1)

/* Parameters for the waiting for link up routine */
#define XPCIEPSU_LINK_WAIT_MAX_RETRIES 		10
#define XPCIEPSU_LINK_WAIT_USLEEP_MIN 		90000

#define XPciePsu_IsEcamBusy(InstancePtr)                                       \
	((XPciePsu_ReadReg((InstancePtr)->Config.Ecam, PCIEPSU_BSC_OFFSET)     \
	  & PCIEPSU_BSC_ECAM_BUSY_MASK)                                        \
		 ? TRUE                                                        \
		 : FALSE)


/** @name ECAM Address Register bitmaps and masks
*
* @{
*/
#define XPCIEPSU_ECAM_MASK 0x0FFFFFFF     /**< Mask of all valid bits */
#define XPCIEPSU_ECAM_BUS_MASK 0x0FF00000 /**< Bus Number Mask */
#define XPCIEPSU_ECAM_DEV_MASK 0x000F8000 /**< Device Number Mask */
#define XPCIEPSU_ECAM_FUN_MASK 0x00007000 /**< Function Number Mask */
#define XPCIEPSU_ECAM_REG_MASK 0x00000FFC /**< Register Number Mask */

#define XPCIEPSU_ECAM_BUS_SHIFT 20 /**< Bus Number Shift Value */
#define XPCIEPSU_ECAM_DEV_SHIFT 15 /**< Device Number Shift Value */
#define XPCIEPSU_ECAM_FUN_SHIFT 12 /**< Function Number Shift Value */
#define XPCIEPSU_ECAM_REG_SHIFT 2  /**< Register Number Shift Value */
/*@}*/
/******************** Macros (Inline Functions) Definitions *******************/
/**************************** Variable Definitions ****************************/

/***************************** Function Prototypes ****************************/
#ifdef __cplusplus
}
#endif

#endif /* XPCIEPSU_HW_H */

/** @} */
