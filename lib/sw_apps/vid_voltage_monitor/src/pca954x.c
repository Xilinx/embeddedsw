/******************************************************************************
 * Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @file pca954x.c
 * @brief PCA9541 bus arbitration and PCA9848 channel select for VEK385 PMBUS1.
 *
 * PMBUS1 is shared with the board System Controller through a PCA9541
 * two-master arbiter, and the rail monitors sit behind a PCA9848 mux. Every
 * transfer goes through vek385_i2c_bus.c, so this file holds no controller
 * state and never touches the XIicPs driver.
 */
#include "pca954x.h"
#include "board_vek385_vmon.h"
#include "vek385_i2c_bus.h"
#include "sleep.h"
#include "xil_printf.h"
#include "xiltimer.h"
#include "xstatus.h"

/** @brief PCA9541 control register address. */
#define PCA9541_CONTROL			0x01U

/** @brief PCA9541 interrupt status register address. */
#define PCA9541_ISTAT			0x02U

/** @brief PCA9541 ISTAT: other master has requested the bus (test flag). */
#define PCA9541_ISTAT_NMYTEST		0x80U

/** @brief PCA9541 ISTAT: bus initialization finished. */
#define PCA9541_ISTAT_BUSINIT		0x02U

/** @brief PCA9541 ISTAT: downstream bus came up cleanly. */
#define PCA9541_ISTAT_BUSOK		0x04U

/** @brief PCA9541 ISTAT: bus ownership was lost. */
#define PCA9541_ISTAT_BUSLOST		0x08U

/** @brief PCA9541 CTL: this master owns the bus. */
#define PCA9541_CTL_MYBUS		0x01U

/** @brief PCA9541 CTL: inverted MYBUS bit. */
#define PCA9541_CTL_NMYBUS		0x02U

/** @brief PCA9541 CTL: bus is on. */
#define PCA9541_CTL_BUSON		0x04U

/** @brief PCA9541 CTL: inverted BUSON bit. */
#define PCA9541_CTL_NBUSON		0x08U

/** @brief PCA9541 CTL: bus initialization in progress. */
#define PCA9541_CTL_BUSINIT		0x10U

/** @brief PCA9541 CTL: inverted test-on bit. */
#define PCA9541_CTL_NTESTON		0x80U

/** @brief Mask of BUSON and NBUSON bits in PCA9541 control register. */
#define BUSON_MASK			(PCA9541_CTL_BUSON | PCA9541_CTL_NBUSON)

/** @brief Mask of MYBUS and NMYBUS bits in PCA9541 control register. */
#define MYBUS_MASK			(PCA9541_CTL_MYBUS | PCA9541_CTL_NMYBUS)

/** @brief Short arbiter retry delay (us) used while actively taking the bus. */
#define PCA9541_DELAY_SHORT_US		50U

/** @brief Long arbiter retry delay (us) used while waiting on the other master. */
#define PCA9541_DELAY_LONG_US		1000U

/** @brief Wait (us) before we stop asking and force bus ownership. */
#define PCA9541_ARB_TIMEOUT_US		1000000U

/** @brief Total wait (us) before the acquire gives up. */
#define PCA9541_TOTAL_TIMEOUT_US	2000000U

/**
 * @brief PCA9541 state-transition table indexed by CTL[3:0].
 * Maps current control register nibble to next write value.
 */
static const u8 Pca9541_Control[16] = {
	4, 0, 1, 5, 4, 4, 5, 5, 0, 0, 1, 1, 0, 4, 5, 1
};

/**
 * @brief Check if PCA9541 reports bus off (invalid BUSON encoding).
 *
 * @param Reg PCA9541 control register value.
 *
 * @return Non-zero if bus is off.
 */
static int Pca9541_BusOff(const u8 Reg)
{
	return ((0U == (Reg & BUSON_MASK)) || (BUSON_MASK == (Reg & BUSON_MASK)));
}

/**
 * @brief Check if this master owns the PCA9541 bus.
 *
 * @param Reg PCA9541 control register value.
 *
 * @return Non-zero if MYBUS encoding indicates ownership.
 */
static int Pca9541_MyBus(const u8 Reg)
{
	return ((0U == (Reg & MYBUS_MASK)) || (MYBUS_MASK == (Reg & MYBUS_MASK)));
}

/**
 * @brief Read one byte from PCA9541 register.
 *
 * The arbiter needs an SMBus read-byte: register pointer, repeated START,
 * then the data byte. A STOP in between drops the pointer and the part then
 * answers every read from CONTROL, so ISTAT looks like a copy of CONTROL.
 * Vek385_I2cRegReadLocked() holds that repeated START. It does not acquire,
 * which suits the arbiter: CONTROL answers whoever owns the downstream bus.
 *
 * @param Reg Register address.
 * @param Val Pointer to receive value.
 *
 * @return XST_SUCCESS on success, otherwise an XStatus error code. A loss of
 *         the shared bus surfaces as XST_IIC_ARB_LOST.
 */
static XStatus Pca9541_RegRead(const u8 Reg, u8 *const Val)
{
	return Vek385_I2cRegReadLocked(VEK385_I2C1_ARB_ADDR, Reg, Val);
}

/**
 * @brief Write one byte to PCA9541 register.
 *
 * @param Reg Register address.
 * @param Val Value to write.
 *
 * @return XST_SUCCESS on success, otherwise an XStatus error code. A loss of
 *         the shared bus surfaces as XST_IIC_ARB_LOST.
 */
static XStatus Pca9541_RegWrite(const u8 Reg, const u8 Val)
{
	u8 Buf[2];

	Buf[0] = Reg;
	Buf[1] = Val;
	return Vek385_I2cWriteRaw(VEK385_I2C1_ARB_ADDR, Buf, 2);
}

/**
 * @brief Read and print the PCA9541 arbiter state over time (debug aid).
 *
 * Samples the CONTROL (0x01) and ISTAT (0x02) registers. Both stay
 * reachable even when we do not own the downstream bus. Use it to see
 * whether the other master (board System Controller) is holding or
 * cycling PMBUS1.
 *
 * @param Samples Number of samples to read and print.
 * @param IntervalUs Delay between samples in microseconds (0 for none).
 */
static void Pca9541_DumpState(const u32 Samples,
			      const u32 IntervalUs)
{
	u32 i;
	u8 Ctl = 0U;
	u8 Istat = 0U;

	xil_printf("PCA9541 @0x%02x arbiter trace (%u samples):\r\n",
		   VEK385_I2C1_ARB_ADDR, Samples);

	for (i = 0U; i < Samples; i++) {
		if ((XST_SUCCESS != Pca9541_RegRead(PCA9541_CONTROL, &Ctl)) ||
		    (XST_SUCCESS != Pca9541_RegRead(PCA9541_ISTAT, &Istat))) {
			xil_printf("  sample %u: arbiter read failed\r\n", i);
		} else {
			xil_printf("  sample %u: CTL=0x%02x ISTAT=0x%02x -> "
				   "bus %s, owner %s%s [businit=%u busok=%u "
				   "buslost=%u ctl_businit=%u]\r\n",
				   i, Ctl, Istat,
				   (0U != Pca9541_BusOff(Ctl)) ? "OFF" : "ON",
				   (0U != Pca9541_MyBus(Ctl)) ? "ours" : "other master",
				   (0U != (Istat & PCA9541_ISTAT_NMYTEST)) ?
				   ", other master requesting" : "",
				   (0U != (Istat & PCA9541_ISTAT_BUSINIT)) ? 1U : 0U,
				   (0U != (Istat & PCA9541_ISTAT_BUSOK)) ? 1U : 0U,
				   (0U != (Istat & PCA9541_ISTAT_BUSLOST)) ? 1U : 0U,
				   (0U != (Ctl & PCA9541_CTL_BUSINIT)) ? 1U : 0U);
		}

		if (IntervalUs > 0U) {
			usleep(IntervalUs);
		}
	}
}

/**
 * @brief Report whether this master currently owns the PCA9541 bus.
 *
 * The other master can take PMBUS1 at any time, so callers must re-read the
 * arbiter rather than trust a cached flag. CONTROL stays reachable even when
 * the downstream bus belongs to the other master.
 *
 * @param Owned Set to 1 when the bus is on and owned here, 0 otherwise.
 * @param Ctl Returns the raw CONTROL value, for logging.
 *
 * @return XST_SUCCESS on success, otherwise an XStatus error code.
 */
XStatus Pca9541_CheckOwnership(u8 *const Owned, u8 *const Ctl)
{
	u8 Reg = 0U;
	XStatus Status = XST_FAILURE;

	Status = Pca9541_RegRead(PCA9541_CONTROL, &Reg);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	*Ctl = Reg;
	*Owned = ((0U == Pca9541_BusOff(Reg)) && (0U != Pca9541_MyBus(Reg))) ?
		 1U : 0U;

done:
	return Status;
}

/**
 * @brief Acquire exclusive I2C1 ownership via PCA9541 arbiter.
 *
 * Follows the upstream Linux i2c-mux-pca9541 arbitration state machine
 * (drivers/i2c/muxes/i2c-mux-pca9541.c).
 *
 * We share PMBUS1 with the board System Controller, so we may have to wait
 * for it. If the bus is off, take it once the other master stops asking. If
 * the other master owns it, keep requesting. If it has not let go after
 * PCA9541_ARB_TIMEOUT_US, force ownership with BUSINIT. Give up after
 * PCA9541_TOTAL_TIMEOUT_US.
 *
 *
 * @return XST_SUCCESS on success, XST_FAILURE on timeout or error.
 */
XStatus Pca9541_Acquire(void)
{
	u32 Elapsed = 0U;	/* real time waited since entry, in us */
	u32 SelectDelay;
	XTime Start;
	XTime Now;
	u8 Reg = 0U;
	u8 Istat = 0U;
	u8 Forced = 0U;
	u8 Contended = 0U;	/* other master was actively using the bus */
	u8 WrVal;
	XStatus WrSt;
	XStatus Status = XST_FAILURE;
#if defined(DEBUG_MODE)
	u8 DbgIter = 0U;	/* bound the per-iteration acquire trace */
	u8 After;
#endif

	VMON_DBG("DBG: PCA9541 @0x%02x acquire start\r\n", VEK385_I2C1_ARB_ADDR);

	/*
	 * Measure real elapsed time, not the sum of the sleeps. Each pass also
	 * spends time on I2C, so summing sleeps would make the timeouts fire too
	 * late.
	 */
	XTime_GetTime(&Start);

	while (Elapsed <= PCA9541_TOTAL_TIMEOUT_US) {
		Status = Pca9541_RegRead(PCA9541_CONTROL, &Reg);
		if (XST_SUCCESS != Status) {
			xil_printf("ERROR: PCA9541 read failed status=%d\r\n",
				   Status);
			goto done;
		}

		SelectDelay = PCA9541_DELAY_LONG_US;

		if (0U != Pca9541_BusOff(Reg)) {
			/*
			 * Bus is off. Take it unless the other master is
			 * requesting ownership (ISTAT NMYTEST) and we have not
			 * yet waited long enough.
			 */
			Status = Pca9541_RegRead(PCA9541_ISTAT, &Istat);
			if (XST_SUCCESS != Status) {
				xil_printf("ERROR: PCA9541 ISTAT read failed "
					   "status=%d\r\n", Status);
				goto done;
			}

			if ((0U == (Istat & PCA9541_ISTAT_NMYTEST)) ||
			    (Elapsed >= PCA9541_ARB_TIMEOUT_US)) {
				WrVal = (u8)(Pca9541_Control[Reg & 0x0FU] |
					     PCA9541_CTL_NTESTON);
				WrSt = Pca9541_RegWrite(PCA9541_CONTROL, WrVal);
#if defined(DEBUG_MODE)
				if (DbgIter < 10U) {
					After = 0xFFU;
					(void)Pca9541_RegRead(
							      PCA9541_CONTROL,
							      &After);
					VMON_DBG("DBG: acq busoff%s CTL=0x%02x "
						 "ISTAT=0x%02x wrote=0x%02x "
						 "wr_st=%d -> CTL=0x%02x\r\n",
						 (0U != Forced) ? " [F]" : "",
						 Reg, Istat, WrVal,
						 (int)WrSt, After);
					DbgIter++;
				}
#else
				(void)WrSt;
#endif
				SelectDelay = PCA9541_DELAY_SHORT_US;
			} else {
				SelectDelay = PCA9541_DELAY_LONG_US * 2U;
			}
		} else if (0U != Pca9541_MyBus(Reg)) {
			/* Bus is on and we own it: clear test/init and finish. */
			if (0U != (Reg & (PCA9541_CTL_NTESTON | PCA9541_CTL_BUSINIT))) {
				(void)Pca9541_RegWrite(PCA9541_CONTROL,
						       Reg & ~(PCA9541_CTL_NTESTON |
							       PCA9541_CTL_BUSINIT));
			}
			VMON_DBG("DBG: PCA9541 acquired CTL=0x%02x after %u us\r\n",
				 Reg, Elapsed);
			if (0U != Contended) {
				xil_printf("NOTE: PMBUS1 %s, waited %u us\r\n",
					   (0U != Forced) ? "taken by force" :
							    "handed over",
					   Elapsed);
			}
			Status = XST_SUCCESS;
			goto done;
		} else {
			/*
			 * Other master owns the bus. Request it, and force
			 * ownership once the arbitration timeout has expired.
			 */
			if (0U == Contended) {
				/* Print once per acquire, not on every poll. */
				xil_printf("NOTE: PMBUS1 busy, other master "
					   "(SC) owns it, waiting "
					   "(CTL=0x%02x)\r\n", Reg);
				Contended = 1U;
			}

			if (Elapsed >= PCA9541_ARB_TIMEOUT_US) {
				if (0U == Forced) {
					xil_printf("WARN: PCA9541 arbitration "
						   "timeout, forcing bus "
						   "ownership (CTL=0x%02x)\r\n",
						   Reg);
					Forced = 1U;
				}
				(void)Pca9541_RegWrite(PCA9541_CONTROL,
						       Pca9541_Control[Reg & 0x0FU] |
						       PCA9541_CTL_BUSINIT |
						       PCA9541_CTL_NTESTON);
			} else if (0U == (Reg & PCA9541_CTL_NTESTON)) {
				(void)Pca9541_RegWrite(PCA9541_CONTROL,
						       Reg | PCA9541_CTL_NTESTON);
			}
		}

		usleep(SelectDelay);

		XTime_GetTime(&Now);
		Elapsed = (u32)(((Now - Start) * 1000000U) / COUNTS_PER_SECOND);
	}

	xil_printf("ERROR: PCA9541 acquire timeout (last CTL=0x%02x)\r\n", Reg);
	/* Keep the post-mortem short: the operation has already failed. */
	Pca9541_DumpState(3U, 50000U);
	Status = XST_FAILURE;

done:
	return Status;
}

/**
 * @brief Release PCA9541 bus ownership.
 *
 *
 * @return XST_SUCCESS on success, otherwise an XStatus error code. A loss of
 *         the shared bus surfaces as XST_IIC_ARB_LOST.
 */
XStatus Pca9541_Release(void)
{
	u8 Reg;
	XStatus Status = XST_FAILURE;

	/*
	 * Upstream behaviour: if the bus is on and we own it, switch it off.
	 * The token stays with us. The other master picks it up from the
	 * bus-off state through plain arbitration, the same path we use at
	 * startup.
	 */
	Status = Pca9541_RegRead(PCA9541_CONTROL, &Reg);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	if ((0U == Pca9541_BusOff(Reg)) && (0U != Pca9541_MyBus(Reg))) {
		Status = Pca9541_RegWrite(PCA9541_CONTROL,
					  (u8)((Reg & PCA9541_CTL_NBUSON) >> 1));
		VMON_DBG("DBG: PCA9541 release turns bus off (CTL was 0x%02x)\r\n",
			 Reg);
	} else {
		VMON_DBG("DBG: PCA9541 release: nothing to do (CTL=0x%02x)\r\n",
			 Reg);
		Status = XST_SUCCESS;
	}

done:
	return Status;
}

/**
 * @brief Select PCA9848 downstream channel(s).
 *
 * @param ChannelMask One-hot channel select mask.
 *
 * @return XST_SUCCESS on success, otherwise an XStatus error code. A loss of
 *         the shared bus surfaces as XST_IIC_ARB_LOST.
 */
XStatus Pca9848_Select(const u8 ChannelMask)
{
	return Vek385_I2cWriteRaw(VEK385_I2C1_MUX_ADDR, &ChannelMask, 1);
}
