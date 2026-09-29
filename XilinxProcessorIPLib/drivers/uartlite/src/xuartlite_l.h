/******************************************************************************
* Copyright (C) 2002 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XUARTLITE_L_H
#define XUARTLITE_L_H

/*****************************************************************************/
/**
*
* @file xuartlite_l.h
* @brief Low-level driver identifiers and functions for the UART Lite device.
*
* This header file contains identifiers and low-level driver functions (or
* macros) that can be used to access the device. High-level driver functions
* are defined in xuartlite.h.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00b rpm  04/25/02 First release
* 1.00b rpm  07/07/03 Removed references to XUartLite_GetControlReg macro
*                     since the control register is write-only
* 1.12a mta  03/21/07 Updated to new coding style
* 1.13a sv   01/21/08 Updated driver to support access through DCR bus
* 2.00a ktn  10/20/09 Updated to use HAL Processor APIs. The macros have been
*		      renamed to remove _m from the name.
* 3.2   sk   11/10/15 Used UINTPTR instead of u32 for Baseaddress CR# 867425.
*                     Changed the prototypes of XUartLite_SendByte,
*                     XUartLite_RecvByte APIs.
* 3.14  vn   09/09/26 Align Doxygen comments with AMD standards
* </pre>
*
******************************************************************************/

/** @addtogroup uartlite Overview
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xil_io.h"

/** @name DCR bus bridge configuration
 *
 * Set to 1 if the UartLite device is accessed through a DCR bus
 * connected to a bridge.
 * @{
 */
/** DCR bus bridge access flag. */
#define XPAR_XUARTLITE_USE_DCR_BRIDGE 0
/** @} */

#if (XPAR_XUARTLITE_USE_DCR_BRIDGE != 0)
#include "xio_dcr.h"
#endif


/************************** Constant Definitions ****************************/

/** @name UART Lite register offsets
 * @{
 */
#if (XPAR_XUARTLITE_USE_DCR_BRIDGE != 0)
#define XUL_RX_FIFO_OFFSET		0	/**< Receive FIFO, read only. */
#define XUL_TX_FIFO_OFFSET		1	/**< Transmit FIFO, write only. */
#define XUL_STATUS_REG_OFFSET		2	/**< Status register, read only. */
#define XUL_CONTROL_REG_OFFSET		3	/**< Control register, write only. */

#else

#define XUL_RX_FIFO_OFFSET		0	/**< Receive FIFO, read only. */
#define XUL_TX_FIFO_OFFSET		4	/**< Transmit FIFO, write only. */
#define XUL_STATUS_REG_OFFSET		8	/**< Status register, read only. */
#define XUL_CONTROL_REG_OFFSET		12	/**< Control register, write only. */

#endif
/** @} */

/** @name Control register bit positions
 * @{
 */
#define XUL_CR_ENABLE_INTR		0x10	/**< Enable interrupt. */
#define XUL_CR_FIFO_RX_RESET		0x02	/**< Reset receive FIFO. */
#define XUL_CR_FIFO_TX_RESET		0x01	/**< Reset transmit FIFO. */
/** @} */

/** @name Status register bit positions
 * @{
 */
#define XUL_SR_PARITY_ERROR		0x80	/**< Parity error detected. */
#define XUL_SR_FRAMING_ERROR		0x40	/**< Framing error detected. */
#define XUL_SR_OVERRUN_ERROR		0x20	/**< Overrun error detected. */
#define XUL_SR_INTR_ENABLED		0x10	/**< Interrupt enabled. */
#define XUL_SR_TX_FIFO_FULL		0x08	/**< Transmit FIFO full. */
#define XUL_SR_TX_FIFO_EMPTY		0x04	/**< Transmit FIFO empty. */
#define XUL_SR_RX_FIFO_FULL		0x02	/**< Receive FIFO full. */
#define XUL_SR_RX_FIFO_VALID_DATA	0x01	/**< Data in receive FIFO. */
/** @} */

/** @name FIFO and serial line constants
 *
 * Fixed hardware parameters for the UartLite IP.
 * @{
 */
/** Transmit/receive FIFO size in bytes. Fixed to 16 in the UartLite IP,
 * not configurable, and not used by the driver.
 */
#define XUL_FIFO_SIZE			16
/** Stop bits, fixed at 1. Baud, parity, and data bits are fixed on a
 * per-instance basis.
 */
#define XUL_STOP_BITS			1
/** @} */

/** @name Parity definitions
 * @{
 */
#define XUL_PARITY_NONE			0/**< No parity. */
#define XUL_PARITY_ODD			1/**< Odd parity. */
#define XUL_PARITY_EVEN			2/**< Even parity. */
/** @} */

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/** @name I/O access method macros
 *
 * Define the appropriate I/O access method to memory mapped I/O or DCR.
 * @{
 */
#if (XPAR_XUARTLITE_USE_DCR_BRIDGE != 0)

/** DCR 32-bit input operation. */
#define XUartLite_In32  XIo_DcrIn
/** DCR 32-bit output operation. */
#define XUartLite_Out32 XIo_DcrOut

#else

/** Memory-mapped 32-bit input operation. */
#define XUartLite_In32  Xil_In32
/** Memory-mapped 32-bit output operation. */
#define XUartLite_Out32 Xil_Out32

#endif
/** @} */


/*****************************************************************************/
/**
*
* @brief Writes a value to a UartLite register.
*
* A 32 bit write is performed.
*
* @param[in]   BaseAddress Base address of the UartLite device.
* @param[in]   RegOffset Register offset from the base to write to.
* @param[in]   Data Data written to the register.
*
* @note        C-style signature:
*              void XUartLite_WriteReg(u32 BaseAddress, u32 RegOffset,
*                                      u32 Data)
*
******************************************************************************/
#define XUartLite_WriteReg(BaseAddress, RegOffset, Data) \
	XUartLite_Out32((BaseAddress) + (RegOffset), (u32)(Data))

/*****************************************************************************/
/**
*
* @brief Reads a value from a UartLite register.
*
* A 32 bit read is performed.
*
* @param[in]   BaseAddress Base address of the UartLite device.
* @param[in]   RegOffset Register offset from the base to read from.
*
* @return      Data read from the register.
*
* @note        C-style signature:
*              u32 XUartLite_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XUartLite_ReadReg(BaseAddress, RegOffset) \
	XUartLite_In32((BaseAddress) + (RegOffset))


/*****************************************************************************/
/**
*
* @brief Sets the contents of the control register.
*
* Use the XUL_CR_* constants defined above to create the bit-mask to be
* written to the register.
*
* @param[in]   BaseAddress Base address of the device.
* @param[in]   Mask The 32-bit value to write to the control register.
*
* @note        C-style Signature:
*              void XUartLite_SetControlReg(u32 BaseAddress, u32 Mask);
*
******************************************************************************/
#define XUartLite_SetControlReg(BaseAddress, Mask) \
	XUartLite_WriteReg((BaseAddress), XUL_CONTROL_REG_OFFSET, (Mask))


/*****************************************************************************/
/**
*
* @brief Returns the contents of the status register.
*
* Use the XUL_SR_* constants defined above to interpret the bit-mask
* returned.
*
* @param[in]   BaseAddress Base address of the device.
*
* @return      A 32-bit value representing the contents of the status register.
*
* @note        C-style Signature:
*              u32 XUartLite_GetStatusReg(u32 BaseAddress);
*
******************************************************************************/
#define XUartLite_GetStatusReg(BaseAddress) \
		XUartLite_ReadReg((BaseAddress), XUL_STATUS_REG_OFFSET)


/*****************************************************************************/
/**
*
* @brief Checks if the receiver has data.
*
* @param[in]   BaseAddress Base address of the device.
*
* @return      TRUE if the receiver is empty, FALSE if there is data present.
*
* @note        C-style Signature:
*              int XUartLite_IsReceiveEmpty(u32 BaseAddress);
*
******************************************************************************/
#define XUartLite_IsReceiveEmpty(BaseAddress) \
  ((XUartLite_GetStatusReg((BaseAddress)) & XUL_SR_RX_FIFO_VALID_DATA) != \
	XUL_SR_RX_FIFO_VALID_DATA)


/*****************************************************************************/
/**
*
* @brief Checks if the transmitter is full.
*
* @param[in]   BaseAddress Base address of the device.
*
* @return      TRUE if the transmitter is full, FALSE otherwise.
*
* @note        C-style Signature:
*              int XUartLite_IsTransmitFull(u32 BaseAddress);
*
******************************************************************************/
#define XUartLite_IsTransmitFull(BaseAddress) \
	((XUartLite_GetStatusReg((BaseAddress)) & XUL_SR_TX_FIFO_FULL) == \
	  XUL_SR_TX_FIFO_FULL)


/*****************************************************************************/
/**
*
* @brief Checks if the interrupt is enabled.
*
* @param[in]   BaseAddress Base address of the device.
*
* @return      TRUE if the interrupt is enabled, FALSE otherwise.
*
* @note        C-style Signature:
*              int XUartLite_IsIntrEnabled(u32 BaseAddress);
*
******************************************************************************/
#define XUartLite_IsIntrEnabled(BaseAddress) \
	((XUartLite_GetStatusReg((BaseAddress)) & XUL_SR_INTR_ENABLED) == \
	  XUL_SR_INTR_ENABLED)


/*****************************************************************************/
/**
*
* @brief Enables the device interrupt.
*
* The control register cannot be read, so this macro just writes the enable
* interrupt bit and clears all others. Since the only other ones are the
* FIFO reset bits, this works without side effects.
*
* @param[in]   BaseAddress Base address of the device.
*
* @note        C-style Signature:
*              void XUartLite_EnableIntr(u32 BaseAddress);
*
******************************************************************************/
#define XUartLite_EnableIntr(BaseAddress) \
		XUartLite_SetControlReg((BaseAddress), XUL_CR_ENABLE_INTR)


/*****************************************************************************/
/**
*
* @brief Disables the device interrupt.
*
* The control register cannot be read, so this macro just clears all bits.
* Since the only other ones are the FIFO reset bits, this works without side
* effects.
*
* @param[in]   BaseAddress Base address of the device.
*
* @note        C-style Signature:
*              void XUartLite_DisableIntr(u32 BaseAddress);
*
******************************************************************************/
#define XUartLite_DisableIntr(BaseAddress) \
		XUartLite_SetControlReg((BaseAddress), 0)

/************************** Function Prototypes *****************************/

void XUartLite_SendByte(UINTPTR BaseAddress, u8 Data);
u8 XUartLite_RecvByte(UINTPTR BaseAddress);

#ifdef __cplusplus
}
#endif

#endif


/** @} */
