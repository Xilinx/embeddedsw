/******************************************************************************
* Copyright (C) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
*
* @file xxxvethernet_hw.h
* @addtogroup xxvethernet Overview
* @{
*
* Defines register offsets, masks, and low-level hardware functions.
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
 * 32-bit.
 */

/** @name XXV Ethernet registers offset
 *  @{
 */
#define XXE_GRR_OFFSET		0x00000000 /**< General register reset offset */
#define XXE_RST_OFFSET		0x00000004 /**< Reset register offset */
#define XXE_MODE_OFFSET		0x00000008 /**< Mode register offset */
#define XXE_TXCFG_OFFSET	0x0000000C /**< Transmit configuration offset */
#define XXE_RXCFG_OFFSET	0x00000014 /**< Receive configuration offset */
#define XXE_RXMTU_OFFSET	0x00000018 /**< Receive MTU register offset */
#define XXE_TICK_OFFSET		0x00000020 /**< Tick register offset */
#define XXE_REV_OFFSET		0x00000024 /**< Revision register offset */
#define XXE_USXGMII_AN_OFFSET	0x000000C8 /**< USXGMII autonegotiation offset */
#define XXE_RSFEC_OFFSET	0x000000D0 /**< RS-FEC register offset */
#define XXE_FEC_OFFSET		0x000000D4 /**< FEC register offset */
#define XXE_ANCR1_OFFSET	0x000000E0 /**< Autonegotiation control 1 offset */
#define XXE_ANCR2_OFFSET	0x000000E4 /**< Autonegotiation control 2 offset */
#define XXE_ANACR_OFFSET	0x000000F8 /**< Autonegotiation ability offset */
#define XXE_LTCR_OFFSET		0x00000100 /**< Link training control offset */
#define XXE_LTTR_OFFSET		0x00000104 /**< Link training timer offset */
#define XXE_LTPR_OFFSET		0x00000108 /**< Link training preset offset */
#define XXE_LTIR_OFFSET		0x0000010C /**< Link training interval offset */
#define XXE_LTSR_OFFSET		0x00000110 /**< Link training status offset */
#define XXE_LTCOR_OFFSET	0x00000130 /**< Link training control offset */
#define XXE_USR0_OFFSET		0x00000184 /**< User register 0 offset */
#define XXE_USR1_OFFSET		0x00000188 /**< User register 1 offset */
/** @} */

/** @name Xxv Ethernet status registers offset
 *  @{
 */
#define XXE_CORESPEEDSR_OFFSET	0x00000180 /**< Core speed status offset */
#define XXE_TXSR_OFFSET		0x00000400 /**< Transmit status register offset */
#define XXE_RXSR_OFFSET		0x00000404 /**< Receive status register offset */
#define XXE_SR_OFFSET		0x00000408 /**< Status register offset */
#define XXE_RXBLSR_OFFSET	0x0000040C /**< Receive block lock status offset */
#define XXE_ANSR_OFFSET		0x00000458 /**< Autonegotiation status offset */
#define XXE_ANASR_OFFSET	0x0000045C /**< Autonegotiation ability status offset */
/** @} */

/* Register masks. The following constants define bit locations of various
 * bits in the registers. Constants are not defined for those registers
 * that have a single bit field representing all 32 bits. For further
 * information on the meaning of the various bit masks, refer to the HW spec.
 */

/** @name MODE register masks
 * @{
 */
#define XXE_MODE_LCLLPBK_MASK	0x80000000 /**< Local loopback mode mask */
/** @} */

/** @name TXCFG register masks
 * @{
 */
#define XXE_TXCFG_TX_MASK	0x00000001 /**< Transmit enable mask */
#define XXE_TXCFG_FCS_MASK	0x00000002 /**< FCS insertion mask */
/** @} */

/** @name RXCFG register masks
 * @{
 */
#define XXE_RXCFG_RX_MASK	0x00000001 /**< Receive enable mask */
#define XXE_RXCFG_DEL_FCS_MASK	0x00000002 /**< FCS strip mask */
#define XXE_RXCFG_IGN_FCS_MASK	0x00000004 /**< FCS ignore mask */
/** @} */

/** @name USXGMII Auto negotiation register masks
 * @{
 */
#define XXE_USXGMII_ANBYPASS_MASK	0x00000001 /**< Autonegotiation bypass mask */
#define XXE_USXGMII_ANENABLE_MASK	0x00000020 /**< Autonegotiation enable mask */
#define XXE_USXGMII_ANMAINRESET_MASK	0x00000040 /**< Autonegotiation main reset mask */
#define XXE_USXGMII_ANRESTART_MASK	0x00000080 /**< Autonegotiation restart mask */
#define XXE_USXGMII_RATE_MASK		0x00000700 /**< Rate field mask */
#define XXE_USXGMII_ANA_MASK		0x00010000 /**< Autonegotiation ability mask */
#define XXE_USXGMII_ANA_SPEED_MASK	0x0E000000 /**< Autonegotiation speed mask */
#define XXE_USXGMII_ANA_FD_MASK		0x10000000 /**< Full duplex ability mask */
#define XXE_USXGMII_ANACK_MASK		0x40000000 /**< Autonegotiation acknowledge mask */
#define XXE_USXGMII_LINK_STS_MASK	0x80000000 /**< Link status mask */

#define XXE_USXGMII_RATE_10M_MASK	0x0 /**< 10M rate value */
#define XXE_USXGMII_RATE_100M_MASK	0x1 /**< 100M rate value */
#define XXE_USXGMII_RATE_1G_MASK	0x2 /**< 1G rate value */
#define XXE_USXGMII_RATE_10G_MASK	0x3 /**< 10G rate value */
#define XXE_USXGMII_RATE_2G5_MASK	0x4 /**< 2.5G rate value */
#define XXE_USXGMII_RATE_SHIFT		8 /**< Rate field shift */
#define XXE_USXGMII_SPEED_SHIFT		25 /**< Speed field shift */
/** @} */

/** @name RXMTU register masks
 * @{
 */
#define XXE_RXMTU_MIN_JUM_MASK	0x000000FF /**< Minimum jumbo frame size mask */
#define XXE_RXMTU_MAX_JUM_MASK	0x7FFF0000 /**< Maximum jumbo frame size mask */
/** @} */

/** @name RXBLSR register masks
 * @{
 */
#define XXE_RXBLKLCK_MASK	0x00000001 /**< Receive block lock mask */
/** @} */

/** @name TICK register masks
 * @{
 */
#define XXE_TICK_STATEN_MASK	0x00000001 /**< Statistics enable mask */
/** @} */

/** @name AN status register masks
 * @{
 */
#define XXE_AN_COMP_MASK	0x00000004 /**< Autonegotiation complete mask */
#define XXE_USXGMII_AN_COMP_MASK	0x00010000 /**< USXGMII autonegotiation complete mask */
/** @} */

/** @name AN ability register masks
 * @{
 */
#define XXE_ANA_10GKR_MASK	0x00000004 /**< 10G KR ability mask */
/** @} */

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
/** @} */

/** @name Transmit Inter-Frame Gap Adjustment Register (TFGP) bit definitions
 *  @{
 */
#define XXE_TFGP_IFGP_MASK		0x0000007F /**< Transmit inter-frame
					            *   gap adjustment value
					            */
/** @} */

#define XXE_STS_TX_ERROR_MASK		0x1 /**< Transmit error status mask */
#define XXE_STS_RX_ERROR_MASK		0xFF0 /**< Receive error status mask */


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
/** @} */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
/** @cond INTERNAL */
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

/** @endcond */

/****************************************************************************/
/**
*
* XXxvEthernet_ReadReg returns the value read from the register specified by
* <i>RegOffset</i>.
*
* @param[in]	BaseAddress is the base address of the Xxv Ethernet device.
* @param[in]	RegOffset is the offset of the register to be read.
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
* @param[in]	BaseAddress is the base address of the Xxv Ethernet device.
* @param[in]	RegOffset is the offset of the register to be written.
* @param[in]	Data is the 32-bit value to write to the register.
*
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

/** @} */
/** @cond INTERNAL */
#endif /* end of protection macro */
/** @endcond */
