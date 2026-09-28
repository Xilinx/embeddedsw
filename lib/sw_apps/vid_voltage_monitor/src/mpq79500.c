/******************************************************************************
 * Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @file mpq79500.c
 * @brief MPQ79500FS I2C register programming for OV/UV thresholds.
 */
#include "board_vek385_vmon.h"
#include "mpq79500.h"
#include "vek385_i2c_bus.h"
#include "xil_printf.h"

/**
 * @brief Print one failed register access.
 *
 * @param Step Human-readable operation name.
 * @param I2cAddr 7-bit I2C slave address.
 * @param Reg Register address.
 * @param Status XStatus error code.
 */
static void Mpq_LogRegError(const char *Step, const u8 I2cAddr,
			      const u8 Reg, const XStatus Status)
{
	xil_printf("ERROR: MPQ79500 @0x%02x %s reg=0x%02x status=%d\r\n",
		   I2cAddr, Step, Reg, Status);
}

/**
 * @brief How many times a banked access re-takes PMBUS1 after losing it.
 *
 * Losing arbitration means the System Controller took the bus, so the whole
 * sequence has to start again from the acquire.
 */
#define MPQ_ARB_LOST_RETRY_MAX		3U

/**
 * @brief SMBus PEC checksum (CRC-8, polynomial 0x07, init 0x00).
 *
 * No reflection and no final XOR.
 *
 * @param Crc Running CRC value (start with 0x00).
 * @param Buf Bytes to fold into the CRC.
 * @param Len Number of bytes.
 *
 * @return Updated CRC value.
 */
static u8 Mpq_Crc8(u8 Crc, const u8 *Buf, const s32 Len)
{
	s32 i;
	u8 j;

	for (i = 0; i < Len; i++) {
		Crc ^= Buf[i];
		for (j = 0U; j < 8U; j++) {
			if (0U != (Crc & 0x80U)) {
				Crc = (u8)((Crc << 1) ^ 0x07U);
			} else {
				Crc = (u8)(Crc << 1);
			}
		}
	}

	return Crc;
}

/**
 * @brief Write one register byte with the SMBus PEC the part expects.
 *
 * The MPQ79500 has PEC enabled in OTP, so every write must end with the
 * checksum or the part ACKs the bytes and silently drops the data. PEC
 * covers the 8-bit write address (7-bit address, R/W bit = 0), the register
 * and the value.
 *
 * The caller must already own PMBUS1, so a bank-select and its access can
 * run on one ownership. A re-acquire in between could re-select the mux and
 * lose the bank set earlier in the same sequence.
 *
 * @param I2cAddr 7-bit I2C slave address.
 * @param Reg Register address.
 * @param Val Value to write.
 *
 * @return XST_SUCCESS on success, otherwise an XStatus error code. A loss of
 *         the shared bus surfaces as XST_IIC_ARB_LOST.
 */
static XStatus Mpq_SmbusWriteByte(const u8 I2cAddr, const u8 Reg, const u8 Val)
{
	u8 Buf[3];
	u8 AddrByte;
	u8 Pec;

	Buf[0] = Reg;
	Buf[1] = Val;

	AddrByte = (u8)((I2cAddr << 1) & 0xFEU);
	Pec = Mpq_Crc8(0x00U, &AddrByte, 1);
	Pec = Mpq_Crc8(Pec, Buf, 2);
	Buf[2] = Pec;

	return Vek385_I2cWriteRaw(I2cAddr, Buf, 3);
}

/**
 * @brief Write one byte to a register in the given bank.
 *
 * Take PMBUS1, select the bank, then write. The two steps must run
 * back-to-back. Otherwise the System Controller could use the bus in
 * between and leave the chip on the wrong bank.
 *
 * On XST_IIC_ARB_LOST the SC has taken PMBUS1, so the whole sequence
 * restarts from the acquire. Retrying just the lost byte would talk to
 * whatever the SC left on the downstream bus.
 *
 * @param I2cAddr 7-bit I2C slave address.
 * @param Bank Bank index (MPQ79500_BANK_STATUS or MPQ79500_BANK_CONFIG).
 * @param Reg Register address.
 * @param Val Value to write.
 *
 * @return XST_SUCCESS on success, otherwise an XStatus error code. A loss of
 *         the shared bus that outlives the retries surfaces as
 *         XST_IIC_ARB_LOST.
 */
static XStatus Mpq_WriteBankReg(const u8 I2cAddr, const u8 Bank,
				const u8 Reg, const u8 Val)
{
	u32 Retry = 0U;
	XStatus Status = XST_FAILURE;

	do {
		Status = Vek385_I2cAcquirePmbus1();
		if (XST_SUCCESS != Status) {
			continue;
		}

		Status = Mpq_SmbusWriteByte(I2cAddr, MPQ79500_BANK_SEL_REG,
					    (u8)(Bank & MPQ79500_BANK_SEL_MASK));
		if (XST_SUCCESS != Status) {
			continue;
		}

		Status = Mpq_SmbusWriteByte(I2cAddr, Reg, Val);
	} while ((XST_IIC_ARB_LOST == Status) &&
		 (++Retry < MPQ_ARB_LOST_RETRY_MAX));

	return Status;
}

/**
 * @brief Read one byte from a register in the given bank.
 *
 * Same bus-hold rules as Mpq_WriteBankReg().
 *
 * @param I2cAddr 7-bit I2C slave address.
 * @param Bank Bank index (MPQ79500_BANK_STATUS or MPQ79500_BANK_CONFIG).
 * @param Reg Register address.
 * @param Val Pointer to receive the register value.
 *
 * @return XST_SUCCESS on success, otherwise an XStatus error code. A loss of
 *         the shared bus surfaces as XST_IIC_ARB_LOST.
 */
static XStatus Mpq_ReadBankReg(const u8 I2cAddr, const u8 Bank,
			       const u8 Reg, u8 *const Val)
{
	u32 Retry = 0U;
	XStatus Status = XST_FAILURE;

	do {
		Status = Vek385_I2cAcquirePmbus1();
		if (XST_SUCCESS != Status) {
			continue;
		}

		Status = Mpq_SmbusWriteByte(I2cAddr, MPQ79500_BANK_SEL_REG,
					    (u8)(Bank & MPQ79500_BANK_SEL_MASK));
		if (XST_SUCCESS != Status) {
			continue;
		}

		Status = Vek385_I2cRegReadLocked(I2cAddr, Reg, Val);
	} while ((XST_IIC_ARB_LOST == Status) &&
		 (++Retry < MPQ_ARB_LOST_RETRY_MAX));

	return Status;
}

/**
 * @brief Compute per-channel configuration register address.
 *
 * @param Channel Monitor channel (MPQ79500_CHANNEL_MIN..MPQ79500_CHANNEL_MAX).
 * @param Offset Register offset within the channel block.
 *
 * @return Absolute register address.
 */
static u8 Mpq_ChannelRegAddr(const u8 Channel, const u8 Offset)
{
	/* Each channel owns a 0x10-byte block in the config bank. */
	return (u8)(MPQ79500_CH_REG_BASE +
		    ((Channel - MPQ79500_CHANNEL_MIN) * MPQ79500_CH_REG_STRIDE) +
		    Offset);
}

/**
 * @brief Bit mask for a monitor channel in multi-channel registers.
 *
 * @param Channel Monitor channel (MPQ79500_CHANNEL_MIN..MPQ79500_CHANNEL_MAX).
 *
 * @return Single-bit mask for the channel.
 */
static u8 Mpq_ChannelBitMask(const u8 Channel)
{
	u8 Mask = 0U;

	/*
	 * Guard the shift: a channel below MPQ79500_CHANNEL_MIN would make the
	 * subtraction wrap and shift past the width of the type. Callers
	 * validate the channel, so 0 here means a caller bug, and a mask of 0
	 * touches no channel.
	 */
	if (MPQ79500_CHANNEL_IS_VALID(Channel)) {
		/* Channel N uses bit (N - 1) in shared enable and flag registers. */
		Mask = (u8)(1U << (Channel - MPQ79500_CHANNEL_MIN));
	}

	return Mask;
}

/**
 * @brief Convert millivolts to an MPQ79500 threshold register code.
 *
 * The chip stores voltage as a byte: code = (mV - 200) / 5. This is
 * the chip register format, not the app OV/UV margin percent.
 *
 * @param Mv Voltage in millivolts.
 *
 * @return Clamped 8-bit register code.
 */
static u8 Mpq_MvToRegCode(u16 Mv)
{
	u32 Reg;

	if (Mv < MPQ79500_MV_OFFSET) {
		Mv = MPQ79500_MV_OFFSET;
	}

	/* Each code step is 5 mV above a 200 mV offset. */
	Reg = (Mv - MPQ79500_MV_OFFSET) / MPQ79500_MV_LSB;
	if (Reg > MPQ79500_REG_CODE_MAX) {
		Reg = MPQ79500_REG_CODE_MAX;
	}

	return (u8)Reg;
}

/**
 * @brief Convert a threshold register code back to millivolts.
 *
 * @param Reg 8-bit register code.
 *
 * @return Voltage in millivolts (200 + Reg * 5).
 */
static u16 Mpq_RegCodeToMv(const u8 Reg)
{
	return (u16)(MPQ79500_MV_OFFSET + ((u16)Reg * MPQ79500_MV_LSB));
}

/**
 * @brief Select the default voltage range for one channel.
 *
 * VRANGE_MULT selects how the chip maps register codes to mV.
 * Clear the channel bit to use 200 mV + 5 mV steps. This is not
 * the same as VEK385_VMON_MARGIN_PCT, which sets the OV/UV window.
 *
 * @param I2cAddr 7-bit I2C slave address.
 * @param Channel Monitor channel (MPQ79500_CHANNEL_MIN..MPQ79500_CHANNEL_MAX).
 *
 * @return XST_SUCCESS on success, otherwise an XStatus error code. A loss of
 *         the shared bus surfaces as XST_IIC_ARB_LOST.
 */
static XStatus Mpq_SelectDefaultVoltageRange(const u8 I2cAddr, const u8 Channel)
{
	u8 Val;
	const u8 Mask = Mpq_ChannelBitMask(Channel);
	XStatus Status = XST_FAILURE;

	Status = Mpq_ReadBankReg(I2cAddr, MPQ79500_BANK_CONFIG,
				 MPQ79500_VRANGE_MULT_REG, &Val);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Val &= (u8)~Mask;

	Status = Mpq_WriteBankReg(I2cAddr, MPQ79500_BANK_CONFIG,
				  MPQ79500_VRANGE_MULT_REG, Val);

done:
	return Status;
}

/**
 * @brief Read a config register and set extra bits without clearing others.
 *
 * @param I2cAddr 7-bit I2C slave address.
 * @param Reg Register address.
 * @param Bits Bit mask to OR into the register.
 *
 * @return XST_SUCCESS on success, otherwise an XStatus error code. A loss of
 *         the shared bus surfaces as XST_IIC_ARB_LOST.
 */
static XStatus Mpq_SetConfigRegBits(const u8 I2cAddr, const u8 Reg,
				    const u8 Bits)
{
	u8 Val;
	XStatus Status = XST_FAILURE;

	Status = Mpq_ReadBankReg(I2cAddr, MPQ79500_BANK_CONFIG, Reg, &Val);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = Mpq_WriteBankReg(I2cAddr, MPQ79500_BANK_CONFIG, Reg,
				  (u8)(Val | Bits));

done:
	return Status;
}

/**
 * @brief Check one MPQ79500 is present via its I2CADDR readback.
 *
 * The chip reports its own I2C address in register 0xF9. If that
 * readback matches the expected address, the monitor is present.
 *
 * @param I2cAddr 7-bit I2C slave address.
 *
 * @return XST_SUCCESS if present and correctly addressed, XST_FAILURE otherwise.
 */
XStatus MPQ79500_Probe(const u8 I2cAddr)
{
	u8 Readback = 0U;
	XStatus Status = XST_FAILURE;

	Status = Vek385_I2cAcquirePmbus1();
	if (XST_SUCCESS != Status) {
		xil_printf("MPQ79500 @0x%02x: PMBUS1 not available (status=%d)\r\n",
			   I2cAddr, Status);
		goto done;
	}

	Status = Vek385_I2cRegReadLocked(I2cAddr, MPQ79500_I2CADDR_REG,
					 &Readback);
	if (XST_SUCCESS != Status) {
		xil_printf("MPQ79500 @0x%02x: not reachable (status=%d)\r\n",
			   I2cAddr, Status);
		goto done;
	}

	if (I2cAddr != Readback) {
		xil_printf("WARN: @0x%02x I2CADDR readback 0x%02x != 0x%02x\r\n",
			   I2cAddr, Readback, I2cAddr);
		Status = XST_FAILURE;
		goto done;
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

/**
 * @brief Check that writes to the MPQ79500 land before programming it.
 *
 * PEC is enabled in OTP, so a write with a bad checksum is ACKed and then
 * dropped. Write BANK_SEL and read it back to catch that before any real
 * configuration goes out.
 *
 * The write and its readback share one PMBUS1 ownership. If we release the
 * bus between them, the System Controller could set BANK_SEL itself, so the
 * readback would not say whether our write landed.
 *
 * @param I2cAddr 7-bit I2C slave address.
 *
 * @return XST_SUCCESS when the write path is good, otherwise an XStatus
 *         error code. A loss of the shared bus surfaces as XST_IIC_ARB_LOST.
 */
XStatus MPQ79500_SelfTest(const u8 I2cAddr)
{
	u8 Prot1 = 0U;
	u8 Prot2 = 0U;
	u8 ProtMon1 = 0U;
	u8 Back = 0U;
	XStatus Status = XST_FAILURE;

	Status = Vek385_I2cAcquirePmbus1();
	if (XST_SUCCESS != Status) {
		xil_printf("ERROR: write-test @0x%02x: PMBUS1 not available\r\n",
			   I2cAddr);
		goto done;
	}

	/*
	 * The protect registers are logged for debug only. A protected group
	 * drops threshold writes, but Vmon_VerifyChannel() already catches
	 * that by reading the window back after programming, so there is no
	 * second gate on them here.
	 */
	(void)Vek385_I2cRegReadLocked(I2cAddr, MPQ79500_PROT1_REG, &Prot1);
	(void)Vek385_I2cRegReadLocked(I2cAddr, MPQ79500_PROT2_REG, &Prot2);
	(void)Vek385_I2cRegReadLocked(I2cAddr, MPQ79500_PROT_MON1_REG, &ProtMon1);
	VMON_DBG("DBG: prot @0x%02x: PROT1=0x%02x PROT2=0x%02x PROT_MON1=0x%02x\r\n",
		 I2cAddr, Prot1, Prot2, ProtMon1);

	Status = Mpq_SmbusWriteByte(I2cAddr, MPQ79500_BANK_SEL_REG,
				    MPQ79500_BANK_SEL_TEST_VAL);
	if (XST_SUCCESS != Status) {
		xil_printf("ERROR: write-test @0x%02x: BANK_SEL write not ACKed\r\n",
			   I2cAddr);
		goto done;
	}

	Status = Vek385_I2cRegReadLocked(I2cAddr, MPQ79500_BANK_SEL_REG, &Back);
	if (XST_SUCCESS != Status) {
		goto restore;
	}
	if (Back != MPQ79500_BANK_SEL_TEST_VAL) {
		xil_printf("ERROR: write-test @0x%02x: BANK_SEL readback=0x%02x, "
			   "write dropped\r\n", I2cAddr, Back);
		Status = XST_FAILURE;
		goto restore;
	}
	VMON_DBG("DBG: write-test @0x%02x: writes OK (PEC byte applied)\r\n",
		 I2cAddr);

restore:
	/*
	 * Put the bank back whichever way the readback went. Leaving bank 1
	 * selected would send every later access to the wrong register.
	 */
	if (XST_SUCCESS != Mpq_SmbusWriteByte(I2cAddr, MPQ79500_BANK_SEL_REG,
					      MPQ79500_BANK_SEL_DEFAULT_VAL)) {
		xil_printf("ERROR: write-test @0x%02x: bank restore failed\r\n",
			   I2cAddr);
		Status = XST_FAILURE;
	}

done:
	return Status;
}

/**
 * @brief Write the OV/UV thresholds for one channel.
 *
 * Writes thresholds only. The channel stays off the interrupt line until
 * MPQ79500_ArmChannel() runs. That lets the caller read the window back and
 * check it before a bad window can pull NIRQ low.
 *
 * @param Cfg Channel configuration (address, channel, target mV, margin).
 *
 * @return XST_SUCCESS on success, otherwise an XStatus error code. A loss of
 *         the shared bus surfaces as XST_IIC_ARB_LOST.
 */
XStatus MPQ79500_ProgramChannel(const MPQ79500_ChannelCfg *Cfg)
{
	u16 UvMv;
	u16 OvMv;
	u8 UvReg;
	u8 OvReg;
	u16 Target;
	u16 MarginMv;
	XStatus Status = XST_FAILURE;

	if ((NULL == Cfg) || !MPQ79500_CHANNEL_IS_VALID(Cfg->Channel)) {
		goto done;
	}

	Target = Cfg->TargetMv;
	MarginMv = (u16)((Target * Cfg->MarginPct) / 100U);

	/*
	 * OV/UV window = target +/- MarginPct (for example 2% on VEK385).
	 * Register codes use 5 mV steps, so the stored limits may round down.
	 * FPD 880 mV at 2% gives UV 863 -> 860 mV and OV 897 -> 895 mV.
	 */
	/* UV limit sits below the target; do not go below the chip minimum. */
	UvMv = (Target > MarginMv) ? (u16)(Target - MarginMv) :
				     (u16)MPQ79500_MV_OFFSET;
	/* OV limit sits above the target. */
	OvMv = (u16)(Target + MarginMv);
	UvReg = Mpq_MvToRegCode(UvMv);
	OvReg = Mpq_MvToRegCode(OvMv);

	VMON_DBG("DBG: program @0x%02x CH%u target=%u mV margin=%u%% "
		 "UV=%u(0x%02x) OV=%u(0x%02x)\r\n",
		 Cfg->I2cAddr, Cfg->Channel, Target, Cfg->MarginPct,
		 UvMv, UvReg, OvMv, OvReg);

	Status = Mpq_SelectDefaultVoltageRange(Cfg->I2cAddr, Cfg->Channel);
	if (XST_SUCCESS != Status) {
		Mpq_LogRegError("vrange_mult", Cfg->I2cAddr,
				MPQ79500_VRANGE_MULT_REG, Status);
		goto done;
	}

	Status = Mpq_WriteBankReg(Cfg->I2cAddr, MPQ79500_BANK_CONFIG,
				  Mpq_ChannelRegAddr(Cfg->Channel,
						     MPQ79500_UV_HF_OFF),
				  UvReg);
	if (XST_SUCCESS != Status) {
		Mpq_LogRegError("uv_hf", Cfg->I2cAddr,
				Mpq_ChannelRegAddr(Cfg->Channel,
						   MPQ79500_UV_HF_OFF),
				Status);
		goto done;
	}

	Status = Mpq_WriteBankReg(Cfg->I2cAddr, MPQ79500_BANK_CONFIG,
				  Mpq_ChannelRegAddr(Cfg->Channel,
						     MPQ79500_OV_HF_OFF),
				  OvReg);
	if (XST_SUCCESS != Status) {
		Mpq_LogRegError("ov_hf", Cfg->I2cAddr,
				Mpq_ChannelRegAddr(Cfg->Channel,
						   MPQ79500_OV_HF_OFF),
				Status);
		goto done;
	}

	Status = Mpq_WriteBankReg(Cfg->I2cAddr, MPQ79500_BANK_CONFIG,
				  Mpq_ChannelRegAddr(Cfg->Channel,
						     MPQ79500_UV_LF_OFF),
				  UvReg);
	if (XST_SUCCESS != Status) {
		Mpq_LogRegError("uv_lf", Cfg->I2cAddr,
				Mpq_ChannelRegAddr(Cfg->Channel,
						   MPQ79500_UV_LF_OFF),
				Status);
		goto done;
	}

	Status = Mpq_WriteBankReg(Cfg->I2cAddr, MPQ79500_BANK_CONFIG,
				  Mpq_ChannelRegAddr(Cfg->Channel,
						     MPQ79500_OV_LF_OFF),
				  OvReg);
	if (XST_SUCCESS != Status) {
		Mpq_LogRegError("ov_lf", Cfg->I2cAddr,
				Mpq_ChannelRegAddr(Cfg->Channel,
						   MPQ79500_OV_LF_OFF),
				Status);
		goto done;
	}

	Status = Mpq_WriteBankReg(Cfg->I2cAddr, MPQ79500_BANK_CONFIG,
				  Mpq_ChannelRegAddr(Cfg->Channel,
						     MPQ79500_FLT_HF_OFF),
				  MPQ79500_FLT_HF_CFG);
	if (XST_SUCCESS != Status) {
		Mpq_LogRegError("flt_hf", Cfg->I2cAddr,
				Mpq_ChannelRegAddr(Cfg->Channel,
						   MPQ79500_FLT_HF_OFF),
				Status);
		goto done;
	}

	Status = Mpq_WriteBankReg(Cfg->I2cAddr, MPQ79500_BANK_CONFIG,
				  Mpq_ChannelRegAddr(Cfg->Channel,
						     MPQ79500_FC_LF_OFF),
				  MPQ79500_FC_LF_CFG);
	if (XST_SUCCESS != Status) {
		Mpq_LogRegError("fc_lf", Cfg->I2cAddr,
				Mpq_ChannelRegAddr(Cfg->Channel,
						   MPQ79500_FC_LF_OFF),
				Status);
		goto done;
	}

	/*
	 * Turn the channel on so its VIN level can be read back before arming.
	 * The IEN_* bits stay clear until MPQ79500_ArmChannel(). NIRQ needs
	 * both, so a fault still cannot reach the pin yet.
	 */
	Status = Mpq_SetConfigRegBits(Cfg->I2cAddr, MPQ79500_VIN_CH_EN_REG,
				      Mpq_ChannelBitMask(Cfg->Channel));
	if (XST_SUCCESS != Status) {
		Mpq_LogRegError("vin_ch_en", Cfg->I2cAddr,
				MPQ79500_VIN_CH_EN_REG, Status);
		goto done;
	}

	VMON_DBG("DBG: programmed @0x%02x CH%u: target=%u mV UV=%u mV OV=%u mV\r\n",
		 Cfg->I2cAddr, Cfg->Channel, Target,
		 Mpq_RegCodeToMv(UvReg), Mpq_RegCodeToMv(OvReg));

	Status = XST_SUCCESS;

done:
	return Status;
}

/**
 * @brief Let a programmed channel's faults reach the interrupt line.
 *
 * Enables the four fault interrupts for the channel. NIRQ needs these bits
 * and the VIN_CH_EN bit set by MPQ79500_ProgramChannel(). Until then a
 * latched fault does not drive the pin. Call this only after the window has
 * been read back.
 *
 * @param I2cAddr 7-bit I2C slave address of the device.
 * @param Channel Channel number to arm.
 *
 * @return XST_SUCCESS on success, otherwise an XStatus error code. A loss of
 *         the shared bus surfaces as XST_IIC_ARB_LOST.
 */
XStatus MPQ79500_ArmChannel(const u8 I2cAddr, const u8 Channel)
{
	XStatus Status = XST_FAILURE;
	u8 ChBit;
	u32 Idx;
	static const struct {
		u8 Reg;			/**< Interrupt-enable register address. */
		const char *Name;	/**< Name used in the error log. */
	} IenRegs[] = {
		{ MPQ79500_IEN_UVHF_REG, "ien_uvhf" },
		{ MPQ79500_IEN_UVLF_REG, "ien_uvlf" },
		{ MPQ79500_IEN_OVHF_REG, "ien_ovhf" },
		{ MPQ79500_IEN_OVLF_REG, "ien_ovlf" }
	};
	const u32 IenCount = (u32)(sizeof(IenRegs) / sizeof(IenRegs[0]));

	if (!MPQ79500_CHANNEL_IS_VALID(Channel)) {
		goto done;
	}

	ChBit = Mpq_ChannelBitMask(Channel);

	for (Idx = 0U; Idx < IenCount; Idx++) {
		Status = Mpq_SetConfigRegBits(I2cAddr, IenRegs[Idx].Reg, ChBit);
		if (XST_SUCCESS != Status) {
			Mpq_LogRegError(IenRegs[Idx].Name, I2cAddr,
					IenRegs[Idx].Reg, Status);
			goto done;
		}
	}

	VMON_DBG("DBG: armed @0x%02x CH%u on NIRQ\r\n", I2cAddr, Channel);

done:
	return Status;
}

/**
 * @brief Move the monitor from the idle state to the active state.
 *
 * At power-up the MPQ79500 sits in the idle state. There it watches
 * OV only and masks UV, assuming the rails are off. Writing REC_START
 * with SEQ = power-on moves it to the active state, so both OV and UV
 * are monitored.
 *
 * @param I2cAddr 7-bit I2C slave address of the device.
 *
 * @return XST_SUCCESS on success, otherwise an XStatus error code. A loss of
 *         the shared bus surfaces as XST_IIC_ARB_LOST.
 */
XStatus MPQ79500_EnterActive(const u8 I2cAddr)
{
	return Mpq_WriteBankReg(I2cAddr, MPQ79500_BANK_CONFIG,
				MPQ79500_SEQ_REC_CTL_REG,
				MPQ79500_REC_START_SEQ_ON);
}

/**
 * @brief Read live VIN level for a monitor channel.
 *
 * @param I2cAddr 7-bit I2C slave address.
 * @param Channel Monitor channel (MPQ79500_CHANNEL_MIN..MPQ79500_CHANNEL_MAX).
 * @param LevelMv Pointer to receive voltage in mV.
 *
 * @return XST_SUCCESS on success, otherwise an XStatus error code. A loss of
 *         the shared bus surfaces as XST_IIC_ARB_LOST.
 */
XStatus MPQ79500_ReadVinLevelMv(const u8 I2cAddr, const u8 Channel,
				u16 *const LevelMv)
{
	u8 Reg;
	XStatus Status = XST_FAILURE;

	if ((NULL == LevelMv) || !MPQ79500_CHANNEL_IS_VALID(Channel)) {
		goto done;
	}

	Status = Mpq_ReadBankReg(I2cAddr, MPQ79500_BANK_STATUS,
				 (u8)(MPQ79500_VIN_LVL_BASE +
				      (Channel - MPQ79500_CHANNEL_MIN)),
				 &Reg);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	*LevelMv = Mpq_RegCodeToMv(Reg);
	Status = XST_SUCCESS;

done:
	return Status;
}

/**
 * @brief Read programmed threshold voltage for a monitor channel.
 *
 * @param I2cAddr 7-bit I2C slave address.
 * @param Channel Monitor channel (MPQ79500_CHANNEL_MIN..MPQ79500_CHANNEL_MAX).
 * @param ThreshOff Threshold register offset (MPQ79500_UV_HF_OFF, etc.).
 * @param ThreshMv Pointer to receive threshold in mV.
 *
 * @return XST_SUCCESS on success, otherwise an XStatus error code. A loss of
 *         the shared bus surfaces as XST_IIC_ARB_LOST.
 */
XStatus MPQ79500_ReadThresholdMv(const u8 I2cAddr, const u8 Channel,
				 const u8 ThreshOff, u16 *const ThreshMv)
{
	u8 Reg;
	XStatus Status = XST_FAILURE;

	if ((NULL == ThreshMv) || !MPQ79500_CHANNEL_IS_VALID(Channel)) {
		goto done;
	}

	Status = Mpq_ReadBankReg(I2cAddr, MPQ79500_BANK_CONFIG,
				 Mpq_ChannelRegAddr(Channel, ThreshOff), &Reg);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	*ThreshMv = Mpq_RegCodeToMv(Reg);
	Status = XST_SUCCESS;

done:
	return Status;
}

/**
 * @brief Read one Bank-0 fault flag register and clear the channel's bit.
 *
 * @param I2cAddr 7-bit I2C slave address.
 * @param Reg Bank-0 flag register (INT_OVHF/OVLF/UVHF/UVLF).
 * @param ChBit Single-bit mask for the channel.
 * @param FaultOut Set to 1 if the channel bit is set.
 *
 * @return XST_SUCCESS on success, otherwise an XStatus error code. A loss of
 *         the shared bus surfaces as XST_IIC_ARB_LOST.
 */
static XStatus Mpq_ReadClearFaultBit(const u8 I2cAddr, const u8 Reg,
				     const u8 ChBit, u8 *const FaultOut)
{
	u8 Val = 0U;
	XStatus Status = XST_FAILURE;

	Status = Mpq_ReadBankReg(I2cAddr, MPQ79500_BANK_STATUS, Reg, &Val);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	if (0U != (Val & ChBit)) {
		*FaultOut = 1U;
		/* Write 1 to clear only this channel's latched fault bit. */
		Status = Mpq_WriteBankReg(I2cAddr, MPQ79500_BANK_STATUS, Reg,
					  ChBit);
	}

done:
	return Status;
}

/**
 * @brief Read (and clear) the OV/UV fault flags for one channel.
 *
 * @param I2cAddr 7-bit I2C slave address.
 * @param Channel Monitor channel (MPQ79500_CHANNEL_MIN..MPQ79500_CHANNEL_MAX).
 * @param OvFault Pointer to receive the over-voltage flag (1/0), or NULL.
 * @param UvFault Pointer to receive the under-voltage flag (1/0), or NULL.
 *
 * @return XST_SUCCESS on success, otherwise an XStatus error code. A loss of
 *         the shared bus surfaces as XST_IIC_ARB_LOST.
 */
XStatus MPQ79500_ReadClearOvUv(const u8 I2cAddr, const u8 Channel,
			       u8 *const OvFault, u8 *const UvFault)
{
	u8 ChBit;
	u8 Ov = 0U;
	u8 Uv = 0U;
	XStatus Status = XST_FAILURE;

	if (!MPQ79500_CHANNEL_IS_VALID(Channel)) {
		goto done;
	}

	ChBit = Mpq_ChannelBitMask(Channel);

	Status = Mpq_ReadClearFaultBit(I2cAddr, MPQ79500_INT_OVHF_REG, ChBit, &Ov);
	if (XST_SUCCESS != Status) {
		goto done;
	}
	Status = Mpq_ReadClearFaultBit(I2cAddr, MPQ79500_INT_OVLF_REG, ChBit, &Ov);
	if (XST_SUCCESS != Status) {
		goto done;
	}
	Status = Mpq_ReadClearFaultBit(I2cAddr, MPQ79500_INT_UVHF_REG, ChBit, &Uv);
	if (XST_SUCCESS != Status) {
		goto done;
	}
	Status = Mpq_ReadClearFaultBit(I2cAddr, MPQ79500_INT_UVLF_REG, ChBit, &Uv);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	if (NULL != OvFault) {
		*OvFault = Ov;
	}
	if (NULL != UvFault) {
		*UvFault = Uv;
	}
	Status = XST_SUCCESS;

done:
	return Status;
}
