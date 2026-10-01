/******************************************************************************
* Copyright (C) 2014 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XQSPIPSU_HW_H
#define XQSPIPSU_HW_H

/*****************************************************************************/
/**
*
* @file xqspipsu_hw.h
* @brief Provides register definitions and access macros for the QSPIPSU driver.
*
* The xqspipsu_hw.h file contains low-level access functions that use the base
* address directly without an instance.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------.
* 1.0   hk  08/21/14 First release
*       hk  03/18/15 Add DMA status register masks required.
*       sk  04/24/15 Modified the code according to MISRAC-2012.
* 1.2	nsk 07/01/16 Added LQSPI supported Masks
*       rk  07/15/16 Added support for TapDelays at different frequencies.
* 1.7	tjs	03/14/18 Added support in EL1 NS mode.
* 1.9	tjs	04/17/18 Updated register addresses as per the latest revision
* 					 of versal (CR#999610)
* 1.9  aru 01/17/19 Fixed the violations for  MISRAC-2012
*                  in safety mode .Done changes such as added U suffix
* 1.11	akm 11/07/19 Removed LQSPI register access in Versal.
* 1.15	akm 12/02/21 Fix Doxygen warnings.
* 1.24  tb  09/30/26 Align Doxygen comments with AMD standards
*
* </pre>
*
******************************************************************************/

/** @addtogroup qspipsu Overview
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xil_io.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/
/**
 * @name Device Base Address
 * Defines the QSPI and QSPIPSU base addresses.
 * @{
 */
/**
 * QSPI base address.
 */
#if defined (versal)
#define XQSPIPS_BASEADDR      0XF1030000U
#else
#define XQSPIPS_BASEADDR      0XFF0F0000U
#endif

#if defined (versal)
#define XQSPIPSU_BASEADDR     0XF1030100U /**< QSPIPSU base address. */
#else
#define XQSPIPSU_BASEADDR     0xFF0F0100U /**< QSPIPSU base address. */
#endif
/** Offset of the GQSPI registers from the QSPI base. */
#define XQSPIPSU_OFFSET     0x100U
/** @} */

/**
 * @name XQSPIPS Enable Register information
 * QSPIPSU Enable Register.
 * @{
 */
/**
 * XQSPIPS_EN_REG register address.
 */
#define XQSPIPS_EN_REG    ( ( XQSPIPS_BASEADDR ) + 0X00000014U )
#define XQSPIPS_EN_SHIFT   0U /**< Enable field shift. */
#define XQSPIPS_EN_WIDTH   1U /**< Enable field width. */
#define XQSPIPS_EN_MASK    0X00000001U /**< Enable field mask. */
/** @} */

/**
 * @name XQSPIPSU configuration Register information
 * Contains the bits for configuring the GQSPI controller.
 * @{
 */
/**
 * XQSPIPSU_CFG register offset.
 */
#define XQSPIPSU_CFG_OFFSET    0X00000000U

#define XQSPIPSU_CFG_MODE_EN_SHIFT   30U /**< Mode enable field shift. */
#define XQSPIPSU_CFG_MODE_EN_WIDTH   2U /**< Mode enable field width. */
#define XQSPIPSU_CFG_MODE_EN_MASK    0XC0000000U /**< Mode enable field mask. */
/** DMA mode enable mask. */
#define XQSPIPSU_CFG_MODE_EN_DMA_MASK	0X80000000U

/** Generic FIFO start mode field shift. */
#define XQSPIPSU_CFG_GEN_FIFO_START_MODE_SHIFT   29U
/** Generic FIFO start mode field width. */
#define XQSPIPSU_CFG_GEN_FIFO_START_MODE_WIDTH   1U
/** Generic FIFO start mode field mask. */
#define XQSPIPSU_CFG_GEN_FIFO_START_MODE_MASK    0X20000000U

/** Start generic FIFO field shift. */
#define XQSPIPSU_CFG_START_GEN_FIFO_SHIFT   28U
/** Start generic FIFO field width. */
#define XQSPIPSU_CFG_START_GEN_FIFO_WIDTH   1U
/** Start generic FIFO field mask. */
#define XQSPIPSU_CFG_START_GEN_FIFO_MASK    0X10000000U

#define XQSPIPSU_CFG_ENDIAN_SHIFT   26U /**< Endianness field shift. */
#define XQSPIPSU_CFG_ENDIAN_WIDTH   1U /**< Endianness field width. */
#define XQSPIPSU_CFG_ENDIAN_MASK    0X04000000U /**< Endianness field mask. */

/** Poll timeout enable field shift. */
#define XQSPIPSU_CFG_EN_POLL_TO_SHIFT   20U
/** Poll timeout enable field width. */
#define XQSPIPSU_CFG_EN_POLL_TO_WIDTH   1U
/** Poll timeout enable field mask. */
#define XQSPIPSU_CFG_EN_POLL_TO_MASK    0X00100000U

#define XQSPIPSU_CFG_WP_HOLD_SHIFT   19U /**< Write protect hold field shift. */
#define XQSPIPSU_CFG_WP_HOLD_WIDTH   1U /**< Write protect hold field width. */
/** Write protect hold field mask. */
#define XQSPIPSU_CFG_WP_HOLD_MASK    0X00080000U

/** Baud rate divisor field shift. */
#define XQSPIPSU_CFG_BAUD_RATE_DIV_SHIFT   3U
/** Baud rate divisor field width. */
#define XQSPIPSU_CFG_BAUD_RATE_DIV_WIDTH   3U
/** Baud rate divisor field mask. */
#define XQSPIPSU_CFG_BAUD_RATE_DIV_MASK    0X00000038U

#define XQSPIPSU_CFG_CLK_PHA_SHIFT   2U /**< Clock phase field shift. */
#define XQSPIPSU_CFG_CLK_PHA_WIDTH   1U /**< Clock phase field width. */
#define XQSPIPSU_CFG_CLK_PHA_MASK    0X00000004U /**< Clock phase field mask. */

#define XQSPIPSU_CFG_CLK_POL_SHIFT   1U /**< Clock polarity field shift. */
#define XQSPIPSU_CFG_CLK_POL_WIDTH   1U /**< Clock polarity field width. */
/** Clock polarity field mask. */
#define XQSPIPSU_CFG_CLK_POL_MASK    0X00000002U
/** @} */

/**
 * @name XQSPIPSU LQSPI Register information
 * Contains the bits for configuring the LQSPI.
 * @{
 */
/**
 * XQSPIPSU_LQSPI register offset.
 */
#if !defined (versal)
#define XQSPIPSU_LQSPI_CR_OFFSET	0X000000A0U
#define XQSPIPSU_LQSPI_CR_LINEAR_MASK     0x80000000U /**< LQSPI mode enable. */
/** Both memories or one. */
#define XQSPIPSU_LQSPI_CR_TWO_MEM_MASK    0x40000000U
/** Separate memory bus. */
#define XQSPIPSU_LQSPI_CR_SEP_BUS_MASK    0x20000000U
#define XQSPIPSU_LQSPI_CR_U_PAGE_MASK     0x10000000U /**< Upper memory page. */
/** Upper memory page. */
#define XQSPIPSU_LQSPI_CR_ADDR_32BIT_MASK     0x01000000U
#define XQSPIPSU_LQSPI_CR_MODE_EN_MASK    0x02000000U /**< Enable mode bits. */
#define XQSPIPSU_LQSPI_CR_MODE_ON_MASK    0x01000000U /**< Mode on. */
/** Mode value for dual I/O or quad I/O. */
#define XQSPIPSU_LQSPI_CR_MODE_BITS_MASK  0x00FF0000U
/** Read instruction code. */
#define XQSPIPS_LQSPI_CR_INST_MASK       0x000000FFU
/** Default LQSPI CR value. */
#define XQSPIPS_LQSPI_CR_RST_STATE       0x80000003U
/** Default 4-byte LQSPI CR value. */
#define XQSPIPS_LQSPI_CR_4_BYTE_STATE       0x88000013U
/** Default LQSPI CFG value. */
#define XQSPIPS_LQSPI_CFG_RST_STATE       0x800238C1U
#endif
/** @} */

/**
 * @name XQSPIPSU Interrupt Status Register information
 * QSPIPSU Interrupt Status Register.
 * @{
 */
/**
 * XQSPIPSU_ISR register offset.
 */
#define XQSPIPSU_ISR_OFFSET	0X00000004U

#define XQSPIPSU_ISR_RXEMPTY_SHIFT   11U /**< RX FIFO empty field shift. */
#define XQSPIPSU_ISR_RXEMPTY_WIDTH   1U /**< RX FIFO empty field width. */
/** RX FIFO empty field mask. */
#define XQSPIPSU_ISR_RXEMPTY_MASK    0X00000800U

/** Generic FIFO full field shift. */
#define XQSPIPSU_ISR_GENFIFOFULL_SHIFT   10U
/** Generic FIFO full field width. */
#define XQSPIPSU_ISR_GENFIFOFULL_WIDTH   1U
/** Generic FIFO full field mask. */
#define XQSPIPSU_ISR_GENFIFOFULL_MASK    0X00000400U

/** Generic FIFO not full field shift. */
#define XQSPIPSU_ISR_GENFIFONOT_FULL_SHIFT   9U
/** Generic FIFO not full field width. */
#define XQSPIPSU_ISR_GENFIFONOT_FULL_WIDTH   1U
/** Generic FIFO not full field mask. */
#define XQSPIPSU_ISR_GENFIFONOT_FULL_MASK    0X00000200U

#define XQSPIPSU_ISR_TXEMPTY_SHIFT   8U /**< TX FIFO empty field shift. */
#define XQSPIPSU_ISR_TXEMPTY_WIDTH   1U /**< TX FIFO empty field width. */
/** TX FIFO empty field mask. */
#define XQSPIPSU_ISR_TXEMPTY_MASK    0X00000100U

/** Generic FIFO empty field shift. */
#define XQSPIPSU_ISR_GENFIFOEMPTY_SHIFT   7U
/** Generic FIFO empty field width. */
#define XQSPIPSU_ISR_GENFIFOEMPTY_WIDTH   1U
/** Generic FIFO empty field mask. */
#define XQSPIPSU_ISR_GENFIFOEMPTY_MASK    0X00000080U

#define XQSPIPSU_ISR_RXFULL_SHIFT   5U /**< RX FIFO full field shift. */
#define XQSPIPSU_ISR_RXFULL_WIDTH   1U /**< RX FIFO full field width. */
#define XQSPIPSU_ISR_RXFULL_MASK    0X00000020U /**< RX FIFO full field mask. */

#define XQSPIPSU_ISR_RXNEMPTY_SHIFT   4U /**< RX FIFO not empty field shift. */
#define XQSPIPSU_ISR_RXNEMPTY_WIDTH   1U /**< RX FIFO not empty field width. */
/** RX FIFO not empty field mask. */
#define XQSPIPSU_ISR_RXNEMPTY_MASK    0X00000010U

#define XQSPIPSU_ISR_TXFULL_SHIFT   3U /**< TX FIFO full field shift. */
#define XQSPIPSU_ISR_TXFULL_WIDTH   1U /**< TX FIFO full field width. */
#define XQSPIPSU_ISR_TXFULL_MASK    0X00000008U /**< TX FIFO full field mask. */

#define XQSPIPSU_ISR_TXNOT_FULL_SHIFT   2U /**< TX FIFO not full field shift. */
#define XQSPIPSU_ISR_TXNOT_FULL_WIDTH   1U /**< TX FIFO not full field width. */
/** TX FIFO not full field mask. */
#define XQSPIPSU_ISR_TXNOT_FULL_MASK    0X00000004U

/** Poll time expire field shift. */
#define XQSPIPSU_ISR_POLL_TIME_EXPIRE_SHIFT   1U
/** Poll time expire field width. */
#define XQSPIPSU_ISR_POLL_TIME_EXPIRE_WIDTH   1U
/** Poll time expire field mask. */
#define XQSPIPSU_ISR_POLL_TIME_EXPIRE_MASK    0X00000002U

/** Write-to-clear bits mask. */
#define XQSPIPSU_ISR_WR_TO_CLR_MASK 0X00000002U
/** @} */

/**
 * @name XQSPIPSU Interrupt Enable Register information
 * Contains the bits for enabling interrupts.
 * @{
 */
/**
 * XQSPIPSU_IER register offset.
 */
#define XQSPIPSU_IER_OFFSET    0X00000008U

#define XQSPIPSU_IER_RXEMPTY_SHIFT   11U /**< RX FIFO empty field shift. */
#define XQSPIPSU_IER_RXEMPTY_WIDTH   1U /**< RX FIFO empty field width. */
/** RX FIFO empty field mask. */
#define XQSPIPSU_IER_RXEMPTY_MASK    0X00000800U

/** Generic FIFO full field shift. */
#define XQSPIPSU_IER_GENFIFOFULL_SHIFT   10U
/** Generic FIFO full field width. */
#define XQSPIPSU_IER_GENFIFOFULL_WIDTH   1U
/** Generic FIFO full field mask. */
#define XQSPIPSU_IER_GENFIFOFULL_MASK    0X00000400U

/** Generic FIFO not full field shift. */
#define XQSPIPSU_IER_GENFIFONOT_FULL_SHIFT   9U
/** Generic FIFO not full field width. */
#define XQSPIPSU_IER_GENFIFONOT_FULL_WIDTH   1U
/** Generic FIFO not full field mask. */
#define XQSPIPSU_IER_GENFIFONOT_FULL_MASK    0X00000200U

#define XQSPIPSU_IER_TXEMPTY_SHIFT   8U /**< TX FIFO empty field shift. */
#define XQSPIPSU_IER_TXEMPTY_WIDTH   1U /**< TX FIFO empty field width. */
/** TX FIFO empty field mask. */
#define XQSPIPSU_IER_TXEMPTY_MASK    0X00000100U

/** Generic FIFO empty field shift. */
#define XQSPIPSU_IER_GENFIFOEMPTY_SHIFT   7U
/** Generic FIFO empty field width. */
#define XQSPIPSU_IER_GENFIFOEMPTY_WIDTH   1U
/** Generic FIFO empty field mask. */
#define XQSPIPSU_IER_GENFIFOEMPTY_MASK    0X00000080U

#define XQSPIPSU_IER_RXFULL_SHIFT   5U /**< RX FIFO full field shift. */
#define XQSPIPSU_IER_RXFULL_WIDTH   1U /**< RX FIFO full field width. */
#define XQSPIPSU_IER_RXFULL_MASK    0X00000020U /**< RX FIFO full field mask. */

#define XQSPIPSU_IER_RXNEMPTY_SHIFT   4U /**< RX FIFO not empty field shift. */
#define XQSPIPSU_IER_RXNEMPTY_WIDTH   1U /**< RX FIFO not empty field width. */
/** RX FIFO not empty field mask. */
#define XQSPIPSU_IER_RXNEMPTY_MASK    0X00000010U

#define XQSPIPSU_IER_TXFULL_SHIFT   3U /**< TX FIFO full field shift. */
#define XQSPIPSU_IER_TXFULL_WIDTH   1U /**< TX FIFO full field width. */
#define XQSPIPSU_IER_TXFULL_MASK    0X00000008U /**< TX FIFO full field mask. */

#define XQSPIPSU_IER_TXNOT_FULL_SHIFT   2U /**< TX FIFO not full field shift. */
#define XQSPIPSU_IER_TXNOT_FULL_WIDTH   1U /**< TX FIFO not full field width. */
/** TX FIFO not full field mask. */
#define XQSPIPSU_IER_TXNOT_FULL_MASK    0X00000004U

/** Poll time expire field shift. */
#define XQSPIPSU_IER_POLL_TIME_EXPIRE_SHIFT   1U
/** Poll time expire field width. */
#define XQSPIPSU_IER_POLL_TIME_EXPIRE_WIDTH   1U
/** Poll time expire field mask. */
#define XQSPIPSU_IER_POLL_TIME_EXPIRE_MASK    0X00000002U
/** @} */

/**
 * @name XQSPIPSU Interrupt Disable Register information
 * Contains the bits for disabling interrupts.
 * @{
 */
/**
 * XQSPIPSU_IDR register offset.
 */
#define XQSPIPSU_IDR_OFFSET    0X0000000CU

#define XQSPIPSU_IDR_RXEMPTY_SHIFT   11U /**< RX FIFO empty field shift. */
#define XQSPIPSU_IDR_RXEMPTY_WIDTH   1U /**< RX FIFO empty field width. */
/** RX FIFO empty field mask. */
#define XQSPIPSU_IDR_RXEMPTY_MASK    0X00000800U

/** Generic FIFO full field shift. */
#define XQSPIPSU_IDR_GENFIFOFULL_SHIFT   10U
/** Generic FIFO full field width. */
#define XQSPIPSU_IDR_GENFIFOFULL_WIDTH   1U
/** Generic FIFO full field mask. */
#define XQSPIPSU_IDR_GENFIFOFULL_MASK    0X00000400U

/** Generic FIFO not full field shift. */
#define XQSPIPSU_IDR_GENFIFONOT_FULL_SHIFT   9U
/** Generic FIFO not full field width. */
#define XQSPIPSU_IDR_GENFIFONOT_FULL_WIDTH   1U
/** Generic FIFO not full field mask. */
#define XQSPIPSU_IDR_GENFIFONOT_FULL_MASK    0X00000200U

#define XQSPIPSU_IDR_TXEMPTY_SHIFT   8U /**< TX FIFO empty field shift. */
#define XQSPIPSU_IDR_TXEMPTY_WIDTH   1U /**< TX FIFO empty field width. */
/** TX FIFO empty field mask. */
#define XQSPIPSU_IDR_TXEMPTY_MASK    0X00000100U

/** Generic FIFO empty field shift. */
#define XQSPIPSU_IDR_GENFIFOEMPTY_SHIFT   7U
/** Generic FIFO empty field width. */
#define XQSPIPSU_IDR_GENFIFOEMPTY_WIDTH   1U
/** Generic FIFO empty field mask. */
#define XQSPIPSU_IDR_GENFIFOEMPTY_MASK    0X00000080U

#define XQSPIPSU_IDR_RXFULL_SHIFT   5U /**< RX FIFO full field shift. */
#define XQSPIPSU_IDR_RXFULL_WIDTH   1U /**< RX FIFO full field width. */
#define XQSPIPSU_IDR_RXFULL_MASK    0X00000020U /**< RX FIFO full field mask. */

#define XQSPIPSU_IDR_RXNEMPTY_SHIFT   4U /**< RX FIFO not empty field shift. */
#define XQSPIPSU_IDR_RXNEMPTY_WIDTH   1U /**< RX FIFO not empty field width. */
/** RX FIFO not empty field mask. */
#define XQSPIPSU_IDR_RXNEMPTY_MASK    0X00000010U

#define XQSPIPSU_IDR_TXFULL_SHIFT   3U /**< TX FIFO full field shift. */
#define XQSPIPSU_IDR_TXFULL_WIDTH   1U /**< TX FIFO full field width. */
#define XQSPIPSU_IDR_TXFULL_MASK    0X00000008U /**< TX FIFO full field mask. */

#define XQSPIPSU_IDR_TXNOT_FULL_SHIFT   2U /**< TX FIFO not full field shift. */
#define XQSPIPSU_IDR_TXNOT_FULL_WIDTH   1U /**< TX FIFO not full field width. */
/** TX FIFO not full field mask. */
#define XQSPIPSU_IDR_TXNOT_FULL_MASK    0X00000004U

/** Poll time expire field shift. */
#define XQSPIPSU_IDR_POLL_TIME_EXPIRE_SHIFT   1U
/** Poll time expire field width. */
#define XQSPIPSU_IDR_POLL_TIME_EXPIRE_WIDTH   1U
/** Poll time expire field mask. */
#define XQSPIPSU_IDR_POLL_TIME_EXPIRE_MASK    0X00000002U

#define XQSPIPSU_IDR_ALL_MASK    0X0FBEU /**< All interrupts disable mask. */
/** @} */

/**
 * @name XQSPIPSU Interrupt Mask Register information
 * Contains the bits for masking interrupts.
 * @{
 */
/**
 * XQSPIPSU_IMR register offset.
 */
#define XQSPIPSU_IMR_OFFSET    0X00000010U

#define XQSPIPSU_IMR_RXEMPTY_SHIFT   11U /**< RX FIFO empty field shift. */
#define XQSPIPSU_IMR_RXEMPTY_WIDTH   1U /**< RX FIFO empty field width. */
/** RX FIFO empty field mask. */
#define XQSPIPSU_IMR_RXEMPTY_MASK    0X00000800U

/** Generic FIFO full field shift. */
#define XQSPIPSU_IMR_GENFIFOFULL_SHIFT   10U
/** Generic FIFO full field width. */
#define XQSPIPSU_IMR_GENFIFOFULL_WIDTH   1U
/** Generic FIFO full field mask. */
#define XQSPIPSU_IMR_GENFIFOFULL_MASK    0X00000400U

/** Generic FIFO not full field shift. */
#define XQSPIPSU_IMR_GENFIFONOT_FULL_SHIFT   9U
/** Generic FIFO not full field width. */
#define XQSPIPSU_IMR_GENFIFONOT_FULL_WIDTH   1U
/** Generic FIFO not full field mask. */
#define XQSPIPSU_IMR_GENFIFONOT_FULL_MASK    0X00000200U

#define XQSPIPSU_IMR_TXEMPTY_SHIFT   8U /**< TX FIFO empty field shift. */
#define XQSPIPSU_IMR_TXEMPTY_WIDTH   1U /**< TX FIFO empty field width. */
/** TX FIFO empty field mask. */
#define XQSPIPSU_IMR_TXEMPTY_MASK    0X00000100U

/** Generic FIFO empty field shift. */
#define XQSPIPSU_IMR_GENFIFOEMPTY_SHIFT   7U
/** Generic FIFO empty field width. */
#define XQSPIPSU_IMR_GENFIFOEMPTY_WIDTH   1U
/** Generic FIFO empty field mask. */
#define XQSPIPSU_IMR_GENFIFOEMPTY_MASK    0X00000080U

#define XQSPIPSU_IMR_RXFULL_SHIFT   5U /**< RX FIFO full field shift. */
#define XQSPIPSU_IMR_RXFULL_WIDTH   1U /**< RX FIFO full field width. */
#define XQSPIPSU_IMR_RXFULL_MASK    0X00000020U /**< RX FIFO full field mask. */

#define XQSPIPSU_IMR_RXNEMPTY_SHIFT   4U /**< RX FIFO not empty field shift. */
#define XQSPIPSU_IMR_RXNEMPTY_WIDTH   1U /**< RX FIFO not empty field width. */
/** RX FIFO not empty field mask. */
#define XQSPIPSU_IMR_RXNEMPTY_MASK    0X00000010U

#define XQSPIPSU_IMR_TXFULL_SHIFT   3U /**< TX FIFO full field shift. */
#define XQSPIPSU_IMR_TXFULL_WIDTH   1U /**< TX FIFO full field width. */
#define XQSPIPSU_IMR_TXFULL_MASK    0X00000008U /**< TX FIFO full field mask. */

#define XQSPIPSU_IMR_TXNOT_FULL_SHIFT   2U /**< TX FIFO not full field shift. */
#define XQSPIPSU_IMR_TXNOT_FULL_WIDTH   1U /**< TX FIFO not full field width. */
/** TX FIFO not full field mask. */
#define XQSPIPSU_IMR_TXNOT_FULL_MASK    0X00000004U

/** Poll time expire field shift. */
#define XQSPIPSU_IMR_POLL_TIME_EXPIRE_SHIFT   1U
/** Poll time expire field width. */
#define XQSPIPSU_IMR_POLL_TIME_EXPIRE_WIDTH   1U
/** Poll time expire field mask. */
#define XQSPIPSU_IMR_POLL_TIME_EXPIRE_MASK    0X00000002U
/** @} */

/**
 * @name XQSPIPSU Enable Register information
 * Contains the bits for enabling the QSPI controller.
 * @{
 */
/**
 * XQSPIPSU_EN_REG register offset.
 */
#define XQSPIPSU_EN_OFFSET    0X00000014U

#define XQSPIPSU_EN_SHIFT   0U /**< Enable field shift. */
#define XQSPIPSU_EN_WIDTH   1U /**< Enable field width. */
#define XQSPIPSU_EN_MASK    0X00000001U /**< Enable field mask. */
/** @} */

/**
 * @name XQSPIPSU TX Data Register information
 * Contains the bits for configuring TXFIFO.
 * @{
 */
/**
 * XQSPIPSU_TXD register offset.
 */
#define XQSPIPSU_TXD_OFFSET    0X0000001CU

#define XQSPIPSU_TXD_SHIFT   0U /**< TX data field shift. */
#define XQSPIPSU_TXD_WIDTH   32U /**< TX data field width. */
#define XQSPIPSU_TXD_MASK    0XFFFFFFFFU /**< TX data field mask. */

#define XQSPIPSU_TXD_DEPTH    64 /**< TX FIFO depth. */
/** @} */

/**
 * @name XQSPIPSU RX Data Register information
 * Contains the bits for configuring RXFIFO.
 * @{
 */
/**
 * XQSPIPSU_RXD register offset.
 */
#define XQSPIPSU_RXD_OFFSET    0X00000020U

#define XQSPIPSU_RXD_SHIFT   0U /**< RX data field shift. */
#define XQSPIPSU_RXD_WIDTH   32U /**< RX data field width. */
#define XQSPIPSU_RXD_MASK    0XFFFFFFFFU /**< RX data field mask. */
/** @} */

/**
 * @name XQSPIPSU TX/RX Threshold Register information
 * Contains the bits for configuring TX/RX Threshold.
 * @{
 */
/**
 * XQSPIPSU_TX_THRESHOLD register offset.
 */
#define XQSPIPSU_TX_THRESHOLD_OFFSET    0X00000028U

/** TX FIFO threshold field shift. */
#define XQSPIPSU_TX_FIFO_THRESHOLD_SHIFT   0U
/** TX FIFO threshold field width. */
#define XQSPIPSU_TX_FIFO_THRESHOLD_WIDTH   6U
/** TX FIFO threshold field mask. */
#define XQSPIPSU_TX_FIFO_THRESHOLD_MASK    0X0000003FU
/** TX FIFO threshold reset value. */
#define XQSPIPSU_TX_FIFO_THRESHOLD_RESET_VAL    0X01U

/** XQSPIPSU_RX_THRESHOLD register offset. */
#define XQSPIPSU_RX_THRESHOLD_OFFSET    0X0000002CU

/** RX FIFO threshold field shift. */
#define XQSPIPSU_RX_FIFO_THRESHOLD_SHIFT   0U
/** RX FIFO threshold field width. */
#define XQSPIPSU_RX_FIFO_THRESHOLD_WIDTH   6U
/** RX FIFO threshold field mask. */
#define XQSPIPSU_RX_FIFO_THRESHOLD_MASK    0X0000003FU
/** RX FIFO threshold reset value. */
#define XQSPIPSU_RX_FIFO_THRESHOLD_RESET_VAL    0X01U

#define XQSPIPSU_RXFIFO_THRESHOLD_OPT 32U /**< Optimal RX FIFO threshold. */
/** @} */

/**
 * @name XQSPIPSU GPIO Register information
 * @{
 */
/**
 * XQSPIPSU_GPIO register offset.
 */
#define XQSPIPSU_GPIO_OFFSET    0X00000030U

#define XQSPIPSU_GPIO_WP_N_SHIFT   0U /**< Write protect field shift. */
#define XQSPIPSU_GPIO_WP_N_WIDTH   1U /**< Write protect field width. */
#define XQSPIPSU_GPIO_WP_N_MASK    0X00000001U /**< Write protect field mask. */
/** @} */

/**
 * @name XQSPIPSU Loopback Master Clock Delay Adjustment Register information
 * Contains the bits for configuring loopback.
 * @{
 */
/**
 * XQSPIPSU_LPBK_DLY_ADJ register offset.
 */
#define XQSPIPSU_LPBK_DLY_ADJ_OFFSET    0X00000038U

/** Use loopback field shift. */
#define XQSPIPSU_LPBK_DLY_ADJ_USE_LPBK_SHIFT   5U
/** Use loopback field width. */
#define XQSPIPSU_LPBK_DLY_ADJ_USE_LPBK_WIDTH   1U
/** Use loopback field mask. */
#define XQSPIPSU_LPBK_DLY_ADJ_USE_LPBK_MASK    0X00000020U

#define XQSPIPSU_LPBK_DLY_ADJ_DLY1_SHIFT   3U /**< Delay 1 field shift. */
#define XQSPIPSU_LPBK_DLY_ADJ_DLY1_WIDTH   2U /**< Delay 1 field width. */
/** Delay 1 field mask. */
#define XQSPIPSU_LPBK_DLY_ADJ_DLY1_MASK    0X00000018U

#define XQSPIPSU_LPBK_DLY_ADJ_DLY0_SHIFT   0U /**< Delay 0 field shift. */
#define XQSPIPSU_LPBK_DLY_ADJ_DLY0_WIDTH   3U /**< Delay 0 field width. */
/** Delay 0 field mask. */
#define XQSPIPSU_LPBK_DLY_ADJ_DLY0_MASK    0X00000007U
/** @} */

/**
 * @name XQSPIPSU GEN_FIFO Register information
 * Contains the bits for configuring GENFIFO.
 * @{
 */
/**
 * XQSPIPSU_GEN_FIFO register offset.
 */
#define XQSPIPSU_GEN_FIFO_OFFSET    0X00000040U

#define XQSPIPSU_GEN_FIFO_DATA_SHIFT   0U /**< Generic FIFO data field shift. */
/** Generic FIFO data field width. */
#define XQSPIPSU_GEN_FIFO_DATA_WIDTH   20U
/** Generic FIFO data field mask. */
#define XQSPIPSU_GEN_FIFO_DATA_MASK    0X000FFFFFU
/** @} */

/**
 * @name XQSPIPSU Select Register information
 * Contains the bits for selecting the GQSPI/LQSPI controller.
 * @{
 */
/**
 * XQSPIPSU_SEL register offset.
 */
#define XQSPIPSU_SEL_OFFSET    0X00000044U

#define XQSPIPSU_SEL_SHIFT   0U /**< Select field shift. */
#define XQSPIPSU_SEL_WIDTH   1U /**< Select field width. */
#if !defined (versal)
#define XQSPIPSU_SEL_LQSPI_MASK    0X0U /**< LQSPI select mask. */
#endif
#define XQSPIPSU_SEL_GQSPI_MASK    0X00000001U /**< GQSPI select mask. */
/** @} */

/**
 * @name XQSPIPSU FIFO Control Register information
 * Contains the bits for controlling TXFIFO and RXFIFO.
 * @{
 */
/**
 * XQSPIPSU_FIFO_CTRL register offset.
 */
#define XQSPIPSU_FIFO_CTRL_OFFSET    0X0000004CU

/** RX FIFO reset field shift. */
#define XQSPIPSU_FIFO_CTRL_RST_RX_FIFO_SHIFT   2U
/** RX FIFO reset field width. */
#define XQSPIPSU_FIFO_CTRL_RST_RX_FIFO_WIDTH   1U
/** RX FIFO reset field mask. */
#define XQSPIPSU_FIFO_CTRL_RST_RX_FIFO_MASK    0X00000004U

/** TX FIFO reset field shift. */
#define XQSPIPSU_FIFO_CTRL_RST_TX_FIFO_SHIFT   1U
/** TX FIFO reset field width. */
#define XQSPIPSU_FIFO_CTRL_RST_TX_FIFO_WIDTH   1U
/** TX FIFO reset field mask. */
#define XQSPIPSU_FIFO_CTRL_RST_TX_FIFO_MASK    0X00000002U

/** Generic FIFO reset field shift. */
#define XQSPIPSU_FIFO_CTRL_RST_GEN_FIFO_SHIFT   0U
/** Generic FIFO reset field width. */
#define XQSPIPSU_FIFO_CTRL_RST_GEN_FIFO_WIDTH   1U
/** Generic FIFO reset field mask. */
#define XQSPIPSU_FIFO_CTRL_RST_GEN_FIFO_MASK    0X00000001U
/** @} */

/**
 * @name XQSPIPSU GENFIFO Threshold Register information
 * Contains the bits for configuring GENFIFO threshold.
 * @{
 */
/**
 * XQSPIPSU_GF_THRESHOLD register offset.
 */
#define XQSPIPSU_GF_THRESHOLD_OFFSET    0X00000050U

/** Generic FIFO threshold field shift. */
#define XQSPIPSU_GEN_FIFO_THRESHOLD_SHIFT   0U
/** Generic FIFO threshold field width. */
#define XQSPIPSU_GEN_FIFO_THRESHOLD_WIDTH   5U
/** Generic FIFO threshold field mask. */
#define XQSPIPSU_GEN_FIFO_THRESHOLD_MASK    0X0000001FU
/** Generic FIFO threshold reset value. */
#define XQSPIPSU_GEN_FIFO_THRESHOLD_RESET_VAL    0X10U
/** @} */

/**
 * @name XQSPIPSU Poll configuration Register information
 * Contains the bits for configuring Poll feature.
 * @{
 */
/**
 * XQSPIPSU_POLL_CFG register offset.
 */
#define XQSPIPSU_POLL_CFG_OFFSET    0X00000054U

/** Upper bus mask enable field shift. */
#define XQSPIPSU_POLL_CFG_EN_MASK_UPPER_SHIFT   31U
/** Upper bus mask enable field width. */
#define XQSPIPSU_POLL_CFG_EN_MASK_UPPER_WIDTH   1U
/** Upper bus mask enable field mask. */
#define XQSPIPSU_POLL_CFG_EN_MASK_UPPER_MASK    0X80000000U

/** Lower bus mask enable field shift. */
#define XQSPIPSU_POLL_CFG_EN_MASK_LOWER_SHIFT   30U
/** Lower bus mask enable field width. */
#define XQSPIPSU_POLL_CFG_EN_MASK_LOWER_WIDTH   1U
/** Lower bus mask enable field mask. */
#define XQSPIPSU_POLL_CFG_EN_MASK_LOWER_MASK    0X40000000U

/** Poll mask enable field shift. */
#define XQSPIPSU_POLL_CFG_MASK_EN_SHIFT   8U
/** Poll mask enable field width. */
#define XQSPIPSU_POLL_CFG_MASK_EN_WIDTH   8U
/** Poll mask enable field mask. */
#define XQSPIPSU_POLL_CFG_MASK_EN_MASK    0X0000FF00U

/** Poll data value field shift. */
#define XQSPIPSU_POLL_CFG_DATA_VALUE_SHIFT   0U
/** Poll data value field width. */
#define XQSPIPSU_POLL_CFG_DATA_VALUE_WIDTH   8U
/** Poll data value field mask. */
#define XQSPIPSU_POLL_CFG_DATA_VALUE_MASK    0X000000FFU

/** XQSPIPSU_P_TO register offset. */
#define XQSPIPSU_P_TO_OFFSET    0X00000058U

#define XQSPIPSU_P_TO_VALUE_SHIFT   0U /**< Poll timeout value field shift. */
#define XQSPIPSU_P_TO_VALUE_WIDTH   32U /**< Poll timeout value field width. */
/** Poll timeout value field mask. */
#define XQSPIPSU_P_TO_VALUE_MASK    0XFFFFFFFFU
/** @} */

/**
 * @name XQSPIPSU Transfer Status Register information
 * Contains the bits for transfer status.
 * @{
 */
/**
 * XQSPIPSU_XFER_STS register offset.
 */
#define XQSPIPSU_XFER_STS_OFFSET	0X0000005CU

/** Pending bytes field shift. */
#define XQSPIPSU_XFER_STS_PEND_BYTES_SHIFT   0U
/** Pending bytes field width. */
#define XQSPIPSU_XFER_STS_PEND_BYTES_WIDTH   32U
/** Pending bytes field mask. */
#define XQSPIPSU_XFER_STS_PEND_BYTES_MASK    0XFFFFFFFFU
/** @} */

/**
 * @name XQSPIPSU GEN_FIFO Snapshot Register information
 * Contains the bits for configuring GENFIFO.
 * @{
 */
/**
 * XQSPIPSU_GF_SNAPSHOT register offset.
 */
#define XQSPIPSU_GF_SNAPSHOT_OFFSET    0X00000060U

/** Generic FIFO snapshot field shift. */
#define XQSPIPSU_GF_SNAPSHOT_SHIFT   0U
/** Generic FIFO snapshot field width. */
#define XQSPIPSU_GF_SNAPSHOT_WIDTH   20U
/** Generic FIFO snapshot field mask. */
#define XQSPIPSU_GF_SNAPSHOT_MASK    0X000FFFFFU
/** @} */

/**
 * @name XQSPIPSU Receive Data Copy Register information
 * @{
 */
/**
 * XQSPIPSU_RX_COPY register offset.
 */
#define XQSPIPSU_RX_COPY_OFFSET    0X00000064U

#define XQSPIPSU_RX_COPY_UPPER_SHIFT   8U /**< Upper RX copy field shift. */
#define XQSPIPSU_RX_COPY_UPPER_WIDTH   8U /**< Upper RX copy field width. */
/** Upper RX copy field mask. */
#define XQSPIPSU_RX_COPY_UPPER_MASK    0X0000FF00U

#define XQSPIPSU_RX_COPY_LOWER_SHIFT   0U /**< Lower RX copy field shift. */
#define XQSPIPSU_RX_COPY_LOWER_WIDTH   8U /**< Lower RX copy field width. */
/** Lower RX copy field mask. */
#define XQSPIPSU_RX_COPY_LOWER_MASK    0X000000FFU
/** @} */

/**
 * @name XQSPIPSU Module Identification Register information
 * @{
 */
/**
 * XQSPIPSU_MOD_ID register offset.
 */
#define XQSPIPSU_MOD_ID_OFFSET    0X000000FCU

#define XQSPIPSU_MOD_ID_SHIFT   0U /**< Module ID field shift. */
#define XQSPIPSU_MOD_ID_WIDTH   32U /**< Module ID field width. */
#define XQSPIPSU_MOD_ID_MASK    0XFFFFFFFFU /**< Module ID field mask. */
/** @} */

/**
 * @name XQSPIPSU DMA Transfer Register information
 * Contains the bits for configuring DMA.
 * @{
 */
/**
 * XQSPIPSU_QSPIDMA_DST_ADDR register offset.
 */
#define XQSPIPSU_QSPIDMA_DST_ADDR_OFFSET    0X00000700U

/** Destination address field shift. */
#define XQSPIPSU_QSPIDMA_DST_ADDR_SHIFT   2U
/** Destination address field width. */
#define XQSPIPSU_QSPIDMA_DST_ADDR_WIDTH   30U
/** Destination address field mask. */
#define XQSPIPSU_QSPIDMA_DST_ADDR_MASK    0XFFFFFFFCU

/** XQSPIPSU_QSPIDMA_DST_SIZE register offset. */
#define XQSPIPSU_QSPIDMA_DST_SIZE_OFFSET    0X00000704U

/** Destination size field shift. */
#define XQSPIPSU_QSPIDMA_DST_SIZE_SHIFT   2U
/** Destination size field width. */
#define XQSPIPSU_QSPIDMA_DST_SIZE_WIDTH   27U
/** Destination size field mask. */
#define XQSPIPSU_QSPIDMA_DST_SIZE_MASK    0X1FFFFFFCU

/** XQSPIPSU_QSPIDMA_DST_STS register offset. */
#define XQSPIPSU_QSPIDMA_DST_STS_OFFSET    0X00000708U

/** Done count field shift. */
#define XQSPIPSU_QSPIDMA_DST_STS_DONE_CNT_SHIFT   13U
/** Done count field width. */
#define XQSPIPSU_QSPIDMA_DST_STS_DONE_CNT_WIDTH   3U
/** Done count field mask. */
#define XQSPIPSU_QSPIDMA_DST_STS_DONE_CNT_MASK    0X0000E000U

/** Destination FIFO level field shift. */
#define XQSPIPSU_QSPIDMA_DST_STS_DST_FIFO_LEVEL_SHIFT   5U
/** Destination FIFO level field width. */
#define XQSPIPSU_QSPIDMA_DST_STS_DST_FIFO_LEVEL_WIDTH   8U
/** Destination FIFO level field mask. */
#define XQSPIPSU_QSPIDMA_DST_STS_DST_FIFO_LEVEL_MASK    0X00001FE0U

/** Outstanding writes field shift. */
#define XQSPIPSU_QSPIDMA_DST_STS_WR_OUTSTANDING_SHIFT   1U
/** Outstanding writes field width. */
#define XQSPIPSU_QSPIDMA_DST_STS_WR_OUTSTANDING_WIDTH   4U
/** Outstanding writes field mask. */
#define XQSPIPSU_QSPIDMA_DST_STS_WR_OUTSTANDING_MASK    0X0000001EU

#define XQSPIPSU_QSPIDMA_DST_STS_BUSY_SHIFT   0U /**< Busy field shift. */
#define XQSPIPSU_QSPIDMA_DST_STS_BUSY_WIDTH   1U /**< Busy field width. */
/** Busy field mask. */
#define XQSPIPSU_QSPIDMA_DST_STS_BUSY_MASK    0X00000001U

/** Write-to-clear bits mask. */
#define XQSPIPSU_QSPIDMA_DST_STS_WTC	0xE000U

/** XQSPIPSU_QSPIDMA_DST_CTRL register offset. */
#define XQSPIPSU_QSPIDMA_DST_CTRL_OFFSET    0X0000070CU

/** FIFO level hit threshold field shift. */
#define XQSPIPSU_QSPIDMA_DST_CTRL_FIFO_LVL_HIT_THRESHOLD_SHIFT   25U
/** FIFO level hit threshold field width. */
#define XQSPIPSU_QSPIDMA_DST_CTRL_FIFO_LVL_HIT_THRESHOLD_WIDTH   7U
/** FIFO level hit threshold field mask. */
#define XQSPIPSU_QSPIDMA_DST_CTRL_FIFO_LVL_HIT_THRESHOLD_MASK    0XFE000000U

/** APB error response field shift. */
#define XQSPIPSU_QSPIDMA_DST_CTRL_APB_ERR_RESP_SHIFT   24U
/** APB error response field width. */
#define XQSPIPSU_QSPIDMA_DST_CTRL_APB_ERR_RESP_WIDTH   1U
/** APB error response field mask. */
#define XQSPIPSU_QSPIDMA_DST_CTRL_APB_ERR_RESP_MASK    0X01000000U

/** Endianness field shift. */
#define XQSPIPSU_QSPIDMA_DST_CTRL_ENDIAN_SHIFT   23U
/** Endianness field width. */
#define XQSPIPSU_QSPIDMA_DST_CTRL_ENDIAN_WIDTH   1U
/** Endianness field mask. */
#define XQSPIPSU_QSPIDMA_DST_CTRL_ENDIAN_MASK    0X00800000U

/** AXI burst type field shift. */
#define XQSPIPSU_QSPIDMA_DST_CTRL_AXI_BRST_TYPE_SHIFT   22U
/** AXI burst type field width. */
#define XQSPIPSU_QSPIDMA_DST_CTRL_AXI_BRST_TYPE_WIDTH   1U
/** AXI burst type field mask. */
#define XQSPIPSU_QSPIDMA_DST_CTRL_AXI_BRST_TYPE_MASK    0X00400000U

/** Timeout value field shift. */
#define XQSPIPSU_QSPIDMA_DST_CTRL_TO_VAL_SHIFT   10U
/** Timeout value field width. */
#define XQSPIPSU_QSPIDMA_DST_CTRL_TO_VAL_WIDTH   12U
/** Timeout value field mask. */
#define XQSPIPSU_QSPIDMA_DST_CTRL_TO_VAL_MASK    0X003FFC00U

/** FIFO threshold field shift. */
#define XQSPIPSU_QSPIDMA_DST_CTRL_FIFO_THRESHOLD_SHIFT   2U
/** FIFO threshold field width. */
#define XQSPIPSU_QSPIDMA_DST_CTRL_FIFO_THRESHOLD_WIDTH   8U
/** FIFO threshold field mask. */
#define XQSPIPSU_QSPIDMA_DST_CTRL_FIFO_THRESHOLD_MASK    0X000003FCU

/** Pause stream field shift. */
#define XQSPIPSU_QSPIDMA_DST_CTRL_PAUSE_STRM_SHIFT   1U
/** Pause stream field width. */
#define XQSPIPSU_QSPIDMA_DST_CTRL_PAUSE_STRM_WIDTH   1U
/** Pause stream field mask. */
#define XQSPIPSU_QSPIDMA_DST_CTRL_PAUSE_STRM_MASK    0X00000002U

/** Pause memory field shift. */
#define XQSPIPSU_QSPIDMA_DST_CTRL_PAUSE_MEM_SHIFT   0U
/** Pause memory field width. */
#define XQSPIPSU_QSPIDMA_DST_CTRL_PAUSE_MEM_WIDTH   1U
/** Pause memory field mask. */
#define XQSPIPSU_QSPIDMA_DST_CTRL_PAUSE_MEM_MASK    0X00000001U

/** DMA destination control reset value. */
#define XQSPIPSU_QSPIDMA_DST_CTRL_RESET_VAL	0x403FFA00U

/** XQSPIPSU_QSPIDMA_DST_I_STS register offset. */
#define XQSPIPSU_QSPIDMA_DST_I_STS_OFFSET    0X00000714U

/** FIFO overflow field shift. */
#define XQSPIPSU_QSPIDMA_DST_I_STS_FIFO_OF_SHIFT   7U
/** FIFO overflow field width. */
#define XQSPIPSU_QSPIDMA_DST_I_STS_FIFO_OF_WIDTH   1U
/** FIFO overflow field mask. */
#define XQSPIPSU_QSPIDMA_DST_I_STS_FIFO_OF_MASK    0X00000080U

/** Invalid APB access field shift. */
#define XQSPIPSU_QSPIDMA_DST_I_STS_INVALID_APB_SHIFT   6U
/** Invalid APB access field width. */
#define XQSPIPSU_QSPIDMA_DST_I_STS_INVALID_APB_WIDTH   1U
/** Invalid APB access field mask. */
#define XQSPIPSU_QSPIDMA_DST_I_STS_INVALID_APB_MASK    0X00000040U

/** Threshold hit field shift. */
#define XQSPIPSU_QSPIDMA_DST_I_STS_THRESHOLD_HIT_SHIFT   5U
/** Threshold hit field width. */
#define XQSPIPSU_QSPIDMA_DST_I_STS_THRESHOLD_HIT_WIDTH   1U
/** Threshold hit field mask. */
#define XQSPIPSU_QSPIDMA_DST_I_STS_THRESHOLD_HIT_MASK    0X00000020U

/** Memory timeout field shift. */
#define XQSPIPSU_QSPIDMA_DST_I_STS_TO_MEM_SHIFT   4U
/** Memory timeout field width. */
#define XQSPIPSU_QSPIDMA_DST_I_STS_TO_MEM_WIDTH   1U
/** Memory timeout field mask. */
#define XQSPIPSU_QSPIDMA_DST_I_STS_TO_MEM_MASK    0X00000010U

/** Stream timeout field shift. */
#define XQSPIPSU_QSPIDMA_DST_I_STS_TO_STRM_SHIFT   3U
/** Stream timeout field width. */
#define XQSPIPSU_QSPIDMA_DST_I_STS_TO_STRM_WIDTH   1U
/** Stream timeout field mask. */
#define XQSPIPSU_QSPIDMA_DST_I_STS_TO_STRM_MASK    0X00000008U

/** AXI BRESP error field shift. */
#define XQSPIPSU_QSPIDMA_DST_I_STS_AXI_BRESP_ERR_SHIFT   2U
/** AXI BRESP error field width. */
#define XQSPIPSU_QSPIDMA_DST_I_STS_AXI_BRESP_ERR_WIDTH   1U
/** AXI BRESP error field mask. */
#define XQSPIPSU_QSPIDMA_DST_I_STS_AXI_BRESP_ERR_MASK    0X00000004U

#define XQSPIPSU_QSPIDMA_DST_I_STS_DONE_SHIFT   1U /**< Done field shift. */
#define XQSPIPSU_QSPIDMA_DST_I_STS_DONE_WIDTH   1U /**< Done field width. */
/** Done field mask. */
#define XQSPIPSU_QSPIDMA_DST_I_STS_DONE_MASK    0X00000002U

/** DMA error interrupts mask. */
#define XQSPIPSU_QSPIDMA_DST_INTR_ERR_MASK    0X000000FCU
/** All DMA interrupts mask. */
#define XQSPIPSU_QSPIDMA_DST_INTR_ALL_MASK    0X000000FEU

/** XQSPIPSU_QSPIDMA_DST_I_EN register offset. */
#define XQSPIPSU_QSPIDMA_DST_I_EN_OFFSET    0X00000718U

/** FIFO overflow field shift. */
#define XQSPIPSU_QSPIDMA_DST_I_EN_FIFO_OF_SHIFT   7U
/** FIFO overflow field width. */
#define XQSPIPSU_QSPIDMA_DST_I_EN_FIFO_OF_WIDTH   1U
/** FIFO overflow field mask. */
#define XQSPIPSU_QSPIDMA_DST_I_EN_FIFO_OF_MASK    0X00000080U

/** Invalid APB access field shift. */
#define XQSPIPSU_QSPIDMA_DST_I_EN_INVALID_APB_SHIFT   6U
/** Invalid APB access field width. */
#define XQSPIPSU_QSPIDMA_DST_I_EN_INVALID_APB_WIDTH   1U
/** Invalid APB access field mask. */
#define XQSPIPSU_QSPIDMA_DST_I_EN_INVALID_APB_MASK    0X00000040U

/** Threshold hit field shift. */
#define XQSPIPSU_QSPIDMA_DST_I_EN_THRESHOLD_HIT_SHIFT   5U
/** Threshold hit field width. */
#define XQSPIPSU_QSPIDMA_DST_I_EN_THRESHOLD_HIT_WIDTH   1U
/** Threshold hit field mask. */
#define XQSPIPSU_QSPIDMA_DST_I_EN_THRESHOLD_HIT_MASK    0X00000020U

/** Memory timeout field shift. */
#define XQSPIPSU_QSPIDMA_DST_I_EN_TO_MEM_SHIFT   4U
/** Memory timeout field width. */
#define XQSPIPSU_QSPIDMA_DST_I_EN_TO_MEM_WIDTH   1U
/** Memory timeout field mask. */
#define XQSPIPSU_QSPIDMA_DST_I_EN_TO_MEM_MASK    0X00000010U

/** Stream timeout field shift. */
#define XQSPIPSU_QSPIDMA_DST_I_EN_TO_STRM_SHIFT   3U
/** Stream timeout field width. */
#define XQSPIPSU_QSPIDMA_DST_I_EN_TO_STRM_WIDTH   1U
/** Stream timeout field mask. */
#define XQSPIPSU_QSPIDMA_DST_I_EN_TO_STRM_MASK    0X00000008U

/** AXI BRESP error field shift. */
#define XQSPIPSU_QSPIDMA_DST_I_EN_AXI_BRESP_ERR_SHIFT   2U
/** AXI BRESP error field width. */
#define XQSPIPSU_QSPIDMA_DST_I_EN_AXI_BRESP_ERR_WIDTH   1U
/** AXI BRESP error field mask. */
#define XQSPIPSU_QSPIDMA_DST_I_EN_AXI_BRESP_ERR_MASK    0X00000004U

#define XQSPIPSU_QSPIDMA_DST_I_EN_DONE_SHIFT   1U /**< Done field shift. */
#define XQSPIPSU_QSPIDMA_DST_I_EN_DONE_WIDTH   1U /**< Done field width. */
/** Done field mask. */
#define XQSPIPSU_QSPIDMA_DST_I_EN_DONE_MASK    0X00000002U

/** XQSPIPSU_QSPIDMA_DST_I_DIS register offset. */
#define XQSPIPSU_QSPIDMA_DST_I_DIS_OFFSET    0X0000071CU

/** FIFO overflow field shift. */
#define XQSPIPSU_QSPIDMA_DST_I_DIS_FIFO_OF_SHIFT   7U
/** FIFO overflow field width. */
#define XQSPIPSU_QSPIDMA_DST_I_DIS_FIFO_OF_WIDTH   1U
/** FIFO overflow field mask. */
#define XQSPIPSU_QSPIDMA_DST_I_DIS_FIFO_OF_MASK    0X00000080U

/** Invalid APB access field shift. */
#define XQSPIPSU_QSPIDMA_DST_I_DIS_INVALID_APB_SHIFT   6U
/** Invalid APB access field width. */
#define XQSPIPSU_QSPIDMA_DST_I_DIS_INVALID_APB_WIDTH   1U
/** Invalid APB access field mask. */
#define XQSPIPSU_QSPIDMA_DST_I_DIS_INVALID_APB_MASK    0X00000040U

/** Threshold hit field shift. */
#define XQSPIPSU_QSPIDMA_DST_I_DIS_THRESHOLD_HIT_SHIFT   5U
/** Threshold hit field width. */
#define XQSPIPSU_QSPIDMA_DST_I_DIS_THRESHOLD_HIT_WIDTH   1U
/** Threshold hit field mask. */
#define XQSPIPSU_QSPIDMA_DST_I_DIS_THRESHOLD_HIT_MASK    0X00000020U

/** Memory timeout field shift. */
#define XQSPIPSU_QSPIDMA_DST_I_DIS_TO_MEM_SHIFT   4U
/** Memory timeout field width. */
#define XQSPIPSU_QSPIDMA_DST_I_DIS_TO_MEM_WIDTH   1U
/** Memory timeout field mask. */
#define XQSPIPSU_QSPIDMA_DST_I_DIS_TO_MEM_MASK    0X00000010U

/** Stream timeout field shift. */
#define XQSPIPSU_QSPIDMA_DST_I_DIS_TO_STRM_SHIFT   3U
/** Stream timeout field width. */
#define XQSPIPSU_QSPIDMA_DST_I_DIS_TO_STRM_WIDTH   1U
/** Stream timeout field mask. */
#define XQSPIPSU_QSPIDMA_DST_I_DIS_TO_STRM_MASK    0X00000008U

/** AXI BRESP error field shift. */
#define XQSPIPSU_QSPIDMA_DST_I_DIS_AXI_BRESP_ERR_SHIFT   2U
/** AXI BRESP error field width. */
#define XQSPIPSU_QSPIDMA_DST_I_DIS_AXI_BRESP_ERR_WIDTH   1U
/** AXI BRESP error field mask. */
#define XQSPIPSU_QSPIDMA_DST_I_DIS_AXI_BRESP_ERR_MASK    0X00000004U

#define XQSPIPSU_QSPIDMA_DST_I_DIS_DONE_SHIFT   1U /**< Done field shift. */
#define XQSPIPSU_QSPIDMA_DST_I_DIS_DONE_WIDTH   1U /**< Done field width. */
/** Done field mask. */
#define XQSPIPSU_QSPIDMA_DST_I_DIS_DONE_MASK    0X00000002U

/** XQSPIPSU_QSPIDMA_DST_IMR register offset. */
#define XQSPIPSU_QSPIDMA_DST_IMR_OFFSET    0X00000720U

/** FIFO overflow field shift. */
#define XQSPIPSU_QSPIDMA_DST_IMR_FIFO_OF_SHIFT   7U
/** FIFO overflow field width. */
#define XQSPIPSU_QSPIDMA_DST_IMR_FIFO_OF_WIDTH   1U
/** FIFO overflow field mask. */
#define XQSPIPSU_QSPIDMA_DST_IMR_FIFO_OF_MASK    0X00000080U

/** Invalid APB access field shift. */
#define XQSPIPSU_QSPIDMA_DST_IMR_INVALID_APB_SHIFT   6U
/** Invalid APB access field width. */
#define XQSPIPSU_QSPIDMA_DST_IMR_INVALID_APB_WIDTH   1U
/** Invalid APB access field mask. */
#define XQSPIPSU_QSPIDMA_DST_IMR_INVALID_APB_MASK    0X00000040U

/** Threshold hit field shift. */
#define XQSPIPSU_QSPIDMA_DST_IMR_THRESHOLD_HIT_SHIFT   5U
/** Threshold hit field width. */
#define XQSPIPSU_QSPIDMA_DST_IMR_THRESHOLD_HIT_WIDTH   1U
/** Threshold hit field mask. */
#define XQSPIPSU_QSPIDMA_DST_IMR_THRESHOLD_HIT_MASK    0X00000020U

/** Memory timeout field shift. */
#define XQSPIPSU_QSPIDMA_DST_IMR_TO_MEM_SHIFT   4U
/** Memory timeout field width. */
#define XQSPIPSU_QSPIDMA_DST_IMR_TO_MEM_WIDTH   1U
/** Memory timeout field mask. */
#define XQSPIPSU_QSPIDMA_DST_IMR_TO_MEM_MASK    0X00000010U

/** Stream timeout field shift. */
#define XQSPIPSU_QSPIDMA_DST_IMR_TO_STRM_SHIFT   3U
/** Stream timeout field width. */
#define XQSPIPSU_QSPIDMA_DST_IMR_TO_STRM_WIDTH   1U
/** Stream timeout field mask. */
#define XQSPIPSU_QSPIDMA_DST_IMR_TO_STRM_MASK    0X00000008U

/** AXI BRESP error field shift. */
#define XQSPIPSU_QSPIDMA_DST_IMR_AXI_BRESP_ERR_SHIFT   2U
/** AXI BRESP error field width. */
#define XQSPIPSU_QSPIDMA_DST_IMR_AXI_BRESP_ERR_WIDTH   1U
/** AXI BRESP error field mask. */
#define XQSPIPSU_QSPIDMA_DST_IMR_AXI_BRESP_ERR_MASK    0X00000004U

#define XQSPIPSU_QSPIDMA_DST_IMR_DONE_SHIFT   1U /**< Done field shift. */
#define XQSPIPSU_QSPIDMA_DST_IMR_DONE_WIDTH   1U /**< Done field width. */
/** Done field mask. */
#define XQSPIPSU_QSPIDMA_DST_IMR_DONE_MASK    0X00000002U

/** XQSPIPSU_QSPIDMA_DST_CTRL2 register offset. */
#define XQSPIPSU_QSPIDMA_DST_CTRL2_OFFSET    0X00000724U

/** RAM EMASA field shift. */
#define XQSPIPSU_QSPIDMA_DST_CTRL2_RAM_EMASA_SHIFT   27U
/** RAM EMASA field width. */
#define XQSPIPSU_QSPIDMA_DST_CTRL2_RAM_EMASA_WIDTH   1U
/** RAM EMASA field mask. */
#define XQSPIPSU_QSPIDMA_DST_CTRL2_RAM_EMASA_MASK    0X08000000U

/** AXI AWCACHE field shift. */
#define XQSPIPSU_QSPIDMA_DST_CTRL2_AWCACHE_SHIFT   24U
/** AXI AWCACHE field width. */
#define XQSPIPSU_QSPIDMA_DST_CTRL2_AWCACHE_WIDTH   3U
/** AXI AWCACHE field mask. */
#define XQSPIPSU_QSPIDMA_DST_CTRL2_AWCACHE_MASK    0X07000000U

/** Timeout enable field shift. */
#define XQSPIPSU_QSPIDMA_DST_CTRL2_TO_EN_SHIFT   22U
/** Timeout enable field width. */
#define XQSPIPSU_QSPIDMA_DST_CTRL2_TO_EN_WIDTH   1U
/** Timeout enable field mask. */
#define XQSPIPSU_QSPIDMA_DST_CTRL2_TO_EN_MASK    0X00400000U

/** RAM EMAB field shift. */
#define XQSPIPSU_QSPIDMA_DST_CTRL2_RAM_EMAB_SHIFT   19U
/** RAM EMAB field width. */
#define XQSPIPSU_QSPIDMA_DST_CTRL2_RAM_EMAB_WIDTH   3U
/** RAM EMAB field mask. */
#define XQSPIPSU_QSPIDMA_DST_CTRL2_RAM_EMAB_MASK    0X00380000U

/** RAM EMAA field shift. */
#define XQSPIPSU_QSPIDMA_DST_CTRL2_RAM_EMAA_SHIFT   16U
/** RAM EMAA field width. */
#define XQSPIPSU_QSPIDMA_DST_CTRL2_RAM_EMAA_WIDTH   3U
/** RAM EMAA field mask. */
#define XQSPIPSU_QSPIDMA_DST_CTRL2_RAM_EMAA_MASK    0X00070000U

/** Timeout prescaler field shift. */
#define XQSPIPSU_QSPIDMA_DST_CTRL2_TO_PRE_SHIFT   4U
/** Timeout prescaler field width. */
#define XQSPIPSU_QSPIDMA_DST_CTRL2_TO_PRE_WIDTH   12U
/** Timeout prescaler field mask. */
#define XQSPIPSU_QSPIDMA_DST_CTRL2_TO_PRE_MASK    0X0000FFF0U

/** Maximum outstanding commands field shift. */
#define XQSPIPSU_QSPIDMA_DST_CTRL2_MAX_OUTS_CMDS_SHIFT   0U
/** Maximum outstanding commands field width. */
#define XQSPIPSU_QSPIDMA_DST_CTRL2_MAX_OUTS_CMDS_WIDTH   4U
/** Maximum outstanding commands field mask. */
#define XQSPIPSU_QSPIDMA_DST_CTRL2_MAX_OUTS_CMDS_MASK    0X0000000FU

/** XQSPIPSU_QSPIDMA_DST_ADDR_MSB register offset. */
#define XQSPIPSU_QSPIDMA_DST_ADDR_MSB_OFFSET    0X00000728U

/** Destination address MSB field shift. */
#define XQSPIPSU_QSPIDMA_DST_ADDR_MSB_SHIFT   0U
/** Destination address MSB field width. */
#define XQSPIPSU_QSPIDMA_DST_ADDR_MSB_WIDTH   12U
/** Destination address MSB field mask. */
#define XQSPIPSU_QSPIDMA_DST_ADDR_MSB_MASK    0X00000FFFU

/** XQSPIPSU_QSPIDMA_FUTURE_ECO register offset. */
#define XQSPIPSU_QSPIDMA_FUTURE_ECO_OFFSET    0X00000EFCU

/** Future ECO value field shift. */
#define XQSPIPSU_QSPIDMA_FUTURE_ECO_VAL_SHIFT   0U
/** Future ECO value field width. */
#define XQSPIPSU_QSPIDMA_FUTURE_ECO_VAL_WIDTH   32U
/** Future ECO value field mask. */
#define XQSPIPSU_QSPIDMA_FUTURE_ECO_VAL_MASK    0XFFFFFFFFU
/** @} */

/**
 * @name XQSPIPSU Generic FIFO masks information
 * Generic FIFO masks information.
 * @{
 */
#define XQSPIPSU_GENFIFO_IMM_DATA_MASK	0xFFU /**< Immediate data mask. */
#define XQSPIPSU_GENFIFO_DATA_XFER	0x100U /**< Data transfer. */
#define XQSPIPSU_GENFIFO_EXP		0x200U /**< Exponent length. */
#define XQSPIPSU_GENFIFO_MODE_SPI	0x400U /**< SPI mode. */
#define XQSPIPSU_GENFIFO_MODE_DUALSPI	0x800U /**< Dual SPI mode. */
#define XQSPIPSU_GENFIFO_MODE_QUADSPI	0xC00U /**< Quad SPI mode. */
/** Mode mask. AND with ~MASK first. */
#define XQSPIPSU_GENFIFO_MODE_MASK	0xC00U
#define XQSPIPSU_GENFIFO_CS_LOWER	0x1000U /**< Lower chip select. */
#define XQSPIPSU_GENFIFO_CS_UPPER	0x2000U /**< Upper chip select. */
#define XQSPIPSU_GENFIFO_BUS_LOWER	0x4000U /**< Lower bus. */
#define XQSPIPSU_GENFIFO_BUS_UPPER	0x8000U /**< Upper bus. */
/** Both buses. The inverse is no bus. */
#define XQSPIPSU_GENFIFO_BUS_BOTH	0xC000U
/** Bus mask. AND with ~MASK first. */
#define XQSPIPSU_GENFIFO_BUS_MASK	0xC000U
/** TX. The inverse is zero pump. */
#define XQSPIPSU_GENFIFO_TX		0x10000U
/** RX. The inverse is RX discard. */
#define XQSPIPSU_GENFIFO_RX		0x20000U
#define XQSPIPSU_GENFIFO_STRIPE		0x40000U /**< Data stripe. */
#define XQSPIPSU_GENFIFO_POLL		0x80000U /**< Poll. */
/** @} */

/**
 * @name XQSPIPSU RX Data Delay Register information
 * @{
 */
/**
 * QSPI data delay register offset.
 */
#define XQSPIPSU_DATA_DLY_ADJ_OFFSET    0X000000F8U

/** Use data delay field shift. */
#define XQSPIPSU_DATA_DLY_ADJ_USE_DATA_DLY_SHIFT   31U
/** Use data delay field width. */
#define XQSPIPSU_DATA_DLY_ADJ_USE_DATA_DLY_WIDTH   1U
/** Use data delay field mask. */
#define XQSPIPSU_DATA_DLY_ADJ_USE_DATA_DLY_MASK    0X80000000U

#define XQSPIPSU_DATA_DLY_ADJ_DLY_SHIFT   28U /**< Data delay field shift. */
#define XQSPIPSU_DATA_DLY_ADJ_DLY_WIDTH   3U /**< Data delay field width. */
/** Data delay field mask. */
#define XQSPIPSU_DATA_DLY_ADJ_DLY_MASK    0X70000000U
/** @} */

/**
 * @name TAPDLY Bypass register information
 * @{
 */
/**
 * Tap delay bypass register offset.
 */
#if defined versal
#define IOU_TAPDLY_BYPASS_OFFSET 0X0000003CU
#else
#define IOU_TAPDLY_BYPASS_OFFSET 0X00000390U
#endif

/** LQSPI RX tap delay bypass field shift. */
#define IOU_TAPDLY_BYPASS_LQSPI_RX_SHIFT 0X02U
#if !defined (versal)
/** LQSPI RX tap delay bypass field width. */
#define IOU_TAPDLY_BYPASS_LQSPI_RX_WIDTH 0X01U
/** LQSPI RX tap delay bypass field mask. */
#define IOU_TAPDLY_BYPASS_LQSPI_RX_MASK 0x00000004U
#endif

#if defined versal
#define IOU_TAPDLY_RESET_STATE 0x4U /**< Tap delay bypass reset state. */
#else
#define IOU_TAPDLY_RESET_STATE 0x7U /**< Tap delay bypass reset state. */
#endif
/** @} */

/***************** Macros (Inline Functions) Definitions *********************/

/** @name Register access functions
 * @{
 */
#define XQspiPsu_In32 Xil_In32   /**< Reads the 32-bit register value. */
#define XQspiPsu_Out32 Xil_Out32 /**< Writes the 32-bit register value. */
/** @} */

/****************************************************************************/
/**
* @brief Reads a register.
*
* @param[in]    BaseAddress Base address of the device.
* @param[in]    RegOffset Offset from the first register of the device to the
*		target register.
*
* @return	The value read from the register.
*
* @note		C-style signature:
*		u32 XQspiPsu_ReadReg(u32 BaseAddress, s32 RegOffset)
*
******************************************************************************/
#define XQspiPsu_ReadReg(BaseAddress, RegOffset) XQspiPsu_In32((BaseAddress) + (RegOffset))

/***************************************************************************/
/**
* @brief Writes to a register.
*
* @param[in]    BaseAddress Base address of the device.
* @param[in]    RegOffset Offset from the first register of the device to the
*		target register.
* @param[in]    RegisterValue Value to write to the register.
*
* @note		C-style signature:
*		void XQspiPsu_WriteReg(u32 BaseAddress, s32 RegOffset,
*				       u32 RegisterValue)
*
******************************************************************************/
#define XQspiPsu_WriteReg(BaseAddress, RegOffset, RegisterValue) XQspiPsu_Out32((BaseAddress) + (RegOffset), (RegisterValue))


#ifdef __cplusplus
}
#endif


/** @} */
#endif /* XQSPIPSU_HW_H */
