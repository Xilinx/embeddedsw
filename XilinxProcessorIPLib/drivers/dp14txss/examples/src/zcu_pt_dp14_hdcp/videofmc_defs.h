/*******************************************************************************
 *
 * Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
 *
*******************************************************************************/
/*****************************************************************************/
/**
*
* @file videofmc_defs.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* X.XX  XX     YY/MM/DD
* 1.00  RHe    16/06/20 Initial release.
* </pre>
*
******************************************************************************/

#ifndef VIDEOFMC_DEFS_H
#define VIDEOFMC_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/
#define I2C_TI_DP141_ADDR   0x05 /**< I2C TI DP141 Address */
#define I2C_MCDP6000_ADDR  0x14 /**< I2C MCDP6000 Address*/
#define I2C_IDT8N49_ADDR   0x7C /**< I2C IDT 8N49N241 Address */
#define I2C_MUX_ADDR       0x75 /**< I2C Mux Address */
#define I2C_CLK_ADDR       0x68 /**< I2C Clk Address */
#define I2C_VFMCEXP_1_ADDR 0x65 /**< I2C VideoFMC IO Expander 1 address */
#define I2C_VFMCEXP_0_ADDR 0x64 /**< I2C VideoFMC IO Expander 0 address */
#define I2C_LMK03318_ADDR  0x50 /**< I2C TI LMK03318 Address */
#define I2C_MUX_ADDR_SI    0x74 /**< I2C Mux Address */
#define I2C_ADDR_SI5328    0x69 /**< Si 5328 ADDR */


/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif

#endif /* VIDEOFMC_H */
