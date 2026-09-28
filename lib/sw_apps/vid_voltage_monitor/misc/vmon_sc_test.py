#!/usr/bin/env python3
# Copyright (c) 2026 Advanced Micro Devices, Inc. All rights reserved.
# SPDX-License-Identifier: MIT
"""
vmon_sc_test.py - OV/UV/safe cycle test for the VID voltage monitor, run on
the System Controller (SC).

Run this ON THE SC. It talks to the MPQ79500
voltage monitor over PMBUS1 and drives the rail with raft-pm-cmd, then checks
that the chip latches an over-voltage (OV) and under-voltage (UV) fault,
pulls PMBUS1_NIRQ low for it, and stays clean when the rail is in range.

Tools required: python3, i2c-tools and raft-pm-cmd. The PMBUS1_NIRQ pin is
read with gpioget from libgpiod; without it the pin checks are skipped.

The log is meant to read like a hand session: for every register access it
prints the i2ctransfer command, then underneath the byte it got back and what
that byte means. It also prints the live rail voltage before each check.

Why the tests are shaped this way:
  - OV  : raise the rail with `raft-pm-cmd setvoltage` to just past the OV edge
          (edge + overshoot, ~20 mV by default), so the rail sits just outside
          the window, not far above it.
  - UV  : drop the rail with `raft-pm-cmd setvoltage` to just below the UV edge
          (edge - overshoot). The chip sees a real under-voltage. UV only
          latches in the ACTIVE state (idle masks UV); the RPU app puts the chip
          in active during arming, so we just check the channel is armed first.
  - safe: put the rail back on target, clear the latches, confirm nothing
          re-latches.

The MPQ79500 needs a PEC (packet error check) byte on every write or it
silently drops the data. Reads need no PEC. We compute the PEC ourselves and
send raw transfers with `i2ctransfer -a` (the -a is needed because 0x05/0x06
sit in the reserved I2C address range).
"""

import argparse
import json
import subprocess
import sys
import time

# ---------------------------------------------------------------------------
# Register map (from mpq79500.c / mpq79500.h in the RPU app)
# ---------------------------------------------------------------------------
BANK_SEL_REG = 0xF0          # 0x00 = status (bank 0), 0x01 = config (bank 1)
BANK_STATUS = 0x00
BANK_CONFIG = 0x01

# Status bank (bank 0): INT_SRC is the chip's own interrupt summary. A set bit
# means the chip decided a fault and is driving its nINT pin. Reading it splits
# chip-side from pin/net-side: if INT_SRC is set but gpioget reads inactive, the
# alert is lost after the chip (U37 IO expander / MIO), not on the chip.
INT_SRC_REG = 0x10

# Status bank (bank 0): interrupt/latch registers, write-1-to-clear.
INT_UVHF_REG = 0x12
INT_UVLF_REG = 0x14
INT_OVHF_REG = 0x16
INT_OVLF_REG = 0x18
VIN_LVL_BASE = 0x40          # per channel: CH1=0x40, CH2=0x41

# Config bank (bank 1): per-channel threshold block.
#   reg = 0x20 + (channel-1)*0x10 + offset
CH_REG_STRIDE = 0x10
UV_HF_OFF = 0x0
OV_HF_OFF = 0x1
UV_LF_OFF = 0x2
OV_LF_OFF = 0x3

# Config bank (bank 1): VIN channel enable, one bit per channel. Arming needs
# the channel turned on here too, otherwise its fault does not drive NIRQ.
VIN_CH_EN_REG = 0x1E

# Config bank (bank 1): interrupt-enable registers, one bit per channel.
# The Bank-0 INT_* flags latch a fault no matter what, but the physical
# NIRQ/ALERT pin only pulls low for faults enabled here. The RPU app sets
# these in MPQ79500_ArmChannel; we must too, or NIRQ never fires.
IEN_UVHF_REG = 0x13
IEN_UVLF_REG = 0x14
IEN_OVHF_REG = 0x15
IEN_OVLF_REG = 0x16

# Active-state entry (same as the app's MPQ79500_EnterActive).
SEQ_REC_CTL_REG = 0xA0       # bank 1
REC_START_SEQ_ON = 0x80      # REC_START=1, SEQ=00b (power-on)

# Threshold / VIN encoding: mV = 200 + 5 * reg
MV_BASE = 200
MV_STEP = 5


def mv_to_reg(mv):
    if mv < MV_BASE:
        mv = MV_BASE
    reg = (mv - MV_BASE) // MV_STEP
    return 255 if reg > 255 else reg


def reg_to_mv(reg):
    return MV_BASE + reg * MV_STEP


def infer_target_mv(uv_mv, ov_mv, margin_pct):
    """Recover the target the app programmed, from the window the chip holds.

    The app derives the window as target +/- margin_pct and then truncates each
    edge onto the chip's 5 mV grid, so the window does not name the target
    directly. Targets come from the application note's power-state ladder and
    so sit on the same 5 mV grid: walk that grid and return the one target that
    reproduces this exact window. Returns None if no single target fits."""
    hits = []
    for t in range(MV_BASE, 1205, MV_STEP):
        m = (t * margin_pct) // 100
        if reg_to_mv(mv_to_reg(t - m)) == uv_mv and \
           reg_to_mv(mv_to_reg(t + m)) == ov_mv:
            hits.append(t)
    return hits[0] if len(hits) == 1 else None


def ch_reg(channel, offset):
    return 0x20 + (channel - 1) * CH_REG_STRIDE + offset


def ch_bit(channel):
    return 1 << (channel - 1)


# ---------------------------------------------------------------------------
# SMBus PEC (CRC-8: poly 0x07, init 0x00, no reflect, no final xor)
# ---------------------------------------------------------------------------
def crc8(data):
    crc = 0
    for b in data:
        crc ^= b
        for _ in range(8):
            crc = ((crc << 1) ^ 0x07) & 0xFF if (crc & 0x80) else (crc << 1) & 0xFF
    return crc


# ---------------------------------------------------------------------------
# I2C helpers via i2ctransfer
# ---------------------------------------------------------------------------
class Mpq:
    def __init__(self, bus, addr, verbose=True):
        self.bus = bus
        self.addr = addr
        self.verbose = verbose

    def _echo_cmd(self, args, note):
        if not self.verbose:
            return
        line = "  $ i2ctransfer -a -y %d %s" % (self.bus, " ".join(args))
        if note:
            line += "   # " + note
        print(line)

    def _echo_meaning(self, text):
        if self.verbose:
            print("      -> " + text)

    def _run(self, args, show=True):
        cmd = ["i2ctransfer", "-a", "-y", str(self.bus)] + args
        out = subprocess.run(cmd, capture_output=True, text=True)
        if out.returncode != 0:
            raise RuntimeError("i2ctransfer failed: %s\n%s" %
                               (" ".join(cmd), out.stderr.strip()))
        raw = out.stdout.strip()
        if self.verbose and show and raw:
            print("      out: %s" % raw)
        return raw

    def wr(self, reg, val, note="", show=True):
        """Write one register with a PEC byte."""
        pec = crc8([self.addr << 1, reg, val])
        args = ["w3@0x%02x" % self.addr, "0x%02x" % reg,
                "0x%02x" % val, "0x%02x" % pec]
        if show:
            self._echo_cmd(args, note)
        self._run(args, show=show)

    def rd(self, reg, note="", show=True):
        """Read one register (no PEC on reads). Returns the byte."""
        args = ["w1@0x%02x" % self.addr, "0x%02x" % reg, "r1@0x%02x" % self.addr]
        if show:
            self._echo_cmd(args, note)
        out = self._run(args, show=show)
        return int(out.split()[0], 16)

    def bank(self, b):
        # Bank select is internal plumbing; keep it out of the log to cut noise.
        self.wr(BANK_SEL_REG, b, show=False)

    def enter_active(self):
        """Move idle -> active so UV is monitored (app does this too)."""
        self.bank(BANK_CONFIG)
        self.wr(SEQ_REC_CTL_REG, REC_START_SEQ_ON,
                note="SEQ_REC_CTL = 0x80 (REC_START, enter active)")

    def arm_alert(self, channel):
        """Arm the channel so a latched fault pulls NIRQ low. Matches the RPU
        app's arming: enable the channel in VIN_CH_EN, then enable the four
        OV/UV interrupts. All Bank-1, read-modify-write OR so other channels
        are left alone. Prints the read-back so the current mask is visible."""
        bit = ch_bit(channel)
        self.bank(BANK_CONFIG)
        for name, reg in (("VIN_CH_EN", VIN_CH_EN_REG),
                          ("IEN_UVHF", IEN_UVHF_REG), ("IEN_UVLF", IEN_UVLF_REG),
                          ("IEN_OVHF", IEN_OVHF_REG), ("IEN_OVLF", IEN_OVLF_REG)):
            cur = self.rd(reg, note="%s read (bank 1)" % name)
            self._echo_meaning("0x%02x  ->  %s before" % (cur, name))
            self.wr(reg, cur | bit,
                    note="%s |= 0x%02x (enable CH%d)" % (name, bit, channel))

    def read_vin_mv(self, channel):
        self.bank(BANK_STATUS)
        reg = VIN_LVL_BASE + (channel - 1)
        val = self.rd(reg, note="VIN_LVL CH%d  (mV = 200 + 5*val)" % channel)
        mv = reg_to_mv(val)
        self._echo_meaning("0x%02x  ->  VIN = %d mV (%.3f V)" %
                           (val, mv, mv / 1000.0))
        return mv

    def read_latches(self, channel):
        """Read the four fault latches for this channel and explain each."""
        bit = ch_bit(channel)
        self.bank(BANK_STATUS)
        res = {}
        table = (("UVHF", INT_UVHF_REG), ("UVLF", INT_UVLF_REG),
                 ("OVHF", INT_OVHF_REG), ("OVLF", INT_OVLF_REG))
        for name, reg in table:
            val = self.rd(reg, note="INT_%s  (CH%d bit = 0x%02x)" %
                          (name, channel, bit))
            latched = bool(val & bit)
            res[name] = latched
            self._echo_meaning("0x%02x  ->  %s %s" %
                               (val, name,
                                "LATCHED (fault seen)" if latched else "clear"))
        return res

    def read_int_src(self):
        """Read INT_SRC (bank 0, 0x10): the chip's own interrupt summary. A
        set bit means the chip decided a fault and is asserting nINT."""
        self.bank(BANK_STATUS)
        val = self.rd(INT_SRC_REG, note="INT_SRC (chip interrupt summary)")
        self._echo_meaning("0x%02x  ->  INT_SRC %s" %
                           (val, "set (chip IS asserting nINT)" if val
                            else "0 (chip is NOT asserting)"))
        return val

    def clear_latches(self, channel):
        """Write-1-to-clear every fault latch for this channel. Kept out of the
        log: it is bookkeeping between checks, not part of the result."""
        bit = ch_bit(channel)
        self.bank(BANK_STATUS)
        for reg in (INT_UVHF_REG, INT_UVLF_REG, INT_OVHF_REG, INT_OVLF_REG):
            self.wr(reg, bit, show=False)

    def read_thresholds(self, channel):
        """Read the four programmed threshold registers for this channel and
        return them in mV. This is what the chip is really enforcing, which is
        not the same as what --target and --margin predict: the app truncates
        each edge onto the chip's 5 mV grid, so the two differ by a few mV."""
        self.bank(BANK_CONFIG)
        out = {}
        for name, off in (("UV_HF", UV_HF_OFF), ("OV_HF", OV_HF_OFF),
                          ("UV_LF", UV_LF_OFF), ("OV_LF", OV_LF_OFF)):
            val = self.rd(ch_reg(channel, off),
                          note="%s CH%d  (mV = 200 + 5*val)" % (name, channel))
            out[name] = reg_to_mv(val)
            self._echo_meaning("0x%02x  ->  %s = %d mV" % (val, name, out[name]))
        return out

    def read_arm_state(self, channel):
        """True if the app already armed this channel: the VIN_CH_EN bit is set
        and all four IEN_* bits are set. The active state itself is not readable
        (REC_START self-clears to 0), so these enable bits - which the app sets
        in the same arming step - are the proxy for 'already armed'. Read only,
        no writes, so it never duplicates the app's arming."""
        bit = ch_bit(channel)
        self.bank(BANK_CONFIG)
        en = self.rd(VIN_CH_EN_REG, show=False) & bit
        ien = all((self.rd(r, show=False) & bit)
                  for r in (IEN_UVHF_REG, IEN_UVLF_REG,
                            IEN_OVHF_REG, IEN_OVLF_REG))
        return bool(en) and ien


# ---------------------------------------------------------------------------
# Rail control via raft-pm-cmd
# ---------------------------------------------------------------------------
def set_voltage(rail, volts, verbose=True):
    if verbose:
        print("  $ raft-pm-cmd setvoltage %s %.3f" % (rail, volts))
    cmd = ["raft-pm-cmd", "setvoltage", rail, "%.3f" % volts]
    out = subprocess.run(cmd, capture_output=True, text=True)
    if verbose and out.stdout.strip():
        print("      out: %s" % out.stdout.strip())
    if out.returncode != 0:
        raise RuntimeError("raft-pm-cmd failed: %s\n%s" %
                           (" ".join(cmd), out.stderr.strip()))


def get_voltage_mv(rail, verbose=True):
    if verbose:
        print("  $ raft-pm-cmd getvoltage %s" % rail)
    out = subprocess.run(["raft-pm-cmd", "getvoltage", rail],
                         capture_output=True, text=True)
    if verbose and out.stdout.strip():
        print("      out: %s" % out.stdout.strip())
    try:
        v = json.loads(out.stdout.strip())["Voltage"]
        return int(round(v * 1000))
    except (ValueError, KeyError, TypeError):
        return None


def show_voltage(rail, verbose=True):
    mv = get_voltage_mv(rail, verbose)
    if mv is None:
        print("  voltage now = (getvoltage failed)")
    else:
        print("  voltage now = %.3f V (%d mV)" % (mv / 1000.0, mv))
    return mv


# ---------------------------------------------------------------------------
# PMBUS1_NIRQ pin read (prove the alert instead of assuming it)
# ---------------------------------------------------------------------------
def _nirq_state(text, verbose=True):
    """Turn gpioget output into 'asserted' (pin low) or 'idle' (pin high).

    gpioinfo shows gpiochip3 line 14 = PMBUS1_NIRQ,
    active-high (libgpiod v2 prints no flag when it is the default), so the
    logical value equals the physical level. NIRQ is open-drain active-low, so:
      value 0 = 'inactive' = pin LOW  = ASSERTED (some device on the net)
      value 1 = 'active'   = pin HIGH = idle
    The pin is a wired-OR of several devices, so 'asserted' only means SOMEONE
    is asserting, not necessarily this chip."""
    tok = text.strip().split()
    val = (tok[-1] if tok else "").strip('"').lower()
    if "inactive" in val:          # value 0 = low = asserted (check before "active")
        return "asserted"
    if "active" in val:            # value 1 = high = idle
        return "idle"
    if val == "0":
        return "asserted"
    if val == "1":
        return "idle"
    if verbose:
        print("      -> could not parse gpioget output: %r" % text.strip())
    return None


def read_nirq(args, verbose=True):
    """Read the PMBUS1_NIRQ pin on the SC.

    Returns 'asserted' (pin low), 'idle' (pin high), or None if it could not be
    read. Tries the plain v1 form ('gpioget <chip> <line>') first, which is what
    the board's SC image (libgpiod v1) uses and which prints the raw
    0/1. The v2 forms ('--numeric -c', '-c') are kept only as a quiet fallback
    for a libgpiod v2 image. Only the command that works is printed, so the
    other forms do not spam the log with option errors. NIRQ is a shared
    wired-OR, so 'asserted' only proves SOME device is pulling it low."""
    chip = args.nirq_chip
    line = str(args.nirq_line)
    last_err = None
    for cmd in (["gpioget", chip, line],                     # libgpiod v1 (raw 0/1)
                ["gpioget", "--numeric", "-c", chip, line],  # libgpiod v2 (0/1)
                ["gpioget", "-c", chip, line]):              # libgpiod v2 (word)
        out = subprocess.run(cmd, capture_output=True, text=True)
        if out.returncode == 0:
            if verbose:
                print("  $ " + " ".join(cmd))
                print("      out: %s" % out.stdout.strip())
            return _nirq_state(out.stdout, verbose)
        if out.stderr.strip():
            last_err = out.stderr.strip().splitlines()[0]
    if verbose and last_err:
        print("  $ gpioget %s %s" % (chip, line))
        print("      -> %s" % last_err)
    return None


# ---------------------------------------------------------------------------
# Test phases
# ---------------------------------------------------------------------------
def any_uv(l):
    return l["UVHF"] or l["UVLF"]


def any_ov(l):
    return l["OVHF"] or l["OVLF"]


def phase_safe(mpq, args, edges):
    uv_edge, ov_edge = edges
    print("-- safe: rail on target, expect no fault --")
    set_voltage(args.rail, args.safe_mv / 1000.0)
    time.sleep(args.settle)
    show_voltage(args.rail)
    mpq.clear_latches(args.channel)
    time.sleep(args.settle)
    l = mpq.read_latches(args.channel)
    vin = mpq.read_vin_mv(args.channel)
    isrc = mpq.read_int_src()
    nirq = read_nirq(args, verbose=mpq.verbose)
    # The pin must also be idle. PMBUS1_NIRQ is a wired-OR across three
    # monitors and five sequencers, so a low pin during the safe phase means
    # the board has no working alert line, even when this monitor is clean.
    # Reading it and not judging it is how a stuck-low net stayed invisible.
    # None means the pin could not be read, which is not treated as a fault.
    ok = (not any_uv(l)) and (not any_ov(l)) and ("asserted" != nirq)
    print_check_summary("safe", args, edges, vin, l, isrc, nirq, ok)
    if any_uv(l) or any_ov(l):
        print("  safe FAIL reason: this monitor latched a fault at the safe "
              "level, so the rail is not inside the window it programmed")
    elif "asserted" == nirq:
        print("  safe FAIL reason: this monitor is clean but the shared "
              "PMBUS1_NIRQ is low, so something else is holding it")
    return ok, vin, l


def phase_ov(mpq, args, edges):
    uv_edge, ov_edge = edges
    print("-- OV: push rail above %d mV, expect OV latch --" % ov_edge)
    set_voltage(args.rail, args.ov_volts)
    time.sleep(args.settle)
    show_voltage(args.rail)
    vin = mpq.read_vin_mv(args.channel)
    l = mpq.read_latches(args.channel)
    isrc = mpq.read_int_src()
    nirq = read_nirq(args, verbose=mpq.verbose)
    # A latch alone does not prove the alert reaches the SC, so the pin must
    # be low too. None means the pin could not be read and is not judged.
    ok = any_ov(l) and ("idle" != nirq)
    print_check_summary("OV", args, edges, vin, l, isrc, nirq, ok)
    if any_ov(l) and "idle" == nirq:
        print("  OV FAIL reason: this monitor latched OV but PMBUS1_NIRQ is "
              "still high, so the alert did not reach the SC")
    # restore to target and clear
    set_voltage(args.rail, args.safe_mv / 1000.0)
    time.sleep(args.settle)
    mpq.clear_latches(args.channel)
    return ok, vin, l


def phase_uv(mpq, args, edges):
    uv_edge, ov_edge = edges
    print("-- UV: pull rail below %d mV, expect UV latch --" % uv_edge)
    set_voltage(args.rail, args.uv_volts)
    time.sleep(args.settle)
    show_voltage(args.rail)
    vin = mpq.read_vin_mv(args.channel)
    l = mpq.read_latches(args.channel)
    isrc = mpq.read_int_src()
    nirq = read_nirq(args, verbose=mpq.verbose)
    # Same rule as OV: the latch and a low pin, unless the pin is unreadable.
    ok = any_uv(l) and ("idle" != nirq)
    print_check_summary("UV", args, edges, vin, l, isrc, nirq, ok)
    if any_uv(l) and "idle" == nirq:
        print("  UV FAIL reason: this monitor latched UV but PMBUS1_NIRQ is "
              "still high, so the alert did not reach the SC")
    # restore to target and clear
    set_voltage(args.rail, args.safe_mv / 1000.0)
    time.sleep(args.settle)
    mpq.clear_latches(args.channel)
    return ok, vin, l


def fmt_latch(l):
    on = [k for k, v in l.items() if v]
    return ",".join(on) if on else "none"


def print_check_summary(kind, args, edges, vin, latches, isrc, nirq, ok):
    """One readable block per check: what was tested, what the chip saw, and
    where the alert (if any) came from."""
    uv_edge, ov_edge = edges
    if isrc:
        src = "this monitor 0x%02x CH%d (INT_SRC=0x%02x)" % (args.addr,
                                                             args.channel, isrc)
    else:
        src = "not this monitor (INT_SRC=0x00)"
    pin = {"asserted": "0  (low = alert asserted)",
           "idle": "1  (high = idle, no alert)"}.get(nirq, "unread")
    print("  +---------------- %-4s check summary ----------------" % kind)
    print("  | monitor       : 0x%02x  CH%d  (%s)"
          % (args.addr, args.channel, args.rail))
    print("  | window        : UV < %d mV   OV > %d mV   (read from the chip)"
          % (uv_edge, ov_edge))
    print("  | measured now  : %d mV   (safe level %d mV)"
          % (vin, args.safe_mv))
    print("  | latched fault : %s" % fmt_latch(latches))
    print("  | alert from    : %s" % src)
    print("  | SC alert pin  : %s line %d = %s"
          % (args.nirq_chip, args.nirq_line, pin))
    print("  | verdict       : %s" % ("PASS" if ok else "FAIL"))
    print("  +----------------------------------------------------")


def scan_net(args):
    """Find who is holding the shared PMBUS1_NIRQ line low.

    NIRQ is a wired-OR of several monitors and five sequencers, so a low pin
    only says "someone is asserting", not who. Read INT_SRC on each monitor
    address to see which monitor (if any) is the one pulling the line down. If
    the pin is low but every monitor reads 0x00, the holder is a sequencer
    (whose I2C addresses we do not have), not a monitor."""
    print("-- scan the shared PMBUS1_NIRQ net --")
    nirq = read_nirq(args, verbose=True)
    print("  shared pin: %s  (active-high: 'inactive'/0 = pin LOW = someone "
          "asserting)\n" % (nirq or "unread"))
    holders = []
    for addr in args.scan_addrs:
        m = Mpq(args.bus, addr, verbose=True)
        print("  monitor 0x%02x:" % addr)
        try:
            isrc = m.read_int_src()
        except RuntimeError as e:
            print("      -> no response (not present?): %s" % e)
            continue
        if isrc:
            holders.append(addr)
    print()
    if holders:
        print("  HOLDER(S): monitor(s) %s are asserting nINT"
              % ", ".join("0x%02x" % a for a in holders))
    elif nirq == "asserted":
        print("  pin is LOW but every monitor reads 0x00 -> a power sequencer "
              "is holding it, not a monitor")
    else:
        print("  net is clean: no monitor asserting and pin not low")
    return 0


def main():
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="Examples:\n"
               "  FPD (default): python3 vmon_sc_test.py\n"
               "  AIE          : python3 vmon_sc_test.py --addr 0x06 "
               "--channel 1 --rail VCC_AIE\n"
               "  (the target is read back from the chip, so neither run "
               "needs --target)\n")
    ap.add_argument("--bus", type=int, default=3, help="I2C bus (default 3)")
    ap.add_argument("--addr", type=lambda x: int(x, 0), default=0x05,
                    help="monitor I2C address (FPD=0x05, AIE=0x06)")
    ap.add_argument("--channel", type=int, default=2,
                    help="monitor channel (FPD=2, AIE=1)")
    ap.add_argument("--rail", default="VCC_FPD",
                    help="raft-pm-cmd rail name (VCC_FPD or VCC_AIE)")
    ap.add_argument("--target", type=int, default=None,
                    help="expected target in mV; only cross-checks the value "
                         "read back from the chip")
    ap.add_argument("--margin", type=int, default=2,
                    help="OV/UV window as %% of target (default 2, matches "
                         "VEK385_VMON_MARGIN_PCT in the app)")
    ap.add_argument("--overshoot-mv", type=int, default=20,
                    help="how far past the edge the OV/UV test drives the rail, "
                         "in mV (default 20). Kept small so the rail sits just "
                         "outside the window, not far from it.")
    ap.add_argument("--ov-volts", type=float, default=None,
                    help="override the OV setvoltage (default: OV edge + "
                         "overshoot, in volts)")
    ap.add_argument("--uv-volts", type=float, default=None,
                    help="override the UV setvoltage (default: UV edge - "
                         "overshoot, in volts)")
    ap.add_argument("--iterations", type=int, default=4,
                    help="number of OV/UV/safe cycles (default 4)")
    ap.add_argument("--settle", type=float, default=1.0,
                    help="seconds to wait after a rail/threshold change")
    ap.add_argument("--quiet", action="store_true",
                    help="hide the per-register command/decode lines")
    ap.add_argument("--nirq-chip", default="gpiochip3",
                    help="gpiochip that carries PMBUS1_NIRQ (default gpiochip3)")
    ap.add_argument("--nirq-line", type=int, default=14,
                    help="line offset for PMBUS1_NIRQ (default 14)")
    ap.add_argument("--scan", action="store_true",
                    help="only scan the shared PMBUS1_NIRQ net, then exit")
    ap.add_argument("--scan-addrs", dest="scan_addrs",
                    type=lambda x: [int(a, 0) for a in x.split(",")],
                    default=[0x05, 0x06],
                    help="monitor addresses to scan (default 0x05,0x06)")
    args = ap.parse_args()

    if args.scan:
        return scan_net(args)

    verbose = not args.quiet
    mpq = Mpq(args.bus, args.addr, verbose=verbose)

    print("VID voltage monitor OV/UV/safe test")
    print("  bus=%d addr=0x%02x ch=%d rail=%s"
          % (args.bus, args.addr, args.channel, args.rail))


    # The RPU app arms the chip in its Phase 2 (thresholds, channel + interrupt
    # enables, then active state). We do not repeat those writes; we only check
    # the channel is armed and arm as a fallback if the app has not run. The
    # active state is not readable (REC_START self-clears), so the enable bits
    # the app sets in the same step are the proxy for "already armed".
    print("-- check the chip is armed (the RPU app arms it in Phase 2) --")
    try:
        if mpq.read_arm_state(args.channel):
            print("  already armed by the app: CH%d enabled, OV/UV interrupts on"
                  "  ->  not re-writing\n" % args.channel)
        else:
            print("  WARNING: channel not armed (did the app run?); arming now")
            mpq.enter_active()
            mpq.arm_alert(args.channel)
            print("  armed by SC as a fallback\n")
    except RuntimeError as e:
        print("ERROR: could not check/arm the channel: %s" % e)
        return 2

    # Read the window the chip is actually enforcing instead of trusting the
    # --target/--margin guess. Without this a wrong window passes unnoticed,
    # which is how a bad target voltage once scored a full pass.
    print("-- read back the programmed window --")
    try:
        th = mpq.read_thresholds(args.channel)
    except RuntimeError as e:
        print("ERROR: could not read the programmed thresholds: %s" % e)
        return 2

    uv_edge, ov_edge = th["UV_HF"], th["OV_HF"]
    edges = (uv_edge, ov_edge)

    if (th["UV_LF"], th["OV_LF"]) != (uv_edge, ov_edge):
        print("  NOTE: HF and LF pairs disagree: HF %d..%d mV, LF %d..%d mV"
              % (uv_edge, ov_edge, th["UV_LF"], th["OV_LF"]))
    live_mv = mpq.read_vin_mv(args.channel)
    if not (uv_edge < live_mv < ov_edge):
        print("  ERROR: the live rail (%d mV) is already outside the programmed "
              "window %d..%d mV." % (live_mv, uv_edge, ov_edge))
        print("         The app armed a window its own rail does not sit in, so "
              "there is no safe level to return to.")
        return 2

    # --target is optional now, and only cross-checks what the app programmed.
    # Model the app exactly: floor the margin, then floor each edge onto the
    # 5 mV grid. Anything else reports a mismatch on a correct board.
    if args.target is not None:
        exp_m = (args.target * args.margin) // 100
        exp_uv = reg_to_mv(mv_to_reg(args.target - exp_m))
        exp_ov = reg_to_mv(mv_to_reg(args.target + exp_m))
        if (uv_edge, ov_edge) != (exp_uv, exp_ov):
            print("  NOTE: chip holds %d..%d mV while --target %d at %d%% "
                  "predicts %d..%d mV (delta %+d/%+d)."
                  % (uv_edge, ov_edge, args.target, args.margin,
                     exp_uv, exp_ov, uv_edge - exp_uv, ov_edge - exp_ov))
            print("        Using the chip's values for the checks below.")

    # Drive the rail just past each edge (edge +/- overshoot), not far past it,
    # so the test stays close to the window and does not swing the rail wide.
    if args.ov_volts is None:
        args.ov_volts = (ov_edge + args.overshoot_mv) / 1000.0
    if args.uv_volts is None:
        args.uv_volts = (uv_edge - args.overshoot_mv) / 1000.0

    tgt = infer_target_mv(uv_edge, ov_edge, args.margin)
    if tgt is None:
        tgt_txt = "target ~%dmV, window midpoint, no exact fit at %d%%" % (
            (uv_edge + ov_edge) // 2, args.margin)
    else:
        tgt_txt = "target %dmV, from the chip" % tgt

    # Park at the centre of the window, never at the measured rail: the rail can
    # already sit within a few mV of an edge, and raft-pm-cmd lands roughly
    # 5 mV high, which is enough to cross it and latch a fault in the safe leg.
    args.safe_mv = tgt if tgt is not None else (uv_edge + ov_edge) // 2
    print("  enforced window: UV<%dmV  OV>%dmV   (%s)"
          % (uv_edge, ov_edge, tgt_txt))

    fails = 0
    cleanup_ok = True
    try:
        for i in range(1, args.iterations + 1):
            print("======================= iteration %d/%d ======================="
                  % (i, args.iterations))
            for name, fn in (("safe", phase_safe), ("OV", phase_ov),
                             ("UV", phase_uv)):
                try:
                    ok, vin, l = fn(mpq, args, edges)
                except RuntimeError as e:
                    print("  %-4s ERROR: %s" % (name, e))
                    fails += 1
                    continue
                verdict = "PASS" if ok else "FAIL"
                if not ok:
                    fails += 1
                print("  RESULT %-4s : %s   vin=%dmV   latch=%s\n"
                      % (name, verdict, vin, fmt_latch(l)))
    finally:
        # Runs on Ctrl+C too, which can land while the rail sits outside the
        # window. Leave the rail on target and the latches clean.
        print("-- cleanup: rail back on target, clear latches --")
        try:
            set_voltage(args.rail, args.safe_mv / 1000.0)
            time.sleep(args.settle)
            mpq.clear_latches(args.channel)
        except RuntimeError as e:
            cleanup_ok = False
            print("  cleanup ERROR: %s" % e)
            print("  restore by hand: raft-pm-cmd setvoltage %s %.3f"
                  % (args.rail, args.safe_mv / 1000.0))

    total = args.iterations * 3
    print("\ndone: %d/%d checks passed" % (total - fails, total))
    return 1 if (fails or not cleanup_ok) else 0


if __name__ == "__main__":
    sys.exit(main())
