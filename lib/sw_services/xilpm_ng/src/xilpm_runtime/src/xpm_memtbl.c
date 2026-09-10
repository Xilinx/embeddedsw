/******************************************************************************
 * Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @file xpm_memtbl.c
 *
 * Memory-range reachability table writer.
 *
 * PLM is the sole writer of the runtime-mutable table fields:
 *   - per-row @c State  (Active/Inactive region reachability)
 *   - header  @c UpdateCounter (seqlock guarding torn reads)
 *
 * Reachability is recomputed for all valid rows on any relevant event
 * (seed pass after PMC CDO, OCM/TCM/DDRMC device state change, PLD power
 * event, PL init-finish). The hooks only READ live node/power state and
 * WRITE the table - they never issue an EEMI/PM call (recursion) and are
 * idempotent. PLM produces the table only; it does not consume it for its
 * own address validation.
 *
 * @note Gated for the versal_2ve_2vm platform via XPLMI_MEMTBL_ADDR,
 *       which is only defined in versal_2ve_2vm/xplmi_hw.h. xilpm_ng is a
 *       versal_2ve_2vm-only (server) library, matching neighboring code that
 *       references XPLMI_* PMC-RAM constants unguarded.
 *
 ******************************************************************************/

#include "xpm_memtbl.h"
#include "xpm_runtime_mem.h"
#include "xpm_mem.h"
#include "xpm_device.h"
#include "xpm_nodeid.h"
#include "xpm_common.h"
#include "xplmi_hw.h"

/*
 * Platform build guard: XPLMI_MEMTBL_ADDR / _LENGTH are defined only in the
 * versal_2ve_2vm xplmi_hw.h. xilpm_ng runtime sources are collected by a
 * collect_by_extension(*.c) glob with no platform filter, so if this file is
 * ever compiled for a platform whose xplmi_hw.h lacks the symbol (e.g. 2VP /
 * 2VP_P), the real bodies below are replaced by no-op stubs. This keeps the
 * unconditional call sites (xpm_runtime_api.c, xpm_runtime_device.c,
 * xpm_power.c) compiling and linking on any platform.
 */
#ifdef XPLMI_MEMTBL_ADDR

/*
 * Compile-time invariants:
 *  - The on-wire row must be exactly 16 B (EntrySize = 4 words) with no
 *    compiler padding, since the recompute loop strides by
 *    sizeof(XPm_MemTblEntry).
 *  - The xilpm_ng view of the table size (XPM_MEMTBL_SIZE_BYTES,
 *    xpm_memtbl.h) must stay equal to the xilplmi reservation length
 *    (XPLMI_MEMTBL_LENGTH, xplmi_hw.h); this .c includes both headers.
 */
_Static_assert(sizeof(XPm_MemTblEntry) == 16U,
	       "XPm_MemTblEntry must be exactly 16 bytes (no padding)");
_Static_assert(XPM_MEMTBL_SIZE_BYTES == XPLMI_MEMTBL_LENGTH,
	       "XPM_MEMTBL_SIZE_BYTES must equal XPLMI_MEMTBL_LENGTH");

/****************************************************************************/
/**
 * @brief  Decode the fixed-layout fields of table header word0.
 *
 * @param  Hdr0		Raw header word0 read from the table.
 * @param  Version	[out] Version[7:0].
 * @param  HdrSize	[out] HeaderSize[11:8] in words.
 * @param  EntSize	[out] EntrySize[15:12] in words.
 * @param  NumEnt	[out] NumEntries[31:16].
 *
 * @return None.
 *
 ****************************************************************************/
static inline void XPm_MemTblParseHdr0(u32 Hdr0, u32 *Version, u32 *HdrSize,
					  u32 *EntSize, u32 *NumEnt)
{
	*Version = (Hdr0 & XPM_MEMTBL_VERSION_MASK) >> XPM_MEMTBL_VERSION_SHIFT;
	*HdrSize = (Hdr0 & XPM_MEMTBL_HDRSIZE_MASK) >> XPM_MEMTBL_HDRSIZE_SHIFT;
	*EntSize = (Hdr0 & XPM_MEMTBL_ENTSIZE_MASK) >> XPM_MEMTBL_ENTSIZE_SHIFT;
	*NumEnt = (Hdr0 & XPM_MEMTBL_NUMENT_MASK) >> XPM_MEMTBL_NUMENT_SHIFT;
}

/****************************************************************************/
/**
 * @brief  Validate a table header word0 snapshot.
 *
 * Runs the present/sanity checks on a caller-supplied @p Hdr0 value so that the
 * same validated snapshot can be reused (no double-read of the header): the
 * primary present signal is NumEntries != 0 (an absent, zeroized region reads
 * 0), backed by a light sanity guard - the fixed header nibbles
 * (EntrySize=0x4, HeaderSize=0x2, Version) must match, NumEntries must be
 * within bounds, and the declared geometry must fit the reservation. These also
 * reject a stale 0xDEADBEEF read should the zeroize ever be bypassed.
 *
 * @param  Hdr0	Raw header word0 read from the table.
 *
 * @return 1U if the header is present and well-formed, 0U otherwise.
 *
 ****************************************************************************/
static inline u32 XPm_MemTblHdr0Valid(u32 Hdr0)
{
	u32 Valid = 0U;
	u32 Version;
	u32 HdrSize;
	u32 EntSize;
	u32 NumEnt;

	XPm_MemTblParseHdr0(Hdr0, &Version, &HdrSize, &EntSize, &NumEnt);

	/* Primary signal: a zeroized (absent) region reads NumEntries == 0. */
	if ((NumEnt != 0U) &&
	    /* Light sanity guard: partial/corrupt write and version mismatch. */
	    (XPM_MEMTBL_ENTSIZE_VAL == EntSize) &&
	    (XPM_MEMTBL_HDRSIZE_VAL == HdrSize) &&
	    (XPM_MEMTBL_VERSION_VAL == Version) &&
	    (NumEnt <= XPM_MEMTBL_MAX_ENTRIES) &&
	    (((HdrSize + (NumEnt * EntSize)) * 4U) <= XPM_MEMTBL_SIZE_BYTES)) {
		Valid = 1U;
	}

	return Valid;
}

/****************************************************************************/
/**
 * @brief  Compute whether a memory region is physically reachable now.
 *
 * Reachability core used by the per-row State refresh. It defers to the shared
 * reachability primitives in xpm_runtime_mem.c so that PLM's own validation and
 * this projection can never disagree: PL fabric via XPm_IsPlFabricReachable(),
 * and OCM/TCM/DDR via XPm_CheckMemDevRangeReachable().
 *
 * @param  RegionAddr	Region start address.
 * @param  RegionSize	Region size in bytes (>= 1).
 * @param  MemFlags	Row MemFlags nibble (Attr[27:24]); 1 => PL fabric.
 *
 * @return 1U if the region is reachable, 0U otherwise.
 *
 * @note   The non-PL path now inherits the cross-type-overlap rejection of the
 * 	   shared core (a stricter, fail-safe outcome).
 *
 ****************************************************************************/
static u32 XPm_MemTblRegionReachable(u64 RegionAddr, u64 RegionSize, u32 MemFlags)
{
	u32 Reachable;

	/*
	 * Only the defined MemFlags encodings are honoured; any undefined/reserved
	 * value falls through to a fail-safe Inactive (Reachable = 0U) rather than
	 * being treated as a device-memory region.
	 *
	 * EnableLog = 0U keeps the shared reachability core silent on this
	 * high-frequency recompute path (logs would spam).
	 */
	if (XPM_MEMTBL_MEMFLAGS_PL == MemFlags) {
		/* PL fabric: no parent controller; PLD ON + CFU EOS decides. */
		Reachable = XPm_IsPlFabricReachable();
	} else if (0U == MemFlags) {
		Reachable = (XPM_SUCCESS == XPm_CheckMemDevRangeReachable(RegionAddr, RegionSize, 0U)) ?
				1U :
				0U;
	} else {
		/* Undefined/reserved MemFlags => fail-safe Inactive. */
		Reachable = 0U;
	}

	return Reachable;
}

/****************************************************************************/
/**
 * @brief  Present-detection for the memory-range table.
 *
 * The PRIMARY present signal is NumEntries != 0. This is meaningful because
 * XPlmi_MemTblInit() zeroizes the reserved region (with ACTUAL zeros, not
 * the 0xDEADBEEF PZM pattern) before XPlm_ProcessPmcCdo on every non-IPU boot:
 * an absent table therefore reads NumEntries == 0, and only the configuration
 * CDO block_write produces a non-zero NumEntries. (On IPU the zeroize is
 * skipped so a preserved table survives; if it was never written, the region
 * carries no live table and NumEntries stays 0.)
 *
 * A light sanity guard is retained as defense against partial/corrupt writes
 * and a version mismatch: the fixed header nibbles (EntrySize=0x4,
 * HeaderSize=0x2, Version) must match, NumEntries must be within bounds, and
 * the declared geometry must fit the reservation. These also reject a stale
 * 0xDEADBEEF read should the zeroize ever be bypassed (EntrySize nibble 0xB).
 *
 * @return 1U if the table is present and well-formed, 0U otherwise.
 *
 ****************************************************************************/
u32 XPm_MemTblIsPresent(void)
{
	/* Read the header once and validate that single snapshot. */
	u32 Hdr0 = XPm_In32(XPLMI_MEMTBL_ADDR + XPM_MEMTBL_HDR0_OFFSET);

	return XPm_MemTblHdr0Valid(Hdr0);
}

/****************************************************************************/
/**
 * @brief  Recompute and write the State field of every valid table row.
 *
 * Present-gated (no-op / fail-safe if the table is absent or corrupt). The
 * whole pass is bracketed by the UpdateCounter seqlock: bumped to odd before
 * the first mutation and back to even after the last, so a concurrent
 * reader can detect and retry a torn read. Each row's State write is a
 * read-modify-write of the Attr word, preserving the tooling's MemFlags /
 * Reserved / AccessPerm bits.
 *
 * Reads only live node/power state; writes only the table. Idempotent.
 *
 * @return None.
 *
 ****************************************************************************/
void XPm_MemTblRecomputeAll(void)
{
	u32 Hdr0;
	u32 NumEnt;
	u32 UpdCnt;

	/*
	 * Read the header ONCE and validate that single snapshot; parse
	 * NumEntries from the SAME local value below (no second header read).
	 * Fail-safe: nothing to maintain if the table is absent/corrupt.
	 */
	Hdr0 = XPm_In32(XPLMI_MEMTBL_ADDR + XPM_MEMTBL_HDR0_OFFSET);
	if (0U == XPm_MemTblHdr0Valid(Hdr0)) {
		goto done;
	}

	NumEnt = (Hdr0 & XPM_MEMTBL_NUMENT_MASK) >> XPM_MEMTBL_NUMENT_SHIFT;

	/*
	 * SINGLE-WRITER INVARIANT: this recompute is the only writer of the table's
	 * runtime-mutable fields (per-row State + UpdateCounter). The seed pass and
	 * both FSM hooks (XPm_MemTblOnDeviceStateChange / OnPowerEvent) all run
	 * only in the cooperative PLM task context, are non-reentrant, and make no
	 * call that re-enters this recompute. The seqlock below therefore protects
	 * concurrent *readers* only; correctness assumes writers are never
	 * concurrent. A critical section would be required here only if that
	 * single-writer invariant is ever broken.
	 *
	 * ORDERING: correctness of the seqlock relies on PMC RAM being
	 * non-cacheable / strongly-ordered, so a reader observes the odd-counter
	 * store, then the row stores, then the even-counter store in program
	 * order (same idiom as the RTCA shared area).
	 *
	 * Seqlock: force UpdateCounter to a guaranteed-odd "write-in-progress"
	 * value before any mutation. (count + 1) | 1 is robust even if the
	 * counter was left odd on entry (e.g. a crash or IPU mid-update); a
	 * plain increment would only turn odd when the counter was even.
	 */
	UpdCnt = (XPm_In32(XPLMI_MEMTBL_ADDR + XPM_MEMTBL_UPDCNT_OFFSET) + 1U) | 1U;
	XPm_Out32(XPLMI_MEMTBL_ADDR + XPM_MEMTBL_UPDCNT_OFFSET, UpdCnt);

	for (u32 i = 0U; i < NumEnt; i++) {
		u32 EntryAddr = XPLMI_MEMTBL_ADDR +
				XPM_MEMTBL_ENTRY0_OFFSET +
				(u32)(i * (u32)sizeof(XPm_MemTblEntry));
		u32 Attr = XPm_In32(EntryAddr + XPM_MEMTBL_ENTRY_ATTR_OFFSET);
		u32 SizeUnits = XPm_In32(EntryAddr + XPM_MEMTBL_ENTRY_SIZE_OFFSET);
		u32 AddrHigh = XPm_In32(EntryAddr + XPM_MEMTBL_ENTRY_ADDRHIGH_OFFSET);
		u32 AddrLow = XPm_In32(EntryAddr + XPM_MEMTBL_ENTRY_ADDRLOW_OFFSET);
		u32 MemFlags = (Attr & XPM_MEMTBL_ENTRY_MEMFLAGS_MASK) >>
			       XPM_MEMTBL_ENTRY_MEMFLAGS_SHIFT;
		u64 RegionAddr = ((u64)AddrHigh << 32U) | (u64)AddrLow;
		u64 RegionSize = (u64)SizeUnits << XPM_MEMTBL_SIZE_SHIFT;
		u32 State;
		u32 NewAttr;

		/*
		 * Zero-size row: force State = Inactive via the same masked RMW
		 * (preserving MemFlags/Reserved/AccessPerm), so a stale ACTIVE
		 * State can never linger, then move on.
		 */
		if (0ULL == RegionSize) {
			NewAttr = (Attr & ~XPM_MEMTBL_ENTRY_STATE_MASK) |
				  ((XPM_MEMTBL_REGION_INACTIVE << XPM_MEMTBL_ENTRY_STATE_SHIFT) &
				   XPM_MEMTBL_ENTRY_STATE_MASK);
			XPm_Out32(EntryAddr + XPM_MEMTBL_ENTRY_ATTR_OFFSET, NewAttr);
			continue;
		}

		State = (1U == XPm_MemTblRegionReachable(RegionAddr, RegionSize, MemFlags)) ?
				XPM_MEMTBL_REGION_ACTIVE :
				XPM_MEMTBL_REGION_INACTIVE;

		/* RMW: update only State[31:28], preserve MemFlags/Reserved/AccessPerm. */
		NewAttr = (Attr & ~XPM_MEMTBL_ENTRY_STATE_MASK) |
			  ((State << XPM_MEMTBL_ENTRY_STATE_SHIFT) & XPM_MEMTBL_ENTRY_STATE_MASK);
		XPm_Out32(EntryAddr + XPM_MEMTBL_ENTRY_ATTR_OFFSET, NewAttr);
	}

	/* Seqlock: back to even (odd + 1) after the last mutation. */
	UpdCnt = UpdCnt + 1U;
	XPm_Out32(XPLMI_MEMTBL_ADDR + XPM_MEMTBL_UPDCNT_OFFSET, UpdCnt);

done:
	return;
}

/****************************************************************************/
/**
 * @brief  Device-state-change hook: recompute the table on OCM/TCM/DDRMC events.
 *
 * Called after a device's Node.State is updated. Filters to memory device
 * subclasses that affect region reachability (OCM/TCM banks and DDR memory
 * controllers) and triggers a full recompute. Ignored for all other devices.
 *
 * @param  DeviceId	Node ID of the device whose state changed.
 *
 * @return None.
 *
 ****************************************************************************/
void XPm_MemTblOnDeviceStateChange(u32 DeviceId)
{
	if (IS_MEM_DEV_OCM(DeviceId) || IS_MEM_DEV_TCM(DeviceId) || IS_MEM_DEV_DDRMC(DeviceId)) {
		XPm_MemTblRecomputeAll();
	}
}

/****************************************************************************/
/**
 * @brief  Power-event hook: recompute the table on PL power-domain events.
 *
 * Called from the power FSM. Filters to the PL power domain (the only path
 * that affects PL-fabric region reachability) and triggers a full recompute.
 * Ignored for all other power nodes.
 *
 * @param  NodeId	Node ID of the power node that changed.
 *
 * @return None.
 *
 ****************************************************************************/
void XPm_MemTblOnPowerEvent(u32 NodeId)
{
	if (PM_POWER_PLD == NodeId) {
		/*
		 * PLD power on/off changes PL-fabric reachability. CFU end-of-startup
		 * (EOS) and DFX reconfiguration, which leave the PLD domain powered
		 * and so raise no power event, are handled at PL init-finish instead.
		 */
		XPm_MemTblRecomputeAll();
	}
}

/****************************************************************************/
/**
 * @brief  Debug helper: dump the table header and every entry.
 *
 * Retained for future debugging - there are no callers, so the linker drops it
 * from the image. Reads the raw table at XPLMI_MEMTBL_ADDR and prints the
 * decoded header + each entry (base address included) so a boot log can be
 * cross-referenced with the source table. Corrupt entry counts are capped at
 * the reservation's maximum before iteration. The routine reads but does not
 * modify the table. Uses PmDbg, which is disabled by default; raise XPmLogLevel
 * to XPM_DBG_VAL (or switch to PmWarn) to enable.
 *
 * @param  Label	Non-NULL short tag identifying the dump call site.
 *
 * @return None.
 *
 ****************************************************************************/
void XPm_MemTblDump(const char *Label)
{
	u32 Hdr0 = XPm_In32(XPLMI_MEMTBL_ADDR + XPM_MEMTBL_HDR0_OFFSET);
	u32 UpdCnt = XPm_In32(XPLMI_MEMTBL_ADDR + XPM_MEMTBL_UPDCNT_OFFSET);
	u32 Version;
	u32 HdrSize;
	u32 EntSize;
	u32 NumEnt;
	u32 i;

	/* Decode the packed table format and entry count from header word zero */
	XPm_MemTblParseHdr0(Hdr0, &Version, &HdrSize, &EntSize, &NumEnt);

	PmDbg("MemTbl[%s] @0x%08x Hdr0=0x%08x Ver=%u HdrSz=%u EntSz=%u NumEnt=%u UpdCnt=0x%08x Present=%u\r\n",
	       Label, (u32)XPLMI_MEMTBL_ADDR, Hdr0, Version, HdrSize, EntSize,
	       NumEnt, UpdCnt, XPm_MemTblHdr0Valid(Hdr0));

	/* Bound the loop against a corrupt NumEntries. */
	if (NumEnt > XPM_MEMTBL_MAX_ENTRIES) {
		NumEnt = XPM_MEMTBL_MAX_ENTRIES;
	}

	/* Decode each fixed-stride entry from the reserved table region */
	for (i = 0U; i < NumEnt; i++) {
		u32 EntryAddr = XPLMI_MEMTBL_ADDR + XPM_MEMTBL_ENTRY0_OFFSET +
				(u32)(i * (u32)sizeof(XPm_MemTblEntry));
		u32 Attr = XPm_In32(EntryAddr + XPM_MEMTBL_ENTRY_ATTR_OFFSET);
		u32 SizeUnits = XPm_In32(EntryAddr + XPM_MEMTBL_ENTRY_SIZE_OFFSET);
		u32 AddrHigh = XPm_In32(EntryAddr + XPM_MEMTBL_ENTRY_ADDRHIGH_OFFSET);
		u32 AddrLow = XPm_In32(EntryAddr + XPM_MEMTBL_ENTRY_ADDRLOW_OFFSET);
		/* Split the packed attributes into state, flags, and permissions */
		u32 State = (Attr & XPM_MEMTBL_ENTRY_STATE_MASK) >> XPM_MEMTBL_ENTRY_STATE_SHIFT;
		u32 MemFlags = (Attr & XPM_MEMTBL_ENTRY_MEMFLAGS_MASK) >> XPM_MEMTBL_ENTRY_MEMFLAGS_SHIFT;
		u32 AccPerm = Attr & XPM_MEMTBL_ENTRY_ACCPERM_MASK;
		u64 RegionAddr = ((u64)AddrHigh << 32U) | (u64)AddrLow;
		u64 RegionSize = (u64)SizeUnits << XPM_MEMTBL_SIZE_SHIFT;
		u64 RegionEnd = (0ULL == RegionSize) ? RegionAddr :
				(RegionAddr + RegionSize - 1ULL);

		PmDbg("  [%u] Start=0x%08x%08x End=0x%08x%08x Size4K=0x%08x State=%u MemFlags=0x%08x AccPerm=0x%08x\r\n",
		       i, (u32)(RegionAddr >> 32U), (u32)RegionAddr,
		       (u32)(RegionEnd >> 32U), (u32)RegionEnd,
		       SizeUnits, State, MemFlags, AccPerm);
	}
}

#else /* XPLMI_MEMTBL_ADDR */

/*
 * No-op fallback for platforms without a mem-table reservation. The table
 * is never present, so present-detection returns 0U and the maintenance
 * entry points do nothing. Call sites remain unconditional.
 */
/****************************************************************************/
/**
 * @brief  Present-detection for the memory-range table (no-op fallback).
 *
 * Platforms without a mem-table reservation never carry a table, so this
 * stub always reports "absent".
 *
 * @return 0U always (table never present on this platform).
 *
 ****************************************************************************/
u32 XPm_MemTblIsPresent(void)
{
	return 0U;
}

/****************************************************************************/
/**
 * @brief  Recompute the table State fields (no-op fallback).
 *
 * No mem-table exists on this platform, so there is nothing to recompute.
 *
 * @return None.
 *
 ****************************************************************************/
void XPm_MemTblRecomputeAll(void)
{
	/* No mem-table on this platform. */
}

/****************************************************************************/
/**
 * @brief  Device-state-change hook (no-op fallback).
 *
 * @param  DeviceId	Node ID of the device whose state changed (unused).
 *
 * @return None.
 *
 ****************************************************************************/
void XPm_MemTblOnDeviceStateChange(u32 DeviceId)
{
	(void)DeviceId;
	/* No mem-table on this platform. */
}

/****************************************************************************/
/**
 * @brief  Power-event hook (no-op fallback).
 *
 * @param  NodeId	Node ID of the power node that changed (unused).
 *
 * @return None.
 *
 ****************************************************************************/
void XPm_MemTblOnPowerEvent(u32 NodeId)
{
	(void)NodeId;
	/* No mem-table on this platform. */
}

/****************************************************************************/
/**
 * @brief  Debug dump helper (no-op fallback).
 *
 * @param  Label	Short tag identifying the dump call site (unused).
 *
 * @return None.
 *
 ****************************************************************************/
void XPm_MemTblDump(const char *Label)
{
	(void)Label;
	/* No mem-table on this platform. */
}

#endif /* XPLMI_MEMTBL_ADDR */
