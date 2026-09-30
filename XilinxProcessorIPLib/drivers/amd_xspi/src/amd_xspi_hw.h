/******************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef AMD_XSPI_HW_H
#define AMD_XSPI_HW_H

/*****************************************************************************/
/**
*
* @file amd_xspi_hw.h
* @brief Defines the hardware registers and macros for the AXI XSPI controller.
*
* This header file contains the hardware register definitions and macros for
* the AXI XSPI controller. This driver supports AXI-Lite mode only.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   akm  07/29/26 Initial release
* 1.0   vn   09/30/26 Align Doxygen comments with AMD standards
* </pre>
*
******************************************************************************/

/** @addtogroup amd_xspi Overview
 * @{
 */


#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_io.h"

/************************** Constant Definitions *****************************/

/** @name Register Offsets
 * @{
 */
/** Version Register. */
#define AMD_XSPI_VERSION_OFFSET			0x0000U
/** Identifier Register. */
#define AMD_XSPI_IDENTIFIER_OFFSET			0x0004U
/** Reserved Register 1. */
#define AMD_XSPI_RSVD1_OFFSET			0x0008U
/** Reserved Register 2. */
#define AMD_XSPI_RSVD2_OFFSET			0x000CU
/** Control Register. */
#define AMD_XSPI_CONTROL_OFFSET			0x0010U
/** Target Select Register. */
#define AMD_XSPI_TARGET_SELECT_OFFSET		0x0014U
/** Transmit Register. */
#define AMD_XSPI_TRANSMIT_OFFSET			0x0018U
/** Receive Register. */
#define AMD_XSPI_RECEIVE_OFFSET			0x001CU
/** Interrupt Enable Register. */
#define AMD_XSPI_IER_OFFSET			0x0020U
/** Interrupt Status Register. */
#define AMD_XSPI_ISR_OFFSET			0x0024U
/** Interrupt Event Register. */
#define AMD_XSPI_INT_EVT_OFFSET			0x0028U
/** TX FIFO Occupancy Register. */
#define AMD_XSPI_TX_OCY_OFFSET			0x002CU
/** RX FIFO Occupancy Register. */
#define AMD_XSPI_RX_OCY_OFFSET			0x0030U
/** @} */

/** @name Version Register bit definitions
 * @{
 */
/** Core major version. */
#define AMD_XSPI_VERSION_MAJOR_MASK		0xFF000000U
/** Core major version shift. */
#define AMD_XSPI_VERSION_MAJOR_SHIFT		24U
/** Core minor version. */
#define AMD_XSPI_VERSION_MINOR_MASK		0x00FF0000U
/** Core minor version shift. */
#define AMD_XSPI_VERSION_MINOR_SHIFT		16U
/** Core revision. */
#define AMD_XSPI_VERSION_REVISION_MASK		0x0000FF00U
/** Core revision shift. */
#define AMD_XSPI_VERSION_REVISION_SHIFT		8U
/** Patch revision. */
#define AMD_XSPI_VERSION_PATCH_MASK		0x000000F0U
#define AMD_XSPI_VERSION_PATCH_SHIFT		4U /**< Patch revision shift. */
/** Internal revision. */
#define AMD_XSPI_VERSION_INTERNAL_MASK		0x0000000EU
/** Internal revision shift. */
#define AMD_XSPI_VERSION_INTERNAL_SHIFT		1U
/** @} */

/** @name Control Register bit definitions
 * @{
 */
/** Inject 2-bit ECC error. */
#define AMD_XSPI_CR_INJ_2BE_MASK		0x00000100U
/** Inject 1-bit ECC error. */
#define AMD_XSPI_CR_INJ_1BE_MASK		0x00000080U
/** Clock Polarity. */
#define AMD_XSPI_CR_CPOL_MASK			0x00000040U
#define AMD_XSPI_CR_CPHA_MASK			0x00000020U /**< Clock Phase. */
/** RX FIFO Reset. */
#define AMD_XSPI_CR_RXFIFO_RESET_MASK		0x00000010U
/** TX FIFO Reset. */
#define AMD_XSPI_CR_TXFIFO_RESET_MASK		0x00000008U
/**
 * Abort Transaction. Not implemented by this core; a running transaction is
 * torn down with SOFT_RESET instead.
 */
#define AMD_XSPI_CR_ABORT_MASK			0x00000004U
/** Core Enable. */
#define AMD_XSPI_CR_ENABLE_MASK			0x00000002U
#define AMD_XSPI_CR_SOFT_RESET_MASK		0x00000001U /**< Soft Reset. */
/** @} */

/** @name Target Select Register bit definitions
 * @{
 */
/** Chip Select (active-high, one-hot). */
#define AMD_XSPI_TSR_CHIP_SEL_MASK		0x000000FFU
/** @} */

/** @name Transmit Register bit definitions
 * @{
 */
/** TX Data (18-bit). */
#define AMD_XSPI_TX_DATA_MASK			0x0003FFFFU
/** @} */

/** @name Receive Register bit definitions
 * @{
 */
/** RX Data (16-bit). */
#define AMD_XSPI_RX_DATA_MASK			0x0000FFFFU
/** @} */

/** @name Interrupt Enable/Status/Event Register bit definitions
 *
 * The same bit positions are used by all three registers, but they do not
 * behave alike. AMD_XSPI_ISR reports each condition as a live level,
 * independently of what AMD_XSPI_IER enables, so it can be polled directly
 * and needs no acknowledgement. AMD_XSPI_INT_EVT latches an event only if
 * the matching IER bit was set when the condition fired, and holds it until
 * it is written back.
 *
 * The polled paths rely on the live behavior. They test AMD_XSPI_ISR once per
 * message without clearing anything between messages. That is only correct
 * because a bit there tracks the current condition rather than remaining set
 * from an earlier transaction.
 *
 * @{
 */
/** RX FIFO Half Full. */
#define AMD_XSPI_INTR_RX_HALF_FULL_MASK		0x00010000U
/** TX FIFO Full. */
#define AMD_XSPI_INTR_TX_FULL_MASK		0x00008000U
/** Transaction Complete (Lite mode). */
#define AMD_XSPI_INTR_TXN_COMPLETE_MASK		0x00004000U
/** Initialization Done. */
#define AMD_XSPI_INTR_INIT_DONE_MASK		0x00002000U
/** Transaction Error. */
#define AMD_XSPI_INTR_TXN_ERROR_MASK		0x00001000U
/** RX FIFO 2-bit ECC Error. */
#define AMD_XSPI_INTR_RX_FIFO_ECC_2BE_MASK	0x00000800U
/** RX FIFO 1-bit ECC Error. */
#define AMD_XSPI_INTR_RX_FIFO_ECC_1BE_MASK	0x00000400U
/** TX FIFO 2-bit ECC Error. */
#define AMD_XSPI_INTR_TX_FIFO_ECC_2BE_MASK	0x00000200U
/** TX FIFO 1-bit ECC Error. */
#define AMD_XSPI_INTR_TX_FIFO_ECC_1BE_MASK	0x00000100U
/** TX FIFO Almost Full. */
#define AMD_XSPI_INTR_TX_ALMOST_FULL_MASK	0x00000080U
/** TX FIFO Half Full. */
#define AMD_XSPI_INTR_TX_HALF_FULL_MASK		0x00000040U
/** TX FIFO Almost Empty. */
#define AMD_XSPI_INTR_TX_ALMOST_EMPTY_MASK	0x00000020U
/** TX FIFO Empty. */
#define AMD_XSPI_INTR_TX_EMPTY_MASK		0x00000010U
/** RX FIFO Almost Full. */
#define AMD_XSPI_INTR_RX_ALMOST_FULL_MASK	0x00000008U
/** RX FIFO Full. */
#define AMD_XSPI_INTR_RX_FULL_MASK		0x00000004U
/** RX FIFO Almost Empty. */
#define AMD_XSPI_INTR_RX_ALMOST_EMPTY_MASK	0x00000002U
/** RX FIFO Empty. */
#define AMD_XSPI_INTR_RX_EMPTY_MASK		0x00000001U

/** All Interrupts. */
#define AMD_XSPI_INTR_ALL_MASK			0x0001FFFFU

/** Every interrupt that reports a fault rather than progress */
#define AMD_XSPI_INTR_ERROR_MASK		(AMD_XSPI_INTR_TXN_ERROR_MASK | \
						 AMD_XSPI_INTR_RX_FIFO_ECC_2BE_MASK | \
						 AMD_XSPI_INTR_RX_FIFO_ECC_1BE_MASK | \
						 AMD_XSPI_INTR_TX_FIFO_ECC_2BE_MASK | \
						 AMD_XSPI_INTR_TX_FIFO_ECC_1BE_MASK)

/**
 * RX fill-level events used to drain receive data while a transaction is
 * still in progress, so a read is not limited to one FIFO load.
 */
#define AMD_XSPI_INTR_RX_THRESHOLD_MASK	(AMD_XSPI_INTR_RX_HALF_FULL_MASK | \
						 AMD_XSPI_INTR_RX_ALMOST_FULL_MASK | \
						 AMD_XSPI_INTR_RX_FULL_MASK)

/**
 * TX fill-level events used to top the transmit FIFO up while a transaction is
 * still in progress. So a write is not limited to one FIFO load.
 *
 * These are the empty-side events, because for the transmit FIFO it is draining
 * that creates the room to write more. This mirrors the RX mask above. Half
 * full is included so the refill starts while the core still has data to clock
 * out. Waiting for empty alone would let it run dry and spend the tCSM budget
 * waiting.
 */
#define AMD_XSPI_INTR_TX_THRESHOLD_MASK	(AMD_XSPI_INTR_TX_HALF_FULL_MASK | \
						 AMD_XSPI_INTR_TX_ALMOST_EMPTY_MASK | \
						 AMD_XSPI_INTR_TX_EMPTY_MASK)
/** @} */

/** @name FIFO Occupancy Register bit definitions
 * @{
 */
/** FIFO Occupancy (10-bit). */
#define AMD_XSPI_FIFO_OCY_MASK			0x000003FFU
/** @} */

/** @name SPI Clock Mode definitions (CPOL/CPHA)
 * For xSPI modes only Mode 0 (CPOL=0, CPHA=0) is supported.
 * @{
 */
#define AMD_XSPI_CLK_MODE_0			0U /**< CPOL=0, CPHA=0. */
#define AMD_XSPI_CLK_MODE_1			1U /**< CPOL=0, CPHA=1. */
#define AMD_XSPI_CLK_MODE_2			2U /**< CPOL=1, CPHA=0. */
#define AMD_XSPI_CLK_MODE_3			3U /**< CPOL=1, CPHA=1. */
/** @} */

/** @name XSPI Controller Operating Mode definitions (C_SPI_MODE)
 * These modes define the data transfer protocol used by the controller.
 * @{
 */
/** Standard SPI half-duplex (io0 only). */
#define AMD_XSPI_MODE_STANDARD_HALF_DUPLEX		0U
/** Standard SPI (io0=MOSI, io1=MISO). */
#define AMD_XSPI_MODE_STANDARD			1U
/** Dual SPI (io0-io1). */
#define AMD_XSPI_MODE_DUAL				2U
/** Quad SPI (io0-io3). */
#define AMD_XSPI_MODE_QUAD				3U
/** xSPI Profile 1.0 (Octal, io0-io7). */
#define AMD_XSPI_MODE_XSPI_PROFILE_1_0		4U
/** xSPI Profile 2.0 (HyperBus). */
#define AMD_XSPI_MODE_XSPI_PROFILE_2_0		5U
/** @} */

/** @name Transmit Register Instruction Format bit definitions
 * In Lite mode, the Transmit Register uses a 32-bit instruction format.
 * User writes instruction first (in 2 AXI-Lite transactions with flags),
 * then command/address/data bytes.
 * @{
 */
/* Bits 31:30 - Reserved */
/** No of cmd+addr bytes [29:28]. */
#define AMD_XSPI_INSTR_CMD_ADDR_BYTES_MASK		0x30000000U
/** No of cmd+addr bytes shift. */
#define AMD_XSPI_INSTR_CMD_ADDR_BYTES_SHIFT	28U
/* Bits 27:26 - Reserved */
/** No of Rd/Wr bytes [25:16]. */
#define AMD_XSPI_INSTR_DATA_BYTES_MASK		0x03FF0000U
/** No of Rd/Wr bytes shift. */
#define AMD_XSPI_INSTR_DATA_BYTES_SHIFT		16U
/** R/W bit [15]: 0=Write, 1=Read. */
#define AMD_XSPI_INSTR_RW_MASK			0x00008000U
/* Bit 14 - Reserved */
/** Dummy/latency cycles [13:8]. */
#define AMD_XSPI_INSTR_DUMMY_CYCLES_MASK		0x00003F00U
/** Dummy/latency cycles shift. */
#define AMD_XSPI_INSTR_DUMMY_CYCLES_SHIFT		8U
/* Bit 7 - Reserved */
/** IO Format [6:4]. */
#define AMD_XSPI_INSTR_IO_FORMAT_MASK		0x00000070U
#define AMD_XSPI_INSTR_IO_FORMAT_SHIFT		4U /**< IO Format shift. */
/* Bit 3 - Reserved */
/** Frame Format [2:0]. */
#define AMD_XSPI_INSTR_FRAME_FORMAT_MASK		0x00000007U
/** Frame Format shift. */
#define AMD_XSPI_INSTR_FRAME_FORMAT_SHIFT		0U
/** @} */

/** @name Command/Address Bytes encoding [29:28]
 * @{
 */
#define AMD_XSPI_CMD_ADDR_1_OR_2_BYTES		0U	   /**< 1 or 2 bytes. */
#define AMD_XSPI_CMD_ADDR_4_BYTES			1U /**< 4 bytes. */
#define AMD_XSPI_CMD_ADDR_5_BYTES			2U /**< 5 bytes. */
#define AMD_XSPI_CMD_ADDR_6_BYTES			3U /**< 6 bytes. */

/**
*
* @brief Checks whether a command/address byte count is valid for the core mode.
*
* The valid counts depend on the mode the core is built in as well as on the
* field above.
*
 * On an xSPI Profile 2.0 core the frame is HyperBus, whose command/address word
 * is always 48 bits, so 6 is the only count the protocol admits. On any other
 * core the count is 1, 2, 4, 5 or 6: one command byte, optionally a command
 * extension byte, and a 3 or 4 byte address. A count of 3, a count of 0 and
 * anything above 6 have no encoding in the field at all. So callers must reject
 * them rather than let them widen to the 6 byte encoding.
*
* @param[in]   Mode SPI mode the core is built in.
* @param[in]   Bytes Command/address byte count to check.
*
* @return      Nonzero if the count is valid, otherwise 0.
*
* @note        C-style signature:
*              u32 AMD_XSPI_IS_VALID_CMD_ADDR_BYTES(u32 Mode, u32 Bytes)
*
 */
#define AMD_XSPI_IS_VALID_CMD_ADDR_BYTES(Mode, Bytes)			\
	(((Mode) == AMD_XSPI_MODE_XSPI_PROFILE_2_0) ? ((Bytes) == 6U) :	\
	 (((Bytes) == 1U) || ((Bytes) == 2U) || ((Bytes) == 4U) ||	\
	  ((Bytes) == 5U) || ((Bytes) == 6U)))

/** Data bytes per transaction. */
#define AMD_XSPI_MAX_DATA_BYTES		512U
/** Dummy/latency cycles. */
#define AMD_XSPI_MAX_DUMMY_CYCLES		63U
/** @} */

/** @name Frame Format encoding [2:0]
 * @{
 */
/** Read with latency/dummy cycles. */
#define AMD_XSPI_FRAME_READ_WITH_LATENCY		0U
/** Write with latency. */
#define AMD_XSPI_FRAME_WRITE_WITH_LATENCY		1U
/** Write with no latency. */
#define AMD_XSPI_FRAME_WRITE_NO_LATENCY		2U
#define AMD_XSPI_FRAME_READ_NO_LATENCY		3U /**< Read with no latency. */
/** @} */

/** @name IO Format encoding [6:4] for Dual/Quad modes
 * - For Standard modes: always 1-1-1 (do not care).
 * - For xSPI HyperBus: always 8D-8D-8D (do not care).
 * - For xSPI Octal: B3=0 for 8D-8D-8D, B3=1 for 1S-1S-1S.
 * @{
 */
/** 1-1-1 (cmd-addr-data all single). */
#define AMD_XSPI_IO_FORMAT_1_1_1			0U
/** 1-1-2 (data dual). */
#define AMD_XSPI_IO_FORMAT_1_1_2			1U
/** 1-2-2 (addr+data dual). */
#define AMD_XSPI_IO_FORMAT_1_2_2			2U
/** 2-2-2 (all dual). */
#define AMD_XSPI_IO_FORMAT_2_2_2			3U
/** 1-1-4 (data quad). */
#define AMD_XSPI_IO_FORMAT_1_1_4			4U
/** 1-4-4 (addr+data quad). */
#define AMD_XSPI_IO_FORMAT_1_4_4			5U
/** 4-4-4 (all quad). */
#define AMD_XSPI_IO_FORMAT_4_4_4			6U
/** 8D-8D-8D for xSPI (B3=0). */
#define AMD_XSPI_IO_FORMAT_8D_8D_8D		0U
/** 1S-1S-1S for xSPI octal (B3=1). */
#define AMD_XSPI_IO_FORMAT_1S_1S_1S		1U
/** @} */

/** @name Transmit Register Flag bits (MSB 2 bits of 18-bit FIFO entry)
 * The TX FIFO is 18 bits wide with MSB 2 bits as flag bits.
 * @{
 */
/** Flag 01: LSB 16-bit instruction. */
#define AMD_XSPI_TX_FLAG_INSTR_LSB			0x1U
/** Flag 10: MSB instruction bits. */
#define AMD_XSPI_TX_FLAG_INSTR_MSB			0x2U
/** Flag 11: Data bytes. */
#define AMD_XSPI_TX_FLAG_DATA			0x3U
/** @} */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* @brief Reads a register value from the AXI XSPI core.
*
* @param[in]   BaseAddress Base address of the AXI XSPI core.
* @param[in]   RegOffset Offset of the register to read.
*
* @return      The 32-bit value of the register.
*
* @note        C-style signature:
*              u32 AmdXspi_ReadReg(UINTPTR BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define AmdXspi_ReadReg(BaseAddress, RegOffset) \
	Xil_In32((BaseAddress) + (u32)(RegOffset))

/*****************************************************************************/
/**
*
* @brief Writes a value to a register in the AXI XSPI core.
*
* @param[in]   BaseAddress Base address of the AXI XSPI core.
* @param[in]   RegOffset Offset of the register to write.
* @param[in]   Data 32-bit value to write to the register.
*
* @note        C-style signature:
*              void AmdXspi_WriteReg(UINTPTR BaseAddress, u32 RegOffset,
*                                    u32 Data)
*
******************************************************************************/
#define AmdXspi_WriteReg(BaseAddress, RegOffset, Data) \
	Xil_Out32((BaseAddress) + (u32)(RegOffset), (u32)(Data))

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif


/** @} */
#endif /* AMD_XSPI_HW_H */
