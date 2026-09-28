<!--
Copyright (c) 2026 Advanced Micro Devices, Inc.  All rights reserved.
SPDX-License-Identifier: MIT
-->

# VID Voltage Monitor

## Brief

This app arms the on-board **MPQ79500** voltage monitor on the **VEK385** (Versal
Gen 2) with an over/under-voltage (OV/UV) window around each rail's target
voltage. PLM reports the rail's VID power state, and the app maps that state to
the target voltage.

Flow:
1. Query the **VCC_FPD** and **VCC_AIE** VID power state from PLM via the PM query
   API (`XPm_GetVidPowerState()` over IPI), then map each state to a voltage with
   the per-rail table in `voltage_monitor.c`. Those tables come from the VID power
   state lists in the application note. The power state counts only when the query
   returns `XST_SUCCESS`. Because the state is used as a table index, it is also
   range-checked: a state below 2 (0 and 1 are the rail OFF and ON modes), a state
   past the end of the table, or one whose table entry is 0 mV means PLM resolved
   something this board has no voltage for, so the app stops instead of arming on a
   guess. A return of `XPM_NO_VID_ADJUST` means the part is not VID
   characterized, so the app uses the rail's default voltage from `board.cdo`
   (880 mV for VCC_FPD, 800 mV for VCC_AIE) so the rail is still monitored, and
   prints why on the console. Any other query failure stops the app.
2. Reach the monitor over **I2C1** through the **PCA9541** bus arbiter and the
   **PCA9848** mux (PMBUS1), re-checking bus ownership on every access (the board
   System Controller shares this bus).
3. Program the OV/UV thresholds (target +/- margin) and enter the active state,
   then release PMBUS1 so the System Controller gets the shared bus back, and hold
   while the monitor watches the rail in hardware and pulls **PMBUS1_NIRQ** low on
   a fault. The release happens whether or not arming succeeded.

## Configurable settings

Everything tunable lives in **`src/board_vek385_vmon.h`**:

| Setting | Macro | Default |
|---|---|---|
| OV/UV margin around target (%) | `VEK385_VMON_MARGIN_PCT` | `2` |
| PCA9541 arbiter address | `VEK385_I2C1_ARB_ADDR` | `0x72` |
| PCA9848 mux address | `VEK385_I2C1_MUX_ADDR` | `0x71` |
| Mux channel for PMBUS1 | `VEK385_I2C1_MUX_CH_PMBUS1` | `0` |
| MPQ79500 monitor addresses | `VEK385_FPD_VMON_I2C_ADDR` / `VEK385_AIE_VMON_I2C_ADDR` | `0x05`/`0x06` |
| VCC_FPD monitor channel | `VEK385_FPD_VMON_CHANNEL` | `2` |
| VCC_AIE monitor channel | `VEK385_AIE_VMON_CHANNEL` | `1` |
| I2C1 clock (Hz) | `VEK385_I2C_SCLK_HZ` | `400000` |

Other notes:
- **Debug prints:** off by default. To turn on the `VMON_DBG(...)` traces, define
  `DEBUG_MODE` by hand in `src/board_vek385_vmon.h` and rebuild.

## How we validated (from the System Controller)

The bench check runs from the **System Controller (SC)**, not the RPU. The RPU app
only arms the monitor; the SC script then moves the rail and confirms the monitor
latches the fault. The script is **`misc/vmon_sc_test.py`**.

It drives the rail with `raft-pm-cmd setvoltage` and reads/clears the MPQ79500
fault latches over I2C with `i2ctransfer` (SMBus PEC byte added on every write). Per
iteration it runs three phases:
- **safe:** rail on target -> no latch.
- **OV:** push the rail above the OV edge -> OV latch, `PMBUS1_NIRQ` low.
- **UV:** drop the rail just below the UV edge (edge minus a small overshoot) with
  `raft-pm-cmd setvoltage` -> UV latch. UV only latches in the active state, so the
  script checks the channel is armed first.

## Steps

1. **Build the application** (standard sw_apps app build) to get
   `vid_voltage_monitor.elf`.
2. **Build the `BOOT.BIN`** with the RPU overlay CDO.
3. **Flash `BOOT.BIN`** to the board. Boot it and confirm the RPU console prints
   `rail(s) armed successfully`.
4. **Run the SC check:**
   - Connect to the SC console (VEK385 revB: `com2`) and log in.
   - Copy the script over:

     ```bash
     scp <user>@<ip>:<path to script> /tmp/
     ```
   - Run it on the SC (defaults: bus 3, FPD monitor at 0x05, channel 2, 2%
     margin). The target is read back from the chip, so it needs no argument:

     ```bash
     python3 /tmp/vmon_sc_test.py
     ```
   - Expect `done: N/N checks passed`.

> Validated on VEK385 using the board's System Controller.
