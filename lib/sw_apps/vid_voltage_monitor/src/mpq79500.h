/******************************************************************************
 * Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @file mpq79500.h
 * @brief MPQ79500FS voltage monitor driver: register map and OV/UV threshold API.
 */
#ifndef MPQ79500_H
#define MPQ79500_H

#include "xil_types.h"
#include "xstatus.h"

/** @brief Bank select register address. */
#define MPQ79500_BANK_SEL_REG		0xF0U

/** @brief Status bank index for VIN level readback. */
#define MPQ79500_BANK_STATUS		0U

/** @brief Configuration bank index for threshold programming. */
#define MPQ79500_BANK_CONFIG		1U

/** @brief Mask for the bank-select bit in BANK_SEL. */
#define MPQ79500_BANK_SEL_MASK		0x01U

/** @brief VIN channel enable register. */
#define MPQ79500_VIN_CH_EN_REG		0x1EU

/** @brief Per-channel voltage range multiplier register. */
#define MPQ79500_VRANGE_MULT_REG	0x1FU

/*
 * Interrupt-enable registers are in Bank 1. Their range (0x13-0x16) sits
 * inside the Bank-0 INT_* fault-flag range (0x12-0x18) below, so some
 * addresses appear in both lists. That is expected. The address is the same
 * but the register differs per bank, and the pairs that collide are
 * unrelated. Addresses come from the MPQ79500FS datasheet.
 */
/** @brief Interrupt enable (Bank 1) - under-voltage high-frequency. */
#define MPQ79500_IEN_UVHF_REG		0x13U

/** @brief Interrupt enable (Bank 1) - under-voltage low-frequency. */
#define MPQ79500_IEN_UVLF_REG		0x14U

/** @brief Interrupt enable (Bank 1) - over-voltage high-frequency. */
#define MPQ79500_IEN_OVHF_REG		0x15U

/** @brief Interrupt enable (Bank 1) - over-voltage low-frequency. */
#define MPQ79500_IEN_OVLF_REG		0x16U

/** @brief Sequence record control register (Bank 1). */
#define MPQ79500_SEQ_REC_CTL_REG	0xA0U

/**
 * @brief SEQ_REC_CTL value to move the device from idle to the active state.
 *
 * REC_START (D7) = 1 with SEQ[1:0] (D6:5) = 00b (power-on). In idle the part
 * monitors OV only and masks UV. The active state adds UV. REC_START is a
 * self-clearing trigger, so it reads back as 0.
 */
#define MPQ79500_REC_START_SEQ_ON	0x80U

/*
 * VENDOR_ID (0x00) and MODEL_REV (0x01) are OTP-loaded and vary per part.
 * They read 0x31, 0x32 or 0x33 on VEK385, so they are not used for detection.
 * Presence comes from the I2CADDR readback below instead.
 */

/** @brief I2CADDR register (bank-independent): reports the address the part uses. */
#define MPQ79500_I2CADDR_REG		0xF9U

/** @brief Group write-protect register 1 (bank-independent, R/W, POR 0x00, sticky). */
#define MPQ79500_PROT1_REG		0xF1U

/** @brief Group write-protect register 2 (bank-independent, R/W, POR 0x00, sticky). */
#define MPQ79500_PROT2_REG		0xF2U

/** @brief PROT_MON1 register (bank-independent, R/W). POR loaded from OTP. */
#define MPQ79500_PROT_MON1_REG		0xF3U

/*
 * OV/UV fault flag registers (Bank 0, read + write-1-to-clear). Each holds one
 * bit per channel (bit N-1 for channel N). This is where the chip latches an
 * over or under voltage event. VMON_STAT (0x30) does not report it.
 */
/** @brief Under-voltage high-frequency fault flags. */
#define MPQ79500_INT_UVHF_REG		0x12U
/** @brief Under-voltage low-frequency fault flags. */
#define MPQ79500_INT_UVLF_REG		0x14U
/** @brief Over-voltage high-frequency fault flags. */
#define MPQ79500_INT_OVHF_REG		0x16U
/** @brief Over-voltage low-frequency fault flags. */
#define MPQ79500_INT_OVLF_REG		0x18U

/** @brief Base address of VIN level registers (CH1..CH6). */
#define MPQ79500_VIN_LVL_BASE		0x40U

/** @brief Base register address of the per-channel config block (CH1). */
#define MPQ79500_CH_REG_BASE		0x20U

/** @brief Register stride between monitor channels. */
#define MPQ79500_CH_REG_STRIDE		0x10U

/** @brief Lowest valid monitor channel number. */
#define MPQ79500_CHANNEL_MIN		1U

/** @brief Highest valid monitor channel number. */
#define MPQ79500_CHANNEL_MAX		6U

/** @brief Largest 8-bit threshold register code. */
#define MPQ79500_REG_CODE_MAX		255U

/** @brief Channel UV high-frequency threshold register offset. */
#define MPQ79500_UV_HF_OFF		0x0U

/** @brief Channel OV high-frequency threshold register offset. */
#define MPQ79500_OV_HF_OFF		0x1U

/** @brief Channel UV low-frequency threshold register offset. */
#define MPQ79500_UV_LF_OFF		0x2U

/** @brief Channel OV low-frequency threshold register offset. */
#define MPQ79500_OV_LF_OFF		0x3U

/** @brief Channel high-frequency filter/debounce register offset. */
#define MPQ79500_FLT_HF_OFF		0x4U

/** @brief Channel low-frequency filter register offset. */
#define MPQ79500_FC_LF_OFF		0x5U

/** @brief High-frequency filter/debounce value written per channel (FLT_HF). */
#define MPQ79500_FLT_HF_CFG		0x22U

/** @brief Low-frequency filter count written per channel (FC_LF). */
#define MPQ79500_FC_LF_CFG		0x04U

/** @brief Offset subtracted before mV is turned into a register code (mV). */
#define MPQ79500_MV_OFFSET		200U

/** @brief Millivolts represented by one register count (mV). */
#define MPQ79500_MV_LSB			5U

/** @brief Test value written to BANK_SEL during the write self-test. */
#define MPQ79500_BANK_SEL_TEST_VAL	0x01U

/** @brief Normal BANK_SEL value restored after the write self-test. */
#define MPQ79500_BANK_SEL_DEFAULT_VAL	0x00U

/** @brief Return non-zero when Channel is in the valid MPQ79500 range. */
#define MPQ79500_CHANNEL_IS_VALID(ch) \
	(((ch) >= MPQ79500_CHANNEL_MIN) && ((ch) <= MPQ79500_CHANNEL_MAX))

/**
 * @brief MPQ79500 channel programming configuration.
 */
typedef struct {
	u8 I2cAddr;   /**< 7-bit I2C slave address. */
	u8 Channel;   /**< Monitor channel number (1..6). */
	u16 TargetMv; /**< Post-VID target voltage in mV. */
	u8 MarginPct; /**< OV/UV margin as percent of target. */
} MPQ79500_ChannelCfg;

XStatus MPQ79500_Probe(const u8 I2cAddr);
XStatus MPQ79500_SelfTest(const u8 I2cAddr);
XStatus MPQ79500_ProgramChannel(const MPQ79500_ChannelCfg *Cfg);
XStatus MPQ79500_ArmChannel(const u8 I2cAddr, const u8 Channel);
XStatus MPQ79500_EnterActive(const u8 I2cAddr);
XStatus MPQ79500_ReadVinLevelMv(const u8 I2cAddr, const u8 Channel,
				u16 *const LevelMv);
XStatus MPQ79500_ReadThresholdMv(const u8 I2cAddr, const u8 Channel,
				 const u8 ThreshOff, u16 *const ThreshMv);
XStatus MPQ79500_ReadClearOvUv(const u8 I2cAddr, const u8 Channel,
			       u8 *const OvFault, u8 *const UvFault);

#endif /* MPQ79500_H */
