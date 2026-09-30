/******************************************************************************
* Copyright (C) 2009 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XDMAPS_HW_H		/* prevent circular inclusions */
/** @brief Include guard for the DMAPS hardware header. */
#define XDMAPS_HW_H		/* by using protection macros */

/**
 * @file xdmaps_hw.h
 * @brief Defines the hardware interface of an XDmaPs device.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date     Changes
 * ----- ----  -------- ----------------------------------------------
 * 1.00a	hbm   08/18/10 First Release
 * 1.01a nm    12/20/12 Added definition XDMAPS_CHANNELS_PER_DEV which specifies
 *		       the maximum number of channels.
 *		       Replaced the usage of XPAR_XDMAPS_CHANNELS_PER_DEV
 *                      with XDMAPS_CHANNELS_PER_DEV defined in xdmaps_hw.h
 * 1.02a sg    05/16/12 Made changes for doxygen
 * 1.06a kpc   07/10/13 Added function prototype
 * </pre>
 *
 ******************************************************************************/

/**
 * @addtogroup dmaps Overview
 * @{
 */


#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xil_io.h"

/************************** Constant Definitions *****************************/

/** @name Register Map
 *
 * Register offsets for the DMAC.
 * @{
 */
#define XDMAPS_DS_OFFSET		0x000 /**< DMA Status Register. */
#define XDMAPS_DPC_OFFSET	0x004 /**< DMA Program Counter Register. */
#define XDMAPS_INTEN_OFFSET	0X020 /**< DMA Interrupt Enable Register. */
#define XDMAPS_ES_OFFSET		0x024 /**< DMA Event Status Register. */
#define XDMAPS_INTSTATUS_OFFSET	0x028 /**< DMA Interrupt Status Register. */
#define XDMAPS_INTCLR_OFFSET	0x02c /**< DMA Interrupt Clear Register. */
#define XDMAPS_FSM_OFFSET 	0x030 /**< DMA Fault Status DMA Manager Register. */
#define XDMAPS_FSC_OFFSET	0x034 /**< DMA Fault Status DMA Channel Register. */
#define XDMAPS_FTM_OFFSET	0x038 /**< DMA Fault Type DMA Manager Register. */

#define XDMAPS_FTC0_OFFSET	0x040 /**< DMA Fault Type for DMA Channel 0. */
/**
 * @brief Returns the FTC register offset for the given channel.
 *
 * @param[in] ch Channel number.
 */
#define XDmaPs_FTCn_OFFSET(ch)	(XDMAPS_FTC0_OFFSET + (ch) * 4)

#define XDMAPS_CS0_OFFSET	0x100 /**< Channel Status for DMA Channel 0. */
/**
 * @brief Returns the CS register offset for the given channel.
 *
 * @param[in] ch Channel number.
 */
#define XDmaPs_CSn_OFFSET(ch)	(XDMAPS_CS0_OFFSET + (ch) * 8)

#define XDMAPS_CPC0_OFFSET	0x104 /**< Channel Program Counter for DMA Channel 0. */
/**
 * @brief Returns the CPC register offset for the given channel.
 *
 * @param[in] ch Channel number.
 */
#define XDmaPs_CPCn_OFFSET(ch)	(XDMAPS_CPC0_OFFSET + (ch) * 8)

#define XDMAPS_SA_0_OFFSET	0x400 /**< Source Address Register for DMA Channel 0. */
/**
 * @brief Returns the SA register offset for the given channel.
 *
 * @param[in] ch Channel number.
 */
#define XDmaPs_SA_n_OFFSET(ch)	(XDMAPS_SA_0_OFFSET + (ch) * 0x20)

#define XDMAPS_DA_0_OFFSET	0x404 /**< Destination Address Register for DMA Channel 0. */
/**
 * @brief Returns the DA register offset for the given channel.
 *
 * @param[in] ch Channel number.
 */
#define XDmaPs_DA_n_OFFSET(ch)	(XDMAPS_DA_0_OFFSET + (ch) * 0x20)

#define XDMAPS_CC_0_OFFSET	0x408 /**< Channel Control Register for DMA Channel 0. */
/**
 * @brief Returns the CC register offset for the given channel.
 *
 * @param[in] ch Channel number.
 */
#define XDmaPs_CC_n_OFFSET(ch)	(XDMAPS_CC_0_OFFSET + (ch) * 0x20)

#define XDMAPS_LC0_0_OFFSET	0x40C /**< Loop Counter 0 for DMA Channel 0. */
/**
 * @brief Returns the LC0 register offset for the given channel.
 *
 * @param[in] ch Channel number.
 */
#define XDmaPs_LC0_n_OFFSET(ch)	(XDMAPS_LC0_0_OFFSET + (ch) * 0x20)
#define XDMAPS_LC1_0_OFFSET	0x410 /**< Loop Counter 1 for DMA Channel 0. */
/**
 * @brief Returns the LC1 register offset for the given channel.
 *
 * @param[in] ch Channel number.
 */
#define XDmaPs_LC1_n_OFFSET(ch)	(XDMAPS_LC1_0_OFFSET + (ch) * 0x20)

#define XDMAPS_DBGSTATUS_OFFSET	0xD00 /**< Debug Status Register. */
#define XDMAPS_DBGCMD_OFFSET	0xD04 /**< Debug Command Register. */
#define XDMAPS_DBGINST0_OFFSET	0xD08 /**< Debug Instruction 0 Register. */
#define XDMAPS_DBGINST1_OFFSET	0xD0C /**< Debug Instruction 1 Register. */

#define XDMAPS_CR0_OFFSET	0xE00 /**< Configuration Register 0. */
#define XDMAPS_CR1_OFFSET	0xE04 /**< Configuration Register 1. */
#define XDMAPS_CR2_OFFSET	0xE08 /**< Configuration Register 2. */
#define XDMAPS_CR3_OFFSET	0xE0C /**< Configuration Register 3. */
#define XDMAPS_CR4_OFFSET	0xE10 /**< Configuration Register 4. */
#define XDMAPS_CRDN_OFFSET	0xE14 /**< Configuration Register Dn. */

#define XDMAPS_PERIPH_ID_0_OFFSET	0xFE0 /**< Peripheral Identification Register 0. */
#define XDMAPS_PERIPH_ID_1_OFFSET	0xFE4 /**< Peripheral Identification Register 1. */
#define XDMAPS_PERIPH_ID_2_OFFSET	0xFE8 /**< Peripheral Identification Register 2. */
#define XDMAPS_PERIPH_ID_3_OFFSET	0xFEC /**< Peripheral Identification Register 3. */
#define XDMAPS_PCELL_ID_0_OFFSET	0xFF0 /**< PrimeCell Identification Register 0. */
#define XDMAPS_PCELL_ID_1_OFFSET	0xFF4 /**< PrimeCell Identification Register 1. */
#define XDMAPS_PCELL_ID_2_OFFSET	0xFF8 /**< PrimeCell Identification Register 2. */
#define XDMAPS_PCELL_ID_3_OFFSET	0xFFC /**< PrimeCell Identification Register 3. */

/** @} */

/** @name Useful register masks
 * @{
 */
#define XDMAPS_DS_DMA_STATUS		0x0F /**< DMA status mask. */
#define XDMAPS_DS_DMA_STATUS_STOPPED	0x00 /**< DMA stopped status value. */

#define XDMAPS_DBGSTATUS_BUSY		0x01 /**< Debug status busy mask. */

#define XDMAPS_CS_ACTIVE_MASK		0x07 /**< Channel status active mask. */

#define XDMAPS_CR1_I_CACHE_LEN_MASK	0x07 /**< Instruction cache length mask. */
/** @} */

/**
 * @brief Constructs the word for the Debug Instruction-0 Register.
 *
 * @param[in] b1 Instruction byte 1.
 * @param[in] b0 Instruction byte 0.
 * @param[in] ch Channel number.
 * @param[in] dbg_th Debug thread encoding: 0 for DMA manager thread, 1 for DMA channel.
 */
#define XDmaPs_DBGINST0(b1, b0, ch, dbg_th) \
	(((b1) << 24) | ((b0) << 16) | (((ch) & 0x7) << 8) | ((dbg_th & 0x1)))

/** @name Device constants
 * @{
 */
#define XDMAPS_CHANNELS_PER_DEV		8 /**< Maximum number of channels per device. */
/** @} */

/** @name Interrupt Registers
 *
 * Interrupt control logic uses the interrupt enable register (IER) and the
 * interrupt disable register (IDR) to set the value of the bits in the
 * interrupt mask register (IMR). The IMR determines whether to pass an
 * interrupt to the interrupt status register (ISR).
 * Writing a 1 to IER enables an interrupt, writing a 1 to IDR disables an
 * interrupt. IMR and ISR are read only, and IER and IDR are write only.
 * Reading either IER or IDR returns 0x00.
 *
 * All four registers have the same bit definitions.
 *
 * @{
 */

#define XDMAPS_INTCLR_ALL_MASK		0xFF /**< Mask to clear all interrupt bits. */

/** @} */

/**
 * @brief Reads a DMAC register.
 *
 * @param[in] BaseAddress Base address of the device.
 * @param[in] RegOffset Offset from the base address of the device.
 *
 * @return The 32-bit register value.
 */
#define XDmaPs_ReadReg(BaseAddress, RegOffset) \
	Xil_In32((BaseAddress) + (RegOffset))

/***************************************************************************/
/**
* @brief Writes a DMAC register.
*
* @param[in] BaseAddress contains the base address of the device.
* @param[in] RegOffset contains the offset from the base address of the device.
* @param[in] RegisterValue is the value to be written to the register.
*
* @note
* C-Style signature:
*    void XDmaPs_WriteReg(u32 BaseAddress, int RegOffset,
*                          u32 RegisterValue)
******************************************************************************/
#define XDmaPs_WriteReg(BaseAddress, RegOffset, RegisterValue) \
	Xil_Out32((BaseAddress) + (RegOffset), (RegisterValue))
/************************** Variable Definitions *****************************/

/************************** Function Prototypes *****************************/
/*
 * Perform reset operation to the dmaps interface
 */
void XDmaPs_ResetHw(u32 BaseAddr);
#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
