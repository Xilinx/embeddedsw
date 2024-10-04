/*******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2023-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
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
#define I2C_MCDP6000_ADDR  0x14 /**< I2C MCDP6000 Address*/
#define I2C_IDT8N49_ADDR   0x7C /**< I2C IDT 8N49N241 Address */
#define I2C_IDT8N49_ADDR_1 0x6C /**< I2C IDT 8N49N241 Address onboard vck190 */
#define I2C_MUX_ADDR       0x75 /**< I2C Mux Address */
#define I2C_VFMCEXP_1_ADDR 0x65 /**< I2C VideoFMC IO Expander 1 address */
#define I2C_VFMCEXP_0_ADDR 0x64 /**< I2C VideoFMC IO Expander 0 address */
#define I2C_LMK03318_ADDR  0x51 /**< I2C TI LMK03318 Address */
#define I2C_MUX_ADDR_SI    0x74 /**< I2C Mux Address */
#define I2C_ADDR_SI5328    0x69 /**< Si 5328 ADDR */
#define I2C_SI5344_ADDR    0x68 /**< I2C SI5344 Address */
#define I2C_MUX_ADDR2      0x74  /**< I2C Mux Address */
#define IIC_SI570_ADDRESS  0x5D

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif

#endif /* VIDEOFMC_H */
