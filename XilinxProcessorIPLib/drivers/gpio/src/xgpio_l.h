/******************************************************************************
* Copyright (C) 2002 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XGPIO_L_H
#define XGPIO_L_H

/*****************************************************************************/
/**
*
* @file xgpio_l.h
* @brief Provides low-level identifiers and driver functions for GPIO access.
*
* This header file contains identifiers and driver functions (or
* macros) that can be used to access the device. Refer to the hardware
* device specification for more details of the device operation.
*
* The macros that are available in this file use a multiply to calculate the
* addresses of registers. You can control whether that multiply is done at
* run time or at compile time. A constant passed as the channel parameter
* causes the multiply to be done at compile time. A variable passed as the
* channel parameter causes it to occur at run time.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a jhl  04/24/02 First release of low level driver
* 2.00a jhl  11/26/03 Added support for dual channels and interrupts. This
*                     change required the functions to be changed such that
*                     the interface is not compatible with previous versions.
*                     See the examples in the example directory for macros
*                     to help compile an application that was designed for
*                     previous versions of the driver. The interrupt registers
*                     are accessible using the ReadReg and WriteReg macros and
*                     a channel parameter was added to the other macros.
* 2.11a mta  03/21/07 Updated to new coding style
* 2.12a sv   11/21/07 Updated driver to support access through DCR bus.
* 3.00a sv   11/21/09 Renamed the macros XGpio_mWriteReg to XGpio_WriteReg
*		      XGpio_mReadReg to XGpio_ReadReg.
*		      Removed the macros XGpio_mSetDataDirection,
*		      XGpio_mGetDataReg and XGpio_mSetDataReg. Users
*		      should use XGpio_WriteReg/XGpio_ReadReg to achieve the
*		      same functionality.
* 4.1  lks   11/18/15 Removed support for DCR bridge
* 4.14  vn   09/10/26 Align Doxygen comments with AMD standards
* </pre>
*
******************************************************************************/

/** @addtogroup gpio_api GPIO APIs
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

/** @name Registers
 *
 * Register offsets for this device.
 * @{
 */
#define XGPIO_DATA_OFFSET	0x0   /**< Data register for first channel. */
/** I/O direction register for first channel. */
#define XGPIO_TRI_OFFSET	0x4
/** Data register for second channel. */
#define XGPIO_DATA2_OFFSET	0x8
/** I/O direction register for second channel. */
#define XGPIO_TRI2_OFFSET	0xC

#define XGPIO_GIE_OFFSET	0x11C /**< Global interrupt enable register. */
#define XGPIO_ISR_OFFSET	0x120 /**< Interrupt status register. */
#define XGPIO_IER_OFFSET	0x128 /**< Interrupt enable register. */

/** @} */

/**
 * @brief Channel offset.
 *
 * The following constant describes the offset of each channel's data and
 * tristate register from the base address.
 */
#define XGPIO_CHAN_OFFSET  8

/** @name Interrupt Status and Enable Register bitmaps and masks
 *
 * Bit definitions for the interrupt status register and interrupt enable
 * registers.
 * @{
 */
#define XGPIO_IR_MASK		0x3 /**< Mask of all bits. */
#define XGPIO_IR_CH1_MASK	0x1 /**< Mask for the first channel. */
#define XGPIO_IR_CH2_MASK	0x2 /**< Mask for the second channel. */
/** @} */


/** @name Global Interrupt Enable Register bitmaps and masks
 *
 * Bit definitions for the Global Interrupt Enable register.
 * @{
 */
/** Mask for Global Interrupt. */
#define XGPIO_GIE_GINTR_ENABLE_MASK	0x80000000
/** @} */



/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

#define XGpio_In32  Xil_In32	/**< Input operations. */
#define XGpio_Out32 Xil_Out32	/**< Output operations. */


/*****************************************************************************/
/**
*
* @brief Writes a 32-bit value to a GPIO register.
*
* If the GPIO core is implemented in a smaller width, only the least
* significant data is written.
*
* @param[in]   BaseAddress Base address of the GPIO device.
* @param[in]   RegOffset Register offset from the base to write to.
* @param[in]   Data Data written to the register.
*
* @note        C-style signature:
*              void XGpio_WriteReg(u32 BaseAddress, u32 RegOffset,
*                                  u32 Data)
*
******************************************************************************/
#define XGpio_WriteReg(BaseAddress, RegOffset, Data) \
	XGpio_Out32((BaseAddress) + (RegOffset), (u32)(Data))

/*****************************************************************************/
/**
*
* @brief Reads a value from a GPIO register.
*
* A 32-bit read is performed. If the GPIO core is implemented in a smaller
* width, only the least significant data is read from the register. The
* most significant data reads as 0.
*
* @param[in]   BaseAddress Base address of the GPIO device.
* @param[in]   RegOffset Register offset from the base to read from.
*
* @return      Data read from the register.
*
* @note        C-style signature:
*              u32 XGpio_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XGpio_ReadReg(BaseAddress, RegOffset) \
	XGpio_In32((BaseAddress) + (RegOffset))

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* end of protection macro */
