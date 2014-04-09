/******************************************************************************
*
* (c) Copyright 2008-2013 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
******************************************************************************/
/***************************************************************************/
/**
*
* @file xtft_hw.h
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
