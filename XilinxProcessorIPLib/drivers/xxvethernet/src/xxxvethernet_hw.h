/******************************************************************************
* Copyright (C) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
*
* @file xxxvethernet_hw.h
* @addtogroup xxvethernet_v1_6
* @{
*
* This file contains definitions for register offset, masks and low level
* hardware functions.
*
* @note
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.0   hk   6/16/17  First release
*       hk   2/15/18  Add support for USXGMII
* 1.5	sk   10/18/20 Correct the Auto-Negotiation ability macro
*		      name (XXE_ANA_OFFSET) with XXE_ANASR_OFFSET.
*
* </pre>

******************************************************************************/
#ifndef XXXVETHERNET_HW_H		/* prevent circular inclusions */
#define XXXVETHERNET_HW_H		/* by using protection macros */

/***************************** Include Files *********************************/

#include "xdebug.h"

#include "xil_io.h"
#include "xil_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************** Constant Definitions *****************************/

/*
 * Register offset definitions. Unless otherwise noted, register access is
 * 32 bit.
 */

/** @name XXV Ethernet registers offset
 *  @{
 */
#define XXE_GRR_OFFSET		0x00000000
#define XXE_RST_OFFSET		0x00000004
#define XXE_MODE_OFFSET		0x00000008
#define XXE_TXCFG_OFFSET	0x0000000C
#define XXE_RXCFG_OFFSET	0x00000014
#define XXE_RXMTU_OFFSET	0x00000018
#define XXE_TICK_OFFSET		0x00000020
#define XXE_REV_OFFSET		0x00000024
#define XXE_USXGMII_AN_OFFSET	0x000000C8
#define XXE_RSFEC_OFFSET	0x000000D0
#define XXE_FEC_OFFSET		0x000000D4
#define XXE_ANCR1_OFFSET	0x000000E0
#define XXE_ANCR2_OFFSET	0x000000E4
#define XXE_ANACR_OFFSET	0x000000F8
#define XXE_LTCR_OFFSET		0x00000100
#define XXE_LTTR_OFFSET		0x00000104
#define XXE_LTPR_OFFSET		0x00000108
#define XXE_LTIR_OFFSET		0x0000010C
#define XXE_LTSR_OFFSET		0x00000110
#define XXE_LTCOR_OFFSET	0x00000130
#define XXE_USR0_OFFSET		0x00000184
#define XXE_USR1_OFFSET		0x00000188

/** @name Xxv Ethernet status registers offset
 *  @{
 */
#define XXE_CORESPEEDSR_OFFSET	0x00000180
#define XXE_TXSR_OFFSET		0x00000400
#define XXE_RXSR_OFFSET		0x00000404
#define XXE_SR_OFFSET		0x00000408
#define XXE_RXBLSR_OFFSET	0x0000040C
#define XXE_ANSR_OFFSET		0x00000458
#define XXE_ANASR_OFFSET	0x0000045C

/* Register masks. The following constants define bit locations of various
 * bits in the registers. Constants are not defined for those registers
 * that have a single bit field representing all 32 bits. For further
 * information on the meaning of the various bit masks, refer to the HW spec.
 */

/** @name MODE register masks
 * @{
 */
#define XXE_MODE_LCLLPBK_MASK	0x80000000


/** @name TXCFG register masks
 * @{
 */
#define XXE_TXCFG_TX_MASK	0x00000001
#define XXE_TXCFG_FCS_MASK	0x00000002

/** @name RXCFG register masks
 * @{
 */
#define XXE_RXCFG_RX_MASK	0x00000001
#define XXE_RXCFG_DEL_FCS_MASK	0x00000002
#define XXE_RXCFG_IGN_FCS_MASK	0x00000004

/** @name USXGMII Auto negotiation register masks
 * @{
 */
#define XXE_USXGMII_ANBYPASS_MASK	0x00000001
#define XXE_USXGMII_ANENABLE_MASK	0x00000020
#define XXE_USXGMII_ANMAINRESET_MASK	0x00000040
#define XXE_USXGMII_ANRESTART_MASK	0x00000080
#define XXE_USXGMII_RATE_MASK		0x00000700
#define XXE_USXGMII_ANA_MASK		0x00010000
#define XXE_USXGMII_ANA_SPEED_MASK	0x0E000000
#define XXE_USXGMII_ANA_FD_MASK		0x10000000
#define XXE_USXGMII_ANACK_MASK		0x40000000
#define XXE_USXGMII_LINK_STS_MASK	0x80000000

#define XXE_USXGMII_RATE_10M_MASK	0x0
#define XXE_USXGMII_RATE_100M_MASK	0x1
#define XXE_USXGMII_RATE_1G_MASK	0x2
#define XXE_USXGMII_RATE_10G_MASK	0x3
#define XXE_USXGMII_RATE_2G5_MASK	0x4
#define XXE_USXGMII_RATE_SHIFT		8
#define XXE_USXGMII_SPEED_SHIFT		25

/** @name RXMTU register masks
 * @{
 */
#define XXE_RXMTU_MIN_JUM_MASK	0x000000FF
#define XXE_RXMTU_MAX_JUM_MASK	0x7FFF0000

/** @name RXBLSR register masks
 * @{
 */
#define XXE_RXBLKLCK_MASK	0x00000001

/** @name TICK register masks
 * @{
 */
#define XXE_TICK_STATEN_MASK	0x00000001

/** @name AN status register masks
 * @{
 */
#define XXE_AN_COMP_MASK	0x00000004
#define XXE_USXGMII_AN_COMP_MASK	0x00010000

/** @name AN ability register masks
 * @{
 */
#define XXE_ANA_10GKR_MASK	0x00000004


/** @name Reset and Address Filter (RAF) Register bit definitions.
 *  These bits are associated with the XAE_RAF_OFFSET register.
 * @{
 */
#define XXE_RAF_STATSRST_MASK  	0x00002000 	   /**< Statistics Counter
						    *   Reset
						    */
#define XXE_RAF_RXBADFRMEN_MASK     	0x00004000 /**< Receive Bad Frame
						    *   Enable
						    */
/*@}*/

/** @name Transmit Inter-Frame Gap Adjustment Register (TFGP) bit definitions
 *  @{
 */
#define XXE_TFGP_IFGP_MASK		0x0000007F /**< Transmit inter-frame
					            *   gap adjustment value
					            */
/*@}*/

#define XXE_STS_TX_ERROR_MASK		0x1
#define XXE_STS_RX_ERROR_MASK		0xFF0


/** @name Other Constant definitions used in the driver
 * @{
 */

#define XXE_SPEED_10_GBPS		10	/**< Speed of 10 Gbps */

#define XXE_LOOPS_TO_COME_OUT_OF_RST	10000	/**< Number of loops in the driver
						 *   API to wait for before
						 *   returning a failure case.
						 */

#define XXE_RST_DELAY_LOOPCNT_VAL	10000	/**< Timeout in ticks used
						  *  while checking if the core
						  *  had come out of reset. The
						  *  exact tick time is defined
						  *  in each case/loop where it
						  *  will be used
						  */
/*@}*/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
xdbg_stmnt(extern int indent_on);

#define XXxvEthernet_indent(RegOffset) \
 ((indent_on && ((RegOffset) >= XXE_GRR_OFFSET) && ((RegOffset) <= 	\
 XXE_ANASR_OFFSET)) ? "\t" : "")


#define XXxvEthernet_reg_name(RegOffset) \
	(((RegOffset) == XXE_GRR_OFFSET) ? "XXE_GRR_OFFSET": \
	((RegOffset) == XXE_RST_OFFSET) ? "XXE_RST_OFFSET": \
	((RegOffset) == XXE_MODE_OFFSET) ? "XXE_MODE_OFFSET": \
	((RegOffset) == XXE_TXCFG_OFFSET) ? "XXE_TXCFG_OFFSET": \
	((RegOffset) == XXE_RXCFG_OFFSET) ? "XXE_RXCFG_OFFSET": \
	((RegOffset) == XXE_RXMTU_OFFSET) ? "XXE_RXMTU_OFFSET": \
	((RegOffset) == XXE_TICK_OFFSET) ? "XXE_TICK_OFFSET": \
	((RegOffset) == XXE_REV_OFFSET) ? "XXE_REV_OFFSET": \
	((RegOffset) == XXE_TXSR_OFFSET) ? "XXE_TXSR_OFFSET": \
	((RegOffset) == XXE_RXSR_OFFSET) ? "XXE_RXSR_OFFSET": \
	((RegOffset) == XXE_SR_OFFSET) ? "XXE_SR_OFFSET": \
	((RegOffset) == XXE_RXBLSR_OFFSET) ? "XXE_RXBLSR_OFFSET": \
	((RegOffset) == XXE_ANSR_OFFSET) ? "XXE_ANSR_OFFSET": \
	((RegOffset) == XXE_ANASR_OFFSET) ? "XXE_ANASR_OFFSET": \
	"unknown")

#define XXxvEthernet_print_reg_o(BaseAddress, RegOffset, Value) 	\
	xdbg_printf(XDBG_DEBUG_TEMAC_REG, "%s0x%0x -> %s(0x%0x)\n", 	\
			XXxvEthernet_indent(RegOffset), (Value), 	\
			XXxvEthernet_reg_name(RegOffset), (RegOffset)) 	\

#define XXxvEthernet_print_reg_i(BaseAddress, RegOffset, Value) \
	xdbg_printf(XDBG_DEBUG_TEMAC_REG, "%s%s(0x%0x) -> 0x%0x\n", \
		XXxvEthernet_indent(RegOffset),  \
		XXxvEthernet_reg_name(RegOffset),(RegOffset), (Value)) \

/****************************************************************************/
/**
*
* XXxvEthernet_ReadReg returns the value read from the register specified by
* <i>RegOffset</i>.
*
* @param	BaseAddress is the base address of the Xxv Ethernet device.
* @param	RegOffset is the offset of the register to be read.
*
* @return	Returns the 32-bit value of the register.
*
* @note		C-style signature:
*		u32 XXxvEthernet_ReadReg(u32 BaseAddress, u32 RegOffset)
*
*****************************************************************************/
#ifdef DEBUG
#define XXxvEthernet_ReadReg(BaseAddress, RegOffset) 			\
({									\
	u32 value; 							\
	value = Xil_In32(((BaseAddress) + (RegOffset))); 		\
	XXxvEthernet_print_reg_i((BaseAddress), (RegOffset), value);	\
})
#else
#define XXxvEthernet_ReadReg(BaseAddress, RegOffset) 			\
	(Xil_In32(((BaseAddress) + (RegOffset))))
#endif

/****************************************************************************/
/**
*
* XXxvEthernet_WriteReg, writes <i>Data</i> to the register specified by
* <i>RegOffset</i>.
*
* @param	BaseAddress is the base address of the Xxv Ethernet device.
* @param	RegOffset is the offset of the register to be written.
* @param	Data is the 32-bit value to write to the register.
*
* @return	None.
*
* @note
* 	C-style signature:
*	void XXxvEthernet_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
*
*****************************************************************************/
#ifdef DEBUG
#define XXxvEthernet_WriteReg(BaseAddress, RegOffset, Data)		\
({ 									\
	XXxvEthernet_print_reg_o((BaseAddress), (RegOffset), (Data));	\
	Xil_Out32(((BaseAddress) + (RegOffset)), (Data)); 		\
})
#else
#define XXxvEthernet_WriteReg(BaseAddress, RegOffset, Data) \
	Xil_Out32(((BaseAddress) + (RegOffset)), (Data))
#endif


#ifdef __cplusplus
  }
#endif

#endif /* end of protection macro */
/** @} */
