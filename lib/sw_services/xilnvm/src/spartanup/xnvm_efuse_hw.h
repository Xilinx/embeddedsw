/******************************************************************************
* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
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
* 3.6   hj      05/27/25 Support XILINX_CTRL PUFHD_INVLD and DIS_SJTAG efuse bit programming
*       aa      07/24/25 Remove unused macros
*       mb      10/03/25 Rename macro names
*       mb      10/04/25 PPKs updates for SPARTANUPLUSAES1
*       mb      11/11/2025 Add support for JTAG Boot mode disable efuse programming
* 3.7   hae     02/27/2026 Support XILINX_CTRL OSPI_RESET_RECOVERY_DELAY_CTRL
*                          and ROM_RSVD_OSPI_DEV_RESET_CHOICE
*                          and ROM_OSPI_CMD_SEQ_CTRL eFuse bit programming
* </pre>
*
******************************************************************************/

#ifndef XNVM_EFUSE_HW_H
#define XNVM_EFUSE_HW_H

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
/** @} */

/** @name Write lock register
 * @{
 */
#define XNVM_EFUSE_WR_LOCK_OFFSET	0x00000000U /**< Efuse Write lock offset */

#define XNVM_EFUSE_WR_UNLOCK_VALUE	0xDF0DU /**< Efuse Write unlock value */
/** @} */

/**
 * @name  Configuration register
 * @{
 */
#define XNVM_EFUSE_CFG_OFFSET		0x00000004U /**< Efuse Configuration offset */
#define XNVM_EFUSE_CFG_MARGIN_RD_MASK	0x0000000cU /**< Efuse Margin Read mask */
#define XNVM_EFUSE_CFG_NORMAL_RD	0x00U /**< Efuse Normal Read */
#define XNVM_EFUSE_CFG_MARGIN_2_RD	0x02U /**< Efuse Margin 2 Read */
#define XNVM_EFUSE_CFG_MARGIN_RD_SHIFT	2U /**< Efuse Margin Read shift */
#define XNVM_EFUSE_CFG_PGM_EN_SHIFT	1U /**< Efuse Program Enable shift */

/** @} */

/**
 * @name  Status register
 * @{
 */
#define XNVM_EFUSE_STS_OFFSET		0x00000008U /**< Efuse Status register offset */

#define XNVM_EFUSE_STS_AES_CRC_PASS_MASK	0x00000080U /**< Efuse AES CRC Pass mask */

#define XNVM_EFUSE_STS_AES_CRC_DONE_MASK	0x00000040U /**< Efuse AES CRC Done mask */

#define XNVM_EFUSE_STS_0_TBIT_SHIFT		0U /**< Efuse 0 TBIT shift */

/** @} */

/**
 * @name  Program address register
 * @{
 */
#define XNVM_EFUSE_PGM_ADDR_OFFSET	0x0000000CU /**< Efuse Program Address offset */
/** @} */

/**
 * @name  Read address register
 * @{
 */
#define XNVM_EFUSE_RD_ADDR_OFFSET	0x00000010U /**< Efuse Read Address offset */
/** @} */

/**
 * @name  Read data register
 * @{
 */
#define XNVM_EFUSE_RD_DATA_OFFSET	0x00000014U /**< Efuse Read Data offset */
/** @} */

/**
 * @name  TPGM register
 * @{
 */
#define XNVM_EFUSE_TPGM_OFFSET		0x00000018U /**< Efuse TPGM offset */
/** @} */

/**
 * @name  TRD register
 * @{
 */
#define XNVM_EFUSE_TRD_OFFSET		0x0000001CU /**< Efuse TRD offset */
/** @} */

/**
 * @name  TSU_H_PS register
 * @{
 */
#define XNVM_EFUSE_TSU_H_PS_OFFSET	0x00000020U /**< Efuse TSU_H_PS offset */
/** @} */

/**
 * @name  TSU_H_PS_CS register
 * @{
 */
#define XNVM_EFUSE_TSU_H_PS_CS_OFFSET	0x00000024U /**< Efuse TSU_H_PS_CS offset */
/** @} */

/**
 * @name  TSU_H_CS register
 * @{
 */
#define XNVM_EFUSE_TSU_H_CS_OFFSET	0x0000002CU /**< Efuse TSU_H_CS offset */
/** @} */

/**
 * @name  ISR register
 * @{
 */
#define XNVM_EFUSE_ISR_OFFSET		0x00000030U /**< Efuse ISR offset */

#define XNVM_EFUSE_ISR_CACHE_ERR_MASK	0x00000010U /**< Efuse ISR Cache Error mask */
/** @} */

/**
 * @name  Cache load register
 * @{
 */
#define XNVM_EFUSE_CACHE_LOAD_OFFSET	0x00000044U /**< Efuse Cache Load offset */
#define XNVM_EFUSE_CACHE_LOAD_MASK	0x00000001U /**< Efuse Cache Load mask */
/** @} */

/**
 * @name  AES CRC register
 * @{
 */
#define XNVM_EFUSE_AES_CRC_OFFSET	0x0000004CU /**< Efuse AES CRC offset */
/** @} */

/**
 * @name  Tbits programming enable register
 * @{
 */
#define XNVM_EFUSE_TEST_CTRL_REG_OFFSET	0x00000100U /**< Efuse Test Control Register offset */
/** @} */

/**
 * @name  Cache offset
 * @{
 */
#define XNVM_EFUSE_XILINX_CTRL_OFFSET		0x00001000U /**< Xilinx control cache offset */
#define XNVM_EFUSE_CONTROL_OFFSET		0x00001004U /**< Control cache offset */
#define XNVM_EFUSE_DNA_OFFSET			0x0000100CU /**< DNA cache offset */
#define XNVM_EFUSE_USER_FUSE_OFFSET		0x00001020U /**< User efuse cache offset */
#define XNVM_EFUSE_PPK0_START_OFFSET		0x00001080U /**< PPK0 cache start offset */
#define XNVM_EFUSE_PPK2_START_OFFSET		0x000010C0U /**< PPK2 cache start offset */
#define XNVM_EFUSE_DEC_ONLY_OFFSET		0x00001140U /**< Decrypt only cache offset */
#define XNVM_EFUSE_AES_IV_RANGE_START_OFFSET	0x00001130U /**< AES IV range cache start offset */
#define XNVM_EFUSE_BLACK_IV_START_OFFSET	0x00001144U /**< IV cache start offset */
#define XNVM_EFUSE_CRC_EN_OFFSET		0x00001150U /**< CRC enable offset */
#define XNVM_EFUSE_SPK_REVOKE_ID_OFFSET		0x00001124U /**< SPK revoke id cache start offset */
#define XNVM_EFUSE_AES_REVOKE_ID_OFFSET		0x0000113CU /**< AES revoke id cache offset */
#define XNVM_EFUSE_XILINX_CTRL_OFFSET		0x00001000U /**< Xilinx Control cache offset */

/** @} */

/**
 * @name  BOOT_MODE_DISABLE
 * Boot mode disable Efuse bits programming related macros
 * @{
 */
#define XNVM_EFUSE_BOOT_MODE_DIS_ROW_60	(60U) /**< Boot mode disable row 60 */
#define XNVM_EFUSE_BOOT_MODE_DIS_ROW_61	(61U) /**< Boot mode disable row 61 */

#define  XNVM_EFUSE_QSPI32_BOOT_MODE_DIS_COL	(8U) /**< QSPI32 Boot mode disable bit column value: 8 */
#define  XNVM_EFUSE_OSPI_BOOT_MODE_DIS_COL	(9U) /**< OSPI Boot mode disable bit column value: 9 */
#define  XNVM_EFUSE_JTAG_BOOT_MODE_DIS_COL	(11U) /**< JTAG Boot mode disable bit column value: 11 */
#define  XNVM_EFUSE_SMAP_BOOT_MODE_DIS_COL	(12U) /**< SMAP Boot mode disable bit column value: 12 */
#define  XNVM_EFUSE_SERIAL_BOOT_MODE_DIS_COL	(13U) /**< Serial Boot mode disable bit column value: 13 */
#define  XNVM_EFUSE_QSPI24_BOOT_MODE_DIS_COL	(15U) /**< QSPI24 Boot mode disable bit column value: 15 */

#define XNVM_EFUSE_QSPI24_BOOT_MODE_DIS_MASK	(0x00000002U) /**< QSPI24 Boot mode disable Mask */
#define XNVM_EFUSE_QSPI32_BOOT_MODE_DIS_MASK	(0x00000004U) /**< QSPI32 Boot mode disable Mask */
#define XNVM_EFUSE_OSPI_BOOT_MODE_DIS_MASK	(0x00000008U) /**< OSPI Boot mode disable Mask */
#define XNVM_EFUSE_JTAG_BOOT_MODE_DIS_MASK	(0x00000020U) /**< JTAG Boot mode disable Mask */
#define XNVM_EFUSE_SMAP_BOOT_MODE_DIS_MASK	(0x00000040U) /**< SMAP Boot mode disable Mask */
#define XNVM_EFUSE_SERIAL_BOOT_MODE_DIS_MASK	(0x00000080U) /**< Serial Boot mode disable Mask */

#define XNVM_EFUSE_BOOT_MODE_DISABLE_EFUSE_BITS		(0x01U) /**< Boot mode disable efuse bits */
#define XNVM_EFUSE_BOOT_MODE_DIS_QSPI24_EFUSE_SHIFT	(0x1U) /**< QSPI24 Boot mode disable shift */
#define XNVM_EFUSE_BOOT_MODE_DIS_QSPI32_EFUSE_SHIFT	(0x2U) /**< QSPI32 Boot mode disable shift */
#define XNVM_EFUSE_BOOT_MODE_DIS_OSPI_EFUSE_SHIFT	(0x3U) /**< OSPI Boot mode disable shift */
#define XNVM_EFUSE_BOOT_MODE_DIS_JTAG_EFUSE_SHIFT	(0x5U) /**< JTAG Boot mode disable shift */
#define XNVM_EFUSE_BOOT_MODE_DIS_SMAP_EFUSE_SHIFT	(0x6U) /**< SMAP Boot mode disable shift */
#define XNVM_EFUSE_BOOT_MODE_DIS_SERIAL_EFUSE_SHIFT	(0x7U) /**< Serial Boot mode disable shift */

#define XNVM_PLM_CONFIG_BASE_ADDRESS	(0x0402B200U) /**< eFuse PLM configuration base address */
#define XNVM_RTCA_EFUSE_CLK_FREQUENCY_OFFSET	(0x0000002CU) /**< eFuse RTC clock frequency offset */
#define XNVM_EFUSE_CLK_CTRL_ADDR	(0x040A003CU) /**< eFuse clock control address */
#define XNVM_EFUSE_IO_CTRL_ADDR		(0x040A00E8U) /**< eFuse I/O control address */
#define XNVM_EFUSE_CLK_SRC_EMCCLK_VALUE	(0x1U) /**< eFuse clock source EMCCLK value */
#define XNVM_EMCCLK_MIN_FREQUENCY	(25000000U) /**< eFuse EMC clock minimum frequency */
#define XNVM_EMCCLK_MAX_FREQUENCY	(200000000U) /**< eFuse EMC clock maximum frequency */
#define XNVM_EFUSE_EMC_CLK_EN_VAL	(1U << 1U) /**< eFuse EMC clock enable value */
/** @} */

/**
 * @name  EFUSE_CACHE_SECURITY_CONTROL_REG
 * Efuse Cache Security Control bits, shifts, masks, rows and columns
 * @{
 */
#define XNVM_EFUSE_SEC_CTRL_BITS			(0x01U) /**< Secure control bits */

#define XNVM_EFUSE_SEC_CTRL_SCAN_CLEAR_EN_SHIFT		(0U) /**< SCAN_CLEAR_EN shift */
#define XNVM_EFUSE_SEC_CTRL_AES_DIS_SHIFT		(1U) /**< AES disable shift */
#define XNVM_EFUSE_SEC_CTRL_RMA_DISABLE_0_SHIFT		(6U) /**< RMA disable 0 shift */
#define XNVM_EFUSE_SEC_CTRL_RMA_ENABLE_0_SHIFT		(7U) /**< RMA enable 0 shift */
#define XNVM_EFUSE_SEC_CTRL_JTAG_DIS_SHIFT		(8U) /**< JTAG disable shift */
#define XNVM_EFUSE_SEC_CTRL_PUF_TEST2_DIS_SHIFT		(9U) /**< PUF test 2 disable shift */
#define XNVM_EFUSE_SEC_CTRL_HASH_PUF_OR_KEY_SHIFT	(10U) /**< Hash PUF or key shift */
#define XNVM_EFUSE_SEC_CTRL_PPK0_WR_LK_SHIFT		(15U) /**< PPK0 write lock shift */
#define XNVM_EFUSE_SEC_CTRL_PPK0_INVLD0_SHIFT		(16U) /**< PPK0 invalidation 0 shift */
#define XNVM_EFUSE_SEC_CTRL_AES_RD_WR_LK_0_SHIFT	(17U) /**< AES read/write lock 0 shift */
#define XNVM_EFUSE_SEC_CTRL_AES_RD_WR_LK_1_SHIFT	(18U) /**< AES read/write lock 1 shift */
#define XNVM_EFUSE_SEC_CTRL_JTAG_ERR_OUT_DIS_SHIFT	(26U) /**< JTAG error output disable shift */
#define XNVM_EFUSE_SEC_CTRL_USER_WR_LK_SHIFT		(27U) /**< User write lock shift */
#define XNVM_EFUSE_SEC_CTRL_PPK0_INVLD1_SHIFT		(31U) /**< PPK0 invalidation 1 shift */

#define XNVM_EFUSE_SEC_CTRL_SCAN_CLEAR_EN_MASK		(0x00000001U) /**< SCAN_CLEAR_EN mask */
#define XNVM_EFUSE_SEC_CTRL_AES_DIS_MASK		(0x00000002U) /**< AES disable mask */
#define XNVM_EFUSE_SEC_CTRL_RMA_DISABLE_0_MASK		(0x00000040U) /**< RMA disable 0 mask */
#define XNVM_EFUSE_SEC_CTRL_RMA_ENABLE_0_MASK		(0x00000080U) /**< RMA enable 0 mask */
#define XNVM_EFUSE_SEC_CTRL_JTAG_DIS_MASK		(0x00000100U) /**< JTAG disable mask */
#define XNVM_EFUSE_SEC_CTRL_PUF_TEST2_DIS_MASK		(0x00000200U) /**< PUF test 2 disable mask */
#define XNVM_EFUSE_SEC_CTRL_HASH_PUF_OR_KEY_MASK	(0x00000400U) /**< Hash PUF or key mask */
#define XNVM_EFUSE_SEC_CTRL_PPK0_WR_LK_MASK		(0x00008000U) /**< PPK0 write lock mask */
#define XNVM_EFUSE_SEC_CTRL_PPK0_INVLD0_MASK		(0x00010000U) /**< PPK0 invalidation 0 mask */
#define XNVM_EFUSE_SEC_CTRL_AES_RD_WR_LK_0_MASK		(0x00020000U) /**< AES read/write lock 0 mask */
#define XNVM_EFUSE_SEC_CTRL_AES_RD_WR_LK_1_MASK		(0x00040000U) /**< AES read/write lock 1 mask */
#define XNVM_EFUSE_SEC_CTRL_JTAG_ERR_OUT_DIS_MASK	(0x04000000U) /**< JTAG error output disable mask */
#define XNVM_EFUSE_SEC_CTRL_USER_WR_LK_MASK		(0x08000000U) /**< User write lock mask */
#define XNVM_EFUSE_SEC_CTRL_PPK0_INVLD1_MASK		(0x80000000U) /**< PPK0 invalidation 1 mask */

#define XNVM_EFUSE_SEC_CTRL_ROW_0			(0U) /**< Secure Control Row 0 */
#define XNVM_EFUSE_SEC_CTRL_ROW_1			(1U) /**< Secure Control Row 1 */
#define XNVM_EFUSE_SEC_CTRL_ROW_2			(2U) /**< Secure Control Row 2 */
#define XNVM_EFUSE_SEC_CTRL_ROW_3			(3U) /**< Secure Control Row 3 */
#define XNVM_EFUSE_SEC_CTRL_ROW_4			(18U) /**< Secure Control Row 4 */

#define	XNVM_EFUSE_SEC_CTRL_RMA_DISABLE_0_COL		(3U) /**< RMA disable 0 bit column value: 3 */
#define	XNVM_EFUSE_SEC_CTRL_RMA_ENABLE_0_COL		(4U) /**< RMA enable 0 bit column value: 4 */
#define XNVM_EFUSE_SEC_CTRL_SCAN_CLEAR_EN_COL		(5U) /**< Scan Clear En bit column value: 5 */
#define	XNVM_EFUSE_SEC_CTRL_HASH_PUF_OR_KEY_COL		(6U) /**< Hash PUF or PPK bit column value: 6 */
#define XNVM_EFUSE_SEC_CTRL_JTAG_ERR_OUT_DIS_COL	(14U) /**< JTAG error out disable bit column value: 14 */
#define XNVM_EFUSE_SEC_CTRL_USER_WR_LK_COL		(15U) /**< User write lock bit column value: 15 */
#define XNVM_EFUSE_SEC_CTRL_PUF_TEST2_DIS_COL		(24U) /**< PUF test2 disable bit column value: 24 */
#define XNVM_EFUSE_SEC_CTRL_PPK0_INVLD_0_COL		(25U) /**< PPK0 invalid 0 bit column value: 25 */
#define XNVM_EFUSE_SEC_CTRL_PPK0_INVLD_1_COL		(26U) /**< PPK0 invalid 1 bit column value: 26 */
#define XNVM_EFUSE_SEC_CTRL_AES_RD_WR_LK_0_COL		(24U) /**< AES read/write lock bit 0 column value: 24 */
#define XNVM_EFUSE_SEC_CTRL_AES_RD_WR_LK_1_COL		(25U) /**< AES read/write lock bit 1 column value: 25 */
#define XNVM_EFUSE_SEC_CTRL_PPK0_WR_LK_COL		(26U) /**< PPK0 write lock bit column value: 26 */
#define XNVM_EFUSE_SEC_CTRL_JTAG_DIS_COL		(29U) /**< AES JTAG disable bit column value: 29 */
#define XNVM_EFUSE_SEC_CTRL_AES_DIS_COL			(30U) /**< AES disable bit column value: 30 */

/** @} */

/**
 * @name  EFUSE_CRC_EN_REG
 * Efuse Cache Crc Enabled Register bits, shifts, masks and columns
 * @{
 */
#define XNVM_EFUSE_SEC_CTRL_DFT_BITS			(2U) /**< DFT bits */

#define XNVM_EFUSE_CRC_EN_SHIFT				(0U) /**< CRC enable shift */
#define XNVM_EFUSE_DFT_DIS_SHIFT			(1U) /**< DFT disable shift */
#define XNVM_EFUSE_LCKDOWN_SHIFT			(3U) /**< Lockdown shift */
#define XNVM_EFUSE_RMA_DISABLE_1_SHIFT			(4U) /**< RMA disable 1 shift */
#define XNVM_EFUSE_RMA_ENABLE_1_SHIFT			(5U) /**< RMA enable 1 shift */

#define XNVM_EFUSE_SEC_CTRL_CRC_EN_MASK			(0X00000001U) /**< CRC enable mask */
#define XNVM_EFUSE_SEC_CTRL_DFT_DIS_0_MASK		(0X00000002U) /**< DFT disable 0 mask */
#define XNVM_EFUSE_SEC_CTRL_DFT_DIS_1_MASK		(0X00000004U) /**< DFT disable 1 mask */
#define XNVM_EFUSE_SEC_CTRL_LCKDOWN_MASK		(0X00000008U) /**< Lockdown mask */
#define XNVM_EFUSE_SEC_CTRL_RMA_DISABLE_1_MASK		(0X00000010U) /**< RMA disable 1 mask */
#define XNVM_EFUSE_SEC_CTRL_RMA_ENABLE_1_MASK		(0X00000020U) /**< RMA enable 1 mask */

#define	XNVM_EFUSE_SEC_CTRL_CRC_EN_COL			(24U) /**< Efuse CRC enable bit column value: 24 */
#define	XNVM_EFUSE_SEC_CTRL_DFT_DISABLE_0_COL		(25U) /**< DFT disable 0 bit column value: 25 */
#define	XNVM_EFUSE_SEC_CTRL_DFT_DISABLE_1_COL		(26U) /**< DFT disable 1 bit column value: 26 */
#define	XNVM_EFUSE_SEC_CTRL_LCKDOWN_COL			(27U) /**< Lockdown enable bit column value: 27 */
#define	XNVM_EFUSE_SEC_CTRL_RMA_DISABLE_1_COL		(28U) /**< RMA disable 1 bit column value: 28 */
#define	XNVM_EFUSE_SEC_CTRL_RMA_ENABLE_1_COL		(29U) /**< RMA enable 1 bit column value: 29 */

/** @} */

#ifndef SPARTANUPLUSAES1
#define XNVM_EFUSE_PPK1_START_OFFSET			(0x000010A0U) /**< PPK1 cache start offset */
#define XNVM_EFUSE_SEC_CTRL_PPK1_WR_LK_SHIFT		(13U) /**< PPK1 write lock shift */
#define XNVM_EFUSE_SEC_CTRL_PPK1_INVLD0_SHIFT		(14U) /**< PPK1 invalidation 0 shift */
#define XNVM_EFUSE_SEC_CTRL_PPK1_INVLD1_SHIFT		(30U) /**< PPK1 invalidation 1 shift */
#define XNVM_EFUSE_SEC_CTRL_PPK1_WR_LK_MASK		(0x00002000U) /**< PPK1 write lock mask */
#define XNVM_EFUSE_SEC_CTRL_PPK1_INVLD0_MASK		(0x00004000U) /**< PPK1 invalidation 0 mask */
#define XNVM_EFUSE_SEC_CTRL_PPK1_INVLD1_MASK		(0x40000000U) /**< PPK1 invalidation 1 mask */
#define XNVM_EFUSE_SEC_CTRL_PPK2_WR_LK_SHIFT		(11U) /**< PPK2 write lock shift */
#define XNVM_EFUSE_SEC_CTRL_PPK2_INVLD0_SHIFT		(12U) /**< PPK2 invalidation 0 shift */
#define XNVM_EFUSE_SEC_CTRL_PPK2_INVLD1_SHIFT		(29U) /**< PPK2 invalidation 1 shift */
#define XNVM_EFUSE_SEC_CTRL_PPK2_WR_LK_MASK		(0x00000800U) /**< PPK2 write lock mask */
#define XNVM_EFUSE_SEC_CTRL_PPK2_INVLD0_MASK		(0x00001000U) /**< PPK2 invalidation 0 mask */
#define XNVM_EFUSE_SEC_CTRL_PPK2_INVLD1_MASK		(0x20000000U) /**< PPK2 invalidation 1 mask */
#define XNVM_EFUSE_SEC_CTRL_PPK1_INVLD_0_COL		(27U) /**< PPK1 invalid 0 bit column value: 27 */
#define XNVM_EFUSE_SEC_CTRL_PPK1_INVLD_1_COL		(28U) /**< PPK1 invalid 1 bit column value: 28 */
#define XNVM_EFUSE_SEC_CTRL_PPK1_WR_LK_COL		(27U) /**< PPK1 write lock bit column value: 27 */
#define XNVM_EFUSE_SEC_CTRL_PPK2_INVLD_0_COL		(29U) /**< PPK2 invalid 0 bit column value: 29 */
#define XNVM_EFUSE_SEC_CTRL_PPK2_INVLD_1_COL		(30U) /**< PPK2 invalid 1 bit column value: 30 */
#define XNVM_EFUSE_SEC_CTRL_PPK2_WR_LK_COL		(28U) /**< PPK2 write lock bit column value: 28 */

#else
#define XNVM_EFUSE_PPK1_START_OFFSET			(0x000010B0U) /**< PPK1 cache start offset */
#define XNVM_EFUSE_SEC_CTRL_PPK1_WR_LK_SHIFT		(11U) /**< PPK1 write lock shift */
#define XNVM_EFUSE_SEC_CTRL_PPK1_INVLD0_SHIFT		(12U) /**< PPK1 invalidation 0 shift */
#define XNVM_EFUSE_SEC_CTRL_PPK1_INVLD1_SHIFT		(29U) /**< PPK1 invalidation 1 shift */
#define XNVM_EFUSE_SEC_CTRL_PPK1_WR_LK_MASK		(0x00000800U) /**< PPK1 write lock mask */
#define XNVM_EFUSE_SEC_CTRL_PPK1_INVLD0_MASK		(0x00001000U) /**< PPK1 invalidation 0 mask */
#define XNVM_EFUSE_SEC_CTRL_PPK1_INVLD1_MASK		(0x20000000U) /**< PPK1 invalidation 1 mask */
#define XNVM_EFUSE_SEC_CTRL_PPK1_INVLD_0_COL		(29U) /**< PPK1 invalid 0 bit column value: 29 */
#define XNVM_EFUSE_SEC_CTRL_PPK1_INVLD_1_COL		(30U) /**< PPK1 invalid 1 bit column value: 30 */
#define XNVM_EFUSE_SEC_CTRL_PPK1_WR_LK_COL		(28U) /**< PPK1 write lock bit column value: 28 */
#define XNVM_EFUSE_BOOT_MODE_DIS_OFFSET			(0x0000104CU) /**< Boot mode disable cache offset */
#endif

/**
 * @name  XILINX_CTRL register
 * Xilinx control register bits, shifts and masks
 * @{
 */
#define XNVM_EFUSE_PUFHD_INVLD_START_COL	        (13U) /**< PUF_HD_INVLD start column */
#define XNVM_EFUSE_PUFHD_INVLD_END_COL		        (14U) /**< PUF_HD_INVLD end column */
#define XNVM_EFUSE_PUFHD_INVLD_NUM_OF_ROWS	        (1U) /**< PUF_HD_INVLD number of rows */
#define XNVM_EFUSE_DIS_SJTAG_COL		        (12U) /**< DIS_SJTAG column */
#define XNVM_EFUSE_OSPI_RESET_RECOVERY_DELAY_CTRL_COL	(11U) /**< OSPI Reset Recovery Delay Control column */
#define XNVM_EFUSE_ROM_RSVD_OSPI_DEV_RESET_CHOICE_COL	(10U) /**< ROM RSVD OSPI Device Reset Choice column */
#define XNVM_EFUSE_ROM_OSPI_CMD_SEQ_CTRL_START_COL	(8U) /**< ROM OSPI Cmd Seq Ctrl start column */
#define XNVM_EFUSE_ROM_OSPI_CMD_SEQ_CTRL_END_COL	(9U) /**< ROM OSPI Cmd Seq Ctrl end column */
#define XNVM_EFUSE_ROM_OSPI_CMD_SEQ_CTRL_NUM_OF_ROWS	(1U) /**< ROM OSPI Cmd Seq Ctrl number of rows */

#define XNVM_EFUSE_PUFHD_INVLD_EFUSE_SHIFT              (0x13U) /**< Puf Invalid shift */
#define XNVM_EFUSE_DIS_SJTAG_EFUSE_SHIFT		(0x12U) /**< Disable Secure JTAG shift */
#define XNVM_EFUSE_OSPI_RESET_RECOVERY_DELAY_CTRL_SHIFT	(0xBU) /**< OSPI Reset Recovery Delay Control shift */
#define XNVM_EFUSE_ROM_RSVD_OSPI_DEV_RESET_CHOICE_SHIFT	(0xAU) /**< ROM RSVD OSPI Device Reset Choice shift */
#define XNVM_EFUSE_ROM_OSPI_CMD_SEQ_CTRL_SHIFT		(0x8U) /**< ROM OSPI Cmd Seq Ctrl shift */

#define XNVM_EFUSE_PUFHD_INVLD_EFUSE_MASK               (0x00006000U) /**< Puf Invalid mask */
#define XNVM_EFUSE_DIS_SJTAG_EFUSE_MASK	                (0x00001000U) /**< Disable Secure JTAG mask */
#define XNVM_EFUSE_OSPI_RESET_RECOVERY_DELAY_CTRL_MASK	(0x00000800U) /**< OSPI Reset Recovery Delay Control mask */
#define XNVM_EFUSE_ROM_RSVD_OSPI_DEV_RESET_CHOICE_MASK	(0x00000400U) /**< ROM RSVD OSPI Device Reset Choice mask */
#define XNVM_EFUSE_ROM_OSPI_CMD_SEQ_CTRL_MASK		(0x00000300U) /**< ROM OSPI Cmd Seq Ctrl mask */

#define XNVM_EFUSE_PUFHD_INVLD_EFUSE_BITS	        (0x02U) /**< Puf Invalid bits */
#define XNVM_EFUSE_DIS_SJTAG_EFUSE_BITS		        (0x01U) /**< Disable Secure JTAG bits */
#define XNVM_EFUSE_OSPI_RESET_RECOVERY_DELAY_CTRL_BITS	(0x01U) /**< OSPI Reset Recovery Delay Control bits */
#define XNVM_EFUSE_ROM_RSVD_OSPI_DEV_RESET_CHOICE_BITS	(0x01U) /**< ROM RSVD OSPI Device Reset Choice bits */
#define XNVM_EFUSE_ROM_OSPI_CMD_SEQ_CTRL_BITS		(0x02U) /**< ROM OSPI Cmd Seq Ctrl bits */

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* XNVM_EFUSE_HW_H */
