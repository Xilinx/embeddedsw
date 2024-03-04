/******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2023-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file videofmc_defs.h
* @addtogroup VideoFMC
* @{
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver  Who      Date      Changes
* ---- ---      --------  --------------------------------------------------.
* 1.00  ND      18/10/22  Common DP 2.1 tx only application for zcu102 and
* 						  vcu118
* 1.01	ND		26/02/24  Added support for 13.5 and 20G
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
#define I2C_MUX_ADDR       0x75 /**< I2C Mux Address */
#define I2C_VFMCEXP_1_ADDR 0x65 /**< I2C VideoFMC IO Expander 1 address */
#define I2C_VFMCEXP_0_ADDR 0x64 /**< I2C VideoFMC IO Expander 0 address */
#define I2C_LMK03318_ADDR  0x51 /**< I2C TI LMK03318 Address */
#define I2C_MUX_ADDR2      0x74  /**< I2C Mux Address */
#define I2C_SI5344_ADDR    0x68 /**< I2C SI5344 Address */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif

#endif /* VIDEOFMC_H */
/** @} */
