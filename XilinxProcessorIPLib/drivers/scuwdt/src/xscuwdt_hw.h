/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XSCUWDT_HW_H
#define XSCUWDT_HW_H

/****************************************************************************/
/**
*
* @file xscuwdt_hw.h
* @brief Defines the hardware interface of the SCU watchdog timer driver.
*
* This file contains the hardware interface to the Xilinx SCU private Watch Dog
* Timer (XSCUWDT).
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- ---------------------------------------------
* 1.00a sdm 01/15/10 First release
* 1.01a bss 02/27/12 Updated the register offsets to start at 0x0 instead
*                    of 0x20 as the base address obtained from the tools
*		     starts at 0x20.
* 1.02a  sg 07/17/12 Included xil_assert.h for CR 667947. This is an issue
*		     when the xstatus.h in the common driver overwrites
*		     the xstatus.h of the standalone BSP during the
*		     libgen.
* 2.1 	sk  02/26/15 Modified the code for MISRA-C:2012 compliance.
* 2.8   vn  07/30/26 Fix Doxygen warnings
* 2.8   vn  09/30/26 Align Doxygen comments with AMD standards
* </pre>
*
******************************************************************************/

/** @addtogroup Overview
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_io.h"
#include "xil_assert.h"
/************************** Constant Definitions *****************************/

/** @name Register Map
 * Offsets of registers from the start of the device. The WDT registers start at
 * an offset 0x20.
 * @{
 */

#define XSCUWDT_LOAD_OFFSET	0x00U /**< Watchdog Load Register. */
#define XSCUWDT_COUNTER_OFFSET	0x04U /**< Watchdog Counter Register. */
#define XSCUWDT_CONTROL_OFFSET	0x08U /**< Watchdog Control Register. */
/** Watchdog Interrupt Status Register. */
#define XSCUWDT_ISR_OFFSET	0x0CU
#define XSCUWDT_RST_STS_OFFSET	0x10U /**< Watchdog Reset Status Register. */
#define XSCUWDT_DISABLE_OFFSET	0x14U /**< Watchdog Disable Register. */
/** @} */

/** @name Watchdog Control register
 * The bits of this register control the prescaler, WD/Timer mode, interrupt
 * enable, auto-reload, and watchdog enable.
 * @{
 */

#define XSCUWDT_CONTROL_PRESCALER_MASK	 0x0000FF00U /**< Prescaler. */
#define XSCUWDT_CONTROL_PRESCALER_SHIFT	 8U /**< Prescaler shift. */
/** Watchdog/Timer mode. */
#define XSCUWDT_CONTROL_WD_MODE_MASK	 0x00000008U
/** Interrupt enable (in timer mode). */
#define XSCUWDT_CONTROL_IT_ENABLE_MASK	 0x00000004U
/** Auto-reload (in timer mode). */
#define XSCUWDT_CONTROL_AUTO_RELOAD_MASK 0x00000002U
#define XSCUWDT_CONTROL_WD_ENABLE_MASK	 0x00000001U /**< Watchdog enable. */
/** @} */

/** @name Interrupt Status register
 * This register indicates the Counter register has reached zero in Counter
 * mode.
 * @{
 */

#define XSCUWDT_ISR_EVENT_FLAG_MASK	0x00000001U /**< Event flag. */
/** @} */

/** @name Reset Status register
 * This register indicates the Counter register has reached zero in Watchdog
 * mode and a reset request is sent.
 * @{
 */

/** Time out occurred. */
#define XSCUWDT_RST_STS_RESET_FLAG_MASK	0x00000001U
/** @} */

/** @name Disable register
 * Writes to this register switch from watchdog mode to timer mode.
 * The software must write 0x12345678 and 0x87654321 successively to the
 * Watchdog Disable Register so that the watchdog mode bit in the Watchdog
 * Control Register is set to zero.
 * @{
 */
/** Watchdog mode disable value 1. */
#define XSCUWDT_DISABLE_VALUE1		0x12345678U
/** Watchdog mode disable value 2. */
#define XSCUWDT_DISABLE_VALUE2		0x87654321U
/** @} */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/****************************************************************************/
/**
*
* @brief Reads the given register.
*
* @param[in]	BaseAddr Base address of the device.
* @param[in]	RegOffset Register offset to be read.
*
* @return	The 32-bit value of the register.
*
* @note		C-style signature:
*		u32 XScuWdt_ReadReg(u32 BaseAddr, u32 RegOffset)
*
*****************************************************************************/
#define XScuWdt_ReadReg(BaseAddr, RegOffset)		\
	Xil_In32((BaseAddr) + ((u32)RegOffset))

/****************************************************************************/
/**
*
* @brief Writes the given register.
*
* @param[in]	BaseAddr Base address of the device.
* @param[in]	RegOffset Register offset to be written.
* @param[in]	Data 32-bit value to write to the register.
*
* @note		C-style signature:
*		void XScuWdt_WriteReg(u32 BaseAddr, u32 RegOffset, u32 Data)
*
*****************************************************************************/
#define XScuWdt_WriteReg(BaseAddr, RegOffset, Data)	\
	Xil_Out32((BaseAddr) + ((u32)RegOffset), ((u32)Data))

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

/** @} */
#endif	/* end of protection macro */
