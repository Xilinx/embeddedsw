/*******************************************************************************
* Copyright (C) 2015 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xhdmiphy1_hw.h
 *
 * This header file contains the identifiers and low-level driver functions (or
 * macros) that can be used to access the device. High-level driver functions
 * are defined in xhdmiphy1.h.
 *
 * @note    None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 *            dd/mm/yy
 * ----- ---- -------- -----------------------------------------------
 * 1.0   gm   10/12/18 Initial release.
 * </pre>
 *
 * @addtogroup xhdmiphy1 Overview
 * @{
*******************************************************************************/

#ifndef XHDMIPHY1_HW_H_
/* Prevent circular inclusions by using protection macros. */
#define XHDMIPHY1_HW_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files **********************************/

#include "xil_io.h"
#include "xil_types.h"

/************************** Constant Definitions ******************************/

/******************************************************************************/
/**
 * Address mapping for the Video PHY core.
 *
*******************************************************************************/
/** @name HDMIPHY core registers: General registers.
  * @{
  */
#define XHDMIPHY1_VERSION_REG            0x000  /**< Version register */
#define XHDMIPHY1_BANK_SELECT_REG        0x00C  /**< Bank select register */
#define XHDMIPHY1_REF_CLK_SEL_REG        0x010  /**< Reference clock select register */
#define XHDMIPHY1_PLL_RESET_REG          0x014  /**< PLL reset register */
#define XHDMIPHY1_COMMON_INIT_REG        0x014  /**< Common initialization register */
#define XHDMIPHY1_PLL_LOCK_STATUS_REG    0x018  /**< PLL lock status register */
#define XHDMIPHY1_TX_INIT_REG            0x01C  /**< Transmitter initialization register */
#define XHDMIPHY1_TX_INIT_STATUS_REG     0x020  /**< Transmitter initialization status register */
#define XHDMIPHY1_RX_INIT_REG            0x024  /**< Receiver initialization register */
#define XHDMIPHY1_RX_INIT_STATUS_REG     0x028  /**< Receiver initialization status register */
#define XHDMIPHY1_IBUFDS_GTXX_CTRL_REG   0x02C  /**< IBUFDS GT control register */
#define XHDMIPHY1_POWERDOWN_CONTROL_REG  0x030  /**< Power down control register */
#define XHDMIPHY1_LOOPBACK_CONTROL_REG   0x038  /**< Loopback control register */
/* @} */

/** @name HDMIPHY core registers: Dynamic reconfiguration port (DRP) registers.
  * @{
  */
/** DRP control register for channel 1 (0x040) */
#define XHDMIPHY1_DRP_CONTROL_CH1_REG    0x040
/** DRP control register for channel 2 (0x044) */
#define XHDMIPHY1_DRP_CONTROL_CH2_REG    0x044
/** DRP control register for channel 3 (0x048) */
#define XHDMIPHY1_DRP_CONTROL_CH3_REG    0x048
/** DRP control register for channel 4 (0x04C) */
#define XHDMIPHY1_DRP_CONTROL_CH4_REG    0x04C
/** DRP status register for channel 1 (0x050) */
#define XHDMIPHY1_DRP_STATUS_CH1_REG     0x050
/** DRP status register for channel 2 (0x054) */
#define XHDMIPHY1_DRP_STATUS_CH2_REG     0x054
/** DRP status register for channel 3 (0x058) */
#define XHDMIPHY1_DRP_STATUS_CH3_REG     0x058
/** DRP status register for channel 4 (0x05C) */
#define XHDMIPHY1_DRP_STATUS_CH4_REG     0x05C
/** DRP control register for common (0x060) */
#define XHDMIPHY1_DRP_CONTROL_COMMON_REG 0x060
/** DRP status register for common (0x064) */
#define XHDMIPHY1_DRP_STATUS_COMMON_REG  0x064
/** DRP control register for TX MMCM (0x124) */
#define XHDMIPHY1_DRP_CONTROL_TXMMCM_REG 0x124
/** DRP status register for TX MMCM (0x128) */
#define XHDMIPHY1_DRP_STATUS_TXMMCM_REG  0x128
/** DRP control register for RX MMCM (0x144) */
#define XHDMIPHY1_DRP_CONTROL_RXMMCM_REG 0x144
/** DRP status register for RX MMCM (0x148) */
#define XHDMIPHY1_DRP_STATUS_RXMMCM_REG  0x148
/* @} */

/** @name HDMIPHY core registers: CPLL Calibration registers.
  * @{
  */
/** CPLL calibration period register (0x068) */
#define XHDMIPHY1_CPLL_CAL_PERIOD_REG    0x068
/** CPLL calibration tolerance register (0x06C) */
#define XHDMIPHY1_CPLL_CAL_TOL_REG       0x06C
/* @} */
/** @name HDMIPHY core registers: GT Debug INTF registers.
  * @{
  */
/** GT debug GPI register (0x068) */
#define XHDMIPHY1_GT_DBG_GPI_REG         0x068
/** GT debug GPO register (0x06C) */
#define XHDMIPHY1_GT_DBG_GPO_REG         0x06C
/* @} */

/** @name HDMIPHY core registers: Transmitter function registers.
  * @{
  */
/** TX control register (0x070) */
#define XHDMIPHY1_TX_CONTROL_REG         0x070
/** TX buffer bypass register (0x074) */
#define XHDMIPHY1_TX_BUFFER_BYPASS_REG   0x074
/** TX status register (0x078) */
#define XHDMIPHY1_TX_STATUS_REG          0x078
/** TX driver register for channels 1 and 2 (0x07C) */
#define XHDMIPHY1_TX_DRIVER_CH12_REG     0x07C
/** TX driver register for channels 3 and 4 (0x080) */
#define XHDMIPHY1_TX_DRIVER_CH34_REG     0x080
/** TX driver extended register (0x084) */
#define XHDMIPHY1_TX_DRIVER_EXT_REG      0x084
/** TX rate register for channels 1 and 2 (0x08C) */
#define XHDMIPHY1_TX_RATE_CH12_REG       0x08C
/** TX rate register for channels 3 and 4 (0x090) */
#define XHDMIPHY1_TX_RATE_CH34_REG       0x090
/* @} */

/** @name HDMIPHY core registers: Receiver function registers.
  * @{
  */
/** RX rate register for channels 1 and 2 (0x98) */
#define XHDMIPHY1_RX_RATE_CH12_REG       0x98
/** RX rate register for channels 3 and 4 (0x9C) */
#define XHDMIPHY1_RX_RATE_CH34_REG       0x9C
/** RX control register (0x100) */
#define XHDMIPHY1_RX_CONTROL_REG         0x100
/** RX status register (0x104) */
#define XHDMIPHY1_RX_STATUS_REG          0x104
/** RX equalization and CDR register (0x108) */
#define XHDMIPHY1_RX_EQ_CDR_REG          0x108
/** RX TDLOCK register (0x10C) */
#define XHDMIPHY1_RX_TDLOCK_REG          0x10C
/* @} */

/** @name HDMIPHY core registers: Interrupt registers.
  * @{
  */
/** Interrupt enable register (0x110) */
#define XHDMIPHY1_INTR_EN_REG            0x110
/** Interrupt disable register (0x114) */
#define XHDMIPHY1_INTR_DIS_REG           0x114
/** Interrupt mask register (0x118) */
#define XHDMIPHY1_INTR_MASK_REG          0x118
/** Interrupt status register (0x11C) */
#define XHDMIPHY1_INTR_STS_REG           0x11C
/* @} */

/** @name User clocking registers: MMCM and BUFGGT registers.
  * @{
  */
/** MMCM TX user clock control register (0x0120) */
#define XHDMIPHY1_MMCM_TXUSRCLK_CTRL_REG 0x0120
/** MMCM TX user clock register 1 (0x0124) */
#define XHDMIPHY1_MMCM_TXUSRCLK_REG1     0x0124
/** MMCM TX user clock register 2 (0x0128) */
#define XHDMIPHY1_MMCM_TXUSRCLK_REG2     0x0128
/** MMCM TX user clock register 3 (0x012C) */
#define XHDMIPHY1_MMCM_TXUSRCLK_REG3     0x012C
/** MMCM TX user clock register 4 (0x0130) */
#define XHDMIPHY1_MMCM_TXUSRCLK_REG4     0x0130
/** BUFGGT TX user clock register (0x0134) */
#define XHDMIPHY1_BUFGGT_TXUSRCLK_REG    0x0134
/** Miscellaneous TX user clock register (0x0138) */
#define XHDMIPHY1_MISC_TXUSRCLK_REG      0x0138

/** MMCM RX user clock control register (0x0140) */
#define XHDMIPHY1_MMCM_RXUSRCLK_CTRL_REG 0x0140
/** MMCM RX user clock register 1 (0x0144) */
#define XHDMIPHY1_MMCM_RXUSRCLK_REG1     0x0144
/** MMCM RX user clock register 2 (0x0148) */
#define XHDMIPHY1_MMCM_RXUSRCLK_REG2     0x0148
/** MMCM RX user clock register 3 (0x014C) */
#define XHDMIPHY1_MMCM_RXUSRCLK_REG3     0x014C
/** MMCM RX user clock register 4 (0x0150) */
#define XHDMIPHY1_MMCM_RXUSRCLK_REG4     0x0150
/** BUFGGT RX user clock register (0x0154) */
#define XHDMIPHY1_BUFGGT_RXUSRCLK_REG    0x0154
/** Miscellaneous RX user clock register (0x0158) */
#define XHDMIPHY1_MISC_RXUSRCLK_REG      0x0158
/* @} */

/** @name Clock detector (HDMI) registers.
  * @{
  */
/** Clock detector control register (0x0200) */
#define XHDMIPHY1_CLKDET_CTRL_REG        0x0200
/** Clock detector status register (0x0204) */
#define XHDMIPHY1_CLKDET_STAT_REG        0x0204
/** Clock detector frequency timer timeout register (0x0208) */
#define XHDMIPHY1_CLKDET_FREQ_TMR_TO_REG 0x0208
/** Clock detector TX frequency register (0x020C) */
#define XHDMIPHY1_CLKDET_FREQ_TX_REG     0x020C
/** Clock detector RX frequency register (0x0210) */
#define XHDMIPHY1_CLKDET_FREQ_RX_REG     0x0210
/** Clock detector TX timer register (0x0214) */
#define XHDMIPHY1_CLKDET_TMR_TX_REG      0x0214
/** Clock detector RX timer register (0x0218) */
#define XHDMIPHY1_CLKDET_TMR_RX_REG      0x0218
/** Clock detector DRU frequency register (0x021C) */
#define XHDMIPHY1_CLKDET_FREQ_DRU_REG    0x021C
/** Clock detector TX FRL frequency register (0x0230) */
#define XHDMIPHY1_CLKDET_FREQ_TX_FRL_REG 0x0230
/** Clock detector RX FRL frequency register (0x0234) */
#define XHDMIPHY1_CLKDET_FREQ_RX_FRL_REG 0x0234

/* @} */

/** @name Data recovery unit registers (HDMI).
  * @{
  */
/** DRU control register (0x0300) */
#define XHDMIPHY1_DRU_CTRL_REG           0x0300
/** DRU status register (0x0304) */
#define XHDMIPHY1_DRU_STAT_REG           0x0304

/**
 * @brief DRU Center Frequency Lower register address calculation.
 * @param Ch Channel number (1-4).
 * @return Register address for the specified channel.
 */
#define XHDMIPHY1_DRU_CFREQ_L_REG(Ch)    (0x0308 + (12 * (Ch - 1)))

/**
 * @brief DRU Center Frequency Higher register address calculation.
 * @param Ch Channel number (1-4).
 * @return Register address for the specified channel.
 */
#define XHDMIPHY1_DRU_CFREQ_H_REG(Ch)    (0x030C + (12 * (Ch - 1)))

/**
 * @brief DRU Gain register address calculation.
 * @param Ch Channel number (1-4).
 * @return Register address for the specified channel.
 */
#define XHDMIPHY1_DRU_GAIN_REG(Ch)       (0x0310 + (12 * (Ch - 1)))
/* @} */

/** @name TMDS Clock Pattern Generator registers (HDMI).
  * @{
  */
/** PATGEN control register (0x0340) */
#define XHDMIPHY1_PATGEN_CTRL_REG        0x0340
/* @} */

/******************************************************************************/

/** @name HDMIPHY core masks, shifts, and register values.
  * @{
  */
/* 0x0F8: VERSION */
#define XHDMIPHY1_VERSION_INTER_REV_MASK \
                0x000000FF  /**< Internal revision. */
#define XHDMIPHY1_VERSION_CORE_PATCH_MASK \
                0x00000F00  /**< Core patch details. */
#define XHDMIPHY1_VERSION_CORE_PATCH_SHIFT 8 /**< Shift bits for core patch
                            details. */
#define XHDMIPHY1_VERSION_CORE_VER_REV_MASK \
                0x0000F000  /**< Core version revision. */
#define XHDMIPHY1_VERSION_CORE_VER_REV_SHIFT 12 /**< Shift bits for core
                            version revision. */
#define XHDMIPHY1_VERSION_CORE_VER_MNR_MASK \
                0x00FF0000  /**< Core minor version. */
#define XHDMIPHY1_VERSION_CORE_VER_MNR_SHIFT 16  /**< Shift bits for core minor
                            version. */
#define XHDMIPHY1_VERSION_CORE_VER_MJR_MASK \
                0xFF000000  /**< Core major version. */
#define XHDMIPHY1_VERSION_CORE_VER_MJR_SHIFT 24  /**< Shift bits for core major
                            version. */
/* 0x00C: BANK_SELECT_REG */
/** TX bank select mask (0x00F) */
#define XHDMIPHY1_BANK_SELECT_TX_MASK    0x00F
/** RX bank select mask (0xF00) */
#define XHDMIPHY1_BANK_SELECT_RX_MASK    0xF00
/** RX bank select shift (8) */
#define XHDMIPHY1_BANK_SELECT_RX_SHIFT   8
/* 0x010: REF_CLK_SEL */
/** QPLL0 reference clock select mask (0x0000000F) */
#define XHDMIPHY1_REF_CLK_SEL_QPLL0_MASK 0x0000000F
/** CPLL reference clock select mask (0x000000F0) */
#define XHDMIPHY1_REF_CLK_SEL_CPLL_MASK  0x000000F0
/** CPLL reference clock select shift (4) */
#define XHDMIPHY1_REF_CLK_SEL_CPLL_SHIFT 4
/** QPLL1 reference clock select mask (0x00000F00) */
#define XHDMIPHY1_REF_CLK_SEL_QPLL1_MASK 0x00000F00
/** QPLL1 reference clock select shift (8) */
#define XHDMIPHY1_REF_CLK_SEL_QPLL1_SHIFT    8
/** GT reference clock 0 selection value (1) */
#define XHDMIPHY1_REF_CLK_SEL_XPLL_GTREFCLK0 1
/** GT reference clock 1 selection value (2) */
#define XHDMIPHY1_REF_CLK_SEL_XPLL_GTREFCLK1 2
/** GT north reference clock 0 selection value (3) */
#define XHDMIPHY1_REF_CLK_SEL_XPLL_GTNORTHREFCLK0 3
/** GT north reference clock 1 selection value (4) */
#define XHDMIPHY1_REF_CLK_SEL_XPLL_GTNORTHREFCLK1 4
/** GT south reference clock 0 selection value (5) */
#define XHDMIPHY1_REF_CLK_SEL_XPLL_GTSOUTHREFCLK0 5
/** GT south reference clock 1 selection value (6) */
#define XHDMIPHY1_REF_CLK_SEL_XPLL_GTSOUTHREFCLK1 6
/** GT east reference clock 0 selection value (3) */
#define XHDMIPHY1_REF_CLK_SEL_XPLL_GTEASTREFCLK0 3
/** GT east reference clock 1 selection value (4) */
#define XHDMIPHY1_REF_CLK_SEL_XPLL_GTEASTREFCLK1 4
/** GT west reference clock 0 selection value (5) */
#define XHDMIPHY1_REF_CLK_SEL_XPLL_GTWESTREFCLK0 5
/** GT west reference clock 1 selection value (6) */
#define XHDMIPHY1_REF_CLK_SEL_XPLL_GTWESTREFCLK1 6
/** GT global reference clock selection value (7) */
#define XHDMIPHY1_REF_CLK_SEL_XPLL_GTGREFCLK 7
/** System clock select mask (0x0F000000) */
#define XHDMIPHY1_REF_CLK_SEL_SYSCLKSEL_MASK 0x0F000000
/** System clock select shift (24) */
#define XHDMIPHY1_REF_CLK_SEL_SYSCLKSEL_SHIFT 24
/** System clock data PLL0 selection value (0) */
#define XHDMIPHY1_REF_CLK_SEL_XXSYSCLKSEL_DATA_PLL0 0
/** System clock data PLL1 selection value (1) */
#define XHDMIPHY1_REF_CLK_SEL_XXSYSCLKSEL_DATA_PLL1 1
/** System clock data CPLL selection value (0) */
#define XHDMIPHY1_REF_CLK_SEL_XXSYSCLKSEL_DATA_CPLL 0
/** System clock data QPLL selection value (1) */
#define XHDMIPHY1_REF_CLK_SEL_XXSYSCLKSEL_DATA_QPLL 1
/** System clock data QPLL0 selection value (3) */
#define XHDMIPHY1_REF_CLK_SEL_XXSYSCLKSEL_DATA_QPLL0 3
/** System clock data QPLL1 selection value (2) */
#define XHDMIPHY1_REF_CLK_SEL_XXSYSCLKSEL_DATA_QPLL1 2
/** System clock output channel selection value (0) */
#define XHDMIPHY1_REF_CLK_SEL_XXSYSCLKSEL_OUT_CH 0
/** System clock output common selection value (1) */
#define XHDMIPHY1_REF_CLK_SEL_XXSYSCLKSEL_OUT_CMN 1
/** System clock output common 0 selection value (2) */
#define XHDMIPHY1_REF_CLK_SEL_XXSYSCLKSEL_OUT_CMN0 2
/** System clock output common 1 selection value (3) */
#define XHDMIPHY1_REF_CLK_SEL_XXSYSCLKSEL_OUT_CMN1 3
/**
 * @brief RX System Clock Select Output mask based on GT type.
 * @param G GT type (GTHE4, GTYE4, GTYE5, GTYP).
 * @return Mask value for the specified GT type.
 */
#define XHDMIPHY1_REF_CLK_SEL_RXSYSCLKSEL_OUT_MASK(G) \
    ((((G) == XHDMIPHY1_GT_TYPE_GTHE4) || \
        ((G) == XHDMIPHY1_GT_TYPE_GTYE4)) ? 0x03000000 : 0x02000000)

/**
 * @brief TX System Clock Select Output mask based on GT type.
 * @param G GT type (GTHE4, GTYE4, GTYE5, GTYP).
 * @return Mask value for the specified GT type.
 */
#define XHDMIPHY1_REF_CLK_SEL_TXSYSCLKSEL_OUT_MASK(G) \
    ((((G) == XHDMIPHY1_GT_TYPE_GTHE4) || \
        ((G) == XHDMIPHY1_GT_TYPE_GTYE4)) ? 0x0C000000 : 0x08000000)

/**
 * @brief RX System Clock Select Data mask based on GT type.
 * @param G GT type (GTHE4, GTYE4, GTYE5, GTYP).
 * @return Mask value for the specified GT type.
 */
#define XHDMIPHY1_REF_CLK_SEL_RXSYSCLKSEL_DATA_MASK(G) \
    ((((G) == XHDMIPHY1_GT_TYPE_GTHE4) || \
        ((G) == XHDMIPHY1_GT_TYPE_GTYE4)) ? 0x30000000 : 0x01000000)
/**
 * @brief TX System Clock Select Data mask based on GT type.
 * @param G GT type (GTHE4, GTYE4, GTYE5, GTYP).
 * @return Mask value for the specified GT type.
 */
#define XHDMIPHY1_REF_CLK_SEL_TXSYSCLKSEL_DATA_MASK(G) \
    ((((G) == XHDMIPHY1_GT_TYPE_GTHE4) || \
        ((G) == XHDMIPHY1_GT_TYPE_GTYE4)) ? 0xC0000000 : 0x04000000)

/**
 * @brief RX System Clock Select Output shift based on GT type.
 * @param G GT type (GTHE4, GTYE4, GTYE5, GTYP).
 * @return Shift value for the specified GT type.
 */
#define XHDMIPHY1_REF_CLK_SEL_RXSYSCLKSEL_OUT_SHIFT(G) \
    ((((G) == XHDMIPHY1_GT_TYPE_GTHE4) || \
        ((G) == XHDMIPHY1_GT_TYPE_GTYE4)) ? 24 : 25)

/**
 * @brief TX System Clock Select Output shift based on GT type.
 * @param G GT type (GTHE4, GTYE4, GTYE5, GTYP).
 * @return Shift value for the specified GT type.
 */
#define XHDMIPHY1_REF_CLK_SEL_TXSYSCLKSEL_OUT_SHIFT(G) \
    ((((G) == XHDMIPHY1_GT_TYPE_GTHE4) || \
        ((G) == XHDMIPHY1_GT_TYPE_GTYE4)) ? 26 : 27)

/**
 * @brief RX System Clock Select Data shift based on GT type.
 * @param G GT type (GTHE4, GTYE4, GTYE5, GTYP).
 * @return Shift value for the specified GT type.
 */
#define XHDMIPHY1_REF_CLK_SEL_RXSYSCLKSEL_DATA_SHIFT(G) \
    ((((G) == XHDMIPHY1_GT_TYPE_GTHE4) || \
        ((G) == XHDMIPHY1_GT_TYPE_GTYE4)) ? 28 : 24)
/**
 * @brief TX System Clock Select Data shift based on GT type.
 * @param G GT type (GTHE4, GTYE4, GTYE5, GTYP).
 * @return Shift value for the specified GT type.
 */
#define XHDMIPHY1_REF_CLK_SEL_TXSYSCLKSEL_DATA_SHIFT(G) \
    ((((G) == XHDMIPHY1_GT_TYPE_GTHE4) || \
        ((G) == XHDMIPHY1_GT_TYPE_GTYE4)) ? 30 : 26)
/* 0x014: PLL_RESET */
/** CPLL reset mask (0x1) */
#define XHDMIPHY1_PLL_RESET_CPLL_MASK    0x1
/** QPLL0 reset mask (0x2) */
#define XHDMIPHY1_PLL_RESET_QPLL0_MASK   0x2
/** QPLL1 reset mask (0x4) */
#define XHDMIPHY1_PLL_RESET_QPLL1_MASK   0x4
/* 0x014: COMMON INIT for Versal Only */
/** GT wizard reset all mask (0x1) */
#define XHDMIPHY1_GTWIZ_RESET_ALL_MASK   0x1
/** PCIe reset all channels mask (0x2) */
#define XHDMIPHY1_PCIERST_ALL_CH_MASK    0x2
/* 0x018: PLL_LOCK_STATUS */
/**
 * @brief CPLL lock status mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return Lock status mask for the specified channel.
 */
#define XHDMIPHY1_PLL_LOCK_STATUS_CPLL_MASK(Ch) \
        (0x01 << (Ch - 1))
/** QPLL0 lock status mask (0x10) */
#define XHDMIPHY1_PLL_LOCK_STATUS_QPLL0_MASK 0x10
/** QPLL1 lock status mask (0x20) */
#define XHDMIPHY1_PLL_LOCK_STATUS_QPLL1_MASK 0x20
/** CPLL lock status mask for all channels */
#define XHDMIPHY1_PLL_LOCK_STATUS_CPLL_ALL_MASK \
        (XHDMIPHY1_PLL_LOCK_STATUS_CPLL_MASK(XHDMIPHY1_CHANNEL_ID_CH1) | \
         XHDMIPHY1_PLL_LOCK_STATUS_CPLL_MASK(XHDMIPHY1_CHANNEL_ID_CH2) | \
         XHDMIPHY1_PLL_LOCK_STATUS_CPLL_MASK(XHDMIPHY1_CHANNEL_ID_CH3) | \
         XHDMIPHY1_PLL_LOCK_STATUS_CPLL_MASK(XHDMIPHY1_CHANNEL_ID_CH4))
/** CPLL lock status mask for HDMI channels (1-3) */
#define XHDMIPHY1_PLL_LOCK_STATUS_CPLL_HDMI_MASK \
        (XHDMIPHY1_PLL_LOCK_STATUS_CPLL_MASK(XHDMIPHY1_CHANNEL_ID_CH1) | \
         XHDMIPHY1_PLL_LOCK_STATUS_CPLL_MASK(XHDMIPHY1_CHANNEL_ID_CH2) | \
         XHDMIPHY1_PLL_LOCK_STATUS_CPLL_MASK(XHDMIPHY1_CHANNEL_ID_CH3))
/** RPLL lock status mask (0xC0) */
#define XHDMIPHY1_PLL_LOCK_STATUS_RPLL_MASK 0xC0
/** LCPLL lock status mask (0x300) */
#define XHDMIPHY1_PLL_LOCK_STATUS_LCPLL_MASK 0x300
/* 0x01C, 0x024: TX_INIT, RX_INIT */
/**
 * @brief GT reset mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return GT reset mask for the specified channel.
 */
#define XHDMIPHY1_TXRX_INIT_GTRESET_MASK(Ch) \
        (0x01 << (8 * (Ch - 1)))

/**
 * @brief PMA reset mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return PMA reset mask for the specified channel.
 */
#define XHDMIPHY1_TXRX_INIT_PMARESET_MASK(Ch) \
        (0x02 << (8 * (Ch - 1)))

/**
 * @brief PCS reset mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return PCS reset mask for the specified channel.
 */
#define XHDMIPHY1_TXRX_INIT_PCSRESET_MASK(Ch) \
        (0x04 << (8 * (Ch - 1)))

/**
 * @brief TX user ready mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return TX user ready mask for the specified channel.
 */
#define XHDMIPHY1_TX_INIT_USERRDY_MASK(Ch) \
        (0x08 << (8 * (Ch - 1)))

/**
 * @brief Link ready sideband mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return Link ready sideband mask for the specified channel.
 */
#define XHDMIPHY1_TXRX_LNKRDY_SB_MASK(Ch) \
        (0x10 << (8 * (Ch - 1)))
/**
 * @brief Master reset mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return Master reset mask for the specified channel.
 */
#define XHDMIPHY1_TXRX_MSTRESET_MASK(Ch) \
        (0x20 << (8 * (Ch - 1)))
/**
 * @brief RX user ready mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return RX user ready mask for the specified channel.
 */
#define XHDMIPHY1_RX_INIT_USERRDY_MASK(Ch) \
        (0x40 << (8 * (Ch - 1)))

/**
 * @brief PLL GT reset mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return PLL GT reset mask for the specified channel.
 */
#define XHDMIPHY1_TXRX_INIT_PLLGTRESET_MASK(Ch) \
        (0x80 << (8 * (Ch - 1)))
/** GT reset mask for all channels */
#define XHDMIPHY1_TXRX_INIT_GTRESET_ALL_MASK \
        (XHDMIPHY1_TXRX_INIT_GTRESET_MASK(XHDMIPHY1_CHANNEL_ID_CH1) | \
         XHDMIPHY1_TXRX_INIT_GTRESET_MASK(XHDMIPHY1_CHANNEL_ID_CH2) | \
         XHDMIPHY1_TXRX_INIT_GTRESET_MASK(XHDMIPHY1_CHANNEL_ID_CH3) | \
         XHDMIPHY1_TXRX_INIT_GTRESET_MASK(XHDMIPHY1_CHANNEL_ID_CH4))
/** Link ready sideband mask for all channels */
#define XHDMIPHY1_TXRX_LNKRDY_SB_ALL_MASK \
        (XHDMIPHY1_TXRX_LNKRDY_SB_MASK(XHDMIPHY1_CHANNEL_ID_CH1) | \
         XHDMIPHY1_TXRX_LNKRDY_SB_MASK(XHDMIPHY1_CHANNEL_ID_CH2) | \
         XHDMIPHY1_TXRX_LNKRDY_SB_MASK(XHDMIPHY1_CHANNEL_ID_CH3) | \
         XHDMIPHY1_TXRX_LNKRDY_SB_MASK(XHDMIPHY1_CHANNEL_ID_CH4))
/** Master reset mask for all channels */
#define XHDMIPHY1_TXRX_MSTRESET_ALL_MASK \
        (XHDMIPHY1_TXRX_MSTRESET_MASK(XHDMIPHY1_CHANNEL_ID_CH1) | \
         XHDMIPHY1_TXRX_MSTRESET_MASK(XHDMIPHY1_CHANNEL_ID_CH2) | \
         XHDMIPHY1_TXRX_MSTRESET_MASK(XHDMIPHY1_CHANNEL_ID_CH3) | \
         XHDMIPHY1_TXRX_MSTRESET_MASK(XHDMIPHY1_CHANNEL_ID_CH4))
/** TX User Ready initialization mask for all channels */
#define XHDMIPHY1_TX_INIT_USERRDY_ALL_MASK \
        (XHDMIPHY1_TX_INIT_USERRDY_MASK(XHDMIPHY1_CHANNEL_ID_CH1) | \
         XHDMIPHY1_TX_INIT_USERRDY_MASK(XHDMIPHY1_CHANNEL_ID_CH2) | \
         XHDMIPHY1_TX_INIT_USERRDY_MASK(XHDMIPHY1_CHANNEL_ID_CH3) | \
         XHDMIPHY1_TX_INIT_USERRDY_MASK(XHDMIPHY1_CHANNEL_ID_CH4))
/** RX User Ready initialization mask for all channels */
#define XHDMIPHY1_RX_INIT_USERRDY_ALL_MASK \
        (XHDMIPHY1_RX_INIT_USERRDY_MASK(XHDMIPHY1_CHANNEL_ID_CH1) | \
         XHDMIPHY1_RX_INIT_USERRDY_MASK(XHDMIPHY1_CHANNEL_ID_CH2) | \
         XHDMIPHY1_RX_INIT_USERRDY_MASK(XHDMIPHY1_CHANNEL_ID_CH3) | \
         XHDMIPHY1_RX_INIT_USERRDY_MASK(XHDMIPHY1_CHANNEL_ID_CH4))
/** TX/RX PLL GT reset initialization mask for all channels */
#define XHDMIPHY1_TXRX_INIT_PLLGTRESET_ALL_MASK \
        (XHDMIPHY1_TXRX_INIT_PLLGTRESET_MASK(XHDMIPHY1_CHANNEL_ID_CH1) | \
         XHDMIPHY1_TXRX_INIT_PLLGTRESET_MASK(XHDMIPHY1_CHANNEL_ID_CH2) | \
         XHDMIPHY1_TXRX_INIT_PLLGTRESET_MASK(XHDMIPHY1_CHANNEL_ID_CH3) | \
         XHDMIPHY1_TXRX_INIT_PLLGTRESET_MASK(XHDMIPHY1_CHANNEL_ID_CH4))
/* 0x020, 0x028: TX_STATUS, RX_STATUS */
/**
 * @brief TX/RX initialization status reset done mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return Reset done status mask for the specified channel.
 */
#define XHDMIPHY1_TXRX_INIT_STATUS_RESETDONE_MASK(Ch) \
        (0x01 << (8 * (Ch - 1)))

/**
 * @brief TX/RX initialization status PMA reset done mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return PMA reset done status mask for the specified channel.
 */
#define XHDMIPHY1_TXRX_INIT_STATUS_PMARESETDONE_MASK(Ch) \
        (0x02 << (8 * (Ch - 1)))

/**
 * @brief TX/RX initialization status power good mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return Power good status mask for the specified channel.
 */
#define XHDMIPHY1_TXRX_INIT_STATUS_POWERGOOD_MASK(Ch) \
        (0x04 << (8 * (Ch - 1)))
/** TX/RX initialization status reset done mask for all channels */
#define XHDMIPHY1_TXRX_INIT_STATUS_RESETDONE_ALL_MASK \
    (XHDMIPHY1_TXRX_INIT_STATUS_RESETDONE_MASK(XHDMIPHY1_CHANNEL_ID_CH1) | \
     XHDMIPHY1_TXRX_INIT_STATUS_RESETDONE_MASK(XHDMIPHY1_CHANNEL_ID_CH2) | \
     XHDMIPHY1_TXRX_INIT_STATUS_RESETDONE_MASK(XHDMIPHY1_CHANNEL_ID_CH3) | \
     XHDMIPHY1_TXRX_INIT_STATUS_RESETDONE_MASK(XHDMIPHY1_CHANNEL_ID_CH4))
/** TX/RX initialization status PMA reset done mask for all channels */
#define XHDMIPHY1_TXRX_INIT_STATUS_PMARESETDONE_ALL_MASK \
    (XHDMIPHY1_TXRX_INIT_STATUS_PMARESETDONE_MASK(XHDMIPHY1_CHANNEL_ID_CH1) | \
     XHDMIPHY1_TXRX_INIT_STATUS_PMARESETDONE_MASK(XHDMIPHY1_CHANNEL_ID_CH2) | \
     XHDMIPHY1_TXRX_INIT_STATUS_PMARESETDONE_MASK(XHDMIPHY1_CHANNEL_ID_CH3) | \
     XHDMIPHY1_TXRX_INIT_STATUS_PMARESETDONE_MASK(XHDMIPHY1_CHANNEL_ID_CH4))
/* 0x02C: IBUFDS_GTXX_CTRL */
/** IBUFDS GT reference clock 0 CEB (Clock Enable Bar) control mask */
#define XHDMIPHY1_IBUFDS_GTXX_CTRL_GTREFCLK0_CEB_MASK    0x1
/** IBUFDS GT reference clock 1 CEB (Clock Enable Bar) control mask */
#define XHDMIPHY1_IBUFDS_GTXX_CTRL_GTREFCLK1_CEB_MASK    0x2
/* 0x030: POWERDOWN_CONTROL */
/**
 * @brief CPLL power down mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return CPLL power down mask for the specified channel.
 */
#define XHDMIPHY1_POWERDOWN_CONTROL_CPLLPD_MASK(Ch) \
        (0x01 << (8 * (Ch - 1)))

/**
 * @brief QPLL0 power down mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return QPLL0 power down mask for the specified channel.
 */
#define XHDMIPHY1_POWERDOWN_CONTROL_QPLL0PD_MASK(Ch) \
        (0x02 << (8 * (Ch - 1)))

/**
 * @brief QPLL1 power down mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return QPLL1 power down mask for the specified channel.
 */
#define XHDMIPHY1_POWERDOWN_CONTROL_QPLL1PD_MASK(Ch) \
        (0x04 << (8 * (Ch - 1)))

/**
 * @brief RX power down mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return RX power down mask for the specified channel.
 */
#define XHDMIPHY1_POWERDOWN_CONTROL_RXPD_MASK(Ch) \
        (0x18 << (8 * (Ch - 1)))

/**
 * @brief RX power down shift for specific channel.
 * @param Ch Channel number (1-4).
 * @return RX power down shift for the specified channel.
 */
#define XHDMIPHY1_POWERDOWN_CONTROL_RXPD_SHIFT(Ch) \
        (3 + (8 * (Ch - 1)))

/**
 * @brief TX power down mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return TX power down mask for the specified channel.
 */
#define XHDMIPHY1_POWERDOWN_CONTROL_TXPD_MASK(Ch) \
        (0x60 << (8 * (Ch - 1)))

/**
 * @brief TX power down shift for specific channel.
 * @param Ch Channel number (1-4).
 * @return TX power down shift for the specified channel.
 */
#define XHDMIPHY1_POWERDOWN_CONTROL_TXPD_SHIFT(Ch) \
        (5 + (8 * (Ch - 1)))

/* 0x038: LOOPBACK_CONTROL */
/**
 * @brief Loopback control channel mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return Loopback control mask for the specified channel.
 */
#define XHDMIPHY1_LOOPBACK_CONTROL_CH_MASK(Ch) \
        (0x03 << (8 * (Ch - 1)))

/**
 * @brief Loopback control channel shift for specific channel.
 * @param Ch Channel number (1-4).
 * @return Loopback control shift for the specified channel.
 */
#define XHDMIPHY1_LOOPBACK_CONTROL_CH_SHIFT(Ch) \
        (8 * (Ch - 1))
/* 0x040, 0x044, 0x048, 0x04C, 0x060: DRP_CONTROL_CH[1-4], DRP_CONTROL_COMMON*/
/** DRP control address mask */
#define XHDMIPHY1_DRP_CONTROL_DRPADDR_MASK   0x00000FFF
/** DRP control enable mask */
#define XHDMIPHY1_DRP_CONTROL_DRPEN_MASK 0x00001000
/** DRP control write enable mask */
#define XHDMIPHY1_DRP_CONTROL_DRPWE_MASK 0x00002000
/** DRP control reset mask */
#define XHDMIPHY1_DRP_CONTROL_DRPRESET_MASK  0x00004000
/** DRP control data input mask */
#define XHDMIPHY1_DRP_CONTROL_DRPDI_MASK 0xFFFF0000
/** DRP control data input shift */
#define XHDMIPHY1_DRP_CONTROL_DRPDI_SHIFT    16
/* 0x050, 0x054, 0x058, 0x05C, 0x064: DRP_STATUS_CH[1-4], DRP_STATUS_COMMON */
/** DRP status data output mask */
#define XHDMIPHY1_DRP_STATUS_DRPO_MASK   0x0FFFF
/** DRP status ready mask */
#define XHDMIPHY1_DRP_STATUS_DRPRDY_MASK 0x10000
/** DRP status busy mask */
#define XHDMIPHY1_DRP_STATUS_DRPBUSY_MASK    0x20000
/* 0x068: CPLL CAL PERIOD */
/** CPLL calibration period mask */
#define XHDMIPHY1_CPLL_CAL_PERIOD_MASK      0x3FFFF
/* 0x06C: CPLL CAL TOLERANCE */
/** CPLL calibration tolerance mask */
#define XHDMIPHY1_CPLL_CAL_TOL_MASK         0x3FFFF
/* 0x068: GPI */
/**
 * @brief TX GPI mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return TX GPI mask for the specified channel.
 */
#define XHDMIPHY1_TX_GPI_MASK(Ch) \
        (0x01 << (Ch - 1))

/**
 * @brief RX GPI mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return RX GPI mask for the specified channel.
 */
#define XHDMIPHY1_RX_GPI_MASK(Ch) \
        (0x10 << (Ch - 1))

/* 0x06C: GPO */
/**
 * @brief TX GPO mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return TX GPO mask for the specified channel.
 */
#define XHDMIPHY1_TX_GPO_MASK(Ch) \
        (0x01 << (Ch - 1))

/**
 * @brief TX GPO mask for all channels based on number of channels.
 * @param NCh Number of channels (3 or 4).
 * @return TX GPO mask for all channels.
 */
#define XHDMIPHY1_TX_GPO_MASK_ALL(NCh) \
		((NCh == 3) ? 0x7 : 0xF)
#define XHDMIPHY1_TX_GPO_SHIFT 0
/**
 * @brief RX GPO mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return RX GPO mask for the specified channel.
 */
#define XHDMIPHY1_RX_GPO_MASK(Ch) \
        (0x10 << (Ch - 1))

/**
 * @brief RX GPO mask for all channels based on number of channels.
 * @param NCh Number of channels (3 or 4).
 * @return RX GPO mask for all channels.
 */
#define XHDMIPHY1_RX_GPO_MASK_ALL(NCh) \
		((NCh == 3) ? 0x70 : 0xF0)
/** RX GPO bit shift offset */
#define XHDMIPHY1_RX_GPO_SHIFT 4
/* 0x070: TX_CONTROL */
/**
 * @brief TX 8B/10B enable mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return TX 8B/10B enable mask for the specified channel.
 */
#define XHDMIPHY1_TX_CONTROL_TX8B10BEN_MASK(Ch) \
        (0x01 << (8 * (Ch - 1)))

/** TX 8B/10B enable mask for all channels */
#define XHDMIPHY1_TX_CONTROL_TX8B10BEN_ALL_MASK \
        (XHDMIPHY1_TX_CONTROL_TX8B10BEN_MASK(XHDMIPHY1_CHANNEL_ID_CH1) | \
        XHDMIPHY1_TX_CONTROL_TX8B10BEN_MASK(XHDMIPHY1_CHANNEL_ID_CH2) | \
        XHDMIPHY1_TX_CONTROL_TX8B10BEN_MASK(XHDMIPHY1_CHANNEL_ID_CH3) | \
        XHDMIPHY1_TX_CONTROL_TX8B10BEN_MASK(XHDMIPHY1_CHANNEL_ID_CH4))

/**
 * @brief TX polarity mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return TX polarity mask for the specified channel.
 */
#define XHDMIPHY1_TX_CONTROL_TXPOLARITY_MASK(Ch) \
        (0x02 << (8 * (Ch - 1)))

/** TX polarity mask for all channels */
#define XHDMIPHY1_TX_CONTROL_TXPOLARITY_ALL_MASK \
        (XHDMIPHY1_TX_CONTROL_TXPOLARITY_MASK(XHDMIPHY1_CHANNEL_ID_CH1) | \
        XHDMIPHY1_TX_CONTROL_TXPOLARITY_MASK(XHDMIPHY1_CHANNEL_ID_CH2) | \
        XHDMIPHY1_TX_CONTROL_TXPOLARITY_MASK(XHDMIPHY1_CHANNEL_ID_CH3) | \
        XHDMIPHY1_TX_CONTROL_TXPOLARITY_MASK(XHDMIPHY1_CHANNEL_ID_CH4))

/**
 * @brief TX PRBS select mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return TX PRBS select mask for the specified channel.
 */
#define XHDMIPHY1_TX_CONTROL_TXPRBSSEL_MASK(Ch) \
        (0x5C << (8 * (Ch - 1)))

/** TX PRBS select mask for all channels */
#define XHDMIPHY1_TX_CONTROL_TXPRBSSEL_ALL_MASK \
        (XHDMIPHY1_TX_CONTROL_TXPRBSSEL_MASK(XHDMIPHY1_CHANNEL_ID_CH1) | \
        XHDMIPHY1_TX_CONTROL_TXPRBSSEL_MASK(XHDMIPHY1_CHANNEL_ID_CH2) | \
        XHDMIPHY1_TX_CONTROL_TXPRBSSEL_MASK(XHDMIPHY1_CHANNEL_ID_CH3) | \
        XHDMIPHY1_TX_CONTROL_TXPRBSSEL_MASK(XHDMIPHY1_CHANNEL_ID_CH4))

/**
 * @brief TX PRBS select shift for specific channel.
 * @param Ch Channel number (1-4).
 * @return TX PRBS select shift for the specified channel.
 */
#define XHDMIPHY1_TX_CONTROL_TXPRBSSEL_SHIFT(Ch) \
        (2 + (8 * (Ch - 1)))

/**
 * @brief TX PRBS force error mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return TX PRBS force error mask for the specified channel.
 */
#define XHDMIPHY1_TX_CONTROL_TXPRBSFORCEERR_MASK(Ch) \
        (0x20 << (8 * (Ch - 1)))
/** TX PRBS force error mask for all channels */
#define XHDMIPHY1_TX_CONTROL_TXPRBSFORCEERR_ALL_MASK \
        (XHDMIPHY1_TX_CONTROL_TXPRBSFORCEERR_MASK(XHDMIPHY1_CHANNEL_ID_CH1) | \
        XHDMIPHY1_TX_CONTROL_TXPRBSFORCEERR_MASK(XHDMIPHY1_CHANNEL_ID_CH2) | \
        XHDMIPHY1_TX_CONTROL_TXPRBSFORCEERR_MASK(XHDMIPHY1_CHANNEL_ID_CH3) | \
        XHDMIPHY1_TX_CONTROL_TXPRBSFORCEERR_MASK(XHDMIPHY1_CHANNEL_ID_CH4))
/* 0x074: TX_BUFFER_BYPASS */
/**
 * @brief TX phase delay reset mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return TX phase delay reset mask for the specified channel.
 */
#define XHDMIPHY1_TX_BUFFER_BYPASS_TXPHDLYRESET_MASK(Ch) \
        (0x01 << (8 * (Ch - 1)))

/**
 * @brief TX phase align mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return TX phase align mask for the specified channel.
 */
#define XHDMIPHY1_TX_BUFFER_BYPASS_TXPHALIGN_MASK(Ch) \
        (0x02 << (8 * (Ch - 1)))

/**
 * @brief TX phase align enable mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return TX phase align enable mask for the specified channel.
 */
#define XHDMIPHY1_TX_BUFFER_BYPASS_TXPHALIGNEN_MASK(Ch) \
        (0x04 << (8 * (Ch - 1)))

/**
 * @brief TX phase delay power down mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return TX phase delay power down mask for the specified channel.
 */
#define XHDMIPHY1_TX_BUFFER_BYPASS_TXPHDLYPD_MASK(Ch) \
        (0x08 << (8 * (Ch - 1)))

/**
 * @brief TX phase initialize mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return TX phase initialize mask for the specified channel.
 */
#define XHDMIPHY1_TX_BUFFER_BYPASS_TXPHINIT_MASK(Ch) \
        (0x10 << (8 * (Ch - 1)))

/**
 * @brief TX delay reset mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return TX delay reset mask for the specified channel.
 */
#define XHDMIPHY1_TX_BUFFER_BYPASS_TXDLYRESET_MASK(Ch) \
        (0x20 << (8 * (Ch - 1)))

/**
 * @brief TX delay bypass mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return TX delay bypass mask for the specified channel.
 */
#define XHDMIPHY1_TX_BUFFER_BYPASS_TXDLYBYPASS_MASK(Ch) \
        (0x40 << (8 * (Ch - 1)))

/**
 * @brief TX delay enable mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return TX delay enable mask for the specified channel.
 */
#define XHDMIPHY1_TX_BUFFER_BYPASS_TXDLYEN_MASK(Ch) \
        (0x80 << (8 * (Ch - 1)))
/* 0x078: TX_STATUS */
/**
 * @brief TX phase align done mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return TX phase align done mask for the specified channel.
 */
#define XHDMIPHY1_TX_STATUS_TXPHALIGNDONE_MASK(Ch) \
        (0x01 << (8 * (Ch - 1)))

/**
 * @brief TX phase initialize done mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return TX phase initialize done mask for the specified channel.
 */
#define XHDMIPHY1_TX_STATUS_TXPHINITDONE_MASK(Ch) \
        (0x02 << (8 * (Ch - 1)))

/**
 * @brief TX delay reset done mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return TX delay reset done mask for the specified channel.
 */
#define XHDMIPHY1_TX_STATUS_TXDLYRESETDONE_MASK(Ch) \
        (0x04 << (8 * (Ch - 1)))

/**
 * @brief TX buffer status mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return TX buffer status mask for the specified channel.
 */
#define XHDMIPHY1_TX_STATUS_TXBUFSTATUS_MASK(Ch) \
        (0x18 << (8 * (Ch - 1)))

/**
 * @brief TX buffer status shift for specific channel.
 * @param Ch Channel number (1-4).
 * @return TX buffer status shift for the specified channel.
 */
#define XHDMIPHY1_TX_STATUS_TXBUFSTATUS_SHIFT(Ch) \
        (3 + (8 * (Ch - 1)))
/* 0x07C, 0x080: TX_DRIVER_CH12, TX_DRIVER_CH34 */
/**
 * @brief TX differential control mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return TX differential control mask for the specified channel.
 */
#define XHDMIPHY1_TX_DRIVER_TXDIFFCTRL_MASK(Ch) \
        (0x000F << (16 * ((Ch - 1) % 2)))

/**
 * @brief TX differential control shift for specific channel.
 * @param Ch Channel number (1-4).
 * @return TX differential control shift for the specified channel.
 */
#define XHDMIPHY1_TX_DRIVER_TXDIFFCTRL_SHIFT(Ch) \
        (16 * ((Ch - 1) % 2))

/**
 * @brief TX electrical idle mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return TX electrical idle mask for the specified channel.
 */
#define XHDMIPHY1_TX_DRIVER_TXELECIDLE_MASK(Ch) \
        (0x0010 << (16 * ((Ch - 1) % 2)))

/**
 * @brief TX electrical idle shift for specific channel.
 * @param Ch Channel number (1-4).
 * @return TX electrical idle shift for the specified channel.
 */
#define XHDMIPHY1_TX_DRIVER_TXELECIDLE_SHIFT(Ch) \
        (4 + (16 * ((Ch - 1) % 2)))

/**
 * @brief TX inhibit mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return TX inhibit mask for the specified channel.
 */
#define XHDMIPHY1_TX_DRIVER_TXINHIBIT_MASK(Ch) \
        (0x0020 << (16 * ((Ch - 1) % 2)))

/**
 * @brief TX inhibit shift for specific channel.
 * @param Ch Channel number (1-4).
 * @return TX inhibit shift for the specified channel.
 */
#define XHDMIPHY1_TX_DRIVER_TXINHIBIT_SHIFT(Ch) \
        (5 + (16 * ((Ch - 1) % 2)))

/**
 * @brief TX post-cursor mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return TX post-cursor mask for the specified channel.
 */
#define XHDMIPHY1_TX_DRIVER_TXPOSTCURSOR_MASK(Ch) \
        (0x07C0 << (16 * ((Ch - 1) % 2)))

/**
 * @brief TX post-cursor shift for specific channel.
 * @param Ch Channel number (1-4).
 * @return TX post-cursor shift for the specified channel.
 */
#define XHDMIPHY1_TX_DRIVER_TXPOSTCURSOR_SHIFT(Ch) \
        (6 + (16 * ((Ch - 1) % 2)))

/**
 * @brief TX pre-cursor mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return TX pre-cursor mask for the specified channel.
 */
#define XHDMIPHY1_TX_DRIVER_TXPRECURSOR_MASK(Ch) \
        (0xF800 << (16 * ((Ch - 1) % 2)))

/**
 * @brief TX pre-cursor shift for specific channel.
 * @param Ch Channel number (1-4).
 * @return TX pre-cursor shift for the specified channel.
 */
#define XHDMIPHY1_TX_DRIVER_TXPRECURSOR_SHIFT(Ch) \
        (11 + (16 * ((Ch - 1) % 2)))

/* 0x084: TX_DRIVER_EXT */
/**
 * @brief TX extended differential control mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return TX extended differential control mask for the specified channel.
 */
#define XHDMIPHY1_TX_DRIVER_EXT_TXDIFFCTRL_MASK(Ch) \
        (0x0001 << (8 * (Ch - 1)))

/**
 * @brief TX extended differential control shift for specific channel.
 * @param Ch Channel number (1-4).
 * @return TX extended differential control shift for the specified channel.
 */
#define XHDMIPHY1_TX_DRIVER_EXT_TXDIFFCTRL_SHIFT(Ch) \
        (8 * (Ch - 1))

/* 0x08C, 0x090: TX_RATE_CH12, TX_RATE_CH34 */
/**
 * @brief TX rate mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return TX rate mask for the specified channel.
 */
#define XHDMIPHY1_TX_RATE_MASK(Ch) \
        (0x00FF << (16 * ((Ch - 1) % 2)))

/**
 * @brief TX rate shift for specific channel.
 * @param Ch Channel number (1-4).
 * @return TX rate shift for the specified channel.
 */
#define XHDMIPHY1_TX_RATE_SHIFT(Ch) \
        (16 * ((Ch - 1) % 2))

/* 0x098, 0x09C: RX_RATE_CH12, RX_RATE_CH34 */
/**
 * @brief RX rate mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return RX rate mask for the specified channel.
 */
#define XHDMIPHY1_RX_RATE_MASK(Ch) \
        (0x00FF << (16 * ((Ch - 1) % 2)))

/**
 * @brief RX rate shift for specific channel.
 * @param Ch Channel number (1-4).
 * @return RX rate shift for the specified channel.
 */
#define XHDMIPHY1_RX_RATE_SHIFT(Ch) \
        (16 * ((Ch - 1) % 2))
/* 0x100: RX_CONTROL */
/**
 * @brief RX 8B/10B enable mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return RX 8B/10B enable mask for the specified channel.
 */
#define XHDMIPHY1_RX_CONTROL_RX8B10BEN_MASK(Ch) \
        (0x02 << (8 * (Ch - 1)))

/** RX 8B/10B enable mask for all channels */
#define XHDMIPHY1_RX_CONTROL_RX8B10BEN_ALL_MASK \
        (XHDMIPHY1_RX_CONTROL_RX8B10BEN_MASK(XHDMIPHY1_CHANNEL_ID_CH1) | \
        XHDMIPHY1_RX_CONTROL_RX8B10BEN_MASK(XHDMIPHY1_CHANNEL_ID_CH2) | \
        XHDMIPHY1_RX_CONTROL_RX8B10BEN_MASK(XHDMIPHY1_CHANNEL_ID_CH3) | \
        XHDMIPHY1_RX_CONTROL_RX8B10BEN_MASK(XHDMIPHY1_CHANNEL_ID_CH4))

/**
 * @brief RX polarity mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return RX polarity mask for the specified channel.
 */
#define XHDMIPHY1_RX_CONTROL_RXPOLARITY_MASK(Ch) \
        (0x04 << (8 * (Ch - 1)))

/** RX polarity mask for all channels */
#define XHDMIPHY1_RX_CONTROL_RXPOLARITY_ALL_MASK \
        (XHDMIPHY1_RX_CONTROL_RXPOLARITY_MASK(XHDMIPHY1_CHANNEL_ID_CH1) | \
        XHDMIPHY1_RX_CONTROL_RXPOLARITY_MASK(XHDMIPHY1_CHANNEL_ID_CH2) | \
        XHDMIPHY1_RX_CONTROL_RXPOLARITY_MASK(XHDMIPHY1_CHANNEL_ID_CH3) | \
        XHDMIPHY1_RX_CONTROL_RXPOLARITY_MASK(XHDMIPHY1_CHANNEL_ID_CH4))

/**
 * @brief RX PRBS counter reset mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return RX PRBS counter reset mask for the specified channel.
 */
#define XHDMIPHY1_RX_CONTROL_RXPRBSCNTRESET_MASK(Ch) \
        (0x08 << (8 * (Ch - 1)))

/**
 * @brief RX PRBS select mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return RX PRBS select mask for the specified channel.
 */
#define XHDMIPHY1_RX_CONTROL_RXPRBSSEL_MASK(Ch) \
        (0xF0 << (8 * (Ch - 1)))

/** RX PRBS select mask for all channels */
#define XHDMIPHY1_RX_CONTROL_RXPRBSSEL_ALL_MASK \
        (XHDMIPHY1_RX_CONTROL_RXPRBSSEL_MASK(XHDMIPHY1_CHANNEL_ID_CH1) | \
        XHDMIPHY1_RX_CONTROL_RXPRBSSEL_MASK(XHDMIPHY1_CHANNEL_ID_CH2) | \
        XHDMIPHY1_RX_CONTROL_RXPRBSSEL_MASK(XHDMIPHY1_CHANNEL_ID_CH3) | \
        XHDMIPHY1_RX_CONTROL_RXPRBSSEL_MASK(XHDMIPHY1_CHANNEL_ID_CH4))

/**
 * @brief RX PRBS select shift for specific channel.
 * @param Ch Channel number (1-4).
 * @return RX PRBS select shift for the specified channel.
 */
#define XHDMIPHY1_RX_CONTROL_RXPRBSSEL_SHIFT(Ch) \
        (4 + (8 * (Ch - 1)))

/* 0x104: RX_STATUS */
/**
 * @brief RX CDR lock mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return RX CDR lock mask for the specified channel.
 */
#define XHDMIPHY1_RX_STATUS_RXCDRLOCK_MASK(Ch) \
        (0x1 << (8 * (Ch - 1)))

/**
 * @brief RX buffer status mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return RX buffer status mask for the specified channel.
 */
#define XHDMIPHY1_RX_STATUS_RXBUFSTATUS_MASK(Ch) \
        (0xE << (8 * (Ch - 1)))

/**
 * @brief RX buffer status shift for specific channel.
 * @param Ch Channel number (1-4).
 * @return RX buffer status shift for the specified channel.
 */
#define XHDMIPHY1_RX_STATUS_RXBUFSTATUS_SHIFT(Ch) \
        (1 + (8 * (Ch - 1)))
/* 0x104: RX_EQ_CDR */
/** @name RX Equalizer and CDR control masks
  * @{
  */
/**
 * @brief RX LPM enable mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return RX LPM enable mask for the specified channel.
 */
#define XHDMIPHY1_RX_CONTROL_RXLPMEN_MASK(Ch) \
        (0x01 << (8 * (Ch - 1)))

/**
 * @brief RX CDR hold mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return RX CDR hold mask for the specified channel.
 */
#define XHDMIPHY1_RX_STATUS_RXCDRHOLD_MASK(Ch) \
        (0x02 << (8 * (Ch - 1)))

/**
 * @brief RX OS override enable mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return RX OS override enable mask for the specified channel.
 */
#define XHDMIPHY1_RX_STATUS_RXOSOVRDEN_MASK(Ch) \
        (0x04 << (8 * (Ch - 1)))

/**
 * @brief RX LPM LFK override enable mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return RX LPM LFK override enable mask for the specified channel.
 */
#define XHDMIPHY1_RX_STATUS_RXLPMLFKLOVRDEN_MASK(Ch) \
        (0x08 << (8 * (Ch - 1)))

/**
 * @brief RX LPM HF override enable mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return RX LPM HF override enable mask for the specified channel.
 */
#define XHDMIPHY1_RX_STATUS_RXLPMHFOVRDEN_MASK(Ch) \
        (0x10 << (8 * (Ch - 1)))
/* @} */
/** RX LPM enable mask for all channels */
#define XHDMIPHY1_RX_CONTROL_RXLPMEN_ALL_MASK \
        (XHDMIPHY1_RX_CONTROL_RXLPMEN_MASK(XHDMIPHY1_CHANNEL_ID_CH1) | \
        XHDMIPHY1_RX_CONTROL_RXLPMEN_MASK(XHDMIPHY1_CHANNEL_ID_CH2) | \
        XHDMIPHY1_RX_CONTROL_RXLPMEN_MASK(XHDMIPHY1_CHANNEL_ID_CH3) | \
        XHDMIPHY1_RX_CONTROL_RXLPMEN_MASK(XHDMIPHY1_CHANNEL_ID_CH4))
/* 0x110, 0x114, 0x118, 0x11C: INTR_EN, INTR_DIS, INTR_MASK, INTR_STS */
/** TX reset done interrupt mask */
#define XHDMIPHY1_INTR_TXRESETDONE_MASK      0x00000001
/** RX reset done interrupt mask */
#define XHDMIPHY1_INTR_RXRESETDONE_MASK      0x00000002
/** CPLL lock interrupt mask */
#define XHDMIPHY1_INTR_CPLL_LOCK_MASK        0x00000004
/** QPLL0 lock interrupt mask */
#define XHDMIPHY1_INTR_QPLL0_LOCK_MASK       0x00000008
/** LCPLL lock interrupt mask (alias for QPLL0) */
#define XHDMIPHY1_INTR_LCPLL_LOCK_MASK       0x00000008
/** TX alignment done interrupt mask */
#define XHDMIPHY1_INTR_TXALIGNDONE_MASK      0x00000010
/** QPLL1 lock interrupt mask */
#define XHDMIPHY1_INTR_QPLL1_LOCK_MASK       0x00000020
/** RPLL lock interrupt mask (alias for QPLL1) */
#define XHDMIPHY1_INTR_RPLL_LOCK_MASK        0x00000020
/** TX clock detector frequency change interrupt mask */
#define XHDMIPHY1_INTR_TXCLKDETFREQCHANGE_MASK   0x00000040
/** RX clock detector frequency change interrupt mask */
#define XHDMIPHY1_INTR_RXCLKDETFREQCHANGE_MASK   0x00000080
/** TX MMCM user clock lock interrupt mask */
#define XHDMIPHY1_INTR_TXMMCMUSRCLK_LOCK_MASK    0x00000200
/** RX MMCM user clock lock interrupt mask */
#define XHDMIPHY1_INTR_RXMMCMUSRCLK_LOCK_MASK    0x00000400
/** TX GPO rising edge interrupt mask */
#define XHDMIPHY1_INTR_TXGPO_RE_MASK             0x00000800
/** RX GPO rising edge interrupt mask */
#define XHDMIPHY1_INTR_RXGPO_RE_MASK             0x00001000
/** TX timer timeout interrupt mask */
#define XHDMIPHY1_INTR_TXTMRTIMEOUT_MASK     0x40000000
/** RX timer timeout interrupt mask */
#define XHDMIPHY1_INTR_RXTMRTIMEOUT_MASK     0x80000000
/** QPLL lock interrupt mask (alias for QPLL0) */
#define XHDMIPHY1_INTR_QPLL_LOCK_MASK        XHDMIPHY1_INTR_QPLL0_LOCK_MASK
/* 0x120, 0x140: MMCM_TXUSRCLK_CTRL, MMCM_RXUSRCLK_CTRL */
/** MMCM user clock control configuration new mask */
#define XHDMIPHY1_MMCM_USRCLK_CTRL_CFG_NEW_MASK  0x01
/** MMCM user clock control reset mask */
#define XHDMIPHY1_MMCM_USRCLK_CTRL_RST_MASK      0x02
/** MMCM user clock control configuration success mask */
#define XHDMIPHY1_MMCM_USRCLK_CTRL_CFG_SUCCESS_MASK  0x10
/** MMCM user clock control locked mask */
#define XHDMIPHY1_MMCM_USRCLK_CTRL_LOCKED_MASK   0x200
/** MMCM user clock control power down mask */
#define XHDMIPHY1_MMCM_USRCLK_CTRL_PWRDWN_MASK   0x400
/** MMCM user clock control locked mask mask */
#define XHDMIPHY1_MMCM_USRCLK_CTRL_LOCKED_MASK_MASK  0x800
/** MMCM user clock control clock input select mask */
#define XHDMIPHY1_MMCM_USRCLK_CTRL_CLKINSEL_MASK 0x1000
/* 0x124, 0x144: MMCM_TXUSRCLK_REG1, MMCM_RXUSRCLK_REG1 */
/** MMCM user clock register 1 divider clock mask */
#define XHDMIPHY1_MMCM_USRCLK_REG1_DIVCLK_MASK \
                    0x00000FF
/** MMCM user clock register 1 clock feedback output multiplier mask */
#define XHDMIPHY1_MMCM_USRCLK_REG1_CLKFBOUT_MULT_MASK \
                    0x000FF00
/** MMCM user clock register 1 clock feedback output multiplier shift */
#define XHDMIPHY1_MMCM_USRCLK_REG1_CLKFBOUT_MULT_SHIFT \
                    8
/** MMCM user clock register 1 clock feedback output fractional mask */
#define XHDMIPHY1_MMCM_USRCLK_REG1_CLKFBOUT_FRAC_MASK \
                    0x3FF0000
/** MMCM user clock register 1 clock feedback output fractional shift */
#define XHDMIPHY1_MMCM_USRCLK_REG1_CLKFBOUT_FRAC_SHIFT \
                    16
/* 0x128, 0x148: MMCM_TXUSRCLK_REG2, MMCM_RXUSRCLK_REG2 */
/** MMCM user clock register 2 divider clock mask */
#define XHDMIPHY1_MMCM_USRCLK_REG2_DIVCLK_MASK \
                    0x00000FF
/** MMCM user clock register 2 clock output 0 fractional mask */
#define XHDMIPHY1_MMCM_USRCLK_REG2_CLKOUT0_FRAC_MASK \
                    0x3FF0000
/** MMCM user clock register 2 clock output 0 fractional shift */
#define XHDMIPHY1_MMCM_USRCLK_REG2_CLKOUT0_FRAC_SHIFT \
                    16
/* 0x12C, 0x130, 0x14C, 0x150: MMCM_TXUSRCLK_REG[3,4], MMCM_RXUSRCLK_REG[3,4]*/
/** MMCM user clock register 3/4 divider clock mask */
#define XHDMIPHY1_MMCM_USRCLK_REG34_DIVCLK_MASK \
                    0x00000FF
/* 0x134, 0x154: BUFGT_TXUSRCLK, BUFGT_RXUSRCLK */
/** BUFGGT user clock clear mask */
#define XHDMIPHY1_BUFGGT_XXUSRCLK_CLR_MASK   0x1
/** BUFGGT user clock divider mask */
#define XHDMIPHY1_BUFGGT_XXUSRCLK_DIV_MASK   0xE
/** BUFGGT user clock divider shift */
#define XHDMIPHY1_BUFGGT_XXUSRCLK_DIV_SHIFT  1
/* 0x138, 0x158: MISC_TXUSRCLK_REG, MISC_RXUSERCLK_REG */
/** Miscellaneous user clock output 1 output enable mask */
#define XHDMIPHY1_MISC_XXUSRCLK_CKOUT1_OEN_MASK  0x1
/** Miscellaneous user clock reference clock CEB (Clock Enable Bar) mask */
#define XHDMIPHY1_MISC_XXUSRCLK_REFCLK_CEB_MASK  0x2
/* 0x200: CLKDET_CTRL */
/** Clock detector control run mask */
#define XHDMIPHY1_CLKDET_CTRL_RUN_MASK           0x1
/** Clock detector control TX timer clear mask */
#define XHDMIPHY1_CLKDET_CTRL_TX_TMR_CLR_MASK        0x2
/** Clock detector control RX timer clear mask */
#define XHDMIPHY1_CLKDET_CTRL_RX_TMR_CLR_MASK        0x4
/** Clock detector control TX frequency reset mask */
#define XHDMIPHY1_CLKDET_CTRL_TX_FREQ_RST_MASK       0x8
/** Clock detector control RX frequency reset mask */
#define XHDMIPHY1_CLKDET_CTRL_RX_FREQ_RST_MASK       0x10
/** Clock detector control frequency lock threshold mask */
#define XHDMIPHY1_CLKDET_CTRL_FREQ_LOCK_THRESH_MASK      0x1FE0
/** Clock detector control frequency lock threshold shift */
#define XHDMIPHY1_CLKDET_CTRL_FREQ_LOCK_THRESH_SHIFT 5
/** Clock detector control accuracy range mask */
#define XHDMIPHY1_CLKDET_CTRL_ACCURACY_RANGE_MASK      0x1E000
/** Clock detector control accuracy range shift */
#define XHDMIPHY1_CLKDET_CTRL_ACCURACY_RANGE_SHIFT 13
/* 0x204: CLKDET_STAT */
/** Clock detector status TX frequency zero mask */
#define XHDMIPHY1_CLKDET_STAT_TX_FREQ_ZERO_MASK      0x1
/** Clock detector status RX frequency zero mask */
#define XHDMIPHY1_CLKDET_STAT_RX_FREQ_ZERO_MASK      0x2
/** Clock detector status TX reference clock lock mask */
#define XHDMIPHY1_CLKDET_STAT_TX_REFCLK_LOCK_MASK        0x3
/** Clock detector status TX reference clock lock capture mask */
#define XHDMIPHY1_CLKDET_STAT_TX_REFCLK_LOCK_CAP_MASK    0x4
/* 0x300: DRU_CTRL */
/**
 * @brief DRU control reset mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return DRU control reset mask for the specified channel.
 */
#define XHDMIPHY1_DRU_CTRL_RST_MASK(Ch)  (0x01 << (8 * (Ch - 1)))

/**
 * @brief DRU control enable mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return DRU control enable mask for the specified channel.
 */
#define XHDMIPHY1_DRU_CTRL_EN_MASK(Ch)   (0x02 << (8 * (Ch - 1)))

/* 0x304: DRU_STAT */
/**
 * @brief DRU status active mask for specific channel.
 * @param Ch Channel number (1-4).
 * @return DRU status active mask for the specified channel.
 */
#define XHDMIPHY1_DRU_STAT_ACTIVE_MASK(Ch)        (0x01 << (8 * (Ch - 1)))
/** DRU status version mask */
#define XHDMIPHY1_DRU_STAT_VERSION_MASK  0xFF000000
/** DRU status version shift */
#define XHDMIPHY1_DRU_STAT_VERSION_SHIFT 24
/* 0x30C, 0x318, 0x324, 0x330: DRU_CFREQ_H_CH[1-4] */
/** DRU center frequency high mask */
#define XHDMIPHY1_DRU_CFREQ_H_MASK       0x1F
/* 0x310, 0x31C, 0x328, 0x334: DRU_GAIN_CH[1-4] */
/** DRU gain G1 mask */
#define XHDMIPHY1_DRU_GAIN_G1_MASK       0x00001F
/** DRU gain G1 shift */
#define XHDMIPHY1_DRU_GAIN_G1_SHIFT      0
/** DRU gain G1 P mask */
#define XHDMIPHY1_DRU_GAIN_G1_P_MASK 0x001F00
/** DRU gain G1 P shift */
#define XHDMIPHY1_DRU_GAIN_G1_P_SHIFT    8
/** DRU gain G2 mask */
#define XHDMIPHY1_DRU_GAIN_G2_MASK       0x1F0000
/** DRU gain G2 shift */
#define XHDMIPHY1_DRU_GAIN_G2_SHIFT      16
/* 0x340 TMDS PATGEN */
/** PATGEN control enable mask */
#define XHDMIPHY1_PATGEN_CTRL_ENABLE_MASK        0x80000000
/** PATGEN control enable shift */
#define XHDMIPHY1_PATGEN_CTRL_ENABLE_SHIFT       31
/** PATGEN control ratio mask */
#define XHDMIPHY1_PATGEN_CTRL_RATIO_MASK     0x7
/** PATGEN control ratio shift */
#define XHDMIPHY1_PATGEN_CTRL_RATIO_SHIFT        0
/* @} */

/******************* Macros (Inline Functions) Definitions ********************/

/** @name Register access macro definitions.
  * @{
  */
/**
 * @brief Read 32-bit value from register.
 *
 * This macro provides a wrapper for reading 32-bit values from memory-mapped registers.
 * It maps to the Xilinx library function Xil_In32.
 *
 * @param Addr Memory address to read from.
 * @return 32-bit value read from the specified address.
 */
#define XHdmiphy1_In32 Xil_In32

/**
 * @brief Write 32-bit value to register.
 *
 * This macro provides a wrapper for writing 32-bit values to memory-mapped registers.
 * It maps to the Xilinx library function Xil_Out32.
 *
 * @param Addr Memory address to write to.
 * @param Data 32-bit value to write to the specified address.
 */
#define XHdmiphy1_Out32 Xil_Out32
/* @} */

/******************************************************************************/
/**
 * This is a low-level function that reads from the specified register.
 *
 * @param   BaseAddress is the base address of the device.
 * @param   RegOffset is the register offset to be read from.
 *
 * @return  The 32-bit value of the specified register.
 *
 * @note    C-style signature:
 *      u32 XHdmiphy1_ReadReg(u32 BaseAddress, u32 RegOffset)
 *
*******************************************************************************/
#define XHdmiphy1_ReadReg(BaseAddress, RegOffset) \
    XHdmiphy1_In32((BaseAddress) + (RegOffset))

/******************************************************************************/
/**
 * This is a low-level function that writes to the specified register.
 *
 * @param   BaseAddress is the base address of the device.
 * @param   RegOffset is the register offset to write to.
 * @param   Data is the 32-bit data to write to the specified register.
 *
 * @return  None.
 *
 * @note    C-style signature:
 *      void XHdmiphy1_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
 *
*******************************************************************************/
#define XHdmiphy1_WriteReg(BaseAddress, RegOffset, Data) \
    XHdmiphy1_Out32((BaseAddress) + (RegOffset), (Data))

#ifdef __cplusplus
}
#endif

#endif /* XHDMIPHY1_HW_H_ */
/** @} */
