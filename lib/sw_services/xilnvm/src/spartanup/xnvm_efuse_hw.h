/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xnvm_efuse_hw.h
* This header file contains identifiers and register-level driver functions (or
* macros) that can be used to access the spartan ultrascale plus eFuse controller.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 1.0   kpt     08/23/24 First release
*
* </pre>
*
******************************************************************************/

#ifndef __XNVM_EFUSE_HW_H__
#define __XNVM_EFUSE_HW_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/************************** Constant Definitions *****************************/

/** @name eFuse PS Base Address
 * @{
 */
#define XNVM_EFUSE_CTRL_BASEADDR	0x04160000U /**< Efuse base address */
/*@}*/

/** @name Write lock register
 * @{
 */
/**< Write lock register offsets and definitions */
#define XNVM_EFUSE_WR_LOCK_OFFSET	0x00000000U

#define XNVM_EFUSE_WR_UNLOCK_VALUE	0xDF0D
/** @} */

/**
 * @name  Configuration register
 * @{
 */
/**< Configuration register offsets and definitions */
#define XNVM_EFUSE_CFG_OFFSET		0x00000004U
#define XNVM_EFUSE_CFG_MARGIN_RD_MASK	0x0000000cU
#define XNVM_EFUSE_CFG_NORMAL_RD	0x00U
#define XNVM_EFUSE_CFG_MARGIN_2_RD	0x02U
#define XNVM_EFUSE_CFG_MARGIN_RD_SHIFT	2U
#define XNVM_EFUSE_CFG_PGM_EN_SHIFT	1U

/** @} */

/**
 * @name  Status register
 * @{
 */
/**< Status register offsets and definitions */
#define XNVM_EFUSE_STS_OFFSET		0x00000008U

#define XNVM_EFUSE_STS_AES_CRC_PASS_MASK	0x00000080U

#define XNVM_EFUSE_STS_AES_CRC_DONE_MASK	0x00000040U

#define XNVM_EFUSE_STS_CACHE_DONE_DEFVAL	0x0U

#define XNVM_EFUSE_STS_0_TBIT_SHIFT		0U

/** @} */

/**
 * @name  Program address register
 * @{
 */
/**< program address register offsets and definitions */
#define XNVM_EFUSE_PGM_ADDR_OFFSET	0x0000000CU
/** @} */

/**
 * @name  Read address register
 * @{
 */
/**< Read address register offsets and definitions */
#define XNVM_EFUSE_RD_ADDR_OFFSET	0x00000010U
/** @} */

/**
 * @name  Read data register
 * @{
 */
/**< Read data register offsets and definitions */
#define XNVM_EFUSE_RD_DATA_OFFSET	0x00000014U
/** @} */

/**
 * @name  TPGM register
 * @{
 */
/**< TPGM register offsets and definitions */
#define XNVM_EFUSE_TPGM_OFFSET		0x00000018U
/** @} */

/**
 * @name  TRD register
 * @{
 */
/**< TRD register offsets and definitions */
#define XNVM_EFUSE_TRD_OFFSET		0x0000001CU
/** @} */

/**
 * @name  TSU_H_PS register
 * @{
 */
/**< TSU_H_PS register offsets and definitions */
#define XNVM_EFUSE_TSU_H_PS_OFFSET	0x00000020U
/** @} */

/**
 * @name  TSU_H_PS_CS register
 * @{
 */
/**< TSU_H_PS_CS register offsets and definitions */
#define XNVM_EFUSE_TSU_H_PS_CS_OFFSET	0x00000024U
/** @} */

/**
 * @name  TSU_H_CS register
 * @{
 */
/**< TSU_H_CS register offsets and definitions */
#define XNVM_EFUSE_TSU_H_CS_OFFSET	0x0000002CU
/** @} */

/**
 * @name  ISR register
 * @{
 */
/**< ISR register offsets and definitions */
#define XNVM_EFUSE_ISR_OFFSET		0x00000030U

#define XNVM_EFUSE_ISR_CACHE_ERR_MASK	0x00000010U
/** @} */

/**
 * @name  Cache load register
 * @{
 */
/**< Cache load register offsets and definitions */
#define XNVM_EFUSE_CACHE_LOAD_OFFSET	0x00000044U
#define XNVM_EFUSE_CACHE_LOAD_MASK	0x00000001U
/** @} */

/**
 * @name  AES CRC register
 * @{
 */
/**< AES CRC register offsets and definitions */
#define XNVM_EFUSE_AES_CRC_OFFSET	0x0000004CU
/** @} */

/**
 * @name  Tbits programming enable register
 * @{
 */
/**< Tbits programming enable register offsets and definitions */
#define XNVM_EFUSE_TEST_CTRL_REG_OFFSET	0x00000100U
/** @} */

/**
 * @name  Cache offset
 * @{
 */
#define XNVM_EFUSE_XILINX_CTRL_OFFSET       0x00001000U /**< Xilinx control cache offset */
#define XNVM_EFUSE_CONTROL_OFFSET           0x00001004U /**< Control cache offset */
#define XNVM_EFUSE_DNA_OFFSET               0x0000100CU /**< DNA cache offset */
#define XNVM_EFUSE_USER_FUSE_OFFSET         0x00001020U /**< User efuse cache offset */
#define XNVM_EFUSE_PPK0_START_OFFSET        0x00001080U /**< PPK0 cache start offset */
#define XNVM_EFUSE_PPK0_END_OFFSET          0x0000109CU /**< PPK0 cache end offset */
#define XNVM_EFUSE_PPK1_START_OFFSET        0x000010A0U /**< PPK1 cache start offset */
#define XNVM_EFUSE_PPK1_END_OFFSET          0x000010BCU /**< PPK1 cache end offset */
#define XNVM_EFUSE_PPK2_START_OFFSET        0x000010C0U /**< PPK2 cache start offset */
#define XNVM_EFUSE_PPK2_END_OFFSET          0x000010DCU /**< PPK2 cache end offset */
#define XNVM_EFUSE_IV_RANGE_START_OFFSET    0x00001130U /**< IV range cache start offset */
#define XNVM_EFUSE_DEC_ONLY_OFFSET          0x00001140U /**< Decrypt only cache offset */
#define XNVM_EFUSE_IV_START_OFFSET          0x00001144U /**< IV cache start offset */
#define XNVM_EFUSE_IV_END_OFFSET            0x0000114CU /**< IV cache end offset */
#define XNVM_EFUSE_CRC_EN_OFFSET            0x00001150U /**< CRC enable offset */
#define XNVM_EFUSE_SPK_REVOKE_ID_OFFSET     0x00001124U /**< SPK revoke id cache start offset */
#define XNVM_EFUSE_SPK_REVOKE_ID_END_OFFSET 0x0000112CU /**< SPK revoke id cache end offset */
#define XNVM_EFUSE_AES_REVOKE_ID_OFFSET     0x0000113CU /**< AES revoke id cache offset */
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __XNVM_EFUSE_HW_H__ */
