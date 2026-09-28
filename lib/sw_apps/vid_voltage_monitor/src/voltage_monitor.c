/******************************************************************************
 * Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @file voltage_monitor.c
 * @brief Program the MPQ79500 OV/UV window from the VID power state on VEK385.
 */
#include "board_vek385_vmon.h"
#include "mpq79500.h"
#include "pm_api_sys.h"
#include "voltage_monitor.h"
#include "sleep.h"
#include "xil_printf.h"
#include "xpm_nodeid.h"
#include "xstatus.h"

/** @brief Nominal VCC_FPD voltage, used when the part is not VID characterized. */
#define VMON_FPD_DEFAULT_MV	(880U)

/** @brief Nominal VCC_AIE voltage, used when the part is not VID characterized. */
#define VMON_AIE_DEFAULT_MV	(800U)

/**
 * @brief Number of entries in a rail's power-state voltage table.
 *
 * The tables are indexed by the power state the PLM reports. States 0 and 1
 * are reserved for OFF and ON, so no VID rail resolves to them. The highest
 * state any rail here uses is 18 (VCC_FPD).
 */
#define VMON_PWR_STATE_TBL_SIZE	(19U)

/**
 * @brief Lowest power state that maps to a VID voltage.
 *
 * Mode 0 turns a rail OFF and mode 1 turns it ON, so the first state VID can
 * resolve to is 2. Anything below this is not a VID result.
 */
#define VMON_PWR_STATE_FIRST	(2U)

/**
 * @brief Mask for the PLM module status inside a reply status.
 *
 * The PLM rewrites a non-success command return as (CdoErr << 16) with the
 * handler's own code in the low half, so an XPM_* value has to be masked out
 * of the reply before it can be compared.
 */
#define VMON_PLM_STATUS_MASK	(0xFFFFU)

/**
 * @brief Voltage in mV for each VCC_FPD power state.
 *
 * From the 17 VCC_FPD VID power states listed in the application note. Index
 * with the power state returned by XPm_GetVidPowerState(). States 0 and 1 are
 * reserved for OFF and ON, so the PLM never returns them and those entries
 * stay 0 to keep the indexing aligned.
 */
static const u16 Vmon_FpdStateMv[VMON_PWR_STATE_TBL_SIZE] = {
	0U,   0U,   800U, 805U, 810U, 815U, 820U, 825U, 830U, 835U,
	840U, 845U, 850U, 855U, 860U, 865U, 870U, 875U, 880U
};

/**
 * @brief Voltage in mV for each VCC_AIE power state.
 *
 * From the 16 VCC_AIE VID power states listed in the application note. Same
 * indexing as Vmon_FpdStateMv. VCC_AIE stops at state 17, so state 18 is 0.
 */
static const u16 Vmon_AieStateMv[VMON_PWR_STATE_TBL_SIZE] = {
	0U,   0U,   725U, 730U, 735U, 740U, 745U, 750U, 755U, 760U,
	765U, 770U, 775U, 780U, 785U, 790U, 795U, 800U, 0U
};

/** @brief Rail identifiers for the on-board MPQ79500 monitors. */
typedef enum {
	VMON_RAIL_FPD = 0,	/**< VCC_FPD  (PM_POWER_VCCINT_PSFP). */
	VMON_RAIL_AIE,		/**< VCC_AIE  (PM_POWER_VCCINT_ME). */
	VMON_RAIL_COUNT
} Vmon_RailId;

/** @brief Target voltages for monitored rails, mapped from the VID power state. */
typedef struct {
	u16 TargetMv[VMON_RAIL_COUNT];	/**< Target regulator voltage in mV per rail. */
} Vmon_TargetInfo;

/**
 * @brief Static rail metadata: PM node id, console name, power-state voltage
 *        table and the board.cdo default used when the part has no VID.
 */
static const struct {
	u32 RailId;		/**< PM power-node id passed to the query API. */
	const char *Name;	/**< Rail name used in console output. */
	const u16 *StateMv;	/**< Voltage per power state, from the application note. */
	u16 DefaultMv;		/**< Voltage used when the rail has no VID state. */
} Vmon_RailMeta[VMON_RAIL_COUNT] = {
	[VMON_RAIL_FPD] = { PM_POWER_VCCINT_PSFP, "VCC_FPD",
			    Vmon_FpdStateMv, VMON_FPD_DEFAULT_MV },
	[VMON_RAIL_AIE] = { PM_POWER_VCCINT_ME, "VCC_AIE",
			    Vmon_AieStateMv, VMON_AIE_DEFAULT_MV },
};

/**
 * @brief Query PLM for each rail's VID power state and map it to a voltage.
 *
 * A rail with no VID falls back to its default voltage.
 *
 * @param Info Filled with per-rail target mV.
 *
 * @return XST_SUCCESS when every rail has a target. XST_FAILURE if Info is
 *         NULL, a query failed, or the PLM reported a state this board has
 *         no voltage for.
 */
static XStatus Vmon_QueryVidTargets(Vmon_TargetInfo *const Info)
{
	u32 Rail;
	u32 PowerState;
	u16 TargetMv;
	XStatus Status = XST_FAILURE;
	XStatus RailStatus;

	if (NULL == Info) {
		goto done;
	}

	for (Rail = 0U; Rail < VMON_RAIL_COUNT; Rail++) {
		PowerState = 0U;

		RailStatus = XPm_GetVidPowerState(Vmon_RailMeta[Rail].RailId,
						  &PowerState);
		/* Each IPI handler may emit PLM UART text; wait before the next call. */
		usleep(VEK385_VMON_UART_IPI_SETTLE_US);

		if (XST_SUCCESS == RailStatus) {
			/*
			 * The state indexes the board table, so check it
			 * before use. Out of range covers
			 * XPM_VID_POWER_STATE_INVALID (0xFF) as well. A 0 mV
			 * entry means the PLM resolved a state this board has
			 * no voltage for, so the two disagree and arming on a
			 * guess would be worse than stopping.
			 */
			if ((PowerState < VMON_PWR_STATE_FIRST) ||
			    (PowerState >= VMON_PWR_STATE_TBL_SIZE) ||
			    (0U == Vmon_RailMeta[Rail].StateMv[PowerState])) {
				xil_printf("ERROR: %s VID power state %u is not "
					   "in the board table\r\n",
					   Vmon_RailMeta[Rail].Name, PowerState);
				goto done;
			}
			TargetMv = Vmon_RailMeta[Rail].StateMv[PowerState];
		} else if (XPM_NO_VID_ADJUST ==
			   ((u32)RailStatus & VMON_PLM_STATUS_MASK)) {
			/* Mask off the CDO command id the PLM packs into the top half. */
			TargetMv = Vmon_RailMeta[Rail].DefaultMv;
			xil_printf("%s    : part is not VID characterized; the application "
				   "uses the default voltage from board.cdo\r\n",
				   Vmon_RailMeta[Rail].Name);
			xil_printf("%s    : using nominal %u mV\r\n",
				   Vmon_RailMeta[Rail].Name, TargetMv);
		} else {
			xil_printf("ERROR: XPm_GetVidPowerState(%s) failed status=%d\r\n",
				   Vmon_RailMeta[Rail].Name, RailStatus);
			goto done;
		}

		Info->TargetMv[Rail] = TargetMv;

		VMON_DBG("DBG: %s -> state %u -> %u mV\r\n",
			 Vmon_RailMeta[Rail].Name, PowerState, TargetMv);
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

/**
 * @brief Print the target voltage picked for each rail.
 *
 * @param Info Filled by Vmon_QueryVidTargets (NULL is ignored).
 */
static void Vmon_PrintTargets(const Vmon_TargetInfo *Info)
{
	u32 Rail;

	if (NULL == Info) {
		return;
	}

	for (Rail = 0U; Rail < VMON_RAIL_COUNT; Rail++) {
		xil_printf("%s    : target %u mV\r\n", Vmon_RailMeta[Rail].Name,
			   Info->TargetMv[Rail]);
	}
}

/**
 * @brief A power rail (VCC_FPD or VCC_AIE) and the monitor that watches it.
 */
typedef struct {
	Vmon_RailId Rail;	/**< Rail id, selects the target voltage. */
	u8 I2cAddr;		/**< MPQ79500 7-bit I2C address. */
	u8 Channel;		/**< Monitor channel (1..6). */
	const char *Name;	/**< Rail name (e.g. "VCC_FPD"). */
} Vmon_Rail;

/**
 * @brief Verify programmed OV/UV thresholds bracket the target voltage.
 *
 * @param Addr 7-bit I2C slave address.
 * @param Channel Monitor channel (1..6).
 * @param TargetMv Expected target voltage in mV.
 * @param RailName Rail name for the log (e.g. "VCC_FPD").
 *
 * @return XST_SUCCESS if the window brackets the target, XST_FAILURE if a
 *         readback failed or the window is wrong.
 */
static XStatus Vmon_VerifyChannel(const u8 Addr, const u8 Channel,
			      const u16 TargetMv, const char *RailName)
{
	u16 ReadUv = 0U;
	u16 ReadOv = 0U;
	u16 ReadUvLf = 0U;
	u16 ReadOvLf = 0U;
	u16 ReadLevel = 0U;
	XStatus Status = XST_FAILURE;
	XStatus Err = XST_SUCCESS;

	Status = MPQ79500_ReadThresholdMv(Addr, Channel, MPQ79500_UV_HF_OFF,
					  &ReadUv);
	if (XST_SUCCESS != Status) {
		xil_printf("ERROR: read UV_HF CH%u failed\r\n", Channel);
		Err = XST_FAILURE;
		goto done;
	}

	Status = MPQ79500_ReadThresholdMv(Addr, Channel, MPQ79500_OV_HF_OFF,
					  &ReadOv);
	if (XST_SUCCESS != Status) {
		xil_printf("ERROR: read OV_HF CH%u failed\r\n", Channel);
		Err = XST_FAILURE;
		goto done;
	}

	/*
	 * ProgramChannel writes the same window into the LF pair as the HF
	 * pair, so read both back. Checking only HF would miss a dropped or
	 * wrong LF write.
	 */
	Status = MPQ79500_ReadThresholdMv(Addr, Channel, MPQ79500_UV_LF_OFF,
					  &ReadUvLf);
	if (XST_SUCCESS != Status) {
		xil_printf("ERROR: read UV_LF CH%u failed\r\n", Channel);
		Err = XST_FAILURE;
		goto done;
	}

	Status = MPQ79500_ReadThresholdMv(Addr, Channel, MPQ79500_OV_LF_OFF,
					  &ReadOvLf);
	if (XST_SUCCESS != Status) {
		xil_printf("ERROR: read OV_LF CH%u failed\r\n", Channel);
		Err = XST_FAILURE;
		goto done;
	}

	Status = MPQ79500_ReadVinLevelMv(Addr, Channel, &ReadLevel);
	if (XST_SUCCESS != Status) {
		xil_printf("WARN: VIN_LVL read CH%u failed\r\n", Channel);
		ReadLevel = 0U;
	}

	xil_printf("%s    : MPQ79500 @ 0x%02x, channel %u\r\n",
		   RailName, Addr, Channel);
	xil_printf("             measured %u mV, target %u mV\r\n",
		   ReadLevel, TargetMv);
	xil_printf("             OV/UV window %u to %u mV\r\n", ReadUv, ReadOv);

	/*
	 * UV must sit below target and OV above target on both HF and LF
	 * threshold pairs. If either pair straddles the target the window
	 * would trip immediately or never detect a real fault.
	 */
	if ((ReadUv >= TargetMv) || (ReadOv <= TargetMv) ||
	    (ReadUvLf >= TargetMv) || (ReadOvLf <= TargetMv)) {
		xil_printf("ERROR: programmed limits invalid for CH%u "
			   "(HF %u-%u, LF %u-%u mV)\r\n",
			   Channel, ReadUv, ReadOv, ReadUvLf, ReadOvLf);
		Err = XST_FAILURE;
	}

	/*
	 * Arm only when the rail is already inside the window. A rail below its
	 * nominal would trip PMBUS1_NIRQ as soon as we arm. ReadLevel is 0 when
	 * the read failed, so skip the check then.
	 */
	if ((0U != ReadLevel) &&
	    ((ReadLevel <= ReadUv) || (ReadLevel >= ReadOv))) {
		xil_printf("ERROR: CH%u measured %u mV is outside the "
			   "programmed window %u-%u mV\r\n",
			   Channel, ReadLevel, ReadUv, ReadOv);
		Err = XST_FAILURE;
	}

done:
	return Err;
}

/**
 * @brief Run the VID power state query and MPQ79500 OV/UV programming.
 *
 * The caller owns the I2C path: PMBUS1 must already be acquired on entry and
 * is left acquired on return. Every rail has to arm, so the first failure
 * stops the run.
 *
 * @return XST_SUCCESS when every rail is armed, otherwise an XStatus error
 *         code from the step that failed.
 */
XStatus Vmon_Example(void)
{
	Vmon_TargetInfo TargetInfo = {0};
	MPQ79500_ChannelCfg Cfg;
	Vmon_Rail Rails[] = {
		{ VMON_RAIL_FPD, VEK385_FPD_VMON_I2C_ADDR,
		  VEK385_FPD_VMON_CHANNEL, "VCC_FPD" },
		{ VMON_RAIL_AIE, VEK385_AIE_VMON_I2C_ADDR,
		  VEK385_AIE_VMON_CHANNEL, "VCC_AIE" },
	};
	u32 RailCount = (u32)(sizeof(Rails) / sizeof(Rails[0]));
	u32 Idx;
	XStatus Status = XST_FAILURE;

	xil_printf("\r\n== Phase 1: VID power state ==\r\n");

	Status = Vmon_QueryVidTargets(&TargetInfo);
	if (XST_SUCCESS != Status) {
		goto done;
	}
	/* Last IPI may still be printing; settle before the Phase-1 summary. */
	usleep(VEK385_VMON_UART_PHASE1_PRINT_SETTLE_US);
	Vmon_PrintTargets(&TargetInfo);

	/* Phase 1 gave every rail a target, or it failed and we never got here. */
	xil_printf("\r\n== Phase 2: arm OV/UV monitor ==\r\n");
	for (Idx = 0U; Idx < RailCount; Idx++) {
		/*
		 * Probe first. PMBUS1 is shared, so on a variant where the
		 * monitor is absent or moved we must not write BANK_SEL on
		 * whatever else answers at this address. Only self-test the
		 * write path once we know our part is there.
		 */
		Status = MPQ79500_Probe(Rails[Idx].I2cAddr);
		if (XST_SUCCESS != Status) {
			xil_printf("ERROR: %s monitor not found at 0x%02x\r\n",
				   Rails[Idx].Name, Rails[Idx].I2cAddr);
			goto done;
		}
		Status = MPQ79500_SelfTest(Rails[Idx].I2cAddr);
		if (XST_SUCCESS != Status) {
			xil_printf("ERROR: %s write path check failed\r\n",
				   Rails[Idx].Name);
			goto done;
		}

		Cfg.I2cAddr = Rails[Idx].I2cAddr;
		Cfg.Channel = Rails[Idx].Channel;
		Cfg.TargetMv = TargetInfo.TargetMv[Rails[Idx].Rail];
		Cfg.MarginPct = VEK385_VMON_MARGIN_PCT;
		Status = MPQ79500_ProgramChannel(&Cfg);
		if (XST_SUCCESS != Status) {
			xil_printf("ERROR: %s monitor program failed\r\n",
				   Rails[Idx].Name);
			goto done;
		}

		/*
		 * Read the thresholds and level back and check the window
		 * brackets the target. A write can be ACKed but not stored:
		 * bad PEC, wrong bank, or a protected register. Do not arm a
		 * rail whose window is wrong.
		 */
		Status = Vmon_VerifyChannel(Cfg.I2cAddr, Cfg.Channel,
					    Cfg.TargetMv, Rails[Idx].Name);
		if (XST_SUCCESS != Status) {
			xil_printf("ERROR: %s window verify failed\r\n",
				   Rails[Idx].Name);
			goto done;
		}

		/*
		 * OV/UV flags are write-1-to-clear and survive a JTAG reset.
		 * Clear them once so NIRQ starts clean.
		 */
		Status = MPQ79500_ReadClearOvUv(Cfg.I2cAddr, Cfg.Channel,
						NULL, NULL);
		if (XST_SUCCESS != Status) {
			xil_printf("ERROR: %s latch clear failed\r\n",
				   Rails[Idx].Name);
			goto done;
		}

		/* Window is checked and the latches are clear, so go live. */
		Status = MPQ79500_ArmChannel(Cfg.I2cAddr, Cfg.Channel);
		if (XST_SUCCESS != Status) {
			xil_printf("ERROR: %s arm failed\r\n",
				   Rails[Idx].Name);
			goto done;
		}

		/*
		 * The monitor powers up in the idle state, where it watches OV
		 * only and masks UV. Move it to the active state so both OV and
		 * UV are live. Half a window is not what this app promises, so
		 * a failure here stops the run like any other arming failure.
		 */
		Status = MPQ79500_EnterActive(Cfg.I2cAddr);
		if (XST_SUCCESS != Status) {
			xil_printf("ERROR: %s active-state entry failed; UV not "
				   "armed\r\n", Rails[Idx].Name);
			goto done;
		}
		xil_printf("             status: armed\r\n");
	}

	xil_printf("\r\nAll %u rail(s) armed successfully.\r\n", RailCount);
	Status = XST_SUCCESS;

done:
	return Status;
}
