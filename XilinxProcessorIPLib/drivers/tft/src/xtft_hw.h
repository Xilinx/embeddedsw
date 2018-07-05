/******************************************************************************
* Copyright (C) 2008 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/***************************************************************************/
/**
*
* @file xtft_hw.h
* @addtogroup tft_v6_2
* @{
* @details
*
* This file defines the macros and definitions for the Xilinx TFT Controller
* device.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver    Who   Date      Changes
* -----  ----  --------  -----------------------------------------------
* 1.00a  sg    03/24/08  First release
* 2.00a	 ktn   07/06/09	 Added Interrupt Enable and Status Register Offset and,
*			 bit masks.
* 3.00a  ktn   10/22/09  Updated driver to use the HAL APIs/macros.
*		         Removed the macros XTft_mSetPixel and XTft_mGetPixel.
* 6.0    sd   07/09/15  Added XTFT_AR_LSB_OFFSET and XTFT_AR_MSB_OFFSET
*			 definitions to the xtft_hw.h file, these offsets
*			 are valid only when the Address Width is greater
*			 than 32 bits.
* </pre>
*
****************************************************************************/
#ifndef XTFT_HW_H  /* prevent circular inclusions */
#define XTFT_HW_H  /* by using protection macros  */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *******************************/
#include "xil_types.h"
#include "xil_assert.h"
#include "xil_io.h"

/************************** Constant Definitions ***************************/

/**
 * @name TFT Register offsets
 *
 * The following definitions provide access to each of the registers of the
 * TFT device.
 * @{
 */
#define XTFT_AR_OFFSET		0 /**< Address Reg (Video memory) Offset */
#define XTFT_CR_OFFSET		4 /**< Control Register Offset */
#define XTFT_IESR_OFFSET	8 /**< Interrupt Enable and Status Reg Offset */

#define XTFT_AR_LSB_OFFSET	0x10 /**< Address Reg LSB (Video memory) Offset */
#define XTFT_AR_MSB_OFFSET	0x14 /**< Address Reg MSB (Video memory) Offset */

/*@}*/

/**
 * @name TFT Control Register (CR) mask(s)
 * @{
 */
#define XTFT_CR_TDE_MASK	0x01 /**< TFT Display Enable Bit Mask */
#define XTFT_CR_DPS_MASK	0x02 /**< TFT Display Scan Control Bit Mask */

/*@}*/

/**
 * @name TFT Interrupt Enable and Status Register (IESR) mask(s)
 * @{
 */
#define XTFT_IESR_VADDRLATCH_STATUS_MASK 0x01 /**< TFT Video Address Latch
							Status Bit Mask */
#define XTFT_IESR_IE_MASK		0x08 /**< TFT Interrupt Enable Mask */

/*@}*/

/**
 * Dimension Definitions
 */
#define XTFT_CHAR_WIDTH			8    /**< Character width */
#define XTFT_CHAR_HEIGHT		12   /**< Character height */
#define XTFT_DISPLAY_WIDTH		640  /**< Width of the screen */
#define XTFT_DISPLAY_HEIGHT		480  /**< Height of the screen */
#define XTFT_DISPLAY_BUFFER_WIDTH	1024 /**< Buffer width of a line */

/*
 * This shift is used for accessing the TFT registers through DCR bus.
 */
#define XTFT_DCR_REG_SHIFT		2    /**< Reg Shift for DCR Access */

/**************************** Type Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *******************/

/************************** Function Prototypes ****************************/

/************************** Variable Definitions ***************************/

/************************** Function Definitions ****************************/

#ifdef __cplusplus
}
#endif

#endif /* XTFT_HW_H */
/** @} */
